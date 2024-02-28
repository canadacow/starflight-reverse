#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cpu/cpu.h"
#include "call.h"
#include "findword.h"
#include "graphics.h"
#include "../patch/patch.h"
#include "../tts/speech.h"

#include <string>
#include <thread>
#include <semaphore>

int main(int argc, char *argv[]) {
    std::string hash;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--hash") == 0 && i + 1 < argc) {
            hash = argv[i + 1];
            i++;
        }
    }
   
    GraphicsInit();
    InitTextToSpeech();

    initFuture.wait();

    InitCPU();
    InitEmulator(hash);
    enum RETURNCODE ret;
    do
    {
        ret = Step();

        if (IsGraphicsShutdown())
            break;

    } while (ret == OK || ret == EXIT);

    GraphicsQuit();
    return 0;
}
