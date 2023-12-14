#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"call.h"
#include"../cpu/cpu.h"
#include"fract.h"
#include"graphics.h"
#include"callstack.h"
#include"findword.h"
#include"../disasOV/global.h"
#include "../util/lodepng.h"

#include "starsystem.h"
#include "instance.h"

#include <stack>
#include <assert.h>
#include <filesystem>
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

const unsigned short cs = StarflightBaseSegment;
const unsigned short ds = StarflightBaseSegment;

unsigned short int regdi = REGDI; // points to word "OPERATOR"
unsigned short int cx = 0x0;
unsigned short int dx = 0x0;

static uint16_t CurrentImageTagForHybridBlit = 0;


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
        assert(false);
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

    for(;;)
    {
        unsigned short ax = Read16(regsi); // si is the forth program counter
        regsi += 2;
        bx = ax;
        unsigned short execaddr = Read16(bx);

        auto ret = Call(execaddr, bx);

        if(ret != OK)
        {
            break;
        }
    }
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

RETURNCODE ForthCall(uint16_t word)
{
    return Call(0x224c, word - 0x2);
}

void ASMCall(uint16_t word)
{
    Call(word, word);
}

uint16_t GetInstanceClass()
{
    ForthCall(0x7520); // @INST-CLASS
    return Pop();
}

uint32_t GetInstanceOffset()
{
    ForthCall(0x750e); // @INST-OFF
    uint32_t hi_iaddr = Pop();
    uint32_t lo_iaddr = Pop();

    return hi_iaddr << 16 | lo_iaddr;
}

uint32_t ForthGetCurrent()
{
    ASMCall(0x7577); // CI
    uint32_t hi_iaddr = Pop();
    uint32_t lo_iaddr = Pop();

    return hi_iaddr << 16 | lo_iaddr;
}

void ForthPushCurrent(uint32_t iaddr)
{
    Push(iaddr & 0xffff);
    Push(iaddr >> 16);
    ASMCall(0x753f); // >C

    ForthCall(0x798c); // SET-CURRENT
}

void ForthPopCurrent()
{
    ForthCall(0x7593); // CDROP
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

void ELLIPSE_INTEGER(int x_center, int y_center, int XRadius, int YRadius, int color, int seg, bool fill)
{
    int XRadiusSq = XRadius * XRadius;
    int YRadiusSq = YRadius * YRadius;
    int twoXRadiusSq = 2 * XRadiusSq;
    int twoYRadiusSq = 2 * YRadiusSq;
    int x = XRadius;
    int y = 0;
    int xChange = YRadiusSq * (1 - 2 * XRadius);
    int yChange = XRadiusSq;
    int ellipseError = 0;
    int stoppingX = twoYRadiusSq * XRadius;
    int stoppingY = 0;

    while (stoppingX >= stoppingY)
    {
        if(fill)
        {
            for (int i = x_center - x; i <= x_center + x; i++)
            {
                GraphicsPixel(i, y_center + y, color, seg, Rotoscope(EllipsePixel));
                GraphicsPixel(i, y_center - y, color, seg, Rotoscope(EllipsePixel));
            }
        }
        else
        {
            GraphicsPixel(x_center + x, y_center + y, color, seg, Rotoscope(EllipsePixel));
            GraphicsPixel(x_center - x, y_center + y, color, seg, Rotoscope(EllipsePixel));
            GraphicsPixel(x_center + x, y_center - y, color, seg, Rotoscope(EllipsePixel));
            GraphicsPixel(x_center - x, y_center - y, color, seg, Rotoscope(EllipsePixel));
        }

        y++;
        stoppingY += twoXRadiusSq;
        ellipseError += yChange;
        yChange += twoXRadiusSq;

        if ((2 * ellipseError + xChange) > 0)
        {
            x--;
            stoppingX -= twoYRadiusSq;
            ellipseError += xChange;
            xChange += twoYRadiusSq;
        }
    }

    x = 0;
    y = YRadius;
    xChange = YRadiusSq;
    yChange = XRadiusSq * (1 - 2 * YRadius);
    ellipseError = 0;
    stoppingX = 0;
    stoppingY = twoXRadiusSq * YRadius;

    while (stoppingX <= stoppingY)
    {
        if(fill)
        {
            for (int i = x_center - x; i <= x_center + x; i++)
            {
                GraphicsPixel(i, y_center + y, color, seg, Rotoscope(EllipsePixel));
                GraphicsPixel(i, y_center - y, color, seg, Rotoscope(EllipsePixel));
            }
        }
        else
        {
            GraphicsPixel(x_center + x, y_center + y, color, seg, Rotoscope(EllipsePixel));
            GraphicsPixel(x_center - x, y_center + y, color, seg, Rotoscope(EllipsePixel));
            GraphicsPixel(x_center + x, y_center - y, color, seg, Rotoscope(EllipsePixel));
            GraphicsPixel(x_center - x, y_center - y, color, seg, Rotoscope(EllipsePixel));
        }

        x++;
        stoppingX += twoYRadiusSq;
        ellipseError += xChange;
        xChange += twoYRadiusSq;

        if ((2 * ellipseError + yChange) > 0)
        {
            y--;
            stoppingY -= twoXRadiusSq;
            ellipseError += yChange;
            yChange += twoXRadiusSq;
        }
    }
}

void DrawELLIPSE(bool fill)
{
    // .ELLIPSE ( x y radius xnumer xdenom -- \ plot a clipped ellp) 
    auto xdenom = Pop();
    auto xnumer = Pop();
    auto radius = Pop();
    auto y = (int16_t)Pop();
    auto x = (int16_t)Pop();
    auto seg = Read16(0x5648);
    auto color = Read16(0x55F2);

    //printf("DrawELLIPSE x: %d, y: %d, radius: %d, xnumer: %d, xdenom: %d color %d\n", x, y, radius, xnumer, xdenom, color);
    //fflush(stdout);

    ELLIPSE_INTEGER(x, y, (radius * xnumer) / xdenom, radius, color, seg, fill);
}

void DrawCIRCLE()
{
    Push(9);
    Push(15);
    DrawELLIPSE(false);
}


void FillCIRCLE()
{
    Push(9);
    Push(15);
    DrawELLIPSE(true);
}

void STP()
{
    // Always pass secure check
    Write16(0x5fd1, 0);
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

    enum RETURNCODE Execute(uint16_t bx, const uint16_t* conditions, uint8_t* flagCache) const
    {
        // Not sure if we must evaluate all conditions anyway as
        // a way to cache them and if that's valuable. We'll see.
        bool result = true;

        for (uint8_t condIdx = 0; condIdx < conditionCount; ++condIdx)
        {
            const WORD* curWord = nullptr;

            auto executeWord = [&](uint16_t execWord) -> enum RETURNCODE
            {
                uint16_t auxSi = regsi;

                auto word = GetWord(execWord, -1);
                auto ret = Call(word->code, word->word - 2);
                curWord = word;

                if (ret != OK) return ret;

                assert(auxSi == regsi);

                return OK;
            };

            auto& condition = conditionIndexes[condIdx];

            auto conditionWord = conditions[condition.getIndex()];
            auto* cacheValue = &flagCache[condition.getIndex()];

            uint16_t poppedValue = *cacheValue;
            if (poppedValue == 0xff)
            {
                auto ret = executeWord(conditionWord);
                if (ret != OK) return ret;

                poppedValue = Pop();

                //*cacheValue = poppedValue & 0xff;
            }

            bool actionResult = false;
            const char* modifier = "";

            if (condition.isNegated())
            {
                modifier = "NOT ";
                actionResult = poppedValue == 0;
            }
            else
            {
                actionResult = poppedValue != 0;
            }

            result = result && actionResult;

            printf("Overlay %s, Code 0x%x, word 0x%x (%d of %d) test is '%s%s' which is now %d, rule currently resolves to %d\n", GetOverlayName(curWord->ov), curWord->code, curWord->word - 2, condIdx, conditionCount, modifier, curWord->name, actionResult, result);

            if (condIdx + 1 == conditionCount)
            {
                if (result)
                {
                    auto ret = executeWord(forthWord);
                    printf("Whole rule (of %d subrules) resolves to true, executing %s:%s\n", conditionCount, GetOverlayName(curWord->ov), curWord->name);
                    if (ret != OK) return ret;
                }
                else
                {
                    printf("Whole rule resolves to false.\n");
                }
           }
        }

       return OK;
    }
};

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

    enum RETURNCODE Execute(uint16_t bx) const
    {

        auto entryStack = regsp;

        for(uint8_t ruleIdx = 0; ruleIdx < ruleCount; ++ruleIdx)
        {
            auto rule = getRuleAtIndex(ruleIdx);
            auto ret = rule->Execute(bx, getConditionArray(), getFlagCache());
            if (ret != OK) return ret;
        }

        assert(regsp == entryStack);

        return OK;
    }
};

union MusicPlayer {
    uint8_t unknown[0x10];
    struct {
        uint8_t _padding0[0x2];
        uint8_t isrEnabled;
    };
    struct {
        uint8_t _padding1[0x5];
        uint16_t currentSequenceAddressInMem;  // 0x5
        uint16_t currentNoteAddressInMem;      // 0x7
        uint8_t  repeats;                      // 0x9
        uint8_t  tickCounter;                  // 0xA
        uint8_t  noteOnDuration;               // 0xB
        uint8_t  restDuration;                 // 0xC
        uint8_t  speakerIsOff;                 // 0xD
        uint16_t freqValue; 
    };
};

static std::jthread s_musicThread{};
static std::atomic<bool> s_musicThreadShouldExit{false};
static MusicPlayer s_player;
uint8_t frequencyLookupTable[] = {
0xf0, 0xfd, 0xf8, 0x7e, 0x7c, 0x3f, 0xbe, 0x1f, 0xfd, 0x0f, 0xef, 0x07, 0xf7, 
0x03, 0xfb, 0x01, 0xb1, 0xef, 0xd8, 0x77, 0xec, 0x3b, 0xf6, 0x1d, 0xfb, 0x0e, 
0x7d, 0x07, 0xbe, 0x03, 0xdf, 0x01, 0x3d, 0xe2, 0x1e, 0x71, 0x8f, 0x38, 0x47, 
0x1c, 0x23, 0x0e, 0x11, 0x07, 0x88, 0x03, 0xc4, 0x01, 0x89, 0xd5, 0xc4, 0x6a, 
0x62, 0x35, 0xb1, 0x1a, 0x58, 0x0d, 0xac, 0x06, 0x56, 0x03, 0xab, 0x01, 0x8e, 
0xc9, 0xc7, 0x64, 0x63, 0x32, 0x31, 0x19, 0x98, 0x0c, 0x4c, 0x06, 0x26, 0x03, 
0x93, 0x01, 0x3d, 0xbe, 0x1e, 0x5f, 0x8f, 0x2f, 0xc7, 0x17, 0xe3, 0x0b, 0xf1, 
0x05, 0xf8, 0x02, 0x7c, 0x01, 0x90, 0xb3, 0xc8, 0x59, 0xe4, 0x2c, 0x72, 0x16, 
0x39, 0x0b, 0x9c, 0x05, 0xce, 0x02, 0x67, 0x01, 0x7c, 0xa9, 0xbe, 0x54, 0x5f, 
0x2a, 0x2f, 0x15, 0x97, 0x0a, 0x4b, 0x05, 0xa5, 0x02, 0x52, 0x01, 0xf8, 0x9f, 
0xfc, 0x4f, 0xfe, 0x27, 0xff, 0x13, 0xff, 0x09, 0xff, 0x04, 0x7f, 0x02, 0x3f, 
0x01, 0xff, 0x96, 0x7f, 0x4b, 0xbf, 0x25, 0xdf, 0x12, 0x6f, 0x09, 0xb7, 0x04, 
0x5b, 0x02, 0x2d, 0x01, 0x84, 0x8e, 0x42, 0x47, 0xa1, 0x23, 0xd0, 0x11, 0xe8, 
0x08, 0x74, 0x04, 0x3a, 0x02, 0x1d, 0x01, 0x85, 0x86, 0x42, 0x43, 0xa1, 0x21, 
0xd0, 0x10, 0x68, 0x08, 0x34, 0x04, 0x1a, 0x02, 0x0d, 0x01, 0xb0, 0xb6, 0xe6, 
0x43, 0x8a, 0xc2, 0xe6, 0x42, 0x8a, 0xc6, 0xe6, 0x42, 0xe4, 0x61, 0x0c, 0x03, 
0xe6, 0x61, 0xc3, 0xba, 0x61, 0x00, 0xec, 0x24, 0xfc, 0xee, 0xc3, 0x56, 0x57, 
0x1e, 0x52, 0x51, 0x53, 0x50, 0x0e, 0x1f, 0x8a, 0x06, 0x02, 0x00, 0x84, 0xc0, 
0x75, 0x03, 0xe9, 0x98, 0x00, 0x33, 0xdb, 0xfe, 0x4f, 0x0a, 0x74, 0x03, 0xe9, 
0x8e, 0x00, 0x8a, 0x47, 0x0d, 0x84, 0xc0, 0x75, 0x13, 
};

#pragma pack(pop)

extern unsigned short regbx;
uint16_t nparmsStackSi = 0;

static std::mutex s_localIconListMutex;
std::vector<Icon> s_localIconList;

std::vector<Icon> GetLocalIconList()
{
    std::lock_guard<std::mutex> lg(s_localIconListMutex);
    return s_localIconList;
}

enum RETURNCODE Call(unsigned short addr, unsigned short bx)
{
    unsigned short i;
    enum RETURNCODE ret;

    regbx = bx;

    auto divideByZero = [&](int16_t& quotient, int16_t& remainder){
        // 0x01C4:                 pop     ax
        // 0x01C5:                 inc     ax
        // 0x01C6:                 push    ax
        // 0x01C7:                 sub     ax, ax
        // 0x01C9:                 sub     dx, dx
        // 0x01CB:                 iret
        // 0x01CC: ; ---------------------------------------------------------------------------
        // 0x01CC:                 xor     bx, bx
        // 0x01CE:                 div     bx
        // 0x01D0:                 retn
        // 0x01d1: ; ---------------------------------------------------------------------------
        // 0x01d1: pop    ax
        // 0x01d2: mov    cx,ax
        // 0x01d4: sub    ax,01D0
        // 0x01d8: jnz    01E0
        // 0x01da: mov    ax,01C7
        // 0x01dd: jmp    01E4
        // 0x01e0: mov    ax,01C4
        // 0x01e3: inc    cx
        // 0x01e4: mov    dx,ds
        // 0x01e6: xor    bx,bx
        // 0x01e8: mov    ds,bx
        // 0x01ea: mov    [bx],ax
        // 0x01ec: mov    ds,dx
        // 0x01ee: push   cx
        // 0x01ef: iret
        // 0x01fa: mov    ax,ds
        // 0x01fc: xor    bx,bx
        // 0x01fe: mov    ds,bx
        // 0x0200: mov    word ptr [bx],01D1
        // 0x0204: add    bx,0002
        // 0x0208: mov    [bx],ax
        // 0x020a: mov    ds,ax
        // 0x020c: call   01CC
        // 0x020f: lodsw
        // 0x0210: mov    bx,ax
        // 0x0212: jmp    word ptr [bx]
        
        printf("Integer divide by zero\n");

        if(false)
        {
            // This code is exhibited in the divide by zero handler. Not sure of its purpose.
            auto val = Pop();
            ++val;
            Push(val);
        }

        quotient = 0;
        remainder = 0;
    };

    const char* baseOverlay = "";
    const char* baseWord = "";
    const char* overlayName = baseOverlay;
    const char* wordName = baseWord;
    uint16_t    wordValue = 0;

    int ovidx = GetOverlayIndex(Read16(0x55a5), &overlayName);
    wordName = FindWordCanFail(bx + 2, ovidx, true);
    overlayName = GetOverlayName(ovidx);
    wordValue = bx + 2;

    // bx contains pointer to WORD
    if ((regsp < FILESTAR0SIZE+0x100) || (regsp > (0xF6F4)))
    {
        printf("Error: stack pointer in invalid area: sp=0x%04x\n", regsp);
        PrintCallstacktrace(bx);
        assert(false);
        return ERROR;
    }
    switch(addr)
    {
        // --- call functions ---

        case 0x224c: // call
            {
                //        .ELLIPSE  codep:0x224c wordp:0x9632 size:0x0020 C-string:'DrawELLIPSE'
                //         .CIRCLE  codep:0x224c wordp:0x965e size:0x000a C-string:'DrawCIRCLE'
                //       FILLELLIP  codep:0x224c wordp:0x9674 size:0x004a C-string:'FILLELLIP'
                //        FILLCIRC  codep:0x224c wordp:0x96ca size:0x000a C-string:'FILLCIRC'

                uint16_t nextInstr = bx + 2;

                int16_t worldXDelta = 0;
                int16_t worldYDelta = 0;

                if(nextInstr == 0xaf81)
                {
                    //WaitForVBlank();
                }

                if (nextInstr == 0xb5aa) // HIMUS
                {
                    std::unordered_map<uint32_t, PlanetSurface> surfaces{};

                    for (const auto& p : planets)
                    {
                        Push(p.second.seed);
                        ForthCall(0xc302); // MERCATOR-GEN

                        PlanetSurface ps{};
                        ps.relief.resize(48 * 24);
                        ps.albedo.resize(48 * 24);

                        uint16_t seg = 0x7e51;

                        const uint8_t* palette = GetPlanetColorMap(p.second.species);

                        uint32_t t = 0;
                        for (int j = 23; j >= 0; j--)
                        {
                            for (int i = 0; i < 48; i++)
                            {
                                int val = 0;
                                if(p.second.species != 18)
                                {
                                    val = (int32_t)(int8_t)Read8Long(seg, j * 48 + i);
                                }
                                else
                                {
                                    val = (int32_t)(int8_t)earthmap[j * 48 + i];
                                }
                                
                                ps.relief[t] = val + 128;

                                int c = val;
                                c = c < 0 ? 0 : ((c >> 1) & 0x38);
                                c = palette[c] & 0xF;

                                uint32_t argb = colortable[c & 0xf] | 0xFF000000;
                                uint32_t abgr = ((argb & 0xFF000000)) | // Keep alpha as is
                                    ((argb & 0xFF) << 16) | // Move red to third position
                                    ((argb & 0xFF00)) | // Keep green as is
                                    ((argb & 0xFF0000) >> 16); // Move blue to rightmost

                                ps.albedo[t] = abgr;
                                ++t;
                            }
                        }

                        surfaces.emplace(p.second.seed, std::move(ps));
                    }

                    GraphicsInitPlanets(surfaces);
                }

                if (nextInstr == 0x7339) // FILE<
                {
                    uint16_t fileNum = Read16(regsp);
                    uint16_t ds = Read16(regsp + 2);

                    //if (ds == 0x0ee1)
                    {
                        CurrentImageTagForHybridBlit = fileNum;
                    }

                    printf("Read file at %04x:%04x\n", ds, fileNum);
                }

                if (nextInstr == 0xe4fa) // (?NEWHEADXY)
                {
                    worldXDelta = (int16_t)Read16(0x5dae);
                    worldYDelta = (int16_t)Read16(0x5db9);
                }

                if (nextInstr == 0x9cfc) // .LOCAL-ICONS basically the screen update function
                {
                    GraphicsReportGameFrame();

                    uint16_t localCount = Read16(0x59f5); // ILOCAL?
                    printf("localCount %d\n", localCount);

                    auto getIcon = [](uint16_t index) -> Icon {
                        uint16_t IXSEG = Read16(0x59be);
                        uint16_t IYSEG = Read16(0x59c2);
                        uint16_t IDSEG = Read16(0x59c6);
                        uint16_t ICSEG = Read16(0x59ca);
                        uint16_t ILSEG = Read16(0x59ce);
                        uint16_t IHSEG = Read16(0x59da);

                        Icon icon;
                        uint16_t IINDEX = index;
                        icon.x = (int32_t)(int16_t)Read16Long(IXSEG, IINDEX * 2);
                        icon.y = (int32_t)(int16_t)Read16Long(IYSEG, IINDEX * 2);
                        icon.id = Read8Long(IDSEG, IINDEX);
                        icon.clr = Read8Long(ICSEG, IINDEX);

                        
                        uint32_t lo_iaddr = Read16Long(ILSEG, IINDEX * 2);
                        uint32_t hi_iaddr = Read8Long(IHSEG, IINDEX);
                        icon.iaddr = (hi_iaddr << 16) | lo_iaddr;

                        return icon;
                    };

                    std::lock_guard<std::mutex> lg(s_localIconListMutex);
                    s_localIconList.clear();

                    //ASMCall(0x7577); // CI
                    //uint16_t hi_iaddr = Pop();
                    //uint16_t lo_iaddr = Pop();
                    //printf("CI is presently 0x%x\n", hi_iaddr << 16 | lo_iaddr);

                    auto currentCI = ForthGetCurrent();

                    for (uint16_t i = 0; i < localCount; ++i)
                    {
                        bool found = false;

                        uint16_t index = i;

                        Icon icon = getIcon(index);

                        Push(icon.x);
                        Push(icon.y);
                        ASMCall(0x9970); // WLD>SCR
                        icon.screenY = (int32_t)(int16_t)Pop();
                        icon.screenX = (int32_t)(int16_t)Pop();

                        Push(icon.screenX);
                        Push(icon.screenY);
                        ASMCall(0x99b4); // SCR>BLT
                        icon.bltY = 120 - (int32_t)(int16_t)Pop();
                        icon.bltX = (int32_t)(int16_t)Pop();

                        icon.screenY = 120 - icon.screenY;
                        icon.icon_type = 0; // Unknown at this point
                        icon.seed = 0; // Also unknown at this point

                        uint32_t current_iaddr = icon.iaddr;

                        ForthPushCurrent(current_iaddr);
                        auto instType = GetInstanceClass();
                        auto instOff = GetInstanceOffset();

                        if (instType == 0xb) // Unbox this box
                        {
                            current_iaddr = instOff;
                            ForthPushCurrent(current_iaddr);
                            instType = GetInstanceClass();
                            instOff = GetInstanceOffset();
                            ForthPopCurrent();
                        }

                        ForthPopCurrent();

                        auto check = ForthGetCurrent();
                        assert(check == currentCI);

                        auto systemIt = starsystem.find(current_iaddr);
                        
                        if(systemIt != starsystem.end())
                        {
                            auto ss = systemIt->second;

                            printf("Object at index %d is star system at %d x %d\n", i, ss.x, ss.y);

                            found = true;
                        }

                        auto planetIt = planets.find(current_iaddr);

                        if(planetIt != planets.end())
                        {
                            auto p = planetIt->second;

                            printf("Object at index %d is star system at %d x %d in orbit %d seed 0x%x\n", i, p.x, p.y, p.orbit, p.seed);

                            icon.seed = p.seed;

                            found = true;
                        }

                        auto it = InstanceTypes.find(instType);
                        assert(it != InstanceTypes.end());

                        if (it->second == "STAR")
                        {
                            icon.icon_type = (uint32_t)IconType::Sun;
                            found = true;
                        }
                        if (it->second == "NEBULA")
                        {
                            icon.icon_type = (uint32_t)IconType::Nebula;
                            found = true;
                        }
                        if (it->second == "PLANET")
                        {
                            icon.icon_type = (uint32_t)IconType::Planet;
                        }
                        if (it->second == "SHIP")
                        {
                            // Handled elsewhere
                            found = true;
                        }
                        if (it->second == "FLUX")
                        {
                            // Handled elsewhere
                            found = true;
                        }

                        printf("Object at index %d is %s, iaddr 0x%x found? %d\n", i, it->second.c_str(), current_iaddr, found);

                        if(!found)
                        {
                            auto inIt = instances.find(icon.iaddr);
                            if (inIt != instances.end())
                            {
                                auto inst = inIt->second;

                                auto it = InstanceTypes.find(inst.classType);
                                assert(it != InstanceTypes.end());

                                if (it->first == 0xb)
                                {
                                    // More things to unbox than just planets and stars?
                                    printf("Object at index %d is %s, iaddr 0x%x offset 0x%x\n", i, it->second.data(), icon.iaddr, inst.off);

                                    assert(false);
                                }
                            }
                            else
                            {
                                printf("Object at index %d has unknown iaddr 0x%x\n", i, icon.iaddr);
                                assert(false);
                            }
                        }

                        //printf("Locus %d of %d index %d, X: %d (%d), Y: %d (%d), ID: %u, CLR: %u, IADDR: %u\n", i, localCount, index, icon.x, icon.screenX, icon.y, icon.screenY, icon.id, icon.clr, (icon.hi_iaddr << 16) | icon.lo_iaddr);

                        s_localIconList.push_back(icon);
                    }

                    // Second pass to find the sun if we're in a solar system
                    bool inSolarSystem = false;
                    int32_t sunLocationX = 0;
                    int32_t sunLocationY = 0;
                    for (auto icon : s_localIconList)
                    {
                        if (icon.icon_type == IconType::Sun)
                        {
                            sunLocationX = icon.x;
                            sunLocationY = icon.y;

                            inSolarSystem = true;
                            break;
                        }
                    }

                    if (inSolarSystem)
                    {
                        // Place the Sun at the center and compute planets relative to the sun.
                        for (auto& icon : s_localIconList)
                        {
                            if (icon.icon_type == IconType::Planet)
                            {
                                icon.planet_to_sunX = icon.x - sunLocationX;
                                icon.planet_to_sunY = icon.y - sunLocationY;
                            }
                        }
                    }
                }

                if(nextInstr == 0xe7ec)
                {
                    // -ENDURIUM
                    // Do nothing as this prevents expending fuel
                }
                else if (nextInstr == 0xeff0)
                {
                    // Infinite money glitch
                    uint32_t balance = 1000000;
                    Push(balance & 0xffff);
                    Push(balance >> 16);
                }
                else if (nextInstr == 0xe60c)
                {
                    // CSCR>EGA
                    uint16_t ds = Read16(0x52b3);
                    uint16_t fileNum = Pop();
                    uint16_t di = 0;

                    Push(ds);
                    Push(fileNum);

                    ForthCall(0x7339); // FILE<
                    ForthCall(0x8bfb); // >HIDDEN
                    ForthCall(0x8fc1); // DARK

                    Rotoscope rs = RunBitPixel;
                    rs.blt_w = 160;
                    rs.blt_h = 200;
                    
                    // With only two splash images (?) not sure how useful
                    // it is to pass the data segment around
                    //rs.splashData.seg = ds;
                    
                    rs.runBitData.tag = fileNum;

                    GraphicsSplash(ds, fileNum);

                    for(int y = 0; y < 200; ++y)
                    {
                        rs.blt_y = 199 - y;

                        for (int x = 0; x < 80; ++x)
                        {
                            uint8_t colors = Read8Long(ds, di);

                            uint8_t firstCGA = (colors >> 4) & 0xf;
                            uint8_t secondCGA = colors & 0xf;

                            //Push(firstCGA);
                            //ASMCall(0x6C86); // C>EGA
                            //uint8_t firstColor = Pop();
                            uint8_t firstColor = CGAToEGA[firstCGA];

                            //Push(secondCGA);
                            //ASMCall(0x6C86); // C>EGA
                            //uint8_t secondColor = Pop();
                            uint8_t secondColor = CGAToEGA[secondCGA];

                            rs.blt_x = x * 2;
                            GraphicsPixel(rs.blt_x, y, firstColor, Read16(0x5648), rs);
                            rs.blt_x = x * 2 + 1;
                            GraphicsPixel(rs.blt_x, y, secondColor, Read16(0x5648), rs);

                            ++di;
                        }
                    }

                    ForthCall(0x8f15); // SCR-RES
                    ForthCall(0x8bd1); // >DISPLAY

                }
                else if (nextInstr == 0xa25d)
                {
                    s_musicThreadShouldExit = true;

                    uint16_t songOffset = 0xe580;
                    uint8_t repeats = Read8(songOffset);
                    uint16_t curNoteAddress = Read16(songOffset + 1);

                    s_player.currentSequenceAddressInMem = songOffset;
                    s_player.currentNoteAddressInMem = curNoteAddress;
                    s_player.repeats = repeats;
                    s_player.tickCounter = 1;
                    s_player.speakerIsOff = 1;
                    s_player.isrEnabled = 1;

                    s_musicThread = std::jthread([&]{
                        for (;;) {
                            if(s_musicThreadShouldExit)
                                break;

                            std::this_thread::sleep_for(std::chrono::microseconds((int)(1000000.0f / 18.2f)));

                            uint16_t noteAddress = s_player.currentNoteAddressInMem;

                            uint8_t noteDuration = Read8(noteAddress++) & 0x7F;
                            if (noteDuration != 0) {
                                uint8_t restDuration = 0;
                                if (noteDuration & 0x40) {
                                    noteDuration &= 0x3F;
                                    restDuration++;
                                }
                                s_player.noteOnDuration = noteDuration;
                                s_player.restDuration = restDuration;
                                uint8_t note = Read8(noteAddress++);
                                s_player.currentNoteAddressInMem = noteAddress;
                                if (note == 0xFF) {
                                    s_player.tickCounter = s_player.noteOnDuration;
                                    BeepOff();
                                    s_player.speakerIsOff = 1;
                                    continue;
                                }
                                uint8_t tickDuration = s_player.noteOnDuration - s_player.restDuration;
                                s_player.tickCounter = tickDuration;
                                BeepTone(frequencyLookupTable[note]);
                                BeepOn();
                                s_player.speakerIsOff = 0;
                                continue;
                            }
                            
                            uint16_t sequenceAddress = s_player.currentSequenceAddressInMem;
                            --s_player.repeats;
                            if (s_player.repeats == 0) {
                                sequenceAddress += 3;
                                uint8_t repeatCount = Read8(sequenceAddress);
                                if (repeatCount == 0) {
                                    s_player.isrEnabled = 0;
                                    break;
                                }
                                s_player.currentSequenceAddressInMem = sequenceAddress;
                                s_player.repeats = repeatCount;
                            }

                            s_player.currentNoteAddressInMem = Read16(sequenceAddress + 1);
                            s_player.tickCounter = 1;
                            s_player.speakerIsOff = 1;
                            BeepOff();
                        }

                        BeepOff();
                    });
                }
                else if (nextInstr == 0xa267)
                {
                    // Music off
                    s_musicThreadShouldExit = true;
                    if(s_musicThread.joinable())
                    {
                        s_musicThread.join();
                    }
                }
                else if (nextInstr == 0x9632)
                {
                    DrawELLIPSE(false);
                }
                else if (nextInstr == 0x965e)
                {
                    DrawCIRCLE();
                }
                else if (nextInstr == 0x9674)
                {
                    DrawELLIPSE(true); // FILL_ELLIPSE
                }
                else if (nextInstr == 0x96ca)
                {
                    FillCIRCLE();
                }
                else if (nextInstr == 0x93fa || nextInstr == 0x94d0 || nextInstr == 0x953f )
                {
                    // {1FONT} and {2FONT}
                    uint16_t character = Pop();
                    uint16_t useFont = 1;
                    if(nextInstr == 0x94d0)
                    {
                        useFont = 2;
                    }
                    else if(nextInstr == 0x953f)
                    {
                        useFont = 3;
                    }

                    printf("{%dFONT} char '%c'\n", useFont, character);

                    int color = Read16(0x55F2); // COLOR
                    int x0 = Read16(0x586E);
                    int y0 = Read16(0x5863);
                    int w = Read16(0x5887);
                    int h = Read16(0x5892);

                    int bufseg = Read16(0x5648);
                    int xormode = Read16(0x587C);

                    auto width = GraphicsFONT(useFont, character, x0, y0, color, xormode, bufseg);

                    x0 += width + 1;
                    Write16(0x586E, x0);
                }
                else if (nextInstr == 0xf4bf && (std::string(overlayName) == "STP-OV"))
                {
                    STP();
                }
                else if (nextInstr == 0xe85e && (std::string(overlayName) == "HYPER-OV"))
                {
                    // The IsIN_dash_NEB test actually checks video memory to see if the ship is actually
                    // within rendered nebula. This is why it broke for EGA because it probably
                    // doesn't test for the right color.

                    // 0xe85c: WORD '?IN-NEB' codep=0x224c wordp=0xe85e params=0 returns=1
                    Push(0);
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

                    for(;;)
                    {
                        unsigned short ax = Read16(regsi); // si is the forth program counter
                        regsi += 2;
                        bx = ax;
                        unsigned short execaddr = Read16(bx);

                        auto ret = Call(execaddr, bx);

                        if(ret != OK)
                        {
                            break;
                        }
                    }
                }

                if (nextInstr == 0xe4fa) // (?NEWHEADXY)
                {
                    Push(worldYDelta);
                    Push(worldXDelta);
                    ASMCall(0x9970); // WLD>SCR
                    int16_t oldY = (int16_t)(int16_t)Pop();
                    int16_t oldX = (int32_t)(int16_t)Pop();

                    Push(Read16(0x5db9));
                    Push(Read16(0x5dae));
                    ASMCall(0x9970); // WLD>SCR
                    int16_t newY = (int16_t)(int16_t)Pop();
                    int16_t newX = (int16_t)(int16_t)Pop();

                    GraphicsSetDeadReckoning(newX - oldX, newY - oldY);
                }
            }
        break;

        case 0xb869: // call rule
            {
            /* 
                uint16_t nextInstr = bx + 2;
                auto meta = (const RuleMetadata*)&mem[nextInstr];
                auto res = meta->Execute(nextInstr);
                if (res != OK) return res;
                Push(0); // Not sure why
                */

                ParameterCall(bx, 0xb869);
            }
        break;

        case 0x1692: // "EXIT"
            regsi = Read16(regbp);
            regbp += 2;
            return EXIT;
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
            ret = Call(Read16(bx), bx);
            if (ret != OK) return ret;
        break;

        case 0x1675: // "CFAEXEC"
            bx = Pop();
            //printf("jump to %x\n", bx);
            ret = Call(Read16(bx), bx);
            if (ret != OK) return ret;
            break;

        case 0x1684: // "EXECUTE"
            bx = Pop() - 2;
            //printf("execute %s\n", FindWord(bx+2, -1));
            ret = Call(Read16(bx), bx);
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

              std::string outStr{};

              for(int i=0; i<n; i++)
              {
                outStr += Read8(offset+i);
              }

              if(outStr == "CAUTION")
              {
                printf("\n");
              }

              printf("TYPE: %s\n", outStr.c_str());

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

            ret = Call(Read16(bx), bx);
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
            //printf("Load overlay '%s'\n", FindWord(bx+2, -1));
            ParameterCall(bx, 0x83f8);
            break;
        case 0x5275: ParameterCall(bx, 0x5275); break; // "OVT" "IARRAYS"
        case 0x4ef5: ParameterCall(bx, 0x4ef5); break; // "BLACK DK-BLUE DL-GREE GREEN RED VIOLET BROWN ... WHITE"
        case 0x6ec1: ParameterCall(bx, 0x6ec1); break; // ":TIMEST :SIGNAT :CKSUM :SAVE :VERSIO"
        case 0x3aec: ParameterCall(bx, 0x3aec); break; // D@ V= C= <OFF> <ON> <BLOCK>
        case 0x3b68:
            {
                ParameterCall(bx, 0x3b68); 
                break; // "(2C:) NULL 0. VANEWSP IROOT .... *EOL"
            }
        case 0x4a96: ParameterCall(bx, 0x4a96); break; // "CASE:"
        case 0x4a4f: // "CASE"
            {
                ParameterCall(bx, 0x4a4f); 
                break; 
            }
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x25D7: // "KEY" read keyboard endless loop, executed by "0x17B7"
        {
            uint16_t key = GraphicsGetKey();
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
            if(GraphicsHasKey())
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x1618: // read constant "2LIT"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0xC3A: // 2@
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0xC24: // 2!_2
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x30a8: // "ADVANCE>DEF"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x4D5C:  // Get segment:offset in array
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x4DA4: // "!OFFSET" sets 2D array pointers for faster access, like in C
        {
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x22AB: // ENCLOSE
        {
            Run8086(cs, addr, ds, cs, &regsp);
            break;
        }

        case 0x1AC0: // ???
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x718d: // "RECADD"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
            break;

        case 0x7295: // "BVSA>OFFBLK". Input: offset in file divided by 16, Prepare for "RECADD"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
            break;

        case 0x7684: // "PRIORITIZE"
            {
                Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x2836: // ?POSITION
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0x0D35: // "FILL"
        {
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0xF4E: // "/"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0xF62: // "/MOD"
        {
            Run8086(cs, addr, ds, cs, &regsp);        
        }
        break;

        case 0x1261: // "="
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x127a: // "0<"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0x71DD: // "DOFFBLK" gets the idx from the dictionary in STARX.com
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x3672:
            Run8086(cs, addr, ds, cs, &regsp);
            break;

        case 0x4a15: // Helper for "CASE"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x3048: // "(BUFFER)"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

            case 0x2F51: // "LWSCAN"
            {
                Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
            break;

// ---------------------------------------------

        case 0x1248: // "<"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x122F: // ">"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x12a1: // "0>"
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x11D8: // "U*"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x1FA: // overwrite interrupt 0 to and div 0?
            printf("Divide by zero interrupt.");
            //PrintCallstacktrace(bx);
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
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x4c87: // (SLIPPER)
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
// ---------------------------------------------
// timer stuff

        case 0x2a9a:  // "TIME"
        {
            //std::this_thread::sleep_for(std::chrono::milliseconds(55));

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
                BeepOn();
                break; // BEEPON_2  
            }
            
        case 0x2653: 
            {
                BeepOff();
                break; // "BEEPOFF"
            }
        
        case 0x2618: 
            {
                auto freq = Pop(); 
                BeepTone(freq);
                break; 
            }

        // --- graphics ---
        case 0x97cc: // COLORMAP. Determine color from given value. For example from landscape height
        {
            Run8086(cs, addr, ds, cs, &regsp);
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

        case 0x8E4F:  // Move display
            {
                auto destOffset = Pop();
                auto destSeg = Pop();
                auto srcOffset = Pop();
                auto srcSeg = Pop();
                
                //printf("move display from (TODO) 0x%04x:0x%04x to 0x%04x:0x%04x\n",  srcSeg, srcOffset, destSeg, destOffset);

                assert(srcOffset == 0);
                assert(destOffset == 0);

                GraphicsCopyLine(srcSeg, destSeg, srcOffset, destOffset, 0x2000);
            }
        break;

        case 0x9367: // "PLOT" 
            {
                int color = Read16(0x55F2);
                GraphicsPixel(Read16(regsp+2), Read16(regsp+0), color, Read16(0x5648), Rotoscope(PlotPixel));
                dx = Pop();
                unsigned short ax = Pop();
            }
        break;

        case 0x9002: // "LPLOT" TODO
            {
                int y = Pop();
                int x = Pop();

                int offset;
                unsigned char color_mask, pixel_data;

                int color = Read16(0x55F2); // COLOR

                // Write pixel data back to the buffer
                GraphicsPixel(x, y, color & 0xf, Read16(0x5648), Rotoscope(PlotPixel));
            }
        break;
        case 0x9017: // LXPLOT 
            {
                int y = Pop();
                int x = Pop();

                Rotoscope rs{};
                auto pixel_data = GraphicsPeek(x, y, Read16(0x5648), &rs);

                int color = Read16(0x55F2); // COLOR

                pixel_data = pixel_data ^ (color & 0xf);

                rs.EGAcolor = color;
                if(rs.content == TextPixel)
                {
                    rs.bgColor = rs.bgColor ^ (color & 0xf);
                    rs.fgColor = rs.fgColor ^ (color & 0xf);
                }

                GraphicsPixel(x, y, pixel_data, Read16(0x5648), rs);
            }
        break;
        case 0x93B1: // "BEXTENT" Part of Bit Block Image Transfer (BLT)
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0x9390: // "?EXTENTX"
           {
                Run8086(cs, addr, ds, cs, &regsp);
           }
        break;
        case 0x902b: // "{BLT}" plot a bit pattern given parameters
            {
                int color = Read16(0x55F2); // COLOR
                int bltseg = Read16(0x58aa); // BLTSEG
                int bltoffs = Read16(0x589d);  // ABLT
                int16_t x0 = (int16_t)Read16(0x586E);
                int16_t y0 = (int16_t)Read16(0x5863);
                int16_t w = (int16_t)Read16(0x5887);
                int16_t h = (int16_t)Read16(0x5892);

                int bufseg = Read16(0x5648);
                int xormode = Read16(0x587C);

                //printf("blt xblt=%i yblt=%i lblt=%i wblt=%i color=%i 0x%04x:0x%04x 0x%04x xor %d\n", x0, y0, w, h, color, bltseg, bltoffs, bufseg, xormode);
                Rotoscope rs{};
                rs.content = PicPixel;
                GraphicsBLT(x0, y0, w, h, (char*)&m[(bltseg<<4) + bltoffs], color, xormode, bufseg, rs);
            }
            //exit(1);
        break;

        case 0x8891: // SCANPOLY
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
        break;
        case 0x90ad: // V>DISPLAY
        {
            uint16_t sourceSegment = Read16(0x55D8); // HBUF-SEG
            uint16_t destinationSegment = Read16(0x55E6); // DBUF-SEG
            uint16_t sourceIndex = 0;
            uint16_t destinationIndex;
            uint16_t loopCounter = 0x0078;

            for (uint16_t bx = 0x6598; loopCounter > 0; bx += 0x02, loopCounter--) {
                destinationIndex = Read16(bx);
                destinationIndex++;

                int sourceX = sourceIndex % 40 + 1;
                int sourceY = sourceIndex / 40;
                int destinationX = destinationIndex % 40 + 3;
                int destinationY = 199 - destinationIndex / 40;

                for (int j = 0; j < 72; ++j) {
                    Rotoscope rs;
                    auto color = GraphicsPeek(sourceX + j, sourceY, sourceSegment, &rs);

                    rs.content = NavigationalPixel;
                    rs.blt_x = j;
                    rs.blt_y = loopCounter - 1;
                    rs.blt_w = 72;
                    rs.blt_h = 120;
                    GraphicsPixel(destinationX + j, destinationY, color, destinationSegment, rs);
                }

                sourceIndex += 40;
            }
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
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x9d18: // ?ILOCUS
        {
            Run8086(cs, addr, ds, cs, &regsp);
#if 0
            uint16_t locusCount = Read16(regsp);

            // Structure to represent an icon
            typedef struct {
                int16_t x;
                int16_t y;
                int16_t screenX;
                int16_t screenY;
                uint8_t id;
                uint8_t clr;
                uint16_t lo_iaddr;
                uint8_t hi_iaddr;
            } Icon;

            auto getIcon = [](uint16_t index) -> Icon {
                uint16_t IXSEG = Read16(0x59be);
                uint16_t IYSEG = Read16(0x59c2);
                uint16_t IDSEG = Read16(0x59c6);
                uint16_t ICSEG = Read16(0x59ca);
                uint16_t ILSEG = Read16(0x59ce);
                uint16_t IHSEG = Read16(0x59da);


                Icon icon;
                uint16_t IINDEX = index;
                icon.x = (int16_t)Read16Long(IXSEG, IINDEX * 2);
                icon.y = (int16_t)Read16Long(IYSEG, IINDEX * 2);
                icon.id = Read8Long(IDSEG, IINDEX);
                icon.clr = Read8Long(ICSEG, IINDEX);
                icon.lo_iaddr = Read16Long(ILSEG, IINDEX * 2);
                icon.hi_iaddr = Read8Long(IHSEG, IINDEX);
                return icon;
            };

            for (uint16_t i = 0; i < locusCount; ++i)
            {
                uint16_t index = Read16(regsp + 2 + (i * 2));

                Icon icon = getIcon(index);

                Push(icon.x);
                Push(icon.y);
                ASMCall(0x9970); // WLD>SCR
                icon.screenY = Pop();
                icon.screenX = Pop();

                printf("Locus %d of %d index %d, X: %d (%d), Y: %d (%d), ID: %u, CLR: %u, IADDR: %u\n", i, locusCount, index, icon.x, icon.screenX, icon.y, icon.screenY, icon.id, icon.clr, (icon.hi_iaddr << 16) | icon.lo_iaddr);
            }
#endif
        }
        break;
        case 0x9e14: // XCHGICON
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x9eb1: // ?IID
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x9a6c: // @IW
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x9a82: // @IH
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x4910: // 2^N
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x9970: // WLD>SCR
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x99b4: // SCR>BLT
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x9055: // LFILLPOLY 
        {
            auto YMIN = Read16(0x57EC);
            auto YMAX = Read16(0x57F7);
            auto SCAN = Read16(0x57D9);

            auto bufseg = Read16(0x5648); // BUF-SEG
            uint8_t color = Read8(0x55F2);

            //printf("LFILLPOLY YMIN %d YMAX %d SCAN 0x%x\n", YMIN, YMAX, SCAN);

            for(uint16_t y = YMIN; y <= YMAX; ++y)
            {
                // 0x0547: mov    cx,[57EC] // YMIN
                // 0x0550: shl    cx,1
                // 0x0552: mov    bx,[57D9] // SCAN
                // 0x0556: add    bx,cx
                uint16_t rasterOffset = (y << 1) + SCAN;

                auto xl = Read8(rasterOffset);
                auto xr = Read8(rasterOffset + 1);
                //printf("LFILLPOLY y %d, xl %d, xr %d color %d\n", y, xl, xr, color);

                #if 1
                for(uint16_t x = xl; x <= xr; ++x)
                {
                    GraphicsPixel(x, y, color, bufseg, Rotoscope(PolyFillPixel));
                }
                #else
                if(y == YMIN || y == YMAX)
                {
                    for(uint16_t x = xl; x <= xr; ++x)
                    {
                        GraphicsPixel(x, y, color, bufseg);
                    }
                }
                else
                {
                    GraphicsPixel(xl, y, color, bufseg);
                    GraphicsPixel(xr, y, color, bufseg);
                }
                #endif
            }
        }
        break;

        case 0x906b: // 'LCOPYBLK'
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

                // At one point I had a purist interpretation here. I couldn't get it work
                // and the flipped coordinate system was just confusing me and the AI.
                // It's complicated because this is a in-graphics-memory move that should
                // preserve overlapping areas. Ain't no one got time for that and I
                // just allocate a temporary vector and copy the region there and
                // back, making everyone's life easier.
                std::vector<uint32_t> pixelData;
                pixelData.resize(width * height);

                std::vector<Rotoscope> rotoData;
                rotoData.resize(width * height);

                uint32_t pd = 0;
                for(int y = 0; y < height; ++y)
                {
                    for(int x = 0; x < width; ++x)
                    {
                        pixelData[pd] = GraphicsPeekDirect(x + fulx, fuly - y, bufseg, &rotoData[pd]);

                        ++pd;
                    }
                }

                pd = 0;
                for(int y = 0; y < height; ++y)
                {
                    for(int x = 0; x < width; ++x)
                    {
                        GraphicsPixelDirect(x + tulx, tuly - y, pixelData[pd], bufseg, rotoData[pd]);
                        ++pd;
                    }
                }
            }
        break;

        case 0x6C86: // "C>EGA" 
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x8F8B: // "BFILL"
        {
            int color = Read16(0x55F2); // COLOR
            //printf("bfill (TODO) color=%i ega=%i segment=0x%04x count=0x%04x\n", Read16(0x55F2), Read16(0x5DA3), Read16(0x5648), Read16(0x5656));
            GraphicsClear(color, Read16(0x5648), Read16(0x5656));
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
            // Should be replaced by actual circle and ellipse functions
            assert(false);
        }
        break;
        case 0x90f9: // TODO ARC
        {
            // Should be replaced by actual circle and ellipse functions
            assert(false);
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
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        // move u from vector stack to parm stack
        case 0x7AFE: // "V>"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        // move u from vector stack to parm stack
        case 0x7B15: // "VI"
            Run8086(cs, addr, ds, cs, &regsp);
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
            uint32_t bufseg = Read16(0x5648);

            Rotoscope rc{};
            rc.content = RunBitPixel;
            rc.runBitData.tag = CurrentImageTagForHybridBlit;
            rc.blt_w = WBLT;

            int xofs = 0;
            int verticalLines = 0;
            for(int i=0; i<temp2; i++)
            {
                int al = Read8Long(segment, offset + 2 + i);
                if (al == 0) continue;
                for(int j=0; j<al; j++)
                {
                    if ((++xofs) >= WBLT)
                    {
                        xofs = 0;
                        verticalLines++;
                    }
                }
            }

            rc.blt_h = verticalLines;

            static std::unordered_map<uint16_t, std::vector<uint32_t>> pixImages;

            auto it = pixImages.find(CurrentImageTagForHybridBlit);
            if(it == pixImages.end())
            {
                std::vector<uint32_t> pixImage;
                pixImage.resize(verticalLines * WBLT);
                auto result = pixImages.emplace(CurrentImageTagForHybridBlit, std::move(pixImage));
                it = result.first;
            }

            uint32_t argb = colortable[color & 0xf] | 0xFF000000;
            uint32_t abgr = ((argb & 0xFF000000)) | // Keep alpha as is
                            ((argb & 0xFF) << 16) | // Move red to third position
                            ((argb & 0xFF00)) | // Keep green as is
                            ((argb & 0xFF0000) >> 16); // Move blue to rightmost

            //printf("color=%i xblt=%i yblt=%i wblt=%i 0x%04x:0x%04x 0x%04x temp2=%i\n", color, XBLT, YBLT, WBLT, segment, offset, regsp, temp2);
            xofs = 0;
            int yofs = 0;
            for(int i=0; i<temp2; i++)
            {
                int al = Read8Long(segment, offset + 2 + i);
                if (al == 0) continue;
                for(int j=0; j<al; j++)
                {
                    rc.blt_x = xofs;
                    rc.blt_y = yofs;

                    Rotoscope inRs{};
                    GraphicsPeek(XBLT + xofs, YBLT - yofs, bufseg, &inRs);
                    if ((i&1) == 0)
                    {
                        rc.bgColor = inRs.EGAcolor;
                        rc.fgColor = color;
                        GraphicsPixel(XBLT+xofs, YBLT-yofs, color, bufseg, rc);

                        it->second[yofs * WBLT + xofs] = abgr;
                    }
                    
                    if ((++xofs) >= WBLT)
                    {
                        xofs = 0;
                        yofs++;
                    }
                }
            }

            // Clear any lines not already rotoscoped a RunBitPxiel with this seg/offset
            for (int y = 0; y < verticalLines; ++y)
            {
                for (int x = 0; x < WBLT; ++x)
                {
                    int xat = x + XBLT;
                    int yat = YBLT - y;

                    Rotoscope inRs{};
                    GraphicsPeek(xat, yat, bufseg, &inRs);
                    if (inRs.content != RunBitPixel)
                    {
                        rc.blt_x = x;
                        rc.blt_y = y;
                        rc.EGAcolor = 0;
                        rc.argb = 0;
                        rc.bgColor = rc.EGAcolor;
                        rc.fgColor = color;
                        GraphicsPixel(xat, yat, 0, bufseg, rc);
                        it->second[y * WBLT + x] = 0xff000000;
                    }
                }
            }

            // Store the ARGB image in pixImage into a png file
            std::string filename = "pix/pix_" + std::to_string(rc.runBitData.tag) + ".png";
            unsigned error = lodepng::encode(filename, (const unsigned char*)it->second.data(), WBLT, verticalLines, LCT_RGBA);
            if(error) printf("encoder error %u: %s\n", error, lodepng_error_text(error));
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

            uint16_t destSeg = dx;
            uint16_t srcSeg = ax;
            cx = 0x02D0;

            // Compute x and y coordinates from si/di address in latched EGA 320x200x16 memory
            int x = si % 40;
            int y = 199 - (si / 40);

            assert(x == 0);
            for(int i = 0; i < 18; ++i)
            {
                for (int j = 0; j < 160; ++j)
                {
                    Rotoscope rs{};
                    auto c = GraphicsPeek(x + j, y - i, srcSeg, &rs);
                    GraphicsPixel(x + j, y - i, c, destSeg, rs);
                }
            }
        }
        break;

        // ---- 3 byte stack ---
        case 0x753F: // ">C"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0x755A: // "C>"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0x7577: // "CI"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0x75d7: // "CDEPTH"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

// -------------------------------

        case 0x4997: // "1.5@"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0x49ae: // "1.5!_2"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

// -------------------------------

        case 0x763a: // "@[IOFF]"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

        case 0x7425: // "IFLDADR"
            Run8086(cs, addr, ds, cs, &regsp);
        break;

// ---------------------------------------------

        case 0x0D10: // "CMOVE"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x2EFE: // "LCMOVE"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x0D9C: //"ADDR>SEG"
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x155E: // "(+LOOP)"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x15D2: // "(LOOP)"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

// ------------------------------------

        case 0x1508:  // "S->D"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x1067: // "D+"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x10B9: // "DNEGATE"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x495E: // "D16*"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x4af3: // +BIT
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x4b08: // D2*
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

// -----------------------------------

        case 0x6f49: // "VA>BLK"
        {
            Run8086(cs, addr, ds, cs, &regsp);
            break;
        }

        // ---- Special Forth interpreter functions ---

        case 0x1818: Find(); break; // "(FIND)" finds a word in the vocabulary

        // -----------------------------------

        case 0x0F22: Run8086(cs, addr, ds, cs, &regsp); break; // 0
        case 0x0F30: Run8086(cs, addr, ds, cs, &regsp); break; // 1
        case 0x0F3F: Run8086(cs, addr, ds, cs, &regsp); break; // 2
        case 0x1340: Run8086(cs, addr, ds, cs, &regsp); break; // OR
        case 0x12F7: Run8086(cs, addr, ds, cs, &regsp); break; // AND
        case 0x1366: Run8086(cs, addr, ds, cs, &regsp); break; // XOR
        case 0x0F74: Run8086(cs, addr, ds, cs, &regsp); break; // "+"
        case 0x4bc5: Run8086(cs, addr, ds, cs, &regsp); break; // "+-@" sign extend
        case 0x0F94: Run8086(cs, addr, ds, cs, &regsp); break; // "-"
        case 0x0FB5: Run8086(cs, addr, ds, cs, &regsp); break; // *
        case 0x11C8: Run8086(cs, addr, ds, cs, &regsp); break; // NEGATE
        case 0x1309: Run8086(cs, addr, ds, cs, &regsp); break;// "NOT"
        case 0x128B: Run8086(cs, addr, ds, cs, &regsp); break; // "0="
        case 0x1007: Run8086(cs, addr, ds, cs, &regsp); break; // 2*
        case 0x4984: Run8086(cs, addr, ds, cs, &regsp); break; // "3+"
        case 0x0FE9: Run8086(cs, addr, ds, cs, &regsp); break; // "1+"
        case 0x0FF8: Run8086(cs, addr, ds, cs, &regsp); break; // "1-"
        case 0x1017: Run8086(cs, addr, ds, cs, &regsp); break; // "2+"
        case 0x1027: Run8086(cs, addr, ds, cs, &regsp); break; // "2-"
        case 0x4935: Run8086(cs, addr, ds, cs, &regsp); break; // 16/
        case 0x1037: Run8086(cs, addr, ds, cs, &regsp); break; // "2/"
        case 0x4949: Run8086(cs, addr, ds, cs, &regsp); break; // "16*"
        case 0x1355:  // "TOGGLE"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        // ----- memory operations -----

        case 0x0BB0: Run8086(cs, addr, ds, cs, &regsp); break; // "@"
        case 0x0C94: Run8086(cs, addr, ds, cs, &regsp); break; // "C@"
        case 0x0BE1: Run8086(cs, addr, ds, cs, &regsp); break; // "!", "<!>"
        case 0x0C60: Run8086(cs, addr, ds, cs, &regsp); break; // "C!"

        case 0x2EB8: // "L!"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x2EA4: // "L@"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x2EE5: // "LC!"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x2eCD: // "LC@"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x49f0: // 'L+-@'
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x0F85:  // "+!"
        {
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x0E08: // "2SWAP"
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x0EB5: // "ROT"
        {
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;
        case 0x4892: break; // "CAPSON" Turn on caps

        case 0x6cd6: // E>CGA
        {
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        case 0x910f: // L@PIXEL
            {
                uint16_t y = Pop();
                uint16_t x = Pop();
                uint8_t c = GraphicsPeek(x, y, Read16(0x5648));
                Push(c);
            }
        break;

        case 0x4b17: // EASY-BITS for SQRT
        {
            Run8086(cs, addr, ds, cs, &regsp);
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
            Run8086(cs, addr, ds, cs, &regsp);
        }
        break;

        // ------- fract-ov operations -------
        case 0xe770: FRACT_FILLARRAY(); break;
        case 0xe537: FRACT_StoreHeight(); break;
        case 0xe75e: FRACT_FRACTALIZE(); break;
        case 0xead1:
            {
                // 0xead1: mov    [EACD],sp // WEACD
                Write16(0xEACD, regsp);
            }
            break;
        case 0xeadc: // WEADC
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0xeaea: // WEAEA
            {
                Run8086(cs, addr, ds, cs, &regsp);            
            }
            break;
        case 0xeaf8: // WEAF8
            {
                Run8086(cs, addr, ds, cs, &regsp);            
            }
            break; // +TMP
        case 0xed34:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0xed44: // -TMP
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0xed50: // @TMP
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
// ================================================
// 0xed60: WORD '!TMP' codep=0xed62 wordp=0xed62
// ================================================
// 0xed62: pop    bx
// 0xed63: shl    bx,1
// 0xed65: neg    bx
// 0xed67: add    bx,[ED30] // [TMP]
// 0xed6b: pop    ax
// 0xed6c: mov    [bx],ax
// 0xed6e: lodsw
// 0xed6f: mov    bx,ax
// 0xed71: jmp    word ptr [bx]
        case 0xed62:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
// 0xee65: mov    cx,es
// 0xee67: mov    dx,[5DA3] // ?EGA
// 0xee6b: and    dx,01
// 0xee6e: jnz    EE76
// 0xee70: mov    dx,[55D8] // HBUF-SEG
// 0xee74: jmp    EE7A
// 0xee76: mov    dx,[52B3] // XBUF-SE
// 0xee7a: mov    es,dx
// 0xee7c: xor    ax,ax
// 0xee7e: pop    bx
// 0xee7f: es:    
// 0xee80: mov    al,[bx]
// 0xee82: inc    bx
// 0xee83: es:    
// 0xee84: push   word ptr [bx]
// 0xee86: add    bx,02
// 0xee89: es:    
// 0xee8a: push   word ptr [bx]
// 0xee8c: push   ax
// 0xee8d: mov    es,cx       
        case 0xee65: // PUSH-POLY
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
// 0xeee5: xor    ax,ax
// 0xeee7: push   ax
// 0xeee8: mov    ax,0004
// 0xeeeb: mov    ah,al
// 0xeeed: push   ax
// 0xeeee: mov    ax,[ED81] // WED81
// 0xeef2: push   ax
        case 0xeee5:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }  
            break;
// 0xee98: mov    dx,[52A2] // POLYSEG
// 0xee9c: mov    es,dx
// 0xee9e: mov    bx,[ED7D] // FADDR
// 0xeea2: pop    ax
// 0xeea3: es:    
// 0xeea4: mov    [bx],al
// 0xeea6: add    bx,03
// 0xeea9: es:    
// 0xeeaa: pop    word ptr [bx]
// 0xeeac: sub    bx,02
// 0xeeaf: es:    
// 0xeeb0: pop    word ptr [bx]
// 0xeeb2: add    bx,04
// 0xeeb5: mov    [ED7D],bx // FADDR
        case 0xee96:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }   
            break;   
// 0xeecc: xor    ax,ax
// 0xeece: pop    ax
// 0xeecf: push   ax
// 0xeed0: cmp    ax,[ED81] // WED81
// 0xeed4: jnz    EEDB
// 0xeed6: mov    ax,0001
// 0xeed9: jmp    EEDD
// 0xeedb: xor    ax,ax
// 0xeedd: push   ax
        case 0xeecc:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;       
// 0xeec2: add    sp,06
        case 0xeec2:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0xdf13:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }   
            break;
        case 0xe1b6:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0xe228:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
// 0xdf02: mov    [DC20],sp // WDC20            
        case 0xdf02:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0xdd2c:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
         
        case 0xe48c:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0xe4aa:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0x9841: // BUFFERXY
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0x9097: // SQLPLOT - square plot, used for the fracal maps
            {
                int16_t y = Pop();
                int16_t x = Pop();

                auto seg = Read16(0x5648);
                auto color = Read16(0x55F2);

                GraphicsPixel(x, y, color, seg, Rotoscope(PlotPixel));
                GraphicsPixel(x, y - 1, color, seg, Rotoscope(PlotPixel));
            }
            break;
        case 0xf0c1: // |EGA
            {
                uint16_t ax;
                uint16_t cx;
                uint16_t ds;
                uint16_t es;
                uint16_t si;
                uint16_t di;

                cx = Pop(); // pop    cx
                ax = Read16(0x55E6); // mov    ax,[55E6] // DBUF-SEG
                dx = Read16(0x55D8); // mov    dx,[55D8] // HBUF-SEG
                bx = 0x00C5; // mov    bx,00C5
                bx <<= 1; // shl    bx,1
                bx += Read16(0x563A); // add    bx,[563A] // YTABL
                si = Read16(bx); // mov    si,[bx]
                di = si; // mov    di,si
                if(cx & 0x0001) // test   cx,0001
                {               // jnz    F0E2
                    std::swap(ax, dx); // xchg   ax,dx
                }
                es = ax; // mov    es,ax
                ds = dx; // mov    ds,dx
                cx = 0x0A28; // mov    cx,0A28
                
                GraphicsCopyLine(es, ds, si, di, cx);
            }
            break;
        case 0xe16b:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;            
        case 0x1047:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0x9081:
            {
                uint16_t width = Pop();
                uint16_t len = Pop();
                uint16_t yul = Pop();
                uint16_t xul = Pop();
                #if 0
                printf("TILEFILL xul %u yul %u len %u width %u\n",
                    xul, yul,
                    len, width);
                #endif

                yul -= 7;

                uint16_t COLOR, TILE_PTR, BUF_SEG;
                TILE_PTR = Read16(0x58CD);
                BUF_SEG = Read16(0x5648);
                COLOR = Read16(TILE_PTR);

                for(uint16_t y = 0; y < len; ++y)
                {
                    for(uint16_t x = 0; x < width; ++x)
                    {
                        GraphicsPixel(xul + x, yul + y, COLOR & 0xff, BUF_SEG, Rotoscope(TilePixel));
                    }
                }
            }
            break;
        case 0xefd9: // CBLTP -- MAPS-OV
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0xe8f3:// CBLTP -- MOVE-OV
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0x48dc:
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
        case 0x8783: // CLIPPER
            {
                Run8086(cs, addr, ds, cs, &regsp);
            }
            break;
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

        for (const auto & entry : std::filesystem::directory_iterator("./")) {
            std::string file = entry.path().filename().string();
            if (file.substr(file.find_last_of(".") + 1) != "zst") continue;
            if (file.find(prefix) == 0) {
                if (!Deserialize(file)) {
                    fprintf(stderr, "Error: Cannot deserialize file %s\n", file.c_str());
                    exit(1);
                } else {
                    printf("File %s was loaded for resumption.\n", file.c_str());
                    break;
                }
            }
        }
    }
}

void EnableDebug()
{
  debuglevel = 1;
}

enum RETURNCODE Step()
{
    unsigned short ax = Read16(regsi); // si is the forth program counter
    regsi += 2;
    unsigned short bx = ax;
    unsigned short execaddr = Read16(bx);
    //if (regsi-2 == 0xea37) printf(" enter %s\n", FindWord(bx+2, -1));

    if (debuglevel)
    {
      int ovidx = GetOverlayIndex(Read16(0x55a5), nullptr);
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

    enum RETURNCODE ret = Call(execaddr, bx);
    return ret;
}

void InitEmulator(std::string hash)
{
    regbp = 0xd4a7 + 0x100 + 0x80; // call stack
    regsp = 0xd4a7 + 0x100;  // initial parameter stack
    LoadSTARFLT(hash);
    inputbuffer.clear();
}
