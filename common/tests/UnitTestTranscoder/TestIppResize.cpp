#include "UnitTestTranscoder.h"

#include "IppLib2/VSVideoProcessingIpp.h"
#include "Transcoder/VSVideoUtils.h"

TEST_F(UnitTestTranscoder, TestIppResize)
{
	VSVideoProcessingIpp p;

	std::vector<VSSize> sizes = {
		{ 320, 176 },
		{ 320, 180 },
		{ 640, 360 }
	};

	for (auto in : sizes)
	{
		std::vector<uint8_t> inData(in.Square() * 3 / 2, 128);

		for (auto out : sizes)
		{
			std::vector<uint8_t> outData(out.Square() * 2 * 3 / 2, 0);

			p.ResampleI420(
				inData.data(),
				in.width, in.height,
				outData.data(),
				out.width, out.height
			);

			for (size_t i = outData.size() / 2; i < outData.size(); i++)
			{
				ASSERT_EQ(outData[i], 0);
			}
		}
	}

	for (auto in : sizes)
	{
		std::vector<uint8_t> inData(in.Square() * 3, 128);

		for (auto out : sizes)
		{
			std::vector<uint8_t> outData(out.Square() * 2 * 3, 0);

			p.ResampleRGB(
				inData.data(),
				in.width, in.width * 3, in.height,
				outData.data(),
				out.width, out.width * 3, out.height
			);

			for (size_t i = outData.size() / 2; i < outData.size(); i++)
			{
				ASSERT_EQ(outData[i], 0);
			}
		}
	}
}

#include "Transcoder/FFCodec.h"

TEST_F(UnitTestTranscoder, TestFfmpegVideoCodecCreate)
{
	std::vector<FF_VCodecID> encoders =
	{
		FFVC_H261,
		FFVC_H263,
		FFVC_H263P,
		FFVC_MPEG4,
	};

	std::vector<FF_VCodecID> decoders =
	{
		FFVC_H261,
		FFVC_H263,
		FFVC_H263P,
		FFVC_H264,
		FFVC_MJPEG,
		FFVC_MPEG4,
		FFVC_H265,
	};

	for (FF_VCodecID id : encoders)
	{
		FF_VideoCodec encoder(id, true);
		ASSERT_EQ(encoder.Init(352, 288, 0, 15), 0) << id;
	}

	for (FF_VCodecID id : decoders)
	{
		FF_VideoCodec decoder(id, false);
		ASSERT_EQ(decoder.Init(352, 288, 0, 15), 0) << id;
	}
}

#include "Transcoder/FFAudioCodec.h"

TEST_F(UnitTestTranscoder, TestFfmpegAudioCodecCreate)
{
	std::vector<int> freq = {
		96000, 88200, 64000, 48000, 44100, 32000,
		24000, 22050, 16000, 12000, 11025, 8000, 7350
	};

	for (int f : freq)
	{
		FFAudioCodec encoder;
		ASSERT_EQ(encoder.Init(f, true, 64000), true);
	}

	for (int f : freq)
	{
		FFAudioCodec decoder;
		ASSERT_EQ(decoder.Init(f, false, 64000), true);
	}
}
