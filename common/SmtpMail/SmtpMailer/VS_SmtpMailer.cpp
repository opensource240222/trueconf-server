
#define _WIN32_DCOM
#pragma warning( disable : 4995 )
#include <stdio.h>
#include <iostream>
#include <objbase.h>
#include "tlb_import/msado15.tlh"
#include "tlb_import/cdosys.tlh"
#include <cdosysstr.h>

#include "../../std/cpplib/VS_MemoryLeak.h"
#include "../../std/cpplib/VS_WideStr.h"
#include "std-generic/cpplib/TimeUtils.h"

#include "VS_SmtpMailer.h"
#include "..\..\std\debuglog\VS_Debug.h"

#define DEBUG_CURRENT_MODULE	VS_DM_SMTP
//////////////////////////////////////////////////////////////////////////////////////////
const char * mailheader[]={
							"Approved",
							"Comment",
							"Content-Base",
							"Content-Description",
							"Content-Disposition",
							"Content-Id",
							"Content-Language",
							"Content-Location",
							"Content-Tansfer-Encoding",
							"Content-Type",
							"Control",
							"Disposition",
							"Disposition-Notification-To",
							"Distribution",
							"Expires",
							"Followup-To",
							"In-Reply-To",
							"Lines",
							"Message-Id",
							"Mime-Version",
							"Newsgroups",
							"Organization",
							"Original-Recipient",
							"Path",
							"Posting-Version",
							"Received",
							"References",
							"Relay-Version",
							"Return-Path",
							"Return-Receipt-To",
							"Summary",
							"Thread-Index",
							"X-Mailer",
							"Xref",
							"X-Unsent"
};
const unsigned Num_of_mailheaders = 35;
//////////////////////////////////////////////////////////////////////////////////////////
const char * httpmail[]  ={
							"Attachmentfilename",
							"Bcc",
							"Cc",
							"Content-Disposition-Type",
							"Content-Media-Type",
							"Date","Datereceived",
							"From",
							"Hasattachment",
							"Htmldescription",
							"Importance",
							"Normalizedsubject",
							"Priority",
							"Reply-To",
							"Sender",
							"Subject",
							"Textdescription",
							"Thread-Topic",
							"To"
};
const unsigned Num_of_httpmails = 19;
//////////////////////////////////////////////////////////////////////////////////////////
const char prefix[]			= "urn:schemas:";
const char httpprefix[]		= "httpmail:";
const char headerprefix[]	= "mailheader:";
//////////////////////////////////////////////////////////////////////////////////////////
struct VS_SmtpMailer_Implementation
{
	HRESULT m_error_code;
	bool IsLastErrorCritical()
	{
		if(m_error_code==0x8004020F)
		{
			dprint1("Critical error. The message will not be saved.\n");
			return true;
		}
		m_error_code = 0;
		return false;
	}
	VS_SmtpMailer_Implementation( const _bstr_t& serverName, int port, VS_SmtpAuthType authType,
								  const char *login, const char *password, bool useSSL ) : isValid(false), m_UseSSL(useSSL)
	{
		m_error_code = 0;
		m_SmtpServer = serverName;		m_Port = port;
		if (login)		m_Login = login;
		if (password)	m_Password = password;
		switch (authType)
		{
		case VS_SimplePass:
			m_AuthType = cdoBasic;
			break;
		case VS_NTLM:
			m_AuthType = cdoNTLM;
		default:
			m_AuthType = cdoAnonymous;
		}
		iMsg.CreateInstance(__uuidof(Message));

		//iMsg->AutoGenerateTextBody = _variant_t(false);

		// make configuration
		iConf.CreateInstance(__uuidof(Configuration));

		/*FieldsPtr Flds;
		Flds = iConf->Fields;

		Flds->Item[cdoSendUsingMethod]->Value		= _variant_t((long)cdoSendUsingPort);
		Flds->Item[cdoSMTPServer]->Value			= _variant_t(m_SmtpServer);
		Flds->Item[cdoSMTPServerPort]->Value		= _variant_t((long)m_Port);
		Flds->Item[cdoSMTPConnectionTimeout]->Value	= _variant_t((long)10);
		Flds->Item[cdoURLGetLatestVersion]->Value   = _variant_t(VARIANT_TRUE);
		Flds->Update();*/
		isValid = false;
	}
	// end VS_SmtpMailer_Implementation::VS_SmtpMailer_Implementation
/////////////////////////////////////////////////////////////////////////////////////////
	VS_SmtpMailer_Implementation( const char * serverName, int port, VS_SmtpAuthType authType,
								  const char *login, const char *password, bool useSSL ) : isValid(false), m_UseSSL(useSSL)
	{
		m_SmtpServer = _bstr_t(serverName);		m_Port = port;
		if (login)		m_Login = login;
		if (password)	m_Password = password;
		switch (authType)
		{
		case VS_SimplePass:
			m_AuthType = cdoBasic;
			break;
		case VS_NTLM:
			m_AuthType = cdoNTLM;
		default:
			m_AuthType = cdoAnonymous;
		}
		iMsg.CreateInstance(__uuidof(Message));

		iMsg->BodyPart->Charset="utf-8";
		iMsg->BodyPart->ContentTransferEncoding=cdo8bit;
		//iMsg->AutoGenerateTextBody = _variant_t(false);

		// make configuration
		iConf.CreateInstance(__uuidof(Configuration));
		isValid = false;
	}
	void VS_SmtpMailer_Connect()
	{
		FieldsPtr Flds;
		Flds = iConf->Fields;
	   	Flds->Item[cdoSendUsingMethod]->Value		= _variant_t((long)cdoSendUsingPort);
		Flds->Item[cdoSMTPServer]->Value			= _variant_t(m_SmtpServer);
		Flds->Item[cdoSMTPServerPort]->Value		= _variant_t(m_Port);
		Flds->Item[cdoSMTPAuthenticate]->Value		= _variant_t(m_AuthType);
		if (m_AuthType == VS_SimplePass)
		{
			Flds->Item[cdoSendUserName]->Value		= _variant_t(m_Login);
			Flds->Item[cdoSendPassword]->Value		= _variant_t(m_Password);
		}
		Flds->Item[cdoSMTPUseSSL]->Value			= _variant_t(m_UseSSL ? VARIANT_TRUE : VARIANT_FALSE);
		Flds->Item[cdoSMTPConnectionTimeout]->Value	= _variant_t((long)10);
		Flds->Item[cdoURLGetLatestVersion]->Value   = _variant_t(VARIANT_TRUE);
		Flds->Update();
		isValid = true;
	}
	// end VS_SmtpMailer_Implementation::VS_SmtpMailer_Connect

	void To( const char * to )
	{
		iMsg->To = _bstr_t(to);
	}
	// end VS_SmtpMailer_Implementation::To

	bool GetTo(char **in , unsigned size )
	{
		_bstr_t TO = iMsg->GetTo();
		if((char *)TO)
			strncpy(*in,(char*)TO,size);
		return true;
	}
	bool GetFrom(char **in , unsigned size )
	{
		_bstr_t FROM = iMsg->GetFrom();
		if((char *)FROM)
			strncpy(*in,(char*)FROM,size);
		return true;
	}
	void BCC( const char * bcc )
	{
		iMsg->BCC = _bstr_t(bcc);
	}
	// end VS_SmtpMailer_Implementation::BCC

	void ReplyTo( const char * rpt )
	{
		iMsg->ReplyTo = _bstr_t(rpt);
	}
	// end VS_SmtpMailer_Implementation::ReplyTo

	void CC( const char * cc )
	{
		iMsg->CC = _bstr_t(cc);
	}
	// end VS_SmtpMailer_Implementation::CC

	void From( const char * from )
	{
		iMsg->From = _bstr_t(from);
	}
	// end VS_SmtpMailer_Implementation::From
	void Subject( const char * subject )
	{
		iMsg->Subject = _bstr_t(subject);
	}
	// end VS_SmtpMailer_Implementation::Subject

	void SetTextBody( const char * text )
	{
		iMsg->TextBody = _bstr_t(text);
	}
	// end VS_SmtpMailer_Implementation::SetTextBody

	void SetHTMLBody( const char * htmlText )
	{
		iMsg->HTMLBody = _bstr_t(htmlText);
		/****************************/
	/*	IBodyPartPtr ibp;
		IBodyPartPtr ibp1;
		FieldsPtr	 flds;
		_StreamPtr	 stm;
		ibp = iMsg;

		ibp1 = ibp->AddBodyPart(-1);
		flds = ibp1->GetFields();
		flds->Item[cdoContentType]->Value				= _bstr_t(cdoTextHTML);
		flds->Item[cdoContentTransferEncoding]->Value	= _bstr_t(cdo7bit);
		flds->Update();
		stm = ibp1->GetDecodedContentStream();
		stm->WriteText(htmlText, adWriteChar);
		stm->Flush();*/
	}
	// end VS_SmtpMailer_Implementation::SetHTMLBody

	bool SetMHTMLBody( const char * url )
	{
		try
		{
			iMsg->CreateMHTMLBody(_bstr_t(url), cdoSuppressNone, _bstr_t(""), _bstr_t(""));
		}
		catch(_com_error e)
		{
			PrintMethodError("SetMHTMLBody", url);
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::SetMHTMLBody

bool InsertImage( const char* filePath, const char* contentId )
	{
		IBodyPartPtr ibp;
		IBodyPartPtr ibp1;
		FieldsPtr	 flds;
		_StreamPtr	 stm;
		try
		{
			ibp = iMsg;
			ibp1 = ibp->AddBodyPart(-1);
			flds = ibp1->GetFields();
			flds->Item[cdoContentType]->Value				= _bstr_t(cdoGif);
			flds->Item[cdoContentTransferEncoding]->Value	= _bstr_t(cdoBase64);
			if ((char *)contentId)
				flds->Item[cdoContentId]->Value				= _bstr_t(contentId);
			flds->Update();
			stm = ibp1->GetDecodedContentStream();
			stm->LoadFromFile(filePath);
			stm->Flush();
		}
		catch(_com_error e)
		{
			PrintMethodError("InsertImage", filePath);
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::InsertImage

	bool AddAttachment( const char* filePath )
	{
		try
		{
			iMsg->AddAttachment(_bstr_t(filePath),_bstr_t(""),_bstr_t(""));
		}
		catch(_com_error e)
		{
			PrintMethodError("AddAttachment", filePath);
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::AddAttachment
	bool LoadTextBody( const char *filePath )
	{

		_StreamPtr  pStm(__uuidof(Stream));
		_variant_t varOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);
		try
		{
			pStm->Open(varOptional, adModeUnknown, adOpenStreamUnspecified,_bstr_t(),_bstr_t());
			pStm->LoadFromFile(_bstr_t(filePath));

			int size = pStm->Size;
			pStm->Charset = cdoUS_ASCII;
			pStm->Type = adTypeText;
			pStm->Position = 2;
			_bstr_t s = pStm->ReadText(size-2);
			SetTextBody(s);

		}
		catch(_com_error e)
		{
			PrintMethodError("LoadFromFile", "");
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::LoadTextBody

	bool LoadHTMLBody( const char * filePath )
	{
		_StreamPtr  pStm(__uuidof(Stream));
		_variant_t varOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);
		try
		{
			pStm->Open(varOptional, adModeUnknown, adOpenStreamUnspecified,_bstr_t(),_bstr_t());
			pStm->LoadFromFile( _bstr_t(filePath) );

			int size = pStm->Size;
			pStm->Charset = cdoUS_ASCII;
			pStm->Type = adTypeText;
			pStm->Position = 2;
			_bstr_t s = pStm->ReadText(size-2);

			SetHTMLBody(s);
		}
		catch(_com_error e)
		{
			PrintMethodError("LoadFromFile", "");
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::LoadHTMLBody

	bool LoadFromFile( const char *filePath )
	{
		//_StreamPtr  pStm(__uuidof(Stream));
		//_variant_t varOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);
		_StreamPtr pStm = iMsg->GetStream();
		//pStm->Release();
		//pStm->Open(
		try
		{

			pStm->LoadFromFile( _bstr_t(filePath ) );

			//pStm->Open(varOptional, adModeUnknown, adOpenStreamUnspecified,_bstr_t(),_bstr_t());
			//pStm->LoadFromFile( _bstr_t(filePath ) );

			/**/
			//int size = pStm->Size;
			pStm->Charset = cdoUS_ASCII;
			pStm->Type = adTypeText;


			//pStm->Position = 3;
			//_bstr_t a = pStm->ReadText(3);
			//_bstr_t s = pStm->ReadText(size-3);
			//pStm->WriteText(s,adWriteChar);
			//_bstr_t test = pStm->ReadText(size-3);
			//pStm->WriteText(test,adWriteChar);

			/**/

		}
		catch(_com_error e)
		{
			PrintMethodError("LoadFromFile", "");
			PrintComError(e);
			return false;
		}

		IDataSourcePtr iDsrc;
		iDsrc = iMsg;

		try
		{
			iDsrc->OpenObject(pStm,_bstr_t(cdoAdoStream));
		}
		catch(_com_error e)
		{
			PrintComError(e);
			return false;
		}

		return true;
	}
	// end VS_SmtpMailer_Implementation::LoadFromFile

	bool SaveToFile( const char * filePath )
	{
		try
		{
			_StreamPtr Stm = iMsg->GetStream();
			_bstr_t A;
			//Stm->SkipLine();
			//Stm->SkipLine();
			A = Stm->ReadText(400000);
			unsigned size = Stm->Size;
			unsigned l=0,i=0;
			char buffer[65535]={0};
			FILE * out;
			char * from=(char*)A;
			bool isBadOleVersion=false;
			const char bad_field1[]="Thread-Topic";
			const char bad_field2[]="Thread-Index";

			if (0==strncmp(bad_field1,from,strlen(bad_field1)) )
			{
				isBadOleVersion = true;
				/// OLE bad work 6.00.2600.0000   XP
				/// OLE good work 6.00.2600.1409 2000
				int missed_lines=0;
				int missed_lines_limit = 3;
				for(i=0;(i<size)&&(missed_lines<missed_lines_limit);i++)
					if((from[i]==0x0d) && (from[i+1]==0x0a))
						missed_lines++;
				i++;
			}
			if(isBadOleVersion) buffer[l++]='\n';
			for(;i<size;i++)
			{
				if( (from[i]==0x0d) && (from[i+1]==0x0a))
				{ buffer[l++]='\n';
				  i++;
				}
				else buffer[l++]=from[i];
			}
			if(!(out=fopen(filePath,"w"))) return false;
			fprintf(out,"%s",buffer);
			fflush(out);
			fclose(out);
			//_variant_t var =Stm->Read(10);
			//_bstr_t A(var);
			//Stm->SaveToFile(_bstr_t(filePath),adSaveCreateOverWrite);
		}
		catch(_com_error e)
		{
			PrintMethodError("SaveToFile", filePath);
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::SaveToFile
/////////////////////////////////////////////////////////////////////////////////////////
	~VS_SmtpMailer_Implementation( void )
	{
		iMsg	= NULL;
		iConf	= NULL;
	}
	// end VS_SmtpMailer_Implementation::~VS_SmtpMailer_Implementation

	bool   isValid;
	IMessagePtr			iMsg;
	IConfigurationPtr	iConf;
	_bstr_t				m_SmtpServer;
	long				m_Port;
	_bstr_t				m_Login;
	_bstr_t				m_Password;
	long				m_AuthType;
	bool				m_UseSSL;
	bool AddField(const char * fld_name,const _bstr_t& fld_cont)
	{
		if(fld_name && !!fld_cont)
		{	unsigned i=0;	bool flag=false;
			char fname[256]={0};
			for(i=0;(i<Num_of_mailheaders)&&(!flag);i++)
			{
				if(_stricmp(fld_name,mailheader[i])==0)
				{	flag=true;
					sprintf(fname,"%s%s%s",prefix,headerprefix,mailheader[i]);
					if(!_strlwr(fname)) return false;
					_bstr_t in    = _bstr_t(fname);
					FieldsPtr	 flds;
					flds = iMsg->GetFields();
					flds->Item[in]->Value = fld_cont;
					flds->Update();
			}	}
			if(!flag)
			{
				for(i=0;(i<Num_of_httpmails)&&(!flag);i++)
				{
					if(_stricmp(fld_name,httpmail[i])==0)
					{	flag=true;
						sprintf(fname,"%s%s%s",prefix,httpprefix,httpmail[i]);
						if(!_strlwr(fname)) return false;
						_bstr_t in    = _bstr_t(fname);
						FieldsPtr	 flds;
						_bstr_t count = _bstr_t(fld_cont);
						flds = iMsg->GetFields();
						flds->Item[in]->Value = count;
						flds->Update();
			}	}	}
			return true;
		}
		return false;
	}
	bool Send(void)
	{
		VS_SmtpMailer_Connect();
		try
		{

			iMsg->Configuration = iConf;
			iMsg->Send();
			// Clear body !!!!!!!
			IBodyPartsPtr iBps;
			iBps = iMsg->GetBodyPart()->GetBodyParts();
			iBps->DeleteAll();

			iMsg->AutoGenerateTextBody = _variant_t(true);
			//iBps->Delete(
		}
		catch(_com_error e)
		{
			_bstr_t from_msg(iMsg->From);
			_bstr_t to_msg(iMsg->To);

			_bstr_t	msg;
			msg = "From:" + from_msg + "; To:" + to_msg;
			PrintMethodError("Send", msg);
			PrintComError(e);

			try
			{
				IBodyPartsPtr iBps;
				iBps = iMsg->GetBodyPart()->GetBodyParts();
				iBps->DeleteAll();
			}
			catch(_com_error e) {}

			return false;
		}
		catch(...)
		{
			PrintMethodError("Send", "Unrecognized error !!!");
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::Send

	void To( const _bstr_t &to )
	{
		iMsg->To = to;
	}
	// end VS_SmtpMailer_Implementation::To

	void CC( const _bstr_t &cc )
	{
		iMsg->CC = cc;
	}
	// end VS_SmtpMailer_Implementation::CC

	void BCC( const _bstr_t &bcc )
	{
		iMsg->BCC = bcc;
	}
	// end VS_SmtpMailer_Implementation::BCC

	void ReplyTo( const _bstr_t &rpt )
	{
		iMsg->ReplyTo = rpt;
	}
	// end VS_SmtpMailer_Implementation::ReplyTo

	void From( const _bstr_t &from )
	{
		iMsg->From = from;
	}
	// end VS_SmtpMailer_Implementation::From

	void Subject( const _bstr_t &subject )
	{
		iMsg->Subject = subject;
	}
	// end VS_SmtpMailer_Implementation::Subject

	void SetTextBody( const _bstr_t &text )
	{
		iMsg->TextBody = text;
	}
	// end VS_SmtpMailer_Implementation::SetTextBody

	void SetHTMLBody( const _bstr_t &htmlText )
	{
		iMsg->HTMLBody = htmlText;
		/****************************/
	/*	IBodyPartPtr ibp;
		IBodyPartPtr ibp1;
		FieldsPtr	 flds;
		_StreamPtr	 stm;
		ibp = iMsg;

		ibp1 = ibp->AddBodyPart(-1);
		flds = ibp1->GetFields();
		flds->Item[cdoContentType]->Value				= _bstr_t(cdoTextHTML);
		flds->Item[cdoContentTransferEncoding]->Value	= _bstr_t(cdo7bit);
		flds->Update();
		stm = ibp1->GetDecodedContentStream();
		stm->WriteText(htmlText, adWriteChar);
		stm->Flush();*/
	}
	// end VS_SmtpMailer_Implementation::SetHTMLBody

	bool SetMHTMLBody( const _bstr_t &url )
	{
		try
		{
			iMsg->CreateMHTMLBody(url, cdoSuppressNone, _bstr_t(""), _bstr_t(""));
		}
		catch(_com_error e)
		{
			PrintMethodError("SetMHTMLBody", url);
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::SetMHTMLBody

	bool SetStreamBody( VS_SmtpBodyType type )
	{
		if (type == VS_TextUnknown)		return false;
		//get text-data from input
		int currentSize = DEFAULT_BUFFER_SIZE;
		int pointer = 0;
		char* storage = (char*)new char[currentSize];
		if (std::cin.fail())
			std::cin.clear();
		char buff[1024];
		while (std::cin)
		{
			std::cin.read(buff, 1024);
			std::cin.gcount();
			if ((pointer + std::cin.gcount()) >= currentSize)
			{
				//resize buffer
				char* tmp = (char*)new char[currentSize*2];
				memcpy(tmp, storage, currentSize);
				currentSize *= 2;
				delete[] storage;
				storage = tmp;
			}
			memcpy(storage + pointer, buff, static_cast<size_t>(std::cin.gcount()));
			pointer += static_cast<size_t>(std::cin.gcount());
		}
		storage[pointer] = 0;

		_bstr_t source(storage);
		delete[] storage;

		//printf("\tSource = %s\n", (char*) source);
		if (type == VS_TextPlain)	SetTextBody(source);
		if (type == VS_TextHtml)	SetHTMLBody(source);
		return true;
	}
	// end VS_SmtpMailer_Implementation::SetStreamBody

	bool InsertImage( const _bstr_t &filePath, const _bstr_t &contentId )
	{
		IBodyPartPtr ibp;
		IBodyPartPtr ibp1;
		FieldsPtr	 flds;
		_StreamPtr	 stm;
		try
		{
			ibp = iMsg;
			ibp1 = ibp->AddBodyPart(-1);
			flds = ibp1->GetFields();
			flds->Item[cdoContentType]->Value				= _bstr_t(cdoGif);
			flds->Item[cdoContentTransferEncoding]->Value	= _bstr_t(cdoBase64);
			if ((char *)contentId)
				flds->Item[cdoContentId]->Value				= contentId;
			flds->Update();
			stm = ibp1->GetDecodedContentStream();
			stm->LoadFromFile((char *)filePath);
			stm->Flush();
		}
		catch(_com_error e)
		{
			PrintMethodError("InsertImage", filePath);
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::InsertImage

	bool AddAttachment( const _bstr_t &filePath )
	{
		try
		{
			iMsg->AddAttachment(filePath,"","");
		}
		catch(_com_error e)
		{
			PrintMethodError("AddAttachment", filePath);
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::AddAttachment

	bool AddAttachmentManual( const char* data )
	{
		VS_WideStr wstr;
		wstr.AssignUTF8(data);
		if (!wstr.Length())
			return true;

		bool res(true);

		try
		{
			// main multipart body part
			IBodyPartPtr body = iMsg->BodyPart->AddBodyPart(-1);
			FieldsPtr fields = body->Fields;
			fields->Item[cdoContentType]->Value = _bstr_t(cdoMultipartAlternative);
			fields->Update();

			// text message
			/*{
				IBodyPartPtr ibp;
				_StreamPtr stm;
				FieldsPtr flds;

				//ibp = iMsg->Attachments->Add(-1);
				//ibp = iMsg->BodyPart->AddBodyPart(-1);
				ibp = body->AddBodyPart(-1);
				flds = ibp->Fields;
				flds->Item[cdoContentType]->Value = _bstr_t("text/plain; charset=UTF-8");
				flds->Item[cdoContentTransferEncoding]->Value = _bstr_t(cdoBase64);
				flds->Item[cdoContentDisposition]->Value = "";
				flds->Update();
				stm = ibp->GetDecodedContentStream();
				stm->Type = adTypeText;
				stm->WriteText(iMsg->TextBody, adWriteChar);

				stm->Flush();
			}*/
			//iMsg->TextBody = _bstr_t("");

			// send invite as text
			{
				IBodyPartPtr ibp;
				_StreamPtr stm;
				FieldsPtr flds;

				//ibp = iMsg->Attachments->Add(-1);
				//ibp = iMsg->BodyPart->AddBodyPart(-1);
				ibp = body->AddBodyPart(-1);
				flds = ibp->Fields;
				flds->Item[cdoContentType]->Value = _bstr_t("text/calendar; charset=UTF-8; method=REQUEST;");
				flds->Item[cdoContentTransferEncoding]->Value = _bstr_t(cdoQuotedPrintable);
				flds->Item[cdoContentDisposition]->Value = "";
				flds->Update();
				stm = ibp->GetDecodedContentStream();
				stm->Type = adTypeText;
				stm->WriteText(wstr.m_str, adWriteChar);

				stm->Flush();
			}
			// send as binary
			{
				IBodyPartPtr ibp;
				_StreamPtr stm;
				FieldsPtr flds;

				//ibp = iMsg->Attachments->Add(-1);
				//ibp = iMsg->BodyPart->AddBodyPart(-1);
				ibp = body->AddBodyPart(-1);
				flds = ibp->Fields;
				flds->Item[cdoContentType]->Value = _bstr_t("application/ics; name=\"invite.ics\"");
				flds->Item[cdoContentDisposition]->Value = _bstr_t("attachment; filename=\"invite.ics\"");
				flds->Item[cdoContentTransferEncoding]->Value = _bstr_t(cdoBase64);
				flds->Update();
				stm = ibp->GetDecodedContentStream();
				stm->Type = adTypeBinary;

				// move data into COM array and then write it
				// into decoded content stream
				{
					VARIANT va;
					LONG i;
					LONG len = (LONG)strlen(data);
					SAFEARRAY *psa = NULL;
					SAFEARRAYBOUND bnd;

					ZeroMemory(&bnd, sizeof(bnd));
					bnd.cElements = len;
					psa = SafeArrayCreate(VT_UI1, 1, &bnd);

					for (i = 0; i < len; i++)
					{
						unsigned char b = (unsigned char)data[i];
						if (FAILED(SafeArrayPutElement(psa, &i, &b)))
						{
							SafeArrayDestroy(psa);
							res = false;
							break;
						}
					}

					if (res == true)
					{
						ZeroMemory(&va, sizeof(va));
						va.vt = VT_ARRAY | VT_UI1;
						va.parray = psa;

						stm->Write(va);
						stm->Flush();
					}
					SafeArrayDestroy(psa);
				}
				stm->Flush();
			}
		}
		catch(_com_error e)
		{
			PrintMethodError("AddAttachmentManual", data);
			PrintComError(e);
			res = false;
		}
		return res;
	}

/*	bool SetAsText( const _bstr_t buffer )
	{
	//printf("SetAsText :\n%s\n\n", buffer);
		_bstr_t data(buffer);
		_StreamPtr  pStm(__uuidof(Stream));
		_variant_t varOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);
		try
		{
			pStm->Open(varOptional, adModeUnknown, adOpenStreamUnspecified,_bstr_t(),_bstr_t());
			pStm->Charset = cdoUS_ASCII;
			pStm->Type = adTypeText;
			pStm->WriteText(_bstr_t(buffer), adWriteChar);
		}
		catch(_com_error e)
		{
			PrintMethodError("SetAsText", "");
			PrintComError(e);
			return false;
		}

		IDataSourcePtr iDsrc;
		iDsrc = iMsg;

		try
		{
			iDsrc->OpenObject(pStm,_bstr_t(cdoAdoStream));
		}
		catch(_com_error e)
		{
			PrintComError(e);
			return false;
		}

		return true;
	}
	// end VS_SmtpMailer_Implementation::SetAsText
*/
	bool LoadTextBody( const _bstr_t &filePath )
	{

		_StreamPtr  pStm(__uuidof(Stream));
		_variant_t varOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);
		try
		{
			pStm->Open(varOptional, adModeUnknown, adOpenStreamUnspecified,_bstr_t(),_bstr_t());
			pStm->LoadFromFile(filePath);

			int size = pStm->Size;
			pStm->Charset = cdoUS_ASCII;
			pStm->Type = adTypeText;
			pStm->Position = 2;
			_bstr_t s = pStm->ReadText(size-2);
			SetTextBody(s);

		}
		catch(_com_error e)
		{
			PrintMethodError("LoadFromFile", "");
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::LoadTextBody

	bool LoadHTMLBody( const _bstr_t &filePath )
	{
		_StreamPtr  pStm(__uuidof(Stream));
		_variant_t varOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);
		try
		{
			pStm->Open(varOptional, adModeUnknown, adOpenStreamUnspecified,_bstr_t(),_bstr_t());
			pStm->LoadFromFile( filePath );

			int size = pStm->Size;
			pStm->Charset = cdoUS_ASCII;
			pStm->Type = adTypeText;
			pStm->Position = 2;
			_bstr_t s = pStm->ReadText(size-2);

			SetHTMLBody(s);
		}
		catch(_com_error e)
		{
			PrintMethodError("LoadFromFile", "");
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::LoadHTMLBody

	bool LoadFromFile( const _bstr_t &filePath )
	{
		_StreamPtr  pStm(__uuidof(Stream));

		_variant_t varOptional(DISP_E_PARAMNOTFOUND,VT_ERROR);
		try
		{
			pStm->Open(varOptional, adModeUnknown, adOpenStreamUnspecified,_bstr_t(),_bstr_t());
			pStm->LoadFromFile( filePath );

			/**/
			int size = pStm->Size;
			pStm->Charset = cdoUS_ASCII;
			pStm->Type = adTypeText;
			pStm->Position = 2;
			_bstr_t s = pStm->ReadText(size-2);
			pStm->WriteText(s,adWriteChar);
			/**/

		}
		catch(_com_error e)
		{
			PrintMethodError("LoadFromFile", "");
			PrintComError(e);
			return false;
		}

		IDataSourcePtr iDsrc;
		iDsrc = iMsg;

		try
		{
			iDsrc->OpenObject(pStm,_bstr_t(cdoAdoStream));
		}
		catch(_com_error e)
		{
			PrintComError(e);
			return false;
		}

		return true;
	}
	// end VS_SmtpMailer_Implementation::LoadFromFile

	bool SaveToFile( const _bstr_t &filePath )
	{
		try
		{
			_StreamPtr Stm = iMsg->GetStream();
			long num = 3;
			_bstr_t A(Stm->Read(num));
			Stm->SaveToFile(filePath,adSaveCreateOverWrite);
		}
		catch(_com_error e)
		{
			PrintMethodError("SaveToFile", filePath);
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::SaveToFile
/*
	bool GetAsText( char **text, int &size )
	{
		try
		{
			_StreamPtr Stm = iMsg->GetStream();
			Stm->Charset = cdoUS_ASCII;
			Stm->Type = adTypeText;
			size = Stm->Size;
			char* temp = (char*)new char[size+1];
			temp[0]=0;
			_bstr_t s = Stm->ReadText(size);
			strcpy(temp, (char*)s);
			*text = temp;
		}
		catch(_com_error e)
		{
			PrintMethodError("GetAsText", "");
			PrintComError(e);
			return false;
		}
		return true;
	}
	// end VS_SmtpMailer_Implementation::GetAsText
*/
	void PrintComError( _com_error &e )
	{
	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());

		char buf[256];
		tu::TimeToNStr(std::chrono::system_clock::now(), buf, 256);
		// Print COM errors.
		dprint1("#at %s\n", buf);
		// Print Com errors.
		dprint1("Error\n");
		m_error_code = e.Error();
		dprint1("\tCode = %08lx\n",m_error_code );

		dprint1("\tCode meaning = %s\n", (char*)e.ErrorMessage());
		dprint1("\tSource = %s\n", (char*) bstrSource);
		dprint1("\tDescription = %s\n", (char*) bstrDescription);

		fflush(stdout);
	}
	// end VS_SmtpMailer_Implementation::PrintComError

	void PrintMethodError( const _bstr_t &method, const _bstr_t &message )
	{
		dprint1("Error in method 'VS_SmtpMailer::%s'\n", (char*)method);
		dprint1("\t%s\n", (char*)message);
	}
	// end VS_SmtpMailer_Implementation::PrintMethodError
};
// end VS_SmtpMailer_Implementation struct

//////////////////////////////////////////////////////////////////////////////////////////

VS_SmtpMailer::VS_SmtpMailer( const wchar_t *serverName, int port, VS_SmtpAuthType authType,
							  const char *login, const char *password, bool useSSL ):imp(0)
{	imp = new VS_SmtpMailer_Implementation( serverName, port, authType, login, password, useSSL );		}
// end VS_SmtpMailer::VS_SmtpMailer

VS_SmtpMailer::~VS_SmtpMailer( void )
{	if (imp)
	{
		delete imp;
		imp = 0;
	}
}
// end VS_SmtpMailer::~VS_SmtpMailer

bool VS_SmtpMailer::Send( void )
{	return !imp ? false : imp->Send();		}
// end VS_SmtpMailer::Send

void VS_SmtpMailer::To( const wchar_t *to )
{	if (imp)	imp->To( to );	}
// end VS_SmtpMailer::To

void VS_SmtpMailer::BCC( const wchar_t *bcc )
{	if (imp)	imp->BCC( bcc );	}
// end VS_SmtpMailer::Bcc

void VS_SmtpMailer::ReplyTo( const wchar_t *rpt )
{	if (imp)	imp->ReplyTo( rpt );	}
// end VS_SmtpMailer::ReplyTo

void VS_SmtpMailer::From( const wchar_t *from )
{	if (imp)	imp->From( from );	}
// end VS_SmtpMailer::From

void VS_SmtpMailer::CC( const wchar_t *cc )
{	if (imp)	imp->CC( cc );	}
// end VS_SmtpMailer::CC

void VS_SmtpMailer::Subject( const wchar_t *subject )
{	if (imp)	imp->Subject( subject );	}
// end VS_SmtpMailer::Subject

void VS_SmtpMailer::SetTextBody( const wchar_t *text )
{	if (imp)	imp->SetTextBody( text );	}
// end VS_SmtpMailer::SetTextBody

void VS_SmtpMailer::SetHTMLBody( const wchar_t *htmlText )
{	if (imp)	imp->SetHTMLBody( htmlText );	}
// end VS_SmtpMailer::SetHTMLBody

bool VS_SmtpMailer::SetMHTMLBody( const wchar_t *url )
{	return !imp ? false : imp->SetMHTMLBody( url );		}
// end VS_SmtpMailer::SetMHTMLBody

bool VS_SmtpMailer::SetStreamBody( VS_SmtpBodyType type )
{	return !imp ? false : imp->SetStreamBody( type );	}
// end VS_SmtpMailer::SetStreamBody

bool VS_SmtpMailer::AddAttachment( const wchar_t *filePath )
{	return !imp ? false : imp->AddAttachment( filePath );	}
// end VS_SmtpMailer::AddAttachment

bool VS_SmtpMailer::LoadTextBody( const wchar_t *filePath )
{	return !imp ? false : imp->LoadTextBody( filePath );	}
// end VS_SmtpMailer::LoadTextBody

bool VS_SmtpMailer::LoadHTMLBody( const wchar_t *filePath )
{	return !imp ? false : imp->LoadHTMLBody( filePath );	}
// end VS_SmtpMailer::LoadHTMLBody

bool VS_SmtpMailer::LoadFromFile( const wchar_t *filePath )
{	return !imp ? false : imp->LoadFromFile( filePath );	}
// end VS_SmtpMailer::LoadFromFile

bool VS_SmtpMailer::InsertImage( const wchar_t *filePath, const wchar_t *contentId )
{	return !imp ? false : imp->InsertImage( filePath, contentId );	}
// end VS_SmtpMailer::InsertImage

bool VS_SmtpMailer::SaveToFile( const wchar_t *filePath )
{	return !imp ? false : imp->SaveToFile( filePath );	}
// end VS_SmtpMailer::SaveToFile
/*
bool VS_SmtpMailer::SetAsText( const _bstr_t buffer )
{	return !imp ? false : imp->SetAsText( buffer );		}
// end VS_SmtpMailer::SetAsText

bool VS_SmtpMailer::GetAsText( _bstr_t *text, int &size )
{	return !imp ? false : imp->GetAsText( text, size );		}
// end VS_SmtpMailer::GetAsText
*/
//////////////////////////////////////////////////////////////////////////////////////////
bool VS_SmtpMailer::IsSendErrorCritical()
{	return !imp ? false : imp->IsLastErrorCritical();	}
//////////////////////////////////////////////////////////////////////////////////////////
VS_SmtpMailer::VS_SmtpMailer( const char * serverName, int port, VS_SmtpAuthType authType,
							  const char *login, const char *password, bool useSSL )
{	imp = new VS_SmtpMailer_Implementation( serverName, port, authType, login, password, useSSL );		}
// end VS_SmtpMailer::VS_SmtpMailer

void VS_SmtpMailer::To( const char * to )
{	if (imp)	imp->To( to );	}
// end VS_SmtpMailer::To
bool VS_SmtpMailer::GetTo(char ** in , unsigned size)
{
	if (imp)
		return imp->GetTo( in , size );
	return false;
}
bool VS_SmtpMailer::GetFrom(char ** in , unsigned size)
{
	if (imp)
		return imp->GetFrom( in , size );
	return false;
}

// end VS_SmtpMailer::GetAllocTo
bool VS_SmtpMailer::AddField(const char * fld_name,const _bstr_t& fld_cont)
{	if (imp)	return imp->AddField( fld_name , fld_cont );
	return false;
}
// end VS_SmtpMailer::AddField
void VS_SmtpMailer::BCC( const char *bcc )
{	if (imp)	imp->BCC( bcc );	}
// end VS_SmtpMailer::Bcc

void VS_SmtpMailer::ReplyTo( const char *rpt )
{	if (imp)	imp->ReplyTo( rpt );	}
// end VS_SmtpMailer::ReplyTo

void VS_SmtpMailer::From( const char *from )
{	if (imp)	imp->From( from );	}
// end VS_SmtpMailer::From

void VS_SmtpMailer::CC( const char *cc )
{	if (imp)	imp->CC( cc );	}
// end VS_SmtpMailer::CC

void VS_SmtpMailer::Subject( const char *subject )
{	if (imp)	imp->Subject( subject );	}
// end VS_SmtpMailer::Subject

void VS_SmtpMailer::SetTextBody( const char *text )
{	if (imp)	imp->SetTextBody( text );	}
// end VS_SmtpMailer::SetTextBody

void VS_SmtpMailer::SetHTMLBody( const char *htmlText )
{	if (imp)	imp->SetHTMLBody( htmlText );	}
// end VS_SmtpMailer::SetHTMLBody

bool VS_SmtpMailer::SetMHTMLBody( const char *url )
{	return !imp ? false : imp->SetMHTMLBody( url );		}
// end VS_SmtpMailer::SetMHTMLBody

bool VS_SmtpMailer::AddAttachment( const char *filePath )
{	return !imp ? false : imp->AddAttachment( filePath );	}
// end VS_SmtpMailer::AddAttachment

bool VS_SmtpMailer::AddAttachmentManual( const char* data )
{	return !imp ? false : imp->AddAttachmentManual( data );	}
// end VS_SmtpMailer::AddAttachmentManual

bool VS_SmtpMailer::LoadTextBody( const char *filePath )
{	return !imp ? false : imp->LoadTextBody( filePath );	}
// end VS_SmtpMailer::LoadTextBody

bool VS_SmtpMailer::LoadHTMLBody( const char *filePath )
{	return !imp ? false : imp->LoadHTMLBody( filePath );	}
// end VS_SmtpMailer::LoadHTMLBody

bool VS_SmtpMailer::LoadFromFile( const char *filePath )
{	return !imp ? false : imp->LoadFromFile( filePath );	}
// end VS_SmtpMailer::LoadFromFile

bool VS_SmtpMailer::InsertImage( const char *filePath, const char *contentId )
{	return !imp ? false : imp->InsertImage( filePath, contentId );	}
// end VS_SmtpMailer::InsertImage

bool VS_SmtpMailer::SaveToFile( const char *filePath )
{	return !imp ? false : imp->SaveToFile( filePath );	}
// end VS_SmtpMailer::SaveToFile
//////////////////////////////////////////////////////////////////////////////////////////
