#include "VS_License.h"
#include "std-generic/clib/rangecd.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/std/cpplib/VS_RegistryConst.h"
#include "SecureLib/VS_Sign.h"

#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"


bool VS_EncodeLicense(const VS_License& lic,VS_License::Type type,uint8_t** buf,int* size, const std::vector<char> &p_key)
{
	VS_Sign					signer;
	VS_SignArg				signarg = {alg_pk_RSA,alg_hsh_SHA1};
	uint32_t				sign_size = VS_SIGN_SIZE;
	if (!lic.IsValid())
	{
		//dprint1("cannot encode: license no valid");
		return false;
	}

  switch (type)
  {
  case VS_License::SIGNED_WITH_HW:
	  {
		  if(!signer.Init(signarg))
			  return false;

		  std::unique_ptr<char, free_deleter> PrivKeyBuf;
		  const char* PrivKey = nullptr;
		  if(p_key.empty())
		  {
			VS_RegistryKey key(false, CONFIGURATION_KEY);
			if(!key.GetValue(PrivKeyBuf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY))
				return false;
			PrivKey = PrivKeyBuf.get();
		  }
		  else
			  PrivKey = p_key.data();
		  if (!signer.SetPrivateKey(PrivKey, store_PEM_BUF))
			  return false;
		  VS_License::SignedHWLicense signed_l;
		  memset(&signed_l,0,sizeof(signed_l));
		  signed_l.type = type;
		  if(lic.m_serverName && lic.m_serverName.Length() <= 256)
			  strncpy(signed_l.serverName,lic.m_serverName,lic.m_serverName.Length());
		  else
			  signed_l.serverName[0] = 0;
		  signed_l.validuntil				= tu::UnixSecondsToWindowsTicks(lic.m_validuntil);
		  signed_l.validafter				= tu::UnixSecondsToWindowsTicks(lic.m_validafter);
		  signed_l.onlineusers				= lic.m_onlineusers;
		  signed_l.terminal_pro_users		= lic.m_terminal_pro_users;
		  signed_l.symmetric_participants	= lic.m_symmetric_participants;
		  signed_l.max_guests				= lic.m_max_guests;
		  signed_l.conferences				= lic.m_conferences;
		  signed_l.restrict					= lic.m_restrict;
		  signed_l.id						= lic.m_id;
		  signed_l.gateways					= lic.m_gateways;
		  signed_l.trial_conf_minutes		= lic.m_trial_conf_minutes;

		  memset(signed_l.md5_hwkey,0,sizeof(signed_l.md5_hwkey));

		  if(lic.m_hw_md5.m_str != nullptr)
			  strcpy(signed_l.md5_hwkey, lic.m_hw_md5.m_str);


		  const int bsize=sizeof(signed_l)-sizeof(signed_l.type);
		  if(!signer.SignData((const unsigned char*)&signed_l,sizeof(signed_l) - sizeof(signed_l.sign),signed_l.sign,&sign_size))
			  return false;

		  *buf=new uint8_t[sizeof(signed_l)];
		  *size=sizeof(signed_l.type);
		  *reinterpret_cast<uint32_t*>(*buf)=signed_l.type;
		  int dsize=RCDV_Encode((uint8_t*)&signed_l+sizeof(signed_l.type),bsize,*buf+sizeof(signed_l.type));
		  *size+=dsize;
	  }
	  break;
  default:
    return false;
  }
    return true;
}
