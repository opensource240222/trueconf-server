#pragma once
#include <string>
#include "VS_Const.h"

class VS_SIPError
{
public:
	VS_SIPError();
	VS_SIPError(const VS_SIPError& src);
	VS_SIPError(TSIPErrorCodes aError, bool aValid);
	virtual ~VS_SIPError() {}

	virtual void Clean() noexcept;

	TSIPErrorCodes GetLastClassError() const;
	bool IsValid() const;

	void SetError(TSIPErrorCodes aError);
	void SetValid(bool aValid);
	VS_SIPError& operator=(const VS_SIPError& src);
protected:
	bool CorrectNewLine( const char* aInput, const std::size_t aSize, std::string &buffer ) const;
	bool operator!=(const VS_SIPError& other)const;
private:
	bool iValid;
	TSIPErrorCodes iError;
};