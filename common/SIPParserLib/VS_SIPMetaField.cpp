#include "VS_SIPMetaField.h"
#include "VS_SIPMetaContainer.h"
#include "VS_SIPField_To.h"
#include "VS_SIPField_From.h"
#include "VS_SIPField_Via.h"
#include "VS_SIPField_RecordRoute.h"
#include "VS_SIPField_Contact.h"
#include "VS_SIPField_CSeq.h"
#include "VS_SIPField_CallID.h"
#include "VS_SIPField_StartLine.h"
#include "VS_SIPField_ContentType.h"
#include "VS_SIPField_ContentLength.h"
#include "VS_SIPField_Auth.h"
#include "VS_SIPField_Expires.h"
#include "VS_SIPField_Event.h"
#include "VS_SIPField_UserAgent.h"
#include "VS_SIPField_SessionExpires.h"
#include "VS_SIPField_MinExpires.h"
#include "VS_SIPField_Require.h"
#include "VS_SIPField_Supported.h"
#include "VS_SIPField_RetryAfter.h"
#include "VS_SIPField_Server.h"
#include "std-generic/compat/memory.h"

VS_SIPMetaField::VS_SIPMetaField():
iTo(0), iFrom(0), iContact(0), iCSeq(0), iCallID(0), iStartLine(0),
iContentType(0), iContentLength(0),	iAuthHeader(0), iExpires(0), iEvent(0), iUserAgent(0),
iSessionExpires(0), iMinExpires(0), iRequire(0), iSupported(0), iRetryAfterField(0), iServer(0), iContainer(vs::make_unique<VS_SIPMetaContainer>())
{

}

VS_SIPMetaField::~VS_SIPMetaField()
{}

TSIPErrorCodes VS_SIPMetaField::Decode(VS_SIPBuffer &aBuffer)
{
	VS_SIPObjectFactory* factory = VS_SIPObjectFactory::Instance();

	assert(factory);

	if ( !factory->IsValid() )
		return factory->GetLastClassError();

// <Decoding LOOP>
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
			break;

		case TSIPErrorCodes::e_EndOfBuffer:
				theSize = 0;
				iContainer->GetSize(theSize);

				if ( theSize > 0 )
				{
					const TSIPErrorCodes err = CreateStaticLinks();
					if (TSIPErrorCodes::e_ok != err )
					{
						SetValid(false);
						SetError(err);
						return err;
					}

					SetValid(true);
					SetError(TSIPErrorCodes::e_ok);
					return TSIPErrorCodes::e_ok;
				}
				else
				{
					SetValid(false);
					SetError(TSIPErrorCodes::e_EndOfBuffer);
					return TSIPErrorCodes::e_EndOfBuffer;
				}
			break;

		default:
			return result.error_code;
		}

		if ( !result.p_field)
			continue;

		const TSIPErrorCodes err = result.p_field->Decode(aBuffer);
		if (TSIPErrorCodes::e_ok != err)
		{
			return err;
		}

		iContainer->AddField(std::move(result.p_field));
	}
// </Decoding LOOP>

	return TSIPErrorCodes::e_null;
}

TSIPErrorCodes VS_SIPMetaField::Encode(VS_SIPBuffer &aBuffer) const
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
		if (TSIPErrorCodes::e_ok != err)
		{
			return err;
		}

		aBuffer.AddData("\r\n");
	}

	return TSIPErrorCodes::e_ok;
}

void VS_SIPMetaField::AddField(std::unique_ptr<VS_BaseField>&& aBaseField)
{
	iContainer->AddField(std::move(aBaseField));

	CreateStaticLinks();
}

void VS_SIPMetaField::EraseFields(const VS_SIPObjectFactory::SIPHeader header)
{
	VS_BaseField* theBase = 0;
	std::size_t theSize = 0;
	iContainer->GetSize(theSize);

	for (std::size_t i = 0; i < theSize;)
	{
		const TSIPErrorCodes err = iContainer->GetField(i, theBase);
		if (TSIPErrorCodes::e_ok != err) {
			SetError(TSIPErrorCodes::e_Content);
			break;
		}

		if (VS_SIPObjectFactory::Instance()->IsFieldOfType(theBase, header)) {
			i = iContainer->EraseField(i);
			theSize--;
		} else {
			i++;
		}
	}

	CreateStaticLinks();
}

bool VS_SIPMetaField::HasField(const VS_SIPObjectFactory::SIPHeader header) {
	VS_BaseField* theBase = 0;
	std::size_t theSize = 0;
	iContainer->GetSize(theSize);

	for (std::size_t i = 0; i < theSize; i++)
	{
		const TSIPErrorCodes err = iContainer->GetField(i, theBase);
		if (TSIPErrorCodes::e_ok != err) {
			SetError(TSIPErrorCodes::e_Content);
			break;
		}

		if (VS_SIPObjectFactory::Instance()->IsFieldOfType(theBase, header)) {
			return true;
		}
	}
	return false;
}

void VS_SIPMetaField::Clear()
{
	iContainer->Clear();
	iVia.clear();
	iRouteSet.clear();

	iTo = 0;
	iFrom = 0;
	iContact = 0;
	iCSeq = 0;
	iCallID = 0;
	iStartLine = 0;
	iContentType = 0;
	iContentLength = 0;
	iAuthHeader.clear();
	iExpires = 0;
	iEvent = 0;
	iUserAgent = 0;
	iSessionExpires = 0;
	iMinExpires = 0;
	iRetryAfterField = 0;

	SetValid(false);
	SetError(TSIPErrorCodes::e_null);
}

bool VS_SIPMetaField::GetType(int &aType) const
{
	VS_BaseField* theBase = 0;

	iContainer->GetField(0, theBase);

	VS_SIPField_StartLine* theStartLine = dynamic_cast<VS_SIPField_StartLine*> (theBase);
	if ( !theStartLine )
		return false;

	aType = theStartLine->GetMessageType();

	return aType != MESSAGE_TYPE_INVALID;
}

TSIPErrorCodes VS_SIPMetaField::CreateStaticLinks()
{
	iVia.clear();
	iRouteSet.clear();
	iTo = 0;
	iFrom = 0;
	iContact = 0;
	iCSeq = 0;
	iCallID = 0;
	iStartLine = 0;
	iContentType = 0;
	iContentLength = 0;
	iAuthHeader.clear();
	iExpires = 0;
	iEvent = 0;
	iUserAgent = 0;
	iSessionExpires = 0;
	iMinExpires = 0;
	iRequire = 0;
	iSupported = 0;
	iRetryAfterField = 0;

	VS_BaseField* theBase = 0;
	TSIPErrorCodes err = TSIPErrorCodes::e_null;

	VS_SIPField_To* To = 0;
	VS_SIPField_From* From = 0;
	VS_SIPField_Via* Via = 0;
	VS_SIPField_RecordRoute* RecordRoute = 0;
	VS_SIPField_Contact* Contact = 0;
	VS_SIPField_CSeq* CSeq = 0;
	VS_SIPField_CallID* CallID = 0;
	VS_SIPField_StartLine* StartLine = 0;
	VS_SIPField_ContentType* ContentType = 0;
	VS_SIPField_ContentLength* ContentLength = 0;
	VS_SIPField_Auth* AuthHeader = 0;
	VS_SIPField_Expires* Expires = 0;
	VS_SIPField_Event* Event = 0;
	VS_SIPField_UserAgent* UserAgent = 0;
	VS_SIPField_SessionExpires *SessionExpires = 0;
	VS_SIPField_MinExpires *MinExpires = 0;
	VS_SIPField_Require *Require = 0;
	VS_SIPField_Supported *Supported = 0;
	VS_SIPField_RetryAfter *RetryAfterField = 0;
	VS_SIPField_Server *Server = 0;

	std::size_t theSize = 0;
	iContainer->GetSize(theSize);

	for (unsigned int i=0; i < theSize; i++)
	{
		err = iContainer->GetField(i, theBase);
		if (TSIPErrorCodes::e_ok != err )
		{
			SetError(TSIPErrorCodes::e_Content);
			return TSIPErrorCodes::e_Content;
		}

		To = dynamic_cast<VS_SIPField_To*> (theBase);
		if ( To )
			{ iTo = To; continue; }

		From = dynamic_cast<VS_SIPField_From*> (theBase);
		if ( From )
			{ iFrom = From; continue; }

		Via = dynamic_cast<VS_SIPField_Via*> (theBase);
		if ( Via )
			{ iVia.push_back(Via); continue; }

		RecordRoute = dynamic_cast<VS_SIPField_RecordRoute*>(theBase);
		if (RecordRoute)
			{ iRouteSet.push_back(RecordRoute); continue; }

		Contact = dynamic_cast<VS_SIPField_Contact*> (theBase);
		if ( Contact )
			{
				if (!iContact) iContact = Contact;
				else iContact->AddContacts(Contact);
				continue;
			}

		CSeq = dynamic_cast<VS_SIPField_CSeq*> (theBase);
		if ( CSeq )
			{ iCSeq = CSeq; continue; }

		CallID = dynamic_cast<VS_SIPField_CallID*> (theBase);
		if ( CallID )
			{ iCallID = CallID; continue; }

		StartLine = dynamic_cast<VS_SIPField_StartLine*> (theBase);
		if ( StartLine )
			{ iStartLine = StartLine; continue; }

		ContentType = dynamic_cast<VS_SIPField_ContentType*> (theBase);
		if ( ContentType )
			{ iContentType = ContentType; continue; }

		ContentLength = dynamic_cast<VS_SIPField_ContentLength*> (theBase);
		if ( ContentLength )
			{ iContentLength = ContentLength; continue; }

		AuthHeader = dynamic_cast<VS_SIPField_Auth*> (theBase);
		if ( AuthHeader )
			{ iAuthHeader.push_back(AuthHeader); continue; }

		Expires = dynamic_cast<VS_SIPField_Expires*> (theBase);
		if ( Expires )
			{ iExpires = Expires; continue; }

		MinExpires = dynamic_cast<VS_SIPField_MinExpires*> (theBase);
		if ( MinExpires )
			{ iMinExpires = MinExpires; continue; }

		Event = dynamic_cast<VS_SIPField_Event*> (theBase);
		if ( Event )
			{ iEvent = Event; continue; }

		UserAgent = dynamic_cast<VS_SIPField_UserAgent*> (theBase);
		if ( UserAgent )
			iUserAgent = UserAgent;

		SessionExpires = dynamic_cast<VS_SIPField_SessionExpires *>(theBase);
		if ( SessionExpires )
			iSessionExpires = SessionExpires;

		Require = dynamic_cast<VS_SIPField_Require *>(theBase);
		if (Require)
			iRequire = Require;

		Supported = dynamic_cast<VS_SIPField_Supported *>(theBase);
		if (Supported)
			iSupported = Supported;

		RetryAfterField = dynamic_cast<VS_SIPField_RetryAfter *>(theBase);
		if (RetryAfterField)
			iRetryAfterField = RetryAfterField;

		Server = dynamic_cast<VS_SIPField_Server *>(theBase);
		if (Server)
			iServer = Server;
	}

// Обязательные хидеры для любого SIP-сообщения
	if ( !iTo || !iFrom || !iCSeq || !iCallID || !iStartLine || !iVia.size())
	{
		SetError(TSIPErrorCodes::e_Content);
		return TSIPErrorCodes::e_Content;
	}

	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}