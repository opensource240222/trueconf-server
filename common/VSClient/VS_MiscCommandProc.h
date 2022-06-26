/**
 **************************************************************************
 * \file VS_MiscCommand.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief Stream miscellaneous command Processing
 *
 * \b Project Client
 * \author SMirnovK
 * \date 07.02.2006
 *
 * $Revision: 1 $
 *
 * $History: VS_MiscCommand.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 14.12.06   Time: 12:54
 * Updated in $/VS/VSClient
 * - changed command intraface for receiver
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 12.12.06   Time: 15:56
 * Updated in $/VS/VSClient
 * - new bitrate control module
 * - comand interfase changed
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 14.11.06   Time: 17:49
 * Updated in $/VS/vsclient
 * - Nhp-2 alfa
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 8.11.06    Time: 16:53
 * Updated in $/VS/VSClient
 * - NHP headers added
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 22.05.06   Time: 10:54
 * Updated in $/VS/VSClient
 * - comand queue as list
 * - sent frame queue as map
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 11.05.06   Time: 13:16
 * Created in $/VS/VSClient
 * - added stream command (alfa version)
 *
 ****************************************************************************/

#pragma once

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "../std/cpplib/VS_Lock.h"
#include "../streams/Command.h"

#include <list>

class VS_MediaFormat;

typedef std::list<stream::Command> VST_CmdList;
/**
 **************************************************************************
 * \brief Forming Command class
 ****************************************************************************/
class VS_CmndProc: private VS_Lock
{
	unsigned short	m_SeqId;		///< increment number in command
	void *			m_CmdEvent;		///< signaled in command forming functions
	VST_CmdList		m_rcv_list;
	VST_CmdList		m_snd_list;
public:
	VS_CmndProc();
	~VS_CmndProc();
	/// read command from rcved queue
	bool ReadRcvCommand(stream::Command& cmd, bool del = true);
	/// get command as raw data and delete it from sent queue
	bool GetSndCommand(void *buff, long &size);
	/// Add command to specified queue
	void AddCommand(stream::Command& cmd, bool snd = true);
	/// new command event
	void * GetCmdEvent() {return m_CmdEvent;}
};

extern VS_CmndProc g_cmdProc;

class VS_MiscCommandProcess
{
public:
	/// Send commands
	void CommandToSend(stream::Command& cmd) { g_cmdProc.AddCommand(cmd); }
	/// Analise incoming commands from stream
	virtual bool SetReceivedCommand(stream::Command& cmd) { return false; }
};