/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron Directory Server
 *
 *
 * $Revision: 14 $
 * $History: VS_RegDBStorage.h $
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 16.03.12   Time: 15:54
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Add OS and CPU info in Regiatration Server
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 28.03.11   Time: 15:35
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - db_null added
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 25.03.11   Time: 15:30
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - check conn required lic in offline registration
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 23.03.11   Time: 17:49
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Server Name Verification added at registration
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 23.02.11   Time: 4:05
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - SecureHandshake ver. 2 added
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 8  *****************
 * User: Dront78      Date: 18.02.10   Time: 10:09
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * Armadillo Security Passport added
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 12.02.10   Time: 15:56
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - ARM Keys added
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 19.11.09   Time: 15:12
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - VS_ServCertInfoInterface::GetPublicKey modified
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:58
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 26.10.09   Time: 20:32
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - sign verification added
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - VCS 3
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 31.03.09   Time: 17:35
 * Created in $/VSNA/Servers/RegistryServer/Services
 * RegistryServer added
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 22.03.07   Time: 16:18
 * Updated in $/VS2005/Servers/DirectoryServer/Services
 * добавлена верификация ProductType при регистрации брокера
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:51
 * Created in $/VS2005/Servers/DirectoryServer/Services
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 2.05.06    Time: 17:56
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 26.04.06   Time: 16:33
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 25.04.06   Time: 15:54
 * Updated in $/VS/Servers/DirectoryServer/Services
 * Added certificate issue
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 30.01.04   Time: 18:39
 * Updated in $/VS/Servers/DirectoryServer/Services
 * добавил правильное поведение в случае поломки базы
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 28.01.04   Time: 20:09
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added license update logic
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 26.01.04   Time: 16:21
 * Updated in $/VS/Servers/DirectoryServer/Services
 * IP logging changed
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 9.01.04    Time: 13:34
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added state save to DB
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 30.12.03   Time: 18:14
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added getkey func
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 29.12.03   Time: 18:27
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added RegisterBroker method
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 25.12.03   Time: 21:22
 * Created in $/VS/Servers/DirectoryServer/Services
 * added registration service and storage
 *
 ****************************************************************************/

#ifndef VS_REGISTRATION_DB_STORAGE_H
#define VS_REGISTRATION_DB_STORAGE_H

#include "VS_RegStorage.h"
#include "../../common/std/cpplib/VS_Lock.h"
#include "tlb_import/msado26.tlh"

/**
 *  Abstract class for storage, capable of storing
 *
 */

class VS_RegDBStorage :public VS_RegStorage, public VS_Lock
{
		static const	int reconnect_max      = 2;
		static const	int reconnect_timeout  = 10000;

		ADODB::_ConnectionPtr db;
		ADODB::_CommandPtr log_event, log_stats,log_ip;
		ADODB::_CommandPtr register_broker,broker_set_state, broker_get_licenses;
		ADODB::_CommandPtr reg_cleanup;
		ADODB::_CommandPtr broker_get_public_key_by_server_name;
		ADODB::_CommandPtr broker_set_arm_hw_key, broker_get_arm_hw_key;
		ADODB::_CommandPtr broker_get_max_license, broker_set_certificate,
			broker_get_certificate, broker_get_new_certificateId;
		ADODB::_CommandPtr get_call_cfg_corrector;
		_variant_t		db_null;
		enum States
		{STATE_CREATED=0,STATE_RUNNING,STATE_RECONNECT,STATE_FAILED};
		int state;
		void ProcessCOMError(_com_error e);
		VS_Map m_brokers;
		bool IsPostgreSQL;
public:
		virtual bool		Test( void )
		{return state==STATE_FAILED?false:true;};

    VS_RegDBStorage();
    virtual ~VS_RegDBStorage(){};
    bool Init();
	bool CleanUp() override;
	bool LogEvent(
		VS_BrokerEvents type,const VS_SimpleStr& server_name=0,
		const char* f1=0,const char* f2=0,const char* f3=0,
		BrokerStates state=BS_VALID,bool by_server_name = true) override;
	bool LogStats(
		const VS_SimpleStr& server_name,
		const VS_MediaBrokerStats* stats) override;
	bool LogIP(const VS_SimpleStr& server_name,const char* ip) override;

	bool RegisterServer(
		const VS_SimpleStr& server_id, const VS_SimpleStr &server_name,
		const VS_SimpleStr& hwkey, const VS_SimpleStr& genkey,
		const VS_SimpleStr& serial,	const VS_SimpleStr& pub_key,
		VS_WideStr &organization_name, VS_WideStr &country,
		VS_WideStr &contact_person, VS_WideStr &contact_email,
		unsigned long &cert_serial_number,
		VS_FileTime &notBefore, VS_FileTime &notAfter,
		VS_SimpleStr &out_key, int &regRes) override;

	bool SetServerCertificate(
		const VS_SimpleStr &srver_name,
		const VS_SimpleStr &cert) override;
	bool GetMaxLicTime(
		const VS_SimpleStr &server_name,
		VS_FileTime &notAfter) override;
	VS_ServCertInfoInterface::get_info_res GetServerCertificate(
		const VS_SimpleStr &server_name,
		VS_SimpleStr &cert) override;
	bool GetNewCertificateId(
		const VS_SimpleStr &server_name, const VS_FileTime &exp_date,
		unsigned long &cert_serial_num) override;

	bool GetArmHwKey(
		const VS_SimpleStr &server_name, unsigned long &arm_hw_key,
		VS_SimpleStr &arm_key, unsigned long &hw_lock) override;
	bool SetArmHwKey(
		const VS_SimpleStr &server_name, const unsigned long arm_hw_key,
		const VS_SimpleStr &arm_key) override;
	VS_ServCertInfoInterface::get_info_res GetPublicKey(
		const VS_SimpleStr& server_name, VS_SimpleStr &pub_key,
		uint32_t &vcs_ver) override;

	bool GetLicenses(const VS_SimpleStr& server, std::map<uint64_t, VS_License>& licenses, bool by_server_name = true) override;

	bool SetState(const VS_SimpleStr& server_name,BrokerStates state) override;
	virtual BrokerStates GetState(const VS_SimpleStr& server_name) override;

	bool GetConfigCorrectorData(const VS_SimpleStr& server_name,
		VS_SimpleStr &result);

};
#endif //VS_REGISTRATION_DB_STORAGE_H