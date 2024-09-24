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

std::binary_semaphore& GraphicsInit();
void GraphicsQuit();

int SDL_main(int argc, char *argv[]) {

    SetCurrentThreadName("Main Thread");

    std::string hash;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--hash") == 0 && i + 1 < argc) {
            hash = argv[i + 1];
            i++;
        }
    }
   
    auto& shutdown = GraphicsInit();

    shutdown.acquire();

    GraphicsQuit();
    return 0;
}
