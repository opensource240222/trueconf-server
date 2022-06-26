#include "VS_SDPMetaField.h"
#include "VS_SIPMetaContainer.h"
#include "VS_SDPField_Attribute.h"
#include "VS_SDPField_Version.h"
#include "VS_SDPField_Connection.h"
#include "VS_SDPField_Origin.h"
#include "VS_SDPField_Bandwidth.h"
#include "VS_SDPField_MediaStream.h"
#include "std-generic/compat/memory.h"

VS_SDPMetaField::VS_SDPMetaField():
iVersion(0), iConnection(0), iOrigin(0), iBandwidth(0), iDirection(SDP_MEDIACHANNELDIRECTION_INVALID), iContainer(vs::make_unique<VS_SIPMetaContainer>())
{}

VS_SDPMetaField::~VS_SDPMetaField()
{}

TSIPErrorCodes VS_SDPMetaField::Decode(VS_SIPBuffer &aBuffer)
{
	VS_SDPObjectFactory* factory = VS_SDPObjectFactory::Instance();
	assert(factory);

	if ( !factory->IsValid() )
		return factory->GetLastClassError();

// *** <Decoding LOOP>
	std::size_t theSize = 0;

	unsigned int max_headers = 100;
	while(max_headers--)
	{

		auto result = factory->CreateField(aBuffer);

		switch(result.error_code)
		{
		case TSIPErrorCodes::e_ok:
				// Nothing do here!
			break;

		case TSIPErrorCodes::e_ObjectFactory:
				aBuffer.SkipHeader();
				continue;
			break;

		case TSIPErrorCodes::e_EndOfBuffer:
				theSize = 0;
				iContainer->GetSize(theSize);

				if ( theSize > 0 )
				{
					TSIPErrorCodes err = CreateStaticLinks();
					if (TSIPErrorCodes::e_ok != err )
					{
						SetValid(false);
						SetError(err);
						return err;
					}

					SetValid(true);
					SetError(TSIPErrorCodes::e_ok);
					return TSIPErrorCodes::e_ok;
				}else{
					SetValid(false);
					SetError(TSIPErrorCodes::e_EndOfBuffer);
					return TSIPErrorCodes::e_EndOfBuffer;
				}
			break;

		default:
			return result.error_code;
		}

		if ( !result.p_field)
		{
			aBuffer.SkipHeader();
			continue;
		}

		result.error_code = result.p_field->Decode(aBuffer);
		switch (result.error_code)
		{
		case TSIPErrorCodes::e_ok:
				// Nothing do here!
			break;

		case TSIPErrorCodes::e_match:				// Не распознанный хидер - например "m=application"
		case TSIPErrorCodes::e_NoSupportedCodecs:	// Не добалять MediaStream в контейнер
			continue;

		default:
				SetValid(false);
				SetError(result.error_code);
				return result.error_code;
			break;
		}

		iContainer->AddField(std::move(result.p_field));
	}
// *** </Decoding LOOP>

	return TSIPErrorCodes::e_null;
}


TSIPErrorCodes VS_SDPMetaField::Encode(VS_SIPBuffer &aBuffer) const
{
	VS_BaseField* theBase = 0;

	std::size_t theSize = 0;
	iContainer->GetSize(theSize);

	for (std::size_t i=0; i < theSize; i++)
	{
		TSIPErrorCodes err = iContainer->GetField(i, theBase);
		if (TSIPErrorCodes::e_ok != err)
		{
			return err;
		}

		err = theBase->Encode(aBuffer);
		if ( (TSIPErrorCodes::e_ok != err) && (TSIPErrorCodes::e_NoSupportedCodecs != err) )
		{
			return err;
		}
	}

	return TSIPErrorCodes::e_ok;
}

void VS_SDPMetaField::AddField(std::unique_ptr<VS_BaseField>&& aBaseField)
{
	iContainer->AddField(std::move(aBaseField));

	CreateStaticLinks();
}

void VS_SDPMetaField::EraseFields(const VS_SDPObjectFactory::SDPHeader header)
{
	VS_BaseField* theBase = 0;
	std::size_t theSize = 0;
	iContainer->GetSize(theSize);

	for (unsigned int i = 0; i < theSize;)
	{
		const TSIPErrorCodes err = iContainer->GetField(i, theBase);
		if (TSIPErrorCodes::e_ok != err) {
			SetError(TSIPErrorCodes::e_Content);
			break;
		}

		if (VS_SDPObjectFactory::Instance()->IsFieldOfType(theBase, header)) {
			i = iContainer->EraseField(i);
			theSize--;
		} else {
			i++;
		}
	}

	CreateStaticLinks();
}

bool VS_SDPMetaField::HasField(const VS_SDPObjectFactory::SDPHeader header)
{
	VS_BaseField* theBase = 0;
	std::size_t theSize = 0;
	iContainer->GetSize(theSize);

	for (unsigned int i = 0; i < theSize; i++)
	{
		const TSIPErrorCodes err = iContainer->GetField(i, theBase);
		if (TSIPErrorCodes::e_ok != err) {
			SetError(TSIPErrorCodes::e_Content);
			break;
		}

		if (VS_SDPObjectFactory::Instance()->IsFieldOfType(theBase, header)) {
			return true;
		}
	}
	return false;
}

void VS_SDPMetaField::Clear()
{
	iContainer->Clear();
	iMediaStreams.clear();

	iVersion = 0;
	iConnection = 0;
	iOrigin = 0;
	iBandwidth = 0;
	iDirection = SDP_MEDIACHANNELDIRECTION_INVALID;

	SetValid(false);
}

TSIPErrorCodes VS_SDPMetaField::CreateStaticLinks()
{
	iVersion = 0;
	iConnection = 0;
	iOrigin = 0;
	iBandwidth = 0;
	iMediaStreams.clear();
	iDirection = SDP_MEDIACHANNELDIRECTION_INVALID;

	VS_BaseField* theBase = 0;

	VS_SDPField_MediaStream* stream = 0;

	std::size_t theSize = 0;
	iContainer->GetSize(theSize);

	for (std::size_t i=0; i < theSize; i++)
	{
		TSIPErrorCodes err = iContainer->GetField(i, theBase);
		if (TSIPErrorCodes::e_ok != err )
		{
			SetError(TSIPErrorCodes::e_Content);
			return TSIPErrorCodes::e_Content;
		}

		VS_SDPField_Version * version = dynamic_cast<VS_SDPField_Version*> (theBase);
		if ( version )
			iVersion = version;

		VS_SDPField_Connection * connection = dynamic_cast<VS_SDPField_Connection*> (theBase);
		if ( connection )
			iConnection = connection;

		VS_SDPField_Origin * Origin = dynamic_cast<VS_SDPField_Origin*> (theBase);
		if (Origin)
			iOrigin = Origin;

		VS_SDPField_Bandwidth * bandwidth = dynamic_cast<VS_SDPField_Bandwidth*> (theBase);
		if ( bandwidth )
			iBandwidth = bandwidth;

		stream = dynamic_cast<VS_SDPField_MediaStream*> (theBase);
		if ( stream )
		{
			if ( !stream->GetConnection() )
				stream->SetConnection(iConnection);

			iMediaStreams.push_back(stream);
		}

		VS_SDPField_Attribute * attr = dynamic_cast<VS_SDPField_Attribute*>(theBase);
		if (attr)
			iDirection = attr->GetDirection();
	}

	if (iDirection != SDP_MEDIACHANNELDIRECTION_INVALID)
	{
		for (size_t i = 0; i < iMediaStreams.size(); ++i)
			if (iMediaStreams[i]->GetMediaDirection() == SDP_MEDIACHANNELDIRECTION_INVALID)
				iMediaStreams[i]->SetMediaDirection(iDirection);
	}

	if ( !iVersion || iMediaStreams.empty())
	{
		SetError(TSIPErrorCodes::e_Content);
		return TSIPErrorCodes::e_Content;
	}

	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}