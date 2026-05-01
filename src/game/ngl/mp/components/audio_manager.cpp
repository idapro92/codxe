#include "pch.h"
#include "audio_manager.h"
//#include "audio_data.h"
#include <cmath>
#include <cstring> // For strcmp
#include <cstdio> // For fopen, fseek, fread
#include "audio_config.h"

#pragma comment(lib, "xaudio2.lib")

namespace ngl
{
namespace mp
{

IXAudio2 *AudioSystem::g_xa = nullptr;
IXAudio2MasteringVoice *AudioSystem::g_master = nullptr;
IXAudio2SourceVoice *AudioSystem::g_voice = nullptr;
CRITICAL_SECTION AudioSystem::g_audioLock;
bool AudioSystem::g_initialized = false;
bool AudioSystem::g_lockCreated = false;

bool AudioSystem::Init()
{
    if (g_initialized)
        return true;

    if (!g_lockCreated)
    {
        InitializeCriticalSection(&g_audioLock);
        g_lockCreated = true;
    }

    EnterCriticalSection(&g_audioLock);

    if (FAILED(XAudio2Create(&g_xa, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        LeaveCriticalSection(&g_audioLock);
        return false;
    }

    if (FAILED(g_xa->CreateMasteringVoice(&g_master)))
    {
        LeaveCriticalSection(&g_audioLock);
        return false;
    }

    // Setup for 44.1kHz 16-bit Mono (Matches your Python script output)
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = 2;
    wfx.nAvgBytesPerSec = 44100 * 2;

    if (FAILED(g_xa->CreateSourceVoice(&g_voice, &wfx)))
    {
        LeaveCriticalSection(&g_audioLock);
        return false;
    }

    g_initialized = true;
    LeaveCriticalSection(&g_audioLock);
    return true;
}

unsigned char *AudioSystem::g_currentPlaybackBuffer = nullptr;

void AudioSystem::Play(const char *alias)
{
    if (!alias)
        return;
    if (!g_initialized && !Init())
        return;

    const AudioEntry *entry = nullptr;
    for (int i = 0; g_audioTable[i].name != nullptr; ++i)
    {
        if (strcmp(g_audioTable[i].name, alias) == 0)
        {
            entry = &g_audioTable[i];
            break;
        }
    }

    if (!entry)
        return;

    EnterCriticalSection(&g_audioLock);

    // 1. Clean up previous buffer
    if (g_currentPlaybackBuffer)
    {
        g_voice->Stop(0);
        g_voice->FlushSourceBuffers();
        delete[] g_currentPlaybackBuffer;
        g_currentPlaybackBuffer = nullptr;
    }

    // 2. Load the data from the bin file
    FILE *f = fopen("game:\\sounds.bin", "rb");
    if (f)
    {
        g_currentPlaybackBuffer = new unsigned char[entry->size];
        fseek(f, entry->offset, SEEK_SET);
        fread(g_currentPlaybackBuffer, 1, entry->size, f);
        fclose(f);

        // 3. Play
        XAUDIO2_BUFFER buffer = {0};
        buffer.AudioBytes = entry->size;
        buffer.pAudioData = g_currentPlaybackBuffer;
        buffer.Flags = XAUDIO2_END_OF_STREAM;

        if (SUCCEEDED(g_voice->SubmitSourceBuffer(&buffer)))
        {
            g_voice->Start(0);
        }
    }

    LeaveCriticalSection(&g_audioLock);
}

void AudioSystem::PlayTestBeep()
{
    // 1. Auto-Initialize the system if it hasn't been started yet
    if (!g_initialized)
    {
        if (!Init())
            return;
    }

    // 2. Define the tone parameters
    const int sampleRate = 44100;
    const double duration = 0.5; // half a second
    const double freq = 440.0;   // "Pitch" of the beep (A4 note)
    const int totalSamples = (int)(sampleRate * duration);

    // 3. Generate the sound data (Only runs the first time)
    static short s_beepRaw[22050]; // 44100 * 0.5
    static bool s_generated = false;

    if (!s_generated)
    {
        const double pi = 3.141592653589793;
        for (int i = 0; i < totalSamples; ++i)
        {
            double time = (double)i / sampleRate;
            // Generate 16-bit PCM: Range is -32768 to 32767
            s_beepRaw[i] = (short)(20000 * sin(2.0 * pi * freq * time));
        }
        s_generated = true;
    }

    // 4. Thread-safe hardware submission
    EnterCriticalSection(&g_audioLock);

    if (g_voice)
    {
        // Clear the throat of the audio hardware
        g_voice->Stop(0);
        g_voice->FlushSourceBuffers();

        XAUDIO2_BUFFER buffer = {0};
        buffer.AudioBytes = totalSamples * sizeof(short);
        buffer.pAudioData = (BYTE *)s_beepRaw;
        buffer.Flags = XAUDIO2_END_OF_STREAM; // Tells XAudio2 this is a one-shot sound

        if (SUCCEEDED(g_voice->SubmitSourceBuffer(&buffer)))
        {
            g_voice->Start(0);
        }
    }

    LeaveCriticalSection(&g_audioLock);
}

void AudioSystem::Shutdown()
{
    if (!g_initialized)
        return;
    EnterCriticalSection(&g_audioLock);
    if (g_voice)
    {
        g_voice->DestroyVoice();
        g_voice = nullptr;
    }
    if (g_master)
    {
        g_master->DestroyVoice();
        g_master = nullptr;
    }
    if (g_xa)
    {
        g_xa->Release();
        g_xa = nullptr;
    }
    g_initialized = false;
    LeaveCriticalSection(&g_audioLock);
}

} // namespace mp
} // namespace ngl