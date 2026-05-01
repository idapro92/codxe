#pragma once
#include <xtl.h>
#include <xaudio2.h>

namespace ngl
{
namespace mp
{

class AudioSystem
{
  public:
    // Manual Init is optional; Play() and PlayTestBeep() call it internally
    static bool Init();

    // Loads from sounds.bin based on audio_config.h metadata
    static void Play(const char *alias);

    // Procedural beep for hardware testing
    static void PlayTestBeep();

    // Releases XAudio2 interfaces and the active playback buffer
    static void Shutdown();

  private:
    static IXAudio2 *g_xa;
    static IXAudio2MasteringVoice *g_master;
    static IXAudio2SourceVoice *g_voice;

    // Buffer for the current sound loaded from the external .bin file
    static unsigned char *g_currentPlaybackBuffer;

    static CRITICAL_SECTION g_audioLock;
    static bool g_initialized;
    static bool g_lockCreated;
};

} // namespace mp
} // namespace ngl