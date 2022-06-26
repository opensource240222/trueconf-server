#include "test_utils.h"

#include <chrono>
#include <fstream>
#include <memory>
#include <algorithm>
#include <numeric>

#include "VSDeltaTime.h"
#include "Transcoder/VideoCodec.h"
#include "Transcoder/VS_OpenH264VideoCodec.h"
#include "Transcoder/VS_VPXVideoCodec.h"
#include "Transcoder/VS_FFVideoCodec.h"
#include "Transcoder/VS_H264IntelVideoCodec.h"
#include "Transcoder/VS_NvidiaVideoCodec.h"
#include "Transcoder/VS_RetriveVideoCodec.h"
#include "Transcoder/VSVideoFileReader.h"
#include "Transcoder/VSVideoFileWriter.h"
#include "std/cpplib/json/elements.h"
#include "std/cpplib/json/writer.h"
#include "Transcoder/VS_RetriveVideoCodec.h"
#include "CalculatePsnr.h"

class PsnrStat
{
public:
	void CalcFrame(uint8_t* img1, uint8_t* img2, int width, int height)
	{
		CalculatePSNR(img1, img2, width, height, &vm, &vm_max, &vm_avg);
		perFrameStat.push_back(vm.Y_YUV);
	}

	double GetAvgStat()
	{
		return 10.0 * log10(255.0 * 255.0 * perFrameStat.size() / vm_avg.Y_YUV);
	}

	const std::vector<double>& GetPerFrameStat()
	{
		return perFrameStat;
	}

private:
	std::vector<double> perFrameStat;
	tVideoMetrics vm, vm_avg, vm_max;
};

int main_encoder_test(std::vector<std::string> args)
{
	using namespace std::chrono;

	std::string inputFileName;
	std::string outputFileName;
	std::string encoderName;
	int targetBitrate = 512;
	int framesToEncode = 0;
	int fps = 0;
	int threads = 1;
	bool psnr = false;
	std::string statFileName = "enc_perfomance.txt";
	bool logPerFrameSize = false;
	bool logPerFrameTime = false;
	bool logPerFramePsnr = false;

	for (int i = 0; i < args.size(); i++)
	{
		std::string command = args[i];

		if (command == "-i")
		{
			inputFileName = args[i + 1];
			i++;
		}
		else if (command == "-c" || command == "--codec")
		{
			encoderName = args[i + 1];
			i++;
		}
		else if (command == "-b")
		{
			targetBitrate = ::atoi(args[i + 1].c_str());
			i++;
		}
		else if (command == "--fps")
		{
			fps = ::atoi(args[i + 1].c_str());
			i++;
		}
		else if (command == "-t" || command == "--threads")
		{
			threads = ::atoi(args[i + 1].c_str());
			i++;
		}
		else if (command == "-f" || command == "--frames")
		{
			framesToEncode = ::atoi(args[i + 1].c_str());
			i++;
		}
		else if (command == "-o")
		{
			outputFileName = args[i + 1];
			i++;
		}
		else if (command == "-l")
		{
			statFileName = args[i + 1];
			i++;
		}
		else if (command == "--psnr")
		{
			psnr = true;
		}
		else if (command == "--per-frame-time")
		{
			logPerFrameTime = true;
		}
		else if (command == "--per-frame-size")
		{
			logPerFrameSize = true;
		}
		else if (command == "--per-frame-psnr")
		{
			logPerFramePsnr = true;
		}
	}

	VSVideoFileReader inFile;
	VSVideoFile::SVideoInfo inVideoInfo;

	if (!inFile.Init(inputFileName))
	{
		std::cerr << "failed open " << inputFileName << std::endl;
		return -1;
	}

	if (!inFile.GetVideoFormat(inVideoInfo))
		return -1;

	if (inVideoInfo.CodecID != VSVideoFile::VCODEC_ID_RAWVIDEO)
	{
		std::cout << inputFileName << " must contain RAW" << std::endl;
		return -1;
	}

	if (!fps)
		fps = inVideoInfo.FPS;

	VSVideoFileWriter outFile;
	VSVideoFile::SVideoInfo outVideoInfo = inVideoInfo;
	std::unique_ptr<VideoCodec> encoder;

	if (encoderName == "h264")
	{
		encoder.reset(VS_RetriveVideoCodec(VS_VCODEC_H264, true));
		outVideoInfo.CodecID = VSVideoFile::VCODEC_ID_H264;
		statFileName = "h264_" + statFileName;
	}
	else if (encoderName == "vp8")
	{
		encoder.reset(VS_RetriveVideoCodec(VS_VCODEC_VPX, true));
		outVideoInfo.CodecID = VSVideoFile::VCODEC_ID_VP8;
		statFileName = "vp8_" + statFileName;
	}

	encoder->Init(inVideoInfo.Width, inVideoInfo.Height, FOURCC_I420, 0, threads, fps);
	encoder->SetBitrate(targetBitrate);

	if (encoderName == "vp8")
	{
		vpx_param par = {};
		par.i_maxinterval = 1000;
		par.bitrate = targetBitrate;
		par.rate_control_method = 1;
		par.deadline = 0;
		par.cpu_used = 0;
		par.me_static_threshold = 800;
		par.error_resilient = 1;
		par.ref_frames = 1;
		encoder->SetCoderOption(&par);
	}

	if (!outputFileName.empty())
	{
		if (!outFile.Init(outputFileName))
		{
			std::cerr << "failed open " << outputFileName << std::endl;
			return -1;
		}

		if (!outFile.SetVideoFormat(outVideoInfo))
			return -1;

		if (!outFile.WriteHeader())
			return -1;
	}

	std::unique_ptr<VideoCodec> decoder;
	std::vector<uint8_t> decodedBuffer(inVideoInfo.Width * inVideoInfo.Height * 3 / 2);
	PsnrStat psnrStat;

	if (psnr)
	{
		if (encoderName == "h264")
		{
			decoder.reset(VS_RetriveVideoCodec(VS_VCODEC_H264, false));
		}
		else if (encoderName == "vp8")
		{
			decoder.reset(VS_RetriveVideoCodec(VS_VCODEC_VPX, false));
		}

		if (decoder->Init(inVideoInfo.Width, inVideoInfo.Height, FOURCC_I420) < 0)
		{
			std::cerr << "fail decoder->Init. Psnr will not be calculated" << std::endl;
			psnr = false;
		}
	}

	std::vector<uint8_t> inBuffer(inVideoInfo.Width * inVideoInfo.Height * 3 / 2);
	std::vector<uint8_t> outBuffer(inVideoInfo.Width * inVideoInfo.Height * 3 / 2);

	size_t frameCount = 0;
	std::vector<float> perFrameTime;
	std::vector<int> perFrameSize;
	VSDeltaTime dt;

	while (frameCount < framesToEncode || !framesToEncode)
	{
		bool isKey = false;
		int readedSize = inFile.ReadVideo(inBuffer.data(), &isKey);

		if (readedSize <= 0)
		{
			if (framesToEncode)
			{
				inFile.SeekToTime(0);
				readedSize = inFile.ReadVideo(inBuffer.data(), &isKey);
			}
			else
			{
				break;
			}
		}

		VS_VideoCodecParam param = {};
		param.cmp.FrameSize = readedSize;
		param.cmp.KeyFrame = (frameCount == 0);

		dt.Tick();
		int converted = encoder->Convert(inBuffer.data(), outBuffer.data(), &param);
		dt.Tick();

		perFrameTime.push_back(dt.DeltaMsF());
		perFrameSize.push_back(converted);

		if (converted >= 0)
		{
			std::cout << "encoded frame " << frameCount << " size : " << converted << std::endl;

			if (!outputFileName.empty() && converted > 0)
			{
				if (!outFile.WriteVideo((char*)outBuffer.data(), converted, param.cmp.IsKeyFrame))
				{
					std::cerr << "failed outFile.WriteVideo" << std::endl;
					return -1;
				}
			}

			if (psnr)
			{
				VS_VideoCodecParam decParam = {};
				decParam.dec.FrameSize = converted;

				decoder->Convert(outBuffer.data(), decodedBuffer.data(), &decParam);

				psnrStat.CalcFrame(inBuffer.data(), decodedBuffer.data(), inVideoInfo.Width, inVideoInfo.Height);
			}
		}

		frameCount++;
	}

	json::Array jsonPerFrameTime;

	for (auto i : perFrameTime)
		jsonPerFrameTime.Insert(json::Number(i));

	json::Array jsonPerFrameSize;

	for (auto i : perFrameSize)
		jsonPerFrameSize.Insert(json::Number(i));

	json::Array jsonPerFramePsnr;

	for (auto i : psnrStat.GetPerFrameStat())
		jsonPerFramePsnr.Insert(json::Number(i));

	float totalTime = std::accumulate(perFrameTime.begin(), perFrameTime.end(), 0);
	float totalSize = std::accumulate(perFrameSize.begin(), perFrameSize.end(), 0);
	float encodeFps = perFrameTime.size() / (totalTime / 1000.f);
	float resultBitrate = (totalSize * fps * 8.0) / (perFrameSize.size() * 1024.0);

	json::Object jsonStat;

	if (logPerFrameTime)
		jsonStat["per_frame_time"] = jsonPerFrameTime;

	if (logPerFrameSize)
		jsonStat["per_frame_size"] = jsonPerFrameSize;

	if (logPerFramePsnr)
		jsonStat["per_frame_psnr"] = jsonPerFramePsnr;

	if (psnr)
		jsonStat["PSNR"] = json::Number(psnrStat.GetAvgStat());

	jsonStat["encoder_fps"] = json::Number(encodeFps);
	jsonStat["exp_bitrate"] = json::Number(targetBitrate);
	jsonStat["res_bitrate"] = json::Number(resultBitrate);

	std::ofstream statFile(statFileName, std::ios::app);

	if (statFile.is_open())
		json::Writer::Write(jsonStat, statFile);
	else
		json::Writer::Write(jsonStat, std::cout);

	return 0;
}

int main_decoder_test(std::vector<std::string> args)
{
	using namespace std::chrono;

	std::string inputFileName;
	std::string outputFileName;
	std::string statFileName;
	std::string decoderName;

	for (int i = 0; i < args.size(); i++)
	{
		std::string command = args[i];

		if (command == "-i")
		{
			inputFileName = args[i + 1];
			i++;
		}
		else if (command == "-o")
		{
			outputFileName = args[i + 1];
			i++;
		}
		else if (command == "-l")
		{
			statFileName = args[i + 1];
			i++;
		}
		else if (command == "-d" || command == "--decoder")
		{
			decoderName = args[i + 1];
			i++;
		}
	}

	VSVideoFileReader inFile;
	VSVideoFile::SVideoInfo inVideoInfo;

	if (inFile.Init(inputFileName))
	{
		if (inFile.GetVideoFormat(inVideoInfo))
		{
			if (decoderName.empty())
			{
				switch (inVideoInfo.CodecID)
				{
				case VSVideoFile::VCODEC_ID_H264:
				{
					decoderName = "h264";
				}
				break;

				case VSVideoFile::VCODEC_ID_VP8:
				{
					decoderName = "vp8";
				}
				break;
				}
			}
		}
		else
		{
			std::cerr << "failed inFile.GetVideoFormat()" << std::endl;
			return -1;
		}
	}
	else
	{
		std::cerr << "failed inFile.Init(" << inputFileName << ")" << std::endl;
		return -1;
	}

	VSVideoFileWriter outFile;
	VSVideoFile::SVideoInfo outVideoInfo;

	if (!outputFileName.empty())
	{
		if (!outFile.Init(outputFileName))
			return -1;

		outVideoInfo = inVideoInfo;
		outVideoInfo.CodecID = VSVideoFile::VCODEC_ID_RAWVIDEO;

		if (!outFile.SetVideoFormat(outVideoInfo))
			return -1;

		if (!outFile.WriteHeader())
			return -1;
	}

	std::unique_ptr<VideoCodec> decoder;

	if (decoderName == "h264")
	{
		decoder.reset(VS_RetriveVideoCodec(VS_VCODEC_H264, false));
	}
	else if (decoderName == "vp8")
	{
		decoder.reset(VS_RetriveVideoCodec(VS_VCODEC_VPX, false));
	}
	else if (decoderName == "vp8_libvpx")
	{
		decoder.reset(new VS_VPXVideoCodec(VS_VCODEC_VPX, false));
	}
	else if (decoderName == "vp8_ffmpeg")
	{
		decoder.reset(new VS_VideoDecoderVP8);
	}
	else
	{
		std::cerr << "unknown decoder : '" << decoderName << "'" << std::endl;
		return -1;
	}

	if (decoder->Init(inVideoInfo.Width, inVideoInfo.Height, FOURCC_I420) < 0)
	{
		std::cerr << "fail decoder->Init " << decoderName << std::endl;
		return -1;
	}

	if (statFileName.empty())
		statFileName = decoderName + "_dec_perfomance.txt";

	std::ofstream statFile(statFileName, std::ios::app);

	std::vector<uint8_t> inBuffer(inVideoInfo.Width * inVideoInfo.Height * 3 / 2);
	std::vector<uint8_t> outBuffer(inVideoInfo.Width * inVideoInfo.Height * 3 / 2);

	int readedSize = 0;
	size_t frameCount = 0;
	bool isKey = false;

	high_resolution_clock::time_point start;
	high_resolution_clock::time_point end;
	nanoseconds totalDecodeTimeNs = nanoseconds(0);

	while ((readedSize = inFile.ReadVideo(inBuffer.data(), &isKey)) > 0)
	{
		VS_VideoCodecParam param = {};
		param.dec.FrameSize = readedSize;

		start = high_resolution_clock::now();
		int converted = decoder->Convert(inBuffer.data(), outBuffer.data(), &param);
		end = high_resolution_clock::now();

		totalDecodeTimeNs += duration_cast<nanoseconds>(end - start);

		if (converted > 0)
		{
			std::cout << "decoded frame " << frameCount << " size : " << converted << std::endl;

			if (!outputFileName.empty())
			{
				if (!outFile.WriteVideo((char*)outBuffer.data(), converted, isKey))
				{
					std::cout << "failed outFile.WriteVideo" << std::endl;
					return -1;
				}
			}
		}

		frameCount++;
	}

	if (statFile.is_open())
	{
		double decodeTimeSec = double(duration_cast<milliseconds>(totalDecodeTimeNs).count()) / 1000;
		double fps = frameCount / decodeTimeSec;

		for (const auto& arg : args)
			statFile << arg << " ";

		statFile << " decoder_fps : " << fps << std::endl;
	}

	return 0;
}

int main_psnr_test(std::vector<std::string> args)
{
	std::string srcFileName;
	std::string dstFileName;
	std::string statFileName = "psnr.txt";

	for (int i = 0; i < args.size(); i++)
	{
		std::string command = args[i];

		if (command == "-s")
		{
			srcFileName = args[i + 1];
			i++;
		}
		else if (command == "-d")
		{
			dstFileName = args[i + 1];
			i++;
		}
		else if (command == "-l")
		{
			statFileName = args[i + 1];
			i++;
		}
	}

	VSVideoFileReader srcFile;
	VSVideoFileReader dstFile;
	std::ofstream statFile;
	VSVideoFile::SVideoInfo srcVideoInfo;
	VSVideoFile::SVideoInfo dstVideoInfo;

	if (!srcFile.Init(srcFileName))
		return -1;

	if (!srcFile.GetVideoFormat(srcVideoInfo))
		return -1;

	if (!dstFile.Init(dstFileName))
		return -1;

	if (!dstFile.GetVideoFormat(dstVideoInfo))
		return -1;

	if (srcVideoInfo.CodecID != VSVideoFile::VCODEC_ID_RAWVIDEO ||
		dstVideoInfo.CodecID != VSVideoFile::VCODEC_ID_RAWVIDEO ||
		srcVideoInfo.Width != dstVideoInfo.Width ||
		srcVideoInfo.Height != dstVideoInfo.Height)
	{
		std::cout << srcFileName << " and " << dstFileName << " incomparable" << std::endl;
		return -1;
	}

	statFile.open(statFileName, std::ios::app);

	std::vector<uint8_t> srcBuffer(srcVideoInfo.Width * srcVideoInfo.Height * 3 / 2);
	std::vector<uint8_t> dstBuffer(dstVideoInfo.Width * dstVideoInfo.Height * 3 / 2);

	int srcReadedSize = 0;
	int dstReadedSize = 0;
	int frameCount = 0;
	bool isKey = 0;

	tVideoMetrics vm = {}, vm_avg = {}, vm_max = {};

	std::vector<double> perFramePsnr;

	while ((srcReadedSize = srcFile.ReadVideo(srcBuffer.data(), &isKey)) > 0 &&
		(dstReadedSize = dstFile.ReadVideo(dstBuffer.data(), &isKey)) > 0)
	{
		CalculatePSNR(srcBuffer.data(), dstBuffer.data(),
			srcVideoInfo.Width, srcVideoInfo.Height,
			&vm, &vm_max, &vm_avg);

		perFramePsnr.push_back(vm.Y_YUV);

		frameCount++;
	}

	double avgPsnrY = 10.0 * log10(255.0 * 255.0 * frameCount / vm_avg.Y_YUV);

	json::Array jsonPerFramePsnr;

	for (double p : perFramePsnr)
		jsonPerFramePsnr.Insert(json::Number(p));

	json::Object j;
	j["per_frame_psnr"] = jsonPerFramePsnr;
	j["avg_psnr"] = json::String(std::to_string(avgPsnrY));

	std::stringstream ss;
	json::Writer::Write(j, statFile);

	return 0;
}

static std::vector<uint8_t> CreateVideoFrame(size_t width, size_t height, size_t seed)
{
	std::vector<uint8_t> result(width * height * 3 / 2);

	for (size_t i = 0; i < result.size(); i++)
		result[i] = (i + seed) % 256;

	return result;
}

int test_record(std::vector<std::string> args)
{
	std::string inputFileName;
	std::string outputFileName;

	for (int i = 0; i < args.size(); i++)
	{
		std::string command = args[i];

		if (command == "-i")
		{
			inputFileName = args[i + 1];
			i++;
		}
		else if (command == "-o")
		{
			outputFileName = args[i + 1];
			i++;
		}
	}

	VSVideoFileReader inFile;
	VSVideoFile::SVideoInfo inVideoInfo;

	if (!inFile.Init(inputFileName))
	{
		std::cerr << "failed open " << inputFileName << std::endl;
		return -1;
	}

	if (!inFile.GetVideoFormat(inVideoInfo))
		return -1;

	VSVideoFileWriter outFile;
	VSVideoFile::SVideoInfo outVideoInfo = inVideoInfo;

	if (!outFile.Init(outputFileName))
	{
		std::cerr << "failed open " << outputFileName << std::endl;
		return -1;
	}

	if (!outFile.SetVideoFormat(outVideoInfo))
		return -1;

	if (!outFile.WriteHeader())
		return -1;

	std::vector<uint8_t> inBuffer(inVideoInfo.Width * inVideoInfo.Height * 3 / 2);
	int readedSize = 0;
	int frameCount = 0;

	do
	{
		bool isKey = false;
		readedSize = inFile.ReadVideo(inBuffer.data(), &isKey);

		if (readedSize > 0)
		{
			std::cout << "frame : " << frameCount << " size : " << readedSize << std::endl;
			if (!outFile.WriteVideo((char*)inBuffer.data(), readedSize, isKey))
			{
				std::cerr << "failed outFile.WriteVideo" << std::endl;
				return -1;
			}
		}

		frameCount++;
	} while (readedSize > 0);

	outFile.Release();

	return 0;
}
