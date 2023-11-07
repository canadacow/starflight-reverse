#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"../cpu/cpu.h"
#include"call.h"
#include"findword.h"
#include"graphics.h"
#include"../patch/patch.h"

#include <string>
#include <thread>
#include <semaphore>

void Continue(PollForInputType pollForInput)
{
    while(1)
    {
       enum RETURNCODE ret = Step(pollForInput);
       if (ret == ERROR) exit(1);
       if (ret != OK) return;
    }
}


int main(int argc, char *argv[]) {
    std::string hash;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--hash") == 0 && i + 1 < argc) {
            hash = argv[i + 1];
            i++;
        }
    }
   
    InitCPU();
    InitEmulator(hash);
    GraphicsInit();

    auto pollForInput = [](uint16_t* key)-> bool {
        if(key == nullptr)
        {
            return GraphicsHasKey();
        }

        *key = GraphicsGetKey();

        return true;
    };
    
    enum RETURNCODE ret;
    do
    {
        ret = Step(pollForInput);

    } while (ret == OK);

    //GraphicsWait();
    GraphicsQuit();
    return 0;
}
