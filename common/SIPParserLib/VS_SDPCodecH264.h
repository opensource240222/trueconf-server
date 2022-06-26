#pragma once
#include <boost/regex.hpp>
#include "VS_SDPCodec.h"

class VS_SDPCodecH264: public VS_SDPCodec
{
	const static char *profile_level_id_base;

	int m_mbps;
	int m_fs;
	int m_br;
	int m_level;

	void FindParam_profile_level_id(string_view);
	void FindParam_mbps(string_view);
	void FindParam_fs(string_view);
	void FindParam_br(string_view);


public:
	const static boost::regex e;
	const static boost::regex e1;
	const static boost::regex e2;
	const static boost::regex e3;

	VS_SDPCodecH264();
	VS_SDPCodecH264(const VS_SDPCodecH264&);
	~VS_SDPCodecH264();

	VS_SDPCodecH264& operator=(const VS_SDPCodecH264&) = delete;

	std::unique_ptr<VS_SDPCodec> Clone() const override;

	static void GetCommonCodec(const VS_SDPCodecH264 &src1, const VS_SDPCodecH264 &src2, VS_SDPCodecH264 &result);

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;

	void FillRcvVideoMode(VS_GatewayVideoMode &mode) const override;
	void FillSndVideoMode(VS_GatewayVideoMode &mode) const override;

	int Level() const;
	void LimitLevel(int level);
	int MaxMbps() const;
	int MaxFs() const;
	int MaxBr() const;
};