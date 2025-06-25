#include "terrain.h"
#include <cmath>
#include <random>

namespace Diligent
{

namespace SF_GLTF
{

TerrainGenerator::TerrainGenerator() {
    // Constructor - could initialize random seed or other state if needed
}

std::vector<TerrainItem> TerrainGenerator::GenerateTerrainItems(
    const float2& currentPosition,
    const Quaternion<float>& currentRotation,
    const TerrainConfig& config
) {
    std::vector<TerrainItem> terrainItems;
    
    // Add the rover/player at current position
    TerrainItem rover{ "Rover", currentPosition, float3{}, currentRotation, true };
    terrainItems.push_back(rover);
    
    // Use provided config or get default
    TerrainConfig actualConfig = config.minerals.empty() && config.ruins.empty() && config.spaceships.empty() 
        ? GetDefaultTerrainConfig() : config;
    
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
    AddRockFormations(terrainItems, currentPosition, actualConfig.rockFormationRadius);
    
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
                                        const float2& centerPosition, float radius) const {
    // TODO: Implement rock formations
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
