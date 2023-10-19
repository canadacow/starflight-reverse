#include <stdint.h>

union MusicPlayer {
    uint8_t unknown[0x10];
    struct {
        uint8_t _padding[0x2];
        uint8_t isrEnabled;
        uint8_t _rest[0x10 - 0x2 - 1];
    };
    struct {
        uint8_t _padding[0xA];
        uint8_t tickCounter;
        uint8_t _rest[0x10 - 0xA - 1];
    };
    struct {
        uint8_t _padding[0xD];
        uint8_t speakerIsOff;
        uint8_t _rest[0x10 - 0xD - 1];
    };
    struct {
        uint8_t _padding[0xC];
        uint8_t restDuration;
        uint8_t _rest[0x10 - 0xC - 1];
    };
    struct {
        uint8_t _padding[0xB];
        uint8_t noteOnDuration;
        uint8_t _rest[0x10 - 0xB - 1];
    };
};

extern uint8_t mem[0x10000];

extern uint16_t frequencyLookupTable[0x7f];

void TurnOnPCSpeaker(uint16_t frequency);
void TurnOffPCSpeaker();

// pit_interrupt_service_routine is an ISR on INT 0x1C, the programmable
// timer. This code reads from state in memory (mem) and the MusicPlayer
// state, enabling the PC speaker at specified frequencies or disabling
// the speaker altogether. The result is music played back
// through the PC speaker.
void pit_interrupt_service_routine(union MusicPlayer* player) {

    if(!player->isrEnabled)
        return;

    player->tickCounter--;
    if (player->tickCounter == 0) {
        if ((player->speakerIsOff) || (player->restDuration == 0)) {
            // Speaker is already off or no rest between notes
            // Continue to next note change
        } else {
            player->tickCounter = player->restDuration;
            TurnOffPCSpeaker();
            player->speakerIsOff = 1;
            return;
        }
    }

    for(;;) {
        uint16_t si = *(uint16_t*)&player->unknown[0x7];
        uint8_t al = mem[si];
        al &= 0x7F;
        if (al != 0) {
            uint8_t ah = 0;
            if (al & 0x40) {
                al &= 0x3F;
                ah++;
            }
            player->noteOnDuration = al;
            player->restDuration = ah;
            al = mem[si];
            ++si;
            *(uint16_t*)&player->unknown[0x7] = si;
            if (al == 0xFF) {
                player->tickCounter = player->noteOnDuration;
                TurnOffPCSpeaker();
                player->speakerIsOff = 1;
                return;
            }
            ah = player->noteOnDuration - player->restDuration;
            player->tickCounter = ah;
            uint16_t dx = al;
            TurnOnPCSpeaker(frequencyLookupTable[dx]);
            player->speakerIsOff = 0;
            return;
        }

        si = *(uint16_t*)&player->unknown[0x5];
        player->unknown[0x9]--;
        if (player->unknown[0x9] == 0) {
            si += 3;
            al = mem[si];
            if (al == 0) {
                player->isrEnabled = 0;
                return;
            }
            *(uint16_t*)&player->unknown[0x5] = si;
            player->unknown[0x9] = al;
        }

        *(uint16_t*)&player->unknown[0x7] = *(uint16_t*)&mem[si + 1];
        player->tickCounter = 1;
        player->speakerIsOff = 1;
    }
}