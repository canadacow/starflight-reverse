#include <stdint.h>
#include <stdio.h>

#include <thread>
#include <chrono>
#include <vector>

const unsigned short int pp__0SONG = 0xe580; // 0SONG size: 23
// {0x01, 0x4b, 0x02, 0x01, 0xbe, 0x01, 0x01, 0x4b, 0x02, 0x01, 0xd9, 0x01, 0x01, 0xf4, 0x01, 0x01, 0x0f, 0x02, 0x01, 0x30, 0x02, 0x00, 0x00}

uint8_t mem[0x1000];

uint16_t frequencyLookupTable[0x100];

void TurnOnPCSpeaker(uint16_t frequency) {
    //printf("* SPEAKER ON FREQ %d *\n", 1193180 / frequency);
}

void TurnOffPCSpeaker() {
    // printf("* SPEAKER OFF *\n");
}

/*
 * The pit_interrupt_service_routine function is responsible for playing music on the PC speaker.
 * It is an Interrupt Service Routine (ISR) that gets triggered by the Programmable Interval Timer (PIT) interrupt (INT 0x1C).
 *
 * The function first checks if the ISR is enabled. If not, it returns immediately.
 * Then, it decreases the tick counter. If the tick counter is still greater than zero, it returns.
 * If the speaker is not off and there is a rest duration, it sets the tick counter to the rest duration, turns off the speaker, and returns.
 * 
 * The function then enters a loop where it reads the next note from memory. If the note duration is not zero, it processes the note.
 * If the note is 0xFF, it sets the tick counter to the note duration, turns off the speaker, and returns.
 * Otherwise, it sets the tick counter to the difference between the note duration and the rest duration, turns on the speaker with the frequency of the note, and returns.
 * 
 * If the note duration is zero, it checks if there are any repeats left. If not, it advances to the next sequence.
 * If there are no more sequences left, it turns off the speaker and returns.
 */
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

void pit_interrupt_service_routine(union MusicPlayer* player) {

    if (!player->isrEnabled)
        return;

    --player->tickCounter;
    if (player->tickCounter > 0)
        return;

    if ((!player->speakerIsOff) && (player->restDuration != 0)) {
        player->tickCounter = player->restDuration;
        TurnOffPCSpeaker();
        player->speakerIsOff = 1;
        return;
    }

    for (;;) {
        uint16_t noteAddress = player->currentNoteAddressInMem;

        uint8_t noteDuration = mem[noteAddress++] & 0x7F;
        if (noteDuration != 0) {
            uint8_t restDuration = 0;
            if (noteDuration & 0x40) {
                noteDuration &= 0x3F;
                restDuration++;
            }
            player->noteOnDuration = noteDuration;
            player->restDuration = restDuration;
            uint8_t note = mem[noteAddress++];
            player->currentNoteAddressInMem = noteAddress;
            if (note == 0xFF) {
                player->tickCounter = player->noteOnDuration;
                TurnOffPCSpeaker();
                player->speakerIsOff = 1;
                return;
            }
            uint8_t tickDuration = player->noteOnDuration - player->restDuration;
            player->tickCounter = tickDuration;
            TurnOnPCSpeaker(frequencyLookupTable[note]);
            player->freqValue = 1193180 / frequencyLookupTable[note];
            player->speakerIsOff = 0;
            return;
        }

        
        uint16_t sequenceAddress = player->currentSequenceAddressInMem;
        --player->repeats;
        if (player->repeats == 0) {
            sequenceAddress += 3;
            uint8_t repeatCount = mem[sequenceAddress];
            if (repeatCount == 0) {
                player->isrEnabled = 0;
                return;
            }
            player->currentSequenceAddressInMem = sequenceAddress;
            player->repeats = repeatCount;
        }

        player->currentNoteAddressInMem = *(uint16_t*)&mem[sequenceAddress + 1];
        player->tickCounter = 1;
        player->speakerIsOff = 1;
    }
}

int main() {

    FILE* fp = fopen("76a4-0000.bin", "rb");
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

    union MusicPlayer mp {};

    mp.currentSequenceAddressInMem = songOffset;
    mp.currentNoteAddressInMem = curNoteAddress;
    mp.repeats = repeats;
    mp.tickCounter = 1;
    mp.speakerIsOff = 1;
    mp.isrEnabled = 1;

    std::vector<uint16_t> notes;

    while (mp.isrEnabled)
    {
        //printf("tick\n");
        pit_interrupt_service_routine(&mp);

        notes.push_back(mp.freqValue);

        //std::this_thread::sleep_for(std::chrono::milliseconds(55));
    }

    return 0;
}