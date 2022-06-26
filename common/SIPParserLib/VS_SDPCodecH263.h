#pragma once

#include "VS_SDPCodec.h"

#include <boost/regex.hpp>

class VS_SDPCodecH263: public VS_SDPCodec
{
public:
	const static boost::regex e1;
	const static boost::regex e2;
	const static boost::regex e3;
	const static boost::regex e4;
	const static boost::regex e5;
	const static boost::regex e6;
	const static boost::regex e7;
	const static boost::regex e8;
	const static boost::regex e9;

	VS_SDPCodecH263();
	VS_SDPCodecH263(const VS_SDPCodecH263&);

	VS_SDPCodecH263& operator=(const VS_SDPCodecH263&) = delete;

	std::unique_ptr<VS_SDPCodec> Clone() const override;

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;

	void FillRcvVideoMode(VS_GatewayVideoMode &mode) const override;
	void FillSndVideoMode(VS_GatewayVideoMode &mode) const override;

	void SetSQCIF(int sqcif);
	int GetSQCIF() const;

	void SetCIF(int cif);
	int GetCIF() const;

	void SetCIF4(int cif4);
	int GetCIF4() const;

	void SetCIF16(int cif16);
	int GetCIF16() const;

	void SetQCIF(int qcif);
	int GetQCIF() const;

	void SetAnnexF(bool is_true);
	bool GetAnnexF() const;

	void SetAnnexI(bool is_true);
	bool GetAnnexI() const;

	void SetAnnexJ(bool is_true);
	bool GetAnnexJ() const;

	void SetAnnexT(bool is_true);
	bool GetAnnexT() const;

private:
	int m_SQCIF;
	int m_QCIF;
	int m_CIF;
    int m_CIF4;
	int m_CIF16;

	int m_AnnexF;
	int m_AnnexI;
	int m_AnnexJ;
	int m_AnnexT;

	bool FindParam_SQCIF(string_view _in);
	bool FindParam_CIF(string_view _in);
	bool FindParam_CIF4(string_view _in);
	bool FindParam_QCIF(string_view _in);

	bool FindParam_AnnexF(string_view _in);
	bool FindParam_AnnexI(string_view _in);
	bool FindParam_AnnexJ(string_view _in);
	bool FindParam_AnnexT(string_view _in);
};