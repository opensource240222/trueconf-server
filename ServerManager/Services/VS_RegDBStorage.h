#pragma once

#include "../../common/tools/Watchdog/VS_Testable.h"
#include "../../ServerServices/VS_MediaBrokerStats.h"
#include "../../common/ldap_core/common/Common.h"
#include "tlb_import/msado26.tlh"

class VS_RegDBStorage: public VS_Testable
{
public:
	VS_RegDBStorage();
	virtual ~VS_RegDBStorage();

	bool Init();
	bool LogStats(const char* sid, const VS_MediaBrokerStats* stats);
	int GetAllAppProperties(VS_AppPropertiesMap &prop_map);

	bool Test( void ) { return true; }
	void ProcessCOMError(_com_error e);

protected:

private:
	bool			m_IsInit;
	static const	int reconnect_max			=2;
	static const	int reconnect_timeout	=10000;

	ADODB::_ConnectionPtr db;

	ADODB::_CommandPtr log_stats, app_prop_get_all_props;
	bool IsPostgreSQL;
};