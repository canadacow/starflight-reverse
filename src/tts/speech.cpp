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

#pragma comment(lib, "winmm.lib")

struct VoiceToSpeak
{
    std::string text;
    std::string voice;
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

    PlaySound((LPCSTR)wavData.data(), NULL, SND_MEMORY | SND_ASYNC);
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

void SayText(std::string text)
{
#if !defined(USE_COQUI)
    std::wstring wText(text.begin(), text.end());
    pVoice->Speak(wText.c_str(), SPF_ASYNC, NULL);
#else
    {
        std::scoped_lock lock(s_phraseMutex);
        VoiceToSpeak voiceToSpeak;
        voiceToSpeak.text = text;
        voiceToSpeak.voice = "borg.mp3";
        s_phrases.push_back(voiceToSpeak);
    }
    s_phrasesAvailable.release();
#endif
}