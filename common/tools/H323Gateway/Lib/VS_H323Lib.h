/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 10.10.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_H323Lib.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "net/Address.h"
#include "net/Port.h"

#include "src/VS_H225Messages.h"
#include "src/VS_CsMessages.h"
#include "src/VS_H245Messages.h"

#include <initializer_list>


typedef VS_H225TransportAddress_IpAddress	VS_H225IpAddress;
typedef VS_H225TransportAddress_Ip6Address	VS_H225Ipv6Address;
//typedef VS_H245UnicastAddress_IPAddress		VS_H245UniMulticastAddress_IPAddress;


void set_ip_address(VS_H225TransportAddress& obj_, const net::address& addr, net::port port);
bool get_ip_address(VS_H225TransportAddress& obj_, net::address& addr, net::port& port) noexcept;
void set_ip_address(VS_H245UnicastAddress& obj_, const net::address& addr, net::port port);
bool get_ip_address(VS_H245UnicastAddress& obj_, net::address& addr, net::port& port) noexcept;

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwAsnObjectId : public VS_AsnObjectId
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwAsnObjectId( const std::uint32_t value[], const std::size_t value_size )
	{	SetValue( value, value_size );	}

	explicit VS_GwAsnObjectId(std::initializer_list<std::uint32_t> value)
	{
		SetValue(value.begin(), value.size());
	}
	// end of VS_GwH225ProtocolIdentifier::VS_GwH225ProtocolIdentifier

	inline void SetValue( const std::uint32_t value[], const std::size_t value_size )
	{
		this->size = value_size > sizeof(this->value) / sizeof(*this->value)
						? sizeof(this->value) / sizeof(*this->value) : value_size;
		memcpy(this->value, static_cast<const void *>(value), this->size * sizeof(*this->value) );
		filled = true;
	}

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwAsnObjectId struct

struct VS_GwAsnBmpString : public VS_AsnBmpString
{
	/////////////////////////////////////////////////////////////////////////////////////

	char *GetValueAsChar( char *buffer, std::size_t *size = nullptr ) const
	{
		const char   *data = static_cast<char *>(value.GetData());
		const unsigned   data_size = value.ByteSize() / 2;
		if (size)
		{	if (data_size < *size)	goto go_nsize;
			std::size_t i = 0;
			for (; i < ( *size - 1 ); ++i)	buffer[i] = data[i * 2 + 1];
			buffer[i] = 0;
		} else
go_nsize:	for (std::size_t i = 0; i < data_size; ++i)	buffer[i] = data[i * 2 + 1];
		*size = data_size;
		return buffer;
	}
	// end of VS_GwAsnBmpString::GetValueAsChar

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwAsnBmpString struct




/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH225ArrayOf_AliasAddress : public Array_of_type<VS_H225AliasAddress>
{
	std::size_t current_index;
	/////////////////////////////////////////////////////////////////////////////////////

	inline bool GetFirstAliasAsChar( char alias[], const std::size_t size, int * flag = 0 )
	{
		current_index =0;
		return Get(alias,size,flag);
	}
	// end of VS_GwH225ArrayOf_AliasAddress::GetFirstAliasAsChar

	/////////////////////////////////////////////////////////////////////////////////////
	inline bool GetNextAliasAsChar( char alias[], const std::size_t size, int * flag = 0 )
	{
		current_index++;
		return Get(alias,size,flag);
	}
	// end of VS_GwH225ArrayOf_AliasAddress::GetFirstAliasAsChar

///////////////////////////////////////
// Added by ktrushnikov at 19.09.2006
//////////////////////////////////////
	bool GetAlias(char (&alias)[256], std::size_t &sz )
	{
		for (std::size_t i=0; i < size(); i++)
		{
			VS_H225AliasAddress &addr = (static_cast<VS_H225AliasAddress *>( data()))[i];
			if (addr.tag == VS_H225AliasAddress::e_h323_ID)
			{
				sz = sizeof(alias);
				VS_GwAsnBmpString* bmpStr = static_cast<VS_GwAsnBmpString*>( addr.choice);
				bmpStr->GetValueAsChar(alias, &sz);
				if ( (*alias) && (sz > 0) && (sz < sizeof(alias)) )
				{
					alias[sz] = 0;
					return true;
				}
			}
		}
		return false;
	}

	bool GetDigit( char (&digit)[256], std::size_t &sz )
	{
		for (std::size_t i=0; i < size(); i++)
		{
			VS_H225AliasAddress &addr = (static_cast<VS_H225AliasAddress *>(data()))[i];
			if (addr.tag == VS_H225AliasAddress::e_dialedDigits)
			{
				VS_AsnIA5String* ia5 = static_cast<VS_AsnIA5String*>(addr.choice);

				sz = sizeof(digit);
				if ( ia5->GetNormalString(digit, sz) && (sz > 1 && sizeof(digit) >= sz))		// skip null-string
				{
					sz--;					// skip "\0"
					return true;
				}
			}
		}
		return false;
	}

	bool GetUrl( char (&url)[512], std::size_t &sz )
	{
		for (std::size_t i=0; i < size(); i++)
		{
			VS_H225AliasAddress &addr = (static_cast<VS_H225AliasAddress *>(data()))[i];
			if (addr.tag == VS_H225AliasAddress::e_url_ID)
			{
				VS_AsnIA5String* ia5 = static_cast<VS_AsnIA5String*>(addr.choice);

				sz = sizeof(url);
				if ( ia5->GetNormalStringNoShift(url, sz) && (sz > 1 && sizeof(url) >= sz) )		// skip null-string
				{
					sz--;					// skip "\0"
					return true;
				}
			}
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////
protected:
	bool NormalizeAt(char alias[], const std::size_t size) const
	{
		char* at   = static_cast<char*>(memchr(alias, '@', size));
		if (at) return true;
		char* tilda = static_cast<char*>(memchr(alias, '_', size));
		if (tilda)
		{
			*tilda = '@';
			return true;
		}
		return false;
	}
	bool Get( char alias[], const std::size_t size, int * flag = 0 )
	{
		if (!filled)	return false;
		for (std::size_t i = current_index; i < length_; ++i)
		{	VS_H225AliasAddress   &addr = (static_cast<VS_H225AliasAddress *>(data()))[i];
			if (addr.tag == VS_H225AliasAddress::e_h323_ID)
			{	VS_GwAsnBmpString   *bmpStr = static_cast<VS_GwAsnBmpString *>(addr.choice);
				char   alias_name[256];		memset(alias_name, 0, sizeof(alias_name) );
				auto   alias_size = sizeof(alias_name);
				bmpStr->GetValueAsChar( alias_name, &alias_size );
				if (*alias_name)
				{	strncpy( alias, alias_name, size - 1 );		alias[size - 1] = 0;
					current_index = i;
				//	NormalizeAt(alias,size);
					return true;
				}
			}else if (addr.tag == VS_H225AliasAddress::e_dialedDigits)
			{
				auto &&digits =
					static_cast<TemplAlphabeticString<VS_H225AliasAddress::dialedDigits_alphabet,
					                                  sizeof(VS_H225AliasAddress::dialedDigits_alphabet),
					                                  VS_H225AliasAddress::dialedDigits_inverse_table, 1, 128, VS_Asn::FixedConstraint,
					                                  false> *>(addr.choice);
				auto sz = size;
					if (digits->GetNormalString(alias,sz))
					{
						if (flag)
						{
							*flag = atoi( alias );
						}
						current_index = i;
						return true;
					}
			}else if (addr.tag == VS_H225AliasAddress::e_partyNumber)
			{
				VS_H225PartyNumber &party = *static_cast<VS_H225PartyNumber*>(addr.choice);
				if (party.tag ==VS_H225PartyNumber::e_e164Number)
				{
					VS_H225PublicPartyNumber &number = *static_cast<VS_H225PublicPartyNumber*>(party.choice);
					std::size_t sz = size;
					if (number.publicNumberDigits.GetNormalString(alias,sz))
					{
						if (flag)
						{
							*flag = atoi( alias );
						}

						current_index = i;
						return true;
					}

				}
			}
		}
		return false;
	}
};
// end of VS_GwH225ArrayOf_AliasAddress struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH225ProtocolIdentifier : public VS_H225ProtocolIdentifier
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH225ProtocolIdentifier( const std::uint32_t value[], const std::size_t value_size )
	{	(reinterpret_cast<VS_GwAsnObjectId *>(this))->SetValue( value, value_size );	}
	// end of VS_GwH225ProtocolIdentifier::VS_GwH225ProtocolIdentifier

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH225ProtocolIdentifier

/////////////////////////////////////////////////////////////////////////////////////////
struct VS_GwH245OpenLogicalChannel : public VS_H245OpenLogicalChannel
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245OpenLogicalChannel( void );

	inline unsigned GetDataType( void ) const
	{	return forwardLogicalChannelParameters.dataType.tag;	}
	// end of VS_GwH245OpenLogicalChannel::GetDataType

	bool GetReverseRtcpIpPort(net::address& rtcp_addr, net::port& port);
	// end of VS_GwH245OpenLogicalChannel::GetReverseRtcpIpPort

	bool GetSeparateStackIpPort(net::address& addr, net::port& port);
	// end of VS_GwH245OpenLogicalChannel::GetT120IpPort

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245OpenLogicalChannel struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245CloseLogicalChannel : public VS_H245CloseLogicalChannel
{
	VS_GwH245CloseLogicalChannel( void );
};
// end of VS_GwH245CloseLogicalChannel struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245CloseLogicalChannelAck : public VS_H245CloseLogicalChannelAck
{
	VS_GwH245CloseLogicalChannelAck( void );
};
// end of VS_GwH245CloseLogicalChannelAck struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245OpenLogicalChannelAck : public VS_H245OpenLogicalChannelAck
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245OpenLogicalChannelAck( void );

	bool GetRtpRtcpIpPort(net::address& rtpAddr, net::port& rtpPort, net::address& rtcpAddr,
	                             net::port& rtcpPort);
	// end of VS_GwH245OpenLogicalChannelAck::GetRtpRtcpIpPort

	bool GetSeparateStackIpPort(net::address& addr, net::port& port);
	// end of VS_GwH245OpenLogicalChannel::GetSeparateStackIpPort

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245OpenLogicalChannelAck struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245MultimediaSystemControlMessage : public VS_H245MultimediaSystemControlMessage
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245MultimediaSystemControlMessage( void );

	inline VS_GwH245OpenLogicalChannel *GetOLC( void )
	{
		VS_H245MultimediaSystemControlMessage   *p = this;
		if (!p)		return nullptr;
		VS_H245RequestMessage   *rq = *p;
		if (!rq)	return nullptr;
		VS_H245OpenLogicalChannel   *olc = *rq;
		if (!olc)	return nullptr;
		return static_cast<VS_GwH245OpenLogicalChannel *>(olc);
	}
	// end of VS_GwH245MultimediaSystemControlMessage::GetOLC

	inline VS_GwH245OpenLogicalChannelAck *GetOLCA( void )
	{
		VS_H245MultimediaSystemControlMessage   *p = this;
		if (!p)		return nullptr;
		VS_H245ResponseMessage   *rm = *p;
		if (!rm)	return nullptr;
		VS_H245OpenLogicalChannelAck   *olca = *rm;
		if (!olca)	return nullptr;
		return static_cast<VS_GwH245OpenLogicalChannelAck *>(olca);
	}
	// end of VS_GwH245MultimediaSystemControlMessage::GetOLCA

	inline VS_GwH245CloseLogicalChannel *GetCLC( void )
	{
		VS_H245MultimediaSystemControlMessage   *p = this;
		if (!p)		return nullptr;
		VS_H245RequestMessage   *rq = *p;
		if (!rq)	return nullptr;
		VS_H245CloseLogicalChannel   *clc = *rq;
		if (!clc)	return nullptr;
		return static_cast<VS_GwH245CloseLogicalChannel *>(clc);
	}
	// end of VS_GwH245MultimediaSystemControlMessage::GetCLC

	inline VS_GwH245CloseLogicalChannelAck *GetCLCA( void )
	{
		VS_H245MultimediaSystemControlMessage   *p = this;
		if (!p)		return nullptr;
		VS_H245ResponseMessage   *rm = *p;
		if (!rm)	return nullptr;
		VS_H245CloseLogicalChannelAck   *clca = *rm;
		if (!clca)	return nullptr;
		return static_cast<VS_GwH245CloseLogicalChannelAck *>(clca);
	}
	// end of VS_GwH245MultimediaSystemControlMessage::GetCLCA

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245MultimediaSystemControlMessage struct
/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245MediaDistributionCapability : public VS_H245MediaDistributionCapability
{
	/////////////////////////////////////////////////////////////////////////////////////

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		printf( "CentralizedControl: %s\n", centralizedControl.value ? "TRUE" : "FALSE" );
		printf( "DistributedControl: %s\n", distributedControl.value ? "TRUE" : "FALSE" );
		printf( "CentralizedAudio: %s\n", centralizedAudio.value ? "TRUE" : "FALSE" );
		printf( "DistributedAudio: %s\n", distributedAudio.value ? "TRUE" : "FALSE" );
		printf( "CentralizedVideo: %s\n", centralizedVideo.value ? "TRUE" : "FALSE" );
		printf( "DistributedVideo: %s\n", distributedVideo.value ? "TRUE" : "FALSE" );
		if (!centralizedData.filled)	puts( "CentralizedData Is Not Filled" );
		else	printf( "CentralizedData.length: %zu\n", centralizedData.size() );
		if (!distributedData.filled)	puts( "DistributedData Is Not Filled" );
		else	printf( "DistributedData.length: %zu\n", distributedData.size() );
	}
	// end of VS_GwH245MediaDistributionCapability::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245MediaDistributionCapability struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245MultipointCapability : public VS_H245MultipointCapability
{
	/////////////////////////////////////////////////////////////////////////////////////

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		printf( "MulticastCapability: %s\n", multicastCapability.value ? "TRUE" : "FALSE" );
		printf( "MultiUniCastConference: %s\n", multiUniCastConference.value ? "TRUE" : "FALSE" );
		printf( "MediaDistributionCapability.length: %zu\n", mediaDistributionCapability.size() );
		for (std::size_t i = 0; i < mediaDistributionCapability.size(); ++i)
		{	printf( "MediaDistributionCapability Number %zu:\n", i + 1 );
			(static_cast<VS_GwH245MediaDistributionCapability *>(mediaDistributionCapability.data()))[i].PrintCapability();
	}	}
	// end of VS_GwH245MultipointCapability::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245MultipointCapability struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245H2250Capability : public VS_H245H2250Capability
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245H2250Capability( void );

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		printf( "MaximumAudioDelayJitter: %u\n", maximumAudioDelayJitter.value );
		puts( "ReceiveMultipointCapability:" );		static_cast<VS_GwH245MultipointCapability &>(receiveMultipointCapability).PrintCapability();
		puts( "TransmitMultipointCapability:" );	static_cast<VS_GwH245MultipointCapability &>(transmitMultipointCapability).PrintCapability();
		puts( "ReceiveAndTransmitMultipointCapability:" );	static_cast<VS_GwH245MultipointCapability &>(receiveAndTransmitMultipointCapability).PrintCapability();
		printf( "McCapability.CentralizedConferenceMC: %s\n", mcCapability.centralizedConferenceMC.value ? "TRUE" : "FALSE" );
		printf( "McCapability.DecentralizedConferenceMC: %s\n", mcCapability.decentralizedConferenceMC.value ? "TRUE" : "FALSE" );
		printf( "RtcpVideoControlCapability: %s\n", rtcpVideoControlCapability.value ? "TRUE" : "FALSE" );
		printf( "MediaPacketizationCapability.H261aVideoPacketization: %s\n", mediaPacketizationCapability.h261aVideoPacketization.value ? "TRUE" : "FALSE" );
	}
	// end of VS_GwH245H2250Capability::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245H2250Capability struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245MultiplexCapability : public VS_H245MultiplexCapability
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245MultiplexCapability( void );

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		switch (tag)
		{
		case e_nonStandard :		puts( "NonStandard" );		return;
		case e_h222Capability :		puts( "H222Capability" );	return;
		case e_h223Capability :		puts( "H223Capability" );	return;
		case e_v76Capability :		puts( "V76Capability" );	return;
		case e_h2250Capability :	puts( "H2250Capability" );	(static_cast<VS_GwH245H2250Capability *>(choice))->PrintCapability();	return;
		default :					printf( "Number Of Choice: %u", tag );
	}	}
	// end of VS_GwH245MultiplexCapability::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245MultiplexCapability struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245Capability : public VS_H245Capability
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245Capability( void );

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		switch (tag)
		{
		case e_nonStandard :									puts( "NonStandard:" );		return;
		case e_receiveVideoCapability :							puts( "ReceiveVideoCapability:" );		return;
		case e_transmitVideoCapability :						puts( "TransmitVideoCapability:" );		return;
		case e_receiveAndTransmitVideoCapability :				puts( "ReceiveAndTransmitVideoCapability:" );	return;
		case e_receiveAudioCapability :							puts( "ReceiveAudioCapability:" );		return;
		case e_transmitAudioCapability :						puts( "TransmitAudioCapability:" );		return;
		case e_receiveAndTransmitAudioCapability :				puts( "ReceiveAndTransmitAudioCapability:" );	return;
		case e_receiveDataApplicationCapability :				puts( "ReceiveDataApplicationCapability:" );	return;
		case e_transmitDataApplicationCapability :				puts( "TransmitDataApplicationCapability:" );	return;
		case e_receiveAndTransmitDataApplicationCapability :	puts( "ReceiveAndTransmitDataApplicationCapability:" );		return;
		case e_h233EncryptionTransmitCapability :				puts( "H233EncryptionTransmitCapability:" );	return;
		case e_h233EncryptionReceiveCapability :				puts( "H233EncryptionReceiveCapability:" );		return;
		case e_conferenceCapability :							puts( "ConferenceCapability:" );	return;
		default :												printf( "Number Of Choice: %u", tag );
	}	}
	// end of VS_GwH245Capability::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245Capability struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245CapabilityTableEntry : public VS_H245CapabilityTableEntry
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245CapabilityTableEntry( void );

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		printf( "CapabilityTableEntryNumber: %u\n", capabilityTableEntryNumber.value );
		puts( "Capability:" );	static_cast<VS_GwH245Capability &>(capability).PrintCapability();
	}
	// end of VS_GwH245CapabilityTableEntry::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245CapabilityTableEntry struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245AlternativeCapabilitySet : public VS_H245AlternativeCapabilitySet
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245AlternativeCapabilitySet( void );

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		printf( "AlternativeCapabilitySet Length: %zu\n", size() );
		for (unsigned i = 0; i < size(); ++i )
			printf( "CapabilityTableEntryNumber Number %u:, Value: %u\n", i + 1, (static_cast<VS_H245CapabilityTableEntryNumber *>(data()))[i].value );
	}
	// end of VS_GwH245AlternativeCapabilitySet::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245AlternativeCapabilitySet struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245CapabilityDescriptor : public VS_H245CapabilityDescriptor
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245CapabilityDescriptor( void );

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		printf( "CapabilityDescriptorNumber: %u\n", capabilityDescriptorNumber.value );
		printf( "SimultaneousCapabilities Length: %zu\n", simultaneousCapabilities.size() );
		for (unsigned i = 0; i < simultaneousCapabilities.size(); ++i)
		{	printf( "SimultaneousCapabilities Number %u:\n", i + 1 );
			(static_cast<VS_GwH245AlternativeCapabilitySet *>(simultaneousCapabilities.data()))[i].PrintCapability();
	}	}
	// end of VS_GwH245CapabilityDescriptor::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245CapabilityDescriptor struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_GwH245TerminalCapabilitySet : public VS_H245TerminalCapabilitySet
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_GwH245TerminalCapabilitySet( void );

	inline void PrintCapability( void )
	{
		if (!filled) {		puts( "Not Filled." );	return;		}
		printf( "SequenceNumber: %u\n", sequenceNumber.value );
		printf( "ProtocolIdentifier: %u,%u,%u,%u,%u,%u\n", protocolIdentifier.value[0], protocolIdentifier.value[1], protocolIdentifier.value[2], protocolIdentifier.value[3], protocolIdentifier.value[4], protocolIdentifier.value[5] );
		puts( "MultiplexCapability:" );
		static_cast<VS_GwH245MultiplexCapability &>(multiplexCapability).PrintCapability();
		printf( "CapabilityTable Length: %zu\n", capabilityTable.size() );
		for (std::size_t i = 0; i < capabilityTable.size(); ++i)
		{	printf( "CapabilityTableEntry Number %zu:\n", i + 1 );
			static_cast<VS_GwH245CapabilityTableEntry &>(capabilityTable[i]).PrintCapability();	}
		printf( "CapabilityDescriptors Length: %zu\n", capabilityDescriptors.size() );
		for ( std::size_t i = 0; i < capabilityDescriptors.size(); ++i)
		{	printf( "CapabilityDescriptor Number %zu:\n", i + 1 );
			(static_cast<VS_GwH245CapabilityDescriptor *>(capabilityDescriptors.data()))[i].PrintCapability();
	}	}
	// end of VS_GwH245TerminalCapabilitySet::PrintCapability

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_GwH245TerminalCapabilitySet struct
