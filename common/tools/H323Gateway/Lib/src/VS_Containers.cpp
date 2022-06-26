
#include "VS_AsnBuffers.h"
#include <memory>
/////////////////////////////////////////////////////////////////////////////////////////

bool VS_AsnBoolean::Decode( VS_PerBuffer &buffer )
{
	return buffer.BooleanDecode( *this );
}
// end of VS_AsnBoolean::Decode

bool VS_AsnBoolean::Encode( VS_PerBuffer &buffer )
{
	return buffer.BooleanEncode( *this );
}
// end of VS_AsnBoolean::Encode

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_ConstrainedArrayAsn::DecodeLength( VS_PerBuffer &buffer )
{
	length_ = 0;
	return buffer.ConstrainedLengthDecode(extendable, constraint, lowerLimit, upperLimit, length_);
}
// end of VS_ConstrainedArrayAsn::DecodeLength

bool VS_ConstrainedArrayAsn::EncodeLength( VS_PerBuffer &buffer )
{
//	length = 0;
	if (!filled)	return false;
	if (!buffer.ConstrainedLengthEncode( extendable, constraint, lowerLimit, upperLimit, length_ , false ))
		return false;
	return true;
}
// end of VS_ConstrainedArrayAsn::EncodeLength

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_AsnSequence::FillExtensionMap( void )
{
	auto lim =extensionMap.upperLimit;
	extensionMap.value.FreeMemory();
	extensionMap.filled = false;
	if (lim > 0)
	{
		bool IsExtensionPresents = false;
		for (decltype(lim) i = 0; i < lim; i++)
		{
			if (extension_ref[i].reference)//option exist
			{
				IsExtensionPresents |= extension_ref[i].reference->filled;
				extensionMap.value.AddBit(extension_ref[i].reference->filled);
			}
		}
		extensionMap.value.ResetIndex();
		if(IsExtensionPresents)
			return (extensionMap.filled = true);
		else {	extensionMap.value.FreeMemory();	return false;	}
	}
	return lim == 0;//(extensionMap.filled = true);
};
bool VS_AsnSequence::FillOptionMap( void )
{	auto lim =optionMap.upperLimit;
	optionMap.value.FreeMemory();
	optionMap.filled = false;
	if (lim > 0)
	{
		std::size_t l = 0;
		for(decltype(basic_num) i=0;i<basic_num;i++)
		{	if(basic_ref[i].flag)//option exist
			{
				if ((l++) >= lim) return false;
				optionMap.value.AddBit(basic_ref[i].reference->filled);
		}	}
		return (optionMap.filled = true);
	}
	return lim == 0;//(optionMap.filled = true);
}
//	end VS_AsnSequence::FillOptionMap

bool VS_AsnSequence::PreambleDecode( VS_PerBuffer &buffer )
{
	return buffer.SequencePreambleDecode( *this );
}
// end of VS_AsnSequence::Decode

bool VS_AsnSequence::PreambleEncode( VS_PerBuffer &buffer )
{
	if (!filled)									return false;
	if (!FillOptionMap())							return false;
	FillExtensionMap();
	if (!buffer.SequencePreambleEncode( *this ))	return false;
	optionMap.value.ResetIndex();
	return true;
}
// end of VS_AsnSequence::Encode

bool VS_AsnSequence::Decode( VS_PerBuffer &buffer )
{	std::uint32_t val =0;
	if (!buffer.SequencePreambleDecode( *this ))	return false;
	////////////////////////////////////////////////////////////////////
//	if (basic_num==0) return false;
	for (decltype(basic_num) i=0;i<basic_num;i++)
	{	if (basic_ref[i].flag) // is option field ?
		{
			if (!optionMap.value.GetBit(val))	return false;
			if (val && !basic_ref[i].reference->Decode( buffer ))	return false;
		}
		else if (!basic_ref[i].reference->Decode( buffer )) return false;
	}
	////////////////////////////////////////////////////////////////////
	if (buffer.SequenceIsKnowExtensionExistDecode( *this ))
	{
		for(decltype(extension_num) i=0;i<extension_num;i++)
		{
			if(!buffer.SequenceKnowExtensionDecode( *this, *((extension_ref[i].reference)))) return false;
		}
		if(!buffer.SequenceMissExtension( *this )) return false;
	}
	////////////////////////////////////////////////////////////////////
	return (filled = true);
}
// end VS_AsnSequence::Decode

bool VS_AsnSequence::Encode( VS_PerBuffer &buffer )
{
	std::uint32_t val = 0;
	if (!PreambleEncode( buffer ))	return false;
	for (decltype(basic_num) i=0;i<basic_num;i++)
	{
		if (basic_ref[i].flag) // is option field ?
		{
			if (!optionMap.value.GetBit(val))	return false;
			if (val && !basic_ref[i].reference->Encode( buffer ))	return false;
		}
		else if (!basic_ref[i].reference->Encode( buffer ))		return false;
	}
	////////////////////////////////////////////////////////////////////
	if (buffer.SequenceIsKnowExtensionExistEncode( *this ))
	{	if(extension_num)
		{	for(std::size_t i=0;i<extension_num;i++)
			{
				if(!buffer.SequenceKnownExtensionEncode( *this, *(extension_ref[i].reference)) ) return false;
	}	}	}
	////////////////////////////////////////////////////////////////////
//	return false;
return true;
}
// end VS_AsnSequence::Encode

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_AsnChoice::Encode( VS_PerBuffer &buffer )
{
	if (!choice || tag > num_ext_choices || !filled )		return false;
	if (!buffer.ChoiceEncode( *this ))		return false;
	if (tag >=num_choices)
	{	if (!choice->filled) return false;
		VS_PerBuffer length_b;
		if (!choice->Encode( length_b )) return false;
		std::size_t byte=0,bit=0,len=0,nbits=0;
		length_b.GetPositionSize(byte,bit);
		if (bit==8)
		{	if(byte==0)
			{	len = 1;	}
			else
			{	len = byte;	nbits= byte*8;	}
		}
		else {	len = byte+1;	nbits = byte*8+(8-bit);		}
		if (!buffer.LengthEncode( 0 , INT_MAX , len )) return false;
		buffer.GetPositionSize(byte,bit);
		byte+=len;
		if (!buffer.AddBits( length_b , nbits )) return false;
		if (!buffer.SetPositionSize( byte , bit ))  return false;
		return true;
	}
	return choice->filled ? choice->Encode( buffer ) : false ;
}
//end VS_AsnChoice::Encode
bool VS_AsnChoice::DecodeChoice( VS_PerBuffer &buffer, VS_Asn *choice )
{
	assert(choice);

	std::unique_ptr<VS_Asn> smart_choice{ choice };

	if (tag >=num_choices)
	{	std::size_t len = 0;
		if (!buffer.LengthDecode(0,INT_MAX,len)) return false;
		if (len*8 > buffer.BitsLeft())	  return false;
		std::size_t byte=0,bit=0;
		buffer.GetPositionIndex(byte,bit);
		byte+=len;
		if ((filled = choice->Decode( buffer )))
		{
			VS_AsnChoice::choice = smart_choice.release();
		}
		else
		{
			return false;
		}
		return buffer.SetPositionIndex(byte, bit);
		//if (!buffer.IncreaseIndex(len*8)) return false;
		//VS_AsnChoice::choice = choice;		return (filled = true);
	}
	if (!choice->Decode( buffer ))
	{
		return false;
	}
	VS_AsnChoice::choice = smart_choice.release();
	return filled = true;
}
// end of VS_AsnChoice::DecodeChoice

/////////////////////////////////////////////////////////////////////////////////////////


// end of VS_AsnRestrictedString::MakeIso646Alphabet

bool VS_AsnRestrictedString::MakeInverseCodeTable(  unsigned char * inverse_tab , const unsigned char * alphabet, std::size_t size)
{
	//if (flag) return false;
	if (size > max_alph_size) return false;
	const unsigned char * pos= nullptr;
	for(std::size_t i=0;i<max_table_size;i++)
	{
		pos =static_cast<const unsigned char *>(memchr(alphabet, i, size));
		if (pos) inverse_tab[i] = static_cast<unsigned char>(static_cast<const unsigned char *>(pos) - static_cast<const unsigned char *>(alphabet));
		else inverse_tab[i]=-1;
		pos = nullptr;
	}
	return true;
}
// end of VS_AsnRestrictedString::MakeInverseCodeTable

//unsigned char VS_AsnRestrictedString::iso646_alphabet[] = { 0 };
//unsigned char VS_AsnRestrictedString::iso646_inverse_table[max_table_size] = { 0 };
//const bool VS_AsnRestrictedString::flag = VS_AsnRestrictedString::MakeIso646Alphabet((unsigned char *)VS_AsnRestrictedString::iso646_alphabet)
//										&& VS_AsnRestrictedString::MakeInverseCodeTable((unsigned char *)VS_AsnRestrictedString::iso646_inverse_table, (unsigned char *)VS_AsnRestrictedString::iso646_alphabet, sizeof(VS_AsnRestrictedString::iso646_alphabet) );

VS_AsnRestrictedString & VS_AsnRestrictedString::operator = ( const VS_AsnRestrictedString &src)
{
	if (this != &src)
	{
		//alphabet_size = src.alphabet_size;
		charSetUnalignedBits = src.charSetUnalignedBits;
		charSetAlignedBits = src.charSetAlignedBits;
		alphabet = src.alphabet;
		inverse_table = src.inverse_table;
		upperLimit = src.upperLimit;
		lowerLimit = src.lowerLimit;
		constraint = src.constraint;
		value = src.value;
		filled = src.filled;
		alphabet_size = src.alphabet_size;
	}	return *this;
}
// end of VS_AsnRestrictedString::operator =

/////////////////////////////////////////////////////////////////////////////////////////
/*
bool VS_AsnPrintableString::MakeInverseCodeTable(  unsigned char * inverse_tab , unsigned char * alphabet,const unsigned size)
{
	if (size > max_printable_alph) return false;
	unsigned char * pos=NULL;
	for(unsigned i=0;i<max_table_size;i++)
	{
		pos =(unsigned char *) memchr(alphabet,i,size);
		if (pos) inverse_tab[i] = (unsigned char)((unsigned char *)pos - (unsigned char *)alphabet);
		else inverse_tab[i]=-1;
		pos = NULL;
	}
	return true;
}
// end of VS_AsnPrintableString::MakeInverseCodeTable
*/

/////////////////////////////////////////////////////////////////////////////////////////
constexpr unsigned char  VS_AsnIA5String::ALPHABETIC[];

unsigned char VS_AsnIA5String::inverse_t[max_table_size] = { 0 };
const bool VS_AsnIA5String::flg =
									VS_AsnIA5String
									::MakeInverseCodeTable(static_cast<unsigned char *>(VS_AsnIA5String::inverse_t)
									, VS_AsnIA5String::ALPHABETIC
									, sizeof(VS_AsnIA5String::ALPHABETIC) );
/////////////////////////////////////////////////////////////////////////////////////////
constexpr  unsigned char  VS_AsnPrintableString::ALPHABETIC[];

unsigned char  VS_AsnPrintableString::inverse_t[max_table_size] = { 0 };
const bool VS_AsnPrintableString::flg = VS_AsnPrintableString
									::MakeInverseCodeTable(static_cast<unsigned char *>(VS_AsnPrintableString::inverse_t)
									, VS_AsnPrintableString::ALPHABETIC
									, sizeof(VS_AsnPrintableString::ALPHABETIC) );

/////////////////////////////////////////////////////////////////////////////////////////
constexpr unsigned char   VS_AsnNumericString::ALPHABETIC[];
unsigned char   VS_AsnNumericString::inverse_t[max_table_size] = { 0 };
const bool VS_AsnNumericString::flg = VS_AsnNumericString
									::MakeInverseCodeTable(static_cast<unsigned char *>(VS_AsnNumericString::inverse_t)
									, VS_AsnNumericString::ALPHABETIC
									, sizeof(VS_AsnNumericString::ALPHABETIC) );
/////////////////////////////////////////////////////////////////////////////////////////
constexpr  unsigned char   VS_AsnGeneralString::ALPHABETIC[];
unsigned char   VS_AsnGeneralString::inverse_t[max_table_size] = { 0 };
const bool VS_AsnGeneralString::flg = VS_AsnGeneralString
									::MakeInverseCodeTable(static_cast<unsigned char *>(VS_AsnGeneralString::inverse_t)
									, VS_AsnGeneralString::ALPHABETIC
									, sizeof(VS_AsnGeneralString::ALPHABETIC) );
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnObjectId::Decode( VS_PerBuffer &buffer )
{	return buffer.ObjectIdDecode( *this );	}
// end of VS_AsnObjectId::Decode

bool VS_AsnObjectId::Encode( VS_PerBuffer &buffer )
{	return buffer.ObjectIdEncode( *this );	}
// end of VS_AsnObjectId::Encode

/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnBmpString::Decode( VS_PerBuffer &buffer)
{	return buffer.BmpStringDecode( *this );		}
// end VS_AsnBmpString::Decode
bool VS_AsnBmpString::Encode( VS_PerBuffer &buffer)
{	return buffer.BmpStringEncode( *this );		}
// end VS_AsnBmpString::Encode
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnGeneralString::Decode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringDecode( *this );		}
// end VS_AsnGeneralString::Decode
bool VS_AsnGeneralString::Encode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringEncode( *this );		}
// end VS_AsnGeneralString::Encode
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnPrintableString::Decode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringDecode( *this );		}
// end VS_AsnPrintableString::Decode
bool VS_AsnPrintableString::Encode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringEncode( *this );		}
// end VS_AsnPrintableString::Encode
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnIA5String::Decode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringDecode( *this );		}
// end VS_AsnIA5String::Decode
bool VS_AsnIA5String::Encode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringEncode( *this );		}
// end VS_AsnIA5String::Encode
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnNumericString::Decode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringDecode( *this );		}
// end VS_AsnNumericString::Decode
bool VS_AsnNumericString::Encode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringEncode( *this );		}
// end VS_AsnNumericString::Encode
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnRestrictedString::Decode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringDecode( *this );		}
// end VS_AsnRestrictedString::Decode
bool VS_AsnRestrictedString::Encode( VS_PerBuffer &buffer)
{	return buffer.RestrictedStringEncode( *this );		}
// end VS_AsnRestrictedString::Encode
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnOctetString::Decode( VS_PerBuffer &buffer)
{	return buffer.OctetStringDecode( *this );		}
// end VS_AsnOctetString::Decode
bool VS_AsnOctetString::Encode( VS_PerBuffer &buffer)
{	return buffer.OctetStringEncode( *this );		}
// end VS_AsnOctetString::Encode
bool VS_AsnOctetString::EncodeSubType( VS_Asn * obj )
{	value.FreeMemory();
	VS_PerBuffer buf;
	if (!obj->Encode( buf )) return false;
	value = buf;
	value.ByteAlignSize();
	value.ResetIndex();
	return true;
}
// end VS_AsnOctetString::EncodeSubType
bool VS_AsnOctetString::DecodeSubType( VS_Asn * obj )
{	value.ResetIndex();
	VS_PerBuffer buf;
	buf.CopyBuff( value );
	return obj->Decode(buf);
}
// end VS_AsnOctetString::DecodeSubType
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnInteger::Decode( VS_PerBuffer &buffer)
{	return buffer.IntegerDecode( *this );		}
// end VS_AsnInteger::Decode
bool VS_AsnInteger::Encode( VS_PerBuffer &buffer)
{	return buffer.IntegerEncode( *this );		}
// end VS_AsnInteger::Encode
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnBitString::Decode( VS_PerBuffer &buffer)
{	return buffer.BitStringDecode( *this );		}
// end VS_AsnBitString::Decode
bool VS_AsnBitString::Encode( VS_PerBuffer &buffer)
{	return buffer.BitStringEncode( *this );		}
// end VS_AsnBitString::Encode
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
bool VS_AsnEnumeration::Decode( VS_PerBuffer &buffer)
{	return buffer.EnumerationDecode( *this );		}
// end VS_AsnEnumeration::Decode
bool VS_AsnEnumeration::Encode( VS_PerBuffer &buffer)
{	return buffer.EnumerationEncode( *this );		}
// end VS_AsnEnumeration::Encode
/////////////////////////////////////////////////////////////////////////////////////////