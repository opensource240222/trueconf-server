/**
 ****************************************************************************
 *
 * (c) 2003-2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: License handling functions
 * File:    License class
 *
 * $Revision: 15 $
 * $History: VS_License.h $
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 24.07.12   Time: 18:20
 * Updated in $/VSNA/Servers/LicenseLib
 *  - flag MULTIGATEWAY_ALLOWED = 0x8000 supported
 *
 * *****************  Version 14  *****************
 * User: Ktrushnikov  Date: 10.05.11   Time: 20:12
 * Updated in $/VSNA/Servers/LicenseLib
 * - HQ_ALLOWED renamed to HD_ALLOWED
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 29.04.10   Time: 20:07
 * Updated in $/VSNA/Servers/LicenseLib
 * - memory leaks removed;
 * - descktop sharing supported (server)
 *
 * *****************  Version 12  *****************
 * User: Dront78      Date: 18.02.10   Time: 10:09
 * Updated in $/VSNA/Servers/LicenseLib
 * Armadillo Security Passport added
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 12.02.10   Time: 15:56
 * Updated in $/VSNA/Servers/LicenseLib
 * - ARM Keys added
 *
 * *****************  Version 10  *****************
 * User: Dront78      Date: 5.02.10    Time: 19:28
 * Updated in $/VSNA/Servers/LicenseLib
 * - added armadillo license checker
 * - added trial storage class
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 22.12.09   Time: 20:04
 * Updated in $/VSNA/Servers/LicenseLib
 *  - Ldap supported
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/LicenseLib
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:58
 * Updated in $/VSNA/Servers/LicenseLib
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 26.10.09   Time: 20:32
 * Updated in $/VSNA/Servers/LicenseLib
 *  - sign verification added
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/Servers/LicenseLib
 *  - VCS 3
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/LicenseLib
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 12.05.09   Time: 15:32
 * Updated in $/VS2005/Servers/LicenseLib
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 13.04.09   Time: 18:37
 * Updated in $/VS2005/Servers/LicenseLib
 * - http://projects.visicron.ru/bin/view/Projects/RegServer26
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/LicenseLib
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 29.08.06   Time: 13:30
 * Updated in $/VS/Servers/LicenseLib
 * added license start time
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 25.08.06   Time: 20:31
 * Updated in $/VS/Servers/LicenseLib
 * added initialization for gw variable
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 25.07.06   Time: 16:29
 * Updated in $/VS/Servers/LicenseLib
 * added roaming control in licenses
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 29.05.06   Time: 14:14
 * Updated in $/VS/Servers/LicenseLib
 * naming fix
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 13.01.06   Time: 16:42
 * Updated in $/VS/Servers/LicenseLib
 * Added SecureLib, SecureHandshake
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 29.01.04   Time: 14:54
 * Updated in $/VS/servers/licenselib
 * added grace period for licensing
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 28.01.04   Time: 20:09
 * Updated in $/VS/Servers/LicenseLib
 * added license update logic
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 26.01.04   Time: 15:40
 * Created in $/VS/Servers/LicenseLib
 * moved license things to lib
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 20.12.03   Time: 14:22
 * Updated in $/VS/std/cpplib
 * added new license fields and ID control
 *
 *
 ****************************************************************************/

#ifndef VS_LICENSE_H
#define VS_LICENSE_H

#include "../common/std/cpplib/VS_Errors.h"
#include "../common/std/cpplib/VS_IDTypes.h"
#include "SecureLib/VS_SecureConstants.h"

#include <cstdint>
#include <vector>
#include <chrono>
#include <memory>

class VS_RegistryKey;

static const size_t ARM_KEY_LENGTH = 512;


//unsigned long VS_CalcCRC(const void* data,int size);

class  VS_License
{
public:
	enum Flags
	{
		NOT_RESTRICTED				= 0x00,
		DEFAULT_FLAGS				= 0x00,
		REG_CONNECTED				= 0x01,
		GRACE_PERIOD				= 0x02,
		ROAMING_ALLOWED				= 0x04,
		ENCRYPT_ALLOWED				= 0x08,
		USER_GROUPS_ALLOWED			= 0x10,
		LDAP_ALLOWED				= 0x20,
		FILETRANSFER_ALLOWED		= 0x40,
		DSHARING_ALLOWED			= 0x80,
		VIDEORECORDING_ALLOWED		= 0x100,
		WHITEBOARD_ALLOWED			= 0x200,
		SLIDESHOW_ALLOWED			= 0x400,
		ASYMMETRICCONF_ALLOWED		= 0x800,
		ROLECONF_ALLOWED			= 0x1000,
		UDPMULTICAST_ALLOWED		= 0x2000,
		HD_ALLOWED					= 0x4000,
		MULTIGATEWAY_ALLOWED		= 0x8000,
		WEBRTC_BROADCAST_ALLOWED	= 0x10000,
		IMPROVED_SECURITY			= 0x20000,
		ENABLE_DIRECTORY			= 0x40000,
		PAID_SERVER					= 0x80000,
		ENABLE_WEBINARS				= 0x100000,

		ENABLE_CONF_BROADCAST		= 0x400000,
		ENABLE_SDK_CLIENTS			= 0x800000,

		// license sharing
		ENTERPRISE_MASTER					= 0x1000000,	// master
		ENTERPRISE_SLAVE					= 0x2000000		// slave

	};

	static const int TC_INFINITY=-2;

	enum Type
	{
		SIGNED_WITH_HW = 3
	};

#pragma pack(push,1)
	struct SignedHWLicense
	{
		uint32_t		type = 0;
		int64_t			validuntil = 0;		// windows ticks i.e. FILETIME
		int64_t			validafter = 0;		// windows ticks i.e. FILETIME
		int				onlineusers = 0;
		char			serverName[257] = {};///ServerName
		int64_t			id = 0;
		int				conferences = 0; //
		int				symmetric_participants = 0;

		int				terminal_pro_users = 0;
		unsigned 		trial_conf_minutes = 0;

		int				max_guests = 0;
		int64_t			restrict = 0;
		int				gateways = 0;
		char			md5_hwkey[33] = {};
		unsigned char	sign[VS_SIGN_SIZE] = {};

		bool Decode(const void * data, int length);
		bool Encode(std::unique_ptr<uint8_t[]>& outBuff, size_t& OUT_len);
		bool MakeSign(const char* privateKey);
		bool VerifySign(const char *publicKey);
		bool VerifySign();

		bool SetHWKey();
		int VerifyHWKey();
  };
#pragma pack(pop)

	int			m_error;
	uint64_t	m_id;
	int64_t		m_restrict;
	int			m_conferences;
	int			m_onlineusers;
	int			m_terminal_pro_users;
	int			m_symmetric_participants;
	int			m_max_guests;
	int         m_gateways;
	unsigned	m_trial_conf_minutes;

	std::chrono::system_clock::time_point		m_validafter;
	std::chrono::system_clock::time_point		m_validuntil;
	VS_SimpleStr	m_serverName;
	VS_SimpleStr	m_hw_md5;
	// When adding new value please consider adding it also to Serialize and Deserialize functions

private:
	void Init()
	{
		m_gateways=0;
		m_validafter= std::chrono::system_clock::time_point();
		m_restrict	= 0;
		m_conferences = 0;
		m_onlineusers = 0;
		m_terminal_pro_users = 0;
		m_symmetric_participants = 0;
		m_max_guests = 0;
		m_gateways = 0;
		m_trial_conf_minutes = 0;
	}

public:
	VS_License ()
		:m_error(VSS_LICENSE_NOT_VALID)
	{
		Init();
	}
	VS_License (const char* name)
		:m_error(VSS_LICENSE_NOT_VALID)
	{
		Init();
		ReadFile(name);
	}
	VS_License (const VS_RegistryKey& key)
		:m_error(VSS_LICENSE_NOT_VALID)
	{
		Init();
		ReadKey(key);
	}
	VS_License (const void* data,int length)
		:m_error(VSS_LICENSE_NOT_VALID)
	{
		Init();
		Decode(data,length);
	}
	virtual ~VS_License()
	{
	}
	bool ReadFile (const char* name);
	bool ReadKey  (const VS_RegistryKey& key);
	bool Decode   (const void* data,int length);
	void ConvertFromSigned(const SignedHWLicense& slic);
	void ConvertToSigned(SignedHWLicense& OUT_slic) const;
	// Serialize and Deserialize do not use signing/verifying with sertificates like VS_EncodeLicense or Decode functions
	bool Serialize(std::unique_ptr<uint8_t[]>& outBuff, size_t& OUT_len) const;
	bool Deserialize(const void* data, size_t length);

	bool IsValid() const;
	// Includes resourses and features provided by the license 'x' into this license.
	// Does nothing if license 'x' is invalid.
	void MergeLicense(const VS_License& x);

	void Clear() { Init(); }
	void SaveToRegistry(VS_RegistryKey& k) const;
	void ReadFromRegistry(VS_RegistryKey& k);
	void Print();

	// for countable license resourses that we can share
	VS_License DeductLicenseCopy(const VS_License& l) const;
	VS_License& DeductLicense(const VS_License& l);
	VS_License& AddLicence(const VS_License& lic);
	VS_License AddLicenceCopy(const VS_License& lic) const;
	bool CompareCountableResources(const VS_License& other) const;
	const VS_License ShareAvailable(const VS_License& to_share) const;
	bool HasSharedResources() const;

	// Helper functions that perform saturation arithmetic in range [0, INT_MAX] and support TC_INFINITY value.
	static int NumAdd(int l, int r);
	static int NumSub(int l, int r);
	static int NumMin(int a, int b);
	static int NumMax(int a, int b);
};


bool VS_EncodeLicense(const VS_License& lic,VS_License::Type type, uint8_t** buf,int* size,const std::vector<char> &p_key = std::vector<char>());

#endif // VS_LICENSE_H
