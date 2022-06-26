#ifndef VS_COREPINANALYSER_H
#define VS_COREPINANALYSER_H

#include <windows.h>
#include "../VSClient/VSTrClientProc.h"

namespace core {
	enum pin {
		loggedin	= STATUS_LOGGEDIN,
		servavail	= STATUS_SERVAVAIL,
		incall		= STATUS_INCALL,
		conference	= STATUS_CONFERENCE,
		userinfo	= STATUS_USERINFO,
		message		= STATUS_MESSAGE,
		command		= STATUS_COMMAND,
		reqinvite	= STATUS_REQINVITE
	};
}

class VS_CoreFlag
{
private:
	unsigned long m_flag;

public:
	VS_CoreFlag();
	virtual ~VS_CoreFlag();
	bool GetPin(core::pin pin);
	void SetPin(core::pin pin, bool enable);
	void SetFlag(unsigned long flag, unsigned long option = 0, unsigned long ext = 0);
	virtual void OnRaise(core::pin pin, unsigned long option = 0, unsigned long ext = 0) = 0;
	virtual void OnFall(core::pin pin, unsigned long option = 0, unsigned long ext = 0) = 0;
};

#endif