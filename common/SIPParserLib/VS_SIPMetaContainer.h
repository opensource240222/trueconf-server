#pragma once
#include "../SIPParserBase/VS_SIPError.h"
#include "../SIPParserBase/VS_BaseField.h"

#include <memory>
#include <vector>

class VS_SIPMetaContainer: public VS_SIPError
{
public:
	void AddField(std::unique_ptr<VS_BaseField>&& aBaseField);
	TSIPErrorCodes GetField(std::size_t aIndex, VS_BaseField* &aBaseField) const;
	std::size_t EraseField(std::size_t aIndex);
	void GetSize(std::size_t &aSize) const;
	void Clear() noexcept;

	VS_SIPMetaContainer();
	~VS_SIPMetaContainer();

private:
	std::vector<std::unique_ptr<VS_BaseField> > iContainer;
};