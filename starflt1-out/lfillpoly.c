#include <stdint.h>

uint16_t ax, bx, cx, dx, di, es;
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
#define YTABL (*((uint16_t*)&memory[0x563A]))
#define COLOR memory[0x55F2]

void outp(uint16_t port, uint8_t val);

// SUB 0037 Named SET-COLO
void SET_COLO() {
    dx = 0x03CE;
    uint8_t ah = COLOR;
    uint8_t al = 0x00;
    outp(dx, al);
    dx++;
    uint8_t temp = ah;
    ah = al;
    al = temp;
    outp(dx, al);
}

// SUB 0053 Named SET-BIT
void SET_BIT() {
    uint16_t temp_dx = dx;
    uint8_t ah = (dx & 0xff);
    dx = 0x03CE;
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
    ah = COLOR;
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
    cx = XSTART;
    ax = XEND;
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
    ax = XSTART;
    if (ax & 0x0003) {
        ax = 0x0000;
    } else {
        ax = 0x0001;
    }
}

// SUB 033B Named YADD
void YADD() {
    dx <<= 1;
    dx += YTABL;
    di = dx;
    dx = memory[di];
    di = dx;
}

void FAST_FIL() {
    uint16_t temp_di = di;
    cx = XEND;
    ax = XSTART;
    cx -= ax;
    uint16_t temp_cx = cx;
    cx >>= 2;
    dx = YLINE;
    YADD();
    dx = XSTART;
    dx >>= 2;
    di += dx;
    dx = 0x00FF;
    SET_BIT();
    uint8_t al = (ax & 0xff);
    while (--cx) {
        memoryES[di] = al;
        --di;
    }
    ax = XSTART;
    cx = temp_cx;
    ax += cx;
    ax -= 0x0004;
    XSTART = ax; // XSTART
    di = temp_di;
}
 
// SUB 03dd Named THESE-BIT
void THESE_BIT() {
    memory[0x57C2]++; // XEND
    cx = memory[0x57B7]; // XSTART
restart:
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
    if (cx != memory[0x57C2]) { // XEND
        goto restart;
    }
    memory[0x57C2]--; // XEND
}

void SOME_BIT() {
    XEND++; // equivalent to inc word ptr [57C2]
    cx = XSTART; // equivalent to mov cx,[57B7]
    dx = cx;
    dx &= 0x00FC;
    dx += 0x04;
looping:
    uint16_t temp_dx = dx;
    uint16_t temp_cx = cx;
    dx = YLINE; // equivalent to push word ptr [57CE] and pop dx
    ax = temp_cx;
    dx <<= 1;
    dx += YTABL; // equivalent to add dx,[563A]
    ax &= 0x0003;
    ax <<= 1;
    bx = 0x92CF;
    bx += ax;
    dx = memory[bx];
    ax = temp_cx;
    ax >>= 2;
    bx = dx;
    ax += memory[bx];
    bx = ax;
    WRITE_DO();
    cx = temp_cx;
    ++cx;
    if (cx != XEND) {
        goto looping;
    }
    --XEND;
    dx = temp_dx;
    XSTART = dx;
}

; SUB 042C Named SOME-BIT
0x042c: inc    word ptr [57C2] // XEND
0x0430: mov    cx,[57B7] // XSTART
0x0434: mov    dx,cx
0x0436: and    dx,00FC
0x043a: add    dx,04
0x043d: push   dx // dx
0x043e: push   cx // dx cx
0x043f: push   dx // dx cx dx
0x0440: push   cx // cx dx cx dx 
0x0441: push   word ptr [57CE] // YLINE
0x0445: pop    dx
0x0446: pop    ax
0x0447: shl    dx,1
0x0449: add    dx,[563A] // YTABL
0x044d: push   dx
0x044e: push   ax
0x044f: and    ax,0003
0x0452: shl    ax,1
0x0454: mov    bx,92CF
0x0457: add    bx,ax
0x0459: mov    dx,[bx]
0x045b: pop    ax
0x045c: shr    ax,1
0x045e: shr    ax,1
0x0460: pop    bx
0x0461: add    ax,[bx]
0x0463: mov    bx,ax
0x0465: call   0246
0x0468: pop    dx
0x0469: pop    cx
0x046a: inc    cx
0x046b: cmp    cx,dx
0x046d: jz     0472
0x046f: jmp    043E
0x0472: dec    word ptr [57C2] // XEND
0x0476: pop    dx
0x0477: mov    [57B7],dx // XSTART
0x047b: ret    

void FVLIN() {
    dx = XEND;
    if (dx > IRIGHT) {
        return;
    }
    ax = XSTART;
    dx = XEND;
    if (ax > dx) {
        return;
    }
    uint16_t temp_es = es;
    ax = BUF_SEG;
    es = ax;
    BYT();
    if (!(ax & 0x0001)) {
        SHOR();
        if (!(ax & 0x0001)) {
            THESE_BIT();
            es = temp_es;
            return;
        }
        goto label_04CD;
    }
    SHOR();
    if (!(ax & 0x0001)) {
        THESE_BIT();
        es = temp_es;
        return;
    }
label_04CD:
    SHOR();
    if (!(ax & 0x0001)) {
        THESE_BIT();
        es = temp_es;
        return;
    }
    SOME_BIT();
    ax = XEND;
    if (ax == XSTART) {
        es = temp_es;
        return;
    }
    THESE_BIT();
    es = temp_es;
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
0x04b9: jmp    04CD
0x04bb: call   0353
0x04be: test   ax,0001
0x04c1: jz     04CA
0x04c3: call   03DD
0x04c6: pop    es
0x04c7: ret    
0x04c8: db 0xeb 0x03 '  '
0x04ca: call   042C
0x04cd: call   0353
0x04d0: test   ax,0001
0x04d3: jz     04DC
0x04d5: call   03DD
0x04d8: pop    es
0x04d9: ret    
0x04da: db 0xeb 0x03 '  '
0x04dc: call   0396
0x04df: mov    ax,[57C2] // XEND
0x04e3: cmp    ax,[57B7] // XSTART
0x04e7: jz     04EC
0x04e9: call   03DD
0x04ec: pop    es
0x04ed: ret    


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