/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 20.09.03     by  A.Slavetsky
//  Modified:     A.Vlaskin, A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_Containers.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ostream>
#include "VS_BaseBuffers.h"

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_PerBuffer;

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_Asn
{
	enum ConstraintType
	{
		Unconstrained,
		PartiallyConstrained,
		FixedConstraint,
		ExtendableConstraint
	};
	// end ConstraintType enum

	/////////////////////////////////////////////////////////////////////////////////////

	explicit VS_Asn( bool extendable ) : filled(false), extendable(extendable) {}
	// end VS_Asn::VS_Asn

	virtual ~VS_Asn( void ) {}
	// end VS_Asn::~VS_Asn

	bool   filled, extendable;		// PER extension capability

	/////////////////////////////////////////////////////////////////////////////////////

	virtual bool Decode(VS_PerBuffer& buffer) = 0;
	// end VS_Asn::Decode

	virtual bool Encode(VS_PerBuffer& buffer) = 0;
	// end VS_Asn::Encode

	VS_Asn(const VS_Asn& src) :filled(src.filled), extendable(src.extendable)
	{}

	VS_Asn& operator=(const VS_Asn& src)
	{
		if (this == &src)
		{
			return *this;
		}
		if (*this != src)
		{
			this->filled = src.filled;
			this->extendable = src.extendable;
		}
		return *this;
	}

	bool operator!=(const VS_Asn& other) const
	{
		return (this->filled != other.filled) || (this->extendable != other.extendable);
	}

	bool operator==(const VS_Asn& other) const
	{
		return !this->operator !=(other);
	}

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_Asn struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_ConstrainedAsn : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_ConstrainedAsn( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable )
		: VS_Asn( extendable )
		, constraint(constraint), lowerLimit(lowerLimit), upperLimit(upperLimit) {}
	// end VS_ConstrainedAsn::VS_ConstrainedAsn

	virtual ~VS_ConstrainedAsn( void ) {}
	// end VS_ConstrainedAsn::~VS_ConstrainedAsn

	ConstraintType   constraint;
	std::int32_t   lowerLimit;
	std::uint32_t   upperLimit;

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_ConstrainedAsn struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_StringAsn : public VS_ConstrainedAsn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_StringAsn( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
					bool extendable )
		: VS_ConstrainedAsn( constraint, lowerLimit, upperLimit, extendable )
	{}
	// end VS_StringAsn::VS_StringAsn

	VS_StringAsn( void *value, std::size_t bits,
					ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
					bool extendable )
		: VS_ConstrainedAsn( constraint, lowerLimit, upperLimit, extendable )
		, value(static_cast<unsigned char *>(value), bits)
	{	filled = VS_StringAsn::value.IsFilled();	}
	// end VS_StringAsn::VS_StringAsn

	virtual ~VS_StringAsn( void ) {}
	// end VS_StringAsn::~VS_StringAsn

	VS_BitBuffer   value;

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_StringAsn struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_ConstrainedArrayAsn : public VS_ConstrainedAsn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_ConstrainedArrayAsn( void )
		: VS_ConstrainedAsn( Unconstrained, 0, INT_MAX, false )
		, asns_(nullptr), length_(0) {}
	// end VS_ConstrainedArrayAsn::VS_ConstrainedArrayAsn

	VS_ConstrainedArrayAsn(std::int32_t lower, std::uint32_t upper
		, ConstraintType constr = VS_Asn::FixedConstraint
		, bool extend = false )
		: VS_ConstrainedAsn( constr, lower, upper, extend )
		, asns_(nullptr), length_(0) {}
	// end VS_ConstrainedArrayAsn::VS_ConstrainedArrayAsn

	/////////////////////////////////////////////////////////////////////////////////////

	VS_ConstrainedArrayAsn(const VS_ConstrainedArrayAsn&) = delete;
	VS_ConstrainedArrayAsn& operator=(const VS_ConstrainedArrayAsn&) = delete;

protected:
	template<class T>
	void DeleteArray(void)
	{
		delete[] static_cast<T*>(asns_);
		asns_ = nullptr;
		length_ = 0;
		filled = false;
	}
	bool DecodeLength(VS_PerBuffer &buffer);
	// ::DecodeLength

	bool EncodeLength(VS_PerBuffer &buffer);
	// ::EncodeLength
protected:
	void   *asns_;
	std::size_t   length_;
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_ConstrainedArrayAsn struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnNull : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////

	explicit VS_AsnNull( bool extendable = false )
		: VS_Asn( extendable ) { filled = true;}
	// end VS_AsnNull::VS_AsnNull
	/////////////////////////////////////////////////////////////////////////////////////

	bool Decode(VS_PerBuffer& /*buffer*/) override { return (filled=true); }
	// end VS_AsnNull::Decode

	bool Encode(VS_PerBuffer& /*buffer*/) override { return true; }
	// end VS_AsnNull::Encode

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnNull struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnBoolean : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnBoolean( bool value, bool extendable = false )
		: VS_Asn( extendable ), value(value)
	{	filled = true;	}
	// end VS_AsnBoolean::VS_AsnBoolean

	explicit VS_AsnBoolean( bool extendable = false )
		: VS_Asn( extendable ), value(false) {}
	// end VS_AsnBoolean::VS_AsnBoolean

	bool   value;

	/////////////////////////////////////////////////////////////////////////////////////

	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnBoolean::Decode

	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnBoolean::Decode

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnBoolean struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnInteger : public VS_ConstrainedAsn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnInteger( const std::uint32_t value,
						ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable = false )
		: VS_ConstrainedAsn( constraint, lowerLimit, upperLimit, extendable ), value(value)
	{	filled = true;	}
	// end VS_AsnInteger::VS_AsnInteger

	VS_AsnInteger( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable = false )
		: VS_ConstrainedAsn( constraint, lowerLimit, upperLimit, extendable )
		, value(0)
		{}
	// end VS_AsnInteger::VS_AsnInteger

	std::uint32_t   value;

	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnInteger::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnInteger::Encode
	/////////////////////////////////////////////////////////////////////////////////////
	inline bool IsUnsigned( void ) const
	{
		return constraint != Unconstrained && lowerLimit >= 0;
	}
	// end VS_AsnInteger::IsUnsigned

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnInteger struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnEnumeration : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////
/*
	VS_AsnEnumeration( const unsigned value, bool extendable = false )
		: VS_Asn( extendable ), value(value),maxEnumValue(0)
	{	filled = true;	}
	// end VS_AsnEnumeration::VS_AsnEnumeration
*/
	VS_AsnEnumeration(std::uint32_t max, std::uint32_t value=0, bool extendable = false )
		: VS_Asn( extendable ), value(value),maxEnumValue(max) {}
	// end VS_AsnEnumeration::VS_AsnEnumeration

	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnEnumeration::Decode

	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnEnumeration::Decode

    std::uint32_t   value;
	std::uint32_t   maxEnumValue;
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnEnumeration struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnReal : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnReal( const double value, bool extendable = false )
		: VS_Asn( extendable ), value(value)
	{	filled = true;	}
	// end VS_AsnReal::VS_AsnReal

	explicit VS_AsnReal( bool extendable = false )
		: VS_Asn( extendable ), value(0.) {}
	// end VS_AsnReal::VS_AsnReal

	double   value;

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnReal struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnObjectId : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnObjectId(const std::uint32_t *c_value, unsigned number = 0, bool extendable = false)
		: VS_Asn(extendable)
	{
		Clear();
		filled = false;
		if (c_value && number > 0)
		{
			const auto num = (number > max_values) ? max_values : number;
			memcpy(value, c_value, static_cast<std::size_t>(4 * num));
			size = num;
			filled = true;
		}
	}
	// end VS_AsnObjectId::VS_AsnObjectId

	explicit VS_AsnObjectId(bool extendable = false)
		: VS_Asn(extendable), size(0) { memset(value, 0, sizeof(value)); }
	// end VS_AsnObjectId::VS_AsnObjectId

	VS_AsnObjectId(const VS_AsnObjectId&src):VS_Asn(src.extendable)
	{
		this->filled = src.filled;
		memcpy(this->value, src.value, sizeof(value[0]) * max_values);
		this->size = src.size;
	}

	void Clear(void) { memset(value, 0, sizeof(value[0]) * max_values);		filled = false; }
	// end VS_AsnObjectId::Clear

	~VS_AsnObjectId(void) {}
	// end VS_AsnObjectId::~VS_AsnObjectId

	std::size_t   size;
	static const unsigned   max_values = 32;
	std::uint32_t   value[max_values];

	/////////////////////////////////////////////////////////////////////////////////////

	unsigned NValues(void) { return max_values; }
	//end VS_AsnObjectId::NValues

	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnObjectId::Decode

	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnObjectId::Encode

	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnObjectId & operator=(const VS_AsnObjectId &src)
	{
		if (this != &src)
		{
			O_CC(filled);	O_CC(size);
			memcpy(value, src.value, sizeof(value));
		}
		return *this;
	}
	// end of VS_AsnObjectId::operator=

	bool operator==( const VS_AsnObjectId &src ) const
	{
		if (!O_T(filled))	return false;
		if (!filled)	return true;
		return O_T(size) && !memcmp( value, src.value, size * sizeof(*value) );
	}
	// end of VS_AsnObjectId::operator==

	friend std::ostream& operator<<(std::ostream& out, const VS_AsnObjectId &id){
		if (!id.filled) return out;

		for (std::size_t i = 0; i < id.size - 1; ++i){
			out << id.value[i] << '.';
		}
		out << id.value[id.size - 1];

		return out;
	}

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnObjectId struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnBitString : public VS_StringAsn // +
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnBitString( void *value, std::size_t size,
						ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable = false )
		: VS_StringAsn( value, size, constraint, lowerLimit, upperLimit, extendable )
		 {}
	// end VS_AsnBitString::VS_AsnBitString

	VS_AsnBitString( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable = false )
		: VS_StringAsn( constraint, lowerLimit, upperLimit, extendable )
		 {}
	// end VS_AsnBitString::VS_AsnBitString
	/////////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnBitString::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnBitString::Encode

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnBitString struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnOctetString : public VS_StringAsn // +
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnOctetString( void *value, std::size_t size,
						ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable = false )
		: VS_StringAsn( value, size, constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnOctetString::VS_AsnOctetString

	VS_AsnOctetString( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable = false )
		: VS_StringAsn( constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnOctetString::VS_AsnOctetString
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnOctetString::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnOctetString::Encode
	bool EncodeSubType( VS_Asn * obj );
	// end EncodeSubType
	bool DecodeSubType( VS_Asn * obj );
	// end DecodeSubType

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnOctetString struct


/////////////////////////////////////////////////////////////////////////////////////////
/*
struct VS_AsnPrintableString : public VS_StringAsn // +
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnPrintableString( void *value, unsigned size,
							ConstraintType constraint, int lowerLimit, unsigned upperLimit,
							bool extendable = false )
		: VS_StringAsn( value, size, constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnPrintableString::VS_AsnPrintableString

	VS_AsnPrintableString( ConstraintType constraint, int lowerLimit, unsigned upperLimit,
							bool extendable = false )
		: VS_StringAsn( constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnPrintableString::VS_AsnPrintableString

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnPrintableString struct

/////////////////////////////////////////////////////////////////////////////////////////
*/
struct VS_AsnVisibleString : public VS_StringAsn // -
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnVisibleString( void *value, std::size_t size,
							ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
							bool extendable = false )
		: VS_StringAsn( value, size, constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnVisibleString::VS_AsnVisibleString

	VS_AsnVisibleString( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
							bool extendable = false )
		: VS_StringAsn( constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnVisibleString::VS_AsnVisibleString

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnVisibleString struct

/////////////////////////////////////////////////////////////////////////////////////////
#define max_table_size 256
#define max_ia5_size 128
#define max_general_size 256//4294967296
#define max_alph_size 256//4294967296
struct VS_AsnRestrictedString : public VS_StringAsn // +
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnRestrictedString(const unsigned char *alphabet,const std::size_t alphabet_size
		, const unsigned char * inverse_table, std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		: VS_StringAsn( constraint, lowerLimit, upperLimit, extendable )
		,alphabet_size(alphabet_size)
		,alphabet(const_cast<unsigned char*>(alphabet))
		,inverse_table(const_cast<unsigned char*>(inverse_table))
	{
		if (alphabet_size==1)	charSetUnalignedBits=1;
		else
		{
			std::size_t   bs = 0;
			while (bs < (sizeof(std::uint32_t) * 8) && alphabet_size > std::size_t(1 << bs))	++bs;
			charSetUnalignedBits = bs;
		}
		charSetAlignedBits = 1;
		while (charSetUnalignedBits > charSetAlignedBits)
		charSetAlignedBits <<= 1;
	}
	// end VS_AsnRestrictedString::VS_AsnRestrictedString

	VS_AsnRestrictedString(const VS_AsnRestrictedString&) = delete;

	/////////////////////////////////////////////////////////////////////////////////////
	std::size_t charSetUnalignedBits;
	std::size_t charSetAlignedBits;
	std::size_t		   alphabet_size;
	unsigned char   *alphabet;
	unsigned char   *inverse_table;

	/////////////////////////////////////////////////////////////////////////////////////

    static bool MakeInverseCodeTable(  unsigned char * inverse_table , const unsigned char * alphabet, std::size_t size);

	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnRestrictedString & operator = ( const VS_AsnRestrictedString &src);// {    return;	}
	// end VS_AsnRestrictedString::operator =
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnRestrictedString::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnRestrictedString::Encode
	/////////////////////////////////////////////////////////////////////////////////////
	std::size_t GetBitsPerChar(bool aligned)
	{
		return (aligned) ? charSetAlignedBits : charSetUnalignedBits;
	}
	bool GetNormalString(char string[], std::size_t &sz)
	{
		if (!filled)
			return false;

		value.ResetIndex();
		const auto bits = GetBitsPerChar(true);
		std::size_t j=0;
		for(;j<sz;++j)
		{
			std::uint32_t bit = 0;
			if (value.GetBits(bit,bits))
			{
				//////////////////////////
				/// Прошу меня простить, но я не знаю,
				/// зачем надо сдвигать на 3.
				/// Так делает Polycom.
				//////////////////////////
				bit -=3;
				//////////////////////////
				string[j] = alphabet[bit];
			}else break;

		}
		string[j] = 0;
		sz = j+1;
		return true;
	}

	bool GetNormalStringNoShift(char string[], std::size_t &sz)
	{
		if (!filled)
			return false;

		value.ResetIndex();
		const auto bits = GetBitsPerChar(true);
		std::size_t j=0;
		for(;j<sz;++j)
		{
			unsigned int bit = 0;
			if (value.GetBits(bit,bits))
			{
				//////////////////////////
				/// Прошу меня простить, но я не знаю,
				/// зачем надо сдвигать на 3.
				/// Так делает Polycom.
				//////////////////////////
	//			bit -=3;
				//////////////////////////
				string[j] = alphabet[bit];
			}else break;

		}
		string[j] = 0;
		sz = j+1;
		return true;
	}

	bool SetNormalString(const char* string, const std::size_t sz)
	{
		value.ResetIndex();

		const auto bits = GetBitsPerChar(true);

		for (std::remove_const<decltype(sz)>::type i=0; i < sz; i++)
		{
			const char ch = string[i];
			decltype(alphabet_size) j=0;
			for(; j < alphabet_size; j++)
			{
				if (ch == alphabet[j])
					break;
			}
			//bit = atoi(&ch);
			auto bit = j;

			//bit = (unsigned int) (a >> (sz - i - 1));
			bit += 3;

			value.AddBits(bit, bits);
		}

		filled = true;

		return true;
	}
};
// end VS_AsnRestrictedString struct

/////////////////////////////////////////////////////////////////////////////////////////
struct VS_AsnNumericString : public VS_AsnRestrictedString
{
	/////////////////////////////////////////////////////////////////////////////////////////
	VS_AsnNumericString( std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		:VS_AsnRestrictedString(ALPHABETIC,sizeof(ALPHABETIC),inverse_t,lowerLimit, upperLimit,constraint, extendable)
	{}
	// end VS_AsnNumericString::VS_AsnNumericString

	VS_AsnNumericString( const unsigned char *alphabet, const std::size_t alphabet_size
		, const unsigned char * inverse_table,std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		:VS_AsnRestrictedString(alphabet,alphabet_size,inverse_table
		,lowerLimit, upperLimit,constraint, extendable)
	{}
	// end VS_AsnNumericString::VS_AsnNumericString

	 static constexpr unsigned char ALPHABETIC[] = { ' ','0','1','2','3','4','5','6','7','8','9' };
	 static unsigned char inverse_t[];
	 static const bool flg;
	 /////////////////////////////////////////////////////////////////////////////////////

	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnNumericString::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnNumericString::Encode

	/////////////////////////////////////////////////////////////////////////////////////

};
// end VS_AsnNumericString struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnIA5String : public  VS_AsnRestrictedString// +
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnIA5String( std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		:VS_AsnRestrictedString(static_cast<const unsigned char*>(ALPHABETIC),sizeof(ALPHABETIC),static_cast<const unsigned char*>(inverse_t),lowerLimit
		, upperLimit,constraint, extendable)
	{}
	// end VS_AsnIA5String::VS_AsnIA5String

	VS_AsnIA5String( const unsigned char *alphabet, const std::size_t alphabet_size
		, const unsigned char * inverse_table,std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		:VS_AsnRestrictedString(alphabet,alphabet_size,inverse_table
		,lowerLimit, upperLimit,constraint, extendable)
	{}
	// end VS_AsnIA5String::VS_AsnIA5String

	static constexpr unsigned char ALPHABETIC[] =
	 {
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
	 	48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,
		95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127
	 };

	 static unsigned char inverse_t[];
	 static const bool flg;
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnIA5String::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnIA5String::Encode
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnIA5String struct

/////////////////////////////////////////////////////////////////////////////////////////
struct VS_AsnPrintableString : public  VS_AsnRestrictedString// +
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnPrintableString( std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		:VS_AsnRestrictedString(ALPHABETIC,sizeof(ALPHABETIC),inverse_t,lowerLimit, upperLimit,constraint, extendable)
	{}
	// end VS_AsnPrintableString::VS_AsnPrintableString

	VS_AsnPrintableString( const unsigned char *alphabet, const std::size_t alphabet_size
		, const unsigned char * inverse_table,std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		:VS_AsnRestrictedString(alphabet,alphabet_size,inverse_table
		,lowerLimit, upperLimit,constraint, extendable)
	{}
	// end VS_AsnPrintableString::VS_AsnPrintableString

	static constexpr unsigned char ALPHABETIC[] = { 32,39,'(',')','+',44,'-','.','/','0','1','2','3','4','5',
		'6','7','8','9',':','=','?','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X',
		'Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };
	 static unsigned char inverse_t[];
	 static const bool flg;
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnPrintableString::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnPrintableString::Encode
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnPrintableString struct

/////////////////////////////////////////////////////////////////////////////////////////
struct VS_AsnGeneralString : public  VS_AsnRestrictedString// +
{
	/////////////////////////////////////////////////////////////////////////////////////
	VS_AsnGeneralString()
	:VS_AsnRestrictedString(ALPHABETIC,sizeof(ALPHABETIC),inverse_t,0
	, ~0,VS_Asn::Unconstrained, false)
	{}
	// end VS_AsnGeneralString::VS_AsnGeneralString

	VS_AsnGeneralString( std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		:VS_AsnRestrictedString(ALPHABETIC,sizeof(ALPHABETIC),inverse_t,lowerLimit
		, upperLimit,constraint, extendable)
	{}
	// end VS_AsnGeneralString::VS_AsnGeneralString

	VS_AsnGeneralString( const unsigned char *alphabet, const std::size_t alphabet_size
		, const unsigned char * inverse_table, std::int32_t lowerLimit, std::uint32_t upperLimit
		, ConstraintType constraint=VS_Asn::FixedConstraint
		, bool extendable = false )
		:VS_AsnRestrictedString(alphabet,alphabet_size,inverse_table
		,lowerLimit, upperLimit,constraint, extendable)
	{}
	// end VS_AsnGeneralString::VS_AsnGeneralString

	 //static bool MakeInverseCodeTable(  unsigned char * inverse_table , unsigned char * alphabet, const unsigned size);

	 static constexpr unsigned char ALPHABETIC[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,
	 	22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,
	 	62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,
	 	102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,
	 	133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,
	 	164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,
	 	195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,
	 	226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254 };
	 static unsigned char inverse_t[];
	 static const bool flg;
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnGeneralString::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnGeneralString::Encode
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnGeneralString struct
/*
struct VS_AsnGeneralString : public VS_StringAsn // -
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnGeneralString( void *value, unsigned size,
							ConstraintType constraint, int lowerLimit, unsigned upperLimit,
							bool extendable = false )
		: VS_StringAsn( value, size, constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnGeneralString::VS_AsnGeneralString

	VS_AsnGeneralString( ConstraintType constraint, int lowerLimit, unsigned upperLimit,
							bool extendable = false )
		: VS_StringAsn( constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnGeneralString::VS_AsnGeneralString

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnGeneralString struct
*/
/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnBmpString : public VS_StringAsn // +
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnBmpString( void *value, std::size_t size,
						ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable = false )
		: VS_StringAsn( value, size, constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnBmpString::VS_AsnBmpString

	VS_AsnBmpString( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
						bool extendable = false )
		: VS_StringAsn( constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnBmpString::VS_AsnBmpString
	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_AsnBmpString::Decode
	bool Encode(VS_PerBuffer& buffer) override;
	// end VS_AsnBmpString::Encode
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnBmpString struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnGeneralisedTime : public VS_AsnVisibleString // -
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnGeneralisedTime( void *value, std::size_t size,
							ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
							bool extendable = false )
		: VS_AsnVisibleString( value, size, constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnGeneralisedTime::VS_AsnGeneralisedTime

	VS_AsnGeneralisedTime( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
							bool extendable = false )
		: VS_AsnVisibleString( constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnGeneralisedTime::VS_AsnGeneralisedTime

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnGeneralisedTime struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnUniversalTime : public VS_AsnVisibleString	// -
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnUniversalTime( void *value, std::size_t size,
							ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
							bool extendable = false )
		: VS_AsnVisibleString( value, size, constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnUniversalTime::VS_AsnUniversalTime

	VS_AsnUniversalTime( ConstraintType constraint, std::int32_t lowerLimit, std::uint32_t upperLimit,
							bool extendable = false )
		: VS_AsnVisibleString( constraint, lowerLimit, upperLimit, extendable ) {}
	// end VS_AsnUniversalTime::VS_AsnUniversalTime

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnUniversalTime struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnChoice : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnChoice( const unsigned tag, VS_Asn *choice, const unsigned num_choices,
					const unsigned num_ext_choices, bool extendable )
		: VS_Asn( extendable ), choice(choice)
		, num_choices(num_choices), num_ext_choices(num_ext_choices), tag(tag)
	{	filled = true;	}
	// end VS_AsnChoice::VS_AsnChoice

	VS_AsnChoice( const unsigned num_choices, const unsigned num_ext_choices, bool extendable )
		: VS_Asn( extendable ), choice(nullptr)
		, num_choices(num_choices), num_ext_choices(num_ext_choices), tag(~0)
		{}
	// end VS_AsnChoice::VS_AsnChoice

	~VS_AsnChoice( void ) {		FreeChoice();		}
	// end VS_AsnChoice::~VS_AsnChoice
	VS_Asn   *choice;

	unsigned   num_choices;
	unsigned   num_ext_choices;

	unsigned   tag;

	/////////////////////////////////////////////////////////////////////////////////////

	inline void Clear( void ) {		tag = ~0;	choice = nullptr;		filled = false;		}
	// end VS_AsnChoice::Clear

	bool DecodeChoice( VS_PerBuffer &buffer, VS_Asn *choice );
	// end of VS_AsnChoice::DecodeChoice

	inline void FreeChoice( void ) noexcept
	{
		delete choice;
		Clear();
	}
	// end VS_AsnChoice::FreeChoice

	bool Encode(VS_PerBuffer& buffer) override;

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnChoice struct
/////////////////////////////////////////////////////////////////////////////////////////

struct VS_Reference_of_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////////
	VS_Reference_of_Asn( void ) : reference(nullptr), flag(false) {	}
	// end VS_Reference_of_Asn:VS_Reference_of_Asn
	VS_Asn * reference;
	bool flag;
	/////////////////////////////////////////////////////////////////////////////////////////
	void Set(VS_Asn * in, const bool in_fl)
	{	reference = in;		flag = in_fl;		}
	// end	VS_Reference_of_Asn::Set
	/////////////////////////////////////////////////////////////////////////////////////////

	VS_Reference_of_Asn& operator=(const VS_Reference_of_Asn& src)
	{
		if (this == &src)
		{
			return *this;
		}
		if (*this != src)
		{
			*this->reference = *src.reference;
			this->flag = src.flag;
		}
		return *this;
	}

	bool operator!=(const VS_Reference_of_Asn& other)const
	{
		return (this->reference != other.reference) || (this->flag != other.flag);
	}

};
// end VS_Array_of_reference struct
/////////////////////////////////////////////////////////////////////////////////////////
struct VS_AsnSequence : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////

	explicit VS_AsnSequence( bool extendable = false )
		: VS_Asn( extendable )
		, basic_ref(nullptr),extension_ref(nullptr)
		, basic_num(0),extension_num(0)
		, optionMap( FixedConstraint, 0, 0 )
		, knownExtensions(0), totalExtensions(0)
		, extensionMap(FixedConstraint, 0, 0)
		{}
	// end VS_AsnSequence::VS_AsnSequence

	VS_AsnSequence(std::uint32_t num_optional,
				   VS_Reference_of_Asn* basic_in,std::uint32_t basic_n,
				   VS_Reference_of_Asn* exten_in ,std::uint32_t exten_n,
		    	   bool extendable )
		: VS_Asn( extendable )
		, basic_ref(basic_in), extension_ref(exten_in)
		, basic_num(basic_n), extension_num(exten_n)
		, optionMap(FixedConstraint, num_optional, num_optional)
		, knownExtensions(exten_n), totalExtensions(0)
		, extensionMap(FixedConstraint, exten_n, exten_n)
		{}
	// end VS_AsnSequence::VS_AsnSequence

	VS_Reference_of_Asn * basic_ref;
	VS_Reference_of_Asn * extension_ref;

	std::uint32_t basic_num;
	std::uint32_t extension_num;

	/////////////////////////////////////////////////////////////////////////////////////
	bool FillOptionMap( void );
	// end VS_AsnSequence::FillOptionMap
	bool FillExtensionMap( void );
	// end VS_AsnSequence::FillExtensionMap

	bool Decode(VS_PerBuffer& buffer) override;

	bool Encode(VS_PerBuffer& buffer) override;

	bool PreambleDecode( VS_PerBuffer &buffer );
	// end VS_AsnSequence::Decode

	bool PreambleEncode( VS_PerBuffer &buffer );
	// end VS_AsnSequence::Encode

	VS_AsnBitString   optionMap;
	std::int32_t   knownExtensions, totalExtensions;
	VS_AsnBitString   extensionMap;

	/////////////////////////////////////////////////////////////////////////////////////

protected:
    VS_AsnSequence(const VS_AsnSequence&src) = default;
	void operator =( const VS_AsnSequence &src )
	{
		this->basic_ref       = src.basic_ref;
		this->extension_ref   = src.extension_ref;
		this->basic_num       = src.basic_num;
		this->extension_num   = src.extension_num;
		this->knownExtensions = src.knownExtensions;
		this->totalExtensions = src.totalExtensions;
	}
	// end VS_AsnSequence::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnSequence struct
/////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
/*
////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnSequence : public VS_Asn
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnSequence( bool extendable = false )
		: VS_Asn( extendable )
		, optionMap( FixedConstraint, 0, 0 )
		, extensionMap( FixedConstraint, 0, 0 )
		, knownExtensions(0), totalExtensions(0) {}
	// end VS_AsnSequence::VS_AsnSequence

	VS_AsnSequence( unsigned num_optional, unsigned num_extensional,
						bool extendable = false )
		: VS_Asn( extendable )
		, optionMap( FixedConstraint, num_optional, num_optional )
		, extensionMap( FixedConstraint, num_extensional, num_extensional )
		, knownExtensions(num_extensional), totalExtensions(0) {}
	// end VS_AsnSequence::VS_AsnSequence

	virtual bool FillOptionMap( void );
	// end VS_AsnSequence::FillOptionMap

	bool PreambleDecode( VS_PerBuffer &buffer );
	// end VS_AsnSequence::Decode

	bool PreambleEncode( VS_PerBuffer &buffer );
	// end VS_AsnSequence::Encode

	VS_AsnBitString   optionMap;
	int   knownExtensions, totalExtensions;
	VS_AsnBitString   extensionMap;

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnSequence struct
*/
/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnSet : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	explicit VS_AsnSet( bool extendable = false )
		: VS_AsnSequence( extendable ) {}
	// end VS_AsnSet::VS_AsnSet

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_AsnSet struct

/////////////////////////////////////////////////////////////////////////////////////////
