#include "VS_RTSP_Date.h"

#include "std-generic/compat/memory.h"
#include <algorithm>
#include <cstring>

std::unique_ptr<VS_BaseField> VS_RTSP_Date_Instance()
{
	return vs::make_unique<VS_RTSP_Date>();
};
TSIPErrorCodes VS_RTSP_Date::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> cpInput;
	std::size_t size = 0;
	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(cpInput, size);
	if (TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;

	}
	std::transform(cpInput.get(), cpInput.get() + size, cpInput.get(), ::toupper);
	const char * iter = strstr(cpInput.get(), "DATE:");
	if (cpInput == nullptr)
	{
		SetError(TSIPErrorCodes::e_InputParam);
		SetValid(false);
		return TSIPErrorCodes::e_InputParam;
	}
	iter += 6;
	m_date.assign(iter, size - (iter - cpInput.get()));
	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_RTSP_Date::Encode(VS_SIPBuffer &aBuffer) const
{
	return TSIPErrorCodes::e_ok;
}