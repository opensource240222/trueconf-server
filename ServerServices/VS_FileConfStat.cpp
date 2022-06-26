#ifdef _WIN32 // not ported
#include "VS_FileConfStat.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../ServersConfigLib/VS_ServersConfigLib.h"

#include <algorithm>
#include <fstream>

#include <Windows.h>

VS_FileConfStat::VS_FileConfStat(): m_buff(0), m_fname(0), n_line(0)
{
	m_buff = new char[2048];	memset(m_buff, 0, 2048);
	m_filebuff = new wchar_t[2048]; memset(m_filebuff, 0, 2048 * sizeof(wchar_t));
}

VS_FileConfStat::~VS_FileConfStat()
{
	free(m_fname); m_fname = 0;
	delete[] m_buff; m_buff = 0;
	delete[] m_filebuff; m_filebuff = 0;
}

void VS_FileConfStat::SaveToFileConfStat(VS_Container& cnt, const TConferenceStatistics* stat, const VS_UserData& ud)
{
	const char* conf_id = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char* user_id = cnt.GetStrValueRef(CALLID_PARAM);

	//VS_UserData ud;
	//if ( !FindUser(user_id, ud) )
	//	return ;

	if ( !stat || !conf_id || !user_id || ud.m_displayName.empty())
		return ;

	// escape ";" in name
	auto dn = ud.m_displayName;
	std::replace(dn.begin(), dn.end(), ';', '_');
	InitConfStatFile();

	std::fstream log_file(m_filebuff, std::ios::app);
	if (!log_file.is_open())
		return ;

	memset(m_buff, 0, 2048);

	char start[512];
	char end[512];

	tu::TimeToGStr(stat->start_part_gmt, start, 512);
	tu::TimeToGStr(stat->end_part_gmt, end, 512);

    snprintf(m_buff, 2048, "%d;%s;%s;%s;%s;%s;%d;%d;%dx%d;%d;%d;%d;%d;%d;%.2f\r\n",
		++n_line, conf_id, user_id, dn.c_str(),
		start, end,
		stat->participant_time, stat->broadcast_time,
		stat->video_w, stat->video_h,
		stat->loss_rcv_packets,
		stat->avg_send_bitrate, stat->avg_rcv_bitrate,
		stat->avg_cpu_load, stat->avg_jitter, stat->avg_send_fps
		);

	log_file << m_buff;
}

void VS_FileConfStat::InitConfStatFile()
{
	memset(m_buff, 0, 2048);

	memset(m_filebuff, 0, 2048 * sizeof(wchar_t));

	if(!VS_GetWorkingDir(m_filebuff, 2048))
	{
		const char* tmp=getenv("TEMP");
		if(!tmp) tmp="C:\\";
		// we shall add filename after, so let's reserve some place.
		mbstowcs(m_filebuff, tmp, 2048 - 256);
	};

	SYSTEMTIME systime;
	GetSystemTime(&systime);

	char fname[64];
	sprintf(fname, "part%2.2u%2.2u.log", systime.wYear-2000, systime.wMonth);

	wcscat(m_filebuff, L"\\");

	size_t length = wcslen(m_filebuff);

	mbstowcs(m_filebuff + length, fname, 256);

	FILE* f = _wfopen(m_filebuff, L"rb");
	if (!f)
		n_line = 0;

	if (!m_fname || (_stricmp(fname, m_fname) != 0))
	{
		if ( f )
		{
			char buff[512];
			memset(buff, 0, 512);

			fseek(f, -500, SEEK_END);
			fread(buff, 500, 1, f);

			char* start = buff;
			char* end = start;
			char* _p = 0;
			while( (_p = (char*) strstr(end, "\r\n")) != 0 )
			{
				start = end;
				end = _p;		end += strlen("\r\n");
			}

			if (start != end)
				n_line = atoi(start);
		}

		if (m_fname) { delete m_fname; m_fname = 0; }
		m_fname = _strdup(fname);
	}

	if (f)	fclose(f);
}

void VS_FileConfStat::TrySaveToFileConfStat(VS_Container& cnt, const VS_UserData& ud)
{
	size_t size = 0;
	const void* buff = cnt.GetBinValueRef(CONF_BASE_STAT_PARAM, size);
	if (buff && size && (size==sizeof(TConferenceStatistics)) )
		SaveToFileConfStat(cnt, (const TConferenceStatistics*) buff, ud);
}
#endif