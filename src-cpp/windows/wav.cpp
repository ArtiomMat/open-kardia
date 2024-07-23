#include "../wav.hpp"

#include <audioclient.h>
#include <mmdeviceapi.h>
#include <iostream>

#define CHECKR(msgstr) \
  do { if (FAILED(r)) \
  { \
    throw com::system_ex_t(msgstr); \
  } } while (0)

namespace wav
{
  static BYTE* pData;
  static UINT32 bufferSize;

  WAVEFORMATEX* pwfx;
  IMMDeviceEnumerator* pEnumerator;
  IMMDevice* pDevice;
  IAudioClient* pAudioClient;
  IAudioRenderClient* pRenderClient;

  void initialize()
  {
    HRESULT r;

    r = CoInitialize(nullptr);
    CHECKR("Initializing COM.");


    r = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    CHECKR("Creating instance.");

    r = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    CHECKR("Retrieving device.");
    
    r = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, nullptr, (void**)&pAudioClient);
    CHECKR("Activating device.");

    r = pAudioClient->GetMixFormat(&pwfx);
    CHECKR("Retrieving wave format.");
    
    r = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10'000'000, 0, pwfx, nullptr);
    CHECKR("Initializing audio client.");

    r = pAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&pRenderClient);
    CHECKR("Getting service.");

    r = pAudioClient->GetBufferSize(&bufferSize);
    CHECKR("Getting buffer size.");

    r = pRenderClient->GetBuffer(bufferSize, &pData);
    CHECKR("Getting buffer itself.");

    printf("Wave module initialized, sample rate of %u\n", pwfx->nSamplesPerSec);
  }
}
