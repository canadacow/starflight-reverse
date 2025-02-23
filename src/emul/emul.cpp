#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cpu/cpu.h"
#include "call.h"
#include "findword.h"
#include "../patch/patch.h"
#include "../tts/speech.h"

#include <string>
#include <thread>
#include <semaphore>

#include <SDL2/SDL.h>

#include "vstrace.h"

#include "fract.h"

std::binary_semaphore& GraphicsInit();
void GraphicsQuit();

void debug_planet_surfaces()
{
    FractalGenerator fract;
    fract.Initialize("sf1_planet_surfaces.bin");

    // Heaven is 0x03b9
    auto fullRes = fract.GetFullResPlanetData(0x03b8);

    _exit(0);
}

int SDL_main(int argc, char *argv[]) {

    //debug_planet_surfaces();

    SetCurrentThreadName("Main Thread");

    std::string hash;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--hash") == 0 && i + 1 < argc) {
            hash = argv[i + 1];
            i++;
        }
    }

    UserMarks& userMarks = UserMarks::getInstance();

    userMarks.startTraceSystem();
    userMarks.initialize("Starflight");
   
    auto& shutdown = GraphicsInit();

    shutdown.acquire();

    userMarks.stopTraceSystem();

    GraphicsQuit();
    return 0;
}
