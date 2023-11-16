#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <string>
#include <algorithm>

#include <unordered_map>

#include"../cpu/cpu.h"
#include"callstack.h"
#include"../disasOV/global.h"

#include"../../starflt1-out/data/dictionary.h"
#include"../../starflt1-out/data/directory.h"

const char* GetOverlayName(int word, int ovidx)
{
    if (word < (FILESTAR0SIZE+0x100)) return "STARFLT";
    return (ovidx==-1)?"STARFLT":overlays[ovidx].name;
}

int FindClosestWord(int si, int ovidx)
{
    int dist = 0x10000;
    int i = 0;
    int word = -1;
    do
    {
        if ((dictionary[i].ov != ovidx) && (dictionary[i].ov != -1)) continue;
        int d = si - dictionary[i].word;
        if (d < 0) continue;
        if (d < dist)
        {
            dist = d;
            word = dictionary[i].word;
        }
    } while(dictionary[++i].name != NULL);
    return word;
}

// ugly code to get our own overlay index from the address in the star file
int GetOverlayIndex(int address, const char** overlayName)
{
    struct OverlayData
    {
        std::string name;
        int index;
    };

    static std::unordered_map<int, OverlayData> overlapMap;

    if(overlapMap.empty())
    {
        overlapMap[0] = {"", -1};
    }

    auto it = overlapMap.find(address);

    if(it != overlapMap.end())
    {
        if(overlayName != nullptr)
        {
            *overlayName = it->second.name.c_str();
        }
        return it->second.index;
    }

    for(int i = 0; dir[i].name != NULL; i++)
    {
        if ((address<<4) == dir[i].start)
        {
            int idx = dir[i].idx;
            for(int j = 0; overlays[j].name != NULL; j++)
            {
                if (overlays[j].id == idx) 
                {
                    OverlayData od{};
                    od.index = j;
                    od.name = std::string(overlays[j].name);
                    auto itNew = overlapMap.emplace(address, od);

                    if(overlayName != nullptr)
                    {
                        *overlayName = itNew.first->second.name.c_str();
                    }

                    return od.index;
                }
            }
        }
    }

    fprintf(stderr, "Error: Cannot find index for address 0x%04x\n", address);
    exit(1);
}

const WORD* GetWord(int word, int ovidx)
{
    if (ovidx == -1) ovidx = GetOverlayIndex(Read16(0x55a5), nullptr); // "OV#"

    for(int i = 0; dictionary[i].name != NULL; i++)
    {
        if(dictionary[i].ov != -1 && dictionary[i].ov != ovidx)
            continue;

        if (word == dictionary[i].word)
        {
            return &dictionary[i];
        }
    }

    return nullptr;
}

static std::unordered_map<int, std::unordered_map<int, const char*>> s_wordDictionary{};

const char* FindWordCanFail(int word, int ovidx, int canFail)
{
    if (ovidx == -1) ovidx = GetOverlayIndex(Read16(0x55a5), nullptr); // "OV#"

    auto& overlayDictionary = s_wordDictionary[ovidx];
    auto it = overlayDictionary.find(word);
    if (it != overlayDictionary.end()) return it->second;

    int i = 0;
    do
    {
        if ((dictionary[i].ov != ovidx) && (dictionary[i].ov != -1)) continue;
        if (word == dictionary[i].word) 
        {
            overlayDictionary[word] = dictionary[i].name;
            return dictionary[i].name;
        }
    } while(dictionary[++i].name != NULL);
    if (word == 0x0) return "";

    if(canFail == 0)
    {
        fprintf(stderr, "Error: Cannot find word 0x%04x\n", word);
        exit(1);
   }

   return "unknown";
}

const char* FindWord(int word, int ovidx)
{
    return FindWordCanFail(word, ovidx, 0);
}

int FindWordByName(char* s, int n)
{
    if (n == 0) return 0;
    int ovidx = GetOverlayIndex(Read16(0x55a5), nullptr); // "OV#"

    std::string temp(s, n);

    int i = 0;
    do
    {
        if ((dictionary[i].ov != ovidx) && (dictionary[i].ov != -1)) continue;
        if (std::equal(temp.begin(), temp.end(), dictionary[i].name, [](char a, char b) { return tolower(a) == tolower(b); })) return dictionary[i].word;
    } while(dictionary[++i].name != NULL);
    //fprintf(stderr, "Error: Cannot find string %s\n", s.c_str());
    return 0;
}

const char *FindDirectoryName(int idx)
{
    int i = 0;
    do
    {
        if (idx == dir[i].idx) return dir[i].name;
    } while(dir[++i].name != NULL);
    fprintf(stderr, "Error: Cannot find directory entry %i\n", idx);
    return NULL;
}
