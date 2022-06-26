/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Common files
 * File:    License class implementation
 *
 * $Revision: 16 $
 * $History: VS_License.cpp $
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 5.10.11    Time: 21:36
 * Updated in $/VSNA/Servers/LicenseLib
 *  - ssl refactoring (SetCert interfaces)
 *
 * *****************  Version 15  *****************
 * User: Dront78      Date: 25.05.11   Time: 18:53
 * Updated in $/VSNA/Servers/LicenseLib
 * - armadillo optimizations disabled totally
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 6.05.11    Time: 20:43
 * Updated in $/VSNA/Servers/LicenseLib
 *  - new reg; new reg cert; cert chain supported in tc_server
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 3-12-10    Time: 16:30
 * Updated in $/VSNA/Servers/LicenseLib
 *  - VSGetServerIdentifyString added (7205)
 *  - sign/verify sign code in secure lib  was little corrected
 *  - SignVerifier added
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 23.03.10   Time: 17:40
 * Updated in $/VSNA/Servers/LicenseLib
 * - log events added
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
 * User: Mushakov     Date: 28.01.10   Time: 19:53
 * Updated in $/VSNA/Servers/LicenseLib
 * - offline registration supported (VCS)
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/LicenseLib
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 8.12.09    Time: 20:23
 * Updated in $/VSNA/Servers/LicenseLib
 *  - ConfRestriction Interface added
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
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 13.04.09   Time: 18:37
 * Updated in $/VS2005/Servers/LicenseLib
 * - http://projects.visicron.ru/bin/view/Projects/RegServer26
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/LicenseLib
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 6.12.06    Time: 14:20
 * Updated in $/VS/Servers/LicenseLib
 * del comments
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 6.12.06    Time: 13:21
 * Updated in $/VS/Servers/LicenseLib
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 6.12.06    Time: 13:08
 * Updated in $/VS/Servers/LicenseLib
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 5.12.06    Time: 19:36
 * Updated in $/VS/Servers/LicenseLib
 * read PrivateKey from registry
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 29.08.06   Time: 13:30
 * Updated in $/VS/Servers/LicenseLib
 * added license start time
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 25.07.06   Time: 16:29
 * Updated in $/VS/Servers/LicenseLib
 * added roaming control in licenses
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 29.05.06   Time: 14:14
 * Updated in $/VS/Servers/LicenseLib
 * naming fix
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 18.05.06   Time: 19:00
 * Updated in $/VS/Servers/LicenseLib
 * fixed VS_SIGN_SIZE using bug
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 13.01.06   Time: 16:42
 * Updated in $/VS/Servers/LicenseLib
 * Added SecureLib, SecureHandshake
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 5.08.04    Time: 18:14
 * Updated in $/VS/Servers/LicenseLib
 * fix in lic
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 5.08.04    Time: 17:45
 * Updated in $/VS/Servers/LicenseLib
 * Changed to safe range decoder
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
 * *****************  Version 6  *****************
 * User: Stass        Date: 23.12.03   Time: 16:59
 * Updated in $/VS/std/cpplib
 * added license reload
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 20.12.03   Time: 14:22
 * Updated in $/VS/std/cpplib
 * added new license fields and ID control
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 9.12.03    Time: 13:39
 * Updated in $/VS/std/cpplib
 * licenses now are read from registry
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 25.11.03   Time: 18:59
 * Updated in $/VS/std/cpplib
 * added licensing
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 24.11.03   Time: 13:07
 * Updated in $/VS/std/cpplib
 * RC license support
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 18.11.03   Time: 17:05
 * Created in $/VS/std/cpplib
 * added first version licensing
 *
 ****************************************************************************/

#include <stdio.h>

#include "VS_License.h"
#include "std-generic/clib/rangecd.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/std/cpplib/VS_RegistryConst.h"
#include "SecureLib/VS_Sign.h"

#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "ProtectionLib/HardwareKey.h"
#include "ProtectionLib/Lib.h"
#include "ProtectionLib/Protection.h"
#include "ProtectionLib/RSA.h"
#include "ProtectionLib/SHA1.h"
#include "../ServerServices/VS_ReadLicense.h"

#include "../common/std/cpplib/VS_Protocol.h"
#include "std/debuglog/VS_Debug.h"

#include "std-generic/compat/memory.h"
#include <cassert>
#include <cinttypes>
#include <climits>

#define VS_PRIVATE_KEY_PASSWORD	0
#define VS_PRIVATE_KEY_FILE	"privatekey.pem"
#define VS_PUBLIC_KEY_FILE "publickey.pem"
#define EMPTY_TIME std::chrono::system_clock::time_point()
#define DEBUG_CURRENT_MODULE VS_DM_REGS

// Modulus of the public RSA key of the RegistryServer.
// The real key has zero byte at position 78.
// We can't store that byte as is because VMProtectDecryptString doesn't support that.
// So we fix that byte when we use the value.
static constexpr char c_regsrv_pubkey_n[] =
	"\xE5\xC6\xB5\x56\x2A\x9A\x6D\xB8\xE9\x1F\xD3\x1A\xC3\xC0\xD5\x59"
	"\x19\x04\x36\xA3\x8A\xF0\x3D\x81\x44\x6E\x09\x58\xB7\xAD\x69\xE4"
	"\xEC\xE1\xA3\x93\xFC\xFB\x1F\xDE\x33\xB7\x1D\xD4\xF7\x1B\xF5\x52"
	"\xB2\x04\x35\x33\x17\x6A\xA6\xD5\x18\x87\x9F\xC4\x3C\xEB\x68\x4A"
	"\x27\x06\x38\x4E\xA1\xDB\xF3\xEB\x73\x3C\x86\xC6\x54\xff\x9C\x0C"
	"\xCB\x39\x5C\x6C\x3E\xCA\x4F\x15\xEE\x08\x81\x65\xCD\x39\xEB\xDA"
	"\x18\xA4\xEE\x5F\x0A\x0C\xF1\xC5\xD6\x65\x69\xE2\x1E\xEF\x23\x08"
	"\x30\x53\x60\xB5\x45\xE7\xBB\x8F\x58\x01\x7A\x90\x4B\xFC\x2A\xC6"
	"\x99\xF1\x80\x5C\xD6\xF4\xF8\x74\x44\x70\x6D\x28\x7C\xAC\x1E\x58"
	"\x11\x3E\x8F\x6C\x31\x1F\x42\xF2\x5C\xFE\x94\x5A\x85\x6A\xE9\x99"
	"\xE4\x2E\xD7\xBA\xD3\x2D\x83\xD6\x50\xD5\xE9\x45\xDC\xBA\x75\x85"
	"\x50\x08\x1B\xC1\x10\x3F\x41\x40\x18\xB2\x83\x01\x2A\x71\x89\xB6"
	"\x91\x91\x76\xA7\x6B\xFA\xAB\x2A\xAA\xB8\x5B\x4A\x5B\x53\x17\x21"
	"\x49\x17\xBD\x8A\x34\x3F\x56\xC2\x03\x85\x1F\x6A\x68\x21\xD3\x5E"
	"\x67\xD4\x63\xF0\x0A\x5B\x35\x43\xB3\x40\x69\x60\xB8\x7D\x12\x78"
	"\x84\xD3\x45\xE3\x98\x71\x3A\x8E\x34\x49\xE0\x26\x2B\x47\x3D\xA7";
static_assert(sizeof(c_regsrv_pubkey_n) == 256 + 1/*null-terminator*/, "");
static_assert(c_regsrv_pubkey_n[77] == '\xff', "");

// Public exponent of the public RSA key of the RegistryServer.
static constexpr unsigned c_regsrv_pubkey_e = 65537;

bool VS_License::ReadFile(const char* name)
{
	FILE* lic=fopen(name,"rb");
    if(lic==NULL)
	{
		m_error=VSS_FILE_READ_ERROR;
		return false;
	}
    fseek(lic,0,SEEK_END);
    long size=ftell(lic);
    if(size<1)
    {
		fclose(lic);
		m_error=VSS_FILE_READ_ERROR;
		return false;
	}
    auto buf = vs::make_unique<unsigned char[]>(size);
    fseek(lic,0,SEEK_SET);
    if (fread(buf.get(),1 ,size, lic) != size)
    {
		fclose(lic);
		m_error=VSS_FILE_READ_ERROR;
		return false;
	}
    bool result=Decode(buf.get(), size);
    fclose(lic);
    return result;
}

bool VS_License::ReadKey (const VS_RegistryKey& key)
{
	if(!key.IsValid())
	{
		m_error=VSS_REGISTRY_ERROR;
		return false;
	}

	int size;
	std::unique_ptr<void, free_deleter> buf;
	if ((size = key.GetValue(buf, VS_REG_BINARY_VT, LICENSE_DATA_TAG)) <= 0)
	{
		m_error=VSS_REGISTRY_ERROR;
		return false;
	}
    return Decode(buf.get(), size);
}

SECURE_FUNC
bool VS_License::Decode(const void* data, int size)
{
	if (size < 4)
	{
		m_error = VSS_LICENSE_NOT_VALID;
		return false;
	}

	switch (*static_cast<const uint32_t*>(data))
    {
	case SIGNED_WITH_HW:
	{
		SignedHWLicense slic;
		if (!slic.Decode(data, size))
		{
			m_error = VSS_LICENSE_VERIFY_ERROR;
			return false;
		}
		if (!slic.VerifySign())
		{
			m_error = VSS_LICENSE_VERIFY_ERROR;
			return false;
		}

		/**
			Вытащить md5 ключа из лицензии и из сертификата, проверить
		*/
		if (slic.md5_hwkey[0] != '\0')
		{
			m_error = slic.VerifyHWKey();
			if (m_error != 0)
				return false;
		}

		ConvertFromSigned(slic);
	};
		break;
    default:
		m_error=VSS_LICENSE_TYPE_UNKNOWN;
		Init(); // FIXME: Init() should be SECURE_FUNC_INTERNAL
		break;
	}

	return true;
}

SECURE_FUNC
void VS_License::ConvertFromSigned(const SignedHWLicense & slic)
{
	m_error = 0;

	m_id = slic.id;
	m_restrict = slic.restrict;
	m_conferences = slic.conferences;
	m_onlineusers = slic.onlineusers;
	m_serverName = slic.serverName;
	m_validuntil = tu::WindowsTickToUnixSeconds(slic.validuntil);
	m_gateways = slic.gateways;
	m_trial_conf_minutes = slic.trial_conf_minutes;
	m_validafter = tu::WindowsTickToUnixSeconds(slic.validafter);
	m_hw_md5 = slic.md5_hwkey;
	m_symmetric_participants = slic.symmetric_participants;
	m_terminal_pro_users = slic.terminal_pro_users;
	m_max_guests = slic.max_guests;
}

SECURE_FUNC
void VS_License::ConvertToSigned(SignedHWLicense & OUT_slic) const
{
	if (this->m_serverName && this->m_serverName.Length() <= 256)
		strncpy(OUT_slic.serverName, this->m_serverName, this->m_serverName.Length());
	else
		OUT_slic.serverName[0] = 0;

	memset(OUT_slic.md5_hwkey, 0, sizeof(OUT_slic.md5_hwkey));
	if (this->m_hw_md5.m_str != nullptr)
		strcpy(OUT_slic.md5_hwkey, this->m_hw_md5.m_str);

	OUT_slic.validuntil = tu::UnixSecondsToWindowsTicks(this->m_validuntil);
	OUT_slic.validafter = tu::UnixSecondsToWindowsTicks(this->m_validafter);
	OUT_slic.onlineusers = this->m_onlineusers;
	OUT_slic.terminal_pro_users = this->m_terminal_pro_users;
	OUT_slic.symmetric_participants = this->m_symmetric_participants;
	OUT_slic.max_guests = this->m_max_guests;
	OUT_slic.conferences = this->m_conferences;
	OUT_slic.restrict = this->m_restrict;
	OUT_slic.id = this->m_id;
	OUT_slic.gateways = this->m_gateways;
	OUT_slic.trial_conf_minutes = this->m_trial_conf_minutes;
}

SECURE_FUNC
bool VS_License::Serialize(std::unique_ptr<uint8_t[]>& outBuff, size_t& OUT_len) const
{
	SignedHWLicense lic;
	ConvertToSigned(lic);
	return lic.Encode(outBuff, OUT_len);
}

SECURE_FUNC
bool VS_License::Deserialize(const void * data, size_t length)
{
	SignedHWLicense lic;
	if (!lic.Decode(data, length))
		return false;
	ConvertFromSigned(lic);
	return true;
}

SECURE_FUNC
bool VS_License::IsValid() const
{
	return m_error == 0;
}

SECURE_FUNC
void VS_License::MergeLicense(const VS_License& x)
{
	if (!x.IsValid())
		return;

	if (!IsValid())
	{
		*this = x;
		return;
	}

	m_restrict = m_restrict | x.m_restrict;

	m_conferences            = NumAdd(m_conferences,            x.m_conferences);
	m_onlineusers            = NumAdd(m_onlineusers,            x.m_onlineusers);
	m_terminal_pro_users     = NumAdd(m_terminal_pro_users,     x.m_terminal_pro_users);
	m_symmetric_participants = NumMax(m_symmetric_participants, x.m_symmetric_participants);
	m_max_guests             = NumAdd(m_max_guests,             x.m_max_guests);
	m_gateways               = NumAdd(m_gateways,               x.m_gateways);

	if (m_trial_conf_minutes == 0 || x.m_trial_conf_minutes == 0)
		m_trial_conf_minutes = 0;
	else
		m_trial_conf_minutes = m_trial_conf_minutes > x.m_trial_conf_minutes ? m_trial_conf_minutes : x.m_trial_conf_minutes;

	if (m_validafter == EMPTY_TIME || (x.m_validafter != EMPTY_TIME && m_validafter < x.m_validafter))
		m_validafter = x.m_validafter;
}

#include "ProtectionLib/OptimizeDisable.h"
void VS_License::SaveToRegistry(VS_RegistryKey& k) const{
NANOBEGIN2;

	k.SetValue(&m_conferences, sizeof(int), VS_REG_INTEGER_VT, LIC_CONFERENCES_TAG);
	dprint4("\tc = %d\n", this->m_conferences);
	k.SetValue(&m_onlineusers, sizeof(int), VS_REG_INTEGER_VT, LIC_ONLINE_USERS_TAG);
	dprint4("\tou = %d\n", this->m_onlineusers);
	decltype(this->m_terminal_pro_users) val = 0;
	if (VS_CheckLicense(LE_PAID_SERVER))
		val = this->m_terminal_pro_users;
	k.SetValue(&val, sizeof(int), VS_REG_INTEGER_VT, LIC_TERMINAL_PRO_USERS_TAG);
	dprint4("\ttu = %d\n", this->m_terminal_pro_users);
	k.SetValue(&m_symmetric_participants, sizeof(int), VS_REG_INTEGER_VT, LIC_SYMMETRIC_PARTICIPANTS_TAG);
	dprint4("\tsp = %d\n", this->m_symmetric_participants);
	k.SetValue(&m_max_guests, sizeof(int), VS_REG_INTEGER_VT, LIC_MAX_GUESTS_TAG);
	dprint4("\tmg = %d\n", this->m_max_guests);
	k.SetValue(&m_gateways, sizeof(int), VS_REG_INTEGER_VT, LIC_GATEWAYS_TAG);
	dprint4("\tgw = %d\n", this->m_gateways);
	k.SetValue(&m_restrict, sizeof(int), VS_REG_INTEGER_VT, LIC_RESTRICTION_TAG);
	dprint4("\tr = %" PRId64 "\n", this->m_restrict);
	k.SetValue(&m_trial_conf_minutes, sizeof(unsigned), VS_REG_INTEGER_VT, LIC_TRIAL_TIME_TAG);
	dprint4("\ttm = %d\n", this->m_trial_conf_minutes);
	char reload_date[128] = { 0 };
	if (this->m_validuntil != EMPTY_TIME && tu::TimeToNStr(this->m_validuntil, reload_date, 128) > 0)
	{
		k.SetString(reload_date, LIC_RELOAD_DATE_TAG);
		dprint4("\trd = %s\n", reload_date);
	}
	else if (this->m_validuntil == EMPTY_TIME)
	{
		k.SetString("never", LIC_RELOAD_DATE_TAG);
		dprint4("\trd = %s\n", "never");
	}

NANOEND2;
}
void VS_License::ReadFromRegistry(VS_RegistryKey & k)
{
NANOBEGIN2;
	k.GetValue(&m_conferences, sizeof(m_conferences), VS_REG_INTEGER_VT, LIC_CONFERENCES_TAG);
	k.GetValue(&m_onlineusers, sizeof(m_onlineusers), VS_REG_INTEGER_VT, LIC_ONLINE_USERS_TAG);
	k.GetValue(&m_terminal_pro_users, sizeof(m_terminal_pro_users), VS_REG_INTEGER_VT, LIC_TERMINAL_PRO_USERS_TAG);
	k.GetValue(&m_symmetric_participants, sizeof(m_symmetric_participants), VS_REG_INTEGER_VT, LIC_SYMMETRIC_PARTICIPANTS_TAG);
	k.GetValue(&m_max_guests, sizeof(m_max_guests), VS_REG_INTEGER_VT, LIC_MAX_GUESTS_TAG);
	k.GetValue(&m_gateways, sizeof(m_gateways), VS_REG_INTEGER_VT, LIC_GATEWAYS_TAG);
	k.GetValue(&m_restrict, sizeof(m_restrict), VS_REG_INTEGER_VT, LIC_RESTRICTION_TAG);
	k.GetValue(&m_trial_conf_minutes, sizeof(m_trial_conf_minutes), VS_REG_INTEGER_VT, LIC_TRIAL_TIME_TAG);

	std::string time_str;
	if (k.GetString(time_str, LIC_RELOAD_DATE_TAG)) {
		auto t = tu::NStrToTimeT(time_str.c_str());
		if (t != static_cast<time_t>(-1))
			m_validuntil = std::chrono::system_clock::from_time_t(t);
	}
NANOEND2;
}
#include "ProtectionLib/OptimizeEnable.h"

void VS_License::Print()
{
	dprint4("\tc = %d\n", this->m_conferences);
	dprint4("\tou = %d\n", this->m_onlineusers);
	dprint4("\ttu = %d\n", this->m_terminal_pro_users);
	dprint4("\tsp = %d\n", this->m_symmetric_participants);
	dprint4("\tmg = %d\n", this->m_max_guests);
	dprint4("\tgw = %d\n", this->m_gateways);
	dprint4("\tr = %" PRId64 "\n", this->m_restrict);
	dprint4("\ttm = %u\n", this->m_trial_conf_minutes);
	dprint4("\terror = %d\n", this->m_error);
}

SECURE_FUNC
VS_License VS_License::DeductLicenseCopy(const VS_License & l) const
{
	VS_License copy = *this;
	return copy.DeductLicense(l);
}

SECURE_FUNC
VS_License & VS_License::DeductLicense(const VS_License & l)
{
	m_onlineusers = NumSub(m_onlineusers, l.m_onlineusers);
	m_gateways = NumSub(m_gateways, l.m_gateways);
	m_max_guests = NumSub(m_max_guests, l.m_max_guests);
	m_terminal_pro_users = NumSub(m_terminal_pro_users, l.m_terminal_pro_users);
	return *this;
}

SECURE_FUNC
VS_License & VS_License::AddLicence(const VS_License & lic)
{
	m_onlineusers = NumAdd(m_onlineusers, lic.m_onlineusers);
	m_terminal_pro_users = NumAdd(m_terminal_pro_users, lic.m_terminal_pro_users);
	m_max_guests = NumAdd(m_max_guests, lic.m_max_guests);
	m_gateways = NumAdd(m_gateways, lic.m_gateways);
	return *this;
}

SECURE_FUNC
VS_License VS_License::AddLicenceCopy(const VS_License & lic) const
{
	VS_License copy = *this;
	return copy.AddLicence(lic);
}

SECURE_FUNC
bool VS_License::CompareCountableResources(const VS_License & other) const
{
	return m_onlineusers == other.m_onlineusers &&
		m_terminal_pro_users == other.m_terminal_pro_users &&
		m_max_guests == other.m_max_guests &&
		m_gateways == other.m_gateways;
}

SECURE_FUNC
const VS_License VS_License::ShareAvailable(const VS_License & to_share) const
{
	VS_License res;
	res.m_onlineusers = NumMin(m_onlineusers, to_share.m_onlineusers);
	res.m_terminal_pro_users = NumMin(m_terminal_pro_users, to_share.m_terminal_pro_users);
	res.m_max_guests = NumMin(m_max_guests, to_share.m_max_guests);
	res.m_gateways = NumMin(m_gateways, to_share.m_gateways);
	res.m_error = m_error;
	return res;
}

SECURE_FUNC
bool VS_License::HasSharedResources() const
{
	return m_onlineusers != 0 || m_terminal_pro_users != 0 || m_max_guests != 0 || m_gateways != 0;
}

SECURE_FUNC
bool VS_License::SignedHWLicense::Decode(const void * data, int length)
{
	const int diff = sizeof(type);
	if (!data || length <= diff) return false;

	memcpy(&type, data, diff);	// copy without decoding
	int s = RCDV_DecodeSafe((uint8_t*)data + diff, ((uint8_t*)this) + diff, sizeof(VS_License::SignedHWLicense) - diff, length - diff);

	if (s != length - diff) return false;
	return true;
}

SECURE_FUNC
bool VS_License::SignedHWLicense::Encode(std::unique_ptr<uint8_t[]>& outBuff, size_t & OUT_len)
{
	const int bsize = sizeof(VS_License::SignedHWLicense);
	outBuff.reset(new uint8_t[bsize]);

	auto pRaw = outBuff.get();
	memcpy(outBuff.get(), &type, sizeof(type));	// copy without encoding
	OUT_len = RCDV_Encode((uint8_t*)this + sizeof(type), bsize - sizeof(type), outBuff.get() + sizeof(type));
	if (OUT_len <= 0) return false;
	OUT_len += sizeof(type);

	return true;
}

SECURE_FUNC
bool VS_License::SignedHWLicense::MakeSign(const char* privateKey) {
	if (!privateKey) return false;

	VS_Sign					signer;
	VS_SignArg				signarg = { alg_pk_RSA,alg_hsh_SHA1 };

	if (!signer.Init(signarg))	return false;
	if (!signer.SetPrivateKey(privateKey, store_PEM_BUF)) return false;

	uint32_t				sign_size = VS_SIGN_SIZE;
	if (!signer.SignData((const unsigned char*)this, sizeof(VS_License::SignedHWLicense) - sizeof(this->sign), this->sign, &sign_size))
		return false;

	return true;
}

SECURE_FUNC
bool VS_License::SignedHWLicense::VerifySign(const char *publicKey) {
	if (!publicKey) return false;

	VS_Sign					verifier;
	VS_SignArg				signarg = { alg_pk_RSA,alg_hsh_SHA1 };

	if ((!verifier.Init(signarg)) ||
		(!verifier.SetPublicKey(publicKey, 0, store_PEM_BUF))) return false;

	if (1 != verifier.VerifySign((const unsigned char *)this, sizeof(SignedHWLicense) - sizeof(this->sign), this->sign, VS_SIGN_SIZE)) return false;
	return true;
}

SECURE_FUNC
bool VS_License::SignedHWLicense::VerifySign()
{
	// Check that all insignificant bytes contain predetermined values.
	// This severely reduces attack surface for finding SHA-1 hash collisions for the license data.

	size_t i = 0;
	for ( ; i < sizeof(serverName); ++i)
		if (serverName[i] == '\0')
			break;
	// Now 'i' contains the index of the first 0 byte in serverName.
	// Check that all remaining bytes are also 0.
	for ( ; i < sizeof(serverName); ++i)
		if (serverName[i] != '\0')
			break;
	if (i != sizeof(serverName))
	{
		dprint1("License(%016" PRIX64 "): bad padding at position %i", this->id, static_cast<unsigned>(i));
		return false;
	}

	protection::SHA1 sha1;
	sha1.Update(this, sizeof(SignedHWLicense) - sizeof(this->sign));
	sha1.Final();
	unsigned char digest[20];
	sha1.GetBytes(digest);

	unsigned char pubkey_n[sizeof(c_regsrv_pubkey_n) - 1/*ignore the the null-terminator, which is not a part of the value*/];
	protection::memcpy(pubkey_n, SECURE_STRING(c_regsrv_pubkey_n), sizeof(pubkey_n));
	if (pubkey_n[77] != 0xff)
		return false; // Something isn't right with the modulus data.
	pubkey_n[77] = 0;
	char n_storage[32 + sizeof(pubkey_n)];
	const auto n = protection::BN_bin2bn_static(pubkey_n, sizeof(pubkey_n), n_storage);
	char e_storage[32];
	const auto e = protection::BN_word2bn_static(c_regsrv_pubkey_e, e_storage);

	const int rsa_result = protection::RSA_verify_SHA1_PKCS1_type_1(digest, sizeof(digest), this->sign, sizeof(this->sign), n, e);
	if (rsa_result != 1)
	{
		dprint1("License(%016" PRIX64 "): verification failed: %i", this->id, rsa_result);
		return false;
	}

	return true;
}

SECURE_FUNC
bool VS_License::SignedHWLicense::SetHWKey()
{
	char buf[33] = {};
	if (VS_RegistryKey(false, CONFIGURATION_KEY).GetValue(buf, sizeof(buf), VS_REG_STRING_VT, "Key") <= 0)
		return false;
	char hw_key[protection::HWKeyLength + 1];
	const auto hw_key_result = protection::ReadHWKey(hw_key);
	if (hw_key_result != 0)
	{
		dstream1 << "HWKey error (set): " << hw_key_result;
		return false;
	}
	protection::GetSaltedHWKey(hw_key, buf, md5_hwkey);
	return true;
}

SECURE_FUNC
int VS_License::SignedHWLicense::VerifyHWKey()
{
	char buf[33] = {};
	if (VS_RegistryKey(false, CONFIGURATION_KEY).GetValue(buf, sizeof(buf), VS_REG_STRING_VT, "Key") <= 0)
		return VSS_LICENSE_KEY_EMTY;
	char hw_key[protection::HWKeyLength + 1];
	const auto hw_key_result = protection::ReadHWKey(hw_key);
	if (hw_key_result != 0)
	{
		dstream1 << "HWKey error: " << hw_key_result;
		return VSS_LICENSE_NO_HWKEY;
	}
	if (!protection::CheckSaltedHWKey(hw_key, buf, md5_hwkey))
		return VSS_LICENSE_KEY_FAILED;
	return 0;
}

SECURE_FUNC
int VS_License::NumAdd(int l, int r)
{
	assert(l >= 0 || l == TC_INFINITY);
	assert(r >= 0 || r == TC_INFINITY);
	if (l == TC_INFINITY || r == TC_INFINITY)
		return TC_INFINITY;

	// Addition of 2 positive ints can't overflow in unsigned int.
	static_assert(static_cast<unsigned>(INT_MAX) * 2 <= UINT_MAX, "");
	const unsigned sum = static_cast<unsigned>(l) + static_cast<unsigned>(r);
	return sum > INT_MAX ? INT_MAX : sum;
}

SECURE_FUNC
int VS_License::NumSub(int l, int r)
{
	assert(l >= 0 || l == TC_INFINITY);
	assert(r >= 0 || r == TC_INFINITY);
	// Order of the following checks ensures that TC_INFINITY-TC_INFINITY == TC_INFINITY.
	// We prefer that result over 0, because TC_INFINITY-x == TC_INFINITY and
	// in practice sharing a large amount of license resources is equivalent to
	// sharing infinite amount of resources.
	if (l == TC_INFINITY)
		return TC_INFINITY;
	if (r == TC_INFINITY)
		return 0;

	// Subtraction of two positive ints can't underflow.
	static_assert(0 - INT_MAX >= INT_MIN, "");
	const int result = l - r;
	return result < 0 ? 0 : result;
}

SECURE_FUNC
int VS_License::NumMin(int a, int b)
{
	assert(a >= 0 || a == TC_INFINITY);
	assert(b >= 0 || b == TC_INFINITY);
	if (a == TC_INFINITY)
		return b;
	if (b == TC_INFINITY)
		return a;

	return a < b ? a : b;
}

SECURE_FUNC
int VS_License::NumMax(int a, int b)
{
	assert(a >= 0 || a == TC_INFINITY);
	assert(b >= 0 || b == TC_INFINITY);
	if (a == TC_INFINITY)
		return TC_INFINITY;
	if (b == TC_INFINITY)
		return TC_INFINITY;

	return a < b ? b : a;
}
