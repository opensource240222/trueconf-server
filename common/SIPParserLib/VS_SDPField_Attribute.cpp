#include "VS_SDPField_Attribute.h"
#include "VS_SDPField_MediaStream.h"
#include "VS_SDPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPField_Attribute::e1("(?i) *a *= *(sendrecv|sendonly|recvonly|inactive) *(?-i)");

VS_SDPField_Attribute::VS_SDPField_Attribute(eType type) : type_(type), direction_(SDP_MEDIACHANNELDIRECTION_INVALID)
{

}

TSIPErrorCodes VS_SDPField_Attribute::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = aBuffer.GetNextBlockAllocConst(ptr, ptr_sz);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	if ( !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e1);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPField_Attribute::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}

	if (regex_match_res) {
		type_ = Direction;
		directionStr_ = m[1];
		std::transform(directionStr_.begin(), directionStr_.end(), directionStr_.begin(), ::tolower);
		if (directionStr_ == "sendrecv")
			direction_ = SDP_MEDIACHANNELDIRECTION_SENDRECV;
		else if (directionStr_ == "sendonly")
			direction_ = SDP_MEDIACHANNELDIRECTION_SEND;
		else if (directionStr_ == "recvonly")
			direction_ = SDP_MEDIACHANNELDIRECTION_RECV;
		else if (directionStr_ == "inactive")
			direction_ = SDP_MEDIACHANNELDIRECTION_INACTIVE;

	} else {
		dstream3 << "[SIPParserLib::SDPError] Attribute Field: buffer not match, dump |" << ptr.get() << "|";
		aBuffer.SkipHeader();
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	aBuffer.SkipHeader();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_Attribute::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("a=");

	if (type_ == Direction) {
		aBuffer.AddData(directionStr_);
	} else if (type_ == XMediaBW) {
		aBuffer.AddData("x-mediabw:");
		aBuffer.AddData(media_bw_str_);
	} else if (type_ == XDeviceCaps) {
		aBuffer.AddData("x-devicecaps:");
		aBuffer.AddData(device_caps_str_);
	}

	aBuffer.AddData("\r\n");
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_Attribute::Init(const VS_SIPGetInfoInterface& call)
{
	if (type_ == None) return TSIPErrorCodes::e_InputParam;

	if (type_ == XMediaBW) {
		media_bw_str_ = "main-video send=4000;recv=4000";
	} else if (type_ == XDeviceCaps) {
		bool audio_rcv = false, audio_snd = false,
			 video_rcv = false, video_snd = false;

		for (auto &&s : call.MediaStreams())
		{
			if (s->GetMediaType() == SDPMediaType::audio) {
				audio_rcv |= s->IsRecv();
				audio_snd |= s->IsSend();
			} else if (s->GetMediaType() == SDPMediaType::video) {
				video_rcv |= s->IsRecv();
				video_snd |= s->IsSend();
			}
		}

		if (audio_rcv || audio_snd) {
			device_caps_str_ += "audio:";
			if (audio_snd) {
				device_caps_str_ += "send";
				device_caps_str_ += audio_rcv ? ",recv;" : ";";
			} else {
				device_caps_str_ += "recv;";
			}
		}

		if (video_rcv || video_snd) {
			device_caps_str_ += "video:";
			if (video_snd) {
				device_caps_str_ += "send";
				device_caps_str_ += video_rcv ? ",recv" : "";
			} else {
				device_caps_str_ += "recv";
			}
		}
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SDPField_Attribute::Clean() noexcept
{
	VS_SIPError::Clean();

	direction_ = SDP_MEDIACHANNELDIRECTION_INVALID;
	directionStr_.clear();
}

std::unique_ptr<VS_BaseField> VS_SDPField_Attribute_Instance()
{
	return vs::make_unique<VS_SDPField_Attribute>();
}

#undef DEBUG_CURRENT_MODULE
