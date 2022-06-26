#include "VS_SDPObjectFactory.h"
#include "VS_SDPCodecH261.h"
#include "VS_SDPCodecH263.h"
#include "VS_SDPCodecH264.h"
#include "VS_SDPCodecXH264UC.h"
#include "VS_SDPCodecG711A.h"
#include "VS_SDPCodecG711U.h"
#include "VS_SDPCodecG722.h"
#include "VS_SDPCodecG723.h"
#include "VS_SDPCodecG728.h"
#include "VS_SDPCodecG729A.h"
#include "VS_SDPCodecSpeex.h"
#include "VS_SDPCodecG722_1.h"
#include "VS_SDPCodecSiren14.h"
#include "VS_SDPCodecOpus.h"
#include "VS_SDPCodecMPEG4ES.h"
#include "VS_SDPCodecMPA.h"
#include "VS_SDPCodecH224.h"
#include "VS_SDPTelEvent.h"
#include "VS_SDPField_Version.h"
#include "VS_SDPField_Connection.h"
#include "VS_SDPField_Bandwidth.h"
#include "VS_SDPField_Origin.h"
#include "VS_SDPField_SessionName.h"
#include "VS_SDPField_Time.h"
#include "VS_SDPField_MediaStream.h"
#include "VS_SDPField_Attribute.h"
#include "../tools/Server/CommonTypes.h"
#include "std/debuglog/VS_Debug.h"
#include <memory>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

VS_SDPObjectFactory* VS_SDPObjectFactory::iThis = nullptr;
const boost::regex VS_SDPObjectFactory::e(" *([[:word:]-]+) *");

VS_SDPObjectFactory::VS_SDPObjectFactory()
{

}

VS_SDPObjectFactory::~VS_SDPObjectFactory()
{
	if ( iThis )
		iThis = 0;
}

VS_SDPObjectFactory* VS_SDPObjectFactory::Instance()
{
	if ( !iThis )
	{
		iThis = new VS_SDPObjectFactory;

		TSIPErrorCodes res = iThis->Init();

		if ( res != TSIPErrorCodes::e_ok )
		{
			iThis->SetError(res);
			iThis->SetValid(false);
		}
	}

	return iThis;
}

TSIPErrorCodes VS_SDPObjectFactory::Init()
{
// Version
	TSIPErrorCodes res = AddClass("V", &VS_SDPField_Version_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Connection
	res = AddClass("C", &VS_SDPField_Connection_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Bandwidth
	res = AddClass("B", &VS_SDPField_Bandwidth_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Origin
	res = AddClass("O", &VS_SDPField_Origin_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// SessionName
	res = AddClass("S", &VS_SDPField_SessionName_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Time
	res = AddClass("T", &VS_SDPField_Time_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Media Stream
	res = AddClass("M", &VS_SDPField_MediaStream_Instance);
	if (TSIPErrorCodes::e_ok !=  res)
		return res;

// Attribute
	res = AddClass("A", &VS_SDPField_Attribute_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;

	//TODO: Добавить все филды VS_SDP...Field

	return res;
}

VS_ObjectFactory::CreateFieldResult  VS_SDPObjectFactory::CreateField(VS_SIPBuffer &aBuffer) const
{
	std::unique_ptr<const char[]> data;
	std::size_t theSize;
	TSIPErrorCodes res = aBuffer.GetHeaderAllocConst("=", data, theSize);
	const char* Header = data.get();
	if (TSIPErrorCodes::e_ok != res)
	{
		return CreateFieldResult(nullptr, res);
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(Header, m, e);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPObjectFactory::CreateField error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		return CreateFieldResult(nullptr, TSIPErrorCodes::e_match);
	}

	const std::string &theHeader = m[1];
	const VS_ObjectFactory::GetInstanceResult get_inst_res = GetInstance(theHeader.c_str());
	if (TSIPErrorCodes::e_ok != get_inst_res.error_code )
		return CreateFieldResult(nullptr, get_inst_res.error_code);

	return CreateFieldResult((*get_inst_res.instance)(), TSIPErrorCodes::e_ok);
}

const char* VS_SDPObjectFactory::GetHeader(const VS_SDPObjectFactory::SDPHeader header)
{
	switch (header)
	{
	case SDPHeader::SDPHeader_Version:			return "V";
	case SDPHeader::SDPHeader_Connection:		return "C";
	case SDPHeader::SDPHeader_Bandwidth:		return "B";
	case SDPHeader::SDPHeader_Origin:			return "O";
	case SDPHeader::SDPHeader_SessionName:		return "S";
	case SDPHeader::SDPHeader_Time:			    return "T";
	case SDPHeader::SDPHeader_MediaStream:		return "M";
	case SDPHeader::SDPHeader_Attribute:		return "A";
	default: return nullptr;
	}
}
//
//int VS_SDPObjectFactory::GetHeader(char* header) const
//{
//	return SDPHeader::SDPHeader_Invalid;
//}

VS_ObjectFactory::CreateFieldResult  VS_SDPObjectFactory::CreateField(VS_SDPObjectFactory::SDPHeader header) const
{
	auto h = this->GetHeader(header);

	if (!h)
		return CreateFieldResult(nullptr, TSIPErrorCodes::e_InputParam);

	return VS_ObjectFactory::CreateField(h);
}

VS_SDPObjectFactory::CreateCodecUniqueResult VS_SDPObjectFactory::CreateCodec(SDPMediaType media_type, int codec_type)
{
	std::unique_ptr<VS_SDPCodec> codec;
	switch (media_type)
	{
	case SDPMediaType::audio:
		switch (codec_type)
		{
		case e_rcvG711Ulaw64k: codec = vs::make_unique<VS_SDPCodecG711U>(); break;
		case e_rcvG711Alaw64k: codec = vs::make_unique<VS_SDPCodecG711A>(); break;
		case e_rcvG722_64k:    codec = vs::make_unique<VS_SDPCodecG722>(); break;
		case e_rcvG723:        codec = vs::make_unique<VS_SDPCodecG723>(); break;
		case e_rcvG728:        codec = vs::make_unique<VS_SDPCodecG728>(); break;
		case e_rcvG729a:       codec = vs::make_unique<VS_SDPCodecG729A>(); break;
		case e_rcvSPEEX_16kHz: codec = vs::make_unique<VS_SDPCodecSpeex>(); break;
		case e_rcvG722124:     codec = vs::make_unique<VS_SDPCodecG722_1>(); break;
		case e_rcvG722132:     codec = vs::make_unique<VS_SDPCodecG722_1>(32000); break;
		case e_rcvSIREN14_24:  codec = vs::make_unique<VS_SDPCodecSiren14>(24000); break;
		case e_rcvSIREN14_32:  codec = vs::make_unique<VS_SDPCodecSiren14>(32000); break;
		case e_rcvSIREN14_48:  codec = vs::make_unique<VS_SDPCodecSiren14>(48000); break;
		case e_rcvOPUS:        codec = vs::make_unique<VS_SDPCodecOpus>(); break;
		case e_rcvMPA:         codec = vs::make_unique<VS_SDPCodecMPA>(); break;
		case c_fakeMPEG4ES:    codec = vs::make_unique<VS_SDPCodecMPEG4ES>(""); break;
		case e_rcvTelEvent:    codec = vs::make_unique<VS_SDPTelEvent>(); break;
		case e_rcvNone:
		default: break;
		} break;
	case SDPMediaType::video:
		switch (static_cast<VS_H323VideoCodec>(codec_type))
		{
		case e_videoH261:      codec = vs::make_unique<VS_SDPCodecH261>(); break;
		case e_videoH263:
		case e_videoH263plus:
		case e_videoH263plus2: codec = vs::make_unique<VS_SDPCodecH263>(); break;
		case e_videoH264:      codec = vs::make_unique<VS_SDPCodecH264>(); break;
		case e_videoXH264UC:   codec = vs::make_unique<VS_SDPCodecXH264UC>(); break;
		case e_videoNone:
		default: break;
		} break;
	case SDPMediaType::application_fecc:
		switch (static_cast<VS_H323DataCodec>(codec_type))
		{
		case VS_H323DataCodec::FECC: codec = vs::make_unique<VS_SDPCodecH224>(); break;
		case VS_H323DataCodec::dataNone:
		default:	break;
		} break;

	default: break;
	}

	if (codec)
	{
		std::uint32_t clock_rate;
		eSDPPayloadType pt;
		const char* name;
		if (codec_type == c_fakeMPEG4ES)
		{
			// Special case.
			// Multiple codecs present themselves as MPEG-4 Elementary Stream, at this point we don't know real codec type.
		}
		else if (vs::GetCodecInfo(media_type, codec_type, clock_rate, pt, name))
		{
			codec->SetClockRate(clock_rate);
			codec->SetMediaType(media_type);
			codec->SetCodecType(codec_type);
			codec->SetPT(pt);
		}
		else
			codec = nullptr;
	}

	const auto error = codec ? TSIPErrorCodes::e_ok : TSIPErrorCodes::e_NoSupportedCodecs;
	return { std::move(codec), error };
}

VS_SDPObjectFactory::CreateCodecSharedResult VS_SDPObjectFactory::CreateCodec(string_view name)
{
	//если кодек с указанным именем есть в контейнере, возвращаем его
	if (name.empty())
	{
		return CreateCodecSharedResult(std::shared_ptr<const VS_SDPCodec>(), TSIPErrorCodes::e_InputParam);
	}

	auto element_with_name = sdp_codec_container.find(name);
	if (element_with_name != sdp_codec_container.end())
	{
		return CreateCodecSharedResult(element_with_name->second, TSIPErrorCodes::e_ok);
	}

	//если нет кодека с таким именем в контейнере, мы его создаем
	std::uint32_t clockRate(0);
	SDPMediaType mt = SDPMediaType::invalid;
	int codec(0);
	eSDPPayloadType pt(eSDPPayloadType::SDP_PT_INVALID);
	const char* sdpname = nullptr;

	const bool res = vs::GetCodecInfoByName( name, mt, codec, clockRate, pt, sdpname);
	if (!res) return CreateCodecSharedResult(std::shared_ptr<const VS_SDPCodec>(), TSIPErrorCodes::e_NoSupportedCodecs);

	auto create_codec_res = CreateCodec(mt, codec);
	if (create_codec_res.error_code != TSIPErrorCodes::e_ok)  return CreateCodecSharedResult(std::shared_ptr<const VS_SDPCodec>(), create_codec_res.error_code);

	create_codec_res.p_field->SetClockRate( clockRate );
	create_codec_res.p_field->SetCodecType( codec );
	if(sdpname) create_codec_res.p_field->SetEncodingName( sdpname );
	create_codec_res.p_field->SetPT( pt );
	create_codec_res.p_field->SetMediaType( mt );
	create_codec_res.p_field->SetValid(true);
	create_codec_res.p_field->SetError(TSIPErrorCodes::e_ok);

	return CreateCodecSharedResult(sdp_codec_container.emplace(name, std::move(create_codec_res.p_field)).first->second, TSIPErrorCodes::e_ok);
}

///////////////this methods aren't using (?), but if it will need in this class, realization will be next//////////////////////
//int VS_SDPObjectFactory::GetCodecByEncodingName(const char* name, const unsigned clock_rate) const
//{
//	return CommonCodecInfo::GetCodecByEncodingName(name, clock_rate);
//}
//
//bool VS_SDPObjectFactory::GetCodecInfo(SDPMediaType mt, int codec, int &clockRate, eSDPPayloadType &PT, char *&encodingName) const
//{
//	return CommonCodecInfo::GetCodecInfo(mt, codec, clockRate, PT, encodingName);
//}
//
//bool VS_SDPObjectFactory::GetCodecInfoByName(const char *_name, SDPMediaType &mt, int &codec,
//											 int &clockRate, eSDPPayloadType &PT, char *&encodingName) const
//{
//	return CommonCodecInfo::GetCodecInfoByName(_name, mt, codec, clockRate, PT, encodingName);
//}
//
//int VS_SDPObjectFactory::GetCodecByStaticPayloadType(const SDPMediaType media_type, const eSDPPayloadType pt) const
//{
//	return CommonCodecInfo::GetCodecByStaticPayloadType(media_type, pt);
//}

eSDP_Bandwidth VS_SDPObjectFactory::GetBandwidthType(const char* type)
{
	if (!type || *type == '\0')
		return SDP_BANDWIDTH_INVALID;

	if (strcmp("CT", type) == 0) {
		return SDP_BANDWIDTH_CT;
	}

	if (strcmp("AS", type) == 0) {
		return SDP_BANDWIDTH_AS;
	}

	if (strcmp("TIAS", type) == 0) {
		return SDP_BANDWIDTH_TIAS;
	}

	return SDP_BANDWIDTH_INVALID;
}

const char* VS_SDPObjectFactory::GetBandwidthType(const eSDP_Bandwidth type)
{
	if (type == SDP_BANDWIDTH_CT) {
		return "CT";
	}
	if (type == SDP_BANDWIDTH_AS) {
		return "AS";
	}
	if (type == SDP_BANDWIDTH_TIAS) {
		return "TIAS";
	}
	return nullptr;
}

bool VS_SDPObjectFactory::IsFieldOfType(const VS_BaseField *base_field, const VS_SDPObjectFactory::SDPHeader header) const
{
	auto h = GetHeader(header);

	GetInstanceResult get_inst_res = GetInstance(h);
	if (TSIPErrorCodes::e_ok != get_inst_res.error_code) {
		return false;
	}

	bool res = typeid(*((*(get_inst_res.instance))())) == typeid(*base_field);

	return res;
}

#undef DEBUG_CURRENT_MODULE
