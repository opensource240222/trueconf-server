#pragma once

#include "VS_SDPCodec.h"

#include <boost/regex.hpp>

class VS_SDPCodecH261: public VS_SDPCodec
{
public:
	const static boost::regex e1;
	const static boost::regex e2;
	const static boost::regex e3;

	VS_SDPCodecH261();
	VS_SDPCodecH261(const VS_SDPCodecH261 &);
	~VS_SDPCodecH261();

	VS_SDPCodecH261 & operator=(const VS_SDPCodecH261 &);

	std::unique_ptr<VS_SDPCodec> Clone() const override;

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;

	void FillRcvVideoMode(VS_GatewayVideoMode &mode) const override;
	void FillSndVideoMode(VS_GatewayVideoMode &mode) const override;

	void SetCIF(int cif);
	int GetCIF() const;

	void SetQCIF(int qcif);
	int GetQCIF() const;

	void SetD(int d);
	int GetD() const;

private:
	int m_CIF;
	int m_QCIF;
	int m_D;

	bool FindParam_CIF(string_view _in);
	bool FindParam_QCIF(string_view _in);
	bool FindParam_D(string_view _in);
	bool operator!=(const VS_SDPCodecH261 &)const;
};