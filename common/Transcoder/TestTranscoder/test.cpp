
#include "std/cpplib/VS_RegistryKey.h"

#include "Transcoder/VS_SpeexAudioCodec.h"
#include "Transcoder/VS_iSACAudioCodec.h"
#include "Transcoder/VS_OpusAudioCodec.h"
#include "Transcoder/VS_MP3AudioCodec.h"
#include "Transcoder/VS_AACAudioCodec.h"
#include "Transcoder/VS_IppAudiCodec.h"
#include "std/cpplib/VS_VideoLevelCaps.h"
#include "Audio/EchoCancel/VS_RtcEchoCancellation.h"

#include "Transcoder/VS_OpenH264VideoCodec.h"
#include "Transcoder/VS_OpenH264SlidesVideoCodec.h"
#include "Transcoder/VS_VPXVideoCodec.h"
#include "Transcoder/VS_FFVideoCodec.h"
#include "Transcoder/VS_RetriveVideoCodec.h"

#include "MixerTest.h"

#ifdef _WIN32
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <list>
#include <time.h>

#include "../../streams/Protocol.h"
#include "../VideoCodec.h"
#include "Transcoder/RtpPayload.h"
#include "../VS_RTP_Buffers.h"
#include "../VS_VS_Buffers.h"
#include "../../Video/WinApi/CAviFile.h"
#include "Video/VSVideoProc.h"
#include "VS_AddLogHeadMedia.h"
//#include "../test263/api.h"
//#include "../parser/VS_H263Parser.h"

/// include audio
#include "../../Audio/WinApi/dsutil.h"
#include "../VS_AudioReSampler.h"
#include "VSClient/VSAudioEchoCancel.h"
#include "../VS_IppAudiCodec.h"
#include "../VS_SpeexAudioCodec.h"
#include "../VS_iSACAudioCodec.h"
#include "../VS_OpusAudioCodec.h"
#include "../../Audio/EchoCancel/VS_RtcEchoCancellation.h"
#include "Transcoder/VS_OpenH264VideoCodec.h"
#include "Transcoder/VS_VPXVideoCodec.h"
#include "Transcoder/VS_FFVideoCodec.h"
#include "Transcoder/VS_H264IntelVideoCodec.h"
#include "Transcoder/VS_NvidiaVideoCodec.h"
#include "Transcoder/VS_RetriveVideoCodec.h"
#include "Transcoder/AudioCodecSystem.h"

#include "Transcoder/VS_VPXStereoVideoCodec.h"

#include <boost/make_shared.hpp>

#pragma comment (lib,"glew32s.lib")
#pragma comment (lib,"libmp3lame.lib")

#include "Dump.h"

HINSTANCE VS_LoadCodecsLib()
{
	static HINSTANCE ippCodecExtLib = 0;
	static bool ippCodecExtLibLoaded = false;
	if (ippCodecExtLib == 0 && !ippCodecExtLibLoaded) {
		ippCodecExtLib = LoadLibrary("CodecsDll");
		ippCodecExtLibLoaded = true;
	}
	return ippCodecExtLib;
}

struct h263streamheader {
	unsigned int	FirstDWord;

	unsigned int	unrestricted_motion_vector:1;
	unsigned int	picture_coding_type:1;
	unsigned int	srcformat:3;
	unsigned int	freeze_picture_release:1;
	unsigned int	doccamera:1;
	unsigned int	splitscreen:1;
	unsigned int	LostBits:24;
};

struct h263streamheader_p12bit {
	unsigned int	FirstDWord;
	//--------------------------
	unsigned int	first8:8;
	//--------------------------
	unsigned int	second4:4;
	unsigned int	pct:3;
	unsigned int	ufep:1;
	//--------------------------
	unsigned int	LostBits:16;
};

struct h263streamheader_p30bit {
	unsigned int	FirstDWord;
	//--------------------------
	unsigned int	first8:8;
	//--------------------------
	unsigned int	second7:7;	// +11 bit
	unsigned int	ufep:1;		// +18 bit
	//--------------------------
	unsigned int	third8:8;	//+ 3 bit
	//--------------------------
	unsigned int	LostBits:2;
	unsigned int	pct:3;
	unsigned int	fourth3:3;	//+ 0 bit
};

static HINSTANCE ippCodecExtLib = 0;
static bool ippCodecExtLibLoaded = false;
AudioCodec* VS_RetriveAudioCodec(int Id, bool isCodec)
{
	if (Id == VS_ACODEC_G711a || Id == VS_ACODEC_G711mu ||
		Id == VS_ACODEC_G728 || Id == VS_ACODEC_G729A || Id == VS_ACODEC_G722 ||
		Id == VS_ACODEC_G723 || Id == VS_ACODEC_G7221_24 || Id == VS_ACODEC_G7221_32 ||
		Id == VS_ACODEC_G7221C_24 || Id == VS_ACODEC_G7221C_32 || Id == VS_ACODEC_G7221C_48) {
		return new VS_IppAudiCodec(Id, isCodec);
	} else if (Id==VS_ACODEC_GSM610) {
		if (isCodec)return new VS_AudioCoderGSM610;
		else return new VS_AudioDecoderGSM610;
	} else if (Id==VS_ACODEC_SPEEX) {
		return new VS_SpeexAudioCodec(VS_ACODEC_SPEEX, isCodec);
	} else if (Id==VS_ACODEC_ISAC) {
		return new VS_iSACAudioCodec(VS_ACODEC_ISAC, isCodec);
	} else if (Id==VS_ACODEC_OPUS_B0914) {
		return new VS_OpusAudioCodec(VS_ACODEC_OPUS_B0914, isCodec);
	} else if	(Id==VS_ACODEC_PCM) {
		return new VS_PcmAudioCodec(VS_ACODEC_PCM, isCodec);
	}
	return 0;
}

#define VS_VCODEC_I264			(DWORD)('46hi')

static HINSTANCE ippVCodecExtLib = 0;
static bool ippVCodecExtLibLoaded = false;

load_balancing::BalancingDevice VS_GetTypeDevice(VideoCodec *codec)
{
	return load_balancing::BalancingDevice::software;
}

VS_EchoCancelBase* VS_RetriveEchoCancel(int Id)
{
	if (Id == 0) return new VS_SpeexEchoCancel();
	else if (Id == 1)  return new VS_WebRTCEchoCancel();
	else if (Id == 2)  return new VS_WebRTCFastEchoCancel();
	return 0;
}

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#if defined(_VPX_INCLUDED_) || defined(_H323GATEWAYCLIENT_)
#	pragma comment (lib,"libvpx.lib")
#else

VPXCodec* VS_RetriveVPXCodec(int tag, bool isCoder)
{
	return 0;
}

#endif

/*NVVideoCodec* VS_RetriveNVCodec(int Id, bool isCoder)
{
	return 0;
}*/

namespace load_balancing
{
	int main();
}

static std::vector<uint8_t> CreateVideoFrame(size_t width, size_t height, size_t seed)
{
	std::vector<uint8_t> result(width * height * 3 / 2);

	for (size_t i = 0; i < result.size(); i++)
		result[i] = (i + seed) % 256;

	return result;
}

#include "Transcoder/StbttDrawText.h"

void test_stb()
{
	std::vector<uint8_t> res;
	std::string text = "@#$!@#$\x31\x32\xe2\x98\xbb\x33\xe2\x99\xa5\x35\xe2\x99\xa3\x34\xe2\x99\xa6\xe2\x98\xba\xc2\xa9\xd1\x8b\xd0\xb2\xd0\xb0\x61\x73\x64\x66";

	/*for (int i = 0; i < 100; i++)
		res = StbttDrawText(text, 242, 16);*/

	for (int w = 30; w < 200; w += 20)
	{
		for (int h = 12; h < 20; h += 2)
		{
			_CrtCheckMemory();
			res = StbttDrawText(text, w, h);

			DumpToFile("res_" + std::to_string(w) + "x" + std::to_string(h) + "_.yuv", res);
			_CrtCheckMemory();
		}
	}
}

void test_converts()
{
	VSVideoProcessingIpp p;

	int w = 1280;
	int h = 720;
	auto src = CreateVideoFrame(w, h, 0);
	std::vector<uint8_t> dst(w * h * 3 * 2);

	p.ConvertI420ToBMF24(
		src.data(),
		src.data() + w * h,
		src.data() + w * h * 5 / 4,
		dst.data(),
		w,
		h,
		w
	);

	DumpToFile("src.yuv", src);
	DumpToFile("rgb.rgb", dst);
}

#include <boost/shared_ptr.hpp>

// initially for LifeSize Bridge 2200
int main_siren14_to_wav(int argc, char *argv[])
{
	std::ifstream file("rtpstream.txt");
	std::string line;
	std::vector<std::vector<boost::shared_ptr<RTPPacket>>> rtp_streams;
	std::vector<unsigned char> cur_buf;
	rtp_streams.resize(2);
	int state(0);
	long current_peer(0);
	long cur_pack;

	if (file.is_open())
	{
		while (file.good())
		{
			getline(file, line);
			switch (state)
			{
			case 0:
			{
					  size_t pos = line.find("char peer");
					  if (std::string::npos == pos)
					  {
						  ///state = 2;
						  break;
					  }
					  else
					  {
						  /**
						  get peer
						  */
						  //size_t pos2 = line.find("_");
						  current_peer = strtol(line.c_str() + strlen("char peer"), 0, 10);
						  pos = line.find("_");
						  cur_pack = strtol(line.c_str() + strlen("char peer") + 2, 0, 10);
						  state = 1;
						  /*boost::shared_ptr<RTPPacket> pack(new RTPPacket);

						  rtp_streams[current_peer].insert(rtp_streams[current_peer].end(),pack);*/
						  cur_buf.clear();
					  }
			}
				break;
			case 1:
			{
					  size_t pos = line.find("0x");
					  const char * p = strstr(line.c_str(), "0x");
					  //const char *p = line.c_str();
					  while (p != 0)
					  {
						  long data = strtol(p + 2, 0, 16);
						  cur_buf.insert(cur_buf.end(), (unsigned char)data);
						  //std::cout<<data;
						  p = strstr(p + 1, "0x");
					  }
					  p = strstr(line.c_str(), "};");
					  if (p != 0)
					  {
						  rtp_streams[current_peer].insert(rtp_streams[current_peer].end(), boost::make_shared<RTPPacket>(cur_buf.data(), cur_buf.size(), true));
						  //rtp_streams[current_peer][cur_pack]->SetData(&cur_buf[0],cur_buf.size());
						  state = 0;
					  }
			}
				break;
			case 2:
				break;
			}

		}

	}

	WAVEFORMATEX wf;
	memset(&wf, 0, sizeof(wf));
	wf.nSamplesPerSec = 32000;
	wf.nChannels = 1;
	wf.wBitsPerSample = 16;
	wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.wFormatTag = VS_ACODEC_PCM;
	VS_RTP_InputBufferAudio i_buf((Rtp_PayloadType)123, true); // swap bytes

	AudioCodec *decoder1 = VS_RetriveAudioCodec(VS_ACODEC_G7221C_48, false);
	//AudioCodec *decoder2 = VS_RetriveAudioCodec(VS_ACODEC_G7221C_32, false);
	CWaveFile wav1, wav2;
	decoder1->Init(&wf);
	wav1.Open("wav1.wav", decoder1->GetPCMFormat(), WAVEFILE_WRITE);
	BYTE *outBuff = new BYTE[10000000];
	BYTE *inBuff = new BYTE[1000000];

	for (size_t i = 0; i<rtp_streams[0].size(); i++)
	{
		boost::shared_ptr<RTPPacket> rtp_pack = rtp_streams[0][i];
		int res = i_buf.Add(rtp_pack.get());
		unsigned long sz(0);
		unsigned long v_int(0);
		char key(0);

		while (i_buf.Get(inBuff, sz, v_int, key) >= 0)
		{
			for (unsigned long i = 0; i < sz; i += 2) {
				unsigned char sample = inBuff[i];
				inBuff[i] = inBuff[i + 1];
				inBuff[i + 1] = sample;
			}
			sz = decoder1->Convert(inBuff, outBuff, sz);
			if (sz>0)
			{

				UINT out_sz(0);
				wav1.Write(sz, outBuff, &out_sz);

			}
		}


	}

	std::cout << "end";

	char c;

	std::cin >> c;

	return 0;
}


int main_rtpg7221_to_wav(int argc, char *argv[])
{
	std::ifstream file("rtpstream.txt");
	std::string line;
	std::vector<std::vector<boost::shared_ptr<RTPPacket>>> rtp_streams;
	std::vector<unsigned char> cur_buf;
	rtp_streams.resize(2);
	int state(0);
	long current_peer(0);
	long cur_pack;

	if(file.is_open())
	{
		while(file.good())
		{
			getline(file,line);
			switch(state)
			{
			case 0:
				{
					size_t pos = line.find("char peer");
					if(std::string::npos == pos)
					{
						///state = 2;
						break;
					}
					else
					{
						/**
							get peer
						*/
						//size_t pos2 = line.find("_");
						current_peer = strtol(line.c_str()+strlen("char peer"),0,10);
						pos = line.find("_");
						cur_pack = strtol(line.c_str() +strlen("char peer")+2,0,10);
						state = 1;
						/*boost::shared_ptr<RTPPacket> pack(new RTPPacket);

						rtp_streams[current_peer].insert(rtp_streams[current_peer].end(),pack);*/
						cur_buf.clear();
					}
				}
				break;
			case 1:
				{
					size_t pos = line.find("0x");
					const char * p = strstr(line.c_str(),"0x");
					//const char *p = line.c_str();
					while(p!=0)
					{
						long data = strtol(p+2,0,16);
						cur_buf.insert(cur_buf.end(),(unsigned char)data);
						//std::cout<<data;
						p = strstr(p+1,"0x");
					}
					p = strstr(line.c_str(),"};");
					if(p!=0)
					{
						rtp_streams[current_peer].insert(rtp_streams[current_peer].end(), boost::make_shared<RTPPacket>(cur_buf.data(), cur_buf.size(), true));
						//rtp_streams[current_peer][cur_pack]->SetData(&cur_buf[0],cur_buf.size());
						state = 0;
					}
				}
				break;
			case 2:
				break;
			}

		}

	}

	WAVEFORMATEX wf;
	memset(&wf,0,sizeof(wf));
	wf.nSamplesPerSec = 16000;
	wf.nChannels = 1;
	wf.wBitsPerSample = 16;
	wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.wFormatTag = VS_ACODEC_PCM;
	VS_RTP_InputBufferAudio i_buf((Rtp_PayloadType) 104);

	AudioCodec *decoder1 = VS_RetriveAudioCodec(VS_ACODEC_G7221_32, false);
	AudioCodec *decoder2 = VS_RetriveAudioCodec(VS_ACODEC_G7221_32, false);
	CWaveFile wav1, wav2;
	decoder1->Init(&wf);
	wav1.Open("wav1.wav",decoder1->GetPCMFormat(),WAVEFILE_WRITE);
	BYTE *outBuff = new BYTE[1000000];
	BYTE *inBuff = new BYTE[1000000];

	for(size_t i = 0;i<rtp_streams[1].size();i++)
	{
		boost::shared_ptr<RTPPacket> rtp_pack = rtp_streams[1][i];
		int res = i_buf.Add(rtp_pack.get());
		unsigned long sz(0);
		unsigned long v_int(0);
		char key(0);

		while(i_buf.Get(inBuff,sz,v_int,key)>=0)
		{
			for (unsigned long i = 0; i < sz; i += 2) {
				unsigned char sample = inBuff[i];
				inBuff[i] = inBuff[i+1];
				inBuff[i+1] = sample;
			}
			sz = decoder1->Convert(inBuff,outBuff,sz);
			if(sz>0)
			{

				UINT out_sz(0);
				wav1.Write(sz,outBuff,&out_sz);

			}
		}


	}

	std::cout<<"end";

	char c;

	std::cin>>c;



	return 0;
}

/*
int main1(int argc, char* argv[])
{
	if (argc==1) return -1;
	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	CAviFile avi;
	FILE* file;
	file = fopen("in.263", "wb");
	avi.Init(argv[1]);
	VideoCodec *codec = new VS_VideoCoderIntelH263;
	int ret = 0;
	ret = codec->Init(avi.m_VideoStreamInfo.rcFrame.right, avi.m_VideoStreamInfo.rcFrame.bottom);
	codec->SetBitrate(1000);
	codec->SetCoderOption(IH263_DEF_OPT);
	int size = 0;
	BYTE* data;
	int totaldata = 0;
	int frames = 0;
	VS_RTP_H263OutputBuffer trpout;
	while ((size = avi.ReadDecompressedVideo(data))>=0) {
		if (size) {
			VS_VideoCodecParam prm;
			prm.cmp.FrameSize = 1000;
			prm.cmp.IsKeyFrame = 0;
			prm.cmp.KeyFrame = 0;
			prm.cmp.Quality = 0;
			size = codec->Convert(data, inBuff, &prm);
			if (size) {
//				fwrite(inBuff, 1, size, file);
				trpout.Add(inBuff, size);
				unsigned long len;
				while(trpout.Get(inBuff, len)) {
					RTPPacket rtp((char*)inBuff, len);
					RTPMediaHeader mh(&rtp);
					if (mh.mode == RTPMODE_H263_A) {
						puts("AA ");
					}
					else if (mh.mode == RTPMODE_H263_B) {
						puts("BB ");
					}
				}
			}
		}
	}
	delete codec;
	return 0;
}
*/
// rtp reading
/*int main2(int argc, char* argv[])
{
	if (argc==1) return -1;
	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	char data[4096];

	FILE   *f1 = fopen(argv[1], "rb" );

	CWaveFile in;
	AudioCodec *codec = VS_RetriveAudioCodec(VS_ACODEC_G728, true);
	VideoCodec *VdecH263 = new VS_VideoDecoderIntelH263;
	VideoCodec *VcodH263 = new VS_VideoCoderIntelH263;
	VideoCodec *VcodXCC = 0;
	VideoCodec *VdecXCC = 0;
	WAVEFORMATEX inf;

	inf.wFormatTag = 1;
	inf.nSamplesPerSec = 8000;
	inf.nChannels = 1;

	if (codec->Init(&inf)<0) return -2;

	static size_t maximum = 0;

	CWaveFile out;
	out.Open("out1.wav", codec->IsCoder()?codec->GetCDCFormat():codec->GetPCMFormat(), WAVEFILE_WRITE);
	CAviFile vout1, vout2, vout3, vout4;
	vout1.Init("vout1.avi", true);
	vout2.Init("vout2.avi", true);
	vout3.Init("vout3.avi", true);
	vout4.Init("vout4.avi", true);
	DWORD read = 0;
	RTPMediaHeader mh;
	VS_RTP_H263InputBuffer buff263;
	VS_RTP_H263OutputBuffer rtp_out;
	while (true) {
		VS_AddLogHeadMedia   log1(f1);
		if (!log1.ReadData(f1, data, 4096)) break;
		RTPPacket rtp(data, log1.length);
		mh.Set(&rtp);
		if (rtp.PayloadType()==RTP_PT_G728) {
			int convert = codec->Convert((BYTE*)rtp.Data(), outBuff, rtp.DataSize());
			if (convert>0) {
				UINT wrote = 0;
				out.Write(convert, outBuff, &wrote);
				read = 0;
			}
		}
		else if (rtp.PayloadType()==RTP_PT_H263) {
			if (!VdecH263->IsValid()) {
				int w = 0, h = 0;
				switch(mh.header.h263a.srcformat)
				{
				case 1: w = 128, h = 96; break;
				case 2: w = 176, h = 144; break;
				case 3: w = 352, h = 288; break;
				case 4: w = 704, h = 576; break;
				case 5: w = 1408, h = 1152; break;
				default: w = 352, h = 288; break;
				}
				VdecH263->Init(w, h);
				VcodH263->Init(w, h);
				VdecXCC->Init(w, h);
				VcodXCC->Init(w, h);
				vout1.SetFormat(VdecH263->GetRGBFormat());
				vout2.SetFormat(VcodH263->GetFccFormat());
				vout3.SetFormat(VcodXCC->GetFccFormat());
				vout4.SetFormat(VdecXCC->GetRGBFormat());
				VcodH263->SetBitrate(300);
			}

			printf("adding %zu bytes type of %d \n", rtp.DataSize(), mh.header.h263a.ftype + mh.header.h263a.pbframes);
			if (mh.header.h263a.ftype + mh.header.h263a.pbframes)
				DebugBreak();
			if (maximum < rtp.DataSize())
				maximum = rtp.DataSize();
			buff263.Add(&rtp);
			DWORD h263size;
			unsigned long VideoIntrval;
			char key = 0;
			while (buff263.Get(inBuff, h263size, VideoIntrval, key)>=0) {
				rtp_out.Add(inBuff, h263size);
				VS_VideoCodecParam prm;
				prm.dec.Flags = 0;
				prm.dec.FrameSize = h263size;
				h263streamheader *header = (h263streamheader *)inBuff;
				int convert = VdecH263->Convert(inBuff, outBuff, &prm);
				printf(" ---- size = %ld \n", h263size);
				if (convert>0) {
					vout1.WriteVideo(outBuff, convert);
					prm.cmp.KeyFrame = 0;
					prm.cmp.FrameSize = 1000;
					prm.cmp.Quality = 0;
					prm.cmp.IsKeyFrame = 0;
					convert = VcodH263->Convert(outBuff, inBuff, &prm);
					if (convert>0) {
						vout2.WriteVideo(inBuff, convert, !!prm.cmp.IsKeyFrame);
					}
					else printf("NOT aviwr2 \n");

					prm.cmp.KeyFrame = 0;
					prm.cmp.FrameSize = 1000;
					prm.cmp.Quality = 0;
					prm.cmp.IsKeyFrame = 0;
					convert = VcodXCC->Convert(outBuff, inBuff, &prm);
					if (convert>0) {
						vout3.WriteVideo(inBuff, convert, !!prm.cmp.IsKeyFrame);
						prm.dec.Flags = 0;
						prm.dec.FrameSize = convert;
						convert = VdecXCC->Convert(inBuff, outBuff, &prm);
						if (convert>0) {
							vout4.WriteVideo(outBuff, convert);
						}
						else printf("NOT aviwr4 \n");
					}
					else printf("NOT aviwr3 \n");
				}
				else
					printf("NOT aviwr1 \n");
			}
		}
	}
	delete codec;
	delete VdecH263;
	delete VcodH263;
	delete VdecXCC;
	delete VcodXCC;
	delete[] inBuff;
	delete[] outBuff;
	printf("maximum = %zu", maximum);
	Sleep(2000);
	return 0;
}*/

// audioresempler
int main6(int argc, char* argv[])
{
	if (argc<2) return -1;
	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	CWaveFile wav1, wav2;
	WAVEFORMATEX *wf1 = (WAVEFORMATEX *)malloc(1000);
	WAVEFORMATEX *wf2 = (WAVEFORMATEX *)malloc(1000);
	wav1.Open(argv[1], wf1, WAVEFILE_READ);
	memcpy(wf1, wav1.GetFormat(), sizeof(WAVEFORMATEX));
	memcpy(wf2, wf1, sizeof(WAVEFORMATEX));
	wf2->nSamplesPerSec = 8000;
	wf2->nAvgBytesPerSec = 16000;
	wav2.Open("out1.wav", wf2, WAVEFILE_WRITE);

	VS_AudioReSampler rsmp;
	DWORD sizeread;
	long converted;
	UINT wrote;

	while(1) {
		sizeread = 0;
		wav1.Read(inBuff, 2560*2, &sizeread);
		if (sizeread==0)
			break;
		converted = rsmp.Process(inBuff, outBuff, sizeread, wf1->nSamplesPerSec, wf2->nSamplesPerSec);
		wav2.Write(converted, outBuff, &wrote);
	}

	delete[] inBuff;
	delete[] outBuff;
	return 0;
}

// audiocodec

static const char audio_id2name[12][128] =
{
	"speex",
	"isac",
	"opus_b_0_9_14",
	"g729_a",
	"g722_1_24kbps",
	"g711_a",
	"g711_mu",
	"msgsm_6_10",
	"g723",
	"g728",
	"g722",
};

int main_audio(int argc, char* argv[])
{
	if (argc < 5) {
		printf("app.exe in.wav tc quality complexity\n");
		printf("tc = 0 - speex\n");
		printf("tc = 1 - isac\n");
		printf("tc = 2 - opus beta 0.9.14\n");
		printf("tc = 3 - g.729.a\n");
		printf("tc = 4 - g.722.1 24 kbps\n");
		printf("quality = [0..10] (valid for tc = 0,1,2)\n");
		printf("complexity = [0..10] (valid only for tc = 2)\n");
		return -1;
	}

	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	CWaveFile wav1, wav2;
	WAVEFORMATEX *wf1 = (WAVEFORMATEX *)malloc(1000);
	HRESULT hr = wav1.Open(argv[1], wf1, WAVEFILE_READ);
	if ( !SUCCEEDED(hr) ) {
		printf("Can't open input file\n");
		return -1;
	}
	memcpy(wf1, wav1.GetFormat(), sizeof(WAVEFORMATEX));

	int codecType = 0;
	int q = 0;
	int cmplx = 10;
	if (argc >= 5) {
		codecType = atoi(argv[2]);
		codecType = codecType < 0 ? 0 : (codecType > 6 ? 6 : codecType);
		q = atoi(argv[3]);
		cmplx = atoi(argv[4]);
	}

	timeBeginPeriod(1);

	AudioCodec *cdc = 0,*dcd = 0;
	unsigned int CodecId = 0;
	unsigned int bufferLen = 640;

	if		(codecType==0) CodecId = VS_ACODEC_SPEEX;
	else if (codecType==1) CodecId = VS_ACODEC_ISAC;
	else if (codecType==2) CodecId = VS_ACODEC_OPUS_B0914;
	else if (codecType==3) CodecId = VS_ACODEC_G729A;
	else if (codecType==4) CodecId = VS_ACODEC_G7221_24;
	else if	(codecType==5) CodecId = VS_ACODEC_G711a;
	else if (codecType==6) CodecId = VS_ACODEC_G711mu;
	else if (codecType==7) CodecId = VS_ACODEC_GSM610;
	else if (codecType==8) CodecId = VS_ACODEC_G723;
	else if (codecType==9) CodecId = VS_ACODEC_G728;
	else if (codecType==10) CodecId = VS_ACODEC_G722;

	VS_MediaFormat mf;
	mf.SetAudio(wf1->nSamplesPerSec, CodecId);
	bufferLen = mf.dwAudioBufferLen;

	cdc = VS_RetriveAudioCodec(CodecId, true);
	dcd = VS_RetriveAudioCodec(CodecId, false);
	if (!cdc || !dcd) {
		printf("Can't init audio codec\n");
		return -2;
	}

	cdc->Init(wf1);
	cdc->SetQuality(q);
	cdc->SetComplexity(cmplx);
	dcd->Init(wf1);
	DWORD sizeread = 0, insize, outsize;
	long converted, cnvall = 0;
	wav1.ResetFile();
	insize = outsize = 0;
	unsigned int cdc_time = 0, dcd_time = 0;

	char buff[256] = {0};
	sprintf(buff, "cdc_%s_q%1d.dat", audio_id2name[codecType], q);
	FILE * fout = fopen(buff, "wb");
	sprintf(buff, "dcd_%s_q%1d.wav", audio_id2name[codecType], q);
	wav2.Open(buff, dcd->GetPCMFormat(), WAVEFILE_WRITE);

	UINT wrotebytes = 0;

	while(1) {
		sizeread = 0;
		wav1.Read(inBuff, bufferLen, &sizeread);
		if (sizeread==0) break;
		insize += sizeread;

		unsigned int stime = timeGetTime();
		converted = cdc->Convert(inBuff, outBuff, sizeread);
		cdc_time += timeGetTime() - stime;
		if (converted > 0) {
			fwrite(outBuff, 1, converted, fout);
			cnvall += converted;
		}

		stime = timeGetTime();
		converted = dcd->Convert(outBuff, inBuff, converted);
		dcd_time += timeGetTime() - stime;
		if (converted > 0) wav2.Write(converted, inBuff, &wrotebytes);

		int diffTime = timeGetTime() - stime;
		int playTime = 1000*wf1->nSamplesPerSec/8000*insize/16000;
	}

	float playTime = (float)insize/wf1->nAvgBytesPerSec;
	float bitrate = (float)cnvall * 8.0 / playTime / 1000.0;
	printf("%20s (q = %2d, cmplx = %2d, btr = %3.1f kbps) : playTime = %6.2f s, codingTime = %7.2f ms [ %7.2f times ], decodingTime = %7.2f ms [ %7.2f times ] \n",
			audio_id2name[codecType], q, cmplx, bitrate, playTime, (float)cdc_time, playTime / cdc_time * 1000, (float)dcd_time, playTime / dcd_time * 1000);

	timeEndPeriod(1);
	wav2.Close();
	delete cdc;
	delete dcd;

	delete[] inBuff;
	delete[] outBuff;
	free(wf1);
	return 0;
}

int audio_decoder(int argc, char* argv[])
{
	if (argc < 4) {
		printf("app.exe in.wav tc KHz\n");
		printf("tc = 0 - speex\n");
		printf("tc = 1 - isac\n");
		printf("tc = 2 - opus beta 0.9.14\n");
		printf("tc = 3 - g.729.a\n");
		printf("tc = 4 - g.722.1 24 kbps\n");
		return -1;
	}

	FILE *faudio = fopen(argv[1], "rb");
	if ( !faudio ) {
		printf("Can't open input file\n");
		return -1;
	}
	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	CWaveFile wav;
	WAVEFORMATEX wf;
	memset(&wf, 0, sizeof(WAVEFORMATEX));
	wf.cbSize = sizeof(WAVEFORMATEX);
	wf.nChannels = 1;
	wf.nSamplesPerSec = atoi(argv[3]);
	wf.wBitsPerSample = 16;
	wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.wFormatTag = WAVE_FORMAT_PCM;

	int codecType = 0;
	if (argc >= 4) {
		codecType = atoi(argv[2]);
		codecType = codecType < 0 ? 0 : (codecType > 6 ? 6 : codecType);
	}

	timeBeginPeriod(1);

	AudioCodec *dcd = 0;
	unsigned int CodecId = 0;
	unsigned int bufferLen = 120;

	if		(codecType==0) CodecId = VS_ACODEC_SPEEX;
	else if (codecType==1) CodecId = VS_ACODEC_ISAC;
	else if (codecType==2) CodecId = VS_ACODEC_OPUS_B0914;
	else if (codecType==3) CodecId = VS_ACODEC_G729A;
	else if (codecType==4) CodecId = VS_ACODEC_G7221_24;
	else if	(codecType==5) CodecId = VS_ACODEC_G711a;
	else if (codecType==6) CodecId = VS_ACODEC_G711mu;
	else if (codecType==7) CodecId = VS_ACODEC_GSM610;
	else if (codecType==8) CodecId = VS_ACODEC_G723;
	else if (codecType==9) CodecId = VS_ACODEC_G728;
	else if (codecType==10) CodecId = VS_ACODEC_G722;

	dcd = VS_RetriveAudioCodec(CodecId, false);
	if (!dcd) {
		printf("Can't init audio codec\n");
		return -2;
	}

	dcd->Init(&wf);
	DWORD sizeread = 0, insize, outsize;
	long converted, cnvall = 0;
	insize = outsize = 0;
	unsigned int cdc_time = 0, dcd_time = 0;

	char buff[256] = {0};
	sprintf(buff, "dcd_%s.wav", audio_id2name[codecType]);
	wav.Open(buff, &wf, WAVEFILE_WRITE);

	UINT wrotebytes = 0;
	int enc_size = 0;

	while(fread(&enc_size, 1, sizeof(int), faudio) == sizeof(int)) {
		fread(outBuff, 1, enc_size, faudio);
		converted = dcd->Convert(outBuff, inBuff, enc_size);
		if (converted > 0) wav.Write(converted, inBuff, &wrotebytes);
		enc_size = 0;
	}

	wav.Close();
	delete dcd;
	delete[] inBuff;
	delete[] outBuff;

	return 0;
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
	//CVideoRenderBase::WindowProc(hWnd, msg, wParam, lParam);
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return  DefWindowProc(hWnd, msg, wParam, lParam);
}

bool CreateMainWindow(HWND &hWnd)
{
    WNDCLASS wc =
    {
        CS_CLASSDC | CS_HREDRAW | CS_VREDRAW,
        MsgProc,
        0L,
        0L,
        NULL,
        LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL,
        "ClassWindow",
    };
	if (!RegisterClass(&wc)) return false;
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = 10;
	rect.bottom = 10;
	AdjustWindowRect(&rect, WS_POPUP | WS_SIZEBOX, TRUE);
    hWnd = CreateWindow(
				"ClassWindow",
                "NameWindow",
                WS_POPUP | WS_SIZEBOX,
				0, 0, 10, 10,
                NULL,
                NULL,
                NULL,
                NULL);
    if (!hWnd) return false;
	//ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
    return true;
}

#include "../../VSClient/VSAudio.h"
#include "../../VSClient/VSAudioDs.h"
#include "../../VSClient/VSAudioNew.h"

// audiocodec
int main12(int argc, char* argv[])
{
	HWND hwnd;
	HANDLE handles;
	CoInitialize(0);

	if (argc<2) return -1;
	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	CWaveFile wav1, wav2;
	WAVEFORMATEX *wf1 = (WAVEFORMATEX *)malloc(1000);
	int codecType = 0;
	if (argc>=4) {
		codecType = atoi(argv[3]);
		codecType = codecType < 0 ? 0 : (codecType > 8? 8 : codecType);
	}

	if (!CreateMainWindow(hwnd))
		return -2;
	VS_AudioDeviceManager::Open(hwnd);
	VS_MediaFormat fmt;
	VS_ACaptureDevice *din = new VS_ACaptureDevice;

	timeBeginPeriod(1);

	AudioCodec *cdc = 0,*dcd = 0;
	int CodecId = 0;
	if		(codecType==0) CodecId = VS_ACODEC_G711a;
	else if (codecType==1) CodecId = VS_ACODEC_G711mu;
	else if (codecType==2) CodecId = VS_ACODEC_GSM610;
	else if (codecType==3) CodecId = VS_ACODEC_G723;
	else if (codecType==4) CodecId = VS_ACODEC_G728;
	else if (codecType==5) CodecId = VS_ACODEC_G729A;
	else if (codecType==6) CodecId = VS_ACODEC_G7221_24;
	else if (codecType==7) CodecId = VS_ACODEC_G722;
	else if (codecType==8) CodecId = VS_ACODEC_SPEEX;
	cdc = VS_RetriveAudioCodec(CodecId, true);
	dcd = VS_RetriveAudioCodec(CodecId, false);
	if (!cdc || !dcd)
		return 0;

	wf1->nChannels = 1;
	wf1->wFormatTag = WAVE_FORMAT_PCM;
	wf1->nSamplesPerSec = 16000;
	wf1->wBitsPerSample = 16;
	wf1->nBlockAlign = wf1->nChannels * wf1->wBitsPerSample;
	wf1->nAvgBytesPerSec = wf1->nSamplesPerSec * wf1->nBlockAlign;
	wf1->cbSize = 0;
	cdc->Init(wf1);
	dcd->Init(wf1);
	DWORD sizeread = 0, insize, outsize;
	long converted;
	int len = 0;
	unsigned int wrotebytes = 0;
	VS_AudioReSampler	resmp;

	fmt.SetAudio(16000, VS_ACODEC_PCM, 100);
	din->Init(-1, &fmt);
	handles = din->GetCmpleteEvent();
	din->SetVolume(0xffff);
	din->Start();

	FILE * fout = fopen("out.cmp", "wb+");
	FILE * fin =  fopen(argv[1], "wb+");
	wav2.Open(argv[2], dcd->GetPCMFormat(), WAVEFILE_WRITE);

	SetEvent(handles);

	while (1) {
		while (din->Capture((char*)inBuff, len, false)>0) {
			fwrite(inBuff, 1, len, fin);
			converted = cdc->Convert(inBuff, outBuff, len);
			fwrite(outBuff, 1, converted, fout);
			converted = dcd->Convert(outBuff, inBuff, converted);
			if (converted > 0)
				wav2.Write(converted, inBuff, &wrotebytes);
		}
		if (/*_kbhit() && */ std::getchar() == 'q') break;
	}

	fclose(fout);
	timeEndPeriod(1);
	wav2.Close();
	delete cdc;
	delete dcd;

	delete[] inBuff;
	delete[] outBuff;
	free(wf1);

	VS_AudioDeviceManager::Close();
	DestroyWindow(hwnd);
	CoUninitialize();

	return 0;
}

/*#include "ippdefs.h"
typedef enum {
		NAL_UT_RESERVED	 = 0x00, // Reserved
		NAL_UT_SLICE	 = 0x01, // Coded Slice - slice_layer_no_partioning_rbsp
		NAL_UT_DPA		 = 0x02, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_DPB		 = 0x03, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_DPC		 = 0x04, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_IDR_SLICE = 0x05, // Coded Slice of a IDR Picture - slice_layer_no_partioning_rbsp
		NAL_UT_SEI		 = 0x06, // Supplemental Enhancement Information - sei_rbsp
		NAL_UT_SPS		 = 0x07, // Sequence Parameter Set - seq_parameter_set_rbsp
		NAL_UT_PPS		 = 0x08, // Picture Parameter Set - pic_parameter_set_rbsp
		NAL_UT_PD		 = 0x09, // Picture Delimiter - pic_delimiter_rbsp
		NAL_UT_FD		 = 0x0a  // Filler Data - filler_data_rbsp
} NAL_Unit_Type;


Ipp32u EndOfNAL(BYTE* in, int insize, Ipp8u* const pout, Ipp8u const uIDC, NAL_Unit_Type const uUnitType)
{
	Ipp32u size, ExtraBytes;
	Ipp8u*	curPtr, *endPtr, *outPtr;

	// get current RBSP compressed size
	size = insize;
	ExtraBytes = 0;

	// Set Pointers
	endPtr = in + size - 1;	// Point at Last byte with data in it.
	curPtr = in;
	outPtr = pout;

	// Write Start Codes, and NAL Header byte

	if ((uUnitType >= NAL_UT_SEI) &&
		(uUnitType <= NAL_UT_PD)) {
		*outPtr++ = 0;	// Write an Extra zero_byte
		ExtraBytes = 1;
	}

	*outPtr++ = 0;
	*outPtr++ = 0;
	*outPtr++ = 1;
	*outPtr++ = (uIDC << 5) | uUnitType;
	ExtraBytes += 4;

	while (curPtr < endPtr-1) {	// Copy all but the last 2 bytes
		*outPtr++ = *curPtr;

		// Check for start code emulation
		if ((*curPtr++ == 0) && (*curPtr == 0) && (!(*(curPtr+1) & 0xfc))) {
			*outPtr++ = *curPtr++;
			*outPtr++ = 0x03;	// Emulation Prevention Byte
			ExtraBytes++;
		}
	}

	if (curPtr < endPtr) {
		*outPtr++ = *curPtr++;
	}
	// copy the last byte
	*outPtr = *curPtr;

	// copy encoded frame to output
	return(size+ExtraBytes);

} // CH263pBs::End*/

//FF_VideoCodec::FF_VideoCodec(FF_VCodecID CocecId, bool IsCoder){};
//FF_VideoCodec::~FF_VideoCodec(){};
//int FF_VideoCodec::Init(int w, int h, int bitrate){return 0;};
//void FF_VideoCodec::Release(){};
//int FF_VideoCodec::Convert(unsigned char *in, unsigned char* out, int* param){return 0;};

int main9(int argc, char* argv[])
{
	if (argc<3) {
		return -1;
	}
	BYTE *outBuff = new BYTE[1000000];
	BYTE *inBuff = new BYTE[1000000];
	CAviFile aviin, aviout;

	//VS_VideoCoderXc02 vcodec;
	//VS_VideoDecoderXc02 decoder;
	if (!aviin.Init(argv[1]))
		return -2;
	if (!aviout.Init(argv[2], true))
		return -2;

	BITMAPINFOHEADER *bmin = 0, bmout;
	int size = 0;
	if ((size = aviin.GetFormat(bmin))>0) {
		bmin = (BITMAPINFOHEADER *)malloc(size);
		aviin.GetFormat(bmin);
	}
	else
		return -3;

	//if (vcodec.Init(bmin->biWidth, bmin->biHeight))
	//	return -5;
	//if (decoder.Init(bmin->biWidth, bmin->biHeight))
	//	return -6;

	//bmout = *vcodec.GetFccFormat();
	int vsize = 0;
	int asize = 0;
	VS_VideoCodecParam param;
	aviout.m_fps = aviin.m_fps;
	//aviout.SetFormat(decoder.GetRGBFormat());

	BYTE* data;
	bool First = true;
	VS_VS_InputBuffer in;
	VS_VS_OutputBuffer out;
	VS_VideoProc proc;
	srand(timeGetTime());
	bool lost = false;

	while ((vsize = aviin.ReadDecompressedVideo(data))>0) {
		BYTE *pY = inBuff;
		BYTE *pU = pY+ bmin->biWidth*bmin->biHeight;
		BYTE *pV = pU+ bmin->biWidth*bmin->biHeight/4;
		//if (vcodec.GetRGBFormat()->biCompression==BI_RGB)
		//	memcpy(inBuff, data, bmin->biWidth*3*bmin->biHeight);
		//else
		//	proc.ConvertBMF24ToI420(data, pY, pU, pV, bmin->biWidth*3, bmin->biHeight, bmin->biWidth);
		param.cmp.FrameSize = 0;
		param.cmp.IsKeyFrame =0;
		param.cmp.KeyFrame =First;
		param.cmp.Quality =0;
		int compressed = 0;//vcodec.Convert(inBuff, outBuff, &param);
		if (compressed > 0) {
			unsigned long size, Videointerval=100;
			stream::Track track;
			BYTE key = param.cmp.IsKeyFrame!=0;
			if (out.Add(outBuff, compressed, stream::Track::video, Videointerval, key))
				while (out.Get(inBuff, size, track)) {
					if (rand()%63==0) {
						First = false;
						lost = true;
						puts("LOST");
						continue;
					}
					if (in.Add(inBuff, size, track)) {
						while (in.Get(inBuff, size, track, Videointerval)>=0) {
							param.dec.FrameSize = size;
							//if ((compressed = decoder.Convert(inBuff, outBuff, &param)) > 0) {
							//	aviout.WriteVideo(outBuff, compressed);
							//	First = false;
							//}
						}
					}
					else {
						if (lost) {
							if (rand()%7) First = true;
							lost = false;
						}
					}
				}
		}
	}
	delete[] inBuff;
	delete[] outBuff;
	return 0;
}

#include "Transcoder/VSAudioVad.h"
// audioVAD
int main10(int argc, char* argv[])
{
	if (argc<2) return -1;
	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	CWaveFile wav1, wav2;
	WAVEFORMATEX *wf1 = (WAVEFORMATEX *)malloc(1000);
	WAVEFORMATEX *wf2 = (WAVEFORMATEX *)malloc(1000);
	wav1.Open(argv[1], wf1, WAVEFILE_READ);
	memcpy(wf1, wav1.GetFormat(), sizeof(WAVEFORMATEX));
	memcpy(wf2, wf1, sizeof(WAVEFORMATEX));
	wav2.Open("vad.wav", wf2, WAVEFILE_WRITE);

	VSAudioVAD vad;
	vad.Init(wf1->nSamplesPerSec, wf1->wBitsPerSample);
	DWORD sizeread;
	UINT wrote;
	const int fr_size = wf1->nSamplesPerSec*20/1000;// 40 ms
	memset(outBuff, 0, fr_size*2);

	while(1) {
		sizeread = 0;
		wav1.Read(inBuff, fr_size*2, &sizeread);
		if (sizeread==0)
			break;
		if (vad.IsVad(inBuff, sizeread))
			wav2.Write(sizeread, inBuff, &wrote);
		else
			wav2.Write(sizeread, outBuff, &wrote);
	}

	delete[] inBuff;
	delete[] outBuff;
	return 0;
}


int main_echo(int argc, char* argv[])
{
	int i = 0, res = 0;
	float j = 0.0;

	LARGE_INTEGER st_t, end_t, all_t, fr;
	QueryPerformanceFrequency(&fr);
	all_t.QuadPart = 0;

	if (argc < 4) return -1;

	int offset = atoi(argv[1]);
	int freq = 16000;// atoi(argv[1]);
	int size_block = (freq == 11025) ? 160 : 320;

	CWaveFile *m_inf = new CWaveFile;
	CWaveFile *m_outf = new CWaveFile;
	CWaveFile *m_compf = new CWaveFile;

	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = freq * 2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;

	//for (j = -40.0; j <= 20.0; j += 1.0) {
	char str[256] = { 0 };
	sprintf(str, "e%s", argv[2]);
	m_inf->Open(argv[2], &wf, WAVEFILE_READ);
	m_outf->Open(argv[3], &wf, WAVEFILE_READ);
	m_compf->Open(argv[4], &wf, WAVEFILE_WRITE);

	unsigned char buff_far[1280 * 4], buff_near[1280 * 4], buff_echo[1280 * 4];

	VS_WebRTCEchoCancel	m_ec;
	m_ec.Init(freq);
	m_ec.Init(freq, 1, 1);

	UINT szwrote = 0;
	UINT szwrite = size_block * 2;
	DWORD szread = 0;
	int NN = 0;
	int sh = ((int)(((40 + j) * freq) / 1000.0 + 0.5)) * 2;

	//offset = -offset;

	if (offset < 0)
		m_outf->Read((BYTE*)buff_near, -offset * (freq / 1000) * sizeof(short), &szread);

	offset = offset * (freq / 1000) * sizeof(short);

	szread = 0;
	//m_inf->Read((BYTE*)buff_near, sh, &szread);
	//if (!(szread == 0 && sh != 0)) {
	//szread = 0;
	//m_outf->Read((BYTE*)buff_far, (int)((40.0 * freq) / 1000.0 + 0.5) * 2, &szread);
	//if (szread) {
	while (1) {
		szread = 0;
		m_inf->Read((BYTE*)buff_near, szwrite, &szread);
		if (szread == 0) break;
		szread = 0;

		if (offset > 0)
		{
			if (offset < szwrite)
				m_outf->Read((BYTE*)buff_far + offset, szwrite - offset, &szread);

			szread = 1;
			offset -= szwrite;
		}
		else
			m_outf->Read((BYTE*)buff_far, szwrite, &szread);


		if (szread == 0) break;
		QueryPerformanceCounter(&st_t);
		m_ec.Cancellate((short*)buff_far, (short*)buff_near, (short*)buff_echo, size_block);
		QueryPerformanceCounter(&end_t);
		all_t.QuadPart += end_t.QuadPart - st_t.QuadPart;
		szwrote = 0;
		m_compf->Write(szwrite, (BYTE*)buff_echo, &szwrote);
		NN++;
	}
	//}
	//}

	if (m_inf) m_inf->Close();
	if (m_outf) m_outf->Close();
	if (m_compf) m_compf->Close();
	//}

	delete m_inf;
	delete m_outf;
	delete m_compf;

	printf("%8.2f ms\n", (all_t.QuadPart * 1000.0) / fr.QuadPart);

	return 0;
}

// rtp binary
int main11(int argc, char* argv[])
{
	if (argc==1)
		return -1;
	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	int psize = 0;

	FILE   *f1 = fopen(argv[1], "rb" );
	if (!f1)
		return -1;

	WAVEFORMATEX inf;
	inf.wFormatTag = 1;
	inf.nSamplesPerSec = 8000;
	inf.nChannels = 1;

	AudioCodec *codec = VS_RetriveAudioCodec(VS_ACODEC_G723, false);
	if (codec->Init(&inf)<0)
		return -2;

	CWaveFile out;
	out.Open("out.wav", codec->GetPCMFormat(), WAVEFILE_WRITE);
	RTPMediaHeader mh;
	while (fread(&psize, 4, 1, f1)==1 && fread(inBuff, psize, 1, f1)>0) {
		RTPPacket rtp(inBuff, psize);
		mh.Set(&rtp);
		int convert = codec->Convert((BYTE*)rtp.Data(), outBuff, rtp.DataSize());
		if (convert>0) {
			UINT wrote = 0;
			out.Write(convert, outBuff, &wrote);
		}
	}
	fclose(f1);
	out.Close();
	delete codec;
	delete[] inBuff;
	delete[] outBuff;
	return 0;
}

int main13(int argc, char* argv[])
{
	FILE *f = fopen(argv[1], "rt");
	long d = 0, a = 0;
	char s[250] = {0};
	char b[250] = {0};
	while (fscanf(f, "CONFS: RemoveConference_Event %08s@ca1.vzochat.com#as, cause = %ld\n", s, &a)!=EOF) {
		d = strtoul(s, 0, 16);
		printf("%s; %ld\n", s, d);
	}
	return 0;
}

/// app.exe vp80|h264|i264|xc02 -i in.avi -o out.avi [options]
/// -b  bitrate = kbps
/// -r	RC type = 0, 1
/// -hw_enc		= hardware encoder, only for i264
/// -hw_dec		= hardware decoder, only for i264
/// -p			= presets, vpx, i264 = [0,1,2]
/// -q			= quality, vpx = [0...16], i264 = [0, 1]
/// -ec			= error resilient, only for vpx
/// -t			= threads number
/// -svc		= only for vpx
/// -test		= only encoding for speed test

#include "CalculatePsnr.h"

int main_video_decode(int argc, char* argv[])
{
	FILE *fvideo = fopen(argv[2], "rb");
	if ( !fvideo ) {
		printf("Can't open input file\n");
		return -1;
	}
	FILE *fvideo_orig = fopen(argv[1], "rb");
	if ( !fvideo_orig ) {
		printf("Can't open input file\n");
		return -1;
	}
	int width = atoi(argv[4]);
	int height = atoi(argv[5]);
	int fps = atoi(argv[6]);
	int write = atoi(argv[7]);
	BYTE *inBuff = new BYTE[1000000], *outBuff = new BYTE[1000000];
	VideoCodec *dcd = VS_RetriveVideoCodec(VS_VCODEC_VPX, false);
	if (dcd == 0) return -1;
	int res = dcd->Init(width, height, FOURCC_I420);
	int enc_size = 0;
	int frames = 0;
	CAviFile aviout;
	BITMAPINFOHEADER bmin;
	memset(&bmin, 0, sizeof(BITMAPINFOHEADER));
	bmin.biSize = sizeof(BITMAPINFOHEADER);
	bmin.biCompression = '024I';
	bmin.biBitCount = 12;
	bmin.biWidth = width;
	bmin.biHeight = height;
	bmin.biSizeImage = width * height * 3 / 2;
	char name[128] = {0};
	aviout.Init(argv[3], true);
	aviout.m_fps = fps;
	aviout.SetFormat(&bmin);

	VS_VideoCodecParam param;
	tVideoMetrics vm, vm_avg, vm_max;
	memset(&vm_avg, 0, sizeof(tVideoMetrics));
	memset(&vm_max, 0, sizeof(tVideoMetrics));
	memset(&vm, 0, sizeof(tVideoMetrics));

	while(fread(&enc_size, 1, sizeof(int), fvideo) == sizeof(int)) {
		fread(inBuff, 1, enc_size, fvideo);
		param.dec.FrameSize = enc_size;
		int converted = dcd->Convert(inBuff, outBuff, &param);
		if (converted > 0) {
			if (write) aviout.WriteVideo(outBuff, converted);
			fread(inBuff, 1, bmin.biSizeImage, fvideo_orig);
			CalculatePSNR(inBuff, outBuff, width, height, &vm, &vm_max, &vm_avg);
			frames++;
		}
		enc_size = 0;
	}

	FILE *fpsnr = fopen("psnr.log", "a");
	double avgPSNR_Y = 10.0 * log10(255.0 * 255.0 * frames / vm_avg.Y_YUV);
	double avgPSNR_U = 10.0 * log10(255.0 * 255.0 * frames / vm_avg.U_YUV);
	double avgPSNR_V = 10.0 * log10(255.0 * 255.0 * frames / vm_avg.V_YUV);
	fprintf(fpsnr, "\n  %15s %30s : %9.6f, %9.6f, %9.6f", argv[1], argv[2], avgPSNR_Y, avgPSNR_U, avgPSNR_V);
	fclose(fpsnr);

	delete dcd;
	delete [] inBuff;
	delete [] outBuff;

	fclose(fvideo);
	fclose(fvideo_orig);

	return 0;
}

#include "Transcoder/VS_VideoCodecManager.h"

int main_video_recompress_svc(int argc, char* argv[])
{
	FILE *fvideo = fopen(argv[1], "rb");
	if (!fvideo) {
		printf("Can't open input file\n");
		return -1;
	}

	int w_in = atoi(argv[3]);
	int h_in = atoi(argv[4]);
	int w_out = atoi(argv[5]);
	int h_out = atoi(argv[6]);
	int fps = atoi(argv[7]);
	int bitrate = atoi(argv[8]);
	int start_frame = atoi(argv[9]);

	BYTE *inBuff = new BYTE[20000000], *outBuff = new BYTE[20000000];
	VS_MediaFormat mf;
	mf.SetVideo(w_out, h_out, VS_VCODEC_VPX, fps, 0, 0x00070100, 0);

	VS_VideoCodecManager *mng = new VS_VideoCodecManager();
	/// !!! in VS_VPXVideoCodec::Init set par.cpu_used = 0; !!!
	bool res = mng->Init(&mf, 78);
	if (res) {
		mng->SetBitrate(bitrate, 5000, fps * 100);

		int enc_size = w_in * h_in * 3 / 2;
		int frames = 0, frames_first = 442;
		CAviFile aviout;
		BITMAPINFOHEADER bmin;
		memset(&bmin, 0, sizeof(BITMAPINFOHEADER));
		bmin.biSize = sizeof(BITMAPINFOHEADER);
		bmin.biCompression = '08PV';
		bmin.biBitCount = 12;
		bmin.biWidth = w_out;
		bmin.biHeight = h_out;
		bmin.biSizeImage = w_out * h_out * 3 / 2;
		char name[128] = { 0 };
		aviout.Init(argv[2], true);
		aviout.m_fps = fps;
		aviout.SetFormat(&bmin);

		VS_VideoProc proc;

		while (fread(inBuff, 1, enc_size, fvideo) == enc_size) {
			if (frames >= start_frame) {
				bool key(false);
				proc.ResampleI420(inBuff, outBuff, w_in, h_in, w_out, h_out);
				if (frames % 200 == 0) {
					key = true;
				}
				int converted = mng->Convert(outBuff, inBuff, &key);
				if (converted > 0) {
					aviout.WriteVideo(inBuff, converted, key);
				}
			}
			frames++;
		}

	}

	delete mng;
	delete[] inBuff;
	delete[] outBuff;

	fclose(fvideo);

	return 0;
}

int main_video_recompress(int argc, char* argv[])
{
	FILE *fvideo = fopen(argv[1], "rb");
	if ( !fvideo ) {
		printf("Can't open input file\n");
		return -1;
	}

	int w_in = atoi(argv[3]);
	int h_in = atoi(argv[4]);
	int w_out = atoi(argv[5]);
	int h_out = atoi(argv[6]);
	int fps = atoi(argv[7]);
	int bitrate = atoi(argv[8]);

	BYTE *inBuff = new BYTE[10000000], *outBuff = new BYTE[10000000];
	VideoCodec *cdc = VS_RetriveVideoCodec(VS_VCODEC_VPX, true);
	if (cdc == 0) return -1;
	int res = cdc->Init(w_out, h_out, FOURCC_I420, 78, 1, fps);

	vpx_param par;
	memset(&par, 0, sizeof(vpx_param));
	par.width = w_out;
	par.height = h_out;
	par.i_maxinterval = 1000;
	par.bitrate = bitrate;
	par.rate_control_method = 1;
	par.deadline = 1;
	par.cpu_used = 0;
	par.me_static_threshold = 800;
	par.error_resilient = 0;
	par.ref_frames = 1;
	res = (cdc->SetCoderOption(&par)) ? 0 : 1;
	uint32_t svc_mode = 0x00030000;
	res = (cdc->SetSVCMode(svc_mode)) ? 0 : 1;

	int enc_size = w_in * h_in * 3 / 2;
	int frames = 0, frames_first = 442;
	CAviFile aviout;
	BITMAPINFOHEADER bmin;
	memset(&bmin, 0, sizeof(BITMAPINFOHEADER));
	bmin.biSize = sizeof(BITMAPINFOHEADER);
	bmin.biCompression = '08PV';
	bmin.biBitCount = 12;
	bmin.biWidth = w_out;
	bmin.biHeight = h_out;
	bmin.biSizeImage = w_out * h_out * 3 / 2;
	char name[128] = {0};
	aviout.Init(argv[2], true);
	aviout.m_fps = fps;
	aviout.SetFormat(&bmin);

	VS_VideoProc proc;

	VS_VideoCodecParam param;
	param.cmp.FrameSize = 0;
	param.cmp.IsKeyFrame = 0;
	param.cmp.KeyFrame = 1;
	param.cmp.Quality = 0;

	while(fread(inBuff, 1, enc_size, fvideo) == enc_size) {
		if (frames >= 8) {
			proc.ResampleI420(inBuff, outBuff, w_in, h_in, w_out, h_out);
			if (frames % 450 == 0) {
				param.cmp.KeyFrame = 1;
			}
			int converted = cdc->Convert(outBuff, inBuff, &param);
			if (converted > 0) {
				aviout.WriteVideo(inBuff, converted, (param.cmp.IsKeyFrame == 1));
			}
			param.cmp.KeyFrame = 0;
		}
		frames++;
	}

	delete cdc;
	delete [] inBuff;
	delete [] outBuff;

	fclose(fvideo);

	return 0;
}

int main_video(int argc, char* argv[])
{

	int i = 0;
	unsigned int fourcc = VS_VCODEC_VPX;
	unsigned int cmp_fourcc;
	unsigned int bitrate = 512;
	unsigned int svc_mode = 0;
	int num_spatial_layers = 1;
	int cdrop = 0;
	unsigned int hw_enc = 0;
	unsigned int hw_dec = 0;
	int type_rc = 0;
	int ec = 0;
	int num_threads = 0;
	int preset = 0;
	int quality = 0;
	char nameInFile[128] = { 0 };
	char nameOutFile[128] = { 0 };
	bool bEncoderPerformance = false;
	bool bPSNR = false;
	/// parse input
	if (argc < 6) return -1;
	for (i = 1; i < argc; i++) {
		if (!argv[i]) return -1;
		if ('-' != argv[i][0]) {
			/// type codec
			if (0 == strcmp(argv[i], "h264")) {
				fourcc = VS_VCODEC_H264;
				cmp_fourcc = '462H';
			}
			else if (0 == strcmp(argv[i], "i264")) {
				fourcc = VS_VCODEC_I264;
				cmp_fourcc = '462H';
			}
			else if (0 == strcmp(argv[i], "vp80")) {
				fourcc = VS_VCODEC_VPX;
				cmp_fourcc = '08PV';
			}
			else if (0 == strcmp(argv[i], "xc02")) {
				fourcc = VS_VCODEC_XC02;
				cmp_fourcc = '20CX';
			}
			else if (0 == strcmp(argv[i], "h261")) {
				fourcc = VS_VCODEC_H261;
				cmp_fourcc = '162H';
			}
			else if (0 == strcmp(argv[i], "h263")) {
				fourcc = VS_VCODEC_H263;
				cmp_fourcc = '362H';
			}
			else if (0 == strcmp(argv[i], "h263p")) {
				fourcc = VS_VCODEC_H263P;
				cmp_fourcc = '362H';
			}
			else if (0 == strcmp(argv[i], "mpeg4")) {
				fourcc = VS_VCODEC_MPEG4;
				cmp_fourcc = '4PMF';
			}
			else {
				return -1;
			}
			continue;
		}
		if (0 == strcmp(argv[i], "-hw_enc")) {
			hw_enc = 1;
		}
		else if (0 == strcmp(argv[i], "-hw_dec")) {
			hw_dec = 1;
		}
		else if (0 == strcmp(argv[i], "-ec")) {
			ec = 1;
		}
		else if (0 == strcmp(argv[i], "-test")) {
			bEncoderPerformance = true;
		}
		else if (0 == strcmp(argv[i], "-psnr")) {
			bPSNR = true;
		}
		else {
			/// if 1 character option
			char ch = argv[i][1];
			i++;
			switch (ch)
			{
			case 'i':
				strcpy(nameInFile, argv[i]);
				break;
			case 'o':
				strcpy(nameOutFile, argv[i]);
				break;
			case 'b':
				bitrate = atoi(argv[i]);
				break;
			case 'r':
				type_rc = atoi(argv[i]);
				break;
			case 't':
				num_threads = atoi(argv[i]);
				break;
			case 'q':
				quality = atoi(argv[i]);
				break;
			case 'p':
				preset = atoi(argv[i]);
				break;
			case 's':
				svc_mode = atoi(argv[i]);
				break;
			default:
				break;
			}
		}
	}
	///

	int res = 0;
	CAviFile aviin, aviout, avicmp;
	CAviFile aviout_svc[10], avicmp_svc[10];

	if (!aviin.Init(nameInFile))
	{
		return -2;
	}

	if (svc_mode >= 5) {
		/*
		/// spatial svc
		num_spatial_layers = 3;
		for (i = 0; i < num_spatial_layers; i++) {
		sprintf(outn[i], "l%d_%s", 2 - i, argv[3]);
		if (!aviout[i].Init(outn[i], true))
		return -2;
		sprintf(cmpn[i], "cmp_l%d_%s", 2 - i, argv[3]);
		if (!avicmp[i].Init(cmpn[i], true))
		return -2;
		}
		*/
	}
	else {
		char cmpn[128];
		if (!aviout.Init(nameOutFile, true))
		{
			return -2;
		}
		sprintf(cmpn, "cmp_%s", nameOutFile);
		if (!avicmp.Init(cmpn, true))
		{
			return -2;
		}
	}

	BITMAPINFOHEADER *bmin = 0, bmout, bmcmp;

	int size = 0;
	if ((size = aviin.GetFormat(bmin)) > 0) {
		bmin = (BITMAPINFOHEADER *)malloc(size);
		aviin.GetFormat(bmin);
	}
	else {
		return -3;
	}

	bool bDecoding = (bmin->biCompression == '462H') || (bmin->biCompression == '08PV') || (bmin->biCompression == '20CX') || (bmin->biCompression == '162H') || (bmin->biCompression == '362H');

	if (bDecoding) {
		if (!aviin.Init(nameInFile, false)) return -2;
	}

	VideoCodec *cdc = 0, *dcd = 0;
	VideoCodec *dcd_svc[10] = { 0 };

	switch (fourcc)
	{
	case VS_VCODEC_H261:
		cdc = VS_RetriveVideoCodec(VS_VCODEC_H261, true);
		dcd = VS_RetriveVideoCodec(VS_VCODEC_H261, false);
		break;
	case VS_VCODEC_H263:
		cdc = VS_RetriveVideoCodec(VS_VCODEC_H263, true);
		dcd = VS_RetriveVideoCodec(VS_VCODEC_H263, false);
		break;
	case VS_VCODEC_H263P:
		cdc = VS_RetriveVideoCodec(VS_VCODEC_H263P, true);
		dcd = VS_RetriveVideoCodec(VS_VCODEC_H263P, false);
		break;
	case VS_VCODEC_H264:
		cdc = VS_RetriveVideoCodec(VS_VCODEC_H264, true);
		dcd = VS_RetriveVideoCodec(VS_VCODEC_H264, false);
		break;
	case VS_VCODEC_I264:
		cdc = VS_RetriveVideoCodec(VS_VCODEC_I264, true, hw_enc, 0);
		if (hw_dec > 0) dcd = VS_RetriveVideoCodec(VS_VCODEC_I264, false, hw_dec, 0);
		else dcd = VS_RetriveVideoCodec(VS_VCODEC_H264, false);
		break;
	case VS_VCODEC_VPX:
		cdc = VS_RetriveVideoCodec(VS_VCODEC_VPX, true);
		dcd = VS_RetriveVideoCodec(VS_VCODEC_VPX, false);
		if (svc_mode >= 5) {
			for (i = 1; i < num_spatial_layers; i++) dcd_svc[i] = VS_RetriveVideoCodec(VS_VCODEC_VPX, false);
		}
		break;
	case VS_VCODEC_XC02:
		cdc = VS_RetriveVideoCodec(VS_VCODEC_XC02, true);
		dcd = VS_RetriveVideoCodec(VS_VCODEC_XC02, false);
		break;
	case VS_VCODEC_MPEG4:
		cdc = VS_RetriveVideoCodec(VS_VCODEC_MPEG4, true);
		dcd = VS_RetriveVideoCodec(VS_VCODEC_MPEG4, false);
		break;
	}
	if (cdc == 0 || dcd == 0) return -1;

	if (!bDecoding) res = cdc->Init(bmin->biWidth, bmin->biHeight, FOURCC_I420, 0, num_threads, aviin.m_fps);
	res = dcd->Init(bmin->biWidth, bmin->biHeight, FOURCC_I420);
	if (svc_mode >= 5) {
		for (i = 1; i < num_spatial_layers; i++) res = dcd_svc[i]->Init(bmin->biWidth >> i, bmin->biHeight >> i, FOURCC_I420);
	}

	switch (fourcc)
	{
	case VS_VCODEC_H261:
	case VS_VCODEC_H263:
	case VS_VCODEC_H263P:
	case VS_VCODEC_H264:
	case VS_VCODEC_XC02:
	case VS_VCODEC_MPEG4:
		res = (cdc->SetBitrate(bitrate)) ? 0 : 1;
		break;
	case VS_VCODEC_I264:
	{
		if (!bDecoding) {
			h264_Param h264Par;
			memset(&h264Par, 0, sizeof(h264_Param));
			h264Par.bitrate = bitrate;
			h264Par.me_quality = preset;
			h264Par.entropy_coding_mode = quality;
			res = (cdc->SetCoderOption(&h264Par)) ? 0 : 1;
		}
		break;
	}
	case VS_VCODEC_VPX:
	{
		vpx_param par;
		memset(&par, 0, sizeof(vpx_param));
		par.i_maxinterval = 1000;
		par.bitrate = bitrate;
		par.rate_control_method = 1;
		par.deadline = preset;
		par.cpu_used = quality;
		par.me_static_threshold = 800;
		par.error_resilient = ec;
		par.ref_frames = 1;
		res = (cdc->SetCoderOption(&par)) ? 0 : 1;
		par.svc_mode = svc_mode;
		if (par.svc_mode > 0) {
			uint32_t mode = 0x00000100;
			switch (par.svc_mode) {
			case 1: mode = 0x00000100; break; /// 2-layers, 3-frame period, not vp8 method
			case 3: mode = 0x00000200; break; /// 3-layers, 4-frame period, not vp8 method
			case 2: mode = 0x00000400; break; /// 3-layers, 4-frame period
			case 4: mode = 0x00000800; break; /// 2-layers, 3-frame period
			case 5: mode = 0x00010100; break; /// spatial + 2-layers, 3-frame period, not vp8 method
			}
			cdc->SetSVCMode(mode);
		}
		break;
	}
	}

	if (res == 0) {
		/// Loop processing
		aviin.Reset();

		tVideoMetrics vm, vm_avg, vm_max;
		memset(&vm_avg, 0, sizeof(tVideoMetrics));
		memset(&vm_max, 0, sizeof(tVideoMetrics));
		memset(&vm, 0, sizeof(tVideoMetrics));

		BYTE *outBuff = new BYTE[30000000];
		BYTE *inBuff = new BYTE[30000000];
		BYTE *data;

		LARGE_INTEGER frequency, start_t, end_t, all_time, time_convert_to, time_convert_from;
		QueryPerformanceFrequency(&frequency);
		all_time.QuadPart = 0;
		time_convert_to.QuadPart = time_convert_from.QuadPart = 0;

		VS_VideoCodecParam param;
		bool First = true;
		int vsize = 0, count = 0;
		int compressed_all = 0;
		int num_dropable = 0;
		int compressed = 0;

		std::string statJson = R"({ "out_file" : ")" + std::string(nameOutFile) + R"(")";

		if (!bDecoding) {
			bmout = *bmin;
			bmout.biCompression = '024I';
			bmout.biBitCount = 12;
			bmout.biSizeImage = bmin->biHeight * bmin->biWidth * 3 / 2;
			bmcmp = *bmin;
			bmcmp.biCompression = cmp_fourcc;
			aviout.m_fps = aviin.m_fps;
			aviout.SetFormat(&bmout);
			avicmp.m_fps = aviin.m_fps;
			avicmp.SetFormat(&bmcmp);
			if (svc_mode >= 5) {
				for (i = 1; i < num_spatial_layers; i++) {
					/// open files , set formats for avi
					/*
					sprintf(nb[i], "fsize_l%d_%s.txt", 2 - i, argv[3]);
					fsize[i] =  fopen(nb[i], "w");
					bmout.biWidth /= 2;
					bmout.biHeight /= 2;
					bmout.biSizeImage /= 4;
					avicmp[i].m_fps = aviin.m_fps;
					avicmp[i].SetFormat(&bmcmp);
					bmcmp.biWidth /= 2;
					bmcmp.biHeight /= 2;
					*/
				}
			}

			char nameTestCmp[1024] = { 0 };
			sprintf(nameTestCmp, "test_%s.cmp", nameOutFile);
			FILE *tempCmp = fopen(nameTestCmp, "wb");
			/// encoding
			while ((vsize = aviin.ReadVideo(inBuff, bmin->biSizeImage)) > 0) {
				param.cmp.FrameSize = 0;
				param.cmp.IsKeyFrame = 0;
				param.cmp.KeyFrame = First;
				param.cmp.Quality = 0;

				/*if (count == 500)
					cdc->SetBitrate(bitrate * 2);

				if (count == 1000)
					cdc->SetBitrate(bitrate * 0.2);*/

				QueryPerformanceCounter(&start_t);
				compressed = cdc->Convert(inBuff, outBuff, &param);   ////////////////////////////////////////////////////////////////////////////
				QueryPerformanceCounter(&end_t);
				time_convert_to.QuadPart += (end_t.QuadPart - start_t.QuadPart);

				std::cout << compressed << std::endl;

				compressed_all += compressed;

				if (compressed >= 0) {
					if (svc_mode == 0) {
						fwrite(&compressed, sizeof(int), 1, tempCmp);
						fwrite(outBuff, compressed, 1, tempCmp);
						avicmp.WriteVideo(outBuff, compressed, (param.cmp.IsKeyFrame == 1));
						First = false;
					}
					else {
						/// svc encode
						compressed -= 8;
						fwrite(&compressed, sizeof(int), 1, tempCmp);
						fwrite(outBuff + 4, compressed, 1, tempCmp);
						avicmp.WriteVideo(outBuff + 4, compressed, (param.cmp.IsKeyFrame == 1));
						First = false;
						/*
						unsigned char *pOut = outBuff;
						for (i = 0; i < num_spatial_layers; i++) {
						//int cmp_size = *(int*)pOut;
						int cmp_size = compressed;
						if (par.svc_mode >= 5) fprintf(fsize[i], "%d\n", cmp_size);
						avicmp[i].WriteVideo(pOut + 4, cmp_size - 8, (param.cmp.IsKeyFrame == 1));
						param.dec.FrameSize = cmp_size - 8;
						//avicmp[i].WriteVideo(pOut, cmp_size - 4, (param.cmp.IsKeyFrame == 1));
						//param.dec.FrameSize = cmp_size - 4;
						QueryPerformanceCounter(&start_t);
						compressed = dcd[i]->Convert(pOut + 4, inBuff, &param);
						//compressed = dcd[i]->Convert(pOut, inBuff, &param);
						QueryPerformanceCounter(&end_t);
						time_convert_from.QuadPart += (end_t.QuadPart - start_t.QuadPart);
						if (compressed >= 0) {
						aviout[i].WriteVideo(inBuff, compressed);
						}
						pOut += cmp_size;
						}
						First = false;
						*/
					}
				}
				count++;
			}

			fclose(tempCmp);

			if (!bEncoderPerformance) {
				FILE *fstat = 0;
				FILE *fpsnr = 0;
				if (bPSNR) {
					char nb[1024];
					sprintf(nb, "_stat_%s.txt", nameOutFile);
					fstat = fopen(nb, "w");
					fpsnr = fopen("_result_calc_psnr.csv", "a");
					aviin.Reset();
				}
				int frames = 0;
				/// decoding
				tempCmp = fopen(nameTestCmp, "rb");
				//if (svc_mode == 0) {
				for (;;) {
					int sr = fread(&compressed, 1, sizeof(int), tempCmp);
					if (sr != sizeof(int))
						break;
					sr = fread(inBuff, 1, compressed, tempCmp);
					if (sr != compressed)
						break;
					param.dec.FrameSize = compressed;
					QueryPerformanceCounter(&start_t);
					int decompressed = dcd->Convert(inBuff, outBuff, &param);
					QueryPerformanceCounter(&end_t);
					time_convert_from.QuadPart += (end_t.QuadPart - start_t.QuadPart);
					if (decompressed >= 0) {
						aviout.WriteVideo(outBuff, decompressed);
						if (bPSNR) {
							vsize = aviin.ReadVideo(inBuff, bmin->biSizeImage);
							CalculatePSNR(inBuff, outBuff, bmin->biWidth, bmin->biHeight, &vm, &vm_max, &vm_avg);
							fprintf(fstat, "\n%5d, %7d, %9.6f, %9.6f, %9.6f", frames, compressed, vm.Y_YUV, vm.U_YUV, vm.V_YUV);
						}
					}
					frames++;
				}
				//} else {
				//
				//}

				if (bPSNR) {
					double avgPSNR_Y = 10.0 * log10(255.0 * 255.0 * frames / vm_avg.Y_YUV);
					double avgPSNR_U = 10.0 * log10(255.0 * 255.0 * frames / vm_avg.U_YUV);
					double avgPSNR_V = 10.0 * log10(255.0 * 255.0 * frames / vm_avg.V_YUV);
					fprintf(fstat, "\n  AVG, %7d, %9.6f, %9.6f, %9.6f", (int)((double)compressed_all / (double)frames), avgPSNR_Y, avgPSNR_U, avgPSNR_V);
					fprintf(fpsnr, "\n  %15s %30s : %9.6f, %9.6f, %9.6f", nameInFile, nameOutFile, avgPSNR_Y, avgPSNR_U, avgPSNR_V);
					fclose(fstat);
					fclose(fpsnr);

					statJson += R"(, "psnr" : )" + std::to_string(avgPSNR_Y);
				}

				fclose(tempCmp);
			}
		}
		else {
			bmout = *bmin;
			bmout.biCompression = '024I';
			bmout.biBitCount = 12;
			bmout.biSizeImage = bmin->biHeight * bmin->biWidth * 3 / 2;
			aviout.m_fps = aviin.m_fps;
			aviout.SetFormat(&bmout);

			/// only decoding
			while ((vsize = aviin.ReadVideo(inBuff, bmout.biSizeImage)) > 0) {
				param.dec.FrameSize = vsize;
				QueryPerformanceCounter(&start_t);
				compressed = dcd->Convert(inBuff, outBuff, &param);
				QueryPerformanceCounter(&end_t);
				time_convert_from.QuadPart += (end_t.QuadPart - start_t.QuadPart);
				if (compressed >= 0) {
					if (!bEncoderPerformance) aviout.WriteVideo(outBuff, compressed);
				}
				count++;
			}
		}

		double time_enc = time_convert_to.QuadPart / double(frequency.QuadPart),
			time_dec = time_convert_from.QuadPart / double(frequency.QuadPart);

		char buff[128];
		sprintf(buff, "%s_perfomance.txt", argv[1]);
		FILE *f = fopen(buff, "a");
		fprintf(f, "%s : %4d x%4d, %3d fps, %5d frames, %5d kbps [ %9d bytes, %5d kbps ], encoder [ t = %9.2f ms, fps = %5.2f ], decoder [ t = %9.2f ms, fps = %5.2f ]\n",
			nameOutFile, (int)bmin->biWidth, (int)bmin->biHeight, (int)(aviin.m_fps), count, bitrate, compressed_all,
			(int)((compressed_all * aviin.m_fps * 8.0) / (count * 1024.0)),
			time_enc * 1000.0, double(count) / time_enc, time_dec * 1000.0, double(count) / time_dec);
		fclose(f);

		statJson += R"(, "w" : )" + std::to_string(bmin->biWidth);
		statJson += R"(, "h" : )" + std::to_string(bmin->biHeight);

		statJson += R"(, "fps" : )" + std::to_string((int)(aviin.m_fps));
		statJson += R"(, "frames" : )" + std::to_string(count);
		statJson += R"(, "exp_brate" : )" + std::to_string(bitrate);
		statJson += R"(, "bytes" : )" + std::to_string(compressed_all);
		statJson += R"(, "res_brate" : )" + std::to_string((int)((compressed_all * aviin.m_fps * 8.0) / (count * 1024.0)));
		statJson += R"(, "encoder_t" : )" + std::to_string(time_enc * 1000.0);
		statJson += R"(, "encoder_fps" : )" + std::to_string(double(count) / time_enc);
		statJson += R"(, "decoder_t" : )" + std::to_string(time_dec * 1000.0);
		statJson += R"(, "decoder_fps" : )" + std::to_string(double(count) / time_dec);

		statJson += R"( })";

		std::string statJsonFileName = std::string(argv[1]) + "_perfomance.json";

		std::ofstream statJsonFile(statJsonFileName.c_str(), std::ios::app | std::ios::ate);

		statJsonFile << statJson << std::endl;

		delete[] outBuff;
		delete[] inBuff;
	}

	delete cdc;
	delete dcd;
	for (i = 1; i < num_spatial_layers; i++) delete dcd_svc[i];
	if (bmin) free(bmin);



	////for (int cntCycle = 0; cntCycle < 10; cntCycle++) {
	//	aviin.Reset();
	//	if (res == 0) {

	//		VS_VideoProc proc, procinv;

	//		BYTE *outBuff = new BYTE[10000000];
	//		BYTE *inBuff = new BYTE[10000000];
	//		BYTE *inBuffNV12 = new BYTE[10000000];
	//		BYTE* data;

	//		bmout = *cdc->GetFccFormat();
	//		bmout.biCompression = '024I';
	//		bmout.biBitCount = 12;
	//		bmout.biSizeImage = bmin->biHeight * bmin->biWidth * 3 / 2;
	//		bmcmp = *cdc->GetFccFormat();
	//		bmcmp.biCompression = cmp_fourcc;
	//		//bmcmp.biCompression = '024I';
	//		//bmout.biBitCount = 12;
	//		//bmcmp.biSizeImage = bmin->biHeight * bmin->biWidth * 3 / 2;

	//		FILE *fsize[4];
	//		char nb[4][1024];
	//		sprintf(nb[3], "fsize_%s.txt", argv[3]);
	//		fsize[3] =  fopen(nb[3], "w");

	//		for (i = 0; i < num_spatial_layers; i++) {
	//			if (par.svc_mode >= 5) {
	//				sprintf(nb[i], "fsize_l%d_%s.txt", 2 - i, argv[3]);
	//				fsize[i] =  fopen(nb[i], "w");
	//			}
	//			aviout[i].m_fps = aviin.m_fps;
	//			aviout[i].SetFormat(&bmout);
	//			bmout.biWidth /= 2;
	//			bmout.biHeight /= 2;
	//			bmout.biSizeImage /= 4;
	//			avicmp[i].m_fps = aviin.m_fps;
	//			avicmp[i].SetFormat(&bmcmp);
	//			bmcmp.biWidth /= 2;
	//			bmcmp.biHeight /= 2;
	//			//bmcmp.biSizeImage /= 4;
	//		}

	//		VS_VideoCodecParam param;
	//		bool First = true;
	//		int vsize = 0, count = 0;
	//		int compressed_all = 0;
	//		int num_dropable = 0;

	//		LARGE_INTEGER frequency, start_t, end_t, all_time, time_convert_to, time_convert_from;
	//		QueryPerformanceFrequency(&frequency);
	//		all_time.QuadPart = 0;
	//		time_convert_to.QuadPart = time_convert_from.QuadPart = 0;

	//		//while ((vsize = aviin.ReadDecompressedVideo(data))>0) {
	//		while ((vsize = aviin.ReadVideo(inBuff, bmin->biSizeImage))>0) {
	//		//while ((vsize = aviin.ReadVideo(inBuff, 0x1000000))>0) {
	//			/*
	//			BYTE *pY = inBuff;
	//			BYTE *pU = pY+ bmin->biWidth*bmin->biHeight;
	//			BYTE *pV = pU+ bmin->biWidth*bmin->biHeight/4;

	//			proc.ConvertBMF24ToI420(data, pY, pU, pV, bmin->biWidth*3, bmin->biHeight, bmin->biWidth);
	//			*/

	//			if (type_codec == '462i') {
	//
	//			}

	//			param.cmp.FrameSize = 0;
	//			param.cmp.IsKeyFrame =0;
	//			param.cmp.KeyFrame = First;
	//			param.cmp.Quality = 0;

	//			QueryPerformanceCounter(&start_t);
	//			int compressed = cdc->Convert(inBuff, outBuff, &param);
	//			QueryPerformanceCounter(&end_t);
	//			time_convert_to.QuadPart += (end_t.QuadPart - start_t.QuadPart);

	//				compressed_all += compressed;
	//			fprintf(fsize[3], "%d\n", compressed);

	//			if (compressed >= 0) {
	//				if (par.svc_mode == 0) {
	//					avicmp[0].WriteVideo(outBuff, compressed, (param.cmp.IsKeyFrame == 1));
	//					param.dec.FrameSize = compressed;
	//					QueryPerformanceCounter(&start_t);
	//					compressed = dcd[0]->Convert(outBuff, inBuff, &param);
	//					QueryPerformanceCounter(&end_t);
	//					time_convert_from.QuadPart += (end_t.QuadPart - start_t.QuadPart);
	//					if (compressed >= 0) {
	//						aviout[0].WriteVideo(inBuff, compressed);
	//						//First = false;
	//					}
	//					First = false;
	//				} else {
	//					unsigned char *pOut = outBuff;
	//					for (i = 0; i < num_spatial_layers; i++) {
	//						//int cmp_size = *(int*)pOut;
	//						int cmp_size = compressed;
	//						if (par.svc_mode >= 5) fprintf(fsize[i], "%d\n", cmp_size);
	//						avicmp[i].WriteVideo(pOut + 4, cmp_size - 8, (param.cmp.IsKeyFrame == 1));
	//						param.dec.FrameSize = cmp_size - 8;
	//						//avicmp[i].WriteVideo(pOut, cmp_size - 4, (param.cmp.IsKeyFrame == 1));
	//						//param.dec.FrameSize = cmp_size - 4;
	//						QueryPerformanceCounter(&start_t);
	//						compressed = dcd[i]->Convert(pOut + 4, inBuff, &param);
	//						//compressed = dcd[i]->Convert(pOut, inBuff, &param);
	//						QueryPerformanceCounter(&end_t);
	//						time_convert_from.QuadPart += (end_t.QuadPart - start_t.QuadPart);
	//						if (compressed >= 0) {
	//							aviout[i].WriteVideo(inBuff, compressed);
	//						}
	//						pOut += cmp_size;
	//					}
	//					First = false;
	//				}
	//			}
	//			count++;
	//		}

	//		if (par.svc_mode >= 5) for (i = 0; i < num_spatial_layers; i++) fclose(fsize[i]);
	//		fclose(fsize[3]);

	//		double time_enc = time_convert_to.QuadPart / double(frequency.QuadPart),
	//			   time_dec = time_convert_from.QuadPart / double(frequency.QuadPart);

	//		char buff[128];
	//		sprintf(buff, "%s_perfomance.txt", argv[1]);
	//		FILE *f = fopen(buff, "a");
	//		fprintf(f, "%s : %4d x%4d, %3d fps, %5d frames, %5d kbps [ %9d bytes, %5d kbps ], encoder [ t = %9.2f ms, fps = %5.2f ], decoder [ t = %9.2f ms, fps = %5.2f ]\n",
	//				argv[3], bmin->biWidth, bmin->biHeight, (int)(aviin.m_fps), count, bitrate, compressed_all,
	//				(int)((compressed_all * aviin.m_fps * 8.0) / (count * 1024.0)),
	//				time_enc * 1000.0, double(count) / time_enc, time_dec * 1000.0, double(count) / time_dec);
	//		fclose(f);

	//		delete [] outBuff;
	//		delete [] inBuff;
	//		delete [] inBuffNV12;
	//	}
	////}

	//delete cdc;
	//for (i = 0; i < num_spatial_layers; i++) delete dcd[i];

	return 0;
}

#include "../VS_MP3AudioCodec.h"
#include "../../Transcoder/VSVideoFile.h"
#include "../../Transcoder/VSVideoFileReader.h"
#include "../../Transcoder/VSVideoFileWriter.h"

int main_mp3(int argc, char* argv[])
{
	VSVideoFileWriter aviOut;
	AudioCodec* encoder = new VS_MP3AudioCodec(VS_ACODEC_MP3, true);
	AudioCodec* decoder = new VS_MP3AudioCodec(VS_ACODEC_MP3, false);

	WAVEFORMATEX wfEnc;
	WAVEFORMATEX wfDec;
	VSVideoFile::SAudioInfo audioInfo;

	wfEnc.cbSize = 0;
	wfEnc.nChannels = 1;
	wfEnc.nSamplesPerSec = 16000;
	wfEnc.wBitsPerSample = 16;
	wfEnc.wFormatTag = VS_ACODEC_MP3;

	wfDec.cbSize = 0;
	wfDec.nChannels = 1;
	wfDec.nSamplesPerSec = 16000;
	wfDec.wBitsPerSample = 16;
	wfDec.wFormatTag = VS_ACODEC_MP3;

	audioInfo.BitsPerSample = 16;
	audioInfo.CodecID = VSVideoFile::ACODEC_ID_MP3;
	audioInfo.NumChannels = 1;
	audioInfo.SampleRate = 16000;

	encoder->Init(&wfEnc);
	decoder->Init(&wfDec);

	aviOut.Init("asdf.avi");

	aviOut.SetAudioFormat(audioInfo);

	aviOut.WriteHeader();

	std::ifstream pcmFile("ymca_16.pcm", std::ifstream::binary);
	std::ofstream mp3File("mp3_16_b32_out.mp3", std::ifstream::binary);
	std::ofstream pcmOut("pcm_16_b32_out.pcm", std::ifstream::binary);
	std::ofstream logFile("log.txt");

	int err;
	int readSize;
	int encodedSize;
	int decodedSize;

	const int numSamples = 100;
	const int sampleRate = 16000;
	const int mp3BufSize = numSamples * 3 / 2 + 7200;

	short* inBuf = new short[numSamples];
	short* outBuf = new short[mp3BufSize];
	unsigned char* mp3Buf = new unsigned char[mp3BufSize];

	logFile << "begin" << std::endl;

	while (!pcmFile.read((char*)inBuf, numSamples * sizeof(short)).eof())
	{
		readSize = pcmFile.gcount();

		logFile << "readSize " << readSize << std::endl;

		encodedSize = encoder->Convert((BYTE*)inBuf, mp3Buf, readSize);

		logFile << "encodedSize " << encodedSize << std::endl;

		if (encodedSize)
		{
			mp3File.write((char*)mp3Buf, encodedSize);
			aviOut.WriteAudioSamples((char*)mp3Buf, encodedSize);

			decodedSize = decoder->Convert(mp3Buf, (BYTE*)outBuf, encodedSize);

			logFile << "decodedSize " << decodedSize << std::endl;

			if (decodedSize)
			{
				pcmOut.write((char*)outBuf, decodedSize);
			}
		}
	}

	aviOut.Release();

	encoder->Release();
	decoder->Release();

	pcmFile.close();
	mp3File.close();
	pcmOut.close();

	delete inBuf;
	delete outBuf;
	delete mp3Buf;

	return 0;
}

int main_mkv(int argc, char* argv[])
{
	VSVideoFileReader inFile;
	VSVideoFileWriter outFile;
	VSVideoFile::SVideoInfo inVideoInfo;
	VSVideoFile::SVideoInfo outVideoInfo;

	std::string inFileName = "C:\\TrueConf\\Recordings\\0000008a_2017-07-28_20-06-10.mkv";
	std::string outFileName = "out.avi";

	if (!inFile.Init(inFileName))
		return -1;

	if (!outFile.Init(outFileName))
		return -1;

	if (!inFile.GetVideoFormat(inVideoInfo))
		return -1;

	outVideoInfo = inVideoInfo;

	outVideoInfo.CodecID = VSVideoFile::VCODEC_ID_VP8;

	if (!outFile.SetVideoFormat(outVideoInfo))
		return -1;

	if (!outFile.WriteHeader())
		return -1;

	char* inBuffer = new char[inVideoInfo.Width * inVideoInfo.Height * 3 / 2];
	char* outBuffer = new char[inVideoInfo.Width * inVideoInfo.Height * 3 / 2];

	int readedSize = 0;
	size_t frameCount = 0;
	size_t totalCompressedSize = 0;
	size_t converted = 0;
	bool isKey = false;

	while ((readedSize = inFile.ReadVideo(inBuffer, &isKey)) > 0)
	{
		if (!outFile.WriteVideo(inBuffer, readedSize, isKey))
			return -1;
	}

	outFile.Release();

	return 0;
}

std::string CodecNameById(VSVideoFile::EVideoCodecID id)
{
	std::map<VSVideoFile::EVideoCodecID, std::string> m = {
		{ VSVideoFile::VCODEC_ID_H263, "H263" },
		{ VSVideoFile::VCODEC_ID_H263P, "H263P" },
		{ VSVideoFile::VCODEC_ID_H264, "H264" },
		{ VSVideoFile::VCODEC_ID_VP8, "VP8" },
		{ VSVideoFile::VCODEC_ID_MPEG4, "MPEG4" },
		{ VSVideoFile::VCODEC_ID_RAWVIDEO, "RAWVIDEO" }
	};

	return m[id];
}

std::string CodecNameById(VSVideoFile::EAudioCodecID id)
{
	std::map<VSVideoFile::EAudioCodecID, std::string> m = {
		{ VSVideoFile::ACODEC_ID_AAC, "AAC" },
		{ VSVideoFile::ACODEC_ID_MP3, "MP3" },
		{ VSVideoFile::ACODEC_ID_OPUS, "OPUS" },
		{ VSVideoFile::ACODEC_ID_PCM, "PCM" }
	};

	return m[id];
}

int main_CVideoFile_test(int argc, char* argv[])
{
	std::vector<std::string> formats = { "avi", "mkv", "mp4" };

	std::vector<VSVideoFile::EVideoCodecID> videoCodecs= {
		VSVideoFile::VCODEC_ID_H263,
		VSVideoFile::VCODEC_ID_H263P,
		VSVideoFile::VCODEC_ID_H264,
		VSVideoFile::VCODEC_ID_VP8,
		VSVideoFile::VCODEC_ID_MPEG4,
		VSVideoFile::VCODEC_ID_RAWVIDEO
	};

	std::vector<VSVideoFile::EAudioCodecID> audioCodecs = {
		VSVideoFile::ACODEC_ID_AAC,
		VSVideoFile::ACODEC_ID_MP3,
		VSVideoFile::ACODEC_ID_OPUS,
		VSVideoFile::ACODEC_ID_PCM
	};

	for (const std::string& f : formats)
	{
		for (VSVideoFile::EVideoCodecID v : videoCodecs)
		{
			for (VSVideoFile::EAudioCodecID a : audioCodecs)
			{
				VSVideoFileWriter file;
				VSVideoFile::SVideoInfo videoInfo;
				VSVideoFile::SAudioInfo audioInfo;

				std::cout << "file.Init(\"file." + f + ", true)";

				if (file.Init("file." + f))
					std::cout << " - ok" << std::endl;
				else
					std::cout << " - failed!" << std::endl;

				videoInfo.CodecID = v;
				videoInfo.FPS = 15;
				videoInfo.Width = 704;
				videoInfo.Height = 576;
				videoInfo.PixFormat = VSVideoFile::PF_YUV420;

				std::cout << "file.SetVideoFormat, vcodec : " + CodecNameById(v);

				if (file.SetVideoFormat(videoInfo))
					std::cout << " - ok" << std::endl;
				else
					std::cout << " - failed!" << std::endl;

				audioInfo.CodecID = a;
				audioInfo.BitsPerSample = 16;
				audioInfo.NumChannels = 1;
				audioInfo.SampleRate = 16000;

				std::cout << "file.SetAudioFormat, acodec : " + CodecNameById(a);

				if (file.SetAudioFormat(audioInfo))
					std::cout << " - ok" << std::endl;
				else
					std::cout << " - failed!" << std::endl;
			}
		}
	}

	return 0;
}

#include <chrono>

#include "../VS_AACAudioCodec.h"

int main_aac(int argc, char* argv[])
{
	std::ifstream pcmInFile;
	std::ofstream compressedFile;
	std::ofstream pcmOutFile;
	std::ofstream logFile;

	std::string pcmInFileName;
	std::string compressedFileName;
	std::string pcmOutFileName;
	std::string logFileName;

	AudioCodec* encoder;
	AudioCodec* decoder;
	std::string codecName;

	WAVEFORMATEX wfEnc;
	WAVEFORMATEX wfDec;

	wfEnc.cbSize = 0;
	wfEnc.nChannels = 1;
	wfEnc.nSamplesPerSec = 16000;
	wfEnc.wBitsPerSample = 16;

	wfDec.cbSize = 0;
	wfDec.nChannels = 1;
	wfDec.nSamplesPerSec = 16000;
	wfDec.wBitsPerSample = 16;

	/**
	*	Parse command line
	*/

	for (int i = 1; i < argc; i++)
	{
		std::string command = argv[i];

		if (command == "-i")
		{
			pcmInFileName = argv[i + 1];
			i++;
		}
		else if (command == "-o")
		{
			pcmOutFileName = argv[i + 1];
			i++;
		}
		else if (command == "-c")
		{
			codecName = argv[i + 1];
			i++;
		}
		else if (command == "-l")
		{
			logFileName = argv[i + 1];
			i++;
		}
	}

	compressedFileName = pcmOutFileName + "." + codecName;

	/**
	*	Create encoder and decoder
	*/

	if (codecName == "aac")
	{
		encoder = new VS_AACAudioCodec(VS_ACODEC_AAC, true);
		decoder = new VS_AACAudioCodec(VS_ACODEC_AAC, false);
		wfEnc.wFormatTag = VS_ACODEC_AAC;
		wfDec.wFormatTag = VS_ACODEC_AAC;
	}
	else if (codecName == "mp3")
	{
		encoder = new VS_MP3AudioCodec(VS_ACODEC_MP3, true);
		decoder = new VS_MP3AudioCodec(VS_ACODEC_MP3, false);
		wfEnc.wFormatTag = VS_ACODEC_MP3;
		wfDec.wFormatTag = VS_ACODEC_MP3;
	}
	else if (codecName == "speex")
	{
		encoder = new VS_SpeexAudioCodec(VS_ACODEC_SPEEX, true);
		decoder = new VS_SpeexAudioCodec(VS_ACODEC_SPEEX, false);
		wfEnc.wFormatTag = VS_ACODEC_SPEEX;
		wfDec.wFormatTag = VS_ACODEC_SPEEX;
	}
	else if (codecName == "opus")
	{
		encoder = new VS_OpusAudioCodec(VS_ACODEC_OPUS_B0914, true);
		decoder = new VS_OpusAudioCodec(VS_ACODEC_OPUS_B0914, false);
		wfEnc.wFormatTag = VS_ACODEC_OPUS_B0914;
		wfDec.wFormatTag = VS_ACODEC_OPUS_B0914;
	}

	if (encoder->Init(&wfEnc) != 0)
		std::cout << "encoder init FAILED" << std::endl;

	if (decoder->Init(&wfDec) != 0)
		std::cout << "decoder init FAILED" << std::endl;

	/**
	*	Open files
	*/

	pcmInFile.open(pcmInFileName.c_str(), std::ios::binary);

	if (!pcmInFile.is_open())
		return -1;

	compressedFile.open(compressedFileName.c_str(), std::ios::binary);

	if (!compressedFile.is_open())
		return -1;

	pcmOutFile.open(pcmOutFileName.c_str(), std::ios::binary);

	if (!pcmOutFile.is_open())
		return -1;

	logFile.open(logFileName.c_str(), std::ios::ate | std::ios::app);

	if (!logFile.is_open())
		return -1;

	/**
	*	Process
	*/

	int err;
	int readedSize;
	int totalReadedSamples = 0;
	int encodedSize = 0;
	int decodedSize = 0;

	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point encodeStart;
	std::chrono::high_resolution_clock::time_point decodeStart;
	std::chrono::milliseconds encodeTime = std::chrono::milliseconds(0);
	std::chrono::milliseconds decodeTime = std::chrono::milliseconds(0);

	const int numSamples = 320;
	const int sampleRate = 16000;

	short* inBuff = new short[numSamples];
	float* compressedBuff = new float[numSamples];
	short* outBuff = new short[sampleRate];

	//logFile << "begin" << std::endl;

	while (!pcmInFile.read((char*)inBuff, numSamples * sizeof(short)).eof())
	{
		readedSize = pcmInFile.gcount();
		totalReadedSamples += readedSize / sizeof(short);

		//logFile << "readSize " << readedSize << std::endl;

		encodeStart = clock.now();
		encodedSize = encoder->Convert((BYTE*)inBuff, (BYTE*)compressedBuff, readedSize);
		encodeTime += std::chrono::duration_cast<std::chrono::milliseconds>(clock.now() - encodeStart);

		//logFile << "encodedSize " << encodedSize << std::endl;

		if (encodedSize)
		{
			compressedFile.write((char*)compressedBuff, encodedSize);

			decodeStart = clock.now();
			decodedSize = decoder->Convert((BYTE*)compressedBuff, (BYTE*)outBuff, encodedSize);
			decodeTime += std::chrono::duration_cast<std::chrono::milliseconds>(clock.now() - decodeStart);

			pcmOutFile.write((char*)outBuff, decodedSize);

			//logFile << "decodedSize " << decodedSize << std::endl;
		}
	}

	logFile << codecName << " | " << pcmInFileName << " | " << pcmOutFileName << " | "
		<< "Samples : " << totalReadedSamples << " | "
		<< "Encode : " << encodeTime.count() << " | "
		<< "Decode : " << decodeTime.count() << std::endl;

	pcmInFile.close();
	compressedFile.close();
	pcmOutFile.close();

	delete[] inBuff;
	delete[] compressedBuff;
	delete[] outBuff;

	return 0;
}

int main_aac_dec(int argc, char* argv[])
{
	std::ifstream in;
	std::ofstream out;

	WAVEFORMATEX wf;
	wf.cbSize = 0;
	wf.nChannels = 2;
	wf.nSamplesPerSec = 24000;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = VS_ACODEC_AAC;

	std::unique_ptr<AudioCodec> decoder(new VS_AACAudioCodec(VS_ACODEC_AAC, false));
	decoder->Init(&wf);

	in.open("test_dec.aac", std::ios::in | std::ios::binary);
	if (!in.is_open())
		return -1;

	out.open("test_dec.s16le", std::ios::out | std::ios::binary);
	if (!out.is_open())
		return -1;

	std::unique_ptr<char[]> in_buf(new char[1024 * 1024]);
	std::unique_ptr<char[]> out_buf(new char[1024 * 1024]);

	while (true)
	{
		in.read(in_buf.get(), 7);
		if (in.gcount() != 7)
			break;

		const unsigned int frame_size
			= (unsigned(in_buf[3] & 0x03) << 11)
			| (unsigned(in_buf[4] & 0xff) << 3)
			| (unsigned(in_buf[5] & 0xe0) >> 5);

		in.read(in_buf.get() + 7, frame_size - 7);
		if (in.gcount() != frame_size - 7)
			break;

		int ret = decoder->Convert((BYTE*)in_buf.get(), (BYTE*)out_buf.get(), frame_size);
		if (ret < 0)
			break;

		out.write(out_buf.get(), ret);
	}

	in.close();
	out.close();

	return 0;
}

int main_vad(int argc, char* argv[])
{
	std::string vadName = "no";
	std::string inputFileName = "";
	std::string outputFileName = "";
	size_t samplesPerSec = 0;
	size_t frameDuration = 0;
	int mode = 0;

	for (int i = 0; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-i")
		{
			inputFileName = argv[i + 1];
			i++;
		}
		else if (arg == "-o")
		{
			outputFileName = argv[i + 1];
			i++;
		}
		else if (arg == "-n")
		{
			vadName = argv[i + 1];
			i++;
		}
		else if (arg == "-s")
		{
			samplesPerSec = ::atoi(argv[i + 1]);
			i++;
		}
		else if (arg == "-f")
		{
			frameDuration = ::atoi(argv[i + 1]);
			i++;
		}
		else if (arg == "-m")
		{
			mode = ::atoi(argv[i + 1]);
			i++;
		}
	}

	std::ifstream inputFile(inputFileName, std::ios::binary);
	std::ofstream outputVoice(outputFileName, std::ios::binary);

	size_t frameSize = frameDuration * samplesPerSec / 1000;

	std::vector<int16_t> inData(frameSize);
	std::vector<int16_t> zeroes(frameSize, 0);

	if (vadName == "vs")
	{
		VSAudioVAD vad;

		vad.Init(samplesPerSec, sizeof(int16_t) * 8);

		size_t readedSize = 0;

		while (true)
		{
			inputFile.read((char*)inData.data(), frameSize);
			readedSize = inputFile.gcount();

			if (readedSize == 0)
				break;

			if (vad.IsVad(inData.data(), readedSize))
				outputVoice.write((char*)inData.data(), readedSize);
			else
				outputVoice.write((char*)zeroes.data(), readedSize);
		}
	}
	/*else if (vadName == "webrtc")
	{
		VS_RtcVoiceActivity vad;

		vad.Init(samplesPerSec, mode);

		size_t readedSize = 0;

		while (true)
		{
			inputFile.read((char*)inData.data(), frameSize);
			readedSize = inputFile.gcount();

			if (readedSize == 0)
				break;

			if (vad.Process(inData.data(), readedSize / sizeof(int16_t)))
				outputVoice.write((char*)inData.data(), readedSize);
			else
				outputVoice.write((char*)zeroes.data(), readedSize);
		}
	}*/

	return 0;
}

#include <bitset>
#include "../../Hidapi/HidDevice.h"

int main_hid_jabra(int argc, char* argv[])
{
	TC_JabraSpeak410Hid device;

	while (true)
	{
		device.UpdateState();

		if (device.IsOnCall())
			std::cout << "On Call" << std::endl;
		else
			std::cout << "Not On Call" << std::endl;

		Sleep(1);
	}
}


int main_hid_usage(int argc, char* argv[])
{
	int res = 0;
	unsigned char buf[256];
	const int MAX_STR = 255;
	wchar_t wstr[MAX_STR];
	std::shared_ptr<HidApi> hidApi(HidApi::GetInstance());
	hid_device *handle;
	int i;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	struct hid_device_info *devs, *cur_dev;

	if (hidApi->hid_init())
		return -1;

	devs = hidApi->hid_enumerate(0x1de7, 0);
	cur_dev = devs;

	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n", cur_dev->interface_number);
		printf("\n");

		if (cur_dev->interface_number == 4)
		{
			std::string path = cur_dev->path;

			handle = hidApi->hid_open_path(path.c_str());

			//handle->input_report_length = 8;
			//handle->output_report_length = 8;
			break;
		}

		cur_dev = cur_dev->next;
	}

	hidApi->hid_free_enumeration(devs);

	//hidApi->hid_set_nonblocking(handle, 1);

	size_t reportSize = handle->input_report_length;
	std::vector<uint8_t> report(reportSize, 0);

	while (res >= 0)
	{
		report[0] = 0x0;
		report[1] = 0xf0;
		report[2] = 0x0f;
		report[3] = 0x00;
		report[4] = 0x05;
		report[5] = 0x0;
		report[6] = 0x2;
		report[7] = 0x2e;
		report[8] = 0x26;

		res = hidApi->hid_write(handle, report.data(), reportSize);

		for (size_t i = 0; i < reportSize; i++)
			report[i] = 0;

		Sleep(40);

		res = hidApi->hid_read(handle, report.data(), reportSize);

		for (size_t i = 0; i < reportSize; i++)
			std::cout << int(report[i]) << " ";
		std::cout << std::endl;

		if (report[8] != 0)
		{
			int a = 0;
		}

		/*if (report[1] & 0x01)
		std::cout << "Call";

		size_t bn = 0;
		std::cin >> bn;

		report[0] = 3;
		report[1] = bn ? 1 << (bn - 1) : 0;
		report[2] = 0;*/


		Sleep(1);

	}

	hidApi->hid_close(handle);
	hidApi->hid_exit();

	return 0;
}


int main_mp4(int argc, char* argv[])
{
	VSVideoFileReader inFile;
	VSVideoFileWriter outFile;
	VSVideoFile::SVideoInfo inVideoInfo;
	VSVideoFile::SVideoInfo outVideoInfo;
	VSVideoFile::SAudioInfo inAudioInfo;
	VSVideoFile::SAudioInfo outAudioInfo;

	std::string inFileName = "d:\\temp\\bad.avi";
	std::string outFileName = "out.avi";

	if (!inFile.Init(inFileName))
		return -1;

	if (!outFile.Init(outFileName))
		return -1;

	if (!inFile.GetVideoFormat(inVideoInfo))
		return -1;

	if (!inFile.GetAudioFormat(inAudioInfo))
		return -1;

	outVideoInfo = inVideoInfo;
	outAudioInfo = inAudioInfo;

	if (!outFile.SetVideoFormat(outVideoInfo))
		return -1;

	if (!outFile.SetAudioFormat(outAudioInfo))
		return -1;

	if (!outFile.WriteHeader())
		return -1;

	char* inVideoBuffer = new char[inVideoInfo.Width * inVideoInfo.Height * 3 / 2];
	char* inAudioBuffer = new char[10000];
	char* outAudioBuffer = new char[10000];

	int audioReadedSize = 1;
	int videoReadedSize = 1;
	bool isKey = false;

	AudioCodec* decoder = new VS_MP3AudioCodec(VS_ACODEC_MP3, false);

	WAVEFORMATEX wfDec;
	wfDec.cbSize = 0;
	wfDec.nChannels = 1;
	wfDec.nSamplesPerSec = 16000;
	wfDec.wBitsPerSample = 16;
	wfDec.wFormatTag = VS_ACODEC_MP3;

	decoder->Init(&wfDec);

	std::ofstream out("out.raw", std::ios::binary);

	while (videoReadedSize > 0 && audioReadedSize > 0)
	{
		videoReadedSize = inFile.ReadVideo(inVideoBuffer, &isKey);
		audioReadedSize = inFile.ReadAudio(inAudioBuffer);

		//size_t decoded = decoder->Convert((BYTE*)inAudioBuffer, (BYTE*)outAudioBuffer, audioReadedSize);

		if (videoReadedSize > 0)
		{
			if (!outFile.WriteVideo(inVideoBuffer, videoReadedSize, isKey))
				return -1;
		}

		out.write(inAudioBuffer, audioReadedSize);

		/*if (audioReadedSize > 0)
		{
			if (!outFile.WriteAudioSamples(outAudioBuffer, decoded))
				return -1;
		}*/
	}

	outFile.Release();

	return 0;
}

#include "../../VSClient/AudioProcessing/VS_AudioProcessing.h"

int main_new_webrtc(int argc, char* argv[])
{
	return 0;
}

int main_test_capt_process_rend(int argc, char* argv[])
{
	VS_MediaFormat fmt;
	fmt.SetAudio(16000, VS_ACODEC_PCM);

	HWND hwnd;
	if (!CreateMainWindow(hwnd))
		return -2;
	VS_AudioDeviceManager::Open(hwnd);

	VSTestAudio testAudio;

	testAudio.Start(&fmt, 1, 0, VSTestAudio::ETestMode::TM_ECHO_DELAY);

	std::cin.get();

	testAudio.Stop();

	return 0;
}

#include <modules/audio_processing/audio_buffer.h>
#include <modules/audio_processing/aec/echo_cancellation.h>
#include <modules/audio_processing/aec/aec_resampler.h>
#include <modules/audio_processing/ns/ns_core.h>
#include <common_audio/ring_buffer.h>

#include "../../VSClient/AudioProcessing/VSEchoDelayDetector.h"

int main_delay_detect(int argc, char* argv[])
{
	std::string capturedFileName;
	std::string renderedFileName;

	for (int i = 1; i < argc; i++)
	{
		std::string command = argv[i];

		if (command == "-c")
		{
			capturedFileName = argv[i + 1];
			i++;
		}
		else if (command == "-r")
		{
			renderedFileName = argv[i + 1];
			i++;
		}
	}

	std::vector<int16_t> capturedData;
	std::vector<int16_t> renderedData;

	// read capture

	std::ifstream capturedFile(capturedFileName, std::ios::binary | std::ios::ate);

	if (capturedFile.is_open())
	{
		capturedData.resize(capturedFile.tellg() / sizeof(int16_t));

		capturedFile.seekg(0);
		capturedFile.read((char*)capturedData.data(), capturedData.size() * sizeof(int16_t));
	}

	// read render

	std::ifstream renderedFile(renderedFileName, std::ios::binary | std::ios::ate);

	if (renderedFile.is_open())
	{
		renderedData.resize(renderedFile.tellg() / sizeof(int16_t));

		renderedFile.seekg(0);
		renderedFile.read((char*)renderedData.data(), renderedData.size() * sizeof(int16_t));
	}

	// init delaydetector

	VSEchoDelayDetector delayDetector;

	delayDetector.Init(0, 0);
	delayDetector.Start();

	// detect

	delayDetector.ProcessAudio(capturedData.data(), renderedData.data(), capturedData.size());

	int32_t delay;
	delayDetector.GetDelay(delay);

	return 0;
}

//#include "../VS_AudioReSampler.h"

#include <chrono>
#include <iomanip>

#include "common_audio\resampler\include\resampler.h"
#include "ipps.h"
//#include "../FFResampler.h"

int main_resampling(int argc, char* argv[])
{
	std::string resamplerName;
	std::string inFileName;
	std::string outFileName;
	int inFrequency;
	int outFrequency;
	int quality = 0;
	std::string perfomanceFileName;

	for (int i = 1; i < argc; i++)
	{
		std::string command = argv[i];

		if (command == "-r")
		{
			resamplerName = argv[i + 1];
			i++;
		}
		else if (command == "-i")
		{
			inFileName = argv[i + 1];
			i++;
		}
		else if (command == "-o")
		{
			outFileName = argv[i + 1];
			i++;
		}
		else if (command == "-fi")
		{
			inFrequency = ::atoi(argv[i + 1]);
			i++;
		}
		else if (command == "-fo")
		{
			outFrequency = ::atoi(argv[i + 1]);
			i++;
		}
		else if (command == "-q")
		{
			quality = ::atoi(argv[i + 1]);
			i++;
		}
		else if (command == "-l")
		{
			perfomanceFileName = argv[i + 1];
			i++;
		}
	}

	std::ifstream inFile(inFileName.c_str(), std::ios::binary);
	std::ofstream outFile(outFileName.c_str(), std::ios::binary);
	std::ofstream perfomanceFile(perfomanceFileName.c_str(), std::ios::ate | std::ios::app);

	if (!inFile.is_open())
		return -1;

	if (!outFile.is_open())
		return -1;

	if (!perfomanceFile.is_open())
		return -1;

	size_t inBufferSize = inFrequency * sizeof(short);
	size_t outBufferSize = inBufferSize * outFrequency / inFrequency;
	uint8_t* inBuff = new uint8_t[inBufferSize];
	uint8_t* outBuff = new uint8_t[outBufferSize];

	clock_t start = 0;
	clock_t total = 0;

	if (resamplerName == "speex")
	{
		VS_AudioReSamplerSpeex* speexResampler = new VS_AudioReSamplerSpeex(quality);
		size_t readed = 0;
		long resampled = 0;

		readed = inFile.read((char*)inBuff, inBufferSize).gcount();

		while (readed > 0)
		{
			start = clock();
			resampled = speexResampler->Process(inBuff, outBuff, readed, inFrequency, outFrequency);
			total += clock() - start;

			if (resampled)
				outFile.write((char*)outBuff, resampled);

			readed = inFile.read((char*)inBuff, inBufferSize).gcount();
		}
	}
	else if (resamplerName == "webrtc")
	{
		webrtc::Resampler webrtcResampler;

		if (webrtcResampler.Reset(inFrequency, outFrequency, 1) != 0)
		{
			perfomanceFile << resamplerName << " | "
				<< std::setw(6) << inFrequency << " | "
				<< std::setw(6) << outFrequency << " | "
				<< " init failed" << std::endl;

			return 0;
		}

		size_t readed;
		size_t resampled;

		readed = inFile.read((char*)inBuff, inBufferSize).gcount();

		while (readed)
		{
			start = clock();
			webrtcResampler.Push((const int16_t*)inBuff, readed / sizeof(short), (int16_t*)outBuff, outBufferSize / sizeof(short), resampled);
			total += clock() - start;

			if (resampled)
				outFile.write((char*)outBuff, resampled * sizeof(short));

			readed = inFile.read((char*)inBuff, inBufferSize).gcount();
		}
	}
	/*else if (resamplerName == "ffmpeg")
	{
		FFResampler ffResampler;
		size_t readed;
		long resampled;

		readed = inFile.read((char*)inBuff, inBufferSize).gcount();

		while (readed)
		{
			start = clock();
			resampled = ffResampler.Resample((const int16_t*)inBuff, readed / sizeof(short), (int16_t*)outBuff, inFrequency, outFrequency);
			total += clock() - start;

			if (resampled)
				outFile.write((char*)outBuff, resampled * sizeof(short));

			readed = inFile.read((char*)inBuff, inBufferSize).gcount();
		}
	}*/
	else if (resamplerName == "ipp")
	{
		// maybe we need update ipp
	}

	perfomanceFile << resamplerName << " q " << quality << " | "
		<< std::setw(6) << inFrequency << " | "
		<< std::setw(6) << outFrequency << " | "
		<< float(total) / CLOCKS_PER_SEC << std::endl;

	return 0;
}

size_t myMin(size_t a, size_t b)
{
	if (a > b)
		return a;

	return b;
}

int main_audio_diff(int argc, char* argv[])
{
	std::string inFileName = argv[1];
	std::string outFileName = argv[2];
	std::string resultFileName = argv[3];

	std::ifstream inFile(inFileName.c_str(), std::ios::binary);
	std::ifstream outFile(outFileName.c_str(), std::ios::binary);
	std::ofstream resultFile(resultFileName.c_str(), std::ios::ate | std::ios::app);

	if (!inFile.is_open())
		return -1;

	if (!outFile.is_open())
		return -1;

	if (!resultFile.is_open())
		return -1;

	uint64_t error = 0;
	uint64_t sqrError = 0;
	uint64_t totalSamples = 0;

	const int bufferSamples = 4096;
int16_t inSample[bufferSamples];
int16_t outSample[bufferSamples];
size_t readed = 0;

while (readed = (myMin(inFile.read((char*)inSample, sizeof(inSample)).gcount(),
	outFile.read((char*)outSample, sizeof(outSample)).gcount()) / sizeof(short)))
{
	totalSamples += readed;

	for (size_t i = 0; i < readed; i++)
	{
		error += abs(outSample[i] - inSample[i]);
		sqrError += error * error;
	}
}

resultFile << "[src : " << inFileName << "] [dst : " << outFileName
<< "] [err : " << float(error) / float(totalSamples) << "] [sqrErr : " << float(sqrError) / float(totalSamples) << "]"
<< std::endl;

inFile.close();
outFile.close();
resultFile.close();
return 0;
}

#include "common_audio\signal_processing\include\real_fft.h"

std::vector<std::vector<uint16_t>> GetHistogramFromFile(const std::string fileName)
{
	std::ifstream inFile(fileName.c_str(), std::ios::binary);

	if (!inFile.is_open())
		throw "no file";

	size_t order = 8;

	RealFFT* fft = WebRtcSpl_CreateRealFFT(order);

	size_t scanWindowLen = 1 << order;
	size_t bufferLen = scanWindowLen * 2;

	size_t scanWindowSize = scanWindowLen * sizeof(int16_t);
	size_t inBufferSize = bufferLen * sizeof(int16_t);
	size_t outBufferSize = bufferLen * sizeof(int16_t);
	std::vector<int16_t> inBuff(bufferLen);
	std::vector<int16_t> outBuff(scanWindowLen + 2);

	std::vector<std::vector<uint16_t>> image;

	size_t readed = 0;
	size_t totalReaded = 0;

	readed = inFile.read((char*)inBuff.data(), scanWindowSize).gcount();

	while ((readed = inFile.read((char*)(inBuff.data() + scanWindowLen), scanWindowSize).gcount()))
	{
		totalReaded += readed;

		for (size_t offset = 0; offset < scanWindowLen; offset += 256)
		{
			WebRtcSpl_RealForwardFFT(fft, inBuff.data() + offset, outBuff.data());

			image.emplace_back(scanWindowLen / 2);

			for (size_t i = 0; i < image.back().size(); i++)
			{
				double re = outBuff[2 * (i + 1)];
				double im = outBuff[2 * (i + 1) + 1];
				uint16_t mag = sqrt(re * re + im * im);

				image.back()[i] = mag;
			}
		}

		std::copy(inBuff.begin() + scanWindowLen, inBuff.end(), inBuff.begin());
	}

	return image;
}

std::vector<double> CalcMatch(
	std::vector<std::vector<uint16_t>> patternFft,
	std::vector<std::vector<uint16_t>> recordFft)
{
	std::vector<double> result;

	for (size_t offset = 0; offset < recordFft.size() - patternFft.size(); offset++)
	{
		double res = 0.0;

		for (size_t i = 0; i < patternFft.size(); i++)
		{
			for (size_t j = 0; j < patternFft.front().size(); j++)
			{
				double p = patternFft[i][j];
				double r = recordFft[i + offset][j];

				res += p / std::numeric_limits<uint16_t>::max() * std::abs(p - r);
			}
		}

		result.push_back(res);
	}

	return result;
}

int main_fft(int argc, char* argv[])
{
	std::string patternFileName;
	std::string recordFileName;

	for (int i = 1; i < argc; i++)
	{
		std::string command = argv[i];

		if (command == "-p")
		{
			patternFileName = argv[i + 1];
			i++;
		}
		else if (command == "-r")
		{
			recordFileName = argv[i + 1];
			i++;
		}
	}

	std::vector<std::vector<uint16_t>> patternFft = GetHistogramFromFile(patternFileName);
	std::vector<std::vector<uint16_t>> recordFft = GetHistogramFromFile(recordFileName);

	std::vector<double> match = CalcMatch(patternFft, recordFft);

	std::ofstream matchFile("out.txt");

	for (double d : match)
	{
		matchFile << d << std::endl;
	}

	return 0;
}

class VS_FramePipe
{
	unsigned char *m_buffer;
	int m_size;
	int m_width;
	int m_height;
	unsigned int m_fourcc;
public :
	VS_FramePipe(unsigned char *p, int len, int w, int h, unsigned int fourcc)
	{
		m_buffer = new unsigned char[len];
		memcpy(m_buffer, p, len);
		m_width = w;
		m_height = h;
		m_size = len;
		m_fourcc = fourcc;
	}
	unsigned char* GetBuffer() { return m_buffer; }
	int GetSize() { return m_size; }
	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }
	unsigned int GetFourcc() { return m_fourcc; }
	void DeleteFrame()
	{
		delete[] m_buffer;
	}
	~VS_FramePipe()
	{

	}
};

class VS_QueuePipe
{
	VS_Lock m_rcvLock;
	std::queue <VS_FramePipe> m_qFrames;
public:
	VS_QueuePipe()
	{

	}
	~VS_QueuePipe()
	{
		m_rcvLock.Lock();
		while (!m_qFrames.empty()) {
			m_qFrames.front().DeleteFrame();
			m_qFrames.pop();
		}
		m_rcvLock.UnLock();
	}
	void PushFrame(VS_FramePipe *frame)
	{
		m_rcvLock.Lock();
		while (m_qFrames.size() > 20) {
			m_qFrames.front().DeleteFrame();
			m_qFrames.pop();
		}
		m_qFrames.push(*frame);
		m_rcvLock.UnLock();
	}
	bool GetFrame(unsigned char *&buffer, int &size, int &w, int &h)
	{
		m_rcvLock.Lock();
		if (!m_qFrames.empty()) {
			buffer = m_qFrames.front().GetBuffer();
			size = m_qFrames.front().GetSize();
			w = m_qFrames.front().GetWidth();
			h = m_qFrames.front().GetHeight();
			m_qFrames.pop();
		}
		bool ret = m_qFrames.empty();
		m_rcvLock.UnLock();
		return ret;
	}
};

std::map <void*, VS_QueuePipe*> g_receiverQueue;
std::map <void*, VS_QueuePipe*> g_encoderQueue;
std::map <void*, VS_QueuePipe*> g_decoderQueue;

/*struct VS_ReceiverPipe : public CVSThread
{
	char m_nameFile[128];
	int m_numRcv;
	HANDLE m_handleRcv[128];
	HANDLE m_hEndRcv;
	double m_framerate;
	int m_rcvFrame;
public:
	VS_ReceiverPipe(double fps, int rcvFrame)
	{
		m_numRcv = 0;
		m_framerate = fps;
		m_rcvFrame = rcvFrame;
		m_hEndRcv = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	~VS_ReceiverPipe()
	{
		ReceiverStop();
		CloseHandle(m_hEndRcv);
	}
	void ReceiverStart(char *fileName);
	void ReceiverStop();
	HANDLE GetEndEvent() { return m_hEndRcv; }
	void ConnectReceiver(HANDLE hRcv);
	void DisconnectReceiver(HANDLE hRcv);
	DWORD Loop(LPVOID lpParameter);
};

void VS_ReceiverPipe::ReceiverStart(char *fileName)
{
	if (m_numRcv > 0) {
		strcpy(m_nameFile, fileName);
		ActivateThread(this);
	}
}

void VS_ReceiverPipe::ReceiverStop()
{
	if (IsThreadActiv()) {
		DesactivateThread();
	}
}

void VS_ReceiverPipe::ConnectReceiver(HANDLE hRcv)
{
	m_handleRcv[m_numRcv] = hRcv;
	m_numRcv++;
	VS_QueuePipe *receiverQueue = new VS_QueuePipe();
	g_receiverQueue.insert(std::pair <void*, VS_QueuePipe*>(hRcv, receiverQueue));
}

void VS_ReceiverPipe::DisconnectReceiver(HANDLE hRcv)
{
	for (int i = 0; i < m_numRcv; i++) {
		if (hRcv == m_handleRcv[i]) {
			for (int j = i; j < m_numRcv - 1; j++) {
				m_handleRcv[j] = m_handleRcv[j + 1];
			}
			std::map<void*, VS_QueuePipe*>::iterator it = g_receiverQueue.find(hRcv);
			if (it != g_receiverQueue.end()) {
				delete it->second;
				g_receiverQueue.erase(it);
			}
			break;
		}
	}
	m_numRcv--;
}

DWORD VS_ReceiverPipe::Loop(LPVOID lpParameter)
{
	CAviFile aviRcv;
	if (aviRcv.Init(m_nameFile)) {
		BITMAPINFOHEADER *bmin = 0;
		int size = 0;
		if ((size = aviRcv.GetFormat(bmin)) > 0) {
			bmin = (BITMAPINFOHEADER *)malloc(size);
			aviRcv.GetFormat(bmin);
			unsigned char *rcvBuff = new unsigned char[bmin->biSizeImage];
			double dt = 1000.0 / m_framerate;
			unsigned int dt_wait = 0;
			unsigned int start_time = timeGetTime();
			int frameCount = 0;
			unsigned int next_time = 0;
			unsigned int readtime = 0;

			do {
				unsigned long res = WaitForSingleObject(m_hEvDie, dt_wait);
				if (res == WAIT_OBJECT_0 + 0) {
					break;
				}
				unsigned int ct = timeGetTime();
				size = aviRcv.ReadVideo(rcvBuff, bmin->biSizeImage);
				readtime += timeGetTime() - ct;
				if (size <= 0 || frameCount > m_rcvFrame) {
					break;
				}
				for (int i = 0; i < m_numRcv; i++) {
					VS_FramePipe frame(rcvBuff, size, bmin->biWidth, bmin->biHeight, bmin->biCompression);
					std::map<void*, VS_QueuePipe*>::iterator it = g_receiverQueue.find(m_handleRcv[i]);
					if (it != g_receiverQueue.end()) {
						it->second->PushFrame(&frame);
					}
				}
				for (int i = 0; i < m_numRcv; i++) {
					SetEvent(m_handleRcv[i]);
				}
				frameCount++;
				if (next_time == 0) {
					next_time = ct;
				}
				next_time += dt;
				double w = next_time - (double)ct + 0.5;
				if (w < 0) w = 0;
				dt_wait = w;
			} while (true);

			double fps = (frameCount * 1000.0) / (double)(timeGetTime() - start_time);
			double ms = (double)readtime / (double)frameCount;
			printf("RECEIVER : fps = %4.2f; avg_ms = %4.2f; count = %d\n", fps, ms, frameCount);
			delete[] rcvBuff;
			free(bmin);
		}
		aviRcv.Release();
	}
	SetEvent(m_hEndRcv);
	return 0;
}

struct VS_EncoderPipe : public CVSThread
{
	VideoCodec *m_encoder;
	HANDLE m_rcvHandle;
	HANDLE m_hEndEnc;
	int m_width;
	int m_height;
	int m_bitrate;
	unsigned int m_framerate;
public:
	VS_EncoderPipe(unsigned int fourcc, int hw, unsigned int framerate, int bitrate)
	{
		m_hEndEnc = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_rcvHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_encoder = VS_RetriveVideoCodec(fourcc, true, hw);
		m_framerate = framerate;
		m_bitrate = bitrate;
		m_width = 0;
		m_height = 0;
		if (m_encoder) {
			ActivateThread(this);
		}
	}
	~VS_EncoderPipe()
	{
		DesactivateThread();
		CloseHandle(m_rcvHandle);
		CloseHandle(m_hEndEnc);
		delete m_encoder;
	}
	HANDLE GetEncoderHandle() { return m_rcvHandle; }
	HANDLE GetEndEvent() { return m_hEndEnc; }
	DWORD Loop(LPVOID lpParameter);
};

DWORD VS_EncoderPipe::Loop(LPVOID lpParameter)
{
	HANDLE handles[2] = { m_hEvDie, m_rcvHandle };
	bool exit = false;
	unsigned int dt_wait = -1;
	unsigned int enctime = 0;
	int frameCount = 0;
	unsigned char *decbuffer = 0;
	VS_VideoCodecParam param;
	memset(&param, 0, sizeof(VS_VideoCodecParam));

	do {
		unsigned long res = WaitForMultipleObjects(2, handles, FALSE, dt_wait);
		if (res == WAIT_OBJECT_0 + 0) {
			exit = true;
			dt_wait = 20;
			ResetEvent(m_hEvDie);
		} else {
			unsigned char *buffer = 0;
			int size = 0, w = 0, h = 0;
			bool ret = true;
			std::map<void*, VS_QueuePipe*>::iterator it = g_receiverQueue.find(m_rcvHandle);
			if (it != g_receiverQueue.end()) {
				ret = it->second->GetFrame(buffer, size, w, h);
			}
			if (buffer) {
				if (m_width != w || m_height != h) {
					if (m_encoder->Init(w, h, FOURCC_I420, 80, 1, m_framerate) == 0) {
						m_encoder->SetBitrate(m_bitrate);
						m_width = w;
						m_height = h;
						delete[] decbuffer;
						decbuffer = new unsigned char[w*h*3/2];
					}
				}
				if (m_encoder->IsValid()) {
					unsigned int ct = timeGetTime();
					int ret = m_encoder->Convert(buffer, decbuffer, &param);
					enctime += timeGetTime() - ct;
					frameCount++;
				}
				delete[] buffer;
			} else {
				if (ret && exit) break;
			}
		}
	} while (true);

	delete[] decbuffer;
	double fps = (frameCount * 1000.0) / (double)(enctime + 1);
	printf("ENCODER %x : fps = %4.2f; ms = %d; count = %d\n", this, fps, enctime, frameCount);
	SetEvent(m_hEndEnc);
	return 0;
}

int main_hardware_transcoder(int argc, char* argv[])
{
	int i = 0;
	unsigned int cmp_fourcc = 0, fourcc = 0;
	int hw_enc = 0, hw_dec = 0;
	int ec = 0;
	bool bEncoderPerformance = false;
	bool bPSNR = false;
	int bitrate = 0;
	int type_rc = 0;
	int preset = 0;
	int quality = 0;
	int num_threads = 1;
	int svc_mode = 0;
	char nameInFile[128];
	char nameOutFile[128];
	/// transcoder
	int numRcv = 1;
	int numSnd = 1;
	unsigned int rcvFramerate = 30;
	int rcvFrame = INT_MAX;
	/// parse input
	for (i = 1; i < argc; i++) {
		if (!argv[i]) return -1;
		if ('-' != argv[i][0]) {
			/// type codec
			if (0 == strcmp(argv[i], "h264")) {
				fourcc = VS_VCODEC_H264;
				cmp_fourcc = '462H';
			}
			else if (0 == strcmp(argv[i], "i264")) {
				fourcc = VS_VCODEC_I264;
				cmp_fourcc = '462H';
			}
			else if (0 == strcmp(argv[i], "vp80")) {
				fourcc = VS_VCODEC_VPX;
				cmp_fourcc = '08PV';
			}
			else if (0 == strcmp(argv[i], "xc02")) {
				fourcc = VS_VCODEC_XC02;
				cmp_fourcc = '20CX';
			}
			else if (0 == strcmp(argv[i], "h261")) {
				fourcc = VS_VCODEC_H261;
				cmp_fourcc = '162H';
			}
			else if (0 == strcmp(argv[i], "h263")) {
				fourcc = VS_VCODEC_H263;
				cmp_fourcc = '362H';
			}
			else if (0 == strcmp(argv[i], "h263p")) {
				fourcc = VS_VCODEC_H263P;
				cmp_fourcc = '362H';
			}
			else {
				return -1;
			}
			continue;
		}
		if (0 == strcmp(argv[i], "-hw_enc")) {
			hw_enc = 1;
		}
		else if (0 == strcmp(argv[i], "-hw_dec")) {
			hw_dec = 1;
		}
		else if (0 == strcmp(argv[i], "-ec")) {
			ec = 1;
		}
		else if (0 == strcmp(argv[i], "-test")) {
			bEncoderPerformance = true;
		}
		else if (0 == strcmp(argv[i], "-psnr")) {
			bPSNR = true;
		}
		else if (0 == strcmp(argv[i], "-nrcv")) {
			i++;
			numRcv = atoi(argv[i]);
		}
		else if (0 == strcmp(argv[i], "-nsnd")) {
			i++;
			numSnd = atoi(argv[i]);
		}
		else if (0 == strcmp(argv[i], "-fps")) {
			i++;
			rcvFramerate = atoi(argv[i]);
		}
		else if (0 == strcmp(argv[i], "-nframe")) {
			i++;
			rcvFrame = atoi(argv[i]);
		}
		else {
			/// if 1 character option
			char ch = argv[i][1];
			i++;
			switch (ch)
			{
			case 'i':
				strcpy(nameInFile, argv[i]);
				break;
			case 'o':
				strcpy(nameOutFile, argv[i]);
				break;
			case 'b':
				bitrate = atoi(argv[i]);
				break;
			case 'r':
				type_rc = atoi(argv[i]);
				break;
			case 't':
				num_threads = atoi(argv[i]);
				break;
			case 'q':
				quality = atoi(argv[i]);
				break;
			case 'p':
				preset = atoi(argv[i]);
				break;
			case 's':
				svc_mode = atoi(argv[i]);
				break;
			default:
				break;
			}
		}
	}
	///

	VS_ReceiverPipe *rcvPipe = new VS_ReceiverPipe(rcvFramerate, rcvFrame);
	VS_EncoderPipe *encPipe[128] = { 0 };

	for (i = 0; i < numSnd; i++) {
		encPipe[i] = new VS_EncoderPipe(fourcc, hw_enc, 30, bitrate);
		rcvPipe->ConnectReceiver(encPipe[i]->GetEncoderHandle());
	}
	rcvPipe->ReceiverStart(nameInFile);

	HANDLE *handles = new HANDLE[1 + numSnd];
	handles[0] = rcvPipe->GetEndEvent();
	for (i = 0; i < numSnd; i++) {
		handles[i+1] = encPipe[i]->GetEndEvent();
	}
	WaitForSingleObject(handles[0], -1);
	for (i = 0; i < numSnd; i++) {
		SetEvent(encPipe[i]->m_hEvDie);
	}
	WaitForMultipleObjects(numSnd, handles + 1, TRUE, -1);
	delete[] handles;

	for (i = 0; i < numSnd; i++) {
		rcvPipe->DisconnectReceiver(encPipe[i]->GetEncoderHandle());
		delete encPipe[i];
	}
	delete rcvPipe;

	return 0;
}
*/
#endif // _WIN32

VideoCodec* VS_RetriveVideoCodec(int Id, bool isCodec, unsigned int typeHardware, int32_t deviceId)
{
	if (Id == VS_VCODEC_H261)
		if (isCodec) return new VS_VideoCoderH261;
		else return new VS_VideoDecoderH261;
	else if (Id == VS_VCODEC_H263)
		if (isCodec) return new VS_VideoCoderH263;
		else return new VS_VideoDecoderH263;
	else if (Id == VS_VCODEC_H263P)
		if (isCodec) return new VS_VideoCoderH263P;
		else return new VS_VideoDecoderH263P;
	else if (Id == VS_VCODEC_MPEG4)
		if (isCodec) return new VS_VideoCoderMPEG4;
		else return new VS_VideoDecoderMPEG4;
	else if (Id == VS_VCODEC_H264)
		if (isCodec) {
#ifdef _WIN32
			if (typeHardware > 0) {
				return new VS_NvidiaVideoEncoder(Id);
			}
			else
#endif
			{
				return new VS_OpenH264VideoCodec(Id, isCodec);
			}
		}
		else {
			return new VS_VideoDecoderH264();
		}
	else if (Id == VS_VCODEC_VPX)
	{
		if (isCodec)
			return new VS_VPXVideoCodec(Id, isCodec);
		else
			return new VS_VideoDecoderVP8;
	}
#ifdef _WIN32
	else if (Id == VS_VCODEC_VPXHD)
		return new VS_VPXHDVideoCodec(Id, isCodec);
	else if (Id == VS_VCODEC_VPXSTEREO)
		return new VS_VPXStereoVideoCodec(Id, isCodec);
	else if (Id == VS_VCODEC_I264)
		return (typeHardware) ? new VS_H264IntelVideoCodec(VS_VCODEC_H264, isCodec) : new VS_H264SwIntelVideoCodec(VS_VCODEC_H264, isCodec);
#endif

	return 0;
}

VideoCodec* VS_RetriveVideoCodec(const VS_MediaFormat &mf, bool encoder)
{
	return VS_RetriveVideoCodec(mf.dwVideoCodecFCC, encoder, mf.dwHWCodec);
}

void VS_UnregisteredVideoCodec(VideoCodec *codec)
{
}

std::uintptr_t VS_GetContextDevice(VideoCodec *codec)
{
	return 0;
}

#include "test_utils.h"

void main_switcher(int argc, char* argv[])
{
	std::vector<std::string> args(argv + 2, argv + argc);

	if (std::string(argv[1]) == "enc")
		main_encoder_test(args);
	else if (std::string(argv[1]) == "dec")
		main_decoder_test(args);
	else if (std::string(argv[1]) == "psnr")
		main_psnr_test(args);
}

int main(int argc, char* argv[])
{
	if (!VS_RegistryKey::InitDefaultBackend("memory:force_lm=false"))
		throw std::runtime_error("Can't initialize registry backend!");

	std::vector<std::string> args(argv + 1, argv + argc);

	TestMixerSetLayout();

	return 0;
}
