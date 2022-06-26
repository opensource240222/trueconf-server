#pragma once

#include <set>

struct sRTPInfo
{
	unsigned int seq;
	unsigned int trackId;
	unsigned int rtptime;
};

enum eRequestType : int
{
	REQUEST_invalid = -1,
	REQUEST_ANNOUNCE,
	REQUEST_DESCRIBE,
	REQUEST_GET_PARAMETER,
	REQUEST_OPTIONS,
	REQUEST_PAUSE,
	REQUEST_PLAY,
	REQUEST_RECORD,
	REQUEST_REDIRECT,
	REQUEST_SETUP,
	REQUEST_SET_PARAMETER,
	REQUEST_TEARDOWN,
};
typedef std::set<eRequestType> CommandsSet;

enum eResponseType : int
{
	RESPONSE_invalid = -1,
	RESPONSE_100_CONTINUE,
	RESPONSE_200_OK,
	RESPONSE_304_Not_Modified,
	RESPONSE_400_Bad_Request,
	RESPONSE_503_Service_Unavailable
};