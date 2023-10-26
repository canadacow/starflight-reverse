#include <stdint.h>

uint16_t ax, bx, cx, dx, es;
uint8_t memory[65536];
uint8_t memoryES[65536];

#define XSTART (*((uint16_t*)&memory[0x57B7]))
#define XEND (*((uint16_t*)&memory[0x57C2]))
#define YMIN (*((uint16_t*)&memory[0x57EC]))
#define YMAX (*((uint16_t*)&memory[0x57F7]))
#define SCAN (*((uint16_t*)&memory[0x57D9]))
#define YLINE (*((uint16_t*)&memory[0x57CE]))
#define IRIGHT (*((uint16_t*)&memory[0x5745]))
#define BUF_SEG (*((uint16_t*)&memory[0x5648]))

void outp(uint16_t port, uint8_t val);

// SUB 37 Named SET-COLO
void SET_COLO() {
    dx = 0x03CE;
    uint8_t ah = memory[0x55F2]; // COLOR
    uint8_t al = 0x00;
    outp(dx, al);
    dx++;
    uint8_t temp = ah;
    ah = al;
    al = temp;
    outp(dx, al);
}

// SUB 53 Named SET-BIT
void SET_BIT() {
    dx = 0x03CE;
    uint8_t ah = dl;
    uint8_t al = 0x08;
    outp(dx, al);
    dx++;
    uint8_t temp = ah;
    ah = al;
    al = temp;
    outp(dx, al);
}

// SUB 0246 Named WRITE-DO
void WRITE_DO() {
    ax = dx;
    uint8_t temp = ah;
    ah = al;
    al = temp;
    dx = 0x03CE;
    al = 0x08;
    outp(dx, al);
    dx++;
    temp = ah;
    ah = al;
    al = temp;
    outp(dx, al);
    dx--;
    ah = memory[0x55F2]; // COLOR
    al = 0x00;
    outp(dx, al);
    dx++;
    temp = ah;
    ah = al;
    al = temp;
    outp(dx, al);
    al = memoryES[bx];
    memoryES[bx] = al;
}

// SUB 0353 Named ?SHOR
void SHOR() {
    cx = memory[0x57B7]; // XSTART
    ax = memory[0x57C2]; // XEND
    ax -= cx;
    ax -= 0x0003;
    if (ax > 0) {
        ax = 0x0001;
    } else {
        ax = 0x0000;
    }
}

// SUB 0376 Named ?-BYT
void BYT() {
    ax = memory[0x57B7]; // XSTART
    if (ax & 0x0003) {
        ax = 0x0000;
    } else {
        ax = 0x0001;
    }
}

/*
; SUB 03dd Named THESE-BIT
THESE-BIT:
0x03dd: inc    word ptr [57C2] // XEND
0x03e1: mov    cx,[57B7] // XSTART
0x03e5: push   cx
0x03e6: push   cx
0x03e7: push   word ptr [57CE] // YLINE
0x03eb: pop    dx
0x03ec: pop    ax
0x03ed: shl    dx,1
0x03ef: add    dx,[563A] // YTABL
0x03f3: push   dx
0x03f4: push   ax
0x03f5: and    ax,0003
0x03f8: shl    ax,1
0x03fa: mov    bx,92CF
0x03fd: add    bx,ax
0x03ff: mov    dx,[bx]
0x0401: pop    ax
0x0402: shr    ax,1
0x0404: shr    ax,1
0x0406: pop    bx
0x0407: add    ax,[bx]
0x0409: mov    bx,ax
0x040b: call   0246
0x040e: pop    cx
0x040f: inc    cx
0x0410: cmp    cx,[57C2] // XEND
0x0414: jz     0419
0x0416: jmp    03E5
0x0419: dec    word ptr [57C2] // XEND
0x041d: ret    
*/

// SUB 03dd Named THESE-BIT
void THESE_BIT() {
    memory[0x57C2]++; // XEND
    cx = memory[0x57B7]; // XSTART
    uint16_t temp_cx = cx;
    uint16_t temp_ax = cx;
    dx = memory[0x57CE]; // YLINE
    ax = temp_ax;
    dx <<= 1;
    dx += memory[0x563A]; // YTABL
    uint16_t temp_dx = dx;
    ax = temp_ax;
    ax &= 0x0003;
    ax <<= 1;
    bx = 0x92CF;
    bx += ax;
    dx = memory[bx];
    ax = temp_ax;
    ax >>= 2;
    bx = temp_dx;
    ax += memory[bx];
    bx = ax;
    WRITE_DO();
    cx = temp_cx;
    cx++;
    if (cx == memory[0x57C2]) { // XEND
        return;
    } else {
        THESE_BIT();
    }
    memory[0x57C2]--; // XEND
}

; SUB 0487 Named FVLIN
FVLIN:
0x0487: mov    dx,[57C2] // XEND
0x048b: cmp    dx,[5745] // IRIGHT
0x048f: ja     04ED
0x0491: mov    ax,[57B7] // XSTART
0x0495: mov    dx,[57C2] // XEND
0x0499: cmp    ax,dx
0x049b: ja     04ED
0x049d: push   es
0x049e: mov    ax,[5648] // BUF-SEG
0x04a2: mov    es,ax
0x04a4: call   0376
0x04a7: test   ax,0001
0x04aa: jz     04BB
0x04ac: call   0353
0x04af: test   ax,0001
0x04b2: jz     04B9
0x04b4: call   03DD
0x04b7: pop    es
0x04b8: ret    

LFILLPOLY:
0x050f: mov    bx,[57EC] // YMIN
0x0513: add    bx,[57F7] // YMAX
0x0517: inc    bx
0x0518: shr    bx,1
0x051a: shl    bx,1
0x051c: add    bx,[57D9] // SCAN
0x0520: xor    ax,ax
0x0522: xor    cx,cx
0x0524: mov    al,[bx]
0x0526: inc    bx
0x0527: mov    cl,[bx]
0x0529: cmp    cx,ax
0x052b: jns    052E
0x052d: retf   
0x052e: mov    dx,03CE
0x0531: mov    al,01
0x0533: mov    ah,0F
0x0535: out    dx,al
0x0536: inc    dx
0x0537: xchg   ah,al
0x0539: out    dx,al
0x053a: mov    dx,00FF
0x053d: call   0053
0x0540: call   0037
0x0543: inc    word ptr [57F7] // YMAX
0x0547: mov    cx,[57EC] // YMIN
0x054b: push   cx
0x054c: mov    [57CE],cx // YLINE
0x0550: shl    cx,1
0x0552: mov    bx,[57D9] // SCAN
0x0556: add    bx,cx
0x0558: sub    ax,ax
0x055a: mov    al,[bx]
0x055c: mov    [57B7],ax // XSTART
0x0560: inc    bx
0x0561: mov    al,[bx]
0x0563: mov    [57C2],ax // XEND
0x0567: call   0487
0x056a: pop    cx
0x056b: inc    cx
0x056c: cmp    cx,[57F7] // YMAX
0x0570: jz     0575
0x0572: jmp    054B
0x0575: dec    word ptr [57F7] // YMAX
0x0579: retf   