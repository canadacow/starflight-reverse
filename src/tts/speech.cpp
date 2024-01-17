#include "speech.h"

#include <assert.h>

#include <sapi.h>

#include <curl/curl.h>

#include <deque>
#include <semaphore>
#include <thread>
#include <mutex>
#include <vector>
#include <stdint.h>

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

extern void queue_speech(int16_t* voiceAudio, uint64_t length);

#pragma comment(lib, "winmm.lib")

struct VoiceToSpeak
{
    std::string text;
    std::string voice;
};

// This is a simple struct representing the header of a WAV file.
struct WavHeader {
    char riff[4]; // Should always contain "RIFF"
    uint32_t overall_size;
    char wave[4]; // Should always contain "WAVE"
    char fmt_chunk_marker[4]; // Should always contain "fmt "
    uint32_t length_of_fmt;
    uint16_t format_type;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byterate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_chunk_header[4]; // Should always contain "data"
    uint32_t data_size;
};

static ISpVoice* pVoice = NULL;

#define USE_COQUI 1

static std::mutex s_phraseMutex;
static std::counting_semaphore<100> s_phrasesAvailable{0};
static std::deque<VoiceToSpeak> s_phrases;
static std::jthread s_speaker;
static bool s_stop;

typedef size_t(*CURL_WRITEFUNCTION_PTR)(void*, size_t, size_t, void*);

static size_t curl_writeFunction(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realSize = size * nmemb;
    auto* data = reinterpret_cast<std::vector<uint8_t>*>(userp);
    data->insert(data->end(), (uint8_t*)contents, (uint8_t*)contents + realSize);
    return realSize;
}

static std::pair<WavHeader, const uint8_t*> parseWavData(const std::vector<uint8_t>& wavData) {
    WavHeader header;
    memcpy(&header, wavData.data(), sizeof(WavHeader));
    const uint8_t* audioData = wavData.data() + sizeof(WavHeader);
    return {header, audioData};
}

void sayTextInternal(VoiceToSpeak voiceToSpeak)
{
    CURL *curl;
    CURLcode res;

    std::string speaker_wav = voiceToSpeak.voice;
    std::string style_wav = "";
    std::string language_id = "";
    std::string url = "http://192.168.100.111:5002/api/tts";
    std::string postData = "text=" + voiceToSpeak.text + "&speaker_wav=" + speaker_wav + "&style_wav=" + style_wav + "&language_id=" + language_id;

    curl = curl_easy_init();
    std::vector<uint8_t> wavData;

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wavData);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }

    assert(wavData.size() > 0);

    auto [header, audioData] = parseWavData(wavData);

    queue_speech((int16_t*)audioData, header.data_size / 2);
}

void InitTextToSpeech()
{
    s_stop = false;

#if !defined(USE_COQUI)
    HRESULT hr = ::CoInitialize(NULL);
    assert(SUCCEEDED(hr));

    hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    assert(SUCCEEDED(hr));
#endif
    s_speaker = std::jthread([]() {
        while (!s_stop) {
            s_phrasesAvailable.acquire();
            VoiceToSpeak voiceToSpeak;
            {
                std::scoped_lock lock(s_phraseMutex);
                if (!s_phrases.empty()) {
                    voiceToSpeak = s_phrases.front();
                    s_phrases.pop_front();
                }
            }
            if (!voiceToSpeak.text.empty()) {
                sayTextInternal(voiceToSpeak);
            }
        }
    });
}

void StopSpeech()
{
    s_stop = true;

    s_phrasesAvailable.release();
}

void SayText(std::string text, int raceNum)
{
#if !defined(USE_COQUI)
    std::wstring wText(text.begin(), text.end());
    pVoice->Speak(wText.c_str(), SPF_ASYNC, NULL);
#else
    {
        std::scoped_lock lock(s_phraseMutex);
        VoiceToSpeak voiceToSpeak;
        voiceToSpeak.text = text;
        switch(raceNum)
        {
            case 9:
                voiceToSpeak.voice = "borg.mp3";
                break;
            default:
                voiceToSpeak.voice = "picard.mp3";
                break;
        }
        
        s_phrases.push_back(voiceToSpeak);
    }
    s_phrasesAvailable.release();
#endif
}