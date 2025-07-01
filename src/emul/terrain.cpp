#include "terrain.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <vector>
#include <queue>

namespace Diligent
{

namespace SF_GLTF
{

// Forward declaration for Poisson disc sampling function
std::vector<float2> PoissonDiscSamplingForTile(float2 tileCenter, float tileWidth, float tileHeight, float minDistance, int baseSeed, int tileX, int tileY);

static std::unordered_map<std::string, float> defaultKeyValueConfig;

std::unordered_map<std::string, float>* TerrainGenerator::activeKeyValueConfig = &defaultKeyValueConfig;

// Simple noise function for deterministic rock placement
float RockNoise(float x, float y, float scale, int seed) {
    int ix = (int)(x * scale * 1000.0f) + seed * 1000000;
    int iy = (int)(y * scale * 1000.0f) + seed * 2000000;
    unsigned int hash = ((ix * 73856093) ^ (iy * 19349663) ^ (seed * 83492791)) & 0x7FFFFFFF;
    return ((float)hash / (float)0x7FFFFFFF) * 2.0f - 1.0f;
}

// Fractal noise for more natural patterns
float RockFractalNoise(float x, float y, int octaves, float persistence, float scale, int seed) {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = scale;
    
    for(int i = 0; i < octaves; i++) {
        value += amplitude * RockNoise(x, y, frequency, seed + i);
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    return value;
}

TerrainGenerator::TerrainGenerator(const std::vector<PlanetType>& planetTypes) : planetTypes(planetTypes) {
    // Constructor - could initialize random seed or other state if needed
}

// Cache management methods
void TerrainGenerator::ClearCache() {
    tileCache.clear();
}

void TerrainGenerator::ClearCacheForType(TileItemType itemType) {
    auto it = tileCache.begin();
    while (it != tileCache.end()) {
        // Extract item type from key (format: "type_x_y")
        std::string key = it->first;
        std::string typePrefix;
        switch (itemType) {
            case TileItemType::Rock: typePrefix = "rock_"; break;
            case TileItemType::Grass: typePrefix = "grass_"; break;
            case TileItemType::Tree: typePrefix = "tree_"; break;
        }
        
        if (key.substr(0, typePrefix.length()) == typePrefix) {
            it = tileCache.erase(it);
        } else {
            ++it;
        }
    }
}

size_t TerrainGenerator::GetCacheSize() const {
    return tileCache.size();
}

std::string TerrainGenerator::MakeCacheKey(TileItemType itemType, int tileX, int tileY) const {
    std::string prefix;
    switch (itemType) {
        case TileItemType::Rock: prefix = "rock_"; break;
        case TileItemType::Grass: prefix = "grass_"; break;
        case TileItemType::Tree: prefix = "tree_"; break;
    }
    return prefix + std::to_string(tileX) + "_" + std::to_string(tileY);
}

XXH64_hash_t TerrainGenerator::CalculateConfigHash(const std::unordered_map<std::string, float>& config) const {
    // Create a vector of key-value pairs for consistent ordering
    std::vector<std::pair<std::string, float>> sortedConfig(config.begin(), config.end());
    std::sort(sortedConfig.begin(), sortedConfig.end());
    
    // Serialize sorted config into a buffer for hashing
    std::vector<char> buffer;
    for (const auto& pair : sortedConfig) {
        // Add key string
        const std::string& key = pair.first;
        buffer.insert(buffer.end(), key.begin(), key.end());
        buffer.push_back('\0'); // Null terminator for string separation
        
        // Add float value as bytes
        const char* valueBytes = reinterpret_cast<const char*>(&pair.second);
        buffer.insert(buffer.end(), valueBytes, valueBytes + sizeof(float));
    }
    
    // Use XXH64 to hash the serialized buffer
    return XXH64(buffer.data(), buffer.size(), 0);
}

// Helper function to get biome type based on height and planet type
BiomInfo TerrainGenerator::GetBiomeTypeAtHeight(float height, const std::string& currentPlanetType) const {
    // Find the current planet type
    const PlanetType* currentPlanet = nullptr;
    for (const auto& planetType : planetTypes) {
        if (planetType.name == currentPlanetType) {
            currentPlanet = &planetType;
            break;
        }
    }
    
    if (!currentPlanet || currentPlanet->boundaries.empty()) {
        return {BiomType::Rock, 0.0f}; // Default fallback
    }
    
    // Find the appropriate biome boundary for this height
    BiomType resultType = currentPlanet->boundaries[0].type; // Default to first boundary
    int currentBoundaryIndex = 0;
    
    for (int i = 0; i < (int)currentPlanet->boundaries.size(); i++) {
        const auto& boundary = currentPlanet->boundaries[i];
        if (height >= boundary.startHeight) {
            resultType = boundary.type;
            currentBoundaryIndex = i;
        } else {
            break;
        }
    }
    
    // Calculate normalized distance within the current biome
    float normalizedDistance = 0.0f;
    
    if (currentBoundaryIndex < (int)currentPlanet->boundaries.size()) {
        float currentBiomeStart = currentPlanet->boundaries[currentBoundaryIndex].startHeight;
        float nextBiomeStart;
        
        // If this is the last biome, use a reasonable upper bound
        if (currentBoundaryIndex == (int)currentPlanet->boundaries.size() - 1) {
            // For the last biome, assume it extends for a reasonable range
            // FIXME: Input from graphics.cpp the actual ranges
            nextBiomeStart = 18.0f;
        } else {
            nextBiomeStart = currentPlanet->boundaries[currentBoundaryIndex + 1].startHeight;
        }
        
        // Calculate normalized position within this biome
        if (nextBiomeStart > currentBiomeStart) {
            normalizedDistance = (height - currentBiomeStart) / (nextBiomeStart - currentBiomeStart);
            // Clamp to [0, 1] range
            normalizedDistance = std::max(0.0f, std::min(1.0f, normalizedDistance));
        }
    }
    
    return {resultType, normalizedDistance};
}

std::vector<TerrainItem> TerrainGenerator::GenerateTerrainItems(
    const float2& currentPosition,
    const Quaternion<float>& currentRotation,
    float tileAspectRatio,
    const std::string& currentPlanetType,
    std::unordered_map<std::string, float>& keyValueConfig,
    const TerrainHeightFunction& heightFunction
) {
    std::vector<TerrainItem> terrainItems;

    // Calculate hash of the current keyValueConfig
    std::size_t newConfigHash = CalculateConfigHash(keyValueConfig);
    
    // If the configuration has changed, clear the entire cache
    if (newConfigHash != currentConfigHash) {
        ClearCache();
        currentConfigHash = newConfigHash;
    }

    activeKeyValueConfig = &keyValueConfig;
    
    // Add the rover/player at current position
    TerrainItem rover{ "Rover", currentPosition, float3{}, currentRotation, true };
    terrainItems.push_back(rover);
    
    // Use provided config or get default
    TerrainConfig actualConfig = GetDefaultTerrainConfig();

    actualConfig.terrainAspectRatio = tileAspectRatio;
    
    // Add all minerals
    for (const auto& mineral : actualConfig.minerals) {
        AddMineral(terrainItems, mineral.position, mineral.type);
    }
    
    // Add all ruins
    for (const auto& ruin : actualConfig.ruins) {
        AddRuin(terrainItems, ruin.position);
    }
    
    // Add all spaceships
    for (const auto& spaceship : actualConfig.spaceships) {
        AddSpaceship(terrainItems, spaceship.position);
    }
    
    // Add rock formations around current position
    AddRockFormations(terrainItems, currentPosition, actualConfig, heightFunction, currentPlanetType);
    
    // Add grass formations around current position
    //AddGrassFormations(terrainItems, currentPosition, actualConfig, heightFunction, currentPlanetType);
    
    // Add tree formations around current position
    AddTreeFormations(terrainItems, currentPosition, actualConfig, heightFunction, currentPlanetType);
    
    return terrainItems;
}

void TerrainGenerator::AddMineral(std::vector<TerrainItem>& terrainItems, 
                                 const float2& worldCoord, int type) const {
    if (type == 6) {
        TerrainItem mineral{ "Endurium", worldCoord, float3{}, Quaternion<float>{}, true };
        terrainItems.push_back(mineral);
        TerrainItem miningSymbol{ "EnduriumSymbol", worldCoord, float3{0.0f, 5.0f, 0.0f}, Quaternion<float>{}, false };
        terrainItems.push_back(miningSymbol);
    } else {
        TerrainItem mineral{ "Mineral", worldCoord, float3{}, Quaternion<float>{}, true };
        terrainItems.push_back(mineral);
        TerrainItem miningSymbol{ "MiningSymbol", worldCoord, float3{0.0f, 5.0f, 0.0f}, Quaternion<float>{}, false };
        terrainItems.push_back(miningSymbol);
    }
}

void TerrainGenerator::AddRuin(std::vector<TerrainItem>& terrainItems, 
                              const float2& worldCoord) const {
    TerrainItem ruin{ "AncientRuin", worldCoord, float3{}, Quaternion<float>{}, true };
    terrainItems.push_back(ruin);
    TerrainItem ruinSymbol{ "AncientRuinSymbol", worldCoord, float3{0.0f, 5.0f, 0.0f}, Quaternion<float>{}, false };
    terrainItems.push_back(ruinSymbol);
}

void TerrainGenerator::AddSpaceship(std::vector<TerrainItem>& terrainItems, 
                                   const float2& worldCoord) const {
    TerrainItem spaceship{ "SF_Ramp", worldCoord, float3{}, Quaternion<float>{}, true };
    terrainItems.push_back(spaceship);
    TerrainItem spaceshipSymbol{ "Starship", worldCoord, float3{0.0f, 5.0f, 0.0f}, Quaternion<float>{}, false };
    terrainItems.push_back(spaceshipSymbol);
}

// Cache-aware tile generation methods
std::vector<TerrainItem> TerrainGenerator::GetOrGenerateRockTile(
    int tileX, int tileY, const TerrainConfig& config,
    const TerrainHeightFunction& heightFunction,
    const std::string& currentPlanetType) {
    
    std::string cacheKey = MakeCacheKey(TileItemType::Rock, tileX, tileY);
    
    // Check if tile is already cached
    auto it = tileCache.find(cacheKey);
    if (it != tileCache.end() && it->second.isGenerated) {
        return it->second.items;
    }
    
    // Generate new tile
    std::vector<TerrainItem> tileItems;
    
    const float minRockDistance = 0.7f;
    const float densityThreshold = 0.7f;
    const int seed = 12345;
    
    // Calculate tile center
    float2 tileCenter = {
        (float)tileX * config.formationRadius + config.formationRadius * 0.5f,
        (float)tileY * config.formationRadius + config.formationRadius * 0.5f
    };
    
    // Generate candidate positions for this tile
    std::vector<float2> candidatePositions = PoissonDiscSamplingForTile(
        tileCenter, config.formationRadius, config.formationRadius, minRockDistance, seed, tileX, tileY);
    
    // Process each candidate position
    for (const float2& worldCoord : candidatePositions) {
        float height = heightFunction(worldCoord);
        
        // Get biome type at this height
        BiomInfo biomeInfo = GetBiomeTypeAtHeight(height, currentPlanetType);
        
        // Skip placement in non-rock biomes
        if (biomeInfo.type != BiomType::Rock) continue;
        
        // Check if rock should be placed using fractal noise (for additional filtering)
        float placementNoise = RockFractalNoise(worldCoord.x, worldCoord.y, 3, 0.5f, 0.1f, seed);
        if(placementNoise <= densityThreshold) continue;
        
        // Position jitter for natural placement
        float2 jitter = {
            RockNoise(worldCoord.x, worldCoord.y, 0.3f, seed + 100) * 0.2f,
            RockNoise(worldCoord.x, worldCoord.y, 0.3f, seed + 200) * 0.2f
        };
        
        float2 rockPos = worldCoord + jitter;

        const int MaxRockTypes = 6;
        
        // Rock type (1-6 for rock_moss_set_01_rock01 through rock06)
        float typeNoise = std::abs(RockNoise(worldCoord.x, worldCoord.y, 0.2f, seed + 400));
        int rockType = (int)(typeNoise * float(MaxRockTypes) + 1.0f);
        
        // Construct rock name
        std::string rockName = "rock_moss_set_01_rock";
        rockName += "0" + std::to_string(rockType);
        
        // Random rotation for visual variety on Y axis
        float rotY = RockNoise(worldCoord.x, worldCoord.y, 0.25f, seed + 501) * 6.28318f;
        Quaternion<float> rotYQuat = Quaternion<float>::RotationFromAxisAngle({0, 1, 0}, rotY);

        // Random scale for visual variety (0.6 to 1.5)
        float scaleFactor = 0.5f + RockNoise(worldCoord.x, worldCoord.y, 0.15f, seed + 600) * 2.5f;
        
        TerrainItem rock{
            rockName,
            rockPos,
            float3{0.0f, 0.0f, 0.0f},
            rotYQuat,
            true,
            float3{scaleFactor, scaleFactor, scaleFactor}
        };
        
        tileItems.push_back(rock);
    }
    
    // Cache the generated items
    CachedTileData& cachedData = tileCache[cacheKey];
    cachedData.items = tileItems;
    cachedData.isGenerated = true;
    
    return tileItems;
}

std::vector<TerrainItem> TerrainGenerator::GetOrGenerateGrassTile(
    int tileX, int tileY, const TerrainConfig& config,
    const TerrainHeightFunction& heightFunction,
    const std::string& currentPlanetType) {
    
    std::string cacheKey = MakeCacheKey(TileItemType::Grass, tileX, tileY);
    
    // Check if tile is already cached
    auto it = tileCache.find(cacheKey);
    if (it != tileCache.end() && it->second.isGenerated) {
        return it->second.items;
    }
    
    // Generate new tile
    std::vector<TerrainItem> tileItems;
    
    const float minGrassDistance = 0.5f;
    const float densityThreshold = 0.6f;
    const int seed = 54321;
    
    // Calculate tile center
    float2 tileCenter = {
        (float)tileX * config.formationRadius + config.formationRadius * 0.5f,
        (float)tileY * config.formationRadius + config.formationRadius * 0.5f
    };
    
    // Generate candidate positions for this tile
    std::vector<float2> candidatePositions = PoissonDiscSamplingForTile(
        tileCenter, config.formationRadius, config.formationRadius, minGrassDistance, seed, tileX, tileY);
    
    // Process each candidate position
    for (const float2& worldCoord : candidatePositions) {
        float height = heightFunction(worldCoord);
        
        // Get biome type at this height
        BiomInfo biomeInfo = GetBiomeTypeAtHeight(height, currentPlanetType);
        
        // Only place grass in grass biomes
        if (biomeInfo.type != BiomType::Grass) continue;
        
        // Check if grass should be placed using fractal noise (for additional filtering)
        float placementNoise = RockFractalNoise(worldCoord.x, worldCoord.y, 3, 0.5f, 0.1f, seed);
        if(placementNoise <= densityThreshold) continue;
        
        // Position jitter for natural placement
        float2 jitter = {
            RockNoise(worldCoord.x, worldCoord.y, 0.3f, seed + 100) * 0.1f,
            RockNoise(worldCoord.x, worldCoord.y, 0.3f, seed + 200) * 0.1f
        };
        
        float2 grassPos = worldCoord + jitter;

        float scale = 1.0f;

        // Generate random rotation around Y axis
        float randomAngle = 0.0f;
        Quaternion<float> grassRotation = Quaternion<float>::RotationFromAxisAngle(float3{0.0f, 1.0f, 0.0f}, randomAngle);
       
        // Randomly select one of the 5 grass cluster types
        int grassTypeIndex = (int)(RockNoise(worldCoord.x, worldCoord.y, 0.1f, seed + 500) * 5.0f);
        std::string grassType = "grass_cluster_0" + std::to_string(grassTypeIndex);
        
        TerrainItem grass{
            grassType,
            grassPos,
            float3{0.0f, 0.0f, 0.0f},
            grassRotation,
            true,
            float3{scale, scale, scale}
        };
        
        tileItems.push_back(grass);
    }
    
    // Cache the generated items
    CachedTileData& cachedData = tileCache[cacheKey];
    cachedData.items = tileItems;
    cachedData.isGenerated = true;
    
    return tileItems;
}

std::vector<TerrainItem> TerrainGenerator::GetOrGenerateTreeTile(
    int tileX, int tileY, const TerrainConfig& config,
    const TerrainHeightFunction& heightFunction,
    const std::string& currentPlanetType) {
    
    std::string cacheKey = MakeCacheKey(TileItemType::Tree, tileX, tileY);
    
    // Check if tile is already cached
    auto it = tileCache.find(cacheKey);
    if (it != tileCache.end() && it->second.isGenerated) {
        return it->second.items;
    }
    
    // Generate new tile
    std::vector<TerrainItem> tileItems;

    const float minTreeDistance = 0.9f;
    const int seed = 67890;
    
    // Calculate tile dimensions based on aspect ratio
    float tileWidth = config.formationRadius * config.terrainAspectRatio;
    float tileHeight = config.formationRadius;
    
    // Calculate tile center
    float2 tileCenter = {
        (float)tileX * tileWidth + tileWidth * 0.5f,
        (float)tileY * tileHeight + tileHeight * 0.5f
    };
    
    // Generate candidate positions for this tile using Poisson disc sampling
    std::vector<float2> candidatePositions = PoissonDiscSamplingForTile(
        tileCenter, tileWidth, tileHeight, minTreeDistance, seed, tileX, tileY);

    float configuredScaleHeight = (*activeKeyValueConfig)["tree_scale_height"];
    float configuredScaleWidth = (*activeKeyValueConfig)["tree_scale_width"];
    float configuredRotation = (*activeKeyValueConfig)["tree_rotation"];
    float configuredTilt = (*activeKeyValueConfig)["tree_tilt"];
    float configuredTiltDirection = (*activeKeyValueConfig)["tree_tilt_direction"];
    
    // Process each candidate position
    for (const float2& worldCoord : candidatePositions) {
        float height = heightFunction(worldCoord);
       
        // Get biome type at this height
        BiomInfo biomeInfo = GetBiomeTypeAtHeight(height, currentPlanetType);
        
        // Allow trees in grass biomes (including high grass areas > 0.75 transitioning to high elevation)
        if (biomeInfo.type != BiomType::Grass) continue;
        
        // Calculate environmental hardiness factors
        float altitude = biomeInfo.normalizedDistance; // 0.0 = sea level, 1.0 = mountain peak
        // FIXME: Input from graphics.cpp the actual ranges
        float latitude = worldCoord.y / 960.0f; // 0.0 = south pole, 1.0 = north pole
        
        // Calculate hardiness requirements (0.0 = easy conditions, 1.0 = extreme conditions)
        float altitudeHardiness = altitude; // Higher altitude = harder conditions
        float latitudeHardiness = (latitude < 0.25f || latitude > 0.75f) ? 0.8f : 0.2f; // Polar regions are harsh
        float combinedHardiness = std::min(1.0f, altitudeHardiness + latitudeHardiness * 0.5f);
        
        // Adjust tree density based on environmental conditions
        // Trees are densest in moderate conditions and sparser in extreme conditions
        float optimalZone = (biomeInfo.type == BiomType::Grass && biomeInfo.normalizedDistance < 0.75f) ? 1.0f : 0.6f;
        float distanceFromCenter = std::abs(biomeInfo.normalizedDistance - 0.5f); // 0.0 at center, 0.5 at extremes
        
        // Altitude-based thinning: trees become dramatically sparser at high elevation
        // 0.0 altitude = 100% density, 0.5 altitude = 50% density, 1.0 altitude = 10% density
        float altitudeThinning = 1.0f - (altitude * altitude * 0.9f); // Quadratic falloff for dramatic effect
        
        float densityProbability = optimalZone * (1.0f - (distanceFromCenter * 1.2f)) * altitudeThinning * (1.0f - combinedHardiness * 0.2f);
        
        // Add some noise to make density variation more natural
        float densityNoise = RockNoise(worldCoord.x, worldCoord.y, 0.05f, seed + 700);
        densityProbability += densityNoise * 0.1f; // ±0.1 variation
        
        // Skip this tree based on density probability
        if (densityProbability < 0.2f) continue;

        // Position jitter for natural placement
        float2 jitter = {
            RockNoise(worldCoord.x, worldCoord.y, 0.3f, seed + 100) * 0.1f,
            RockNoise(worldCoord.x, worldCoord.y, 0.3f, seed + 200) * 0.1f
        };
        
        float2 treePos = worldCoord + jitter;

        // Override configured scale with random variation
        configuredScaleHeight = 0.75f + RockNoise(worldCoord.x, worldCoord.y, 0.15f, seed + 300) * 0.5f; // 0.75 to 1.25
        configuredScaleWidth = 0.90f + RockNoise(worldCoord.x, worldCoord.y, 0.15f, seed + 400) * 0.2f; // 0.90 to 1.10

        // Generate random rotation around Y axis for natural variety
        float randomAngle = RockNoise(worldCoord.x, worldCoord.y, 0.2f, seed + 400) * 2.0f * 3.14159f;
        Quaternion<float> treeRotation = Quaternion<float>::RotationFromAxisAngle(float3{0.0f, 1.0f, 0.0f}, randomAngle);

        // Add small random tilt for natural tree leaning
        float tiltAngle = RockNoise(worldCoord.x, worldCoord.y, 0.4f, seed + 600) * 0.15f; // ±0.15 radians (~8.6 degrees)
        // Calculate tilt axis from configurable direction (0 = north/+Z, π/2 = east/+X, π = south/-Z, 3π/2 = west/-X)
        configuredTiltDirection = RockNoise(worldCoord.x, worldCoord.y, 0.25f, seed + 800) * 2.0f * PI_F;
        float3 tiltAxis = float3{std::sin(configuredTiltDirection), 0.0f, std::cos(configuredTiltDirection)};
        Quaternion<float> tiltRotation = Quaternion<float>::RotationFromAxisAngle(tiltAxis, tiltAngle);
        
        // Combine the Y rotation with the tilt
        treeRotation = tiltRotation * treeRotation;
       
        // Generate tree type based on environmental hardiness
        // Tree hardiness categories:
        // Ultra-hardy (0.8-1.0): 2 (bare), 7 (bare/birch), 1 (conifer) - extreme conditions
        // Hardy (0.6-0.8): 4 (dark conifer), 8 (pine), 10 (tall evergreen) - harsh conditions
        // Moderate (0.3-0.6): 3 (round green), 6 (willow), 9 (round canopy) - temperate conditions
        // Tender (0.0-0.3): 5 (dense foliage), 11 (broad tropical) - optimal conditions
        
        int treeType;
        float noiseValue = RockNoise(worldCoord.x, worldCoord.y, 0.1f, seed + 500);
        float selectionNoise = std::abs(noiseValue) + altitude * 0.3f + latitude * 0.2f;
        
        if (combinedHardiness >= 0.8f) {
            // Ultra-harsh conditions - only the most resilient trees survive
            // High altitude + polar conditions, low oxygen, extreme cold, no seasons
            std::vector<int> ultraHardyTrees = {2, 7, 1}; // bare trees and hardy conifers
            int index = (int)(selectionNoise * ultraHardyTrees.size());
            treeType = ultraHardyTrees[index % ultraHardyTrees.size()];
        } else if (combinedHardiness >= 0.6f) {
            // Harsh conditions - hardy conifers dominate
            // High altitude or polar conditions, cold, low oxygen or no seasons
            std::vector<int> hardyTrees = {4, 8, 10, 1}; // mostly evergreens
            int index = (int)(selectionNoise * hardyTrees.size());
            treeType = hardyTrees[index % hardyTrees.size()];
        } else if (combinedHardiness >= 0.3f) {
            // Moderate conditions - mixed deciduous and some evergreens
            // Mid-altitude temperate zones with reasonable growing conditions
            std::vector<int> moderateTrees = {3, 6, 9, 4}; // mixed deciduous and some conifers
            int index = (int)(selectionNoise * moderateTrees.size());
            treeType = moderateTrees[index % moderateTrees.size()];
        } else {
            // Optimal conditions - lush growth possible
            // Low altitude, temperate to tropical, good oxygen, mild seasons
            std::vector<int> tenderTrees = {5, 11, 9, 3}; // lush foliage and broad canopy
            int index = (int)(selectionNoise * tenderTrees.size());
            treeType = tenderTrees[index % tenderTrees.size()];
        }
        
        std::string treeName = std::string("tree_") + (treeType < 10 ? "0" : "") + std::to_string(treeType);
        
        TerrainItem tree{
            treeName,
            treePos,
            float3{0.0f, 0.0f, 0.0f},
            treeRotation,
            false,
            float3{configuredScaleWidth, configuredScaleHeight, configuredScaleWidth}
        };
        
        tileItems.push_back(tree);
    }
    
    // Cache the generated items
    CachedTileData& cachedData = tileCache[cacheKey];
    cachedData.items = tileItems;
    cachedData.isGenerated = true;
    
    return tileItems;
}

// Deterministic Poisson Disc Sampling for a fixed tile
std::vector<float2> PoissonDiscSamplingForTile(float2 tileCenter, float tileWidth, float tileHeight, float minDistance, int baseSeed, int tileX, int tileY) {
    std::vector<float2> points;
    std::vector<float2> activeList;
    
    // Create unique seed for this tile
    int tileSeed = baseSeed + tileX * 73856093 + tileY * 19349663;
    std::mt19937 rng(tileSeed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Grid for spatial hashing to speed up nearest neighbor checks
    float cellSize = minDistance / std::sqrt(2.0f);
    int gridWidth = (int)std::ceil(tileWidth / cellSize);
    int gridHeight = (int)std::ceil(tileHeight / cellSize);
    std::vector<std::vector<int>> grid(gridWidth * gridHeight);
    
    float2 tileMin = { tileCenter.x - tileWidth * 0.5f, tileCenter.y - tileHeight * 0.5f };
    float2 tileMax = { tileCenter.x + tileWidth * 0.5f, tileCenter.y + tileHeight * 0.5f };
    
    auto getGridIndex = [&](float2 point) -> int {
        int x = (int)((point.x - tileMin.x) / cellSize);
        int y = (int)((point.y - tileMin.y) / cellSize);
        if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) return -1;
        return y * gridWidth + x;
    };
    
    auto isValidPoint = [&](float2 candidate) -> bool {
        // Check if point is within tile bounds
        if (candidate.x < tileMin.x || candidate.x > tileMax.x || 
            candidate.y < tileMin.y || candidate.y > tileMax.y) return false;
        
        // Check minimum distance to existing points using grid
        int gridIdx = getGridIndex(candidate);
        if (gridIdx == -1) return false;
        
        // Check surrounding grid cells
        int gx = gridIdx % gridWidth;
        int gy = gridIdx / gridWidth;
        
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                int nx = gx + dx;
                int ny = gy + dy;
                if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) continue;
                
                int neighborIdx = ny * gridWidth + nx;
                for (int pointIdx : grid[neighborIdx]) {
                    float2 existingPoint = points[pointIdx];
                    float distSq = (candidate.x - existingPoint.x) * (candidate.x - existingPoint.x) +
                                  (candidate.y - existingPoint.y) * (candidate.y - existingPoint.y);
                    if (distSq < minDistance * minDistance) {
                        return false;
                    }
                }
            }
        }
        return true;
    };
    
    // Start with initial point at tile center
    float2 initialPoint = tileCenter;
    points.push_back(initialPoint);
    activeList.push_back(initialPoint);
    
    int gridIdx = getGridIndex(initialPoint);
    if (gridIdx >= 0) {
        grid[gridIdx].push_back(0);
    }
    
    const int maxAttempts = 30;
    
    while (!activeList.empty()) {
        // Pick random point from active list
        int randomIndex = (int)(dist(rng) * activeList.size());
        float2 activePoint = activeList[randomIndex];
        
        bool foundValidPoint = false;
        
        // Try to generate new points around the active point
        for (int attempt = 0; attempt < maxAttempts; attempt++) {
            // Generate random point in annulus (ring) around active point
            float angle = dist(rng) * 2.0f * 3.14159f;
            float distance = minDistance + dist(rng) * minDistance; // Between minDistance and 2*minDistance
            
            float2 candidate = {
                activePoint.x + distance * std::cos(angle),
                activePoint.y + distance * std::sin(angle)
            };
            
            if (isValidPoint(candidate)) {
                points.push_back(candidate);
                activeList.push_back(candidate);
                
                int candidateGridIdx = getGridIndex(candidate);
                if (candidateGridIdx >= 0) {
                    grid[candidateGridIdx].push_back((int)points.size() - 1);
                }
                foundValidPoint = true;
                break;
            }
        }
        
        // If no valid point found, remove from active list
        if (!foundValidPoint) {
            activeList.erase(activeList.begin() + randomIndex);
        }
    }
    
    return points;
}

void TerrainGenerator::AddRockFormations(std::vector<TerrainItem>& terrainItems, 
                                        const float2& centerPosition, const TerrainConfig& config,
                                        const TerrainHeightFunction& heightFunction,
                                        const std::string& currentPlanetType) {
    
    // Determine which tiles intersect with the sampling radius
    float2 minBounds = { centerPosition.x - config.formationRadius, centerPosition.y - config.formationRadius };
    float2 maxBounds = { centerPosition.x + config.formationRadius, centerPosition.y + config.formationRadius };
    
    int minTileX = (int)std::floor(minBounds.x / config.formationRadius);
    int maxTileX = (int)std::floor(maxBounds.x / config.formationRadius);
    int minTileY = (int)std::floor(minBounds.y / config.formationRadius);
    int maxTileY = (int)std::floor(maxBounds.y / config.formationRadius);
    
    // Generate rocks for each intersecting tile using cache
    for (int tileY = minTileY; tileY <= maxTileY; tileY++) {
        for (int tileX = minTileX; tileX <= maxTileX; tileX++) {
            // Get cached or generate tile items
            std::vector<TerrainItem> tileItems = GetOrGenerateRockTile(
                tileX, tileY, config, heightFunction, currentPlanetType);
            
            // Filter items within radius and add to result
            for (const TerrainItem& item : tileItems) {
                float dx = item.tilePosition.x - centerPosition.x;
                float dy = item.tilePosition.y - centerPosition.y;
                if (dx*dx + dy*dy <= config.formationRadius*config.formationRadius) {
                    terrainItems.push_back(item);
                }
            }
        }
    }
}

void TerrainGenerator::AddGrassFormations(std::vector<TerrainItem>& terrainItems, 
                                         const float2& centerPosition, const TerrainConfig& config,
                                         const TerrainHeightFunction& heightFunction,
                                         const std::string& currentPlanetType) {
    
    // Determine which tiles intersect with the sampling radius
    float2 minBounds = { centerPosition.x - config.formationRadius, centerPosition.y - config.formationRadius };
    float2 maxBounds = { centerPosition.x + config.formationRadius, centerPosition.y + config.formationRadius };
    
    int minTileX = (int)std::floor(minBounds.x / config.formationRadius);
    int maxTileX = (int)std::floor(maxBounds.x / config.formationRadius);
    int minTileY = (int)std::floor(minBounds.y / config.formationRadius);
    int maxTileY = (int)std::floor(maxBounds.y / config.formationRadius);
    
    // Generate grass for each intersecting tile using cache
    for (int tileY = minTileY; tileY <= maxTileY; tileY++) {
        for (int tileX = minTileX; tileX <= maxTileX; tileX++) {
            // Get cached or generate tile items
            std::vector<TerrainItem> tileItems = GetOrGenerateGrassTile(
                tileX, tileY, config, heightFunction, currentPlanetType);
            
            // Filter items within radius and add to result
            for (const TerrainItem& item : tileItems) {
                float dx = item.tilePosition.x - centerPosition.x;
                float dy = item.tilePosition.y - centerPosition.y;
                if (dx*dx + dy*dy <= config.formationRadius*config.formationRadius) {
                    terrainItems.push_back(item);
                }
            }
        }
    }
}

void TerrainGenerator::AddTreeFormations(std::vector<TerrainItem>& terrainItems, 
                                        const float2& centerPosition, const TerrainConfig& config,
                                        const TerrainHeightFunction& heightFunction,
                                        const std::string& currentPlanetType) {
    
    // Calculate tile dimensions based on aspect ratio
    float tileWidth = config.formationRadius * config.terrainAspectRatio;
    float tileHeight = config.formationRadius;
    
    // Determine which tiles intersect with the sampling radius
    float2 minBounds = { centerPosition.x - config.formationRadius, centerPosition.y - config.formationRadius };
    float2 maxBounds = { centerPosition.x + config.formationRadius, centerPosition.y + config.formationRadius };
    
    int minTileX = (int)std::floor(minBounds.x / tileWidth);
    int maxTileX = (int)std::floor(maxBounds.x / tileWidth);
    int minTileY = (int)std::floor(minBounds.y / tileHeight);
    int maxTileY = (int)std::floor(maxBounds.y / tileHeight);
    
    // Generate trees for each intersecting tile using cache
    for (int tileY = minTileY; tileY <= maxTileY; tileY++) {
        for (int tileX = minTileX; tileX <= maxTileX; tileX++) {
            // Get cached or generate tile items
            std::vector<TerrainItem> tileItems = GetOrGenerateTreeTile(
                tileX, tileY, config, heightFunction, currentPlanetType);
            
            // Filter items within radius and add to result
            for (const TerrainItem& item : tileItems) {
                float dx = item.tilePosition.x - centerPosition.x;
                float dy = item.tilePosition.y - centerPosition.y;
                if (dx*dx + dy*dy <= config.formationRadius*config.formationRadius) {
                    terrainItems.push_back(item);
                }
            }
        }
    }
}

TerrainGenerator::TerrainConfig TerrainGenerator::GetDefaultTerrainConfig() const {
    TerrainConfig config;
    
    // Default minerals (converted from the hardcoded values in graphics.cpp)
    config.minerals = {
        {{2021.0f, 960.0f - 287.0f}, 6, 43},   // Endurium
        {{2031.0f, 960.0f - 290.0f}, 12, 41},
        {{2026.0f, 960.0f - 274.0f}, 11, 81},
        {{2025.0f, 960.0f - 293.0f}, 5, 11},
        {{2023.0f, 960.0f - 271.0f}, 11, 18},
        {{2021.0f, 960.0f - 290.0f}, 12, 61},
        {{2028.0f, 960.0f - 275.0f}, 12, 31},
        {{2027.0f, 960.0f - 279.0f}, 5, 21},
        {{2022.0f, 960.0f - 295.0f}, 12, 33},
        {{2019.0f, 960.0f - 289.0f}, 15, 25},
        {{2019.0f, 960.0f - 289.0f}, 11, 89},
        {{2016.0f, 960.0f - 287.0f}, 3, 58},
        {{2016.0f, 960.0f - 277.0f}, 11, 66},
        {{1998.0f, 960.0f - 293.0f}, 15, 40},
        {{1998.0f, 960.0f - 280.0f}, 11, 40},
        {{2005.0f, 960.0f - 283.0f}, 11, 95},
        {{2001.0f, 960.0f - 287.0f}, 11, 62},
        {{2014.0f, 960.0f - 293.0f}, 11, 41},
        {{1999.0f, 960.0f - 296.0f}, 11, 80},
        {{2019.0f, 960.0f - 298.0f}, 11, 99},
        {{2016.0f, 960.0f - 299.0f}, 11, 92},
        {{2034.0f, 960.0f - 274.0f}, 12, 76}
    };
    
    // Default ruins
    config.ruins = {
        {{2022.0f, 960.0f - 288.0f}, 2}  // Species 2
    };
    
    // Default spaceships
    config.spaceships = {
        {{2011.0f, 960.0f - 287.0f}}
    };
       
    return config;
}

} // namespace SF_GLTF

} // namespace Diligent
