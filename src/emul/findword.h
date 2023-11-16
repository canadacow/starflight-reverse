#ifndef FINDWORD_H
#define FINDWORD_H

#include"../../starflt1-out/data/dictionary.h"

int GetOverlayIndex(int address, const char** overlayName);
int FindClosestWord(int si, int ovidx);
const char* GetOverlayName(int word, int ovidx);
const char* FindWord(int word, int ovidx);
const char* FindWordCanFail(int word, int ovidx, int canFail);
int FindWordByName(char* s, int n);
const char *FindDirectoryName(int idx);

const WORD* GetWord(int word, int ovidx);

#endif