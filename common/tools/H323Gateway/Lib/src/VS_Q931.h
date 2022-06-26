/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 22.12.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_H323PDUs.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VS_BaseBuffers.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/VS_WideStr.h"

#include "std-generic/compat/memory.h"

#ifndef NDEBUG
#include <limits>
#endif

#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER

struct VS_Q931
{
	/////////////////////////////////////////////////////////////////////////////////////

	enum {	e_alertingMsg			= 0x01,		// H.225-7.3.1
			e_callProceedingMsg		= 0x02,		// H.225-7.3.2
			e_connectMsg			= 0x07,		// H.225-7.3.3
			e_connectAcknowledgeMsg	= 0x0F,		// H.225-7.3.4
			e_disconnectMsg			= 0x45,		// H.225-7.3.5
			e_informationMsg		= 0x7B,		// H.225-7.3.6
			e_progressMsg			= 0x03,		// H.225-7.3.7
			e_releaseMsg			= 0x4D,		// H.225-7.3.8
			e_releaseCompleteMsg	= 0x5A,		// H.225-7.3.9
			e_setupMsg				= 0x05,		// H.225-7.3.10
			e_setupAcknowledgeMsg	= 0x0D,		// H.225-7.3.11
			e_statusMsg				= 0x7D,		// H.225-7.3.12
			e_statusInquiryMsg		= 0x75,		// H.225-7.3.13
			e_facilityMsg			= 0x62,		// H.225-7.4.1
			e_notifyMsg				= 0x6E	};	// H.225-7.4.2

    enum {	e_bearerCapabilityIE			= 0x04,		// H.225-7.2.2.1
			e_callIdentityIE				= 0x10,		// H.225-7.2.2.2
			e_callStateIE					= 0x14,		// H.225-7.2.2.3
			e_calledPartyNumberIE			= 0x70,		// H.225-7.2.2.4
			e_calledPartySubaddressIE		= 0x71,		// H.225-7.2.2.5
			e_callingPartyNumberIE			= 0x6C,		// H.225-7.2.2.6
			e_callingPartySubaddressIE		= 0x6D,		// H.225-7.2.2.7
			e_causeIE						= 0x08,		// H.225-7.2.2.8
			e_channelIdentificationIE		= 0x18,		// H.225-7.2.2.9
			e_connectedNumberIE				= 0x4C,		// H.225-7.2.2.10
			e_connectedSubAddressIE			= 0x4D,		// H.225-7.2.2.11
			e_congestionLevelIE				= 0xB0,		// H.225-7.2.2.12
			e_dateTimeIE					= 0x29,		// H.225-7.2.2.13
			e_displayIE						= 0x28,		// H.225-7.2.2.14
			e_extendedFacilityIE			= 0x0D,		// H.225-7.2.2.15
			e_facilityIE					= 0x1C,		// H.225-7.2.2.16
			e_highLayerCompatibilityIE		= 0x3D,		// H.225-7.2.2.17
			e_keypadFacilityIE				= 0x2C,		// H.225-7.2.2.18
			e_lowLayerCompatibilityIE		= 0x3C,		// H.225-7.2.2.19
			e_moreDataIE					= 0xA0,		// H.225-7.2.2.20
			e_networkSpecificFacilitiesIE	= 0x20,		// H.225-7.2.2.21
			e_notificationIndicatorIE		= 0x27,		// H.225-7.2.2.22
			e_progressIndicatorIE			= 0x1E,		// H.225-7.2.2.23
			e_redirectingNumberIE			= 0x74,		// H.225-7.2.2.24
			e_repeatIndicatorIE				= 0xD0,		// H.225-7.2.2.25
			e_restartIndicatorIE			= 0x79,		// H.225-7.2.2.26
			e_segmentedMessageIE			= 0x00,		// H.225-7.2.2.27
			e_sendingCompleteIE				= 0xA1,		// H.225-7.2.2.28
			e_signalIE						= 0x34,		// H.225-7.2.2.29
			e_transitNetworkSelectionIE		= 0x78,		// H.225-7.2.2.30
			e_userUserIE					= 0x7E	};	// H.225-7.2.2.31

	/////////////////////////////////////////////////////////////////////////////////////

	VS_Q931( void )
		: protocolDiscriminator(~0), lengthCallReference(~0)
		, fromDestination(~0), callReference(~0), messageType(~0)
	{}
	// end of VS_Q931::VS_Q931

	~VS_Q931( void ) {}
	// end of VS_Q931::~VS_Q931

    unsigned   protocolDiscriminator, lengthCallReference,
				fromDestination, callReference, messageType;

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool DecodeMHeader( VS_BitBuffer &buffer )
	{	//DebugBreak();
		if (!buffer.GetBits( protocolDiscriminator, 8 ))	return false;
		if (protocolDiscriminator != 0x08)					return false;
		if (!buffer.GetBits( lengthCallReference, 8 ))		return false;
		if (lengthCallReference != 0x02)					return false;
		if (!buffer.GetBit( fromDestination ))				return false;
		if (!buffer.GetBits( callReference, 15 ))			return false;
		if (!buffer.GetBits( messageType, 8 ))				return false;
		return true;
	}
	// end of VS_Q931::DecodeMHeader

	inline bool EncodeMHeader( VS_BitBuffer &buffer )
	{	//DebugBreak();
		protocolDiscriminator = 0x08;
		if (!buffer.AddBits( protocolDiscriminator, 8 ))	return false;
		lengthCallReference = 0x02;
		if (!buffer.AddBits( lengthCallReference, 8 ))		return false;
		buffer.AddBit(fromDestination);
		if (!buffer.AddBits( callReference, 15 ))			return false;
		if (!buffer.AddBits( messageType, 8 ))				return false;
		return true;
	}
	// end of VS_Q931::EncodeMHeader

	static inline bool GetUserUserIE( VS_BitBuffer &buffer,
		unsigned char(&displayName)[82 + 1 /*0-terminator*/]  /*ITU-T Rec. Q.931 (05/98):  The minimum length is 2 octets; the maximum length is network dependent and is either 34 or 82 octets*/,
		unsigned char (&e164)[50] )
	{	//DebugBreak();
		while (buffer.BitsLeft())
		{	std::uint32_t   val;
			if (!buffer.GetBits( val, 8 ))	break;
			if (!( val & 0x80 ))
			{	if (val != e_userUserIE)
				{	if (val==e_displayIE)
					{
						if (!buffer.GetBits( val, 8 ))
							break;

						if(val < sizeof(displayName))
						{
							if(buffer.GetBits(displayName, 8 * val))
							{
								displayName[val] = 0;
								dstream4 << "Q.931 DisplayName: " << displayName;
							}
						}
						else
						{
							dprint4("bad display name at Q.931\n");
							if (!buffer.IncreaseIndex(val * 8))
								break;
						}
					}////////////////////////////////////////////
					else if (val==e_calledPartyNumberIE)
					{
						std::uint32_t length = 0;
						if (!buffer.GetBits( length, 8 ))		break;
						if ( (length > 1) && (length < sizeof(e164)) )
						{
							std::uint32_t  gb = 0;
							if (!buffer.GetBits(gb,8)) break;

							length--;	// Skip 1st byte - Type of number && Numbering plan identification

							VS_PerBuffer stringLooksToBeIA5;
							if (!stringLooksToBeIA5.AddBits(buffer, length*8)) break;

							if (!stringLooksToBeIA5.GetBits(e164, length * 8)) break;
							e164[length] = 0;

							dprint4("e_calledPartyNumberIE = %s\n", e164);
						}else
							if (!buffer.IncreaseIndex( length * 8 ))	break;


					}else////////////////////////////////////////////
					{	if (!buffer.GetBits( val, 8 ))		break;
						if (!buffer.IncreaseIndex( val * 8 ))	break;
					}
				} else
				{	if (!buffer.GetBits( val, 16 ))		break;
					if (val < 4)	break;
					if (!buffer.IncreaseIndex( 8 ))		break;
					--val;		val *= 8;
					if ( val > buffer.BitsLeft())	break;
					return true;
		}	}	}
		return false;
	}
	// end of VS_Q931::GetUserUserIE

	static inline bool SetUserUserIE( VS_BitBuffer &buffer, VS_BitBuffer &uuIE, const int8_t* rate_multiplier = nullptr,
		bool isConnect = false, const unsigned char * display_name=nullptr,
		const unsigned char * digits = nullptr)
	{
		// Q.931 - 4.5.5 Bearer capability
		unsigned char data[] = {
			0x04,		// Bearer capability information element identifier
			0x04,		// Length: 4 bytes
			0x88,		// bit8=1; bit 6,7=0 coding standard; bit1-5=Unrestricted digital information
			0x18,		// bit8=0 extension indicator; bit 6,7=0 (Transfer mode = Circuit mode); bit1-5=11000 (Information transfer rate: Multirate (64 kbit/s base rate))
			0x80+120,	// Rate multiplier=120
			0xa5
		};
		if (isConnect)
		{
			if (rate_multiplier && *rate_multiplier)
				data[4] = 0x80 | (*rate_multiplier & 0x7f);		// first bit always = 1
			buffer.AddBits(data, sizeof(data) * 8);
		}
		if (display_name)
		{
			const auto dm = 40;
			auto size = std::char_traits<char>::length((const char*)display_name);
			size = (size>34)?34:size;
			if (!buffer.AddBits( dm , 8 )) return false;
			if (!buffer.AddBits( size , 8 )) return false;
			buffer.AddBits(display_name, size * 8);
		}
		if (digits)
		{
			const auto calledPartyNumber = e_calledPartyNumberIE;
			const auto type = 161;
			auto size = std::char_traits<char>::length((const char *)digits);
			size += 1; ///for type
			if (!buffer.AddBits( calledPartyNumber , 8 )) return false;
			assert(size <= static_cast<size_t>(std::numeric_limits<std::uint32_t>::max()));
			if (!buffer.AddBits( static_cast<std::uint32_t>(size) , 8 )) return false;
			if (!buffer.AddBits( type , 8 )) return false;
			buffer.AddBits(digits, (size - 1) * 8);
		}
		if (!buffer.AddBits( e_userUserIE, 8 ))		return false;
		if (!buffer.AddBits( uuIE.ByteSize() + 1  //+(isConnect==true)*sizeof(data) , 16 ))		return false;
												, 16 ))		return false;
		if (!buffer.AddBits( 0x05, 8 ))		return false;
		if (!buffer.AddBits( uuIE ))	return false;
		return true;
	}
	// end of VS_Q931::SetUserUserIE

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_Q931 struct

#undef DEBUG_CURRENT_MODULE
