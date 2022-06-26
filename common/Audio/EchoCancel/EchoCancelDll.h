
#ifndef VS_AEC_DLL_H
#define VS_AEC_DLL_H

class VS_EchoCancel
{
public:
	virtual ~VS_EchoCancel() {};
	virtual void Init(int frequency) = 0;
	virtual void Init(int frequency, int num_mic, int num_spk) = 0;
	virtual void Release() = 0;
	virtual void Cancellate(short* far_end, short* near_end, short* echo, int samples) = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) VS_EchoCancel* VS_CreateAEC(int Id);

#ifdef __cplusplus
}
#endif

#endif /* VS_AEC_DLL_H */