/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 14.01.04     by  A.Slavetsky
//  Modified:     A.Vlaskin, A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_H245Messages.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////



#pragma once

#include "VS_Containers.h"
#include "VS_AsnBuffers.h"
#include "VS_CommonMessages.h"
#include <array>


//////////////////////CLASS VS_H245CapabilityDescriptorNumber /////////////////////////

typedef TemplInteger<0,255,VS_Asn::FixedConstraint,0>  VS_H245CapabilityDescriptorNumber;
//////////////////////CLASS VS_H245CapabilityTableEntryNumber /////////////////////////

typedef TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  VS_H245CapabilityTableEntryNumber;
//////////////////////CLASS VS_H245AlternativeCapabilitySet /////////////////////////

typedef Constrained_array_of_type<  VS_H245CapabilityTableEntryNumber ,1,256,VS_Asn::FixedConstraint,0  >  VS_H245AlternativeCapabilitySet;


//////////////////////CLASS VS_H245LogicalChannelNumber /////////////////////////

typedef TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  VS_H245LogicalChannelNumber;

//////////////////////CLASS VS_H245MaximumBitRate /////////////////////////

typedef TemplInteger<2147483647,2147483647,VS_Asn::FixedConstraint,0>  VS_H245MaximumBitRate;

//////////////////////CLASS VS_H245MaxRedundancy /////////////////////////

typedef TemplInteger<1,127,VS_Asn::FixedConstraint,0>  VS_H245MaxRedundancy;

//////////////////////CLASS VS_H245SequenceNumber /////////////////////////

typedef TemplInteger<0,255,VS_Asn::FixedConstraint,0>  VS_H245SequenceNumber;

//////////////////////CLASS VS_H245MultiplexTableEntryNumber /////////////////////////

typedef TemplInteger<1,15,VS_Asn::FixedConstraint,0>  VS_H245MultiplexTableEntryNumber;

//////////////////////CLASS VS_H245IV16 /////////////////////////

typedef TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  VS_H245IV16;
//////////////////////CLASS VS_H245IV8 /////////////////////////

typedef TemplOctetString<8,8,VS_Asn::FixedConstraint,0>  VS_H245IV8;

//////////////////////CLASS VS_H245TerminalNumber /////////////////////////

typedef TemplInteger<0,192,VS_Asn::FixedConstraint,0>  VS_H245TerminalNumber;

//////////////////////CLASS VS_H323IV16 /////////////////////////

typedef TemplOctetString<16, 16, VS_Asn::FixedConstraint, 0>  VS_H323IV16;
//////////////////////CLASS VS_H323IV8 /////////////////////////

typedef TemplOctetString<8, 8, VS_Asn::FixedConstraint, 0>  VS_H323IV8;
//////////////////////CLASS VS_H323KeyMaterial /////////////////////////

typedef TemplBitString<1, 2048, VS_Asn::FixedConstraint, 0>  VS_H323KeyMaterial;
//////////////////////CLASS VS_H323Identifier /////////////////////////

typedef TemplBmpString<1, 128, VS_Asn::FixedConstraint, 0>  VS_H323H235Identifier;
//////////////////////CLASS VS_H323Password /////////////////////////

typedef TemplBmpString<1, 128, VS_Asn::FixedConstraint, 0>  VS_H323Password;
//////////////////////CLASS VS_H323RandomVal /////////////////////////

typedef TemplInteger< 0, INT_MAX, VS_Asn::Unconstrained, false>  VS_H323RandomVal;
//////////////////////CLASS VS_H323TimeStamp /////////////////////////

typedef TemplInteger<1, 4294967295, VS_Asn::FixedConstraint, 0>  VS_H323TimeStamp;
//////////////////////CLASS VS_H323ChallengeString /////////////////////////

typedef TemplOctetString<8, 128, VS_Asn::FixedConstraint, 0>  VS_H323ChallengeString;


//////////////////////CLASS VS_H245QOSMode /////////////////////////

struct VS_H245QOSMode : public VS_AsnChoice
{
	 VS_H245QOSMode( void );

 	enum{
	e_guaranteedQOS,
	e_controlledLoad
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245QOSMode & src);


	void Show( void ) const;

};


//////////////////////CLASS VS_H245ATMParameters /////////////////////////

struct VS_H245ATMParameters : public VS_AsnSequence
{
	 VS_H245ATMParameters( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  maxNTUSize ;
 	 VS_AsnBoolean  atmUBR ;
 	 VS_AsnBoolean  atmrtVBR ;
 	 VS_AsnBoolean  atmnrtVBR ;
 	 VS_AsnBoolean  atmABR ;
 	 VS_AsnBoolean  atmCBR ;
 	void operator=(const VS_H245ATMParameters& src);

};


//////////////////////CLASS VS_H245NonStandardIdentifier /////////////////////////

struct VS_H245NonStandardIdentifier : public VS_AsnChoice
{
	 VS_H245NonStandardIdentifier( void );

 	enum{
	e_object,
	e_h221NonStandard
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NonStandardIdentifier & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NonStandardParameter /////////////////////////

struct VS_H245NonStandardParameter : public VS_AsnSequence
{
	 VS_H245NonStandardParameter( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245NonStandardIdentifier  nonStandardIdentifier ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  data ;
 	void operator=(const VS_H245NonStandardParameter& src);

};
//////////////////////CLASS VS_H245V75Parameters /////////////////////////

struct VS_H245V75Parameters : public VS_AsnSequence
{
	 VS_H245V75Parameters( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  audioHeaderPresent ;
 	void operator=(const VS_H245V75Parameters& src);

};
struct VS_H245UnicastAddress_IPAddress;
struct VS_H245UnicastAddress_IP6Address;
//////////////////////CLASS VS_H245UnicastAddress /////////////////////////

struct VS_H245UnicastAddress : public VS_AsnChoice
{
	 VS_H245UnicastAddress( void );

 	enum{
	e_iPAddress,
	e_iPXAddress,
	e_iP6Address,
	e_netBios,
	e_iPSourceRouteAddress,
	e_nsap,
	e_nonStandardAddress
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245UnicastAddress & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245UnicastAddress_IPAddress*(void);
	 operator VS_H245UnicastAddress_IP6Address*(void);
	 void operator=(VS_H245UnicastAddress_IPAddress *uma_ipa);
	 void operator=(VS_H245UnicastAddress_IP6Address *uma_ipa);

	void Show( void ) const;

};


//////////////////////CLASS VS_H245MediaTransportType /////////////////////////

struct VS_H245MediaTransportType : public VS_AsnChoice
{
	 VS_H245MediaTransportType( void );

 	enum{
	e_ip_UDP,
	e_ip_TCP,
	e_atm_AAL5_UNIDIR,
	e_atm_AAL5_BIDIR,
	e_atm_AAL5_compressed
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MediaTransportType & src);


	void Show( void ) const;

};
//////////////////////CLASS VS_H245NonStandardMessage /////////////////////////

struct VS_H245NonStandardMessage : public VS_AsnSequence
{
	 VS_H245NonStandardMessage( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245NonStandardParameter  nonStandardData ;
 	void operator=(const VS_H245NonStandardMessage& src);

};




//////////////////////CLASS VS_H245MulticastAddress /////////////////////////

struct VS_H245MulticastAddress : public VS_AsnChoice
{
	 VS_H245MulticastAddress( void );

 	enum{
	e_iPAddress,
	e_iP6Address,
	e_nsap,
	e_nonStandardAddress
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MulticastAddress & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245TransportAddress /////////////////////////

struct VS_H245TransportAddress : public VS_AsnChoice
{
	 VS_H245TransportAddress( void );

 	enum{
	e_unicastAddress,
	e_multicastAddress
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245TransportAddress & src);

	 operator VS_H245UnicastAddress *( void );
	 operator VS_H245MulticastAddress *( void );

 	void operator=( VS_H245UnicastAddress *ua );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245DataProtocolCapability /////////////////////////

struct VS_H245DataProtocolCapability : public VS_AsnChoice
{
	 VS_H245DataProtocolCapability( void );

 	enum{
	e_nonStandard,
	e_v14buffered,
	e_v42lapm,
	e_hdlcFrameTunnelling,
	e_h310SeparateVCStack,
	e_h310SingleVCStack,
	e_transparent,
	e_segmentationAndReassembly,
	e_hdlcFrameTunnelingwSAR,
	e_v120,
	e_separateLANStack,
	e_v76wCompression,
	e_tcp,
	e_udp
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DataProtocolCapability & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MediaChannelCapability /////////////////////////

struct VS_H245MediaChannelCapability : public VS_AsnSequence
{
	 VS_H245MediaChannelCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245MediaTransportType  mediaTransport ;
 	void operator=(const VS_H245MediaChannelCapability& src);

};
//////////////////////CLASS VS_H245H223AL1MParameters_TransferMode /////////////////////////

struct VS_H245H223AL1MParameters_TransferMode : public VS_AsnChoice
{
	 VS_H245H223AL1MParameters_TransferMode( void );

 	enum{
	e_framed,
	e_unframed
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AL1MParameters_TransferMode & src);


	void Show( void ) const;

};


//////////////////////CLASS VS_H245GSMAudioCapability /////////////////////////

struct VS_H245GSMAudioCapability : public VS_AsnSequence
{
	 VS_H245GSMAudioCapability( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  audioUnitSize ;
 	 VS_AsnBoolean  comfortNoise ;
 	 VS_AsnBoolean  scrambled ;
 	void operator=(const VS_H245GSMAudioCapability& src);

};
//////////////////////CLASS VS_H245MobileMultilinkReconfigurationIndication /////////////////////////

struct VS_H245MobileMultilinkReconfigurationIndication : public VS_AsnSequence
{
	 VS_H245MobileMultilinkReconfigurationIndication( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  sampleSize ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  samplesPerFrame ;
 	void operator=(const VS_H245MobileMultilinkReconfigurationIndication& src);

};

//////////////////////CLASS VS_H245FlowControlIndication_Scope /////////////////////////

struct VS_H245FlowControlIndication_Scope : public VS_AsnChoice
{
	 VS_H245FlowControlIndication_Scope( void );

 	enum{
	e_logicalChannelNumber,
	e_resourceID,
	e_wholeMultiplex
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FlowControlIndication_Scope & src);

	 operator VS_H245LogicalChannelNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245FlowControlIndication_Restriction /////////////////////////

struct VS_H245FlowControlIndication_Restriction : public VS_AsnChoice
{
	 VS_H245FlowControlIndication_Restriction( void );

 	enum{
	e_maximumBitRate,
	e_noRestriction
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FlowControlIndication_Restriction & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245FlowControlIndication /////////////////////////

struct VS_H245FlowControlIndication : public VS_AsnSequence
{
	 VS_H245FlowControlIndication( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245FlowControlIndication_Scope	 scope ;
 	VS_H245FlowControlIndication_Restriction	 restriction ;
 	void operator=(const VS_H245FlowControlIndication& src);

};

struct VS_H245NonStandardParameter;
//////////////////////CLASS VS_H245UserInputIndication_UserInputSupportIndication /////////////////////////

struct VS_H245UserInputIndication_UserInputSupportIndication : public VS_AsnChoice
{
	 VS_H245UserInputIndication_UserInputSupportIndication( void );

 	enum{
	e_nonStandard,
	e_basicString,
	e_iA5String,
	e_generalString,
	e_encryptedBasicString,
	e_encryptedIA5String,
	e_encryptedGeneralString
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245UserInputIndication_UserInputSupportIndication & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245UserInputIndication_Signal_Rtp /////////////////////////

struct VS_H245UserInputIndication_Signal_Rtp : public VS_AsnSequence
{
	 VS_H245UserInputIndication_Signal_Rtp( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  timestamp ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  expirationTime ;
 	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	void operator=(const VS_H245UserInputIndication_Signal_Rtp& src);

};

//////////////////////CLASS VS_H245UserInputIndication_SignalUpdate_Rtp /////////////////////////

struct VS_H245UserInputIndication_SignalUpdate_Rtp : public VS_AsnSequence
{
	 VS_H245UserInputIndication_SignalUpdate_Rtp( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	void operator=(const VS_H245UserInputIndication_SignalUpdate_Rtp& src);

};
//////////////////////CLASS VS_H245UserInputIndication_SignalUpdate /////////////////////////

struct VS_H245UserInputIndication_SignalUpdate : public VS_AsnSequence
{
	 VS_H245UserInputIndication_SignalUpdate( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  duration ;
 	VS_H245UserInputIndication_SignalUpdate_Rtp	 rtp ;
 	void operator=(const VS_H245UserInputIndication_SignalUpdate& src);

};



//////////////////////CLASS VS_H245UserInputIndication /////////////////////////

struct VS_H245UserInputIndication : public VS_AsnChoice
{
	 VS_H245UserInputIndication( void );

 	enum{
	e_nonStandard,
	e_alphanumeric,
	e_userInputSupportIndication,
	e_signal,
	e_signalUpdate,
	e_extendedAlphanumeric,
	e_encryptedAlphanumeric
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245UserInputIndication & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245Params /////////////////////////

struct VS_H245Params : public VS_AsnSequence
{
	 VS_H245Params( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245IV8  iv8 ;
 	 VS_H245IV16  iv16 ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  iv ;
 	void operator=(const VS_H245Params& src);

};

struct VS_H323Params : public VS_AsnSequence
{
	VS_H323Params(void);
    VS_H323Params(const VS_H323Params& other);

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 3;
    VS_Reference_of_Asn  e_ref[extension_root];

	TemplInteger< 0, INT_MAX, VS_Asn::Unconstrained, false>  ranInt;
	VS_H323IV8  iv8;
	VS_H323IV16  iv16;
	TemplOctetString< 0, INT_MAX, VS_Asn::Unconstrained, false>  iv;
	TemplOctetString< 0, INT_MAX, VS_Asn::Unconstrained, false>  clearSalt;
	void operator=(const VS_H323Params& src);

};

//////////////////////CLASS VS_H323KeySyncMaterial /////////////////////////

struct VS_H323KeySyncMaterial : public VS_AsnSequence
{
	VS_H323KeySyncMaterial(void);
    VS_H323KeySyncMaterial(const VS_H323KeySyncMaterial &src);

	static const unsigned basic_root = 2;
    VS_Reference_of_Asn  ref[basic_root];
	static const unsigned extension_root = 0;

	VS_H323H235Identifier  generalID;
	VS_H323KeyMaterial  keyMaterial;
	void operator=(const VS_H323KeySyncMaterial& src);

};

typedef Type_id< VS_H323KeySyncMaterial >  VS_H323EncodedKeySyncMaterial;

//////////////////////CLASS VS_H323V3KeySyncMaterial /////////////////////////

struct VS_H323V3KeySyncMaterial : public VS_AsnSequence
{
	VS_H323V3KeySyncMaterial(void);
    VS_H323V3KeySyncMaterial(const VS_H323V3KeySyncMaterial& src);

	static const unsigned basic_root = 8;
    VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;

	VS_H323H235Identifier  generalID;
	VS_AsnObjectId  algorithmOID;
	VS_H323Params  paramS;
	TemplOctetString< 0, INT_MAX, VS_Asn::Unconstrained, false>  encryptedSessionKey;
	TemplOctetString< 0, INT_MAX, VS_Asn::Unconstrained, false>  encryptedSaltingKey;
	TemplOctetString< 0, INT_MAX, VS_Asn::Unconstrained, false>  clearSaltingKey;
	VS_H323Params  paramSsalt;
	VS_AsnObjectId  keyDerivationOID;
	void operator=(const VS_H323V3KeySyncMaterial& src);

};

//////////////////////CLASS VS_H323EncryptedEncodedKeySyncMaterial /////////////////////////

struct VS_H323EncryptedEncodedKeySyncMaterial : public VS_AsnSequence
{
	VS_H323EncryptedEncodedKeySyncMaterial(void);
    VS_H323EncryptedEncodedKeySyncMaterial(const VS_H323EncryptedEncodedKeySyncMaterial&);

	static const unsigned basic_root = 3;
    VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;

	VS_AsnObjectId  algorithmOID;
	VS_H323Params  paramS;
	TemplOctetString< 0, INT_MAX, VS_Asn::Unconstrained, false>  encryptedData;
	void operator=(const VS_H323EncryptedEncodedKeySyncMaterial& src);

};

struct VS_H323KeySignedMaterial : public VS_AsnSequence
{
	VS_H323KeySignedMaterial(void);
    VS_H323KeySignedMaterial(const VS_H323KeySignedMaterial&);

	static const unsigned basic_root = 5;
    VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;

	VS_H323H235Identifier  generalId;
	VS_H323RandomVal  mrandom;
	VS_H323RandomVal  srandom;
	VS_H323TimeStamp  timeStamp;
	VS_H323EncryptedEncodedKeySyncMaterial  encrptval;
	void operator=(const VS_H323KeySignedMaterial& src);

};
typedef Type_id< VS_H323KeySignedMaterial >  VS_H323EncodedKeySignedMaterial;

//////////////////////CLASS VS_H323SignedEncodedKeySignedMaterial /////////////////////////

struct VS_H323SignedEncodedKeySignedMaterial : public VS_AsnSequence
{
	VS_H323SignedEncodedKeySignedMaterial(void);
    VS_H323SignedEncodedKeySignedMaterial(const VS_H323SignedEncodedKeySignedMaterial&);

	static const unsigned basic_root = 4;
    VS_Reference_of_Asn  ref[basic_root];
	static const unsigned extension_root = 0;

	VS_H323EncodedKeySignedMaterial  toBeSigned;
	VS_AsnObjectId  algorithmOID;
	VS_H323Params  paramS;
	TemplBitString< 0, INT_MAX, VS_Asn::Unconstrained, false>  signature;
	void operator=(const VS_H323SignedEncodedKeySignedMaterial& src);

};

//////////////////////CLASS VS_H323H235Key /////////////////////////

struct VS_H323H235Key : public VS_AsnChoice
{
	VS_H323H235Key(void);

	enum{
		e_secureChannel,
		e_sharedSecret,
		e_certProtectedKey,
		e_secureSharedSecret
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H323H235Key & src);

	operator VS_H323KeyMaterial *(void);
	operator VS_H323EncryptedEncodedKeySyncMaterial *(void);
	operator VS_H323SignedEncodedKeySignedMaterial *(void);
	operator VS_H323V3KeySyncMaterial *(void);

	void Show(void);

};

//////////////////////CLASS VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric /////////////////////////

struct VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric : public VS_AsnSequence
{
	 VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnObjectId  algorithmOID ;
 	 VS_H245Params  paramS ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  encrypted ;
 	void operator=(const VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric& src);

};
//////////////////////CLASS VS_H245UserInputIndication_ExtendedAlphanumeric /////////////////////////

struct VS_H245UserInputIndication_ExtendedAlphanumeric : public VS_AsnSequence
{
	 VS_H245UserInputIndication_ExtendedAlphanumeric( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_AsnGeneralString  alphanumeric ;
 	 VS_AsnNull  rtpPayloadIndication ;
 	VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric	 encryptedAlphanumeric ;
 	void operator=(const VS_H245UserInputIndication_ExtendedAlphanumeric& src);

};
//////////////////////CLASS VS_H245UserInputIndication_EncryptedAlphanumeric /////////////////////////

struct VS_H245UserInputIndication_EncryptedAlphanumeric : public VS_AsnSequence
{
	 VS_H245UserInputIndication_EncryptedAlphanumeric( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnObjectId  algorithmOID ;
 	 VS_H245Params  paramS ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  encrypted ;
 	void operator=(const VS_H245UserInputIndication_EncryptedAlphanumeric& src);

};



//////////////////////CLASS VS_H245UserInputIndication_Signal /////////////////////////

struct VS_H245UserInputIndication_Signal : public VS_AsnSequence
{
	 VS_H245UserInputIndication_Signal( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];


	 /////////////////////////////////////////////////////////////////////////////////////////
	 static unsigned char   signalType_alphabet[17];
	 static unsigned char   signalType_inverse_table[256];
	 static const bool      signalType_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////
	//TemplAlphabeticString< signalType_alphabet, signalType_alphabet_size,signalType_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>  signalType ;
	 TemplAlphabeticString<signalType_alphabet, sizeof(signalType_alphabet), signalType_inverse_table, 1, 1, VS_Asn::FixedConstraint, 0> signalType;
 	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  duration ;
 	VS_H245UserInputIndication_Signal_Rtp	 rtp ;
 	 VS_AsnNull  rtpPayloadIndication ;
 	 VS_H245Params  paramS ;
 	TemplOctetString<1,1,VS_Asn::FixedConstraint,0>  encryptedSignalType ;
 	 VS_AsnObjectId  algorithmOID ;
 	void operator=(const VS_H245UserInputIndication_Signal& src);

};


//////////////////////CLASS VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery /////////////////////////
struct VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery : public VS_AsnChoice
{
	 VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery( void );

 	enum{
	e_nullClockRecovery,
	e_srtsClockRecovery,
	e_adaptiveClockRecovery
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection /////////////////////////

struct VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection : public VS_AsnChoice
{
	 VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection( void );

 	enum{
	e_nullErrorCorrection,
	e_longInterleaver,
	e_shortInterleaver,
	e_errorCorrectionOnly
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCIndication_Aal_Aal1 /////////////////////////

struct VS_H245NewATMVCIndication_Aal_Aal1 : public VS_AsnSequence
{
	 VS_H245NewATMVCIndication_Aal_Aal1( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery	 clockRecovery ;
 	VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection	 errorCorrection ;
 	 VS_AsnBoolean  structuredDataTransfer ;
 	 VS_AsnBoolean  partiallyFilledCells ;
 	void operator=(const VS_H245NewATMVCIndication_Aal_Aal1& src);

};
//////////////////////CLASS VS_H245NewATMVCIndication_Aal_Aal5 /////////////////////////

struct VS_H245NewATMVCIndication_Aal_Aal5 : public VS_AsnSequence
{
	 VS_H245NewATMVCIndication_Aal_Aal5( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  forwardMaximumSDUSize ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  backwardMaximumSDUSize ;
 	void operator=(const VS_H245NewATMVCIndication_Aal_Aal5& src);

};
//////////////////////CLASS VS_H245NewATMVCIndication_Aal /////////////////////////

struct VS_H245NewATMVCIndication_Aal : public VS_AsnChoice
{
	 VS_H245NewATMVCIndication_Aal( void );

 	enum{
	e_aal1,
	e_aal5
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCIndication_Aal & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCIndication_Multiplex /////////////////////////

struct VS_H245NewATMVCIndication_Multiplex : public VS_AsnChoice
{
	 VS_H245NewATMVCIndication_Multiplex( void );

 	enum{
	e_noMultiplex,
	e_transportStream,
	e_programStream
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCIndication_Multiplex & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCIndication_ReverseParameters_Multiplex /////////////////////////

struct VS_H245NewATMVCIndication_ReverseParameters_Multiplex : public VS_AsnChoice
{
	 VS_H245NewATMVCIndication_ReverseParameters_Multiplex( void );

 	enum{
	e_noMultiplex,
	e_transportStream,
	e_programStream
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCIndication_ReverseParameters_Multiplex & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCIndication_ReverseParameters /////////////////////////

struct VS_H245NewATMVCIndication_ReverseParameters : public VS_AsnSequence
{
	 VS_H245NewATMVCIndication_ReverseParameters( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  bitRate ;
 	 VS_AsnBoolean  bitRateLockedToPCRClock ;
 	 VS_AsnBoolean  bitRateLockedToNetworkClock ;
 	VS_H245NewATMVCIndication_ReverseParameters_Multiplex	 multiplex ;
 	void operator=(const VS_H245NewATMVCIndication_ReverseParameters& src);

};
//////////////////////CLASS VS_H245NewATMVCIndication /////////////////////////

struct VS_H245NewATMVCIndication : public VS_AsnSequence
{
	 VS_H245NewATMVCIndication( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  resourceID ;
 	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  bitRate ;
 	 VS_AsnBoolean  bitRateLockedToPCRClock ;
 	 VS_AsnBoolean  bitRateLockedToNetworkClock ;
 	VS_H245NewATMVCIndication_Aal	 aal ;
 	VS_H245NewATMVCIndication_Multiplex	 multiplex ;
 	VS_H245NewATMVCIndication_ReverseParameters	 reverseParameters ;
 	void operator=(const VS_H245NewATMVCIndication& src);

};
//////////////////////CLASS VS_H245VendorIdentification /////////////////////////

struct VS_H245VendorIdentification : public VS_AsnSequence
{
	 VS_H245VendorIdentification( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245NonStandardIdentifier  vendor ;
 	TemplOctetString<1,256,VS_Asn::FixedConstraint,0>  productNumber ;
 	TemplOctetString<1,256,VS_Asn::FixedConstraint,0>  versionNumber ;
 	void operator=(const VS_H245VendorIdentification& src);

};
//////////////////////CLASS VS_H245MCLocationIndication /////////////////////////

struct VS_H245MCLocationIndication : public VS_AsnSequence
{
	 VS_H245MCLocationIndication( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TransportAddress  signalAddress ;
 	void operator=(const VS_H245MCLocationIndication& src);

};
//////////////////////CLASS VS_H245H2250MaximumSkewIndication /////////////////////////

struct VS_H245H2250MaximumSkewIndication : public VS_AsnSequence
{
	 VS_H245H2250MaximumSkewIndication( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  logicalChannelNumber1 ;
 	 VS_H245LogicalChannelNumber  logicalChannelNumber2 ;
 	TemplInteger<0,4095,VS_Asn::FixedConstraint,0>  maximumSkew ;
 	void operator=(const VS_H245H2250MaximumSkewIndication& src);

};
//////////////////////CLASS VS_H245H223SkewIndication /////////////////////////

struct VS_H245H223SkewIndication : public VS_AsnSequence
{
	 VS_H245H223SkewIndication( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  logicalChannelNumber1 ;
 	 VS_H245LogicalChannelNumber  logicalChannelNumber2 ;
 	TemplInteger<0,4095,VS_Asn::FixedConstraint,0>  skew ;
 	void operator=(const VS_H245H223SkewIndication& src);

};
//////////////////////CLASS VS_H245JitterIndication_Scope /////////////////////////

struct VS_H245JitterIndication_Scope : public VS_AsnChoice
{
	 VS_H245JitterIndication_Scope( void );

 	enum{
	e_logicalChannelNumber,
	e_resourceID,
	e_wholeMultiplex
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245JitterIndication_Scope & src);

	 operator VS_H245LogicalChannelNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245JitterIndication /////////////////////////

struct VS_H245JitterIndication : public VS_AsnSequence
{
	 VS_H245JitterIndication( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245JitterIndication_Scope	 scope ;
 	TemplInteger<0,3,VS_Asn::FixedConstraint,0>  estimatedReceivedJitterMantissa ;
 	TemplInteger<0,7,VS_Asn::FixedConstraint,0>  estimatedReceivedJitterExponent ;
 	TemplInteger<0,15,VS_Asn::FixedConstraint,0>  skippedFrameCount ;
 	TemplInteger<0,262143,VS_Asn::FixedConstraint,0>  additionalDecoderBuffer ;
 	void operator=(const VS_H245JitterIndication& src);

};
//////////////////////CLASS VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs /////////////////////////

struct VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs : public VS_AsnSequence
{
	 VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,8192,VS_Asn::FixedConstraint,0>  firstMB ;
 	TemplInteger<1,8192,VS_Asn::FixedConstraint,0>  numberOfMBs ;
 	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  temporalReference ;
 	void operator=(const VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs& src);

};
struct VS_H245TransportCapability;
//////////////////////CLASS VS_H245MiscellaneousIndication_Type /////////////////////////

struct VS_H245MiscellaneousIndication_Type : public VS_AsnChoice
{
	 VS_H245MiscellaneousIndication_Type( void );

 	enum{
	e_logicalChannelActive,
	e_logicalChannelInactive,
	e_multipointConference,
	e_cancelMultipointConference,
	e_multipointZeroComm,
	e_cancelMultipointZeroComm,
	e_multipointSecondaryStatus,
	e_cancelMultipointSecondaryStatus,
	e_videoIndicateReadyToActivate,
	e_videoTemporalSpatialTradeOff,
	e_videoNotDecodedMBs,
	e_transportCapability
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MiscellaneousIndication_Type & src);

	 operator VS_H245TransportCapability *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MiscellaneousIndication /////////////////////////

struct VS_H245MiscellaneousIndication : public VS_AsnSequence
{
	 VS_H245MiscellaneousIndication( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	VS_H245MiscellaneousIndication_Type	 type ;
 	void operator=(const VS_H245MiscellaneousIndication& src);

};
//////////////////////CLASS VS_H245VideoIndicateCompose /////////////////////////

struct VS_H245VideoIndicateCompose : public VS_AsnSequence
{
	 VS_H245VideoIndicateCompose( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  compositionNumber ;
 	void operator=(const VS_H245VideoIndicateCompose& src);

};
//////////////////////CLASS VS_H245TerminalYouAreSeeingInSubPictureNumber /////////////////////////

struct VS_H245TerminalYouAreSeeingInSubPictureNumber : public VS_AsnSequence
{
	 VS_H245TerminalYouAreSeeingInSubPictureNumber( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalNumber  terminalNumber ;
 	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  subPictureNumber ;
 	void operator=(const VS_H245TerminalYouAreSeeingInSubPictureNumber& src);

};
struct VS_H245TerminalLabel;
//////////////////////CLASS VS_H245ConferenceIndication /////////////////////////

struct VS_H245ConferenceIndication : public VS_AsnChoice
{
	 VS_H245ConferenceIndication( void );

 	enum{
	e_sbeNumber,
	e_terminalNumberAssign,
	e_terminalJoinedConference,
	e_terminalLeftConference,
	e_seenByAtLeastOneOther,
	e_cancelSeenByAtLeastOneOther,
	e_seenByAll,
	e_cancelSeenByAll,
	e_terminalYouAreSeeing,
	e_requestForFloor,
	e_withdrawChairToken,
	e_floorRequested,
	e_terminalYouAreSeeingInSubPictureNumber,
	e_videoIndicateCompose
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ConferenceIndication & src);

	 operator VS_H245TerminalLabel *( void );
	 operator VS_H245TerminalYouAreSeeingInSubPictureNumber *( void );
	 operator VS_H245VideoIndicateCompose *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245FunctionNotSupported_Cause /////////////////////////

struct VS_H245FunctionNotSupported_Cause : public VS_AsnChoice
{
	 VS_H245FunctionNotSupported_Cause( void );

 	enum{
	e_syntaxError,
	e_semanticError,
	e_unknownFunction
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FunctionNotSupported_Cause & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245FunctionNotSupported /////////////////////////

struct VS_H245FunctionNotSupported : public VS_AsnSequence
{
	 VS_H245FunctionNotSupported( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245FunctionNotSupported_Cause	 cause ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  returnedFunction ;
 	void operator=(const VS_H245FunctionNotSupported& src);

};

struct VS_H245RequestMessage;
struct VS_H245ResponseMessage;
struct VS_H245CommandMessage;
//////////////////////CLASS VS_H245FunctionNotUnderstood /////////////////////////

struct VS_H245FunctionNotUnderstood : public VS_AsnChoice
{
	 VS_H245FunctionNotUnderstood( void );

 	enum{
	e_request,
	e_response,
	e_command
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FunctionNotUnderstood & src);

	 operator VS_H245RequestMessage *( void );
	 operator VS_H245ResponseMessage *( void );
	 operator VS_H245CommandMessage *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MobileMultilinkReconfigurationCommand_Status /////////////////////////

struct VS_H245MobileMultilinkReconfigurationCommand_Status : public VS_AsnChoice
{
	 VS_H245MobileMultilinkReconfigurationCommand_Status( void );

 	enum{
	e_synchronized,
	e_reconfiguration
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MobileMultilinkReconfigurationCommand_Status & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MobileMultilinkReconfigurationCommand /////////////////////////

struct VS_H245MobileMultilinkReconfigurationCommand : public VS_AsnSequence
{
	 VS_H245MobileMultilinkReconfigurationCommand( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  sampleSize ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  samplesPerFrame ;
 	VS_H245MobileMultilinkReconfigurationCommand_Status	 status ;
 	void operator=(const VS_H245MobileMultilinkReconfigurationCommand& src);

};
//////////////////////CLASS VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery /////////////////////////

struct VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery : public VS_AsnChoice
{
	 VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery( void );

 	enum{
	e_nullClockRecovery,
	e_srtsClockRecovery,
	e_adaptiveClockRecovery
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection /////////////////////////

struct VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection : public VS_AsnChoice
{
	 VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection( void );

 	enum{
	e_nullErrorCorrection,
	e_longInterleaver,
	e_shortInterleaver,
	e_errorCorrectionOnly
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCCommand_Aal_Aal1 /////////////////////////

struct VS_H245NewATMVCCommand_Aal_Aal1 : public VS_AsnSequence
{
	 VS_H245NewATMVCCommand_Aal_Aal1( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery	 clockRecovery ;
 	VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection	 errorCorrection ;
 	 VS_AsnBoolean  structuredDataTransfer ;
 	 VS_AsnBoolean  partiallyFilledCells ;
 	void operator=(const VS_H245NewATMVCCommand_Aal_Aal1& src);

};
//////////////////////CLASS VS_H245NewATMVCCommand_Aal_Aal5 /////////////////////////

struct VS_H245NewATMVCCommand_Aal_Aal5 : public VS_AsnSequence
{
	 VS_H245NewATMVCCommand_Aal_Aal5( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  forwardMaximumSDUSize ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  backwardMaximumSDUSize ;
 	void operator=(const VS_H245NewATMVCCommand_Aal_Aal5& src);

};
//////////////////////CLASS VS_H245NewATMVCCommand_Aal /////////////////////////

struct VS_H245NewATMVCCommand_Aal : public VS_AsnChoice
{
	 VS_H245NewATMVCCommand_Aal( void );

 	enum{
	e_aal1,
	e_aal5
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCCommand_Aal & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCCommand_Multiplex /////////////////////////

struct VS_H245NewATMVCCommand_Multiplex : public VS_AsnChoice
{
	 VS_H245NewATMVCCommand_Multiplex( void );

 	enum{
	e_noMultiplex,
	e_transportStream,
	e_programStream
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCCommand_Multiplex & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCCommand_ReverseParameters_Multiplex /////////////////////////

struct VS_H245NewATMVCCommand_ReverseParameters_Multiplex : public VS_AsnChoice
{
	 VS_H245NewATMVCCommand_ReverseParameters_Multiplex( void );

 	enum{
	e_noMultiplex,
	e_transportStream,
	e_programStream
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NewATMVCCommand_ReverseParameters_Multiplex & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NewATMVCCommand_ReverseParameters /////////////////////////

struct VS_H245NewATMVCCommand_ReverseParameters : public VS_AsnSequence
{
	 VS_H245NewATMVCCommand_ReverseParameters( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  bitRate ;
 	 VS_AsnBoolean  bitRateLockedToPCRClock ;
 	 VS_AsnBoolean  bitRateLockedToNetworkClock ;
 	VS_H245NewATMVCCommand_ReverseParameters_Multiplex	 multiplex ;
 	void operator=(const VS_H245NewATMVCCommand_ReverseParameters& src);

};
//////////////////////CLASS VS_H245NewATMVCCommand /////////////////////////

struct VS_H245NewATMVCCommand : public VS_AsnSequence
{
	 VS_H245NewATMVCCommand( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  resourceID ;
 	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  bitRate ;
 	 VS_AsnBoolean  bitRateLockedToPCRClock ;
 	 VS_AsnBoolean  bitRateLockedToNetworkClock ;
 	VS_H245NewATMVCCommand_Aal	 aal ;
 	VS_H245NewATMVCCommand_Multiplex	 multiplex ;
 	VS_H245NewATMVCCommand_ReverseParameters	 reverseParameters ;
 	void operator=(const VS_H245NewATMVCCommand& src);

};
//////////////////////CLASS VS_H245H223MultiplexReconfiguration_H223ModeChange /////////////////////////

struct VS_H245H223MultiplexReconfiguration_H223ModeChange : public VS_AsnChoice
{
	 VS_H245H223MultiplexReconfiguration_H223ModeChange( void );

 	enum{
	e_toLevel0,
	e_toLevel1,
	e_toLevel2,
	e_toLevel2withOptionalHeader
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223MultiplexReconfiguration_H223ModeChange & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag /////////////////////////

struct VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag : public VS_AsnChoice
{
	 VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag( void );

 	enum{
	e_start,
	e_stop
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223MultiplexReconfiguration /////////////////////////

struct VS_H245H223MultiplexReconfiguration : public VS_AsnChoice
{
	 VS_H245H223MultiplexReconfiguration( void );

 	enum{
	e_h223ModeChange,
	e_h223AnnexADoubleFlag
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223MultiplexReconfiguration & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245PictureReference /////////////////////////

struct VS_H245PictureReference : public VS_AsnChoice
{
	 VS_H245PictureReference( void );

 	enum{
	e_pictureNumber,
	e_longTermPictureIndex
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245PictureReference & src);


	void Show( void ) const;

};


//////////////////////CLASS VS_H245KeyProtectionMethod /////////////////////////

struct VS_H245KeyProtectionMethod : public VS_AsnSequence
{
	 VS_H245KeyProtectionMethod( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  secureChannel ;
 	 VS_AsnBoolean  sharedSecret ;
 	 VS_AsnBoolean  certProtectedKey ;
 	void operator=(const VS_H245KeyProtectionMethod& src);

};
//////////////////////CLASS VS_H245EncryptionUpdateRequest /////////////////////////

struct VS_H245EncryptionUpdateRequest : public VS_AsnSequence
{
	 VS_H245EncryptionUpdateRequest( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245KeyProtectionMethod  keyProtectionMethod ;
 	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  synchFlag ;
 	void operator=(const VS_H245EncryptionUpdateRequest& src);

};
//////////////////////CLASS VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB /////////////////////////

struct VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB : public VS_AsnSequence
{
	 VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,17,VS_Asn::FixedConstraint,0>  firstGOB ;
 	TemplInteger<1,18,VS_Asn::FixedConstraint,0>  numberOfGOBs ;
 	void operator=(const VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB& src);

};
//////////////////////CLASS VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB /////////////////////////

struct VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB : public VS_AsnSequence
{
	 VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  firstGOB ;
 	TemplInteger<1,8192,VS_Asn::FixedConstraint,0>  firstMB ;
 	TemplInteger<1,8192,VS_Asn::FixedConstraint,0>  numberOfMBs ;
 	void operator=(const VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB& src);

};
//////////////////////CLASS VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount /////////////////////////

struct VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount : public VS_AsnChoice
{
	 VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount( void );

 	enum{
	e_doOneProgression,
	e_doContinuousProgressions,
	e_doOneIndependentProgression,
	e_doContinuousIndependentProgressions
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart /////////////////////////

struct VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart : public VS_AsnSequence
{
	 VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount	 repeatCount ;
 	void operator=(const VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart& src);

};
//////////////////////CLASS VS_H245MiscellaneousCommand_Type_VideoBadMBs /////////////////////////

struct VS_H245MiscellaneousCommand_Type_VideoBadMBs : public VS_AsnSequence
{
	 VS_H245MiscellaneousCommand_Type_VideoBadMBs( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,9216,VS_Asn::FixedConstraint,0>  firstMB ;
 	TemplInteger<1,9216,VS_Asn::FixedConstraint,0>  numberOfMBs ;
 	TemplInteger<0,1023,VS_Asn::FixedConstraint,0>  temporalReference ;
 	void operator=(const VS_H245MiscellaneousCommand_Type_VideoBadMBs& src);

};
//////////////////////CLASS VS_H245MiscellaneousCommand_Type_LostPartialPicture /////////////////////////

struct VS_H245MiscellaneousCommand_Type_LostPartialPicture : public VS_AsnSequence
{
	 VS_H245MiscellaneousCommand_Type_LostPartialPicture( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245PictureReference  pictureReference ;
 	TemplInteger<1,9216,VS_Asn::FixedConstraint,0>  firstMB ;
 	TemplInteger<1,9216,VS_Asn::FixedConstraint,0>  numberOfMBs ;
 	void operator=(const VS_H245MiscellaneousCommand_Type_LostPartialPicture& src);

};

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck /////////////////////////

struct VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck : public VS_AsnSequence
{
	 VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  synchFlag ;
 	void operator=(const VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck& src);

};

struct VS_H245EncryptionSync;
//////////////////////CLASS VS_H245MiscellaneousCommand_Type /////////////////////////

struct VS_H245MiscellaneousCommand_Type : public VS_AsnChoice
{
	 VS_H245MiscellaneousCommand_Type( void );

 	enum{
	e_equaliseDelay,
	e_zeroDelay,
	e_multipointModeCommand,
	e_cancelMultipointModeCommand,
	e_videoFreezePicture,
	e_videoFastUpdatePicture,
	e_videoFastUpdateGOB,
	e_videoTemporalSpatialTradeOff,
	e_videoSendSyncEveryGOB,
	e_videoSendSyncEveryGOBCancel,
	e_videoFastUpdateMB,
	e_maxH223MUXPDUsize,
	e_encryptionUpdate,
	e_encryptionUpdateRequest,
	e_switchReceiveMediaOff,
	e_switchReceiveMediaOn,
	e_progressiveRefinementStart,
	e_progressiveRefinementAbortOne,
	e_progressiveRefinementAbortContinuous,
	e_videoBadMBs,
	e_lostPicture,
	e_lostPartialPicture,
	e_recoveryReferencePicture,
	e_encryptionUpdateCommand,
	e_encryptionUpdateAck
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MiscellaneousCommand_Type & src);

	 operator VS_H245EncryptionSync *( void );
	 operator VS_H245EncryptionUpdateRequest *( void );
	 operator VS_H245PictureReference *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245EncryptionUpdateDirection /////////////////////////

struct VS_H245EncryptionUpdateDirection : public VS_AsnChoice
{
	 VS_H245EncryptionUpdateDirection( void );

 	enum{
	e_masterToSlave,
	e_slaveToMaster
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245EncryptionUpdateDirection & src);


	void Show( void ) const;

};
//////////////////////CLASS VS_H245MiscellaneousCommand /////////////////////////

struct VS_H245MiscellaneousCommand : public VS_AsnSequence
{
	 VS_H245MiscellaneousCommand( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	VS_H245MiscellaneousCommand_Type	 type ;
 	 VS_H245EncryptionUpdateDirection  direction ;
 	void operator=(const VS_H245MiscellaneousCommand& src);

};


//////////////////////CLASS VS_H245SubstituteConferenceIDCommand /////////////////////////

struct VS_H245SubstituteConferenceIDCommand : public VS_AsnSequence
{
	 VS_H245SubstituteConferenceIDCommand( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  conferenceIdentifier ;
 	void operator=(const VS_H245SubstituteConferenceIDCommand& src);

};
//////////////////////CLASS VS_H245ConferenceCommand /////////////////////////

struct VS_H245ConferenceCommand : public VS_AsnChoice
{
	 VS_H245ConferenceCommand( void );

 	enum{
	e_broadcastMyLogicalChannel,
	e_cancelBroadcastMyLogicalChannel,
	e_makeTerminalBroadcaster,
	e_cancelMakeTerminalBroadcaster,
	e_sendThisSource,
	e_cancelSendThisSource,
	e_dropConference,
	e_substituteConferenceIDCommand
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ConferenceCommand & src);

	 operator VS_H245LogicalChannelNumber *( void );
	 operator VS_H245TerminalLabel *( void );
	 operator VS_H245SubstituteConferenceIDCommand *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245EndSessionCommand_GstnOptions /////////////////////////

struct VS_H245EndSessionCommand_GstnOptions : public VS_AsnChoice
{
	 VS_H245EndSessionCommand_GstnOptions( void );

 	enum{
	e_telephonyMode,
	e_v8bis,
	e_v34DSVD,
	e_v34DuplexFAX,
	e_v34H324
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245EndSessionCommand_GstnOptions & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245EndSessionCommand_IsdnOptions /////////////////////////

struct VS_H245EndSessionCommand_IsdnOptions : public VS_AsnChoice
{
	 VS_H245EndSessionCommand_IsdnOptions( void );

 	enum{
	e_telephonyMode,
	e_v140,
	e_terminalOnHold
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245EndSessionCommand_IsdnOptions & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245EndSessionCommand /////////////////////////

struct VS_H245EndSessionCommand : public VS_AsnChoice
{
	 VS_H245EndSessionCommand( void );

 	enum{
	e_nonStandard,
	e_disconnect,
	e_gstnOptions,
	e_isdnOptions
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245EndSessionCommand & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245FlowControlCommand_Scope /////////////////////////

struct VS_H245FlowControlCommand_Scope : public VS_AsnChoice
{
	 VS_H245FlowControlCommand_Scope( void );

 	enum{
	e_logicalChannelNumber,
	e_resourceID,
	e_wholeMultiplex
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FlowControlCommand_Scope & src);

	 operator VS_H245LogicalChannelNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245FlowControlCommand_Restriction /////////////////////////

struct VS_H245FlowControlCommand_Restriction : public VS_AsnChoice
{
	 VS_H245FlowControlCommand_Restriction( void );

 	enum{
	e_maximumBitRate,
	e_noRestriction
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FlowControlCommand_Restriction & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245FlowControlCommand /////////////////////////

struct VS_H245FlowControlCommand : public VS_AsnSequence
{
	 VS_H245FlowControlCommand( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245FlowControlCommand_Scope	 scope ;
 	VS_H245FlowControlCommand_Restriction	 restriction ;
 	void operator=(const VS_H245FlowControlCommand& src);

};
//////////////////////CLASS VS_H245EncryptionCommand_EncryptionAlgorithmID /////////////////////////

struct VS_H245EncryptionCommand_EncryptionAlgorithmID : public VS_AsnSequence
{
	 VS_H245EncryptionCommand_EncryptionAlgorithmID( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  h233AlgorithmIdentifier ;
 	 VS_H245NonStandardParameter  associatedAlgorithm ;
 	void operator=(const VS_H245EncryptionCommand_EncryptionAlgorithmID& src);

};
//////////////////////CLASS VS_H245EncryptionCommand /////////////////////////

struct VS_H245EncryptionCommand : public VS_AsnChoice
{
	 VS_H245EncryptionCommand( void );

 	enum{
	e_encryptionSE,
	e_encryptionIVRequest,
	e_encryptionAlgorithmID
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245EncryptionCommand & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245SendTerminalCapabilitySet_SpecificRequest /////////////////////////

struct VS_H245SendTerminalCapabilitySet_SpecificRequest : public VS_AsnSequence
{
	 VS_H245SendTerminalCapabilitySet_SpecificRequest( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  multiplexCapability ;
 	Constrained_array_of_type<  VS_H245CapabilityTableEntryNumber ,1,65535,VS_Asn::FixedConstraint,0  >  capabilityTableEntryNumbers ;
 	Constrained_array_of_type<  VS_H245CapabilityDescriptorNumber ,1,256,VS_Asn::FixedConstraint,0  >  capabilityDescriptorNumbers ;
 	void operator=(const VS_H245SendTerminalCapabilitySet_SpecificRequest& src);

};
//////////////////////CLASS VS_H245SendTerminalCapabilitySet /////////////////////////

struct VS_H245SendTerminalCapabilitySet : public VS_AsnChoice
{
	 VS_H245SendTerminalCapabilitySet( void );

 	enum{
	e_specificRequest,
	e_genericRequest
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245SendTerminalCapabilitySet & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245LogicalChannelRateRelease /////////////////////////

struct VS_H245LogicalChannelRateRelease : public VS_AsnSequence
{
	 VS_H245LogicalChannelRateRelease( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245LogicalChannelRateRelease& src);

};
//////////////////////CLASS VS_H245LogicalChannelRateRejectReason /////////////////////////

struct VS_H245LogicalChannelRateRejectReason : public VS_AsnChoice
{
	 VS_H245LogicalChannelRateRejectReason( void );

 	enum{
	e_undefinedReason,
	e_insufficientResources
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245LogicalChannelRateRejectReason & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245LogicalChannelRateReject /////////////////////////

struct VS_H245LogicalChannelRateReject : public VS_AsnSequence
{
	 VS_H245LogicalChannelRateReject( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	 VS_H245LogicalChannelRateRejectReason  rejectReason ;
 	 VS_H245MaximumBitRate  currentMaximumBitRate ;
 	void operator=(const VS_H245LogicalChannelRateReject& src);

};
//////////////////////CLASS VS_H245LogicalChannelRateAcknowledge /////////////////////////

struct VS_H245LogicalChannelRateAcknowledge : public VS_AsnSequence
{
	 VS_H245LogicalChannelRateAcknowledge( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	 VS_H245MaximumBitRate  maximumBitRate ;
 	void operator=(const VS_H245LogicalChannelRateAcknowledge& src);

};
//////////////////////CLASS VS_H245LogicalChannelRateRequest /////////////////////////

struct VS_H245LogicalChannelRateRequest : public VS_AsnSequence
{
	 VS_H245LogicalChannelRateRequest( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	 VS_H245MaximumBitRate  maximumBitRate ;
 	void operator=(const VS_H245LogicalChannelRateRequest& src);

};

//////////////////////CLASS VS_H245ConnectionIdentifier /////////////////////////

struct VS_H245ConnectionIdentifier : public VS_AsnSequence
{
	 VS_H245ConnectionIdentifier( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  channelTag ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  sequenceNumber ;
 	void operator=(const VS_H245ConnectionIdentifier& src);

};
struct VS_H245NonStandardMessage;
//////////////////////CLASS VS_H245DialingInformationNetworkType /////////////////////////

struct VS_H245DialingInformationNetworkType : public VS_AsnChoice
{
	 VS_H245DialingInformationNetworkType( void );

 	enum{
	e_nonStandard,
	e_n_isdn,
	e_gstn,
	e_mobile
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DialingInformationNetworkType & src);

	 operator VS_H245NonStandardMessage *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245DialingInformationNumber /////////////////////////

struct VS_H245DialingInformationNumber : public VS_AsnSequence
{
	 VS_H245DialingInformationNumber( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplNumericString<0,40,VS_Asn::FixedConstraint,0>  networkAddress ;
 	TemplIA5String<1,40,VS_Asn::FixedConstraint,0>  subAddress ;
 	Constrained_array_of_type<  VS_H245DialingInformationNetworkType ,1,255,VS_Asn::FixedConstraint,0  >  networkType ;
 	void operator=(const VS_H245DialingInformationNumber& src);

};
//////////////////////CLASS VS_H245DialingInformation /////////////////////////

struct VS_H245DialingInformation : public VS_AsnChoice
{
	 VS_H245DialingInformation( void );

 	enum{
	e_nonStandard,
	e_differential,
	e_infoNotAvailable
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DialingInformation & src);

	 operator VS_H245NonStandardMessage *( void );
	 operator VS_H245DialingInformationNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultilinkIndication_CrcDesired /////////////////////////

struct VS_H245MultilinkIndication_CrcDesired : public VS_AsnSequence
{
	 VS_H245MultilinkIndication_CrcDesired( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245MultilinkIndication_CrcDesired& src);

};
//////////////////////CLASS VS_H245MultilinkIndication_ExcessiveError /////////////////////////

struct VS_H245MultilinkIndication_ExcessiveError : public VS_AsnSequence
{
	 VS_H245MultilinkIndication_ExcessiveError( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245ConnectionIdentifier  connectionIdentifier ;
 	void operator=(const VS_H245MultilinkIndication_ExcessiveError& src);

};
//////////////////////CLASS VS_H245MultilinkIndication /////////////////////////

struct VS_H245MultilinkIndication : public VS_AsnChoice
{
	 VS_H245MultilinkIndication( void );

 	enum{
	e_nonStandard,
	e_crcDesired,
	e_excessiveError
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultilinkIndication & src);

	 operator VS_H245NonStandardMessage *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultilinkResponse_CallInformation /////////////////////////

struct VS_H245MultilinkResponse_CallInformation : public VS_AsnSequence
{
	 VS_H245MultilinkResponse_CallInformation( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245DialingInformation  dialingInformation ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  callAssociationNumber ;
 	void operator=(const VS_H245MultilinkResponse_CallInformation& src);

};
//////////////////////CLASS VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected /////////////////////////

struct VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected : public VS_AsnChoice
{
	 VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected( void );

 	enum{
	e_connectionsNotAvailable,
	e_userRejected
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultilinkResponse_AddConnection_ResponseCode /////////////////////////

struct VS_H245MultilinkResponse_AddConnection_ResponseCode : public VS_AsnChoice
{
	 VS_H245MultilinkResponse_AddConnection_ResponseCode( void );

 	enum{
	e_accepted,
	e_rejected
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultilinkResponse_AddConnection_ResponseCode & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultilinkResponse_AddConnection /////////////////////////

struct VS_H245MultilinkResponse_AddConnection : public VS_AsnSequence
{
	 VS_H245MultilinkResponse_AddConnection( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	VS_H245MultilinkResponse_AddConnection_ResponseCode	 responseCode ;
 	void operator=(const VS_H245MultilinkResponse_AddConnection& src);

};
//////////////////////CLASS VS_H245MultilinkResponse_RemoveConnection /////////////////////////

struct VS_H245MultilinkResponse_RemoveConnection : public VS_AsnSequence
{
	 VS_H245MultilinkResponse_RemoveConnection( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245ConnectionIdentifier  connectionIdentifier ;
 	void operator=(const VS_H245MultilinkResponse_RemoveConnection& src);

};
//////////////////////CLASS VS_H245MultilinkResponse_MaximumHeaderInterval /////////////////////////

struct VS_H245MultilinkResponse_MaximumHeaderInterval : public VS_AsnSequence
{
	 VS_H245MultilinkResponse_MaximumHeaderInterval( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  currentInterval ;
 	void operator=(const VS_H245MultilinkResponse_MaximumHeaderInterval& src);

};
//////////////////////CLASS VS_H245MultilinkResponse /////////////////////////

struct VS_H245MultilinkResponse : public VS_AsnChoice
{
	 VS_H245MultilinkResponse( void );

 	enum{
	e_nonStandard,
	e_callInformation,
	e_addConnection,
	e_removeConnection,
	e_maximumHeaderInterval
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultilinkResponse & src);

	 operator VS_H245NonStandardMessage *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultilinkRequest_CallInformation /////////////////////////

struct VS_H245MultilinkRequest_CallInformation : public VS_AsnSequence
{
	 VS_H245MultilinkRequest_CallInformation( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  maxNumberOfAdditionalConnections ;
 	void operator=(const VS_H245MultilinkRequest_CallInformation& src);

};
//////////////////////CLASS VS_H245MultilinkRequest_AddConnection /////////////////////////

struct VS_H245MultilinkRequest_AddConnection : public VS_AsnSequence
{
	 VS_H245MultilinkRequest_AddConnection( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	 VS_H245DialingInformation  dialingInformation ;
 	void operator=(const VS_H245MultilinkRequest_AddConnection& src);

};
//////////////////////CLASS VS_H245MultilinkRequest_RemoveConnection /////////////////////////

struct VS_H245MultilinkRequest_RemoveConnection : public VS_AsnSequence
{
	 VS_H245MultilinkRequest_RemoveConnection( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245ConnectionIdentifier  connectionIdentifier ;
 	void operator=(const VS_H245MultilinkRequest_RemoveConnection& src);

};
//////////////////////CLASS VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType /////////////////////////

struct VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType : public VS_AsnChoice
{
	 VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType( void );

 	enum{
	e_currentIntervalInformation,
	e_requestedInterval
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultilinkRequest_MaximumHeaderInterval /////////////////////////

struct VS_H245MultilinkRequest_MaximumHeaderInterval : public VS_AsnSequence
{
	 VS_H245MultilinkRequest_MaximumHeaderInterval( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType	 requestType ;
 	void operator=(const VS_H245MultilinkRequest_MaximumHeaderInterval& src);

};
//////////////////////CLASS VS_H245MultilinkRequest /////////////////////////

struct VS_H245MultilinkRequest : public VS_AsnChoice
{
	 VS_H245MultilinkRequest( void );

 	enum{
	e_nonStandard,
	e_callInformation,
	e_addConnection,
	e_removeConnection,
	e_maximumHeaderInterval
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultilinkRequest & src);

	 operator VS_H245NonStandardMessage *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245RemoteMCResponse_Reject /////////////////////////

struct VS_H245RemoteMCResponse_Reject : public VS_AsnChoice
{
	 VS_H245RemoteMCResponse_Reject( void );

 	enum{
	e_unspecified,
	e_functionNotSupported
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RemoteMCResponse_Reject & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245RemoteMCResponse /////////////////////////

struct VS_H245RemoteMCResponse : public VS_AsnChoice
{
	 VS_H245RemoteMCResponse( void );

 	enum{
	e_accept,
	e_reject
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RemoteMCResponse & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245RemoteMCRequest /////////////////////////

struct VS_H245RemoteMCRequest : public VS_AsnChoice
{
	 VS_H245RemoteMCRequest( void );

 	enum{
	e_masterActivate,
	e_slaveActivate,
	e_deActivate
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RemoteMCRequest & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245Password /////////////////////////

typedef TemplOctetString<1,32,VS_Asn::FixedConstraint,0>  VS_H245Password;
//////////////////////CLASS VS_H245ConferenceID /////////////////////////

typedef TemplOctetString<1,32,VS_Asn::FixedConstraint,0>  VS_H245ConferenceID;
//////////////////////CLASS VS_H245TerminalID /////////////////////////

typedef TemplOctetString<1,128,VS_Asn::FixedConstraint,0>  VS_H245TerminalID;



//////////////////////CLASS VS_H245ConferenceResponse_MakeMeChairResponse /////////////////////////

struct VS_H245ConferenceResponse_MakeMeChairResponse : public VS_AsnChoice
{
	 VS_H245ConferenceResponse_MakeMeChairResponse( void );

 	enum{
	e_grantedChairToken,
	e_deniedChairToken
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ConferenceResponse_MakeMeChairResponse & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245ConferenceResponse_ExtensionAddressResponse /////////////////////////

struct VS_H245ConferenceResponse_ExtensionAddressResponse : public VS_AsnSequence
{
	 VS_H245ConferenceResponse_ExtensionAddressResponse( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalID  extensionAddress ;
 	void operator=(const VS_H245ConferenceResponse_ExtensionAddressResponse& src);

};


//////////////////////CLASS VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse /////////////////////////

struct VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse : public VS_AsnChoice
{
	 VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse( void );

 	enum{
	e_grantedBroadcastMyLogicalChannel,
	e_deniedBroadcastMyLogicalChannel
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse /////////////////////////

struct VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse : public VS_AsnChoice
{
	 VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse( void );

 	enum{
	e_grantedMakeTerminalBroadcaster,
	e_deniedMakeTerminalBroadcaster
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245ConferenceResponse_SendThisSourceResponse /////////////////////////

struct VS_H245ConferenceResponse_SendThisSourceResponse : public VS_AsnChoice
{
	 VS_H245ConferenceResponse_SendThisSourceResponse( void );

 	enum{
	e_grantedSendThisSource,
	e_deniedSendThisSource
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ConferenceResponse_SendThisSourceResponse & src);


	void Show( void ) const;

};

struct VS_H245RequestAllTerminalIDsResponse;
//////////////////////CLASS VS_H245ConferenceResponse /////////////////////////

struct VS_H245ConferenceResponse : public VS_AsnChoice
{
	 VS_H245ConferenceResponse( void );

 	enum{
	e_mCTerminalIDResponse,
	e_terminalIDResponse,
	e_conferenceIDResponse,
	e_passwordResponse,
	e_terminalListResponse,
	e_videoCommandReject,
	e_terminalDropReject,
	e_makeMeChairResponse,
	e_extensionAddressResponse,
	e_chairTokenOwnerResponse,
	e_terminalCertificateResponse,
	e_broadcastMyLogicalChannelResponse,
	e_makeTerminalBroadcasterResponse,
	e_sendThisSourceResponse,
	e_requestAllTerminalIDsResponse,
	e_remoteMCResponse
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ConferenceResponse & src);

	 operator VS_H245TerminalLabel *( void );
	 operator VS_H245RequestAllTerminalIDsResponse *( void );
	 operator VS_H245RemoteMCResponse *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245McuNumber /////////////////////////

typedef TemplInteger<0,192,VS_Asn::FixedConstraint,0>  VS_H245McuNumber;
//////////////////////CLASS VS_H245TerminalLabel /////////////////////////

struct VS_H245TerminalLabel : public VS_AsnSequence
{
	 VS_H245TerminalLabel( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245McuNumber  mcuNumber ;
 	 VS_H245TerminalNumber  terminalNumber ;
 	void operator=(const VS_H245TerminalLabel& src);

};

//////////////////////CLASS VS_H245ConferenceResponse_TerminalCertificateResponse /////////////////////////

struct VS_H245ConferenceResponse_TerminalCertificateResponse : public VS_AsnSequence
{
	 VS_H245ConferenceResponse_TerminalCertificateResponse( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalLabel  terminalLabel ;
 	TemplOctetString<1,65535,VS_Asn::FixedConstraint,0>  certificateResponse ;
 	void operator=(const VS_H245ConferenceResponse_TerminalCertificateResponse& src);

};

//////////////////////CLASS VS_H245ConferenceResponse_ChairTokenOwnerResponse /////////////////////////

struct VS_H245ConferenceResponse_ChairTokenOwnerResponse : public VS_AsnSequence
{
	 VS_H245ConferenceResponse_ChairTokenOwnerResponse( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalLabel  terminalLabel ;
 	 VS_H245TerminalID  terminalID ;
 	void operator=(const VS_H245ConferenceResponse_ChairTokenOwnerResponse& src);

};

//////////////////////CLASS VS_H245ConferenceResponse_PasswordResponse /////////////////////////

struct VS_H245ConferenceResponse_PasswordResponse : public VS_AsnSequence
{
	 VS_H245ConferenceResponse_PasswordResponse( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalLabel  terminalLabel ;
 	 VS_H245Password  password ;
 	void operator=(const VS_H245ConferenceResponse_PasswordResponse& src);

};
//////////////////////CLASS VS_H245ConferenceResponse_ConferenceIDResponse /////////////////////////

struct VS_H245ConferenceResponse_ConferenceIDResponse : public VS_AsnSequence
{
	 VS_H245ConferenceResponse_ConferenceIDResponse( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalLabel  terminalLabel ;
 	 VS_H245ConferenceID  conferenceID ;
 	void operator=(const VS_H245ConferenceResponse_ConferenceIDResponse& src);

};

//////////////////////CLASS VS_H245ConferenceResponse_TerminalIDResponse /////////////////////////

struct VS_H245ConferenceResponse_TerminalIDResponse : public VS_AsnSequence
{
	 VS_H245ConferenceResponse_TerminalIDResponse( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalLabel  terminalLabel ;
 	 VS_H245TerminalID  terminalID ;
 	void operator=(const VS_H245ConferenceResponse_TerminalIDResponse& src);

};
//////////////////////CLASS VS_H245ConferenceResponse_MCTerminalIDResponse /////////////////////////

struct VS_H245ConferenceResponse_MCTerminalIDResponse : public VS_AsnSequence
{
	 VS_H245ConferenceResponse_MCTerminalIDResponse( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalLabel  terminalLabel ;
 	 VS_H245TerminalID  terminalID ;
 	void operator=(const VS_H245ConferenceResponse_MCTerminalIDResponse& src);

};

//////////////////////CLASS VS_H245TerminalInformation /////////////////////////

struct VS_H245TerminalInformation : public VS_AsnSequence
{
	 VS_H245TerminalInformation( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalLabel  terminalLabel ;
 	 VS_H245TerminalID  terminalID ;
 	void operator=(const VS_H245TerminalInformation& src);

};

//////////////////////CLASS VS_H245RequestAllTerminalIDsResponse /////////////////////////

struct VS_H245RequestAllTerminalIDsResponse : public VS_AsnSequence
{
	 VS_H245RequestAllTerminalIDsResponse( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245TerminalInformation ,0,INT_MAX,VS_Asn::Unconstrained,0  >  terminalInformation ;
 	void operator=(const VS_H245RequestAllTerminalIDsResponse& src);

};

//////////////////////CLASS VS_H245Criteria /////////////////////////

struct VS_H245Criteria : public VS_AsnSequence
{
	 VS_H245Criteria( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnObjectId  field ;
 	TemplOctetString<1,65535,VS_Asn::FixedConstraint,0>  value ;
 	void operator=(const VS_H245Criteria& src);

};
//////////////////////CLASS VS_H245CertSelectionCriteria /////////////////////////

typedef Constrained_array_of_type<  VS_H245Criteria ,1,16,VS_Asn::FixedConstraint,0  >  VS_H245CertSelectionCriteria;
//////////////////////CLASS VS_H245ConferenceRequest_RequestTerminalCertificate /////////////////////////

struct VS_H245ConferenceRequest_RequestTerminalCertificate : public VS_AsnSequence
{
	 VS_H245ConferenceRequest_RequestTerminalCertificate( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245TerminalLabel  terminalLabel ;
 	 VS_H245CertSelectionCriteria  certSelectionCriteria ;
 	TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  sRandom ;
 	void operator=(const VS_H245ConferenceRequest_RequestTerminalCertificate& src);

};
//////////////////////CLASS VS_H245ConferenceRequest /////////////////////////

struct VS_H245ConferenceRequest : public VS_AsnChoice
{
	 VS_H245ConferenceRequest( void );

 	enum{
	e_terminalListRequest,
	e_makeMeChair,
	e_cancelMakeMeChair,
	e_dropTerminal,
	e_requestTerminalID,
	e_enterH243Password,
	e_enterH243TerminalID,
	e_enterH243ConferenceID,
	e_enterExtensionAddress,
	e_requestChairTokenOwner,
	e_requestTerminalCertificate,
	e_broadcastMyLogicalChannel,
	e_makeTerminalBroadcaster,
	e_sendThisSource,
	e_requestAllTerminalIDs,
	e_remoteMCRequest
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ConferenceRequest & src);

	 operator VS_H245TerminalLabel *( void );
	 operator VS_H245LogicalChannelNumber *( void );
	 operator VS_H245RemoteMCRequest *( void );

	void Show( void ) const;

};

struct VS_H245VideoCapability;
struct VS_H245AudioCapability;
struct VS_H245DataApplicationCapability;
//////////////////////CLASS VS_H245CommunicationModeTableEntry_DataType /////////////////////////

struct VS_H245CommunicationModeTableEntry_DataType : public VS_AsnChoice
{
	 VS_H245CommunicationModeTableEntry_DataType( void );

 	enum{
	e_videoData,
	e_audioData,
	e_data
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CommunicationModeTableEntry_DataType & src);

	 operator VS_H245VideoCapability *( void );
	 operator VS_H245AudioCapability *( void );
	 operator VS_H245DataApplicationCapability *( void );

	void Show( void ) const;

};

struct VS_H245CommunicationModeTableEntry;
//////////////////////CLASS VS_H245CommunicationModeResponse /////////////////////////

struct VS_H245CommunicationModeResponse : public VS_AsnChoice
{
	 VS_H245CommunicationModeResponse( void );

 	enum{
	e_communicationModeTable
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CommunicationModeResponse & src);

	 operator VS_H245CommunicationModeTableEntry *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245CommunicationModeRequest /////////////////////////

struct VS_H245CommunicationModeRequest : public VS_AsnSequence
{
	 VS_H245CommunicationModeRequest( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245CommunicationModeRequest& src);

};
//////////////////////CLASS VS_H245CommunicationModeCommand /////////////////////////

struct VS_H245CommunicationModeCommand : public VS_AsnSequence
{
	 VS_H245CommunicationModeCommand( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245CommunicationModeTableEntry ,1,256,VS_Asn::FixedConstraint,0  >  communicationModeTable ;
 	void operator=(const VS_H245CommunicationModeCommand& src);

};
//////////////////////CLASS VS_H245MaintenanceLoopOffCommand /////////////////////////

struct VS_H245MaintenanceLoopOffCommand : public VS_AsnSequence
{
	 VS_H245MaintenanceLoopOffCommand( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245MaintenanceLoopOffCommand& src);

};
//////////////////////CLASS VS_H245MaintenanceLoopReject_Type /////////////////////////

struct VS_H245MaintenanceLoopReject_Type : public VS_AsnChoice
{
	 VS_H245MaintenanceLoopReject_Type( void );

 	enum{
	e_systemLoop,
	e_mediaLoop,
	e_logicalChannelLoop
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MaintenanceLoopReject_Type & src);

	 operator VS_H245LogicalChannelNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MaintenanceLoopReject_Cause /////////////////////////

struct VS_H245MaintenanceLoopReject_Cause : public VS_AsnChoice
{
	 VS_H245MaintenanceLoopReject_Cause( void );

 	enum{
	e_canNotPerformLoop
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MaintenanceLoopReject_Cause & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MaintenanceLoopReject /////////////////////////

struct VS_H245MaintenanceLoopReject : public VS_AsnSequence
{
	 VS_H245MaintenanceLoopReject( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245MaintenanceLoopReject_Type	 type ;
 	VS_H245MaintenanceLoopReject_Cause	 cause ;
 	void operator=(const VS_H245MaintenanceLoopReject& src);

};
//////////////////////CLASS VS_H245MaintenanceLoopAck_Type /////////////////////////

struct VS_H245MaintenanceLoopAck_Type : public VS_AsnChoice
{
	 VS_H245MaintenanceLoopAck_Type( void );

 	enum{
	e_systemLoop,
	e_mediaLoop,
	e_logicalChannelLoop
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MaintenanceLoopAck_Type & src);

	 operator VS_H245LogicalChannelNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MaintenanceLoopAck /////////////////////////

struct VS_H245MaintenanceLoopAck : public VS_AsnSequence
{
	 VS_H245MaintenanceLoopAck( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245MaintenanceLoopAck_Type	 type ;
 	void operator=(const VS_H245MaintenanceLoopAck& src);

};
//////////////////////CLASS VS_H245MaintenanceLoopRequest_Type /////////////////////////

struct VS_H245MaintenanceLoopRequest_Type : public VS_AsnChoice
{
	 VS_H245MaintenanceLoopRequest_Type( void );

 	enum{
	e_systemLoop,
	e_mediaLoop,
	e_logicalChannelLoop
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MaintenanceLoopRequest_Type & src);

	 operator VS_H245LogicalChannelNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MaintenanceLoopRequest /////////////////////////

struct VS_H245MaintenanceLoopRequest : public VS_AsnSequence
{
	 VS_H245MaintenanceLoopRequest( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245MaintenanceLoopRequest_Type	 type ;
 	void operator=(const VS_H245MaintenanceLoopRequest& src);

};
//////////////////////CLASS VS_H245RoundTripDelayResponse /////////////////////////

struct VS_H245RoundTripDelayResponse : public VS_AsnSequence
{
	 VS_H245RoundTripDelayResponse( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	void operator=(const VS_H245RoundTripDelayResponse& src);

};
//////////////////////CLASS VS_H245RoundTripDelayRequest /////////////////////////

struct VS_H245RoundTripDelayRequest : public VS_AsnSequence
{
	 VS_H245RoundTripDelayRequest( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	void operator=(const VS_H245RoundTripDelayRequest& src);

};
//////////////////////CLASS VS_H245EncryptionMode /////////////////////////

struct VS_H245EncryptionMode : public VS_AsnChoice
{
	 VS_H245EncryptionMode( void );

 	enum{
	e_nonStandard,
	e_h233Encryption
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245EncryptionMode & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245DataMode_Application_Nlpid /////////////////////////

struct VS_H245DataMode_Application_Nlpid : public VS_AsnSequence
{
	 VS_H245DataMode_Application_Nlpid( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245DataProtocolCapability  nlpidProtocol ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  nlpidData ;
 	void operator=(const VS_H245DataMode_Application_Nlpid& src);

};

struct VS_H245DataProtocolCapability;
struct VS_H245GenericCapability;
//////////////////////CLASS VS_H245DataMode_Application /////////////////////////

struct VS_H245DataMode_Application : public VS_AsnChoice
{
	 VS_H245DataMode_Application( void );

 	enum{
	e_nonStandard,
	e_t120,
	e_dsm_cc,
	e_userData,
	e_t84,
	e_t434,
	e_h224,
	e_nlpid,
	e_dsvdControl,
	e_h222DataPartitioning,
	e_t30fax,
	e_t140,
	e_t38fax,
	e_genericDataMode
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DataMode_Application & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245DataProtocolCapability *( void );
	 operator VS_H245GenericCapability *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245DataMode /////////////////////////

struct VS_H245DataMode : public VS_AsnSequence
{
	 VS_H245DataMode( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245DataMode_Application	 application ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  bitRate ;
 	void operator=(const VS_H245DataMode& src);

};

//////////////////////CLASS VS_H245G7231AnnexCMode_G723AnnexCAudioMode /////////////////////////

struct VS_H245G7231AnnexCMode_G723AnnexCAudioMode : public VS_AsnSequence
{
	 VS_H245G7231AnnexCMode_G723AnnexCAudioMode( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<27,78,VS_Asn::FixedConstraint,0>  highRateMode0 ;
 	TemplInteger<27,78,VS_Asn::FixedConstraint,0>  highRateMode1 ;
 	TemplInteger<23,66,VS_Asn::FixedConstraint,0>  lowRateMode0 ;
 	TemplInteger<23,66,VS_Asn::FixedConstraint,0>  lowRateMode1 ;
 	TemplInteger<6,17,VS_Asn::FixedConstraint,0>  sidMode0 ;
 	TemplInteger<6,17,VS_Asn::FixedConstraint,0>  sidMode1 ;
 	void operator=(const VS_H245G7231AnnexCMode_G723AnnexCAudioMode& src);

};
//////////////////////CLASS VS_H245G7231AnnexCMode /////////////////////////

struct VS_H245G7231AnnexCMode : public VS_AsnSequence
{
	 VS_H245G7231AnnexCMode( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  maxAl_sduAudioFrames ;
 	 VS_AsnBoolean  silenceSuppression ;
 	VS_H245G7231AnnexCMode_G723AnnexCAudioMode	 g723AnnexCAudioMode ;
 	void operator=(const VS_H245G7231AnnexCMode& src);

};
//////////////////////CLASS VS_H245IS13818AudioMode_AudioLayer /////////////////////////

struct VS_H245IS13818AudioMode_AudioLayer : public VS_AsnChoice
{
	 VS_H245IS13818AudioMode_AudioLayer( void );

 	enum{
	e_audioLayer1,
	e_audioLayer2,
	e_audioLayer3
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245IS13818AudioMode_AudioLayer & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245IS13818AudioMode_AudioSampling /////////////////////////

struct VS_H245IS13818AudioMode_AudioSampling : public VS_AsnChoice
{
	 VS_H245IS13818AudioMode_AudioSampling( void );

 	enum{
	e_audioSampling16k,
	e_audioSampling22k05,
	e_audioSampling24k,
	e_audioSampling32k,
	e_audioSampling44k1,
	e_audioSampling48k
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245IS13818AudioMode_AudioSampling & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245IS13818AudioMode_MultichannelType /////////////////////////

struct VS_H245IS13818AudioMode_MultichannelType : public VS_AsnChoice
{
	 VS_H245IS13818AudioMode_MultichannelType( void );

 	enum{
	e_singleChannel,
	e_twoChannelStereo,
	e_twoChannelDual,
	e_threeChannels2_1,
	e_threeChannels3_0,
	e_fourChannels2_0_2_0,
	e_fourChannels2_2,
	e_fourChannels3_1,
	e_fiveChannels3_0_2_0,
	e_fiveChannels3_2
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245IS13818AudioMode_MultichannelType & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245IS13818AudioMode /////////////////////////

struct VS_H245IS13818AudioMode : public VS_AsnSequence
{
	 VS_H245IS13818AudioMode( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245IS13818AudioMode_AudioLayer	 audioLayer ;
 	VS_H245IS13818AudioMode_AudioSampling	 audioSampling ;
 	VS_H245IS13818AudioMode_MultichannelType	 multichannelType ;
 	 VS_AsnBoolean  lowFrequencyEnhancement ;
 	 VS_AsnBoolean  multilingual ;
 	TemplInteger<1,1130,VS_Asn::FixedConstraint,0>  bitRate ;
 	void operator=(const VS_H245IS13818AudioMode& src);

};
//////////////////////CLASS VS_H245IS11172AudioMode_AudioLayer /////////////////////////

struct VS_H245IS11172AudioMode_AudioLayer : public VS_AsnChoice
{
	 VS_H245IS11172AudioMode_AudioLayer( void );

 	enum{
	e_audioLayer1,
	e_audioLayer2,
	e_audioLayer3
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245IS11172AudioMode_AudioLayer & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245IS11172AudioMode_AudioSampling /////////////////////////

struct VS_H245IS11172AudioMode_AudioSampling : public VS_AsnChoice
{
	 VS_H245IS11172AudioMode_AudioSampling( void );

 	enum{
	e_audioSampling32k,
	e_audioSampling44k1,
	e_audioSampling48k
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245IS11172AudioMode_AudioSampling & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245IS11172AudioMode_MultichannelType /////////////////////////

struct VS_H245IS11172AudioMode_MultichannelType : public VS_AsnChoice
{
	 VS_H245IS11172AudioMode_MultichannelType( void );

 	enum{
	e_singleChannel,
	e_twoChannelStereo,
	e_twoChannelDual
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245IS11172AudioMode_MultichannelType & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245IS11172AudioMode /////////////////////////

struct VS_H245IS11172AudioMode : public VS_AsnSequence
{
	 VS_H245IS11172AudioMode( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245IS11172AudioMode_AudioLayer	 audioLayer ;
 	VS_H245IS11172AudioMode_AudioSampling	 audioSampling ;
 	VS_H245IS11172AudioMode_MultichannelType	 multichannelType ;
 	TemplInteger<1,448,VS_Asn::FixedConstraint,0>  bitRate ;
 	void operator=(const VS_H245IS11172AudioMode& src);

};
//////////////////////CLASS VS_H245AudioMode_G7231 /////////////////////////

struct VS_H245AudioMode_G7231 : public VS_AsnChoice
{
	 VS_H245AudioMode_G7231( void );

 	enum{
	e_noSilenceSuppressionLowRate,
	e_noSilenceSuppressionHighRate,
	e_silenceSuppressionLowRate,
	e_silenceSuppressionHighRate
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245AudioMode_G7231 & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245G729Extensions /////////////////////////

struct VS_H245G729Extensions : public VS_AsnSequence
{
	 VS_H245G729Extensions( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  audioUnit ;
 	 VS_AsnBoolean  annexA ;
 	 VS_AsnBoolean  annexB ;
 	 VS_AsnBoolean  annexD ;
 	 VS_AsnBoolean  annexE ;
 	 VS_AsnBoolean  annexF ;
 	 VS_AsnBoolean  annexG ;
 	 VS_AsnBoolean  annexH ;
 	void operator=(const VS_H245G729Extensions& src);

};
struct VS_H245VBDMode;
//////////////////////CLASS VS_H245AudioMode /////////////////////////

struct VS_H245AudioMode : public VS_AsnChoice
{
	 VS_H245AudioMode( void );

 	enum{
	e_nonStandard,
	e_g711Alaw64k,
	e_g711Alaw56k,
	e_g711Ulaw64k,
	e_g711Ulaw56k,
	e_g722_64k,
	e_g722_56k,
	e_g722_48k,
	e_g728,
	e_g729,
	e_g729AnnexA,
	e_g7231,
	e_is11172AudioMode,
	e_is13818AudioMode,
	e_g729wAnnexB,
	e_g729AnnexAwAnnexB,
	e_g7231AnnexCMode,
	e_gsmFullRate,
	e_gsmHalfRate,
	e_gsmEnhancedFullRate,
	e_genericAudioMode,
	e_g729Extensions,
	e_vbd
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245AudioMode & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245IS11172AudioMode *( void );
	 operator VS_H245IS13818AudioMode *( void );
	 operator VS_H245G7231AnnexCMode *( void );
	 operator VS_H245GSMAudioCapability *( void );
	 operator VS_H245GenericCapability *( void );
	 operator VS_H245G729Extensions *( void );
	 operator VS_H245VBDMode *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245VBDMode /////////////////////////

struct VS_H245VBDMode : public VS_AsnSequence
{
	 VS_H245VBDMode( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245AudioMode  type ;
 	void operator=(const VS_H245VBDMode& src);

};

//////////////////////CLASS VS_H245IS11172VideoMode /////////////////////////

struct VS_H245IS11172VideoMode : public VS_AsnSequence
{
	 VS_H245IS11172VideoMode( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  constrainedBitstream ;
 	TemplInteger<0,1073741823,VS_Asn::FixedConstraint,0>  videoBitRate ;
 	TemplInteger<0,262143,VS_Asn::FixedConstraint,0>  vbvBufferSize ;
 	TemplInteger<0,16383,VS_Asn::FixedConstraint,0>  samplesPerLine ;
 	TemplInteger<0,16383,VS_Asn::FixedConstraint,0>  linesPerFrame ;
 	TemplInteger<0,15,VS_Asn::FixedConstraint,0>  pictureRate ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  luminanceSampleRate ;
 	void operator=(const VS_H245IS11172VideoMode& src);

};
//////////////////////CLASS VS_H245H263VideoMode_Resolution /////////////////////////

struct VS_H245H263VideoMode_Resolution : public VS_AsnChoice
{
	 VS_H245H263VideoMode_Resolution( void );

 	enum{
	e_sqcif,
	e_qcif,
	e_cif,
	e_cif4,
	e_cif16,
	e_custom
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H263VideoMode_Resolution & src);


	void Show( void ) const;

};


//////////////////////CLASS VS_H245H262VideoMode_ProfileAndLevel /////////////////////////

struct VS_H245H262VideoMode_ProfileAndLevel : public VS_AsnChoice
{
	 VS_H245H262VideoMode_ProfileAndLevel( void );

 	enum{
	e_profileAndLevel_SPatML,
	e_profileAndLevel_MPatLL,
	e_profileAndLevel_MPatML,
	e_profileAndLevel_MPatH_14,
	e_profileAndLevel_MPatHL,
	e_profileAndLevel_SNRatLL,
	e_profileAndLevel_SNRatML,
	e_profileAndLevel_SpatialatH_14,
	e_profileAndLevel_HPatML,
	e_profileAndLevel_HPatH_14,
	e_profileAndLevel_HPatHL
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H262VideoMode_ProfileAndLevel & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H262VideoMode /////////////////////////

struct VS_H245H262VideoMode : public VS_AsnSequence
{
	 VS_H245H262VideoMode( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245H262VideoMode_ProfileAndLevel	 profileAndLevel ;
 	TemplInteger<0,1073741823,VS_Asn::FixedConstraint,0>  videoBitRate ;
 	TemplInteger<0,262143,VS_Asn::FixedConstraint,0>  vbvBufferSize ;
 	TemplInteger<0,16383,VS_Asn::FixedConstraint,0>  samplesPerLine ;
 	TemplInteger<0,16383,VS_Asn::FixedConstraint,0>  linesPerFrame ;
 	TemplInteger<0,15,VS_Asn::FixedConstraint,0>  framesPerSecond ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  luminanceSampleRate ;
 	void operator=(const VS_H245H262VideoMode& src);

};
//////////////////////CLASS VS_H245H261VideoMode_Resolution /////////////////////////

struct VS_H245H261VideoMode_Resolution : public VS_AsnChoice
{
	 VS_H245H261VideoMode_Resolution( void );

 	enum{
	e_qcif,
	e_cif
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H261VideoMode_Resolution & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H261VideoMode /////////////////////////

struct VS_H245H261VideoMode : public VS_AsnSequence
{
	 VS_H245H261VideoMode( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245H261VideoMode_Resolution	 resolution ;
 	TemplInteger<1,19200,VS_Asn::FixedConstraint,0>  bitRate ;
 	 VS_AsnBoolean  stillImageTransmission ;
 	void operator=(const VS_H245H261VideoMode& src);

};
struct VS_H245H263VideoMode;
//////////////////////CLASS VS_H245VideoMode /////////////////////////

struct VS_H245VideoMode : public VS_AsnChoice
{
	 VS_H245VideoMode( void );

 	enum{
	e_nonStandard,
	e_h261VideoMode,
	e_h262VideoMode,
	e_h263VideoMode,
	e_is11172VideoMode,
	e_genericVideoMode
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245VideoMode & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245H261VideoMode *( void );
	 operator VS_H245H262VideoMode *( void );
	 operator VS_H245H263VideoMode *( void );
	 operator VS_H245IS11172VideoMode *( void );
	 operator VS_H245GenericCapability *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245RedundancyEncodingMode_SecondaryEncoding /////////////////////////

struct VS_H245RedundancyEncodingMode_SecondaryEncoding : public VS_AsnChoice
{
	 VS_H245RedundancyEncodingMode_SecondaryEncoding( void );

 	enum{
	e_nonStandard,
	e_audioData
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RedundancyEncodingMode_SecondaryEncoding & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245AudioMode *( void );

	void Show( void ) const;

};


//////////////////////CLASS VS_H245V76ModeParameters /////////////////////////

struct VS_H245V76ModeParameters : public VS_AsnChoice
{
	 VS_H245V76ModeParameters( void );

 	enum{
	e_suspendResumewAddress,
	e_suspendResumewoAddress
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245V76ModeParameters & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223ModeParameters_AdaptationLayerType_Al3 /////////////////////////

struct VS_H245H223ModeParameters_AdaptationLayerType_Al3 : public VS_AsnSequence
{
	 VS_H245H223ModeParameters_AdaptationLayerType_Al3( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,2,VS_Asn::FixedConstraint,0>  controlFieldOctets ;
 	TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>  sendBufferSize ;
 	void operator=(const VS_H245H223ModeParameters_AdaptationLayerType_Al3& src);

};
struct VS_H245H223AL1MParameters;
struct VS_H245H223AL2MParameters;
struct VS_H245H223AL3MParameters;
//////////////////////CLASS VS_H245H223ModeParameters_AdaptationLayerType /////////////////////////

struct VS_H245H223ModeParameters_AdaptationLayerType : public VS_AsnChoice
{
	 VS_H245H223ModeParameters_AdaptationLayerType( void );

 	enum{
	e_nonStandard,
	e_al1Framed,
	e_al1NotFramed,
	e_al2WithoutSequenceNumbers,
	e_al2WithSequenceNumbers,
	e_al3,
	e_al1M,
	e_al2M,
	e_al3M
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223ModeParameters_AdaptationLayerType & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245H223AL1MParameters *( void );
	 operator VS_H245H223AL2MParameters *( void );
	 operator VS_H245H223AL3MParameters *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223ModeParameters /////////////////////////

struct VS_H245H223ModeParameters : public VS_AsnSequence
{
	 VS_H245H223ModeParameters( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245H223ModeParameters_AdaptationLayerType	 adaptationLayerType ;
 	 VS_AsnBoolean  segmentableFlag ;
 	void operator=(const VS_H245H223ModeParameters& src);

};
//////////////////////CLASS VS_H245FECMode_Rfc2733Format /////////////////////////

struct VS_H245FECMode_Rfc2733Format : public VS_AsnChoice
{
	 VS_H245FECMode_Rfc2733Format( void );

 	enum{
	e_rfc2733rfc2198,
	e_rfc2733sameport,
	e_rfc2733diffport
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FECMode_Rfc2733Format & src);

	 operator VS_H245MaxRedundancy *( void );

	void Show( void ) const;

};


//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort /////////////////////////

struct VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort : public VS_AsnSequence
{
	 VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  protectedSessionID ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  protectedPayloadType ;
 	void operator=(const VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort& src);

};

//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream /////////////////////////

struct VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream : public VS_AsnChoice
{
	 VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream( void );

 	enum{
	e_differentPort,
	e_samePort
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode_Mode /////////////////////////

struct VS_H245DepFECMode_Rfc2733Mode_Mode : public VS_AsnChoice
{
	 VS_H245DepFECMode_Rfc2733Mode_Mode( void );

 	enum{
	e_redundancyEncoding,
	e_separateStream
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DepFECMode_Rfc2733Mode_Mode & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode /////////////////////////

struct VS_H245DepFECMode_Rfc2733Mode : public VS_AsnSequence
{
	 VS_H245DepFECMode_Rfc2733Mode( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245DepFECMode_Rfc2733Mode_Mode	 mode ;
 	void operator=(const VS_H245DepFECMode_Rfc2733Mode& src);

};
//////////////////////CLASS VS_H245DepFECMode /////////////////////////

struct VS_H245DepFECMode : public VS_AsnChoice
{
	 VS_H245DepFECMode( void );

 	enum{
	e_rfc2733Mode
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DepFECMode & src);


	void Show( void ) const;

};


struct VS_H245H235Mode;
struct VS_H245FECMode;
//////////////////////CLASS VS_H245RedundancyEncodingDTModeElement_Type /////////////////////////

struct VS_H245RedundancyEncodingDTModeElement_Type : public VS_AsnChoice
{
	 VS_H245RedundancyEncodingDTModeElement_Type( void );

 	enum{
	e_nonStandard,
	e_videoMode,
	e_audioMode,
	e_dataMode,
	e_encryptionMode,
	e_h235Mode,
	e_fecMode
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RedundancyEncodingDTModeElement_Type & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245VideoMode *( void );
	 operator VS_H245AudioMode *( void );
	 operator VS_H245DataMode *( void );
	 operator VS_H245EncryptionMode *( void );
	 operator VS_H245H235Mode *( void );
	 operator VS_H245FECMode *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245RedundancyEncodingDTModeElement /////////////////////////

struct VS_H245RedundancyEncodingDTModeElement : public VS_AsnSequence
{
	 VS_H245RedundancyEncodingDTModeElement( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245RedundancyEncodingDTModeElement_Type	 type ;
 	void operator=(const VS_H245RedundancyEncodingDTModeElement& src);

};

//////////////////////CLASS VS_H245MultiplexedStreamModeParameters /////////////////////////

struct VS_H245MultiplexedStreamModeParameters : public VS_AsnSequence
{
	 VS_H245MultiplexedStreamModeParameters( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	void operator=(const VS_H245MultiplexedStreamModeParameters& src);

};
//////////////////////CLASS VS_H245H235Mode_MediaMode /////////////////////////

struct VS_H245H235Mode_MediaMode : public VS_AsnChoice
{
	 VS_H245H235Mode_MediaMode( void );

 	enum{
	e_nonStandard,
	e_videoMode,
	e_audioMode,
	e_dataMode
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H235Mode_MediaMode & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245VideoMode *( void );
	 operator VS_H245AudioMode *( void );
	 operator VS_H245DataMode *( void );

	void Show( void ) const;

};

struct VS_H245RedundancyEncodingDTMode;
struct VS_H245MultiplexedStreamParameter;
struct VS_H245MultiplePayloadStreamMode;

//////////////////////CLASS VS_H245ModeElementType /////////////////////////

struct VS_H245ModeElementType : public VS_AsnChoice
{
	 VS_H245ModeElementType( void );

 	enum{
	e_nonStandard,
	e_videoMode,
	e_audioMode,
	e_dataMode,
	e_encryptionMode,
	e_h235Mode,
	e_multiplexedStreamMode,
	e_redundancyEncodingDTMode,
	e_multiplePayloadStreamMode,
	e_depFfecMode,
	e_fecMode
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ModeElementType & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245VideoMode *( void );
	 operator VS_H245AudioMode *( void );
	 operator VS_H245DataMode *( void );
	 operator VS_H245EncryptionMode *( void );
	 operator VS_H245H235Mode *( void );
	 operator VS_H245MultiplexedStreamParameter *( void );
	 operator VS_H245RedundancyEncodingDTMode *( void );
	 operator VS_H245MultiplePayloadStreamMode *( void );
	 operator VS_H245DepFECMode *( void );
	 operator VS_H245FECMode *( void );

	void Show( void ) const;

};
//////////////////////CLASS VS_H245MultiplePayloadStreamElementMode /////////////////////////

struct VS_H245MultiplePayloadStreamElementMode : public VS_AsnSequence
{
	 VS_H245MultiplePayloadStreamElementMode( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245ModeElementType  type ;
 	void operator=(const VS_H245MultiplePayloadStreamElementMode& src);

};

//////////////////////CLASS VS_H245MultiplePayloadStreamMode /////////////////////////

struct VS_H245MultiplePayloadStreamMode : public VS_AsnSequence
{
	 VS_H245MultiplePayloadStreamMode( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245MultiplePayloadStreamElementMode ,0,INT_MAX,VS_Asn::Unconstrained,0  >  elements ;
 	void operator=(const VS_H245MultiplePayloadStreamMode& src);

};


//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort /////////////////////////

struct VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort : public VS_AsnSequence
{
	 VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245ModeElementType  protectedType ;
 	void operator=(const VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort& src);

};

//////////////////////CLASS VS_H245FECMode /////////////////////////

struct VS_H245FECMode : public VS_AsnSequence
{
	 VS_H245FECMode( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245ModeElementType  protectedElement ;
 	 VS_AsnObjectId  fecScheme ;
 	VS_H245FECMode_Rfc2733Format	 rfc2733Format ;
 	void operator=(const VS_H245FECMode& src);

};
//////////////////////CLASS VS_H245RequestModeRelease /////////////////////////

struct VS_H245RequestModeRelease : public VS_AsnSequence
{
	 VS_H245RequestModeRelease( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245RequestModeRelease& src);

};
//////////////////////CLASS VS_H245RequestModeReject_Cause /////////////////////////

struct VS_H245RequestModeReject_Cause : public VS_AsnChoice
{
	 VS_H245RequestModeReject_Cause( void );

 	enum{
	e_modeUnavailable,
	e_multipointConstraint,
	e_requestDenied
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RequestModeReject_Cause & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245RequestModeReject /////////////////////////

struct VS_H245RequestModeReject : public VS_AsnSequence
{
	 VS_H245RequestModeReject( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	VS_H245RequestModeReject_Cause	 cause ;
 	void operator=(const VS_H245RequestModeReject& src);

};
//////////////////////CLASS VS_H245RequestModeAck_Response /////////////////////////

struct VS_H245RequestModeAck_Response : public VS_AsnChoice
{
	 VS_H245RequestModeAck_Response( void );

 	enum{
	e_willTransmitMostPreferredMode,
	e_willTransmitLessPreferredMode
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RequestModeAck_Response & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245RequestModeAck /////////////////////////

struct VS_H245RequestModeAck : public VS_AsnSequence
{
	 VS_H245RequestModeAck( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	VS_H245RequestModeAck_Response	 response ;
 	void operator=(const VS_H245RequestModeAck& src);

};

//////////////////////CLASS VS_H245RequestMultiplexEntryRelease /////////////////////////

struct VS_H245RequestMultiplexEntryRelease : public VS_AsnSequence
{
	 VS_H245RequestMultiplexEntryRelease( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245MultiplexTableEntryNumber ,1,15,VS_Asn::FixedConstraint,0  >  entryNumbers ;
 	void operator=(const VS_H245RequestMultiplexEntryRelease& src);

};
//////////////////////CLASS VS_H245RequestMultiplexEntryRejectionDescriptions_Cause /////////////////////////

struct VS_H245RequestMultiplexEntryRejectionDescriptions_Cause : public VS_AsnChoice
{
	 VS_H245RequestMultiplexEntryRejectionDescriptions_Cause( void );

 	enum{
	e_unspecifiedCause
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RequestMultiplexEntryRejectionDescriptions_Cause & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245RequestMultiplexEntryRejectionDescriptions /////////////////////////

struct VS_H245RequestMultiplexEntryRejectionDescriptions : public VS_AsnSequence
{
	 VS_H245RequestMultiplexEntryRejectionDescriptions( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245MultiplexTableEntryNumber  multiplexTableEntryNumber ;
 	VS_H245RequestMultiplexEntryRejectionDescriptions_Cause	 cause ;
 	void operator=(const VS_H245RequestMultiplexEntryRejectionDescriptions& src);

};
//////////////////////CLASS VS_H245RequestMultiplexEntryReject /////////////////////////

struct VS_H245RequestMultiplexEntryReject : public VS_AsnSequence
{
	 VS_H245RequestMultiplexEntryReject( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245MultiplexTableEntryNumber ,1,15,VS_Asn::FixedConstraint,0  >  entryNumbers ;
 	Constrained_array_of_type<  VS_H245RequestMultiplexEntryRejectionDescriptions ,1,15,VS_Asn::FixedConstraint,0  >  rejectionDescriptions ;
 	void operator=(const VS_H245RequestMultiplexEntryReject& src);

};
//////////////////////CLASS VS_H245RequestMultiplexEntryAck /////////////////////////

struct VS_H245RequestMultiplexEntryAck : public VS_AsnSequence
{
	 VS_H245RequestMultiplexEntryAck( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245MultiplexTableEntryNumber ,1,15,VS_Asn::FixedConstraint,0  >  entryNumbers ;
 	void operator=(const VS_H245RequestMultiplexEntryAck& src);

};
//////////////////////CLASS VS_H245RequestMultiplexEntry /////////////////////////

struct VS_H245RequestMultiplexEntry : public VS_AsnSequence
{
	 VS_H245RequestMultiplexEntry( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245MultiplexTableEntryNumber ,1,15,VS_Asn::FixedConstraint,0  >  entryNumbers ;
 	void operator=(const VS_H245RequestMultiplexEntry& src);

};
//////////////////////CLASS VS_H245MultiplexEntrySendRelease /////////////////////////

struct VS_H245MultiplexEntrySendRelease : public VS_AsnSequence
{
	 VS_H245MultiplexEntrySendRelease( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245MultiplexTableEntryNumber ,1,15,VS_Asn::FixedConstraint,0  >  multiplexTableEntryNumber ;
 	void operator=(const VS_H245MultiplexEntrySendRelease& src);

};
//////////////////////CLASS VS_H245MultiplexEntryRejectionDescriptions_Cause /////////////////////////

struct VS_H245MultiplexEntryRejectionDescriptions_Cause : public VS_AsnChoice
{
	 VS_H245MultiplexEntryRejectionDescriptions_Cause( void );

 	enum{
	e_unspecifiedCause,
	e_descriptorTooComplex
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultiplexEntryRejectionDescriptions_Cause & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultiplexEntryRejectionDescriptions /////////////////////////

struct VS_H245MultiplexEntryRejectionDescriptions : public VS_AsnSequence
{
	 VS_H245MultiplexEntryRejectionDescriptions( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245MultiplexTableEntryNumber  multiplexTableEntryNumber ;
 	VS_H245MultiplexEntryRejectionDescriptions_Cause	 cause ;
 	void operator=(const VS_H245MultiplexEntryRejectionDescriptions& src);

};
//////////////////////CLASS VS_H245MultiplexEntrySendReject /////////////////////////

struct VS_H245MultiplexEntrySendReject : public VS_AsnSequence
{
	 VS_H245MultiplexEntrySendReject( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	Constrained_array_of_type<  VS_H245MultiplexEntryRejectionDescriptions ,1,15,VS_Asn::FixedConstraint,0  >  rejectionDescriptions ;
 	void operator=(const VS_H245MultiplexEntrySendReject& src);

};
//////////////////////CLASS VS_H245MultiplexEntrySendAck /////////////////////////

struct VS_H245MultiplexEntrySendAck : public VS_AsnSequence
{
	 VS_H245MultiplexEntrySendAck( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	Constrained_array_of_type<  VS_H245MultiplexTableEntryNumber ,1,15,VS_Asn::FixedConstraint,0  >  multiplexTableEntryNumber ;
 	void operator=(const VS_H245MultiplexEntrySendAck& src);

};

struct VS_H245MultiplexElement;
//////////////////////CLASS VS_H245MultiplexElement_Type /////////////////////////

struct VS_H245MultiplexElement_Type : public VS_AsnChoice
{
	 VS_H245MultiplexElement_Type( void );

 	enum{
	e_logicalChannelNumber,
	e_subElementList
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultiplexElement_Type & src);

	 operator VS_H245MultiplexElement *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultiplexElement_RepeatCount /////////////////////////

struct VS_H245MultiplexElement_RepeatCount : public VS_AsnChoice
{
	 VS_H245MultiplexElement_RepeatCount( void );

 	enum{
	e_finite,
	e_untilClosingFlag
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultiplexElement_RepeatCount & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultiplexElement /////////////////////////

struct VS_H245MultiplexElement : public VS_AsnSequence
{
	 VS_H245MultiplexElement( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245MultiplexElement_Type	 type ;
 	VS_H245MultiplexElement_RepeatCount	 repeatCount ;
 	void operator=(const VS_H245MultiplexElement& src);

};
//////////////////////CLASS VS_H245MultiplexEntryDescriptor /////////////////////////

struct VS_H245MultiplexEntryDescriptor : public VS_AsnSequence
{
	 VS_H245MultiplexEntryDescriptor( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245MultiplexTableEntryNumber  multiplexTableEntryNumber ;
 	Constrained_array_of_type<  VS_H245MultiplexElement ,1,256,VS_Asn::FixedConstraint,0  >  elementList ;
 	void operator=(const VS_H245MultiplexEntryDescriptor& src);

};
//////////////////////CLASS VS_H245MultiplexEntrySend /////////////////////////

struct VS_H245MultiplexEntrySend : public VS_AsnSequence
{
	 VS_H245MultiplexEntrySend( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	Constrained_array_of_type<  VS_H245MultiplexEntryDescriptor ,1,15,VS_Asn::FixedConstraint,0  >  multiplexEntryDescriptors ;
 	void operator=(const VS_H245MultiplexEntrySend& src);

};
//////////////////////CLASS VS_H245RequestChannelCloseRelease /////////////////////////

struct VS_H245RequestChannelCloseRelease : public VS_AsnSequence
{
	 VS_H245RequestChannelCloseRelease( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	void operator=(const VS_H245RequestChannelCloseRelease& src);

};
//////////////////////CLASS VS_H245RequestChannelCloseReject_Cause /////////////////////////

struct VS_H245RequestChannelCloseReject_Cause : public VS_AsnChoice
{
	 VS_H245RequestChannelCloseReject_Cause( void );

 	enum{
	e_unspecified
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RequestChannelCloseReject_Cause & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245RequestChannelCloseReject /////////////////////////

struct VS_H245RequestChannelCloseReject : public VS_AsnSequence
{
	 VS_H245RequestChannelCloseReject( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	VS_H245RequestChannelCloseReject_Cause	 cause ;
 	void operator=(const VS_H245RequestChannelCloseReject& src);

};
//////////////////////CLASS VS_H245RequestChannelCloseAck /////////////////////////

struct VS_H245RequestChannelCloseAck : public VS_AsnSequence
{
	 VS_H245RequestChannelCloseAck( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	void operator=(const VS_H245RequestChannelCloseAck& src);

};
//////////////////////CLASS VS_H245RequestChannelClose_Reason /////////////////////////

struct VS_H245RequestChannelClose_Reason : public VS_AsnChoice
{
	 VS_H245RequestChannelClose_Reason( void );

 	enum{
	e_unknown,
	e_normal,
	e_reopen,
	e_reservationFailure
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RequestChannelClose_Reason & src);


	void Show( void ) const;

};


//////////////////////CLASS VS_H245CloseLogicalChannelAck /////////////////////////

struct VS_H245CloseLogicalChannelAck : public VS_AsnSequence
{
	 VS_H245CloseLogicalChannelAck( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	void operator=(const VS_H245CloseLogicalChannelAck& src);

};
//////////////////////CLASS VS_H245CloseLogicalChannel_Source /////////////////////////

struct VS_H245CloseLogicalChannel_Source : public VS_AsnChoice
{
	 VS_H245CloseLogicalChannel_Source( void );

 	enum{
	e_user,
	e_lcse
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CloseLogicalChannel_Source & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245CloseLogicalChannel_Reason /////////////////////////

struct VS_H245CloseLogicalChannel_Reason : public VS_AsnChoice
{
	 VS_H245CloseLogicalChannel_Reason( void );

 	enum{
	e_unknown,
	e_reopen,
	e_reservationFailure
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CloseLogicalChannel_Reason & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245CloseLogicalChannel /////////////////////////

struct VS_H245CloseLogicalChannel : public VS_AsnSequence
{
	 VS_H245CloseLogicalChannel( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	VS_H245CloseLogicalChannel_Source	 source ;
 	VS_H245CloseLogicalChannel_Reason	 reason ;
 	void operator=(const VS_H245CloseLogicalChannel& src);

};
//////////////////////CLASS VS_H245H2250LogicalChannelAckParameters /////////////////////////

struct VS_H245H2250LogicalChannelAckParameters : public VS_AsnSequence
{
	 VS_H245H2250LogicalChannelAckParameters( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	Constrained_array_of_type<  VS_H245NonStandardParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  nonStandard ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  sessionID ;
 	 VS_H245TransportAddress  mediaChannel ;
 	 VS_H245TransportAddress  mediaControlChannel ;
 	TemplInteger<96,127,VS_Asn::FixedConstraint,0>  dynamicRTPPayloadType ;
 	 VS_AsnBoolean  flowControlToZero ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  portNumber ;
 	void operator=(const VS_H245H2250LogicalChannelAckParameters& src);

};
//////////////////////CLASS VS_H245OpenLogicalChannelConfirm /////////////////////////

struct VS_H245OpenLogicalChannelConfirm : public VS_AsnSequence
{
	 VS_H245OpenLogicalChannelConfirm( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	void operator=(const VS_H245OpenLogicalChannelConfirm& src);

};
//////////////////////CLASS VS_H245OpenLogicalChannelReject_Cause /////////////////////////

struct VS_H245OpenLogicalChannelReject_Cause : public VS_AsnChoice
{
	 VS_H245OpenLogicalChannelReject_Cause( void );

 	enum{
	e_unspecified,
	e_unsuitableReverseParameters,
	e_dataTypeNotSupported,
	e_dataTypeNotAvailable,
	e_unknownDataType,
	e_dataTypeALCombinationNotSupported,
	e_multicastChannelNotAllowed,
	e_insufficientBandwidth,
	e_separateStackEstablishmentFailed,
	e_invalidSessionID,
	e_masterSlaveConflict,
	e_waitForCommunicationMode,
	e_invalidDependentChannel,
	e_replacementForRejected,
	e_securityDenied
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245OpenLogicalChannelReject_Cause & src);


	void Show( void ) const ;

};

//////////////////////CLASS VS_H245OpenLogicalChannelReject /////////////////////////

struct VS_H245OpenLogicalChannelReject : public VS_AsnSequence
{
	 VS_H245OpenLogicalChannelReject( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	VS_H245OpenLogicalChannelReject_Cause	 cause ;
 	void operator=(const VS_H245OpenLogicalChannelReject& src);

};

struct VS_H245H222LogicalChannelParameters;
struct VS_H245H2250LogicalChannelParameters;
//////////////////////CLASS VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters /////////////////////////

struct VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters : public VS_AsnChoice
{
	 VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters( void );

 	enum{
	e_h222LogicalChannelParameters,
	e_h2250LogicalChannelParameters
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters & src);

	 operator VS_H245H222LogicalChannelParameters *( void );
	 operator VS_H245H2250LogicalChannelParameters *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters /////////////////////////

struct VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters : public VS_AsnSequence
{
	 VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245LogicalChannelNumber  reverseLogicalChannelNumber ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  portNumber ;
 	VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters	 multiplexParameters ;
 	 VS_H245LogicalChannelNumber  replacementFor ;
 	void operator=(const VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters& src);

};
//////////////////////CLASS VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters /////////////////////////

struct VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters : public VS_AsnChoice
{
	 VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters( void );

 	enum{
	e_h2250LogicalChannelAckParameters
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters & src);

	operator VS_H245H2250LogicalChannelAckParameters *( void );
	void operator=( VS_H245H2250LogicalChannelAckParameters *h225lcap );

	void Show( void ) const;

};


//////////////////////CLASS VS_H245EscrowData /////////////////////////

struct VS_H245EscrowData : public VS_AsnSequence
{
	 VS_H245EscrowData( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnObjectId  escrowID ;
 	TemplBitString<1,65535,VS_Asn::FixedConstraint,0>  escrowValue ;
 	void operator=(const VS_H245EscrowData& src);

};


//////////////////////CLASS VS_H245MulticastAddress_IPAddress /////////////////////////

struct VS_H245MulticastAddress_IPAddress : public VS_AsnSequence
{
	 VS_H245MulticastAddress_IPAddress( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<4,4,VS_Asn::FixedConstraint,0>  network ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  tsapIdentifier ;
 	void operator=(const VS_H245MulticastAddress_IPAddress& src);

};
//////////////////////CLASS VS_H245MulticastAddress_IP6Address /////////////////////////

struct VS_H245MulticastAddress_IP6Address : public VS_AsnSequence
{
	 VS_H245MulticastAddress_IP6Address( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  network ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  tsapIdentifier ;
 	void operator=(const VS_H245MulticastAddress_IP6Address& src);

};


//////////////////////CLASS VS_H245UnicastAddress_IPAddress /////////////////////////

struct VS_H245UnicastAddress_IPAddress : public VS_AsnSequence
{
	 VS_H245UnicastAddress_IPAddress( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<4,4,VS_Asn::FixedConstraint,0>  network ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  tsapIdentifier ;
 	void operator=(const VS_H245UnicastAddress_IPAddress& src);

};
//////////////////////CLASS VS_H245UnicastAddress_IPXAddress /////////////////////////

struct VS_H245UnicastAddress_IPXAddress : public VS_AsnSequence
{
	 VS_H245UnicastAddress_IPXAddress( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<6,6,VS_Asn::FixedConstraint,0>  node ;
 	TemplOctetString<4,4,VS_Asn::FixedConstraint,0>  netnum ;
 	TemplOctetString<2,2,VS_Asn::FixedConstraint,0>  tsapIdentifier ;
 	void operator=(const VS_H245UnicastAddress_IPXAddress& src);

};
//////////////////////CLASS VS_H245UnicastAddress_IP6Address /////////////////////////

struct VS_H245UnicastAddress_IP6Address : public VS_AsnSequence
{
	 VS_H245UnicastAddress_IP6Address( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  network ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  tsapIdentifier ;
 	void operator=(const VS_H245UnicastAddress_IP6Address& src);

};
//////////////////////CLASS VS_H245UnicastAddress_IPSourceRouteAddress_Routing /////////////////////////

struct VS_H245UnicastAddress_IPSourceRouteAddress_Routing : public VS_AsnChoice
{
	 VS_H245UnicastAddress_IPSourceRouteAddress_Routing( void );

 	enum{
	e_strict,
	e_loose
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245UnicastAddress_IPSourceRouteAddress_Routing & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245UnicastAddress_IPSourceRouteAddress /////////////////////////

struct VS_H245UnicastAddress_IPSourceRouteAddress : public VS_AsnSequence
{
	 VS_H245UnicastAddress_IPSourceRouteAddress( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245UnicastAddress_IPSourceRouteAddress_Routing	 routing ;
 	TemplOctetString<4,4,VS_Asn::FixedConstraint,0>  network ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  tsapIdentifier ;
 	Constrained_array_of_type< TemplOctetString<4,4,VS_Asn::FixedConstraint,0> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  route ;
 	void operator=(const VS_H245UnicastAddress_IPSourceRouteAddress& src);

};




//////////////////////CLASS VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport /////////////////////////

struct VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport : public VS_AsnSequence
{
	 VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport& src);

};
//////////////////////CLASS VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport /////////////////////////

struct VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport : public VS_AsnSequence
{
	 VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245LogicalChannelNumber  protectedChannel ;
 	void operator=(const VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport& src);

};
//////////////////////CLASS VS_H245FECData_Rfc2733_PktMode /////////////////////////

struct VS_H245FECData_Rfc2733_PktMode : public VS_AsnChoice
{
	 VS_H245FECData_Rfc2733_PktMode( void );

 	enum{
	e_rfc2198coding,
	e_rfc2733sameport,
	e_rfc2733diffport
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FECData_Rfc2733_PktMode & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245FECData_Rfc2733 /////////////////////////

struct VS_H245FECData_Rfc2733 : public VS_AsnSequence
{
	 VS_H245FECData_Rfc2733( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  protectedPayloadType ;
 	 VS_AsnObjectId  fecScheme ;
 	VS_H245FECData_Rfc2733_PktMode	 pktMode ;
 	void operator=(const VS_H245FECData_Rfc2733& src);

};
//////////////////////CLASS VS_H245FECData /////////////////////////

struct VS_H245FECData : public VS_AsnChoice
{
	 VS_H245FECData( void );

 	enum{
	e_rfc2733
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FECData & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort /////////////////////////

struct VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort : public VS_AsnSequence
{
	 VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  protectedSessionID ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  protectedPayloadType ;
 	void operator=(const VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort& src);

};
//////////////////////CLASS VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort /////////////////////////

struct VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort : public VS_AsnSequence
{
	 VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  protectedPayloadType ;
 	void operator=(const VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort& src);

};
//////////////////////CLASS VS_H245DepFECData_Rfc2733_Mode_SeparateStream /////////////////////////

struct VS_H245DepFECData_Rfc2733_Mode_SeparateStream : public VS_AsnChoice
{
	 VS_H245DepFECData_Rfc2733_Mode_SeparateStream( void );

 	enum{
	e_differentPort,
	e_samePort
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DepFECData_Rfc2733_Mode_SeparateStream & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245DepFECData_Rfc2733_Mode /////////////////////////

struct VS_H245DepFECData_Rfc2733_Mode : public VS_AsnChoice
{
	 VS_H245DepFECData_Rfc2733_Mode( void );

 	enum{
	e_redundancyEncoding,
	e_separateStream
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DepFECData_Rfc2733_Mode & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245DepFECData_Rfc2733 /////////////////////////

struct VS_H245DepFECData_Rfc2733 : public VS_AsnSequence
{
	 VS_H245DepFECData_Rfc2733( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245DepFECData_Rfc2733_Mode	 mode ;
 	void operator=(const VS_H245DepFECData_Rfc2733& src);

};
//////////////////////CLASS VS_H245DepFECData /////////////////////////

struct VS_H245DepFECData : public VS_AsnChoice
{
	 VS_H245DepFECData( void );

 	enum{
	e_rfc2733
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DepFECData & src);


	void Show( void ) const;

};




//////////////////////CLASS VS_H245RTPPayloadType_PayloadDescriptor /////////////////////////

struct VS_H245RTPPayloadType_PayloadDescriptor : public VS_AsnChoice
{
	 VS_H245RTPPayloadType_PayloadDescriptor( void );

 	enum{
	e_nonStandardIdentifier,
	e_rfc_number,
	e_oid
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RTPPayloadType_PayloadDescriptor & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245RTPPayloadType /////////////////////////

struct VS_H245RTPPayloadType : public VS_AsnSequence
{
	 VS_H245RTPPayloadType( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245RTPPayloadType_PayloadDescriptor	 payloadDescriptor ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  payloadType ;
 	void operator=(const VS_H245RTPPayloadType& src);

};
//////////////////////CLASS VS_H245H2250LogicalChannelParameters_MediaPacketization /////////////////////////

struct VS_H245H2250LogicalChannelParameters_MediaPacketization : public VS_AsnChoice
{
	 VS_H245H2250LogicalChannelParameters_MediaPacketization( void );

 	enum{
	e_h261aVideoPacketization,
	e_rtpPayloadType
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H2250LogicalChannelParameters_MediaPacketization & src);

	 operator VS_H245RTPPayloadType *( void );

	void Show( void ) const;

};


//////////////////////CLASS VS_H245CRCLength /////////////////////////

struct VS_H245CRCLength : public VS_AsnChoice
{
	 VS_H245CRCLength( void );

 	enum{
	e_crc8bit,
	e_crc16bit,
	e_crc32bit
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CRCLength & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245V76HDLCParameters /////////////////////////

struct VS_H245V76HDLCParameters : public VS_AsnSequence
{
	 VS_H245V76HDLCParameters( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245CRCLength  crcLength ;
 	TemplInteger<1,4095,VS_Asn::FixedConstraint,0>  n401 ;
 	 VS_AsnBoolean  loopbackTestProcedure ;
 	void operator=(const VS_H245V76HDLCParameters& src);

};
//////////////////////CLASS VS_H245V76LogicalChannelParameters_SuspendResume /////////////////////////

struct VS_H245V76LogicalChannelParameters_SuspendResume : public VS_AsnChoice
{
	 VS_H245V76LogicalChannelParameters_SuspendResume( void );

 	enum{
	e_noSuspendResume,
	e_suspendResumewAddress,
	e_suspendResumewoAddress
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245V76LogicalChannelParameters_SuspendResume & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery /////////////////////////

struct VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery : public VS_AsnChoice
{
	 VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery( void );

 	enum{
	e_rej,
	e_sREJ,
	e_mSREJ
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245V76LogicalChannelParameters_Mode_ERM /////////////////////////

struct VS_H245V76LogicalChannelParameters_Mode_ERM : public VS_AsnSequence
{
	 VS_H245V76LogicalChannelParameters_Mode_ERM( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,127,VS_Asn::FixedConstraint,0>  windowSize ;
 	VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery	 recovery ;
 	void operator=(const VS_H245V76LogicalChannelParameters_Mode_ERM& src);

};
//////////////////////CLASS VS_H245V76LogicalChannelParameters_Mode /////////////////////////

struct VS_H245V76LogicalChannelParameters_Mode : public VS_AsnChoice
{
	 VS_H245V76LogicalChannelParameters_Mode( void );

 	enum{
	e_eRM,
	e_uNERM
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245V76LogicalChannelParameters_Mode & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245V76LogicalChannelParameters /////////////////////////

struct VS_H245V76LogicalChannelParameters : public VS_AsnSequence
{
	 VS_H245V76LogicalChannelParameters( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245V76HDLCParameters  hdlcParameters ;
 	VS_H245V76LogicalChannelParameters_SuspendResume	 suspendResume ;
 	 VS_AsnBoolean  uIH ;
 	VS_H245V76LogicalChannelParameters_Mode	 mode ;
 	 VS_H245V75Parameters  v75Parameters ;
 	void operator=(const VS_H245V76LogicalChannelParameters& src);

};
//////////////////////CLASS VS_H245H223AnnexCArqParameters_NumberOfRetransmissions /////////////////////////

struct VS_H245H223AnnexCArqParameters_NumberOfRetransmissions : public VS_AsnChoice
{
	 VS_H245H223AnnexCArqParameters_NumberOfRetransmissions( void );

 	enum{
	e_finite,
	e_infinite
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AnnexCArqParameters_NumberOfRetransmissions & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223AnnexCArqParameters /////////////////////////

struct VS_H245H223AnnexCArqParameters : public VS_AsnSequence
{
	 VS_H245H223AnnexCArqParameters( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245H223AnnexCArqParameters_NumberOfRetransmissions	 numberOfRetransmissions ;
 	TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>  sendBufferSize ;
 	void operator=(const VS_H245H223AnnexCArqParameters& src);

};
//////////////////////CLASS VS_H245H223AL3MParameters_HeaderFormat /////////////////////////

struct VS_H245H223AL3MParameters_HeaderFormat : public VS_AsnChoice
{
	 VS_H245H223AL3MParameters_HeaderFormat( void );

 	enum{
	e_sebch16_7,
	e_golay24_12
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AL3MParameters_HeaderFormat & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223AL3MParameters_CrcLength /////////////////////////

struct VS_H245H223AL3MParameters_CrcLength : public VS_AsnChoice
{
	 VS_H245H223AL3MParameters_CrcLength( void );

 	enum{
	e_crc4bit,
	e_crc12bit,
	e_crc20bit,
	e_crc28bit,
	e_crc8bit,
	e_crc16bit,
	e_crc32bit,
	e_crcNotUsed
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AL3MParameters_CrcLength & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223AL3MParameters_ArqType /////////////////////////

struct VS_H245H223AL3MParameters_ArqType : public VS_AsnChoice
{
	 VS_H245H223AL3MParameters_ArqType( void );

 	enum{
	e_noArq,
	e_typeIArq,
	e_typeIIArq
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AL3MParameters_ArqType & src);

	 operator VS_H245H223AnnexCArqParameters *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223AL3MParameters /////////////////////////

struct VS_H245H223AL3MParameters : public VS_AsnSequence
{
	 VS_H245H223AL3MParameters( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H245H223AL3MParameters_HeaderFormat	 headerFormat ;
 	VS_H245H223AL3MParameters_CrcLength	 crcLength ;
 	TemplInteger<8,32,VS_Asn::FixedConstraint,0>  rcpcCodeRate ;
 	VS_H245H223AL3MParameters_ArqType	 arqType ;
 	 VS_AsnBoolean  alpduInterleaving ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  rsCodeCorrection ;
 	void operator=(const VS_H245H223AL3MParameters& src);

};
//////////////////////CLASS VS_H245H223AL2MParameters_HeaderFEC /////////////////////////

struct VS_H245H223AL2MParameters_HeaderFEC : public VS_AsnChoice
{
	 VS_H245H223AL2MParameters_HeaderFEC( void );

 	enum{
	e_sebch16_5,
	e_golay24_12
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AL2MParameters_HeaderFEC & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223AL2MParameters /////////////////////////

struct VS_H245H223AL2MParameters : public VS_AsnSequence
{
	 VS_H245H223AL2MParameters( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245H223AL2MParameters_HeaderFEC	 headerFEC ;
 	 VS_AsnBoolean  alpduInterleaving ;
 	void operator=(const VS_H245H223AL2MParameters& src);

};

//////////////////////CLASS VS_H245H223AL1MParameters_HeaderFEC /////////////////////////

struct VS_H245H223AL1MParameters_HeaderFEC : public VS_AsnChoice
{
	 VS_H245H223AL1MParameters_HeaderFEC( void );

 	enum{
	e_sebch16_7,
	e_golay24_12
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AL1MParameters_HeaderFEC & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223AL1MParameters_CrcLength /////////////////////////

struct VS_H245H223AL1MParameters_CrcLength : public VS_AsnChoice
{
	 VS_H245H223AL1MParameters_CrcLength( void );

 	enum{
	e_crc4bit,
	e_crc12bit,
	e_crc20bit,
	e_crc28bit,
	e_crc8bit,
	e_crc16bit,
	e_crc32bit,
	e_crcNotUsed
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AL1MParameters_CrcLength & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223AL1MParameters_ArqType /////////////////////////

struct VS_H245H223AL1MParameters_ArqType : public VS_AsnChoice
{
	 VS_H245H223AL1MParameters_ArqType( void );

 	enum{
	e_noArq,
	e_typeIArq,
	e_typeIIArq
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223AL1MParameters_ArqType & src);

	 operator VS_H245H223AnnexCArqParameters *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223AL1MParameters /////////////////////////

struct VS_H245H223AL1MParameters : public VS_AsnSequence
{
	 VS_H245H223AL1MParameters( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H245H223AL1MParameters_TransferMode	 transferMode ;
 	VS_H245H223AL1MParameters_HeaderFEC	 headerFEC ;
 	VS_H245H223AL1MParameters_CrcLength	 crcLength ;
 	TemplInteger<8,32,VS_Asn::FixedConstraint,0>  rcpcCodeRate ;
 	VS_H245H223AL1MParameters_ArqType	 arqType ;
 	 VS_AsnBoolean  alpduInterleaving ;
 	 VS_AsnBoolean  alsduSplitting ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  rsCodeCorrection ;
 	void operator=(const VS_H245H223AL1MParameters& src);

};
//////////////////////CLASS VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3 /////////////////////////

struct VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3 : public VS_AsnSequence
{
	 VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,2,VS_Asn::FixedConstraint,0>  controlFieldOctets ;
 	TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>  sendBufferSize ;
 	void operator=(const VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3& src);

};
//////////////////////CLASS VS_H245H223LogicalChannelParameters_AdaptationLayerType /////////////////////////

struct VS_H245H223LogicalChannelParameters_AdaptationLayerType : public VS_AsnChoice
{
	 VS_H245H223LogicalChannelParameters_AdaptationLayerType( void );

 	enum{
	e_nonStandard,
	e_al1Framed,
	e_al1NotFramed,
	e_al2WithoutSequenceNumbers,
	e_al2WithSequenceNumbers,
	e_al3,
	e_al1M,
	e_al2M,
	e_al3M
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223LogicalChannelParameters_AdaptationLayerType & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245H223AL1MParameters *( void );
	 operator VS_H245H223AL2MParameters *( void );
	 operator VS_H245H223AL3MParameters *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223LogicalChannelParameters /////////////////////////

struct VS_H245H223LogicalChannelParameters : public VS_AsnSequence
{
	 VS_H245H223LogicalChannelParameters( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245H223LogicalChannelParameters_AdaptationLayerType	 adaptationLayerType ;
 	 VS_AsnBoolean  segmentableFlag ;
 	void operator=(const VS_H245H223LogicalChannelParameters& src);

};
//////////////////////CLASS VS_H245H222LogicalChannelParameters /////////////////////////

struct VS_H245H222LogicalChannelParameters : public VS_AsnSequence
{
	 VS_H245H222LogicalChannelParameters( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  resourceID ;
 	TemplInteger<0,8191,VS_Asn::FixedConstraint,0>  subChannelID ;
 	TemplInteger<0,8191,VS_Asn::FixedConstraint,0>  pcr_pid ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  programDescriptors ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  streamDescriptors ;
 	void operator=(const VS_H245H222LogicalChannelParameters& src);

};

struct VS_H245RedundancyEncoding;
struct VS_H245MultiplePayloadStream;
//////////////////////CLASS VS_H245H235Media_MediaType /////////////////////////

struct VS_H245H235Media_MediaType : public VS_AsnChoice
{
	 VS_H245H235Media_MediaType( void );

 	enum{
	e_nonStandard,
	e_videoData,
	e_audioData,
	e_data,
	e_redundancyEncoding,
	e_multiplePayloadStream,
	e_depFfec,
	e_fec
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H235Media_MediaType & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245VideoCapability *( void );
	 operator VS_H245AudioCapability *( void );
	 operator VS_H245DataApplicationCapability *( void );
	 operator VS_H245RedundancyEncoding *( void );
	 operator VS_H245MultiplePayloadStream *( void );
	 operator VS_H245DepFECData *( void );
	 operator VS_H245FECData *( void );

	void Show( void ) const;

};

struct VS_H245H235Media;
struct VS_H245MultiplexedStreamParameter;


//////////////////////CLASS VS_H245DataType /////////////////////////

struct VS_H245DataType : public VS_AsnChoice
{
	 VS_H245DataType( void );

 	enum{
	e_nonStandard,
	e_nullData,
	e_videoData,
	e_audioData,
	e_data,
	e_encryptionData,
	e_h235Control,
	e_h235Media,
	e_multiplexedStream,
	e_redundancyEncoding,
	e_multiplePayloadStream,
	e_depFfec,
	e_fec
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DataType & src);

	void operator=( VS_H245VideoCapability *vc );
	void operator=( VS_H245AudioCapability *ac );



	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245VideoCapability *( void );
	 operator VS_H245AudioCapability *( void );
	 operator VS_H245DataApplicationCapability *( void );
	 operator VS_H245EncryptionMode *( void );
	 operator VS_H245H235Media *( void );
	 operator VS_H245MultiplexedStreamParameter *( void );
	 operator VS_H245RedundancyEncoding *( void );
	 operator VS_H245MultiplePayloadStream *( void );
	 operator VS_H245DepFECData *( void );
	 operator VS_H245FECData *( void );



	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultiplePayloadStreamElement /////////////////////////

struct VS_H245MultiplePayloadStreamElement : public VS_AsnSequence
{
	 VS_H245MultiplePayloadStreamElement( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245DataType  dataType ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  payloadType ;
 	void operator=(const VS_H245MultiplePayloadStreamElement& src);

};
//////////////////////CLASS VS_H245RedundancyEncodingElement /////////////////////////

struct VS_H245RedundancyEncodingElement : public VS_AsnSequence
{
	 VS_H245RedundancyEncodingElement( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245DataType  dataType ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  payloadType ;
 	void operator=(const VS_H245RedundancyEncodingElement& src);

};
//////////////////////CLASS VS_H245MultiplePayloadStream /////////////////////////

struct VS_H245MultiplePayloadStream : public VS_AsnSequence
{
	 VS_H245MultiplePayloadStream( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245MultiplePayloadStreamElement ,0,INT_MAX,VS_Asn::Unconstrained,0  >  elements ;
 	void operator=(const VS_H245MultiplePayloadStream& src);

};





//////////////////////CLASS VS_H245RedundancyEncoding_RtpRedundancyEncoding /////////////////////////

struct VS_H245RedundancyEncoding_RtpRedundancyEncoding : public VS_AsnSequence
{
	 VS_H245RedundancyEncoding_RtpRedundancyEncoding( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245RedundancyEncodingElement  primary ;
 	Constrained_array_of_type<  VS_H245RedundancyEncodingElement ,0,INT_MAX,VS_Asn::Unconstrained,0  >  secondary ;
 	void operator=(const VS_H245RedundancyEncoding_RtpRedundancyEncoding& src);

};



//////////////////////CLASS VS_H245Q2931Address_Address /////////////////////////

struct VS_H245Q2931Address_Address : public VS_AsnChoice
{
	 VS_H245Q2931Address_Address( void );

 	enum{
	e_internationalNumber,
	e_nsapAddress
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245Q2931Address_Address & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245Q2931Address /////////////////////////

struct VS_H245Q2931Address : public VS_AsnSequence
{
	 VS_H245Q2931Address( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245Q2931Address_Address	 address ;
 	TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  subaddress ;
 	void operator=(const VS_H245Q2931Address& src);

};
//////////////////////CLASS VS_H245NetworkAccessParameters_Distribution /////////////////////////

struct VS_H245NetworkAccessParameters_Distribution : public VS_AsnChoice
{
	 VS_H245NetworkAccessParameters_Distribution( void );

 	enum{
	e_unicast,
	e_multicast
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NetworkAccessParameters_Distribution & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NetworkAccessParameters_NetworkAddress /////////////////////////

struct VS_H245NetworkAccessParameters_NetworkAddress : public VS_AsnChoice
{
	 VS_H245NetworkAccessParameters_NetworkAddress( void );

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static unsigned char   e164Address_alphabet[];
	 static unsigned		e164Address_alphabet_size;
	 static unsigned char   e164Address_inverse_table[];
	 static const bool      e164Address_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

 	enum{
	e_q2931Address,
	e_e164Address,
	e_localAreaAddress
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NetworkAccessParameters_NetworkAddress & src);

	 operator VS_H245Q2931Address *( void );
	 operator VS_H245TransportAddress *( void );

	void Show( void ) const;

};


//////////////////////CLASS VS_H245NetworkAccessParameters_T120SetupProcedure /////////////////////////

struct VS_H245NetworkAccessParameters_T120SetupProcedure : public VS_AsnChoice
{
	 VS_H245NetworkAccessParameters_T120SetupProcedure( void );

 	enum{
	e_originateCall,
	e_waitForCall,
	e_issueQuery
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245NetworkAccessParameters_T120SetupProcedure & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245NetworkAccessParameters /////////////////////////

struct VS_H245NetworkAccessParameters : public VS_AsnSequence
{
	 VS_H245NetworkAccessParameters( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H245NetworkAccessParameters_Distribution	 distribution ;
 	VS_H245NetworkAccessParameters_NetworkAddress	 networkAddress ;
 	 VS_AsnBoolean  associateConference ;
 	TemplOctetString<1,255,VS_Asn::FixedConstraint,0>  externalReference ;
 	VS_H245NetworkAccessParameters_T120SetupProcedure	 t120SetupProcedure ;
 	void operator=(const VS_H245NetworkAccessParameters& src);

};

//////////////////////CLASS VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters /////////////////////////

struct VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters : public VS_AsnChoice
{
	 VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters( void );

 	enum{
	e_h222LogicalChannelParameters,
	e_h223LogicalChannelParameters,
	e_v76LogicalChannelParameters,
	e_h2250LogicalChannelParameters,
	e_none
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters & src);
	void operator=( VS_H245H2250LogicalChannelParameters *h2250lcp );

	 operator VS_H245H222LogicalChannelParameters *( void );
	 operator VS_H245H223LogicalChannelParameters *( void );
	 operator VS_H245V76LogicalChannelParameters *( void );
	 operator VS_H245H2250LogicalChannelParameters *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters /////////////////////////

struct VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters : public VS_AsnSequence
{
	 VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  portNumber ;
 	 VS_H245DataType  dataType ;
 	VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters	 multiplexParameters ;
 	 VS_H245LogicalChannelNumber  forwardLogicalChannelDependency ;
 	 VS_H245LogicalChannelNumber  replacementFor ;
 	void operator=(const VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters& src);

};
//////////////////////CLASS VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters /////////////////////////

struct VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters : public VS_AsnChoice
{
	 VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters( void );

 	enum{
	e_h223LogicalChannelParameters,
	e_v76LogicalChannelParameters,
	e_h2250LogicalChannelParameters
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters & src);

	 operator VS_H245H223LogicalChannelParameters *( void );
	 operator VS_H245V76LogicalChannelParameters *( void );
	 operator VS_H245H2250LogicalChannelParameters *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters /////////////////////////

struct VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters : public VS_AsnSequence
{
	 VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245DataType  dataType ;
 	VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters	 multiplexParameters ;
 	 VS_H245LogicalChannelNumber  reverseLogicalChannelDependency ;
 	 VS_H245LogicalChannelNumber  replacementFor ;
 	void operator=(const VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters& src);

};


//////////////////////CLASS VS_H245FECCapability_Rfc2733Format /////////////////////////

struct VS_H245FECCapability_Rfc2733Format : public VS_AsnChoice
{
	 VS_H245FECCapability_Rfc2733Format( void );

 	enum{
	e_rfc2733rfc2198,
	e_rfc2733sameport,
	e_rfc2733diffport
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245FECCapability_Rfc2733Format & src);

	 operator VS_H245MaxRedundancy *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245FECCapability /////////////////////////

struct VS_H245FECCapability : public VS_AsnSequence
{
	 VS_H245FECCapability( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245CapabilityTableEntryNumber  protectedCapability ;
 	 VS_AsnObjectId  fecScheme ;
 	VS_H245FECCapability_Rfc2733Format	 rfc2733Format ;
 	void operator=(const VS_H245FECCapability& src);

};
//////////////////////CLASS VS_H245DepFECCapability_Rfc2733_SeparateStream /////////////////////////

struct VS_H245DepFECCapability_Rfc2733_SeparateStream : public VS_AsnSequence
{
	 VS_H245DepFECCapability_Rfc2733_SeparateStream( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  separatePort ;
 	 VS_AsnBoolean  samePort ;
 	void operator=(const VS_H245DepFECCapability_Rfc2733_SeparateStream& src);

};
//////////////////////CLASS VS_H245DepFECCapability_Rfc2733 /////////////////////////

struct VS_H245DepFECCapability_Rfc2733 : public VS_AsnSequence
{
	 VS_H245DepFECCapability_Rfc2733( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  redundancyEncoding ;
 	VS_H245DepFECCapability_Rfc2733_SeparateStream	 separateStream ;
 	void operator=(const VS_H245DepFECCapability_Rfc2733& src);

};
//////////////////////CLASS VS_H245DepFECCapability /////////////////////////

struct VS_H245DepFECCapability : public VS_AsnChoice
{
	 VS_H245DepFECCapability( void );

 	enum{
	e_rfc2733
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DepFECCapability & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultiplePayloadStreamCapability /////////////////////////

struct VS_H245MultiplePayloadStreamCapability : public VS_AsnSequence
{
	 VS_H245MultiplePayloadStreamCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245AlternativeCapabilitySet ,1,256,VS_Asn::FixedConstraint,0  >  capabilities ;
 	void operator=(const VS_H245MultiplePayloadStreamCapability& src);

};
//////////////////////CLASS VS_H245NoPTAudioToneCapability /////////////////////////

struct VS_H245NoPTAudioToneCapability : public VS_AsnSequence
{
	 VS_H245NoPTAudioToneCapability( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245NoPTAudioToneCapability& src);

};
//////////////////////CLASS VS_H245NoPTAudioTelephonyEventCapability /////////////////////////

struct VS_H245NoPTAudioTelephonyEventCapability : public VS_AsnSequence
{
	 VS_H245NoPTAudioTelephonyEventCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_AsnGeneralString  audioTelephoneEvent ;
 	void operator=(const VS_H245NoPTAudioTelephonyEventCapability& src);

};
//////////////////////CLASS VS_H245AudioToneCapability /////////////////////////

struct VS_H245AudioToneCapability : public VS_AsnSequence
{
	 VS_H245AudioToneCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<96,127,VS_Asn::FixedConstraint,0>  dynamicRTPPayloadType ;
 	void operator=(const VS_H245AudioToneCapability& src);

};
//////////////////////CLASS VS_H245AudioTelephonyEventCapability /////////////////////////

struct VS_H245AudioTelephonyEventCapability : public VS_AsnSequence
{
	 VS_H245AudioTelephonyEventCapability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<96,127,VS_Asn::FixedConstraint,0>  dynamicRTPPayloadType ;
 	VS_AsnGeneralString  audioTelephoneEvent ;
 	void operator=(const VS_H245AudioTelephonyEventCapability& src);

};

struct VS_H245H222Capability;
struct VS_H245H223Capability;
//////////////////////CLASS VS_H245MultiplexFormat /////////////////////////

struct VS_H245MultiplexFormat : public VS_AsnChoice
{
	 VS_H245MultiplexFormat( void );

 	enum{
	e_nonStandard,
	e_h222Capability,
	e_h223Capability
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultiplexFormat & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245H222Capability *( void );
	 operator VS_H245H223Capability *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultiplexedStreamParameter /////////////////////////

struct VS_H245MultiplexedStreamParameter : public VS_AsnSequence
{
	 VS_H245MultiplexedStreamParameter( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245MultiplexFormat  multiplexFormat ;
 	 VS_AsnBoolean  controlOnMuxStream ;
 	void operator=(const VS_H245MultiplexedStreamParameter& src);

};
//////////////////////CLASS VS_H245MultiplexedStreamCapability /////////////////////////

struct VS_H245MultiplexedStreamCapability : public VS_AsnSequence
{
	 VS_H245MultiplexedStreamCapability( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245MultiplexFormat  multiplexFormat ;
 	 VS_AsnBoolean  controlOnMuxStream ;
 	Constrained_array_of_type<  VS_H245AlternativeCapabilitySet ,1,256,VS_Asn::FixedConstraint,0  >  capabilityOnMuxStream ;
 	void operator=(const VS_H245MultiplexedStreamCapability& src);

};
struct VS_H245GenericParameter;
//////////////////////CLASS VS_H245ParameterValue /////////////////////////

struct VS_H245ParameterValue : public VS_AsnChoice
{
	 VS_H245ParameterValue( void );

 	enum{
	e_logical,
	e_booleanArray,
	e_unsignedMin,
	e_unsignedMax,
	e_unsigned32Min,
	e_unsigned32Max,
	e_octetString,
	e_genericParameter
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ParameterValue & src);

	 operator VS_H245GenericParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245ParameterIdentifier /////////////////////////

struct VS_H245ParameterIdentifier : public VS_AsnChoice
{
	 VS_H245ParameterIdentifier( void );

 	enum{
	e_standard,
	e_h221NonStandard,
	e_uuid,
	e_domainBased
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ParameterIdentifier & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245GenericParameter /////////////////////////

struct VS_H245GenericParameter : public VS_AsnSequence
{
	 VS_H245GenericParameter( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245ParameterIdentifier  parameterIdentifier ;
 	 VS_H245ParameterValue  parameterValue ;
 	Constrained_array_of_type<  VS_H245ParameterIdentifier ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supersedes ;
 	void operator=(const VS_H245GenericParameter& src);

};
//////////////////////CLASS VS_H245EncryptionSync /////////////////////////

typedef Type_id<VS_H323H235Key> VS_H323EncodedH235Key;

struct VS_H245EncryptionSync : public VS_AsnSequence
{
	 VS_H245EncryptionSync( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
    VS_Reference_of_Asn  e_ref[extension_root];

	 VS_H245NonStandardParameter  nonStandard ;
 	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  synchFlag ;
	TemplOctetString<1, 65535, VS_Asn::FixedConstraint, 0>  h235Key;
 	Constrained_array_of_type<  VS_H245EscrowData ,1,256,VS_Asn::FixedConstraint,0  >  escrowentry ;
 	 VS_H245GenericParameter  genericParameter ;
 	void operator=(const VS_H245EncryptionSync& src);

};

//////////////////////CLASS VS_H245OpenLogicalChannel /////////////////////////

struct VS_H245OpenLogicalChannel : public VS_AsnSequence
{
	 VS_H245OpenLogicalChannel( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters	 forwardLogicalChannelParameters ;
 	VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters	 reverseLogicalChannelParameters ;
 	 VS_H245NetworkAccessParameters  separateStack ;
 	 VS_H245EncryptionSync  encryptionSync ;
 	void operator=(const VS_H245OpenLogicalChannel& src);

};

//////////////////////CLASS VS_H245OpenLogicalChannelAck /////////////////////////

struct VS_H245OpenLogicalChannelAck : public VS_AsnSequence
{
	 VS_H245OpenLogicalChannelAck( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 3;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters	 reverseLogicalChannelParameters ;
 	 VS_H245NetworkAccessParameters  separateStack ;
 	VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters	 forwardMultiplexAckParameters ;
 	 VS_H245EncryptionSync  encryptionSync ;
 	void operator=(const VS_H245OpenLogicalChannelAck& src);

};

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand /////////////////////////

struct VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand : public VS_AsnSequence
{
	 VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245EncryptionSync  encryptionSync ;
 	 VS_H245MultiplePayloadStream  multiplePayloadStream ;
 	void operator=(const VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand& src);

};
//////////////////////CLASS VS_H245CapabilityIdentifier /////////////////////////

struct VS_H245CapabilityIdentifier : public VS_AsnChoice
{
	 VS_H245CapabilityIdentifier( void );

 	enum{
	e_standard,
	e_h221NonStandard,
	e_uuid,
	e_domainBased
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CapabilityIdentifier & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245GenericCapability /////////////////////////

struct VS_H245GenericCapability : public VS_AsnSequence
{
	 VS_H245GenericCapability( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245CapabilityIdentifier  capabilityIdentifier ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  maxBitRate ;
 	Constrained_array_of_type<  VS_H245GenericParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  collapsing ;
 	Constrained_array_of_type<  VS_H245GenericParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  nonCollapsing ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  nonCollapsingRaw ;
 	 VS_H245DataProtocolCapability  transport ;
 	void operator=(const VS_H245GenericCapability& src);

};


//////////////////////CLASS VS_H245ConferenceCapability /////////////////////////

struct VS_H245ConferenceCapability : public VS_AsnSequence
{
	 VS_H245ConferenceCapability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	Constrained_array_of_type<  VS_H245NonStandardParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  nonStandardData ;
 	 VS_AsnBoolean  chairControlCapability ;
 	 VS_AsnBoolean  videoIndicateMixingCapability ;
 	 VS_AsnBoolean  multipointVisualizationCapability ;
 	void operator=(const VS_H245ConferenceCapability& src);

};
//////////////////////CLASS VS_H245UserInputCapability /////////////////////////

struct VS_H245UserInputCapability : public VS_AsnChoice
{
	 VS_H245UserInputCapability( void );

 	enum{
	e_nonStandard,
	e_basicString,
	e_iA5String,
	e_generalString,
	e_dtmf,
	e_hookflash,
	e_extendedAlphanumeric,
	e_encryptedBasicString,
	e_encryptedIA5String,
	e_encryptedGeneralString,
	e_secureDTMF
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245UserInputCapability & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245IntegrityCapability /////////////////////////

struct VS_H245IntegrityCapability : public VS_AsnSequence
{
	 VS_H245IntegrityCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245NonStandardParameter  nonStandard ;
 	void operator=(const VS_H245IntegrityCapability& src);

};
//////////////////////CLASS VS_H245AuthenticationCapability /////////////////////////

struct VS_H245AuthenticationCapability : public VS_AsnSequence
{
	 VS_H245AuthenticationCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245NonStandardParameter  nonStandard ;
 	 VS_AsnObjectId  antiSpamAlgorithm ;
 	void operator=(const VS_H245AuthenticationCapability& src);

};
//////////////////////CLASS VS_H245MediaEncryptionAlgorithm /////////////////////////

struct VS_H245MediaEncryptionAlgorithm : public VS_AsnChoice
{
	 VS_H245MediaEncryptionAlgorithm( void );

 	enum{
	e_nonStandard,
	e_algorithm
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MediaEncryptionAlgorithm & src);

	 operator VS_H245NonStandardParameter *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245EncryptionCapability /////////////////////////

typedef Constrained_array_of_type<  VS_H245MediaEncryptionAlgorithm ,1,256,VS_Asn::FixedConstraint,0  >  VS_H245EncryptionCapability;
//////////////////////CLASS VS_H245EncryptionAuthenticationAndIntegrity /////////////////////////

struct VS_H245EncryptionAuthenticationAndIntegrity : public VS_AsnSequence
{
	 VS_H245EncryptionAuthenticationAndIntegrity( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245EncryptionCapability  encryptionCapability ;
 	 VS_H245AuthenticationCapability  authenticationCapability ;
 	 VS_H245IntegrityCapability  integrityCapability ;
 	 VS_H245GenericCapability  genericH235SecurityCapability ;
 	void operator=(const VS_H245EncryptionAuthenticationAndIntegrity& src);

};

//////////////////////CLASS VS_H245H235Media /////////////////////////

struct VS_H245H235Media : public VS_AsnSequence
{
	 VS_H245H235Media( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245EncryptionAuthenticationAndIntegrity  encryptionAuthenticationAndIntegrity ;
 	VS_H245H235Media_MediaType	 mediaType ;
 	void operator=(const VS_H245H235Media& src);

};
//////////////////////CLASS VS_H245H235Mode /////////////////////////

struct VS_H245H235Mode : public VS_AsnSequence
{
	 VS_H245H235Mode( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245EncryptionAuthenticationAndIntegrity  encryptionAuthenticationAndIntegrity ;
 	VS_H245H235Mode_MediaMode	 mediaMode ;
 	void operator=(const VS_H245H235Mode& src);

};
//////////////////////CLASS VS_H245T38FaxTcpOptions /////////////////////////

struct VS_H245T38FaxTcpOptions : public VS_AsnSequence
{
	 VS_H245T38FaxTcpOptions( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  t38TCPBidirectionalMode ;
 	void operator=(const VS_H245T38FaxTcpOptions& src);

};
//////////////////////CLASS VS_H245T38FaxUdpOptions_T38FaxUdpEC /////////////////////////

struct VS_H245T38FaxUdpOptions_T38FaxUdpEC : public VS_AsnChoice
{
	 VS_H245T38FaxUdpOptions_T38FaxUdpEC( void );

 	enum{
	e_t38UDPFEC,
	e_t38UDPRedundancy
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245T38FaxUdpOptions_T38FaxUdpEC & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245T38FaxUdpOptions /////////////////////////

struct VS_H245T38FaxUdpOptions : public VS_AsnSequence
{
	 VS_H245T38FaxUdpOptions( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger< 0,INT_MAX,VS_Asn::Unconstrained,false>  t38FaxMaxBuffer ;
 	TemplInteger< 0,INT_MAX,VS_Asn::Unconstrained,false>  t38FaxMaxDatagram ;
 	VS_H245T38FaxUdpOptions_T38FaxUdpEC	 t38FaxUdpEC ;
 	void operator=(const VS_H245T38FaxUdpOptions& src);

};
//////////////////////CLASS VS_H245T38FaxRateManagement /////////////////////////

struct VS_H245T38FaxRateManagement : public VS_AsnChoice
{
	 VS_H245T38FaxRateManagement( void );

 	enum{
	e_localTCF,
	e_transferredTCF
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245T38FaxRateManagement & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245T38FaxProfile /////////////////////////

struct VS_H245T38FaxProfile : public VS_AsnSequence
{
	 VS_H245T38FaxProfile( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  fillBitRemoval ;
 	 VS_AsnBoolean  transcodingJBIG ;
 	 VS_AsnBoolean  transcodingMMR ;
 	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  version ;
 	 VS_H245T38FaxRateManagement  t38FaxRateManagement ;
 	 VS_H245T38FaxUdpOptions  t38FaxUdpOptions ;
 	 VS_H245T38FaxTcpOptions  t38FaxTcpOptions ;
 	void operator=(const VS_H245T38FaxProfile& src);

};

//////////////////////CLASS VS_H245DataMode_Application_T38fax /////////////////////////

struct VS_H245DataMode_Application_T38fax : public VS_AsnSequence
{
	 VS_H245DataMode_Application_T38fax( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245DataProtocolCapability  t38FaxProtocol ;
 	 VS_H245T38FaxProfile  t38FaxProfile ;
 	void operator=(const VS_H245DataMode_Application_T38fax& src);

};
//////////////////////CLASS VS_H245T84Profile_T84Restricted /////////////////////////

struct VS_H245T84Profile_T84Restricted : public VS_AsnSequence
{
	 VS_H245T84Profile_T84Restricted( void );

	static const unsigned basic_root = 19;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  qcif ;
 	 VS_AsnBoolean  cif ;
 	 VS_AsnBoolean  ccir601Seq ;
 	 VS_AsnBoolean  ccir601Prog ;
 	 VS_AsnBoolean  hdtvSeq ;
 	 VS_AsnBoolean  hdtvProg ;
 	 VS_AsnBoolean  g3FacsMH200x100 ;
 	 VS_AsnBoolean  g3FacsMH200x200 ;
 	 VS_AsnBoolean  g4FacsMMR200x100 ;
 	 VS_AsnBoolean  g4FacsMMR200x200 ;
 	 VS_AsnBoolean  jbig200x200Seq ;
 	 VS_AsnBoolean  jbig200x200Prog ;
 	 VS_AsnBoolean  jbig300x300Seq ;
 	 VS_AsnBoolean  jbig300x300Prog ;
 	 VS_AsnBoolean  digPhotoLow ;
 	 VS_AsnBoolean  digPhotoMedSeq ;
 	 VS_AsnBoolean  digPhotoMedProg ;
 	 VS_AsnBoolean  digPhotoHighSeq ;
 	 VS_AsnBoolean  digPhotoHighProg ;
 	void operator=(const VS_H245T84Profile_T84Restricted& src);

};
//////////////////////CLASS VS_H245T84Profile /////////////////////////

struct VS_H245T84Profile : public VS_AsnChoice
{
	 VS_H245T84Profile( void );

 	enum{
	e_t84Unrestricted,
	e_t84Restricted
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245T84Profile & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245V42bis /////////////////////////

struct VS_H245V42bis : public VS_AsnSequence
{
	 VS_H245V42bis( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,65536,VS_Asn::FixedConstraint,0>  numberOfCodewords ;
 	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  maximumStringLength ;
 	void operator=(const VS_H245V42bis& src);

};
//////////////////////CLASS VS_H245CompressionType /////////////////////////

struct VS_H245CompressionType : public VS_AsnChoice
{
	 VS_H245CompressionType( void );

 	enum{
	e_v42bis
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CompressionType & src);

	 operator VS_H245V42bis *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245DataProtocolCapability_V76wCompression /////////////////////////

struct VS_H245DataProtocolCapability_V76wCompression : public VS_AsnChoice
{
	 VS_H245DataProtocolCapability_V76wCompression( void );

 	enum{
	e_transmitCompression,
	e_receiveCompression,
	e_transmitAndReceiveCompression
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DataProtocolCapability_V76wCompression & src);

	 operator VS_H245CompressionType *( void );

	void Show( void ) const;

};



//////////////////////CLASS VS_H245DataApplicationCapability_Application_T84 /////////////////////////

struct VS_H245DataApplicationCapability_Application_T84 : public VS_AsnSequence
{
	 VS_H245DataApplicationCapability_Application_T84( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245DataProtocolCapability  t84Protocol ;
 	 VS_H245T84Profile  t84Profile ;
 	void operator=(const VS_H245DataApplicationCapability_Application_T84& src);

};
//////////////////////CLASS VS_H245DataApplicationCapability_Application_Nlpid /////////////////////////

struct VS_H245DataApplicationCapability_Application_Nlpid : public VS_AsnSequence
{
	 VS_H245DataApplicationCapability_Application_Nlpid( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245DataProtocolCapability  nlpidProtocol ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  nlpidData ;
 	void operator=(const VS_H245DataApplicationCapability_Application_Nlpid& src);

};
//////////////////////CLASS VS_H245DataApplicationCapability_Application_T38fax /////////////////////////

struct VS_H245DataApplicationCapability_Application_T38fax : public VS_AsnSequence
{
	 VS_H245DataApplicationCapability_Application_T38fax( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245DataProtocolCapability  t38FaxProtocol ;
 	 VS_H245T38FaxProfile  t38FaxProfile ;
 	void operator=(const VS_H245DataApplicationCapability_Application_T38fax& src);

};
//////////////////////CLASS VS_H245DataApplicationCapability_Application /////////////////////////

struct VS_H245DataApplicationCapability_Application : public VS_AsnChoice
{
	 VS_H245DataApplicationCapability_Application( void );

 	enum{
	e_nonStandard,
	e_t120,
	e_dsm_cc,
	e_userData,
	e_t84,
	e_t434,
	e_h224,
	e_nlpid,
	e_dsvdControl,
	e_h222DataPartitioning,
	e_t30fax,
	e_t140,
	e_t38fax,
	e_genericDataCapability
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245DataApplicationCapability_Application & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245DataProtocolCapability *( void );
	 operator VS_H245GenericCapability *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245DataApplicationCapability /////////////////////////

struct VS_H245DataApplicationCapability : public VS_AsnSequence
{
	 VS_H245DataApplicationCapability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245DataApplicationCapability_Application	 application ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  maxBitRate ;
 	void operator=(const VS_H245DataApplicationCapability& src);

};


//////////////////////CLASS VS_H245IS13818AudioCapability /////////////////////////

struct VS_H245IS13818AudioCapability : public VS_AsnSequence
{
	 VS_H245IS13818AudioCapability( void );

	static const unsigned basic_root = 21;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  audioLayer1 ;
 	 VS_AsnBoolean  audioLayer2 ;
 	 VS_AsnBoolean  audioLayer3 ;
 	 VS_AsnBoolean  audioSampling16k ;
 	 VS_AsnBoolean  audioSampling22k05 ;
 	 VS_AsnBoolean  audioSampling24k ;
 	 VS_AsnBoolean  audioSampling32k ;
 	 VS_AsnBoolean  audioSampling44k1 ;
 	 VS_AsnBoolean  audioSampling48k ;
 	 VS_AsnBoolean  singleChannel ;
 	 VS_AsnBoolean  twoChannels ;
 	 VS_AsnBoolean  threeChannels2_1 ;
 	 VS_AsnBoolean  threeChannels3_0 ;
 	 VS_AsnBoolean  fourChannels2_0_2_0 ;
 	 VS_AsnBoolean  fourChannels2_2 ;
 	 VS_AsnBoolean  fourChannels3_1 ;
 	 VS_AsnBoolean  fiveChannels3_0_2_0 ;
 	 VS_AsnBoolean  fiveChannels3_2 ;
 	 VS_AsnBoolean  lowFrequencyEnhancement ;
 	 VS_AsnBoolean  multilingual ;
 	TemplInteger<1,1130,VS_Asn::FixedConstraint,0>  bitRate ;
 	void operator=(const VS_H245IS13818AudioCapability& src);

};
//////////////////////CLASS VS_H245IS11172AudioCapability /////////////////////////

struct VS_H245IS11172AudioCapability : public VS_AsnSequence
{
	 VS_H245IS11172AudioCapability( void );

	static const unsigned basic_root = 9;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  audioLayer1 ;
 	 VS_AsnBoolean  audioLayer2 ;
 	 VS_AsnBoolean  audioLayer3 ;
 	 VS_AsnBoolean  audioSampling32k ;
 	 VS_AsnBoolean  audioSampling44k1 ;
 	 VS_AsnBoolean  audioSampling48k ;
 	 VS_AsnBoolean  singleChannel ;
 	 VS_AsnBoolean  twoChannels ;
 	TemplInteger<1,448,VS_Asn::FixedConstraint,0>  bitRate ;
 	void operator=(const VS_H245IS11172AudioCapability& src);

};
//////////////////////CLASS VS_H245G7231AnnexCCapability_G723AnnexCAudioMode /////////////////////////

struct VS_H245G7231AnnexCCapability_G723AnnexCAudioMode : public VS_AsnSequence
{
	 VS_H245G7231AnnexCCapability_G723AnnexCAudioMode( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<27,78,VS_Asn::FixedConstraint,0>  highRateMode0 ;
 	TemplInteger<27,78,VS_Asn::FixedConstraint,0>  highRateMode1 ;
 	TemplInteger<23,66,VS_Asn::FixedConstraint,0>  lowRateMode0 ;
 	TemplInteger<23,66,VS_Asn::FixedConstraint,0>  lowRateMode1 ;
 	TemplInteger<6,17,VS_Asn::FixedConstraint,0>  sidMode0 ;
 	TemplInteger<6,17,VS_Asn::FixedConstraint,0>  sidMode1 ;
 	void operator=(const VS_H245G7231AnnexCCapability_G723AnnexCAudioMode& src);

};
//////////////////////CLASS VS_H245G7231AnnexCCapability /////////////////////////

struct VS_H245G7231AnnexCCapability : public VS_AsnSequence
{
	 VS_H245G7231AnnexCCapability( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  maxAl_sduAudioFrames ;
 	 VS_AsnBoolean  silenceSuppression ;
 	VS_H245G7231AnnexCCapability_G723AnnexCAudioMode	 g723AnnexCAudioMode ;
 	void operator=(const VS_H245G7231AnnexCCapability& src);

};

//////////////////////CLASS VS_H245AudioCapability_G7231 /////////////////////////

struct VS_H245AudioCapability_G7231 : public VS_AsnSequence
{
	 VS_H245AudioCapability_G7231( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  maxAl_sduAudioFrames ;
 	 VS_AsnBoolean  silenceSuppression ;
 	void operator=(const VS_H245AudioCapability_G7231& src);

};
struct VS_H245VBDCapability;
//////////////////////CLASS VS_H245AudioCapability /////////////////////////

struct VS_H245AudioCapability : public VS_AsnChoice
{
	 VS_H245AudioCapability( void );

 	enum{
	e_nonStandard,
	e_g711Alaw64k,
	e_g711Alaw56k,
	e_g711Ulaw64k,
	e_g711Ulaw56k,
	e_g722_64k,
	e_g722_56k,
	e_g722_48k,
	e_g7231,
	e_g728,
	e_g729,
	e_g729AnnexA,
	e_is11172AudioCapability,
	e_is13818AudioCapability,
	e_g729wAnnexB,
	e_g729AnnexAwAnnexB,
	e_g7231AnnexCCapability,
	e_gsmFullRate,
	e_gsmHalfRate,
	e_gsmEnhancedFullRate,
	e_genericAudioCapability,
	e_g729Extensions,
	e_vbd,
	e_audioTelephonyEvent,
	e_audioTone
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245AudioCapability & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245IS11172AudioCapability *( void );
	 operator VS_H245IS13818AudioCapability *( void );
	 operator VS_H245G7231AnnexCCapability *( void );
	 operator VS_H245GSMAudioCapability *( void );
	 operator VS_H245GenericCapability *( void );
	 operator VS_H245G729Extensions *( void );
	 operator VS_H245VBDCapability *( void );
	 operator VS_H245NoPTAudioTelephonyEventCapability *( void );
	 operator VS_H245NoPTAudioToneCapability *( void );

	void Show( void ) const;

};
//////////////////////CLASS VS_H245VBDCapability /////////////////////////

struct VS_H245VBDCapability : public VS_AsnSequence
{
	 VS_H245VBDCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245AudioCapability  type ;
 	void operator=(const VS_H245VBDCapability& src);

};
//////////////////////CLASS VS_H245IS11172VideoCapability /////////////////////////

struct VS_H245IS11172VideoCapability : public VS_AsnSequence
{
	 VS_H245IS11172VideoCapability( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  constrainedBitstream ;
 	TemplInteger<1073741823,1073741823,VS_Asn::FixedConstraint,0>  videoBitRate ;
 	TemplInteger<262143,262143,VS_Asn::FixedConstraint,0>  vbvBufferSize ;
 	TemplInteger<0,16383,VS_Asn::FixedConstraint,0>  samplesPerLine ;
 	TemplInteger<0,16383,VS_Asn::FixedConstraint,0>  linesPerFrame ;
 	TemplInteger<0,15,VS_Asn::FixedConstraint,0>  pictureRate ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  luminanceSampleRate ;
 	 VS_AsnBoolean  videoBadMBsCap ;
 	void operator=(const VS_H245IS11172VideoCapability& src);

};
//////////////////////CLASS VS_H245H263Version3Options /////////////////////////

struct VS_H245H263Version3Options : public VS_AsnSequence
{
	 VS_H245H263Version3Options( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  dataPartitionedSlices ;
 	 VS_AsnBoolean  fixedPointIDCT0 ;
 	 VS_AsnBoolean  interlacedFields ;
 	 VS_AsnBoolean  currentPictureHeaderRepetition ;
 	 VS_AsnBoolean  previousPictureHeaderRepetition ;
 	 VS_AsnBoolean  nextPictureHeaderRepetition ;
 	 VS_AsnBoolean  pictureNumber ;
 	 VS_AsnBoolean  spareReferencePictures ;
 	void operator=(const VS_H245H263Version3Options& src);

};
//////////////////////CLASS VS_H245H263ModeComboFlags /////////////////////////

struct VS_H245H263ModeComboFlags : public VS_AsnSequence
{
	 VS_H245H263ModeComboFlags( void );

	static const unsigned basic_root = 21;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  unrestrictedVector ;
 	 VS_AsnBoolean  arithmeticCoding ;
 	 VS_AsnBoolean  advancedPrediction ;
 	 VS_AsnBoolean  pbFrames ;
 	 VS_AsnBoolean  advancedIntraCodingMode ;
 	 VS_AsnBoolean  deblockingFilterMode ;
 	 VS_AsnBoolean  unlimitedMotionVectors ;
 	 VS_AsnBoolean  slicesInOrder_NonRect ;
 	 VS_AsnBoolean  slicesInOrder_Rect ;
 	 VS_AsnBoolean  slicesNoOrder_NonRect ;
 	 VS_AsnBoolean  slicesNoOrder_Rect ;
 	 VS_AsnBoolean  improvedPBFramesMode ;
 	 VS_AsnBoolean  referencePicSelect ;
 	 VS_AsnBoolean  dynamicPictureResizingByFour ;
 	 VS_AsnBoolean  dynamicPictureResizingSixteenthPel ;
 	 VS_AsnBoolean  dynamicWarpingHalfPel ;
 	 VS_AsnBoolean  dynamicWarpingSixteenthPel ;
 	 VS_AsnBoolean  reducedResolutionUpdate ;
 	 VS_AsnBoolean  independentSegmentDecoding ;
 	 VS_AsnBoolean  alternateInterVLCMode ;
 	 VS_AsnBoolean  modifiedQuantizationMode ;
 	 VS_AsnBoolean  enhancedReferencePicSelect ;
 	 VS_H245H263Version3Options  h263Version3Options ;
 	void operator=(const VS_H245H263ModeComboFlags& src);

};
//////////////////////CLASS VS_H245H263VideoModeCombos /////////////////////////

struct VS_H245H263VideoModeCombos : public VS_AsnSequence
{
	 VS_H245H263VideoModeCombos( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245H263ModeComboFlags  h263VideoUncoupledModes ;
 	Constrained_array_of_type<  VS_H245H263ModeComboFlags ,1,16,VS_Asn::FixedConstraint,0  >  h263VideoCoupledModes ;
 	void operator=(const VS_H245H263VideoModeCombos& src);

};
//////////////////////CLASS VS_H245CustomPCF /////////////////////////

struct VS_H245CustomPCF : public VS_AsnSequence
{
	 VS_H245CustomPCF( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1000,1001,VS_Asn::FixedConstraint,0>  clockConversionCode ;
 	TemplInteger<1,127,VS_Asn::FixedConstraint,0>  clockDivisor ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  customMPI ;
 	void operator=(const VS_H245CustomPCF& src);

};
//////////////////////CLASS VS_H245ExtendedPAR /////////////////////////

struct VS_H245ExtendedPAR : public VS_AsnSequence
{
	 VS_H245ExtendedPAR( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  width ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  height ;
 	void operator=(const VS_H245ExtendedPAR& src);

};
//////////////////////CLASS VS_H245CustomPictureFormat_MPI /////////////////////////

struct VS_H245CustomPictureFormat_MPI : public VS_AsnSequence
{
	 VS_H245CustomPictureFormat_MPI( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,31,VS_Asn::FixedConstraint,0>  standardMPI ;
 	Constrained_array_of_type<  VS_H245CustomPCF ,1,16,VS_Asn::FixedConstraint,0  >  customPCF ;
 	void operator=(const VS_H245CustomPictureFormat_MPI& src);

};
//////////////////////CLASS VS_H245CustomPictureFormat_PixelAspectInformation /////////////////////////

struct VS_H245CustomPictureFormat_PixelAspectInformation : public VS_AsnChoice
{
	 VS_H245CustomPictureFormat_PixelAspectInformation( void );

 	enum{
	e_anyPixelAspectRatio,
	e_pixelAspectCode,
	e_extendedPAR
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CustomPictureFormat_PixelAspectInformation & src);

	 operator VS_H245ExtendedPAR *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245CustomPictureFormat /////////////////////////

struct VS_H245CustomPictureFormat : public VS_AsnSequence
{
	 VS_H245CustomPictureFormat( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  maxCustomPictureWidth ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  maxCustomPictureHeight ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  minCustomPictureWidth ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  minCustomPictureHeight ;
 	VS_H245CustomPictureFormat_MPI	 mPI ;
 	VS_H245CustomPictureFormat_PixelAspectInformation	 pixelAspectInformation ;
 	void operator=(const VS_H245CustomPictureFormat& src);

};
//////////////////////CLASS VS_H245CustomPictureClockFrequency /////////////////////////

struct VS_H245CustomPictureClockFrequency : public VS_AsnSequence
{
	 VS_H245CustomPictureClockFrequency( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1000,1001,VS_Asn::FixedConstraint,0>  clockConversionCode ;
 	TemplInteger<1,127,VS_Asn::FixedConstraint,0>  clockDivisor ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  sqcifMPI ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  qcifMPI ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  cifMPI ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  cif4MPI ;
 	TemplInteger<1,2048,VS_Asn::FixedConstraint,0>  cif16MPI ;
 	void operator=(const VS_H245CustomPictureClockFrequency& src);

};
//////////////////////CLASS VS_H245RefPictureSelection_AdditionalPictureMemory /////////////////////////

struct VS_H245RefPictureSelection_AdditionalPictureMemory : public VS_AsnSequence
{
	 VS_H245RefPictureSelection_AdditionalPictureMemory( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  sqcifAdditionalPictureMemory ;
 	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  qcifAdditionalPictureMemory ;
 	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  cifAdditionalPictureMemory ;
 	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  cif4AdditionalPictureMemory ;
 	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  cif16AdditionalPictureMemory ;
 	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  bigCpfAdditionalPictureMemory ;
 	void operator=(const VS_H245RefPictureSelection_AdditionalPictureMemory& src);

};
//////////////////////CLASS VS_H245RefPictureSelection_VideoBackChannelSend /////////////////////////

struct VS_H245RefPictureSelection_VideoBackChannelSend : public VS_AsnChoice
{
	 VS_H245RefPictureSelection_VideoBackChannelSend( void );

 	enum{
	e_none,
	e_ackMessageOnly,
	e_nackMessageOnly,
	e_ackOrNackMessageOnly,
	e_ackAndNackMessage
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RefPictureSelection_VideoBackChannelSend & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters /////////////////////////

struct VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters : public VS_AsnSequence
{
	 VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,128,VS_Asn::FixedConstraint,0>  mpuHorizMBs ;
 	TemplInteger<1,72,VS_Asn::FixedConstraint,0>  mpuVertMBs ;
 	TemplInteger<1,65536,VS_Asn::FixedConstraint,0>  mpuTotalNumber ;
 	void operator=(const VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters& src);

};
//////////////////////CLASS VS_H245RefPictureSelection_EnhancedReferencePicSelect /////////////////////////

struct VS_H245RefPictureSelection_EnhancedReferencePicSelect : public VS_AsnSequence
{
	 VS_H245RefPictureSelection_EnhancedReferencePicSelect( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters	 subPictureRemovalParameters ;
 	void operator=(const VS_H245RefPictureSelection_EnhancedReferencePicSelect& src);

};
//////////////////////CLASS VS_H245RefPictureSelection /////////////////////////

struct VS_H245RefPictureSelection : public VS_AsnSequence
{
	 VS_H245RefPictureSelection( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H245RefPictureSelection_AdditionalPictureMemory	 additionalPictureMemory ;
 	 VS_AsnBoolean  videoMux ;
 	VS_H245RefPictureSelection_VideoBackChannelSend	 videoBackChannelSend ;
 	VS_H245RefPictureSelection_EnhancedReferencePicSelect	 enhancedReferencePicSelect ;
 	void operator=(const VS_H245RefPictureSelection& src);

};
//////////////////////CLASS VS_H245TransparencyParameters /////////////////////////

struct VS_H245TransparencyParameters : public VS_AsnSequence
{
	 VS_H245TransparencyParameters( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  presentationOrder ;
 	TemplInteger<-262144,262143,VS_Asn::FixedConstraint,0>  offset_x ;
 	TemplInteger<-262144,262143,VS_Asn::FixedConstraint,0>  offset_y ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  scale_x ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  scale_y ;
 	void operator=(const VS_H245TransparencyParameters& src);

};
//////////////////////CLASS VS_H245H263Options /////////////////////////

struct VS_H245H263Options : public VS_AsnSequence
{
	 VS_H245H263Options( void );

	static const unsigned basic_root = 29;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  advancedIntraCodingMode ;
 	 VS_AsnBoolean  deblockingFilterMode ;
 	 VS_AsnBoolean  improvedPBFramesMode ;
 	 VS_AsnBoolean  unlimitedMotionVectors ;
 	 VS_AsnBoolean  fullPictureFreeze ;
 	 VS_AsnBoolean  partialPictureFreezeAndRelease ;
 	 VS_AsnBoolean  resizingPartPicFreezeAndRelease ;
 	 VS_AsnBoolean  fullPictureSnapshot ;
 	 VS_AsnBoolean  partialPictureSnapshot ;
 	 VS_AsnBoolean  videoSegmentTagging ;
 	 VS_AsnBoolean  progressiveRefinement ;
 	 VS_AsnBoolean  dynamicPictureResizingByFour ;
 	 VS_AsnBoolean  dynamicPictureResizingSixteenthPel ;
 	 VS_AsnBoolean  dynamicWarpingHalfPel ;
 	 VS_AsnBoolean  dynamicWarpingSixteenthPel ;
 	 VS_AsnBoolean  independentSegmentDecoding ;
 	 VS_AsnBoolean  slicesInOrder_NonRect ;
 	 VS_AsnBoolean  slicesInOrder_Rect ;
 	 VS_AsnBoolean  slicesNoOrder_NonRect ;
 	 VS_AsnBoolean  slicesNoOrder_Rect ;
 	 VS_AsnBoolean  alternateInterVLCMode ;
 	 VS_AsnBoolean  modifiedQuantizationMode ;
 	 VS_AsnBoolean  reducedResolutionUpdate ;
 	 VS_H245TransparencyParameters  transparencyParameters ;
 	 VS_AsnBoolean  separateVideoBackChannel ;
 	 VS_H245RefPictureSelection  refPictureSelection ;
 	Constrained_array_of_type<  VS_H245CustomPictureClockFrequency ,1,16,VS_Asn::FixedConstraint,0  >  customPictureClockFrequency ;
 	Constrained_array_of_type<  VS_H245CustomPictureFormat ,1,16,VS_Asn::FixedConstraint,0  >  customPictureFormat ;
 	Constrained_array_of_type<  VS_H245H263VideoModeCombos ,1,16,VS_Asn::FixedConstraint,0  >  modeCombos ;
 	 VS_AsnBoolean  videoBadMBsCap ;
 	 VS_H245H263Version3Options  h263Version3Options ;
 	void operator=(const VS_H245H263Options& src);

};
//////////////////////CLASS VS_H245EnhancementOptions /////////////////////////

struct VS_H245EnhancementOptions : public VS_AsnSequence
{
	 VS_H245EnhancementOptions( void );

	static const unsigned basic_root = 16;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  sqcifMPI ;
 	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  qcifMPI ;
 	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  cifMPI ;
 	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  cif4MPI ;
 	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  cif16MPI ;
 	TemplInteger<1,192400,VS_Asn::FixedConstraint,0>  maxBitRate ;
 	 VS_AsnBoolean  unrestrictedVector ;
 	 VS_AsnBoolean  arithmeticCoding ;
 	 VS_AsnBoolean  temporalSpatialTradeOffCapability ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowSqcifMPI ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowQcifMPI ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowCifMPI ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowCif4MPI ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowCif16MPI ;
 	 VS_AsnBoolean  errorCompensation ;
 	 VS_H245H263Options  h263Options ;
 	void operator=(const VS_H245EnhancementOptions& src);

};
//////////////////////CLASS VS_H245BEnhancementParameters /////////////////////////

struct VS_H245BEnhancementParameters : public VS_AsnSequence
{
	 VS_H245BEnhancementParameters( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245EnhancementOptions  enhancementOptions ;
 	TemplInteger<1,64,VS_Asn::FixedConstraint,0>  numberOfBPictures ;
 	void operator=(const VS_H245BEnhancementParameters& src);

};
//////////////////////CLASS VS_H245EnhancementLayerInfo /////////////////////////

struct VS_H245EnhancementLayerInfo : public VS_AsnSequence
{
	 VS_H245EnhancementLayerInfo( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  baseBitRateConstrained ;
 	Constrained_array_of_type<  VS_H245EnhancementOptions ,1,14,VS_Asn::FixedConstraint,0  >  snrEnhancement ;
 	Constrained_array_of_type<  VS_H245EnhancementOptions ,1,14,VS_Asn::FixedConstraint,0  >  spatialEnhancement ;
 	Constrained_array_of_type<  VS_H245BEnhancementParameters ,1,14,VS_Asn::FixedConstraint,0  >  bPictureEnhancement ;
 	void operator=(const VS_H245EnhancementLayerInfo& src);

};

//////////////////////CLASS VS_H245H263VideoMode /////////////////////////

struct VS_H245H263VideoMode : public VS_AsnSequence
{
	 VS_H245H263VideoMode( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 3;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H245H263VideoMode_Resolution	 resolution ;
 	TemplInteger<1,19200,VS_Asn::FixedConstraint,0>  bitRate ;
 	 VS_AsnBoolean  unrestrictedVector ;
 	 VS_AsnBoolean  arithmeticCoding ;
 	 VS_AsnBoolean  advancedPrediction ;
 	 VS_AsnBoolean  pbFrames ;
 	 VS_AsnBoolean  errorCompensation ;
 	 VS_H245EnhancementLayerInfo  enhancementLayerInfo ;
 	 VS_H245H263Options  h263Options ;
 	void operator=(const VS_H245H263VideoMode& src);

};

//////////////////////CLASS VS_H245H263VideoCapability /////////////////////////

struct VS_H245H263VideoCapability : public VS_AsnSequence
{
	 VS_H245H263VideoCapability( void );

	static const unsigned basic_root = 13;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 8;
	VS_Reference_of_Asn e_ref[extension_root];

	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  sqcifMPI ;
 	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  qcifMPI ;
 	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  cifMPI ;
 	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  cif4MPI ;
 	TemplInteger<1,32,VS_Asn::FixedConstraint,0>  cif16MPI ;
 	TemplInteger<1,192400,VS_Asn::FixedConstraint,0>  maxBitRate ;
 	 VS_AsnBoolean  unrestrictedVector ;
 	 VS_AsnBoolean  arithmeticCoding ;
 	 VS_AsnBoolean  advancedPrediction ;
 	 VS_AsnBoolean  pbFrames ;
 	 VS_AsnBoolean  temporalSpatialTradeOffCapability ;
 	TemplInteger<0,524287,VS_Asn::FixedConstraint,0>  hrd_B ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  bppMaxKb ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowSqcifMPI ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowQcifMPI ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowCifMPI ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowCif4MPI ;
 	TemplInteger<1,3600,VS_Asn::FixedConstraint,0>  slowCif16MPI ;
 	 VS_AsnBoolean  errorCompensation ;
 	 VS_H245EnhancementLayerInfo  enhancementLayerInfo ;
 	 VS_H245H263Options  h263Options ;
 	void operator=(const VS_H245H263VideoCapability& src);

};
//////////////////////CLASS VS_H245H262VideoCapability /////////////////////////

struct VS_H245H262VideoCapability : public VS_AsnSequence
{
	 VS_H245H262VideoCapability( void );

	static const unsigned basic_root = 18;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  profileAndLevel_SPatML ;
 	 VS_AsnBoolean  profileAndLevel_MPatLL ;
 	 VS_AsnBoolean  profileAndLevel_MPatML ;
 	 VS_AsnBoolean  profileAndLevel_MPatH_14 ;
 	 VS_AsnBoolean  profileAndLevel_MPatHL ;
 	 VS_AsnBoolean  profileAndLevel_SNRatLL ;
 	 VS_AsnBoolean  profileAndLevel_SNRatML ;
 	 VS_AsnBoolean  profileAndLevel_SpatialatH_14 ;
 	 VS_AsnBoolean  profileAndLevel_HPatML ;
 	 VS_AsnBoolean  profileAndLevel_HPatH_14 ;
 	 VS_AsnBoolean  profileAndLevel_HPatHL ;
 	TemplInteger<1073741823,1073741823,VS_Asn::FixedConstraint,0>  videoBitRate ;
 	TemplInteger<262143,262143,VS_Asn::FixedConstraint,0>  vbvBufferSize ;
 	TemplInteger<0,16383,VS_Asn::FixedConstraint,0>  samplesPerLine ;
 	TemplInteger<0,16383,VS_Asn::FixedConstraint,0>  linesPerFrame ;
 	TemplInteger<0,15,VS_Asn::FixedConstraint,0>  framesPerSecond ;
 	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  luminanceSampleRate ;
 	 VS_AsnBoolean  videoBadMBsCap ;
 	void operator=(const VS_H245H262VideoCapability& src);

};
//////////////////////CLASS VS_H245H261VideoCapability /////////////////////////

struct VS_H245H261VideoCapability : public VS_AsnSequence
{
	 VS_H245H261VideoCapability( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	TemplInteger<1,4,VS_Asn::FixedConstraint,0>  qcifMPI ;
 	TemplInteger<1,4,VS_Asn::FixedConstraint,0>  cifMPI ;
 	 VS_AsnBoolean  temporalSpatialTradeOffCapability ;
 	TemplInteger<1,19200,VS_Asn::FixedConstraint,0>  maxBitRate ;
 	 VS_AsnBoolean  stillImageTransmission ;
 	 VS_AsnBoolean  videoBadMBsCap ;
 	void operator=(const VS_H245H261VideoCapability& src);

};
//////////////////////CLASS VS_H245ExtendedVideoCapability /////////////////////////

struct VS_H245ExtendedVideoCapability : public VS_AsnSequence
{
	 VS_H245ExtendedVideoCapability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245VideoCapability ,0,INT_MAX,VS_Asn::Unconstrained,0  >  videoCapability ;
 	Constrained_array_of_type<  VS_H245GenericCapability ,0,INT_MAX,VS_Asn::Unconstrained,0  >  videoCapabilityExtension ;
 	void operator=(const VS_H245ExtendedVideoCapability& src);

};
//////////////////////CLASS VS_H245VideoCapability /////////////////////////

struct VS_H245VideoCapability : public VS_AsnChoice
{
	 VS_H245VideoCapability( void );

 	enum{
	e_nonStandard,
	e_h261VideoCapability,
	e_h262VideoCapability,
	e_h263VideoCapability,
	e_is11172VideoCapability,
	e_genericVideoCapability,
	e_extendedVideoCapability
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245VideoCapability & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245H261VideoCapability *( void );
	 operator VS_H245H262VideoCapability *( void );
	 operator VS_H245H263VideoCapability *( void );
	 operator VS_H245IS11172VideoCapability *( void );
	 operator VS_H245GenericCapability *( void );
	 operator VS_H245ExtendedVideoCapability *( void );

	 void Show(void) const;

};

//////////////////////CLASS VS_H245MediaDistributionCapability /////////////////////////

struct VS_H245MediaDistributionCapability : public VS_AsnSequence
{
	 VS_H245MediaDistributionCapability( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  centralizedControl ;
 	 VS_AsnBoolean  distributedControl ;
 	 VS_AsnBoolean  centralizedAudio ;
 	 VS_AsnBoolean  distributedAudio ;
 	 VS_AsnBoolean  centralizedVideo ;
 	 VS_AsnBoolean  distributedVideo ;
 	Constrained_array_of_type<  VS_H245DataApplicationCapability ,0,INT_MAX,VS_Asn::Unconstrained,0  >  centralizedData ;
 	Constrained_array_of_type<  VS_H245DataApplicationCapability ,0,INT_MAX,VS_Asn::Unconstrained,0  >  distributedData ;
 	void operator=(const VS_H245MediaDistributionCapability& src);

};
//////////////////////CLASS VS_H245MultipointCapability /////////////////////////

struct VS_H245MultipointCapability : public VS_AsnSequence
{
	 VS_H245MultipointCapability( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  multicastCapability ;
 	 VS_AsnBoolean  multiUniCastConference ;
 	Constrained_array_of_type<  VS_H245MediaDistributionCapability ,0,INT_MAX,VS_Asn::Unconstrained,0  >  mediaDistributionCapability ;
 	void operator=(const VS_H245MultipointCapability& src);

};
//////////////////////CLASS VS_H245RTPH263VideoRedundancyFrameMapping /////////////////////////

struct VS_H245RTPH263VideoRedundancyFrameMapping : public VS_AsnSequence
{
	 VS_H245RTPH263VideoRedundancyFrameMapping( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,15,VS_Asn::FixedConstraint,0>  threadNumber ;
 	Constrained_array_of_type< TemplInteger<0,255,VS_Asn::FixedConstraint,0> ,1,256,VS_Asn::FixedConstraint,0  >  frameSequence ;
 	void operator=(const VS_H245RTPH263VideoRedundancyFrameMapping& src);

};
//////////////////////CLASS VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping /////////////////////////

struct VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping : public VS_AsnChoice
{
	 VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping( void );

 	enum{
	e_roundrobin,
	e_custom
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping & src);

	 operator VS_H245RTPH263VideoRedundancyFrameMapping *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245RTPH263VideoRedundancyEncoding /////////////////////////

struct VS_H245RTPH263VideoRedundancyEncoding : public VS_AsnSequence
{
	 VS_H245RTPH263VideoRedundancyEncoding( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,16,VS_Asn::FixedConstraint,0>  numberOfThreads ;
 	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  framesBetweenSyncPoints ;
 	VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping	 frameToThreadMapping ;
 	Constrained_array_of_type< TemplInteger<0,15,VS_Asn::FixedConstraint,0> ,1,256,VS_Asn::FixedConstraint,0  >  containedThreads ;
 	void operator=(const VS_H245RTPH263VideoRedundancyEncoding& src);

};
//////////////////////CLASS VS_H245RedundancyEncodingMethod /////////////////////////

struct VS_H245RedundancyEncodingMethod : public VS_AsnChoice
{
	 VS_H245RedundancyEncodingMethod( void );

 	enum{
	e_nonStandard,
	e_rtpAudioRedundancyEncoding,
	e_rtpH263VideoRedundancyEncoding
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RedundancyEncodingMethod & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245RTPH263VideoRedundancyEncoding *( void );

	void Show( void ) const;

};
//////////////////////CLASS VS_H245RedundancyEncoding /////////////////////////

struct VS_H245RedundancyEncoding : public VS_AsnSequence
{
	 VS_H245RedundancyEncoding( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245RedundancyEncodingMethod  redundancyEncodingMethod ;
 	 VS_H245DataType  secondaryEncoding ;
 	VS_H245RedundancyEncoding_RtpRedundancyEncoding	 rtpRedundancyEncoding ;
 	void operator=(const VS_H245RedundancyEncoding& src);

};
//////////////////////CLASS VS_H245CommunicationModeTableEntry /////////////////////////

struct VS_H245CommunicationModeTableEntry : public VS_AsnSequence
{
	 VS_H245CommunicationModeTableEntry( void );

	static const unsigned basic_root = 10;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 3;
	VS_Reference_of_Asn e_ref[extension_root];

	Constrained_array_of_type<  VS_H245NonStandardParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  nonStandard ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  sessionID ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  associatedSessionID ;
 	 VS_H245TerminalLabel  terminalLabel ;
 	TemplBmpString<1,128,VS_Asn::FixedConstraint,0>  sessionDescription ;
 	VS_H245CommunicationModeTableEntry_DataType	 dataType ;
 	 VS_H245TransportAddress  mediaChannel ;
 	 VS_AsnBoolean  mediaGuaranteedDelivery ;
 	 VS_H245TransportAddress  mediaControlChannel ;
 	 VS_AsnBoolean  mediaControlGuaranteedDelivery ;
 	 VS_H245RedundancyEncoding  redundancyEncoding ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  sessionDependency ;
 	 VS_H245TerminalLabel  destination ;
 	void operator=(const VS_H245CommunicationModeTableEntry& src);

};


//////////////////////CLASS VS_H245RedundancyEncodingDTMode /////////////////////////

struct VS_H245RedundancyEncodingDTMode : public VS_AsnSequence
{
	 VS_H245RedundancyEncodingDTMode( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245RedundancyEncodingMethod  redundancyEncodingMethod ;
 	 VS_H245RedundancyEncodingDTModeElement  primary ;
 	Constrained_array_of_type<  VS_H245RedundancyEncodingDTModeElement ,0,INT_MAX,VS_Asn::Unconstrained,0  >  secondary ;
 	void operator=(const VS_H245RedundancyEncodingDTMode& src);

};
//////////////////////CLASS VS_H245RedundancyEncodingMode /////////////////////////

struct VS_H245RedundancyEncodingMode : public VS_AsnSequence
{
	 VS_H245RedundancyEncodingMode( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245RedundancyEncodingMethod  redundancyEncodingMethod ;
 	VS_H245RedundancyEncodingMode_SecondaryEncoding	 secondaryEncoding ;
 	void operator=(const VS_H245RedundancyEncodingMode& src);

};

//////////////////////CLASS VS_H245H2250ModeParameters /////////////////////////

struct VS_H245H2250ModeParameters : public VS_AsnSequence
{
	 VS_H245H2250ModeParameters( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245RedundancyEncodingMode  redundancyEncodingMode ;
 	void operator=(const VS_H245H2250ModeParameters& src);

};

//////////////////////CLASS VS_H245ModeElement /////////////////////////

struct VS_H245ModeElement : public VS_AsnSequence
{
	 VS_H245ModeElement( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245ModeElementType  type ;
 	 VS_H245H223ModeParameters  h223ModeParameters ;
 	 VS_H245V76ModeParameters  v76ModeParameters ;
 	 VS_H245H2250ModeParameters  h2250ModeParameters ;
 	 VS_H245GenericCapability  genericModeParameters ;
 	 VS_H245MultiplexedStreamModeParameters  multiplexedStreamModeParameters ;
 	 VS_H245LogicalChannelNumber  logicalChannelNumber ;
 	void operator=(const VS_H245ModeElement& src);

};

//////////////////////CLASS VS_H245ModeDescription /////////////////////////

typedef Constrained_array_of_type<  VS_H245ModeElement ,1,256,VS_Asn::FixedConstraint,0  >  VS_H245ModeDescription;

//////////////////////CLASS VS_H245RequestMode /////////////////////////

struct VS_H245RequestMode : public VS_AsnSequence
{
	 VS_H245RequestMode( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	Constrained_array_of_type<  VS_H245ModeDescription ,1,256,VS_Asn::FixedConstraint,0  >  requestedModes ;
 	void operator=(const VS_H245RequestMode& src);

};


//////////////////////CLASS VS_H245RedundancyEncodingCapability /////////////////////////

struct VS_H245RedundancyEncodingCapability : public VS_AsnSequence
{
	 VS_H245RedundancyEncodingCapability( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245RedundancyEncodingMethod  redundancyEncodingMethod ;
 	 VS_H245CapabilityTableEntryNumber  primaryEncoding ;
 	Constrained_array_of_type<  VS_H245CapabilityTableEntryNumber ,1,256,VS_Asn::FixedConstraint,0  >  secondaryEncoding ;
 	void operator=(const VS_H245RedundancyEncodingCapability& src);

};




//////////////////////CLASS VS_H245MediaTransportType_Atm_AAL5_compressed /////////////////////////

struct VS_H245MediaTransportType_Atm_AAL5_compressed : public VS_AsnSequence
{
	 VS_H245MediaTransportType_Atm_AAL5_compressed( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  variable_delta ;
 	void operator=(const VS_H245MediaTransportType_Atm_AAL5_compressed& src);

};

//////////////////////CLASS VS_H245RSVPParameters /////////////////////////

struct VS_H245RSVPParameters : public VS_AsnSequence
{
	 VS_H245RSVPParameters( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245QOSMode  qosMode ;
 	TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  tokenRate ;
 	TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  bucketSize ;
 	TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  peakRate ;
 	TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  minPoliced ;
 	TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  maxPktSize ;
 	void operator=(const VS_H245RSVPParameters& src);

};

//////////////////////CLASS VS_H245QOSCapability /////////////////////////

struct VS_H245QOSCapability : public VS_AsnSequence
{
	 VS_H245QOSCapability( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245NonStandardParameter  nonStandardData ;
 	 VS_H245RSVPParameters  rsvpParameters ;
 	 VS_H245ATMParameters  atmParameters ;
 	void operator=(const VS_H245QOSCapability& src);

};


//////////////////////CLASS VS_H245TransportCapability /////////////////////////

struct VS_H245TransportCapability : public VS_AsnSequence
{
	 VS_H245TransportCapability( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245NonStandardParameter  nonStandard ;
 	Constrained_array_of_type<  VS_H245QOSCapability ,1,256,VS_Asn::FixedConstraint,0  >  qOSCapabilities ;
 	Constrained_array_of_type<  VS_H245MediaChannelCapability ,1,256,VS_Asn::FixedConstraint,0  >  mediaChannelCapabilities ;
 	void operator=(const VS_H245TransportCapability& src);

};

//////////////////////CLASS VS_H245H2250LogicalChannelParameters /////////////////////////

struct VS_H245H2250LogicalChannelParameters : public VS_AsnSequence
{
	 VS_H245H2250LogicalChannelParameters( void );

	static const unsigned basic_root = 11;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 3;
	VS_Reference_of_Asn e_ref[extension_root];

	Constrained_array_of_type<  VS_H245NonStandardParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  nonStandard ;
 	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  sessionID ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  associatedSessionID ;
 	 VS_H245TransportAddress  mediaChannel ;
 	 VS_AsnBoolean  mediaGuaranteedDelivery ;
 	 VS_H245TransportAddress  mediaControlChannel ;
 	 VS_AsnBoolean  mediaControlGuaranteedDelivery ;
 	 VS_AsnBoolean  silenceSuppression ;
 	 VS_H245TerminalLabel  destination ;
 	TemplInteger<96,127,VS_Asn::FixedConstraint,0>  dynamicRTPPayloadType ;
 	VS_H245H2250LogicalChannelParameters_MediaPacketization	 mediaPacketization ;
 	 VS_H245TransportCapability  transportCapability ;
 	 VS_H245RedundancyEncoding  redundancyEncoding ;
 	 VS_H245TerminalLabel  source ;
 	void operator=(const VS_H245H2250LogicalChannelParameters& src);

};
//////////////////////CLASS VS_H245RequestChannelClose /////////////////////////

struct VS_H245RequestChannelClose : public VS_AsnSequence
{
	 VS_H245RequestChannelClose( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H245LogicalChannelNumber  forwardLogicalChannelNumber ;
 	 VS_H245QOSCapability  qosCapability ;
 	VS_H245RequestChannelClose_Reason	 reason ;
 	void operator=(const VS_H245RequestChannelClose& src);

};



//////////////////////CLASS VS_H245MediaPacketizationCapability /////////////////////////

struct VS_H245MediaPacketizationCapability : public VS_AsnSequence
{
	 VS_H245MediaPacketizationCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  h261aVideoPacketization ;
 	Constrained_array_of_type<  VS_H245RTPPayloadType ,1,256,VS_Asn::FixedConstraint,0  >  rtpPayloadType ;
 	void operator=(const VS_H245MediaPacketizationCapability& src);

};
//////////////////////CLASS VS_H245H2250Capability_McCapability /////////////////////////

struct VS_H245H2250Capability_McCapability : public VS_AsnSequence
{
	 VS_H245H2250Capability_McCapability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  centralizedConferenceMC ;
 	 VS_AsnBoolean  decentralizedConferenceMC ;
 	void operator=(const VS_H245H2250Capability_McCapability& src);

};
//////////////////////CLASS VS_H245H2250Capability /////////////////////////

struct VS_H245H2250Capability : public VS_AsnSequence
{
	 VS_H245H2250Capability( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];

	TemplInteger<0,1023,VS_Asn::FixedConstraint,0>  maximumAudioDelayJitter ;
 	 VS_H245MultipointCapability  receiveMultipointCapability ;
 	 VS_H245MultipointCapability  transmitMultipointCapability ;
 	 VS_H245MultipointCapability  receiveAndTransmitMultipointCapability ;
 	VS_H245H2250Capability_McCapability	 mcCapability ;
 	 VS_AsnBoolean  rtcpVideoControlCapability ;
 	 VS_H245MediaPacketizationCapability  mediaPacketizationCapability ;
 	 VS_H245TransportCapability  transportCapability ;
 	Constrained_array_of_type<  VS_H245RedundancyEncodingCapability ,1,256,VS_Asn::FixedConstraint,0  >  redundancyEncodingCapability ;
 	 VS_AsnBoolean  logicalChannelSwitchingCapability ;
 	 VS_AsnBoolean  t120DynamicPortCapability ;
 	void operator=(const VS_H245H2250Capability& src);

};
//////////////////////CLASS VS_H245V75Capability /////////////////////////

struct VS_H245V75Capability : public VS_AsnSequence
{
	 VS_H245V75Capability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  audioHeader ;
 	void operator=(const VS_H245V75Capability& src);

};
//////////////////////CLASS VS_H245V76Capability /////////////////////////

struct VS_H245V76Capability : public VS_AsnSequence
{
	 VS_H245V76Capability( void );

	static const unsigned basic_root = 15;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  suspendResumeCapabilitywAddress ;
 	 VS_AsnBoolean  suspendResumeCapabilitywoAddress ;
 	 VS_AsnBoolean  rejCapability ;
 	 VS_AsnBoolean  sREJCapability ;
 	 VS_AsnBoolean  mREJCapability ;
 	 VS_AsnBoolean  crc8bitCapability ;
 	 VS_AsnBoolean  crc16bitCapability ;
 	 VS_AsnBoolean  crc32bitCapability ;
 	 VS_AsnBoolean  uihCapability ;
 	TemplInteger<2,8191,VS_Asn::FixedConstraint,0>  numOfDLCS ;
 	 VS_AsnBoolean  twoOctetAddressFieldCapability ;
 	 VS_AsnBoolean  loopBackTestCapability ;
 	TemplInteger<1,4095,VS_Asn::FixedConstraint,0>  n401Capability ;
 	TemplInteger<1,127,VS_Asn::FixedConstraint,0>  maxWindowSizeCapability ;
 	 VS_H245V75Capability  v75Capability ;
 	void operator=(const VS_H245V76Capability& src);

};
//////////////////////CLASS VS_H245H223AnnexCCapability /////////////////////////

struct VS_H245H223AnnexCCapability : public VS_AsnSequence
{
	 VS_H245H223AnnexCCapability( void );

	static const unsigned basic_root = 13;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  videoWithAL1M ;
 	 VS_AsnBoolean  videoWithAL2M ;
 	 VS_AsnBoolean  videoWithAL3M ;
 	 VS_AsnBoolean  audioWithAL1M ;
 	 VS_AsnBoolean  audioWithAL2M ;
 	 VS_AsnBoolean  audioWithAL3M ;
 	 VS_AsnBoolean  dataWithAL1M ;
 	 VS_AsnBoolean  dataWithAL2M ;
 	 VS_AsnBoolean  dataWithAL3M ;
 	 VS_AsnBoolean  alpduInterleaving ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  maximumAL1MPDUSize ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  maximumAL2MSDUSize ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  maximumAL3MSDUSize ;
 	 VS_AsnBoolean  rsCodeCapability ;
 	void operator=(const VS_H245H223AnnexCCapability& src);

};
//////////////////////CLASS VS_H245H223Capability_H223MultiplexTableCapability_Enhanced /////////////////////////

struct VS_H245H223Capability_H223MultiplexTableCapability_Enhanced : public VS_AsnSequence
{
	 VS_H245H223Capability_H223MultiplexTableCapability_Enhanced( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,15,VS_Asn::FixedConstraint,0>  maximumNestingDepth ;
 	TemplInteger<2,255,VS_Asn::FixedConstraint,0>  maximumElementListSize ;
 	TemplInteger<2,255,VS_Asn::FixedConstraint,0>  maximumSubElementListSize ;
 	void operator=(const VS_H245H223Capability_H223MultiplexTableCapability_Enhanced& src);

};
//////////////////////CLASS VS_H245H223Capability_H223MultiplexTableCapability /////////////////////////

struct VS_H245H223Capability_H223MultiplexTableCapability : public VS_AsnChoice
{
	 VS_H245H223Capability_H223MultiplexTableCapability( void );

 	enum{
	e_basic,
	e_enhanced
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245H223Capability_H223MultiplexTableCapability & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245H223Capability_MobileOperationTransmitCapability /////////////////////////

struct VS_H245H223Capability_MobileOperationTransmitCapability : public VS_AsnSequence
{
	 VS_H245H223Capability_MobileOperationTransmitCapability( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  modeChangeCapability ;
 	 VS_AsnBoolean  h223AnnexA ;
 	 VS_AsnBoolean  h223AnnexADoubleFlag ;
 	 VS_AsnBoolean  h223AnnexB ;
 	 VS_AsnBoolean  h223AnnexBwithHeader ;
 	void operator=(const VS_H245H223Capability_MobileOperationTransmitCapability& src);

};
//////////////////////CLASS VS_H245H223Capability_MobileMultilinkFrameCapability /////////////////////////

struct VS_H245H223Capability_MobileMultilinkFrameCapability : public VS_AsnSequence
{
	 VS_H245H223Capability_MobileMultilinkFrameCapability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  maximumSampleSize ;
 	TemplInteger<1,65025,VS_Asn::FixedConstraint,0>  maximumPayloadLength ;
 	void operator=(const VS_H245H223Capability_MobileMultilinkFrameCapability& src);

};
//////////////////////CLASS VS_H245H223Capability /////////////////////////

struct VS_H245H223Capability : public VS_AsnSequence
{
	 VS_H245H223Capability( void );

	static const unsigned basic_root = 14;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 6;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  transportWithI_frames ;
 	 VS_AsnBoolean  videoWithAL1 ;
 	 VS_AsnBoolean  videoWithAL2 ;
 	 VS_AsnBoolean  videoWithAL3 ;
 	 VS_AsnBoolean  audioWithAL1 ;
 	 VS_AsnBoolean  audioWithAL2 ;
 	 VS_AsnBoolean  audioWithAL3 ;
 	 VS_AsnBoolean  dataWithAL1 ;
 	 VS_AsnBoolean  dataWithAL2 ;
 	 VS_AsnBoolean  dataWithAL3 ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  maximumAl2SDUSize ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  maximumAl3SDUSize ;
 	TemplInteger<0,1023,VS_Asn::FixedConstraint,0>  maximumDelayJitter ;
 	VS_H245H223Capability_H223MultiplexTableCapability	 h223MultiplexTableCapability ;
 	 VS_AsnBoolean  maxMUXPDUSizeCapability ;
 	 VS_AsnBoolean  nsrpSupport ;
 	VS_H245H223Capability_MobileOperationTransmitCapability	 mobileOperationTransmitCapability ;
 	 VS_H245H223AnnexCCapability  h223AnnexCCapability ;
 	TemplInteger<1,19200,VS_Asn::FixedConstraint,0>  bitRate ;
 	VS_H245H223Capability_MobileMultilinkFrameCapability	 mobileMultilinkFrameCapability ;
 	void operator=(const VS_H245H223Capability& src);

};
//////////////////////CLASS VS_H245VCCapability_Aal1 /////////////////////////

struct VS_H245VCCapability_Aal1 : public VS_AsnSequence
{
	 VS_H245VCCapability_Aal1( void );

	static const unsigned basic_root = 9;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  nullClockRecovery ;
 	 VS_AsnBoolean  srtsClockRecovery ;
 	 VS_AsnBoolean  adaptiveClockRecovery ;
 	 VS_AsnBoolean  nullErrorCorrection ;
 	 VS_AsnBoolean  longInterleaver ;
 	 VS_AsnBoolean  shortInterleaver ;
 	 VS_AsnBoolean  errorCorrectionOnly ;
 	 VS_AsnBoolean  structuredDataTransfer ;
 	 VS_AsnBoolean  partiallyFilledCells ;
 	void operator=(const VS_H245VCCapability_Aal1& src);

};
//////////////////////CLASS VS_H245VCCapability_Aal5 /////////////////////////

struct VS_H245VCCapability_Aal5 : public VS_AsnSequence
{
	 VS_H245VCCapability_Aal5( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  forwardMaximumSDUSize ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  backwardMaximumSDUSize ;
 	void operator=(const VS_H245VCCapability_Aal5& src);

};
//////////////////////CLASS VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates /////////////////////////

struct VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates : public VS_AsnSequence
{
	 VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  lowerBitRate ;
 	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  higherBitRate ;
 	void operator=(const VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates& src);

};
//////////////////////CLASS VS_H245VCCapability_AvailableBitRates_Type /////////////////////////

struct VS_H245VCCapability_AvailableBitRates_Type : public VS_AsnChoice
{
	 VS_H245VCCapability_AvailableBitRates_Type( void );

 	enum{
	e_singleBitRate,
	e_rangeOfBitRates
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245VCCapability_AvailableBitRates_Type & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245VCCapability_AvailableBitRates /////////////////////////

struct VS_H245VCCapability_AvailableBitRates : public VS_AsnSequence
{
	 VS_H245VCCapability_AvailableBitRates( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245VCCapability_AvailableBitRates_Type	 type ;
 	void operator=(const VS_H245VCCapability_AvailableBitRates& src);

};
//////////////////////CLASS VS_H245VCCapability_Aal1ViaGateway /////////////////////////

struct VS_H245VCCapability_Aal1ViaGateway : public VS_AsnSequence
{
	 VS_H245VCCapability_Aal1ViaGateway( void );

	static const unsigned basic_root = 10;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H245Q2931Address ,1,256,VS_Asn::FixedConstraint,0  >  gatewayAddress ;
 	 VS_AsnBoolean  nullClockRecovery ;
 	 VS_AsnBoolean  srtsClockRecovery ;
 	 VS_AsnBoolean  adaptiveClockRecovery ;
 	 VS_AsnBoolean  nullErrorCorrection ;
 	 VS_AsnBoolean  longInterleaver ;
 	 VS_AsnBoolean  shortInterleaver ;
 	 VS_AsnBoolean  errorCorrectionOnly ;
 	 VS_AsnBoolean  structuredDataTransfer ;
 	 VS_AsnBoolean  partiallyFilledCells ;
 	void operator=(const VS_H245VCCapability_Aal1ViaGateway& src);

};
//////////////////////CLASS VS_H245VCCapability /////////////////////////

struct VS_H245VCCapability : public VS_AsnSequence
{
	 VS_H245VCCapability( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H245VCCapability_Aal1	 aal1 ;
 	VS_H245VCCapability_Aal5	 aal5 ;
 	 VS_AsnBoolean  transportStream ;
 	 VS_AsnBoolean  programStream ;
 	VS_H245VCCapability_AvailableBitRates	 availableBitRates ;
 	VS_H245VCCapability_Aal1ViaGateway	 aal1ViaGateway ;
 	void operator=(const VS_H245VCCapability& src);

};
//////////////////////CLASS VS_H245H222Capability /////////////////////////

struct VS_H245H222Capability : public VS_AsnSequence
{
	 VS_H245H222Capability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  numberOfVCs ;
 	Constrained_array_of_type<  VS_H245VCCapability ,0,INT_MAX,VS_Asn::Unconstrained,0  >  vcCapability ;
 	void operator=(const VS_H245H222Capability& src);

};
//////////////////////CLASS VS_H245MultiplexCapability /////////////////////////

struct VS_H245MultiplexCapability : public VS_AsnChoice
{
	 VS_H245MultiplexCapability( void );

 	enum{
	e_nonStandard,
	e_h222Capability,
	e_h223Capability,
	e_v76Capability,
	e_h2250Capability,
	e_genericMultiplexCapability
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultiplexCapability & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245H222Capability *( void );
	 operator VS_H245H223Capability *( void );
	 operator VS_H245V76Capability *( void );
	 operator VS_H245H2250Capability *( void );
	 operator VS_H245GenericCapability *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245H235SecurityCapability /////////////////////////

struct VS_H245H235SecurityCapability : public VS_AsnSequence
{
	 VS_H245H235SecurityCapability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245EncryptionAuthenticationAndIntegrity  encryptionAuthenticationAndIntegrity ;
 	 VS_H245CapabilityTableEntryNumber  mediaCapability ;
 	void operator=(const VS_H245H235SecurityCapability& src);

};
//////////////////////CLASS VS_H245Capability_H233EncryptionReceiveCapability /////////////////////////

struct VS_H245Capability_H233EncryptionReceiveCapability : public VS_AsnSequence
{
	 VS_H245Capability_H233EncryptionReceiveCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  h233IVResponseTime ;
 	void operator=(const VS_H245Capability_H233EncryptionReceiveCapability& src);

};
//////////////////////CLASS VS_H245Capability /////////////////////////

struct VS_H245Capability : public VS_AsnChoice
{
	 VS_H245Capability( void );

 	enum{
	e_nonStandard,
	e_receiveVideoCapability,
	e_transmitVideoCapability,
	e_receiveAndTransmitVideoCapability,
	e_receiveAudioCapability,
	e_transmitAudioCapability,
	e_receiveAndTransmitAudioCapability,
	e_receiveDataApplicationCapability,
	e_transmitDataApplicationCapability,
	e_receiveAndTransmitDataApplicationCapability,
	e_h233EncryptionTransmitCapability,
	e_h233EncryptionReceiveCapability,
	e_conferenceCapability,
	e_h235SecurityCapability,
	e_maxPendingReplacementFor,
	e_receiveUserInputCapability,
	e_transmitUserInputCapability,
	e_receiveAndTransmitUserInputCapability,
	e_genericControlCapability,
	e_receiveMultiplexedStreamCapability,
	e_transmitMultiplexedStreamCapability,
	e_receiveAndTransmitMultiplexedStreamCapability,
	e_receiveRTPAudioTelephonyEventCapability,
	e_receiveRTPAudioToneCapability,
	e_depFfecCapability,
	e_multiplePayloadStreamCapability,
	e_fecCapability,
	e_redundancyEncodingCap,
	e_oneOfCapabilities
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245Capability & src);

	 operator VS_H245NonStandardParameter *( void );
	 operator VS_H245VideoCapability *( void );
	 operator VS_H245AudioCapability *( void );
	 operator VS_H245DataApplicationCapability *( void );
	 operator VS_H245ConferenceCapability *( void );
	 operator VS_H245H235SecurityCapability *( void );
	 operator VS_H245UserInputCapability *( void );
	 operator VS_H245GenericCapability *( void );
	 operator VS_H245MultiplexedStreamCapability *( void );
	 operator VS_H245AudioTelephonyEventCapability *( void );
	 operator VS_H245AudioToneCapability *( void );
	 operator VS_H245DepFECCapability *( void );
	 operator VS_H245MultiplePayloadStreamCapability *( void );
	 operator VS_H245FECCapability *( void );
	 operator VS_H245RedundancyEncodingCapability *( void );
	 operator VS_H245AlternativeCapabilitySet *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245TerminalCapabilitySetRelease /////////////////////////

struct VS_H245TerminalCapabilitySetRelease : public VS_AsnSequence
{
	 VS_H245TerminalCapabilitySetRelease( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245TerminalCapabilitySetRelease& src);

};
//////////////////////CLASS VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded /////////////////////////

struct VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded : public VS_AsnChoice
{
	 VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded( void );

 	enum{
	e_highestEntryNumberProcessed,
	e_noneProcessed
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded & src);

	 operator VS_H245CapabilityTableEntryNumber *( void );

	 void Show(void) const;

};

//////////////////////CLASS VS_H245TerminalCapabilitySetReject_Cause /////////////////////////

struct VS_H245TerminalCapabilitySetReject_Cause : public VS_AsnChoice
{
	 VS_H245TerminalCapabilitySetReject_Cause( void );

 	enum{
	e_unspecified,
	e_undefinedTableEntryUsed,
	e_descriptorCapacityExceeded,
	e_tableEntryCapacityExceeded
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245TerminalCapabilitySetReject_Cause & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245TerminalCapabilitySetReject /////////////////////////

struct VS_H245TerminalCapabilitySetReject : public VS_AsnSequence
{
	 VS_H245TerminalCapabilitySetReject( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	VS_H245TerminalCapabilitySetReject_Cause	 cause ;
 	void operator=(const VS_H245TerminalCapabilitySetReject& src);

};
//////////////////////CLASS VS_H245TerminalCapabilitySetAck /////////////////////////

struct VS_H245TerminalCapabilitySetAck : public VS_AsnSequence
{
	 VS_H245TerminalCapabilitySetAck( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	void operator=(const VS_H245TerminalCapabilitySetAck& src);

};
//////////////////////CLASS VS_H245CapabilityDescriptor /////////////////////////

struct VS_H245CapabilityDescriptor : public VS_AsnSequence
{
	 VS_H245CapabilityDescriptor( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245CapabilityDescriptorNumber  capabilityDescriptorNumber ;
 	Constrained_array_of_type<  VS_H245AlternativeCapabilitySet ,1,256,VS_Asn::FixedConstraint,0  >  simultaneousCapabilities ;
 	void operator=(const VS_H245CapabilityDescriptor& src);

};
//////////////////////CLASS VS_H245CapabilityTableEntry /////////////////////////

struct VS_H245CapabilityTableEntry : public VS_AsnSequence
{
	 VS_H245CapabilityTableEntry( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245CapabilityTableEntryNumber  capabilityTableEntryNumber ;
 	 VS_H245Capability  capability ;
 	void operator=(const VS_H245CapabilityTableEntry& src);

};
//////////////////////CLASS VS_H245TerminalCapabilitySet /////////////////////////

struct VS_H245TerminalCapabilitySet : public VS_AsnSequence
{
	 VS_H245TerminalCapabilitySet( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245SequenceNumber  sequenceNumber ;
 	 VS_AsnObjectId  protocolIdentifier ;
 	 VS_H245MultiplexCapability  multiplexCapability ;
 	Constrained_array_of_type<  VS_H245CapabilityTableEntry ,1,256,VS_Asn::FixedConstraint,0  >  capabilityTable ;
 	Constrained_array_of_type<  VS_H245CapabilityDescriptor ,1,256,VS_Asn::FixedConstraint,0  >  capabilityDescriptors ;
 	void operator=(const VS_H245TerminalCapabilitySet& src);

};
//////////////////////CLASS VS_H245MasterSlaveDeterminationRelease /////////////////////////

struct VS_H245MasterSlaveDeterminationRelease : public VS_AsnSequence
{
	 VS_H245MasterSlaveDeterminationRelease( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H245MasterSlaveDeterminationRelease& src);

};
//////////////////////CLASS VS_H245MasterSlaveDeterminationReject_Cause /////////////////////////

struct VS_H245MasterSlaveDeterminationReject_Cause : public VS_AsnChoice
{
	 VS_H245MasterSlaveDeterminationReject_Cause( void );

 	enum{
	e_identicalNumbers
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MasterSlaveDeterminationReject_Cause & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MasterSlaveDeterminationReject /////////////////////////

struct VS_H245MasterSlaveDeterminationReject : public VS_AsnSequence
{
	 VS_H245MasterSlaveDeterminationReject( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245MasterSlaveDeterminationReject_Cause	 cause ;
 	void operator=(const VS_H245MasterSlaveDeterminationReject& src);

};
//////////////////////CLASS VS_H245MasterSlaveDeterminationAck_Decision /////////////////////////

struct VS_H245MasterSlaveDeterminationAck_Decision : public VS_AsnChoice
{
	 VS_H245MasterSlaveDeterminationAck_Decision( void );

 	enum{
	e_master,
	e_slave,
	e_indeterminate
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MasterSlaveDeterminationAck_Decision & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H245MasterSlaveDeterminationAck /////////////////////////

struct VS_H245MasterSlaveDeterminationAck : public VS_AsnSequence
{
	 VS_H245MasterSlaveDeterminationAck( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H245MasterSlaveDeterminationAck_Decision	 decision ;
 	void operator=(const VS_H245MasterSlaveDeterminationAck& src);

};
//////////////////////CLASS VS_H245MasterSlaveDetermination /////////////////////////

struct VS_H245MasterSlaveDetermination : public VS_AsnSequence
{
	 VS_H245MasterSlaveDetermination( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  terminalType ;
 	TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>  statusDeterminationNumber ;
 	void operator=(const VS_H245MasterSlaveDetermination& src);

};
//////////////////////CLASS VS_H245NonStandardIdentifier_H221NonStandard /////////////////////////

struct VS_H245NonStandardIdentifier_H221NonStandard : public VS_AsnSequence
{
	 VS_H245NonStandardIdentifier_H221NonStandard( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  t35CountryCode ;
 	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  t35Extension ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  manufacturerCode ;

    template <class TLog> void Show(TLog * tLog=0)
    {
        if (!tLog) return;
        tLog->CPrintf("\n\t H221NonStandard::t35CountryCode = %d",t35CountryCode.value);
        tLog->CPrintf("\n\t H221NonStandard::t35Extension = %d",t35Extension.value);
        tLog->CPrintf("\n\t H221NonStandard::manufacturerCode = %d",manufacturerCode.value);
    }

 	void operator=(const VS_H245NonStandardIdentifier_H221NonStandard& src);

};


//////////////////////CLASS VS_H245GenericMessage /////////////////////////

struct VS_H245GenericMessage : public VS_AsnSequence
{
	 VS_H245GenericMessage( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H245CapabilityIdentifier  messageIdentifier ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  subMessageIdentifier ;
 	Constrained_array_of_type<  VS_H245GenericParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  messageContent ;
 	void operator=(const VS_H245GenericMessage& src);

};

//////////////////////CLASS VS_H245IndicationMessage /////////////////////////

struct VS_H245IndicationMessage : public VS_AsnChoice
{
	 VS_H245IndicationMessage( void );

 	enum{
	e_nonStandard,
	e_functionNotUnderstood,
	e_masterSlaveDeterminationRelease,
	e_terminalCapabilitySetRelease,
	e_openLogicalChannelConfirm,
	e_requestChannelCloseRelease,
	e_multiplexEntrySendRelease,
	e_requestMultiplexEntryRelease,
	e_requestModeRelease,
	e_miscellaneousIndication,
	e_jitterIndication,
	e_h223SkewIndication,
	e_newATMVCIndication,
	e_userInput,
	e_h2250MaximumSkewIndication,
	e_mcLocationIndication,
	e_conferenceIndication,
	e_vendorIdentification,
	e_functionNotSupported,
	e_multilinkIndication,
	e_logicalChannelRateRelease,
	e_flowControlIndication,
	e_mobileMultilinkReconfigurationIndication,
	e_genericIndication
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245IndicationMessage & src);

	 operator VS_H245NonStandardMessage *( void );
	 operator VS_H245FunctionNotUnderstood *( void );
	 operator VS_H245MasterSlaveDeterminationRelease *( void );
	 operator VS_H245TerminalCapabilitySetRelease *( void );
	 operator VS_H245OpenLogicalChannelConfirm *( void );
	 operator VS_H245RequestChannelCloseRelease *( void );
	 operator VS_H245MultiplexEntrySendRelease *( void );
	 operator VS_H245RequestMultiplexEntryRelease *( void );
	 operator VS_H245RequestModeRelease *( void );
	 operator VS_H245MiscellaneousIndication *( void );
	 operator VS_H245JitterIndication *( void );
	 operator VS_H245H223SkewIndication *( void );
	 operator VS_H245NewATMVCIndication *( void );
	 operator VS_H245UserInputIndication *( void );
	 operator VS_H245H2250MaximumSkewIndication *( void );
	 operator VS_H245MCLocationIndication *( void );
	 operator VS_H245ConferenceIndication *( void );
	 operator VS_H245VendorIdentification *( void );
	 operator VS_H245FunctionNotSupported *( void );
	 operator VS_H245MultilinkIndication *( void );
	 operator VS_H245LogicalChannelRateRelease *( void );
	 operator VS_H245FlowControlIndication *( void );
	 operator VS_H245MobileMultilinkReconfigurationIndication *( void );
	 operator VS_H245GenericMessage *( void );

	void Show( void ) const;

	void operator=(VS_H245MasterSlaveDeterminationRelease *msdrel);

};

//////////////////////CLASS VS_H245CommandMessage /////////////////////////

struct VS_H245CommandMessage : public VS_AsnChoice
{
	 VS_H245CommandMessage( void );

 	enum{
	e_nonStandard,
	e_maintenanceLoopOffCommand,
	e_sendTerminalCapabilitySet,
	e_encryptionCommand,
	e_flowControlCommand,
	e_endSessionCommand,
	e_miscellaneousCommand,
	e_communicationModeCommand,
	e_conferenceCommand,
	e_h223MultiplexReconfiguration,
	e_newATMVCCommand,
	e_mobileMultilinkReconfigurationCommand,
	e_genericCommand
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245CommandMessage & src);

	 operator VS_H245NonStandardMessage *( void );
	 operator VS_H245MaintenanceLoopOffCommand *( void );
	 operator VS_H245SendTerminalCapabilitySet *( void );
	 operator VS_H245EncryptionCommand *( void );
	 operator VS_H245FlowControlCommand *( void );
	 operator VS_H245EndSessionCommand *( void );
	 operator VS_H245MiscellaneousCommand *( void );
	 operator VS_H245CommunicationModeCommand *( void );
	 operator VS_H245ConferenceCommand *( void );
	 operator VS_H245H223MultiplexReconfiguration *( void );
	 operator VS_H245NewATMVCCommand *( void );
	 operator VS_H245MobileMultilinkReconfigurationCommand *( void );
	 operator VS_H245GenericMessage *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245ResponseMessage /////////////////////////

struct VS_H245ResponseMessage : public VS_AsnChoice
{
	 VS_H245ResponseMessage( void );

 	enum{
	e_nonStandard,
	e_masterSlaveDeterminationAck,
	e_masterSlaveDeterminationReject,
	e_terminalCapabilitySetAck,
	e_terminalCapabilitySetReject,
	e_openLogicalChannelAck,
	e_openLogicalChannelReject,
	e_closeLogicalChannelAck,
	e_requestChannelCloseAck,
	e_requestChannelCloseReject,
	e_multiplexEntrySendAck,
	e_multiplexEntrySendReject,
	e_requestMultiplexEntryAck,
	e_requestMultiplexEntryReject,
	e_requestModeAck,
	e_requestModeReject,
	e_roundTripDelayResponse,
	e_maintenanceLoopAck,
	e_maintenanceLoopReject,
	e_communicationModeResponse,
	e_conferenceResponse,
	e_multilinkResponse,
	e_logicalChannelRateAcknowledge,
	e_logicalChannelRateReject,
	e_genericResponse
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245ResponseMessage & src);

	 operator VS_H245NonStandardMessage *( void );
	 operator VS_H245MasterSlaveDeterminationAck *( void );
	 operator VS_H245MasterSlaveDeterminationReject *( void );
	 operator VS_H245TerminalCapabilitySetAck *( void );
	 operator VS_H245TerminalCapabilitySetReject *( void );
	 operator VS_H245OpenLogicalChannelAck *( void );
	 operator VS_H245OpenLogicalChannelReject *( void );
	 operator VS_H245CloseLogicalChannelAck *( void );
	 operator VS_H245RequestChannelCloseAck *( void );
	 operator VS_H245RequestChannelCloseReject *( void );
	 operator VS_H245MultiplexEntrySendAck *( void );
	 operator VS_H245MultiplexEntrySendReject *( void );
	 operator VS_H245RequestMultiplexEntryAck *( void );
	 operator VS_H245RequestMultiplexEntryReject *( void );
	 operator VS_H245RequestModeAck *( void );
	 operator VS_H245RequestModeReject *( void );
	 operator VS_H245RoundTripDelayResponse *( void );
	 operator VS_H245MaintenanceLoopAck *( void );
	 operator VS_H245MaintenanceLoopReject *( void );
	 operator VS_H245CommunicationModeResponse *( void );
	 operator VS_H245ConferenceResponse *( void );
	 operator VS_H245MultilinkResponse *( void );
	 operator VS_H245LogicalChannelRateAcknowledge *( void );
	 operator VS_H245LogicalChannelRateReject *( void );
	 operator VS_H245GenericMessage *( void );

 	void operator=( VS_H245TerminalCapabilitySetAck *tcsa );
	void operator=( VS_H245TerminalCapabilitySetReject *tcsj );
	void operator=( VS_H245MasterSlaveDeterminationAck *msda );
	void operator=( VS_H245MasterSlaveDeterminationReject *msdr);
	void operator=( VS_H245OpenLogicalChannelAck *olca );
	void operator=( VS_H245OpenLogicalChannelReject *olca );

	void Show( void ) const;

};

//////////////////////CLASS VS_H245RequestMessage /////////////////////////

struct VS_H245RequestMessage : public VS_AsnChoice
{
	 VS_H245RequestMessage( void );

 	enum{
	e_nonStandard,
	e_masterSlaveDetermination,
	e_terminalCapabilitySet,
	e_openLogicalChannel,
	e_closeLogicalChannel,
	e_requestChannelClose,
	e_multiplexEntrySend,
	e_requestMultiplexEntry,
	e_requestMode,
	e_roundTripDelayRequest,
	e_maintenanceLoopRequest,
	e_communicationModeRequest,
	e_conferenceRequest,
	e_multilinkRequest,
	e_logicalChannelRateRequest,
	e_genericRequest
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245RequestMessage & src);

	void operator=( VS_H245OpenLogicalChannel *olc );
	void operator=( VS_H245MasterSlaveDetermination *msd );
	void operator=( VS_H245TerminalCapabilitySet *tcs );
	void operator=( VS_H245RoundTripDelayRequest *tcs );

	 operator VS_H245NonStandardMessage *( void );
	 operator VS_H245MasterSlaveDetermination *( void );
	 operator VS_H245TerminalCapabilitySet *( void );
	 operator VS_H245OpenLogicalChannel *( void );
	 operator VS_H245CloseLogicalChannel *( void );
	 operator VS_H245RequestChannelClose *( void );
	 operator VS_H245MultiplexEntrySend *( void );
	 operator VS_H245RequestMultiplexEntry *( void );
	 operator VS_H245RequestMode *( void );
	 operator VS_H245RoundTripDelayRequest *( void );
	 operator VS_H245MaintenanceLoopRequest *( void );
	 operator VS_H245CommunicationModeRequest *( void );
	 operator VS_H245ConferenceRequest *( void );
	 operator VS_H245MultilinkRequest *( void );
	 operator VS_H245LogicalChannelRateRequest *( void );
	 operator VS_H245GenericMessage *( void );



	void Show( void ) const;

};

//////////////////////CLASS VS_H245MultimediaSystemControlMessage /////////////////////////

struct VS_H245MultimediaSystemControlMessage : public VS_AsnChoice
{
	 VS_H245MultimediaSystemControlMessage( void );

 	enum{
	e_request,
	e_response,
	e_command,
	e_indication
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H245MultimediaSystemControlMessage & src);

	 operator VS_H245RequestMessage *( void );
	 operator VS_H245ResponseMessage *( void );
	 operator VS_H245CommandMessage *( void );
	 operator VS_H245IndicationMessage *( void );

 	void operator=( VS_H245RequestMessage *rm );
	void operator=( VS_H245ResponseMessage *rm );
	void operator=( VS_H245CommandMessage *cm );
	void operator=( VS_H245IndicationMessage *im );

	void Show( void ) const;

};

