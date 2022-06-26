/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron Registration Server
 *
 * \file VS_RegStorage.h
 * This is the abstract class, base of directory server
 * storage class
 *
 * $Revision: 11 $
 * $History: VS_RegStorage.h $
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 16.03.12   Time: 15:54
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Add OS and CPU info in Regiatration Server
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 25.03.11   Time: 15:30
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - check conn required lic in offline registration
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
 * User: Mushakov     Date: 30.10.09   Time: 14:49
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Sign added in TransportHandshake
 *  - Sign Verify added in TransportRouter
 *  - hserr_verify_failed transport result code added
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
 * User: Mushakov     Date: 31.03.09   Time: 17:36
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
 * User: Smirnov      Date: 4.03.04    Time: 21:44
 * Updated in $/VS/Servers/DirectoryServer/Services
 * collected registry for servers
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 30.01.04   Time: 18:39
 * Updated in $/VS/Servers/DirectoryServer/Services
 * добавил правильное поведение в случае поломки базы
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 28.01.04   Time: 20:09
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added license update logic
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 26.01.04   Time: 16:21
 * Updated in $/VS/Servers/DirectoryServer/Services
 * IP logging changed
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
#ifndef VS_REG_STORAGE_H
#define VS_REG_STORAGE_H


#include "../../ServerServices/Common.h"
#include "../../ServerServices/VS_MediaBrokerStats.h"
#include "transport/VS_ServCertInfoInterface.h"
#include "../../common/tools/Watchdog/VS_Testable.h"

#include <map>

class VS_FileTime;
class VS_License;

/**
 *  Abstract class for registration server storage
 *
 */
class VS_RegStorage : public VS_ServCertInfoInterface, public VS_Testable
{
public:

    int             error_code;

    VS_RegStorage() :error_code(0) {};

    virtual ~VS_RegStorage(){};

		enum BrokerStates
		{
			BS_DISCONNECTED=0,
			BS_INVALID_KEY,
			BS_TEMPORARY,
			BS_VALID,
			BS_UNAUTHORIZED

		};

		virtual bool CleanUp()=0;

		virtual bool LogEvent(VS_BrokerEvents type,const VS_SimpleStr& server_name=0,const char* f1=0,const char* f2=0,const char* f3=0,BrokerStates state=BS_VALID,bool by_server_name = true)=0;
		virtual bool LogStats(const VS_SimpleStr& server_name,const VS_MediaBrokerStats* stats)=0;
		virtual bool LogIP(const VS_SimpleStr& server_name,const char* ip)=0;

		/**
			значения regRes:
			  1 - OK
			  0 - неправильный код или server_id
			  2 - сервер hardware locked;
			  3 - имя сервера уже занято
			  4 - перезапрос сертификата запрещен
			  5 - нет действующих лицензий/сертификата (или до их истечения менее 5 дней);

			  -1 - не удается выдать сертификат	(проблемы CA)
		*/
		virtual bool RegisterServer(const VS_SimpleStr& server_id,const VS_SimpleStr &server_name, const VS_SimpleStr& hwkey,
									const VS_SimpleStr& genkey,const VS_SimpleStr& serial,
									const VS_SimpleStr& pub_key,
									VS_WideStr &organization_name, VS_WideStr &country,
									VS_WideStr &contact_person, VS_WideStr &contact_email,
									unsigned long &cert_serial_number,
									VS_FileTime &notBefore, VS_FileTime &notAfter, VS_SimpleStr &out_key,
									int &regRes)=0;
		virtual bool SetServerCertificate(const VS_SimpleStr &srver_name, const VS_SimpleStr &cert) = 0;
		virtual bool GetMaxLicTime(const VS_SimpleStr &server_name, VS_FileTime &notAfter) = 0;
		virtual bool GetNewCertificateId(const VS_SimpleStr &server_name, const VS_FileTime &exp_date, unsigned long &cert_serial_num) = 0;


		virtual bool GetArmHwKey(const VS_SimpleStr &server_name, unsigned long &arm_hw_key, VS_SimpleStr &arm_key, unsigned long &hw_lock) = 0;
		virtual bool SetArmHwKey(const VS_SimpleStr &server_name, const unsigned long arm_hw_key, const VS_SimpleStr &arm_key) = 0;
		virtual bool GetLicenses(const VS_SimpleStr& server, std::map<uint64_t, VS_License>& licenses, bool by_server_name = true) = 0;

		virtual bool SetState(const VS_SimpleStr& server_name,BrokerStates state)=0;
		virtual BrokerStates GetState(const VS_SimpleStr& server_name)=0;

		virtual bool GetConfigCorrectorData(const VS_SimpleStr& server_name, VS_SimpleStr &result) = 0;
};


#endif //VS_REGSERVER_STORAGE_H