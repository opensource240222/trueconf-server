#pragma once

#include "../SIPParserBase/VS_ObjectFactory.h"
#include "VS_SDPCodec.h"
#include "std-generic/cpplib/StrCompare.h"

#include <boost/regex.hpp>
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"

class VS_BaseField;
class VS_SIPBuffer;
//class VS_SDPCodec;

enum class SDPMediaType : int;
enum eSDPPayloadType;
enum eSDP_Bandwidth;

/*********************************************
 * Singleton
 *********************************************/

class VS_SDPObjectFactory: public VS_ObjectFactory
{
public:
	const static boost::regex e;

	enum class SDPHeader
	{
		SDPHeader_Invalid = 0,
		SDPHeader_Version,
		SDPHeader_Connection,
		SDPHeader_Bandwidth,
		SDPHeader_Origin,
		SDPHeader_SessionName,
		SDPHeader_Time,
		SDPHeader_MediaStream,
		SDPHeader_Attribute
	};

	struct CreateCodecUniqueResult
	{
		std::unique_ptr<VS_SDPCodec> p_field;
		TSIPErrorCodes error_code = TSIPErrorCodes::e_null;

		CreateCodecUniqueResult(std::unique_ptr<VS_SDPCodec>&& arg_p_field, TSIPErrorCodes arg_error_code)
			:p_field(std::move(arg_p_field))
			, error_code(arg_error_code)
		{}
	};

	struct CreateCodecSharedResult
	{
		std::shared_ptr<const VS_SDPCodec> p_field = nullptr;
		TSIPErrorCodes error_code = TSIPErrorCodes::e_null;

		CreateCodecSharedResult(const std::shared_ptr<const VS_SDPCodec>& arg_p_field, TSIPErrorCodes arg_error_code)
			:p_field(arg_p_field)
			, error_code(arg_error_code)
		{}
	};


	static VS_SDPObjectFactory* Instance();

	CreateFieldResult CreateField(VS_SIPBuffer &aBuffer) const override;
	CreateFieldResult CreateField(VS_SDPObjectFactory::SDPHeader header) const;
	static eSDP_Bandwidth GetBandwidthType(const char* type);
	static const char* GetBandwidthType(const eSDP_Bandwidth type);
	bool IsFieldOfType(const VS_BaseField *field, const VS_SDPObjectFactory::SDPHeader header) const;
	virtual ~VS_SDPObjectFactory();

	/////// For Codecs ////////

	//int GetCodecByEncodingName(const char* name, const unsigned clock_rate = 0) const;
	CreateCodecUniqueResult CreateCodec(SDPMediaType media_type, int codec_type);
	//int GetCodecByStaticPayloadType(const SDPMediaType media_type, const eSDPPayloadType pt) const;
	CreateCodecSharedResult CreateCodec(string_view name);

	//////////////////////////

protected:
	VS_SDPObjectFactory();
private:
	static VS_SDPObjectFactory * iThis;

	TSIPErrorCodes Init();
	static const char* GetHeader(const VS_SDPObjectFactory::SDPHeader header);
	//bool GetCodecInfoByName(const char *name, SDPMediaType &mt, int &codec, int &clockRate, eSDPPayloadType &PT, char *&encodingName) const;
	//bool GetCodecInfo(SDPMediaType mt, int codec, int &clockRate, eSDPPayloadType &PT, char *&encodingName) const;

	vs::map<std::string, std::shared_ptr<const VS_SDPCodec>, vs::str_less> sdp_codec_container;
};