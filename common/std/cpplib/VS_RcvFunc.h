/**
 **************************************************************************
 * \file VS_RcvFunc.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Class for stream filtering for participants in multiconference
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 09.10.03
 *
 * $Revision: 2 $
 *
 * $History: VS_RcvFunc.h $
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 13.01.10   Time: 21:43
 * Updated in $/VSNA/std/cpplib
 * SS & DS support from old arch:
 * - ConnectServices added
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 19.11.03   Time: 14:11
 * Updated in $/VS/std/cpplib
 * defaults for multi
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 29.10.03   Time: 19:15
 * Updated in $/VS/std/cpplib
 * multi connects scheme
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 24.10.03   Time: 17:39
 * Updated in $/VS/std/cpplib
 * MULTI test client and other files
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 9.10.03    Time: 19:44
 * Created in $/VS/std/cpplib
 * new methods in client
 * new files in std...
 *
 ****************************************************************************/
#ifndef VS_RCV_FUNC_H
#define VS_RCV_FUNC_H

#include "../../streams/Protocol.h"

#include <cstring>

/****************************************************************************
 * Defines
 ****************************************************************************/
#define VS_RCV_FUNC_SEPARATOR "-<%%>-"

/**
 **************************************************************************
 * \brief contain participant reciever help functions
 ****************************************************************************/
class VS_RcvFunc
{
public:
	enum Fltr_Opt {
		FLTR_SILENT			= 0x00000000,
		FLTR_SENDER			= 0x00000010,
		FLTR_RECIEVER		= 0x00000020,
		FLTR_RCV_AUDIO		= 0x00000100,
		FLTR_RCV_VIDEO		= 0x00000200,
		FLTR_RCV_DATA		= 0x00000400,
		FLTR_RCV_STAT		= 0x00000800,
		FLTR_ALL_MEDIA		= FLTR_RCV_STAT|FLTR_RCV_AUDIO|FLTR_RCV_VIDEO|FLTR_RCV_DATA,
		FLTR_ALL_			= 0xffffffff,
		FLTR_DEFAULT_MULTIS	= FLTR_RCV_STAT|FLTR_RCV_AUDIO|FLTR_RCV_VIDEO|FLTR_RCV_DATA,
		FLTR_DEFAULT_PUBLIC = FLTR_ALL_MEDIA,
		FLTR_DEFAULT_PRIVAT = FLTR_ALL_MEDIA
	};
	static inline void SetName(char* name, const char *snd, const char *rcv){
		if (!name || !snd || !rcv) return;
		strcpy(name, snd);
		if (strcmp(snd, rcv)!=0) {
			strcat(name, VS_RCV_FUNC_SEPARATOR);
			strcat(name, rcv);
		}
	}
	static inline void GetNames(const char *name, char* snd, char *rcv){
		if (!name || !snd || !rcv) return;
		char copy[1024]; *copy = 0;
		strncpy(copy, name, 256);
		char * sub = strstr(copy, VS_RCV_FUNC_SEPARATOR);
		if (sub) {
			strcpy(rcv, sub+strlen(VS_RCV_FUNC_SEPARATOR));
			*sub = 0; // for snd
		}
		else {
			strcpy(rcv, copy); // = snd = name
		}
		strcpy(snd, copy);
	}
	static inline void SetTracks(long fltr, stream::Track* tracks, unsigned& nTracks) {
		nTracks = 0;
		if (fltr & FLTR_RCV_STAT) tracks[nTracks++] = stream::Track::old_command;
		if (fltr & FLTR_RCV_AUDIO)tracks[nTracks++] = stream::Track::audio;
		if (fltr & FLTR_RCV_VIDEO)tracks[nTracks++] = stream::Track::video;
		if (fltr & FLTR_RCV_DATA) tracks[nTracks++] = stream::Track::data;
	}
	static inline void SetFltr(long& fltr, const stream::Track* tracks, unsigned nTracks) {
		fltr = 0;
		if (nTracks>255) return;
		unsigned int i;
		for (i = 0; i<nTracks; i++) {
			if (tracks[i] == stream::Track::old_command) fltr|= FLTR_RCV_STAT;
			if (tracks[i] == stream::Track::audio      ) fltr|= FLTR_RCV_AUDIO;
			if (tracks[i] == stream::Track::video      ) fltr|= FLTR_RCV_VIDEO;
			if (tracks[i] == stream::Track::data       ) fltr|= FLTR_RCV_DATA;
		}
	}
};

#endif
