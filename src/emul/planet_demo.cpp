#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <cstdint>
#include <cmath>
#include <algorithm>

#include "starsystem.h"

#ifdef _WIN32
typedef void* HWND;
#define NULL 0
#define SW_SHOW 5

extern "C" {
    __declspec(dllimport) HWND __stdcall GetConsoleWindow();
    __declspec(dllimport) int __stdcall AllocConsole();
    __declspec(dllimport) int __stdcall ShowWindow(HWND, int);
}

void ShowConsoleWindow() {
    HWND hwnd = GetConsoleWindow();
    if (hwnd == NULL) {
        AllocConsole();
        hwnd = GetConsoleWindow();
    }
    ShowWindow(hwnd, SW_SHOW);
}
#endif

// Global variables
uint16_t GLOBALSEED = 0;
uint16_t HYDRO = 0;
uint16_t ATMO = 0;
uint16_t PEAK = 0;
uint16_t YABS = 0;

// String tables for terrain descriptions
const std::vector<std::string> terrainDescriptions = {
    "SNOW AND ICE",
    "BLACK AND STAR-FILLED.",
    "CRATERED ROCK AND DUST",
    "SAND",
    "ROCK AND DIRT",
    "CRACKED PLAINS AND HILLS OF",
    "VOLCANIC ROCK, STEAMING VENTS,",
    "BOILING POOLS AND FLOWING LAVA",
    "ROCKY SAND",
    "PEBBLY SAND",
    "POWDERY SAND",
    "COARSE SAND",
    "OCEANS AND ",
    "SMOOTH AND FEATURELESS"
};

const std::vector<std::string> colorDescriptions = {
    "DARK GREY",
    "BLUE TINTED",
    "LIGHT GREY",
    "GREEN TINTED",
    "YELLOWISH",
    "RED TINTED",
    "REDDISH",
    "YELLOW TINTED",
    "BROWNISH",
    "MUDDY BROWN",
    "DARK BLUE",
    "ORANGEISH",
    "PURPLEISH",
    "GREENISH",
    "BLUEISH"
};

const std::vector<std::string> vegetationDescriptions = {
    "MOSS-LIKE",
    "GRASS-LIKE",
    "LICHEN-LIKE",
    "GELATINOUS",
    "COVERED IN PLACES BY A",
    "WITH "
};

// Atmosphere descriptions - changed to match Starflight's exact atmosphere types
const std::vector<std::string> atmosphereDescriptions = {
    "NONE",                      // 0
    "NITROGEN",                  // 1
    "OXYGEN",                    // 2
    "METHANE",                   // 3
    "CARBON MONOXIDE",           // 4
    "WATER",                     // 5
    "AMMONIA COMPOUNDS",         // 6
    "CHLORINE COMPOUNDS",        // 7
    "METHANOL, ETHANOL",         // 8
    "SULFUR COMPOUNDS",          // 9
    "SILICON COMPOUNDS",         // 10
    "SODIUM COMPOUNDS",          // 11
    "FLUORINE COMPOUNDS",        // 12
    "METAL COMPOUNDS",           // 13
    "HYDROGEN, HELIUM",          // 14
    "NITROGEN, OXYGEN",          // 15
    "OXYGEN, CARBON DIOXIDE",    // 16
    "AMMONIA, HYDROGEN",         // 17
    "METHANE, AMMONIA, ARGON",   // 18
    "FLUORINE, CHLORINE",        // 19
    "METHANE, HYDROGEN CYANIDE", // 20
    "OXYGEN, HYDROGEN",          // 21
    "HYDROGEN"                   // 22
};

// Temperature descriptions - exactly matched to the DrawTDESC function
const std::vector<std::string> temperatureDescriptions = {
    "SUBFREEZING",  // 0
    "ARCTIC",       // 1
    "TEMPERATE",    // 2
    "TROPICAL",     // 3
    "SEARING",      // 4
    "INFERNO"       // 5
};

// Atmosphere density descriptions to match the (.ATMO) function
const std::vector<std::string> atmoDensityDescriptions = {
    "NONE",
    "VERY THIN",
    "THIN",
    "MODERATE",
    "THICK",
    "VERY THICK"
};

// Weather/atmosphere activity descriptions to match the (.WEATH) function
const std::vector<std::string> weatherDescriptions = {
    "NONE", 
    "CALM",
    "MODERATE",
    "VIOLENT",
    "VERY VIOLENT"
};

// Hydro and Atmo tables from the original code
const std::vector<uint8_t> SUBHYDRO = {1, 2, 3, 4, 0};
const std::vector<uint8_t> TEMPHYDR = {5, 6, 7, 8, 0, 5, 5};
const std::vector<uint8_t> INFHYDRO = {9, 10, 11, 12, 7, 13, 0};
const std::vector<uint8_t> SUBATMO = {14, 12, 7, 6, 0};
const std::vector<uint8_t> TEMPATMO = {15, 16, 17, 15, 18, 19, 20, 0, 15};
const std::vector<uint8_t> INFATMO = {14, 21, 22, 12, 7, 13, 0};

// RNG function
uint16_t RRND(uint16_t low, uint16_t high) {
    uint16_t ax = GLOBALSEED;
    uint16_t cx = 0x7abd;
    ax = ((int16_t)cx) * ((int16_t)ax);
    ax += 0x1b0f;
    GLOBALSEED = ax;

    uint16_t bx = high;
    cx = low;
    bx -= cx;
    uint32_t mul = ((uint32_t)ax) * ((uint32_t)bx);
    uint16_t dx = ((mul >> 16) & 0xFFFF) + cx;
    return dx;
}

// Helper function to simulate SLIPPER operation
bool SLIPPER(uint16_t value) {
    // In the original code, this seems to check if a value is within a certain range
    // For simplicity, we'll just check if it's even
    return (value % 2) == 0;
}

// Helper function for WECE7
uint16_t WECE7(uint16_t min, uint16_t max, uint16_t factor) {
    uint16_t result;
    do {
        result = RRND(0, 0x0100);
    } while (!SLIPPER(result));
    
    // WECC9 function
    return min + 1 + ((result * factor) / 0x0100);
}

// Generate planet mass based on planet type
void generatePlanetMass(const PLANETENTRY& planet, const PLANETTYPEENTRY& planetType) {
    // Mass tables from the original code
    const std::vector<uint16_t> LO_MASS = {0, 1, 50, 90, 110, 1000, 101, 100};
    const std::vector<uint16_t> HI_MASS = {0, 50, 90, 110, 1000, 10000, 102, 101};
    
    uint8_t planetClass = planetType.d1;
    uint16_t lowMass = LO_MASS[planetClass];
    uint16_t highMass = HI_MASS[planetClass];
    
    uint16_t mass = RRND(lowMass, highMass);
    std::cout << "Planet mass: " << mass << std::endl;
}

// Generate atmosphere data
void generateAtmosphere(const PLANETENTRY& planet, const PLANETTYPEENTRY& planetType) {
    uint8_t atmoDensity = planetType.atmodensity;
    
    // Calculate atmosphere activity to match WF01C function
    uint8_t atmoActivity = std::min(atmoDensity, (uint8_t)4);
    // Original uses WECBD (random) to potentially reduce activity
    if (RRND(1, 100) < 50) {
        atmoActivity--;
    }
    atmoActivity = std::max(atmoActivity, (uint8_t)0);
    
    std::cout << "Atmosphere density: " << (int)atmoDensity << " (" 
              << atmoDensityDescriptions[atmoDensity] << ")" << std::endl;
    std::cout << "Atmosphere activity: " << (int)atmoActivity << " (" 
              << weatherDescriptions[atmoActivity] << ")" << std::endl;
    
    // Determine atmosphere type - match WF1C8
    if (atmoDensity > 0) {
        uint8_t atmoType;
        do {
            // Match WF118 and WF18E functions
            int tempIndex = (planetType.coldest + planetType.warmest) / 2;
            
            if (tempIndex == 0) {
                atmoType = SUBATMO[RRND(0, SUBATMO.size()-1)];
            } else if (tempIndex == 5) {
                atmoType = INFATMO[RRND(0, INFATMO.size()-1)];
            } else {
                atmoType = TEMPATMO[RRND(0, TEMPATMO.size()-1)];
            }
        } while (atmoType == 0);
        
        ATMO = atmoType;
    } else {
        ATMO = 0;
    }
    
    std::string atmoDesc = "NONE";
    if (ATMO > 0 && ATMO < atmosphereDescriptions.size()) {
        atmoDesc = atmosphereDescriptions[ATMO];
    }
    
    std::cout << "Atmosphere type: " << ATMO << " (" << atmoDesc << ")" << std::endl;
}

// Generate hydrosphere data
void generateHydrosphere(const PLANETENTRY& planet, const PLANETTYPEENTRY& planetType) {
    // Check gas giants (surftype == 2) or special planet types
    // In original code, special types are 0x12 and 0x22
    if (planetType.idx == 0x12 || planetType.idx == 0x22) {
        HYDRO = 5; // Set to WATER for gas giants
        std::cout << "Hydrosphere type: " << HYDRO << " (WATER - gas giant)" << std::endl;
        return;
    }
    
    // WF162 function - notGasGiant is true if surftype != 2
    bool notGasGiant = (planetType.surftype != 2);
    uint8_t hydroType;
    
    do {
        // Match WF118 and WF128 functions
        int tempIndex = (planetType.coldest + planetType.warmest) / 2;
        
        if (tempIndex == 0) {
            hydroType = SUBHYDRO[RRND(0, SUBHYDRO.size()-1)];
        } else if (tempIndex == 5) {
            hydroType = INFHYDRO[RRND(0, INFHYDRO.size()-1)];
        } else {
            hydroType = TEMPHYDR[RRND(0, TEMPHYDR.size()-1)];
        }
        // Original loops while hydro is 0 OR (notGasGiant AND hydro is 0)
        // which simplifies to: while hydro is 0
    } while (hydroType == 0);
    
    HYDRO = hydroType;
    
    std::string hydroDesc = "NONE";
    if (HYDRO > 0 && HYDRO < atmosphereDescriptions.size()) {
        hydroDesc = atmosphereDescriptions[HYDRO];
    }
    
    std::cout << "Hydrosphere type: " << HYDRO << " (" << hydroDesc << ")" << std::endl;
}

// Generate terrain description
std::string generateTerrainDescription(const PLANETENTRY& planet, const PLANETTYPEENTRY& planetType) {
    // Based on surface type, matches the original functions
    switch (planetType.surftype) {
        case 1: // Empty/special - WF30A function
            return ""; 
            
        case 2: // Normal terrain - WF40E function
            if (ATMO > 0) {
                // WF3BA function
                if (RRND(1, 100) < 31) {
                    int sandType = RRND(0, 4);
                    switch (sandType) {
                        case 0: return "ROCKY SAND";
                        case 1: return "PEBBLY SAND";
                        case 2: return "POWDERY SAND";
                        case 3: return "COARSE SAND";
                        default: return "SAND";
                    }
                } else {
                    return "ROCK AND DIRT";
                }
            } else {
                return "CRATERED ROCK AND DUST";
            }
            
        case 3: // Cracked plains - WF389 function
            return "CRACKED PLAINS AND HILLS OF";
            
        case 4: // Volcanic
            return "VOLCANIC ROCK, STEAMING VENTS,\nBOILING POOLS AND FLOWING LAVA";
            
        case 5: // Sandy
            return "SAND";
            
        default: // Smooth
            return "SMOOTH AND FEATURELESS";
    }
}

// Main function to generate planet data
void generatePlanetData(const PLANETENTRY& planet, const PLANETTYPEENTRY& planetType) {
    std::cout << "Generating data for planet at orbit " << planet.orbit << std::endl;
    
    // Set the global seed based on planet data
    GLOBALSEED = planet.seed;
    
    // Generate peak value (used for terrain generation) - matches WF06E
    PEAK = 30;
    
    // Generate planet mass
    generatePlanetMass(planet, planetType);
    
    // Generate hydrosphere data - must come before atmosphere for accuracy
    generateHydrosphere(planet, planetType);
    
    // Generate atmosphere data
    generateAtmosphere(planet, planetType);
    
    // Generate terrain description
    std::string terrainDesc = generateTerrainDescription(planet, planetType);
    std::cout << "Terrain: " << terrainDesc << std::endl;
    
    // Display temperature range - matches DrawTDESC and WF068
    int coldTempIndex = planetType.coldest;
    int warmTempIndex = planetType.warmest;
    
    std::cout << "Temperature: ";
    if (coldTempIndex >= 0 && coldTempIndex < temperatureDescriptions.size()) {
        std::cout << temperatureDescriptions[coldTempIndex];
    }
    
    // If temperature range spans multiple values
    if (coldTempIndex != warmTempIndex && 
        warmTempIndex >= 0 && warmTempIndex < temperatureDescriptions.size()) {
        std::cout << " TO " << temperatureDescriptions[warmTempIndex];
    }
    std::cout << std::endl;
    
    // Output global weather/atmospheric activity info
    std::cout << "Global Weather: " << weatherDescriptions[std::min((int)planetType.atmodensity, 4)] << std::endl;
    
    // Properly display surface type - matches DrawSURF function
    std::cout << "Predominant Surface: ";
    switch (planetType.surftype) {
        case 1: std::cout << "GAS"; break;
        case 2: std::cout << "LIQUID"; break;
        case 3: std::cout << "FROZEN"; break;
        case 5: std::cout << "ROCK"; break;
        case 4: std::cout << "MOLTEN"; break;
        default: std::cout << "UNKNOWN"; break;
    }
    std::cout << std::endl;
}

void run_planet_demo() {

    ShowConsoleWindow();

    uint16_t planetInstanceIndex = 0x10ad;
    
    PLANETENTRY planet = planets.at(planetInstanceIndex);
    
    PLANETTYPEENTRY planetType = planettypes[planet.species];    
    
    generatePlanetData(planet, planetType);
}