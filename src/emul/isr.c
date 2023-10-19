#include <stdint.h>

struct MusicPlayer {
    uint8_t unknown[0xf];
};

extern uint8_t mem[0x10000];

extern uint16_t frequencyLookupTable[0x7f];

void TurnOnPCSpeaker(uint16_t frequency);
void TurnOffPCSpeaker();

void interrupt_service_routine(struct MusicPlayer* player) {

    uint8_t al = player->unknown[0x2];
    if(al == 0)
        return;

    player->unknown[0xA]--;
    if (player->unknown[0xA] == 0) {
        if (player->unknown[0xD] != 0) {
            return;
        }
        if (player->unknown[0xC] != 0) {
            return;
        }
        player->unknown[0xA] = player->unknown[0xC];
        TurnOffPCSpeaker();
        player->unknown[0xD] = 1;
        return;
    }

    for(;;) {
        uint16_t si = *(uint16_t*)&player->unknown[0x7];
        al = mem[si];
        al &= 0x7F;
        if (al != 0) {
            uint8_t ah = 0;
            if (al & 0x40) {
                al &= 0x3F;
                ah++;
            }
            player->unknown[0xB] = al;
            player->unknown[0xC] = ah;
            al = mem[si];
            ++si;
            *(uint16_t*)&player->unknown[0x7] = si;
            if (al == 0xFF) {
                player->unknown[0xA] = player->unknown[0xB];
                TurnOffPCSpeaker();
                player->unknown[0xD] = 1;
                return;
            }
            ah = player->unknown[0xB] - player->unknown[0xC];
            player->unknown[0xA] = ah;
            uint16_t dx = al;
            TurnOnPCSpeaker(frequencyLookupTable[dx]);
            player->unknown[0xD] = 0;
            return;
        }

        si = *(uint16_t*)&player->unknown[0x5];
        player->unknown[0x9]--;
        if (player->unknown[0x9] == 0) {
            si += 3;
            al = mem[si];
            if (al == 0) {
                player->unknown[0x2] = 0;
                return;
            }
            *(uint16_t*)&player->unknown[0x5] = si;
            player->unknown[0x9] = al;
        }

        *(uint16_t*)&player->unknown[0x7] = *(uint16_t*)&mem[si + 1];
        player->unknown[0xA] = 1;
        player->unknown[0xD] = 1;
    }
}