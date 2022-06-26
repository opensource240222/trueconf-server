#include "VS_RTPModuleParameters.h"
#include "../FrameFilterLib/Base/TraceLog.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_RegistryConst.h"
#include "../std/debuglog/VS_Debug.h"
#include "std/cpplib/VS_Utils.h"
#include "std/Globals.h"
#include "std/VS_TransceiverInfo.h"

#include <cstdlib>
#include <cstring>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_RTPModuleParameters::VS_RTPModuleParameters()
	: language("en")
	, slide_show_duration_s(0)
	, gconf_width(640)
	, gconf_height(360)
	, in_min_keyframe_interval_s(2)
	, in_max_keyframe_interval_s(15)
	, slide_interval_us(1000000)
	, slide_dimension(1280)
	, slide_quality(40)
	, ffl_trace_flags(0)
	, force_transcoding(false)
	, in_video_transcoding(true)
	, use_uniform_transmit(true)
	, content_as_slides(true)
{
	Update();
}

static std::pair<unsigned, unsigned> ParseResolution(const char* value)
{
	if (std::strcmp(value, "360p") == 0)
		return {640, 360};
	if (std::strcmp(value, "720p") == 0)
		return {1280, 720};
	if (std::strcmp(value, "1080p") == 0)
		return {1920, 1080};

	auto sep = std::strpbrk(value, "xX ");
	if (!sep)
		return {0, 0};

	char* p_end;
	unsigned w = std::strtoul(value, &p_end, 10);
	if (w < 64 || p_end != sep)
		return {0, 0};
	unsigned h = std::strtoul(sep + 1, &p_end, 10);
	if (h < 64 || p_end != value + std::strlen(value))
		return {0, 0};

	return {w, h};
}

void VS_RTPModuleParameters::Update()
{
	VS_RegistryKey key_Configuration(false, CONFIGURATION_KEY);
	VS_RegistryKey key_WebManager(false, "WebManager");
	VS_RegistryKey key_Transcoders(false, TRANSCODERS_KEY);
	VS_RegistryKey key_AppProperties(false, "AppProperties");

	log_dir = vs::GetLogDirectory() + ts::LOG_DIRECTORY_NAME;
	dstream4 << "VS_RTPModuleParameters: loaded work_dir=\"" << log_dir << "\"";
	{
		if (key_WebManager.GetString(language, "Language"))
			dstream4 << "VS_RTPModuleParameters: loaded language=\"" << language << "\"";
		else
			language = "en";
	}
	{
		if (key_Configuration.GetString(background_image_path, "SipBackgroundPath"))
			dstream4 << "VS_RTPModuleParameters: loaded background_image_path=\"" << background_image_path << "\"";
		else
			background_image_path.clear();
	}
	{
		long value(0);
		if (key_Transcoders.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "Force Transcoding") > 0)
		{
			force_transcoding = value != 0;
			dstream4 << "VS_RTPModuleParameters: loaded force_transcoding=" << std::boolalpha << force_transcoding;
		}
		else
			force_transcoding = false;
	}
	{
		std::string value;
		if (key_Configuration.GetString(value, "RTP GroupConf Resolution"))
		{
			std::tie(gconf_width, gconf_height) = ParseResolution(value.c_str());
			auto ds = dstream4;
			ds << "VS_RTPModuleParameters: loaded gconf_resolution=\"" << value << "\" -> ";
			if (gconf_width && gconf_height)
				ds << "width=" << gconf_width << ", height=" << gconf_height;
			else
			{
				ds << "invalid";
				gconf_width = 640;
				gconf_height = 360;
			}

		}
		else
		{
			gconf_width = 640;
			gconf_height = 360;
		}
	}
	{
		int32_t value;
		if (key_Configuration.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "RTP Input Video Transcoding") > 0)
		{
			in_video_transcoding = value != 0;
			dstream4 << "VS_RTPModuleParameters: loaded in_video_transcoding=" << std::boolalpha << in_video_transcoding;
		}
		else
			in_video_transcoding = true;
	}
	{
		if (key_Configuration.GetValue(&in_min_keyframe_interval_s, sizeof(in_min_keyframe_interval_s), VS_REG_INTEGER_VT, "RTP Input Min Keyframe Interval") > 0)
		{
			dstream4 << "VS_RTPModuleParameters: loaded in_min_keyframe_interval_s=" << in_min_keyframe_interval_s;
		}
		else
			in_min_keyframe_interval_s = 2;
	}
	{
		if (key_Configuration.GetValue(&in_max_keyframe_interval_s, sizeof(in_max_keyframe_interval_s), VS_REG_INTEGER_VT, "RTP Input Max Keyframe Interval") > 0)
		{
			dstream4 << "VS_RTPModuleParameters: loaded in_max_keyframe_interval_s=" << in_max_keyframe_interval_s;
		}
		else
			in_max_keyframe_interval_s = 15;
	}
	{
		if (key_Configuration.GetValue(&ffl_trace_flags, sizeof(ffl_trace_flags), VS_REG_INTEGER_VT, "RTP Module Trace") > 0)
		{
			dstream4 << "VS_RTPModuleParameters: loaded ffl_trace_flags=0x" << std::hex << ffl_trace_flags;
		}
		else
			ffl_trace_flags = ffl::TraceLog::TRACE_FORMATS | ffl::TraceLog::TRACE_DELETION | ffl::TraceLog::TRACE_MESSAGES | ffl::TraceLog::PRINT_ALL;
	}
	{
		long value(0);
		if (key_Configuration.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "Use UniformTransmit") > 0)
		{
			use_uniform_transmit = value != 0;
			dstream4 << "VS_RTPModuleParameters: loaded use_uniform_transmit=" << std::boolalpha << use_uniform_transmit;
		}
		else
			use_uniform_transmit = true;
	}
	{
		long value(0);
		if (key_Configuration.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "Content As Slides") > 0)
		{
			content_as_slides = value != 0;
			dstream4 << "VS_RTPModuleParameters: loaded content_as_slides=" << std::boolalpha << content_as_slides;
		} else
			content_as_slides = true;
	}
	{
		if (key_AppProperties.GetString(slide_upload_url, "slideshow2_upload_url"))
			dstream4 << "VS_RTPModuleParameters: loaded slide_upload_url=\"" << slide_upload_url << "\"";
		else
			slide_upload_url.clear();
	}
	{
		if (key_Configuration.GetValue(&slide_interval_us, sizeof(slide_interval_us), VS_REG_INTEGER_VT, "Gateway Slide Interval"))
		{
			dstream4 << "VS_RTPModuleParameters: loaded slide_interval_us=" << slide_interval_us;
		}
		else
			slide_interval_us = 1000000;
	}
	{
		if (key_Configuration.GetValue(&slide_dimension, sizeof(slide_dimension), VS_REG_INTEGER_VT, "Gateway Slide Dimension"))
		{
			dstream4 << "VS_RTPModuleParameters: loaded slide_dimension=" << slide_dimension;
		}
		else
			slide_dimension = 1280;
	}
	{
		if (key_Configuration.GetValue(&slide_quality, sizeof(slide_quality), VS_REG_INTEGER_VT, "Gateway Slide Quality"))
		{
			dstream4 << "VS_RTPModuleParameters: loaded slide_quality=" << slide_quality;
		}
		else
			slide_quality = 40;
	}
	UpdateSlideshow();
}

void VS_RTPModuleParameters::UpdateSlideshow()
{
	VS_RegistryKey key_Configuration(false, CONFIGURATION_KEY);
	{
		if (key_Configuration.GetValue(&slide_show_duration_s, sizeof(slide_show_duration_s), VS_REG_INTEGER_VT, "SIP Slideshow Duration") > 0)
		{
		}
		else
			slide_show_duration_s = 0;
	}
}
