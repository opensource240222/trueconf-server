#include "VS_H323CapabilityGenerator.h"
#include "src/VS_H245Messages.h"
#include "../../../acs/Lib/VS_AcsLog.h"

VS_H323_GenericCapabilityGenerator::VS_H323_GenericCapabilityGenerator()
    : identifier_len(0), identifier{}, maxBitRate(0), i(0)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
VS_H323_GenericCapabilityGenerator::~VS_H323_GenericCapabilityGenerator()
{
}
/////////////////////////////////////////////////////////////////////////////////////////
VS_H323_GenericCapabilityGenerator::VS_H323_GenericCapabilityGenerator(const std::uint32_t * id, const std::size_t len)
	: VS_H323_GenericCapabilityGenerator::VS_H323_GenericCapabilityGenerator()
{
    SetCapabilityId(id, len);
}

VS_H323_GenericCapabilityGenerator::VS_H323_GenericCapabilityGenerator(const VS_AsnObjectId &oid)
	: VS_H323_GenericCapabilityGenerator::VS_H323_GenericCapabilityGenerator(oid.value, oid.size)
{}

/////////////////////////////////////////////////////////////////////////////////////////
std::size_t VS_H323_GenericCapabilityGenerator::GenerateParametrsArrayAct(
    TYPE_PArameterContainer & cont, void *& array)
{
    TYPE_PArameterContainer::iterator it;

	std::unique_ptr<VS_H245GenericParameter[]> ret = vs::make_unique<VS_H245GenericParameter[]>(cont.size());

    for (it = cont.begin(), i = 0; it != cont.end(); ++it, ++i)
    {
        VS_H323_GenericParameter * param = &*it;

        TemplInteger<0,127> * pid = new TemplInteger<0,127>;
        if (pid)
        {
            if (param->paramIndex==0)
                pid->value = i+1;
            else
                pid->value = param->paramIndex;
            pid->filled = true;
            ret[i].parameterIdentifier.choice = pid;
            ret[i].parameterIdentifier.tag = VS_H245ParameterIdentifier::e_standard;
            ret[i].parameterIdentifier.filled = true;

            VS_Asn * value = nullptr;
            switch(param->paramType)
            {
            case VS_H245ParameterValue::e_booleanArray:
                {
                    value = static_cast<TemplInteger<0,255>*>
                        ( new TemplInteger<0,255>);
                    (static_cast<TemplInteger<0,255>*>(value))->value = param->paramValue;
                    (static_cast<TemplInteger<0,255>*>(value))->filled = true;
                }break;
            case VS_H245ParameterValue::e_unsignedMin:
            case VS_H245ParameterValue::e_unsignedMax:
                {
                    value = static_cast<TemplInteger<0,65535>*>
                        ( new TemplInteger<0,65535>);
                    (static_cast<TemplInteger<0,65535>*>(value))->value = param->paramValue;
                    (static_cast<TemplInteger<0,65535>*>(value))->filled = true;
                }break;
            case VS_H245ParameterValue::e_unsigned32Min:
            case VS_H245ParameterValue::e_unsigned32Max:
                {
                    value = static_cast<TemplInteger<0,4294967295>*>
                        ( new TemplInteger<0,4294967295>);
                    (static_cast<TemplInteger<0,4294967295>*>(value))->value = param->paramValue;
                    (static_cast<TemplInteger<0,4294967295>*>(value))->filled = true;
                }break;
            default: break;
            }
            if (!value)
            {
                return 0;
            }
            ret[i].parameterValue.tag = param->paramType;
            ret[i].parameterValue.choice = value;
            ret[i].parameterValue.filled = true;
            ret[i].filled = true;
        }
        else
        {
            return 0;
        }
    }
    if(i!=0)
    {
        array = ret.release();
        return i;
    }
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
std::size_t VS_H323_GenericCapabilityGenerator::GenerateParametrsArray(
        const bool &isFromCollapsing,
        void *& array)
{
    std::size_t ret = 0;
    if (isFromCollapsing)
    {
        if (!collapsingCont.empty())
        {
            ret = GenerateParametrsArrayAct( collapsingCont , array );
        }
    }
	else if (!nonCollapsingCont.empty())
    {
        ret = GenerateParametrsArrayAct( nonCollapsingCont , array );
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////
VS_H245GenericCapability * VS_H323_GenericCapabilityGenerator::Generate()
{
    if (!TestID(static_cast<const std::uint32_t*>(identifier))) return nullptr;
    VS_H245GenericCapability * ret = new VS_H245GenericCapability;
    if (!ret) return ret;
    VS_AsnObjectId * obj = new VS_AsnObjectId;
    if (obj!=nullptr)
    {
        obj->Clear();
        memcpy(obj->value,identifier,sizeof(identifier));
        obj->filled = true;

		if (identifier_len == 0) // default behavior - copy up to the lfirst zero element
		{
			unsigned int lastNotZero = 0;
			for (i = 0; i < 32; i++)
				if (identifier[i] != 0)
					lastNotZero = i;
			obj->size = (lastNotZero < 6) ? 6 : (lastNotZero + 1);
		}
		else
		{
			obj->size = identifier_len;
		}

        ret->capabilityIdentifier.tag = VS_H245CapabilityIdentifier::e_standard;

        ret->capabilityIdentifier.choice  = obj;
        ret->capabilityIdentifier.filled = true;
        ////
        if (maxBitRate>0)
        {
            ret->maxBitRate.filled = true;
            ret->maxBitRate.value = maxBitRate;
        }
        ////
        std::size_t length_c;
        ////
		std::size_t length_nc;
        ////
		void *data = nullptr;
        if ((length_c=GenerateParametrsArray( true , data )))
        {
			assert(data);
            ret->collapsing.reset(static_cast<decltype(ret->collapsing.data())>(data), length_c);
        }

		data = nullptr;
        if ((length_nc = GenerateParametrsArray( false , data )))
        {
			assert(data);
            ret->nonCollapsing.reset(static_cast<decltype(ret->collapsing.data())>(data), length_nc);
        }

        if (length_c || length_nc)
        {
            ret->filled= true;
            return ret;
        }
    }
    delete ret; ret = nullptr;
    return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_H323_GenericCapabilityGenerator::TestStandartParameter
    (const unsigned &type,
    const std::uint32_t &value) const
{
    switch(type)
    {
    case VS_H245ParameterValue::e_booleanArray:
        {
            if (value<=255)
                return true;
        }break;
    case VS_H245ParameterValue::e_unsignedMin:
    case VS_H245ParameterValue::e_unsignedMax:
        {
            if (value<=65535)
                return true;
        }break;
    case VS_H245ParameterValue::e_unsigned32Min:
    case VS_H245ParameterValue::e_unsigned32Max:
        {
            return true;
        }break;
    default: break;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_H323_GenericCapabilityGenerator::AddStandartIntParametr
        (const bool isToCollapsing,
         const unsigned type,
         std::uint32_t value,
         std::size_t index)
{
    TYPE_PArameterContainer * cont = nullptr;
    if (isToCollapsing)
    {
        cont = &collapsingCont;
    }
    else
    {
        cont = &nonCollapsingCont;
    }
    if (TestStandartParameter( type,value))
    {
        VS_H323_GenericParameter newParam;
        newParam.paramType = type;
        newParam.paramValue = value;
        newParam.paramIndex = index;
        cont->insert( cont->end(), newParam );
        return true;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_H323_GenericCapabilityGenerator::SetMaxBitRate(const uint32_t &maxbitrate)
{
    if (maxbitrate<(100*1024*1024))///100 Mbit/s
    {
        maxBitRate = maxbitrate;
        return true;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_H323_GenericCapabilityGenerator::TestID(const std::uint32_t * id)
{
    auto res = 0;
    for(i=0;i<32;i++)
        res+=id[i];
    return res != 0;
}
bool VS_H323_GenericCapabilityGenerator::SetCapabilityId(const std::uint32_t * id, const std::size_t len)
{
	std::size_t count;
	const std::size_t max_elements = (sizeof(identifier) / sizeof(identifier[0]));

    if (!TestID(id)) return false;

	if (len > max_elements)
	{
		count = max_elements;
	}
	else
	{
		count = (len == 0 ? max_elements : len);
	}

	if (len == 0)
		identifier_len = 0;
	else
		identifier_len = count;

    memcpy(identifier, id,  count * sizeof(identifier[0]));
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////
void VS_H323_GenericCapabilityGenerator::ShowGenericCapability
    ( VS_H245GenericCapability * gcap,
      VS_AcsLog * tLog)
{
    //Sleep(40000);

    tLog->CPrintf("\n\t GEN: %x ",gcap);
    if (!gcap) return;
    tLog->CPrintf("\n\t GENERIC CAPABILITY!!!");

    if (gcap->capabilityIdentifier.tag ==
        VS_H245CapabilityIdentifier::e_standard)
    {
		tLog->CPrintf("\n\t ID.TAG == standard");
        VS_AsnObjectId * protocol  =
            static_cast<VS_AsnObjectId*>
            (gcap->capabilityIdentifier.choice);
        unsigned int i=0;
        tLog->CPrintf("\n\t GEN.CAP.Protocol: ");
        for(;i<protocol->NValues();i++)
        tLog->CPrintf("%u|", protocol->value[i]);
        tLog->CPrintf("\n\t GEN.CAP.Prot: %u",protocol->value[3]);

        if (protocol->value[3]==7221)
        {
            VS_PerBuffer buff;
            if(!gcap->Encode(buff))
            {
                tLog->CPrintf("\n\t Error in if(!gcap->Encode(buff))!");
                //Sleep(30000);
                return;
            }else
            {

            }
        }

    }else if( gcap->capabilityIdentifier.tag ==
        gcap->capabilityIdentifier.e_h221NonStandard)
    {
        tLog->CPrintf("\n\t GEN.CAP.e_h221NonStandard");
        VS_H245NonStandardParameter * param =
            static_cast<VS_H245NonStandardParameter*>
            (gcap->capabilityIdentifier.choice);
        tLog->CPrintf("\n\t GEN.nonStandardIdentifier TAG = %d",
            param->nonStandardIdentifier.tag);
        std::size_t bytes = 0;
		std::size_t bits = 0;

        param->data.value.GetPositionSize( bytes,bits);
        tLog->CPrintf("\n\t GEN.DATA.SIZE = %u : %u",bytes,bits);
        if(bytes)
        {
            const auto val = new unsigned char[bytes+1];
            if(param->data.value.GetBits( val , bytes*8 ))
            {
				decltype(bytes) i = 0;
				for(i=0;i<bytes;i++)
                   tLog->CPrintf("%u|", val[i]);
            }
        }
        if (param->nonStandardIdentifier.tag==
            param->nonStandardIdentifier.e_h221NonStandard)
        {
            VS_H245NonStandardIdentifier_H221NonStandard * h221 =
                static_cast<VS_H245NonStandardIdentifier_H221NonStandard*>
                (param->nonStandardIdentifier.choice);
            if(!h221) return;
            h221->Show< VS_AcsLog >( tLog );

        }

    }  else
	{
        tLog->CPrintf("\n\t ID.TAG == %u",gcap->capabilityIdentifier.tag);
	}
    if (gcap->maxBitRate.filled)
    {
        tLog->CPrintf("\n\t GEN.CAP.MaxBitrate: %d", gcap->maxBitRate.value);
    }
    if (gcap->collapsing.filled)
    {
        std::size_t i = 0;
        VS_H245GenericParameter * param =
            static_cast< VS_H245GenericParameter*>
            (gcap->collapsing.data());

        for (i=0;i<gcap->collapsing.size();i++)
        {
            tLog->CPrintf("\n\t GEN.CAP.param ID.TAG : %u", param[i].parameterIdentifier.tag);
            tLog->CPrintf("\n\t GEN.CAP.param VAL.TAG: %u", param[i].parameterValue.tag);
            if (param[i].parameterIdentifier.tag==
                VS_H245ParameterIdentifier::e_standard)
            {
                TemplInteger<0,255> * pinteger =
					static_cast<TemplInteger<0,255> *>(( param[i].parameterIdentifier.choice));
                tLog->CPrintf("\n\t GEN.CAP.param PAR : %u",pinteger->value);

                switch(param[i].parameterValue.tag)
                {
                case VS_H245ParameterValue::e_booleanArray:
                    {
                        TemplInteger<0,255> * integer =
							static_cast<TemplInteger<0,255> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - booleanArray: %u",
                            integer->value);
                    }break;
                case VS_H245ParameterValue::e_unsignedMin:
                    {
                        TemplInteger<0,65535> * integer =
							static_cast<TemplInteger<0,65535> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - unsignedMin: %u",
                            integer->value);
                    }break;
                case VS_H245ParameterValue::e_unsignedMax :
                    {
                        TemplInteger<0,65535> * integer =
							static_cast<TemplInteger<0,65535> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - e_unsignedMax: %u",
                            integer->value);
                    }break;
                case VS_H245ParameterValue::e_unsigned32Min :
                    {
                        TemplInteger<0,4294967295> * integer =
							static_cast<TemplInteger<0,4294967295> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - e_unsigned32Min: %u",
                            integer->value);
                    }break;
                case VS_H245ParameterValue::e_unsigned32Max :
                    {
                        TemplInteger<0,4294967295> * integer =
							static_cast<TemplInteger<0,4294967295> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - e_unsigned32Max: %u",
                            integer->value);
                    }break;

                default: break;
                }
            }
        }
    }
    if (gcap->nonCollapsing.filled)
    {
        std::size_t i = 0;
        VS_H245GenericParameter * param =
            static_cast< VS_H245GenericParameter*>
            (gcap->nonCollapsing.data());

        for (i=0;i<gcap->nonCollapsing.size();i++)
        {
            tLog->CPrintf("\n\t GEN.CAP.param ID.TAG : %u", param[i].parameterIdentifier.tag);
            tLog->CPrintf("\n\t GEN.CAP.param VAL.TAG: %u", param[i].parameterValue.tag);
            if (param[i].parameterIdentifier.tag==
                VS_H245ParameterIdentifier::e_standard)
            {
                TemplInteger<0,255> * pinteger =
					static_cast<TemplInteger<0,255> *>(( param[i].parameterIdentifier.choice));
                tLog->CPrintf("\n\t GEN.CAP.param PAR : %u",pinteger->value);

                switch(param[i].parameterValue.tag)
                {
                case VS_H245ParameterValue::e_booleanArray:
                    {
                        TemplInteger<0,255> * integer =
							static_cast<TemplInteger<0,255> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - booleanArray: %u",
                            integer->value);
                    }break;
                case VS_H245ParameterValue::e_unsignedMin:
                    {
                        TemplInteger<0,65535> * integer =
							static_cast<TemplInteger<0,65535> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - unsignedMin: %u",
                            integer->value);
                    }break;
                case VS_H245ParameterValue::e_unsignedMax :
                    {
                        TemplInteger<0,65535> * integer =
							static_cast<TemplInteger<0,65535> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - e_unsignedMax: %u",
                            integer->value);
                    }break;
                case VS_H245ParameterValue::e_unsigned32Min :
                    {
                        TemplInteger<0,4294967295> * integer =
							static_cast<TemplInteger<0,4294967295> *>
                            (param[i].parameterValue.choice);
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - e_unsigned32Min: %u",
                            integer->value);
                    }break;
                case VS_H245ParameterValue::e_unsigned32Max :
                    {
                        TemplInteger<0,4294967295> * integer =
							static_cast<TemplInteger<0,4294967295> *>
                            ((param[i].parameterValue.choice));
                        tLog->CPrintf("\n\t GEN.CAP.param VAL - e_unsigned32Max: %u",
                            integer->value);
                    }break;

                default: break;
                }

            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////