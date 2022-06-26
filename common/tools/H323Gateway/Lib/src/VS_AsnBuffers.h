/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 20.09.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_BitBuffers.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <assert.h>
#include <climits>

#include "VS_BaseBuffers.h"
#include "VS_Containers.h"
#include "std-generic/attributes.h"

/////////////////////////////////////////////////////////////////////////////////////////

static inline std::size_t CountBits( const std::size_t range )
{
	switch (range)
	{
	case 0 :	return sizeof(std::uint32_t) * 8;
	case 1 :	return 1;
	default:	break;
	}
	std::size_t   bs = 0;
	while (bs < (sizeof(std::uint32_t) * 8) && range > static_cast<std::size_t>(1 << bs))	++bs; //Только больше потому, что
	return bs;// к range пользованели прибавляют 1 ! (c)Alex
};
// end CountBits


/////////////////////////////////////////////////////////////////////////////////////////

struct VS_PerBuffer : public VS_BitBuffer
{
	VS_PerBuffer( const void *buffer, const std::size_t size, const bool alignment = true )
		: VS_BitBuffer( buffer, size )
		, aligned(alignment) {}
	// end VS_PerBuffer::VS_PerBuffer

	explicit VS_PerBuffer( const bool alignment = true )
		  : aligned(alignment) {}
	// end VS_PerBuffer::VS_PerBuffer

	~VS_PerBuffer( void ) {}
	// end VS_PerBuffer::~VS_PerBuffer

	bool   aligned;

	/////////////////////////////////////////////////////////////////////////////////////
	inline void CopyBuff( const VS_BitBuffer &src)
	{
		this->CopyBuffer( src );
	}
	inline bool SmallUnsignedDecode( std::uint32_t &value )	// 10.6
	{
		std::uint32_t   bt;
		if (!GetBit( bt ))		return false;
		if (!bt)	return GetBits( value, 6 );	// 10.6.1
		std::size_t    ln;
		if (!LengthDecode( 0, INT_MAX, ln ))	return false;	// 10.6.2
		ByteAlign();
		return GetBits(value , ln * 8 );
	}
	// end VS_PerBuffer::SmallUnsignedDecode

	inline bool SmallUnsignedEncode(const std::uint32_t value )	// 10.6
	{

		if (value<=63)
		{
			AddBit(0);
			return AddBits( value, 6 );	// 10.6.1
		}
		else
		{
			const auto ln = CountBits(value+1);
			AddBit(1);
			if (!LengthEncode( 0, INT_MAX, (ln+7)/8 ))	return false;	// 10.6.2
			ByteAlignSize();
			return AddBits(value , ((ln+7)/8) * 8 );
		}
		return false;
	}
	// end VS_PerBuffer::SmallUnsignedEncode

	inline bool UnsignedDecode( std::uint32_t lower, std::uint32_t upper, std::uint32_t &value )	// 10.5
	{
		if (lower == upper) {	value = lower;	return true;	}	// 10.5.4
		const auto rg = (upper - lower) + 1;
		auto bs = CountBits(rg);
		if (aligned && (rg == 0 || rg > 255))		// not 10.5.6 and not 10.5.7.1
		{	if (bs > 16)							// not 10.5.7.4
			{	LengthDecode( 1, ( bs + 7 ) / 8, bs );	// 12.2.6
				bs *= 8;
			} else if (bs > 8)	// not 10.5.7.2
				bs = 16;		// 10.5.7.3
			ByteAlign();		// 10.7.5.2 - 10.7.5.4
		}
		if (!GetBits( value, bs ))	return false;
		value += lower;		return true;
	}
	// end VS_PerBuffer::UnsignedDecode

	inline bool UnsignedEncode( std::uint32_t lower, std::uint32_t upper, const std::uint32_t value)	// 10.5//не оттестирована
	{	auto val = value;
		if (lower == upper)  return true;		// 10.5.4
		/*
		unsigned   rg = ( upper - lower ) + 1, bs = CountBits( rg ),bv = CountBits( val - lower);
		if (aligned && (rg == 0 || rg > 255))		// not 10.5.6 and not 10.5.7.1
		{	if (bs > 16)							// not 10.5.7.4
			{
				bv = (bv+7)/8;
				LengthEncode( 1, ( bs + 7 ) / 8, bv );	// 12.2.6
				bv *=8; ;
			} else if (bs > 8)	// not 10.5.7.2
				bv = 16;		// 10.5.7.3
			else if (bs==8) bv = 8;
			ByteAlignSize();	// 10.7.5.2 - 10.7.5.4
			bs=bv;
		}
		val-=lower;
		if (!AddBits( val, bs ))	return false;
		return true;
		//*/
///*
		const auto   range = ( upper - lower ) + 1;//, bs = CountBits( rg ),bv = CountBits( val - lower);
		auto   nBits = CountBits(range);
		if (static_cast<std::uint32_t>(val) < lower)
			val = 0;
		else
			val -= lower;
		if (aligned && (range == 0 || range > 255))
		{ // not 10.5.6 and not 10.5.7.1
			if (nBits > 16)
			{                           // not 10.5.7.4
				const auto numBytes = val == 0 ? 1 : (((CountBits(val + 1))+7)/8);
				LengthEncode( 1, (nBits+7)/8,numBytes);    // 12.2.6
				nBits = numBytes*8;
			}
			else if (nBits > 8)      // not 10.5.7.2
				nBits = 16;            // 10.5.7.3
			ByteAlignSize();             // 10.7.5.2 - 10.7.5.4
		}
		return AddBits(val, nBits);//*/
	}
	// end VS_PerBuffer::UnsignedEncode

	inline bool LengthDecode(std::uint32_t lower, std::uint32_t upper, std::size_t &length )	// 10.9
	{
		if (upper != INT_MAX && !aligned)
		{	if (upper - lower > 0xffff)		return false;	// 10.9.4.2 unsupported
			std::uint32_t   bs = 0;
			if (!GetBits( bs , CountBits( upper - lower + 1 ) ))	return false;
			length = lower + bs;	return true;			// 10.9.4.1
		}
		if (upper <= 65536)
		{
			auto value = static_cast<std::uint32_t>(length);
			const auto res = UnsignedDecode(lower, upper, value);	// 10.9.3.3
			length = value;
			return res;
		}
		ByteAlign();	// 10.9.3.5
		std::uint32_t   bt;
		if (!GetBit( bt ))		return false;
		if (!bt)
		{
			auto value = static_cast<std::uint32_t>(length);
			const auto res = GetBits(value, 7); // 10.9.3.6
			length = value;
			return res;
		}
		if (!GetBit( bt ))		return false;
		if (!bt)
		{
			auto value = static_cast<std::uint32_t>(length);
			const auto res = GetBits(value, 14); // 10.9.3.6
			length = value;
			return res;
		}
		return false;	// 10.9.3.8 unsupported
	}
	// end VS_PerBuffer::LengthDecode

	inline bool LengthEncode( std::uint32_t lower, std::uint32_t upper, const std::size_t length )	// 10.9//не оттестирована
	{
		if (upper != INT_MAX && !aligned)
		{	if (upper - lower > 0xffff)		return false;	// 10.9.4.2 unsupported
			const std::uint32_t  bs = static_cast<std::uint32_t>(length) - lower;				// 10.9.4.1
			return AddBits(bs, CountBits(upper - lower + 1));
		}
		if (upper < 65536)
			return UnsignedEncode( lower, upper, length );	// 10.9.3.3
		ByteAlignSize();	// 10.9.3.5
		if (length <= 127)
		{
			AddBit(0);
			return AddBits(static_cast<std::uint32_t>(length), 7);
		}
		if (length < 16*1024)
		{
			AddBit(1);
			AddBit(0);
			return AddBits(static_cast<std::uint32_t>(length), 14);
		}
		return false; // else not supported 10.9.3.8
	}
	// end VS_PerBuffer::LengthEncode

	inline bool ConstrainedLengthDecode( bool extendable, VS_Asn::ConstraintType constraint,
												std::uint32_t lower, std::uint32_t upper, std::size_t &length )
	{
		std::uint32_t   bt;
		if ((extendable && GetBit( bt )) || constraint == VS_Asn::Unconstrained)
				return LengthDecode( 0, INT_MAX, length );
		return LengthDecode( lower, upper, length );
	}
	// end VS_PerBuffer::ConstrainedLengthDecode

	inline bool ConstrainedLengthEncode( bool extendable, VS_Asn::ConstraintType constraint,
											std::uint32_t lower, std::uint32_t upper, const std::size_t length,
											bool first_bit )
	{
		if ((extendable && first_bit) || constraint == VS_Asn::Unconstrained)
		{
			if (first_bit)
			{
				AddBit(1);
				return false;
			}
			return LengthEncode( 0, INT_MAX, length );
		}
		return LengthEncode( lower, upper, length );
	}
	// end VS_PerBuffer::ConstrainedLengthDecode

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool NullDecode( VS_AsnNull &null )
	{
		return (null.filled = true);
	}
	// end VS_PerBuffer::NullDecode

	inline bool NullEncode( const VS_AsnNull & )
	{
		return true;
	}
	// end VS_PerBuffer::NullEncode

	inline bool BooleanDecode( VS_AsnBoolean &boolean )		// 11.0
	{
		unsigned   val = 0;
		if (!GetBit( val ))		return false;
		boolean.value = (val != 0);
		return (boolean.filled = true);
	}
	// end VS_PerBuffer::BooleanDecode

	inline bool BooleanEncode( const VS_AsnBoolean &boolean )	// 11.0
 	{
		if(boolean.filled)
		{
			AddBit(static_cast<std::uint32_t>(boolean.value));
			return true;
		}
		return false;
 	}
	// end VS_PerBuffer::BooleanEncode

	inline bool IntegerDecode( VS_AsnInteger &integer )
	{
		switch (integer.constraint)
		{
		case VS_Asn::FixedConstraint :		break;	// 12.2.1 & 12.2.2
		case VS_Asn::ExtendableConstraint :
			std::uint32_t   bt;
			if (!GetBit( bt ))		return false;
			if (!bt)	break;		// 12.1
		// Fall into default case for unconstrained or partially constrained
			VS_FALLTHROUGH;
		default :	// 12.2.6
			std::size_t   ln = 0;
			if (!LengthDecode( 0, INT_MAX, ln ))	return false;
			ln *= 8;
			if (!GetBits(integer.value , ln))	return false;
			if (integer.IsUnsigned())
				integer.value += integer.lowerLimit;
			else if ((integer.value & (1 << ( ln - 1 ))) != 0)	// Negative
				integer.value |= UINT_MAX << ln;				// Sign extend
			return (integer.filled = true);
		}
		if (static_cast<std::uint32_t>(integer.lowerLimit) != integer.upperLimit)	// 12.2.2
		{
			if (UnsignedDecode( integer.lowerLimit, integer.upperLimit, integer.value ))		// which devolves to 10.5
					return integer.filled = true;
			return false;
		}
		integer.value = integer.lowerLimit;		// 12.2.1
		return (integer.filled = true);
	}
	// end VS_PerBuffer::IntegerDecode

	inline bool IntegerEncode( const VS_AsnInteger &intt )
	{
		if (!intt.filled) return false;
		switch(intt.constraint)
		{
		case VS_Asn::FixedConstraint :		break;	// 12.2.1 & 12.2.2
		case VS_Asn::ExtendableConstraint :
			AddBit(unsigned(intt.value > intt.upperLimit));	/*here must be extension marker */
			if(intt.value < intt.upperLimit) break;
			VS_FALLTHROUGH;
		default:
			{
			std::uint32_t nBits=0,bitsall=0;
				std::uint32_t vl = intt.value;
			if (intt.IsUnsigned())
				{
					vl-=intt.lowerLimit;
					nBits = CountBits(vl+1);
				}
			else if (static_cast<std::int32_t>(vl) > 0)
				{	nBits += CountBits(vl+1);
					bitsall = ((nBits+7)/8)*8;
					if ((vl & (1<<(bitsall-1)))) nBits+=1;//My AdOn Alex(Это фиксит баг OpenH323 )
				}	else nBits += CountBits(-static_cast<std::int32_t>(vl)+1);

			const std::size_t bytes = (nBits+7)/8;
			if (!LengthEncode(0,INT_MAX, bytes)) return false;
			if (!AddBits(vl,bytes*8)) return false;
			return true;
			}
		}
		//unsigned vl =  intt.value - intt.lowerLimit;
		if(static_cast<std::uint32_t>(intt.lowerLimit) != intt.upperLimit)
			return UnsignedEncode(intt.lowerLimit,intt.upperLimit,intt.value);
		return true;//иначе ничего не кодируем!!! /12
	}
	// end VS_PerBuffer::IntegerEncode
/*
	inline bool EnumerationDecode( VS_AsnEnumeration & )
	{
	}
	// end VS_PerBuffer::EnumerationDecode

	inline bool EnumerationEncode( const VS_AsnEnumeration & )
	{
	}
	// end VS_PerBuffer::EnumerationEncode
*/
	inline bool RealDecode( VS_AsnReal & )
	{
		return false;
	}
	// end VS_PerBuffer::RealDecode

	inline bool RealEncode( const VS_AsnReal & )
	{
		return false;
	}
	// end VS_PerBuffer::RealEncode

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool ObjectIdDecode( VS_AsnObjectId &ob_id )
	{
		ob_id.Clear();
		ob_id.filled = true;
		///x0691.23 - doc "длинна "на практике" будет помещяться в 1 октет
		std::size_t intLength =0;
		///255 - потому, что "длинна "на практике" будет помещяться в 1 октет" (х0691.23)
		if (!LengthDecode(0,255,intLength)) return false;//
		ob_id.size = 1;
		if (intLength==0) return true;
		ByteAlign();
		std::uint32_t subId=0;
		std::size_t i = 1;
		ob_id.value[0]=0;
		unsigned char byte;
		while (intLength > 0) {
			subId = 0;
			do {    /// shift and add in low order 7 bits
				std::uint32_t a = 0;
				if(!GetBits(a,8)) return  false;
				byte = static_cast<unsigned char>(a);
				subId = (subId << 7) + (byte & 0x7f);
				intLength--;
			} while ((byte & 0x80) != 0);
			if (i < (ob_id.NValues() - 1))  {	ob_id.value[i++]=subId;		ob_id.size = i;		}
		}
		subId = ob_id.value[1];
		if (subId < 40) {
			ob_id.value[0] = 0;
			ob_id.value[1] = subId;
		}
		else if (subId < 80) {
			ob_id.value[0] = 1;
			ob_id.value[1] = subId-40;
		}
		else {
			ob_id.value[0] = 2;
			ob_id.value[1] = subId-80;
		}
		return (ob_id.filled = true);
	}
	// end VS_PerBuffer::ObjectIdDecode

	inline bool ObjectIdEncode( const VS_AsnObjectId &objId )
	{
		//return false; // не рабочий (небыло тестов)
		if (!objId.filled) return false;
		int length = 0;
		std::size_t i;

		std::uint32_t ID =0;
		std::uint32_t theBits =0;
		for(i = 1;i<objId.size;i++)
		{
			if(i==1)
			{
				ID = objId.value[0]*40+objId.value[1];
				theBits += (ID==0)	?	 7	 :	 7*((CountBits(ID)+6)/7);//чтобы кратно 7 было
			}
			else theBits += (objId.value[i]==0)	?	 7	 :	7*((CountBits(objId.value[i])+6)/7) ;//чтобы кратно 7 было
		}
		std::uint32_t byte=0;
		//
		length +=theBits/7;
		if (!LengthEncode( 0 , 255 , length ))	return false;
		if (length ==0) return true;
		ByteAlignSize();
		//length-=1;
		for(i=1;((i<objId.size) && (length >= 0));i++)
		{
			if(i==1)
			{
				theBits = (ID==0)	?	 7	 :	 7*((CountBits(ID)+6)/7);//чтобы кратно 7 было
			}
			else
			{	theBits = (objId.value[i]==0)	?	 7	 :	7*((CountBits(objId.value[i])+6)/7) ;//чтобы кратно 7 было
				ID = objId.value[i];
			}
			while( theBits > 0){
				theBits-=7;
				if(theBits)	byte = ((ID >> theBits ) & 0xff)| 0x80 ;
				else byte = (ID & 0xff) & 127;
				if (!AddBits( byte , 8 )) return false;
				length-=1;
		}	}
		return true;
	}
	// end VS_PerBuffer::ObjectIdEncode

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool BitStringDecode( VS_AsnBitString &bitString )	// 15
	{
		std::size_t bits=0;
		if(!ConstrainedLengthDecode( bitString.extendable, bitString.constraint,
			bitString.lowerLimit, bitString.upperLimit, bits ))	return false;

		if (!bits)	return (bitString.filled = true);   // 15.7
		if (bits > BitsLeft())		return false;
		if (bits <= 16)
		{///15.8
			if(!GetBits(bitString.value,bits)) return false;
			return (bitString.filled = true);
		}
		if ((bits >16) && (bits < 65535))
		{///15.9
			ByteAlign();
			if(!GetBits(bitString.value,bits)) return false;
			return (bitString.filled = true);
		}
		else return false;

		return (bitString.filled = true);
	}
	// end VS_PerBuffer::BitStringDecode

	inline bool BitStringEncode( const VS_AsnBitString &bitString )
	{
		if (!bitString.filled) return false;
		const auto bits = bitString.value.BitsLeft();
		if (!bits) return true;
		if(!ConstrainedLengthEncode( bitString.extendable, bitString.constraint,
			bitString.lowerLimit, bitString.upperLimit, bits, false ))	return false;

		if (bits <= 16)		// 15.8
		{
			return AddBits( const_cast<VS_BitBuffer &>(bitString.value), bits );
		}
		if ((bits>16) && (bits < 65535))	// 15.9
		{
			ByteAlignSize();	return AddBits( const_cast<VS_BitBuffer &>(bitString.value), bits );
		}
		return false;
	}
	// end VS_PerBuffer::BitStringEncode

	inline bool OctetStringDecode( VS_AsnOctetString &octet_str )
	{
	if (octet_str.upperLimit<=0) return false;
		if (static_cast<std::uint32_t>(octet_str.lowerLimit)==octet_str.upperLimit)
			{
				if(octet_str.upperLimit<=2) ///16.6
					{
						if(!GetBits( octet_str.value , octet_str.upperLimit*8 )) return false;
						return (octet_str.filled = true);
					}
				else
					{ if(octet_str.upperLimit<=65535)
						{//16.7
							ByteAlign();
							if(!GetBits( octet_str.value , octet_str.upperLimit*8 )) return false;
							return (octet_str.filled = true);
						}
					else{
							return false; ///not support yet.16.8
						}
					}
			}
		else
			{
				std::size_t length=0;
				if (!ConstrainedLengthDecode(octet_str.extendable,
					octet_str.constraint,octet_str.lowerLimit,octet_str.upperLimit,length)) return false;
				ByteAlign();//length*=8;
				if (!GetBits(octet_str.value,length*8)) return false;
				return (octet_str.filled = true);

			}
		return false;
	}
	// end VS_PerBuffer::OctetStringDecode

	inline bool OctetStringEncode( const VS_AsnOctetString &octet_str )
	{
		if (!octet_str.filled) return false;
		if (static_cast<std::uint32_t>(octet_str.lowerLimit)==octet_str.upperLimit)
		{
			if(octet_str.upperLimit<=2) ///16.6
			{
				if(!AddBits( (VS_BitBuffer &)octet_str.value , octet_str.upperLimit*8 )) return false;
				return true;
			}
			else
			{ if(octet_str.upperLimit<=65535)
				{//16.7
					ByteAlignSize();
					if(!AddBits( (VS_BitBuffer &)octet_str.value , octet_str.upperLimit*8 )) return false;
					return  true;
				}
			else{
					return false; ///not support yet.16.8
				}
		}   }
		else
		{
			if (!ConstrainedLengthEncode(octet_str.extendable,
										 octet_str.constraint,
										 octet_str.lowerLimit,
										 octet_str.upperLimit,
										 octet_str.value.ByteSize(),
										  false )) return false;
			ByteAlignSize();
			if (!AddBits( (VS_BitBuffer &)octet_str.value , octet_str.value.ByteSize()*8 )) return false;
			return true;
		}

		return false;//метод не оттестированн
	}
	// end VS_PerBuffer::

	inline bool BmpStringDecode( VS_AsnBmpString & bmpString)
	{
		 const auto charSetUnalignedBits = 16;
		 const auto  charSetAlignedBits = 16;
		 std::size_t nBits=0;
		 std::size_t length=0;
		 if (!ConstrainedLengthDecode(bmpString.extendable,bmpString.constraint,
			 bmpString.lowerLimit,bmpString.upperLimit,length))   return false;
		 nBits = (aligned) ? charSetAlignedBits : charSetUnalignedBits;
		 if ((bmpString.constraint == VS_Asn::Unconstrained || bmpString.upperLimit*nBits > 16) && (aligned))
			ByteAlign();
		 for(decltype(length) i=0;i<length;i++)
		 {
			if(!GetBits(bmpString.value,nBits)) return false;
		 }
		return (bmpString.filled = true);
	}
	// end VS_PerBuffer::BmpStringDecode

	inline bool BmpStringEncode( VS_AsnBmpString &string )
	{
		if (!string.filled) return false;
	    const auto charSetUnalignedBits = 16;
		const auto charSetAlignedBits = 16;
		std::size_t length = 0;
		length =( aligned ) ? (string.value.BitsLeft()+7)/charSetAlignedBits
			:	(string.value.BitsLeft()+7)/charSetUnalignedBits;

		std::size_t nBits=0;
		 //
		 if (!ConstrainedLengthEncode( string.extendable , string.constraint ,
			 string.lowerLimit , string.upperLimit , length , false ))   return false;
		 //
		 nBits = (aligned) ? charSetAlignedBits : charSetUnalignedBits;
		 if ((string.constraint == VS_Asn::Unconstrained || string.upperLimit*nBits > 16) && (aligned))
			ByteAlignSize();
		 for (decltype(length) i=0;i<length;i++)
		 	if(!AddBits( string.value , nBits )) return false;
		  return true;
		//return false; //метод не оттестированн
	}
	// end VS_PerBuffer::BmpStringEncode

	/////////////////////////////////////////////////////////////////////////////////////
	inline bool ChoiceMissExtensionObject(  VS_AsnChoice &choice )	//22 , 10.2
	{	std::size_t len=0;
		if (!choice.extendable) return false;
		if (!LengthDecode(0,INT_MAX,len)) return false;
		if (!IncreaseIndex(len*8)) return false;
		return true;
	}
	// end  VS_PerBuffer::ChoiceMissExtensionObject( void )

	inline bool ChoiceDecode( VS_AsnChoice &choice )	// 22
	{
		choice.FreeChoice();
		if (choice.extendable)
		{
			std::uint32_t   bt;
			if (!GetBit( bt ))		return false;
			if (bt)
			{//extension marker is presetn 22.5
				unsigned tag=0;
				if (!SmallUnsignedDecode(tag)) return false;
				choice.tag = tag + choice.num_choices;
				return true;
		}	}
		if (choice.num_choices < 2)		choice.tag = 0;//22.4
		else if (!UnsignedDecode( 0, choice.num_choices - 1, choice.tag ))	return false;
		return true;
	}
	// end VS_PerBuffer::ChoiceDecode

	inline bool ChoiceEncode( const VS_AsnChoice &choice )
	{
		if (choice.extendable)
		{
			if ((choice.tag >= choice.num_choices) && (choice.tag < choice.num_ext_choices))
			{
				AddBit(1);
				const auto val = choice.tag - choice.num_choices;
				return SmallUnsignedEncode(val);
			}
			AddBit(0);
		}
		if (choice.num_choices < 2)
		{
			return choice.tag == 0;
		}
		return UnsignedEncode(0, choice.num_choices - 1, choice.tag);
	}
	// end VS_PerBuffer::ChoiceDecode

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool SequencePreambleDecode( VS_AsnSequence &sequence )// 18
	{
		if (sequence.extendable)
		{
			std::uint32_t   bt;
			if (!GetBit( bt ))		return false;
			sequence.totalExtensions = bt ? -1 : 0;		// 18.1
		} else		sequence.totalExtensions = 0;
		return BitStringDecode( sequence.optionMap );	// 18.2
	}
	// end VS_PerBuffer::SequencePreambleDecode
	inline bool SequencePreambleEncode( const VS_AsnSequence &sequence )
	{
		if (sequence.extendable)
		{
			AddBit(unsigned(sequence.extensionMap.filled));
		}
		if (sequence.optionMap.upperLimit)
			return BitStringEncode( sequence.optionMap );
		return true;
	}
	// end VS_PerBuffer::SequencePreambleEncode
	inline bool SequenceExtensionMapEncode( VS_AsnBitString &string)
	{
		const uint32_t len=string.upperLimit;
		if (!string.filled || (len == 0)) return false;
		if (!SmallUnsignedEncode( len - 1 )) return false;
		if (!AddBits( string.value, len )) return false;
		return true;
	}
	// end VS_PerBuffer::SequenceExtensionMapEncode
	inline bool SequenceExtensionMapDecode(VS_AsnBitString& string)
	{	std::uint32_t len=0;
		if (!SmallUnsignedDecode( len )) return false;
		len++;
		if (!GetBits( string.value, len)) return false;
		return (string.filled = true);
	}
	// end VS_PerBuffer::SequenceExtensionMapDecode

	inline bool SequenceIsKnowExtensionExistEncode(VS_AsnSequence& seq)
	{	if (seq.extendable)
		{	//if ((seq.extension_num>0) && (!seq.FillExtensionMap())) return false;
			if ((seq.extension_num == 0) || (!seq.extensionMap.filled)) return false;
			seq.totalExtensions	= seq.extensionMap.upperLimit;
			if(!SequenceExtensionMapEncode( seq.extensionMap )) return false;
			seq.extensionMap.value.ResetIndex();
			return true;
		}
		return false;
	}
	// end VS_PerBuffer::SequenceIsKnowExtensionExistEncode

	inline bool SequenceIsKnowExtensionExistDecode( VS_AsnSequence &seq)
	{
		if (seq.totalExtensions < 0 ) //расширеный маркер присутствует и мы читаем первое поле
		{
			if (!SequenceExtensionMapDecode(seq.extensionMap)) return false;
			seq.totalExtensions = seq.extensionMap.value.BitsLeft();
		}
		if (seq.totalExtensions == 0) //расширенный маркер отсутствует
			return false;
		return true;
	}
	// end VS_PerBuffer::SequenceIsKnowExtensionExistDecode




	inline bool SequenceIsKnowExtensionExist( VS_AsnSequence &seq)
	{
		if (seq.totalExtensions < 0 ) //расширеный маркер присутствует и мы читаем первое поле
		{
			if (!SequenceExtensionMapDecode(seq.extensionMap)) return false;
			seq.totalExtensions = seq.extensionMap.value.BitsLeft();
		}
		if (seq.totalExtensions == 0) //расширенный маркер отсутствует
			return false;
		return true;
	}
	// end VS_PerBuffer::SequenceIsKnowExtensionExist

	inline bool SequenceKnowExtensionDecode(VS_AsnSequence &seq ,VS_Asn  & field)
	{
		if (seq.totalExtensions<0) return false; ///не должно быть
		if (seq.totalExtensions==0) return true;
		else
		{
			std::uint32_t val=0;
			seq.totalExtensions--;
			if (!seq.extensionMap.value.GetBit( val )) return false;
			if (val)
			{
	/*			unsigned len=0;
				if(!LengthDecode( 0, INT_MAX , len )) return false;
				if (len*8 > BitsLeft())	  return false;
				if (!field.Decode( *this )) return false;
*/
				std::size_t len = 0;
				if (!LengthDecode(0,INT_MAX,len)) return false;
				if (len*8 > BitsLeft())	  return false;
				std::size_t byte=0,bit=0;
				GetPositionIndex(byte,bit);
				byte+=len;
				if (!field.Decode( *this ))	return false;
				if (!SetPositionIndex(byte,bit)) return false;
			}
			return true;
		}
		return false;
	}
	// end VS_PerBuffer::KnowExtensionDecode
	inline bool SequenceKnownExtensionEncode( VS_AsnSequence &seq, VS_Asn & field)
	{	if (seq.totalExtensions<0) return false; ///не должно быть
		if (seq.totalExtensions==0) return true;
		else
		{
			std::uint32_t val=0;
			seq.totalExtensions--;
			if (!seq.extensionMap.value.GetBit( val )) return false;
			if (val)
			{	VS_PerBuffer length_b;
				if (!field.Encode( length_b )) return false;
				std::size_t byte=0,bit=0,len=0,nbits=0;
				length_b.GetPositionSize(byte,bit);
				if (bit==8)
				{	if(byte==0)
					{	len = 1;	}
					else
					{	len = byte;	nbits= byte*8;	}
				}
				else {	len = byte+1;	nbits = byte*8+(8 - bit);		}
				if (!LengthEncode( 0 , INT_MAX , len )) return false;
				GetPositionSize(byte,bit);
				byte+=len;
				if (!AddBits( length_b , nbits )) return false;
				if (!SetPositionSize( byte , bit ))  return false;	/*
				VS_PerBuffer temp;
				if (!field.Encode( temp )) return false;
				unsigned len = temp.ByteSize();
				temp.ByteAlignSize();
				if (!AddBits(temp,len*8)) return false;*/
			}
			return true;
		}
		return false;
	}
	// end VS_PerBuffer::SequenceKnownEncode



	inline bool SequenceMissExtensionObjects( VS_AsnSequence &seq, const std::uint32_t num = ~0 )
	{
		if (seq.totalExtensions <0) return false; ///не должно быть
		const auto number_of_miss = (num >static_cast<std::uint32_t>(seq.totalExtensions) ) ? static_cast<std::uint32_t>(seq.totalExtensions) : num;
		for (std::remove_const<decltype(number_of_miss)>::type i = 0; i < number_of_miss; ++i)
			if (!SequenceMissExtentionObject( seq ))	return false;
		return true;
	}
	// end VS_PerBuffer::SequenceMissExtentionObject

	inline bool SequenceMissExtension( VS_AsnSequence &seq)
	{	if (SequenceIsKnowExtensionExist( seq ))//расширения присутствуют
		{	if(!SequenceMissExtensionObjects( seq ))	return false;
			return (seq.filled = true);
		}
		else return true;
	}
	// end VS_PerBuffer::SequenceMissExtentionObject

	inline bool SequenceMissExtentionObject(VS_AsnSequence &seq )
	{
		if (seq.totalExtensions <0) return false; ///не должно быть
		if (seq.totalExtensions==0) return true;
		else
		{
			std::uint32_t val=0;
			seq.totalExtensions--;
			if (!seq.extensionMap.value.GetBit( val )) return false;
			if (val)
			{	std::size_t len=0;
			if(!LengthDecode( 0, INT_MAX , len )) return false;
			return IncreaseIndex(len*8);
			}
			return true;
		}
	}
	// end VS_PerBuffer::SequenceMissExtentionObject

	inline bool SequenceUnknownDecode( VS_AsnSequence & )
	{
		return false;
	}
	// end VS_PerBuffer::SequenceUnknownDecode

	inline bool SequenceUnknownEncode( const VS_AsnSequence & )
	{
		return false;
	}
	// end VS_PerBuffer::SequenceUnknownEncode

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool IA5StringDecode( VS_AsnIA5String &ia5String )
	{
		std::size_t length=0;
		if (!ConstrainedLengthDecode( ia5String.extendable,
									  ia5String.constraint,
									  ia5String.lowerLimit,
									  ia5String.upperLimit, length)) return false;
		if (length==0) return true;
		const auto nBits = (aligned) ? ia5String.charSetAlignedBits : ia5String.charSetUnalignedBits;
		const auto totalBits = ia5String.upperLimit*nBits;
		if (ia5String.constraint == VS_Asn::Unconstrained ||
            (ia5String.lowerLimit == static_cast<std::int32_t>(ia5String.upperLimit) ? (totalBits > 16) : (totalBits >= 16)))
		{
			if (aligned)
				ByteAlign();
		}
		if (!GetBits(ia5String.value , length*nBits )) return false;

		return ia5String.filled = true;
	}
	// end VS_PerBuffer::IA5StringDecode

	inline bool IA5StringEncode( VS_AsnIA5String &ia5String )
	{
		if (!ia5String.filled) return false;
		const auto nBits = (aligned) ? ia5String.charSetAlignedBits : ia5String.charSetUnalignedBits;
		const auto length = ia5String.value.BitsLeft()/nBits;
		std::uint32_t totalBits = ia5String.upperLimit*nBits;

		if (!ConstrainedLengthEncode( ia5String.extendable,
									  ia5String.constraint,
									  ia5String.lowerLimit,
									  ia5String.upperLimit, length , false)) return false;
		if (length==0) return true;
		if (ia5String.constraint == VS_Asn::Unconstrained ||
            (ia5String.lowerLimit == static_cast<std::int32_t>(ia5String.upperLimit) ? (totalBits > 16) : (totalBits >= 16)))
		{
			if (aligned)
				ByteAlignSize();
		}
		for(std::remove_const<decltype(length)>::type i=0;i<length;i++)
		{
			if (!ia5String.value.GetBits(totalBits, ia5String.charSetAlignedBits)) return false;
			if (!AddBits( totalBits  , nBits )) return false;
		}

		return true;
	}
	// end VS_PerBuffer::IA5StringEncode

	/////////////////////////////////////////////////////////////////////////////////////


	inline bool RestrictedStringDecode( VS_AsnRestrictedString &restString )
	{
		std::size_t length=0;
		if (!ConstrainedLengthDecode( restString.extendable,
									  restString.constraint,
									  restString.lowerLimit,
									  restString.upperLimit, length)) return false;
		if (length==0) return true;
		const auto nBits = (aligned) ? restString.charSetAlignedBits : restString.charSetUnalignedBits;
		const auto totalBits = restString.upperLimit*nBits;
		if (restString.constraint == VS_Asn::Unconstrained ||
            (restString.lowerLimit == static_cast<std::int32_t>(restString.upperLimit) ? (totalBits > 16) : (totalBits >= 16)))
		{
			if (aligned)
				ByteAlign();
		}
		if (!GetBits( restString.value , length*nBits )) return false;

		return restString.filled = true;
	}
	// end VS_PerBuffer::RestrictedStringDecode

	inline bool RestrictedStringEncode( VS_AsnRestrictedString &restString )
	{
		if (!restString.filled) return false;
		const auto nBits = (aligned) ? restString.charSetAlignedBits : restString.charSetUnalignedBits;
		const auto length = restString.value.BitsLeft()/nBits;
		std::uint32_t totalBits = restString.upperLimit*nBits;

		if (!ConstrainedLengthEncode( restString.extendable,
									  restString.constraint,
									  restString.lowerLimit,
									  restString.upperLimit, length , false)) return false;
		if (length==0) return true;
		if (restString.constraint == VS_Asn::Unconstrained ||
            (restString.lowerLimit == static_cast<std::int32_t>(restString.upperLimit) ? (totalBits > 16) : (totalBits >= 16)))
		{
			if (aligned)
				ByteAlignSize();
		}
		const auto pie = restString.charSetAlignedBits;
		for(std::remove_const<decltype(length)>::type i=0;i<length;i++)
		{
			if (!restString.value.GetBits( totalBits, pie )) return false;
			if (!AddBits( totalBits  , nBits )) return false;
		}

		return true;
	}
	// end VS_PerBuffer::RestrictedStringEncode
	/////////////////////////////////////////////////////////////////////////////////////
	inline bool EnumerationDecode(VS_AsnEnumeration &enm)
	{
		if (enm.extendable) {  // 13.3
			std::uint32_t val = 0;
			if (GetBit(val) && (val)) {
				val = 0;
				if (SmallUnsignedDecode(val) &&
					(val > 0) &&
					UnsignedDecode(0, val - 1, enm.value)) return (enm.filled = true);
				return false;
			}
		}

		if (!UnsignedDecode(0, enm.maxEnumValue, enm.value)) return false;  // 13.2
		return (enm.filled = true);
	}
	/////////////////////////////////////////////////////////////////////////////////////
	inline bool EnumerationEncode(VS_AsnEnumeration &enm)
	{	if (!enm.filled) return false;
		if (enm.extendable) {  // 13.3
			const bool extended = enm.value > enm.maxEnumValue;
			AddBit(extended);
			if (extended) {
				if (!SmallUnsignedEncode(1+enm.value)) return false;
				if (!UnsignedEncode( 0, enm.value,enm.value)) return false;
				return true;
				}
			}

		return UnsignedEncode(0, enm.maxEnumValue,enm.value);  // 13.2
	}

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_PerBuffer struct

/////////////////////////////////////////////////////////////////////////////////////////
