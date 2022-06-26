/**
 **************************************************************************
 * \file VS_MiscCommand.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief Stream miscellaneous command processing
 *
 * \b Project Client
 * \author SMirnovK
 * \date 08.02.2006
 *
 * $Revision: 2 $
 *
 * $History: VS_MiscCommand.cpp $
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 27.04.11   Time: 10:25
 * Updated in $/VSNA/VSClient
 * - were added InfoRcvMFormat command (subtype = VS_CMNDSTYPE_INFO)
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 16.12.06   Time: 13:44
 * Updated in $/VS/VSClient
 * - change interface TimeDelay() command
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 14.12.06   Time: 12:54
 * Updated in $/VS/VSClient
 * - changed command intraface for receiver
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 13.12.06   Time: 18:33
 * Updated in $/VS/VSClient
 * - comands in NHP intrfaces
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 12.12.06   Time: 15:56
 * Updated in $/VS/VSClient
 * - new bitrate control module
 * - comand interfase changed
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 14.11.06   Time: 17:49
 * Updated in $/VS/VSClient
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

#include "VS_MiscCommandProc.h"

VS_CmndProc g_cmdProc;

VS_CmndProc::VS_CmndProc()
{
	m_SeqId = 0;
	m_CmdEvent = CreateEvent(0, 0, 0, 0);
}

VS_CmndProc::~VS_CmndProc()
{
	CloseHandle(m_CmdEvent); m_CmdEvent = 0;
}

bool VS_CmndProc::ReadRcvCommand(stream::Command& cmd, bool del)
{
	VS_AutoLock lock(this);
	if (m_rcv_list.empty())
		return false;
	cmd = m_rcv_list.front();
	if (del)
		m_rcv_list.pop_front();
	return true;
}

bool VS_CmndProc::GetSndCommand(void *buff, long &size)
{
	VS_AutoLock lock(this);
	size = 0;
	if (m_snd_list.empty())
		return false;
	auto& cmd = m_snd_list.front();
	size = cmd.Size();
	if (!buff)
		return false;
	memcpy(buff, &cmd, size);
	m_snd_list.pop_front();
	return true;
}

void VS_CmndProc::AddCommand(stream::Command& cmd, bool snd)
{
	VS_AutoLock lock(this);
	if (snd)
		m_snd_list.push_back(cmd);
	else
		m_rcv_list.push_back(cmd);
	SetEvent(m_CmdEvent);
}