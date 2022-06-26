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
/// \file VS_BaseBuffers.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory.h>
#include <stdlib.h>
#include <cstddef>
#include <cstdint>
#include <climits>
#include <assert.h>
#include <type_traits>

#define   O_C(name)   if(src.name.filled) name = src.name
#define   O_CC(name)  name = src.name
#define   O_T(name)   name == src.name
#define   O_CP(name)  if(name != nullptr && src.name != nullptr) *name = *(src.name)//for pointers
#define   O_CSA(name, counter) for (std::size_t i = 0; i < counter; ++i){ name[i] = src.name[i];}//for static arrays

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_ByteBuffer
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_ByteBuffer( void )
		: buffer(nullptr), size(0), byte_size(0), byte_index(0) {}
	// end VS_ByteBuffer::VS_ByteBuffer

	VS_ByteBuffer( const void *buffer, const std::size_t size )
		: buffer(nullptr), size(0), byte_size(0), byte_index(0)
	{	AddBytes( (unsigned char *)buffer, size );	}
	// end VS_ByteBuffer::VS_ByteBuffer

	VS_ByteBuffer( const VS_ByteBuffer& other)
		: buffer(nullptr), size(0), byte_size(0), byte_index(0)
	{
		if (other.byte_size)
		{
			AddBytes(other.buffer, other.byte_size);
		}
	}

	~VS_ByteBuffer( void ) {	FreeMemory();	}
	// end VS_ByteBuffer::~VS_ByteBuffer

protected:
	static const unsigned   size_rate = 32;
	unsigned char   *buffer;
	std::size_t   size, byte_size, byte_index;

	/////////////////////////////////////////////////////////////////////////////////////
	/*
	 * @throw std::bad_alloc
	 */
	inline void AddMemory( std::size_t n_bytes )
	{
		// (byte_size+1) плюс один потому, что VS_BitBuffer считает память как (byte_size + bit_size)
		// Поэтому мы должны выделять на байт больше, чтобы VS_BitBuffer не писал за пределы блока памяти
		// 09.03.2007 (c) ktrushnikov
		const std::size_t   new_size = (((byte_size + 1) + n_bytes + (size_rate - 1)) / size_rate) * size_rate;
		if (!new_size) { FreeMemory();	return; }

		if (new_size == size)	return;
		buffer = static_cast<unsigned char *>(realloc(buffer, new_size * sizeof(unsigned char)));
		//crash if bad realloc: buffer == nullptr
		memset((buffer + size), 0, new_size - size);

		size = new_size;
	}
	// end VS_ByteBuffer::AddMemory

	inline void ResetMemory(void)
	{
		buffer = nullptr;
		size = 0;
		byte_size = 0;
		byte_index = 0;
	}
	// end VS_ByteBuffer::ResetMemory

public:
	inline void FreeMemory( void )
	{
		free( buffer );
		ResetMemory();
	}
	// end VS_ByteBuffer::FreeMemory

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool IsFilled( void ) const {		return byte_size != 0;		}
	// end VS_ByteBuffer::IsFilled

	inline std::size_t ByteSize( void ) const {	return byte_size;	}
	// end VS_ByteBuffer::ByteSize

	inline unsigned ByteIndex( void ) const {	return byte_index;	}
	// end VS_ByteBuffer::ByteIndex

	inline void *GetData() const {		return buffer;		}
	// end VS_BitBuffer::GetData

	inline void ResetIndex( void ) {	byte_index = 0;		}
	// end VS_ByteBuffer::ResetIndex

	/////////////////////////////////////////////////////////////////////////////////////

	inline void AddByte( const unsigned char value )
	{
		AddMemory(1);
		buffer[byte_size] = value;	++byte_size;
	}
	// end VS_ByteBuffer::AddByte

	inline void AddBytes( const unsigned char value[], std::size_t n_bytes )
	{
		AddMemory(n_bytes);
		memcpy( &buffer[byte_size], value, n_bytes );
		byte_size += n_bytes;
	}
	// end VS_ByteBuffer::AddByte

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool GetByte( unsigned char &value )
	{
		if (byte_index >= byte_size)		return false;
		value = buffer[byte_index];		++byte_index;		return true;
	}
	// end VS_ByteBuffer::GetByte

	inline bool GetBytes( unsigned char value[], std::size_t n_bytes )
	{
		const auto  new_index = byte_index + n_bytes;
		if (new_index > byte_size)		return false;
		memcpy( value, &buffer[byte_index], n_bytes );
		byte_index = new_index;		return true;
	}
	// end VS_ByteBuffer::GetBytes

	/////////////////////////////////////////////////////////////////////////////////////

	VS_ByteBuffer & operator=(const VS_ByteBuffer &src)
	{
		if (this != &src)
		{
			FreeMemory();
			if (src.byte_size) AddBytes(src.buffer, src.byte_size);
		}
		return *this;
	}
	// end VS_ByteBuffer::operator=

	bool operator==( const VS_ByteBuffer &dst ) const
	{
		const auto  sz = ByteSize();
		return sz == dst.ByteSize()
				&& (!sz || !memcmp( buffer, dst.buffer, sz ));
	}
	// end VS_ByteBuffer::operator==

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_ByteBuffer struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_BitBuffer : protected VS_ByteBuffer
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_BitBuffer( void )
		: bit_size(8), bit_index(8) {}
	// end VS_BitBuffer::VS_BitBuffer

	VS_BitBuffer( const void *buffer, const std::size_t bits )
		: VS_ByteBuffer( buffer, ( bits + 7 ) / 8 )
		, bit_size(!( bits & 7 ) ? 8 : ( bits & 7 )), bit_index(8) {}
	// end VS_BitBuffer::VS_BitBuffer

	VS_BitBuffer(const VS_BitBuffer& src) :VS_ByteBuffer(src), bit_size(src.bit_size), bit_index(src.bit_index)
	{}
	~VS_BitBuffer( void ) {}
	// end VS_BitBuffer::~VS_BitBuffer

protected:
	// - bit_size - индекс, от 8 до 1, следующего заносимого бита в текущем добавляемом байте,
	// - bit_index - индекс, от 8 до 1, бита текущего читаемого бита.
	unsigned   bit_size, bit_index;

	/////////////////////////////////////////////////////////////////////////////////////

	inline void AddMemory( std::size_t n_bits )
	{
		if (bit_size != 8)
		{
			if (bit_size >= n_bits)	return;
			n_bits -= bit_size;
		}
		VS_ByteBuffer::AddMemory(( n_bits + 7 ) / 8 );
	}
	// end VS_BitBuffer::AddMemory

	inline void AddBitAct( const unsigned value )
	{
		buffer[byte_size] |= ( value & 1 ) << --bit_size;
		if (!bit_size) {	++byte_size;	bit_size = 8;	}
	}
	// end VS_BitBuffer::AddBitAct

	inline void AddBitsAct( const std::uint32_t value, std::size_t n_bits )
	{
		if (bit_size <= n_bits)
		{	n_bits -= bit_size;
			buffer[byte_size] |= value >> n_bits;
			++byte_size;	bit_size = 8;
			while (n_bits >= 8)
			{	n_bits -= 8;	buffer[byte_size++] = value >> n_bits;
		}	}
		if (n_bits)
		{	bit_size -= n_bits;
			buffer[byte_size] |= ( value & (( 1 << n_bits ) - 1 )) << bit_size;
	}	}
	// end VS_BitBuffer::AddBitsAct

	inline void AddBitsAct( const unsigned char value[], std::size_t n_bits )
	{
		std::uint32_t   ind = 0;
		while (n_bits > 8) {	AddBitsAct( static_cast<std::uint32_t>(value[ind++]), 8 );	n_bits -= 8;	}
		AddBitsAct( static_cast<std::uint32_t>(value[ind] >> (8 - n_bits)), n_bits );
	}
	// end VS_BitBuffer::AddBitsAct

	inline void AddBitsAct( VS_BitBuffer &value, std::size_t n_bits )
	{
		while (n_bits > ( sizeof(std::uint32_t) * 8 ))
		{	AddBitsAct( value.GetBitsAct( sizeof(std::uint32_t) * 8 ), ( sizeof(std::uint32_t) * 8 ));
			n_bits -= sizeof(std::uint32_t) * 8;		}
		AddBitsAct( value.GetBitsAct( n_bits ), n_bits );
	}
	// end VS_BitBuffer::AddBitsAct

	inline void AddZerosAct( std::size_t n_bits )
	{
		while (n_bits > ( sizeof(std::uint32_t) * 8 ))
		{	AddBitsAct( static_cast<std::uint32_t>(0), sizeof(std::uint32_t) * 8 );
			n_bits -= sizeof(std::size_t) * 8;		}
		AddBitsAct( static_cast<std::uint32_t>(0), n_bits );
	}
	// end VS_BitBuffer::AddZerosAct

	inline void AddOnesAct( std::size_t n_bits )
	{
		while (n_bits > ( sizeof(std::uint32_t) * 8 ))
		{	AddBitsAct( std::uint32_t(~0), sizeof(std::uint32_t) * 8 );
			n_bits -= sizeof(std::uint32_t) * 8;		}
		AddBitsAct( std::uint32_t(~0), n_bits );
	}
	// end VS_BitBuffer::AddOnesAct

	inline std::uint32_t GetBitAct( void )
	{
		const auto val = ( buffer[byte_index] >> --bit_index ) & 1;
		if (!bit_index) {	++byte_index;	bit_index = 8;	}
		return val;
	}
	// end VS_BitBuffer::GetBitAct

	inline std::uint32_t GetBitsAct( std::size_t n_bits )
	{
		std::uint32_t   val;
		if (n_bits < bit_index)
		{	bit_index -= n_bits;
			val = ( buffer[byte_index] >> bit_index ) & (( 1 << n_bits ) - 1 );
		} else if (n_bits == bit_index)
		{	val = buffer[byte_index] & (( 1 << n_bits ) - 1 );
			++byte_index;	bit_index = 8;
		} else
		{	val = buffer[byte_index] & (( 1 << bit_index ) - 1 );
			n_bits -= bit_index;	++byte_index;		bit_index = 8;
			while (n_bits >= 8)
			{	val = ( val << 8 ) | buffer[byte_index];
				n_bits -= 8;	++byte_index;	}
			if (n_bits)
			{	bit_index -= n_bits;
				val = ( val << n_bits ) | ( buffer[byte_index] >> bit_index );
		}	}
		return val;
	}
	// end VS_BitBuffer::GetBitsAct

	inline void GetBitsAct( unsigned char value[], std::size_t n_bits )
	{
		std::uint32_t   ind = 0;
		while (n_bits > 8) {	value[ind++] = GetBitsAct( 8 );		n_bits -= 8;	}
		value[ind] = GetBitsAct( n_bits ) << ( 8 - n_bits );
	}
	// end VS_BitBuffer::GetBitsAct

public:
	inline void ResetMemory( void )
	{	bit_size = bit_index = 8;	VS_ByteBuffer::ResetMemory();	}
	// end VS_BitBuffer::ResetMemory

	inline void FreeMemory( void )
	{	bit_size = bit_index = 8;	VS_ByteBuffer::FreeMemory();	}
	// end VS_BitBuffer::FreeMemory

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool IsFilled( void ) const {		return byte_size != 0 || bit_size != 8;		}
	// end VS_BitBuffer::IsFilled

	inline std::size_t ByteSize( void ) const
	{	return byte_size + static_cast<std::size_t>(bit_size != 8);	}
	// end VS_BitBuffer::ByteSize

	inline std::size_t ByteIndex( void ) const
	{	return byte_index + static_cast<std::size_t>(bit_index != 8);		}
	// end VS_BitBuffer::ByteIndex

	inline std::size_t BitSize( void ) const
	{	return ( byte_size * 8 ) + ( 8 - bit_size );	}
	// end VS_BitBuffer::BitSize

	inline void *GetData() const { 	return buffer; 	}
	// end VS_BitBuffer::GetData

	inline std::size_t BitsLeft( void ) const
	{
		return (( byte_size - byte_index ) * 8 ) + bit_index - bit_size;
	}
	// end VS_BitBuffer::BitsLeft

	inline void ResetIndex( void ) {	bit_index = 8;	VS_ByteBuffer::ResetIndex();	}
	// end VS_ByteBuffer::ResetIndex

	inline bool IncreaseIndex(  std::size_t bits )
	{
		if (BitsLeft() < bits)	return false;
		if (bit_index > bits)	bit_index -= bits;
		else if (bit_index == bits) {		++byte_index;	bit_index = 8;		}
		else {		bits -= bit_index;		const auto   bytes = bits / 8;
					byte_index += 1 + bytes;	bits -= bytes * 8;
					bit_index = 8 - bits;		}
		return true;
	}
    // end VS_BitBuffer::IncreaseIndex
	inline bool SetPositionIndex(std::size_t byte, unsigned bit)
	{	if (bit>8) return false;
    	if (byte*8+bit > byte_size*8+bit_size) return false;
		byte_index=byte;	bit_index = bit;
		return true;
	}
	// VS_BitBuffer::SetPositionIndex
	inline bool SetPositionSize(std::size_t byte, unsigned bit)
	{
		typedef std::make_signed<std::size_t>::type s_size_t;

		const auto byte_num = static_cast<s_size_t>(byte) - static_cast<s_size_t>(byte_size);
		const auto bit_num = static_cast<s_size_t>(bit_size) - static_cast<s_size_t>(bit) ;//обратная нумерация
		if ((bit_num==0) && (byte_num==0))	return true;
		if ((bit>8) || (byte_num<0 ) || ((byte_num==0)&&(bit_num<0))) return false;
		if (byte_num==0)
		{
			AddBits(std::uint32_t(0), bit_num);
		}
		AddZeros( byte_num*8 + bit_num );
		return true;
	}
	// VS_BitBuffer::SetPositionSize
	inline void GetPositionIndex(std::size_t & byte,std::size_t & bit) const
	{	byte = byte_index;
		bit = bit_index;
	}
	// VS_BitBuffer::GetPositionIndex
	inline void GetPositionSize(std::size_t &byte,std::size_t &bit) const
	{	byte = byte_size;
		bit = bit_size;
	}
	// VS_BitBuffer::GetPositionSize

	inline void ByteAlign( void )
	{	if (bit_index != 8) {	++byte_index;	bit_index = 8;	}}
	// end VS_BitBuffer::ByteAlign

	inline void ByteAlignSize( void )
	{	if (bit_size != 8) {	++byte_size;	bit_size = 8;	}}
	// end VS_BitBuffer::ByteAlignSize

	/////////////////////////////////////////////////////////////////////////////////////

	inline void AddBit( const unsigned value )
	{
		AddMemory(1);
		AddBitAct(value);
	}
	// end VS_BitBuffer::AddBit

	inline bool AddBits( const std::uint32_t value, std::size_t n_bits )
	{	if (n_bits==0) return true;
		if ( n_bits > ( sizeof(value) * 8 )) return false;
		AddMemory(n_bits);
		AddBitsAct( value, n_bits );	return true;
	}
	// end VS_BitBuffer::AddBits

	inline void AddBits( const unsigned char value[], std::size_t n_bits )
	{
		if (n_bits==0) return;
		AddMemory(n_bits);
		AddBitsAct( value, n_bits );
	}
	// end VS_BitBuffer::AddBits

	inline bool AddBits( VS_BitBuffer &value, std::size_t n_bits )
	{	if (n_bits==0) return true;
		if ( value.BitsLeft() < n_bits ) return false;
		AddMemory(n_bits);
		AddBitsAct( value, n_bits );	return true;
	}
	// end VS_BitBuffer::AddBits

	inline bool AddBits( VS_BitBuffer &value )
	{
		return AddBits( value, value.BitSize() );
	}
	// end VS_BitBuffer::AddBits

	inline void AddZeros( std::size_t n_bits )
	{
		if (n_bits == 0) return;
		AddMemory(n_bits);
		AddZerosAct(n_bits);
	}
	// end VS_BitBuffer::AddZeros

	inline void AddOnes( std::size_t n_bits )		// В смысле единиц (а не однажды).
	{	if (n_bits==0) return;
		AddMemory(n_bits);
		AddOnesAct(n_bits);
	}
	// end VS_BitBuffer::AddOnes

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool GetBit( std::uint32_t &value )
	{
		if (!BitsLeft())	return false;
		value = GetBitAct();	return true;
	}
	// end VS_BitBuffer::GetBit

	inline bool GetBits(std::uint32_t &value, std::size_t n_bits )
	{	if (n_bits==0) return true;
		if (n_bits > ( sizeof(value) * 8 ) || BitsLeft() < n_bits)	return false;
		value = GetBitsAct( n_bits );	return true;
	}
	// end VS_BitBuffer::GetBits

	inline bool GetBits( unsigned char value[], std::size_t n_bits )
	{	if (n_bits==0) return true;
		if (BitsLeft() < n_bits)	return false;
		GetBitsAct( value, n_bits );	return true;
	}
	// end VS_BitBuffer::GetBits

	inline bool GetBits( VS_BitBuffer &value, std::size_t n_bits )
	{
		return value.AddBits( *this, n_bits );
	}
	// end VS_BitBuffer::GetBits

	/////////////////////////////////////////////////////////////////////////////////////

	VS_BitBuffer & operator=( const VS_BitBuffer &src )
	{
		if (this != &src)
		{
			FreeMemory();
			const auto  src_bits = src.BitSize();
			if (src_bits) AddBits(src.buffer, src_bits);
		}
		return *this;
	}
	// end VS_BitBuffer::operator=
	void CopyBuffer( const VS_BitBuffer &src )
	{
		FreeMemory();
		const auto  src_bits = src.BitSize();
		if (src_bits)	AddBits(src.buffer, src_bits);
	}
	// end VS_BitBuffer::CopyBuffer
	bool operator==( const VS_BitBuffer &dst ) const
	{
		const auto   sz = BitSize();
		return sz == dst.BitSize()
				&& (!sz || !memcmp( buffer, dst.buffer, ByteSize() ));
	}
	// end VS_ByteBuffer::operator==

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_BitBuffer struct

/////////////////////////////////////////////////////////////////////////////////////////
