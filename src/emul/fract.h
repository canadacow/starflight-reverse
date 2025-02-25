#pragma once

#include <filesystem>
#include <unordered_map>
#include"../cpu/cpu.h"

const int16_t planet_usable_width = 38;
const int16_t planet_usable_height = 9;

const int planet_contour_width = 61;
const int planet_contour_height = 101;

struct PlanetSurface
{
    std::vector<int8_t>   native;
    std::vector<uint8_t>  relief;
    std::vector<uint32_t> albedo;
};

struct FullResPlanetData {
    static constexpr size_t size = planet_contour_width * planet_contour_height * 
                                   planet_usable_width * planet_usable_height;
    
    std::vector<uint8_t> image;   // Dynamic array for planet_image
    std::vector<uint32_t> albedo; // Dynamic array for planet_albedo

    FullResPlanetData() : image(size), albedo(size) {} // Pre-allocate vectors to correct size
};

void FRACT_FILLARRAY();
void Ext_FRACT_StoreHeight();
void FRACT_FRACTALIZE();

void FRACT_NEWCONTOUR();

class FractalGenerator
{
public:
    FractalGenerator() = default;
    ~FractalGenerator() = default;

    // Loads the 48x24 native images
    bool Initialize(const std::filesystem::path& planetDatabase);

    PlanetSurface GetPlanetSurface(uint16_t seed);
    FullResPlanetData GetFullResPlanetData(uint16_t planetInstanceIndex);

private:
    std::unordered_map<uint16_t, std::vector<int8_t>> nativeImages;
};
