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
/// \file VS_RasMessages.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VS_H225Messages.h"
typedef VS_H225PerCallInfoType VS_H225PerCallInfo;

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasGatekeeperRequest : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasGatekeeperRequest( void )
		: VS_AsnSequence(4,ref,basic_root,e_ref,10,true)
	{
		ref[0].Set( &requestSeqNum     , 0 );
		ref[1].Set( &protocolIdentifier, 0 );
		ref[2].Set( &nonStandardData   , 1 );
		ref[3].Set( &rasAddress        , 0 );
		ref[4].Set( &endpointType        , 0 );
		ref[5].Set( &gatekeeperIdentifier, 1 );
		ref[6].Set( &callServices        , 1 );
		ref[7].Set( &endpointAlias       , 1 );

		e_ref[0].Set(&alternateEndpoints,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&authenticationCapability,1);
		e_ref[4].Set(&algorithmOIDs,1);
		e_ref[5].Set(&integrity,1);
		e_ref[6].Set(&integrityCheckValue,1);
		e_ref[7].Set(&supportsAltGK,1);
		e_ref[8].Set(&featureSet,1);
		e_ref[9].Set(&genericData,1);
	}
	// end of VS_RasGatekeeperRequest constructor

	static const unsigned basic_root = 8;
	VS_Reference_of_Asn ref[basic_root];

	static const unsigned extension_root = 10;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum					requestSeqNum;
	VS_AsnObjectId							protocolIdentifier;
	VS_H225NonStandardParameter				nonStandardData;//opt
	VS_H225TransportAddress					rasAddress;
	VS_H225EndpointType						endpointType;
	TemplBitString<1,128>					gatekeeperIdentifier;//opt
	VS_H225QseriesOptions					callServices;//opt
	Array_of_type<VS_H225AliasAddress>		endpointAlias;//opt

	Array_of_type<VS_H225Endpoint>			alternateEndpoints;//opt ext
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type<  VS_H235AuthenticationMechanism ,0,INT_MAX,VS_Asn::Unconstrained,0  >  authenticationCapability ;
 	Constrained_array_of_type<  VS_AsnObjectId ,0,INT_MAX,VS_Asn::Unconstrained,0  >  algorithmOIDs ;
 	Constrained_array_of_type<  VS_H225IntegrityMechanism ,0,INT_MAX,VS_Asn::Unconstrained,0  >  integrity ;
 	 VS_H225Icv  integrityCheckValue ;
 	 VS_AsnNull  supportsAltGK ;
 	 VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;

	//static const unsigned extension_root = 0;
	//VS_Reference_of_Asn e_ref[extension_root];
/*
	VS_H225ArrayOf_Endpoint   alternateEndpoints;//ext
	VS_H225ArrayOf_ClearToken   tokens;									//made
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;						//made
	VS_H225ArrayOf_AuthenticationMechanism   authenticationCapability;
	VS_H225ArrayOf_PASN_ObjectId   algorithmOIDs;
	VS_H225ArrayOf_IntegrityMechanism   integrity;						//made
	VS_H225ICV   integrityCheckValue;
	VS_AsnNull   supportsAltGK;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/

	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasGatekeeperRequest &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(rasAddress);
		O_C(endpointType);
		O_C(gatekeeperIdentifier);
		O_C(callServices);
		O_C(endpointAlias);
		O_C(alternateEndpoints);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(authenticationCapability);
		O_C(algorithmOIDs);
		O_C(integrity);
		O_C(integrityCheckValue);
		O_C(supportsAltGK);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

		//		O_C(alternateEndpoints);	O_C(tokens);	O_C(cryptoTokens);
//		O_C(authenticationCapability);	O_C(algorithmOIDs);		O_C(integrity);
//		O_C(integrityCheckValue);	O_C(supportsAltGK);		O_C(featureSet);
//		O_C(genericData);
	}
	// end of VS_RasGatekeeperRequest::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_RasGatekeeperRequest struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasGatekeeperConfirm : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasGatekeeperConfirm( void )
		: VS_AsnSequence( 2 ,ref,basic_root,e_ref,9, true )
	{	ref[0].Set( &requestSeqNum			, 0 );
		ref[1].Set( &protocolIdentifier		, 0 );
		ref[2].Set( &nonStandardData		, 1 );
		ref[3].Set( &gatekeeperIdentifier   , 1 );
		ref[4].Set( &rasAddress				, 0 );

		e_ref[0].Set(&alternateGatekeeper,1);
		e_ref[1].Set(&authenticationMode,1);
		e_ref[2].Set(&tokens,1);
		e_ref[3].Set(&cryptoTokens,1);
		e_ref[4].Set(&algorithmOID,1);
		e_ref[5].Set(&integrity,1);
		e_ref[6].Set(&integrityCheckValue,1);
		e_ref[7].Set(&featureSet,1);
		e_ref[8].Set(&genericData,1);
	}
	// end VS_RasGatekeeperConfirm::VS_RasGatekeeperConfirm

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 9;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225NonStandardParameter   nonStandardData;		//o
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier; //o
	VS_H225TransportAddress   rasAddress;

//ext
	Constrained_array_of_type<  VS_H225AlternateGK ,0,INT_MAX,VS_Asn::Unconstrained,0  >  alternateGatekeeper ;
 	VS_H235AuthenticationMechanism  authenticationMode ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_AsnObjectId  algorithmOID ;
 	Constrained_array_of_type<  VS_H225IntegrityMechanism ,0,INT_MAX,VS_Asn::Unconstrained,0  >  integrity ;
 	VS_H225Icv  integrityCheckValue ;
 	VS_H225FeatureSet  featureSet ;
	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
/*
	VS_H225ArrayOf_AlternateGK   alternateGatekeeper;//x
	H235_AuthenticationMechanism   authenticationMode; //passible
	VS_H225ArrayOf_ClearToken   tokens;				   //passible
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_AsnObjectId   algorithmOID;
	VS_H225ArrayOf_IntegrityMechanism   integrity;
	VS_H225ICV   integrityCheckValue;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasGatekeeperConfirm &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(gatekeeperIdentifier);
		O_C(rasAddress);

		O_C(alternateGatekeeper);
		O_C(authenticationMode);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(algorithmOID);
		O_C(integrity);
		O_C(integrityCheckValue);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
		//		O_C(alternateGatekeeper);	O_C(authenticationMode);	O_C(tokens);
//		O_C(cryptoTokens);	O_C(algorithmOID);	O_C(integrity);		O_C(integrityCheckValue);
//		O_C(featureSet);	O_C(genericData);
	}
	// end of VS_RasGatekeeperConfirm::operator =

	/////////////////////////////////////////////////////////////////////////////////////

	void operator=( const VS_H225RequestSeqNum &rsn )
	{	requestSeqNum = rsn;	filled = true;		}
	// end VS_RasGatekeeperConfirm::operator=

	void operator=( const VS_H225ProtocolIdentifier &pi )
	{	protocolIdentifier = pi;	filled = true;	}
	// end VS_RasGatekeeperConfirm::operator=

	void operator=( const VS_H225TransportAddress &ta )
	{	rasAddress = ta;	filled = true;	}
	// end VS_RasGatekeeperConfirm::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasGatekeeperConfirm struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasGatekeeperReject : public VS_AsnSequence
{
	VS_RasGatekeeperReject( void )
		: VS_AsnSequence( 2 ,ref,basic_root,e_ref,6, true )
	{
		ref[0].Set( &requestSeqNum			, 0 );
		ref[1].Set( &protocolIdentifier		, 0 );
		ref[2].Set( &nonStandardData		, 1 );
		ref[3].Set( &gatekeeperIdentifier	, 1 );
		ref[4].Set( &rejectReason			, 0 );

		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&featureSet,1);
		e_ref[5].Set(&genericData,1);
	}
	// end VS_RasGatekeeperReject::VS_RasGatekeeperReject

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 6;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225NonStandardParameter   nonStandardData;//o
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier;//o
	VS_H225GatekeeperRejectReason   rejectReason;
	//ext
 	VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
 	VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
/*
	VS_H225AltGKInfo   altGKInfo;//x
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasGatekeeperReject &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(gatekeeperIdentifier);
		O_C(rejectReason);

		O_C(altGKInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
		//		O_C(altGKInfo);		O_C(tokens);	O_C(cryptoTokens);		O_C(integrityCheckValue);
//		O_C(featureSet);	O_C(genericData);
	}
	// end of VS_RasGatekeeperReject::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasGatekeeperReject struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasRegistrationRequest : public VS_AsnSequence
{
	  enum OptionalFields {
      e_nonStandardData,
      e_terminalAlias,
      e_gatekeeperIdentifier,
      e_alternateEndpoints,
      e_timeToLive,
      e_tokens,
      e_cryptoTokens,
      e_integrityCheckValue,
      e_keepAlive,
      e_endpointIdentifier,
      e_willSupplyUUIEs,
      e_maintainConnection,
      e_alternateTransportAddresses,
      e_additiveRegistration,
      e_terminalAliasPattern,
      e_supportsAltGK,
      e_usageReportingCapability,
      e_multipleCalls,
      e_supportedH248Packages,
      e_callCreditCapability,
      e_capacityReportingCapability,
      e_capacity,
      e_featureSet,
      e_genericData
    };
	VS_RasRegistrationRequest( void )
		: VS_AsnSequence( 3 ,ref,basic_root,e_ref,extension_root, true )
		{	ref[0].Set( &requestSeqNum			, 0 );
			ref[1].Set( &protocolIdentifier		, 0 );
			ref[2].Set( &nonStandardData		, 1 );
			ref[3].Set( &discoveryComplete		, 0 );
			ref[4].Set( &callSignalAddress		, 0 );
			ref[5].Set( &rasAddress				, 0 );
			ref[6].Set( &terminalType			, 0 );
			ref[7].Set( &terminalAlias			, 1 );
			ref[8].Set( &gatekeeperIdentifier	, 1 );
			ref[9].Set( &endpointVendor			, 0 );

			e_ref[0].Set(&alternateEndpoints,1);
			e_ref[1].Set(&timeToLive,1);
			e_ref[2].Set(&tokens,1);
			e_ref[3].Set(&cryptoTokens,1);
			e_ref[4].Set(&integrityCheckValue,1);
			e_ref[5].Set(&keepAlive,0);
			e_ref[6].Set(&endpointIdentifier,1);
			e_ref[7].Set(&willSupplyUUIEs,0);
			e_ref[8].Set(&maintainConnection,0);
			e_ref[9].Set(&alternateTransportAddresses,1);
			e_ref[10].Set(&additiveRegistration,1);
			e_ref[11].Set(&terminalAliasPattern,1);
			e_ref[12].Set(&supportsAltGK,1);
			e_ref[13].Set(&usageReportingCapability,1);
			e_ref[14].Set(&multipleCalls,1);
			e_ref[15].Set(&supportedH248Packages,1);
			e_ref[16].Set(&callCreditCapability,1);
			e_ref[17].Set(&capacityReportingCapability,1);
			e_ref[18].Set(&capacity,1);
			e_ref[19].Set(&featureSet,1);
			e_ref[20].Set(&genericData,1);
			e_ref[21].Set(&restart, 1);
			e_ref[22].Set(&supportsACFSequences, 1);
			e_ref[23].Set(&supportsAssignedGK, 0);

		}
	// end VS_RasRegistrationRequest::VS_RasRegistrationRequest
	static const unsigned basic_root = 10;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 24;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225ProtocolIdentifier		protocolIdentifier;
	VS_H225NonStandardParameter     nonStandardData;//o
	VS_AsnBoolean					discoveryComplete;
	Array_of_type<VS_H225TransportAddress>		callSignalAddress;
	Array_of_type<VS_H225TransportAddress>		rasAddress;
	VS_H225EndpointType				terminalType;
	Array_of_type<VS_H225AliasAddress>		terminalAlias;//o
	VS_H225GatekeeperIdentifier		gatekeeperIdentifier;//o
	VS_H225VendorIdentifier			 endpointVendor;

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
	VS_AsnNull  restart ;
	VS_AsnNull  supportsACFSequences ;
	VS_AsnBoolean  supportsAssignedGK ;

/*
	VS_H225ArrayOf_Endpoint   alternateEndpoints;//0
	VS_H225TimeToLive   timeToLive;				//1
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_AsnBoolean   keepAlive;
	VS_H225EndpointIdentifier   endpointIdentifier;
	VS_AsnBoolean   willSupplyUUIEs;
	VS_AsnBoolean   maintainConnection;
	VS_H225AlternateTransportAddresses   alternateTransportAddresses;
	VS_AsnNull   additiveRegistration;
	VS_H225ArrayOf_AddressPattern   terminalAliasPattern;
	VS_AsnNull   supportsAltGK;
	VS_H225RasUsageInfoTypes   usageReportingCapability;
	VS_AsnBoolean   multipleCalls;
	VS_H225ArrayOf_H248PackagesDescriptor   supportedH248Packages;
	VS_H225CallCreditCapability   callCreditCapability;
	VS_H225CapacityReportingCapability   capacityReportingCapability;
	VS_H225CallCapacity   capacity;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasRegistrationRequest &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
	    O_C(nonStandardData);//o
		O_C(discoveryComplete);
		O_C(callSignalAddress);
		O_C(rasAddress);
		O_C(terminalType);
		O_C(terminalAlias);//o
		O_C(gatekeeperIdentifier);//o
	    O_C(endpointVendor);

		O_C(alternateEndpoints);
		O_C(timeToLive);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(keepAlive);
		O_C(endpointIdentifier);
		O_C(willSupplyUUIEs);
		O_C(maintainConnection);
		O_C(alternateTransportAddresses);
		O_C(additiveRegistration);
		O_C(terminalAliasPattern);
		O_C(supportsAltGK);
		O_C(usageReportingCapability);
		O_C(multipleCalls);
		O_C(supportedH248Packages);
		O_C(callCreditCapability);
		O_C(capacityReportingCapability);
		O_C(capacity);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
/*
	   O_C(alternateEndpoints;//x	   O_C(timeToLive;				//1
	   O_C(tokens;	   O_C(cryptoTokens;	   O_C(integrityCheckValue;
	   O_C(keepAlive;	   O_C(endpointIdentifier;	   O_C(willSupplyUUIEs;
	   O_C(maintainConnection;	   O_C(alternateTransportAddresses;	   O_C(additiveRegistration;
	   O_C(terminalAliasPattern;	   O_C(supportsAltGK;	   O_C(usageReportingCapability;
	   O_C(multipleCalls;	   O_C(supportedH248Packages;	   O_C(callCreditCapability;
	   O_C(capacityReportingCapability;	   O_C(capacity;	   O_C(featureSet;
	   O_C(genericData);
*/

	}
	// end of ::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasRegistrationRequest struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasRegistrationConfirm : public VS_AsnSequence
{
	VS_RasRegistrationConfirm( void )
		: VS_AsnSequence ( 3 ,ref,basic_root,e_ref,extension_root, true )
	{	ref[0].Set( &requestSeqNum				, 0 );
		ref[1].Set( &protocolIdentifier			, 0 );
		ref[2].Set( &nonStandardData			, 1 );
		ref[3].Set( &callSignalAddress			, 0 );
		ref[4].Set( &terminalAlias				, 1 );
		ref[5].Set( &gatekeeperIdentifier		, 1 );
		ref[6].Set( &endpointIdentifier			, 0 );

		e_ref[0].Set(&alternateGatekeeper,1);
		e_ref[1].Set(&timeToLive,1);
		e_ref[2].Set(&tokens,1);
		e_ref[3].Set(&cryptoTokens,1);
		e_ref[4].Set(&integrityCheckValue,1);
		e_ref[5].Set(&willRespondToIRR,0);
		e_ref[6].Set(&preGrantedARQ,1);
		e_ref[7].Set(&maintainConnection,0);
		e_ref[8].Set(&serviceControl,1);
		e_ref[9].Set(&supportsAdditiveRegistration,1);
		e_ref[10].Set(&terminalAliasPattern,1);
		e_ref[11].Set(&supportedPrefixes,1);
		e_ref[12].Set(&usageSpec,1);
		e_ref[13].Set(&featureServerAlias,1);
		e_ref[14].Set(&capacityReportingSpec,1);
		e_ref[15].Set(&featureSet,1);
		e_ref[16].Set(&genericData,1);
	}
	//end VS_RasRegistrationConfirm::VS_RasRegistrationConfirm
	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 17;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225NonStandardParameter   nonStandardData;//o
	Array_of_type<VS_H225TransportAddress>  callSignalAddress;
	Array_of_type<VS_H225AliasAddress>		terminalAlias;//o
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier;//o
	VS_H225EndpointIdentifier   endpointIdentifier;

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

/*
	VS_H225ArrayOf_AlternateGK   alternateGatekeeper;//x
	VS_H225TimeToLive   timeToLive;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_AsnBoolean   willRespondToIRR;
	VS_H225RegistrationConfirm_preGrantedARQ   preGrantedARQ;
	VS_AsnBoolean   maintainConnection;
	VS_H225ArrayOf_ServiceControlSession   serviceControl;
	VS_AsnNull   supportsAdditiveRegistration;
	VS_H225ArrayOf_AddressPattern   terminalAliasPattern;
	VS_H225ArrayOf_SupportedPrefix   supportedPrefixes;
	VS_H225ArrayOf_RasUsageSpecification   usageSpec;
	VS_H225AliasAddress   featureServerAlias;
	VS_H225CapacityReportingSpecification   capacityReportingSpec;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////
	void operator =( const VS_RasRegistrationConfirm &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(callSignalAddress);
		O_C(terminalAlias);
		O_C(gatekeeperIdentifier);
		O_C(endpointIdentifier);

		O_C(alternateGatekeeper);
		O_C(timeToLive);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(willRespondToIRR);
		O_C(preGrantedARQ);
		O_C(maintainConnection);
		O_C(serviceControl);
		O_C(supportsAdditiveRegistration);
		O_C(terminalAliasPattern);
		O_C(supportedPrefixes);
		O_C(usageSpec);
		O_C(featureServerAlias);
		O_C(capacityReportingSpec);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

//		O_C(timeToLive);	O_C(integrityCheckValue);
//		O_C(alternateEndpoints);	O_C(tokens);	O_C(cryptoTokens);
//		O_C(willRespondToIRR);	O_C(preGrantedARQ);	O_C(maintainConnection);
//		O_C(serviceControl);		O_C(supportsAdditiveRegistration);		O_C(terminalAliasPattern);
//		O_C(supportedPrefixes);	O_C(usageSpec);		O_C(featureServerAlias);
//		O_C(capacityReportingSpec);	O_C(featureSet);		O_C(genericData);
	}
	// end of VS_RasRegistrationConfirm::operator =



	void operator=( const VS_H225RequestSeqNum &rsn )
	{	requestSeqNum = rsn;	filled = true;		}
	// end VS_RasRegistrationConfirm::operator=

	void operator=( const VS_H225ProtocolIdentifier &pi )
	{	protocolIdentifier = pi;	filled = true;	}
	// end VS_RasRegistrationConfirm::operator=

	void operator=( const Array_of_type<VS_H225TransportAddress> &ta )
	{	callSignalAddress = ta;		filled = true;	}
	// end VS_RasRegistrationConfirm::operator=

	void operator=( const Array_of_type<VS_H225AliasAddress>	 &aa )
	{	terminalAlias = aa;		filled = true;		}
	// end VS_RasRegistrationConfirm::operator=

	void operator=( const VS_H225EndpointIdentifier &ei )
	{	endpointIdentifier = ei;	filled = true;	}
	// end VS_RasRegistrationConfirm::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasRegistrationConfirm struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasRegistrationReject : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////
	VS_RasRegistrationReject( void )
		: VS_AsnSequence ( 2 ,ref,basic_root,e_ref , extension_root, true )
	{	ref[0].Set( &requestSeqNum			, 0 );
		ref[1].Set( &protocolIdentifier		, 0 );
		ref[2].Set( &nonStandardData		, 1 );
		ref[3].Set( &rejectReason			, 0 );
		ref[4].Set( &gatekeeperIdentifier	, 1 );

		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&featureSet,1);
		e_ref[5].Set(&genericData,1);
	}
	// end VS_RasRegistrationReject::VS_RasRegistrationReject
	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 6;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225NonStandardParameter   nonStandardData;//o
	VS_H225RegistrationRejectReason   rejectReason;
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier;//o

 	VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
 	VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
	/*
	VS_H225AltGKInfo   altGKInfo;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
	*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasRegistrationReject &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(rejectReason);
		O_C(gatekeeperIdentifier);

		O_C(altGKInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

//		O_C(altGKInfo);		O_C(tokens);	O_C(cryptoTokens);
//		O_C(integrityCheckValue);	O_C(featureSet);
//		O_C(genericData);
	}
	// end of VS_RasRegistrationReject::operator =

	void operator=( const VS_H225RequestSeqNum &rsn )
	{	requestSeqNum = rsn;	filled = true;		}
	// end VS_RasRegistrationReject::operator=

	void operator=( const VS_H225ProtocolIdentifier &pi )
	{	protocolIdentifier = pi;	filled = true;	}
	// end VS_RasRegistrationReject::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasRegistrationReject struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasUnregistrationRequest : public VS_AsnSequence
{
	VS_RasUnregistrationRequest( void )
		: VS_AsnSequence( 3 ,ref,basic_root, e_ref , extension_root , true )
	{	ref[0].Set( &requestSeqNum		, 0 );
		ref[1].Set( &callSignalAddress	, 0 );
		ref[2].Set( &endpointAlias		, 1 );
		ref[3].Set( &nonStandardData	, 1 );
		ref[4].Set( &endpointIdentifier	, 1 );

		e_ref[0].Set(&alternateEndpoints,1);
		e_ref[1].Set(&gatekeeperIdentifier,1);
		e_ref[2].Set(&tokens,1);
		e_ref[3].Set(&cryptoTokens,1);
		e_ref[4].Set(&integrityCheckValue,1);
		e_ref[5].Set(&reason,1);
		e_ref[6].Set(&endpointAliasPattern,1);
		e_ref[7].Set(&supportedPrefixes,1);
		e_ref[8].Set(&alternateGatekeeper,1);
		e_ref[9].Set(&genericData,1);
	}
	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 10;
	VS_Reference_of_Asn e_ref[extension_root];


	VS_H225RequestSeqNum  requestSeqNum;
	Array_of_type<VS_H225TransportAddress>  callSignalAddress;
	Array_of_type<VS_H225AliasAddress>  endpointAlias;//o
	VS_H225NonStandardParameter   nonStandardData;//o
	VS_H225EndpointIdentifier   endpointIdentifier;//o

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
/*
	VS_H225ArrayOf_Endpoint   alternateEndpoints;//x
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225UnregRequestReason   reason;
	VS_H225ArrayOf_AddressPattern   endpointAliasPattern;
	VS_H225ArrayOf_SupportedPrefix   supportedPrefixes;
	VS_H225ArrayOf_AlternateGK   alternateGatekeeper;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasUnregistrationRequest &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(callSignalAddress);
		O_C(endpointAlias);
		O_C(nonStandardData);
		O_C(endpointIdentifier);

		O_C(alternateEndpoints);
		O_C(gatekeeperIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(reason);
		O_C(endpointAliasPattern);
		O_C(supportedPrefixes);
		O_C(alternateGatekeeper);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
//		O_C(alternateEndpoints);	O_C(tokens);	O_C(cryptoTokens);
//		O_C(gatekeeperIdentifier);	O_C(endpointAliasPattern);		O_C(reason);
//		O_C(integrityCheckValue);	O_C(alternateGatekeeper);		O_C(featureSet);
//		O_C(genericData);		O_C(endpointAliasPattern);	O_C(supportedPrefixes);
	}
	// end of VS_RasUnregistrationRequest::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasUnregistrationRequest struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasUnregistrationConfirm : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasUnregistrationConfirm( void )
		: VS_AsnSequence( 1 ,ref,basic_root,e_ref , extension_root, true )
	{	ref[0].Set( &requestSeqNum		, 0 );
		ref[1].Set( &nonStandardData	, 1 );
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&genericData,1);
	}	// end VS_RasUnregistrationConfirm::VS_RasUnregistrationConfirm
	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum    requestSeqNum;
	VS_H225NonStandardParameter   nonStandardData;//o

 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
/*
	VS_H225ArrayOf_ClearToken   tokens;//x
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasUnregistrationConfirm &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);

		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
//		O_C(tokens);	O_C(cryptoTokens);
//		O_C(integrityCheckValue);
//		O_C(genericData);
	}
	// end of VS_RasUnregistrationConfirm::operator =

	void operator=( const VS_H225RequestSeqNum &rsn )
	{	requestSeqNum = rsn;	filled = true;	}
	// end VS_RasUnregistrationConfirm::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasUnregistrationConfirm struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasUnregistrationReject : public VS_AsnSequence
{
	VS_RasUnregistrationReject( void )
		: VS_AsnSequence( 1 ,ref,basic_root,e_ref , extension_root, true)
	{	ref[0].Set( &requestSeqNum		, 0 );
		ref[1].Set( &rejectReason		, 0 );
		ref[2].Set( &nonStandardData	, 1 );

		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&genericData,1);
	}
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225UnregRejectReason   rejectReason;
	VS_H225NonStandardParameter   nonStandardData;//o

 	VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];

	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];
/*
	VS_H225AltGKInfo   altGKInfo;//x
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasUnregistrationReject &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(rejectReason);
		O_C(nonStandardData);

		O_C(altGKInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

//		O_C(altGKInfo);	O_C(tokens);	O_C(cryptoTokens);
//		O_C(integrityCheckValue);
//		O_C(genericData);
	}
	// end of VS_RasUnregistrationReject::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasUnregistrationReject struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasAdmissionRequest : public VS_AsnSequence
{
	VS_RasAdmissionRequest( void )
		: VS_AsnSequence( 7 ,ref,basic_root,e_ref , extension_root, true)
	{	ref[0].Set( &requestSeqNum			, 0 );
		ref[1].Set( &callType				, 0 );
		ref[2].Set( &callModel				, 1 );
		ref[3].Set( &endpointIdentifier		, 0 );
		ref[4].Set( &destinationInfo		, 1 );
		ref[5].Set( &destCallSignalAddress	, 1 );
		ref[6].Set( &destExtraCallInfo		, 1 );
		ref[7].Set( &srcInfo				, 0 );
		ref[8].Set( &srcCallSignalAddress	, 1 );
		ref[9].Set( &bandWidth				, 0 );
		ref[10].Set( &callReferenceValue	, 0 );
		ref[11].Set( &nonStandardData		, 1 );
		ref[12].Set( &callServices			, 1 );
		ref[13].Set( &conferenceID			, 0 );
		ref[14].Set( &activeMC				, 0 );
		ref[15].Set( &answerCall			, 0 );

		e_ref[0].Set(&canMapAlias,0);
		e_ref[1].Set(&callIdentifier,0);
		e_ref[2].Set(&srcAlternatives,1);
		e_ref[3].Set(&destAlternatives,1);
		e_ref[4].Set(&gatekeeperIdentifier,1);
		e_ref[5].Set(&tokens,1);
		e_ref[6].Set(&cryptoTokens,1);
		e_ref[7].Set(&integrityCheckValue,1);
		e_ref[8].Set(&transportQOS,1);
		e_ref[9].Set(&willSupplyUUIEs,0);
		e_ref[10].Set(&callLinkage,1);
		e_ref[11].Set(&gatewayDataRate,1);
		e_ref[12].Set(&capacity,1);
		e_ref[13].Set(&circuitInfo,1);
		e_ref[14].Set(&desiredProtocols,1);
		e_ref[15].Set(&desiredTunnelledProtocol,1);
		e_ref[16].Set(&featureSet,1);
		e_ref[17].Set(&genericData,1);

	}
	static const unsigned basic_root = 16;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 18;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225CallType   callType;
	VS_H225CallModel   callModel;//o
	VS_H225EndpointIdentifier   endpointIdentifier;
	Array_of_type<VS_H225AliasAddress>   destinationInfo;//o
	VS_H225TransportAddress   destCallSignalAddress;//o
	Array_of_type<VS_H225AliasAddress>  destExtraCallInfo;//o
	Array_of_type<VS_H225AliasAddress>   srcInfo;
	VS_H225TransportAddress   srcCallSignalAddress;//o
	VS_H225BandWidth   bandWidth;
	VS_H225CallReferenceValue   callReferenceValue;
	VS_H225NonStandardParameter   nonStandardData;//o
	VS_H225QseriesOptions   callServices;//o
	VS_H225ConferenceIdentifier   conferenceID;
	VS_AsnBoolean   activeMC;
	VS_AsnBoolean   answerCall;

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

/*
	VS_AsnBoolean   canMapAlias;//x
	VS_H225CallIdentifier   callIdentifier;
	VS_H225ArrayOf_Endpoint   srcAlternatives;
	VS_H225ArrayOf_Endpoint   destAlternatives;
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225TransportQOS   transportQOS;
	VS_AsnBoolean   willSupplyUUIEs;
	VS_H225CallLinkage   callLinkage;
	VS_H225DataRate   gatewayDataRate;
	VS_H225CallCapacity   capacity;
	VS_H225CircuitInfo   circuitInfo;
	VS_H225ArrayOf_SupportedProtocols   desiredProtocols;
	VS_H225TunnelledProtocol   desiredTunnelledProtocol;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasAdmissionRequest &src )
	{
		O_CC(filled);
		O_C(requestSeqNum );
		O_C(callType );
		O_C( callModel );
		O_C( endpointIdentifier );
		O_C( destinationInfo );//o
		O_C( destCallSignalAddress );
		O_C(  destExtraCallInfo );
		O_C( srcInfo );
		O_C(srcCallSignalAddress );
		O_C( bandWidth );
		O_C( callReferenceValue );
		O_C( nonStandardData );
		O_C( callServices );
		O_C( conferenceID );
		O_C( activeMC );
		O_C( answerCall );

		O_C(canMapAlias);
		O_C(callIdentifier);
		O_C(srcAlternatives);
		O_C(destAlternatives);
		O_C(gatekeeperIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(transportQOS);
		O_C(willSupplyUUIEs);
		O_C(callLinkage);
		O_C(gatewayDataRate);
		O_C(capacity);
		O_C(circuitInfo);
		O_C(desiredProtocols);
		O_C(desiredTunnelledProtocol);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

	//		O_C(   canMapAlias );//x	O_C(   callIdentifier );
	//		O_C(   srcAlternatives );	O_C(   destAlternatives );
	//		O_C(   gatekeeperIdentifier );	O_C(   tokens );	O_C(   cryptoTokens );
	//		O_C(   integrityCheckValue );	O_C(   transportQOS );	O_C(   willSupplyUUIEs );
	//		O_C(   callLinkage );	O_C(   gatewayDataRate );	O_C(   capacity );
	//		O_C(   circuitInfo );	O_C(   desiredProtocols );	O_C(   desiredTunnelledProtocol );
	//		O_C(   featureSet );	O_C(   genericData );

	}
	// end of VS_RasAdmissionRequest::operator =


	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasAdmissionRequest struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasAdmissionConfirm : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasAdmissionConfirm( void )
		: VS_AsnSequence( 2 ,ref,basic_root,e_ref , extension_root, true )
	{	ref[0].Set(&requestSeqNum	,0);
		ref[1].Set(&bandWidth		,0);
		ref[2].Set(&callModel		,0);
		ref[3].Set(&destCallSignalAddress,0);
		ref[4].Set(&irrFrequency	,1);
		ref[5].Set(&nonStandardData	,1);

		e_ref[0].Set(&destinationInfo,1);
		e_ref[1].Set(&destExtraCallInfo,1);
		e_ref[2].Set(&destinationType,1);
		e_ref[3].Set(&remoteExtensionAddress,1);
		e_ref[4].Set(&alternateEndpoints,1);
		e_ref[5].Set(&tokens,1);
		e_ref[6].Set(&cryptoTokens,1);
		e_ref[7].Set(&integrityCheckValue,1);
		e_ref[8].Set(&transportQOS,1);
		e_ref[9].Set(&willRespondToIRR,0);
		e_ref[10].Set(&uuiesRequested,0);
		e_ref[11].Set(&language,1);
		e_ref[12].Set(&alternateTransportAddresses,1);
		e_ref[13].Set(&useSpecifiedTransport,1);
		e_ref[14].Set(&circuitInfo,1);
		e_ref[15].Set(&usageSpec,1);
		e_ref[16].Set(&supportedProtocols,1);
		e_ref[17].Set(&serviceControl,1);
		e_ref[18].Set(&multipleCalls,1);
		e_ref[19].Set(&featureSet,1);
		e_ref[20].Set(&genericData,1);

	}
	// end VS_RasAdmissionConfirm::VS_RasAdmissionConfirm
	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 21;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225BandWidth   bandWidth;
	VS_H225CallModel   callModel;
	VS_H225TransportAddress   destCallSignalAddress;
	TemplInteger<1 , 65535>   irrFrequency;//o
	VS_H225NonStandardParameter   nonStandardData;//o

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
/*
	Array_of_type<VS_H225AliasAddress>	destinationInfo;//x
	Array_of_type<VS_H225AliasAddress> destExtraCallInfo;
	VS_H225EndpointType   destinationType;
	Array_of_type<VS_H225AliasAddress>   remoteExtensionAddress;
	VS_H225ArrayOf_Endpoint   alternateEndpoints;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225TransportQOS   transportQOS;
	VS_AsnBoolean   willRespondToIRR;
	VS_H225UUIEsRequested   uuiesRequested;
	TemplIA5String<1,32> language;
	//VS_H225AdmissionConfirm_language   language;
	VS_H225AlternateTransportAddresses   alternateTransportAddresses;
	VS_H225UseSpecifiedTransport   useSpecifiedTransport;
	VS_H225CircuitInfo   circuitInfo;
	VS_H225ArrayOf_RasUsageSpecification   usageSpec;
	VS_H225ArrayOf_SupportedProtocols   supportedProtocols;
	VS_H225ArrayOf_ServiceControlSession   serviceControl;
	VS_AsnBoolean   multipleCalls;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasAdmissionConfirm &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(bandWidth);
		O_C(callModel);
		O_C(destCallSignalAddress);
		O_C(irrFrequency);
		O_C(nonStandardData);

		O_C(destinationInfo);
		O_C(destExtraCallInfo);
		O_C(destinationType);
		O_C(remoteExtensionAddress);
		O_C(alternateEndpoints);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(transportQOS);
		O_C(willRespondToIRR);
		O_C(uuiesRequested);
		O_C(language);
		O_C(alternateTransportAddresses);
		O_C(useSpecifiedTransport);
		O_C(circuitInfo);
		O_C(usageSpec);
		O_C(supportedProtocols);
		O_C(serviceControl);
		O_C(multipleCalls);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

//	O_C(	destinationInfo );	O_C( destExtraCallInfo ); 	O_C( destinationType );
//	O_C(   remoteExtensionAddress );	O_C( alternateEndpoints ); 	O_C(   tokens );
//	O_C(   cryptoTokens ); 	O_C(   integrityCheckValue ); 	O_C(   transportQOS );
//	O_C(   willRespondToIRR ); 	O_C(   uuiesRequested ); 	O_C(   language );
//	O_C(   alternateTransportAddresses ); 	O_C(   useSpecifiedTransport ); 	O_C(   circuitInfo );
//	O_C(   usageSpec ); 	O_C(   supportedProtocols ); 	O_C(   serviceControl );
//	O_C(   multipleCalls ); 	O_C(   featureSet ); 	O_C(   genericData );
	}
	// end of VS_RasAdmissionConfirm::operator =

	void operator=( const VS_H225RequestSeqNum &rsn )
	{	requestSeqNum = rsn;	filled = true;	}
	// end VS_RasAdmissionConfirm::operator=

	void operator=( const VS_H225BandWidth &bw )
	{	bandWidth = bw;		filled = true;		}
	// end VS_RasAdmissionConfirm::operator=

	void operator=( const VS_H225CallModel &cm )
	{	callModel = cm;		filled = true;		}
	// end VS_RasAdmissionConfirm::operator=

	void operator=( const VS_H225TransportAddress &ta )
	{	destCallSignalAddress = ta;		filled = true;		}
	// end VS_RasAdmissionConfirm::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasAdmissionConfirm struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasAdmissionReject : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasAdmissionReject( void )
		: VS_AsnSequence( 1 , ref,basic_root,e_ref , extension_root,true )
	{	ref[0].Set(&requestSeqNum		,0);
		ref[1].Set(&rejectReason		,0);
		ref[2].Set(&nonStandardData		,1);

		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&callSignalAddress,1);
		e_ref[4].Set(&integrityCheckValue,1);
		e_ref[5].Set(&serviceControl,1);
		e_ref[6].Set(&featureSet,1);
		e_ref[7].Set(&genericData,1);
	}
	//end VS_RasAdmissionReject::VS_RasAdmissionReject

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 8;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225AdmissionRejectReason   rejectReason;
	VS_H225NonStandardParameter   nonStandardData;//o

 	VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	Constrained_array_of_type<  VS_H225TransportAddress ,0,INT_MAX,VS_Asn::Unconstrained,0  >  callSignalAddress ;
 	VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;
 	VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
/*
	VS_H225AltGKInfo   altGKInfo;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ArrayOf_TransportAddress   callSignalAddress;
	VS_H225ICV   integrityCheckValue;
	VS_H225ArrayOf_ServiceControlSession   serviceControl;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasAdmissionReject &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(rejectReason);
		O_C(nonStandardData);

		O_C(altGKInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(callSignalAddress);
		O_C(integrityCheckValue);
		O_C(serviceControl);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

//	O_C(	altGKInfo );	O_C( tokens ); 	O_C( cryptoTokens );
//	O_C(   callSignalAddress );	O_C( integrityCheckValue ); 	O_C(   serviceControl );
//	O_C(   featureSet ); 	O_C(   genericData );
	}
	// end of VS_RasAdmissionReject::operator =


	void operator=( const VS_H225RequestSeqNum &rsn )
	{	requestSeqNum = rsn;	filled = true;	}
	// end VS_RasAdmissionReject::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasAdmissionReject struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasBandwidthRequest : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasBandwidthRequest( void )
		:VS_AsnSequence( 2 ,ref, basic_root,e_ref , extension_root,true )
	{	ref[0].Set(&requestSeqNum		,0);
		ref[1].Set(&endpointIdentifier	,0);
		ref[2].Set(&conferenceID		,0);
		ref[3].Set(&callReferenceValue	,0);

		ref[4].Set(&callType			,1);
		ref[5].Set(&bandWidth			,0);
		ref[6].Set(&nonStandardData		,1);

		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&gatekeeperIdentifier,1);
		e_ref[2].Set(&tokens,1);
		e_ref[3].Set(&cryptoTokens,1);
		e_ref[4].Set(&integrityCheckValue,1);
		e_ref[5].Set(&answeredCall,0);
		e_ref[6].Set(&callLinkage,1);
		e_ref[7].Set(&capacity,1);
		e_ref[8].Set(&usageInformation,1);
		e_ref[9].Set(&bandwidthDetails,1);
		e_ref[10].Set(&genericData,1);

	}
	// end VS_RasBandwidthRequest::VS_RasBandwidthRequest
	static const unsigned basic_root = 7;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 11;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum  requestSeqNum;
	VS_H225EndpointIdentifier   endpointIdentifier;
	VS_H225ConferenceIdentifier   conferenceID;
	VS_H225CallReferenceValue   callReferenceValue;

	VS_H225CallType   callType;					  //o
	VS_H225BandWidth   bandWidth;
	VS_H225NonStandardParameter   nonStandardData;//o

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
/*
	VS_H225CallIdentifier   callIdentifier;//x
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_AsnBoolean   answeredCall;
	VS_H225CallLinkage   callLinkage;
	VS_H225CallCapacity   capacity;
	VS_H225RasUsageInformation   usageInformation;
	VS_H225ArrayOf_BandwidthDetails   bandwidthDetails;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasBandwidthRequest &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(endpointIdentifier);
		O_C(conferenceID);
		O_C(callReferenceValue);
		O_C(callType);
		O_C(bandWidth);
		O_C(nonStandardData);

		O_C(callIdentifier);
		O_C(gatekeeperIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(answeredCall);
		O_C(callLinkage);
		O_C(capacity);
		O_C(usageInformation);
		O_C(bandwidthDetails);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   callIdentifier );	O_C(   gatekeeperIdentifier );	O_C(   tokens );
	O_C(   cryptoTokens );	O_C(   integrityCheckValue );	O_C(   answeredCall );
	O_C(   callLinkage );	O_C(   capacity );	O_C(   usageInformation );
	O_C(   bandwidthDetails );	O_C(   genericData );
*/
	}
	// end of VS_RasBandwidthRequest::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasBandwidthRequest struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasBandwidthConfirm : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	 VS_RasBandwidthConfirm( void )
		 :VS_AsnSequence( 1 ,ref, basic_root,e_ref , extension_root , true )
	{	ref[0].Set(&requestSeqNum		,0);
		ref[1].Set(&bandWidth			,0);
		ref[2].Set(&nonStandardData		,1);

		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&capacity,1);
		e_ref[4].Set(&genericData,1);
	}
	// end VS_RasBandwidthConfirm::VS_RasBandwidthConfirm

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225BandWidth   bandWidth;
	VS_H225NonStandardParameter   nonStandardData;//o

 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
 	VS_H225CallCapacity  capacity ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
/*
	VS_H225ArrayOf_ClearToken   tokens;//x
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225CallCapacity   capacity;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasBandwidthConfirm &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(bandWidth);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(capacity);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   tokens );
	O_C(   cryptoTokens );	O_C(   integrityCheckValue );	O_C(   capacity );
	O_C(   genericData );
*/
	}
	// end of VS_RasBandwidthConfirm::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasBandwidthConfirm struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasBandwidthReject : public VS_AsnSequence
{
	VS_RasBandwidthReject( void )
			:VS_AsnSequence( 1 ,ref, basic_root, e_ref , extension_root, true )
	{	ref[0].Set(&requestSeqNum		,0);
		ref[1].Set(&rejectReason		,0);
		ref[2].Set(&allowedBandWidth	,0);
		ref[3].Set(&nonStandardData		,1);

		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&genericData,1);
	}
	// end VS_RasBandwidthReject::VS_RasBandwidthReject
	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum  requestSeqNum;
	VS_H225BandRejectReason   rejectReason;
	VS_H225BandWidth   allowedBandWidth;
	VS_H225NonStandardParameter   nonStandardData;//o

 	VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
/*
	VS_H225AltGKInfo   altGKInfo;//x
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////


	void operator =( const VS_RasBandwidthReject &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(rejectReason);
		O_C(allowedBandWidth);
		O_C(nonStandardData);

		O_C(altGKInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

//		O_C(altGKInfo);	O_C(tokens);	O_C(cryptoTokens);
//		O_C(integrityCheckValue);	O_C(genericData);
	}
	// end of VS_RasBandwidthReject::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasBandwidthReject struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasDisengageRequest : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasDisengageRequest( void )
		: VS_AsnSequence( 1 ,ref, basic_root,e_ref , extension_root, true )
	{	ref[0].Set(&requestSeqNum		,0);
		ref[1].Set(&endpointIdentifier		,0);
		ref[2].Set(&conferenceID			,0);
		ref[3].Set(&callReferenceValue		,0);
		ref[4].Set(&disengageReason		,0);
		ref[5].Set(&nonStandardData		,1);
		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&gatekeeperIdentifier,1);
		e_ref[2].Set(&tokens,1);
		e_ref[3].Set(&cryptoTokens,1);
		e_ref[4].Set(&integrityCheckValue,1);
		e_ref[5].Set(&answeredCall,0);
		e_ref[6].Set(&callLinkage,1);
		e_ref[7].Set(&capacity,1);
		e_ref[8].Set(&circuitInfo,1);
		e_ref[9].Set(&usageInformation,1);
		e_ref[10].Set(&terminationCause,1);
		e_ref[11].Set(&serviceControl,1);
		e_ref[12].Set(&genericData,1);
	}
	// end VS_RasDisengageRequest::VS_RasDisengageRequest

	static const unsigned basic_root = 6;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 13;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225EndpointIdentifier   endpointIdentifier;
	VS_H225ConferenceIdentifier   conferenceID;
	VS_H225CallReferenceValue   callReferenceValue;
	VS_H225DisengageReason   disengageReason;
	VS_H225NonStandardParameter   nonStandardData;//o

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
/*
	VS_H225CallIdentifier   callIdentifier;//x
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_AsnBoolean   answeredCall;
	VS_H225CallLinkage   callLinkage;
	VS_H225CallCapacity   capacity;
	VS_H225CircuitInfo   circuitInfo;
	VS_H225RasUsageInformation   usageInformation;
	VS_H225CallTerminationCause   terminationCause;
	VS_H225ArrayOf_ServiceControlSession   serviceControl;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasDisengageRequest &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(endpointIdentifier);
		O_C(conferenceID);
		O_C(callReferenceValue);
		O_C(disengageReason);
		O_C(nonStandardData);
		O_C(callIdentifier);
		O_C(gatekeeperIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(answeredCall);
		O_C(callLinkage);
		O_C(capacity);
		O_C(circuitInfo);
		O_C(usageInformation);
		O_C(terminationCause);
		O_C(serviceControl);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   callIdentifier );	O_C(   gatekeeperIdentifier );	O_C(   tokens );
	O_C(   cryptoTokens );	O_C(   integrityCheckValue );	O_C(   answeredCall );
	O_C(   callLinkage );	O_C(   capacity );	O_C(   circuitInfo );
	O_C(   usageInformation );	O_C(   terminationCause );	O_C(   serviceControl );
	O_C(   genericData );
*/
	}
	// end of VS_RasDisengageRequest::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasDisengageRequest struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasDisengageConfirm : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasDisengageConfirm( void )
		: VS_AsnSequence( 1 ,ref, basic_root,e_ref , extension_root, true )
	{	ref[0].Set(&requestSeqNum	,0);
		ref[1].Set(&nonStandardData	,1);

		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&capacity,1);
		e_ref[4].Set(&circuitInfo,1);
		e_ref[5].Set(&usageInformation,1);
		e_ref[6].Set(&genericData,1);
	}
	// end VS_RasDisengageConfirm::VS_RasDisengageConfirm

	static const unsigned basic_root = 2;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 7;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225NonStandardParameter   nonStandardData;//o

 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
 	VS_H225CallCapacity  capacity ;
 	VS_H225CircuitInfo  circuitInfo ;
 	VS_H225RasUsageInformation  usageInformation ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
/*
	VS_H225ArrayOf_ClearToken   tokens;//x
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225CallCapacity   capacity;
	VS_H225CircuitInfo   circuitInfo;
	VS_H225RasUsageInformation   usageInformation;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasDisengageConfirm &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);

		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(capacity);
		O_C(circuitInfo);
		O_C(usageInformation);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   tokens );
	O_C(   cryptoTokens );	O_C(   integrityCheckValue );	O_C(   capacity );
	O_C(   circuitInfo );	O_C(   usageInformation );	O_C(   genericData );

*/
	}
	// end of VS_RasDisengageConfirm::operator =

	void operator=( const VS_H225RequestSeqNum &rsn )
	{	requestSeqNum = rsn;	filled = true;	}
	// end VS_RasDisengageConfirm::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasDisengageConfirm struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasDisengageReject : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasDisengageReject( void )
		: VS_AsnSequence( 1 ,ref, basic_root,e_ref , extension_root, true )
	{	ref[0].Set(&requestSeqNum	,0);
		ref[1].Set(&rejectReason	,0);
		ref[2].Set(&nonStandardData	,1);

		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&genericData,1);
	}
	// end VS_RasDisengageReject::VS_RasDisengageReject

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 5;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225DisengageRejectReason   rejectReason;
	VS_H225NonStandardParameter   nonStandardData;//o

 	VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
/*
	VS_H225AltGKInfo   altGKInfo;//x
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasDisengageReject &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);
		O_C(rejectReason);

		O_C(nonStandardData);
		O_C(altGKInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(altGKInfo);	O_C(   tokens );	O_C(   cryptoTokens );
	O_C(   integrityCheckValue );		O_C(   genericData );

*/
	}
	// end of VS_RasDisengageReject::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasDisengageReject struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasLocationRequest : public VS_AsnSequence
{
	VS_RasLocationRequest( void )
		: VS_AsnSequence( 2 ,ref, basic_root,e_ref , extension_root, true )
	{	ref[0].Set(&requestSeqNum	,0);
		ref[1].Set(&endpointIdentifier	,1);
		ref[2].Set(&destinationInfo	,0);
		ref[3].Set(&nonStandardData	,1);
		ref[4].Set(&replyAddress	,0);

		e_ref[0].Set(&sourceInfo,1);
		e_ref[1].Set(&canMapAlias,0);
		e_ref[2].Set(&gatekeeperIdentifier,1);
		e_ref[3].Set(&tokens,1);
		e_ref[4].Set(&cryptoTokens,1);
		e_ref[5].Set(&integrityCheckValue,1);
		e_ref[6].Set(&desiredProtocols,1);
		e_ref[7].Set(&desiredTunnelledProtocol,1);
		e_ref[8].Set(&featureSet,1);
		e_ref[9].Set(&genericData,1);
		e_ref[10].Set(&hopCount,1);
		e_ref[11].Set(&circuitInfo,1);
	}
	// end VS_RasLocationRequest::VS_RasLocationRequest

	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 12;
	VS_Reference_of_Asn e_ref[extension_root];


	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225EndpointIdentifier   endpointIdentifier;//o
	Array_of_type<VS_H225AliasAddress> destinationInfo;
	VS_H225NonStandardParameter   nonStandardData;//o
	VS_H225TransportAddress   replyAddress;

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
/*
	VS_H225ArrayOf_AliasAddress   sourceInfo;//x
	VS_AsnBoolean   canMapAlias;
	VS_H225GatekeeperIdentifier   gatekeeperIdentifier;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225ArrayOf_SupportedProtocols   desiredProtocols;
	VS_H225TunnelledProtocol   desiredTunnelledProtocol;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
	VS_AsnInteger   hopCount;
	VS_H225CircuitInfo   circuitInfo;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasLocationRequest &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(endpointIdentifier);
		O_C(nonStandardData);
		O_C(destinationInfo);
		O_C(replyAddress);

		O_C(sourceInfo);
		O_C(canMapAlias);
		O_C(gatekeeperIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(desiredProtocols);
		O_C(desiredTunnelledProtocol);
		O_C(featureSet);
		O_C(genericData);
		O_C(hopCount);
		O_C(circuitInfo);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   sourceInfo );	O_C(   canMapAlias );	O_C(   gatekeeperIdentifier );
	O_C(  tokens );		O_C(   cryptoTokens );	O_C(   integrityCheckValue );
	O_C(   desiredProtocols );	O_C(   desiredTunnelledProtocol );
	O_C(   featureSet );	O_C(   genericData );	O_C(   hopCount );
	O_C(   circuitInfo );
*/
	}
	// end of VS_RasLocationRequest::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasLocationRequest struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasLocationConfirm : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasLocationConfirm( void )
		: VS_AsnSequence( 1 ,ref, basic_root,e_ref , extension_root, true )
	{	ref[0].Set(&requestSeqNum		,0);
		ref[1].Set(&callSignalAddress	,0);
		ref[2].Set(&rasAddress			,0);
		ref[3].Set(&nonStandardData		,1);

		e_ref[0].Set(&destinationInfo,1);
		e_ref[1].Set(&destExtraCallInfo,1);
		e_ref[2].Set(&destinationType,1);
		e_ref[3].Set(&remoteExtensionAddress,1);
		e_ref[4].Set(&alternateEndpoints,1);
		e_ref[5].Set(&tokens,1);
		e_ref[6].Set(&cryptoTokens,1);
		e_ref[7].Set(&integrityCheckValue,1);
		e_ref[8].Set(&alternateTransportAddresses,1);
		e_ref[9].Set(&supportedProtocols,1);
		e_ref[10].Set(&multipleCalls,1);
		e_ref[11].Set(&featureSet,1);
		e_ref[12].Set(&genericData,1);
		e_ref[13].Set(&circuitInfo,1);
		e_ref[14].Set(&serviceControl,1);
	}
	// end VS_RasLocationConfirm::VS_RasLocationConfirm

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];

	static const unsigned extension_root = 15;
	VS_Reference_of_Asn e_ref[extension_root];


	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225TransportAddress   callSignalAddress;
	VS_H225TransportAddress   rasAddress;
	VS_H225NonStandardParameter   nonStandardData;//o

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
/*
	VS_H225ArrayOf_AliasAddress   destinationInfo;//x
	VS_H225ArrayOf_AliasAddress   destExtraCallInfo;
	VS_H225EndpointType   destinationType;
	VS_H225ArrayOf_AliasAddress   remoteExtensionAddress;
	VS_H225ArrayOf_Endpoint   alternateEndpoints;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225AlternateTransportAddresses   alternateTransportAddresses;
	VS_H225ArrayOf_SupportedProtocols   supportedProtocols;
	VS_AsnBoolean   multipleCalls;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
	VS_H225CircuitInfo   circuitInfo;
	VS_H225ArrayOf_ServiceControlSession   serviceControl;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasLocationConfirm &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(callSignalAddress);
		O_C(nonStandardData);
		O_C(rasAddress);

		O_C(destinationInfo);
		O_C(destExtraCallInfo);
		O_C(destinationType);
		O_C(remoteExtensionAddress);
		O_C(alternateEndpoints);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(alternateTransportAddresses);
		O_C(supportedProtocols);
		O_C(multipleCalls);
		O_C(featureSet);
		O_C(genericData);
		O_C(circuitInfo);
		O_C(serviceControl);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   destinationInfo ); 	O_C(   destExtraCallInfo );
	O_C(   destinationType ); 	O_C(   remoteExtensionAddress );
	O_C(   alternateEndpoints ); 	O_C(   tokens );
	O_C(   cryptoTokens ); 	O_C(   integrityCheckValue );
	O_C(   alternateTransportAddresses ); 	O_C(   supportedProtocols );
	O_C(   multipleCalls ); 	O_C(   featureSet );
	O_C(   genericData ); 	O_C(   circuitInfo );
	O_C(   serviceControl );
*/
	}
	// end of VS_RasLocationConfirm::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasLocationConfirm struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasLocationReject : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasLocationReject( void )
		: VS_AsnSequence( 1 ,ref, basic_root,e_ref , extension_root, true )
	{	ref[0].Set(&requestSeqNum		,0);
		ref[1].Set(&rejectReason		,0);
		ref[2].Set(&nonStandardData		,1);

		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&featureSet,1);
		e_ref[5].Set(&genericData,1);
		e_ref[6].Set(&serviceControl,1);
	}
	// end VS_RasLocationConfirm::VS_RasLocationConfirm
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225LocationRejectReason   rejectReason;
	VS_H225NonStandardParameter   nonStandardData;//o

 	VS_H225AltGKInfo  altGKInfo ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	VS_H225Icv  integrityCheckValue ;
	VS_H225FeatureSet  featureSet ;
 	Constrained_array_of_type<  VS_H225GenericData ,0,INT_MAX,VS_Asn::Unconstrained,0  >  genericData ;
 	Constrained_array_of_type<  VS_H225ServiceControlSession ,0,INT_MAX,VS_Asn::Unconstrained,0  >  serviceControl ;

	static const unsigned basic_root = 3;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 7;
	VS_Reference_of_Asn e_ref[extension_root];
/*
	VS_H225AltGKInfo   altGKInfo;//x
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
	VS_H225ArrayOf_ServiceControlSession   serviceControl;
*/
	/////////////////////////////////////////////////////////////////////////////////////


	void operator =( const VS_RasLocationReject &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(rejectReason);
		O_C(nonStandardData);

		O_C(altGKInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(featureSet);
		O_C(genericData);
		O_C(serviceControl);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

//		O_C(altGKInfo);	O_C(tokens);	O_C(cryptoTokens);
//		O_C(integrityCheckValue);	O_C(serviceControl);		O_C(featureSet);
//		O_C(genericData);
	}
	// end of VS_RasLocationReject::operator =


	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasLocationReject struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasInfoRequest : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasInfoRequest( void )
		:VS_AsnSequence( 2 , ref, basic_root,e_ref , extension_root,  true )
	{	ref[0].Set(&requestSeqNum		,0);
		ref[1].Set(&callReferenceValue	,0);
		ref[2].Set(&nonStandardData		,1);
		ref[3].Set(&replyAddress		,1);

		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&uuiesRequested,1);
		e_ref[5].Set(&callLinkage,1);
		e_ref[6].Set(&usageInfoRequested,1);
		e_ref[7].Set(&segmentedResponseSupported,1);
		e_ref[8].Set(&nextSegmentRequested,1);
		e_ref[9].Set(&capacityInfoRequested,1);
		e_ref[10].Set(&genericData,1);
	}
	// end VS_RasInfoRequest::VS_RasInfoRequest

	static const unsigned basic_root = 4;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 11;
	VS_Reference_of_Asn e_ref[extension_root];

	VS_H225RequestSeqNum			requestSeqNum;
	VS_H225CallReferenceValue		callReferenceValue;
	VS_H225NonStandardParameter		nonStandardData;//o
	VS_H225TransportAddress			replyAddress;//o

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
/*
	VS_H225CallIdentifier			callIdentifier;//x
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225UUIEsRequested   uuiesRequested;
	VS_H225CallLinkage   callLinkage;
	VS_H225RasUsageInfoTypes   usageInfoRequested;
	VS_AsnNull   segmentedResponseSupported;
	VS_AsnInteger   nextSegmentRequested;
	VS_AsnNull   capacityInfoRequested;
	VS_H225ArrayOf_GenericData   genericData;
	, nextSegmentRequested( VS_Asn::FixedConstraint,0,65535)
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasInfoRequest &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(callReferenceValue);
		O_C(nonStandardData);
		O_C(replyAddress);
		O_C(callIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(uuiesRequested);
		O_C(callLinkage);
		O_C(usageInfoRequested);
		O_C(segmentedResponseSupported);
		O_C(nextSegmentRequested);
		O_C(capacityInfoRequested);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(  callIdentifier ); O_C(   tokens ); O_C(   cryptoTokens );
	O_C(   integrityCheckValue ); O_C(   uuiesRequested ); 	O_C(   callLinkage );
	O_C(   usageInfoRequested ); 	O_C(   segmentedResponseSupported );
	O_C(   nextSegmentRequested ); 	O_C(   capacityInfoRequested);
	O_C(   genericData );

*/
	}
	// end of VS_RasInfoRequest::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasInfoRequest struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasInfoRequestResponse : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasInfoRequestResponse( void )
		:VS_AsnSequence( 3, ref, basic_root,e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&requestSeqNum,0);
		ref[2].Set(&endpointType,0);
		ref[3].Set(&endpointIdentifier,0);
		ref[4].Set(&rasAddress,0);
		ref[5].Set(&callSignalAddress,0);
		ref[6].Set(&endpointAlias,1);
		ref[7].Set(&perCallInfo,1);
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&needResponse,0);
		e_ref[4].Set(&capacity,1);
		e_ref[5].Set(&irrStatus,1);
		e_ref[6].Set(&unsolicited,0);
		e_ref[7].Set(&genericData,1);
	}
	// end VS_RasInfoRequestResponse::VS_RasInfoRequestResponse


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
//	VS_H225InfoRequestResponse_perCallInfo   perCallInfo;//o
/*
	VS_H225ArrayOf_ClearToken   tokens;//x
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_AsnBoolean   needResponse;
	VS_H225CallCapacity   capacity;
	VS_H225InfoRequestResponseStatus   irrStatus;
	VS_AsnBoolean   unsolicited;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasInfoRequestResponse &src )
	{
		O_CC(filled);
		O_C(nonStandardData);
		O_C(requestSeqNum);
		O_C(endpointType);
		O_C(endpointIdentifier);
		O_C(rasAddress);
		O_C(callSignalAddress);
		O_C(endpointAlias);
		O_C(perCallInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(needResponse);
		O_C(capacity);
		O_C(irrStatus);
		O_C(unsolicited);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   tokens );	O_C(   cryptoTokens ) ;	O_C(   integrityCheckValue );
	O_C(   needResponse ); 	O_C(   capacity ); 	O_C(   irrStatus );
	O_C(   unsolicited ); 	O_C(   genericData );
*/
	}
	// end of VS_RasInfoRequestResponse::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasInfoRequestResponse struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasNonStandardMessage : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasNonStandardMessage( void )
		:VS_AsnSequence( 0 , ref, basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,0);
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&featureSet,1);
		e_ref[4].Set(&genericData,1);
	}
	// end VS_RasNonStandardMessage::VS_RasNonStandardMessage

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
/*
	VS_H225ArrayOf_ClearToken   tokens;//x
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasNonStandardMessage &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   tokens );	O_C(   cryptoTokens ) ;	O_C(   integrityCheckValue );
	O_C(   featureSet ); 	 	O_C(   genericData );
*/
	}
	// end of VS_RasNonStandardMessage::operator =

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasNonStandardMessage struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_RasUnknownMessageResponse : public VS_AsnSequence
{
	/////////////////////////////////////////////////////////////////////////////////////////

	VS_RasUnknownMessageResponse( void )
		:VS_AsnSequence( (unsigned)0, ref,basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&messageNotUnderstood,0);
	}

	static const unsigned basic_root = 1;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 4;
	VS_Reference_of_Asn e_ref[extension_root];

	 VS_H225RequestSeqNum  requestSeqNum ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  messageNotUnderstood ;
/*
	VS_H225ArrayOf_ClearToken   tokens;//ex
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_AsnOctetString   messageNotUnderstood;
	//,messageNotUnderstood(VS_Asn::Unconstrained,0,65535)
*/
	/////////////////////////////////////////////////////////////////////////////////////

	void operator =( const VS_RasUnknownMessageResponse &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(messageNotUnderstood);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);

/*
	O_C(   tokens );	O_C(   cryptoTokens ) ;	O_C(   integrityCheckValue );
	O_C(   messageNotUnderstood );
*/
	}
	// end of VS_RasUnknownMessageResponse::operator =
	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasUnknownMessageResponse struct

/////////////////////////////////////////////////////////////////////////////////////////
struct VS_RasRequestInProgress : public VS_AsnSequence
{
	VS_RasRequestInProgress(void):
		VS_AsnSequence(4 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,1);
		ref[2].Set(&tokens,1);
		ref[3].Set(&cryptoTokens,1);
		ref[4].Set(&integrityCheckValue,1);
		ref[5].Set(&delay,0);
	}

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
	void operator =( const VS_RasRequestInProgress &src )
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(delay);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}
};/*
struct VS_RasRequestInProgress : public VS_AsnSequence
{
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225NonStandardParameter   nonStandardData;//o
	VS_H225ArrayOf_ClearToken   tokens;//o
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;//o
	VS_H225ICV   integrityCheckValue;//o
	VS_AsnInteger   delay;

	bool Decode(VS_PerBuffer& buffer) override
	{	return false;	}
	// end ::Decode
};
// end VS_RasRequestInProgress struct
*/
/////////////////////////////////////////////////////////////////////////////////////////
struct VS_RasResourcesAvailableIndicate : public VS_AsnSequence
{
	VS_RasResourcesAvailableIndicate( void ):
		VS_AsnSequence(4 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&endpointIdentifier,0);
		ref[4].Set(&protocols,0);
		ref[5].Set(&almostOutOfResources,0);
		ref[6].Set(&tokens,1);
		ref[7].Set(&cryptoTokens,1);
		ref[8].Set(&integrityCheckValue,1);
		e_ref[0].Set(&capacity,1);
		e_ref[1].Set(&genericData,1);
	}

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
 	void operator=(const VS_RasResourcesAvailableIndicate& src)
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(endpointIdentifier);
		O_C(protocols);
		O_C(almostOutOfResources);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(capacity);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}
};/*
struct VS_RasResourcesAvailableIndicate : public VS_AsnSequence
{
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225NonStandardParameter   nonStandardData;
	VS_H225EndpointIdentifier   endpointIdentifier;
	VS_H225ArrayOf_SupportedProtocols   protocols;
	VS_AsnBoolean   almostOutOfResources;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225CallCapacity   capacity;
	VS_H225ArrayOf_GenericData   genericData;

	bool Decode(VS_PerBuffer& buffer) override
	{	return false;	}
	// end ::Decode
};
// end VS_RasResourcesAvailableIndicate struct

/////////////////////////////////////////////////////////////////////////////////////////
*/struct VS_RasResourcesAvailableConfirm : public VS_AsnSequence
{
	VS_RasResourcesAvailableConfirm (void)
	:VS_AsnSequence(4 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&tokens,1);
		ref[4].Set(&cryptoTokens,1);
		ref[5].Set(&integrityCheckValue,1);
		e_ref[0].Set(&genericData,1);
	}
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
 	void operator=(const VS_RasResourcesAvailableConfirm& src)
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(genericData);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}
};/*
struct VS_RasResourcesAvailableConfirm : public VS_AsnSequence
{
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225ProtocolIdentifier   protocolIdentifier;
	VS_H225NonStandardParameter   nonStandardData;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225ArrayOf_GenericData   genericData;

	bool Decode(VS_PerBuffer& buffer) override
	{	return false;	}
	// end ::Decode
};
// end VS_RasResourcesAvailableConfirm struct

/////////////////////////////////////////////////////////////////////////////////////////
*/struct VS_RasInfoRequestAck : public VS_AsnSequence
{
	VS_RasInfoRequestAck( void )
	:VS_AsnSequence(4 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,1);
		ref[2].Set(&tokens,1);
		ref[3].Set(&cryptoTokens,1);
		ref[4].Set(&integrityCheckValue,1);
	}
	static const unsigned basic_root = 5;
	VS_Reference_of_Asn ref[basic_root];
	static const unsigned extension_root = 0;
	VS_Reference_of_Asn* e_ref = nullptr;

	 VS_H225RequestSeqNum  requestSeqNum ;
 	 VS_H225NonStandardParameter  nonStandardData ;
 	Constrained_array_of_type<  VS_H225ClearToken ,0,INT_MAX,VS_Asn::Unconstrained,0  >  tokens ;
 	Constrained_array_of_type<  VS_H225CryptoH323Token ,0,INT_MAX,VS_Asn::Unconstrained,0  >  cryptoTokens ;
 	 VS_H225Icv  integrityCheckValue ;
 	void operator=(const VS_RasInfoRequestAck& src)
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}
};/*
struct VS_RasInfoRequestAck : public VS_AsnSequence
{
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225NonStandardParameter   nonStandardData;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;

	bool Decode(VS_PerBuffer& buffer) override
	{	return false;	}
	// end ::Decode
};
// end VS_RasInfoRequestAck struct

/////////////////////////////////////////////////////////////////////////////////////////
*/struct VS_RasInfoRequestNak : public VS_AsnSequence
{
	VS_RasInfoRequestNak( void )
	:VS_AsnSequence(5 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,1);
		ref[2].Set(&nakReason,0);
		ref[3].Set(&altGKInfo,1);
		ref[4].Set(&tokens,1);
		ref[5].Set(&cryptoTokens,1);
		ref[6].Set(&integrityCheckValue,1);
	}
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
 	void operator=(const VS_RasInfoRequestNak& src)
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);
		O_C(nakReason);
		O_C(altGKInfo);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}
};/*
struct VS_RasInfoRequestNak : public VS_AsnSequence
{
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225NonStandardParameter   nonStandardData;
	VS_H225InfoRequestNakReason   nakReason;
	VS_H225AltGKInfo   altGKInfo;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;

	bool Decode(VS_PerBuffer& buffer) override
	{	return false;	}
	// end ::Decode
};
// end VS_RasInfoRequestNak struct

/////////////////////////////////////////////////////////////////////////////////////////
*/struct VS_RasServiceControlIndication : public VS_AsnSequence
{
	VS_RasServiceControlIndication(void)
	:VS_AsnSequence(8 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,1);
		ref[2].Set(&serviceControl,0);
		ref[3].Set(&endpointIdentifier,1);
		ref[4].Set(&callSpecific,1);
		ref[5].Set(&tokens,1);
		ref[6].Set(&cryptoTokens,1);
		ref[7].Set(&integrityCheckValue,1);
		ref[8].Set(&featureSet,1);
		ref[9].Set(&genericData,1);
	}
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
 	void operator=(const VS_RasServiceControlIndication& src)
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);
		O_C(serviceControl);
		O_C(endpointIdentifier);
		O_C(callSpecific);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(featureSet);
		O_C(genericData);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}
};/*
struct VS_RasServiceControlIndication : public VS_AsnSequence
{
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225NonStandardParameter   nonStandardData;
	VS_H225ArrayOf_ServiceControlSession   serviceControl;
	VS_H225EndpointIdentifier   endpointIdentifier;
	VS_H225ServiceControlIndication_callSpecific   callSpecific;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;

	bool Decode(VS_PerBuffer& buffer) override
	{	return false;	}
	// end ::Decode
};
// end VS_RasServiceControlIndication struct

/////////////////////////////////////////////////////////////////////////////////////////
*/struct VS_RasServiceControlResponse : public VS_AsnSequence
{
	VS_RasServiceControlResponse( void )
	:VS_AsnSequence(7 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&result,1);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&tokens,1);
		ref[4].Set(&cryptoTokens,1);
		ref[5].Set(&integrityCheckValue,1);
		ref[6].Set(&featureSet,1);
		ref[7].Set(&genericData,1);
	}
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
 	void operator=(const VS_RasServiceControlResponse& src)
	{
		O_CC(filled);
		O_C(requestSeqNum);
		O_C(result);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(featureSet);
		O_C(genericData);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}
};/*
struct VS_RasServiceControlResponse : public VS_AsnSequence
{
	VS_H225RequestSeqNum   requestSeqNum;
	VS_H225ServiceControlResponse_result   result;
	VS_H225NonStandardParameter   nonStandardData;
	VS_H225ArrayOf_ClearToken   tokens;
	VS_H225ArrayOf_CryptoH323Token   cryptoTokens;
	VS_H225ICV   integrityCheckValue;
	VS_H225FeatureSet   featureSet;
	VS_H225ArrayOf_GenericData   genericData;

	bool Decode(VS_PerBuffer& buffer) override
	{	return false;	}
	// end ::Decode
};
// end VS_RasServiceControlResponse struct

/////////////////////////////////////////////////////////////////////////////////////////
*/
struct VS_RasMessage : public VS_AsnChoice
{
	/////////////////////////////////////////////////////////////////////////////////////

	enum Choices
	{
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
		e_requestInProgress,//x
		e_resourcesAvailableIndicate,
		e_resourcesAvailableConfirm,
		e_infoRequestAck,
		e_infoRequestNak,
		e_serviceControlIndication,
		e_serviceControlResponse
	};
	// end Choices enum

	/////////////////////////////////////////////////////////////////////////////////////

	VS_RasMessage( void )
		: VS_AsnChoice( 25 , 32,true ) {}
	// end VS_RasMessage::VS_RasMessage

	/////////////////////////////////////////////////////////////////////////////////////

	bool Encode(VS_PerBuffer& buffer) override
	{
		if (!choice || tag > 32 || !filled )		return false;
		if (!buffer.ChoiceEncode( *this ))		return false;
		return choice->filled ? choice->Encode( buffer ) : false;
	}
	// end VS_RasMessage::Encode

	bool Decode(VS_PerBuffer& buffer) override
	{

		if (!buffer.ChoiceDecode( *this ))	return false;
		switch (tag)
		{
		case e_gatekeeperRequest :				return DecodeChoice( buffer , new VS_RasGatekeeperRequest);
		case e_gatekeeperConfirm :				return DecodeChoice( buffer , new VS_RasGatekeeperConfirm);
		case e_gatekeeperReject :				return DecodeChoice( buffer , new VS_RasGatekeeperReject);
		case e_registrationRequest :			return DecodeChoice( buffer , new VS_RasRegistrationRequest);
		case e_registrationConfirm :			return DecodeChoice( buffer , new VS_RasRegistrationConfirm);
		case e_registrationReject :				return DecodeChoice( buffer , new VS_RasRegistrationReject);
		case e_unregistrationRequest :			return DecodeChoice( buffer , new VS_RasUnregistrationRequest);
		case e_unregistrationConfirm :			return DecodeChoice( buffer , new VS_RasUnregistrationConfirm);
		case e_unregistrationReject :			return DecodeChoice( buffer , new VS_RasUnregistrationReject);
		case e_admissionRequest :				return DecodeChoice( buffer , new VS_RasAdmissionRequest);
		case e_admissionConfirm :				return DecodeChoice( buffer , new VS_RasAdmissionConfirm);
		case e_admissionReject :				return DecodeChoice( buffer , new VS_RasAdmissionReject);
		case e_bandwidthRequest :				return DecodeChoice( buffer , new VS_RasBandwidthRequest);
		case e_bandwidthConfirm :				return DecodeChoice( buffer , new VS_RasBandwidthConfirm);
		case e_bandwidthReject :				return DecodeChoice( buffer , new VS_RasBandwidthReject);
		case e_disengageRequest :				return DecodeChoice( buffer , new VS_RasDisengageRequest);
		case e_disengageConfirm :				return DecodeChoice( buffer , new VS_RasDisengageConfirm);
		case e_disengageReject :				return DecodeChoice( buffer , new VS_RasDisengageReject);
		case e_locationRequest :				return DecodeChoice( buffer , new VS_RasLocationRequest);
		case e_locationConfirm :				return DecodeChoice( buffer , new VS_RasLocationConfirm);
		case e_locationReject :				   return DecodeChoice( buffer , new VS_RasLocationReject);
		case e_infoRequest :					return DecodeChoice( buffer , new VS_RasInfoRequest);
		case e_infoRequestResponse :			return DecodeChoice( buffer , new VS_RasInfoRequestResponse);
		case e_nonStandardMessage :				return DecodeChoice( buffer , new VS_RasNonStandardMessage);
		case e_unknownMessageResponse :			return DecodeChoice( buffer , new VS_RasUnknownMessageResponse);
		case e_requestInProgress :
//			choice = new VS_RasRequestInProgress;
			return DecodeChoice( buffer, new VS_RasRequestInProgress);
		case e_resourcesAvailableIndicate :
//			choice = new VS_RasResourcesAvailableIndicate;
			return DecodeChoice(buffer, new VS_RasResourcesAvailableIndicate);
		case e_resourcesAvailableConfirm :
//			choice = new VS_RasResourcesAvailableConfirm;
			return DecodeChoice(buffer, new VS_RasResourcesAvailableConfirm);
		case e_infoRequestAck :
//			choice = new VS_RasInfoRequestAck;
			return DecodeChoice(buffer, new VS_RasInfoRequestAck);
		case e_infoRequestNak :
//			choice = new VS_RasInfoRequestNak;
			return DecodeChoice(buffer, new VS_RasInfoRequestNak);
		case e_serviceControlIndication :
//			choice = new VS_RasServiceControlIndication;
			return DecodeChoice( buffer, new VS_RasServiceControlIndication);
		case e_serviceControlResponse :
//			choice = new VS_RasServiceControlResponse;
			return DecodeChoice( buffer, new VS_RasServiceControlResponse);
		default :							return buffer.ChoiceMissExtensionObject(*this);
	}	}
	// end VS_RasMessage::Decode

	/////////////////////////////////////////////////////////////////////////////////////

	operator VS_RasGatekeeperRequest *( void );
	// end VS_RasMessage::operator VS_RasGatekeeperRequest *

	operator VS_RasGatekeeperConfirm *( void );
	// end VS_RasMessage::operator VS_RasGatekeeperConfirm *

	void operator=( VS_RasGatekeeperConfirm *gcf )
	{	Clear();	choice = gcf;	tag = e_gatekeeperConfirm;		filled = true;	}
	// end VS_RasMessage::operator=

	operator VS_RasGatekeeperReject *( void );
	// end VS_RasMessage::operator VS_RasGatekeeperReject *

	operator VS_RasRegistrationRequest *( void );
	// end VS_RasMessage::operator VS_RasRegistrationRequest *

	operator VS_RasRegistrationConfirm *( void );
	// end VS_RasMessage::operator VS_RasRegistrationConfirm *

	void operator=( VS_RasRegistrationConfirm *rcf )
	{	Clear();	choice = rcf;	tag = e_registrationConfirm;	filled = true;	}
	// end VS_RasMessage::operator=

	operator VS_RasRegistrationReject *( void );
	// end VS_RasMessage::operator VS_RasRegistrationReject *

	void operator=( VS_RasRegistrationReject *rrj )
	{	Clear();	choice = rrj;	tag = e_registrationReject;		filled = true;	}
	// end VS_RasMessage::operator=

	operator VS_RasUnregistrationRequest *( void );
	// end VS_RasMessage::operator VS_RasUnregistrationRequest *

	operator VS_RasUnregistrationConfirm *( void );
	// end VS_RasMessage::operator VS_RasUnregistrationConfirm *

	void operator=( VS_RasUnregistrationConfirm *rcf )
	{	Clear();	choice = rcf;	tag = e_unregistrationConfirm;	filled = true;	}
	// end VS_RasMessage::operator=

	void operator=( VS_RasUnregistrationReject *rej )
	{	Clear();	choice = rej;	tag = e_unregistrationReject;	filled = true;	}
	// end VS_RasMessage::operator=

	operator VS_RasUnregistrationReject *( void );
	// end VS_RasMessage::operator VS_RasUnregistrationReject *

	operator VS_RasAdmissionRequest *( void );
	// end VS_RasMessage::operator VS_RasAdmissionRequest *

	operator VS_RasAdmissionConfirm *( void );
	// end VS_RasMessage::operator VS_RasAdmissionConfirm *

	void operator=( VS_RasAdmissionConfirm *acf )
	{	Clear();	choice = acf;	tag = e_admissionConfirm;	filled = true;	}
	// end VS_RasMessage::operator=

	operator VS_RasAdmissionReject *( void );
	// end VS_RasMessage::operator VS_RasAdmissionReject *

	void operator=( VS_RasAdmissionReject *arj )
	{	Clear();	choice = arj;	tag = e_admissionReject;	filled = true;	}
	// end VS_RasMessage::operator=

	operator VS_RasBandwidthRequest *( void );
	// end VS_RasMessage::operator VS_RasBandwidthRequest *

	operator VS_RasBandwidthConfirm *( void );
	// end VS_RasMessage::operator VS_RasBandwidthConfirm *

	operator VS_RasBandwidthReject *( void );
	// end VS_RasMessage::operator VS_RasBandwidthReject *

	operator VS_RasDisengageRequest *( void );
	// end VS_RasMessage::operator VS_RasDisengageRequest *

	void operator=( VS_RasDisengageRequest *drq )
	{	Clear();	choice = drq;	tag = e_disengageRequest;	filled = true;	}
	// end VS_RasMessage::operator=

	operator VS_RasDisengageConfirm *( void );
	// end VS_RasMessage::operator VS_RasDisengageConfirm *

	void operator=( VS_RasDisengageConfirm *dcf )
	{	Clear();	choice = dcf;	tag = e_disengageConfirm;	filled = true;	}
	// end VS_RasMessage::operator=

	void operator=( VS_RasLocationRequest *lrq )
	{	Clear();	choice = lrq;	tag = e_locationRequest;	filled = true;	}
	// end VS_RasMessage::operator=

	void operator=( VS_RasLocationConfirm *lcf )
	{	Clear();	choice = lcf;	tag = e_locationConfirm;	filled = true;	}
	// end VS_RasMessage::operator=

	void operator=( VS_RasLocationReject *lrj )
	{	Clear();	choice = lrj;	tag = e_locationReject ;	filled = true;	}
	// end VS_RasMessage::operator=

    void operator=( VS_RasBandwidthReject *lrj )
    {	Clear();	choice = lrj;	tag = e_bandwidthReject ;	filled = true;	}
	// end VS_RasMessage::operator=

	void operator=( VS_RasBandwidthRequest *lrq )
    {	Clear();	choice = lrq;	tag = e_bandwidthRequest ;	filled = true;	}
	// end VS_RasMessage::operator=

	void operator=( VS_RasBandwidthConfirm *lrc )
    {	Clear();	choice = lrc;	tag = e_bandwidthConfirm ;	filled = true;	}
	// end VS_RasMessage::operator=

	operator VS_RasDisengageReject *( void );
	// end VS_RasMessage::operator VS_RasDisengageReject *

	operator VS_RasLocationRequest *( void );
	// end VS_RasMessage::operator VS_RasLocationRequest *

	operator VS_RasLocationConfirm *( void );
	// end VS_RasMessage::operator VS_RasLocationConfirm *

	operator VS_RasLocationReject *( void );
	// end VS_RasMessage::operator VS_RasLocationReject *

	operator VS_RasInfoRequest *( void );
	// end VS_RasMessage::operator VS_RasInfoRequest *

	operator VS_RasInfoRequestResponse *( void );
	// end VS_RasMessage::operator VS_RasInfoRequestResponse *

	operator VS_RasNonStandardMessage *( void );
	// end VS_RasMessage::operator VS_RasNonStandardMessage *

	operator VS_RasUnknownMessageResponse *( void );
	// end VS_RasMessage::operator VS_RasUnknownMessageResponse *

	operator VS_RasRequestInProgress *( void );
	// end VS_RasMessage::operator VS_RasRequestInProgress *

	operator VS_RasResourcesAvailableIndicate *( void );
	// end VS_RasMessage::operator VS_RasResourcesAvailableIndicate *

	operator VS_RasResourcesAvailableConfirm *( void );
	// end VS_RasMessage::operator VS_RasResourcesAvailableConfirm *

	operator VS_RasInfoRequestAck *( void );
	// end VS_RasMessage::operator VS_RasInfoRequestAck *

	operator VS_RasInfoRequestNak *( void );
	// end VS_RasMessage::operator VS_RasInfoRequestNak *

	operator VS_RasServiceControlIndication *( void );
	// end VS_RasMessage::operator VS_RasServiceControlIndication *

	operator VS_RasServiceControlResponse *( void );
	// end VS_RasMessage::operator VS_RasServiceControlResponse *

	void operator=(const VS_RasMessage &src )
	{
	//	FreeChoice();
	//	tag = src.tag;
		if (!src.filled) return;
		num_choices = src.num_choices;
		switch(src.tag)
		{
		case e_gatekeeperRequest :     CopyChoice<VS_RasGatekeeperRequest>(src,*this); return;
		case e_gatekeeperConfirm :	   CopyChoice<VS_RasGatekeeperConfirm>(src,*this); return;
		case e_gatekeeperReject :	   CopyChoice<VS_RasGatekeeperReject>(src,*this); return;
		case e_registrationRequest:	   CopyChoice<VS_RasRegistrationRequest>(src,*this); return;
		case e_registrationConfirm :	 CopyChoice<VS_RasRegistrationConfirm>(src,*this); return;
		case e_registrationReject :		 CopyChoice<VS_RasRegistrationReject>(src,*this); return;
		case e_unregistrationRequest :	CopyChoice<VS_RasUnregistrationRequest>(src,*this); return;
		case e_unregistrationConfirm :	CopyChoice<VS_RasUnregistrationConfirm>(src,*this); return;
		case e_unregistrationReject :	CopyChoice<VS_RasUnregistrationReject>(src,*this); return;
		case e_admissionRequest :		CopyChoice<VS_RasAdmissionRequest>(src,*this); return;
		case e_admissionConfirm :		CopyChoice<VS_RasAdmissionConfirm>(src,*this); return;
		case e_admissionReject :		CopyChoice<VS_RasAdmissionReject>(src,*this); return;
		case e_bandwidthRequest :		CopyChoice<VS_RasBandwidthRequest>(src,*this); return;
		case e_bandwidthConfirm :		CopyChoice<VS_RasBandwidthConfirm>(src,*this); return;
		case e_bandwidthReject :		CopyChoice<VS_RasBandwidthReject>(src,*this); return;
		case e_disengageRequest :		CopyChoice<VS_RasDisengageRequest>(src,*this); return;
		case e_disengageConfirm :		CopyChoice<VS_RasDisengageConfirm>(src,*this); return;
		case e_disengageReject :		CopyChoice<VS_RasDisengageReject>(src,*this); return;
		case e_locationRequest :		CopyChoice<VS_RasLocationRequest>(src,*this); return;
		case e_locationConfirm :		CopyChoice<VS_RasLocationConfirm>(src,*this); return;
		case e_locationReject :			CopyChoice<VS_RasDisengageReject>(src,*this); return;
		case e_infoRequest :			CopyChoice<VS_RasInfoRequest>(src,*this); return;
		case e_infoRequestResponse :	CopyChoice<VS_RasInfoRequestResponse>(src,*this); return;
		case e_nonStandardMessage :		CopyChoice<VS_RasNonStandardMessage>(src,*this); return;
		case e_unknownMessageResponse :	CopyChoice<VS_RasUnknownMessageResponse>(src,*this); return;

		}
	}
	// end VS_RasMessage::operator=

	/////////////////////////////////////////////////////////////////////////////////////
};
// end VS_RasMessage struct

/////////////////////////////////////////////////////////////////////////////////////////
