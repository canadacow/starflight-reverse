#include "terrain.h"
#include <cmath>
#include <random>
#include <algorithm>

namespace Diligent
{

namespace SF_GLTF
{

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

TerrainGenerator::TerrainGenerator() {
    // Constructor - could initialize random seed or other state if needed
}

std::vector<TerrainItem> TerrainGenerator::GenerateTerrainItems(
    const float2& currentPosition,
    const Quaternion<float>& currentRotation,
    TerrainConfig* config,
    const TerrainHeightFunction& heightFunction
) {
    std::vector<TerrainItem> terrainItems;
    
    // Add the rover/player at current position
    TerrainItem rover{ "Rover", currentPosition, float3{}, currentRotation, true };
    terrainItems.push_back(rover);
    
    // Use provided config or get default
    TerrainConfig actualConfig = config == nullptr ? GetDefaultTerrainConfig() : *config;
    
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
    AddRockFormations(terrainItems, currentPosition, actualConfig.rockFormationRadius, heightFunction);
    
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

void TerrainGenerator::AddRockFormations(std::vector<TerrainItem>& terrainItems, 
                                        const float2& centerPosition, float radius,
                                        const TerrainHeightFunction& heightFunction) const {
    const float sampleInterval = 0.5f;
    const float densityThreshold = 0.85f;
    const int seed = 12345;
    const float worldSize = radius * 2.0f; // Use radius to determine sampling area

    float2 wholeCenterPosition = {
        std::floor(centerPosition.x),
        std::floor(centerPosition.y)
    };
    
    // Sample at regular intervals around the center position
    for(float x = -radius; x < radius; x += sampleInterval) {
        for(float y = -radius; y < radius; y += sampleInterval) {
           
            // World coordinates
            float2 worldCord = { wholeCenterPosition.x + x, wholeCenterPosition.y + y };

            float height = heightFunction(worldCord);
            
            // Check if rock should be placed using fractal noise
            float placementNoise = RockFractalNoise(worldCord.x, worldCord.y, 3, 0.5f, 0.1f, seed);
            if(placementNoise <= densityThreshold) continue;
            
            // Position jitter for natural placement
            float2 jitter = {
                RockNoise(worldCord.x, worldCord.y, 0.3f, seed + 100) * 0.2f,
                RockNoise(worldCord.x, worldCord.y, 0.3f, seed + 200) * 0.2f
            };
            
            float2 rockPos = worldCord + jitter;

            const int MaxRockTypes = 6;
            
            // Rock type (1-6 for rock_moss_set_01_rock01 through rock06)
            float typeNoise = std::abs(RockNoise(worldCord.x, worldCord.y, 0.2f, seed + 400));
            int rockType = (int)(typeNoise * float(MaxRockTypes) + 1.0f);
            
            // Construct rock name
            std::string rockName = "rock_moss_set_01_rock";
            rockName += "0" + std::to_string(rockType);
            
            // Random rotation for visual variety
            float rotAngle = RockNoise(worldCord.x, worldCord.y, 0.25f, seed + 500) * 6.28318f;

            // Random scale for visual variety (0.8 to 1.2)
            float scaleFactor = 0.8f + RockNoise(worldCord.x, worldCord.y, 0.15f, seed + 600) * 0.4f;
            
            TerrainItem rock{
                rockName,
                rockPos,
                float3{0.0f, 0.0f, 0.0f},
                Quaternion<float>::RotationFromAxisAngle({0, 1, 0}, rotAngle),
                false, // no alignment to the ground
                scaleFactor
            };
            
            terrainItems.push_back(rock);
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
