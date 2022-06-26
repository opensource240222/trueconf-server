#include "VS_SIPField_Date.h"
#include "VS_SIPObjectFactory.h"
#include "std-generic/cpplib/TimeUtils.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/clib/vs_time.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Date::e(
	"(?i)"
	" *(?:date) *: *"
	"(\\w{3},){0,1} "	// day of week (not necessary) Mon, Tue ...
	"(\\d{2}) *"		// day
	"(\\w{3}) *"		// month (Jan, Feb ... )
	"(\\d{4}) *"		// year
	"(\\d{2}):(\\d{2}):(\\d{2}) GMT"	// hours:minutes:seconds (rfc 3261 restricts the time zone in SIP-date to "GMT")
	" *(?-i)"
	);

VS_SIPField_Date::VS_SIPField_Date(): m_date(0){
	Clean();
}

TSIPErrorCodes VS_SIPField_Date::Init(const VS_SIPGetInfoInterface& call){

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Date::Encode(VS_SIPBuffer &aBuffer) const{
	aBuffer.AddData("Date: ");
	aBuffer.AddData(tu::TimeToString(std::chrono::system_clock::now(), RFC822_TIME_FMT, false));

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Date::Decode(VS_SIPBuffer &aBuffer){
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if (TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	if (!ptr || !ptr_sz)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Date::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Date Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	int iDayOfWeek = 0;		const std::string &day_of_week = m[1];
	int iDay = 0;			const std::string &day = m[2];
	int iMonth = 0;			const std::string &month = m[3];
	int iYear = 0;			const std::string &year = m[4];
	int iHours = 0;		 	const std::string &hours = m[5];
	int iMinutes = 0;		const std::string &minutes = m[6];
	int iSeconds = 0;		const std::string &seconds = m[7];

	if (!day_of_week.empty()){ iDayOfWeek = DayOfWeekToInt(day_of_week); }
	if (!day.empty()) iDay = strtol(day.c_str(), nullptr, 10);
	if (!month.empty()) iMonth = MonthToInt(month);
	if (!year.empty()) iYear = strtol(year.c_str(), nullptr, 10);
	if (!hours.empty()) iHours = strtol(hours.c_str(), nullptr, 10);
	if (!minutes.empty()) iMinutes = strtol(minutes.c_str(), nullptr, 10);
	if (!seconds.empty()) iSeconds = strtol(seconds.c_str(), nullptr, 10);

	time(&m_date);
	tm timeinfo{};
	gmtime_r(&m_date, &timeinfo);

	timeinfo.tm_wday = iDayOfWeek;
	timeinfo.tm_mday = iDay;
	timeinfo.tm_mon = iMonth;
	timeinfo.tm_year = iYear - 1900;
	timeinfo.tm_hour = iHours;
	timeinfo.tm_min = iMinutes;
	timeinfo.tm_sec = iSeconds;

	m_date = mktime(&timeinfo);
	if (m_date == -1) return TSIPErrorCodes::e_badObjectState;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

int VS_SIPField_Date::DayOfWeekToInt(const std::string &day)
{
	if (strncasecmp(day.c_str(), "Sun",3) == 0){
		return 0;
	}
	if (strncasecmp(day.c_str(), "Mon", 3) == 0){
		return 1;
	}
	if (strncasecmp(day.c_str(), "Tue", 3) == 0){
		return 2;
	}
	if (strncasecmp(day.c_str(), "Wed", 3) == 0){
		return 3;
	}
	if (strncasecmp(day.c_str(), "Thu", 3) == 0){
		return 4;
	}
	if (strncasecmp(day.c_str(), "Fri", 3) == 0){
		return 5;
	}
	if (strncasecmp(day.c_str(), "Sat", 3) == 0){
		return 6;
	}
	return -1;
}

int VS_SIPField_Date::MonthToInt(const std::string &month)
{
	if (strcasecmp(month.c_str(), "Jan") == 0){
		return 0;
	}
	if (strcasecmp(month.c_str(), "Feb") == 0){
		return 1;
	}
	if (strcasecmp(month.c_str(), "Mar") == 0){
		return 2;
	}
	if (strcasecmp(month.c_str(), "Apr") == 0){
		return 3;
	}
	if (strcasecmp(month.c_str(), "May") == 0){
		return 4;
	}
	if (strcasecmp(month.c_str(), "Jun") == 0){
		return 5;
	}
	if (strcasecmp(month.c_str(), "Jul") == 0){
		return 6;
	}
	if (strcasecmp(month.c_str(), "Aug") == 0){
		return 7;
	}
	if (strcasecmp(month.c_str(), "Sep") == 0){
		return 8;
	}
	if (strcasecmp(month.c_str(), "Oct") == 0){
		return 9;
	}
	if (strcasecmp(month.c_str(), "Nov") == 0){
		return 10;
	}
	if (strcasecmp(month.c_str(), "Dec") == 0){
		return 11;
	}
	return -1;
}

std::unique_ptr<VS_BaseField> VS_SIPField_Date_Instance()
{
	return vs::make_unique<VS_SIPField_Date>();
}

#undef DEBUG_CURRENT_MODULE
