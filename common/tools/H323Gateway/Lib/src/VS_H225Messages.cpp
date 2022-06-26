#include "VS_H225Messages.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER

//////////////////////CLASS VS_H225ServiceControlResponse_Result /////////////////////////
 	 VS_H225ServiceControlResponse_Result::VS_H225ServiceControlResponse_Result( void )
	:VS_AsnChoice(5 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225ServiceControlResponse_Result::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_started : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_failed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_stopped : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_notAvailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_neededFeatureNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225ServiceControlResponse_Result::operator=(const VS_H225ServiceControlResponse_Result & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_started : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_failed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_stopped : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_notAvailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_neededFeatureNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225ServiceControlResponse_Result::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225ServiceControlResponse_Result::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_started :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_failed :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_stopped :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_notAvailable :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_neededFeatureNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225ServiceControlResponse /////////////////////////
 	 VS_H225ServiceControlResponse :: VS_H225ServiceControlResponse( void )
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
	void VS_H225ServiceControlResponse::operator=(const VS_H225ServiceControlResponse& src)
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

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ServiceControlIndication_CallSpecific /////////////////////////
 	 VS_H225ServiceControlIndication_CallSpecific :: VS_H225ServiceControlIndication_CallSpecific( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&callIdentifier,0);
		ref[1].Set(&conferenceID,0);
		ref[2].Set(&answeredCall,0);
	}
	void VS_H225ServiceControlIndication_CallSpecific::operator=(const VS_H225ServiceControlIndication_CallSpecific& src)
	{

		O_CC(filled);
		O_C(callIdentifier);
		O_C(conferenceID);
		O_C(answeredCall);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ServiceControlIndication /////////////////////////
 	 VS_H225ServiceControlIndication :: VS_H225ServiceControlIndication( void )
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
	void VS_H225ServiceControlIndication::operator=(const VS_H225ServiceControlIndication& src)
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

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ResourcesAvailableConfirm /////////////////////////
 	 VS_H225ResourcesAvailableConfirm :: VS_H225ResourcesAvailableConfirm( void )
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
	void VS_H225ResourcesAvailableConfirm::operator=(const VS_H225ResourcesAvailableConfirm& src)
	{

		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(genericData);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ResourcesAvailableIndicate /////////////////////////
 	 VS_H225ResourcesAvailableIndicate :: VS_H225ResourcesAvailableIndicate( void )
	:VS_AsnSequence(4 , ref , basic_root, e_ref , extension_root , 1 )
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
	void VS_H225ResourcesAvailableIndicate::operator=(const VS_H225ResourcesAvailableIndicate& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RequestInProgress /////////////////////////
 	 VS_H225RequestInProgress :: VS_H225RequestInProgress( void )
	:VS_AsnSequence(4 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,1);
		ref[2].Set(&tokens,1);
		ref[3].Set(&cryptoTokens,1);
		ref[4].Set(&integrityCheckValue,1);
		ref[5].Set(&delay,0);
	}
	void VS_H225RequestInProgress::operator=(const VS_H225RequestInProgress& src)
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

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225UnknownMessageResponse /////////////////////////
 	 VS_H225UnknownMessageResponse :: VS_H225UnknownMessageResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&messageNotUnderstood,0);
	}
	void VS_H225UnknownMessageResponse::operator=(const VS_H225UnknownMessageResponse& src)
	{

		O_CC(filled);
		O_C(requestSeqNum);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(messageNotUnderstood);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225NonStandardMessage /////////////////////////
 	 VS_H225NonStandardMessage :: VS_H225NonStandardMessage( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,0);
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&featureSet,1);
		e_ref[4].Set(&genericData,1);
	}
	void VS_H225NonStandardMessage::operator=(const VS_H225NonStandardMessage& src)
	{

		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(featureSet);
		O_C(genericData);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////CLASS VS_H225InfoRequestNakReason /////////////////////////
 	 VS_H225InfoRequestNakReason::VS_H225InfoRequestNakReason( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225InfoRequestNakReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_notRegistered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225InfoRequestNakReason::operator=(const VS_H225InfoRequestNakReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_notRegistered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225InfoRequestNakReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225InfoRequestNakReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_notRegistered :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225InfoRequestNak /////////////////////////
 	 VS_H225InfoRequestNak :: VS_H225InfoRequestNak( void )
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
	void VS_H225InfoRequestNak::operator=(const VS_H225InfoRequestNak& src)
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

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225InfoRequestAck /////////////////////////
 	 VS_H225InfoRequestAck :: VS_H225InfoRequestAck( void )
	:VS_AsnSequence(4 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,1);
		ref[2].Set(&tokens,1);
		ref[3].Set(&cryptoTokens,1);
		ref[4].Set(&integrityCheckValue,1);
	}
	void VS_H225InfoRequestAck::operator=(const VS_H225InfoRequestAck& src)
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

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225InfoRequestResponseStatus /////////////////////////
 	 VS_H225InfoRequestResponseStatus::VS_H225InfoRequestResponseStatus( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225InfoRequestResponseStatus::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_complete : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_incomplete : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_segment : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		case e_invalidCall : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225InfoRequestResponseStatus::operator=(const VS_H225InfoRequestResponseStatus & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_complete : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_incomplete : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_segment : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_invalidCall : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225InfoRequestResponseStatus::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225InfoRequestResponseStatus::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_complete :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_incomplete :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_segment :  dprint4("\n\t choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_invalidCall :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225PduType /////////////////////////
 	 VS_H225PduType :: VS_H225PduType( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&h323pdu,0);
		ref[1].Set(&sent,0);
	}
	void VS_H225PduType::operator=(const VS_H225PduType& src)
	{

		O_CC(filled);
		O_C(h323pdu);
		O_C(sent);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225PerCallInfoType /////////////////////////
 	 VS_H225PerCallInfoType :: VS_H225PerCallInfoType( void )
	:VS_AsnSequence(5 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&callReferenceValue,0);
		ref[2].Set(&conferenceID,0);
		ref[3].Set(&originator,1);
		ref[4].Set(&audio,1);
		ref[5].Set(&video,1);
		ref[6].Set(&data,1);
		ref[7].Set(&h245,0);
		ref[8].Set(&callSignaling,0);
		ref[9].Set(&callType,0);
		ref[10].Set(&bandWidth,0);
		ref[11].Set(&callModel,0);
		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&substituteConfIDs,0);
		e_ref[4].Set(&pdu,1);
		e_ref[5].Set(&callLinkage,1);
		e_ref[6].Set(&usageInformation,1);
		e_ref[7].Set(&circuitInfo,1);
	}
	void VS_H225PerCallInfoType::operator=(const VS_H225PerCallInfoType& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(callReferenceValue);
		O_C(conferenceID);
		O_C(originator);
		O_C(audio);
		O_C(video);
		O_C(data);
		O_C(h245);
		O_C(callSignaling);
		O_C(callType);
		O_C(bandWidth);
		O_C(callModel);
		O_C(callIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(substituteConfIDs);
		O_C(pdu);
		O_C(callLinkage);
		O_C(usageInformation);
		O_C(circuitInfo);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225InfoRequestResponse /////////////////////////
 	 VS_H225InfoRequestResponse :: VS_H225InfoRequestResponse( void )
	:VS_AsnSequence(3 , ref , basic_root, e_ref , extension_root , 1 )
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
	void VS_H225InfoRequestResponse::operator=(const VS_H225InfoRequestResponse& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225InfoRequest /////////////////////////
 	 VS_H225InfoRequest :: VS_H225InfoRequest( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&callReferenceValue,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&replyAddress,1);
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
	void VS_H225InfoRequest::operator=(const VS_H225InfoRequest& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225DisengageRejectReason /////////////////////////
 	 VS_H225DisengageRejectReason::VS_H225DisengageRejectReason( void )
	:VS_AsnChoice(2 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225DisengageRejectReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_notRegistered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_requestToDropOther : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225DisengageRejectReason::operator=(const VS_H225DisengageRejectReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_notRegistered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_requestToDropOther : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225DisengageRejectReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225DisengageRejectReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_notRegistered :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_requestToDropOther :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////CLASS VS_H225DisengageReject /////////////////////////
 	 VS_H225DisengageReject :: VS_H225DisengageReject( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&rejectReason,0);
		ref[2].Set(&nonStandardData,1);
		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&genericData,1);
	}
	void VS_H225DisengageReject::operator=(const VS_H225DisengageReject& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225DisengageConfirm /////////////////////////
 	 VS_H225DisengageConfirm :: VS_H225DisengageConfirm( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,1);
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&capacity,1);
		e_ref[4].Set(&circuitInfo,1);
		e_ref[5].Set(&usageInformation,1);
		e_ref[6].Set(&genericData,1);
	}
	void VS_H225DisengageConfirm::operator=(const VS_H225DisengageConfirm& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225DisengageReason /////////////////////////
 	 VS_H225DisengageReason::VS_H225DisengageReason( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225DisengageReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_forcedDrop : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_normalDrop : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225DisengageReason::operator=(const VS_H225DisengageReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_forcedDrop : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_normalDrop : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225DisengageReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225DisengageReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_forcedDrop :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_normalDrop :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225DisengageRequest /////////////////////////
 	 VS_H225DisengageRequest :: VS_H225DisengageRequest( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&endpointIdentifier,0);
		ref[2].Set(&conferenceID,0);
		ref[3].Set(&callReferenceValue,0);
		ref[4].Set(&disengageReason,0);
		ref[5].Set(&nonStandardData,1);
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
	void VS_H225DisengageRequest::operator=(const VS_H225DisengageRequest& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225LocationRejectReason /////////////////////////
 	 VS_H225LocationRejectReason::VS_H225LocationRejectReason( void )
	:VS_AsnChoice(4 , 10 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225LocationRejectReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_notRegistered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidPermission : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_requestDenied : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_aliasesInconsistent : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_routeCalltoSCN : return DecodeChoice( buffer , new VS_H225PartyNumber);
		case e_resourceUnavailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_genericDataReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_neededFeatureNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225LocationRejectReason::operator=(const VS_H225LocationRejectReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_notRegistered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidPermission : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_requestDenied : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_aliasesInconsistent : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_routeCalltoSCN : CopyChoice< VS_H225PartyNumber >(src,*this); return;
		case e_resourceUnavailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_genericDataReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_neededFeatureNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225LocationRejectReason::operator VS_H225PartyNumber *( void )
	{	return dynamic_cast< VS_H225PartyNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225LocationRejectReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225LocationRejectReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_notRegistered :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidPermission :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_requestDenied :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_aliasesInconsistent :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_routeCalltoSCN :  dprint4("\n\t choice: VS_H225PartyNumber ");return;
		case e_resourceUnavailable :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_genericDataReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_neededFeatureNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225LocationReject /////////////////////////
 	 VS_H225LocationReject :: VS_H225LocationReject( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&rejectReason,0);
		ref[2].Set(&nonStandardData,1);
		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&featureSet,1);
		e_ref[5].Set(&genericData,1);
		e_ref[6].Set(&serviceControl,1);
	}
	void VS_H225LocationReject::operator=(const VS_H225LocationReject& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225LocationConfirm /////////////////////////
 	 VS_H225LocationConfirm :: VS_H225LocationConfirm( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&callSignalAddress,0);
		ref[2].Set(&rasAddress,0);
		ref[3].Set(&nonStandardData,1);
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
	void VS_H225LocationConfirm::operator=(const VS_H225LocationConfirm& src)
	{

		O_CC(filled);
		O_C(requestSeqNum);
		O_C(callSignalAddress);
		O_C(rasAddress);
		O_C(nonStandardData);
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225LocationRequest /////////////////////////
 	 VS_H225LocationRequest :: VS_H225LocationRequest( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&endpointIdentifier,1);
		ref[2].Set(&destinationInfo,0);
		ref[3].Set(&nonStandardData,1);
		ref[4].Set(&replyAddress,0);
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
	void VS_H225LocationRequest::operator=(const VS_H225LocationRequest& src)
	{

		O_CC(filled);
		O_C(requestSeqNum);
		O_C(endpointIdentifier);
		O_C(destinationInfo);
		O_C(nonStandardData);
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225BandRejectReason /////////////////////////
 	 VS_H225BandRejectReason::VS_H225BandRejectReason( void )
	:VS_AsnChoice(6 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225BandRejectReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_notBound : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidConferenceID : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidPermission : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_insufficientResources : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidRevision : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225BandRejectReason::operator=(const VS_H225BandRejectReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_notBound : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidConferenceID : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidPermission : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_insufficientResources : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidRevision : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225BandRejectReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225BandRejectReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_notBound :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidConferenceID :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidPermission :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_insufficientResources :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidRevision :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225BandwidthReject /////////////////////////
 	 VS_H225BandwidthReject :: VS_H225BandwidthReject( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&rejectReason,0);
		ref[2].Set(&allowedBandWidth,0);
		ref[3].Set(&nonStandardData,1);
		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&genericData,1);
	}
	void VS_H225BandwidthReject::operator=(const VS_H225BandwidthReject& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225BandwidthConfirm /////////////////////////
 	 VS_H225BandwidthConfirm :: VS_H225BandwidthConfirm( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&bandWidth,0);
		ref[2].Set(&nonStandardData,1);
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&capacity,1);
		e_ref[4].Set(&genericData,1);
	}
	void VS_H225BandwidthConfirm::operator=(const VS_H225BandwidthConfirm& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225BandwidthRequest /////////////////////////
 	 VS_H225BandwidthRequest :: VS_H225BandwidthRequest( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&endpointIdentifier,0);
		ref[2].Set(&conferenceID,0);
		ref[3].Set(&callReferenceValue,0);
		ref[4].Set(&callType,1);
		ref[5].Set(&bandWidth,0);
		ref[6].Set(&nonStandardData,1);
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
	void VS_H225BandwidthRequest::operator=(const VS_H225BandwidthRequest& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225AdmissionRejectReason /////////////////////////
 	 VS_H225AdmissionRejectReason::VS_H225AdmissionRejectReason( void )
	:VS_AsnChoice(8 , 18 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225AdmissionRejectReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_calledPartyNotRegistered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidPermission : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_requestDenied : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_callerNotRegistered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_routeCallToGatekeeper : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidEndpointIdentifier : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_resourceUnavailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_qosControlNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_incompleteAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_aliasesInconsistent : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_routeCallToSCN : return DecodeChoice( buffer , new VS_H225PartyNumber);
		case e_exceedsCallCapacity : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_collectDestination : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_collectPIN : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_genericDataReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_neededFeatureNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225AdmissionRejectReason::operator=(const VS_H225AdmissionRejectReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_calledPartyNotRegistered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidPermission : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_requestDenied : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_callerNotRegistered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_routeCallToGatekeeper : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidEndpointIdentifier : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_resourceUnavailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_qosControlNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_incompleteAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_aliasesInconsistent : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_routeCallToSCN : CopyChoice< VS_H225PartyNumber >(src,*this); return;
		case e_exceedsCallCapacity : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_collectDestination : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_collectPIN : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_genericDataReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_neededFeatureNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225AdmissionRejectReason::operator VS_H225PartyNumber *( void )
	{	return dynamic_cast< VS_H225PartyNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225AdmissionRejectReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225AdmissionRejectReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_calledPartyNotRegistered :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidPermission :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_requestDenied :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_callerNotRegistered :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_routeCallToGatekeeper :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidEndpointIdentifier :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_resourceUnavailable :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_qosControlNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_incompleteAddress :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_aliasesInconsistent :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_routeCallToSCN :  dprint4("\n\t choice: VS_H225PartyNumber ");return;
		case e_exceedsCallCapacity :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_collectDestination :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_collectPIN :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_genericDataReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_neededFeatureNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225AdmissionReject /////////////////////////
 	 VS_H225AdmissionReject :: VS_H225AdmissionReject( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&rejectReason,0);
		ref[2].Set(&nonStandardData,1);
		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&callSignalAddress,1);
		e_ref[4].Set(&integrityCheckValue,1);
		e_ref[5].Set(&serviceControl,1);
		e_ref[6].Set(&featureSet,1);
		e_ref[7].Set(&genericData,1);
	}
	void VS_H225AdmissionReject::operator=(const VS_H225AdmissionReject& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225UUIEsRequested /////////////////////////
 	 VS_H225UUIEsRequested :: VS_H225UUIEsRequested( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&setup,0);
		ref[1].Set(&callProceeding,0);
		ref[2].Set(&connect,0);
		ref[3].Set(&alerting,0);
		ref[4].Set(&information,0);
		ref[5].Set(&releaseComplete,0);
		ref[6].Set(&facility,0);
		ref[7].Set(&progress,0);
		ref[8].Set(&empty,0);
		e_ref[0].Set(&status,0);
		e_ref[1].Set(&statusInquiry,0);
		e_ref[2].Set(&setupAcknowledge,0);
		e_ref[3].Set(&notify,0);
	}
	void VS_H225UUIEsRequested::operator=(const VS_H225UUIEsRequested& src)
	{

		O_CC(filled);
		O_C(setup);
		O_C(callProceeding);
		O_C(connect);
		O_C(alerting);
		O_C(information);
		O_C(releaseComplete);
		O_C(facility);
		O_C(progress);
		O_C(empty);
		O_C(status);
		O_C(statusInquiry);
		O_C(setupAcknowledge);
		O_C(notify);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225AdmissionConfirm /////////////////////////
 	 VS_H225AdmissionConfirm :: VS_H225AdmissionConfirm( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&bandWidth,0);
		ref[2].Set(&callModel,0);
		ref[3].Set(&destCallSignalAddress,0);
		ref[4].Set(&irrFrequency,1);
		ref[5].Set(&nonStandardData,1);
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
	void VS_H225AdmissionConfirm::operator=(const VS_H225AdmissionConfirm& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TransportQOS /////////////////////////
 	 VS_H225TransportQOS::VS_H225TransportQOS( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225TransportQOS::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_endpointControlled : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_gatekeeperControlled : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_noControl : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225TransportQOS::operator=(const VS_H225TransportQOS & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_endpointControlled : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_gatekeeperControlled : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_noControl : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225TransportQOS::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225TransportQOS::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_endpointControlled :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_gatekeeperControlled :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_noControl :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225CallModel /////////////////////////
 	 VS_H225CallModel::VS_H225CallModel( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225CallModel::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_direct : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_gatekeeperRouted : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225CallModel::operator=(const VS_H225CallModel & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_direct : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_gatekeeperRouted : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225CallModel::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225CallModel::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_direct :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_gatekeeperRouted :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225CallType /////////////////////////
 	 VS_H225CallType::VS_H225CallType( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225CallType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_pointToPoint : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_oneToN : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_nToOne : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_nToN : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225CallType::operator=(const VS_H225CallType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_pointToPoint : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_oneToN : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_nToOne : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_nToN : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225CallType::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225CallType::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_pointToPoint :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_oneToN :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_nToOne :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_nToN :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225AdmissionRequest /////////////////////////
 	 VS_H225AdmissionRequest :: VS_H225AdmissionRequest( void )
	:VS_AsnSequence(7 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&callType,0);
		ref[2].Set(&callModel,1);
		ref[3].Set(&endpointIdentifier,0);
		ref[4].Set(&destinationInfo,1);
		ref[5].Set(&destCallSignalAddress,1);
		ref[6].Set(&destExtraCallInfo,1);
		ref[7].Set(&srcInfo,0);
		ref[8].Set(&srcCallSignalAddress,1);
		ref[9].Set(&bandWidth,0);
		ref[10].Set(&callReferenceValue,0);
		ref[11].Set(&nonStandardData,1);
		ref[12].Set(&callServices,1);
		ref[13].Set(&conferenceID,0);
		ref[14].Set(&activeMC,0);
		ref[15].Set(&answerCall,0);
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
	void VS_H225AdmissionRequest::operator=(const VS_H225AdmissionRequest& src)
	{

		O_CC(filled);
		O_C(requestSeqNum);
		O_C(callType);
		O_C(callModel);
		O_C(endpointIdentifier);
		O_C(destinationInfo);
		O_C(destCallSignalAddress);
		O_C(destExtraCallInfo);
		O_C(srcInfo);
		O_C(srcCallSignalAddress);
		O_C(bandWidth);
		O_C(callReferenceValue);
		O_C(nonStandardData);
		O_C(callServices);
		O_C(conferenceID);
		O_C(activeMC);
		O_C(answerCall);
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225UnregRejectReason /////////////////////////
 	 VS_H225UnregRejectReason::VS_H225UnregRejectReason( void )
	:VS_AsnChoice(3 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225UnregRejectReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_notCurrentlyRegistered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_callInProgress : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_permissionDenied : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225UnregRejectReason::operator=(const VS_H225UnregRejectReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_notCurrentlyRegistered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_callInProgress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_permissionDenied : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225UnregRejectReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225UnregRejectReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_notCurrentlyRegistered :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_callInProgress :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_permissionDenied :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225UnregistrationReject /////////////////////////
 	 VS_H225UnregistrationReject :: VS_H225UnregistrationReject( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&rejectReason,0);
		ref[2].Set(&nonStandardData,1);
		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&genericData,1);
	}
	void VS_H225UnregistrationReject::operator=(const VS_H225UnregistrationReject& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225UnregistrationConfirm /////////////////////////
 	 VS_H225UnregistrationConfirm :: VS_H225UnregistrationConfirm( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&nonStandardData,1);
		e_ref[0].Set(&tokens,1);
		e_ref[1].Set(&cryptoTokens,1);
		e_ref[2].Set(&integrityCheckValue,1);
		e_ref[3].Set(&genericData,1);
	}
	void VS_H225UnregistrationConfirm::operator=(const VS_H225UnregistrationConfirm& src)
	{

		O_CC(filled);
		O_C(requestSeqNum);
		O_C(nonStandardData);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(integrityCheckValue);
		O_C(genericData);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225UnregRequestReason /////////////////////////
 	 VS_H225UnregRequestReason::VS_H225UnregRequestReason( void )
	:VS_AsnChoice(4 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225UnregRequestReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_reregistrationRequired : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_ttlExpired : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_maintenance : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225UnregRequestReason::operator=(const VS_H225UnregRequestReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_reregistrationRequired : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_ttlExpired : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_maintenance : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225UnregRequestReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225UnregRequestReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_reregistrationRequired :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_ttlExpired :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_maintenance :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225UnregistrationRequest /////////////////////////
 	 VS_H225UnregistrationRequest :: VS_H225UnregistrationRequest( void )
	:VS_AsnSequence(3 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&callSignalAddress,0);
		ref[2].Set(&endpointAlias,1);
		ref[3].Set(&nonStandardData,1);
		ref[4].Set(&endpointIdentifier,1);
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
	void VS_H225UnregistrationRequest::operator=(const VS_H225UnregistrationRequest& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RegistrationRejectReason_InvalidTerminalAliases /////////////////////////
 	 VS_H225RegistrationRejectReason_InvalidTerminalAliases :: VS_H225RegistrationRejectReason_InvalidTerminalAliases( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&terminalAlias,1);
		ref[1].Set(&terminalAliasPattern,1);
		ref[2].Set(&supportedPrefixes,1);
	}
	void VS_H225RegistrationRejectReason_InvalidTerminalAliases::operator=(const VS_H225RegistrationRejectReason_InvalidTerminalAliases& src)
	{

		O_CC(filled);
		O_C(terminalAlias);
		O_C(terminalAliasPattern);
		O_C(supportedPrefixes);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RegistrationRejectReason /////////////////////////
 	 VS_H225RegistrationRejectReason::VS_H225RegistrationRejectReason( void )
	:VS_AsnChoice(8 , 17 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225RegistrationRejectReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_discoveryRequired : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidRevision : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidCallSignalAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidRASAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_duplicateAlias : return DecodeChoice( buffer , new VS_H225AliasAddress);
		case e_invalidTerminalType : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transportNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transportQOSNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_resourceUnavailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidAlias : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_fullRegistrationRequired : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_additiveRegistrationNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidTerminalAliases : return DecodeChoice( buffer , new VS_H225RegistrationRejectReason_InvalidTerminalAliases	 );
		case e_genericDataReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_neededFeatureNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225RegistrationRejectReason::operator=(const VS_H225RegistrationRejectReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_discoveryRequired : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidRevision : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidCallSignalAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidRASAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_duplicateAlias : CopyChoice< VS_H225AliasAddress >(src,*this); return;
		case e_invalidTerminalType : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transportNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transportQOSNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_resourceUnavailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidAlias : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_fullRegistrationRequired : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_additiveRegistrationNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidTerminalAliases : CopyChoice<VS_H225RegistrationRejectReason_InvalidTerminalAliases	  >(src,*this);  return;
		case e_genericDataReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_neededFeatureNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225RegistrationRejectReason::operator VS_H225AliasAddress *( void )
	{	return dynamic_cast< VS_H225AliasAddress * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225RegistrationRejectReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225RegistrationRejectReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_discoveryRequired :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidRevision :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidCallSignalAddress :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidRASAddress :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_duplicateAlias :  dprint4("\n\t choice: VS_H225AliasAddress ");return;
		case e_invalidTerminalType :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_transportNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_transportQOSNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_resourceUnavailable :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidAlias :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_fullRegistrationRequired :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_additiveRegistrationNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidTerminalAliases :  dprint4("\n\t choice: VS_H225RegistrationRejectReason_InvalidTerminalAliases	 ");return;
		case e_genericDataReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_neededFeatureNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225RegistrationReject /////////////////////////
 	 VS_H225RegistrationReject :: VS_H225RegistrationReject( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&rejectReason,0);
		ref[4].Set(&gatekeeperIdentifier,1);
		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&featureSet,1);
		e_ref[5].Set(&genericData,1);
	}
	void VS_H225RegistrationReject::operator=(const VS_H225RegistrationReject& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RegistrationConfirm_PreGrantedARQ /////////////////////////
 	 VS_H225RegistrationConfirm_PreGrantedARQ :: VS_H225RegistrationConfirm_PreGrantedARQ( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&makeCall,0);
		ref[1].Set(&useGKCallSignalAddressToMakeCall,0);
		ref[2].Set(&answerCall,0);
		ref[3].Set(&useGKCallSignalAddressToAnswer,0);
		e_ref[0].Set(&irrFrequencyInCall,1);
		e_ref[1].Set(&totalBandwidthRestriction,1);
		e_ref[2].Set(&alternateTransportAddresses,1);
		e_ref[3].Set(&useSpecifiedTransport,1);
	}
	void VS_H225RegistrationConfirm_PreGrantedARQ::operator=(const VS_H225RegistrationConfirm_PreGrantedARQ& src)
	{

		O_CC(filled);
		O_C(makeCall);
		O_C(useGKCallSignalAddressToMakeCall);
		O_C(answerCall);
		O_C(useGKCallSignalAddressToAnswer);
		O_C(irrFrequencyInCall);
		O_C(totalBandwidthRestriction);
		O_C(alternateTransportAddresses);
		O_C(useSpecifiedTransport);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RegistrationConfirm /////////////////////////
 	 VS_H225RegistrationConfirm :: VS_H225RegistrationConfirm( void )
	:VS_AsnSequence(3 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&callSignalAddress,0);
		ref[4].Set(&terminalAlias,1);
		ref[5].Set(&gatekeeperIdentifier,1);
		ref[6].Set(&endpointIdentifier,0);
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
	void VS_H225RegistrationConfirm::operator=(const VS_H225RegistrationConfirm& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RegistrationRequest /////////////////////////
 	 VS_H225RegistrationRequest :: VS_H225RegistrationRequest( void )
	:VS_AsnSequence(3 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&discoveryComplete,0);
		ref[4].Set(&callSignalAddress,0);
		ref[5].Set(&rasAddress,0);
		ref[6].Set(&terminalType,0);
		ref[7].Set(&terminalAlias,1);
		ref[8].Set(&gatekeeperIdentifier,1);
		ref[9].Set(&endpointVendor,0);
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
	}
	void VS_H225RegistrationRequest::operator=(const VS_H225RegistrationRequest& src)
	{

		O_CC(filled);
		O_C(requestSeqNum);
		O_C(protocolIdentifier);
		O_C(nonStandardData);
		O_C(discoveryComplete);
		O_C(callSignalAddress);
		O_C(rasAddress);
		O_C(terminalType);
		O_C(terminalAlias);
		O_C(gatekeeperIdentifier);
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225GatekeeperRejectReason /////////////////////////
 	 VS_H225GatekeeperRejectReason::VS_H225GatekeeperRejectReason( void )
	:VS_AsnChoice(4 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225GatekeeperRejectReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_resourceUnavailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_terminalExcluded : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidRevision : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenial : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_genericDataReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_neededFeatureNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225GatekeeperRejectReason::operator=(const VS_H225GatekeeperRejectReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_resourceUnavailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_terminalExcluded : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidRevision : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenial : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_genericDataReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_neededFeatureNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225GatekeeperRejectReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225GatekeeperRejectReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_resourceUnavailable :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_terminalExcluded :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidRevision :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenial :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_genericDataReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_neededFeatureNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225GatekeeperReject /////////////////////////
 	 VS_H225GatekeeperReject :: VS_H225GatekeeperReject( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&gatekeeperIdentifier,1);
		ref[4].Set(&rejectReason,0);
		e_ref[0].Set(&altGKInfo,1);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&integrityCheckValue,1);
		e_ref[4].Set(&featureSet,1);
		e_ref[5].Set(&genericData,1);
	}
	void VS_H225GatekeeperReject::operator=(const VS_H225GatekeeperReject& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225GatekeeperConfirm /////////////////////////
 	 VS_H225GatekeeperConfirm :: VS_H225GatekeeperConfirm( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&gatekeeperIdentifier,1);
		ref[4].Set(&rasAddress,0);
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
	void VS_H225GatekeeperConfirm::operator=(const VS_H225GatekeeperConfirm& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225GatekeeperRequest /////////////////////////
 	 VS_H225GatekeeperRequest :: VS_H225GatekeeperRequest( void )
	:VS_AsnSequence(4 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&requestSeqNum,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&nonStandardData,1);
		ref[3].Set(&rasAddress,0);
		ref[4].Set(&endpointType,0);
		ref[5].Set(&gatekeeperIdentifier,1);
		ref[6].Set(&callServices,1);
		ref[7].Set(&endpointAlias,1);
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
	void VS_H225GatekeeperRequest::operator=(const VS_H225GatekeeperRequest& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RasMessage /////////////////////////
 	 VS_H225RasMessage::VS_H225RasMessage( void )
	:VS_AsnChoice(25 , 32 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225RasMessage::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_gatekeeperRequest : return DecodeChoice( buffer , new VS_H225GatekeeperRequest);
		case e_gatekeeperConfirm : return DecodeChoice( buffer , new VS_H225GatekeeperConfirm);
		case e_gatekeeperReject : return DecodeChoice( buffer , new VS_H225GatekeeperReject);
		case e_registrationRequest : return DecodeChoice( buffer , new VS_H225RegistrationRequest);
		case e_registrationConfirm : return DecodeChoice( buffer , new VS_H225RegistrationConfirm);
		case e_registrationReject : return DecodeChoice( buffer , new VS_H225RegistrationReject);
		case e_unregistrationRequest : return DecodeChoice( buffer , new VS_H225UnregistrationRequest);
		case e_unregistrationConfirm : return DecodeChoice( buffer , new VS_H225UnregistrationConfirm);
		case e_unregistrationReject : return DecodeChoice( buffer , new VS_H225UnregistrationReject);
		case e_admissionRequest : return DecodeChoice( buffer , new VS_H225AdmissionRequest);
		case e_admissionConfirm : return DecodeChoice( buffer , new VS_H225AdmissionConfirm);
		case e_admissionReject : return DecodeChoice( buffer , new VS_H225AdmissionReject);
		case e_bandwidthRequest : return DecodeChoice( buffer , new VS_H225BandwidthRequest);
		case e_bandwidthConfirm : return DecodeChoice( buffer , new VS_H225BandwidthConfirm);
		case e_bandwidthReject : return DecodeChoice( buffer , new VS_H225BandwidthReject);
		case e_disengageRequest : return DecodeChoice( buffer , new VS_H225DisengageRequest);
		case e_disengageConfirm : return DecodeChoice( buffer , new VS_H225DisengageConfirm);
		case e_disengageReject : return DecodeChoice( buffer , new VS_H225DisengageReject);
		case e_locationRequest : return DecodeChoice( buffer , new VS_H225LocationRequest);
		case e_locationConfirm : return DecodeChoice( buffer , new VS_H225LocationConfirm);
		case e_locationReject : return DecodeChoice( buffer , new VS_H225LocationReject);
		case e_infoRequest : return DecodeChoice( buffer , new VS_H225InfoRequest);
		case e_infoRequestResponse : return DecodeChoice( buffer , new VS_H225InfoRequestResponse);
		case e_nonStandardMessage : return DecodeChoice( buffer , new VS_H225NonStandardMessage);
		case e_unknownMessageResponse : return DecodeChoice( buffer , new VS_H225UnknownMessageResponse);
		case e_requestInProgress : return DecodeChoice( buffer , new VS_H225RequestInProgress);
		case e_resourcesAvailableIndicate : return DecodeChoice( buffer , new VS_H225ResourcesAvailableIndicate);
		case e_resourcesAvailableConfirm : return DecodeChoice( buffer , new VS_H225ResourcesAvailableConfirm);
		case e_infoRequestAck : return DecodeChoice( buffer , new VS_H225InfoRequestAck);
		case e_infoRequestNak : return DecodeChoice( buffer , new VS_H225InfoRequestNak);
		case e_serviceControlIndication : return DecodeChoice( buffer , new VS_H225ServiceControlIndication);
		case e_serviceControlResponse : return DecodeChoice( buffer , new VS_H225ServiceControlResponse);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225RasMessage::operator=(const VS_H225RasMessage & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_gatekeeperRequest : CopyChoice< VS_H225GatekeeperRequest >(src,*this); return;
		case e_gatekeeperConfirm : CopyChoice< VS_H225GatekeeperConfirm >(src,*this); return;
		case e_gatekeeperReject : CopyChoice< VS_H225GatekeeperReject >(src,*this); return;
		case e_registrationRequest : CopyChoice< VS_H225RegistrationRequest >(src,*this); return;
		case e_registrationConfirm : CopyChoice< VS_H225RegistrationConfirm >(src,*this); return;
		case e_registrationReject : CopyChoice< VS_H225RegistrationReject >(src,*this); return;
		case e_unregistrationRequest : CopyChoice< VS_H225UnregistrationRequest >(src,*this); return;
		case e_unregistrationConfirm : CopyChoice< VS_H225UnregistrationConfirm >(src,*this); return;
		case e_unregistrationReject : CopyChoice< VS_H225UnregistrationReject >(src,*this); return;
		case e_admissionRequest : CopyChoice< VS_H225AdmissionRequest >(src,*this); return;
		case e_admissionConfirm : CopyChoice< VS_H225AdmissionConfirm >(src,*this); return;
		case e_admissionReject : CopyChoice< VS_H225AdmissionReject >(src,*this); return;
		case e_bandwidthRequest : CopyChoice< VS_H225BandwidthRequest >(src,*this); return;
		case e_bandwidthConfirm : CopyChoice< VS_H225BandwidthConfirm >(src,*this); return;
		case e_bandwidthReject : CopyChoice< VS_H225BandwidthReject >(src,*this); return;
		case e_disengageRequest : CopyChoice< VS_H225DisengageRequest >(src,*this); return;
		case e_disengageConfirm : CopyChoice< VS_H225DisengageConfirm >(src,*this); return;
		case e_disengageReject : CopyChoice< VS_H225DisengageReject >(src,*this); return;
		case e_locationRequest : CopyChoice< VS_H225LocationRequest >(src,*this); return;
		case e_locationConfirm : CopyChoice< VS_H225LocationConfirm >(src,*this); return;
		case e_locationReject : CopyChoice< VS_H225LocationReject >(src,*this); return;
		case e_infoRequest : CopyChoice< VS_H225InfoRequest >(src,*this); return;
		case e_infoRequestResponse : CopyChoice< VS_H225InfoRequestResponse >(src,*this); return;
		case e_nonStandardMessage : CopyChoice< VS_H225NonStandardMessage >(src,*this); return;
		case e_unknownMessageResponse : CopyChoice< VS_H225UnknownMessageResponse >(src,*this); return;
		case e_requestInProgress : CopyChoice< VS_H225RequestInProgress >(src,*this); return;
		case e_resourcesAvailableIndicate : CopyChoice< VS_H225ResourcesAvailableIndicate >(src,*this); return;
		case e_resourcesAvailableConfirm : CopyChoice< VS_H225ResourcesAvailableConfirm >(src,*this); return;
		case e_infoRequestAck : CopyChoice< VS_H225InfoRequestAck >(src,*this); return;
		case e_infoRequestNak : CopyChoice< VS_H225InfoRequestNak >(src,*this); return;
		case e_serviceControlIndication : CopyChoice< VS_H225ServiceControlIndication >(src,*this); return;
		case e_serviceControlResponse : CopyChoice< VS_H225ServiceControlResponse >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225RasMessage::operator VS_H225GatekeeperRequest *( void )
	{	return dynamic_cast< VS_H225GatekeeperRequest * >(choice);    }

 	VS_H225RasMessage::operator VS_H225GatekeeperConfirm *( void )
	{	return dynamic_cast< VS_H225GatekeeperConfirm * >(choice);    }

 	VS_H225RasMessage::operator VS_H225GatekeeperReject *( void )
	{	return dynamic_cast< VS_H225GatekeeperReject * >(choice);    }

 	VS_H225RasMessage::operator VS_H225RegistrationRequest *( void )
	{	return dynamic_cast< VS_H225RegistrationRequest * >(choice);    }

 	VS_H225RasMessage::operator VS_H225RegistrationConfirm *( void )
	{	return dynamic_cast< VS_H225RegistrationConfirm * >(choice);    }

 	VS_H225RasMessage::operator VS_H225RegistrationReject *( void )
	{	return dynamic_cast< VS_H225RegistrationReject * >(choice);    }

 	VS_H225RasMessage::operator VS_H225UnregistrationRequest *( void )
	{	return dynamic_cast< VS_H225UnregistrationRequest * >(choice);    }

 	VS_H225RasMessage::operator VS_H225UnregistrationConfirm *( void )
	{	return dynamic_cast< VS_H225UnregistrationConfirm * >(choice);    }

 	VS_H225RasMessage::operator VS_H225UnregistrationReject *( void )
	{	return dynamic_cast< VS_H225UnregistrationReject * >(choice);    }

 	VS_H225RasMessage::operator VS_H225AdmissionRequest *( void )
	{	return dynamic_cast< VS_H225AdmissionRequest * >(choice);    }

 	VS_H225RasMessage::operator VS_H225AdmissionConfirm *( void )
	{	return dynamic_cast< VS_H225AdmissionConfirm * >(choice);    }

 	VS_H225RasMessage::operator VS_H225AdmissionReject *( void )
	{	return dynamic_cast< VS_H225AdmissionReject * >(choice);    }

 	VS_H225RasMessage::operator VS_H225BandwidthRequest *( void )
	{	return dynamic_cast< VS_H225BandwidthRequest * >(choice);    }

 	VS_H225RasMessage::operator VS_H225BandwidthConfirm *( void )
	{	return dynamic_cast< VS_H225BandwidthConfirm * >(choice);    }

 	VS_H225RasMessage::operator VS_H225BandwidthReject *( void )
	{	return dynamic_cast< VS_H225BandwidthReject * >(choice);    }

 	VS_H225RasMessage::operator VS_H225DisengageRequest *( void )
	{	return dynamic_cast< VS_H225DisengageRequest * >(choice);    }

 	VS_H225RasMessage::operator VS_H225DisengageConfirm *( void )
	{	return dynamic_cast< VS_H225DisengageConfirm * >(choice);    }

 	VS_H225RasMessage::operator VS_H225DisengageReject *( void )
	{	return dynamic_cast< VS_H225DisengageReject * >(choice);    }

 	VS_H225RasMessage::operator VS_H225LocationRequest *( void )
	{	return dynamic_cast< VS_H225LocationRequest * >(choice);    }

 	VS_H225RasMessage::operator VS_H225LocationConfirm *( void )
	{	return dynamic_cast< VS_H225LocationConfirm * >(choice);    }

 	VS_H225RasMessage::operator VS_H225LocationReject *( void )
	{	return dynamic_cast< VS_H225LocationReject * >(choice);    }

 	VS_H225RasMessage::operator VS_H225InfoRequest *( void )
	{	return dynamic_cast< VS_H225InfoRequest * >(choice);    }

 	VS_H225RasMessage::operator VS_H225InfoRequestResponse *( void )
	{	return dynamic_cast< VS_H225InfoRequestResponse * >(choice);    }

 	VS_H225RasMessage::operator VS_H225NonStandardMessage *( void )
	{	return dynamic_cast< VS_H225NonStandardMessage * >(choice);    }

 	VS_H225RasMessage::operator VS_H225UnknownMessageResponse *( void )
	{	return dynamic_cast< VS_H225UnknownMessageResponse * >(choice);    }

 	VS_H225RasMessage::operator VS_H225RequestInProgress *( void )
	{	return dynamic_cast< VS_H225RequestInProgress * >(choice);    }

 	VS_H225RasMessage::operator VS_H225ResourcesAvailableIndicate *( void )
	{	return dynamic_cast< VS_H225ResourcesAvailableIndicate * >(choice);    }

 	VS_H225RasMessage::operator VS_H225ResourcesAvailableConfirm *( void )
	{	return dynamic_cast< VS_H225ResourcesAvailableConfirm * >(choice);    }

 	VS_H225RasMessage::operator VS_H225InfoRequestAck *( void )
	{	return dynamic_cast< VS_H225InfoRequestAck * >(choice);    }

 	VS_H225RasMessage::operator VS_H225InfoRequestNak *( void )
	{	return dynamic_cast< VS_H225InfoRequestNak * >(choice);    }

 	VS_H225RasMessage::operator VS_H225ServiceControlIndication *( void )
	{	return dynamic_cast< VS_H225ServiceControlIndication * >(choice);    }

 	VS_H225RasMessage::operator VS_H225ServiceControlResponse *( void )
	{	return dynamic_cast< VS_H225ServiceControlResponse * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225RasMessage::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225RasMessage::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_gatekeeperRequest :  dprint4("\n\t choice: VS_H225GatekeeperRequest ");return;
		case e_gatekeeperConfirm :  dprint4("\n\t choice: VS_H225GatekeeperConfirm ");return;
		case e_gatekeeperReject :  dprint4("\n\t choice: VS_H225GatekeeperReject ");return;
		case e_registrationRequest :  dprint4("\n\t choice: VS_H225RegistrationRequest ");return;
		case e_registrationConfirm :  dprint4("\n\t choice: VS_H225RegistrationConfirm ");return;
		case e_registrationReject :  dprint4("\n\t choice: VS_H225RegistrationReject ");return;
		case e_unregistrationRequest :  dprint4("\n\t choice: VS_H225UnregistrationRequest ");return;
		case e_unregistrationConfirm :  dprint4("\n\t choice: VS_H225UnregistrationConfirm ");return;
		case e_unregistrationReject :  dprint4("\n\t choice: VS_H225UnregistrationReject ");return;
		case e_admissionRequest :  dprint4("\n\t choice: VS_H225AdmissionRequest ");return;
		case e_admissionConfirm :  dprint4("\n\t choice: VS_H225AdmissionConfirm ");return;
		case e_admissionReject :  dprint4("\n\t choice: VS_H225AdmissionReject ");return;
		case e_bandwidthRequest :  dprint4("\n\t choice: VS_H225BandwidthRequest ");return;
		case e_bandwidthConfirm :  dprint4("\n\t choice: VS_H225BandwidthConfirm ");return;
		case e_bandwidthReject :  dprint4("\n\t choice: VS_H225BandwidthReject ");return;
		case e_disengageRequest :  dprint4("\n\t choice: VS_H225DisengageRequest ");return;
		case e_disengageConfirm :  dprint4("\n\t choice: VS_H225DisengageConfirm ");return;
		case e_disengageReject :  dprint4("\n\t choice: VS_H225DisengageReject ");return;
		case e_locationRequest :  dprint4("\n\t choice: VS_H225LocationRequest ");return;
		case e_locationConfirm :  dprint4("\n\t choice: VS_H225LocationConfirm ");return;
		case e_locationReject :  dprint4("\n\t choice: VS_H225LocationReject ");return;
		case e_infoRequest :  dprint4("\n\t choice: VS_H225InfoRequest ");return;
		case e_infoRequestResponse :  dprint4("\n\t choice: VS_H225InfoRequestResponse ");return;
		case e_nonStandardMessage :  dprint4("\n\t choice: VS_H225NonStandardMessage ");return;
		case e_unknownMessageResponse :  dprint4("\n\t choice: VS_H225UnknownMessageResponse ");return;
		case e_requestInProgress :  dprint4("\n\t choice: VS_H225RequestInProgress ");return;
		case e_resourcesAvailableIndicate :  dprint4("\n\t choice: VS_H225ResourcesAvailableIndicate ");return;
		case e_resourcesAvailableConfirm :  dprint4("\n\t choice: VS_H225ResourcesAvailableConfirm ");return;
		case e_infoRequestAck :  dprint4("\n\t choice: VS_H225InfoRequestAck ");return;
		case e_infoRequestNak :  dprint4("\n\t choice: VS_H225InfoRequestNak ");return;
		case e_serviceControlIndication :  dprint4("\n\t choice: VS_H225ServiceControlIndication ");return;
		case e_serviceControlResponse :  dprint4("\n\t choice: VS_H225ServiceControlResponse ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225RTPSession /////////////////////////
 	 VS_H225RTPSession :: VS_H225RTPSession( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&rtpAddress,0);
		ref[1].Set(&rtcpAddress,0);
		ref[2].Set(&cname,0);
		ref[3].Set(&ssrc,0);
		ref[4].Set(&sessionId,0);
		ref[5].Set(&associatedSessionIds,0);
		e_ref[0].Set(&multicast,1);
		e_ref[1].Set(&bandwidth,1);
	}
	void VS_H225RTPSession::operator=(const VS_H225RTPSession& src)
	{

		O_CC(filled);
		O_C(rtpAddress);
		O_C(rtcpAddress);
		O_C(cname);
		O_C(ssrc);
		O_C(sessionId);
		O_C(associatedSessionIds);
		O_C(multicast);
		O_C(bandwidth);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TransportChannelInfo /////////////////////////
 	 VS_H225TransportChannelInfo :: VS_H225TransportChannelInfo( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sendAddress,1);
		ref[1].Set(&recvAddress,1);
	}
	void VS_H225TransportChannelInfo::operator=(const VS_H225TransportChannelInfo& src)
	{

		O_CC(filled);
		O_C(sendAddress);
		O_C(recvAddress);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225FeatureSet /////////////////////////
 	 VS_H225FeatureSet :: VS_H225FeatureSet( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&replacementFeatureSet,0);
		ref[1].Set(&neededFeatures,1);
		ref[2].Set(&desiredFeatures,1);
		ref[3].Set(&supportedFeatures,1);
	}
	void VS_H225FeatureSet::operator=(const VS_H225FeatureSet& src)
	{

		O_CC(filled);
		O_C(replacementFeatureSet);
		O_C(neededFeatures);
		O_C(desiredFeatures);
		O_C(supportedFeatures);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Content /////////////////////////
 	 VS_H225Content::VS_H225Content( void )
	:VS_AsnChoice(12 , 12 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225Content::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_raw : return DecodeChoice( buffer , new TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  );
		case e_text : return DecodeChoice( buffer , new TemplIA5String<0,INT_MAX,VS_Asn::Unconstrained,false>  );
		case e_unicode : return DecodeChoice( buffer , new TemplBmpString<0,INT_MAX,VS_Asn::Unconstrained,false>  );
		case e_bool : return DecodeChoice( buffer , new  VS_AsnBoolean  );
		case e_number8 : return DecodeChoice( buffer , new TemplInteger<0,255,VS_Asn::FixedConstraint,0>  );
		case e_number16 : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		case e_number32 : return DecodeChoice( buffer , new TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  );
		case e_id : return DecodeChoice( buffer , new VS_H225GenericIdentifier);
		case e_alias : return DecodeChoice( buffer , new VS_H225AliasAddress);
		case e_transport : return DecodeChoice( buffer , new VS_H225TransportAddress);
		case e_compound : return DecodeChoice( buffer , new VS_H225EnumeratedParameter);
		case e_nested : return DecodeChoice( buffer , new VS_H225GenericData);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225Content::operator=(const VS_H225Content & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_raw : CopyChoice<TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>   >(src,*this);  return;
		case e_text : CopyChoice<TemplIA5String<0,INT_MAX,VS_Asn::Unconstrained,false>   >(src,*this);  return;
		case e_unicode : CopyChoice<TemplBmpString<0,INT_MAX,VS_Asn::Unconstrained,false>   >(src,*this);  return;
		case e_bool : CopyChoice< VS_AsnBoolean   >(src,*this);  return;
		case e_number8 : CopyChoice<TemplInteger<0,255,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_number16 : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_number32 : CopyChoice<TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_id : CopyChoice< VS_H225GenericIdentifier >(src,*this); return;
		case e_alias : CopyChoice< VS_H225AliasAddress >(src,*this); return;
		case e_transport : CopyChoice< VS_H225TransportAddress >(src,*this); return;
		case e_compound : CopyChoice< VS_H225EnumeratedParameter >(src,*this); return;
		case e_nested : CopyChoice< VS_H225GenericData >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225Content::operator VS_H225GenericIdentifier *( void )
	{	return dynamic_cast< VS_H225GenericIdentifier * >(choice);    }

 	VS_H225Content::operator VS_H225AliasAddress *( void )
	{	return dynamic_cast< VS_H225AliasAddress * >(choice);    }

 	VS_H225Content::operator VS_H225TransportAddress *( void )
	{	return dynamic_cast< VS_H225TransportAddress * >(choice);    }

 	VS_H225Content::operator VS_H225EnumeratedParameter *( void )
	{	return dynamic_cast< VS_H225EnumeratedParameter * >(choice);    }

 	VS_H225Content::operator VS_H225GenericData *( void )
	{	return dynamic_cast< VS_H225GenericData * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225Content::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225Content::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_raw :  dprint4("\n\t choice: TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
		case e_text :  dprint4("\n\t choice: TemplIA5String<0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
		case e_unicode :  dprint4("\n\t choice: TemplBmpString<0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
		case e_bool :  dprint4("\n\t choice:  VS_AsnBoolean  ");return;
		case e_number8 :  dprint4("\n\t choice: TemplInteger<0,255,VS_Asn::FixedConstraint,0>  ");return;
		case e_number16 :  dprint4("\n\t choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_number32 :  dprint4("\n\t choice: TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  ");return;
		case e_id :  dprint4("\n\t choice: VS_H225GenericIdentifier ");return;
		case e_alias :  dprint4("\n\t choice: VS_H225AliasAddress ");return;
		case e_transport :  dprint4("\n\t choice: VS_H225TransportAddress ");return;
		case e_compound :  dprint4("\n\t choice: VS_H225EnumeratedParameter ");return;
		case e_nested :  dprint4("\n\t choice: VS_H225GenericData ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225EnumeratedParameter /////////////////////////
 	 VS_H225EnumeratedParameter :: VS_H225EnumeratedParameter( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&id,0);
		ref[1].Set(&content,1);
	}
	void VS_H225EnumeratedParameter::operator=(const VS_H225EnumeratedParameter& src)
	{

		O_CC(filled);
		O_C(id);
		O_C(content);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225GenericIdentifier /////////////////////////
 	 VS_H225GenericIdentifier::VS_H225GenericIdentifier( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225GenericIdentifier::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_standard : return DecodeChoice( buffer , new TemplInteger<0,16383,VS_Asn::FixedConstraint,1>  );
		case e_oid : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		case e_nonStandard : return DecodeChoice( buffer , new VS_H225GloballyUniqueID);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225GenericIdentifier::operator=(const VS_H225GenericIdentifier & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_standard : CopyChoice<TemplInteger<0,16383,VS_Asn::FixedConstraint,1>   >(src,*this);  return;
		case e_oid : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		case e_nonStandard : CopyChoice< VS_H225GloballyUniqueID >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225GenericIdentifier::operator VS_H225GloballyUniqueID *( void )
	{	return dynamic_cast< VS_H225GloballyUniqueID * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225GenericIdentifier::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225GenericIdentifier::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_standard :  dprint4("\n\t choice: TemplInteger<0,16383,VS_Asn::FixedConstraint,1>  ");return;
		case e_oid :  dprint4("\n\t choice:  VS_AsnObjectId  ");return;
		case e_nonStandard :  dprint4("\n\t choice: VS_H225GloballyUniqueID ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225GenericData /////////////////////////
 	 VS_H225GenericData :: VS_H225GenericData( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&id,0);
		ref[1].Set(&parameters,1);
	}
	void VS_H225GenericData::operator=(const VS_H225GenericData& src)
	{

		O_CC(filled);
		O_C(id);
		O_C(parameters);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CallCreditServiceControl_BillingMode /////////////////////////
 	 VS_H225CallCreditServiceControl_BillingMode::VS_H225CallCreditServiceControl_BillingMode( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225CallCreditServiceControl_BillingMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_credit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_debit : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225CallCreditServiceControl_BillingMode::operator=(const VS_H225CallCreditServiceControl_BillingMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_credit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_debit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225CallCreditServiceControl_BillingMode::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225CallCreditServiceControl_BillingMode::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_credit :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_debit :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225CallCreditServiceControl_CallStartingPoint /////////////////////////
 	 VS_H225CallCreditServiceControl_CallStartingPoint::VS_H225CallCreditServiceControl_CallStartingPoint( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225CallCreditServiceControl_CallStartingPoint::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_alerting : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_connect : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225CallCreditServiceControl_CallStartingPoint::operator=(const VS_H225CallCreditServiceControl_CallStartingPoint & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_alerting : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_connect : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225CallCreditServiceControl_CallStartingPoint::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225CallCreditServiceControl_CallStartingPoint::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_alerting :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_connect :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225CallCreditServiceControl /////////////////////////
 	 VS_H225CallCreditServiceControl :: VS_H225CallCreditServiceControl( void )
	:VS_AsnSequence(5 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&amountString,1);
		ref[1].Set(&billingMode,1);
		ref[2].Set(&callDurationLimit,1);
		ref[3].Set(&enforceCallDurationLimit,1);
		ref[4].Set(&callStartingPoint,1);
	}
	void VS_H225CallCreditServiceControl::operator=(const VS_H225CallCreditServiceControl& src)
	{

		O_CC(filled);
		O_C(amountString);
		O_C(billingMode);
		O_C(callDurationLimit);
		O_C(enforceCallDurationLimit);
		O_C(callStartingPoint);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CallCreditCapability /////////////////////////
 	 VS_H225CallCreditCapability :: VS_H225CallCreditCapability( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&canDisplayAmountString,1);
		ref[1].Set(&canEnforceDurationLimit,1);
	}
	void VS_H225CallCreditCapability::operator=(const VS_H225CallCreditCapability& src)
	{

		O_CC(filled);
		O_C(canDisplayAmountString);
		O_C(canEnforceDurationLimit);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225BandwidthDetails /////////////////////////
 	 VS_H225BandwidthDetails :: VS_H225BandwidthDetails( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sender,0);
		ref[1].Set(&multicast,0);
		ref[2].Set(&bandwidth,0);
		ref[3].Set(&rtcpAddresses,0);
	}
	void VS_H225BandwidthDetails::operator=(const VS_H225BandwidthDetails& src)
	{

		O_CC(filled);
		O_C(sender);
		O_C(multicast);
		O_C(bandwidth);
		O_C(rtcpAddresses);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CallTerminationCause /////////////////////////
 	 VS_H225CallTerminationCause::VS_H225CallTerminationCause( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225CallTerminationCause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_releaseCompleteReason : return DecodeChoice( buffer , new VS_H225ReleaseCompleteReason);
		case e_releaseCompleteCauseIE : return DecodeChoice( buffer , new TemplOctetString<2,32,VS_Asn::FixedConstraint,0>  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225CallTerminationCause::operator=(const VS_H225CallTerminationCause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_releaseCompleteReason : CopyChoice< VS_H225ReleaseCompleteReason >(src,*this); return;
		case e_releaseCompleteCauseIE : CopyChoice<TemplOctetString<2,32,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225CallTerminationCause::operator VS_H225ReleaseCompleteReason *( void )
	{	return dynamic_cast< VS_H225ReleaseCompleteReason * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225CallTerminationCause::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225CallTerminationCause::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_releaseCompleteReason :  dprint4("\n\t choice: VS_H225ReleaseCompleteReason ");return;
		case e_releaseCompleteCauseIE :  dprint4("\n\t choice: TemplOctetString<2,32,VS_Asn::FixedConstraint,0>  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225RasUsageInformation /////////////////////////
 	 VS_H225RasUsageInformation :: VS_H225RasUsageInformation( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandardUsageFields,0);
		ref[1].Set(&alertingTime,1);
		ref[2].Set(&connectTime,1);
		ref[3].Set(&endTime,1);
	}
	void VS_H225RasUsageInformation::operator=(const VS_H225RasUsageInformation& src)
	{

		O_CC(filled);
		O_C(nonStandardUsageFields);
		O_C(alertingTime);
		O_C(connectTime);
		O_C(endTime);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RasUsageSpecification_When /////////////////////////
 	 VS_H225RasUsageSpecification_When :: VS_H225RasUsageSpecification_When( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&start,1);
		ref[1].Set(&end,1);
		ref[2].Set(&inIrr,1);
	}
	void VS_H225RasUsageSpecification_When::operator=(const VS_H225RasUsageSpecification_When& src)
	{

		O_CC(filled);
		O_C(start);
		O_C(end);
		O_C(inIrr);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RasUsageSpecification_CallStartingPoint /////////////////////////
 	 VS_H225RasUsageSpecification_CallStartingPoint :: VS_H225RasUsageSpecification_CallStartingPoint( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&alerting,1);
		ref[1].Set(&connect,1);
	}
	void VS_H225RasUsageSpecification_CallStartingPoint::operator=(const VS_H225RasUsageSpecification_CallStartingPoint& src)
	{

		O_CC(filled);
		O_C(alerting);
		O_C(connect);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RasUsageSpecification /////////////////////////
 	 VS_H225RasUsageSpecification :: VS_H225RasUsageSpecification( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&when,0);
		ref[1].Set(&callStartingPoint,1);
		ref[2].Set(&required,0);
	}
	void VS_H225RasUsageSpecification::operator=(const VS_H225RasUsageSpecification& src)
	{

		O_CC(filled);
		O_C(when);
		O_C(callStartingPoint);
		O_C(required);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225RasUsageInfoTypes /////////////////////////
 	 VS_H225RasUsageInfoTypes :: VS_H225RasUsageInfoTypes( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandardUsageTypes,0);
		ref[1].Set(&startTime,1);
		ref[2].Set(&endTime,1);
		ref[3].Set(&terminationCause,1);
	}
	void VS_H225RasUsageInfoTypes::operator=(const VS_H225RasUsageInfoTypes& src)
	{

		O_CC(filled);
		O_C(nonStandardUsageTypes);
		O_C(startTime);
		O_C(endTime);
		O_C(terminationCause);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ServiceControlSession_Reason /////////////////////////
 	 VS_H225ServiceControlSession_Reason::VS_H225ServiceControlSession_Reason( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225ServiceControlSession_Reason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_open : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_refresh : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_close : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225ServiceControlSession_Reason::operator=(const VS_H225ServiceControlSession_Reason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_open : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_refresh : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_close : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225ServiceControlSession_Reason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225ServiceControlSession_Reason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_open :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_refresh :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_close :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225ServiceControlSession /////////////////////////
 	 VS_H225ServiceControlSession :: VS_H225ServiceControlSession( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sessionId,0);
		ref[1].Set(&contents,1);
		ref[2].Set(&reason,0);
	}
	void VS_H225ServiceControlSession::operator=(const VS_H225ServiceControlSession& src)
	{

		O_CC(filled);
		O_C(sessionId);
		O_C(contents);
		O_C(reason);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ServiceControlDescriptor /////////////////////////
 	 VS_H225ServiceControlDescriptor::VS_H225ServiceControlDescriptor( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225ServiceControlDescriptor::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_url : return DecodeChoice( buffer , new TemplIA5String<0,512,VS_Asn::FixedConstraint,0>  );
		case e_signal : return DecodeChoice( buffer , new VS_H225H248SignalsDescriptor);
		case e_nonStandard : return DecodeChoice( buffer , new VS_H225NonStandardParameter);
		case e_callCreditServiceControl : return DecodeChoice( buffer , new VS_H225CallCreditServiceControl);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225ServiceControlDescriptor::operator=(const VS_H225ServiceControlDescriptor & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_url : CopyChoice<TemplIA5String<0,512,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_signal : CopyChoice< VS_H225H248SignalsDescriptor >(src,*this); return;
		case e_nonStandard : CopyChoice< VS_H225NonStandardParameter >(src,*this); return;
		case e_callCreditServiceControl : CopyChoice< VS_H225CallCreditServiceControl >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225ServiceControlDescriptor::operator VS_H225H248SignalsDescriptor *( void )
	{	return dynamic_cast< VS_H225H248SignalsDescriptor * >(choice);    }

 	VS_H225ServiceControlDescriptor::operator VS_H225NonStandardParameter *( void )
	{	return dynamic_cast< VS_H225NonStandardParameter * >(choice);    }

 	VS_H225ServiceControlDescriptor::operator VS_H225CallCreditServiceControl *( void )
	{	return dynamic_cast< VS_H225CallCreditServiceControl * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225ServiceControlDescriptor::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225ServiceControlDescriptor::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_url :  dprint4("\n\t choice: TemplIA5String<0,512,VS_Asn::FixedConstraint,0>  ");return;
		case e_signal :  dprint4("\n\t choice: VS_H225H248SignalsDescriptor ");return;
		case e_nonStandard :  dprint4("\n\t choice: VS_H225NonStandardParameter ");return;
		case e_callCreditServiceControl :  dprint4("\n\t choice: VS_H225CallCreditServiceControl ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225GroupID /////////////////////////
 	 VS_H225GroupID :: VS_H225GroupID( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&member,1);
		ref[1].Set(&group,0);
	}
	void VS_H225GroupID::operator=(const VS_H225GroupID& src)
	{

		O_CC(filled);
		O_C(member);
		O_C(group);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CicInfo /////////////////////////
 	 VS_H225CicInfo :: VS_H225CicInfo( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&cic,0);
		ref[1].Set(&pointCode,0);
	}
	void VS_H225CicInfo::operator=(const VS_H225CicInfo& src)
	{

		O_CC(filled);
		O_C(cic);
		O_C(pointCode);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CircuitIdentifier /////////////////////////
 	 VS_H225CircuitIdentifier::VS_H225CircuitIdentifier( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225CircuitIdentifier::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_cic : return DecodeChoice( buffer , new VS_H225CicInfo);
		case e_group : return DecodeChoice( buffer , new VS_H225GroupID);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225CircuitIdentifier::operator=(const VS_H225CircuitIdentifier & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_cic : CopyChoice< VS_H225CicInfo >(src,*this); return;
		case e_group : CopyChoice< VS_H225GroupID >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225CircuitIdentifier::operator VS_H225CicInfo *( void )
	{	return dynamic_cast< VS_H225CicInfo * >(choice);    }

 	VS_H225CircuitIdentifier::operator VS_H225GroupID *( void )
	{	return dynamic_cast< VS_H225GroupID * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225CircuitIdentifier::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225CircuitIdentifier::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_cic :  dprint4("\n\t choice: VS_H225CicInfo ");return;
		case e_group :  dprint4("\n\t choice: VS_H225GroupID ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225CircuitInfo /////////////////////////
 	 VS_H225CircuitInfo :: VS_H225CircuitInfo( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sourceCircuitID,1);
		ref[1].Set(&destinationCircuitID,1);
		ref[2].Set(&genericData,1);
	}
	void VS_H225CircuitInfo::operator=(const VS_H225CircuitInfo& src)
	{

		O_CC(filled);
		O_C(sourceCircuitID);
		O_C(destinationCircuitID);
		O_C(genericData);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CallsAvailable /////////////////////////
 	 VS_H225CallsAvailable :: VS_H225CallsAvailable( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&calls,0);
		ref[1].Set(&group,1);
	}
	void VS_H225CallsAvailable::operator=(const VS_H225CallsAvailable& src)
	{

		O_CC(filled);
		O_C(calls);
		O_C(group);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CallCapacityInfo /////////////////////////
 	 VS_H225CallCapacityInfo :: VS_H225CallCapacityInfo( void )
	:VS_AsnSequence(11 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&voiceGwCallsAvailable,1);
		ref[1].Set(&h310GwCallsAvailable,1);
		ref[2].Set(&h320GwCallsAvailable,1);
		ref[3].Set(&h321GwCallsAvailable,1);
		ref[4].Set(&h322GwCallsAvailable,1);
		ref[5].Set(&h323GwCallsAvailable,1);
		ref[6].Set(&h324GwCallsAvailable,1);
		ref[7].Set(&t120OnlyGwCallsAvailable,1);
		ref[8].Set(&t38FaxAnnexbOnlyGwCallsAvailable,1);
		ref[9].Set(&terminalCallsAvailable,1);
		ref[10].Set(&mcuCallsAvailable,1);
	}
	void VS_H225CallCapacityInfo::operator=(const VS_H225CallCapacityInfo& src)
	{

		O_CC(filled);
		O_C(voiceGwCallsAvailable);
		O_C(h310GwCallsAvailable);
		O_C(h320GwCallsAvailable);
		O_C(h321GwCallsAvailable);
		O_C(h322GwCallsAvailable);
		O_C(h323GwCallsAvailable);
		O_C(h324GwCallsAvailable);
		O_C(t120OnlyGwCallsAvailable);
		O_C(t38FaxAnnexbOnlyGwCallsAvailable);
		O_C(terminalCallsAvailable);
		O_C(mcuCallsAvailable);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CallCapacity /////////////////////////
 	 VS_H225CallCapacity :: VS_H225CallCapacity( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&maximumCallCapacity,1);
		ref[1].Set(&currentCallCapacity,1);
	}
	void VS_H225CallCapacity::operator=(const VS_H225CallCapacity& src)
	{

		O_CC(filled);
		O_C(maximumCallCapacity);
		O_C(currentCallCapacity);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CapacityReportingSpecification_When /////////////////////////
 	 VS_H225CapacityReportingSpecification_When :: VS_H225CapacityReportingSpecification_When( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&callStart,1);
		ref[1].Set(&callEnd,1);
	}
	void VS_H225CapacityReportingSpecification_When::operator=(const VS_H225CapacityReportingSpecification_When& src)
	{

		O_CC(filled);
		O_C(callStart);
		O_C(callEnd);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CapacityReportingSpecification /////////////////////////
 	 VS_H225CapacityReportingSpecification :: VS_H225CapacityReportingSpecification( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&when,0);
	}
	void VS_H225CapacityReportingSpecification::operator=(const VS_H225CapacityReportingSpecification& src)
	{

		O_CC(filled);
		O_C(when);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CapacityReportingCapability /////////////////////////
 	 VS_H225CapacityReportingCapability :: VS_H225CapacityReportingCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&canReportCallCapacity,0);
	}
	void VS_H225CapacityReportingCapability::operator=(const VS_H225CapacityReportingCapability& src)
	{

		O_CC(filled);
		O_C(canReportCallCapacity);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225SupportedPrefix /////////////////////////
 	 VS_H225SupportedPrefix :: VS_H225SupportedPrefix( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&prefix,0);
	}
	void VS_H225SupportedPrefix::operator=(const VS_H225SupportedPrefix& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(prefix);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CallLinkage /////////////////////////
 	 VS_H225CallLinkage :: VS_H225CallLinkage( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&globalCallId,1);
		ref[1].Set(&threadId,1);
	}
	void VS_H225CallLinkage::operator=(const VS_H225CallLinkage& src)
	{

		O_CC(filled);
		O_C(globalCallId);
		O_C(threadId);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225DataRate /////////////////////////
 	 VS_H225DataRate :: VS_H225DataRate( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&channelRate,0);
		ref[2].Set(&channelMultiplier,1);
	}
	void VS_H225DataRate::operator=(const VS_H225DataRate& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(channelRate);
		O_C(channelMultiplier);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CryptoH323Token /////////////////////////
 	VS_H225CryptoH323Token::VS_H225CryptoH323Token( void )
		:VS_AsnChoice(8, 8, 1) {}

	bool VS_H225CryptoH323Token::Decode(VS_PerBuffer &buffer)
	{
		if (!buffer.ChoiceDecode(*this)) return false;

		switch(tag)
		{
			case e_cryptoEPPwdHash : return DecodeChoice(buffer , new VS_H225CryptoEPPwdHash);
			default: return buffer.ChoiceMissExtensionObject(*this);
		}
	}

	void VS_H225CryptoH323Token::operator=(const VS_H225CryptoH323Token & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
			case e_cryptoEPPwdHash : CopyChoice< VS_H225CryptoEPPwdHash >(src,*this); break;
		default: break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CryptoEPPwdHash /////////////////////////

	VS_H225CryptoEPPwdHash::VS_H225CryptoEPPwdHash()
		:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 0 )
	{
		ref[0].Set(&alias, 0);
		ref[1].Set(&timestamp, 0);
		ref[2].Set(&token, 0);
	}

	void VS_H225CryptoEPPwdHash::operator=(const VS_H225CryptoEPPwdHash& src)
	{
		O_C(alias);
		O_C(timestamp);
		O_C(token);

		O_CC(filled);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Icv /////////////////////////
 	 VS_H225Icv :: VS_H225Icv( void )
	: VS_AsnSequence(0, ref, basic_root, nullptr, extension_root, false)
{
	ref[0].Set(&algorithmOID, 0);
	ref[1].Set(&icv, 0);
}

	void VS_H225Icv::operator=(const VS_H225Icv& src)
	{

		O_CC(filled);
		O_C(algorithmOID);
		O_C(icv);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225IntegrityMechanism /////////////////////////
 	 VS_H225IntegrityMechanism::VS_H225IntegrityMechanism( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225IntegrityMechanism::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H225NonStandardParameter);
		case e_digSig : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_iso9797 : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		case e_nonIsoIM : return DecodeChoice( buffer , new VS_H225NonIsoIntegrityMechanism);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225IntegrityMechanism::operator=(const VS_H225IntegrityMechanism & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H225NonStandardParameter >(src,*this); return;
		case e_digSig : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_iso9797 : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		case e_nonIsoIM : CopyChoice< VS_H225NonIsoIntegrityMechanism >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225IntegrityMechanism::operator VS_H225NonStandardParameter *( void )
	{	return dynamic_cast< VS_H225NonStandardParameter * >(choice);    }

 	VS_H225IntegrityMechanism::operator VS_H225NonIsoIntegrityMechanism *( void )
	{	return dynamic_cast< VS_H225NonIsoIntegrityMechanism * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225IntegrityMechanism::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225IntegrityMechanism::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_nonStandard :  dprint4("\n\t choice: VS_H225NonStandardParameter ");return;
		case e_digSig :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_iso9797 :  dprint4("\n\t choice:  VS_AsnObjectId  ");return;
		case e_nonIsoIM :  dprint4("\n\t choice: VS_H225NonIsoIntegrityMechanism ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225NonIsoIntegrityMechanism /////////////////////////
 	 VS_H225NonIsoIntegrityMechanism::VS_H225NonIsoIntegrityMechanism( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225NonIsoIntegrityMechanism::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_hMAC_MD5 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_hMAC_iso10118_2_s : return DecodeChoice( buffer , new VS_H225EncryptIntAlg);
		case e_hMAC_iso10118_2_l : return DecodeChoice( buffer , new VS_H225EncryptIntAlg);
		case e_hMAC_iso10118_3 : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225NonIsoIntegrityMechanism::operator=(const VS_H225NonIsoIntegrityMechanism & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_hMAC_MD5 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_hMAC_iso10118_2_s : CopyChoice< VS_H225EncryptIntAlg >(src,*this); return;
		case e_hMAC_iso10118_2_l : CopyChoice< VS_H225EncryptIntAlg >(src,*this); return;
		case e_hMAC_iso10118_3 : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225NonIsoIntegrityMechanism::operator VS_H225EncryptIntAlg *( void )
	{	return dynamic_cast< VS_H225EncryptIntAlg * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225NonIsoIntegrityMechanism::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225NonIsoIntegrityMechanism::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_hMAC_MD5 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_hMAC_iso10118_2_s :  dprint4("\n\t choice: VS_H225EncryptIntAlg ");return;
		case e_hMAC_iso10118_2_l :  dprint4("\n\t choice: VS_H225EncryptIntAlg ");return;
		case e_hMAC_iso10118_3 :  dprint4("\n\t choice:  VS_AsnObjectId  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225EncryptIntAlg /////////////////////////
 	 VS_H225EncryptIntAlg::VS_H225EncryptIntAlg( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225EncryptIntAlg::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H225NonStandardParameter);
		case e_isoAlgorithm : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225EncryptIntAlg::operator=(const VS_H225EncryptIntAlg & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H225NonStandardParameter >(src,*this); return;
		case e_isoAlgorithm : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225EncryptIntAlg::operator VS_H225NonStandardParameter *( void )
	{	return dynamic_cast< VS_H225NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225EncryptIntAlg::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225EncryptIntAlg::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_nonStandard :  dprint4("\n\t choice: VS_H225NonStandardParameter ");return;
		case e_isoAlgorithm :  dprint4("\n\t choice:  VS_AsnObjectId  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225CallIdentifier /////////////////////////
 	 VS_H225CallIdentifier :: VS_H225CallIdentifier( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&guid,0);
	}
	void VS_H225CallIdentifier::operator=(const VS_H225CallIdentifier& src)
	{

		O_CC(filled);
		O_C(guid);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Q954Details /////////////////////////
 	 VS_H225Q954Details :: VS_H225Q954Details( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&conferenceCalling,0);
		ref[1].Set(&threePartyService,0);
	}
	void VS_H225Q954Details::operator=(const VS_H225Q954Details& src)
	{

		O_CC(filled);
		O_C(conferenceCalling);
		O_C(threePartyService);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225QseriesOptions /////////////////////////
 	 VS_H225QseriesOptions :: VS_H225QseriesOptions( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&q932Full,0);
		ref[1].Set(&q951Full,0);
		ref[2].Set(&q952Full,0);
		ref[3].Set(&q953Full,0);
		ref[4].Set(&q955Full,0);
		ref[5].Set(&q956Full,0);
		ref[6].Set(&q957Full,0);
		ref[7].Set(&q954Info,0);
	}
	void VS_H225QseriesOptions::operator=(const VS_H225QseriesOptions& src)
	{

		O_CC(filled);
		O_C(q932Full);
		O_C(q951Full);
		O_C(q952Full);
		O_C(q953Full);
		O_C(q955Full);
		O_C(q956Full);
		O_C(q957Full);
		O_C(q954Info);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H245Security /////////////////////////
 	 VS_H225H245Security::VS_H225H245Security( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225H245Security::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H225NonStandardParameter);
		case e_noSecurity : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_tls : return DecodeChoice( buffer , new VS_H225SecurityCapabilities);
		case e_ipsec : return DecodeChoice( buffer , new VS_H225SecurityCapabilities);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225H245Security::operator=(const VS_H225H245Security & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H225NonStandardParameter >(src,*this); return;
		case e_noSecurity : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_tls : CopyChoice< VS_H225SecurityCapabilities >(src,*this); return;
		case e_ipsec : CopyChoice< VS_H225SecurityCapabilities >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225H245Security::operator VS_H225NonStandardParameter *( void )
	{	return dynamic_cast< VS_H225NonStandardParameter * >(choice);    }

 	VS_H225H245Security::operator VS_H225SecurityCapabilities *( void )
	{	return dynamic_cast< VS_H225SecurityCapabilities * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225H245Security::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225H245Security::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_nonStandard :  dprint4("\n\t choice: VS_H225NonStandardParameter ");return;
		case e_noSecurity :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_tls :  dprint4("\n\t choice: VS_H225SecurityCapabilities ");return;
		case e_ipsec :  dprint4("\n\t choice: VS_H225SecurityCapabilities ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225SecurityCapabilities /////////////////////////
 	 VS_H225SecurityCapabilities :: VS_H225SecurityCapabilities( void )
	:VS_AsnSequence(1 , ref , basic_root,  nullptr , extension_root , true )
	{
		ref[0].Set(&nonStandard,1);
		ref[1].Set(&encryption,0);
		ref[2].Set(&authenticaton,0);
		ref[3].Set(&integrity,0);
	}
	void VS_H225SecurityCapabilities::operator=(const VS_H225SecurityCapabilities& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_C(encryption);
		O_C(authenticaton);
		O_C(integrity);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225SecurityServiceMode /////////////////////////
 	 VS_H225SecurityServiceMode::VS_H225SecurityServiceMode( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225SecurityServiceMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H225NonStandardParameter);
		case e_none : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_default : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225SecurityServiceMode::operator=(const VS_H225SecurityServiceMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H225NonStandardParameter >(src,*this); return;
		case e_none : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_default : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225SecurityServiceMode::operator VS_H225NonStandardParameter *( void )
	{	return dynamic_cast< VS_H225NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225SecurityServiceMode::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225SecurityServiceMode::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_nonStandard :  dprint4("\n\t choice: VS_H225NonStandardParameter ");return;
		case e_none :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_default :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225AltGKInfo /////////////////////////
 	 VS_H225AltGKInfo :: VS_H225AltGKInfo( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&alternateGatekeeper,0);
		ref[1].Set(&altGKisPermanent,0);
	}
	void VS_H225AltGKInfo::operator=(const VS_H225AltGKInfo& src)
	{

		O_CC(filled);
		O_C(alternateGatekeeper);
		O_C(altGKisPermanent);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225AlternateGK /////////////////////////
 	 VS_H225AlternateGK :: VS_H225AlternateGK( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr, extension_root , true )
	{
		ref[0].Set(&rasAddress,0);
		ref[1].Set(&gatekeeperIdentifier,1);
		ref[2].Set(&needToRegister,0);
		ref[3].Set(&priority,0);
	}
	void VS_H225AlternateGK::operator=(const VS_H225AlternateGK& src)
	{

		O_CC(filled);
		O_C(rasAddress);
		O_C(gatekeeperIdentifier);
		O_C(needToRegister);
		O_C(priority);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225UseSpecifiedTransport /////////////////////////
 	 VS_H225UseSpecifiedTransport::VS_H225UseSpecifiedTransport( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225UseSpecifiedTransport::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_tcp : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_annexE : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225UseSpecifiedTransport::operator=(const VS_H225UseSpecifiedTransport & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_tcp : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_annexE : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225UseSpecifiedTransport::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225UseSpecifiedTransport::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_tcp :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_annexE :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225AlternateTransportAddresses /////////////////////////
 	 VS_H225AlternateTransportAddresses :: VS_H225AlternateTransportAddresses( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&annexE,1);
	}
	void VS_H225AlternateTransportAddresses::operator=(const VS_H225AlternateTransportAddresses& src)
	{

		O_CC(filled);
		O_C(annexE);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Endpoint /////////////////////////
 	 VS_H225Endpoint :: VS_H225Endpoint( void )
	:VS_AsnSequence(10 , ref , basic_root, e_ref , extension_root , true )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&aliasAddress,1);
		ref[2].Set(&callSignalAddress,1);
		ref[3].Set(&rasAddress,1);
		ref[4].Set(&endpointType,1);
		ref[5].Set(&tokens,1);
		ref[6].Set(&cryptoTokens,1);
		ref[7].Set(&priority,1);
		ref[8].Set(&remoteExtensionAddress,1);
		ref[9].Set(&destExtraCallInfo,1);
		e_ref[0].Set(&alternateTransportAddresses,1);
	}
	void VS_H225Endpoint::operator=(const VS_H225Endpoint& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(aliasAddress);
		O_C(callSignalAddress);
		O_C(rasAddress);
		O_C(endpointType);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(priority);
		O_C(remoteExtensionAddress);
		O_C(destExtraCallInfo);
		O_C(alternateTransportAddresses);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ExtendedAliasAddress /////////////////////////
 	 VS_H225ExtendedAliasAddress :: VS_H225ExtendedAliasAddress( void )

	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&address,0);
		ref[1].Set(&presentationIndicator,1);
		ref[2].Set(&screeningIndicator,1);
	}
	void VS_H225ExtendedAliasAddress::operator=(const VS_H225ExtendedAliasAddress& src)
	{

		O_CC(filled);
		O_C(address);
		O_C(presentationIndicator);
		O_C(screeningIndicator);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225GSM_UIM /////////////////////////
//////////////////////CLASS VS_H225GSM_UIM /////////////////////////
 	 VS_H225GSM_UIM :: VS_H225GSM_UIM( void )
	:VS_AsnSequence(6 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&imsi,1);
		ref[1].Set(&tmsi,1);
		ref[2].Set(&msisdn,1);
		ref[3].Set(&imei,1);
		ref[4].Set(&hplmn,1);
		ref[5].Set(&vplmn,1);
	}

	 /////////////////////////////////////////////////////////////////////////////////////////
	 constexpr unsigned char VS_H225GSM_UIM::IMSI_ALPHABET[];
	unsigned char  VS_H225GSM_UIM::imsi_inverse_table[256]={0};
	 const bool VS_H225GSM_UIM::imsi_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225GSM_UIM::imsi_inverse_table,
		 VS_H225GSM_UIM::IMSI_ALPHABET,
		 sizeof(VS_H225GSM_UIM::IMSI_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 constexpr unsigned char VS_H225GSM_UIM::MSISDN_ALPHABET[];
	unsigned char  VS_H225GSM_UIM::msisdn_inverse_table[256]={0};
	 const bool VS_H225GSM_UIM::msisdn_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225GSM_UIM::msisdn_inverse_table,
		 VS_H225GSM_UIM::MSISDN_ALPHABET,
		 sizeof(VS_H225GSM_UIM::MSISDN_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 constexpr unsigned char VS_H225GSM_UIM::IMEI_ALPHABET[];
	unsigned char  VS_H225GSM_UIM::imei_inverse_table[256]={0};
	 const bool VS_H225GSM_UIM::imei_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225GSM_UIM::imei_inverse_table,
		 VS_H225GSM_UIM::IMEI_ALPHABET,
		 sizeof(VS_H225GSM_UIM::IMEI_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225GSM_UIM::HPLMN_ALPHABET[];
	unsigned char  VS_H225GSM_UIM::hplmn_inverse_table[256]={0};
	 const bool VS_H225GSM_UIM::hplmn_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225GSM_UIM::hplmn_inverse_table,
		 VS_H225GSM_UIM::HPLMN_ALPHABET,
		 sizeof(VS_H225GSM_UIM::HPLMN_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 constexpr unsigned char VS_H225GSM_UIM::VPLMN_ALPHABET[];
	unsigned char  VS_H225GSM_UIM::vplmn_inverse_table[256]={0};
	 const bool VS_H225GSM_UIM::vplmn_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225GSM_UIM::vplmn_inverse_table,
		 VS_H225GSM_UIM::VPLMN_ALPHABET,
		 sizeof(VS_H225GSM_UIM::VPLMN_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225GSM_UIM::operator=(const VS_H225GSM_UIM& src)
	{

		O_CC(filled);
		O_C(imsi);
		O_C(tmsi);
		O_C(msisdn);
		O_C(imei);
		O_C(hplmn);
		O_C(vplmn);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ANSI_41_UIM_System_id /////////////////////////
 	 VS_H225ANSI_41_UIM_System_id::VS_H225ANSI_41_UIM_System_id( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225ANSI_41_UIM_System_id::SID_ALPHABET[];
	unsigned  VS_H225ANSI_41_UIM_System_id::sid_alphabet_size=sizeof(VS_H225ANSI_41_UIM_System_id::SID_ALPHABET);
	unsigned char  VS_H225ANSI_41_UIM_System_id::sid_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM_System_id::sid_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM_System_id::sid_inverse_table,
		 VS_H225ANSI_41_UIM_System_id::SID_ALPHABET,
		 VS_H225ANSI_41_UIM_System_id::sid_alphabet_size );

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225ANSI_41_UIM_System_id::MID_ALPHABET[];
	unsigned  VS_H225ANSI_41_UIM_System_id::mid_alphabet_size=sizeof(VS_H225ANSI_41_UIM_System_id::MID_ALPHABET);
	unsigned char  VS_H225ANSI_41_UIM_System_id::mid_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM_System_id::mid_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM_System_id::mid_inverse_table,
		 VS_H225ANSI_41_UIM_System_id::MID_ALPHABET,
		 VS_H225ANSI_41_UIM_System_id::mid_alphabet_size );

	 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225ANSI_41_UIM_System_id::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_sid : return DecodeChoice(buffer, new TemplAlphabeticString<SID_ALPHABET, sizeof(SID_ALPHABET), sid_inverse_table, 1, 4, VS_Asn::FixedConstraint, 0>);
		case e_mid : return DecodeChoice(buffer, new TemplAlphabeticString<MID_ALPHABET, sizeof(MID_ALPHABET), mid_inverse_table, 1, 4, VS_Asn::FixedConstraint, 0>);

		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225ANSI_41_UIM_System_id::operator=(const VS_H225ANSI_41_UIM_System_id & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_sid : CopyChoice<TemplAlphabeticString<SID_ALPHABET, sizeof(SID_ALPHABET), sid_inverse_table, 1, 4, VS_Asn::FixedConstraint, 0>>(src, *this);  return;
		case e_mid : CopyChoice<TemplAlphabeticString<MID_ALPHABET, sizeof(MID_ALPHABET), mid_inverse_table, 1, 4, VS_Asn::FixedConstraint, 0>>(src, *this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225ANSI_41_UIM_System_id::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225ANSI_41_UIM_System_id::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_sid :  dprint4("\n\t choice: TemplAlphabeticString< sid_alphabet, sid_alphabet_size,sid_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
		case e_mid :  dprint4("\n\t choice: TemplAlphabeticString< mid_alphabet, mid_alphabet_size,mid_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225ANSI_41_UIM /////////////////////////
//////////////////////CLASS VS_H225ANSI_41_UIM /////////////////////////
 	 VS_H225ANSI_41_UIM :: VS_H225ANSI_41_UIM( void )
	: VS_AsnSequence(11 , ref , basic_root, nullptr, extension_root , true )
	{
		ref[0].Set(&imsi,1);
		ref[1].Set(&min,1);
		ref[2].Set(&mdn,1);
		ref[3].Set(&msisdn,1);
		ref[4].Set(&esn,1);
		ref[5].Set(&mscid,1);
		ref[6].Set(&system_id,0);
		ref[7].Set(&systemMyTypeCode,1);
		ref[8].Set(&systemAccessType,1);
		ref[9].Set(&qualificationInformationCode,1);
		ref[10].Set(&sesn,1);
		ref[11].Set(&soc,1);
	}

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225ANSI_41_UIM::IMSSI_APLHABET[];
	unsigned char  VS_H225ANSI_41_UIM::imsi_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM::imsi_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM::imsi_inverse_table,
		 VS_H225ANSI_41_UIM::IMSSI_APLHABET,
		 sizeof(VS_H225ANSI_41_UIM::IMSSI_APLHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 constexpr unsigned char VS_H225ANSI_41_UIM::MIN_ALPHABET[];
	unsigned char  VS_H225ANSI_41_UIM::min_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM::min_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM::min_inverse_table,
		 VS_H225ANSI_41_UIM::MIN_ALPHABET,
		 sizeof(VS_H225ANSI_41_UIM::MIN_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	 constexpr unsigned char VS_H225ANSI_41_UIM::MDN_ALPHABET[];

	unsigned char  VS_H225ANSI_41_UIM::mdn_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM::mdn_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM::mdn_inverse_table,
		 VS_H225ANSI_41_UIM::MDN_ALPHABET,
		 sizeof(VS_H225ANSI_41_UIM::MDN_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225ANSI_41_UIM::MSISDN_ALPHABET[];
	unsigned char  VS_H225ANSI_41_UIM::msisdn_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM::msisdn_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM::msisdn_inverse_table,
		 VS_H225ANSI_41_UIM::MSISDN_ALPHABET,
		 sizeof(VS_H225ANSI_41_UIM::MSISDN_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225ANSI_41_UIM::ESN_ALPHABET[];
	unsigned char  VS_H225ANSI_41_UIM::esn_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM::esn_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM::esn_inverse_table,
		 VS_H225ANSI_41_UIM::ESN_ALPHABET,
		 sizeof(VS_H225ANSI_41_UIM::ESN_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225ANSI_41_UIM::MSCID_ALPHABET[];

	unsigned char  VS_H225ANSI_41_UIM::mscid_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM::mscid_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM::mscid_inverse_table,
		 VS_H225ANSI_41_UIM::MSCID_ALPHABET,
		 sizeof(VS_H225ANSI_41_UIM::MSCID_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225ANSI_41_UIM::SESN_ALPHABET[];

	unsigned char  VS_H225ANSI_41_UIM::sesn_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM::sesn_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM::sesn_inverse_table,
		 VS_H225ANSI_41_UIM::SESN_ALPHABET,
		 sizeof(VS_H225ANSI_41_UIM::SESN_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225ANSI_41_UIM::SOC_ALPHABET[];

	unsigned char  VS_H225ANSI_41_UIM::soc_inverse_table[256]={0};
	 const bool VS_H225ANSI_41_UIM::soc_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225ANSI_41_UIM::soc_inverse_table,
		 VS_H225ANSI_41_UIM::SOC_ALPHABET,
		 sizeof(VS_H225ANSI_41_UIM::SOC_ALPHABET));

	 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225ANSI_41_UIM::operator=(const VS_H225ANSI_41_UIM& src)
	{

		O_CC(filled);
		O_C(imsi);
		O_C(min);
		O_C(mdn);
		O_C(msisdn);
		O_C(esn);
		O_C(mscid);
		O_C(system_id);
		O_C(systemMyTypeCode);
		O_C(systemAccessType);
		O_C(qualificationInformationCode);
		O_C(sesn);
		O_C(soc);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225MobileUIM /////////////////////////
 	 VS_H225MobileUIM::VS_H225MobileUIM( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225MobileUIM::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_ansi_41_uim : return DecodeChoice( buffer , new VS_H225ANSI_41_UIM);
		case e_gsm_uim : return DecodeChoice( buffer , new VS_H225GSM_UIM);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225MobileUIM::operator=(const VS_H225MobileUIM & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_ansi_41_uim : CopyChoice< VS_H225ANSI_41_UIM >(src,*this); return;
		case e_gsm_uim : CopyChoice< VS_H225GSM_UIM >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225MobileUIM::operator VS_H225ANSI_41_UIM *( void )
	{	return dynamic_cast< VS_H225ANSI_41_UIM * >(choice);    }

 	VS_H225MobileUIM::operator VS_H225GSM_UIM *( void )
	{	return dynamic_cast< VS_H225GSM_UIM * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225MobileUIM::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225MobileUIM::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_ansi_41_uim :  dprint4("\n\t choice: VS_H225ANSI_41_UIM ");return;
		case e_gsm_uim :  dprint4("\n\t choice: VS_H225GSM_UIM ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225PrivateTypeOfNumber /////////////////////////
 	 VS_H225PrivateTypeOfNumber::VS_H225PrivateTypeOfNumber( void )
	:VS_AsnChoice(6 , 6 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225PrivateTypeOfNumber::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unknown : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_level2RegionalNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_level1RegionalNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_pISNSpecificNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_localNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_abbreviatedNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225PrivateTypeOfNumber::operator=(const VS_H225PrivateTypeOfNumber & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unknown : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_level2RegionalNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_level1RegionalNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_pISNSpecificNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_localNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_abbreviatedNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225PrivateTypeOfNumber::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225PrivateTypeOfNumber::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_unknown :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_level2RegionalNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_level1RegionalNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_pISNSpecificNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_localNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_abbreviatedNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225PublicTypeOfNumber /////////////////////////
 	 VS_H225PublicTypeOfNumber::VS_H225PublicTypeOfNumber( void )
	:VS_AsnChoice(6 , 6 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225PublicTypeOfNumber::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unknown : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_internationalNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_nationalNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_networkSpecificNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_subscriberNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_abbreviatedNumber : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225PublicTypeOfNumber::operator=(const VS_H225PublicTypeOfNumber & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unknown : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_internationalNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_nationalNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_networkSpecificNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_subscriberNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_abbreviatedNumber : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225PublicTypeOfNumber::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225PublicTypeOfNumber::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_unknown :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_internationalNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_nationalNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_networkSpecificNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_subscriberNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_abbreviatedNumber :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225PrivatePartyNumber /////////////////////////
 	 VS_H225PrivatePartyNumber :: VS_H225PrivatePartyNumber( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , false )
	{
		ref[0].Set(&privateTypeOfNumber,0);
		ref[1].Set(&privateNumberDigits,0);
	}
	void VS_H225PrivatePartyNumber::operator=(const VS_H225PrivatePartyNumber& src)
	{

		O_CC(filled);
		O_C(privateTypeOfNumber);
		O_C(privateNumberDigits);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225PublicPartyNumber /////////////////////////
 	 VS_H225PublicPartyNumber :: VS_H225PublicPartyNumber( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , false )
	{
		ref[0].Set(&publicTypeOfNumber,0);
		ref[1].Set(&publicNumberDigits,0);
	}
	void VS_H225PublicPartyNumber::operator=(const VS_H225PublicPartyNumber& src)
	{

		O_CC(filled);
		O_C(publicTypeOfNumber);
		O_C(publicNumberDigits);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225PartyNumber /////////////////////////
 	 VS_H225PartyNumber::VS_H225PartyNumber( void )
	:VS_AsnChoice(5 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225PartyNumber::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_e164Number : return DecodeChoice( buffer , new VS_H225PublicPartyNumber);
		case e_dataPartyNumber : return DecodeChoice( buffer , new VS_H225NumberDigits);
		case e_telexPartyNumber : return DecodeChoice( buffer , new VS_H225NumberDigits);
		case e_privateNumber : return DecodeChoice( buffer , new VS_H225PrivatePartyNumber);
		case e_nationalStandardPartyNumber : return DecodeChoice( buffer , new VS_H225NumberDigits);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225PartyNumber::operator=(const VS_H225PartyNumber & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_e164Number : CopyChoice< VS_H225PublicPartyNumber >(src,*this); return;
		case e_dataPartyNumber : CopyChoice< VS_H225NumberDigits >(src,*this); return;
		case e_telexPartyNumber : CopyChoice< VS_H225NumberDigits >(src,*this); return;
		case e_privateNumber : CopyChoice< VS_H225PrivatePartyNumber >(src,*this); return;
		case e_nationalStandardPartyNumber : CopyChoice< VS_H225NumberDigits >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225PartyNumber::operator VS_H225PublicPartyNumber *( void )
	{	return dynamic_cast< VS_H225PublicPartyNumber * >(choice);    }

 	VS_H225PartyNumber::operator VS_H225NumberDigits *( void )
	{	return dynamic_cast< VS_H225NumberDigits * >(choice);    }

 	VS_H225PartyNumber::operator VS_H225PrivatePartyNumber *( void )
	{	return dynamic_cast< VS_H225PrivatePartyNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225PartyNumber::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225PartyNumber::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_e164Number :  dprint4("\n\t choice: VS_H225PublicPartyNumber ");return;
		case e_dataPartyNumber :  dprint4("\n\t choice: VS_H225NumberDigits ");return;
		case e_telexPartyNumber :  dprint4("\n\t choice: VS_H225NumberDigits ");return;
		case e_privateNumber :  dprint4("\n\t choice: VS_H225PrivatePartyNumber ");return;
		case e_nationalStandardPartyNumber :  dprint4("\n\t choice: VS_H225NumberDigits ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225AddressPattern_Range /////////////////////////
 	 VS_H225AddressPattern_Range :: VS_H225AddressPattern_Range( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , false )
	{
		ref[0].Set(&startOfRange,0);
		ref[1].Set(&endOfRange,0);
	}
	void VS_H225AddressPattern_Range::operator=(const VS_H225AddressPattern_Range& src)
	{

		O_CC(filled);
		O_C(startOfRange);
		O_C(endOfRange);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225AddressPattern /////////////////////////
 	 VS_H225AddressPattern::VS_H225AddressPattern( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225AddressPattern::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_wildcard : return DecodeChoice( buffer , new VS_H225AliasAddress);
		case e_range : return DecodeChoice( buffer , new VS_H225AddressPattern_Range	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225AddressPattern::operator=(const VS_H225AddressPattern & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_wildcard : CopyChoice< VS_H225AliasAddress >(src,*this); return;
		case e_range : CopyChoice<VS_H225AddressPattern_Range	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225AddressPattern::operator VS_H225AliasAddress *( void )
	{	return dynamic_cast< VS_H225AliasAddress * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225AddressPattern::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225AddressPattern::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_wildcard :  dprint4("\n\t choice: VS_H225AliasAddress ");return;
		case e_range :  dprint4("\n\t choice: VS_H225AddressPattern_Range	 ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

////////////////////////CLASS VS_H225AliasAddress /////////////////////////
// 	 VS_H225AliasAddress::VS_H225AliasAddress( void )
//	:VS_AsnChoice(2 , 7 , 1 )
//	{
//	}
//
//	 /////////////////////////////////////////////////////////////////////////////////////////
//	unsigned char VS_H225AliasAddress::dialedDigits_alphabet[]=
//	{'0','1','2','3','4','5','6','7','8','9','#','*',',' };
//	unsigned  VS_H225AliasAddress::dialedDigits_alphabet_size=sizeof(VS_H225AliasAddress::dialedDigits_alphabet);
//	unsigned char  VS_H225AliasAddress::dialedDigits_inverse_table[256]={0};
//	 const bool VS_H225AliasAddress::dialedDigits_flag_set_table =
//	 VS_AsnRestrictedString::MakeInverseCodeTable(
//		 VS_H225AliasAddress::dialedDigits_inverse_table,
//		 VS_H225AliasAddress::dialedDigits_alphabet,
//		 VS_H225AliasAddress::dialedDigits_alphabet_size );
//
//	 /////////////////////////////////////////////////////////////////////////////////////////
//
// /////////////////////////////////////////////////////////////////////////////////////////
//	bool VS_H225AliasAddress::Decode( VS_PerBuffer &buffer )
//	{
//		if (!buffer.ChoiceDecode(*this)) return false;
//		switch(tag)
//		{
//		case e_dialedDigits : return DecodeChoice( buffer , new TemplAlphabeticString< dialedDigits_alphabet, dialedDigits_alphabet_size,dialedDigits_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>  );
//		case e_h323_ID : return DecodeChoice( buffer , new TemplBmpString<1,256,VS_Asn::FixedConstraint,0>  );
//		case e_url_ID : return DecodeChoice( buffer , new TemplIA5String<1,512,VS_Asn::FixedConstraint,0>  );
//		case e_transportID : return DecodeChoice( buffer , new VS_H225TransportAddress);
//		case e_email_ID : return DecodeChoice( buffer , new TemplIA5String<1,512,VS_Asn::FixedConstraint,0>  );
//		case e_partyNumber : return DecodeChoice( buffer , new VS_H225PartyNumber);
//		case e_mobileUIM : return DecodeChoice( buffer , new VS_H225MobileUIM);
//		default: return buffer.ChoiceMissExtensionObject(*this);
//		}
//
//	}
//
//	void VS_H225AliasAddress::operator=(const VS_H225AliasAddress & src)
//	{
//		FreeChoice();
//		if (!src.filled) return;
//		switch(src.tag)
//		{
//		case e_dialedDigits : CopyChoice<TemplAlphabeticString< dialedDigits_alphabet, dialedDigits_alphabet_size,dialedDigits_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>   >(src,*this);  return;
//		case e_h323_ID : CopyChoice<TemplBmpString<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
//		case e_url_ID : CopyChoice<TemplIA5String<1,512,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
//		case e_transportID : CopyChoice< VS_H225TransportAddress >(src,*this); return;
//		case e_email_ID : CopyChoice<TemplIA5String<1,512,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
//		case e_partyNumber : CopyChoice< VS_H225PartyNumber >(src,*this); return;
//		case e_mobileUIM : CopyChoice< VS_H225MobileUIM >(src,*this); return;
//		default:		return;
//		}
//
//		return;
//	}
//
// /////////////////////////////////////////////////////////////////////////////////////////
//	VS_H225AliasAddress::operator VS_H225TransportAddress *( void )
//	{	return dynamic_cast< VS_H225TransportAddress * >(choice);    }
//
// 	VS_H225AliasAddress::operator VS_H225PartyNumber *( void )
//	{	return dynamic_cast< VS_H225PartyNumber * >(choice);    }
//
// 	VS_H225AliasAddress::operator VS_H225MobileUIM *( void )
//	{	return dynamic_cast< VS_H225MobileUIM * >(choice);    }
//
//
// /////////////////////////////////////////////////////////////////////////////////////////
//	void VS_H225AliasAddress::Show( void ) const
//	{
//		dprint4("\n\t----------- VS_H225AliasAddress::SHOW-----------");
//		if (!filled) return;
//		dprint4("Choice tag = %d ",tag);
//		switch(tag)
//		{
//		case e_dialedDigits :  dprint4("\n\t choice: TemplAlphabeticString< dialedDigits_alphabet, dialedDigits_alphabet_size,dialedDigits_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
//		case e_h323_ID :  dprint4("\n\t choice: TemplBmpString<1,256,VS_Asn::FixedConstraint,0>  ");return;
//		case e_url_ID :  dprint4("\n\t choice: TemplIA5String<1,512,VS_Asn::FixedConstraint,0>  ");return;
//		case e_transportID :  dprint4("\n\t choice: VS_H225TransportAddress ");return;
//		case e_email_ID :  dprint4("\n\t choice: TemplIA5String<1,512,VS_Asn::FixedConstraint,0>  ");return;
//		case e_partyNumber :  dprint4("\n\t choice: VS_H225PartyNumber ");return;
//		case e_mobileUIM :  dprint4("\n\t choice: VS_H225MobileUIM ");return;
//		default: dprint4("\n\t unknown choice: %d",tag); return ;
//		}
//
//	}


//////////////////////CLASS VS_H225AliasAddress /////////////////////////
 	 VS_H225AliasAddress::VS_H225AliasAddress( void )
	:VS_AsnChoice(2 , 7 , 1 )
	{
	}

	 /////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char VS_H225AliasAddress::dialedDigits_alphabet[];

	unsigned char  VS_H225AliasAddress::dialedDigits_inverse_table[256]={0};
	 const bool VS_H225AliasAddress::dialedDigits_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H225AliasAddress::dialedDigits_inverse_table,
		 VS_H225AliasAddress::dialedDigits_alphabet,
		 sizeof(VS_H225AliasAddress::dialedDigits_alphabet) );

	 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225AliasAddress::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_dialedDigits : return DecodeChoice(buffer, new TemplAlphabeticString<dialedDigits_alphabet, sizeof(dialedDigits_alphabet), dialedDigits_inverse_table, 1, 128, VS_Asn::FixedConstraint, 0>);
		case e_h323_ID : return DecodeChoice( buffer , new TemplBmpString<1,256,VS_Asn::FixedConstraint,0>  );
		case e_url_ID : return DecodeChoice( buffer , new TemplIA5String<1,512,VS_Asn::FixedConstraint,0>  );
		case e_transportID : return DecodeChoice( buffer , new VS_H225TransportAddress);
		case e_email_ID : return DecodeChoice( buffer , new TemplIA5String<1,512,VS_Asn::FixedConstraint,0>  );
		case e_partyNumber : return DecodeChoice( buffer , new VS_H225PartyNumber);
		case e_mobileUIM : return DecodeChoice( buffer , new VS_H225MobileUIM);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225AliasAddress::operator=(const VS_H225AliasAddress & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_dialedDigits : CopyChoice<TemplAlphabeticString<dialedDigits_alphabet, sizeof(dialedDigits_alphabet), dialedDigits_inverse_table, 1, 128, VS_Asn::FixedConstraint, 0>>(src, *this);  return;
		case e_h323_ID : CopyChoice<TemplBmpString<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_url_ID : CopyChoice<TemplIA5String<1,512,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_transportID : CopyChoice< VS_H225TransportAddress >(src,*this); return;
		case e_email_ID : CopyChoice<TemplIA5String<1,512,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_partyNumber : CopyChoice< VS_H225PartyNumber >(src,*this); return;
		case e_mobileUIM : CopyChoice< VS_H225MobileUIM >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225AliasAddress::operator VS_H225TransportAddress *( void )
	{	return dynamic_cast< VS_H225TransportAddress * >(choice);    }

 	VS_H225AliasAddress::operator VS_H225PartyNumber *( void )
	{	return dynamic_cast< VS_H225PartyNumber * >(choice);    }

 	VS_H225AliasAddress::operator VS_H225MobileUIM *( void )
	{	return dynamic_cast< VS_H225MobileUIM * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225AliasAddress::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225AliasAddress::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_dialedDigits :  dprint4("\n\t choice: TemplAlphabeticString< dialedDigits_alphabet, dialedDigits_alphabet_size,dialedDigits_inverse_table,1,128,VS_Asn::FixedConstraint,0>  ");return;
		case e_h323_ID :  dprint4("\n\t choice: TemplBmpString<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_url_ID :  dprint4("\n\t choice: TemplIA5String<1,512,VS_Asn::FixedConstraint,0>  ");return;
		case e_transportID :  dprint4("\n\t choice: VS_H225TransportAddress ");return;
		case e_email_ID :  dprint4("\n\t choice: TemplIA5String<1,512,VS_Asn::FixedConstraint,0>  ");return;
		case e_partyNumber :  dprint4("\n\t choice: VS_H225PartyNumber ");return;
		case e_mobileUIM :  dprint4("\n\t choice: VS_H225MobileUIM ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

	std::string VS_H225AliasAddress::String(void) const
	{
		switch (this->tag)
		{
		case VS_H225AliasAddress::e_h323_ID:
		{
			auto choice = static_cast<TemplBmpString<1, 256, VS_Asn::FixedConstraint, 0>*>(this->choice);
			return VS_H323String(choice->value).MakeString();
		}
		case VS_H225AliasAddress::e_dialedDigits:
		{
			VS_AsnIA5String* choice = static_cast<VS_AsnIA5String*>(this->choice);
			const auto len_buff = 256;
			char buff[len_buff];
			std::size_t sz = len_buff;
			if (choice->GetNormalString(buff, sz))
			{
				assert(sz != 0);
				return std::string{ buff, sz - 1 /*0-terminator*/ };
			}
			break;
		}
		default: break;
		}
		return {};
	}


//////////////////////CLASS VS_H225NonStandardIdentifier /////////////////////////
 	 VS_H225NonStandardIdentifier::VS_H225NonStandardIdentifier( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225NonStandardIdentifier::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_object : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		case e_h221NonStandard : return DecodeChoice( buffer , new VS_H225H221NonStandard);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225NonStandardIdentifier::operator=(const VS_H225NonStandardIdentifier & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_object : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		case e_h221NonStandard : CopyChoice< VS_H225H221NonStandard >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225NonStandardIdentifier::operator VS_H225H221NonStandard *( void )
	{	return dynamic_cast< VS_H225H221NonStandard * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225NonStandardIdentifier::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225NonStandardIdentifier::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_object :  dprint4("\n\t choice:  VS_AsnObjectId  ");return;
		case e_h221NonStandard :  dprint4("\n\t choice: VS_H225H221NonStandard ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225NonStandardParameter /////////////////////////
 	 VS_H225NonStandardParameter :: VS_H225NonStandardParameter( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , false )
	{
		ref[0].Set(&nonStandardIdentifier,0);
		ref[1].Set(&data,0);
	}
	void VS_H225NonStandardParameter::operator=(const VS_H225NonStandardParameter& src)
	{

		O_CC(filled);
		O_C(nonStandardIdentifier);
		O_C(data);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TunnelledProtocolAlternateIdentifier /////////////////////////
 	 VS_H225TunnelledProtocolAlternateIdentifier :: VS_H225TunnelledProtocolAlternateIdentifier( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&protocolType,0);
		ref[1].Set(&protocolVariant,1);
	}
	void VS_H225TunnelledProtocolAlternateIdentifier::operator=(const VS_H225TunnelledProtocolAlternateIdentifier& src)
	{

		O_CC(filled);
		O_C(protocolType);
		O_C(protocolVariant);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TunnelledProtocol_Id /////////////////////////
 	 VS_H225TunnelledProtocol_Id::VS_H225TunnelledProtocol_Id( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225TunnelledProtocol_Id::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_tunnelledProtocolObjectID : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		case e_tunnelledProtocolAlternateID : return DecodeChoice( buffer , new VS_H225TunnelledProtocolAlternateIdentifier);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225TunnelledProtocol_Id::operator=(const VS_H225TunnelledProtocol_Id & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_tunnelledProtocolObjectID : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		case e_tunnelledProtocolAlternateID : CopyChoice< VS_H225TunnelledProtocolAlternateIdentifier >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225TunnelledProtocol_Id::operator VS_H225TunnelledProtocolAlternateIdentifier *( void )
	{	return dynamic_cast< VS_H225TunnelledProtocolAlternateIdentifier * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225TunnelledProtocol_Id::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225TunnelledProtocol_Id::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_tunnelledProtocolObjectID :  dprint4("\n\t choice:  VS_AsnObjectId  ");return;
		case e_tunnelledProtocolAlternateID :  dprint4("\n\t choice: VS_H225TunnelledProtocolAlternateIdentifier ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225TunnelledProtocol /////////////////////////
 	 VS_H225TunnelledProtocol :: VS_H225TunnelledProtocol( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&id,0);
		ref[1].Set(&subIdentifier,1);
	}
	void VS_H225TunnelledProtocol::operator=(const VS_H225TunnelledProtocol& src)
	{

		O_CC(filled);
		O_C(id);
		O_C(subIdentifier);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H221NonStandard /////////////////////////
 	 VS_H225H221NonStandard :: VS_H225H221NonStandard( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&t35CountryCode,0);
		ref[1].Set(&t35Extension,0);
		ref[2].Set(&manufacturerCode,0);
	}
	void VS_H225H221NonStandard::operator=(const VS_H225H221NonStandard& src)
	{

		O_CC(filled);
		O_C(t35CountryCode);
		O_C(t35Extension);
		O_C(manufacturerCode);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225VendorIdentifier /////////////////////////
 	 VS_H225VendorIdentifier :: VS_H225VendorIdentifier( void )
	:VS_AsnSequence(2 , ref , basic_root,  nullptr, extension_root , true )
	{
		//ref[0].Set(&vendor,0);
		ref[0].Set(&h221NonStandard,0);
		ref[1].Set(&productId,1);
		ref[2].Set(&versionId,1);
	}
	void VS_H225VendorIdentifier::operator=(const VS_H225VendorIdentifier& src)
	{

		O_CC(filled);
		//O_C(vendor);
		O_C(h221NonStandard);
		O_C(productId);
		O_C(versionId);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225GatekeeperInfo /////////////////////////
 	 VS_H225GatekeeperInfo :: VS_H225GatekeeperInfo( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&nonStandardData,1);
	}
	void VS_H225GatekeeperInfo::operator=(const VS_H225GatekeeperInfo& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TerminalInfo /////////////////////////
 	 VS_H225TerminalInfo :: VS_H225TerminalInfo( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr, extension_root, true )
	{
		ref[0].Set(&nonStandardData,1);
	}
	void VS_H225TerminalInfo::operator=(const VS_H225TerminalInfo& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225McuInfo /////////////////////////
 	 VS_H225McuInfo :: VS_H225McuInfo( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&protocol,1);
	}
	void VS_H225McuInfo::operator=(const VS_H225McuInfo& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(protocol);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////


//////////////////////CLASS VS_H225T38FaxAnnexbOnlyCaps /////////////////////////
 	 VS_H225T38FaxAnnexbOnlyCaps :: VS_H225T38FaxAnnexbOnlyCaps( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr , extension_root , false)
	{
	}
	void VS_H225T38FaxAnnexbOnlyCaps::operator=(const VS_H225T38FaxAnnexbOnlyCaps& src)
	{

		O_CC(filled);
		O_CP(e_ref);
		O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////


//////////////////////CLASS VS_H225NonStandardProtocol /////////////////////////
 	 VS_H225NonStandardProtocol :: VS_H225NonStandardProtocol( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr, extension_root , true )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&dataRatesSupported,1);
		ref[2].Set(&supportedPrefixes,0);
	}
	void VS_H225NonStandardProtocol::operator=(const VS_H225NonStandardProtocol& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225T120OnlyCaps /////////////////////////
 	 VS_H225T120OnlyCaps :: VS_H225T120OnlyCaps( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&dataRatesSupported,1);
		e_ref[1].Set(&supportedPrefixes,0);
	}
	void VS_H225T120OnlyCaps::operator=(const VS_H225T120OnlyCaps& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225VoiceCaps /////////////////////////
 	 VS_H225VoiceCaps :: VS_H225VoiceCaps( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&dataRatesSupported,1);
		e_ref[1].Set(&supportedPrefixes,0);
	}
	void VS_H225VoiceCaps::operator=(const VS_H225VoiceCaps& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H324Caps /////////////////////////
 	 VS_H225H324Caps :: VS_H225H324Caps( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&dataRatesSupported,1);
		e_ref[1].Set(&supportedPrefixes,0);
	}
	void VS_H225H324Caps::operator=(const VS_H225H324Caps& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H323Caps /////////////////////////
 	 VS_H225H323Caps :: VS_H225H323Caps( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&dataRatesSupported,1);
		e_ref[1].Set(&supportedPrefixes,0);
	}
	void VS_H225H323Caps::operator=(const VS_H225H323Caps& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H322Caps /////////////////////////
 	 VS_H225H322Caps :: VS_H225H322Caps( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&dataRatesSupported,1);
		e_ref[1].Set(&supportedPrefixes,0);
	}
	void VS_H225H322Caps::operator=(const VS_H225H322Caps& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H321Caps /////////////////////////
 	 VS_H225H321Caps :: VS_H225H321Caps( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&dataRatesSupported,1);
		e_ref[1].Set(&supportedPrefixes,0);
	}
	void VS_H225H321Caps::operator=(const VS_H225H321Caps& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H320Caps /////////////////////////
 	 VS_H225H320Caps :: VS_H225H320Caps( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&dataRatesSupported,1);
		e_ref[1].Set(&supportedPrefixes,0);
	}
	void VS_H225H320Caps::operator=(const VS_H225H320Caps& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H310Caps /////////////////////////
 	 VS_H225H310Caps :: VS_H225H310Caps( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		e_ref[0].Set(&dataRatesSupported,1);
		e_ref[1].Set(&supportedPrefixes,0);
	}
	void VS_H225H310Caps::operator=(const VS_H225H310Caps& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(dataRatesSupported);
		O_C(supportedPrefixes);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225SupportedProtocols /////////////////////////
 	 VS_H225SupportedProtocols::VS_H225SupportedProtocols( void )
	:VS_AsnChoice(9 , 11 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225SupportedProtocols::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandardData : return DecodeChoice( buffer , new VS_H225NonStandardParameter);
		case e_h310 : return DecodeChoice( buffer , new VS_H225H310Caps);
		case e_h320 : return DecodeChoice( buffer , new VS_H225H320Caps);
		case e_h321 : return DecodeChoice( buffer , new VS_H225H321Caps);
		case e_h322 : return DecodeChoice( buffer , new VS_H225H322Caps);
		case e_h323 : return DecodeChoice( buffer , new VS_H225H323Caps);
		case e_h324 : return DecodeChoice( buffer , new VS_H225H324Caps);
		case e_voice : return DecodeChoice( buffer , new VS_H225VoiceCaps);
		case e_t120_only : return DecodeChoice( buffer , new VS_H225T120OnlyCaps);
		case e_nonStandardProtocol : return DecodeChoice( buffer , new VS_H225NonStandardProtocol);
		case e_t38FaxAnnexbOnly : return DecodeChoice( buffer , new VS_H225T38FaxAnnexbOnlyCaps);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225SupportedProtocols::operator=(const VS_H225SupportedProtocols & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandardData : CopyChoice< VS_H225NonStandardParameter >(src,*this); return;
		case e_h310 : CopyChoice< VS_H225H310Caps >(src,*this); return;
		case e_h320 : CopyChoice< VS_H225H320Caps >(src,*this); return;
		case e_h321 : CopyChoice< VS_H225H321Caps >(src,*this); return;
		case e_h322 : CopyChoice< VS_H225H322Caps >(src,*this); return;
		case e_h323 : CopyChoice< VS_H225H323Caps >(src,*this); return;
		case e_h324 : CopyChoice< VS_H225H324Caps >(src,*this); return;
		case e_voice : CopyChoice< VS_H225VoiceCaps >(src,*this); return;
		case e_t120_only : CopyChoice< VS_H225T120OnlyCaps >(src,*this); return;
		case e_nonStandardProtocol : CopyChoice< VS_H225NonStandardProtocol >(src,*this); return;
		case e_t38FaxAnnexbOnly : CopyChoice< VS_H225T38FaxAnnexbOnlyCaps >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225SupportedProtocols::operator VS_H225NonStandardParameter *( void )
	{	return dynamic_cast< VS_H225NonStandardParameter * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225H310Caps *( void )
	{	return dynamic_cast< VS_H225H310Caps * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225H320Caps *( void )
	{	return dynamic_cast< VS_H225H320Caps * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225H321Caps *( void )
	{	return dynamic_cast< VS_H225H321Caps * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225H322Caps *( void )
	{	return dynamic_cast< VS_H225H322Caps * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225H323Caps *( void )
	{	return dynamic_cast< VS_H225H323Caps * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225H324Caps *( void )
	{	return dynamic_cast< VS_H225H324Caps * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225VoiceCaps *( void )
	{	return dynamic_cast< VS_H225VoiceCaps * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225T120OnlyCaps *( void )
	{	return dynamic_cast< VS_H225T120OnlyCaps * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225NonStandardProtocol *( void )
	{	return dynamic_cast< VS_H225NonStandardProtocol * >(choice);    }

 	VS_H225SupportedProtocols::operator VS_H225T38FaxAnnexbOnlyCaps *( void )
	{	return dynamic_cast< VS_H225T38FaxAnnexbOnlyCaps * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225SupportedProtocols::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225SupportedProtocols::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_nonStandardData :  dprint4("\n\t choice: VS_H225NonStandardParameter ");return;
		case e_h310 :  dprint4("\n\t choice: VS_H225H310Caps ");return;
		case e_h320 :  dprint4("\n\t choice: VS_H225H320Caps ");return;
		case e_h321 :  dprint4("\n\t choice: VS_H225H321Caps ");return;
		case e_h322 :  dprint4("\n\t choice: VS_H225H322Caps ");return;
		case e_h323 :  dprint4("\n\t choice: VS_H225H323Caps ");return;
		case e_h324 :  dprint4("\n\t choice: VS_H225H324Caps ");return;
		case e_voice :  dprint4("\n\t choice: VS_H225VoiceCaps ");return;
		case e_t120_only :  dprint4("\n\t choice: VS_H225T120OnlyCaps ");return;
		case e_nonStandardProtocol :  dprint4("\n\t choice: VS_H225NonStandardProtocol ");return;
		case e_t38FaxAnnexbOnly :  dprint4("\n\t choice: VS_H225T38FaxAnnexbOnlyCaps ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225GatewayInfo /////////////////////////
 	 VS_H225GatewayInfo :: VS_H225GatewayInfo( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&protocol,1);
		ref[1].Set(&nonStandardData,1);
	}
	void VS_H225GatewayInfo::operator=(const VS_H225GatewayInfo& src)
	{

		O_CC(filled);
		O_C(protocol);
		O_C(nonStandardData);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225EndpointType /////////////////////////
 	 VS_H225EndpointType :: VS_H225EndpointType( void )
	:VS_AsnSequence(6 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&vendor,1);
		ref[2].Set(&gatekeeper,1);
		ref[3].Set(&gateway,1);
		ref[4].Set(&mcu,1);
		//ref[5].Set(&terminal,1);
		ref[5].Set(&terminalInfo,1);
		ref[6].Set(&mc,0);
		ref[7].Set(&undefinedNode,0);
		e_ref[0].Set(&set,1);
		e_ref[1].Set(&supportedTunnelledProtocols,1);
	}
	void VS_H225EndpointType::operator=(const VS_H225EndpointType& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(vendor);
		O_C(gatekeeper);
		O_C(gateway);
		O_C(mcu);
		//O_C(terminal);
		O_C(terminalInfo);
		O_C(mc);
		O_C(undefinedNode);
		O_C(set);
		O_C(supportedTunnelledProtocols);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Notify_UUIE /////////////////////////
 	 VS_H225Notify_UUIE :: VS_H225Notify_UUIE( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&callIdentifier,0);
		ref[2].Set(&tokens,1);
		ref[3].Set(&cryptoTokens,1);
	}
	void VS_H225Notify_UUIE::operator=(const VS_H225Notify_UUIE& src)
	{

		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(callIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225SetupAcknowledge_UUIE /////////////////////////
 	 VS_H225SetupAcknowledge_UUIE :: VS_H225SetupAcknowledge_UUIE( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&callIdentifier,0);
		ref[2].Set(&tokens,1);
		ref[3].Set(&cryptoTokens,1);
	}
	void VS_H225SetupAcknowledge_UUIE::operator=(const VS_H225SetupAcknowledge_UUIE& src)
	{

		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(callIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225StatusInquiry_UUIE /////////////////////////
 	 VS_H225StatusInquiry_UUIE :: VS_H225StatusInquiry_UUIE( void )
//TODO:may be bug
	:VS_AsnSequence(2 , ref , basic_root, /*e_ref*/ nullptr , extension_root , true )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&callIdentifier,0);
		ref[2].Set(&tokens,1);
		ref[3].Set(&cryptoTokens,1);
	}
	void VS_H225StatusInquiry_UUIE::operator=(const VS_H225StatusInquiry_UUIE& src)
	{

		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(callIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Status_UUIE /////////////////////////
 	 VS_H225Status_UUIE :: VS_H225Status_UUIE( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&callIdentifier,0);
		ref[2].Set(&tokens,1);
		ref[3].Set(&cryptoTokens,1);
	}
	void VS_H225Status_UUIE::operator=(const VS_H225Status_UUIE& src)
	{

		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(callIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TransportAddress_IpAddress /////////////////////////
 	 VS_H225TransportAddress_IpAddress :: VS_H225TransportAddress_IpAddress( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , false )
	{
		ref[0].Set(&ip,0);
		ref[1].Set(&port,0);
	}
	void VS_H225TransportAddress_IpAddress::operator=(const VS_H225TransportAddress_IpAddress& src)
	{

		O_CC(filled);
		O_C(ip);
		O_C(port);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TransportAddress_IpSourceRoute_Routing /////////////////////////
 	 VS_H225TransportAddress_IpSourceRoute_Routing::VS_H225TransportAddress_IpSourceRoute_Routing( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225TransportAddress_IpSourceRoute_Routing::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_strict : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_loose : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225TransportAddress_IpSourceRoute_Routing::operator=(const VS_H225TransportAddress_IpSourceRoute_Routing & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_strict : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_loose : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225TransportAddress_IpSourceRoute_Routing::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225TransportAddress_IpSourceRoute_Routing::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_strict :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_loose :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225TransportAddress_IpSourceRoute /////////////////////////
 	 VS_H225TransportAddress_IpSourceRoute :: VS_H225TransportAddress_IpSourceRoute( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&ip,0);
		ref[1].Set(&port,0);
		ref[2].Set(&route,0);
		ref[3].Set(&routing,0);
	}
	void VS_H225TransportAddress_IpSourceRoute::operator=(const VS_H225TransportAddress_IpSourceRoute& src)
	{

		O_CC(filled);
		O_C(ip);
		O_C(port);
		O_C(route);
		O_C(routing);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TransportAddress_IpxAddress /////////////////////////
 	 VS_H225TransportAddress_IpxAddress :: VS_H225TransportAddress_IpxAddress( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , false )
	{
		ref[0].Set(&node,0);
		ref[1].Set(&netnum,0);
		ref[2].Set(&port,0);
	}
	void VS_H225TransportAddress_IpxAddress::operator=(const VS_H225TransportAddress_IpxAddress& src)
	{

		O_CC(filled);
		O_C(node);
		O_C(netnum);
		O_C(port);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TransportAddress_Ip6Address /////////////////////////
 	 VS_H225TransportAddress_Ip6Address :: VS_H225TransportAddress_Ip6Address( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&ip,0);
		ref[1].Set(&port,0);
	}
	void VS_H225TransportAddress_Ip6Address::operator=(const VS_H225TransportAddress_Ip6Address& src)
	{

		O_CC(filled);
		O_C(ip);
		O_C(port);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225TransportAddress /////////////////////////
 	 VS_H225TransportAddress::VS_H225TransportAddress( void )
	:VS_AsnChoice(7 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225TransportAddress::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_ipAddress : return DecodeChoice( buffer , new VS_H225TransportAddress_IpAddress	 );
		case e_ipSourceRoute : return DecodeChoice( buffer , new VS_H225TransportAddress_IpSourceRoute	 );
		case e_ipxAddress : return DecodeChoice( buffer , new VS_H225TransportAddress_IpxAddress	 );
		case e_ip6Address : return DecodeChoice( buffer , new VS_H225TransportAddress_Ip6Address	 );
		case e_netBios : return DecodeChoice( buffer , new TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  );
		case e_nsap : return DecodeChoice( buffer , new TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  );
		case e_nonStandardAddress : return DecodeChoice( buffer , new VS_H225NonStandardParameter);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}
	void VS_H225TransportAddress::operator=(VS_H225TransportAddress_IpAddress *src)
	{
		FreeChoice();
		if (!src || !src->filled)	return;
		choice = src;		tag = e_ipAddress;
		filled = true;
	}
	bool VS_H225TransportAddress::operator==(const VS_H225TransportAddress &src )
	{
		if((filled==src.filled) && (tag==src.tag))
			switch (src.tag)
			{
			case e_ipAddress:			return (*(static_cast<VS_H225TransportAddress_IpAddress*>(choice))==*(static_cast<VS_H225TransportAddress_IpAddress*>(src.choice)));
			case e_ipSourceRoute:		return false;
			case e_ipxAddress:			return false;
			case e_ip6Address:			return false;
			case e_netBios:				return false;
			case e_nsap:				return false;
			case e_nonStandardAddress:	return false;
			default : return false;
			}
		return false;
	}
	void VS_H225TransportAddress::operator=(const VS_H225TransportAddress & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_ipAddress : CopyChoice<VS_H225TransportAddress_IpAddress	  >(src,*this);  return;
		case e_ipSourceRoute : CopyChoice<VS_H225TransportAddress_IpSourceRoute	  >(src,*this);  return;
		case e_ipxAddress : CopyChoice<VS_H225TransportAddress_IpxAddress	  >(src,*this);  return;
		case e_ip6Address : CopyChoice<VS_H225TransportAddress_Ip6Address	  >(src,*this);  return;
		case e_netBios : CopyChoice<TemplOctetString<16,16,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_nsap : CopyChoice<TemplOctetString<1,20,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_nonStandardAddress : CopyChoice< VS_H225NonStandardParameter >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225TransportAddress::operator VS_H225NonStandardParameter *( void )
	{	return dynamic_cast< VS_H225NonStandardParameter * >(choice);    }

	VS_H225TransportAddress::operator VS_H225TransportAddress_IpAddress*( void )
	{	return dynamic_cast<VS_H225TransportAddress_IpAddress *>(choice); 	}
	// end VS_H225TransportAddress::operator VS_H225IpAddress *

	VS_H225TransportAddress::operator VS_H225TransportAddress_Ip6Address*( void )
	{	return dynamic_cast<VS_H225TransportAddress_Ip6Address *>(choice); 	}
	// end VS_H225TransportAddress::operator VS_H225Ip6Address *

	VS_H225TransportAddress::operator VS_H225TransportAddress_IpxAddress*( void )
	{	return dynamic_cast<VS_H225TransportAddress_IpxAddress *>(choice); 	}
	// end VS_H225TransportAddress::operator VS_H225IpAddress *

	VS_H225TransportAddress::operator VS_AsnOctetString*( void )
	{	return dynamic_cast<VS_AsnOctetString *>(choice);	}
	// end VS_H225TransportAddress::operator VS_AsnOctetString *

	VS_H225TransportAddress::operator VS_H225TransportAddress_IpSourceRoute*( void )
	{	return dynamic_cast<VS_H225TransportAddress_IpSourceRoute *>(choice);	}
	// end VS_H225TransportAddress::operator VS_H225IpSourceRoute *

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225TransportAddress::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225TransportAddress::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_ipAddress :  dprint4("\n\t choice: VS_H225TransportAddress_IpAddress	 ");return;
		case e_ipSourceRoute :  dprint4("\n\t choice: VS_H225TransportAddress_IpSourceRoute	 ");return;
		case e_ipxAddress :  dprint4("\n\t choice: VS_H225TransportAddress_IpxAddress	 ");return;
		case e_ip6Address :  dprint4("\n\t choice: VS_H225TransportAddress_Ip6Address	 ");return;
		case e_netBios :  dprint4("\n\t choice: TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  ");return;
		case e_nsap :  dprint4("\n\t choice: TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  ");return;
		case e_nonStandardAddress :  dprint4("\n\t choice: VS_H225NonStandardParameter ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225Progress_UUIE /////////////////////////
 	 VS_H225Progress_UUIE :: VS_H225Progress_UUIE( void )
	:VS_AsnSequence(5 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&destinationInfo,0);
		ref[2].Set(&h245Address,1);
		ref[3].Set(&callIdentifier,0);
		ref[4].Set(&h245SecurityMode,1);
		ref[5].Set(&tokens,1);
		ref[6].Set(&cryptoTokens,1);
		ref[7].Set(&fastStart,1);
		e_ref[0].Set(&multipleCalls,0);
		e_ref[1].Set(&maintainConnection,0);
		e_ref[2].Set(&fastConnectRefused,1);
	}
	void VS_H225Progress_UUIE::operator=(const VS_H225Progress_UUIE& src)
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
		O_C(fastConnectRefused);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225FacilityReason /////////////////////////
 	 VS_H225FacilityReason::VS_H225FacilityReason( void )
	:VS_AsnChoice(4 , 11 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225FacilityReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_routeCallToGatekeeper : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_callForwarded : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_routeCallToMC : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_conferenceListChoice : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_startH245 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_noH245 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_newTokens : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_featureSetUpdate : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_forwardedElements : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transportedInformation : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225FacilityReason::operator=(const VS_H225FacilityReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_routeCallToGatekeeper : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_callForwarded : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_routeCallToMC : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_conferenceListChoice : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_startH245 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_noH245 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_newTokens : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_featureSetUpdate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_forwardedElements : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transportedInformation : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225FacilityReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225FacilityReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_routeCallToGatekeeper :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_callForwarded :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_routeCallToMC :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_conferenceListChoice :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_startH245 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_noH245 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_newTokens :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_featureSetUpdate :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_forwardedElements :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_transportedInformation :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225ConferenceList /////////////////////////
 	 VS_H225ConferenceList :: VS_H225ConferenceList( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&conferenceID,1);
		ref[1].Set(&conferenceAlias,1);
		ref[2].Set(&nonStandardData,1);
	}
	void VS_H225ConferenceList::operator=(const VS_H225ConferenceList& src)
	{

		O_CC(filled);
		O_C(conferenceID);
		O_C(conferenceAlias);
		O_C(nonStandardData);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Facility_UUIE /////////////////////////
 	 VS_H225Facility_UUIE :: VS_H225Facility_UUIE( void )
	:VS_AsnSequence(3 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&protocolIdentifier,0);
		ref[1].Set(&alternativeAddress,1);
		ref[2].Set(&alternativeAliasAddress,1);
		ref[3].Set(&conferenceID,1);
		ref[4].Set(&reason,0);
		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&destExtraCallInfo,1);
		e_ref[2].Set(&remoteExtensionAddress,1);
		e_ref[3].Set(&tokens,1);
		e_ref[4].Set(&cryptoTokens,1);
		e_ref[5].Set(&conferences,1);
		e_ref[6].Set(&h245Address,1);
		e_ref[7].Set(&fastStart,1);
		e_ref[8].Set(&multipleCalls,0);
		e_ref[9].Set(&maintainConnection,0);
		e_ref[10].Set(&fastConnectRefused,1);
		e_ref[11].Set(&serviceControl,1);
		e_ref[12].Set(&circuitInfo,1);
		e_ref[13].Set(&featureSet,1);
		e_ref[14].Set(&destinationInfo,1);
		e_ref[15].Set(&h245SecurityMode,1);
	}
	void VS_H225Facility_UUIE::operator=(const VS_H225Facility_UUIE& src)
	{

		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(alternativeAddress);
		O_C(alternativeAliasAddress);
		O_C(conferenceID);
		O_C(reason);
		O_C(callIdentifier);
		O_C(destExtraCallInfo);
		O_C(remoteExtensionAddress);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(conferences);
		O_C(h245Address);
		O_C(fastStart);
		O_C(multipleCalls);
		O_C(maintainConnection);
		O_C(fastConnectRefused);
		O_C(serviceControl);
		O_C(circuitInfo);
		O_C(featureSet);
		O_C(destinationInfo);
		O_C(h245SecurityMode);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225PresentationIndicator /////////////////////////
 	 VS_H225PresentationIndicator::VS_H225PresentationIndicator( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225PresentationIndicator::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_presentationAllowed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_presentationRestricted : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_addressNotAvailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225PresentationIndicator::operator=(const VS_H225PresentationIndicator & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_presentationAllowed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_presentationRestricted : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_addressNotAvailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225PresentationIndicator::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225PresentationIndicator::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_presentationAllowed :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_presentationRestricted :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_addressNotAvailable :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225ScnConnectionAggregation /////////////////////////
 	 VS_H225ScnConnectionAggregation::VS_H225ScnConnectionAggregation( void )
	:VS_AsnChoice(6 , 6 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225ScnConnectionAggregation::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_auto : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_none : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_h221 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_bonded_mode1 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_bonded_mode2 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_bonded_mode3 : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225ScnConnectionAggregation::operator=(const VS_H225ScnConnectionAggregation & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_auto : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_none : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_h221 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_bonded_mode1 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_bonded_mode2 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_bonded_mode3 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225ScnConnectionAggregation::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225ScnConnectionAggregation::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_auto :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_none :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_h221 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_bonded_mode1 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_bonded_mode2 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_bonded_mode3 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225ScnConnectionType /////////////////////////
 	 VS_H225ScnConnectionType::VS_H225ScnConnectionType( void )
	:VS_AsnChoice(7 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225ScnConnectionType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unknown : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_bChannel : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_hybrid2x64 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_hybrid384 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_hybrid1536 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_hybrid1920 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_multirate : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225ScnConnectionType::operator=(const VS_H225ScnConnectionType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unknown : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_bChannel : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_hybrid2x64 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_hybrid384 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_hybrid1536 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_hybrid1920 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_multirate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225ScnConnectionType::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225ScnConnectionType::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_unknown :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_bChannel :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_hybrid2x64 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_hybrid384 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_hybrid1536 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_hybrid1920 :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_multirate :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225Setup_UUIE_ConferenceGoal /////////////////////////
 	 VS_H225Setup_UUIE_ConferenceGoal::VS_H225Setup_UUIE_ConferenceGoal( void )
	:VS_AsnChoice(3 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225Setup_UUIE_ConferenceGoal::Decode( VS_PerBuffer &buffer )
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

	void VS_H225Setup_UUIE_ConferenceGoal::operator=(const VS_H225Setup_UUIE_ConferenceGoal & src)
	{
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

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225Setup_UUIE_ConferenceGoal::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225Setup_UUIE_ConferenceGoal::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_create :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_join :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invite :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_capability_negotiation :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_callIndependentSupplementaryService :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225Setup_UUIE_ConnectionParameters /////////////////////////
 	 VS_H225Setup_UUIE_ConnectionParameters :: VS_H225Setup_UUIE_ConnectionParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&connectionType,0);
		ref[1].Set(&numberOfScnConnections,0);
		ref[2].Set(&connectionAggregation,0);
	}
	void VS_H225Setup_UUIE_ConnectionParameters::operator=(const VS_H225Setup_UUIE_ConnectionParameters& src)
	{

		O_CC(filled);
		O_C(connectionType);
		O_C(numberOfScnConnections);
		O_C(connectionAggregation);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Setup_UUIE /////////////////////////
 	 VS_H225Setup_UUIE :: VS_H225Setup_UUIE( void )
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
	void VS_H225Setup_UUIE::operator=(const VS_H225Setup_UUIE& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225ReleaseCompleteReason /////////////////////////
 	 VS_H225ReleaseCompleteReason::VS_H225ReleaseCompleteReason( void )
	:VS_AsnChoice(12 , 22 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225ReleaseCompleteReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noBandwidth : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_gatekeeperResources : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_unreachableDestination : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_destinationRejection : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidRevision : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_noPermission : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_unreachableGatekeeper : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_gatewayResources : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_badFormatAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_adaptiveBusy : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_inConf : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_facilityCallDeflection : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenied : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_calledPartyNotRegistered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_callerNotRegistered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_newConnectionNeeded : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_nonStandardReason : return DecodeChoice( buffer , new VS_H225NonStandardParameter);
		case e_replaceWithConferenceInvite : return DecodeChoice( buffer , new VS_H225ConferenceIdentifier);
		case e_genericDataReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_neededFeatureNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_tunnelledSignallingRejected : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225ReleaseCompleteReason::operator=(const VS_H225ReleaseCompleteReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noBandwidth : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_gatekeeperResources : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_unreachableDestination : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_destinationRejection : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidRevision : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_noPermission : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_unreachableGatekeeper : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_gatewayResources : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_badFormatAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_adaptiveBusy : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_inConf : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_facilityCallDeflection : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenied : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_calledPartyNotRegistered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_callerNotRegistered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_newConnectionNeeded : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_nonStandardReason : CopyChoice< VS_H225NonStandardParameter >(src,*this); return;
		case e_replaceWithConferenceInvite : CopyChoice< VS_H225ConferenceIdentifier >(src,*this); return;
		case e_genericDataReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_neededFeatureNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_tunnelledSignallingRejected : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225ReleaseCompleteReason::operator VS_H225NonStandardParameter *( void )
	{	return dynamic_cast< VS_H225NonStandardParameter * >(choice);    }

 	VS_H225ReleaseCompleteReason::operator VS_H225ConferenceIdentifier *( void )
	{	return dynamic_cast< VS_H225ConferenceIdentifier * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225ReleaseCompleteReason::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225ReleaseCompleteReason::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_noBandwidth :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_gatekeeperResources :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_unreachableDestination :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_destinationRejection :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_invalidRevision :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_noPermission :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_unreachableGatekeeper :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_gatewayResources :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_badFormatAddress :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_adaptiveBusy :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_inConf :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_undefinedReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_facilityCallDeflection :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_securityDenied :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_calledPartyNotRegistered :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_callerNotRegistered :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_newConnectionNeeded :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_nonStandardReason :  dprint4("\n\t choice: VS_H225NonStandardParameter ");return;
		case e_replaceWithConferenceInvite :  dprint4("\n\t choice: VS_H225ConferenceIdentifier ");return;
		case e_genericDataReason :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_neededFeatureNotSupported :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_tunnelledSignallingRejected :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225ReleaseComplete_UUIE /////////////////////////
 	 VS_H225ReleaseComplete_UUIE :: VS_H225ReleaseComplete_UUIE( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
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
	void VS_H225ReleaseComplete_UUIE::operator=(const VS_H225ReleaseComplete_UUIE& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Information_UUIE /////////////////////////
 	 VS_H225Information_UUIE :: VS_H225Information_UUIE( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&protocolIdentifier,0);
		e_ref[0].Set(&callIdentifier,0);
		e_ref[1].Set(&tokens,1);
		e_ref[2].Set(&cryptoTokens,1);
		e_ref[3].Set(&fastStart,1);
		e_ref[4].Set(&fastConnectRefused,1);
		e_ref[5].Set(&circuitInfo,1);
	}
	void VS_H225Information_UUIE::operator=(const VS_H225Information_UUIE& src)
	{

		O_CC(filled);
		O_C(protocolIdentifier);
		O_C(callIdentifier);
		O_C(tokens);
		O_C(cryptoTokens);
		O_C(fastStart);
		O_C(fastConnectRefused);
		O_C(circuitInfo);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Connect_UUIE /////////////////////////
 	 VS_H225Connect_UUIE :: VS_H225Connect_UUIE( void )
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
	void VS_H225Connect_UUIE::operator=(const VS_H225Connect_UUIE& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225CallProceeding_UUIE /////////////////////////
 	 VS_H225CallProceeding_UUIE :: VS_H225CallProceeding_UUIE( void )
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
		e_ref[7].Set(&fastConnectRefused,1);
		e_ref[8].Set(&featureSet,1);
	}
	void VS_H225CallProceeding_UUIE::operator=(const VS_H225CallProceeding_UUIE& src)
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
		O_C(fastConnectRefused);
		O_C(featureSet);
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225Alerting_UUIE /////////////////////////
 	 VS_H225Alerting_UUIE :: VS_H225Alerting_UUIE( void )
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
	void VS_H225Alerting_UUIE::operator=(const VS_H225Alerting_UUIE& src)
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225StimulusControl /////////////////////////
 	 VS_H225StimulusControl :: VS_H225StimulusControl( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandard,1);
		ref[1].Set(&isText,1);
		ref[2].Set(&h248Message,1);
	}
	void VS_H225StimulusControl::operator=(const VS_H225StimulusControl& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_C(isText);
		O_C(h248Message);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H323_UU_PDU_H323_message_body /////////////////////////
 	 VS_H225H323_UU_PDU_H323_message_body::VS_H225H323_UU_PDU_H323_message_body( void )
	:VS_AsnChoice(7 , 13 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H225H323_UU_PDU_H323_message_body::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_setup : return DecodeChoice( buffer , new VS_H225Setup_UUIE);
		case e_callProceeding : return DecodeChoice( buffer , new VS_H225CallProceeding_UUIE);
		case e_connect : return DecodeChoice( buffer , new VS_H225Connect_UUIE);
		case e_alerting : return DecodeChoice( buffer , new VS_H225Alerting_UUIE);
		case e_information : return DecodeChoice( buffer , new VS_H225Information_UUIE);
		case e_releaseComplete : return DecodeChoice( buffer , new VS_H225ReleaseComplete_UUIE);
		case e_facility : return DecodeChoice( buffer , new VS_H225Facility_UUIE);
		case e_progress : return DecodeChoice( buffer , new VS_H225Progress_UUIE);
		case e_empty : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_status : return DecodeChoice( buffer , new VS_H225Status_UUIE);
		case e_statusInquiry : return DecodeChoice( buffer , new VS_H225StatusInquiry_UUIE);
		case e_setupAcknowledge : return DecodeChoice( buffer , new VS_H225SetupAcknowledge_UUIE);
		case e_notify : return DecodeChoice( buffer , new VS_H225Notify_UUIE);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H225H323_UU_PDU_H323_message_body::operator=(const VS_H225H323_UU_PDU_H323_message_body & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_setup : CopyChoice< VS_H225Setup_UUIE >(src,*this); return;
		case e_callProceeding : CopyChoice< VS_H225CallProceeding_UUIE >(src,*this); return;
		case e_connect : CopyChoice< VS_H225Connect_UUIE >(src,*this); return;
		case e_alerting : CopyChoice< VS_H225Alerting_UUIE >(src,*this); return;
		case e_information : CopyChoice< VS_H225Information_UUIE >(src,*this); return;
		case e_releaseComplete : CopyChoice< VS_H225ReleaseComplete_UUIE >(src,*this); return;
		case e_facility : CopyChoice< VS_H225Facility_UUIE >(src,*this); return;
		case e_progress : CopyChoice< VS_H225Progress_UUIE >(src,*this); return;
		case e_empty : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_status : CopyChoice< VS_H225Status_UUIE >(src,*this); return;
		case e_statusInquiry : CopyChoice< VS_H225StatusInquiry_UUIE >(src,*this); return;
		case e_setupAcknowledge : CopyChoice< VS_H225SetupAcknowledge_UUIE >(src,*this); return;
		case e_notify : CopyChoice< VS_H225Notify_UUIE >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225Setup_UUIE *( void )
	{	return dynamic_cast< VS_H225Setup_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225CallProceeding_UUIE *( void )
	{	return dynamic_cast< VS_H225CallProceeding_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225Connect_UUIE *( void )
	{	return dynamic_cast< VS_H225Connect_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225Alerting_UUIE *( void )
	{	return dynamic_cast< VS_H225Alerting_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225Information_UUIE *( void )
	{	return dynamic_cast< VS_H225Information_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225ReleaseComplete_UUIE *( void )
	{	return dynamic_cast< VS_H225ReleaseComplete_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225Facility_UUIE *( void )
	{	return dynamic_cast< VS_H225Facility_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225Progress_UUIE *( void )
	{	return dynamic_cast< VS_H225Progress_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225Status_UUIE *( void )
	{	return dynamic_cast< VS_H225Status_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225StatusInquiry_UUIE *( void )
	{	return dynamic_cast< VS_H225StatusInquiry_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225SetupAcknowledge_UUIE *( void )
	{	return dynamic_cast< VS_H225SetupAcknowledge_UUIE * >(choice);    }

 	VS_H225H323_UU_PDU_H323_message_body::operator VS_H225Notify_UUIE *( void )
	{	return dynamic_cast< VS_H225Notify_UUIE * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H225H323_UU_PDU_H323_message_body::Show( void ) const
	{
		dprint4("\n\t----------- VS_H225H323_UU_PDU_H323_message_body::SHOW-----------");
		if (!filled) return;
		dprint4("Choice tag = %u ",tag);
		switch(tag)
		{
		case e_setup :  dprint4("\n\t choice: VS_H225Setup_UUIE ");return;
		case e_callProceeding :  dprint4("\n\t choice: VS_H225CallProceeding_UUIE ");return;
		case e_connect :  dprint4("\n\t choice: VS_H225Connect_UUIE ");return;
		case e_alerting :  dprint4("\n\t choice: VS_H225Alerting_UUIE ");return;
		case e_information :  dprint4("\n\t choice: VS_H225Information_UUIE ");return;
		case e_releaseComplete :  dprint4("\n\t choice: VS_H225ReleaseComplete_UUIE ");return;
		case e_facility :  dprint4("\n\t choice: VS_H225Facility_UUIE ");return;
		case e_progress :  dprint4("\n\t choice: VS_H225Progress_UUIE ");return;
		case e_empty :  dprint4("\n\t choice:  VS_AsnNull  ");return;
		case e_status :  dprint4("\n\t choice: VS_H225Status_UUIE ");return;
		case e_statusInquiry :  dprint4("\n\t choice: VS_H225StatusInquiry_UUIE ");return;
		case e_setupAcknowledge :  dprint4("\n\t choice: VS_H225SetupAcknowledge_UUIE ");return;
		case e_notify :  dprint4("\n\t choice: VS_H225Notify_UUIE ");return;
		default: dprint4("\n\t unknown choice: %u",tag); return ;
		}

	}

//////////////////////CLASS VS_H225H323_UU_PDU_TunnelledSignallingMessage /////////////////////////
 	 VS_H225H323_UU_PDU_TunnelledSignallingMessage :: VS_H225H323_UU_PDU_TunnelledSignallingMessage( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&tunnelledProtocolID,0);
		ref[1].Set(&messageContent,0);
		ref[2].Set(&tunnellingRequired,1);
		ref[3].Set(&nonStandardData,1);
	}
	void VS_H225H323_UU_PDU_TunnelledSignallingMessage::operator=(const VS_H225H323_UU_PDU_TunnelledSignallingMessage& src)
	{

		O_CC(filled);
		O_C(tunnelledProtocolID);
		O_C(messageContent);
		O_C(tunnellingRequired);
		O_C(nonStandardData);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H323_UU_PDU /////////////////////////
 	 VS_H225H323_UU_PDU :: VS_H225H323_UU_PDU( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&h323_message_body,0);
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
	void VS_H225H323_UU_PDU::operator=(const VS_H225H323_UU_PDU& src)
	{

		O_CC(filled);
		O_C(h323_message_body);
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
		O_CSA(e_ref, extension_root);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H323_UserInformation_User_data /////////////////////////
 	 VS_H225H323_UserInformation_User_data :: VS_H225H323_UserInformation_User_data( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , true )
	{
		ref[0].Set(&protocol_discriminator,0);
		ref[1].Set(&user_information,0);
	}
	void VS_H225H323_UserInformation_User_data::operator=(const VS_H225H323_UserInformation_User_data& src)
	{

		O_CC(filled);
		O_C(protocol_discriminator);
		O_C(user_information);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H225H323_UserInformation /////////////////////////
 	 VS_H225H323_UserInformation :: VS_H225H323_UserInformation( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr, extension_root , true )
	{
		ref[0].Set(&h323_uu_pdu,0);
		ref[1].Set(&user_data,1);
	}
	void VS_H225H323_UserInformation::operator=(const VS_H225H323_UserInformation& src)
	{

		O_CC(filled);
		O_C(h323_uu_pdu);
		O_C(user_data);
		O_CP(e_ref);
		O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////
#undef DEBUG_CURRENT_MODULE