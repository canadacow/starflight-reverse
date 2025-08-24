struct UniformBlock {
    int GRAPHICS_MODE_WIDTH;
    int GRAPHICS_MODE_HEIGHT;
    int WINDOW_WIDTH;
    int WINDOW_HEIGHT;
    uint useRotoscope;
    uint useEGA;
    float iTime;
    float worldX;
    float worldY;
    float heading;
    float deadX;
    float deadY;
    // V= CONTEXT-ID#   ( 0=planet surface, 1=orbit, 2=system)         
    // (3 = hyperspace, 4 = encounter, 5 = starport)
    uint game_context; 
    uint alienVar1;
    float screenX;
    float screenY;
    float adjust;
    float planetSize;

    float orbitCamX;
    float orbitCamY;
    float orbitCamZ;

    float nebulaBase;
    float nebulaMultiplier;

    uint orbitMask;

    float zoomLevel;

    float thrust;

    float menuVisibility;
    float blurAmount;
};

struct Icon {
    uint isActive;
    float x;
    float y;
    float screenX;

    float screenY;
    float bltX;
    float bltY;
    uint id;

    uint clr;
    uint icon_type;
    int planet_to_sunX;
    int planet_to_sunY;

    uint planetIndex;
    float objectHeading;
    float x1;
    float y1;
};

#define Icon_Other  0
#define Icon_Sun    1
#define Icon_Planet 2
#define Icon_Nebula 3
#define Icon_Flux   4