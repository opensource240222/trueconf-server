/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 19.12.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_CsMessages.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VS_AsnBuffers.h"
#include "VS_H225Messages.h"

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////CLASS VS_H225Alerting_UUIE /////////////////////////

struct VS_CsAlertingUuie : public VS_AsnSequence
{
	 VS_CsAlertingUuie( void );

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
 	void operator=(const VS_CsAlertingUuie& src);
	//bool operator==(const VS_H225Alerting_UUIE& src);

};
/*
struct VS_CsAlertingUuie : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsAlertingUuie( void )
//		: VS_AsnSequence( 1, ref, basic_root, e_ref, extension_root, true )
		: VS_AsnSequence( 1, ref, basic_root, 0, 0, true )
	{
		ref[0].Set( &protocolIdentifier, 0 );
		ref[1].Set( &destinationInfo, 0 );
		ref[2].Set( &h245Address, 1 );
//		e_ref[0].Set( &callIdentifier, 0 );
	}
	// end of VS_CsAlertingUuie constructor

	static const unsigned   basic_root = 3;
	VS_Reference_of_Asn   ref[basic_root];
//	static const unsigned   extension_root = 1;
//	VS_Reference_of_Asn   e_ref[extension_root];

	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225EndpointType   destinationInfo;
	VS_H225TransportAddress   h245Address;				// OPTIONAL
//	VS_H225CallIdentifier   callIdentifier;				// EXTENSION
//	h245SecurityMode	H245Security OPTIONAL,
//	tokens			SEQUENCE OF ClearToken OPTIONAL,
//	cryptoTokens		SEQUENCE OF CryptoH323Token OPTIONAL,
//	fastStart		SEQUENCE OF OCTET STRING OPTIONAL,
//	multipleCalls		BOOLEAN,
//	maintainConnection	BOOLEAN,
//	alertingAddress	SEQUENCE OF AliasAddress OPTIONAL,
//	presentationIndicator	PresentationIndicator OPTIONAL,
//	screeningIndicator	ScreeningIndicator OPTIONAL,
//	fastConnectRefused	NULL OPTIONAL,
//	serviceControl		SEQUENCE OF ServiceControlSession OPTIONAL,
//	capacity		CallCapacity OPTIONAL,
//	featureSet		FeatureSet OPTIONAL

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsAlertingUuie struct
*/
/////////////////////////////////////////////////////////////////////////////////////////

struct VS_CsReleaseCompleteUuie_ReleaseCompleteReason : public VS_AsnNullsChoice
{
	/////////////////////////////////////////////////////////////////////////////////////

	enum Choices {		e_noBandwidth,
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
						e_undefinedReason		};

	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsReleaseCompleteUuie_ReleaseCompleteReason( void )
		: VS_AsnNullsChoice( 12, 12, true ) {}
	// end of VS_CsReleaseCompleteUuie_ReleaseCompleteReason constructor

	/////////////////////////////////////////////////////////////////////////////////////

	void operator=( const VS_CsReleaseCompleteUuie_ReleaseCompleteReason &src )
	{	VS_AsnNullsChoice::operator =( (VS_AsnNullsChoice &)src );	}
	// end of VS_CsReleaseCompleteUuie_ReleaseCompleteReason::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsReleaseCompleteUuie_ReleaseCompleteReason struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_CsReleaseCompleteUuie : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsReleaseCompleteUuie( void )
		: VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&reason,1);
		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&busyAddress,1);
		e_ref[4].Set(&presentationIndicator,1);
		e_ref[5].Set(&screeningIndicator,1);
		e_ref[6].Set(&capacity,1);
		e_ref[7].Set(&serviceControl,1);
		e_ref[8].Set(&featureSet,1);
	}
	// end of VS_CsReleaseCompleteUuie constructor

	static const unsigned   basic_root = 2;
	VS_Reference_of_Asn   ref[basic_root];
	static const unsigned extension_root = 9;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225ProtocolIdentifier   protocolIdentifier;
	//VS_CsReleaseCompleteUuie_ReleaseCompleteReason   reason;	// OPTIONAL
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

	/////////////////////////////////////////////////////////////////////////////////////

	void operator=( const VS_CsReleaseCompleteUuie &src )
	{
		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(reason);
		O_C(callIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(busyAddress);
		O_C(presentationIndicator);
		O_C(screeningIndicator);
		O_C(capacity);
		O_C(serviceControl);
		O_C(featureSet);
		O_CSA(ref, basic_root);
		O_CSA(e_ref, extension_root);
	}
	// end of VS_CsReleaseCompleteUuie::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsReleaseCompleteUuie struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_CsSetupUuie_ConferenceGoal : public VS_AsnChoice
{
	/////////////////////////////////////////////////////////////////////////////////////

	enum {		e_create,
				e_join,
				e_invite,
				e_capability_negotiation,
				e_callIndependentSupplementaryService		};

	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsSetupUuie_ConferenceGoal( void )
		: VS_AsnChoice(3 , 5 , 1 ) {}
	// end of VS_CsSetupUuie_ConferenceGoal constructor

	/////////////////////////////////////////////////////////////////////////////////////

	void operator=( const VS_CsSetupUuie_ConferenceGoal &src )
	{
		//VS_AsnNullsChoice::operator =( (VS_AsnNullsChoice &)src );
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_create : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_join : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invite : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_capability_negotiation : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_callIndependentSupplementaryService : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}
	// end of VS_CsSetupUuie_ConferenceGoal::operator=
	bool Decode(VS_PerBuffer& buffer) override
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_create : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_join : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invite : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_capability_negotiation : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_callIndependentSupplementaryService : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}
	void Show( void )
	{
		printf("\n\t----------- VS_CsSetupUuie_ConferenceGoal::SHOW-----------");
		if (!filled) return;
		printf("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_create :  printf("\n\t choice:  VS_AsnNull  ");return;
		case e_join :  printf("\n\t choice:  VS_AsnNull  ");return;
		case e_invite :  printf("\n\t choice:  VS_AsnNull  ");return;
		case e_capability_negotiation :  printf("\n\t choice:  VS_AsnNull  ");return;
		case e_callIndependentSupplementaryService :  printf("\n\t choice:  VS_AsnNull  ");return;
		default: printf("\n\t unknown choice: %u",tag); return ;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsSetupUuie_ConferenceGoal struct

/////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Setup_UUIE /////////////////////////

struct VS_CsSetupUuie : public VS_AsnSequence
{
	 VS_CsSetupUuie( void );

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
	VS_CsSetupUuie_ConferenceGoal	 conferenceGoal ;
 	 VS_H225QseriesOptions  callServices ;
 	 VS_H225CallType  callType ;
 	 VS_H225TransportAddress  sourceCallSignalAddress ;
 	 VS_H225AliasAddress  remoteExtensionAddress ;
 	 VS_H225CallIdentifier  callIdentifier ;
 	Constrained_array_of_type<  VS_H225H245Security ,0,INT_MAX,VS_Asn::Unconstrained,0  >  h245SecurityCapability ;
	VS_H225ArrayOf_ClearToken  tokens ;
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
 	void operator=(const VS_CsSetupUuie& src);
	//bool operator==(const VS_H225Setup_UUIE& src);

};
/*
//////////////////////CLASS VS_H225Facility_UUIE /////////////////////////
struct VS_CsSetupUuie : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsSetupUuie( void )
		: VS_AsnSequence( 7, ref, basic_root, e_ref, extension_root, true )
	{
		ref[0].Set( &protocolIdentifier, 0 );
		ref[1].Set( &h245Address, 1 );
		ref[2].Set( &sourceAddress, 1 );
		ref[3].Set( &sourceInfo, 0 );
		ref[4].Set( &destinationAddress, 1 );
		ref[5].Set( &destCallSignalAddress, 1 );
		ref[6].Set( &destExtraCallInfo, 1 );
		ref[7].Set( &destExtraCRV, 1 );
		ref[8].Set( &activeMC, 0 );
		ref[9].Set( &conferenceID, 0 );
		ref[10].Set( &conferenceGoal, 0 );
		ref[11].Set( &callServices, 1 );
		ref[12].Set( &callType, 0 );
		e_ref[0].Set( &sourceCallSignalAddress, 1 );
		e_ref[1].Set( &remoteExtensionAddress, 1 );
		e_ref[2].Set( &callIdentifier, 0 );
	}
	// end of VS_CsSetupUuie constructor

	static const unsigned   basic_root = 13;
	VS_Reference_of_Asn   ref[basic_root];
	static const unsigned   extension_root = 3;
	VS_Reference_of_Asn   e_ref[extension_root];

	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225TransportAddress   h245Address;						// OPTIONAL
	Array_of_type<VS_H225AliasAddress>   sourceAddress;			// OPTIONAL
	VS_H225EndpointType   sourceInfo;
	Array_of_type<VS_H225AliasAddress>   destinationAddress;	// OPTIONAL
	VS_H225TransportAddress   destCallSignalAddress;			// OPTIONAL
	Array_of_type<VS_H225AliasAddress>   destExtraCallInfo;		// OPTIONAL
	Array_of_type<VS_H225CallReferenceValue>   destExtraCRV;	// OPTIONAL
	VS_AsnBoolean   activeMC;
	VS_H225ConferenceIdentifier   conferenceID;
	VS_CsSetupUuie_ConferenceGoal   conferenceGoal;
	VS_H225QseriesOptions   callServices;						// OPTIONAL
	VS_H225CallType   callType;
	VS_H225TransportAddress   sourceCallSignalAddress;			// EXTENSION,OPTIONAL
	VS_H225AliasAddress   remoteExtensionAddress;				// OPTIONAL
	VS_H225CallIdentifier   callIdentifier;
//	h245SecurityCapability	SEQUENCE OF H245Security OPTIONAL,
//	tokens			SEQUENCE OF ClearToken OPTIONAL,
//	cryptoTokens		SEQUENCE OF CryptoH323Token OPTIONAL,
//	fastStart		SEQUENCE OF OCTET STRING OPTIONAL,
//	mediaWaitForConnect	BOOLEAN,
//	canOverlapSend		BOOLEAN

	/////////////////////////////////////////////////////////////////////////////////////

	void operator=( const VS_CsSetupUuie &src )
	{
		O_CC(filled);	O_C(protocolIdentifier);	O_C(h245Address);	O_C(sourceAddress);
		O_C(sourceInfo);	O_C(destinationAddress);	O_C(destCallSignalAddress);
		O_C(destExtraCallInfo);		O_C(destExtraCRV);		O_C(activeMC);
		O_C(conferenceID);	O_C(conferenceGoal);	O_C(callServices);	O_C(callType);
		O_C(sourceCallSignalAddress);	O_C(remoteExtensionAddress);	O_C(callIdentifier);
//		O_C(h245SecurityCapability);	O_C(tokens);	O_C(cryptoTokens);
//		O_C(fastStart);		O_C(mediaWaitForConnect);	O_C(canOverlapSend);
	}
	// end of VS_CsSetupUuie::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsSetupUuie struct

/////////////////////////////////////////////////////////////////////////////////////////
*/

//////////////////////CLASS VS_H225Connect_UUIE /////////////////////////

struct VS_CsConnectUuie : public VS_AsnSequence
{
	 VS_CsConnectUuie( void );

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
	 VS_H225ArrayOf_ClearToken  tokens;
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
 	void operator=(const VS_CsConnectUuie& src);
	///////////////////////
	void operator=( const VS_H225ProtocolIdentifier &src )
	{	protocolIdentifier = src;	filled = true;	}
	// end of VS_CsConnectUuie::operator= VS_H225ProtocolIdentifier

	void operator=( const VS_H225TransportAddress &src )
	{	h245Address = src;	filled = true;	}
	// end of VS_CsConnectUuie::operator= VS_H225TransportAddress

	void operator=( const VS_H225EndpointType &src )
	{	destinationInfo = src;	filled = true;	}
	// end of VS_CsConnectUuie::operator= VS_H225EndpointType

	void operator=( const VS_H225ConferenceIdentifier &src )
	{	conferenceID = src;		filled = true;	}
	// end of VS_CsConnectUuie::operator= VS_H225ConferenceIdentifier

};



/*
struct VS_CsConnectUuie : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsConnectUuie( void )
		: VS_AsnSequence( 1, ref, basic_root, 0, 0, true )
	{
		ref[0].Set( &protocolIdentifier, 0 );
		ref[1].Set( &h245Address, 1 );
		ref[2].Set( &destinationInfo, 0 );
		ref[3].Set( &conferenceID, 0 );
	}
	// end of VS_CsConnectUuie constructor

	static const unsigned   basic_root = 4;
	VS_Reference_of_Asn   ref[basic_root];

	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225TransportAddress   h245Address;				// OPTIONAL
	VS_H225EndpointType   destinationInfo;
	VS_H225ConferenceIdentifier   conferenceID;

	/////////////////////////////////////////////////////////////////////////////////////

	void operator=( const VS_CsConnectUuie &src )
	{
		O_CC(filled);	O_C(protocolIdentifier);	O_C(h245Address);
		O_C(destinationInfo);	O_C(conferenceID);
	}
	// end of VS_CsConnectUuie::operator=

	void operator=( const VS_H225ProtocolIdentifier &src )
	{	protocolIdentifier = src;	filled = true;	}
	// end of VS_CsConnectUuie::operator= VS_H225ProtocolIdentifier

	void operator=( const VS_H225TransportAddress &src )
	{	h245Address = src;	filled = true;	}
	// end of VS_CsConnectUuie::operator= VS_H225TransportAddress

	void operator=( const VS_H225EndpointType &src )
	{	destinationInfo = src;	filled = true;	}
	// end of VS_CsConnectUuie::operator= VS_H225EndpointType

	void operator=( const VS_H225ConferenceIdentifier &src )
	{	conferenceID = src;		filled = true;	}
	// end of VS_CsConnectUuie::operator= VS_H225ConferenceIdentifier

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsConnectUuie struct
*/
/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////CLASS VS_H225CallProceeding_UUIE /////////////////////////

////////struct VS_H225CallProceeding_UUIE : public VS_AsnSequence
////////{
////////	 VS_H225CallProceeding_UUIE( void );
////////
////////	static const unsigned basic_root = 3;
////////	VS_Reference_of_Asn ref[basic_root];
////////	static const unsigned extension_root = 5;
////////	VS_Reference_of_Asn e_ref[extension_root];
////////
////////	 VS_H225ProtocolIdentifier  protocolIdentifier ;
//////// 	 VS_H225EndpointType  destinationInfo ;
//////// 	 VS_H225TransportAddress  h245Address ;
////////	 //ex
//////// 	 VS_H225CallIdentifier  callIdentifier ;
//////// 	 //VS_H225H245Security  h245SecurityMode ;
////////	 VS_AsnNull  h245SecurityMode ;
//////// 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
//////// 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
//////// 	Constrained_array_of_type< TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false> ,0,INT_MAX,VS_Asn::Unconstrained,0  >  fastStart ;
//////// 	void operator=(const VS_H225CallProceeding_UUIE& src);
////////	//bool operator==(const VS_H225CallProceeding_UUIE& src);
////////
////////};
/////////////////////////////////////////////////////////////////////////////////////

struct VS_CsH323MessageBody : public VS_AsnChoice
{
	/////////////////////////////////////////////////////////////////////////////////////

	enum {
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

	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsH323MessageBody( void )
		: VS_AsnChoice(7 , 13 , 1 ) {}
	// end of VS_CsH323MessageBody constructor

	/////////////////////////////////////////////////////////////////////////////////////

	bool Decode(VS_PerBuffer& buffer) override;
	// end VS_CsH323MessageBody::Decode

	/////////////////////////////////////////////////////////////////////////////////////
	void operator=(const VS_CsH323MessageBody & src);

	operator VS_CsSetupUuie*( void );
	// end VS_CsH323MessageBody::operator VS_CsSetupUuie *

	operator VS_CsConnectUuie*( void );
	// end VS_CsH323MessageBody::operator VS_CsConnectUuie *

	operator VS_CsAlertingUuie*( void );
	// end VS_CsH323MessageBody::operator VS_CsAlertingUuie *

	operator VS_CsReleaseCompleteUuie*( void );
	// end VS_CsH323MessageBody::operator VS_CsReleaseCompleteUuie*

	operator VS_H225StatusInquiry_UUIE*(void);
	// end VS_CsH323MessageBody::operator VS_H225StatusInquiry_UUIE*

	void operator=( VS_CsAlertingUuie *alertingUuie );
	// end VS_CsH323MessageBody::operator= VS_CsAlertingUuie *

	void operator=( VS_CsConnectUuie *connectUuie );
	// end VS_CsH323MessageBody::operator= VS_CsConnectUuie *

	void operator=( VS_CsSetupUuie *setupUuie );
	// end VS_CsH323MessageBody::operator= VS_CsSetupUuie *

	void operator=( VS_H225CallProceeding_UUIE *connectUuie );
	// end VS_CsH323MessageBody::operator= VS_H225CallProceeding_UUIE *

	void operator=(VS_H225Status_UUIE *statusUuie);
	// end VS_CsH323MessageBody::operator= VS_H225Status_UUIE *

	void operator=( VS_CsReleaseCompleteUuie *connectUuie );
	// end VS_CsH323MessageBody::operator= VS_CsReleaseCompleteUuie *

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_CsH323MessageBody struct


//////////////////////CLASS VS_H225H323_UU_PDU /////////////////////////

struct VS_CsH323UuPdu : public VS_AsnSequence
{
	 VS_CsH323UuPdu( void );

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 9;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_CsH323MessageBody	 h323MessageBody ;
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
 	void operator=(const VS_CsH323UuPdu& src);
	//bool operator==(const VS_H225H323_UU_PDU& src);

};
/////////////////////////////////////////////////////////////////////////////////////////
/*
struct VS_CsH323UuPdu : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsH323UuPdu( void )
		: VS_AsnSequence( 1, ref, basic_root, 0, 0, true )
	{
		ref[0].Set( &h323MessageBody, 0 );
		ref[1].Set( &nonStandardData, 1 );
	}
	// end of VS_CsH323UuPdu constructor

	static const unsigned   basic_root = 2;
	VS_Reference_of_Asn   ref[basic_root];

	VS_CsH323MessageBody   h323MessageBody;
	VS_H225NonStandardParameter   nonStandardData;	// OPTIONAL

	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_CsH323UuPdu &src )
	{
		O_CC(filled);	O_C(h323MessageBody);	O_C(nonStandardData);
	}
	// end of VS_CsH323UuPdu::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsH323UuPdu struct

/////////////////////////////////////////////////////////////////////////////////////////
*/
struct VS_CsUserData : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsUserData( void )
		: VS_AsnSequence( 1, ref, basic_root, 0, 0, true )
	{
		ref[0].Set( &protocolDiscriminator, 0 );
		ref[1].Set( &userInformation, 1 );
	}
	// end of VS_CsUserData constructor

	static const unsigned   basic_root = 2;
	VS_Reference_of_Asn   ref[basic_root];

	TemplInteger<0,255>   protocolDiscriminator;
	TemplOctetString<1,131>   userInformation;			// OPTIONAL

	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_CsUserData &src )
	{
		O_CC(filled);	O_C(protocolDiscriminator);		O_C(userInformation);
		O_CSA(ref, basic_root);
	}
	// end of VS_CsUserData::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsUserData struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_CsH323UserInformation : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_CsH323UserInformation( void )
		: VS_AsnSequence( 1, ref, basic_root, 0, 0, true )
	{
		ref[0].Set( &h323UuPdu	, 0 );
		ref[1].Set( &userData	, 1 );
	}
	// end of VS_CsH323UserInformation constructor
	static const unsigned   basic_root = 2;
	VS_Reference_of_Asn   ref[basic_root];

	VS_CsH323UuPdu   h323UuPdu;
	VS_CsUserData   userData;		// OPTIONAL

	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_CsH323UserInformation &src )
	{
		O_CC(filled);	O_C(h323UuPdu);		O_C(userData);
		O_CSA(ref, basic_root);
	}
	// end of VS_CsH323UserInformation::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_CsH323UserInformation struct

/////////////////////////////////////////////////////////////////////////////////////////
