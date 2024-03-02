#pragma once

#include "../emul/platform.h"

#include <string>

void InitTextToSpeech();
void SayText(std::string text, int raceNum);
void StopSpeech();