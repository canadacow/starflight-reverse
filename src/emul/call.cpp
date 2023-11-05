#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<time.h>
#include<sys/time.h>

#include"call.h"
#include"../cpu/cpu.h"
#include"fract.h"
#include"graphics.h"
#include"callstack.h"
#include"findword.h"
#include"../disasOV/global.h"

#include <stack>
#include <assert.h>
#include <dirent.h>
#include <string>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <cmath>
#include <thread>
#include <deque>

#include <zstd.h>
#include <xxhash.h>

unsigned int debuglevel = 0;

const unsigned short cs = 0x192;
const unsigned short ds = 0x192;

unsigned short int regdi = REGDI; // points to word "OPERATOR"
unsigned short int cx = 0x0;
unsigned short int dx = 0x0;


// ------------------------------------------------

std::deque<uint16_t> inputbuffer{};

void FillKeyboardBufferString(const char *str)
{
  //printf("Interpret '%s'\n", str);
    int n = strlen(str);
    inputbuffer.clear();
    for(int i=0; i<n; i++)
    {
        char c = str[i];
        if (c == 0xa) c = 0xd;
        inputbuffer.push_back(c);
    }
}

void FillKeyboardBufferKey(unsigned short c)
{
    inputbuffer.clear();

    if (c == 0xa) c = 0xd;

    inputbuffer.push_back(c);
}

// ------------------------------------------------

void PrintCStack()
{
  int cxsp = Read16(0x54B0);
  int n = ((0x6398 - cxsp)&0xFF)/3;
  printf("=== CSTACK ===\n");
  for(int i=0; i<n+2; i++)
  {
    cxsp += 3;
    printf(" %i: 0x%06x\n", n-i-1, (Read8(cxsp+2)<<16) | Read16(cxsp));
  }
  printf("==============\n");
}
// ------------------------------------------------
void PrintCache()
{
  /*
  : .BUFSTUFF ( SEG -- )
    >R  I 0 L@ 6 .R
        I 2 LC@ 5 .R
        I 3 LC@ 5 .R
        R> 6 L@ OFFSET @ - 5 .R ;

  : .CACHE
    CR ."  C#   SEG   UPDATE  MT   BLK "
    CR ." LPREV:" LPREV @ .BUFSTUFF
    CR ." PREV :" PREV  @ .BUFSTUFF
    CR ." USE  :" USE   @ .BUFSTUFF
    #CACHE @ 0 DO CR I 5 .R  I 'CACHE ! [SEGCACHE] L@
                  .BUFSTUFF KEY DROP LOOP ;
  */
  // Logic from word .CACHE in disys.txt
  printf("C#     SEG    UPDATE MT   BLK\n");
  unsigned short seg = Read16(0x2c6c);
  printf("LPREV: 0x%04x 0x%02x   0x%02x 0x%04x\n", Read16Long(seg, 0), Read8Long(seg, 2), Read8Long(seg, 3), (unsigned short int)Read16Long(seg, 6)-Read16(0x2c79));
  seg = Read16(0x2c84);
  printf("PREV : 0x%04x 0x%02x   0x%02x 0x%04x\n", Read16Long(seg, 0), Read8Long(seg, 2), Read8Long(seg, 3), (unsigned short int)Read16Long(seg, 6)-Read16(0x2c79));
  seg = Read16(0x2cbe);
  printf("USE  : 0x%04x 0x%02x   0x%02x 0x%04x\n", Read16Long(seg, 0), Read8Long(seg, 2), Read8Long(seg, 3), (unsigned short int)Read16Long(seg, 6)-Read16(0x2c79));
  int ncache = Read16(0x09ef);
  for(int i=0; i<ncache; i++)
  {
    seg = Read16Long(Read16(0x2c9d), 2*i); // SEGCACHE:2*i
    printf("%5i  ", i);
    printf("0x%04x 0x%02x   0x%02x 0x%04x\n", Read16Long(seg, 0), Read8Long(seg, 2), Read8Long(seg, 3), (unsigned short int)Read16Long(seg, 6)-Read16(0x2c79));
  }

}


// ------------------------------------------------

uint8_t STARA[256000];
uint8_t STARA_ORIG[256000];
uint8_t STARB[362496];
uint8_t STARB_ORIG[362496];

bool Serialize(uint64_t& combinedHash, std::string& filename)
{
    uint64_t hashA = XXH64(STARA, sizeof(STARA), 0);
    combinedHash = XXH64(STARB, sizeof(STARB), hashA);

    std::stringstream ss;
    ss << std::hex << combinedHash;
    filename = "sf-" + ss.str() + ".zst";
    
    FILE* file = fopen(filename.c_str(), "wb");
    if (file == NULL)
    {
        printf("Could not open file %s\n", filename.c_str());
        return false;
    }

    ZSTD_CCtx* cctx = ZSTD_createCCtx();
    if (cctx == NULL)
    {
        printf("Could not create ZSTD_CCtx\n");
        fclose(file);
        return false;
    }

    // Calculate the total size of STARA and STARB
    size_t totalSize = sizeof(STARA) + sizeof(STARB);

    // Create a buffer to hold both STARA and STARB
    std::vector<unsigned char> combinedData(totalSize);

    // Copy STARA and STARB into the combined buffer
    memcpy(combinedData.data(), STARA, sizeof(STARA));
    for(size_t i = 0; i < sizeof(STARA); i++)
    {
        combinedData[i] ^= STARA_ORIG[i];
    }

    memcpy(combinedData.data() + sizeof(STARA), STARB, sizeof(STARB));
    for(size_t i = 0; i < sizeof(STARB); i++)
    {
        combinedData[i + sizeof(STARA)] ^= STARB_ORIG[i];
    }

    // Compress the combined buffer
    std::vector<unsigned char> compressedData(ZSTD_compressBound(totalSize));
    size_t const cSize = ZSTD_compressCCtx(cctx, compressedData.data(), compressedData.size(), combinedData.data(), totalSize, 1);
    if (ZSTD_isError(cSize))
    {
        printf("Error compressing data: %s\n", ZSTD_getErrorName(cSize));
        ZSTD_freeCCtx(cctx);
        fclose(file);
        return false;
    }

    // Write the compressed data to the file
    fwrite(compressedData.data(), 1, cSize, file);

    ZSTD_freeCCtx(cctx);
    fclose(file);
    return true;
}

bool Deserialize(const std::string& filename)
{
    FILE* file = fopen(filename.c_str(), "rb");
    if (file == NULL)
    {
        printf("Could not open file %s\n", filename.c_str());
        return false;
    }

    ZSTD_DCtx* dctx = ZSTD_createDCtx();
    if (dctx == NULL)
    {
        printf("Could not create ZSTD_DCtx\n");
        fclose(file);
        return false;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    size_t compressedSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read the compressed data from the file
    std::vector<unsigned char> compressedData(compressedSize);
    fread(compressedData.data(), 1, compressedSize, file);

    // Prepare a buffer for the decompressed data
    size_t totalSize = sizeof(STARA) + sizeof(STARB);
    std::vector<unsigned char> decompressedData(totalSize);

    // Decompress the data
    size_t const dSize = ZSTD_decompressDCtx(dctx, decompressedData.data(), decompressedData.size(), compressedData.data(), compressedSize);
    if (ZSTD_isError(dSize))
    {
        printf("Error decompressing data: %s\n", ZSTD_getErrorName(dSize));
        ZSTD_freeDCtx(dctx);
        fclose(file);
        return false;
    }

    // Copy the decompressed data to STARA and STARB
    memcpy(STARA, decompressedData.data(), sizeof(STARA));
    for(size_t i = 0; i < sizeof(STARA); i++)
    {
        STARA[i] ^= STARA_ORIG[i];
    }

    memcpy(STARB, decompressedData.data() + sizeof(STARA), sizeof(STARB));
    for(size_t i = 0; i < sizeof(STARB); i++)
    {
        STARB[i] ^= STARB_ORIG[i];
    }

    ZSTD_freeDCtx(dctx);
    fclose(file);
    return true;
}



void HandleInterrupt()
{
    static int disktransferaddress_segment = -1;
    static int disktransferaddress_offset = -1;

    #pragma pack(push, 1)
    struct FCB {
        uint8_t driveNumber; // 0 = default, 1 = A:, 2 = B:, etc.
        char filename[8]; // Filename (8 bytes, padded with spaces)
        char extension[3]; // Extension (3 bytes, padded with spaces)
        uint16_t currentBlockNumber; // Current block number
        uint16_t recordSize; // Record size in 128-byte records
        uint32_t fileSize; // File size in records
        uint16_t dateOfLastWrite; // Date of last write (in DOS date format)
        uint16_t timeOfLastWrite; // Time of last write (in DOS time format)
        uint8_t reserved[8]; // Reserved
        uint8_t recordWithinCurrentBlock; // Record within current block
        uint32_t randomRecordNumber; // Random record number
    };
    #pragma pack(pop)

    int i = 0;
    int interrupt = Pop();
    mem[0x16C9] = interrupt;
    int flags = Read16(0x16b4);
    int ax = Read16(0x16b6);
    int bx = Read16(0x16b8);
    cx = Read16(0x16ba);
    dx = Read16(0x16bc);
    int ds = Read16(0x16c4);
    int es = Read16(0x16c6);
    int tempdi = Read16(0x16c0);
    int tempsi = Read16(0x16c2);

    //printf("interrupt 0x%x with ax=0x%04x bx=0x%04x flags=%x es=0x%04x\n", interrupt, ax, bx, flags, es);

    if ((interrupt == 0x10) && ((ax>>8) == 0xF))
    {
        // get current video mode
        ax = 0x3;
    } else
    if (interrupt == 0x11)
    {
        // return equipment list ???
        ax = 0xd426;
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x0D))
    {
        // drive reset
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x0F))
    {
        // open file
        ax = 0x0;
        Write8(tempdi, 0x3); // drive number
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x10))
    {
        // close file
        ax = 0x0;
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x1a))
    {
        // set disk transfer address
        //printf("  set disk transfer to 0x%04x:0x%04x\n", ds, dx);
        disktransferaddress_segment = ds;
        disktransferaddress_offset = dx;
        // set to 0x117B:0x0008
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x19)) // get default drive
    {
        ax = 0x2;
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x22))
    {
        const FCB* fcb = (const FCB*)&mem[dx];

        auto size = fcb->fileSize;
        auto block = fcb->randomRecordNumber;

        const uint8_t* dataSource = (const uint8_t*)&m[(disktransferaddress_segment<<4)+disktransferaddress_offset];

        auto filename = std::string(fcb->filename);
        filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());

        size_t recordSizeBytes = fcb->recordSize;

        size_t offset = fcb->randomRecordNumber * fcb->recordSize;

        uint8_t* fileTarget = nullptr;
        if(filename == "STARA")
        {
            fileTarget = (uint8_t*)&STARA[offset];
        }
        else if(filename == "STARB")
        {
            fileTarget = (uint8_t*)&STARB[offset];
        }
        else
        {
            assert(false);
        }

        memcpy(fileTarget, dataSource, recordSizeBytes);

        printf("Write %s block=%zu size=%zu\n", filename.c_str(), offset, recordSizeBytes);
        ax = 0x0;
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x21))
    {
        // random read
        //record size

        const FCB* fcb = (const FCB*)&mem[dx];

        auto size = fcb->fileSize;
        auto block = fcb->randomRecordNumber;

        auto filename = std::string(fcb->filename);
        filename.erase(std::remove_if(filename.begin(), filename.end(), ::isspace), filename.end());

        size_t offset = fcb->randomRecordNumber * fcb->recordSize;

        const uint8_t* fileSource = nullptr;
        if(filename == "STARA")
        {
            fileSource = (const uint8_t*)&STARA[offset];
        }
        else if(filename == "STARB")
        {
            fileSource = (const uint8_t*)&STARB[offset];
        }
        else
        {
            assert(false);
        }

        uint8_t* dataTarget = (uint8_t*)&m[(disktransferaddress_segment<<4)+disktransferaddress_offset];
        size_t recordSizeBytes = fcb->recordSize;

        memcpy(dataTarget, fileSource, recordSizeBytes);

        printf("Read %s block=%zu size=%zu\n", filename.c_str(), offset, recordSizeBytes);

        ax = 0x0;
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x11)) // find first file
    {
        // Search for first entry using FCB
        //printf("  Load ");
        //for(i=0; i<11; i++) printf("%c", mem[dx+1+i]);
        //printf("\n");

        // file found
        ax = 0x0;
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x4A))
    {
        // modify allocated memory block
        //printf("modify allocated memory block: segment=0x%04x new size=0x%04x\n", es, bx);
        ax = 0x1FE;
        bx = bx;
        //for(i=0xFFD0; i<0x10000; i++) mem[i] = 0x0;
    } else
    if ((interrupt == 0x21) && ((ax>>8) == 0x29))
    {
        // Parse a Filename for FCB
        //printf("  ds:si 0x%x:0x%x\n", ds, tempsi);
        //printf("  es:di 0x%x:0x%x\n", es, tempdi);
        //printf("  Load ");
        //for(i=0; i<11; i++) printf("%c", mem[tempsi+i]);
        //printf("\n");
        for(i=0; i<5; i++) mem[tempdi+i+1] = mem[tempsi+i]; // STARA or STARB
        mem[tempdi+0x0] = 0x0; // drive number
        // filesize
        if (mem[tempsi+4] == 'B')
        {
            // 354 kB
            mem[tempdi+0x10] = 0x00;
            mem[tempdi+0x11] = 0x88;
            mem[tempdi+0x12] = 0x05;
            mem[tempdi+0x13] = 0x00;

        } else
        {
            // 256 kB
            mem[tempdi+0x10] = 0x00;
            mem[tempdi+0x11] = 0xE8;
            mem[tempdi+0x12] = 0x03;
            mem[tempdi+0x13] = 0x00;
        }

        ax = 0;
        bx = 0;
    } else
    {
        fprintf(stderr, "unknown interrupt request\n");
        exit(1);
    }
    Write16(0x16b4, flags); //flags
    Write16(0x16b6, ax);
    Write16(0x16b8, bx);
    Write16(0x16ba, cx);
    Write16(0x16bc, dx);
    Write16(0x16c0, tempdi);
    Write16(0x16c4, ds);
    Write16(0x16c6, es);
}

void XCHG(unsigned short *a, unsigned short *b)
{
    unsigned short temp = *a;
    *a = *b;
    *b = temp;
}

void ParameterCall(unsigned short bx, unsigned short addr)
{
    // call word 0x1649;
    //printf("Parametercall addr=%04x, si=%04x bx=%04x content=0x%04x\n", addr, regsi, bx+2, Read16(bx+2));

    regbp -= 2;
    Write16(regbp, regsi);
    DefineCallStack(regbp, 1);

    // next address after the call contains forth code, so change pointer
    Push(addr+3);
    regsi = Pop();

    bx += 2; // push address of variable in the overlays
    Push(bx);
}


void LXCHG16(unsigned short es, unsigned short bx, unsigned short ax) //  "{LXCHG}"
{
    unsigned short cx = Read16Long(es, bx);
    unsigned short temp = Read16Long(es, ax);
    Write16Long(es, ax, cx);
    Write16Long(es, bx, temp);
// 0x2f36: push   cx
// 0x2f38: mov    cx,es:[bx]
// 0x2f3a: xchg   ax,bx
// 0x2f3d: xchg   es:[bx],cx
// 0x2f3f: xchg   ax,bx
// 0x2f42: mov    es:[bx],cx
// 0x2f44: pop    cx
// 0x2f45: ret
}

void LXCHG8(unsigned short es, unsigned short bx, unsigned short ax)
{
    unsigned short cx = Read8Long(es, bx);
    unsigned short temp = Read8Long(es, ax);
    Write8Long(es, ax, cx&0xFF);
    Write8Long(es, bx, temp&0xFF);
// 0x49cc: mov    cl,es:[bx]
// 0x49ce: xchg   ax,bx
// 0x49d1: xchg   es:[bx],cl
// 0x49d3: xchg   ax,bx
// 0x49d6: mov    es:[bx],cl
}


void Find() // "(FIND)"
{
    //Find word in the vocabulary
    unsigned short bx = Pop(); // first entry in vocabulary
    unsigned short cx = Pop(); // length and string of entry
    int n = Read8(cx);
/*
    printf("Find: '");
    for(int i=0; i<n; i++)
        printf("%c", Read8(cx+1+i));
    printf("'\n");
*/

// ------------------------------------
// Implementation on our own vocabulary
// ------------------------------------

    if (n == 0)
    {
        Push(0x253e);
        Push(0xC0);
        Push(1);
        return;
    }
    int word = FindWordByName((char*)&mem[cx+1], n);
    //printf("Found 0x%04x\n", word);
    if (word == 0x0)
    {
        //Push(0);
        //return;
    } else
    {
        Push(word);
        Push(0x80 + n); // bitfield. bit 7 is always 1, bit 6 the IMMEDIATE flag, bit 5 is the SMUDGE flag, bits 0-4 store the word length.
        Push(1);
        return;
    }

// ----------------------

    Push(regsi);
    Push(regdi);
    unsigned char al, ah;
    unsigned char dl, dh;
    dl = 0x3F;
    dh = 0x7F;
    //printf("first vocabulary entry address=%x\n  word to search at address=%x\n", bx, cx);

    while(1)
    {
        //printf("0x%04x\n",bx);
        if (bx == 0) // word is not found, return 0
        {
            //printf("  word not found\n");
            //x1859:
            regdi = Pop();
            regsi = Pop();
            Push(0);
            return;
        }

        // x182A:
        regsi = cx;
        regdi = bx;
        al = Read8(regsi);  // get number of letters of word to search
        regsi++;
        ah = Read8(regdi) & dl; // get number of letters in word

        if (ah != al) // length don't match, go to next word entry
        {
            //1822:
            bx -= 2;
            bx = Read16(bx);
            continue;
        }
        x1837:
        do
        {
            regdi++;
            al = Read8(regsi);
            regsi++;
            ah = Read8(regdi);
            //printf("find %c at 0x%04x\n", (ah)&0x7F, di);
            ah = ah ^ al;

        } while(ah == 0);

        ah = ah & dh;
        if (ah != 0) // doesn't match, go to next word entry
        {
            bx -= 2;
            bx = Read16(bx);
            continue;
        }
        //0x1843:
        unsigned short ax = regdi;
        regdi = Pop();
        regsi = Pop();
        ax += 3;
        Push(ax);
        dl = Read8(bx);
        dh = ah;
        Push(dl);
        Push(1);
        //printf("  word found at bx=0x%04x ax=0x%04x dl=0x%04x\n", bx, ax, (dh<<8)|dl);
        return;
    }
}

void POLY_WINDOW_FILL()
{
    uint16_t color = Pop();
    uint16_t x1 = Pop();
    uint16_t y1 = Pop();
    uint16_t x0 = Pop();
    uint16_t y0 = Pop();

    if(x0 > x1)
    {
        uint16_t temp = x0;
        x0 = x1;
        x1 = temp;
    }
    if(y0 > y1)
    {
        uint16_t temp = y0;
        y0 = y1;
        y1 = temp;
    }

    for(uint16_t y = y0; y <= y1; ++y)
    {
        for(uint16_t x = x0; x <= x1; ++x)
        {
            GraphicsPixel(x, y, color, 0);
        }
    }
}

#pragma pack(push, 1)

struct ConditionIndex {
    uint8_t value;

    bool isNegated() const {
        return (value & 0x80) == 0;
    }

    uint8_t getIndex() const {
        return value & 0x7F;
    }
};

struct Rule {
    uint8_t conditionCount;
    uint16_t forthWord;
    ConditionIndex conditionIndexes[1]; // Use as a flexible array member

    enum RETURNCODE Execute(uint16_t bx, const uint16_t* conditions, uint8_t* flagCache, PollForInputType pollForInput) const
    {
        // Not sure if we must evaluate all conditions anyway as
        // a way to cache them and if that's valuable. We'll see.
        bool result = true;

        for(uint8_t condIdx = 0; condIdx < conditionCount; ++condIdx)
        {
            auto executeWord = [&](uint16_t execWord) -> enum RETURNCODE
            {
                uint16_t auxSi = regsi;

                auto word = GetWord(execWord, -1);
                auto ret = Call(word->code, word->word - 2, pollForInput);
                if (ret != OK) return ret;

                while(regsi != auxSi)
                {
                    ret = Step(pollForInput);
                    if (ret != OK) return ret;
                }

                return OK;
            };

            auto& condition = conditionIndexes[condIdx];

            auto conditionWord = conditions[condition.getIndex()];
            auto* cacheValue = &flagCache[condition.getIndex()];

            uint16_t poppedValue = *cacheValue;
            if(poppedValue == 0xff)
            {
                auto ret = executeWord(conditionWord);
                if (ret != OK) return ret;
                
                poppedValue = Pop();
                *cacheValue = poppedValue & 0xff;
            }

            if(condition.isNegated())
            {
                result = result && (poppedValue == 0);
            }
            else
            {
                result = result && (poppedValue != 0);
            }

            if((result) && (condIdx + 1 == conditionCount))
            {
                auto ret = executeWord(forthWord);
                if (ret != OK) return ret;
            }
        }

       return OK;
    }
};

#include <sys/mman.h>

struct RuleMetadata {
    uint8_t ruleIndexMax;       // RULEIM
    uint8_t conditionLimit;     // CONDLIM
    uint8_t ruleCount;          // RULECNT
    uint16_t rulePointers[1];   // RULEARRp

    const Rule* getRuleAtIndex(uint8_t idx) const {
        uint16_t ruleAddr = rulePointers[idx];
        return reinterpret_cast<Rule*>(&mem[ruleAddr]);
    }

    uint16_t* getConditionArray() const {
        return (uint16_t*)((uint8_t*)&rulePointers[0] + (2 * ruleIndexMax));
    }

    uint8_t* getFlagCache() const {
        return ((uint8_t*)&rulePointers[0] + (2 * ruleIndexMax) + (2 * conditionLimit));
    }

    enum RETURNCODE Execute(uint16_t bx, PollForInputType pollForInput) const
    {
        for(uint8_t ruleIdx = 0; ruleIdx < ruleCount; ++ruleIdx)
        {
            auto rule = getRuleAtIndex(ruleIdx);
            auto ret = rule->Execute(bx, getConditionArray(), getFlagCache(), pollForInput);
            if (ret != OK) return ret;
        }

        return OK;
    }
};

#pragma pack(pop)

struct EGAFunctions
{
    void TREJECT_00A5()
    {
        uint8_t& al = reinterpret_cast<uint8_t*>(&ax)[0];
        uint8_t& bl = reinterpret_cast<uint8_t*>(&bx)[0];
        uint8_t& bh = reinterpret_cast<uint8_t*>(&bx)[1];
        uint8_t& dl = reinterpret_cast<uint8_t*>(&dx)[0];

        Push(di);
        Push(si);

        bl = 0x0F;
        bh = 0;

        si = Read16(0x569B); // VIN
        di = Read16(0x56B0); // OIN
        uint16_t bp = Read16(0x5752); // IBELOW
        cx = Read16(0x5686); // #IN

        dx = 0;

        do {
            ax = Read16(si);
            si += 2;

            if (ax >= bp) {
                dx |= 0x04;
            }

            if (ax <= Read16(0x575F)) { // IABOVE
                dx |= 0x08;
            }

            ax = Read16(si);
            si += 2;

            if (ax >= Read16(0x5738)) { // ILEFT
                dx |= 0x01;
            }

            if (ax <= Read16(0x5745)) { // IRIGHT
                dx |= 0x02;
            }

            ax = dx;
            Write8(di, al);
            di++;

            bl &= dl;
            bh |= dl;

            cx--;
        } while (cx != 0);

        ax = 0;
        ax = bl;
        Write16(0x56E8, ax); // TRJCT

        ax = bh;
        Write16(0x56DC, ax); // TACCPT

        si = Pop();
        di = Pop();
    }

    void WRITEDO_0246()
    {
        uint8_t color = Read8(0x55F2);

        uint16_t x = (bx % 40) * 8;
        uint16_t y = bx / 40;

        GraphicsPixel(x, y, color, 0);
    }

    void RLPLOT()
    {
        dx += Read16(0x5A6E); // EY
        ax += Read16(0x5A65); // EX
        Push(di);
        Push(ax);
        Push(dx);
        bx = Read16(0x569B); // VIN
        Write16(bx, dx);
        bx += 2;
        Write16(bx, ax);
        bx = 1;
        Write16(0x5686, bx); // #IN
        TREJECT_00A5();
        dx = Pop();
        ax = Pop();
        di = Pop();
        cx = Read16(0x56DC); // TACCPT
        if (cx != 0) {
            return;
        }
        dx <<= 1;
        dx += Read16(0x563A); // YTABL
        Push(dx);
        Push(ax);
        ax &= 0x0003;
        ax <<= 1;
        bx = 0x92CF;
        bx += ax;
        dx = Read16(bx);
        ax = Pop();
        ax >>= 1;
        ax >>= 1;
        bx = Pop();
        ax += Read16(bx);
        bx = ax;
        Push(es);
        cx = Read16(0x5648); // BUF-SEG
        es = cx;
        WRITEDO_0246();
        es = Pop();
    }

    void OneHalf_0B20()
    {
        XASP_0B06(); // Sets ax to a quotient value of two numbers in memory

        Push(ax);
        Push(dx);
        RLPLOT(); // Function call to RLPLOT

        dx = Pop();
        ax = Pop();
        Push(ax);
        Push(dx);
        ax = -ax;
        RLPLOT(); // Function call to RLPLOT

        dx = Pop();
        ax = Pop();
        Push(ax);
        Push(dx);
        ax = -ax;
        dx = -dx;
        RLPLOT(); // Function call to RLPLOT

        dx = Pop();
        ax = Pop();
        Push(ax);
        Push(dx);
        dx = -dx;
        RLPLOT(); // Function call to RLPLOT

        dx = Pop();
        ax = Pop();
    }

    void S_OneHalf_0B53()
    {
        uint8_t& bl = reinterpret_cast<uint8_t*>(&bx)[0];

        // call 0B06
        XASP_0B06();
        
        bx = Read16(0x5A65); // mov bx,[5A65] // EX

        bx += ax;

        if (bx <= Read16(0x5745)) { // cmp bx,[5745] // IRIGHT
            bx = Read16(0x5745); // mov bx,[5745] // IRIGHT
        }

        if (bx >= Read16(0x5738)) { // cmp bx,[5738] // ILEFT
            bx = Read16(0x5738); // mov bx,[5738] // ILEFT
            bx--;
        }

        Write8(0x48C7, bl); // mov [48C7],bl
        
        bx = Read16(0x5A65); // mov bx,[5A65] // EX

        bx -= ax;

        if (bx >= Read16(0x5738)) { // cmp bx,[5738] // ILEFT
            bx = Read16(0x5738); // mov bx,[5738] // ILEFT
        }

        if (bx <= Read16(0x5745)) { // cmp bx,[5745] // IRIGHT
            bl = Read8(0x48C7); // mov bl,[48C7]
            bx++;
        }

        Write8(0x48C6, bl); // mov [48C6],bl // ZZZ

        cx = Read16(0x5A6E); // mov cx,[5A6E] // EY

        cx += dx;

        if (cx <= Read16(0x575F)) { // cmp cx,[575F] // IABOVE
            if (cx >= Read16(0x5752)) { // cmp cx,[5752] // IBELOW
                bx = cx;
                bx <<= 1;
                bx += Read16(0x57D9); // add bx,[57D9] // SCAN
                Push(Read16(0x48C6)); // push word ptr [48C6] // ZZZ
                Write16(bx, Pop()); // pop word ptr [bx]
            }
        }

        cx = Read16(0x5A6E); // mov cx,[5A6E] // EY
        
        cx -= dx; // sub cx,dx
        
        if (cx >= Read16(0x5752)) { // cmp cx,[5752] // IBELOW
            if (cx <= Read16(0x575F)) { // cmp cx,[575F] // IABOVE
                bx = cx;
                bx <<= 1;
                bx += Read16(0x57D9); // add bx,[57D9] // SCAN
                Push(Read16(0x48C6)); // push word ptr [48C6] // ZZZ
                Write16(bx, Pop()); // pop word ptr [bx]
            }
        }
    }

    void SCANELLIP_0BE7()
    {
        Push(ax);
        S_OneHalf_0B53();
        ax = Pop();
        std::swap(dx, ax);
        S_OneHalf_0B53();
    }

    void ELLIP_0C01()
    {
        Push(ax);
        OneHalf_0B20();
        ax = Pop();
        std::swap(dx, ax);
        OneHalf_0B20();
    }

    void XASP_0B06()
    {
        uint16_t square = Read16(0x5A86); // XNUMER
        uint16_t xdenom = Read16(0x5A93); // XDENOM
        int32_t temp = (int32_t)square * (int32_t)square; // imul dx
        ax = (uint16_t)(temp / xdenom); // idiv cx
    }

    void EEXTENT()
    {
        //Write16(0x6A89, Pop()); // LRTRN
        //Write16(0x6A8B, Pop()); // LRTRN
        ax = Read16(0x5A79); // ERAD
        dx = ax;
        XASP_0B06();
        cx = Read16(0x5A65); // EX
        cx -= ax;
        Push(cx);
        cx = Read16(0x5A6E); // EY
        cx -= dx;
        Push(cx);
        cx = Read16(0x5A65); // EX
        cx += ax;
        Push(cx);
        cx = Read16(0x5A6E); // EY
        cx += dx;
        Push(cx);
        //Push(Read16(0x6A8B)); // LRTRN
        //Push(Read16(0x6A89)); // LRTRN
    }

    void W0C5A()
    {
        uint16_t callAddr = Read16Long(cs, 0x0C5A);
        switch(callAddr)
        {
            case 0xbe7: // ({SCANELLIP} )
                SCANELLIP_0BE7();
                break;
            case 0xc01: // ({.ELLIP} ) '
                ELLIP_0C01();
                break;
            default:
                printf("ARC? Call addr 0x%x\n", callAddr);
                //assert(false);
                break;
        }
    }

    void ARC()
    {
        printf("TODO ARC\n");
        ax = Read16(0x5AA0); // <ARC1>
        Write16Long(cs, 0x0C5A, ax); // W0C5A
        ax = Read16(0x5A79); // ERAD
        dx = ax;
        ax <<= 1;
        cx = 0x0003;
        cx -= ax;
        ax = 0;
        if (ax >= dx) {
            goto label0CBE;
        }
        do {
            Push(ax);
            Push(cx);
            Push(dx);
            W0C5A();
            dx = Pop();
            cx = Pop();
            ax = Pop();
            if (cx >= 0) {
                cx += 0x06;
                uint16_t bx = ax;
                bx <<= 2;
                cx += bx;
            } else {
                cx += 0x0A;
                uint16_t bx = ax;
                bx -= dx;
                bx <<= 2;
                cx += bx;
                dx--;
            }
            ax++;
        } while (ax < dx);
        label0CBE:
        if (dx != ax) {
            W0C5A();
        }
    }

    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
    uint16_t di;
    uint16_t si;
    uint16_t es;
};

EGAFunctions ega{};

enum RETURNCODE Call(unsigned short addr, unsigned short bx, PollForInputType pollForInput)
{
    unsigned short i;
    enum RETURNCODE ret;

    //int ovidx = GetOverlayIndex(Read16(0x55a5));
    //printf("Step 0x%04x addr 0x%04x - OV %s WORD 0x%04x %s\n", regsi-2, addr,  GetOverlayName(regsi, ovidx), bx+2, FindWordCanFail(bx+2, ovidx, true));

    // bx contains pointer to WORD
    if ((regsp < FILESTAR0SIZE+0x100) || (regsp > (0xF6F4)))
    {
        fprintf(stderr, "Error: stack pointer in invalid area: sp=0x%04x\n", regsp);
        PrintCallstacktrace(bx);
        return ERROR;
    }
    switch(addr)
    {
        // --- call functions ---

        case 0x224c: // call
            {
                uint16_t nextInstr = bx + 2;
                if(nextInstr == 0xa0f0)
                {
                    POLY_WINDOW_FILL();
                }
                else if (nextInstr == 0x2af1)
                {
                    // Sleep
                    auto sleepInMs = Pop();
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleepInMs));
                }
                else
                {
                    bx += 2;
                    regbp -= 2;
                    Write16(regbp, regsi);
                    regsi = bx;
                    DefineCallStack(regbp, 1);
                }
            }
        break;

        case 0xb869: // call rule
            {
                uint16_t nextInstr = bx + 2;
                auto meta = (const RuleMetadata*)&mem[nextInstr];
                auto res = meta->Execute(nextInstr, pollForInput);
                if (ret != OK) return ret;
            }
        break;

        case 0x1692: // "EXIT"
            regsi = Read16(regbp);
            regbp += 2;
        break;

        // --- branching ---
        case 0x1662: regsi += Read16(regsi); break; // BRANCH
        case 0x15FC: if (Pop() == 0) regsi += Read16(regsi); else regsi+=2; break; // 0BRANCH


        // --- callstack operations mainly for loops ---

        case 0x0E52: Push(Read16(regbp+0)); break; // "I"
        case 0x0E62: Push(Read16(regbp+2)); break; // "I'"
        case 0x0E70: Push(Read16(regbp+4)); break; // "J"

        // ---- call stack operations ---

        case 0x0EA4: Push(Read16(regbp+0)); break; // "R@"
        case 0x0E92: // "R>" pop variables from the callstack
            Push(Read16(regbp));
            regbp += 2;
        break;

        case 0xDB6: // ">R" push variables on the callstack
            bx = Pop();
            regbp -= 2;
            Write16(regbp, bx);
            DefineCallStack(regbp, 0);
        break;

        // ------------------------------

        case 0x3A39: // "@EXECUTE"
            bx = Read16(Pop()) - 2;
            //printf("jump to %x\n", bx);
            ret = Call(Read16(bx), bx, pollForInput);
            if (ret != OK) return ret;
        break;

        case 0x1675: // "CFAEXEC"
            bx = Pop();
            //printf("jump to %x\n", bx);
            ret = Call(Read16(bx), bx, pollForInput);
            if (ret != OK) return ret;
            break;

        case 0x1684: // "EXECUTE"
            bx = Pop() - 2;
            //printf("execute %s\n", FindWord(bx+2, -1));
            ret = Call(Read16(bx), bx, pollForInput);
            if (ret != OK) return ret;
        break;

        case 0x17B7: // Exec function pointers "CREATE" "TYPE", "CALL", "ABORT" "PAGE", ...
        {
            //printf("%04x\n", bx);
            //exit(1);
            int bxtemp = bx;
            bx += 2;  // points to data in word
            bx = Read16(bx);
            if (bx == 0x5a) // EMIT
            {
              //printf("EMIT: \n");
              //PrintCallstacktrace(bxtemp);
            }

            if (bx == 0x6e) // TYPE
            {
              int n = Read16(regsp);
              int offset = Read16(regsp+2);
              //printf("(TYPE): ");
              for(int i=0; i<n; i++)
                  printf("%c", Read8(offset+i));
              //if (Read8(offset+n-1) != '\n') printf("\n");
            }

            bx = Read16(bx+regdi);
            bx -= 2;

            const char *s = FindWord(bx+2, -1);
            //printf("Execute %s\n", s);
            if (strcmp(s, "(TYPE)") == 0)
            {
                int n = Read16(regsp);
                int offset = Read16(regsp+2);
                /*
                printf("(TYPE): ");
                for(int i=0; i<n; i++)
                    printf("%c", Read8(offset+i));
                printf("\n");
                */
                GraphicsText((char*)&mem[offset], n);
            }
            if (strcmp(s, "(CR)") == 0)
            {
                printf("\n");
                GraphicsCarriageReturn();
            }
            if (strcmp(s, "(POSITION)") == 0)
            {
                //printf("%i, %i\n", Read16(regsp), Read16(regsp+2));
                GraphicsSetCursor(Read16(regsp), Read16(regsp+2));
            }
            if (strcmp(s, "(EMIT)") == 0) // print one char
            {
                // TODO if terminal mode
                printf("%c", Read16(regsp));
                // TODO otherwise
                GraphicsChar(Read16(regsp));
                //GraphicsUpdate();
                //printf("(EMIT) %i\n", Read16(regsp));
            }
            if (strcmp(s, "(WORD)") == 0) // scans input stream for char and copies
            {
                //unsigned short offset = Read16(REGDI+0x12);
                //PrintCallstacktrace(bx);
                //printf("%i offset %i\n", Read16(regsp), offset);
                //exit(1);
            }
            if (strcmp(s, "(KEY)") == 0)
            {
                /*
                if (inputbuffer.size() == 0)
                {
                    regsi -= 2;
                    return INPUT;
                }
                */
            }

            ret = Call(Read16(bx), bx, pollForInput);
            if (ret != OK) return ret;
        }
        break;

        case 0x21C9: // TODO for forth interpreter, is incomplete
        {
            // Check for stack size and jump to error handling
            //printf("Info: %i 0x%x\n", Read16(0x0A0B), Read16(regdi+4));
            unsigned short ax = Read16(0x0A0B); // #SPACE (256)
            ax += Read16(regdi+4);
            if ((Read16(0x0A0B) + Read16(regdi+4)) > 0xFFFF)
            {
                fprintf(stderr, "TODO: Error handling part 1. Empty or full stack\n");
                exit(1);
            }
            // if error:
            /*
            bx = 0xB10;
            bx = Read16(bx);
            printf("Jump to %x\n", bx);
            exit(1);
            */
// 0x21c9: mov    ax,[0A0B] // #SPACE
// 0x21cc: add    ax,[di+04]
// 0x21cf: jo     21E0
// 0x21d1: cmp    ax,sp
// 0x21d3: ja     21E0
// 0x21d5: mov    ax,[di]
// 0x21d7: cmp    ax,sp
// 0x21d9: jb     21E0
// 0x21db: lodsw
// 0x21dc: mov    bx,ax
// 0x21de: jmp    word ptr [bx]
// 0x21e0: mov    bx,0B10
// 0x21e3: jmp    word ptr [bx]

        }
        break;

        case 0x16a3: // "GO"
            {
                uint64_t binHash = 0;
                std::string filename{};

                bool serialized = Serialize(binHash, filename);

                auto printSerialized = [&]{
                    if(serialized)
                    {
                        printf("Saved state to %s\n", filename.c_str());
                    }
                    else
                    {
                        printf("Failed to serialize state.\n");
                    }
                };

                bx = Pop();
                printf("jump to %x. Either stop (0x0) or restart (0x100)\n", bx);
                printSerialized();
                
                // Undo redirection
                fflush(stdout);
                fflush(stderr);
                freopen("/dev/stdout", "a", stdout);
                freopen("/dev/stderr", "a", stderr);
                printSerialized();
            }
            return EXIT;
        break;

        case 0x174C: Write16(regbp+2, Read16(regbp)); break; // "(LEAVE)"
        case 0x0EC8: regbp = Read16(regdi+2); break; // RP!

        case 0x83f8: // all overlays
            printf("Load overlay '%s'\n", FindWord(bx+2, -1));
            ParameterCall(bx, 0x83f8);
            break;
        case 0x5275: ParameterCall(bx, 0x5275); break; // "OVT" "IARRAYS"
        case 0x4ef5: ParameterCall(bx, 0x4ef5); break; // "BLACK DK-BLUE DL-GREE GREEN RED VIOLET BROWN ... WHITE"
        case 0x6ec1: ParameterCall(bx, 0x6ec1); break; // ":TIMEST :SIGNAT :CKSUM :SAVE :VERSIO"
        case 0x3aec: ParameterCall(bx, 0x3aec); break; // D@ V= C= <OFF> <ON> <BLOCK>
        case 0x3b68: ParameterCall(bx, 0x3b68); break; // "(2C:) NULL 0. VANEWSP IROOT .... *EOL"
        case 0x4a96: ParameterCall(bx, 0x4a96); break; // "CASE:"
        case 0x4a4f: ParameterCall(bx, 0x4a4f); break; // "CASE"
        case 0xeca2: ParameterCall(bx, 0xeca2); break; // in PL-SET Overlay
        case 0xe114: ParameterCall(bx, 0xe114); break; // in LP Overlay
        case 0xe2cf: ParameterCall(bx, 0xe2cf); break; // in LP Overlay
        case 0xe4f0: ParameterCall(bx, 0xe4f0); break; // in LP Overlay
        case 0xf0cc: ParameterCall(bx, 0xf0cc); break; // in STORM-OV
        case 0xe940: ParameterCall(bx, 0xe940); break; // in VITA-OV
        case 0xe555: ParameterCall(bx, 0xe555); break; // in HP-OV
        case 0xe420: ParameterCall(bx, 0xe420); break; // in HP-OV
        case 0xea73: ParameterCall(bx, 0xea73); break; // in SENT-OV
        case 0x4e00: ParameterCall(bx, 0x4e00); break; // Arrays
        case 0x744d: ParameterCall(bx, 0x744d); break; // "INST-SI" "INST-PR" "%NAME" "PHR-CNT" "TEXT-CO" "PHRASE$" ...
        //case 0x1AB5: ParameterCall(bx, 0x1AB5); break; // "FORTH MUSIC IT-VOC MISC-"
        case 0xe211: 
            {
                ParameterCall(bx, 0xe211); // in COMBAT-OV
                break; 
            }
        case 0x7227:
        {
            //printf("Receive %s from STAR*.COM dictionary for index 0x%x: '%s'\n", FindWord(bx+2, -1), Read16(regsp), FindDirectoryName(Read16(regsp)));
            //PrintCallstacktrace(bx);
            //printf("slen of =%i regsp=%i\n", Read16(regsp), regsp);
            const char *s = FindDirectoryName(Read16(regsp));
            if (s == NULL)
            {
              PrintCallstacktrace(bx);
              return ERROR;
            }

            //printf("Load data    '%s'\n", s);
            // "FILE-NA FILE-TY FILE-ST FILE-EN FILE-#R FILE-RL FILE-SL"
            ParameterCall(bx, 0x7227);
          }
        break;

        case 0x73ea: // load data
        {
          const char *s = FindDirectoryName(Read16(0x548f)); // FILE#
          if (s == NULL)
          {
            PrintCallstacktrace(bx);
            return ERROR;
          }
          printf("load adata FILE# '%s' RECORD# %i at regsi=0x%04x\n", s, Read16(0x549d), regsi-2);
          //printf("%i %i\n", Read16(0x547b), Read16(0x547f));
          /*
          if ((Read16(0x549d) == 51) && (regsi == 0xf01c+2))
          {
            printf("read from 51\n");
            PrintCache();
            PrintCStack();
            PrintCallstacktrace(bx);
            exit(1);
          }
          */
          ParameterCall(bx, 0x73ea);
        }
        break;

        // -----------------------------------

        case 0x8A2D: // calculate memory offset for given coordinates. Interleaved. Maybe CGA?
        {
            unsigned short ax = Pop() >> 1;
            bx = 199 - Pop();
            if (bx & 1)
            {
                ax += 0x2000;
                bx &= 0x00FE; // remove lower bit
            }
            Push(bx * 40 + ax);
        }
        break;

        case 0x25D7: // "KEY" read keyboard endless loop, executed by "0x17B7"
        {
            uint16_t key{};
            pollForInput(&key);
            Push(key);

            /*
            // 1. either low byte ascii, high byte 0
            // 2. or low byte scancode, high byte 1
            unsigned short c;
            c = GraphicsGetChar();
            //printf("key %i\n", c);
            if (c == 0xa) c = 0xd;
            Push(c);
            */
        }
        break;

        case 0x25bc: // "(?TERMINAL)" keyboard check buffer
            if(pollForInput(nullptr))
            {
                Push(1);
            } 
            else
            {
                Push(0);
            }
        break;

        case 0x1D29: Push(bx+2); break; // get pointer to variable or table

        case 0x2214: Push(Read16(bx+2)); break; // get constant

        case 0x175F: // read constant "LIT"
        {
            Push(Read16(regsi));
            regsi += 2;
        }
        break;

        case 0x1618: // read constant "2LIT"
            Push(Read16(regsi));
            regsi += 2;
            Push(Read16(regsi));
            regsi += 2;
        break;

        case 0xC3A: // 2@
            bx = Pop();
            Push(Read16(bx+2));
            Push(Read16(bx));
            //printf("read address %x:%x\n", Read16(bx+2), Read16(bx));
        break;

        case 0xC24: // 2!_2
        {
            bx = Pop();
            unsigned short ax = Pop();
            cx = Pop();
            Write16(bx, ax);
            Write16(bx+2, cx);
        }
        break;

        case 0x30a8: // "ADVANCE>DEF"
        {
            //printf("Advance>def %i\n", Read16(regsp));
            unsigned short ax = Pop();
            if (ax != 0)
            {
                bx = ax - 2;
                LXCHG16(Read16(0x2C04), bx, ax); // BLKCACHE
                LXCHG16(Read16(0x2C9D), bx, ax); // SEGCACHE
            }
        }
        break;

        case 0x4D5C:  // Get segment:offset in array
        {
          unsigned short ax;
          // get Segment
          bx = Pop()+6;
          ax = Read16(bx);

          // get size of array or pointer to pointer array
          bx -= 2;
          cx = Read16(bx);
          bx = cx;

          cx = Pop() << 1;
          bx += cx;
          cx = Read16Long(ax, bx);
          cx += Pop();
          Push(ax);
          Push(cx);

// 0x4d5c: pop    bx
// 0x4d5d: add    bx,06
// 0x4d60: mov    ax,[bx]
// 0x4d62: sub    bx,02
// 0x4d65: mov    cx,[bx]
// 0x4d67: mov    bx,cx

// 0x4d69: pop    cx
// 0x4d6a: shl    cx,1
// 0x4d6c: add    bx,cx

// 0x4d6e: push   ds
// 0x4d6f: mov    ds,ax
// 0x4d71: mov    cx,[bx]
// 0x4d73: pop    ds
// 0x4d74: pop    dx
// 0x4d75: add    cx,dx
// 0x4d77: push   ax
// 0x4d78: push   cx
        }
        break;

        case 0x4DA4: // "!OFFSET" sets 2D array pointers for faster access, like in C
        {
            unsigned short ax;
            bx = Pop();
            int tempsi = Read16(bx);
            int tempcx = Read16(bx+2);
            int tempbp = Read16(bx+4);
            unsigned short es = Read16(bx+6);
            int tempdi = (tempcx<<1)+tempbp;
            do
            {
                ax = tempsi*tempcx;
                Write16Long(es, tempdi, ax);
                tempdi-=2;
                tempcx--;
            } while(tempcx != 0);
            ax = 0;
            Write16Long(es, tempdi, ax);
        }
        break;

        case 0x160:  // "(!OLD)" read interrupt vector table
            bx = Pop();
            //printf("read interrupt %x\n", bx);
            if (bx == 0x9)
            {
                Push(0xf000);
                Push(0xe987);
            } else
            if (bx == 0x0)
            {
                Push(0xf000);
                Push(0x1060);
            } else
            if (bx == 0x1b)
            {
                Push(0xf000);
                Push(0x1060);
            } else
            if (bx == 0x1c)
            {
                Push(Read16Long(0, bx*4+2));
                Push(Read16Long(0, bx*4+0));
            } else
            if (bx == 0x24)
            {
                Push(0x118);
                Push(0x110);
            } else
            {
                Push(0);
                Push(0);
            }
        break;

        case 0x138: // "(!SET)" // write interrupt vector table
        {
            bx = Pop();
            unsigned short ax = Pop();
            cx = Pop();
            Write16Long(0, bx*4+0, ax);
            Write16Long(0, bx*4+2, cx);

            // Write out 1024 bytes at ax,cx to a file
            char filename[64];
            sprintf(filename, "%04x-%04x.bin", cx, ax);
            printf("write interrupt vector table - %s\n", filename);
            FILE *fp = fopen(filename, "wb");
            if (fp != NULL)
            {
                unsigned long addr = ComputeAddress(cx, ax);
                fwrite(&m[addr], 1, 1024, fp);
                fclose(fp);
            }

            if(bx == 0x1c)
            {
                unsigned short tempbx = 0;

                sprintf(filename, "%04x-%04x.bin", cx, tempbx);
                printf("write ISR jump - %s\n", filename);
                fp = fopen(filename, "wb");
                if (fp != NULL)
                {
                    unsigned long addr = ComputeAddress(cx, tempbx);
                    fwrite(&m[addr], 1, 1024, fp);
                    fclose(fp);
                }
            }
        }
        break;

        case 0x14BD: // "DIGIT"
        {
            // TODO: handle base if decimal or hex
            dx = Pop(); // base
            unsigned short ax = (Pop()&0xFF);

            if ((ax < '0') || (ax > '9'))
            {
                Push(0);
                break;
            }
            Push(ax-0x30);
            Push(1);
        }
            //printf("Digit base=%i ascii=%i\n", dx, ax);
        break;

        case 0x22AB: // ENCLOSE
        {
            unsigned short ax = Pop() & 0xFF;
            bx = Pop();
            //printf("enclose at ax:0x%04x bx:0x%04x\n", ax, bx);
            //printf("%c%c%c%c%c\n", mem[bx+0], mem[bx+1], mem[bx+2], mem[bx+3], mem[bx+4], mem[bx+5]);
            Push(bx);
            dx = 0xFFFF;
            bx--;

            // check for spaces
            x22b4:
                bx++;
                dx++;
                if ((ax&0xFF) == Read8(bx)) goto x22b4;

            Push(dx);

            if ((ax>>8) == Read8(bx))
            {
                ax = dx;
                dx++;
                Push(dx);
                Push(ax);
                break;
            }

            x22C9:
            bx++;
            dx++;
            if ((ax&0xFF) == Read8(bx)) goto x22DC;
            if (((ax>>8)&0xFF) != Read8(bx)) goto x22C9;
            ax = dx;
            Push(dx);
            Push(ax);
            break;

            x22DC:
                ax = dx;
                ax++;
                Push(dx);
                Push(ax);
            break;
        }

        case 0x1AC0: // ???
        {
            bx = Pop();
            unsigned short ax = Read8(bx);
            bx++;
            ax = ((ax + Read8(bx)) & 6) + 6;
            Push(ax);
        }
        break;

        case 0x718d: // "RECADD"
        {
            // transforms the result of "BVSA>OFFBLK" to position on disk in chunks of 1024 dependend on the record alignment
            int sp0 = Pop();
            int sp2 = Pop();
            int recordsize = Pop();
            int recordidx = Pop();

            //printf("RECADD 0x%04x 0x%04x recordsize=%i recordidx=%i\n", sp0, sp2, recordsize, recordidx);

            unsigned short ax = 1024 - sp2;
            if (recordsize == 0)
            {
              printf("Integer divide by zero\n");
              PrintCallstacktrace(bx);
              exit(1);
            }
            ax = ax / recordsize;
            if (ax > recordidx)
            {
                sp2 += recordidx * recordsize;
            } else
            {
                recordidx -= ax;
                sp0++;
                sp2 = 0;
                ax = 1024 / recordsize;
                cx = ax;
                ax = recordidx / cx;
                sp0 += ax;
                sp2 = (recordidx%cx) * recordsize;
            }
            Push(sp2);
            Push(sp0);
        }
            break;

        case 0x7295: // "BVSA>OFFBLK". Input: offset in file divided by 16, Prepare for "RECADD"
        {
            //printf("BVSA>OFFBLK 0x%04x 0x%04x 0x%04x\n", Read16(regsp), ((Read16(regsp)-0x3e80)& 0x3F)<<4, (Read16(regsp)-0x3e80)>>6);
            unsigned short ax = Pop() - 0x3e80; // BLOVSA
            Push((ax & 0x3F) << 4);
            Push(ax >> 6);
            // 0x7295: pop    ax
            // 0x7296: sub    ax,3E80
            // 0x7299: mov    cx,0006
            // 0x729c: xor    dx,dx
            // 0x729e: shr    ax,1
            // 0x72a0: rcr    dl,1
            // 0x72a2: loop   729E
            // 0x72a4: shl    dx,1
            // 0x72a6: shl    dx,1
            // 0x72a8: push   dx
            // 0x72a9: push   ax
        }
            break;

        case 0x7684: // "PRIORITIZE"
            {
                unsigned short ax = Pop();
                //printf("PRIORITIZE %i\n", ax);
                if (ax == 0)
                {
                    Push(ax);
                } else
                {
                    bx = ax - 2;
                    LXCHG16(Read16(0x54EA), bx, ax); //  LOISEG
                    LXCHG16(Read16(0x54F2), bx, ax); //  LOCSEC
                    bx >>= 1;
                    ax >>= 1;
                    LXCHG8(Read16(0x54EE), bx, ax); // HIISEG
                    Push(bx << 1);
                }
            }
            break;
// 0x7684: pop    ax
// 0x7685: or     ax,ax
// 0x7687: jz     76B0

// 0x7689: mov    bx,ax
// 0x768b: sub    bx,02
// 0x768e: push   es
// 0x768f: push   word ptr [54EA] // LOISEG
// 0x7693: pop    es
// 0x7694: call   2F36  LXCHG
// 0x7697: push   word ptr [54F2] // LOCSEC
// 0x769b: pop    es
// 0x769c: call   2F36 LXCHG
// 0x769f: shr    bx,1
// 0x76a1: shr    ax,1
// 0x76a3: push   word ptr [54EE] // HIISEG
// 0x76a7: pop    es
// 0x76a8: call   49CA
// 0x76ab: pop    es
// 0x76ac: shl    bx,1
// 0x76ae: mov    ax,bx

// 0x76b0: push   ax
// 0x76b1: lodsw
// 0x76b2: mov    bx,ax
// 0x76b4: jmp    word ptr [bx]

        case 0x143A: // ">UPPERCASE"
            cx = Pop();
            bx = Pop();
            dx = 0x617A;
            //printf("Uppercase %i at 0x%04x\n", cx, bx);
            for(i=0; i<cx; i++)
            {
                unsigned short ax = Read8(bx);
                if ((ax >= 0x61) && (ax <= 0x7A)) ax &= 0xDF;
                Write8(bx, ax);
                bx++;
            }
            //printf("\n");
            break;

        case 0x2852: // "CUR>ADDR"
        {
          // 0x270e: mov    bx,[di+1A]
          // 0x2711: mov    al,[di+1C]
          // 0x2714: add    al,bl
          // 0x2716: mov    bl,50
          // 0x2718: mul    bl
          // 0x271a: mov    bl,bh
          // 0x271c: mov    bh,[di+1D]
          // 0x271f: add    bl,bh
          // 0x2721: sub    bh,bh
          // 0x2723: add    ax,bx
          // 0x2725: ret
            bx = Read16(regdi + 0x1A);
            unsigned char bh = (bx >> 8);
            unsigned char bl = (bx & 0xFF);
            unsigned short ax = Read8(regdi + 0x1C);
            ax = (ax + bl) & 0xFF;
            bl = 0x50;
            ax *= 0x50;
            bl = bh;
            bh = Read8(regdi + 0x1D);
            bl += bh;
            bh = 0;
            ax += bl;
            Push(ax << 1);
        }
        break;

        case 0x2836: // ?POSITION
            Push(Read8(regdi + 0x1a));
            Push(Read8(regdi + 0x1b));
        break;

        case 0x0D35: // "FILL"
        {
            unsigned short ax = Pop()&0xFF;
            cx = Pop();
            int tempdi = Pop();
            for(i=0; i<cx; i++)
                Write8(tempdi + i, ax);
        }
        break;

        case 0x16D9: // INTERRUPT
            HandleInterrupt();
            break;

        case 0x2973: // SCROLLUP
        {
          // 0x2973: pop    ax
          // 0x2974: mov    ah,06
          // 0x2976: sub    cx,cx
          // 0x2978: mov    dh,[di+1E]
          // 0x297b: mov    dl,[di+1F]
          // 0x297e: mov    bx,[di+20]
          // 0x2981: push   bp
          // 0x2982: push   si
          // 0x2983: int    10
          // 0x2985: pop    si
          // 0x2986: pop    bp

            unsigned short ax = Pop();
            ax = (ax & 0xFF) | (0x0600);
            dx = Read8(regdi + 0x1e)<<8;
            dx |= Read8(regdi + 0x1f);
            bx = Read16(regdi + 0x20);
            cx = 0;
            // int 10h
        }
        break;

        case 0x11ED: // "U/MOD"
        {
          if (Read16(regsp) == 0)
          {
            printf("Integer divide by zero\n");
            PrintCallstacktrace(bx);
            exit(1);
          }
            bx = Pop();
            dx = Pop();
            unsigned short ax = Pop();
            unsigned int dividend = ax | ((unsigned int)dx<<16);
            Push(dividend % bx);
            Push(dividend / bx);
        }
        break;

        case 0xF4E: // "/"
        {
            signed short divisor = Pop();
            signed short dividend = Pop();
            if (divisor == 0)
            {
              printf("Integer divide by zero\n");
              PrintCallstacktrace(bx);
              exit(1);
            }
            Push(dividend/divisor);
        }
        break;

        case 0xF62: // "/MOD"
        {
            signed short divisor = Pop(); // bx
            signed short dividend = Pop();
            if (divisor == 0)
            {
              printf("Integer divide by zero\n");
              PrintCallstacktrace(bx);
              exit(1);
            }
            Push(dividend%divisor);
            Push(dividend/divisor);
        }
        break;

        case 0x1261: // "="
        {
            if (Pop() == Pop()) Push(1); else Push(0);
        }
        break;

        case 0x127a: // "0<"
            Push((Pop()&0x8000)?1:0);
        break;

        case 0x71DD: // "DOFFBLK" gets the idx from the dictionary in STARX.com
        {
          // 0x71dd: pop    ax
          // 0x71de: cmp    ax,0090
          // 0x71e1: js     71EF
          // 0x71e3: mov    bx,[535E] // DIRBLK
          // 0x71e7: add    bx,03
          // 0x71ea: sub    ax,0090
          // 0x71ed: jmp    7213
          // 0x71ef: cmp    ax,0060
          // 0x71f2: js     7200
          // 0x71f4: mov    bx,[535E] // DIRBLK
          // 0x71f8: add    bx,02
          // 0x71fb: sub    ax,0060
          // 0x71fe: jmp    7213
          // 0x7200: cmp    ax,0030
          // 0x7203: js     720F
          // 0x7205: mov    bx,[535E] // DIRBLK
          // 0x7209: inc    bx
          // 0x720a: sub    ax,0030
          // 0x720d: jmp    7213
          // 0x720f: mov    bx,[535E] // DIRBLK
          // 0x7213: mov    cx,0015
          // 0x7216: imul   cx
          // 0x7218: push   ax
          // 0x7219: push   bx
            unsigned short ax = Pop();
            //printf("DOFFBLK: read idx 0x%x  from STARA.COM or STARB.COM FILE\n", ax);
            if (ax >= 0x90)
            {
                bx = Read16(0x535e) + 3; // DIRBLK
                ax -= 0x90;
            } else
            if (ax >= 0x60)
            {
                bx = Read16(0x535e) + 2;
                ax -= 0x60;
            } else
            if (ax >= 0x30)
            {
                bx = Read16(0x535e) + 1;
                ax -= 0x30;
            } else
            {
                bx = Read16(0x535e) + 0;
            }
            //0x72313:
            ax *= 0x15;
            Push(ax);
            Push(bx);
        }
        break;

        case 0x3672:
            Push(Pop() + Read16(0x2c79)); // OFFSET
            break;

        case 0x4a15: // Helper for "CASE"
        {
            bx = Pop(); // pointer to case struct
            unsigned short ax = Pop(); // switch(ax)....
            //printf("case at 0x%04x: %i\n", bx, ax);
            cx = Read16(bx); // number of case entries
            bx += 2;
            dx = Read16(bx); // default word
            bx += 2;
            for(int i=0; i<cx; i++)
            {
                if (Read16(bx) == ax)
                {
                    dx = Read16(bx+2);
                    break;
                }
                bx += 4;
            }
            Push(dx);
        }
        break;

        case 0x3048: // "(BUFFER)"
        {
          // print CACHE
          //PrintCache();
          // 0x3048: pop    cx
          // 0x3049: mov    dx,es
          // 0x304b: xor    bx,bx
          // 0x304d: xor    ax,ax
          // 0x304f: push   word ptr [2C84] // PREV
          // 0x3053: pop    es

          // 0x3054: es:
          // 0x3055: cmp    [bx+06],cx
          // 0x3058: jnz    3064
          // cmp     byte ptr es:[bx+3], 0
          // jnz     short loc_3062
          // 0x3061: inc    ax
          // 0x3062: jmp    3084

          // 0x3064: push   word ptr [2CBE] // USE
          // 0x3068: pop    es
          // 0x306a: cmp    [es:bx+06],cx
          // 0x306d: jnz    3084
          //         cmp     byte ptr es:[bx+3], 0
          //         jnz     short loc_3084

          // 0x3076: inc    ax
          // 0x3077: push   word ptr [2C84] // PREV
          // 0x307b: push   es
          // 0x307c: pop    word ptr [2C84] // PREV
          // 0x3080: pop    word ptr [2CBE] // USE

          // 0x3084: or     ax,ax
          // 0x3086: jnz    308B

          // 0x3088: push   cx
          // 0x3089: jmp    3094

          // 0x308b: push   es
          // 0x308c: mov    cx,[2C84] // PREV
          // 0x3090: mov    [2C6C],cx // LPREV

          // 0x3094: push   ax
          // 0x3095: mov    es,dx

            cx = Pop();
            //printf("Buffer 0x%04x\n", cx-Read16(0x2c79));

            unsigned short ax = 0;
            unsigned short es = Read16(0x2c84); // "PREV"
            //printf("es : 0x%04x check with 0x%04x\n", es, cx);
            if (Read16Long(es, 6) == cx)
            {
                if (Read8Long(es, 3) == 0) ax++;
                //goto x3084;
            } else
            {
                // 0x3064:
                es = Read16(0x2cbe); // "USE"
                //printf("es : 0x%04x check with 0x%04x\n", es, cx);
                if (Read16Long(es, 6) == cx)
                if (Read8Long(es, 3) == 0)
                {
                    ax++;
                    Write16(0x2CBE, Read16(0x2c84));
                    Write16(0x2C84, es);
                    //printf("change 2cbe to %x\n", Read16(0x2c84));
                }
            }
            //x3084:
            if (ax == 0)
            {
                Push(cx);
            } else
            {
                Push(es);
                Write16(0x2c6c, Read16(0x2c84)); // "LPREV" = "PREV"
            }
            Push(ax);
        }
        break;

            case 0x2F51: // "LWSCAN"
            {
                // search in string for occurence of sign
                // return 1 or 0;
                unsigned short ax = Pop();
                cx = Pop();
                //printf("search for 0x%x (%i words)\n", ax, cx);
                if (cx == 0)
                {
                    regsp += 4;
                    Push(0);
                } else
                {
                    int tempdi = Pop();
                    int es = Pop();
                    //printf("es:di 0x%x:0x%x\n", es, tempdi);
                    // repne scasw
                    do {
                        unsigned short x = Read16Long(es, tempdi);
                        tempdi += 2;
                        cx--;
                        if (x == ax) break;
                    } while(cx != 0);
                    tempdi-=2;
                    if (ax == Read16Long(es, tempdi))
                    {
                        Push(tempdi);
                        Push(1);
                    } else
                    {
                        Push(0);
                    }
                }
            }
            break;

            case 0x367F: // ??? something with file content. Probably traverse directory of STARA.COM or STARB.COM
            {
                unsigned short ax;
                dx = Pop();
                //printf("search %i\n", dx);
                Push(regdi);
                Push(regsi);
                Push(regbp);
                cx = Read16(0x2B3A); // #FILES This is always 4
                regdi = 0x2D74; // OFFSETS
                bx = 0;
                //0x368C:
                for(;cx!=0; cx--)
                {
                    regbp = (cx-1)<<1;
                    regsi = 0x2D74 + regbp;
                    //printf("%i %i %i\n", cx, regbp, Read16(regsi));
                    ax = dx;
                    ax = ax - Read16(regsi);
                    if (ax&0x8000) continue;
                    regsi = 0x2D4C + regbp;
                    if (ax > Read16(regsi)) continue;
                    bx++;
                    break;
                }

                //x36AA:
                regbp >>= 1;
                cx = regbp;
                regbp = Pop();
                regsi = Pop();
                regdi = Pop();
                Push(ax);
                Push(cx);
                Push(bx);

                // 0x368c: mov    bp,cx
                // 0x368e: dec    bp
                // 0x368f: shl    bp,1
                // 0x3691: mov    si,di
                // 0x3693: add    si,bp
                // 0x3695: mov    ax,dx
                // 0x3697: sub    ax,[si]
                // 0x3699: js     36A8
                // 0x369b: mov    si,2D4C
                // 0x369e: add    si,bp
                // 0x36a0: cmp    [si],ax
                // 0x36a2: js     36A8
                // 0x36a4: inc    bx
                // 0x36a5: mov    cx,0001
                // 0x36a8: loop   368C
            }
            break;

        case 0x36BB: // ???
            if (Read16(regsp) > 3)
            {
              fprintf(stderr, "Error: %i larger than expected\n", Read16(regsp));
              exit(1);
            }
            cx = Read8(Pop() + 0x2D23); // =DRIVENUMBERS
            if (cx == 0) cx = Read16(0x2C15); // DEFAULTDRV
            cx--;
            Push(cx);
            break;

// ---------------------------------------------

        case 0x1248: // "<"
        {
            signed short int ax = Pop();
            signed short int _dx = Pop();
            //printf("input %x %x\n", ax, dx);
            if (_dx < ax) Push(1); else Push(0);
        }
        break;

        case 0x122F: // ">"
        {
            signed short int ax = Pop();
            signed short int _dx = Pop();
            if (_dx > ax) Push(1); else Push(0);
        }
        break;

        case 0x12a1: // "0>"
            if (((signed short int)Pop()) > 0) Push(1); else Push(0);
        break;
// 0x12a1: pop    ax
// 0x12a2: neg    ax
// 0x12a4: cwd
// 0x12a5: neg    dx
// 0x12a7: push   dx
// 0x12a8: lodsw
// 0x12a9: mov    bx,ax
// 0x12ab: jmp    word ptr [bx]

        case 0x12E1: // "U<"
        {
            unsigned short ax = Pop();
            dx = Pop();
            if (dx < ax) Push(1); else Push(0);
        }
        break;

        case 0x11D8: // "U*"
        {
            unsigned int ax = Pop();
            unsigned int dx = Pop();
            unsigned int x = ax * dx;
            Push(x&0xFFFF);
            Push((x>>16)&0xFFFF);
            break;
        }

        case 0x1FA: // overwrite interrupt 0 to and div 0?
            /*Push(0x20F);Push(0x7246);Push(0x1FE);Push(0x1CF);Pop();Pop();Pop();Pop(); */
        break;

        /*
        00006D12  pop cx
        00006D13  or cx,cx
        00006D15  jns 0x6d4d

        00006D17  mov bx,[0x54a1]
        00006D1B  mov dx,bx
        00006D1D  add dx,byte +0x7
        00006D20  cmp cx,dx
        00006D22  jng 0x6d4b  (jle)

        00006D24  add dx,0x401
        00006D28  cmp dx,cx
        00006D2A  jng 0x6d32  (jle)
        00006D2C  mov byte [bx+0x2],0xff
        00006D30  jmp short 0x6d4b

        00006D32  mov bx,[0x54a5]
        00006D36  mov dx,bx
        00006D38  add dx,byte +0x7
        00006D3B  cmp cx,dx
        00006D3D  jng 0x6d4b (jle)
        00006D3F  add dx,0x401
        00006D43  cmp dx,cx
        00006D45  jng 0x6d4b (jle)
        00006D47  mov byte [bx+0x2],0xff

        00006D4B  jmp short 0x6d5f

        00006D4D  cmp cx,0x63ef
        00006D51  js 0x6d5f
        00006D53  cmp cx,0x64fd
        00006D57  jns 0x6d5f
        00006D59  mov bx,0x63ee
        00006D5C  mov byte [bx],0xff

        00006D5F  push cx
        00006D60  lodsw
        00006D61  mov bx,ax
        00006D63  jmp word [bx]
        */

        case 0x6D12: // "?UPDATE" converts addr to addr
            // if addr is in a block buffer or instance buffer set the update flag
            // used in CMOVE() function, when something is copied and the overlay is merged.
            //fprintf(stderr, "?UPDATE incomplete?");
            //exit(1);
            // IBFR from 0x63ec to 0x64fc
            cx = Pop();
            //printf("?Update of 0x%04x\n", cx);
            if (cx & 0x8000)
            {
              /*
              00006D17  mov bx,[0x54a1]
              00006D1B  mov dx,bx
              00006D1D  add dx,byte +0x7

              00006D20  cmp cx,dx
              00006D22  jng 0x6d4b // jle

              00006D24  add dx,0x401
              00006D28  cmp dx,cx
              00006D2A  jng 0x6d32
              00006D2C  mov byte [bx+0x2],0xff
              00006D30  jmp short 0x6d4b

              00006D32  mov bx,[0x54a5]
              00006D36  mov dx,bx
              00006D38  add dx,byte +0x7
              00006D3B  cmp cx,dx
              00006D3D  jng 0x6d4b
              00006D3F  add dx,0x401
              00006D43  cmp dx,cx
              00006D45  jng 0x6d4b
              00006D47  mov byte [bx+0x2],0xff
              */
                // the if thens are wrong
                bx = Read16(0x54a1); // 1BUFADR = 0xf7d0
                dx = bx + 7;
                if ((signed short)cx > (signed short)dx)
                {
                  dx += 0x401;
                  if ((signed short)dx > (signed short)cx)
                  {
                      Write8(bx+2, 0xFF);
                  } else
                  {
                      bx = Read16(0x54a5); // 2BUFADR = 0xfbe0
                      dx = bx + 7;
                      if ((signed short)cx > (signed short)dx) // jle
                      {
                          dx += 0x401;
                          if ((signed short)dx > (signed short)cx)
                            Write8(bx+2, 0xFF);
                      }
                  }
                }
            } else
            {
              /*
              cmp cx,0x63ef
              js 0x6d5f
              cmp cx,0x64fd
              jns 0x6d5f
              mov bx,0x63ee
              mov byte [bx],0xff
              */
            //x6d4d:
              if ((cx >= 0x63ef) && (cx < 0x64fd)) {
                Write8(0x63ee, 0xff);
              }
            }
            //x6D5F:
            Push(cx);
        break;

        case 0x4c87: // (SLIPPER)
        {
          // 0x4c87: pop    ax
          // 0x4c88: mov    bx,ax
          // 0x4c8a: mov    cx,[4C57] // PEAK
          // 0x4c8e: sub    bx,cx
          // 0x4c90: add    bx,0080
          // 0x4c94: cmp    bh,00
          // 0x4c97: jnz    4CB1

          // 0x4c99: mov    dx,bx
          // 0x4c9b: and    dx,0007
          // 0x4c9f: mov    cl,03
          // 0x4ca1: shr    bx,cl
          // 0x4ca3: mov    cl,[bx+4C5B]
          // 0x4ca7: mov    bx,[4C4C] // FILTER
          // 0x4cab: xchg   dx,cx
          // 0x4cad: shr    bx,cl

          // 0x4caf: jmp    4CB3

          // 0x4cb1: sub    bx,bx

          // 0x4cb3: and    bx,dx

          // 0x4cb5: jz     4CB8
          // 0x4cb7: push   ax
          // 0x4cb8: push   bx

          unsigned short ax = Pop();
          bx = ax;
          cx = Read16(0x4c57); // pp_PEAK
          bx -= cx;
          bx += 0x80;
          if ((bx&0xFF00) == 0)
          {
            dx = bx & 0x7;
            bx >>= 3;
            cx = (cx & 0xFF00) | Read8(bx+0x4C5B); // CURVE
            bx = Read16(0x4c4c); // FILTER
            unsigned short temp;
            temp = dx;
            dx = cx;
            cx = temp;
            bx >>= cx&0xFF;
          } else
          {
            bx = 0;
          }

          bx = bx & dx;
          if (bx != 0)
          {
            Push(ax);
          }
          Push(bx);
          //printf("SLIPPER %i %i\n", ax, bx);
        }
        break;
// ---------------------------------------------
// timer stuff

        case 0x2a9a:  // "TIME"
        {
            //PrintCallstacktrace(bx);
            auto now = std::chrono::high_resolution_clock::now();

            // Get the time since epoch in milliseconds
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

            Write16(0x18A, (uint16_t)(millis & 0xffff)); // TIME low
            Write16(0x188, (uint16_t)((millis >> 16) & 0xffff));   // TIME high
            /*
            static int ntime = 0;
            Write16(0x18A, ntime); // TIME low
            Write16(0x188, (ntime >> 16));   // TIME high
            ntime++;
            */
            if(Read16(0x0193) != 0)
            {
                printf("We wrote 0x%04x:0x%04x - 193 - 0x%04x\n", Read16(0x018A), Read16(0x0188), Read16(0x0193));
            }
            Write16(0x193, 0x0);
            Push(0x188);
        }
        break;
// ---------------------------------------------
        // --- sound stuff ---

        case 0x2638: 
            {
                //printf("Beep\n");
                break; // BEEPON_2  
            }
            
        case 0x2653: 
            {
                //printf("Beep off\n");
                break; // "BEEPOFF"
            }
        
        case 0x2618: 
            {
                printf("Tone\n");
                Pop(); break; // "TONE"
            }

        // --- graphics ---
        case 0x97cc: // COLORMAP. Determine color from given value. For example from landscape height
        {
            bx = Pop();
            if (bx&0x8000) // is negative
            {
                bx = 0;
            } else
            {
                bx = (bx >> 1) & 0x38;
            }
            bx += 0x6a3f; // CMAP
            Write16(0x58cd, bx);
            unsigned short ax = Read8(bx);
            Write16(0x55f2, ax); // COLOR
            bx += 2;
            ax = Read8(bx);
            Write16(0x55ff, ax); // DCOLOR
        }
        break;
// 0x97cc: pop    bx
// 0x97cd: or     bx,bx
// 0x97cf: jns    97D6
// 0x97d1: mov    bx,0000
// 0x97d4: jmp    97DB

// 0x97d6: shr    bx,1
// 0x97d8: and    bx,38

// 0x97db: add    bx,6A3F // CMAP
// 0x97df: mov    [58CD],bx // TILE-PTR
// 0x97e3: xor    ax,ax
// 0x97e5: mov    al,[bx]
// 0x97e7: mov    [55F2],ax // COLOR
// 0x97eb: add    bx,02
// 0x97ee: mov    al,[bx]
// 0x97f0: mov    [55FF],ax // DCOLOR

        case 0x8E4F:  // TODO Move entire display to/from seg
            {
                /*
                // 0x8e4f: pop    bx
                // 0x8e50: pop    dx
                // 0x8e51: pop    cx
                // 0x8e52: pop    ax
                // 0x8e53: push   ds
                // 0x8e54: push   si
                // 0x8e55: push   es
                // 0x8e56: push   di
                // 0x8e57: mov    di,bx
                // 0x8e59: mov    es,dx
                // 0x8e5b: mov    si,cx
                // 0x8e5d: mov    ds,ax
                */
                auto destOffset = Pop();
                auto destSeg = Pop();
                auto srcOffset = Pop();
                auto srcSeg = Pop();
                
                printf("move display from (TODO) 0x%04x:0x%04x to 0x%04x:0x%04x\n", 
                    srcSeg, srcOffset, destSeg, destOffset);
                
                // 0x2000 bytes are always copied, source to dest.
                // I have yet to see this not align to page boundaries
                assert(srcOffset == 0);
                assert(destOffset == 0);

                for(int y = 0; y < 200; ++y)
                {
                    for(int x = 0; x < 160; ++x)
                    {
                        auto c = GraphicsPeek(x, y, srcSeg);

                        GraphicsPixel(x, y, c, destSeg);
                    }
                }
            }
        break;

        case 0x9367: // "PLOT" TODO
            // ignore return address from call
            {
            //printf("(plot) %i %i seg=0x%04x color=%i\n",
            //    Read16(regsp+2), Read16(regsp+0), Read16(0x5648), Read16(0x55F2));
            int color = Read16(0x55F2);
            GraphicsPixel(Read16(regsp+2), Read16(regsp+0), color, Read16(0x5648));
            dx = Pop();
            unsigned short ax = Pop();
            /*
            dx <<= 1;
            dx += Read16(0x563A); // YTABL
            Push(dx);
            Push(ax);
            ax = (ax&3)<<1;
            bx = 0x92CF + ax;
            dx = Read16(bx);
            ax = Pop()>>2;
            bx = Pop();
            ax += Read16(bx);
            bx = ax;
            cx = Read16(0x5648); // BUF-SEG
            unsigned short es = cx;
            ax = dx;
            */
            }
        break;

        case 0x9002: // "LPLOT" TODO
            {
                auto lplot = [](int x, int y) {
                    int offset;
                    unsigned char color_mask, pixel_data;

                    // Fetch pixel data from the buffer
                    pixel_data = GraphicsPeek(x, y, 0);

                    int color = Read16(0x55F2); // COLOR

                    /*
                    // Calculate color mask
                    color_mask = (color & 1) ? 0xF0 : 0x0F;

                    // Apply color mask to pixel data
                    pixel_data = (pixel_data & color_mask) | ((color << 4) & ~color_mask);
                    */

                    pixel_data = color & 0xf;

                    // Write pixel data back to the buffer
                    GraphicsPixel(x, y, pixel_data, 0);
                };

                int y = Pop();
                int x = Pop();

                printf("LPLOT (TODO) %i %i\n", x, y);
                lplot(x, y);
            }
        break;
        case 0x9017: // LXPLOT TODO
            {
               auto lxplot = [](int x, int y) {
                    int offset;
                    unsigned char color_mask, pixel_data;

                    // Fetch pixel data from the buffer
                    pixel_data = GraphicsPeek(x, y, 0);

                    int color = Read16(0x55F2); // COLOR

                    // Apply color mask to pixel data
                    pixel_data = pixel_data ^ (color & 0xf);

                    // Write pixel data back to the buffer
                    GraphicsPixel(x, y, pixel_data, 0);
                };

                int y = Pop();
                int x = Pop();

                printf("LXPLOT (TODO) %i %i\n", x, y);
                lxplot(x, y);
            }
        break;
        case 0x93B1: // "BEXTENT" Part of Bit Block Image Transfer (BLT)
            //printf("blt xblt=%i yblt=%i lblt=%i wblt=%i\n", Read16(0x586E), Read16(0x5863), Read16(0x5887), Read16(0x5892));
            Push(Read16(0x586E)); // xblt
            Push(Read16(0x5863) - Read16(0x5887) + 1); // yblt - lblt + 1
            Push(Read16(0x586E) + Read16(0x5892) - 1); // xblt + wblt - 1
            Push(Read16(0x5863)); // yblt
        break;

        case 0x9390: // "?EXTENTX"
           // text loc. of extent rel. to clipping window - result is in trjct & taccpt
           // IMPORTANT: USES VIN & OIN AS TEMP SPACE
           {
            int y2 = Pop();
            int x2 = Pop();
            int y1 = Pop();
            int x1 = Pop();
            /*
            printf("EXTENTX (TODO) %i %i %i %i\n", x1, y1, x2, y2);
            int color = 5;
            for(int x = x1; x <= x2; x++) {
                GraphicsPixel(x, y1, color);
                GraphicsPixel(x, y2, color);
            }
            for(int y = y1; y <= y2; y++) {
                GraphicsPixel(x1, y, color);
                GraphicsPixel(x2, y, color);
            }
            */
            //exit(1);
           }
        break;
// ================================================
// 0x938e: WORD '?EXTENTX' codep=0x9390 parp=0x9390 params=4 returns=0
// ================================================
// 0x9390: mov    bx,[569B] // VIN
// 0x9394: mov    cx,0004
// 0x9397: pop    word ptr [bx]
// 0x9399: add    bx,02
// 0x939c: loop   9397
// 0x939e: mov    bx,0002
// 0x93a1: mov    [5686],bx // #IN
// 0x93a5: push   di
// 0x93a6: call   8538
// 0x93a9: pop    di
// 0x93aa: lodsw
// 0x93ab: mov    bx,ax
// 0x93ad: jmp    word ptr [bx]

        case 0x902b: // "{BLT}" plot a bit pattern given parameters
            {
                int color = Read16(0x55F2); // COLOR
                int bltseg = Read16(0x58aa); // BLTSEG
                int bltoffs = Read16(0x589d);  // ABLT
                int x0 = Read16(0x586E);
                int y0 = Read16(0x5863);
                int w = Read16(0x5887);
                int h = Read16(0x5892);

                int bufseg = Read16(0x5648);
                int xormode = Read16(0x587C);

                printf("blt xblt=%i yblt=%i lblt=%i wblt=%i color=%i 0x%04x:0x%04x 0x%04x xor %d\n", x0, y0, w, h, color, bltseg, bltoffs, bufseg, xormode);
                GraphicsBLT(x0, y0, w, h, (char*)&m[(bltseg<<4) + bltoffs], color, xormode, bufseg);
            }
            //exit(1);
        break;

        case 0x8891: // SCANPOLY TODO
            //fprintf(stderr, "SCANPOLY TODO\n");
            {
                int VIN = Read16(0x569B);
                int nIN = Read16(0x5686);
                printf("scanpoly (TODO) 0x%04x %i\n", VIN, nIN);
                for(int i=0; i<nIN; i++)
                {
                    //printf("%i %i\n", Read16(VIN + i*4 + 0), Read16(VIN + i*4 + 2));
                }
                /*
                for(int i=44; i<114; i++)
                for(int j=192; j<199; j++)
                {
                    GraphicsPixel(i, j, 0xF);
                }
                */
            }
            //GraphicsUpdate();
            //exit(1);
        break;
        case 0x90ad: // V>DISPLAY
        {
            // TODO: This function reads and writes directly from
            // paged video RAM (e.g. 0xA200 -> 0xA000)
            uint16_t es, ds;
            uint16_t ax, cx, di, si;
            
            auto COPYLIN = [&](uint16_t count){
                /*
                uint8_t al = 0x05;
                uint8_t ah = 0x01;
                uint16_t dx = 0x03CE;

                OutPort(dx, al);
                dx++;
                uint8_t temp = ah;
                ah = al;
                al = temp;
                OutPort(dx, al);
                */

                // repz movsb
                while (count != 0) {
                    m[(es << 4) + di] = m[(ds << 4) + si];
                    si++;
                    di++;
                    count--;
                }

                /*
                al = 0x05;
                ah = 0x00;
                dx = 0x03CE;
                OutPort(dx, al);
                dx++;
                temp = ah;
                ah = al;
                al = temp;
                OutPort(dx, al);
                */
            };

            auto VDISPLAY = [&]{
                cx = 0x0078;
                di = bx;
                di = m[(ds << 4) + di];
                di++;

                while (cx != 0) {
                    COPYLIN(0x12);
                    bx += 2;
                    cx--;
                }
            };

            bx = 6598;
            es = Read16(0x55E6); // DBUF-SEG
            ds = Read16(0x55D8); // HBUF-SEG
            si = 0;

            VDISPLAY();
        }
        break;
        case 0x9aba: // !IB
        {
// ================================================
// 0x9ab2: WORD '!IB' codep=0x9aba wordp=0x9aba
// ================================================
// 0x9aba: pop    ax
// 0x9abb: pop    cx
// 0x9abc: push   es
// 0x9abd: mov    es,ax
// 0x9abf: mov    bx,[5A02] // IINDEX
// 0x9ac3: es:    
// 0x9ac4: mov    [bx],cl
// 0x9ac6: pop    es
// 0x9ac7: lodsw
// 0x9ac8: mov    bx,ax
// 0x9aca: jmp    word ptr [bx]
            uint16_t es = Pop();
            uint16_t cx = Pop();
            bx = Read16(0x5A02); // IINDEX
            m[(es << 4) + bx] = cx & 0xff;
        }
        break;
        case 0x9a9e: // !IW
        {
            // ================================================
// 0x9a96: WORD '!IW' codep=0x9a9e wordp=0x9a9e
// ================================================
// 0x9a9e: pop    ax
// 0x9a9f: pop    cx
// 0x9aa0: push   es
// 0x9aa1: mov    es,ax
// 0x9aa3: mov    bx,[5A02] // IINDEX
// 0x9aa7: shl    bx,1
// 0x9aa9: es:    
// 0x9aaa: mov    [bx],cx
// 0x9aac: pop    es
// 0x9aad: lodsw
// 0x9aae: mov    bx,ax
// 0x9ab0: jmp    word ptr [bx]
            uint16_t es = Pop();
            uint16_t cx = Pop();
            bx = Read16(0x5A02); // IINDEX
            bx <<= 1;
            *(uint16_t*)&m[(es << 4) + bx] = cx;
        }
        break;
        case 0x9d18: // ?ILOCUS
        {
            struct Icon {
                int16_t x;
                int16_t y;
                uint16_t idx;
            };

            auto ILOCUS = [&](int16_t x, int16_t y, int16_t radius, std::vector<Icon>& icons) -> std::vector<uint16_t> {
                std::vector<uint16_t> result;
                for (const auto& icon : icons) {
                    int xdist = icon.x - x;
                    if (xdist < 0) xdist = -2;
                    if (xdist <= radius) {
                        int ydist = icon.y - y;
                        if (ydist < 0) ydist = -2;
                        if (ydist <= radius) {
                            result.push_back(icon.idx);
                        }
                    }
                }
                return result;
            };

            int qty = Pop();
            uint16_t base = Read16(Pop());
            int16_t radius = (int16_t)Pop();
            int16_t y = (int16_t)Pop();
            int16_t x = (int16_t)Pop();

            uint16_t IXSEG = Read16(0x59BE);
            uint16_t IYSEG = Read16(0x59C2);

            std::vector<Icon> icons{};
            for(int i = 1; i <= qty; ++i)
            {
                Icon icon{};
                uint16_t offset = ((i - 1) + base) << 1;
                icon.x = (int16_t)Read16Long(IXSEG, offset);
                icon.y = (int16_t)Read16Long(IYSEG, offset);
                icon.idx = offset;

                icons.push_back(icon);
            }

            std::vector<uint16_t> outIdx = ILOCUS(x, y, radius, icons);

            for(auto i : outIdx)
            {
                Push(i);
            }

            Push(outIdx.size());
        }
        break;
        case 0x9e14: // XCHGICON
        {
// ================================================
// 0x9e12: WORD 'XCHGICON' codep=0x9e14 wordp=0x9e14
// ================================================
// 0x9e14: pop    ax
// 0x9e15: pop    bx
// 0x9e16: push   es
// 0x9e17: push   word ptr [59C6] // IDSEG
// 0x9e1b: pop    es
// 0x9e1c: call   49CA
// 0x9e1f: push   word ptr [59CA] // ICSEG
// 0x9e23: pop    es
// 0x9e24: call   49CA
// 0x9e27: push   word ptr [59DA] // IHSEG
// 0x9e2b: pop    es
// 0x9e2c: call   49CA
// 0x9e2f: shl    ax,1
// 0x9e31: shl    bx,1
// 0x9e33: push   word ptr [59BE] // IXSEG
// 0x9e37: pop    es
// 0x9e38: call   2F36
// 0x9e3b: push   word ptr [59C2] // IYSEG
// 0x9e3f: pop    es
// 0x9e40: call   2F36
// 0x9e43: push   word ptr [59CE] // ILSEG
// 0x9e47: pop    es
// 0x9e48: call   2F36
// 0x9e4b: pop    es
// 0x9e4c: lodsw
// 0x9e4d: mov    bx,ax
// 0x9e4f: jmp    word ptr [bx]
            uint16_t es, ds;
            uint16_t ax, cx, di, si;

            auto sub49CA = [&](){
                /*
                mov     cl, es:[bx]
                xchg    bx, ax
                xchg    cl, es:[bx]
                xchg    bx, ax
                mov     es:[bx], cl
                */
               uint8_t cl = m[(es << 4) + bx];
               std::swap(bx, ax);
               std::swap(cl, m[(es << 4) + bx]);
               std::swap(bx, ax);
               m[(es << 4) + bx] = cl;
            };

            auto sub2F36 = [&](){
                /*
                // 0x2f38: mov    cx,es:[bx]
                // 0x2f3a: xchg   ax,bx
                // 0x2f3d: xchg   es:[bx],cx
                // 0x2f3f: xchg   ax,bx
                // 0x2f42: mov    es:[bx],cx
                */
               cx = *(uint16_t*)&m[(es << 4) + bx];
               std::swap(ax, bx);
               std::swap(cx, *(uint16_t*)&m[(es << 4) + bx]);
               std::swap(ax, bx);
               *(uint16_t*)&m[(es << 4) + bx] = cx;
            };

            ax = Pop();
            bx = Pop();

            es = Read16(0x59C6);
            sub49CA();

            es = Read16(0x59CA);
            sub49CA();

            es = Read16(0x59DA);
            sub49CA();

            ax <<= 1;
            bx <<= 1;
            
            es = Read16(0x59BE);
            sub2F36();

            es = Read16(0x59C2);
            sub2F36();

            es = Read16(0x59CE);
            sub2F36();
        }
        break;
        case 0x9eb1: // ?IID
        {
// ================================================
// 0x9eaf: WORD '?IID' codep=0x9eb1 wordp=0x9eb1
// ================================================
// 0x9eb1: pop    cx
// 0x9eb2: pop    word ptr [5B04] // BICON
// 0x9eb6: pop    dx
// 0x9eb7: pop    word ptr [48C8] // ZZZ
// 0x9ebb: xor    ax,ax
// 0x9ebd: push   es
// 0x9ebe: pop    word ptr [48C6] // ZZZ
// 0x9ec2: push   word ptr [59C6] // IDSEG
// 0x9ec6: pop    es
// 0x9ec7: push   ax
// 0x9ec8: or     cx,cx
// 0x9eca: jle    9EE8
// 0x9ecc: mov    bx,cx
// 0x9ece: dec    bx
// 0x9ecf: add    bx,[5B04] // BICON
// 0x9ed3: es:    
// 0x9ed4: mov    al,[bx]
// 0x9ed6: cmp    ax,dx
// 0x9ed8: jns    9EE6
// 0x9eda: cmp    ax,[48C8] // ZZZ
// 0x9ede: jle    9EE6
// 0x9ee0: pop    ax
// 0x9ee1: push   bx
// 0x9ee2: inc    ax
// 0x9ee3: push   ax
// 0x9ee4: xor    ax,ax
// 0x9ee6: loop   9ECC
// 0x9ee8: push   word ptr [48C6] // ZZZ
// 0x9eec: pop    es
            uint16_t es, ds;
            uint16_t ax, cx, dx, di, si;

            cx = Pop();
            Write16(0x5B04, Pop()); // BICON
            dx = Pop();
            Write16(0x48C8, Pop()); // ZZZ

            ax = 0;

            // Set es to the value of IDSEG
            es = Read16(0x59C6); // IDSEG

            // Push ax onto the stack
            Push(ax);

            // If cx is less than or equal to 0, jump to 9EE8
            if (cx <= 0) {
                // Jump to 9EE8
            } else {
                // Loop until cx is 0
                do {
                    // Decrement bx
                    bx = cx - 1;

                    // Add the value of BICON to bx
                    bx += Read16(0x5B04); // BICON

                    // Read the value at the address pointed to by bx in es segment
                    ax = *(uint16_t*)&m[(es << 4) + bx];

                    // If ax is less than dx and ax is less than or equal to ZZZ, jump to 9EE6
                    if (ax >= dx && ax <= Read16(0x48C8)) { // ZZZ
                        // Jump to 9EE6
                    } else {
                        // Pop ax from the stack
                        ax = Pop();

                        // Push bx onto the stack
                        Push(bx);

                        // Increment ax
                        ax++;

                        // Push ax onto the stack
                        Push(ax);

                        // Set ax to 0
                        ax = 0;
                    }

                    // Decrement cx
                    cx--;
                } while (cx != 0);
            }
        }
        break;
        case 0x9a6c: // @IW
        {
            // 0x9a6c: pop    ax
            // 0x9a6d: push   es
            // 0x9a6e: mov    es,ax
            // 0x9a70: mov    bx,[5A02] // IINDEX
            // 0x9a74: shl    bx,1
            // 0x9a76: es:    
            // 0x9a77: mov    ax,[bx]
            // 0x9a79: pop    es
            // 0x9a7a: push   ax
            uint16_t es, ds;
            uint16_t ax, cx, dx, di, si;
            es = Pop();

            bx = Read16(0x5A02); // IINDEX
            bx <<= 1;
            
            ax = Read16Long(es, bx);
            Push(ax);
        }
        break;
        case 0x9a82: // W9A82
        {
            // 0x9a82: pop    ax
            // 0x9a83: push   es
            // 0x9a84: mov    es,ax
            // 0x9a86: mov    bx,[5A02] // IINDEX
            // 0x9a8a: xor    ax,ax
            // 0x9a8c: es:    
            // 0x9a8d: mov    al,[bx]
            // 0x9a8f: pop    es
            // 0x9a90: push   ax
            uint16_t es, ds;
            uint16_t ax, cx, dx, di, si;
            es = Pop();

            bx = Read16(0x5A02); // IINDEX
            ax = 0;
            
            ax = Read8Long(es, bx);
            Push(ax);
        }
        break;
        case 0x4910: // 2^N
        {
            // 0x4910: pop    cx
            // 0x4911: xor    ax,ax
            // 0x4913: stc    
            // 0x4914: inc    cx
            // 0x4915: jcxz   4919
            // 0x4917: rcl    ax,cl
            // 0x4919: push   ax

            uint16_t cx = Pop(); // pop cx
            uint16_t ax = 0; // xor ax, ax
            cx++; // inc cx
            if (cx != 0) {
                ax = 1 << (cx - 1); // rcl ax, cl
            }
            Push(ax); // push ax
        }
        break;
        case 0x9970: // WLD>SCR
        {
            // 0x9970: pop    ax
            // 0x9971: sub    ax,[5B31] // BVIS
            // 0x9975: mov    cx,[6221] // YWLD:YPIX
            // 0x9979: imul   cx
            // 0x997b: mov    cx,[6223] // YWLD:YPIX
            // 0x997f: idiv   cx
            // 0x9981: add    ax,[596B] // YLLDEST
            // 0x9985: mov    [48C6],ax // ZZZ
            // 0x9989: pop    ax
            // 0x998a: sub    ax,[5B3C] // LVIS
            // 0x998e: mov    cx,[6211] // XWLD:XPIX
            // 0x9992: imul   cx
            // 0x9994: mov    cx,[6213] // XWLD:XPIX
            // 0x9998: idiv   cx
            // 0x999a: add    ax,[595D] // XLLDEST
            // 0x999e: push   ax
            // 0x999f: push   word ptr [48C6] // ZZZ
            uint16_t ax, cx;
            ax = Pop();
            ax -= Read16(0x5B31); // BVIS
            cx = Read16(0x6221); // YWLD:YPIX
            ax *= cx;
            cx = Read16(0x6223); // YWLD:YPIX
            ax /= cx;
            ax += Read16(0x596B); // YLLDEST
            Write16(0x48C6, ax); // ZZZ
            ax = Pop();
            ax -= Read16(0x5B3C); // LVIS
            cx = Read16(0x6211); // XWLD:XPIX
            ax *= cx;
            cx = Read16(0x6213); // XWLD:XPIX
            ax /= cx;
            ax += Read16(0x595D); // XLLDEST
            Push(ax);
            Push(Read16(0x48C6)); // ZZZ
        }
        break;
        case 0x99b4: // SCR>BLT
        {
        // 0x99b4: pop    ax
        // 0x99b5: add    ax,0007
        // 0x99b8: sub    ax,[5A4E] // CENTERADJUST
        // 0x99bc: pop    cx
        // 0x99bd: sub    cx,[5A4E] // CENTERADJUST
        // 0x99c1: push   cx
        // 0x99c2: push   ax

            uint16_t ax, cx;
            ax = Pop();
            ax += 7;
            ax -= Read16(0x5A4E); // CENTERADJUST

            cx = Pop();
            cx -= Read16(0x5A4E); // CENTERADJUST

            Push(cx);
            Push(ax);
        }
        break;
        case 0x9055: // LFILLPOLY TODO
        {
            //printf("LFILLPOLY (TODO)\n");
            PrintCallstacktrace(bx);
            //exit(-1);
            int color = Read16(0x55F2); // COLOR

            /* 
            #define XSTART (*((uint16_t*)&memory[0x57B7]))
#define XEND (*((uint16_t*)&memory[0x57C2]))
#define YMIN (*((uint16_t*)&memory[0x57EC]))
#define YMAX (*((uint16_t*)&memory[0x57F7]))
#define SCAN (*((uint16_t*)&memory[0x57D9]))
#define YLINE (*((uint16_t*)&memory[0x57CE]))
#define IRIGHT (*((uint16_t*)&memory[0x5745]))
#define BUF_SEG (*((uint16_t*)&memory[0x5648]))
#define YTABL (*((uint16_t*)&memory[0x563A]))
            */

            {
                    std::stack<uint16_t> s;
                    bx = Read16(0x57EC); // YMIN;
                    bx += Read16(0x57F7); // YMAX;
                    bx++;
                    bx >>= 1;
                    bx <<= 1;
                    bx += Read16(0x57D9); //SCAN;
                    uint16_t ax = 0;
                    uint16_t cx = 0;
                    ax = Read16(bx);
                    bx++;
                    cx = Read16(bx);
                    if (cx >= ax) {
                        // Enabling all color planes for drawing operations on the EGA.
                        uint16_t ymax = Read16(0x57F7); // YMAX
                        ++ymax;
                        Write16(0x57F7, ymax); // YMAX++;
                        cx = Read16(0x57EC); //YMIN;
                        do {
                            s.push(cx);
                            Write16(0x57CE, cx); // YLINE
                            cx <<= 1;
                            bx = Read16(0x57D9); //SCAN;
                            bx += cx;
                            ax = 0;
                            ax = Read16(bx);
                            Write16(0x57B7, ax); // XSTART
                            bx++;
                            ax = Read16(bx);
                            Write16(0x57C2, ax); // XEND
                            /*
                            printf("LFILLPOLY Color %d YMIN %d YMAX %d, XSTART %d, XEND %d\n",
                                color,
                                Read16(0x57EC),
                                Read16(0x57F7),
                                Read16(0x57B7),
                                Read16(0x57C2)
                            );
                            */
                            //FVLIN();
                            cx = s.top(); s.pop();
                            cx++;
                        } while (cx != Read16(0x57F7)); //YMAX

                        --ymax;
                        Write16(0x57F7, ymax); // YMAX--;
                    }
            }
        }
        break;

        case 0x906b: // 'LCOPYBLK' TODO
            {
                uint16_t tuly = Pop();
                uint16_t tulx = Pop();
                uint16_t flry = Pop();
                uint16_t flrx = Pop();
                uint16_t fuly = Pop();
                uint16_t fulx = Pop();

                int width = (flrx - fulx);
                int height = (fuly - flry) + 1;

                auto bufseg = Read16(0x5648); // BUF-SEG

                // This segment may vary. Want to catch it here.
                assert(bufseg == 0xa000);

                // At one point I had a purist interpretation here. I couldn't get it work
                // and the flipped coordinate system was just confusing me and the AI.
                // It's complicated because this is a in-graphics-memory move that should
                // preserve overlapping areas. Ain't no one got time for that and I
                // just allocate a temporary vector and copy the region there and
                // back, making everyone's life easier.
                std::vector<uint32_t> pixelData;
                pixelData.resize(width * height);

                uint32_t pd = 0;
                for(int y = 0; y < height; ++y)
                {
                    for(int x = 0; x < width; ++x)
                    {
                        pixelData[pd++] = GraphicsPeekDirect(x + fulx, fuly - y, 0);
                    }
                }

                pd = 0;
                for(int y = 0; y < height; ++y)
                {
                    for(int x = 0; x < width; ++x)
                    {
                        GraphicsPixelDirect(x + tulx, tuly - y, pixelData[pd++], 0);
                    }
                }
            }
        break;

        case 0x6C86: // "C>EGA" // TODO???
        {
            dx = Pop() & 0xF;
            Push(regsi);
            unsigned short ax = 0;
            Push(0x0);
            cx = 0x10;
            // decrementing lodsb
            regsi = 0x4FCA;
            for(;cx>0; cx--)
            {
                ax = Read8(regsi) & 0xF;
                regsi--;
                if (ax == dx)
                {
                    ax = Pop();
                    ax = cx;
                    Push(ax);
                    cx = 1;
                }
            }
            ax = Pop();
            regsi = 0x4FCD;
            regsi = regsi + ax - 1;
            ax = Read8(regsi);
            regsi = Pop();
            Push(ax);
        }
// 0x6c86: pop    dx
// 0x6c87: and    dx,0F
// 0x6c8a: push   si
// 0x6c8b: xor    ax,ax

// 0x6c8d: push   ax
// 0x6c8e: mov    cx,0010
// 0x6c91: std
// 0x6c92: mov    si,4FCA

// 0x6c95: lodsb
// 0x6c96: and    ax,000F
// 0x6c99: cmp    ax,dx
// 0x6c9b: jnz    6CA4
// 0x6c9d: pop    ax
// 0x6c9e: mov    ax,cx
// 0x6ca0: push   ax
// 0x6ca1: mov    cx,0001
// 0x6ca4: loop   6C95

// 0x6ca6: pop    ax
// 0x6ca7: mov    si,4FCD
// 0x6caa: add    si,ax
// 0x6cac: dec    si
// 0x6cad: lodsb
// 0x6cae: pop    si
// 0x6caf: cld
// 0x6cb0: push   ax
        break;

        case 0x8F8B: // "BFILL" TODO
        {
            int color = Read16(0x55F2); // COLOR
            printf("bfill (TODO) color=%i ega=%i segment=0x%04x count=0x%04x\n", Read16(0x55F2), Read16(0x5DA3), Read16(0x5648), Read16(0x5656));
            GraphicsClear(color, Read16(0x5648));
        }
        break;

        case 0x903f: // "LLINE"
        {
            /*
            for(int i=0x9002; i<0x9126; i++)
            {
                if (Read8(i) == 0x9a)
                {
                    int seg = Read16(i+3);
                    int ofs = Read16(i+1);
                printf("%20s 0x%02x, 0x%04x, 0x%04x, 0x%02x : 0x%02x 0x%02x\n", FindWord(i, -1), Read8(i), seg, ofs, Read8(i+5), Read8Long(seg, ofs), Read8Long(seg, ofs+1));
                }
            }
            exit(1);
            */
            int y2 = Pop();
            int x2 = Pop();
            int y1 = Pop();
            int x1 = Pop();
            //printf("lline %i %i %i %i\n", x1, y1, x2, y2);
            GraphicsLine(x1, y1, x2, y2, Read16(0x55F2), Read16(0x587C), Read16(0x5648));
        }
        break;

        case 0x90c3: // TODO EEXTENT
        {
            ega.EEXTENT();
        }
        break;
        case 0x90f9: // TODO ARC
        {
            ega.ARC();
        }
        break;

        case 0x8D09: // "DISPLAY" wait for vertical retrace
            {
                static auto start = std::chrono::system_clock::now();
                auto now = std::chrono::system_clock::now();

                constexpr std::chrono::nanoseconds frame_duration = std::chrono::nanoseconds(16670000); // 1/60th of a second

                auto ticks = now - start;

                //auto floor_ticks = std::floor(std::chrono::duration<double, std::nano>(ticks).count() / frame_duration.count());
                auto current_time_in_vblank_fraction = std::chrono::duration<double, std::nano>(ticks).count() / frame_duration.count();
                auto next_time_in_vblanks = std::ceil(std::chrono::duration<double, std::nano>(ticks).count() / frame_duration.count());

                auto time_to_wait_in_vblanks = next_time_in_vblanks - current_time_in_vblank_fraction;
                auto time_to_wait_in_ns = std::chrono::nanoseconds(static_cast<int64_t>(time_to_wait_in_vblanks * frame_duration.count()));

                auto start_wait = std::chrono::system_clock::now();
                while (std::chrono::system_clock::now() - start_wait < time_to_wait_in_ns) {
                    std::this_thread::yield();
                }
            }
            //GraphicsUpdate();
        break;

        case 0x8D8B: // >LORES
            {
                printf("set video mode monitor=%i ?ega=%i\n", Read16(0x584A), Read16(0x5DA3));

                uint16_t ax, cx, dx, di, si;
                uint8_t& al = reinterpret_cast<uint8_t*>(&ax)[0];
                uint8_t& ah = reinterpret_cast<uint8_t*>(&ax)[1];
                uint8_t& bl = reinterpret_cast<uint8_t*>(&bx)[0];
                uint8_t& bh = reinterpret_cast<uint8_t*>(&bx)[1];

                auto Outport = [&](uint16_t port, uint8_t val) {
                    printf("Output 0x%x, 0x%x\n", port, val);
                };

                auto Sub8CEB = [&]() {
                    ax = 0x8D;
                    printf("Set video mode %d\n", al & 0xf);
                    dx = 0x03CE;
                    al = 1;
                    ah = 0x0F;
                    Outport(dx, al); // EGA: graph 1 and 2 addr reg: enable set/reset
                    dx++;
                    Outport(dx, ah); // EGA port: graphics controller data register
                };

                auto SET6845 = [&]() {
                    cx = 0x0E;
                    ah = 0;
                    for (int i = 0; i < cx; i++) {
                        dx = 0x03D4;
                        al = ah;
                        Outport(dx, al);
                        dx++;
                        si = Read8(0x8C5C + i);
                        Outport(dx, si);
                        ah++;
                        dx--;
                    }
                };

                di = Read16(0x584A); // MONITOR
                if (di == 4) {
                    // HGRAPH();
                    printf("Hercules graphics not supported.\n");
                    assert(false);
                } else {
                    ax = Read16(0x5DA3); // ?EGA
                    if ((ax & 0x00FF) != 0) {
                        Sub8CEB();
                    } else {
                        dx = 0x03D4;
                        si = 0x8C5C;
                        SET6845();
                        if (Read16(0x84EC) == 1) { // ?TANDRG
                            //Sub8D48();
                            printf("Tandy graphics not supported.\n");
                            assert(false);
                        } else {
                            bx = 0x1A27;
                            dx = 0x03D8;
                            if (di == 1) {
                                bx = 0x1E27;
                            }
                            if (di == 2) {
                                bx = 0x2A20;
                            }
                            Outport(dx, bh);
                            dx++;
                            Outport(dx, bl);
                        }
                    }
                }
            }

            // out 3ce,0x0F01 write to all four planes at once?
            GraphicsMode(1);
        break;

        case 0x8DF0: // set text mode
        {
            printf("set text mode\n");
            GraphicsMode(0);
        }
        break;

        case 0x2767: // "POSITION"
        {
            // set cursor position
            unsigned short ax = Pop();
            bx = Pop();
            Write8(regdi+0x1B, ax&0xFF);
            Write8(regdi+0x1A, bx&0xFF);
            // int10h
            //printf("position %i %i di=0x%04x\n", ax, bx, di);
        }
        break;

        // move u from parm stack to the vector stack. Used as the Overlay call stack
        case 0x7AE7: // ">V"
            //printf("Push 0x%04x\n", Read16(regsp));
            bx = Read16(0x54B4); // VSP
            Write16(bx, Pop());
            Write16(0x54B4, Read16(0x54B4)-2);
        break;

        // move u from vector stack to parm stack
        case 0x7AFE: // "V>"
            //printf("Pop\n");
            Write16(0x54B4, Read16(0x54B4)+2);
            bx = Read16(0x54B4);
            Push(Read16(bx));
        break;

        // move u from vector stack to parm stack
        case 0x7B15: // "VI"
            Push(Read16(Read16(0x54B4)+2));
        break;

        case 0x29FC: // "V!"
        {
            // Graphics card output. Prints chars on screen
            cx = ds;
            int seg = Read16(regdi + 0x18); // screen segment
            //ds = ax;
            bx = Pop(); // offset
            unsigned short ax = Pop(); // color and char
            Write16Long(seg, bx, ax);
            //ds = cx;
            //printf("PutChar 0x%04x 0x%04x at segment: 0x%04x\n", bx, ax, seg);
        }
        break;

        case 0x27F8: // Color text video memory // "">VMOVE""
        {
            unsigned short int es = Read16(regdi+0x18);
            unsigned short ax = Read16(regdi+0x20);
            cx = Pop();
            int tempdi = Pop();
            int tempsi = Pop();
            int i=0;
            //printf("Write %i bytes/words to 0x%x:0x%x\n", cx, es, tempdi);
            //0x2809
            if (es == 0xb800)
            {
                /*
                for(i=0; i<cx; i++)
                    printf("%c", Read8(tempsi+i));
                printf("\n");
                */
            }
            for(i=2; i<cx; i++)
            {
                ax = (ax & 0xFF00) | (Read8(tempsi)&0xFF);
                Write16Long(es, tempdi, ax);
                tempsi++;
                tempdi+=2;
            }
        }
        break;

        case 0xf379: // ".EGARUNBIT" from BLT-OV\n
        {
            int color = Read8(0x55F2)&0xF;
            //printf(".EGARUNBIT color=0x%04x\n", color);
            //int bright = Read8(0x535C);
            int XBLT = Read16(0x586E);
            int YBLT = Read16(0x5863);
            int WBLT = Read16(0x5892);
            int offset = Pop();
            int segment = Pop();
            int temp2 = Read16Long(segment, offset); // size of this color segment - 6

            //printf("color=%i xblt=%i yblt=%i wblt=%i 0x%04x:0x%04x 0x%04x temp2=%i\n", color, XBLT, YBLT, WBLT, segment, offset, regsp, temp2);
            int xofs = 0;
            int yofs = 0;
            for(int i=0; i<temp2; i++)
            {
                int al = Read8Long(segment, offset + 2 + i);
                if (al == 0) continue;
                for(int j=0; j<al; j++)
                {
                    if ((i&1) == 0) GraphicsPixel(XBLT+xofs, YBLT-yofs, color, 0);
                    if ((++xofs) >= WBLT)
                    {
                        xofs = 0;
                        yofs++;
                    }
                }
            }
            //GraphicsUpdate();
        }
        break;

        case 0xec7d: // "|EGA" in PORTMENU-OV
        {
            uint16_t cx = Pop();

            uint16_t ax = Read16(0x55E6); // DBUF-SEG
            uint16_t dx = Read16(0x55D8); // HBUF-SEG
            bx = Read16(0xEBFC); // WEBFC

            bx += 3;
            bx <<= 1;
            bx += Read16(0x563A); // YTABL

            uint16_t si = Read16(bx);
            uint16_t di = si;

            if (cx & 0x0001) {
                std::swap(ax, dx);
            }

            uint16_t destSeg = ax;
            uint16_t srcSeg = dx;
            cx = 0x02D0;

            // Compute x and y coordinates from si/di address in latched EGA 320x200x16 memory
            int x = si % 40;
            int y = 199 - (si / 40);

            assert(x == 0);
            for(int i = 0; i < 18; ++i)
            {
                for (int j = 0; j < 160; ++j)
                {
                    auto c = GraphicsPeek(x + j, y - i, srcSeg);
                    GraphicsPixel(x + j, y - i, c, destSeg);
                }
            }

            //printf("|EGA %i\n", cx);
            //exit(1);
        }
        break;

        // ---- 3 byte stack ---
        case 0x753F: // ">C"
            //printf("PushC at 0x%04x\n", regsi-2);
            //PrintCallstacktrace(bx);
/*
            if (((Read16(regsp)&0xFF) == 0) && (Read16(regsp+2) == 0))
            {
              printf("Error, try to push illegal stack value\n");
              PrintCallstacktrace(bx);
              PrintCStack();
              exit(1);
            }
*/
            bx = Read16(0x54b0); // CXSP
            Write8(bx+2, Pop()&0xFF);
            Write16(bx, Pop());
            Write16(0x54b0, bx-3);
        break;

        case 0x755A: // "C>"
        // 0x755a: add    word ptr [54B0],03 // CXSP
        // 0x755f: mov    bx,[54B0] // CXSP
        // 0x7563: push   word ptr [bx]
        // 0x7565: xor    ax,ax
        // 0x7567: mov    al,[bx+02]
        // 0x756a: push   ax
            //printf("PopC at 0x%04x\n", regsi-2);
            //PrintCallstacktrace(bx);
            //PrintCStack();
            if (Read16(0x54b0) >= 0x6398+3)
            {
              printf("Error: pop empty C stack\n");
              PrintCallstacktrace(bx);
              exit(1);
            }
            bx = Read16(0x54b0)+3; // CXSP
            Write16(0x54b0, bx);
            Push(Read16(bx));
            Push(Read8(bx+2));
        break;

        case 0x7577: // "CI"
            bx = Read16(0x54b0); // CXSP
            Push(Read16(bx+3));
            Push(Read8(bx+5));
        break;

        case 0x75d7: // "CDEPTH"
            Push(((0x6398 - Read16(0x54B0))&0xFF)/3);
        break;

// -------------------------------

        case 0x4997: // "1.5@"
            bx = Pop();
            Push(Read16(bx));
            Push(Read16(bx+2)&0xFF);
        break;

        case 0x49ae: // "1.5!_2"
            bx = Pop();
            Write8(bx+2, Pop()&0xFF);
            Write16(bx+0, Pop());
        break;

// -------------------------------

        case 0x763a: // "@[IOFF]"
            Push(Read16(0x558C)); // [IOFF]
        break;

        case 0x7425: // "IFLDADR"
            Push(Read8(Pop()+1) + 0x63ef);
        break;

// ---------------------------------------------

        case 0x0D10: // "CMOVE"
        {
            cx = Pop();
            int dst = Pop();
            int src = Pop();
            //printf("cmove: copy 0x%04x to 0x%04x size=%i\n", src, dst, cx);
            for(i=0; i<cx; i++)
            {
                mem[dst+i] = mem[src+i];
            }
        }
        break;

        case 0x2EFE: // "LCMOVE"
        {
            int tempcx = Pop();
            int tempdi = Pop();
            int tempes = Pop();
            int tempsi = Pop();
            int tempds = Pop();
            //printf("LCMOVE from 0x%04x:0x%04x to 0x%04x:0x%04x  %i bytes\n", tempds, tempsi, tempes, tempdi, tempcx);
            //PrintCallstacktrace(bx);
            for(i=0; i<tempcx; i++)
                Write8Long(tempes, tempdi+i, Read8Long(tempds, tempsi+i));
        }
        break;

        case 0x0D9C: //"ADDR>SEG"
            Push((Pop()>>4) + cs); // ds = cs
        break;
// --------------------------

        case 0x15BA: // "(DO)"
        {
            dx = Pop();
            unsigned short ax = Pop();
            int temp = regsp;
            regsp = regbp;
            Push(ax);
            Push(dx);
            regbp = regsp;
            regsp = temp;
            DefineCallStack(regbp, 0);
            DefineCallStack(regbp+2, 0);
        }
        break;

        case 0x1593: // "(/LOOP)"
        {
            unsigned short ax = Pop() + Read16(regbp);

            if ((signed short int)ax >= (signed short int)Read16(regbp+2))
            {
                // exit loop
                regbp += 4;
                regsi += 2;
            } else
            {
                Write16(regbp, ax);
                regsi += Read16(regsi);
            }
        }
        break;

        case 0x155E: // "(+LOOP)"
        {
            bx = Pop();  // stepsize
            unsigned short ax = Read16(regbp) + bx;
            //printf("stepsize %i, comparison index: %x dest: %x \n", (short)bx, Read16(regbp), Read16(regbp+2));
            if (bx&0x8000) // if stepsize is negative
            {
                //0x1577:
                if ((signed short)ax >= (signed short)Read16(regbp+2))
                {
                    //0x156D:
                    Write16(regbp, ax);
                    regsi += Read16(regsi);
                } else
                {
                    //0x157C: loop finished
                    // exit loop
                    regbp += 4;
                    regsi += 2;
                }

            } else
            {
                //0x1568:
                if ((signed short)ax >= (signed short)Read16(regbp+2))
                {
                    //0x157C:
                    // exit loop
                    regbp += 4; // loop finished
                    regsi += 2;
                } else
                {
                    //0x156D:
                    Write16(regbp, ax);
                    regsi += Read16(regsi);
                }
            }
        }
        break;

        case 0x15D2: // "(LOOP)"
        {
            unsigned short ax = Read16(regbp)+1; // index
            bx = Read16(regbp+2);
            if ((signed short int)ax >= (signed short int)bx)
            {
                // exit loop
                regbp += 4;
                regsi += 2;
            } else
            {
                Write16(regbp, ax);
                regsi += Read16(regsi);
            }
        }
        break;

// ------------------------------------

        case 0x1508:  // "S->D"
            if (Read16(regsp)&0x8000) Push(0xFFFF); else Push(0x0);
        break;

        case 0x1067: // "D+"
        {
            unsigned int ax = Pop();
            unsigned int dx = Pop();
            unsigned int bx = Pop();
            unsigned int cx = Pop();
            unsigned int l = (ax << 16) | dx;
            unsigned int r = (bx << 16) | cx;
            l += r;
            Push(l&0xFFFF);
            Push((l>>16)&0xFFFF);
            break;
        }

        case 0x10B9: // "DNEGATE"
        {
            unsigned int cx = Pop();
            dx = Pop();
            //printf("neg: 0x%x 0x%x\n", cx, dx);
            int x = (cx << 16) | dx;
            x = -x;
            Push(x&0xFFFF);
            Push((x >> 16) & 0xFFFF);
            break;
        }

        case 0x495E: // "D16*"
        {
            unsigned int ax = Pop();
            dx = Pop();
            unsigned int x = (ax << 16) | dx;
            x = x << 4;
            Push(x&0xFFFF);
            Push((x>>16)&0xFFFF);
            break;
        }

        case 0x4af3: // +BIT
        {
            unsigned short num = Pop(); // Pop the number from the stack
            unsigned short count = 0;
            for (int i = 0; i < 16; i++) {
                if (num & 1) {
                    count++;
                }
                num >>= 1;
            }
            Push(count); // Push the result back onto the stack
            break;
        }

        case 0x4b08: // D2*
        {
          unsigned int ax = Pop();
          unsigned short cx = Pop();

          unsigned int x = (ax<<16)|cx;
          x <<= 1;
          Push(x&0xFFFF);
          Push(x>>16);
        }
        break;

// -----------------------------------

        case 0x6f49: // "VA>BLK"
        {
            unsigned long long int ax = Pop();
            unsigned long long int dx = Pop();
            long long x = (ax << 32) | (dx << 16);
            //printf("VA>BLK %04x %04x 0x%llx %i\n", (int)ax, (int)dx, x, (int)sizeof(x));
            x = x >> 0xA;
            Push((x&0xFFFF) >> 6);
            Push((x>>16)&0xFFFF);
            break;
        }

        // ---- Special Forth interpreter functions ---

        case 0x1818: Find(); break; // "(FIND)" finds a word in the vocabulary

        // -----------------------------------

        case 0x0F22: Push(0x0); break; // 0
        case 0x0F30: Push(0x1); break; // 1
        case 0x0F3F: Push(0x2); break; // 2
        case 0x1340: Push(Pop() | Pop()); break; // OR
        case 0x12F7: Push(Pop() & Pop()); break; // AND
        case 0x1366: Push(Pop() ^ Pop()); break; // XOR
        case 0x0F74: Push(Pop() + Pop()); break; // "+"
        case 0x4bc5: Push((signed short)((char)Read8(Pop()))); break; // "+-@" sign extend
        case 0x0F94: bx = Pop(); Push(Pop() - bx); break; // "-"
        case 0x0FB5: Push(Pop() * Pop()); break; // *
        case 0x11C8: Push(-Pop()); break; // NEGATE
        case 0x1309: if (Pop() == 0) Push(1); else Push(0); break;// "NOT"
        case 0x128B: if (Pop() == 0) Push(1); else Push(0); break; // "0="
        case 0x1007: Push(Pop()*2); break; // 2*
        case 0x4984: Push(Pop()+3); break; // "3+"
        case 0x0FE9: Push(Pop()+1); break; // "1+"
        case 0x0FF8: Push(Pop()-1); break; // "1-"
        case 0x1017: Push(Pop()+2); break; // "2+"
        case 0x1027: Push(Pop()-2); break; // "2-"
        case 0x4935: Push(Pop()>>4); break; // 16/
        case 0x1037: Push(((signed short int)Pop())>>1); break; // "2/"
        case 0x4949: Push(Pop() << 4); break; // "16*"
        case 0x1355:  // "TOGGLE"
        {
            unsigned short ax = Pop();
            bx = Pop();
            Write8(bx, Read8(bx)^(ax&0xFF));
        }
        break;

        // ----- memory operations -----

        case 0x0BB0: Push(Read16(Pop())); break; // "@"
        case 0x0C94: bx = Pop(); Push(Read8(bx)); break; // "C@"
        case 0x0BE1: bx = Pop(); Write16(bx, Pop()); break; // "!", "<!>"
        case 0x0C60: bx = Pop(); Write8(bx,Pop()&0xFF); break; // "C!"

        case 0x2EB8: // "L!"
        {
            bx = Pop();
            unsigned short ds = Pop();
            unsigned short ax = Pop();
            Write16Long(ds, bx, ax);
            //printf("Write16Long to 0x%04x:0x%04x = %x\n", ds, bx, ax);
        }
        break;
        case 0x2EA4: // "L@"
        {
            bx = Pop();
            unsigned short ds = Pop();
            //printf("Read16 to 0x%x:0x%x\n", ds, bx);
            //unsigned short int x = m[(ds<<4)+bx] | (m[(ds<<4)+bx+1]<<8);
            unsigned short int x = Read16Long(ds, bx);
            Push(x);
        }
        break;
        case 0x2EE5: // "LC!"
        {
            bx = Pop();
            unsigned short ds = Pop();
            unsigned short ax = Pop();
            Write8Long(ds, bx, ax&0xFF);
            //printf("Write8Long to 0x%x:0x%x = %02x\n", ds, bx, ax&0xFF);
        }
        break;
        case 0x2eCD: // "LC@"
        {
            bx = Pop();
            unsigned short ds = Pop();
            unsigned short ax = Read8Long(ds, bx);
            Push(ax);
        }
        break;

        case 0x49f0: // 'L+-@'
        {
            bx = Pop();
            unsigned short ds = Pop();
            signed short int ax = (signed short int)((signed char)Read8Long(ds, bx));
            Push(ax);
        }
        break;

        case 0x0F85:  // "+!"
        {
            bx = Pop();
            unsigned short ax = Pop();
            Write16(bx, Read16(bx) + ax);
        }
        break;

        // ------ standard stack operations ------
        case 0x0E34: Pop(); break; // "DROP"
        case 0x0DE0: Pop(); Pop(); break; // "2DROP"
        case 0x0E81: Push(Read16(regsp+2)); break; // "OVER"
        case 0x0E43: Push(Read16(regsp)); break; // "DUP"
        case 0x0DF2: bx = regsp; Push(Read16(bx+2)); Push(Read16(bx)); break; // "2DUP"
        case 0x0DCA: if (Read16(regsp) != 0) Push(Read16(regsp)); break; //"?DUP"
        case 0x0EF4: // "SWAP"
        {
            dx = Pop();
            unsigned short ax = Pop();
            Push(dx);
            Push(ax);
        }
        break;
        case 0x0E08: // "2SWAP"
        {
            unsigned short ax = Pop();
            bx = Pop();
            cx = Pop();
            dx = Pop();
            Push(bx);
            Push(ax);
            Push(dx);
            Push(cx);
        }
        break;
        case 0x0EB5: // "ROT"
        {
            dx = Pop();
            bx = Pop();
            unsigned short ax = Pop();
            Push(bx);
            Push(dx);
            Push(ax);
        }
        break;

        // ------------------------------

        case 0x1792: Push(Read16(bx+2) + regdi); /*printf("Read 0x%04x\n", Read16(bx+2)); */break; // di is always the same value. Points to WORD "OPERATOR"
        case 0x0c17: Push(cs); break; // "(CS?)"
        case 0x0F14: Push(regsp); break; // "SP@"
        case 0x49c2: Push(ds); break; // "@DS"
        case 0x0ad1: Push(regdi); break; // "ME"
        case 0x0f05: regsp = Read16(regdi);break; // "SP!"
        case 0x4873: Write16(Pop(), 1); break; // ON
        case 0x4886: Write16(Pop(), 0); break; // "OFF"
        case 0x0D7C: Push((Pop()-cs)<<4); break; // "SEG>ADDR"
        case 0x4abb: // FRND
        {
            unsigned short ax, cx;
            ax = Read16(0x4ab0); // SEED
            cx = 0x7abd;
            ax = ((signed short)cx) * ((signed short)ax);
            ax += 0x1b0f;
            Write16(0x4ab0, ax); // SEED
            Push(ax);
        }
        break;
        case 0x4892: break; // "CAPSON" Turn on caps

        case 0x6cd6: // TODO E>CGA
        {
            //printf("TODO E>CGA\n");
            dx = Pop(); // COLOR
            dx &= 0x0F;
            uint16_t ax = 0;
            uint16_t si = 0x4FDC;
            for (cx = 0x0010; cx > 0; --cx) {
                ax = Read8(si++) & 0x000F;
                if (ax == dx) {
                    ax = cx;
                    break;
                }
            }
            si = 0x4FBB + ax - 1;
            ax = Read8(si++);
            Push(ax);
        }
        
        // 0x6cd6: pop    dx
        // 0x6cd7: and    dx,0F
        // 0x6cda: push   si
        // 0x6cdb: xor    ax,ax
        // 0x6cdd: push   ax
        // 0x6cde: mov    cx,0010
        // 0x6ce1: std
        // 0x6ce2: mov    si,4FDC
        // 0x6ce5: lodsb
        // 0x6ce6: and    ax,000F
        // 0x6ce9: cmp    ax,dx
        // 0x6ceb: jnz    6CF4
        // 0x6ced: pop    ax
        // 0x6cee: mov    ax,cx
        // 0x6cf0: push   ax
        // 0x6cf1: mov    cx,0001
        // 0x6cf4: loop   6CE5
        // 0x6cf6: pop    ax
        // 0x6cf7: mov    si,4FBB
        // 0x6cfa: add    si,ax
        // 0x6cfc: dec    si
        // 0x6cfd: lodsb
        // 0x6cfe: pop    si
        // 0x6cff: cld
        // 0x6d00: push   ax
        // 0x6d01: lodsw
        // 0x6d02: mov    bx,ax
        // 0x6d04: jmp    word ptr [bx]
        break;

        case 0x910f: // TODO L@PIXEL
            {
                uint16_t y = Pop();
                uint16_t x = Pop();
                uint8_t c = GraphicsPeek(x, y, 0);
                //printf("L@PIXEL (TODO) x=%d, y=%d c=%d", x, y, c);
                Push(c);
            }
        break;

        case 0x4b17: // TODO EASY-BITS for SQRT
        {
          // 0x4b17: pop    cx
          // 0x4b18: pop    ax
          // 0x4b19: pop    dx
          // 0x4b1a: pop    bx
          // 0x4b1b: shl    bx,1
          // 0x4b1d: rcl    dx,1
          // 0x4b1f: shl    bx,1
          // 0x4b21: rcl    dx,1
          // 0x4b23: sub    dx,ax
          // 0x4b25: jns    4B2E

          // 0x4b27: add    dx,ax
          // 0x4b29: shl    ax,1
          // 0x4b2b: dec    ax

          // 0x4b2c: jmp    4B32
          // 0x4b2e: inc    ax
          // 0x4b2f: shl    ax,1
          // 0x4b31: inc    ax

          // 0x4b32: loop   4B1B
          // 0x4b34: push   bx
          // 0x4b35: push   dx
          // 0x4b36: push   ax
          cx = Pop();
          unsigned short ax = Pop();
          dx = Pop();
          bx = Pop();
          do
          {
            unsigned int x = ((unsigned int)dx<<16) | bx;
            x <<= 2;
            dx = x >> 16;
            bx = x&0xFFFF;

            dx -= ax;
            if (dx&0x8000)
            {
              dx += ax;
              ax <<= 1;
              ax--;
            } else
            {
              ax++;
              ax <<= 1;
              ax++;
            }
            cx--;
          } while(cx > 0);

          Push(bx);
          Push(dx);
          Push(ax);

        // 0x4b1b: shl    bx,1
        // 0x4b1d: rcl    dx,1
        // 0x4b1f: shl    bx,1
        // 0x4b21: rcl    dx,1

        // 0x4b23: sub    dx,ax
        // 0x4b25: jns    4B2E
        // 0x4b27: add    dx,ax
        // 0x4b29: shl    ax,1
        // 0x4b2b: dec    ax
        // 0x4b2c: jmp    4B32
        // 0x4b2e: inc    ax
        // 0x4b2f: shl    ax,1
        // 0x4b31: inc    ax
        // 0x4b32: loop   4B1B

        // 0x4b34: push   bx
        // 0x4b35: push   dx
        // 0x4b36: push   ax
        }
        break;

        case 0xeb4a: // PRMSAV - save parm stack image at addr
        {
            bx = Pop();
            Write16(bx, regsp); // mov [bx], sp
            bx += 2; // inc bx, inc bx
            uint16_t si = Read16(0x078C); // mov si, [078C]
            si -= 0x03E8; // sub si, 03E8
            uint16_t di = bx; // mov di, bx
            uint16_t cx = 0x03E8; // mov cx, 03E8
            while (cx--) // repz
            {
                Write8(di++, Read8(si++)); // movsb
            }
        }
        break;
        case 0xeb72: // PRMLOD - load parm stack image from addr
        {
            bx = Pop(); // pop bx
            regsp = Read16(bx); // mov sp,[bx]
            bx += 2; // inc bx, inc bx
            uint16_t si = bx; // mov si, bx
            uint16_t di = Read16(0x078C); // mov di, [078C]
            di -= 0x03E8; // sub di, 03E8
            uint16_t cx = 0x03E8; // mov cx, 03E8
            while (cx--) // repz
            {
                Write8(di++, Read8(si++)); // movsb
            }
            regsp += Read16(0xDE72); // add    sp,[DE72] // WDE72
        }
        break;
        case 0xebaa: // RETSAV - save return stack image at addr
        {
            bx = Pop(); // pop bx
            Write16(bx, regbp); // mov [bx], bp
            bx += 2; // inc bx, inc bx
            uint16_t si = Read16(0x078E); // mov si, [078E]
            si -= 0x00DC; // sub si, 00DC
            uint16_t di = bx; // mov di, bx
            uint16_t cx = 0x00DC; // mov cx, 00DC
            while (cx--) // repz
            {
                Write8(di++, Read8(si++)); // movsb
            }
        }
        break;
        case 0xebd2: // RETJMP - load return stack image from addr
        {
            bx = Pop(); // pop bx
            regbp = Read16(bx); // mov bp,[bx]
            bx += 2; // inc bx, inc bx
            uint16_t si = bx; // mov si, bx
            uint16_t di = Read16(0x078E); // mov di, [078E]
            di -= 0x00DC; // sub di, 00DC
            uint16_t cx = 0x00DC; // mov cx, 00DC
            while (cx--) // repz
            {
                Write8(di++, Read8(si++)); // movsb
            }
            regbp += Read16(0xDE6E); // add bp,[DE6E]
            regsi = Read16(regbp); // mov si,[bp+00]
            regbp += 2; // inc bp, inc bp
        }
        break;        
        case 0x937b: // SETCOLOR
        {
            uint8_t c = Read8(0x55F2);
            //printf("SETCOLOR %d\n", c);
        }
        break;
        case 0x992d: // "?INVIS"
        {
          unsigned short ax = 0;
          dx = Pop();
          if (dx < Read16(0x5B31)) ax |= 4; // BVIS
          if ((short int)dx > (short int)Read16(0x5B22)) ax |= 8;
          dx = Pop();
          if (dx < Read16(0x5B3C)) ax |= 1; // LVIS
          if ((short int)dx > (short int)Read16(0x5B26)) ax |= 2;

          if (ax == 0)
          {
            ax++;
          } else
          {
            ax = 0;
          }
          Push(ax);

        // 0x992d: sub    ax,ax
        // 0x992f: pop    dx
        // 0x9930: cmp    dx,[5B31] // BVIS
        // 0x9934: jns    9939
        // 0x9936: or     ax,0004

        // 0x9939: cmp    dx,[5B22] // W5B22
        // 0x993d: jle    9942
        // 0x993f: or     ax,0008

        // 0x9942: pop    dx
        // 0x9943: cmp    dx,[5B3C] // LVIS
        // 0x9947: jns    994C
        // 0x9949: or     ax,0001

        // 0x994c: cmp    dx,[5B26] // W5B26
        // 0x9950: jle    9955
        // 0x9952: or     ax,0002

        // 0x9955: or     ax,ax
        // 0x9957: jnz    995C
        // 0x9959: inc    ax
        // 0x995a: jmp    995E
        // 0x995c: xor    ax,ax
        // 0x995e: push   ax
        }
        break;

        // ------- fract-ov operations -------
        case 0xe770: FRACT_FILLARRAY(); break;
        case 0xe537: FRACT_StoreHeight(); break;
        case 0xe75e: FRACT_FRACTALIZE(); break;

        default:
            printf("unknown opcode 0x%04x at si = 0x%04x\n", addr, regsi);
            PrintCallstacktrace(bx);
            fflush(stdout);
            return ERROR;
        break;
    }
    return OK;
}

void SaveSTARFLT()
{
    FILE *fp;
    int ret;
    fp = fopen("starflt1-in/STARPAT.COM", "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Cannot write file %s\n", FILESTAR0);
        exit(1);
    }
    ret = fwrite(&mem[0x100], FILESTAR0SIZE, 1, fp);
    fclose(fp);
}

void LoadSTARFLT(std::string hash)
{
    FILE *fp;
    int ret;
    memset(mem, 0, 0x10000);
    fp = fopen(FILESTAR0, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Cannot find file %s\n", FILESTAR0);
        exit(1);
    }
    ret = fread(&mem[0x100], FILESTAR0SIZE, 1, fp);
    fclose(fp);

    fp = fopen(FILESTARA, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", FILESTARA);
        exit(1);
    }
    ret = fread(STARA_ORIG, 256000, 1, fp);
    fclose(fp);

    fp = fopen(FILESTARB, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", FILESTARB);
        exit(1);
    }
    ret = fread(STARB_ORIG, 362496, 1, fp);
    fclose(fp);

    if(hash.length() == 0)
    {
        memcpy(STARA, STARA_ORIG, sizeof(STARA));
        memcpy(STARB, STARB_ORIG, sizeof(STARB));
    }
    else
    {
        std::string prefix = "sf-" + hash;
        std::string filename = prefix + ".zst";

        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir ("./")) != NULL) {
            while ((ent = readdir (dir)) != NULL) {
                std::string file = ent->d_name;
                if (file.substr(file.find_last_of(".") + 1) != "zst") continue;
                if (file.find(prefix) == 0) {
                    if (!Deserialize(file)) {
                        fprintf(stderr, "Error: Cannot deserialize file %s\n", file.c_str());
                        exit(1);
                    } else {
                        printf("File %s was loade for resumption.\n", file.c_str());
                        break;
                    }
                }
            }
            closedir (dir);
        } else {
            perror ("Could not open directory");
            exit(1);
        }
    }
}

void EnableDebug()
{
  debuglevel = 1;
}

enum RETURNCODE Step(PollForInputType pollForInput)
{
    unsigned short ax = Read16(regsi); // si is the forth program counter
    regsi += 2;
    unsigned short bx = ax;
    unsigned short execaddr = Read16(bx);
    //if (regsi-2 == 0xea37) printf(" enter %s\n", FindWord(bx+2, -1));

    if (debuglevel)
    {
      int ovidx = GetOverlayIndex(Read16(0x55a5));
      printf("si=0x%04x exec=0x%04x word=0x%04x sp=0x%04x ov=%2i", regsi-2, execaddr, bx+2, regsp, ovidx);
      printf(" %s\n", FindWord(bx+2, -1));
    }

    if (bx == 0xe121-2) printf("lookup 0x%04x 0x%04x 0x%04x\n", Read16(regsp), Read16(regsp+2), Read16(regsp+4));
    if (execaddr == 0xe114) printf("iaddr 0x%04x 0x%04x 0x%04x\n", Read16(regsp), Read16(regsp+2), Read16(regsp+4));

/*
    if (bx == 0x79f4-2) // >C+S
    {
      printf(">C+S 0x%02x 0x%02x\n", Read16(regsp), Read16(regsp+2));
      PrintCallstacktrace(bx);
    }
*/
    //printf("  0x%04x  %15s   %s\n", regsi, GetOverlayName(regsi, ovidx), FindWord(bx+2, ovidx));

    enum RETURNCODE ret = Call(execaddr, bx, pollForInput);
    return ret;
}

void InitEmulator(std::string hash)
{
    regbp = 0xd4a7 + 0x100 + 0x80; // call stack
    regsp = 0xd4a7 + 0x100;  // initial parameter stack
    LoadSTARFLT(hash);
    inputbuffer.clear();
}
