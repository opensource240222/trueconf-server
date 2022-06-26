#pragma once

#include <vector>
#include <cstdlib>
#include "../../../tools/H323Gateway/Lib/src/VS_AsnBuffers.h"

struct VS_H323_GenericParameter
{
    VS_H323_GenericParameter() : paramIndex(0), paramValue(0), paramType(0) {}
	std::uint32_t paramIndex;
    std::uint32_t paramValue;
	unsigned paramType;
};

struct VS_H245GenericCapability;
struct VS_H245GenericParameter;
class VS_AcsLog;

struct VS_H323_GenericCapabilityGenerator
{
    VS_H323_GenericCapabilityGenerator();
    ~VS_H323_GenericCapabilityGenerator();
    /////////////////////////////////////////////////////////////////////////////////////
    VS_H245GenericCapability * Generate();
    std::size_t GenerateParametrsArray(const bool &isFromCollapsing,
                                    void *& array);
    /////////////////////////////////////////////////////////////////////////////////////
    VS_H323_GenericCapabilityGenerator (const std::uint32_t * id, const std::size_t len = 0);
	explicit VS_H323_GenericCapabilityGenerator(const VS_AsnObjectId &oid);

    bool TestID                         (const std::uint32_t * id);
    static void ShowGenericCapability   ( VS_H245GenericCapability * gcap,
                                            VS_AcsLog * tLog);
    bool AddStandartIntParametr (const bool isToCollapsing,
                                 const unsigned type,
                                 std::uint32_t value,
                                 std::size_t index = 0);
    bool SetMaxBitRate          (const uint32_t &maxbitrate);
	bool SetCapabilityId(const std::uint32_t * id, const std::size_t len = 0);
    bool TestStandartParameter  (const unsigned &type,
                                 const std::uint32_t &value) const;
	std::size_t identifier_len;
	std::uint32_t identifier[32];
    std::uint32_t maxBitRate;

	typedef std::vector<VS_H323_GenericParameter> TYPE_PArameterContainer;
    TYPE_PArameterContainer collapsingCont;
    TYPE_PArameterContainer nonCollapsingCont;
	std::size_t i;
protected:
    std::size_t GenerateParametrsArrayAct(TYPE_PArameterContainer & cont,
                                    void *& array);
};
/////////////////////////////////////////////////////////////////////////////////////////
