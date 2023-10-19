#include <stdint.h>
#include <stdio.h>

#include <thread>
#include <chrono>

union MusicPlayer {
    uint8_t unknown[0x10];
    struct {
        uint8_t _padding[0x2];
        uint8_t isrEnabled; 
    };
    struct {
        uint8_t _padding[0x5];
        uint16_t currentSequenceAddressInMem;  // 0x5
        uint16_t currentNoteAddressInMem;      // 0x7
        uint8_t  stanzasLeftInSequence;        // 0x9
        uint8_t  tickCounter;                  // 0xA
        uint8_t  noteOnDuration;               // 0xB
        uint8_t  restDuration;                 // 0xC
        uint8_t  speakerIsOff;                 // 0xD
    };
};

const unsigned short int pp__0SONG = 0xe580; // 0SONG size: 23
// {0x01, 0x4b, 0x02, 0x01, 0xbe, 0x01, 0x01, 0x4b, 0x02, 0x01, 0xd9, 0x01, 0x01, 0xf4, 0x01, 0x01, 0x0f, 0x02, 0x01, 0x30, 0x02, 0x00, 0x00}

extern uint8_t mem[0x10000];

extern uint16_t frequencyLookupTable[0x100];

void TurnOnPCSpeaker(uint16_t frequency) {
    printf("* SPEAKER ON FREQ %d *\n", frequency);
}

void TurnOffPCSpeaker() {
    printf("* SPEAKER OFF *\n");
}

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
        uint16_t si = player->currentNoteAddressInMem;

        uint8_t noteDuration = mem[si++] & 0x7F;
        if (noteDuration != 0) {
            uint8_t ah = 0;
            if (noteDuration & 0x40) {
                noteDuration &= 0x3F;
                ah++;
            }
            player->noteOnDuration = noteDuration;
            player->restDuration = ah;
            uint8_t note = mem[si++];
            player->currentNoteAddressInMem = si;
            if (note == 0xFF) {
                player->tickCounter = player->noteOnDuration;
                TurnOffPCSpeaker();
                player->speakerIsOff = 1;
                return;
            }
            ah = player->noteOnDuration - player->restDuration;
            player->tickCounter = ah;
            TurnOnPCSpeaker(frequencyLookupTable[note]);
            player->speakerIsOff = 0;
            return;
        }

        si = player->currentSequenceAddressInMem;
        --player->stanzasLeftInSequence;
        if (player->stanzasLeftInSequence == 0) {
            si += 3;
            uint8_t al = mem[si];
            if (al == 0) {
                player->isrEnabled = 0;
                return;
            }
            player->currentSequenceAddressInMem = si;
            player->stanzasLeftInSequence = al;
        }

        player->currentNoteAddressInMem = *(uint16_t*)&mem[si + 1];
        player->tickCounter = 1;
        player->speakerIsOff = 1;
    }
}

void main() {

    FILE *fp = fopen("76a4-0000.bin", "rb");
    if (fp != NULL) {
        fread(mem, 1, 0x1000, fp);
        fclose(fp);
    }

    fp = fopen("76a4-0000.bin", "rb");
    if (fp != NULL) {
        fseek(fp, 0x24, SEEK_SET);
        fread(frequencyLookupTable, 1, 0x100, fp);
        fclose(fp);
    }

    uint16_t songOffset = 0x280;
    uint8_t repeats = mem[songOffset];
    uint16_t curNoteAddress = *(uint16_t*)&mem[songOffset + 1];

    union MusicPlayer mp{};

    mp.currentSequenceAddressInMem = songOffset;
    mp.currentNoteAddressInMem = curNoteAddress;
    mp.stanzasLeftInSequence = repeats;
    mp.tickCounter = 1;
    mp.speakerIsOff = 1;
    mp.isrEnabled = 1;

    while(mp.isrEnabled)
    {
        printf("tick\n");
        pit_interrupt_service_routine(&mp);

        std::this_thread::sleep_for(std::chrono::milliseconds(55));
    }
}