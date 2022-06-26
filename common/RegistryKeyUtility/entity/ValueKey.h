#pragma once
#include <string>

class ValueKey
{
public:
	enum class ValueType
	{
		non_type = -1,
		i32,
		i64,
		str,
		bin,
		b64
	};

	struct ValueName
	{
		std::string keyName;
		std::string valueName;
	};

	ValueKey() = default;
	ValueKey(ValueName name, const ::ValueKey::ValueType type, std::string value);


	static const char *ValueTypeToStr(const ValueType type);
	static ValueType StrToValueType(const char *type);

	void StreamShowData(std::ostream &) const;

	const ValueName& GetName() const;
	void SetName(ValueName name);

	const ValueType& GetType() const;
	void SetType(const ValueType type);
	const std::string& GetValue() const;
	void SetValue(std::string value);
	void SetKeyName(std::string kName);
	void SetValueName(std::string vName);
private:
	ValueName name_;
	ValueType type_;
	std::string value_;
};