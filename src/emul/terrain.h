#pragma once

#include <vector>
#include <functional>
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
        
        // Terrain bounds (for coordinate conversion)
        float2 terrainMaxSize = {2304.0f * 6.666f, 960.0f * 4.0f}; // TERRAIN_MAX_X * TileSize.x, TERRAIN_MAX_Y * TileSize.y
        float terrainMaxY = 960.0f;
        
        // Predefined terrain data
        std::vector<MineralData> minerals;
        std::vector<RuinData> ruins;
        std::vector<SpaceshipData> spaceships;
    };

    TerrainGenerator(const std::vector<PlanetType>& planetTypes);
    ~TerrainGenerator() = default;
    TerrainGenerator() = delete;

    // Main terrain generation function
    std::vector<TerrainItem> GenerateTerrainItems(
        const float2& currentPosition,
        const Quaternion<float>& currentRotation,
        const std::string& currentPlanetType,
        TerrainConfig* config = nullptr,
        const TerrainHeightFunction& heightFunction = nullptr
    );

private:
    // Helper functions for adding different types of terrain objects
    void AddMineral(std::vector<TerrainItem>& terrainItems, 
                   const float2& worldCoord, int type) const;
    
    void AddRuin(std::vector<TerrainItem>& terrainItems, 
                const float2& worldCoord) const;
    
    void AddSpaceship(std::vector<TerrainItem>& terrainItems, 
                     const float2& worldCoord) const;
    
    void AddRockFormations(std::vector<TerrainItem>& terrainItems, 
                          const float2& centerPosition, float radius,
                          const TerrainHeightFunction& heightFunction,
                          const std::string& currentPlanetType) const;

    // Default terrain configuration
    TerrainConfig GetDefaultTerrainConfig() const;
    
    // Noise function for rock placement
    float SimpleNoise(float x, float y, float scale = 1.0f) const;

    // Helper function to get biome type based on height
    BiomType GetBiomeTypeAtHeight(float height, const std::string& currentPlanetType) const;

    std::vector<PlanetType> planetTypes;
};

} // namespace SF_GLTF
} // namespace Diligent

