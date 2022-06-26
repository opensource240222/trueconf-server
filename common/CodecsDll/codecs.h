
#ifndef VS_CODECS_DLL_H
#define VS_CODECS_DLL_H

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) void* GetAudioCodec(int Id, bool coder);
__declspec(dllexport) void* GetVideoCodec(int Id, bool coder);

#ifdef __cplusplus
}
#endif

#endif /*VS_CODECS_DLL_H*/
