#include "VS_SIPError.h"

VS_SIPError::VS_SIPError()
{
	VS_SIPError::Clean();
}


VS_SIPError::VS_SIPError(TSIPErrorCodes aError, bool aValid):
	iValid(aValid), iError(aError)
{

}

TSIPErrorCodes VS_SIPError::GetLastClassError() const
{
	return iError;
}

bool VS_SIPError::IsValid() const
{
	return iValid;
}

void VS_SIPError::SetError(TSIPErrorCodes aError)
{
	iError = aError;
}

void VS_SIPError::SetValid(bool aValid)
{
	iValid = aValid;
}

void VS_SIPError::Clean() noexcept
{
	iValid = false;
	iError = TSIPErrorCodes::e_null;
}

bool VS_SIPError::CorrectNewLine( const char* aInput, const std::size_t aSize, std::string &buffer ) const
{
	bool make_copy = aInput[0] == '\n';

	for (std::size_t i = 1; !make_copy && i < aSize; i++)
	{
		if ( aInput[i] == '\n' && aInput[i - 1] != '\r')
			make_copy = true;
	}

	if (!make_copy) return false;

	buffer.clear();

	for (std::size_t i = 0; i < aSize; i++)
	{
		if ( aInput[i] == '\n' && (i == 0 || aInput[i - 1] != '\r') )
			buffer.push_back('\r');
		buffer.push_back( aInput[ i ] );
	}

	return true;
}

VS_SIPError& VS_SIPError::operator=(const VS_SIPError& src)
{
	if (this == &src)
	{
		return *this;
	}

	if (*this != src)
	{
		iValid = src.iValid;
		iError = src.iError;
	}
	return *this;
}

bool VS_SIPError::operator!=(const VS_SIPError& other)const
{
	return (iValid != other.iValid) || (iError != other.iError);
}


VS_SIPError::VS_SIPError(const VS_SIPError& src)
{
	iValid = src.iValid;
	iError = src.iError;
}