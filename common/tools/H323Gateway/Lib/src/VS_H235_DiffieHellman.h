#pragma once

#include "tools/H323Gateway/Lib/src/VS_H235Enumerations.h"

#include "std-generic/cpplib/Box.h"

#include <memory>
#include <map>
#include <string>
#include <vector>

struct DH_deleter
{
	void operator()(void* p) const;
};
struct BN_deleter
{
	void operator()(void* p) const;
};

class VS_DiffieHellman
{
public:
	/**Create a set of Diffie-Hellman parameters.
	*/
	VS_DiffieHellman(
		const char * pData,					/// P data - string representation of digital in decimal
		const char * gData);				/// G data - string representation of digital in decimal

	/**Generate a set of Diffie-Hellman parameters from given key size.
	*/
	VS_DiffieHellman(std::size_t key_size, std::size_t generator /*= DH_GENERATOR_2 */);
	VS_DiffieHellman(const VS_DiffieHellman&);
	~VS_DiffieHellman();
	VS_DiffieHellman& operator=(const VS_DiffieHellman&);

	bool GenerateHalfKey();
	template<typename BIGNUMPTR /*= const BIGNUM* */>
	void SetRemoteKey(BIGNUMPTR remKey);
	bool ComputeMasterKey(std::vector<uint8_t> &MasterKey);
	template<typename BIGNUMPTR /*=const BIGNUM* */>
	BIGNUMPTR GetPublicKey() const;
	bool TestParamsEqual(VS_DiffieHellman& other) const;
	bool IsZeroDHGroup() const;
protected:
	static void TraceError();
	void ValidateClassState();					// to verify after execution of non-const functions

	struct dh_ptr_tag;
	vs::Box<dh_ptr_tag, sizeof(void*)> m_dh; /// Local DiffieHellman
	struct bignum_raw_ptr_tag;
	vs::Box<bignum_raw_ptr_tag, sizeof(void*)> m_remKey;/// Remote Public Key
};

struct VS_BitBuffer;
struct VS_GwAsnObjectId;
class VS_H235_DiffieHellman;
typedef std::tuple<VS_GwAsnObjectId, VS_H235_DiffieHellman> dh_params;

class VS_H235_DiffieHellman : public VS_DiffieHellman{
public:
	VS_H235_DiffieHellman(
		const char * pData,					/// P data
		const char * gData)					/// G data
		:VS_DiffieHellman(pData,gData){}
	VS_H235_DiffieHellman();// by default generate params for DHdummy

	void Encode_HalfKey(VS_BitBuffer & hk) const;
	void Decode_HalfKey(const VS_BitBuffer & hk);

	bool Encode_G(VS_BitBuffer & g) const;
	void Decode_G(const VS_BitBuffer & g);

	bool Encode_P(VS_BitBuffer & p) const;
	void Decode_P(const VS_BitBuffer & p);

	static VS_H235_DiffieHellman dummy;	// due to high expensivnes of params generation make it static
	static std::map<dh_oid, dh_params> CreateDiffieHellmanParams();
};