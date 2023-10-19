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
    struct {
        uint8_t _padding[0x9];
        uint8_t stanzasLeftInSequence;
        uint8_t _rest[0x10 - 0x9 - 1];
    };
    struct {
        uint8_t _padding[0x5];
        uint16_t currentSequenceAddressInMem;
        uint8_t _rest[0x10 - 0x5 - 2];
    };
    struct {
        uint8_t _padding[0x7];
        uint16_t currentNoteAddressInMem;
        uint8_t _rest[0x10 - 0x7 - 2];
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
        uint16_t si = player->currentNoteAddressInMem;

        // 'al' is the current note in memory. It is a single byte value.
        // The lower 6 bits (after masking with 0x3F) represent the duration of the note when the speaker is on.
        // The 7th bit (0x40) if set, indicates that there is a rest duration after the note.
        // The rest duration is equal to the note duration if the 7th bit is set.
        // If 'al' is 0xFF, it indicates the end of the current sequence of notes.
        // In this case, the speaker is turned off and the function returns.
        // If 'al' is not 0xFF, the speaker is turned on at a frequency corresponding to the note.
        // The frequency is looked up from the 'frequencyLookupTable' using 'al' as the index.
        // The speaker is then turned on at this frequency.
        // The function then returns, leaving the speaker on for the duration of the note.

        uint8_t noteVal = mem[si++] & 0x7F;
        if (noteVal != 0) {
            uint8_t ah = 0;
            if (noteVal & 0x40) {
                noteVal &= 0x3F;
                ah++;
            }
            player->noteOnDuration = noteVal;
            player->restDuration = ah;
            uint8_t al = mem[si++];
            player->currentNoteAddressInMem = si;
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

        si = player->currentSequenceAddressInMem;
        --player->stanzasLeftInSequence;
        if (player->stanzasLeftInSequence == 0) {
            si += 3;
            al = mem[si];
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