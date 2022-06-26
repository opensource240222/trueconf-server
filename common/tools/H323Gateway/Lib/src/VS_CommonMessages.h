/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 16.01.04     by  A.Slavetsky
//  Modified:     A.Vlaskin, A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_CommonMessages.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VS_Containers.h"
#include "VS_AsnBuffers.h"

#include <cstddef>
#include <memory>
#include "std-generic/compat/memory.h"

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AsnNullsChoice : public VS_AsnChoice
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AsnNullsChoice( const unsigned tag, const unsigned num_choices,
						const unsigned num_ext_choices, bool extendable )
		: VS_AsnChoice( tag, nullptr, num_choices, num_ext_choices, extendable ) {}
	// end of VS_AsnNullsChoice constructor

	VS_AsnNullsChoice( const unsigned num_choices, const unsigned num_ext_choices,
						bool extendable )
		: VS_AsnChoice( num_choices, num_ext_choices, extendable ) {}
	// end of VS_AsnNullsChoice constructor

	/////////////////////////////////////////////////////////////////////////////////////

	bool Decode(VS_PerBuffer& buffer) override
	{
		if (!buffer.ChoiceDecode( *this ))	return false;
		if (!extendable)
		{	if (tag >= num_choices)		return false;
		} else
		{	if (tag >= num_ext_choices)
				return buffer.ChoiceMissExtensionObject(*this);
		}
		return filled = true;
	}
	// end of VS_AsnNullsChoice::Decode

	bool Encode(VS_PerBuffer& buffer) override
	{
		if (!filled || tag > num_ext_choices)	return false;
		if (!buffer.ChoiceEncode( *this ))	return false;
		return true;
	}
	// end of VS_AsnNullsChoice::Encode

	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_AsnNullsChoice &src )
	{
		FreeChoice();
		if (!src.filled)	return;
		O_CC(tag);	filled = true;
	}
	// end of VS_AsnNullsChoice::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_AsnNullsChoice struct

template <class Type>
struct Array_of_type : public VS_ConstrainedArrayAsn
{
	typedef Type value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef pointer iterator;
	typedef const_pointer const_iterator;
	typedef std::size_t  size_type;
	typedef std::ptrdiff_t difference_type;

	size_type size() const { return length_; }
	bool empty() const { return size() == 0; }
	      pointer data()       { return static_cast<Type*>(asns_); }
	const_pointer data() const { return static_cast<const Type*>(asns_); }
   	     reference operator[](size_type pos) { assert(pos < length_); return data()[pos]; }
   const_reference operator[](size_type pos) const { assert(pos < length_); return data()[pos]; }
	      iterator begin()       { return data(); }
	const_iterator begin() const { return data(); }
	      iterator end()         { return data() + size(); }
	const_iterator end()   const { return data() + size(); }

	/////////////////////////////////////////////////////////////////////////////////////

	Array_of_type( void )
		: VS_ConstrainedArrayAsn()
	{}
	// end Array_of_type::Array_of_type
	virtual ~Array_of_type()
	{
		DeleteArray<Type>();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode( VS_PerBuffer &buffer ) override
	{
		DeleteArray<Type>();
		if (!DecodeLength( buffer )) return false;

		std::unique_ptr<Type[]> types = vs::make_unique<Type[]>(length_);

		for (size_type i = 0; i < length_; ++i)
		{
			if (!types[i].Decode(buffer))
			{
				return false;
			}
		}
		asns_ = types.release();
		filled = true;
		return true;
	}
	// end Array_of_type::Decode

	bool Encode( VS_PerBuffer &buffer ) override
	{
		if (!EncodeLength( buffer ))	return false;
		Type   *types = static_cast<Type *>(asns_);
		for (size_type i = 0; i < length_; ++i)
			if (!types[i].Encode( buffer ))	return false;
		return true;
	}
	// end Array_of_type::Encode

	/////////////////////////////////////////////////////////////////////////////////////

	Array_of_type<Type> &operator=( const Array_of_type<Type> &src )
	{
		DeleteArray<Type>();
		if (!src.filled) return *this;
		if (src.length_ && src.asns_)
		{
			length_ = src.length_;
			Type * types = new Type[length_];
			for (size_type i = 0; i < length_; ++i)	types[i] = (static_cast<Type *>(src.asns_))[i];
			asns_ = types;
			constraint = src.constraint;
			extendable = src.extendable;
			filled = true;
		}
		return *this;
	}

	void reset(Type *types = nullptr, const std::size_t length = 0) noexcept
	{
		assert(types ? length : !length);

		DeleteArray<Type>();
		length_ = length;
		asns_ = types;
		filled = (types != nullptr && length != 0);
	}

	// end Array_of_type::operator= const Array_of_type &
protected:
	Array_of_type(std::int32_t lower, std::uint32_t upper
		, ConstraintType constr, bool extend) : VS_ConstrainedArrayAsn(lower, upper, constr, extend)
	{
	}
	/////////////////////////////////////////////////////////////////////////////////////
};
// end of Array_of_type template

/////////////////////////////////////////////////////////////////////////////////////////
template <class Type,int low, unsigned upp
		, VS_Asn::ConstraintType cons = VS_Asn::FixedConstraint
		, bool ext = false> struct Constrained_array_of_type : public Array_of_type<Type>
{
	/////////////////////////////////////////////////////////////////////////////////////
	Constrained_array_of_type( void )
		: Array_of_type<Type>(low,upp,cons,ext)
	{}
	/////////////////////////////////////////////////////////////////////////////////////
};
// end of Constrained_array_of_type template
/////////////////////////////////////////////////////////////////////////////////////

template <int lowL
		 ,unsigned uppL
		 ,VS_Asn::ConstraintType cstrnt=VS_Asn::FixedConstraint
		 ,bool ext=false
		 > struct TemplInteger: public VS_AsnInteger
{
	/////////////////////////////////////////////////////////////////////////////////////

	TemplInteger( void )
		: VS_AsnInteger(cstrnt ,lowL ,uppL , ext )
		{}
	// end TemplInteger::TemplInteger

	/////////////////////////////////////////////////////////////////////////////////////

	bool Decode( VS_PerBuffer &buffer ) override
	{	return buffer.IntegerDecode( *this );	}
	// end TemplInteger::Decode

	bool Encode( VS_PerBuffer &buffer ) override
	{	return buffer.IntegerEncode( *this );	}
	// end TemplInteger::Encode

	bool operator==( const TemplInteger<lowL,uppL,cstrnt,ext> &src ) const
	{	return value == src.value;	}
	// end TemplInteger::operator==( const TemplInteger<cstrnt,lowL,uppL,ext> & )

	void operator=( const TemplInteger<lowL,uppL,cstrnt,ext>& src )
	{
		if (!src.filled) {		value = 0;	filled = false;		}
		else {		value = src.value;		filled = true;		}
	}
	// end TemplInteger::operator==( const TemplInteger<cstrnt,lowL,uppL,ext> & )

	/////////////////////////////////////////////////////////////////////////////////////
};
// end TemplInteger template

/////////////////////////////////////////////////////////////////////////////////////////

template <int lowL
		 ,unsigned uppL
		 ,VS_Asn::ConstraintType costr=VS_Asn::FixedConstraint
		 ,bool ext = false
		 > struct TemplOctetString: public VS_AsnOctetString
{
	/////////////////////////////////////////////////////////////////////////////////////

	TemplOctetString( void )
		:VS_AsnOctetString(costr,lowL,uppL,ext)
	{}
	// end TemplOctetString::TemplOctetString

	/////////////////////////////////////////////////////////////////////////////////////

	bool Decode(VS_PerBuffer& buffer) override
	{	return buffer.OctetStringDecode( *this );		}
	// end TemplOctetString::Decode

	inline bool Encode(VS_PerBuffer& buffer) override
	{	return buffer.OctetStringEncode( *this );		}
	// end TemplOctetString::Encode

	/////////////////////////////////////////////////////////////////////////////////////
	void operator=(const TemplOctetString<lowL,uppL,costr,ext> &src)
	{
		value = src.value;
		filled = src.filled;
	}
	/////////////////////////////////////////////////////////////////////////////////////
	bool operator==(const TemplOctetString<lowL,uppL,costr,ext> &src)
	{	return( value == src.value); 	}

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of TemplOctetString template

/////////////////////////////////////////////////////////////////////////////////////////

template <const unsigned char *alphabet__,
		  unsigned alphabet_size__,
		  unsigned char * inverse_table__,
		  int lowL__,
		  unsigned uppL__,
		  VS_Asn::ConstraintType costr__=VS_Asn::FixedConstraint,
		  bool ext__ = false
		  > struct TemplAlphabeticString: public VS_AsnRestrictedString
{
	/////////////////////////////////////////////////////////////////////////////////////

	TemplAlphabeticString( void )
		: VS_AsnRestrictedString(alphabet__, alphabet_size__, inverse_table__, lowL__, uppL__, costr__, ext__)
	{}
	// end TemplAlphabeticString::TemplAlphabeticString
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override
	{	return VS_AsnRestrictedString::Decode( buffer );		}
	// end TemplAlphabeticString::Decode

	inline bool Encode(VS_PerBuffer& buffer) override
	{	return VS_AsnRestrictedString::Encode( buffer );		}
	// end TemplAlphabeticString::Encode

	/////////////////////////////////////////////////////////////////////////////////////
};
//end TemplAlphabeticString template
/////////////////////////////////////////////////////////////////////////////////////////

template <int lowL
		 ,unsigned uppL
		 ,VS_Asn::ConstraintType costr=VS_Asn::FixedConstraint
		 ,bool ext = false
		 > struct TemplIA5String: public VS_AsnIA5String
{
	/////////////////////////////////////////////////////////////////////////////////////

	TemplIA5String( void )
		:VS_AsnIA5String(lowL,uppL,costr,ext)
	{}
	// end TemplIA5String::TemplIA5String
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override
	{	return buffer.IA5StringDecode( *this );		}
	// end TemplIA5String::Decode

	inline bool Encode(VS_PerBuffer& buffer) override
	{	return buffer.IA5StringEncode( *this );		}
	// end TemplIA5String::Encode

	/////////////////////////////////////////////////////////////////////////////////////
};
//end TemplIA5String template

/////////////////////////////////////////////////////////////////////////////////////
template <int lowL
		 ,unsigned uppL
		 ,VS_Asn::ConstraintType costr=VS_Asn::FixedConstraint
		 ,bool ext = false
		 > struct TemplNumericString: public VS_AsnNumericString
{
	/////////////////////////////////////////////////////////////////////////////////////

	TemplNumericString( void )
		:VS_AsnNumericString(lowL,uppL,costr,ext)
	{}
	// end TemplIA5String::TemplIA5String
	/////////////////////////////////////////////////////////////////////////////////////
	//bool Decode(VS_PerBuffer& buffer) override
	//{	return buffer.NumericString( *this );		}
	// end TemplIA5String::Decode

	//inline bool Encode(VS_PerBuffer& buffer) override
	//{	return buffer.IA5StringEncode( *this );		}
	// end TemplIA5String::Encode

	/////////////////////////////////////////////////////////////////////////////////////
};
//end TemplIA5String template

/////////////////////////////////////////////////////////////////////////////////////

#define DEF_Simple_Templ(name) \
	template <int lowL \
		 ,unsigned uppL \
		 ,VS_Asn::ConstraintType costr=VS_Asn::FixedConstraint \
		 ,bool ext = false \
		 > struct Templ##name : public VS_Asn##name \
	{ \
	Templ##name() \
	: VS_Asn##name(costr, lowL, uppL, ext) \
	{}\
	bool Decode(VS_PerBuffer& buffer) override \
	{	return buffer.name##Decode(*this);		} \
	bool Encode(VS_PerBuffer& buffer) override \
	{	return buffer.name##Encode(*this);		} \
	void operator=(const Templ##name<lowL, uppL, costr, ext>& src) \
	{	O_CC(filled);	O_CC(value);		}\
	bool operator==(const Templ##name<lowL, uppL, costr, ext>& src) \
	{	return( value == src.value); 	}\
	};\
	/*
//	void operator=(const Templ##name& src) \
//	{	O_C(value); 	O_CC(filled);	O_C(constraint);\
//		O_C(upperLimit);	O_C(lowerLimit);}\*/

#define DEF_String_Templ(name) \
	template <int lowL \
		 ,unsigned uppL \
		 ,VS_Asn::ConstraintType costr=VS_Asn::FixedConstraint \
		 ,bool ext = false \
		> struct Templ##name : public VS_Asn##name \
	{ \
	Templ##name() \
	: VS_Asn##name(lowL, uppL, costr, ext) \
	{}\
	bool Decode(VS_PerBuffer& buffer) override \
	{	return buffer.RestrictedStringDecode( *this );		}\
	bool Encode(VS_PerBuffer& buffer) override \
	{	return buffer.RestrictedStringEncode( *this );		}\
	};\
/*
 *	DEF_Simple_Templ - дефайновый темплейт для простых ASN типов, кроме алфавитных строк
 *	DEF_String_Templ - дефайновый темплейт только!!! для алфавитных строк
 *  (с) Alex
 */
//DEF_Simple_Templ(OctetString);	// Define Temlate of OctetString
DEF_Simple_Templ(BmpString);	// Define Temlate of BmpString
DEF_Simple_Templ(BitString);	// Define Temlate of BitString
DEF_String_Templ(PrintableString);	// Define Temlate of PrintableString

/////////////////////////////////////////////////////////////////////////////////////
template <class T> void CopyChoice(const VS_AsnChoice &src,VS_AsnChoice & dest)//(tag,src.choice,*this)
{
	dest.FreeChoice();
	T * obj = new T;
	*obj = *static_cast<T*>(src.choice);
	dest.choice = obj;
	dest.tag = src.tag;
	dest.filled = true;
};

/////////////////////////////////////////////////////////////////////////////////////
template <typename Type> struct Type_id : public VS_AsnOctetString
{	Type_id( void )
		: VS_AsnOctetString(VS_Asn::Unconstrained,0,UINT_MAX)
	{}
	bool EncodeSubType( Type * obj )
	{	return VS_AsnOctetString::EncodeSubType( obj );	}
	// end EncodeSubType
	bool DecodeSubType( Type * obj )
	{	return VS_AsnOctetString::DecodeSubType( obj );	}
	// end DecodeSubType
};
// end Type_id template
/////////////////////////////////////////////////////////////////////////////////////

template< bool initValue> class VS_TemplAsnNull :
 public VS_AsnNull
 {
 public:
	 VS_TemplAsnNull():
	VS_AsnNull()
	{	VS_AsnNull::filled = initValue;
	}
 };
