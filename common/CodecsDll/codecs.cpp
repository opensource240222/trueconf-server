
#include "Transcoder/AudioCodec.h"
#include "Transcoder/VideoCodec.h"
#include "codecs.h"
#include "../VSClient/VS_ApplicationInfo.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

void* GetAudioCodec(int Id, bool coder)
{
	return VS_RetriveAudioCodec(Id, coder);
}

void* GetVideoCodec(int Id, bool coder)
{
	return VS_RetriveVideoCodec(Id, coder);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
#ifdef _SVKS_M_BUILD_
			VS_RegistryKey::SetDefaultRoot("SVKS-M\\Codecs");
#else
			VS_RegistryKey::SetDefaultRoot("TrueConf\\Codecs");
#endif
            break;
    }
    return TRUE;
}
