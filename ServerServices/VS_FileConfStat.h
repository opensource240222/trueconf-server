#include "../common/std/cpplib/VS_UserData.h"
#include "../common/std/statistics/TConferenceStatistics.h"

class VS_FileConfStat
{
public:
	VS_FileConfStat();
	virtual ~VS_FileConfStat();

	void SaveToFileConfStat(VS_Container& cnt, const TConferenceStatistics* stat, const VS_UserData& ud);
	void InitConfStatFile();
	void TrySaveToFileConfStat(VS_Container& cnt, const VS_UserData& ud);
protected:

private:
	// a buffer, that contains name of log file
	wchar_t* m_filebuff;
	char* m_buff;
	char* m_fname;
	int n_line;
};