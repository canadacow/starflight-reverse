#ifndef FINDWORD_H
#define FINDWORD_H

int GetOverlayIndex(int address);
int FindClosestWord(int si, int ovidx);
char* GetOverlayName(int word, int ovidx);
const char* FindWord(int word, int ovidx);
const char* FindWordCanFail(int word, int ovidx, bool canFail);
int FindWordByName(char* s, int n);
char *FindDirectoryName(int idx);

#endif