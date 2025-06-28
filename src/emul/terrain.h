#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include "BasicMath.hpp"
#include "DynamicMesh.hpp"

namespace Diligent
{

namespace SF_GLTF
{

// Forward declaration
using TerrainHeightFunction = std::function<float(const float2&)>;

// Biome system structures (matching graphics.cpp)
enum class BiomType {
    None,
    Rock,
    Beach,
    Water,
    Grass,
    Ice,
    Lava,
};

struct BiomBoundary
{
    std::string name;
    float startHeight;
    BiomType type;
};

struct PlanetType
{
    std::string name;
    std::vector<BiomBoundary> boundaries;
};

class TerrainGenerator {
public:
    struct MineralData {
        float2 position;
        int type;
        int quantity;
    };

    struct RuinData {
        float2 position;
        int species;
    };

    struct SpaceshipData {
        float2 position;
    };

    struct TerrainConfig {
        // Rock formation settings
        float rockFormationRadius = 16.0f;

        float terrainAspectRatio = ( 6.666f / 4.0f );
        
        // Terrain bounds (for coordinate conversion)
        float2 terrainMaxSize = {2304.0f * 6.666f, 960.0f * 4.0f}; // TERRAIN_MAX_X * TileSize.x, TERRAIN_MAX_Y * TileSize.y
        float terrainMaxY = 960.0f;
        
        // Predefined terrain data
        std::vector<MineralData> minerals;
        std::vector<RuinData> ruins;
        std::vector<SpaceshipData> spaceships;
    };

    // Tile cache types
    enum class TileItemType {
        Rock,
        Grass,
        Tree
    };

    struct TileCoord {
        int x, y;
        
        bool operator==(const TileCoord& other) const {
            return x == other.x && y == other.y;
        }
    };

    struct TileCoordHash {
        std::size_t operator()(const TileCoord& coord) const {
            return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.y) << 1);
        }
    };

    struct CachedTileData {
        std::vector<TerrainItem> items;
        bool isGenerated = false;
    };

    TerrainGenerator(const std::vector<PlanetType>& planetTypes);
    ~TerrainGenerator() = default;
    TerrainGenerator() = delete;

    // Main terrain generation function
    std::vector<TerrainItem> GenerateTerrainItems(
        const float2& currentPosition,
        const Quaternion<float>& currentRotation,
        float tileAspectRatio,
        const std::string& currentPlanetType,
        TerrainConfig* config = nullptr,
        const TerrainHeightFunction& heightFunction = nullptr
    );

    // Cache management
    void ClearCache();
    void ClearCacheForType(TileItemType itemType);
    size_t GetCacheSize() const;

private:
    // Helper functions for adding different types of terrain objects
    void AddMineral(std::vector<TerrainItem>& terrainItems, 
                   const float2& worldCoord, int type) const;
    
    void AddRuin(std::vector<TerrainItem>& terrainItems, 
                const float2& worldCoord) const;
    
    void AddSpaceship(std::vector<TerrainItem>& terrainItems, 
                     const float2& worldCoord) const;
    
    void AddRockFormations(std::vector<TerrainItem>& terrainItems, 
                          const float2& centerPosition, const TerrainConfig& config,
                          const TerrainHeightFunction& heightFunction,
                          const std::string& currentPlanetType);

    void AddGrassFormations(std::vector<TerrainItem>& terrainItems, 
                          const float2& centerPosition, const TerrainConfig& config,
                          const TerrainHeightFunction& heightFunction,
                          const std::string& currentPlanetType);

    void AddTreeFormations(std::vector<TerrainItem>& terrainItems, 
                          const float2& centerPosition, const TerrainConfig& config,
                          const TerrainHeightFunction& heightFunction,
                          const std::string& currentPlanetType);

    // Cache-aware tile generation methods
    std::vector<TerrainItem> GetOrGenerateRockTile(
        int tileX, int tileY, float tileSize,
        const TerrainHeightFunction& heightFunction,
        const std::string& currentPlanetType);

    std::vector<TerrainItem> GetOrGenerateGrassTile(
        int tileX, int tileY, float tileSize,
        const TerrainHeightFunction& heightFunction,
        const std::string& currentPlanetType);

    std::vector<TerrainItem> GetOrGenerateTreeTile(
        int tileX, int tileY, float tileSize,
        const TerrainHeightFunction& heightFunction,
        const std::string& currentPlanetType);

    // Helper methods for cache key generation
    std::string MakeCacheKey(TileItemType itemType, int tileX, int tileY) const;

    // Default terrain configuration
    TerrainConfig GetDefaultTerrainConfig() const;
    
    // Noise function for rock placement
    float SimpleNoise(float x, float y, float scale = 1.0f) const;

    // Helper function to get biome type based on height
    BiomType GetBiomeTypeAtHeight(float height, const std::string& currentPlanetType) const;

    std::vector<PlanetType> planetTypes;
    
    // Tile caches - separate cache for each item type
    std::unordered_map<std::string, CachedTileData> tileCache;
};

} // namespace SF_GLTF
} // namespace Diligent

