#pragma once
#include "CommandItem.h"
#include "../entity/ValueKey.h"

class VS_RegistryKey;
enum RegistryVT : int;


class RegistryKeyCommand : public CommandItem
{
public:
	void CompletedExecute() override;

public:
	static ValueKey::ValueType RegistryVtToValueType(const RegistryVT vt);

protected:
	bool SetValue(const ValueKey &value, VS_RegistryKey &regKey) const;
};