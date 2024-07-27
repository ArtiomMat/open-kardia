#include "../wav.hpp"
#include "../mft.hpp"

#include <audioclient.h>
#include <mmdeviceapi.h>

#include <cstdlib>
#include <cstdio>

#define CHECKR(msgstr) \
  do { if (FAILED(hr)) \
  { \
    throw com::system_ex_t(msgstr); \
  } } while (0)

namespace wav
{
  // struct file_thread_t : mft::thread_t
  // {
  //   void begin()
  //   {
  //     while (1)
  //     {

  //     }
  //   }

  //   void end()
  //   {

  //   }
  // };

  static WAVEFORMATEX wave_format;
  static IMMDeviceEnumerator* enumerator;
  static IMMDevice* device;
  static IAudioClient* audio_client;
  static IAudioRenderClient* render_client;

  static HRESULT hr;

  static UINT32 buf_n;

  // static file_thread_t* file_thread = nullptr;

  void initialize(unsigned buffer_duration)
  {
    if (initialized)
    {
      return;
    }

    hr = CoInitialize(nullptr);
    CHECKR("Initializing COM.");

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    CHECKR("Creating instance.");

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    CHECKR("Retrieving device.");
    
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audio_client);
    CHECKR("Activating device.");

    static WAVEFORMATEX* def_wave_fromat;
    hr = audio_client->GetMixFormat(&def_wave_fromat);
    CHECKR("Retrieving wave format.");

    wave_format = {0};
    wave_format.wFormatTag = WAVE_FORMAT_PCM;
    wave_format.nChannels = CHANNELS_N;
    wave_format.wBitsPerSample = BITS_PER_SAMPLE; // Stutters and shifts the audio of the entire os if bps don't match up(specifically when I did 8 instead of 16). The default wave format(atleast for me) has 32 bps, which apparantly does not work with WAVE_FORMAT_PCM(1)? the format that works is WAVE_FORMAT_EXTENSIBLE(65534), no idea what it even is, don't wanna know, just gonna keep it like this and hope for the best.
    wave_format.nSamplesPerSec = SAMPLE_RATE;
    wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
    wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
    wave_format.cbSize = 0;

    CoTaskMemFree(def_wave_fromat); // Free the default wave format we got

    // Allocate a buffer 
    hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, buffer_duration*10'000, 0, &wave_format, nullptr);
    CHECKR("Initializing audio client.");

    hr = audio_client->GetService(__uuidof(IAudioRenderClient), (void**)&render_client);
    CHECKR("Getting service.");

    hr = audio_client->GetBufferSize(&buf_n);
    CHECKR("Getting buffer size.");

    hr = audio_client->Start();
    CHECKR("Starting the client.");

    // file_thread = new file_thread_t();

    printf("Wave module initialized, buffer duration of %dms.\n", buffer_duration);

    initialized = true;
  }

  void begin_playback()
  {
    UINT32 padding;

    // Padding appears to be how much of the buffer is written
    hr = audio_client->GetCurrentPadding(&padding);
    CHECKR("Getting padding.");
    
    write_samples_n = buf_n - padding;
    
    hr = render_client->GetBuffer(write_samples_n, &samples);
    CHECKR("Getting buffer for rendering.");

    memset(samples, 0, write_samples_n * CHANNELS_N * BITS_PER_SAMPLE/8);
  }

  void end_playback()
  {
    // Multiply each amplitude by master volume.
    for (unsigned i = 0; i < write_samples_n * CHANNELS_N; i++)
    {
      INT16& amp16 = reinterpret_cast<INT16*>(samples)[i];
      amp16 = (INT32)amp16 * volume / 100;
    }

    hr = render_client->ReleaseBuffer(write_samples_n, 0);
    CHECKR("Releasing buffer.");
  }

  void shutdown()
  {
    if (!initialized)
    {
      return;
    }
    audio_client->Stop();

    render_client->Release();
    audio_client->Release();
    device->Release();
    enumerator->Release();

    CoUninitialize();

    puts("Wave module shutdown.");
    initialized = false;
  }
}
