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
/// \file VS_H225Messages.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////



#pragma once

#include "VS_Containers.h"
#include "VS_AsnBuffers.h"
#include "VS_CommonMessages.h"
#include "../VS_H323String.h"


struct VS_H225ClearToken : public VS_AsnSequence
{
	void operator=(const VS_H225ClearToken & src)
	{	O_CC(filled);	}
	// end ::operator=
} ;

//////////////////////CLASS H225ScreeningIndicator /////////////////////////

//typedef  VS_AsnEnumeration  VS_H225ScreeningIndicator;
struct VS_H225ScreeningIndicator :
						public VS_AsnEnumeration
{
	enum{
	 e_userProvidedNotScreened,
	 e_userProvidedVerifiedAndPassed,
	 e_userProvidedVerifiedAndFaile,
	 e_networkProvided};// ...
	VS_H225ScreeningIndicator( void )
		: VS_AsnEnumeration(3,0,true)
		{}
	// end VS_H248SignalType::VS_H248SignalType
};
//////////////////////CLASS VS_H225GloballyUniqueID /////////////////////////

typedef TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  VS_H225GloballyUniqueID;
//////////////////////CLASS VS_H225TimeToLive /////////////////////////

typedef TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  VS_H225TimeToLive;
//////////////////////CLASS VS_H225EndpointIdentifier /////////////////////////

typedef TemplBmpString<1,128,VS_Asn::FixedConstraint,0>  VS_H225EndpointIdentifier;
//////////////////////CLASS VS_H225CallReferenceValue /////////////////////////

typedef TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  VS_H225CallReferenceValue;
//////////////////////CLASS VS_H225BandWidth /////////////////////////

typedef TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  VS_H225BandWidth;
//////////////////////CLASS VS_H225GatekeeperIdentifier /////////////////////////

typedef TemplBmpString<1,128,VS_Asn::FixedConstraint,0>  VS_H225GatekeeperIdentifier;
//////////////////////CLASS VS_H225RequestSeqNum /////////////////////////

typedef TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  VS_H225RequestSeqNum;
//////////////////////CLASS VS_H225ConferenceIdentifier /////////////////////////



typedef TemplOctetString<2,2>  VS_H248Name;
typedef TemplOctetString<4,4>  VS_H248PkgdName;

 /////////////////////////////////////////////////////////////////////////////////////
struct VS_H248PackagesItem : public VS_AsnSequence
{
	VS_H248PackagesItem( void )
	: VS_AsnSequence( 0 ,ref,basic_root,0,0,true)
	{	ref[0].Set(&ackageName	   ,0);
		ref[1].Set(&packageVersion ,0);
	}
	// end VS_H248PackagesItem::VS_H248PackagesItem
	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];

	VS_H248Name			ackageName;//     Name,
	TemplInteger<0,99> packageVersion;//  INTEGER(0..99),
	//...
	void operator=(const VS_H248PackagesItem &src)
	{
		O_CC(filled);
		O_C(ackageName);
		O_C(packageVersion);
		O_CSA(ref, basic_root);
	}
	// end VS_H248PackagesItem::operato=
};
// end VS_H248PackagesItem struct
/////////////////////////////////////////////////////////////////////////////////////

typedef Array_of_type<VS_H248PackagesItem> VS_H248PackagesDescriptor;

//////////////////////CLASS VS_H225H248PackagesDescriptor /////////////////////////

//typedef TemplInteger< 0,INT_MAX,VS_Asn::Unconstrained,false>  VS_H225H248PackagesDescriptor;
typedef  VS_H248PackagesDescriptor  VS_H225H248PackagesDescriptor;

//////////////////////CLASS VS_H225ProtocolIdentifier /////////////////////////

typedef  VS_AsnObjectId  VS_H225ProtocolIdentifier;

typedef  VS_H225GloballyUniqueID  VS_H225ConferenceIdentifier;
//typedef TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  VS_H225ConferenceIdentifier;



typedef TemplInteger<1,4294967295>  VS_H225TimeStamp;//			::= INTEGER(1..4294967295)

struct VS_H225AuthenticationMechanism : public VS_AsnSequence
{
	void operator=(const VS_H225AuthenticationMechanism & src)
	{	O_CC(filled);	}
	// end ::operator=
} ;


//////////////////////CLASS VS_H225H221NonStandard /////////////////////////

struct VS_H225H221NonStandard : public VS_AsnSequence
{
	 VS_H225H221NonStandard( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  t35CountryCode ;
 	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  t35Extension ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  manufacturerCode ;
 	void operator=(const VS_H225H221NonStandard& src);

};
//////////////////////CLASS VS_H225NonStandardIdentifier /////////////////////////

struct VS_H225NonStandardIdentifier : public VS_AsnChoice
{
	 VS_H225NonStandardIdentifier( void );

 	enum{
	e_object,
	e_h221NonStandard
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225NonStandardIdentifier & src);

	 operator VS_H225H221NonStandard *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225PresentationIndicator /////////////////////////

struct VS_H225PresentationIndicator : public VS_AsnChoice
{
	 VS_H225PresentationIndicator( void );

 	enum{
	e_presentationAllowed,
	e_presentationRestricted,
	e_addressNotAvailable
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225PresentationIndicator & src);


	void Show( void ) const;

};
//////////////////////CLASS VS_H225UseSpecifiedTransport /////////////////////////

struct VS_H225UseSpecifiedTransport : public VS_AsnChoice
{
	 VS_H225UseSpecifiedTransport( void );

 	enum{
	e_tcp,
	e_annexE
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225UseSpecifiedTransport & src);


	void Show( void ) const;

};
//////////////////////CLASS VS_H225VendorIdentifier /////////////////////////

struct VS_H225VendorIdentifier : public VS_AsnSequence
{
	 VS_H225VendorIdentifier( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 //VS_H225H221NonStandard  vendor ;
	VS_H225H221NonStandard	h221NonStandard;
 	TemplOctetString<1,256,VS_Asn::FixedConstraint,0>  productId ;
 	TemplOctetString<1,256,VS_Asn::FixedConstraint,0>  versionId ;
 	void operator=(const VS_H225VendorIdentifier& src);

};
//////////////////////CLASS VS_H225CallType /////////////////////////

struct VS_H225CallType : public VS_AsnChoice
{
	 VS_H225CallType( void );

 	enum{
	e_pointToPoint,
	e_oneToN,
	e_nToOne,
	e_nToN
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225CallType & src);


	void Show( void ) const;

};
//////////////////////CLASS VS_H225ServiceControlSession_Reason /////////////////////////

struct VS_H225ServiceControlSession_Reason : public VS_AsnChoice
{
	VS_H225ServiceControlSession_Reason( void );

 	enum{
	e_open,
	e_refresh,
	e_close
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225ServiceControlSession_Reason & src);


	void Show( void ) const;
};
//////////////////////CLASS VS_H225Icv /////////////////////////

struct VS_H225Icv : public VS_AsnSequence
{
	 VS_H225Icv( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnObjectId  algorithmOID ;
 	TemplBitString< 0,INT_MAX,VS_Asn::Unconstrained,false>  icv ;
 	void operator=(const VS_H225Icv& src);

};
//////////////////////CLASS VS_H225GenericIdentifier /////////////////////////

struct VS_H225GenericIdentifier : public VS_AsnChoice
{
	 VS_H225GenericIdentifier( void );

 	enum{
	e_standard,
	e_oid,
	e_nonStandard
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225GenericIdentifier & src);

	 operator VS_H225GloballyUniqueID *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225AliasAddress /////////////////////////
struct VS_H225TransportAddress;
struct VS_H225PartyNumber;
struct VS_H225MobileUIM;

struct VS_H225AliasAddress : public VS_AsnChoice
{
	 VS_H225AliasAddress( void );

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char  dialedDigits_alphabet[] = { '0','1','2','3','4','5','6','7','8','9','#','*',',' };
	 static unsigned char   dialedDigits_inverse_table[256];
	 static const bool      dialedDigits_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

 	enum Type{
	e_dialedDigits,
	e_h323_ID,
	e_url_ID,
	e_transportID,
	e_email_ID,
	e_partyNumber,
	e_mobileUIM
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225AliasAddress & src);

	 operator VS_H225TransportAddress *( void );
	 operator VS_H225PartyNumber *( void );
	 operator VS_H225MobileUIM *( void );

	void Show( void ) const;

	std::string String(void) const;
};

//////////////////////CLASS VS_H225CryptoH323Token /////////////////////////
struct VS_H225CryptoH323Token : public VS_AsnChoice
{
	VS_H225CryptoH323Token();

	enum
	{
		e_cryptoEPPwdHash
	};

	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225CryptoH323Token & src);
};



struct VS_H225GenericIdentifier;
struct VS_H225AliasAddress;
struct VS_H225TransportAddress;
struct VS_H225EnumeratedParameter;
struct VS_H225GenericData;
//////////////////////CLASS VS_H225Content /////////////////////////

struct VS_H225Content : public VS_AsnChoice
{
	 VS_H225Content( void );

 	enum{
	e_raw,
	e_text,
	e_unicode,
	e_bool,
	e_number8,
	e_number16,
	e_number32,
	e_id,
	e_alias,
	e_transport,
	e_compound,
	e_nested
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225Content & src);

	 operator VS_H225GenericIdentifier *( void );
	 operator VS_H225AliasAddress *( void );
	 operator VS_H225TransportAddress *( void );
	 operator VS_H225EnumeratedParameter *( void );
	 operator VS_H225GenericData *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225EnumeratedParameter /////////////////////////

struct VS_H225EnumeratedParameter : public VS_AsnSequence
{
	 VS_H225EnumeratedParameter( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225GenericIdentifier  id ;
 	 VS_H225Content  content ;
 	void operator=(const VS_H225EnumeratedParameter& src);

};
//////////////////////CLASS VS_H225GenericData /////////////////////////

struct VS_H225GenericData : public VS_AsnSequence
{
	 VS_H225GenericData( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225GenericIdentifier  id ;
 	Constrained_array_of_type<  VS_H225EnumeratedParameter ,1,512,VS_Asn::FixedConstraint,0  >  parameters ;
 	void operator=(const VS_H225GenericData& src);

};
//////////////////////CLASS VS_H225FeatureDescriptor /////////////////////////
//typedef TemplInteger< 0,INT_MAX,VS_Asn::Unconstrained,false>  VS_H225FeatureDescriptor;
typedef  VS_H225GenericData  VS_H225FeatureDescriptor;

//////////////////////CLASS VS_H225ServiceControlResponse_Result /////////////////////////

struct VS_H225ServiceControlResponse_Result : public VS_AsnChoice
{
	 VS_H225ServiceControlResponse_Result( void );

 	enum{
	e_started,
	e_failed,
	e_stopped,
	e_notAvailable,
	e_neededFeatureNotSupported
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225ServiceControlResponse_Result & src);


	void Show( void ) const;

};
//////////////////////CLASS VS_H225NonStandardParameter /////////////////////////

struct VS_H225NonStandardParameter : public VS_AsnSequence
{
	 VS_H225NonStandardParameter( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225NonStandardIdentifier  nonStandardIdentifier ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  data ;
 	void operator=(const VS_H225NonStandardParameter& src);

};

//////////////////////CLASS VS_H225TerminalInfo /////////////////////////

struct VS_H225TerminalInfo : public VS_AsnSequence
{
	 VS_H225TerminalInfo( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225NonStandardParameter  nonStandardData ;
 	void operator=(const VS_H225TerminalInfo& src);

};
//////////////////////CLASS VS_H225FeatureSet /////////////////////////

struct VS_H225FeatureSet : public VS_AsnSequence
{
	 VS_H225FeatureSet( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  replacementFeatureSet ;
 	Constrained_array_of_type<  VS_H225FeatureDescriptor ,0,INT_MAX,VS_Asn::Unconstrained,0  >  neededFeatures ;
 	Constrained_array_of_type<  VS_H225FeatureDescriptor ,0,INT_MAX,VS_Asn::Unconstrained,0  >  desiredFeatures ;
 	Constrained_array_of_type<  VS_H225FeatureDescriptor ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedFeatures ;
 	void operator=(const VS_H225FeatureSet& src);

};

//////////////////////CLASS VS_H225CallIdentifier /////////////////////////

struct VS_H225CallIdentifier : public VS_AsnSequence
{
	 VS_H225CallIdentifier( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225GloballyUniqueID  guid ;
 	void operator=(const VS_H225CallIdentifier& src);

};
//////////////////////CLASS VS_H225ServiceControlIndication_CallSpecific /////////////////////////

struct VS_H225ServiceControlIndication_CallSpecific : public VS_AsnSequence
{
	 VS_H225ServiceControlIndication_CallSpecific( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225CallIdentifier  callIdentifier ;
 	 VS_H225ConferenceIdentifier  conferenceID ;
 	 VS_AsnBoolean  answeredCall ;
 	void operator=(const VS_H225ServiceControlIndication_CallSpecific& src);

};
//////////////////////CLASS VS_H225CicInfo /////////////////////////

struct VS_H225CicInfo : public VS_AsnSequence
{
	 VS_H225CicInfo( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type< TemplOctetString<2,4,VS_Asn::FixedConstraint,0> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cic ;
 	TemplOctetString<2,5,VS_Asn::FixedConstraint,0>  pointCode ;
 	void operator=(const VS_H225CicInfo& src);

};
//////////////////////CLASS VS_H225GroupID /////////////////////////

struct VS_H225GroupID : public VS_AsnSequence
{
	 VS_H225GroupID( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type< TemplInteger<0,65535,VS_Asn::FixedConstraint,0> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  member ;
 	TemplIA5String<1,128,VS_Asn::FixedConstraint,0>  group ;
 	void operator=(const VS_H225GroupID& src);

};


//////////////////////CLASS VS_H225CircuitIdentifier /////////////////////////

struct VS_H225CircuitIdentifier : public VS_AsnChoice
{
	 VS_H225CircuitIdentifier( void );

 	enum{
	e_cic,
	e_group
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225CircuitIdentifier & src);

	 operator VS_H225CicInfo *( void );
	 operator VS_H225GroupID *( void );

	void Show( void ) const;

};
/////////////////////////////////////////////////////////////////////////////////////
struct VS_H248SignalRequest : public VS_AsnChoice
{
	  enum {e_signal,//      Signal,
			e_seqSigList};//  SeqSigList,
                //      ...
	VS_H248SignalRequest( void )
	:VS_AsnChoice(2,2,true)
	{}
	// end VS_H248SignalRequest::VS_H248SignalRequest
	/////////////////////////////////////////////////////////////////////////////////////
	bool Decode(VS_PerBuffer& buffer) override
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
			case e_signal:		return DecodeChoice( buffer , new VS_H225CicInfo);
			case e_seqSigList:	return DecodeChoice( buffer , new VS_H225GroupID);
			default:			return buffer.ChoiceMissExtensionObject(*this);
		}
	}
	// end VS_H225CircuitIdentifier::Decode

	/////////////////////////////////////////////////////////////////////////////////////


	void operator=(const VS_H225CircuitIdentifier &src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
			case e_signal:		return CopyChoice< VS_H225CicInfo >( src ,*this);
			case e_seqSigList:	return CopyChoice< VS_H225GroupID >( src ,*this);
			default: return ;
		}
		return;
	}
	// end VS_H225CircuitIdentifier::operator=
	/////////////////////////////////////////////////////////////////////////////////////
};
struct VS_H225CallCreditServiceControl;
//////////////////////CLASS VS_H225H248SignalsDescriptor /////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

typedef Array_of_type<VS_H248SignalRequest> VS_H248SignalsDescriptor;

//typedef TemplInteger< 0,INT_MAX,VS_Asn::Unconstrained,false>  VS_H225H248SignalsDescriptor;
typedef  VS_H248SignalsDescriptor  VS_H225H248SignalsDescriptor;
//////////////////////CLASS VS_H225ServiceControlDescriptor /////////////////////////
//struct VS_H225H248SignalsDescriptor;


struct VS_H225ServiceControlDescriptor : public VS_AsnChoice
{
	 VS_H225ServiceControlDescriptor( void );

 	enum{
	e_url,
	e_signal,
	e_nonStandard,
	e_callCreditServiceControl
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225ServiceControlDescriptor & src);

	 operator VS_H225H248SignalsDescriptor *( void );
	 operator VS_H225NonStandardParameter *( void );
	 operator VS_H225CallCreditServiceControl *( void );

	void Show( void ) const;

};
//////////////////////CLASS VS_H225ServiceControlSession /////////////////////////

struct VS_H225ServiceControlSession : public VS_AsnSequence
{
	 VS_H225ServiceControlSession( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  sessionId ;
 	 VS_H225ServiceControlDescriptor  contents ;
 	VS_H225ServiceControlSession_Reason	 reason ;
 	void operator=(const VS_H225ServiceControlSession& src);

};
//////////////////////CLASS VS_H225ServiceControlIndication /////////////////////////

struct VS_H225ServiceControlIndication : public VS_AsnSequence
{
	 VS_H225ServiceControlIndication( void );

	static const unsigned basic_root = 10;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	VS_H225ServiceControlIndication_CallSpecific	 callSpecific ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225ServiceControlIndication& src);

};
//////////////////////CLASS VS_H225ResourcesAvailableConfirm /////////////////////////

struct VS_H225ResourcesAvailableConfirm : public VS_AsnSequence
{
	 VS_H225ResourcesAvailableConfirm( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225ResourcesAvailableConfirm& src);

};

struct VS_H225NonStandardParameter;
struct VS_H225H310Caps;
struct VS_H225H320Caps;
struct VS_H225H321Caps;
struct VS_H225H322Caps;
struct VS_H225H323Caps;
struct VS_H225H324Caps;
struct VS_H225VoiceCaps;
struct VS_H225T120OnlyCaps;
struct VS_H225NonStandardProtocol;
struct VS_H225T38FaxAnnexbOnlyCaps;
//////////////////////CLASS VS_H225SupportedProtocols /////////////////////////

struct VS_H225SupportedProtocols : public VS_AsnChoice
{
	 VS_H225SupportedProtocols( void );

 	enum{
	e_nonStandardData,
	e_h310,
	e_h320,
	e_h321,
	e_h322,
	e_h323,
	e_h324,
	e_voice,
	e_t120_only,
	e_nonStandardProtocol,
	e_t38FaxAnnexbOnly
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225SupportedProtocols & src);

	 operator VS_H225NonStandardParameter *( void );
	 operator VS_H225H310Caps *( void );
	 operator VS_H225H320Caps *( void );
	 operator VS_H225H321Caps *( void );
	 operator VS_H225H322Caps *( void );
	 operator VS_H225H323Caps *( void );
	 operator VS_H225H324Caps *( void );
	 operator VS_H225VoiceCaps *( void );
	 operator VS_H225T120OnlyCaps *( void );
	 operator VS_H225NonStandardProtocol *( void );
	 operator VS_H225T38FaxAnnexbOnlyCaps *( void );

	void Show( void ) const;

};
//////////////////////CLASS VS_H225CallsAvailable /////////////////////////

struct VS_H225CallsAvailable : public VS_AsnSequence
{
	 VS_H225CallsAvailable( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  calls ;
 	TemplIA5String<1,128,VS_Asn::FixedConstraint,0>  group ;
 	void operator=(const VS_H225CallsAvailable& src);

};

//////////////////////CLASS VS_H225CallCapacityInfo /////////////////////////

struct VS_H225CallCapacityInfo : public VS_AsnSequence
{
	 VS_H225CallCapacityInfo( void );

	static const unsigned basic_root = 11;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  voiceGwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h310GwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h320GwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h321GwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h322GwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h323GwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h324GwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  t120OnlyGwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  t38FaxAnnexbOnlyGwCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  terminalCallsAvailable ;
 	Constrained_array_of_type<  VS_H225CallsAvailable ,0,INT_MAX,VS_Asn::Unconstrained,0  >  mcuCallsAvailable ;
 	void operator=(const VS_H225CallCapacityInfo& src);

};
//////////////////////CLASS VS_H225CallCapacity /////////////////////////

struct VS_H225CallCapacity : public VS_AsnSequence
{
	 VS_H225CallCapacity( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225CallCapacityInfo  maximumCallCapacity ;
 	 VS_H225CallCapacityInfo  currentCallCapacity ;
 	void operator=(const VS_H225CallCapacity& src);

};
//////////////////////CLASS VS_H225ResourcesAvailableIndicate /////////////////////////

struct VS_H225ResourcesAvailableIndicate : public VS_AsnSequence
{
	 VS_H225ResourcesAvailableIndicate( void );

	static const unsigned basic_root = 9;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	Constrained_array_of_type<  VS_H225SupportedProtocols ,0,INT_MAX,VS_Asn::Unconstrained,0  >  protocols ;
 	 VS_AsnBoolean  almostOutOfResources ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225CallCapacity  capacity ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225ResourcesAvailableIndicate& src);

};
//////////////////////CLASS VS_H225RequestInProgress /////////////////////////

struct VS_H225RequestInProgress : public VS_AsnSequence
{
	 VS_H225RequestInProgress( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  delay ;
 	void operator=(const VS_H225RequestInProgress& src);

};
//////////////////////CLASS VS_H225UnknownMessageResponse /////////////////////////

struct VS_H225UnknownMessageResponse : public VS_AsnSequence
{
	 VS_H225UnknownMessageResponse( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  messageNotUnderstood ;
 	void operator=(const VS_H225UnknownMessageResponse& src);

};
//////////////////////CLASS VS_H225NonStandardMessage /////////////////////////

struct VS_H225NonStandardMessage : public VS_AsnSequence
{
	 VS_H225NonStandardMessage( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225NonStandardMessage& src);

};
//////////////////////CLASS VS_H225InfoRequestNakReason /////////////////////////

struct VS_H225InfoRequestNakReason : public VS_AsnChoice
{
	 VS_H225InfoRequestNakReason( void );

 	enum{
	e_notRegistered,
	e_securityDenial,
	e_undefinedReason
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225InfoRequestNakReason & src);


	void Show( void ) const;

};
struct VS_H225TransportAddress_IpAddress;
struct VS_H225TransportAddress_Ip6Address;
struct VS_H225TransportAddress_IpxAddress;
struct VS_H225TransportAddress_IpSourceRoute;
//////////////////////CLASS VS_H225TransportAddress /////////////////////////

struct VS_H225TransportAddress : public VS_AsnChoice
{
	 VS_H225TransportAddress( void );

 	enum{
	e_ipAddress,
	e_ipSourceRoute,
	e_ipxAddress,
	e_ip6Address,
	e_netBios,
	e_nsap,
	e_nonStandardAddress
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225TransportAddress & src);

	void operator=( VS_H225TransportAddress_IpAddress *src );

	// end VS_H225TransportAddress::operator= ( VS_H225IpAddress * )
	bool operator==(const VS_H225TransportAddress &src );
	/*{
		if((filled==src.filled) && (tag==src.tag))
			switch (src.tag)
			{
			case e_ipAddress:			return (*((VS_H225TransportAddress_IpAddress*)choice)==*((VS_H225TransportAddress_IpAddress*)src.choice));
			case e_ipSourceRoute:		return false;
			case e_ipxAddress:			return false;
			case e_ip6Address:			return false;
			case e_netBios:				return false;
			case e_nsap:				return false;
			case e_nonStandardAddress:	return false;
			default : return false;
			}
		return false;
	}*/


	operator VS_H225TransportAddress_IpAddress * ( void );
	operator VS_H225TransportAddress_Ip6Address * ( void );
	operator VS_H225TransportAddress_IpxAddress * ( void );
	operator VS_H225TransportAddress_IpSourceRoute * ( void );
	operator VS_AsnOctetString * ( void );

	operator VS_H225NonStandardParameter *( void );

	void Show( void ) const;

};
//////////////////////CLASS VS_H225AlternateGK /////////////////////////

struct VS_H225AlternateGK : public VS_AsnSequence
{
	 VS_H225AlternateGK( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225TransportAddress  rasAddress ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	 VS_AsnBoolean  needToRegister ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  priority ;
 	void operator=(const VS_H225AlternateGK& src);

};
//////////////////////CLASS VS_H225AltGKInfo /////////////////////////

struct VS_H225AltGKInfo : public VS_AsnSequence
{
	 VS_H225AltGKInfo( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H225AlternateGK ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateGatekeeper ;
 	 VS_AsnBoolean  altGKisPermanent ;
 	void operator=(const VS_H225AltGKInfo& src);

};
//////////////////////CLASS VS_H225InfoRequestNak /////////////////////////

struct VS_H225InfoRequestNak : public VS_AsnSequence
{
	 VS_H225InfoRequestNak( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225InfoRequestNakReason  nakReason ;
 	 VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	void operator=(const VS_H225InfoRequestNak& src);

};
//////////////////////CLASS VS_H225InfoRequestAck /////////////////////////

struct VS_H225InfoRequestAck : public VS_AsnSequence
{
	 VS_H225InfoRequestAck( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	void operator=(const VS_H225InfoRequestAck& src);

};
//////////////////////CLASS VS_H225InfoRequestResponseStatus /////////////////////////

struct VS_H225InfoRequestResponseStatus : public VS_AsnChoice
{
	 VS_H225InfoRequestResponseStatus( void );

 	enum{
	e_complete,
	e_incomplete,
	e_segment,
	e_invalidCall
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225InfoRequestResponseStatus & src);


	void Show( void ) const;

};
//////////////////////CLASS VS_H225CallLinkage /////////////////////////

struct VS_H225CallLinkage : public VS_AsnSequence
{
	 VS_H225CallLinkage( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225GloballyUniqueID  globalCallId ;
 	 VS_H225GloballyUniqueID  threadId ;
 	void operator=(const VS_H225CallLinkage& src);

};


//////////////////////CLASS VS_H225TransportChannelInfo /////////////////////////

struct VS_H225TransportChannelInfo : public VS_AsnSequence
{
	 VS_H225TransportChannelInfo( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225TransportAddress  sendAddress ;
 	 VS_H225TransportAddress  recvAddress ;
 	void operator=(const VS_H225TransportChannelInfo& src);

};
//////////////////////CLASS VS_H225RTPSession /////////////////////////

struct VS_H225RTPSession : public VS_AsnSequence
{
	 VS_H225RTPSession( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225TransportChannelInfo  rtpAddress ;
 	 VS_H225TransportChannelInfo  rtcpAddress ;
 	TemplPrintableString<0,INT_MAX,VS_Asn::Unconstrained>  cname ;
 	TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  ssrc ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  sessionId ;
 	Constrained_array_of_type< TemplInteger<1,255,VS_Asn::FixedConstraint,0> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  associatedSessionIds ;
 	 VS_AsnNull  multicast ;
 	 VS_H225BandWidth  bandwidth ;
 	void operator=(const VS_H225RTPSession& src);

};

//////////////////////CLASS VS_H225CallModel /////////////////////////

struct VS_H225CallModel : public VS_AsnChoice
{
	 VS_H225CallModel( void );

 	enum{
	e_direct,
	e_gatekeeperRouted
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225CallModel & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225CircuitInfo /////////////////////////

struct VS_H225CircuitInfo : public VS_AsnSequence
{
	 VS_H225CircuitInfo( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225CircuitIdentifier  sourceCircuitID ;
 	 VS_H225CircuitIdentifier  destinationCircuitID ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225CircuitInfo& src);

};


struct VS_H225TransportAddress;
struct VS_H225PartyNumber;
struct VS_H225MobileUIM;

////////////////////////CLASS VS_H225AliasAddress /////////////////////////
//
//struct VS_H225AliasAddress : public VS_AsnChoice
//{
//	 VS_H225AliasAddress( void );
//
//	 /////////////////////////////////////////////////////////////////////////////////////////
//	 static unsigned char   dialedDigits_alphabet[];
//	 static unsigned		dialedDigits_alphabet_size;
//	 static unsigned char   dialedDigits_inverse_table[];
//	 static const bool      dialedDigits_flag_set_table;
//
//	 /////////////////////////////////////////////////////////////////////////////////////////
//
// 	enum{
//	e_dialedDigits,
//	e_h323_ID,
//	e_url_ID,
//	e_transportID,
//	e_email_ID,
//	e_partyNumber,
//	e_mobileUIM
//	};
//	bool Decode(VS_PerBuffer& buffer) override;
//
//	void operator=(const VS_H225AliasAddress & src);
//
//	 operator VS_H225TransportAddress *( void );
//	 operator VS_H225PartyNumber *( void );
//	 operator VS_H225MobileUIM *( void );
//
//	void Show( void ) const;
//
//};




//////////////////////CLASS VS_H225GatekeeperInfo /////////////////////////

struct VS_H225GatekeeperInfo : public VS_AsnSequence
{
	 VS_H225GatekeeperInfo( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225NonStandardParameter  nonStandardData ;
 	void operator=(const VS_H225GatekeeperInfo& src);

};

//////////////////////CLASS VS_H225GatewayInfo /////////////////////////

struct VS_H225GatewayInfo : public VS_AsnSequence
{
	 VS_H225GatewayInfo( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H225SupportedProtocols ,0,INT_MAX,VS_Asn::Unconstrained,0  >  protocol ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	void operator=(const VS_H225GatewayInfo& src);

};

//////////////////////CLASS VS_H225DisengageRejectReason /////////////////////////

struct VS_H225DisengageRejectReason : public VS_AsnChoice
{
	 VS_H225DisengageRejectReason( void );

 	enum{
	e_notRegistered,
	e_requestToDropOther,
	e_securityDenial
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225DisengageRejectReason & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225DisengageReject /////////////////////////

struct VS_H225DisengageReject : public VS_AsnSequence
{
	 VS_H225DisengageReject( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225DisengageRejectReason  rejectReason ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225DisengageReject& src);

};

//////////////////////CLASS VS_H225DisengageReason /////////////////////////

struct VS_H225DisengageReason : public VS_AsnChoice
{
	 VS_H225DisengageReason( void );

 	enum{
	e_forcedDrop,
	e_normalDrop,
	e_undefinedReason
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225DisengageReason & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225LocationRejectReason /////////////////////////

struct VS_H225LocationRejectReason : public VS_AsnChoice
{
	 VS_H225LocationRejectReason( void );

 	enum{
	e_notRegistered,
	e_invalidPermission,
	e_requestDenied,
	e_undefinedReason,
	e_securityDenial,
	e_aliasesInconsistent,
	e_routeCalltoSCN,
	e_resourceUnavailable,
	e_genericDataReason,
	e_neededFeatureNotSupported
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225LocationRejectReason & src);

	 operator VS_H225PartyNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225LocationReject /////////////////////////

struct VS_H225LocationReject : public VS_AsnSequence
{
	 VS_H225LocationReject( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 7;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225LocationRejectReason  rejectReason ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	void operator=(const VS_H225LocationReject& src);

};

//////////////////////CLASS VS_H225AlternateTransportAddresses /////////////////////////

struct VS_H225AlternateTransportAddresses : public VS_AsnSequence
{
	 VS_H225AlternateTransportAddresses( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  annexE ;
 	void operator=(const VS_H225AlternateTransportAddresses& src);

};






//////////////////////CLASS VS_H225BandRejectReason /////////////////////////

struct VS_H225BandRejectReason : public VS_AsnChoice
{
	 VS_H225BandRejectReason( void );

 	enum{
	e_notBound,
	e_invalidConferenceID,
	e_invalidPermission,
	e_insufficientResources,
	e_invalidRevision,
	e_undefinedReason,
	e_securityDenial
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225BandRejectReason & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225BandwidthReject /////////////////////////

struct VS_H225BandwidthReject : public VS_AsnSequence
{
	 VS_H225BandwidthReject( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225BandRejectReason  rejectReason ;
 	 VS_H225BandWidth  allowedBandWidth ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225BandwidthReject& src);

};
//////////////////////CLASS VS_H225BandwidthConfirm /////////////////////////

struct VS_H225BandwidthConfirm : public VS_AsnSequence
{
	 VS_H225BandwidthConfirm( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225BandWidth  bandWidth ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225CallCapacity  capacity ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225BandwidthConfirm& src);

};
//////////////////////CLASS VS_H225BandwidthDetails /////////////////////////

struct VS_H225BandwidthDetails : public VS_AsnSequence
{
	 VS_H225BandwidthDetails( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  sender ;
 	 VS_AsnBoolean  multicast ;
 	 VS_H225BandWidth  bandwidth ;
 	 VS_H225TransportChannelInfo  rtcpAddresses ;
 	void operator=(const VS_H225BandwidthDetails& src);

};

//////////////////////CLASS VS_H225AdmissionRejectReason /////////////////////////

struct VS_H225AdmissionRejectReason : public VS_AsnChoice
{
	 VS_H225AdmissionRejectReason( void );

 	enum{
	e_calledPartyNotRegistered,
	e_invalidPermission,
	e_requestDenied,
	e_undefinedReason,
	e_callerNotRegistered,
	e_routeCallToGatekeeper,
	e_invalidEndpointIdentifier,
	e_resourceUnavailable,
	e_securityDenial,
	e_qosControlNotSupported,
	e_incompleteAddress,
	e_aliasesInconsistent,
	e_routeCallToSCN,
	e_exceedsCallCapacity,
	e_collectDestination,
	e_collectPIN,
	e_genericDataReason,
	e_neededFeatureNotSupported
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225AdmissionRejectReason & src);

	 operator VS_H225PartyNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225AdmissionReject /////////////////////////

struct VS_H225AdmissionReject : public VS_AsnSequence
{
	 VS_H225AdmissionReject( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 8;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225AdmissionRejectReason  rejectReason ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  callSignalAddress ;
 	 VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225AdmissionReject& src);

};
//////////////////////CLASS VS_H225UUIEsRequested /////////////////////////

struct VS_H225UUIEsRequested : public VS_AsnSequence
{
	 VS_H225UUIEsRequested( void );

	static const unsigned basic_root = 9;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  setup ;
 	 VS_AsnBoolean  callProceeding ;
 	 VS_AsnBoolean  connect ;
 	 VS_AsnBoolean  alerting ;
 	 VS_AsnBoolean  information ;
 	 VS_AsnBoolean  releaseComplete ;
 	 VS_AsnBoolean  facility ;
 	 VS_AsnBoolean  progress ;
 	 VS_AsnBoolean  empty ;
 	 VS_AsnBoolean  status ;
 	 VS_AsnBoolean  statusInquiry ;
 	 VS_AsnBoolean  setupAcknowledge ;
 	 VS_AsnBoolean  notify ;
 	void operator=(const VS_H225UUIEsRequested& src);

};

//////////////////////CLASS VS_H225TransportQOS /////////////////////////

struct VS_H225TransportQOS : public VS_AsnChoice
{
	 VS_H225TransportQOS( void );

 	enum{
	e_endpointControlled,
	e_gatekeeperControlled,
	e_noControl
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225TransportQOS & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225UnregRejectReason /////////////////////////

struct VS_H225UnregRejectReason : public VS_AsnChoice
{
	 VS_H225UnregRejectReason( void );

 	enum{
	e_notCurrentlyRegistered,
	e_callInProgress,
	e_undefinedReason,
	e_permissionDenied,
	e_securityDenial
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225UnregRejectReason & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225UnregistrationReject /////////////////////////

struct VS_H225UnregistrationReject : public VS_AsnSequence
{
	 VS_H225UnregistrationReject( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225UnregRejectReason  rejectReason ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225UnregistrationReject& src);

};
//////////////////////CLASS VS_H225UnregistrationConfirm /////////////////////////

struct VS_H225UnregistrationConfirm : public VS_AsnSequence
{
	 VS_H225UnregistrationConfirm( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225UnregistrationConfirm& src);

};
//////////////////////CLASS VS_H225UnregRequestReason /////////////////////////

struct VS_H225UnregRequestReason : public VS_AsnChoice
{
	 VS_H225UnregRequestReason( void );

 	enum{
	e_reregistrationRequired,
	e_ttlExpired,
	e_securityDenial,
	e_undefinedReason,
	e_maintenance
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225UnregRequestReason & src);


	void Show( void ) const;

};


//////////////////////CLASS VS_H225RegistrationRejectReason /////////////////////////

struct VS_H225RegistrationRejectReason : public VS_AsnChoice
{
	 VS_H225RegistrationRejectReason( void );

 	enum{
	e_discoveryRequired,
	e_invalidRevision,
	e_invalidCallSignalAddress,
	e_invalidRASAddress,
	e_duplicateAlias,
	e_invalidTerminalType,
	e_undefinedReason,
	e_transportNotSupported,
	e_transportQOSNotSupported,
	e_resourceUnavailable,
	e_invalidAlias,
	e_securityDenial,
	e_fullRegistrationRequired,
	e_additiveRegistrationNotSupported,
	e_invalidTerminalAliases,
	e_genericDataReason,
	e_neededFeatureNotSupported
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225RegistrationRejectReason & src);

	 operator VS_H225AliasAddress *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225RegistrationReject /////////////////////////

struct VS_H225RegistrationReject : public VS_AsnSequence
{
	 VS_H225RegistrationReject( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 6;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225RegistrationRejectReason  rejectReason ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	 VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225RegistrationReject& src);

};
//////////////////////CLASS VS_H225RegistrationConfirm_PreGrantedARQ /////////////////////////

struct VS_H225RegistrationConfirm_PreGrantedARQ : public VS_AsnSequence
{
	 VS_H225RegistrationConfirm_PreGrantedARQ( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_AsnBoolean  makeCall ;
 	 VS_AsnBoolean  useGKCallSignalAddressToMakeCall ;
 	 VS_AsnBoolean  answerCall ;
 	 VS_AsnBoolean  useGKCallSignalAddressToAnswer ;
 	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  irrFrequencyInCall ;
 	 VS_H225BandWidth  totalBandwidthRestriction ;
 	 VS_H225AlternateTransportAddresses  alternateTransportAddresses ;
 	 VS_H225UseSpecifiedTransport  useSpecifiedTransport ;
 	void operator=(const VS_H225RegistrationConfirm_PreGrantedARQ& src);

};


//////////////////////CLASS VS_H225GatekeeperRejectReason /////////////////////////

struct VS_H225GatekeeperRejectReason : public VS_AsnChoice
{
	 VS_H225GatekeeperRejectReason( void );

 	enum{
	e_resourceUnavailable,
	e_terminalExcluded,
	e_invalidRevision,
	e_undefinedReason,
	e_securityDenial,
	e_genericDataReason,
	e_neededFeatureNotSupported
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225GatekeeperRejectReason & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225GatekeeperReject /////////////////////////

struct VS_H225GatekeeperReject : public VS_AsnSequence
{
	 VS_H225GatekeeperReject( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 6;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	 VS_H225GatekeeperRejectReason  rejectReason ;
 	 VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225GatekeeperReject& src);

};






//////////////////////CLASS VS_H225CallCreditServiceControl_BillingMode /////////////////////////

struct VS_H225CallCreditServiceControl_BillingMode : public VS_AsnChoice
{
	 VS_H225CallCreditServiceControl_BillingMode( void );

 	enum{
	e_credit,
	e_debit
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225CallCreditServiceControl_BillingMode & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225CallCreditServiceControl_CallStartingPoint /////////////////////////

struct VS_H225CallCreditServiceControl_CallStartingPoint : public VS_AsnChoice
{
	 VS_H225CallCreditServiceControl_CallStartingPoint( void );

 	enum{
	e_alerting,
	e_connect
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225CallCreditServiceControl_CallStartingPoint & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225CallCreditServiceControl /////////////////////////

struct VS_H225CallCreditServiceControl : public VS_AsnSequence
{
	 VS_H225CallCreditServiceControl( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplBmpString<1,512,VS_Asn::FixedConstraint,0>  amountString ;
 	VS_H225CallCreditServiceControl_BillingMode	 billingMode ;
 	TemplInteger<1,4294967295,VS_Asn::FixedConstraint,0>  callDurationLimit ;
 	 VS_AsnBoolean  enforceCallDurationLimit ;
 	VS_H225CallCreditServiceControl_CallStartingPoint	 callStartingPoint ;
 	void operator=(const VS_H225CallCreditServiceControl& src);

};
//////////////////////CLASS VS_H225CallCreditCapability /////////////////////////

struct VS_H225CallCreditCapability : public VS_AsnSequence
{
	 VS_H225CallCreditCapability( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  canDisplayAmountString ;
 	 VS_AsnBoolean  canEnforceDurationLimit ;
 	void operator=(const VS_H225CallCreditCapability& src);

};



//////////////////////CLASS VS_H225RasUsageInformation /////////////////////////

struct VS_H225RasUsageInformation : public VS_AsnSequence
{
	 VS_H225RasUsageInformation( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H225NonStandardParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  nonStandardUsageFields ;
 	 VS_H225TimeStamp  alertingTime ;
 	 VS_H225TimeStamp  connectTime ;
 	 VS_H225TimeStamp  endTime ;
 	void operator=(const VS_H225RasUsageInformation& src);

};
//////////////////////CLASS VS_H225BandwidthRequest /////////////////////////

struct VS_H225BandwidthRequest : public VS_AsnSequence
{
	 VS_H225BandwidthRequest( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 11;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	 VS_H225ConferenceIdentifier  conferenceID ;
 	 VS_H225CallReferenceValue  callReferenceValue ;
 	 VS_H225CallType  callType ;
 	 VS_H225BandWidth  bandWidth ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_AsnBoolean  answeredCall ;
 	 VS_H225CallLinkage  callLinkage ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225RasUsageInformation  usageInformation ;
 	Constrained_array_of_type<  VS_H225BandwidthDetails ,0,INT_MAX,VS_Asn::Unconstrained,0  >  bandwidthDetails ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225BandwidthRequest& src);

};
//////////////////////CLASS VS_H225DisengageConfirm /////////////////////////

struct VS_H225DisengageConfirm : public VS_AsnSequence
{
	 VS_H225DisengageConfirm( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 7;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	 VS_H225RasUsageInformation  usageInformation ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225DisengageConfirm& src);

};
//////////////////////CLASS VS_H225RasUsageSpecification_When /////////////////////////

struct VS_H225RasUsageSpecification_When : public VS_AsnSequence
{
	 VS_H225RasUsageSpecification_When( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnNull  start ;
 	 VS_AsnNull  end ;
 	 VS_AsnNull  inIrr ;
 	void operator=(const VS_H225RasUsageSpecification_When& src);

};
//////////////////////CLASS VS_H225RasUsageSpecification_CallStartingPoint /////////////////////////

struct VS_H225RasUsageSpecification_CallStartingPoint : public VS_AsnSequence
{
	 VS_H225RasUsageSpecification_CallStartingPoint( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnNull  alerting ;
 	 VS_AsnNull  connect ;
 	void operator=(const VS_H225RasUsageSpecification_CallStartingPoint& src);

};


//////////////////////CLASS VS_H225RasUsageInfoTypes /////////////////////////

struct VS_H225RasUsageInfoTypes : public VS_AsnSequence
{
	 VS_H225RasUsageInfoTypes( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H225NonStandardParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  nonStandardUsageTypes ;
 	 VS_AsnNull  startTime ;
 	 VS_AsnNull  endTime ;
 	 VS_AsnNull  terminationCause ;
 	void operator=(const VS_H225RasUsageInfoTypes& src);

};
//////////////////////CLASS VS_H225RasUsageSpecification /////////////////////////

struct VS_H225RasUsageSpecification : public VS_AsnSequence
{
	 VS_H225RasUsageSpecification( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H225RasUsageSpecification_When	 when ;
 	VS_H225RasUsageSpecification_CallStartingPoint	 callStartingPoint ;
 	 VS_H225RasUsageInfoTypes  required ;
 	void operator=(const VS_H225RasUsageSpecification& src);

};
//////////////////////CLASS VS_H225InfoRequest /////////////////////////

struct VS_H225InfoRequest : public VS_AsnSequence
{
	 VS_H225InfoRequest( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 11;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225CallReferenceValue  callReferenceValue ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225TransportAddress  replyAddress ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225UUIEsRequested  uuiesRequested ;
 	 VS_H225CallLinkage  callLinkage ;
 	 VS_H225RasUsageInfoTypes  usageInfoRequested ;
 	 VS_AsnNull  segmentedResponseSupported ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  nextSegmentRequested ;
 	 VS_AsnNull  capacityInfoRequested ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225InfoRequest& src);

};




//////////////////////CLASS VS_H225CapacityReportingSpecification_When /////////////////////////

struct VS_H225CapacityReportingSpecification_When : public VS_AsnSequence
{
	 VS_H225CapacityReportingSpecification_When( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnNull  callStart ;
 	 VS_AsnNull  callEnd ;
 	void operator=(const VS_H225CapacityReportingSpecification_When& src);

};
//////////////////////CLASS VS_H225CapacityReportingSpecification /////////////////////////

struct VS_H225CapacityReportingSpecification : public VS_AsnSequence
{
	 VS_H225CapacityReportingSpecification( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H225CapacityReportingSpecification_When	 when ;
 	void operator=(const VS_H225CapacityReportingSpecification& src);

};
//////////////////////CLASS VS_H225CapacityReportingCapability /////////////////////////

struct VS_H225CapacityReportingCapability : public VS_AsnSequence
{
	 VS_H225CapacityReportingCapability( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  canReportCallCapacity ;
 	void operator=(const VS_H225CapacityReportingCapability& src);

};
//////////////////////CLASS VS_H225SupportedPrefix /////////////////////////

struct VS_H225SupportedPrefix : public VS_AsnSequence
{
	 VS_H225SupportedPrefix( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225AliasAddress  prefix ;
 	void operator=(const VS_H225SupportedPrefix& src);

};

//////////////////////CLASS VS_H225DataRate /////////////////////////

struct VS_H225DataRate : public VS_AsnSequence
{
	 VS_H225DataRate( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225BandWidth  channelRate ;
 	TemplInteger<1,256,VS_Asn::FixedConstraint,0>  channelMultiplier ;
 	void operator=(const VS_H225DataRate& src);

};

//////////////////////CLASS VS_H225ServiceControlResponse /////////////////////////

struct VS_H225ServiceControlResponse : public VS_AsnSequence
{
	 VS_H225ServiceControlResponse( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225RequestSeqNum  requestSeqNum ;
 	VS_H225ServiceControlResponse_Result	 result ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225ServiceControlResponse& src);

};
struct VS_H225NonIsoIntegrityMechanism;
//////////////////////CLASS VS_H225IntegrityMechanism /////////////////////////

struct VS_H225IntegrityMechanism : public VS_AsnChoice
{
	 VS_H225IntegrityMechanism( void );

 	enum{
	e_nonStandard,
	e_digSig,
	e_iso9797,
	e_nonIsoIM
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225IntegrityMechanism & src);

	 operator VS_H225NonStandardParameter *( void );
	 operator VS_H225NonIsoIntegrityMechanism *( void );

	void Show( void ) const;

};


//////////////////////CLASS VS_H225GatekeeperConfirm /////////////////////////

struct VS_H225GatekeeperConfirm : public VS_AsnSequence
{
	 VS_H225GatekeeperConfirm( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 9;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	 VS_H225TransportAddress  rasAddress ;
 	Constrained_array_of_type<  VS_H225AlternateGK ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateGatekeeper ;
 	 VS_H225AuthenticationMechanism  authenticationMode ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_AsnObjectId  algorithmOID ;
 	Constrained_array_of_type<  VS_H225IntegrityMechanism ,0,INT_MAX,VS_Asn::Unconstrained,0  >  integrity ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225GatekeeperConfirm& src);

};
struct VS_H225EncryptIntAlg;
//////////////////////CLASS VS_H225NonIsoIntegrityMechanism /////////////////////////

struct VS_H225NonIsoIntegrityMechanism : public VS_AsnChoice
{
	 VS_H225NonIsoIntegrityMechanism( void );

 	enum{
	e_hMAC_MD5,
	e_hMAC_iso10118_2_s,
	e_hMAC_iso10118_2_l,
	e_hMAC_iso10118_3
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225NonIsoIntegrityMechanism & src);

	 operator VS_H225EncryptIntAlg *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225EncryptIntAlg /////////////////////////

struct VS_H225EncryptIntAlg : public VS_AsnChoice
{
	 VS_H225EncryptIntAlg( void );

 	enum{
	e_nonStandard,
	e_isoAlgorithm
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225EncryptIntAlg & src);

	 operator VS_H225NonStandardParameter *( void );

	void Show( void ) const;

};


//////////////////////CLASS VS_H225Q954Details /////////////////////////

struct VS_H225Q954Details : public VS_AsnSequence
{
	 VS_H225Q954Details( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  conferenceCalling ;
 	 VS_AsnBoolean  threePartyService ;
 	void operator=(const VS_H225Q954Details& src);

};
//////////////////////CLASS VS_H225QseriesOptions /////////////////////////

struct VS_H225QseriesOptions : public VS_AsnSequence
{
	 VS_H225QseriesOptions( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_AsnBoolean  q932Full ;
 	 VS_AsnBoolean  q951Full ;
 	 VS_AsnBoolean  q952Full ;
 	 VS_AsnBoolean  q953Full ;
 	 VS_AsnBoolean  q955Full ;
 	 VS_AsnBoolean  q956Full ;
 	 VS_AsnBoolean  q957Full ;
 	 VS_H225Q954Details  q954Info ;
 	void operator=(const VS_H225QseriesOptions& src);

};
struct VS_H225SecurityCapabilities;
//////////////////////CLASS VS_H225H245Security /////////////////////////

struct VS_H225H245Security : public VS_AsnChoice
{
	 VS_H225H245Security( void );

 	enum{
	e_nonStandard,
	e_noSecurity,
	e_tls,
	e_ipsec
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225H245Security & src);

	 operator VS_H225NonStandardParameter *( void );
	 operator VS_H225SecurityCapabilities *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225SecurityServiceMode /////////////////////////

struct VS_H225SecurityServiceMode : public VS_AsnChoice
{
	 VS_H225SecurityServiceMode( void );

 	enum{
	e_nonStandard,
	e_none,
	e_default
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225SecurityServiceMode & src);

	 operator VS_H225NonStandardParameter *( void );

	void Show( void ) const;

};
//////////////////////CLASS VS_H225SecurityCapabilities /////////////////////////

struct VS_H225SecurityCapabilities : public VS_AsnSequence
{
	 VS_H225SecurityCapabilities( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225NonStandardParameter  nonStandard ;
 	 VS_H225SecurityServiceMode  encryption ;
 	 VS_H225SecurityServiceMode  authenticaton ;
 	 VS_H225SecurityServiceMode  integrity ;
 	void operator=(const VS_H225SecurityCapabilities& src);

};

//////////////////////CLASS VS_H225ExtendedAliasAddress /////////////////////////

struct VS_H225ExtendedAliasAddress : public VS_AsnSequence
{
	 VS_H225ExtendedAliasAddress( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225AliasAddress  address ;
 	 VS_H225PresentationIndicator  presentationIndicator ;
 	 VS_H225ScreeningIndicator  screeningIndicator ;
 	void operator=(const VS_H225ExtendedAliasAddress& src);

};
//////////////////////CLASS VS_H225GSM_UIM /////////////////////////

struct VS_H225GSM_UIM : public VS_AsnSequence
{
	 VS_H225GSM_UIM( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;


	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   IMSI_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };;
	 static unsigned char   imsi_inverse_table[256];
	 static const bool      imsi_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   MSISDN_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };;
	 static unsigned char   msisdn_inverse_table[256];
	 static const bool      msisdn_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   IMEI_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   imei_inverse_table[256];
	 static const bool      imei_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   HPLMN_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   hplmn_inverse_table[256];
	 static const bool      hplmn_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char  VPLMN_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '#', '*', 'a', 'b', 'c' };
	 static unsigned char   vplmn_inverse_table[256];
	 static const bool      vplmn_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////
//	TemplAlphabeticString< imsi_alphabet, &VS_H225GSM_UIM::imsi_alphabet_size,imsi_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>  imsi ;
	TemplAlphabeticString<IMSI_ALPHABET, sizeof(IMSI_ALPHABET), imsi_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> imsi;
 	TemplOctetString<1,4,VS_Asn::FixedConstraint,0>  tmsi ;
	TemplAlphabeticString<MSISDN_ALPHABET, sizeof(MSISDN_ALPHABET), msisdn_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> msisdn;
	TemplAlphabeticString<IMEI_ALPHABET, sizeof(IMEI_ALPHABET), imei_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> imei;
	TemplAlphabeticString<HPLMN_ALPHABET, sizeof(HPLMN_ALPHABET), hplmn_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> hplmn;
	TemplAlphabeticString<VPLMN_ALPHABET, sizeof(VPLMN_ALPHABET), vplmn_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> vplmn;
 	void operator=(const VS_H225GSM_UIM& src);

};
//////////////////////CLASS VS_H225ANSI_41_UIM_System_id /////////////////////////

struct VS_H225ANSI_41_UIM_System_id : public VS_AsnChoice
{
	 VS_H225ANSI_41_UIM_System_id( void );

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   SID_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned		sid_alphabet_size;
	 static unsigned char   sid_inverse_table[];
	 static const bool      sid_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   MID_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned		mid_alphabet_size;
	 static unsigned char   mid_inverse_table[];
	 static const bool      mid_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

 	enum{
	e_sid,
	e_mid
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225ANSI_41_UIM_System_id & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225ANSI_41_UIM /////////////////////////

struct VS_H225ANSI_41_UIM : public VS_AsnSequence
{
	 VS_H225ANSI_41_UIM( void );

	static const unsigned basic_root = 12;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;


	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   IMSSI_APLHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   imsi_inverse_table[256];
	 static const bool      imsi_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   MIN_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   min_inverse_table[256];
	 static const bool      min_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   MDN_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   mdn_inverse_table[256];
	 static const bool      mdn_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   MSISDN_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   msisdn_inverse_table[256];
	 static const bool      msisdn_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   ESN_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   esn_inverse_table[256];
	 static const bool      esn_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   MSCID_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   mscid_inverse_table[256];
	 static const bool      mscid_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char   SESN_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   sesn_inverse_table[256];
	 static const bool      sesn_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 static constexpr unsigned char SOC_ALPHABET[] = { '0','1','2','3','4','5','6','7','8','9','#','*','a','b','c' };
	 static unsigned char   soc_inverse_table[256];
	 static const bool      soc_flag_set_table;

	 /////////////////////////////////////////////////////////////////////////////////////////
	TemplAlphabeticString<IMSSI_APLHABET, sizeof(IMSSI_APLHABET), imsi_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> imsi;
	TemplAlphabeticString<MIN_ALPHABET, sizeof(MIN_ALPHABET), min_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> min;
	TemplAlphabeticString<MDN_ALPHABET, sizeof(MDN_ALPHABET), mdn_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> mdn;
	TemplAlphabeticString<MSISDN_ALPHABET, sizeof(MSISDN_ALPHABET), msisdn_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> msisdn;
	TemplAlphabeticString<ESN_ALPHABET, sizeof(ESN_ALPHABET), esn_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> esn;
	TemplAlphabeticString<MSCID_ALPHABET, sizeof(MSCID_ALPHABET), mscid_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> mscid;
 	VS_H225ANSI_41_UIM_System_id	 system_id ;
 	TemplOctetString<1,1,VS_Asn::FixedConstraint,0>  systemMyTypeCode ;
 	TemplOctetString<1,1,VS_Asn::FixedConstraint,0>  systemAccessType ;
 	TemplOctetString<1,1,VS_Asn::FixedConstraint,0>  qualificationInformationCode ;
	TemplAlphabeticString<SESN_ALPHABET, sizeof(SESN_ALPHABET), sesn_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> sesn;
	TemplAlphabeticString<SOC_ALPHABET, sizeof(SOC_ALPHABET), soc_inverse_table, 0, INT_MAX, VS_Asn::Unconstrained, false> soc;
 	void operator=(const VS_H225ANSI_41_UIM& src);

};
//////////////////////CLASS VS_H225TBCD_STRING /////////////////////////

typedef TemplIA5String<0,INT_MAX,VS_Asn::Unconstrained,false>  VS_H225TBCD_STRING;
//////////////////////CLASS VS_H225MobileUIM /////////////////////////

struct VS_H225MobileUIM : public VS_AsnChoice
{
	 VS_H225MobileUIM( void );

 	enum{
	e_ansi_41_uim,
	e_gsm_uim
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225MobileUIM & src);

	 operator VS_H225ANSI_41_UIM *( void );
	 operator VS_H225GSM_UIM *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225PrivateTypeOfNumber /////////////////////////

struct VS_H225PrivateTypeOfNumber : public VS_AsnChoice
{
	 VS_H225PrivateTypeOfNumber( void );

 	enum{
	e_unknown,
	e_level2RegionalNumber,
	e_level1RegionalNumber,
	e_pISNSpecificNumber,
	e_localNumber,
	e_abbreviatedNumber
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225PrivateTypeOfNumber & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225PublicTypeOfNumber /////////////////////////

struct VS_H225PublicTypeOfNumber : public VS_AsnChoice
{
	 VS_H225PublicTypeOfNumber( void );

 	enum{
	e_unknown,
	e_internationalNumber,
	e_nationalNumber,
	e_networkSpecificNumber,
	e_subscriberNumber,
	e_abbreviatedNumber
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225PublicTypeOfNumber & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225NumberDigits /////////////////////////

typedef TemplIA5String<1,128,VS_Asn::FixedConstraint,0>  VS_H225NumberDigits;
//////////////////////CLASS VS_H225PrivatePartyNumber /////////////////////////

struct VS_H225PrivatePartyNumber : public VS_AsnSequence
{
	 VS_H225PrivatePartyNumber( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225PrivateTypeOfNumber  privateTypeOfNumber ;
 	 VS_H225NumberDigits  privateNumberDigits ;
 	void operator=(const VS_H225PrivatePartyNumber& src);

};
//////////////////////CLASS VS_H225PublicPartyNumber /////////////////////////

struct VS_H225PublicPartyNumber : public VS_AsnSequence
{
	 VS_H225PublicPartyNumber( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225PublicTypeOfNumber  publicTypeOfNumber ;
 	 VS_H225NumberDigits  publicNumberDigits ;
 	void operator=(const VS_H225PublicPartyNumber& src);

};
//////////////////////CLASS VS_H225PartyNumber /////////////////////////

struct VS_H225PartyNumber : public VS_AsnChoice
{
	 VS_H225PartyNumber( void );

 	enum{
	e_e164Number,
	e_dataPartyNumber,
	e_telexPartyNumber,
	e_privateNumber,
	e_nationalStandardPartyNumber
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225PartyNumber & src);

	 operator VS_H225PublicPartyNumber *( void );
	 operator VS_H225NumberDigits *( void );
	 operator VS_H225PrivatePartyNumber *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225AddressPattern_Range /////////////////////////

struct VS_H225AddressPattern_Range : public VS_AsnSequence
{
	 VS_H225AddressPattern_Range( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225PartyNumber  startOfRange ;
 	 VS_H225PartyNumber  endOfRange ;
 	void operator=(const VS_H225AddressPattern_Range& src);

};
//////////////////////CLASS VS_H225AddressPattern /////////////////////////

struct VS_H225AddressPattern : public VS_AsnChoice
{
	 VS_H225AddressPattern( void );

 	enum{
	e_wildcard,
	e_range
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225AddressPattern & src);

	 operator VS_H225AliasAddress *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225RegistrationConfirm /////////////////////////

struct VS_H225RegistrationConfirm : public VS_AsnSequence
{
	 VS_H225RegistrationConfirm( void );

	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 17;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  callSignalAddress ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  terminalAlias ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	Constrained_array_of_type<  VS_H225AlternateGK ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateGatekeeper ;
 	 VS_H225TimeToLive  timeToLive ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_AsnBoolean  willRespondToIRR ;
 	VS_H225RegistrationConfirm_PreGrantedARQ	 preGrantedARQ ;
 	 VS_AsnBoolean  maintainConnection ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_AsnNull  supportsAdditiveRegistration ;
 	Constrained_array_of_type<  VS_H225AddressPattern ,0,INT_MAX,VS_Asn::Unconstrained,0  >  terminalAliasPattern ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	Constrained_array_of_type<  VS_H225RasUsageSpecification ,0,INT_MAX,VS_Asn::Unconstrained,0  >  usageSpec ;
 	 VS_H225AliasAddress  featureServerAlias ;
 	 VS_H225CapacityReportingSpecification  capacityReportingSpec ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225RegistrationConfirm& src);

};



//////////////////////CLASS VS_H225RegistrationRejectReason_InvalidTerminalAliases /////////////////////////

struct VS_H225RegistrationRejectReason_InvalidTerminalAliases : public VS_AsnSequence
{
	 VS_H225RegistrationRejectReason_InvalidTerminalAliases( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  terminalAlias ;
 	Constrained_array_of_type<  VS_H225AddressPattern ,0,INT_MAX,VS_Asn::Unconstrained,0  >  terminalAliasPattern ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225RegistrationRejectReason_InvalidTerminalAliases& src);

};


//////////////////////CLASS VS_H225TunnelledProtocolAlternateIdentifier /////////////////////////

struct VS_H225TunnelledProtocolAlternateIdentifier : public VS_AsnSequence
{
	 VS_H225TunnelledProtocolAlternateIdentifier( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplIA5String<1,64,VS_Asn::FixedConstraint,0>  protocolType ;
 	TemplIA5String<1,64,VS_Asn::FixedConstraint,0>  protocolVariant ;
 	void operator=(const VS_H225TunnelledProtocolAlternateIdentifier& src);

};
//////////////////////CLASS VS_H225TunnelledProtocol_Id /////////////////////////

struct VS_H225TunnelledProtocol_Id : public VS_AsnChoice
{
	 VS_H225TunnelledProtocol_Id( void );

 	enum{
	e_tunnelledProtocolObjectID,
	e_tunnelledProtocolAlternateID
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225TunnelledProtocol_Id & src);

	 operator VS_H225TunnelledProtocolAlternateIdentifier *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225TunnelledProtocol /////////////////////////

struct VS_H225TunnelledProtocol : public VS_AsnSequence
{
	 VS_H225TunnelledProtocol( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	VS_H225TunnelledProtocol_Id	 id ;
 	TemplIA5String<1,64,VS_Asn::FixedConstraint,0>  subIdentifier ;
 	void operator=(const VS_H225TunnelledProtocol& src);

};



//////////////////////CLASS VS_H225LocationRequest /////////////////////////

struct VS_H225LocationRequest : public VS_AsnSequence
{
	 VS_H225LocationRequest( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 12;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destinationInfo ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225TransportAddress  replyAddress ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  sourceInfo ;
 	 VS_AsnBoolean  canMapAlias ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225SupportedProtocols ,0,INT_MAX,VS_Asn::Unconstrained,0  >  desiredProtocols ;
 	 VS_H225TunnelledProtocol  desiredTunnelledProtocol ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	TemplInteger<1,255,VS_Asn::FixedConstraint,0>  hopCount ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	void operator=(const VS_H225LocationRequest& src);

};
//////////////////////CLASS VS_H225McuInfo /////////////////////////

struct VS_H225McuInfo : public VS_AsnSequence
{
	 VS_H225McuInfo( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225SupportedProtocols ,0,INT_MAX,VS_Asn::Unconstrained,0  >  protocol ;
 	void operator=(const VS_H225McuInfo& src);

};
//////////////////////CLASS VS_H225EndpointType /////////////////////////

struct VS_H225EndpointType : public VS_AsnSequence
{
	 VS_H225EndpointType( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225VendorIdentifier  vendor ;
 	 VS_H225GatekeeperInfo  gatekeeper ;
 	 VS_H225GatewayInfo  gateway ;
 	 VS_H225McuInfo  mcu ;
 	 //VS_H225TerminalInfo  terminal ;
  	 VS_H225TerminalInfo  terminalInfo;
 	 VS_AsnBoolean  mc ;
 	 VS_AsnBoolean  undefinedNode ;
 	TemplBitString<32,32,VS_Asn::FixedConstraint,0>  set ;
 	Constrained_array_of_type<  VS_H225TunnelledProtocol ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedTunnelledProtocols ;
 	void operator=(const VS_H225EndpointType& src);

};

//////////////////////CLASS VS_H225Endpoint /////////////////////////
struct VS_H225Endpoint : public VS_AsnSequence
{
	 VS_H225Endpoint( void );

	static const unsigned basic_root = 10;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  aliasAddress ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  callSignalAddress ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  rasAddress ;
 	 VS_H225EndpointType  endpointType ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	TemplInteger<0,127,VS_Asn::FixedConstraint,0>  priority ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  remoteExtensionAddress ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destExtraCallInfo ;
 	 VS_H225AlternateTransportAddresses  alternateTransportAddresses ;
 	void operator=(const VS_H225Endpoint& src);

};
//////////////////////CLASS VS_H225LocationConfirm /////////////////////////

struct VS_H225LocationConfirm : public VS_AsnSequence
{
	 VS_H225LocationConfirm( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 15;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225TransportAddress  callSignalAddress ;
 	 VS_H225TransportAddress  rasAddress ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destinationInfo ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destExtraCallInfo ;
 	 VS_H225EndpointType  destinationType ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  remoteExtensionAddress ;
 	Constrained_array_of_type<  VS_H225Endpoint ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateEndpoints ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225AlternateTransportAddresses  alternateTransportAddresses ;
 	Constrained_array_of_type<  VS_H225SupportedProtocols ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedProtocols ;
 	 VS_AsnBoolean  multipleCalls ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	void operator=(const VS_H225LocationConfirm& src);

};

//////////////////////CLASS VS_H225AdmissionConfirm /////////////////////////

struct VS_H225AdmissionConfirm : public VS_AsnSequence
{
	 VS_H225AdmissionConfirm( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 21;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225BandWidth  bandWidth ;
 	 VS_H225CallModel  callModel ;
 	 VS_H225TransportAddress  destCallSignalAddress ;
 	TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  irrFrequency ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destinationInfo ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destExtraCallInfo ;
 	 VS_H225EndpointType  destinationType ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  remoteExtensionAddress ;
 	Constrained_array_of_type<  VS_H225Endpoint ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateEndpoints ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225TransportQOS  transportQOS ;
 	 VS_AsnBoolean  willRespondToIRR ;
 	 VS_H225UUIEsRequested  uuiesRequested ;
 	Constrained_array_of_type< TemplIA5String<1,32,VS_Asn::FixedConstraint,0> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  language ;
 	 VS_H225AlternateTransportAddresses  alternateTransportAddresses ;
 	 VS_H225UseSpecifiedTransport  useSpecifiedTransport ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	Constrained_array_of_type<  VS_H225RasUsageSpecification ,0,INT_MAX,VS_Asn::Unconstrained,0  >  usageSpec ;
 	Constrained_array_of_type<  VS_H225SupportedProtocols ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedProtocols ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_AsnBoolean  multipleCalls ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225AdmissionConfirm& src);

};
//////////////////////CLASS VS_H225GatekeeperRequest /////////////////////////

struct VS_H225GatekeeperRequest : public VS_AsnSequence
{
	 VS_H225GatekeeperRequest( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 10;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225TransportAddress  rasAddress ;
 	 VS_H225EndpointType  endpointType ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	 VS_H225QseriesOptions  callServices ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  endpointAlias ;
 	Constrained_array_of_type<  VS_H225Endpoint ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateEndpoints ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type<  VS_H225AuthenticationMechanism ,0,INT_MAX,VS_Asn::Unconstrained,0  >  authenticationCapability ;
 	Constrained_array_of_type<  VS_AsnObjectId ,0,INT_MAX,VS_Asn::Unconstrained,0  >  algorithmOIDs ;
 	Constrained_array_of_type<  VS_H225IntegrityMechanism ,0,INT_MAX,VS_Asn::Unconstrained,0  >  integrity ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_AsnNull  supportsAltGK ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225GatekeeperRequest& src);

};



//////////////////////CLASS VS_H225AdmissionRequest /////////////////////////

struct VS_H225AdmissionRequest : public VS_AsnSequence
{
	 VS_H225AdmissionRequest( void );

	static const unsigned basic_root = 16;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 18;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225CallType  callType ;
 	 VS_H225CallModel  callModel ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destinationInfo ;
 	 VS_H225TransportAddress  destCallSignalAddress ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destExtraCallInfo ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  srcInfo ;
 	 VS_H225TransportAddress  srcCallSignalAddress ;
 	 VS_H225BandWidth  bandWidth ;
 	 VS_H225CallReferenceValue  callReferenceValue ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225QseriesOptions  callServices ;
 	 VS_H225ConferenceIdentifier  conferenceID ;
 	 VS_AsnBoolean  activeMC ;
 	 VS_AsnBoolean  answerCall ;
 	 VS_AsnBoolean  canMapAlias ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225Endpoint ,0,INT_MAX,VS_Asn::Unconstrained,0  >  srcAlternatives ;
 	Constrained_array_of_type<  VS_H225Endpoint ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destAlternatives ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225TransportQOS  transportQOS ;
 	 VS_AsnBoolean  willSupplyUUIEs ;
 	 VS_H225CallLinkage  callLinkage ;
 	 VS_H225DataRate  gatewayDataRate ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	Constrained_array_of_type<  VS_H225SupportedProtocols ,0,INT_MAX,VS_Asn::Unconstrained,0  >  desiredProtocols ;
 	 VS_H225TunnelledProtocol  desiredTunnelledProtocol ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225AdmissionRequest& src);

};

//////////////////////CLASS VS_H225UnregistrationRequest /////////////////////////


struct VS_H225UnregistrationRequest : public VS_AsnSequence
{
	 VS_H225UnregistrationRequest( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 10;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  callSignalAddress ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  endpointAlias ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	Constrained_array_of_type<  VS_H225Endpoint ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateEndpoints ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_H225UnregRequestReason  reason ;
 	Constrained_array_of_type<  VS_H225AddressPattern ,0,INT_MAX,VS_Asn::Unconstrained,0  >  endpointAliasPattern ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	Constrained_array_of_type<  VS_H225AlternateGK ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateGatekeeper ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225UnregistrationRequest& src);

};



//////////////////////CLASS VS_H225RegistrationRequest /////////////////////////

struct VS_H225RegistrationRequest : public VS_AsnSequence
{
	 VS_H225RegistrationRequest( void );

	static const unsigned basic_root = 10;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 21;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_AsnBoolean  discoveryComplete ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  callSignalAddress ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  rasAddress ;
 	 VS_H225EndpointType  terminalType ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  terminalAlias ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	 VS_H225VendorIdentifier  endpointVendor ;
 	Constrained_array_of_type<  VS_H225Endpoint ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateEndpoints ;
 	 VS_H225TimeToLive  timeToLive ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_AsnBoolean  keepAlive ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	 VS_AsnBoolean  willSupplyUUIEs ;
 	 VS_AsnBoolean  maintainConnection ;
 	 VS_H225AlternateTransportAddresses  alternateTransportAddresses ;
 	 VS_AsnNull  additiveRegistration ;
 	Constrained_array_of_type<  VS_H225AddressPattern ,0,INT_MAX,VS_Asn::Unconstrained,0  >  terminalAliasPattern ;
 	 VS_AsnNull  supportsAltGK ;
 	 VS_H225RasUsageInfoTypes  usageReportingCapability ;
 	 VS_AsnBoolean  multipleCalls ;
 	Constrained_array_of_type<  VS_H225H248PackagesDescriptor ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedH248Packages ;
 	 VS_H225CallCreditCapability  callCreditCapability ;
 	 VS_H225CapacityReportingCapability  capacityReportingCapability ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225RegistrationRequest& src);

};



//////////////////////CLASS VS_H225T38FaxAnnexbOnlyCaps /////////////////////////

struct VS_H225T38FaxAnnexbOnlyCaps : public VS_AsnSequence
{
	 VS_H225T38FaxAnnexbOnlyCaps( void );

	static const unsigned basic_root = 0;
	VS_Reference_of_Asn* ref = nullptr;
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H225T38FaxAnnexbOnlyCaps& src);

};
//////////////////////CLASS VS_H225NonStandardProtocol /////////////////////////

struct VS_H225NonStandardProtocol : public VS_AsnSequence
{
	 VS_H225NonStandardProtocol( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225NonStandardProtocol& src);

};
//////////////////////CLASS VS_H225T120OnlyCaps /////////////////////////

struct VS_H225T120OnlyCaps : public VS_AsnSequence
{
	 VS_H225T120OnlyCaps( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225T120OnlyCaps& src);

};
//////////////////////CLASS VS_H225VoiceCaps /////////////////////////

struct VS_H225VoiceCaps : public VS_AsnSequence
{
	 VS_H225VoiceCaps( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225VoiceCaps& src);

};
//////////////////////CLASS VS_H225H324Caps /////////////////////////

struct VS_H225H324Caps : public VS_AsnSequence
{
	 VS_H225H324Caps( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225H324Caps& src);

};
//////////////////////CLASS VS_H225H323Caps /////////////////////////

struct VS_H225H323Caps : public VS_AsnSequence
{
	 VS_H225H323Caps( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225H323Caps& src);

};
//////////////////////CLASS VS_H225H322Caps /////////////////////////

struct VS_H225H322Caps : public VS_AsnSequence
{
	 VS_H225H322Caps( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225H322Caps& src);

};
//////////////////////CLASS VS_H225H321Caps /////////////////////////

struct VS_H225H321Caps : public VS_AsnSequence
{
	 VS_H225H321Caps( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225H321Caps& src);

};
//////////////////////CLASS VS_H225H320Caps /////////////////////////

struct VS_H225H320Caps : public VS_AsnSequence
{
	 VS_H225H320Caps( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225H320Caps& src);

};
//////////////////////CLASS VS_H225H310Caps /////////////////////////

struct VS_H225H310Caps : public VS_AsnSequence
{
	 VS_H225H310Caps( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225DataRate ,0,INT_MAX,VS_Asn::Unconstrained,0  >  dataRatesSupported ;
 	Constrained_array_of_type<  VS_H225SupportedPrefix ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedPrefixes ;
 	void operator=(const VS_H225H310Caps& src);

};


//////////////////////CLASS VS_H225Notify_UUIE /////////////////////////

struct VS_H225Notify_UUIE : public VS_AsnSequence
{
	 VS_H225Notify_UUIE( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	void operator=(const VS_H225Notify_UUIE& src);

};
//////////////////////CLASS VS_H225SetupAcknowledge_UUIE /////////////////////////

struct VS_H225SetupAcknowledge_UUIE : public VS_AsnSequence
{
	 VS_H225SetupAcknowledge_UUIE( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	void operator=(const VS_H225SetupAcknowledge_UUIE& src);

};
//////////////////////CLASS VS_H225StatusInquiry_UUIE /////////////////////////

struct VS_H225StatusInquiry_UUIE : public VS_AsnSequence
{
	 VS_H225StatusInquiry_UUIE( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	void operator=(const VS_H225StatusInquiry_UUIE& src);

};
//////////////////////CLASS VS_H225Status_UUIE /////////////////////////

struct VS_H225Status_UUIE : public VS_AsnSequence
{
	 VS_H225Status_UUIE( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	void operator=(const VS_H225Status_UUIE& src);

};
//////////////////////CLASS VS_H225TransportAddress_IpAddress /////////////////////////

struct VS_H225TransportAddress_IpAddress : public VS_AsnSequence
{
	 VS_H225TransportAddress_IpAddress( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<4,4,VS_Asn::FixedConstraint,0>  ip ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  port ;
 	void operator=(const VS_H225TransportAddress_IpAddress& src);

	bool operator==( const VS_H225TransportAddress_IpAddress &src )
	{
		return (O_T(ip)) && (O_T(port)) && (O_T(filled));
	}


};
//////////////////////CLASS VS_H225TransportAddress_IpSourceRoute_Routing /////////////////////////

struct VS_H225TransportAddress_IpSourceRoute_Routing : public VS_AsnChoice
{
	 VS_H225TransportAddress_IpSourceRoute_Routing( void );

 	enum{
	e_strict,
	e_loose
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225TransportAddress_IpSourceRoute_Routing & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225TransportAddress_IpSourceRoute /////////////////////////

struct VS_H225TransportAddress_IpSourceRoute : public VS_AsnSequence
{
	 VS_H225TransportAddress_IpSourceRoute( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<4,4,VS_Asn::FixedConstraint,0>  ip ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  port ;
 	Constrained_array_of_type< TemplOctetString<4,4,VS_Asn::FixedConstraint,0> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  route ;
 	VS_H225TransportAddress_IpSourceRoute_Routing	 routing ;
 	void operator=(const VS_H225TransportAddress_IpSourceRoute& src);

};
//////////////////////CLASS VS_H225TransportAddress_IpxAddress /////////////////////////

struct VS_H225TransportAddress_IpxAddress : public VS_AsnSequence
{
	 VS_H225TransportAddress_IpxAddress( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<6,6,VS_Asn::FixedConstraint,0>  node ;
 	TemplOctetString<4,4,VS_Asn::FixedConstraint,0>  netnum ;
 	TemplOctetString<2,2,VS_Asn::FixedConstraint,0>  port ;
 	void operator=(const VS_H225TransportAddress_IpxAddress& src);

};
//////////////////////CLASS VS_H225TransportAddress_Ip6Address /////////////////////////

struct VS_H225TransportAddress_Ip6Address : public VS_AsnSequence
{
	 VS_H225TransportAddress_Ip6Address( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  ip ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  port ;
 	void operator=(const VS_H225TransportAddress_Ip6Address& src);

};


//////////////////////CLASS VS_H225Progress_UUIE /////////////////////////

struct VS_H225Progress_UUIE : public VS_AsnSequence
{
	 VS_H225Progress_UUIE( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 3;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225EndpointType  destinationInfo ;
 	 VS_H225TransportAddress  h245Address ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	 VS_H225H245Security  h245SecurityMode ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  fastStart ;
 	 VS_AsnBoolean  multipleCalls ;
 	 VS_AsnBoolean  maintainConnection ;
 	 VS_AsnNull  fastConnectRefused ;
 	void operator=(const VS_H225Progress_UUIE& src);

};
//////////////////////CLASS VS_H225FacilityReason /////////////////////////

struct VS_H225FacilityReason : public VS_AsnChoice
{
	 VS_H225FacilityReason( void );

 	enum{
	e_routeCallToGatekeeper,
	e_callForwarded,
	e_routeCallToMC,
	e_undefinedReason,
	e_conferenceListChoice,
	e_startH245,
	e_noH245,
	e_newTokens,
	e_featureSetUpdate,
	e_forwardedElements,
	e_transportedInformation
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225FacilityReason & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225ConferenceList /////////////////////////

struct VS_H225ConferenceList : public VS_AsnSequence
{
	 VS_H225ConferenceList( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225ConferenceIdentifier  conferenceID ;
 	 VS_H225AliasAddress  conferenceAlias ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	void operator=(const VS_H225ConferenceList& src);

};
//////////////////////CLASS VS_H225Facility_UUIE /////////////////////////

struct VS_H225Facility_UUIE : public VS_AsnSequence
{
	 VS_H225Facility_UUIE( void );

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 16;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225TransportAddress  alternativeAddress ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternativeAliasAddress ;
 	 VS_H225ConferenceIdentifier  conferenceID ;
 	 VS_H225FacilityReason  reason ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destExtraCallInfo ;
 	 VS_H225AliasAddress  remoteExtensionAddress ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type<  VS_H225ConferenceList ,0,INT_MAX,VS_Asn::Unconstrained,0  >  conferences ;
 	 VS_H225TransportAddress  h245Address ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  fastStart ;
 	 VS_AsnBoolean  multipleCalls ;
 	 VS_AsnBoolean  maintainConnection ;
 	 VS_AsnNull  fastConnectRefused ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	 VS_H225FeatureSet  featureSet ;
 	 VS_H225EndpointType  destinationInfo ;
 	 VS_H225H245Security  h245SecurityMode ;
 	void operator=(const VS_H225Facility_UUIE& src);

};

//////////////////////CLASS VS_H225ScnConnectionAggregation /////////////////////////

struct VS_H225ScnConnectionAggregation : public VS_AsnChoice
{
	 VS_H225ScnConnectionAggregation( void );

 	enum{
	e_auto,
	e_none,
	e_h221,
	e_bonded_mode1,
	e_bonded_mode2,
	e_bonded_mode3
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225ScnConnectionAggregation & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225ScnConnectionType /////////////////////////

struct VS_H225ScnConnectionType : public VS_AsnChoice
{
	 VS_H225ScnConnectionType( void );

 	enum{
	e_unknown,
	e_bChannel,
	e_hybrid2x64,
	e_hybrid384,
	e_hybrid1536,
	e_hybrid1920,
	e_multirate
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225ScnConnectionType & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225Setup_UUIE_ConferenceGoal /////////////////////////

struct VS_H225Setup_UUIE_ConferenceGoal : public VS_AsnChoice
{
	 VS_H225Setup_UUIE_ConferenceGoal( void );

 	enum{
	e_create,
	e_join,
	e_invite,
	e_capability_negotiation,
	e_callIndependentSupplementaryService
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225Setup_UUIE_ConferenceGoal & src);


	void Show( void ) const;

};

//////////////////////CLASS VS_H225Setup_UUIE_ConnectionParameters /////////////////////////

struct VS_H225Setup_UUIE_ConnectionParameters : public VS_AsnSequence
{
	 VS_H225Setup_UUIE_ConnectionParameters( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225ScnConnectionType  connectionType ;
 	TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  numberOfScnConnections ;
 	 VS_H225ScnConnectionAggregation  connectionAggregation ;
 	void operator=(const VS_H225Setup_UUIE_ConnectionParameters& src);

};
//////////////////////CLASS VS_H225Setup_UUIE /////////////////////////

struct VS_H225Setup_UUIE : public VS_AsnSequence
{
	 VS_H225Setup_UUIE( void );

	static const unsigned basic_root = 13;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 26;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225TransportAddress  h245Address ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  sourceAddress ;
 	 VS_H225EndpointType  sourceInfo ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destinationAddress ;
 	 VS_H225TransportAddress  destCallSignalAddress ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destExtraCallInfo ;
 	Constrained_array_of_type<  VS_H225CallReferenceValue ,0,INT_MAX,VS_Asn::Unconstrained,0  >  destExtraCRV ;
 	 VS_AsnBoolean  activeMC ;
 	 VS_H225ConferenceIdentifier  conferenceID ;
 	VS_H225Setup_UUIE_ConferenceGoal	 conferenceGoal ;
 	 VS_H225QseriesOptions  callServices ;
 	 VS_H225CallType  callType ;
 	 VS_H225TransportAddress  sourceCallSignalAddress ;
 	 VS_H225AliasAddress  remoteExtensionAddress ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225H245Security ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h245SecurityCapability ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  fastStart ;
 	 VS_AsnBoolean  mediaWaitForConnect ;
 	 VS_AsnBoolean  canOverlapSend ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	 VS_AsnBoolean  multipleCalls ;
 	 VS_AsnBoolean  maintainConnection ;
 	VS_H225Setup_UUIE_ConnectionParameters	 connectionParameters ;
 	Constrained_array_of_type< TemplIA5String<1,32,VS_Asn::FixedConstraint,0> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  language ;
 	 VS_H225PresentationIndicator  presentationIndicator ;
 	 VS_H225ScreeningIndicator  screeningIndicator ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_AsnNull  symmetricOperationRequired ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	Constrained_array_of_type<  VS_H225SupportedProtocols ,0,INT_MAX,VS_Asn::Unconstrained,0  >  desiredProtocols ;
 	Constrained_array_of_type<  VS_H225FeatureDescriptor ,0,INT_MAX,VS_Asn::Unconstrained,0  >  neededFeatures ;
 	Constrained_array_of_type<  VS_H225FeatureDescriptor ,0,INT_MAX,VS_Asn::Unconstrained,0  >  desiredFeatures ;
 	Constrained_array_of_type<  VS_H225FeatureDescriptor ,0,INT_MAX,VS_Asn::Unconstrained,0  >  supportedFeatures ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  parallelH245Control ;
 	Constrained_array_of_type<  VS_H225ExtendedAliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  additionalSourceAddresses ;
 	void operator=(const VS_H225Setup_UUIE& src);

};

struct VS_H225DisengageRequest;
struct VS_H225InfoRequestResponse;
//////////////////////CLASS VS_H225RasMessage /////////////////////////

struct VS_H225RasMessage : public VS_AsnChoice
{
	 VS_H225RasMessage( void );

 	enum{
	e_gatekeeperRequest,
	e_gatekeeperConfirm,
	e_gatekeeperReject,
	e_registrationRequest,
	e_registrationConfirm,
	e_registrationReject,
	e_unregistrationRequest,
	e_unregistrationConfirm,
	e_unregistrationReject,
	e_admissionRequest,
	e_admissionConfirm,
	e_admissionReject,
	e_bandwidthRequest,
	e_bandwidthConfirm,
	e_bandwidthReject,
	e_disengageRequest,
	e_disengageConfirm,
	e_disengageReject,
	e_locationRequest,
	e_locationConfirm,
	e_locationReject,
	e_infoRequest,
	e_infoRequestResponse,
	e_nonStandardMessage,
	e_unknownMessageResponse,
	e_requestInProgress,
	e_resourcesAvailableIndicate,
	e_resourcesAvailableConfirm,
	e_infoRequestAck,
	e_infoRequestNak,
	e_serviceControlIndication,
	e_serviceControlResponse
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225RasMessage & src);

	 operator VS_H225GatekeeperRequest *( void );
	 operator VS_H225GatekeeperConfirm *( void );
	 operator VS_H225GatekeeperReject *( void );
	 operator VS_H225RegistrationRequest *( void );
	 operator VS_H225RegistrationConfirm *( void );
	 operator VS_H225RegistrationReject *( void );
	 operator VS_H225UnregistrationRequest *( void );
	 operator VS_H225UnregistrationConfirm *( void );
	 operator VS_H225UnregistrationReject *( void );
	 operator VS_H225AdmissionRequest *( void );
	 operator VS_H225AdmissionConfirm *( void );
	 operator VS_H225AdmissionReject *( void );
	 operator VS_H225BandwidthRequest *( void );
	 operator VS_H225BandwidthConfirm *( void );
	 operator VS_H225BandwidthReject *( void );
	 operator VS_H225DisengageRequest *( void );
	 operator VS_H225DisengageConfirm *( void );
	 operator VS_H225DisengageReject *( void );
	 operator VS_H225LocationRequest *( void );
	 operator VS_H225LocationConfirm *( void );
	 operator VS_H225LocationReject *( void );
	 operator VS_H225InfoRequest *( void );
	 operator VS_H225InfoRequestResponse *( void );
	 operator VS_H225NonStandardMessage *( void );
	 operator VS_H225UnknownMessageResponse *( void );
	 operator VS_H225RequestInProgress *( void );
	 operator VS_H225ResourcesAvailableIndicate *( void );
	 operator VS_H225ResourcesAvailableConfirm *( void );
	 operator VS_H225InfoRequestAck *( void );
	 operator VS_H225InfoRequestNak *( void );
	 operator VS_H225ServiceControlIndication *( void );
	 operator VS_H225ServiceControlResponse *( void );

	void Show( void ) const;

};
//////////////////////CLASS VS_H225ReleaseCompleteReason /////////////////////////

struct VS_H225ReleaseCompleteReason : public VS_AsnChoice
{
	 VS_H225ReleaseCompleteReason( void );

 	enum{
	e_noBandwidth,
	e_gatekeeperResources,
	e_unreachableDestination,
	e_destinationRejection,
	e_invalidRevision,
	e_noPermission,
	e_unreachableGatekeeper,
	e_gatewayResources,
	e_badFormatAddress,
	e_adaptiveBusy,
	e_inConf,
	e_undefinedReason,
	e_facilityCallDeflection,
	e_securityDenied,
	e_calledPartyNotRegistered,
	e_callerNotRegistered,
	e_newConnectionNeeded,
	e_nonStandardReason,
	e_replaceWithConferenceInvite,
	e_genericDataReason,
	e_neededFeatureNotSupported,
	e_tunnelledSignallingRejected
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225ReleaseCompleteReason & src);

	 operator VS_H225NonStandardParameter *( void );
	 operator VS_H225ConferenceIdentifier *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225CallTerminationCause /////////////////////////

struct VS_H225CallTerminationCause : public VS_AsnChoice
{
	 VS_H225CallTerminationCause( void );

 	enum{
	e_releaseCompleteReason,
	e_releaseCompleteCauseIE
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225CallTerminationCause & src);

	 operator VS_H225ReleaseCompleteReason *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225DisengageRequest /////////////////////////

struct VS_H225DisengageRequest : public VS_AsnSequence
{
	 VS_H225DisengageRequest( void );

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 13;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	 VS_H225ConferenceIdentifier  conferenceID ;
 	 VS_H225CallReferenceValue  callReferenceValue ;
 	 VS_H225DisengageReason  disengageReason ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	 VS_H225GatekeeperIdentifier  gatekeeperIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_AsnBoolean  answeredCall ;
 	 VS_H225CallLinkage  callLinkage ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	 VS_H225RasUsageInformation  usageInformation ;
 	 VS_H225CallTerminationCause  terminationCause ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225DisengageRequest& src);

};
//////////////////////CLASS VS_H225ReleaseComplete_UUIE /////////////////////////

struct VS_H225ReleaseComplete_UUIE : public VS_AsnSequence
{
	 VS_H225ReleaseComplete_UUIE( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 9;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225ReleaseCompleteReason  reason ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  busyAddress ;
 	 VS_H225PresentationIndicator  presentationIndicator ;
 	 VS_H225ScreeningIndicator  screeningIndicator ;
 	 VS_H225CallCapacity  capacity ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_H225FeatureSet  featureSet ;
 	void operator=(const VS_H225ReleaseComplete_UUIE& src);

};
//////////////////////CLASS VS_H225Information_UUIE /////////////////////////

struct VS_H225Information_UUIE : public VS_AsnSequence
{
	 VS_H225Information_UUIE( void );

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 6;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  fastStart ;
 	 VS_AsnNull  fastConnectRefused ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	void operator=(const VS_H225Information_UUIE& src);

};
//////////////////////CLASS VS_H225Connect_UUIE /////////////////////////

struct VS_H225Connect_UUIE : public VS_AsnSequence
{
	 VS_H225Connect_UUIE( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 15;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225TransportAddress  h245Address ;
 	 VS_H225EndpointType  destinationInfo ;
 	 VS_H225ConferenceIdentifier  conferenceID ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	 VS_H225H245Security  h245SecurityMode ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  fastStart ;
 	 VS_AsnBoolean  multipleCalls ;
 	 VS_AsnBoolean  maintainConnection ;
 	Constrained_array_of_type< TemplIA5String<1,32,VS_Asn::FixedConstraint,0> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  language ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  connectedAddress ;
 	 VS_H225PresentationIndicator  presentationIndicator ;
 	 VS_H225ScreeningIndicator  screeningIndicator ;
 	 VS_AsnNull  fastConnectRefused ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225FeatureSet  featureSet ;
 	void operator=(const VS_H225Connect_UUIE& src);

};
//////////////////////CLASS VS_H225CallProceeding_UUIE /////////////////////////

struct VS_H225CallProceeding_UUIE : public VS_AsnSequence
{
	 VS_H225CallProceeding_UUIE( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 9;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225EndpointType  destinationInfo ;
 	 VS_H225TransportAddress  h245Address ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	 VS_H225H245Security  h245SecurityMode ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  fastStart ;
 	 VS_AsnBoolean  multipleCalls ;
 	 VS_AsnBoolean  maintainConnection ;
 	 VS_AsnNull  fastConnectRefused ;
 	 VS_H225FeatureSet  featureSet ;
 	void operator=(const VS_H225CallProceeding_UUIE& src);

};


//////////////////////CLASS VS_H225Alerting_UUIE /////////////////////////

struct VS_H225Alerting_UUIE : public VS_AsnSequence
{
	 VS_H225Alerting_UUIE( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 14;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225ProtocolIdentifier  protocolIdentifier ;
 	 VS_H225EndpointType  destinationInfo ;
 	 VS_H225TransportAddress  h245Address ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	 VS_H225H245Security  h245SecurityMode ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  fastStart ;
 	 VS_AsnBoolean  multipleCalls ;
 	 VS_AsnBoolean  maintainConnection ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alertingAddress ;
 	 VS_H225PresentationIndicator  presentationIndicator ;
 	 VS_H225ScreeningIndicator  screeningIndicator ;
 	 VS_AsnNull  fastConnectRefused ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225FeatureSet  featureSet ;
 	void operator=(const VS_H225Alerting_UUIE& src);

};

//////////////////////CLASS VS_H225H323_UU_PDU_H323_message_body /////////////////////////

struct VS_H225H323_UU_PDU_H323_message_body : public VS_AsnChoice
{
	 VS_H225H323_UU_PDU_H323_message_body( void );

 	enum{
	e_setup,
	e_callProceeding,
	e_connect,
	e_alerting,
	e_information,
	e_releaseComplete,
	e_facility,
	e_progress,
	e_empty,
	e_status,
	e_statusInquiry,
	e_setupAcknowledge,
	e_notify
	};
	bool Decode(VS_PerBuffer& buffer) override;

	void operator=(const VS_H225H323_UU_PDU_H323_message_body & src);

	 operator VS_H225Setup_UUIE *( void );
	 operator VS_H225CallProceeding_UUIE *( void );
	 operator VS_H225Connect_UUIE *( void );
	 operator VS_H225Alerting_UUIE *( void );
	 operator VS_H225Information_UUIE *( void );
	 operator VS_H225ReleaseComplete_UUIE *( void );
	 operator VS_H225Facility_UUIE *( void );
	 operator VS_H225Progress_UUIE *( void );
	 operator VS_H225Status_UUIE *( void );
	 operator VS_H225StatusInquiry_UUIE *( void );
	 operator VS_H225SetupAcknowledge_UUIE *( void );
	 operator VS_H225Notify_UUIE *( void );

	void Show( void ) const;

};

//////////////////////CLASS VS_H225StimulusControl /////////////////////////

struct VS_H225StimulusControl : public VS_AsnSequence
{
	 VS_H225StimulusControl( void );

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225NonStandardParameter  nonStandard ;
 	 VS_AsnNull  isText ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  h248Message ;
 	void operator=(const VS_H225StimulusControl& src);

};
//////////////////////CLASS VS_H225H323_UU_PDU_TunnelledSignallingMessage /////////////////////////

struct VS_H225H323_UU_PDU_TunnelledSignallingMessage : public VS_AsnSequence
{
	 VS_H225H323_UU_PDU_TunnelledSignallingMessage( void );

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225TunnelledProtocol  tunnelledProtocolID ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  messageContent ;
 	 VS_AsnNull  tunnellingRequired ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	void operator=(const VS_H225H323_UU_PDU_TunnelledSignallingMessage& src);

};

//////////////////////CLASS VS_H225H323_UU_PDU /////////////////////////

struct VS_H225H323_UU_PDU : public VS_AsnSequence
{
	 VS_H225H323_UU_PDU( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 9;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225H323_UU_PDU_H323_message_body	 h323_message_body ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h4501SupplementaryService ;
 	 VS_AsnBoolean  h245Tunneling ;
 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h245Control ;
 	Constrained_array_of_type<  VS_H225NonStandardParameter ,0,INT_MAX,VS_Asn::Unconstrained,0  >  nonStandardControl ;
 	 VS_H225CallLinkage  callLinkage ;
 	VS_H225H323_UU_PDU_TunnelledSignallingMessage	 tunnelledSignallingMessage ;
 	 VS_AsnNull  provisionalRespToH245Tunneling ;
 	 VS_H225StimulusControl  stimulusControl ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225H323_UU_PDU& src);

};
//////////////////////CLASS VS_H225PduType /////////////////////////

struct VS_H225PduType : public VS_AsnSequence
{
	 VS_H225PduType( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225H323_UU_PDU  h323pdu ;
 	 VS_AsnBoolean  sent ;
 	void operator=(const VS_H225PduType& src);

};
//////////////////////CLASS VS_H225PerCallInfoType /////////////////////////

struct VS_H225PerCallInfoType : public VS_AsnSequence
{
	 VS_H225PerCallInfoType( void );

	static const unsigned basic_root = 12;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 8;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225CallReferenceValue  callReferenceValue ;
 	 VS_H225ConferenceIdentifier  conferenceID ;
 	 VS_AsnBoolean  originator ;
 	Constrained_array_of_type<  VS_H225RTPSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  audio ;
 	Constrained_array_of_type<  VS_H225RTPSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  video ;
 	Constrained_array_of_type<  VS_H225TransportChannelInfo ,0,INT_MAX,VS_Asn::Unconstrained,0  >  data ;
 	 VS_H225TransportChannelInfo  h245 ;
 	 VS_H225TransportChannelInfo  callSignaling ;
 	 VS_H225CallType  callType ;
 	 VS_H225BandWidth  bandWidth ;
 	 VS_H225CallModel  callModel ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type<  VS_H225ConferenceIdentifier ,0,INT_MAX,VS_Asn::Unconstrained,0  >  substituteConfIDs ;
 	Constrained_array_of_type<  VS_H225PduType ,0,INT_MAX,VS_Asn::Unconstrained,0  >  pdu ;
 	 VS_H225CallLinkage  callLinkage ;
 	 VS_H225RasUsageInformation  usageInformation ;
 	 VS_H225CircuitInfo  circuitInfo ;
 	void operator=(const VS_H225PerCallInfoType& src);

};

//////////////////////CLASS VS_H225InfoRequestResponse /////////////////////////

struct VS_H225InfoRequestResponse : public VS_AsnSequence
{
	 VS_H225InfoRequestResponse( void );

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 8;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225NonStandardParameter  nonStandardData ;
 	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225EndpointType  endpointType ;
 	 VS_H225EndpointIdentifier  endpointIdentifier ;
 	 VS_H225TransportAddress  rasAddress ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  callSignalAddress ;
 	Constrained_array_of_type<  VS_H225AliasAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  endpointAlias ;
 	Constrained_array_of_type<  VS_H225PerCallInfoType ,0,INT_MAX,VS_Asn::Unconstrained,0  >  perCallInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_AsnBoolean  needResponse ;
 	 VS_H225CallCapacity  capacity ;
 	 VS_H225InfoRequestResponseStatus  irrStatus ;
 	 VS_AsnBoolean  unsolicited ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	void operator=(const VS_H225InfoRequestResponse& src);

};
//////////////////////CLASS VS_H225H323_UserInformation_User_data /////////////////////////

struct VS_H225H323_UserInformation_User_data : public VS_AsnSequence
{
	 VS_H225H323_UserInformation_User_data( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	TemplInteger<0,255,VS_Asn::FixedConstraint,0>  protocol_discriminator ;
 	TemplOctetString<1,131,VS_Asn::FixedConstraint,0>  user_information ;
 	void operator=(const VS_H225H323_UserInformation_User_data& src);

};
//////////////////////CLASS VS_H225H323_UserInformation /////////////////////////

struct VS_H225H323_UserInformation : public VS_AsnSequence
{
	 VS_H225H323_UserInformation( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225H323_UU_PDU  h323_uu_pdu ;
 	VS_H225H323_UserInformation_User_data	 user_data ;
 	void operator=(const VS_H225H323_UserInformation& src);

};
////////////////////////////=235/////////////////////////////////////////////////////////////

typedef TemplOctetString<8,128> VS_H235ChallengeString;//		::= OCTET STRING (SIZE(8..128))
typedef TemplInteger<1,4294967295>  VS_H235TimeStamp;//			::= INTEGER(1..4294967295)
typedef TemplInteger<0,INT_MAX,VS_Asn::Unconstrained> VS_H235RandomVal;//			::= INTEGER -- 32-bit Integer
typedef TemplBmpString<1,128>  VS_H235Password;//			::= BMPString (SIZE (1..128))
typedef TemplBmpString<1,128> VS_H235Identifier;//			::= BMPString (SIZE (1..128))
typedef TemplBitString<1,2048> VS_H235KeyMaterial;//		::= BIT STRING(SIZE(1..2048))

/////////////////////////////////////////////////////////////////////////////////////
struct VS_H235AuthenticationBES : public VS_AsnNullsChoice
{
	enum{e_default,	//NULL, -- encrypted ClearToken
	e_radius};//		NULL, -- RADIUS-challenge/response
	VS_H235AuthenticationBES( void )
		: VS_AsnNullsChoice(2,2,true)
	{}
	// end VS_H235AuthenticationBES::VS_H235AuthenticationBES
};
// end VS_H235AuthenticationBES struct
/////////////////////////////////////////////////////////////////////////////////////

struct VS_H235NonStandardParameter : public VS_AsnSequence
{
/////////////////////////////////////////////////////////////////////////////////////
	VS_H235NonStandardParameter( void )
		: VS_AsnSequence( 0,ref,basic_root,0,0,false)
	{
		ref[0].Set( &nonStandardIdentifier     , 0 );
		ref[1].Set( &data, 0 );
	}
	// end VS_H235NonStandardParameter::VS_H235NonStandardParameter
	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	/////////////////////////////////////////////////////////////////////////////////////
	VS_AsnObjectId nonStandardIdentifier;//OBJECT IDENTIFIER,
	TemplOctetString<0,INT_MAX,Unconstrained>  data;//	OCTET STRING
	void operator=( const VS_H235NonStandardParameter &src)
	{
		O_CC(filled);
		O_C(nonStandardIdentifier);
		O_C(data);
		O_CSA(ref, basic_root);
	}
	// end VS_H235NonStandardParameter::operator=
};
// end VS_H235NonStandardParameter struct
/////////////////////////////////////////////////////////////////////////////////////

struct VS_H235AuthenticationMechanism : public VS_AsnChoice
{
/////////////////////////////////////////////////////////////////////////////////////
	enum {
	e_dhExch,  //   		NULL, -- Diffie-Hellman
	e_pwdSymEnc,//		NULL, -- password with symmetric encryption
	e_pwdHash	,//	NULL, -- password with hashing
	e_certSign,//		NULL, -- Certificate with signature
	e_ipsec	,//		NULL, -- IPSEC based connection
	e_tls		,//	NULL,
	e_nonStandard,// 	NonStandardParameter, -- something else.
	//...,
	e_authenticationBES};//	AuthenticationBES -- user authentication for BES
	VS_H235AuthenticationMechanism( void )
		: VS_AsnChoice(7,8,true)
	{}
	// end VS_H235AuthenticationMechanism::VS_H235AuthenticationMechanism

	bool Decode(VS_PerBuffer& buffer) override
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
			case e_dhExch:
			case e_pwdSymEnc:
			case e_pwdHash:
			case e_certSign:
			case e_ipsec:
			case e_tls:						return DecodeChoice( buffer , new VS_AsnNull);
			case e_nonStandard:				return DecodeChoice( buffer , new VS_H225NonStandardParameter);
			case e_authenticationBES:			return DecodeChoice( buffer , new VS_H235AuthenticationBES);
			default:						return buffer.ChoiceMissExtensionObject(*this);
		}
	}
	// end VS_H235AuthenticationMechanism::Decode

	/////////////////////////////////////////////////////////////////////////////////////

	operator VS_AsnNull * ( void );

	void operator=(const VS_H235AuthenticationMechanism &src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
			case e_dhExch:				break;//null
			case e_pwdSymEnc:			break;//null
			case e_pwdHash:				break;//null
			case e_certSign:			break;//null
			case e_ipsec:				break;//null
			case e_tls:					break;//null
			case e_nonStandard:			return CopyChoice<VS_H225NonStandardParameter>( src ,*this);
			case e_authenticationBES:	return CopyChoice<VS_H235AuthenticationBES>( src ,*this);
			default: return ;
		}
		CopyChoice<VS_AsnNull>(src,*this);
		return;
	}
	// end VS_H235AuthenticationMechanism::operator=
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_H235AuthenticationMechanism struct

/////////////////////////////////////////////////////////////////////////////////////

struct VS_H235DHset : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////
	VS_H235DHset( void )
		: VS_AsnSequence( 0,ref,basic_root,0,0,true)
	{	ref[0].Set( &halfkey    , 0 );
		ref[1].Set( &modSize	, 0 );
		ref[2].Set( &generator	, 0 );
	}
	// end VS_H235DHset::VS_H235DHset
	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];

	TemplBitString<0,2048>	halfkey;//		BIT STRING (SIZE(0..2048)), -- = g^x mod n
	TemplBitString<0,2048>  modSize;//		BIT STRING (SIZE(0..2048)), --  n
	TemplBitString<0,2048>  generator;//	BIT STRING (SIZE(0..2048)), -- g
//	...
	void operator=( const VS_H235DHset &src)
	{	O_CC(filled);
		O_C(halfkey);
		O_C(modSize);
		O_C(generator);
		O_CSA(ref, basic_root);
	}
	// end VS_H235DHset::operator=
/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_H235DHset struct
/////////////////////////////////////////////////////////////////////////////////////

struct VS_H235ECpoint : public VS_AsnSequence
{
/////////////////////////////////////////////////////////////////////////////////////
	VS_H235ECpoint( void )
	: VS_AsnSequence( 2,ref,basic_root,0,0,true)
	{	ref[0].Set( &x  , 1 );
		ref[1].Set( &y	, 1 );
	}
	// end VS_H235ECpoint::VS_H235ECpoint
	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];

	TemplBitString<0,511> 	x;//BIT STRING (SIZE(0..511)) OPTIONAL,
	TemplBitString<0,511> 	y;//BIT STRING (SIZE(0..511)) OPTIONAL,
	//	...
	void operator=( const VS_H235ECpoint &src)
	{	O_CC(filled);
		O_C(x);
		O_C(y);
		O_CSA(ref, basic_root);
	}
	// end VS_H235ECpoint::operator=
/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_H235ECpoint struct
/////////////////////////////////////////////////////////////////////////////////////

struct VS_H235TypedCertificate : public VS_AsnSequence
{
/////////////////////////////////////////////////////////////////////////////////////
	VS_H235TypedCertificate( void )
	: VS_AsnSequence( 0,ref,basic_root,0,0,true)
	{	ref[0].Set( &type			, 0 );
		ref[1].Set( &certificate	, 0 );
	}
	// end VS_H235TypedCertificate::VS_H235TypedCertificate
	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];

	VS_AsnObjectId type;//			OBJECT IDENTIFIER,
	TemplOctetString<0,INT_MAX,VS_Asn::Unconstrained>  certificate;//		OCTET STRING,
	//...
	void operator=( const VS_H235TypedCertificate &src)
	{	O_CC(filled);
		O_C(type);
		O_C(certificate);
		O_CSA(ref, basic_root);
	}
	// end VS_H235TypedCertificate::operator=
/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_H235TypedCertificate struct
/////////////////////////////////////////////////////////////////////////////////////

struct VS_H235ECKASDH_Eckasdhp : public VS_AsnSequence
{
/////////////////////////////////////////////////////////////////////////////////////
	VS_H235ECKASDH_Eckasdhp( void )
	: VS_AsnSequence( 0,ref,basic_root,0,0,true)
	{	ref[0].Set( &public_key		, 0 );
		ref[1].Set( &modulus		, 0 );
		ref[2].Set( &base			, 0 );
		ref[3].Set( &weierstrassA	, 0 );
		ref[4].Set( &weierstrassB	, 0 );
	}
	// end VS_H235ECKASDH_Eckasdhp::VS_H235ECKASDH_Eckasdhp
	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	/////////////////////////////////////////////////////////////////////////////////////
	VS_H235ECpoint			public_key;
	TemplBitString<0,511>	modulus;
	VS_H235ECpoint			base ;
	TemplBitString<0,511>	weierstrassA;
	TemplBitString<0,511>	weierstrassB;
	void operator=( const VS_H235ECKASDH_Eckasdhp &src)
	{	O_CC(filled);
		O_C(public_key);
		O_C(modulus);
		O_C(base);
		O_C(weierstrassA);
		O_C(weierstrassB);
		O_CSA(ref, basic_root);

	}
	// end VS_H235ECKASDH_Eckasdhp::operator=
/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_H235ECKASDH_Eckasdhp struct

/////////////////////////////////////////////////////////////////////////////////////
//typedef VS_H235ECKASDH_Eckasdh VS_H235ECKASDH_Eckasdhp;
typedef VS_H235ECKASDH_Eckasdhp VS_H235ECKASDH_Eckasdh2;


/////////////////////////////////////////////////////////////////////////////////////
struct VS_H235ECKASDH : public VS_AsnChoice
{
	enum { e_eckasdhp,
		   e_eckasdh2};
	VS_H235ECKASDH( void )
		: VS_AsnChoice(2,2,true)
	{}
	// end VS_H235ECKASDH::VS_H235ECKASDH

	bool Decode(VS_PerBuffer& buffer) override
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
			case e_eckasdhp:	return DecodeChoice( buffer , new VS_H235ECKASDH_Eckasdhp);
			case e_eckasdh2:	return DecodeChoice( buffer , new VS_H235ECKASDH_Eckasdh2);
			default:			return buffer.ChoiceMissExtensionObject(*this);
		}
	}
	// end VS_H235ECKASDH::Decode

	/////////////////////////////////////////////////////////////////////////////////////

	//operator VS_AsnNull * ( void );

	void operator=(const VS_H235AuthenticationMechanism &src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
			case e_eckasdhp:		return CopyChoice<VS_H235ECKASDH_Eckasdhp>( src ,*this);
			case e_eckasdh2:		return CopyChoice<VS_H235ECKASDH_Eckasdh2>( src ,*this);
			default: return ;
		}
		return;
	}
	// end VS_H235ECKASDH::operator=
	/////////////////////////////////////////////////////////////////////////////////////

};
// end VS_H235ECKASDH struct
/////////////////////////////////////////////////////////////////////////////////////

struct VS_H235ClearToken : public VS_AsnSequence
{
/////////////////////////////////////////////////////////////////////////////////////
	VS_H235ClearToken( void )
	: VS_AsnSequence( 8,ref,basic_root,e_ref,extension_root,true)
	{	ref[0].Set( &tokenOID		, 0 );
		ref[1].Set( &timeStamp		, 1 );
		ref[2].Set( &password		, 1 );
		ref[3].Set( &dhkey			, 1 );
		ref[4].Set( &challenge		, 1 );
		ref[5].Set( &random			, 1 );
		ref[6].Set( &certificate	, 1 );
		ref[7].Set( &generalID		, 1 );
		ref[8].Set( &nonStandard	, 1 );

		e_ref[0].Set( &eckasdhkey	, 1 );
		e_ref[1].Set( &sendersID	, 1 );

	}
	// end VS_H235ClearToken::VS_H235ClearToken
	static const unsigned basic_root = 9;
	VS_Reference_of_Asn ref[basic_root];

	static const unsigned extension_root = 2;
	VS_Reference_of_Asn e_ref[extension_root];

	/////////////////////////////////////////////////////////////////////////////////////
	VS_AsnObjectId	tokenOID;//		OBJECT IDENTIFIER,
	VS_H235TimeStamp	timeStamp;//		TimeStamp OPTIONAL,
	VS_H235Password		password;//		Password OPTIONAL,
	VS_H235DHset			dhkey	;//		DHset OPTIONAL,
	VS_H235ChallengeString challenge;//		ChallengeString OPTIONAL,
	VS_H235RandomVal	random	;//	RandomVal OPTIONAL,
	VS_H235TypedCertificate certificate;//		TypedCertificate OPTIONAL,
	VS_H235Identifier	generalID	;//	Identifier OPTIONAL,
	VS_H235NonStandardParameter nonStandard ;//	NonStandardParameter OPTIONAL,
	//...,
	VS_H235ECKASDH		eckasdhkey	;//	ECKASDH OPTIONAL, -- elliptic curve Key Agreement Scheme-Diffie
	VS_H235Identifier	sendersID	;//	Identifier OPTIONAL
	void operator=( const VS_H235ClearToken &src)
	{	O_CC(filled);
		O_C(tokenOID);
		O_C(timeStamp);
		O_C(password);
		O_C(dhkey);
		O_C(challenge);
		O_C(random);
		O_C(certificate);
		O_C(generalID);
		O_C(nonStandard);

		O_C(eckasdhkey);
		O_C(sendersID);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}
	// end VS_H235ClearToken::operator=
/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_H235ClearToken struct
/////////////////////////////////////////////////////////////////////////////////////

typedef Array_of_type<VS_H235ClearToken> VS_H225ArrayOf_ClearToken;

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
typedef TemplOctetString<8,8,VS_Asn::FixedConstraint> VS_H235IV8;// ::= OCTET STRING (SIZE(8))
typedef TemplOctetString<8,8,VS_Asn::FixedConstraint> VS_H235IV16 ;//::= OCTET STRING (SIZE(16))

struct VS_H235Params : public VS_AsnSequence
{
	VS_H235Params( void )
	: VS_AsnSequence( 2 ,ref,basic_root,e_ref,extension_root,true)
	{	ref[0].Set( &ranInt		, 1 );
		ref[1].Set( &iv8		, 1 );

		e_ref[0].Set( &iv16	, 1 );
	}
	// end VS_H235Params::VS_H235Params
	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];

	static const unsigned extension_root = 1;
	VS_Reference_of_Asn e_ref[extension_root];

	TemplInteger<0,INT_MAX,Unconstrained> ranInt;//	INTEGER OPTIONAL, -- some integer value
	VS_H235IV8	iv8;//			IV8 OPTIONAL,	-- 8 octet initialization vector
		//...,
	VS_H235IV16	iv16;//			IV16 OPTIONAL	-- 16 octet initialization vector
	void operator=( const VS_H235Params &src)
	{	O_CC(filled);
		O_C(ranInt);
		O_C(iv8);
		O_C(iv16);
		O_CSA(ref, basic_root);
		O_CSA(e_ref, extension_root);
	}
	// end VS_H235Params::operator=
};
// end VS_H235Params struct
/////////////////////////////////////////////////////////////////////////////////////
template <class ToBeHashed > struct VS_H235HASHED : public VS_AsnSequence
{
	VS_H235HASHED< ToBeHashed >( void )
	: VS_AsnSequence( 0 ,ref,basic_root,0,0,false)
	{	ref[0].Set( &algorithmOID		, 0 );
		ref[1].Set( &paramS				, 0 );
		ref[2].Set( &hash				, 0 );
	}
	// end VS_H235HASHED::VS_H235HASHED
	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];

	VS_AsnObjectId	algorithmOID;//		OBJECT IDENTIFIER,
	VS_H235Params	paramS;//			Params,	-- any "runtime" parameters
	TemplBitString<0,INT_MAX,Unconstrained>  hash;//				BIT STRING
	void operator=( const VS_H235HASHED &src)
	{	O_CC(filled);
		O_C(algorithmOID);
		O_C(paramS);
		O_C(hash);
		O_CSA(ref, basic_root);
	}
	// end VS_H235HASHED::operator=

};
// end VS_H235HASHED struct
/////////////////////////////////////////////////////////////////////////////////////
template <class ToBeEncrypted > struct VS_H235ENCRYPTED  : public VS_AsnSequence
{
	VS_H235ENCRYPTED< ToBeEncrypted >( void )
	: VS_AsnSequence( 0 ,ref,basic_root,0,0,false)
	{	ref[0].Set( &algorithmOID		, 0 );
		ref[1].Set( &paramS				, 0 );
		ref[2].Set( &encryptedData				, 0 );
	}
	// end VS_H235ENCRYPTED::VS_H235ENCRYPTED
	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];

	VS_AsnObjectId	algorithmOID;//		OBJECT IDENTIFIER,
	VS_H235Params	paramS;//			Params,	-- any "runtime" parameters
	TemplOctetString<0,INT_MAX,Unconstrained>  encryptedData;//				BIT STRING
	void operator=( const VS_H235ENCRYPTED &src)
	{	O_CC(filled);
		O_C(algorithmOID);
		O_C(paramS);
		O_C(encryptedData);
		O_CSA(ref, basic_root);
	}
	// end VS_H235ENCRYPTED::operator=
};
// end VS_H235ENCRYPTED struct
/////////////////////////////////////////////////////////////////////////////////////
template <class ToBeSigned> struct VS_H235SIGNED   : public VS_AsnSequence
{
	VS_H235SIGNED< ToBeSigned >( void )
	: VS_AsnSequence( 0 ,ref,basic_root,0,0,false)
	{
		ref[0].Set( &toBeSigned		, 0 );
		ref[1].Set( &algorithmOID	, 0 );
		ref[2].Set( &paramS			, 0 );
		ref[3].Set( &signature		, 0 );
	}
	// end VS_H235SIGNED::VS_H235SIGNED
	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];

	ToBeSigned toBeSigned;
	VS_AsnObjectId	algorithmOID;//		OBJECT IDENTIFIER,
	VS_H235Params	paramS;//			Params,	-- any "runtime" parameters
	TemplBitString<0,INT_MAX,Unconstrained>  signature;//				BIT STRING
	void operator=( const VS_H235SIGNED &src)
	{	O_CC(filled);
		O_C(toBeSigned);
		O_C(algorithmOID);
		O_C(paramS);
		O_C(signature);
		O_CSA(ref, basic_root);
	}
	// end VS_H235SIGNED::operator=
};
// end VS_H235SIGNED struct
/////////////////////////////////////////////////////////////////////////////////////

typedef VS_H235ClearToken VS_H235PwdCertToken;
typedef VS_H235ClearToken VS_H235FastStartToken;
typedef Type_id<VS_H235PwdCertToken>  VS_H235EncodedPwdCertToken;
typedef Type_id<VS_H235ClearToken>  VS_H235EncodedGeneralToken;
typedef Type_id<VS_H235FastStartToken>  VS_H235EncodedFastStartToken;



/////////////////////////////////////////////////////////////////////////////////////
typedef VS_H248PkgdName VS_H248SignalName;
typedef TemplInteger<0,65353> VS_H248StreamID;
typedef TemplOctetString<0,UINT_MAX,VS_Asn::Unconstrained>  VS_H248Value;
/////////////////////////////////////////////////////////////////////////////////////
struct VS_H248SigParameter_ExtraInfo : VS_AsnChoice
{	enum{   e_relation, // Relation,
            e_range   , // BOOLEAN,
			e_sublist }; // BOOLEAN
	VS_H248SigParameter_ExtraInfo( void )
	:VS_AsnChoice(3,3,false)
		{}
	// end VS_H248SigParameter_ExtraInfo::VS_H248SigParameter_ExtraInfo
	bool Decode(VS_PerBuffer& buffer) override
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
			case e_relation:	return false; /// not resolved
			case e_range:		return DecodeChoice( buffer , new VS_AsnBoolean);
			case e_sublist:		return DecodeChoice( buffer , new VS_AsnBoolean);
			default:			return buffer.ChoiceMissExtensionObject(*this);
		}
	}
	// end VS_H248SigParameter_ExtraInfo::Decode

	/////////////////////////////////////////////////////////////////////////////////////
	void operator=(const VS_H248SigParameter_ExtraInfo &src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
			case e_relation:return; /// not resolved
			case e_range:	return CopyChoice< VS_AsnBoolean >( src ,*this);
			case e_sublist:	return CopyChoice< VS_AsnBoolean >( src ,*this);
			default: return ;
		}
		return;
	}
	// end VS_H248SigParameter_ExtraInfo::operator=
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_H248SigParameter_ExtraInfo struct
/////////////////////////////////////////////////////////////////////////////////////
struct VS_H248SigParameter : public VS_AsnSequence
{
  VS_H248SigParameter( void )
  : VS_AsnSequence(1,ref,basic_root,0,0,true)
  {	ref[0].Set( &sigParameterName  ,0);
	ref[1].Set( &value			   ,0);
	ref[2].Set( &extraInfo		   ,1);
  }
  // end VS_H248SigParameter::VS_H248SigParameter
  static const unsigned basic_root = 3;
  VS_Reference_of_Asn ref[basic_root];

  VS_H248Name sigParameterName;//  Name,
  VS_H248Value value;//             Value,
  VS_H248SigParameter_ExtraInfo extraInfo;//   CHOICE  OPTIONAL,
  //...
  void operator=(const VS_H248SigParameter &src)
 {
	O_CC(filled);
	O_C(sigParameterName);
	O_C(value);
	O_C(extraInfo);
	O_CSA(ref, basic_root);
  }
// end VS_H248SigParameter::operato=
};
// end VS_H248SigParameter struct
/////////////////////////////////////////////////////////////////////////////////////
struct VS_H248SignalType : public VS_AsnEnumeration
{
	enum{
	 e_brief,
	 e_onOff,
	 e_timeOut};// ...
	VS_H248SignalType( void )
		: VS_AsnEnumeration(2,0,true)
		{}
	// end VS_H248SignalType::VS_H248SignalType
};
// end VS_H248SignalType struct
/////////////////////////////////////////////////////////////////////////////////////
struct VS_H248Signal : public VS_AsnSequence//üþóºª ñvª¹ ÿ¨þñûõüüv ÷ôõ¸¹
{
	VS_H248Signal( void )
	: VS_AsnSequence(5,ref,basic_root,0,0,true)
	{
		ref[0].Set(&signalName	,0);
		ref[1].Set(&streamID	,1);
		ref[2].Set(&sigType		,1);
		ref[3].Set(&duration	,1);
		ref[4].Set(&notifyCompletion ,1);
		ref[5].Set(&keepActive	,1);
		ref[6].Set(&sigParList	,0);

	}
	// end VS_H248Signal::VS_H248Signal
	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];

	VS_H248SignalName signalName;
	VS_H248StreamID streamID;//o
	VS_H248SignalType sigType;//o
	TemplInteger<0,65535> duration;//o
	TemplBitString<4,4,VS_Asn::FixedConstraint,true>  notifyCompletion;//o//üþóºª ñvª¹ ÿ¨þñûõüüv ÷ôõ¸¹
	// ýõ ¸þò¸õü ¨ð÷þñ¨ðû¸  úðú ªðúøõ úûð¸¸v ¨ð÷¨ºûøòðª¹
	VS_AsnBoolean keepActive;//o
	Array_of_type<VS_H248SigParameter> sigParList;
	//...

	void operator=( const VS_H248Signal &src)
	{	O_CC(filled);
		O_C(signalName);		O_C(streamID);		O_C(sigType);
		O_C(duration);		O_C(notifyCompletion);		O_C(keepActive);
		O_C(sigParList);
		O_CSA(ref, basic_root);

	}
	// end VS_H248Signal::operator=
};
// end VS_H248Signal struct

//////////////////////CLASS VS_H225CryptoEPPwdHash /////////////////////////

struct VS_H225CryptoEPPwdHash : public VS_AsnSequence
{
	VS_H225CryptoEPPwdHash();

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	void operator=(const VS_H225CryptoEPPwdHash& src);

	// Fields of the message.
	VS_H225AliasAddress					alias;
	VS_H225TimeStamp					timestamp;
	VS_H235HASHED<VS_H235PwdCertToken>	token;
};