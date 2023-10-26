#include <stdint.h>
#include <stack>

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
// this function sets the color for subsequent drawing operations on the EGA video card 
// by writing the color value to the Set/Reset Register of the Graphics Controller.
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
// dx on input represents the bit mask that will be used for subsequent write operations to video memory.
//    GR08 - 7 6 5 4 3 2 1 0 - Bit Mask Register
//           | | | | | | | |
//           | | | | | | | +-- write enable bit 0
//           | | | | | | +---- write enable bit 1
//           | | | | | +------ write enable bit 2
//           | | | | +-------- write enable bit 3
//           | | | +---------- write enable bit 4
//           | | +------------ write enable bit 5
//           | +-------------- write enable bit 6
//           +---------------- write enable bit 7
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

// SUB 0396 Named FAST-FIL
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
 
// SUB 03DD Named THESE-BIT
void THESE_BIT() {
    ++XEND; // XEND
    cx = XSTART; // XSTART
    do
    {
        uint16_t temp_cx = cx;
        uint16_t temp_ax = cx;
        dx = YLINE; // YLINE
        ax = temp_ax;
        dx <<= 1;
        dx += YTABL; // YTABL
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
    } while (cx != XEND);
    
    --XEND;
}

// SUB 042C Named SOME-BIT
void SOME_BIT() {
    std::stack<uint16_t> s;
    XEND++;
    cx = XSTART;
    dx = cx;
    dx &= 0x00FC;
    dx += 0x04;
    s.push(dx);
    do {
        s.push(cx);
        s.push(dx);
        s.push(cx);
        s.push(YLINE);
        dx = s.top(); s.pop();
        ax = s.top(); s.pop();
        dx <<= 1;
        dx += YTABL;
        s.push(dx);
        s.push(ax);
        ax &= 0x0003;
        ax <<= 1;
        bx = 0x92CF;
        bx += ax;
        dx = memory[bx];
        ax = s.top(); s.pop();
        ax >>= 2;
        bx = s.top(); s.pop();
        ax += memory[bx]; 
        bx = ax;
        WRITE_DO();
        dx = s.top(); s.pop(); 
        cx = s.top(); s.pop(); 
        cx++;
    } while (cx != dx);

    XEND--;
    dx = s.top(); s.pop();
    XSTART = dx;
}

// Sub 0487 Named FVLIN
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

void LFILLPOLY() {
    std::stack<uint16_t> s;
    bx = YMIN;
    bx += YMAX;
    bx++;
    bx >>= 1;
    bx <<= 1;
    bx += SCAN;
    ax = 0;
    cx = 0;
    ax = memory[bx];
    bx++;
    cx = memory[bx];
    if (cx >= ax) {
        // Enabling all color planes for drawing operations on the EGA.
        dx = 0x03CE;
        outp(dx, 0x01);
        dx++;
        outp(dx, 0x0F);
        dx = 0x00FF;
        SET_BIT();
        SET_COLO();
        YMAX++;
        cx = YMIN;
        do {
            s.push(cx);
            YLINE = cx;
            cx <<= 1;
            bx = SCAN;
            bx += cx;
            ax = 0;
            ax = memory[bx];
            XSTART = ax;
            bx++;
            ax = memory[bx];
            XEND = ax;
            FVLIN();
            cx = s.top(); s.pop();
            cx++;
        } while (cx != YMAX);

        YMAX--;
    }
}


// ================================================
// 0x8885: WORD 'SCANPOLY' codep=0x8891 wordp=0x8891 params=0 returns=0
// ================================================
push   di
xor    cx,cx
mov    [57FB],cx // #HORIZ
mov    [57DD],cx // ELEMENT
mov    di,[569B] // VIN
mov    ax,[5686] // #IN
dec    ax
shl    ax,1
shl    ax,1
add    di,ax
mov    cx,[di]
mov    [5708],cx // W5708
add    di,02
mov    cx,[di]
mov    [5704],cx // W5704
mov    di,[569B] // VIN
mov    cx,[di]
mov    [57EC],cx // YMIN
mov    [57F7],cx // YMAX
mov    cx,[5686] // #IN
mov    ax,[di]
mov    [5714],ax // W5714
add    di,02
mov    bx,[di]
mov    [5710],bx // W5710
add    di,02
mov    dx,[5708] // W5708
cmp    ax,dx
jnz    88FF
inc    word ptr [57FB] // #HORIZ
cmp    bx,[5704] // W5704
jns    88F7
push   word ptr [5704] // W5704
push   bx
jmp    88FC
push   bx
push   word ptr [5704] // W5704
push   ax
jmp    8937
mov    dx,[57EC] // YMIN
cmp    dx,ax
jle    890D
mov    [57EC],ax // YMIN
jmp    8919
mov    dx,[57F7] // YMAX
cmp    dx,ax
jns    8919
mov    [57F7],ax // YMAX
xor    dx,dx
mov    bx,[5708] // W5708
cmp    bx,ax
jle    8924
inc    dx
mov    [57DD],dx // ELEMENT
add    dx,[57D9] // SCAN
mov    [57E1],dx // SCAN+
push   cx
push   di
call   87E1
pop    di
pop    cx
mov    ax,[5714] // W5714
mov    [5708],ax // W5708
mov    ax,[5710] // W5710
mov    [5704],ax // W5704
dec    cx
jz     894D
jmp    88CC
cmp    [57FB],cx // #HORIZ
jle    898F
mov    ax,0900
mov    [5708],ax // W5708
mov    cx,[57FB] // #HORIZ
pop    dx
mov    di,dx
shl    di,1
add    di,[57D9] // SCAN
cmp    dx,[5708] // W5708
jnz    8982
pop    ax
mov    ah,[di]
cmp    al,ah
jns    8976
mov    [di],al
inc    di
pop    ax
mov    ah,[di]
cmp    al,ah
jle    8980
mov    [di],al
jmp    898D
mov    [5708],dx // W5708
pop    ax
mov    [di],al
pop    ax
inc    di
mov    [di],al
loop   895E
pop    di
lodsw
mov    bx,ax
jmp    word ptr [bx]