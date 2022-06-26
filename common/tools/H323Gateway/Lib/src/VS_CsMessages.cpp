/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 25.12.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_CsMessages.cpp
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#include "VS_CsMessages.h"

bool VS_CsH323MessageBody::Decode( VS_PerBuffer &buffer )
{
	FreeChoice();
	if (!buffer.ChoiceDecode( *this ))		return false;
	switch (tag)
	{
	case e_setup :				return DecodeChoice( buffer , new VS_CsSetupUuie );
	case e_callProceeding :		return DecodeChoice( buffer , new VS_H225CallProceeding_UUIE );
	case e_connect :			return DecodeChoice( buffer , new VS_CsConnectUuie );
	case e_alerting :			return DecodeChoice( buffer , new VS_CsAlertingUuie );
	case e_information :		return DecodeChoice( buffer , new VS_H225Information_UUIE);
	case e_releaseComplete :	return DecodeChoice( buffer , new VS_CsReleaseCompleteUuie );
	case e_facility :			return DecodeChoice( buffer , new VS_H225Facility_UUIE);
	case e_progress :			return DecodeChoice( buffer , new VS_H225Progress_UUIE);
	case e_empty :				return DecodeChoice( buffer , new VS_AsnNull);
	case e_status :				return DecodeChoice( buffer , new VS_H225Status_UUIE);
	case e_statusInquiry :		return DecodeChoice( buffer , new VS_H225StatusInquiry_UUIE);
	case e_setupAcknowledge :	return DecodeChoice( buffer , new VS_H225SetupAcknowledge_UUIE);
	case e_notify :				return DecodeChoice( buffer , new VS_H225Notify_UUIE);
	default:					return buffer.ChoiceMissExtensionObject(*this);
}	}
// end VS_CsH323MessageBody::Decode

void VS_CsH323MessageBody::operator=(const VS_CsH323MessageBody & src)
{
	FreeChoice();
	if (!src.filled) return;
	switch(src.tag)
	{
	case e_setup : CopyChoice< VS_CsSetupUuie >(src,*this); return;
	case e_callProceeding : CopyChoice< VS_H225CallProceeding_UUIE >(src,*this); return;
	case e_connect : CopyChoice< VS_CsConnectUuie >(src,*this); return;
	case e_alerting : CopyChoice< VS_CsAlertingUuie >(src,*this); return;
	case e_information : CopyChoice< VS_H225Information_UUIE >(src,*this); return;
	case e_releaseComplete : CopyChoice< VS_CsReleaseCompleteUuie >(src,*this); return;
	case e_facility : CopyChoice< VS_H225Facility_UUIE >(src,*this); return;
	case e_progress : CopyChoice< VS_H225Progress_UUIE >(src,*this); return;
	case e_empty : CopyChoice< VS_AsnNull   >(src,*this);  return;
	case e_status : CopyChoice< VS_H225Status_UUIE >(src,*this); return;
	case e_statusInquiry : CopyChoice< VS_H225StatusInquiry_UUIE >(src,*this); return;
	case e_setupAcknowledge : CopyChoice< VS_H225SetupAcknowledge_UUIE >(src,*this); return;
	case e_notify : CopyChoice< VS_H225Notify_UUIE >(src,*this); return;
	default:		return;
	}
}

VS_CsH323MessageBody::operator VS_CsSetupUuie*( void )
{	return dynamic_cast<VS_CsSetupUuie*>(choice);		}
// end VS_CsH323MessageBody::operator VS_CsSetupUuie*

VS_CsH323MessageBody::operator VS_CsConnectUuie*( void )
{	return dynamic_cast<VS_CsConnectUuie*>(choice);	}
// end VS_CsH323MessageBody::operator VS_CsConnectUuie*

VS_CsH323MessageBody::operator VS_CsAlertingUuie*( void )
{	return dynamic_cast<VS_CsAlertingUuie*>(choice);	}
// end VS_CsH323MessageBody::operator VS_CsAlertingUuie*

VS_CsH323MessageBody::operator VS_CsReleaseCompleteUuie*( void )
{	return dynamic_cast<VS_CsReleaseCompleteUuie*>(choice);	}
// end VS_CsH323MessageBody::operator VS_CsReleaseCompleteUuie*

VS_CsH323MessageBody::operator VS_H225StatusInquiry_UUIE*(void)
{	return dynamic_cast<VS_H225StatusInquiry_UUIE*>(choice);	}
// end VS_CsH323MessageBody::operator VS_H225StatusInquiry_UUIE*


void VS_CsH323MessageBody::operator=( VS_CsAlertingUuie *alertingUuie )
{
	FreeChoice();
	choice = alertingUuie;
	tag = e_alerting;
	filled = true;
}
// end VS_CsH323MessageBody::operator= VS_CsAlertingUuie*

void VS_CsH323MessageBody::operator=( VS_CsConnectUuie *connectUuie )
{
	FreeChoice();
	choice = connectUuie;
	tag = e_connect;
	filled = true;
}
// end VS_CsH323MessageBody::operator= VS_CsConnectUuie*

void VS_CsH323MessageBody::operator=( VS_CsSetupUuie *setupUuie )
{
	FreeChoice();
	choice = setupUuie;
	tag = e_setup;
	filled = true;
}
// end VS_CsH323MessageBody::operator= VS_CsConnectUuie*

void VS_CsH323MessageBody::operator=( VS_H225CallProceeding_UUIE *callProceeding )
{
	FreeChoice();
	choice = callProceeding;
	tag = e_callProceeding;
	filled = true;
}
// end VS_CsH323MessageBody::operator= VS_CsConnectUuie*

void VS_CsH323MessageBody::operator=(VS_H225Status_UUIE *statusUuie)
{
	FreeChoice();
	choice = statusUuie;
	tag = e_status;
	filled = true;
}
// end VS_CsH323MessageBody::operator= VS_H225Status_UUIE*

void VS_CsH323MessageBody::operator=( VS_CsReleaseCompleteUuie *releaseComplete )
{
	FreeChoice();
	choice = releaseComplete;
	tag = e_releaseComplete;
	filled = true;
}
// end VS_CsH323MessageBody::operator= VS_CsConnectUuie*



/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////CLASS VS_H225Setup_UUIE /////////////////////////
 	 VS_CsSetupUuie :: VS_CsSetupUuie( void )
	:VS_AsnSequence(7 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&h245Address,1);
		ref[2].Set(&sourceAddress,1);
		ref[3].Set(&sourceInfo,0);
		ref[4].Set(&destinationAddress,1);
		ref[5].Set(&destCallSignalAddress,1);
		ref[6].Set(&destExtraCallInfo,1);
		ref[7].Set(&destExtraCRV,1);
		ref[8].Set(&activeMC,0);
		ref[9].Set(&conferenceID,0);
		ref[10].Set(&conferenceGoal,0);
		ref[11].Set(&callServices,1);
		ref[12].Set(&callType,0);
		e_ref[0].Set(&sourceCallSignalAddress,1);
		e_ref[1].Set(&remoteExtensionAddress,1);
		e_ref[2].Set(&callIdentifier,0);
		e_ref[3].Set(&h245SecurityCapability,1);
		e_ref[4].Set(&tokens,1);
		e_ref[5].Set(&cryptoTokens,1);
		e_ref[6].Set(&fastStart,1);
		e_ref[7].Set(&mediaWaitForConnect,0);
		e_ref[8].Set(&canOverlapSend,0);
		e_ref[9].Set(&endpointIdentifier,1);
		e_ref[10].Set(&multipleCalls,0);
		e_ref[11].Set(&maintainConnection,0);
		e_ref[12].Set(&connectionParameters,1);
		e_ref[13].Set(&language,1);
		e_ref[14].Set(&presentationIndicator,1);
		e_ref[15].Set(&screeningIndicator,1);
		e_ref[16].Set(&serviceControl,1);
		e_ref[17].Set(&symmetricOperationRequired,1);
		e_ref[18].Set(&capacity,1);
		e_ref[19].Set(&circuitInfo,1);
		e_ref[20].Set(&desiredProtocols,1);
		e_ref[21].Set(&neededFeatures,1);
		e_ref[22].Set(&desiredFeatures,1);
		e_ref[23].Set(&supportedFeatures,1);
		e_ref[24].Set(&parallelH245Control,1);
		e_ref[25].Set(&additionalSourceAddresses,1);
	}
	void VS_CsSetupUuie::operator=(const VS_CsSetupUuie& src)
	{

		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(h245Address);
		O_C(sourceAddress);
		O_C(sourceInfo);
		O_C(destinationAddress);
		O_C(destCallSignalAddress);
		O_C(destExtraCallInfo);
		O_C(destExtraCRV);
		O_C(activeMC);
		O_C(conferenceID);
		O_C(conferenceGoal);
		O_C(callServices);
		O_C(callType);
		O_C(sourceCallSignalAddress);
		O_C(remoteExtensionAddress);
		O_C(callIdentifier);
		O_C(h245SecurityCapability);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(fastStart);
		O_C(mediaWaitForConnect);
		O_C(canOverlapSend);
		O_C(endpointIdentifier);
		O_C(multipleCalls);
		O_C(maintainConnection);
		O_C(connectionParameters);
		O_C(language);
		O_C(presentationIndicator);
		O_C(screeningIndicator);
		O_C(serviceControl);
		O_C(symmetricOperationRequired);
		O_C(capacity);
		O_C(circuitInfo);
		O_C(desiredProtocols);
		O_C(neededFeatures);
		O_C(desiredFeatures);
		O_C(supportedFeatures);
		O_C(parallelH245Control);
		O_C(additionalSourceAddresses);
		O_CSA(ref, basic_root);
		O_CSA(e_ref, extension_root);
	}
/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////CLASS VS_H225H323_UU_PDU /////////////////////////
 	 VS_CsH323UuPdu :: VS_CsH323UuPdu( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&h323MessageBody,0);
		ref[1].Set(&nonStandardData,1);
		e_ref[0].Set(&h4501SupplementaryService,1);
		e_ref[1].Set(&h245Tunneling,0);
		e_ref[2].Set(&h245Control,1);
		e_ref[3].Set(&nonStandardControl,1);
		e_ref[4].Set(&callLinkage,1);
		e_ref[5].Set(&tunnelledSignallingMessage,1);
		e_ref[6].Set(&provisionalRespToH245Tunneling,1);
		e_ref[7].Set(&stimulusControl,1);
		e_ref[8].Set(&genericData,1);
	}
	void VS_CsH323UuPdu::operator=(const VS_CsH323UuPdu& src)
	{

		O_CC(filled);
		O_C(h323MessageBody);
		O_C(nonStandardData);
		O_C(h4501SupplementaryService);
		O_C(h245Tunneling);
		O_C(h245Control);
		O_C(nonStandardControl);
		O_C(callLinkage);
		O_C(tunnelledSignallingMessage);
		O_C(provisionalRespToH245Tunneling);
		O_C(stimulusControl);
		O_C(genericData);
		O_CSA(ref, basic_root);
		O_CSA(e_ref, extension_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////
/*
	bool VS_H225H323_UU_PDU::operator==(const VS_H225H323_UU_PDU& src)
	{
		bool res = src.filled;
 		if (res&&
		 O_T(h323_message_body)&&
		 O_T(nonStandardData)&&
		 O_T(h4501SupplementaryService)&&
		 O_T(h245Tunneling)&&
		 O_T(h245Control)&&
		 O_T(nonStandardControl)
		) return true;
		 else return false;
	}
	*/
//////////////////////CLASS VS_H225CallProceeding_UUIE /////////////////////////
/* 	 VS_H225CallProceeding_UUIE :: VS_H225CallProceeding_UUIE( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		h245SecurityMode.filled = false;
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&destinationInfo,0);
		ref[2].Set(&h245Address,1);
		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&h245SecurityMode,1);
		e_ref[2].Set(&tokens,1);
		e_ref[3].Set(&cryptoTokens,1);
		e_ref[4].Set(&fastStart,1);
	}
	void VS_H225CallProceeding_UUIE::operator=(const VS_H225CallProceeding_UUIE& src)
	{

		O_C(protocolIdentifier);
		O_C(destinationInfo);
		O_C(h245Address);
		O_C(callIdentifier);
		O_C(h245SecurityMode);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(fastStart);
	}*/

 /////////////////////////////////////////////////////////////////////////////////////////
/*
	bool VS_H225CallProceeding_UUIE::operator==(const VS_H225CallProceeding_UUIE& src)
	{
		bool res = src.filled;
 		if (res&&
		 O_T(protocolIdentifier)&&
		 O_T(destinationInfo)&&
		 O_T(h245Address)&&
		 O_T(callIdentifier)&&
		 O_T(h245SecurityMode)&&
		 O_T(tokens)&&
		 O_T(cryptoTokens)&&
		 O_T(fastStart)
		) return true;
		 else return false;
	}
*/
//////////////////////CLASS VS_H225Connect_UUIE /////////////////////////
 	 VS_CsConnectUuie :: VS_CsConnectUuie( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&h245Address,1);
		ref[2].Set(&destinationInfo,0);
		ref[3].Set(&conferenceID,0);
		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&h245SecurityMode,1);
		e_ref[2].Set(&tokens,1);
		e_ref[3].Set(&cryptoTokens,1);
		e_ref[4].Set(&fastStart,1);
		e_ref[5].Set(&multipleCalls,0);
		e_ref[6].Set(&maintainConnection,0);
		e_ref[7].Set(&language,1);
		e_ref[8].Set(&connectedAddress,1);
		e_ref[9].Set(&presentationIndicator,1);
		e_ref[10].Set(&screeningIndicator,1);
		e_ref[11].Set(&fastConnectRefused,1);
		e_ref[12].Set(&serviceControl,1);
		e_ref[13].Set(&capacity,1);
		e_ref[14].Set(&featureSet,1);
	}
	void VS_CsConnectUuie::operator=(const VS_CsConnectUuie& src)
	{
		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(h245Address);
		O_C(destinationInfo);
		O_C(conferenceID);
		O_C(callIdentifier);
		O_C(h245SecurityMode);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(fastStart);
		O_C(multipleCalls);
		O_C(maintainConnection);
		O_C(language);
		O_C(connectedAddress);
		O_C(presentationIndicator);
		O_C(screeningIndicator);
		O_C(fastConnectRefused);
		O_C(serviceControl);
		O_C(capacity);
		O_C(featureSet);
		O_CSA(ref, basic_root);
		O_CSA(e_ref, extension_root);
	}
/////////////////////////////////////////////////////////////////////////////////
//////////////////////CLASS VS_H225Alerting_UUIE /////////////////////////
 	 VS_CsAlertingUuie :: VS_CsAlertingUuie( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&destinationInfo,0);
		ref[2].Set(&h245Address,1);
		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&h245SecurityMode,1);
		e_ref[2].Set(&tokens,1);
		e_ref[3].Set(&cryptoTokens,1);
		e_ref[4].Set(&fastStart,1);
		e_ref[5].Set(&multipleCalls,0);
		e_ref[6].Set(&maintainConnection,0);
		e_ref[7].Set(&alertingAddress,1);
		e_ref[8].Set(&presentationIndicator,1);
		e_ref[9].Set(&screeningIndicator,1);
		e_ref[10].Set(&fastConnectRefused,1);
		e_ref[11].Set(&serviceControl,1);
		e_ref[12].Set(&capacity,1);
		e_ref[13].Set(&featureSet,1);
	}
	void VS_CsAlertingUuie::operator=(const VS_CsAlertingUuie& src)
	{

		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(destinationInfo);
		O_C(h245Address);
		O_C(callIdentifier);
		O_C(h245SecurityMode);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(fastStart);
		O_C(multipleCalls);
		O_C(maintainConnection);
		O_C(alertingAddress);
		O_C(presentationIndicator);
		O_C(screeningIndicator);
		O_C(fastConnectRefused);
		O_C(serviceControl);
		O_C(capacity);
		O_C(featureSet);
		O_CSA(ref, basic_root);
		O_CSA(e_ref, extension_root);
	}
/////////////////////////////////////////////////////////////////////////////////
