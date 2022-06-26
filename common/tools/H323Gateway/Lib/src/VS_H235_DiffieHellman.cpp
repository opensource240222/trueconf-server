#include "VS_H235_DiffieHellman.h"
#include "SecureLib/OpenSSLCompat/tc_dh.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/debuglog/VS_Debug.h"
#include "tools/H323Gateway/Lib/VS_H323Lib.h"

#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/dh.h>

#include <cassert>

#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER

void DH_deleter::operator()(void *p) const{ ::DH_free(static_cast<DH*>(p));}
void BN_deleter::operator()(void* p) const { ::BN_free(static_cast<BIGNUM*>(p));}

struct VS_DiffieHellman::dh_ptr_tag : vs::BoxTag<std::unique_ptr<DH, DH_deleter>>{};
struct VS_DiffieHellman::bignum_raw_ptr_tag : vs::BoxTag<const BIGNUM*> {};

VS_DiffieHellman::~VS_DiffieHellman() = default;
VS_DiffieHellman::VS_DiffieHellman(const char * pData,const char * gData)
	: m_dh(DH_new()), m_remKey(nullptr)
{
	VS_SCOPE_EXIT{ ValidateClassState(); };

	if (m_dh.get() == nullptr) {
		dstream4 << "VS_DiffieHellman\tFailed to allocate DH\n";
		return;
	}

	BIGNUM* p = BN_new();
	BIGNUM* g = BN_new();
	BN_dec2bn(&p, pData);
	BN_dec2bn(&g, gData);

	DH_set0_pqg(&*m_dh.get(), p, NULL, g);

	if (!DH_get0_p(&*m_dh.get()) || !DH_get0_g(&*m_dh.get())) {
		dstream4 << "VS_DiffieHellman\tWrong crypto params\n";
		m_dh = nullptr;
	}
}

VS_DiffieHellman::VS_DiffieHellman(std::size_t key_size, std::size_t generator) : m_dh(DH_new()), m_remKey(nullptr){
	VS_SCOPE_EXIT{ ValidateClassState(); };

	if (m_dh.get() == nullptr) {
		dstream4 << "VS_DiffieHellman\tFailed to allocate DH\n";
		return;
	}

	if (1 != DH_generate_parameters_ex(m_dh.get().get(), key_size, generator, nullptr)){
		dstream4 << "VS_DiffieHellman\tERROR generating DH parameters ";
		TraceError();
		return;
	}

	int codes(0);
	if (1 != DH_check(m_dh.get().get(), &codes)){
		TraceError();
	}
	if (codes != 0)	{
		/* Problems have been found with the generated parameters */
		dstream4 << "VS_DiffieHellman DH_check failed\n";
		return;
	}
}

void VS_DiffieHellman::ValidateClassState(){
	if (m_dh.get() == nullptr){
		dstream4 << "ERROR\tVS_DiffieHellman class works incorrectly\n";
	}
	assert(m_dh.get() != nullptr);
}

static DH * DH_dup(const DH * dh)
{
	if (dh == nullptr)
		return nullptr;

	DH * ret = DH_new();
	if (ret == nullptr)
		return nullptr;

	BIGNUM *retp, *retq, *retg, *retpub, *retpriv;
	retp = retq = retg = retpub = retpriv = NULL;
	if (DH_get0_p(dh))
		retp = BN_dup(DH_get0_p(dh));
	if (DH_get0_q(dh))
		retq = BN_dup(DH_get0_q(dh));
	if (DH_get0_g(dh))
		retg = BN_dup(DH_get0_g(dh));
	if (DH_get0_pub_key(dh))
		retpub = BN_dup(DH_get0_pub_key(dh));
	if (DH_get0_priv_key(dh))
		retpriv = BN_dup(DH_get0_priv_key(dh));
	DH_set0_pqg(ret, retp, retq, retg);
	DH_set0_key(ret, retpub, retpriv);
	return ret;
}

VS_DiffieHellman::VS_DiffieHellman(const VS_DiffieHellman & diffie)
	: m_remKey(nullptr)
{
	VS_SCOPE_EXIT{ ValidateClassState(); };
	m_dh.get().reset(DH_dup(diffie.m_dh.get().get()));
}


VS_DiffieHellman & VS_DiffieHellman::operator=(const VS_DiffieHellman & other)
{
	VS_SCOPE_EXIT{ ValidateClassState(); };

	if (this != &other) {
		m_dh.get().reset(DH_dup(other.m_dh.get().get()));
		m_remKey = nullptr;
	}
	return *this;
}

bool VS_DiffieHellman::GenerateHalfKey()
{
	VS_SCOPE_EXIT{ ValidateClassState(); };

	if (!DH_generate_key(m_dh.get().get())) {	// halfkey = (g^x)%p where g and p set by us, x - secret chosen by openssl
		dstream4 << "VS_DiffieHellman\tERROR generating DH halfkey ";
		TraceError();
		return false;
	}

	return true;
}

void VS_DiffieHellman::TraceError(){
	char buf[256];
	ERR_error_string(ERR_get_error(), buf);
	dstream4 << buf;
}
template<>
void VS_DiffieHellman::SetRemoteKey(const BIGNUM *remKey)
{
	m_remKey = remKey;
}

template<>
const BIGNUM * VS_DiffieHellman::GetPublicKey() const
{
	return DH_get0_pub_key(&*m_dh.get());
}

bool VS_DiffieHellman::TestParamsEqual(VS_DiffieHellman& other) const{
	return BN_cmp(DH_get0_p(&*m_dh.get()), DH_get0_p(&*other.m_dh.get())) == 0 &&
           BN_cmp(DH_get0_g(&*m_dh.get()), DH_get0_g(&*other.m_dh.get())) == 0;
}

bool VS_DiffieHellman::ComputeMasterKey(std::vector<uint8_t> &MasterKey)
{
	VS_SCOPE_EXIT{ ValidateClassState(); };

	if (!m_remKey.get()) {
		dstream4 << "VS_DiffieHellman\tERROR Generating Shared DH: No remote key!\n";
		return false;
	}

	if (!DH_get0_priv_key(&*m_dh.get()))
		GenerateHalfKey();

	int len = DH_size(m_dh.get().get());
	unsigned char * buf = static_cast<unsigned char *>(OPENSSL_malloc(len));
	VS_SCOPE_EXIT{ OPENSSL_free(buf); };

	int out = DH_compute_key(buf, m_remKey, m_dh.get().get());	// master_key = (m_remKey^x)%p where m_remKey - halfkey from network and x - secret chosen by openssl
	if (out <= 0) {
		dstream4 << "VS_DiffieHellman\tERROR Generating Shared DH!\n";
		return false;
	}

	MasterKey.assign(buf, buf + out);
	return true;
}

bool VS_DiffieHellman::IsZeroDHGroup() const{
	return BN_is_zero(DH_get0_g(&*m_dh.get())) || BN_is_zero(DH_get0_p(&*m_dh.get()));
}

VS_H235_DiffieHellman::VS_H235_DiffieHellman() :VS_DiffieHellman(512, DH_GENERATOR_2)
{}
void VS_H235_DiffieHellman::Encode_HalfKey(VS_BitBuffer& hk) const
{
	int len_p = BN_num_bytes(DH_get0_p(&*m_dh.get()));
	int len_key = BN_num_bytes(DH_get0_pub_key(&*m_dh.get()));
	int bits_p = BN_num_bits(DH_get0_p(&*m_dh.get()));

	// halfkey is padded out to the length of P
	unsigned char * data = static_cast<unsigned char *>(OPENSSL_malloc(len_p));
	VS_SCOPE_EXIT{ OPENSSL_free(data); };

	if (data != nullptr) {
		memset(data, 0, len_p);

		if (BN_bn2bin(DH_get0_pub_key(&*m_dh.get()), data + len_p - len_key) > 0)
			hk.AddBits(data, bits_p);
		else
			dstream4 << "VS_H235_DiffieHellman\tFailed to encode halfkey\n";
	}
}

void VS_H235_DiffieHellman::Decode_HalfKey(const VS_BitBuffer & hk)
{
	VS_SCOPE_EXIT{ ValidateClassState(); };

	const uint8_t *data = static_cast<const uint8_t *>(hk.GetData());
	DH_set0_key(&*m_dh.get(), BN_bin2bn(data, hk.ByteSize(), nullptr), NULL);
}

bool VS_H235_DiffieHellman::Encode_P(VS_BitBuffer & p) const
{
	uint8_t * data = static_cast<unsigned char *>(OPENSSL_malloc(BN_num_bytes(DH_get0_p(&*m_dh.get()))));
	VS_SCOPE_EXIT{ OPENSSL_free(data); };

	memset(data, 0, BN_num_bytes(DH_get0_p(&*m_dh.get())));
	if (BN_bn2bin(DH_get0_p(&*m_dh.get()), data) > 0)
		p.AddBits(data, BN_num_bits(DH_get0_p(&*m_dh.get())));
	else
		dstream4 << "VS_H235_DiffieHellman\tFailed to encode P\n";
	return true;
}

void VS_H235_DiffieHellman::Decode_P(const VS_BitBuffer & p)
{
	VS_SCOPE_EXIT{ ValidateClassState(); };

	if (p.BitSize() == 0)
		return;

	DH_set0_pqg(&*m_dh.get(), BN_bin2bn(static_cast<const uint8_t *>(p.GetData()), p.ByteSize(), nullptr),
		NULL, NULL);
}

bool VS_H235_DiffieHellman::Encode_G(VS_BitBuffer & g) const
{
	int len_p = BN_num_bytes(DH_get0_p(&*m_dh.get()));
	int len_g = BN_num_bytes(DH_get0_g(&*m_dh.get()));
	int bits_p = BN_num_bits(DH_get0_p(&*m_dh.get()));

	// G is padded out to the length of P
	uint8_t * data = static_cast<unsigned char *>(OPENSSL_malloc(len_p));
	VS_SCOPE_EXIT{ OPENSSL_free(data); };

	memset(data, 0, len_p);
	if (BN_bn2bin(DH_get0_g(&*m_dh.get()), data + len_p - len_g) > 0)
		g.AddBits(data, bits_p);
	else
		dstream4 << "VS_H235_DiffieHellman\tFailed to encode G\n";
	return true;
}

void VS_H235_DiffieHellman::Decode_G(const VS_BitBuffer & g)
{
	VS_SCOPE_EXIT{ ValidateClassState(); };

	if (g.BitSize() == 0)
		return;

	DH_set0_pqg(&*m_dh.get(), NULL, NULL,
		BN_bin2bn(static_cast<const uint8_t *>(g.GetData()), g.ByteSize(), nullptr));
}

const VS_GwAsnObjectId h235DHdummyID{ 0, 0, 8, 235, 0, 3, 40 };	// "DH Dummy" явно предоставл€ема€ нестандартна€ DH - группа (512 бит)
const VS_GwAsnObjectId h235DH1024ID{ 0, 0, 8, 235, 0, 3, 43 };	// 1024-битова€ DH-группа
const VS_GwAsnObjectId h235DH1536ID{ 0, 0, 8, 235, 0, 3, 44 };	// 1536-битова€ DH-группа

VS_H235_DiffieHellman VS_H235_DiffieHellman::dummy;

#define DH1024ModSize "179769313486231590770839156793787453197860296048756011706444"\
		"423684197180216158519368947833795864925541502180565485980503"\
		"646440548199239100050792877003355816639229553136239076508735"\
		"759914822574862575007425302077447712589550957937778424442426"\
		"617334727629299387668709205606050270810842907692932019128194"\
		"467627007"
#define DH1536ModSize "241031242692103258855207602219756607485695054850245994265411"\
	"694195810883168261222889009385826134161467322714147790401219"\
	"650364895705058263194273070680500922306273474534107340669624"\
	"601458936165977404102716924945320037872943417032584377865919"\
	"814376319377685986952408894019557734611984354530154704374720"\
	"774996976375008430892633929555996888245787241299381012913029"\
	"459299994792636526405928464720973038494721168143446471443848"\
	"8520940127459844288859336526896320919633919"

std::map<dh_oid, dh_params> VS_H235_DiffieHellman::CreateDiffieHellmanParams(){
	vs::map<dh_oid, dh_params> res;
	//res.emplace(dh_oid::DH1536, std::make_tuple(h235DH1536ID, VS_H235_DiffieHellman(DH1536ModSize, "2"))); no algorithms for 1536 dh_group
	res.emplace(dh_oid::DH1024, std::make_tuple(h235DH1024ID, VS_H235_DiffieHellman(DH1024ModSize, "2")));
	//res.emplace(dh_oid::DHdummy, std::make_tuple(h235DHdummyID, VS_H235_DiffieHellman::dummy)); do no use weak algorithms
	return res;
}