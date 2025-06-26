#pragma once

#include <vector>
#include <functional>
#include "BasicMath.hpp"
#include "DynamicMesh.hpp"

namespace Diligent
{

namespace SF_GLTF
{

typedef std::function<float(float2)> TerrainHeightFunction;

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

    TerrainGenerator();
    ~TerrainGenerator() = default;

    // Main terrain generation function
    std::vector<TerrainItem> GenerateTerrainItems(
        const float2& currentPosition,
        const Quaternion<float>& currentRotation,
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
                          const TerrainHeightFunction& heightFunction) const;

    // Default terrain configuration
    TerrainConfig GetDefaultTerrainConfig() const;
    
    // Noise function for rock placement
    float SimpleNoise(float x, float y, float scale = 1.0f) const;
};

} // namespace SF_GLTF
} // namespace Diligent

