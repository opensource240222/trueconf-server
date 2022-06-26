#ifndef VS_CORESTATEMACHINE_H
#define VS_CORESTATEMACHINE_H

#include "VS_CorePinAnalyser.h"
#include "VS_OleProtoHelper.h"

#include "../std/cpplib/VS_SimpleStr.h"
#include "VSClient/VS_Dmodule.h"
#include "VSClient/VSClientBase.h"
#include "VSClient/VSTrClientProc.h"
#include "../transport/client/VS_TransportClient.h"

class VS_AddressBookManager;
class VS_LoginManager;
class VS_ChatManager;
class VS_ServerStateManager;
class VS_ConferenceManager;

class VS_CoreStateMachine : public VS_CoreFlag, public CVSThread
{
protected:
	VS_SimpleStr m_user;	// login
	VS_SimpleStr m_pass;	// password

	// Delphi compability mode
	DWORD_PTR *superFlags;
	char *superReg1;
	char *superReg2;
	DWORD_PTR confHandle;
	HANDLE m_threadSync;
	HANDLE m_helperSync;
	CVSTrClientProc			*m_ContP;	// client proc
	VS_AddressBookManager	*m_abm;		// address bock handler
	VS_LoginManager			*m_login;	// login handler
	VS_ChatManager			*m_chat;	// chat handler
	VS_ServerStateManager	*m_ssm;		// server state handler
	VS_ConferenceManager	*m_conf;	// conference handler
	VS_OleProtoHelper		*m_helper;	// protocol helper class

	virtual DWORD Loop(LPVOID lpParameter);
	void Shutdown();

public:
	VS_CoreStateMachine(VS_AddressBookManager *abm, VS_LoginManager *lgm, VS_ChatManager *chm, VS_ServerStateManager *ssm, VS_ConferenceManager *conf);
	virtual ~VS_CoreStateMachine();
	virtual void OnRaise(core::pin pin, unsigned long option, unsigned long ext);
	virtual void OnFall(core::pin pin, unsigned long option, unsigned long ext);
	virtual void OnNotify(unsigned long option, unsigned long ext);
	VS_AddressBookManager& AddressBookManager();
	VS_LoginManager& LoginManager();
	VS_ChatManager& ChatManager();
	VS_ServerStateManager& ServerStateManager();
	VS_ConferenceManager& ConferenceManager();
	VS_OleProtoHelper& ProtocolHelper();
};

#endif