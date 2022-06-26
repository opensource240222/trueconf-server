//////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 12.11.02     This version is taken from NAS projects and advanced by A.Slavetsky.
//
//////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_SmtpMailer.h
/// \brief
/// \note
///

#ifndef VS_SMTP_MAILER_H
#define VS_SMTP_MAILER_H

class _bstr_t;
#define   DEFAULT_BUFFER_SIZE   51200

enum VS_SmtpBodyType {		VS_TextUnknown = 0, VS_TextPlain, VS_TextHtml		};

enum VS_SmtpAuthType { VS_NoAuth, VS_SimplePass, VS_NTLM };

class VS_SmtpMailer
{
public:
	VS_SmtpMailer( const wchar_t *serverName, int port, VS_SmtpAuthType authType, const char *login, const char *password,
				   bool useSSL );
	VS_SmtpMailer( const char * serverName, int port, VS_SmtpAuthType authType, const char *login, const char *password,
				   bool useSSL );
	~VS_SmtpMailer();
	struct VS_SmtpMailer_Implementation   *imp;
	bool	IsValid() const { return imp != 0; }
/////////////////////////////////////////////////////////////////////////////////////////
	void	To				( const char *to );
	bool 	GetTo			( char **in , unsigned size );
	bool	GetFrom			( char **in , unsigned size );
	bool	AddField		( const char * fld_name,const _bstr_t& fld_cont);
	void	From			( const char *from );
	void	CC				( const char *cc );
	void	BCC				( const char *bcc );
	void	ReplyTo			( const char *rpt );
	void	Subject			( const char *subject );
	void	SetTextBody		( const char *text );
	void	SetHTMLBody		( const char *htmlText );
	bool	SetMHTMLBody	( const char *url );
	bool	AddAttachment	( const char *filePath );
	bool	LoadTextBody	( const char *filePath );
	bool	LoadHTMLBody	( const char *filePath );
	bool	LoadFromFile	( const char *filePath );
	bool	InsertImage		( const char *filePath, const char *contentId =NULL );
	bool	SaveToFile		( const char *filePath );
	bool    IsSendErrorCritical();
//	bool	SetAsText		( const char *buffer );
//	bool	GetAsText		( char ** text, int &size );

	bool	Send			( void );
	void	To				( const wchar_t *to );
	void	From			( const wchar_t *from );
	void	CC				( const wchar_t *cc );
	void	BCC				( const wchar_t *bcc );
	void	ReplyTo		( const wchar_t *rpt );
	void	Subject			( const wchar_t *subject );
	void	SetTextBody		( const wchar_t *text );
	void	SetHTMLBody		( const wchar_t *htmlText );
	bool	SetMHTMLBody	( const wchar_t *url );
	bool	SetStreamBody	( VS_SmtpBodyType type );
	bool	AddAttachment	( const wchar_t *filePath );
	bool	AddAttachmentManual( const char* data );
	bool	LoadTextBody	( const wchar_t *filePath );
	bool	LoadHTMLBody	( const wchar_t *filePath );
	bool	LoadFromFile	( const wchar_t *filePath );
	bool	InsertImage		( const wchar_t *filePath, const wchar_t *contentId = NULL );
	bool	SaveToFile		( const wchar_t *filePath );
	//	bool	SetAsText		( const wchar_t *buffer );
//	bool	GetAsText		( _bstr_t *text, int &size );
};
// end VS_SmtpMailer class

#endif  // VS_SMTP_MAILER_H
