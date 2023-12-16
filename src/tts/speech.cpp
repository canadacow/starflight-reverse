#include "speech.h"

#include <assert.h>

#include <sapi.h>

static ISpVoice* pVoice = NULL;

void InitTextToSpeech()
{
    HRESULT hr = ::CoInitialize(NULL);
    assert(SUCCEEDED(hr));

    hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    assert(SUCCEEDED(hr));
}

void SayText(std::string text)
{
    std::wstring wText(text.begin(), text.end());
    pVoice->Speak(wText.c_str(), SPF_ASYNC, NULL);
}