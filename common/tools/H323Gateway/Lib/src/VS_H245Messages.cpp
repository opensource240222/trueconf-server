/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 15.01.04     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_H245Messages.cpp
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#include "VS_H245Messages.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER

/////////////////////////////////////////////////////////////////////////////////////////



//////////////////////CLASS VS_H245MobileMultilinkReconfigurationIndication /////////////////////////
 	 VS_H245MobileMultilinkReconfigurationIndication :: VS_H245MobileMultilinkReconfigurationIndication( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sampleSize,0);
		ref[1].Set(&samplesPerFrame,0);
	}
	void VS_H245MobileMultilinkReconfigurationIndication::operator=(const VS_H245MobileMultilinkReconfigurationIndication& src)
	{

		O_CC(filled);
		O_C(sampleSize);
		O_C(samplesPerFrame);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245FlowControlIndication_Scope /////////////////////////
 	 VS_H245FlowControlIndication_Scope::VS_H245FlowControlIndication_Scope( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FlowControlIndication_Scope::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_logicalChannelNumber : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_resourceID : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		case e_wholeMultiplex : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245FlowControlIndication_Scope::operator=(const VS_H245FlowControlIndication_Scope & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_logicalChannelNumber : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_resourceID : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_wholeMultiplex : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245FlowControlIndication_Scope::operator VS_H245LogicalChannelNumber *( void )
	{	return dynamic_cast< VS_H245LogicalChannelNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FlowControlIndication_Scope::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_logicalChannelNumber :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_resourceID :  dprint4("choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_wholeMultiplex :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FlowControlIndication_Restriction /////////////////////////
 	 VS_H245FlowControlIndication_Restriction::VS_H245FlowControlIndication_Restriction( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FlowControlIndication_Restriction::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_maximumBitRate : return DecodeChoice( buffer , new TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>  );
		case e_noRestriction : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245FlowControlIndication_Restriction::operator=(const VS_H245FlowControlIndication_Restriction & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_maximumBitRate : CopyChoice<TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_noRestriction : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FlowControlIndication_Restriction::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_maximumBitRate :  dprint4("choice: TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>  ");return;
		case e_noRestriction :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FlowControlIndication /////////////////////////
 	 VS_H245FlowControlIndication :: VS_H245FlowControlIndication( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&scope,0);
		ref[1].Set(&restriction,0);
	}
	void VS_H245FlowControlIndication::operator=(const VS_H245FlowControlIndication& src)
	{

		O_CC(filled);
		O_C(scope);
		O_C(restriction);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputIndication_UserInputSupportIndication /////////////////////////
 	 VS_H245UserInputIndication_UserInputSupportIndication::VS_H245UserInputIndication_UserInputSupportIndication( void )
	:VS_AsnChoice(4 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245UserInputIndication_UserInputSupportIndication::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_basicString : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_iA5String : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_generalString : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_encryptedBasicString : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_encryptedIA5String : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_encryptedGeneralString : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245UserInputIndication_UserInputSupportIndication::operator=(const VS_H245UserInputIndication_UserInputSupportIndication & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_basicString : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_iA5String : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_generalString : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_encryptedBasicString : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_encryptedIA5String : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_encryptedGeneralString : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245UserInputIndication_UserInputSupportIndication::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245UserInputIndication_UserInputSupportIndication::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_basicString :  dprint4("choice:  VS_AsnNull  ");return;
		case e_iA5String :  dprint4("choice:  VS_AsnNull  ");return;
		case e_generalString :  dprint4("choice:  VS_AsnNull  ");return;
		case e_encryptedBasicString :  dprint4("choice:  VS_AsnNull  ");return;
		case e_encryptedIA5String :  dprint4("choice:  VS_AsnNull  ");return;
		case e_encryptedGeneralString :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245UserInputIndication_Signal_Rtp /////////////////////////
 	 VS_H245UserInputIndication_Signal_Rtp :: VS_H245UserInputIndication_Signal_Rtp( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&timestamp,1);
		ref[1].Set(&expirationTime,1);
		ref[2].Set(&logicalChannelNumber,0);
	}
	void VS_H245UserInputIndication_Signal_Rtp::operator=(const VS_H245UserInputIndication_Signal_Rtp& src)
	{

		O_CC(filled);
		O_C(timestamp);
		O_C(expirationTime);
		O_C(logicalChannelNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputIndication_Signal /////////////////////////
 	 VS_H245UserInputIndication_Signal :: VS_H245UserInputIndication_Signal( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&signalType,0);
		ref[1].Set(&duration,1);
		ref[2].Set(&rtp,1);
		e_ref[0].Set(&rtpPayloadIndication,1);
		e_ref[1].Set(&paramS,1);
		e_ref[2].Set(&encryptedSignalType,1);
		e_ref[3].Set(&algorithmOID,1);
	}

	 /////////////////////////////////////////////////////////////////////////////////////////
	unsigned char VS_H245UserInputIndication_Signal::signalType_alphabet[]=
	{'0','1','2','3','4','5','6','7','8','9','#','*','A','B','C','D','!' };
	unsigned char  VS_H245UserInputIndication_Signal::signalType_inverse_table[256]={0};
	 const bool VS_H245UserInputIndication_Signal::signalType_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H245UserInputIndication_Signal::signalType_inverse_table,
		 VS_H245UserInputIndication_Signal::signalType_alphabet,
		 sizeof(VS_H245UserInputIndication_Signal::signalType_alphabet));

	 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245UserInputIndication_Signal::operator=(const VS_H245UserInputIndication_Signal& src)
	{

		O_CC(filled);
		O_C(signalType);
		O_C(duration);
		O_C(rtp);
		O_C(rtpPayloadIndication);
		O_C(paramS);
		O_C(encryptedSignalType);
		O_C(algorithmOID);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputIndication_SignalUpdate_Rtp /////////////////////////
 	 VS_H245UserInputIndication_SignalUpdate_Rtp :: VS_H245UserInputIndication_SignalUpdate_Rtp( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&logicalChannelNumber,0);
	}
	void VS_H245UserInputIndication_SignalUpdate_Rtp::operator=(const VS_H245UserInputIndication_SignalUpdate_Rtp& src)
	{

		O_CC(filled);
		O_C(logicalChannelNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputIndication_SignalUpdate /////////////////////////
 	 VS_H245UserInputIndication_SignalUpdate :: VS_H245UserInputIndication_SignalUpdate( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&duration,0);
		ref[1].Set(&rtp,1);
	}
	void VS_H245UserInputIndication_SignalUpdate::operator=(const VS_H245UserInputIndication_SignalUpdate& src)
	{

		O_CC(filled);
		O_C(duration);
		O_C(rtp);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric /////////////////////////
 	 VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric :: VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&algorithmOID,0);
		ref[1].Set(&paramS,1);
		ref[2].Set(&encrypted,0);
	}
	void VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric::operator=(const VS_H245UserInputIndication_ExtendedAlphanumeric_EncryptedAlphanumeric& src)
	{

		O_CC(filled);
		O_C(algorithmOID);
		O_C(paramS);
		O_C(encrypted);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputIndication_ExtendedAlphanumeric /////////////////////////
 	 VS_H245UserInputIndication_ExtendedAlphanumeric :: VS_H245UserInputIndication_ExtendedAlphanumeric( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&alphanumeric,0);
		ref[1].Set(&rtpPayloadIndication,1);
		e_ref[0].Set(&encryptedAlphanumeric,1);
	}
	void VS_H245UserInputIndication_ExtendedAlphanumeric::operator=(const VS_H245UserInputIndication_ExtendedAlphanumeric& src)
	{

		O_CC(filled);
		O_C(alphanumeric);
		O_C(rtpPayloadIndication);
		O_C(encryptedAlphanumeric);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputIndication_EncryptedAlphanumeric /////////////////////////
 	 VS_H245UserInputIndication_EncryptedAlphanumeric :: VS_H245UserInputIndication_EncryptedAlphanumeric( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&algorithmOID,0);
		ref[1].Set(&paramS,1);
		ref[2].Set(&encrypted,0);
	}
	void VS_H245UserInputIndication_EncryptedAlphanumeric::operator=(const VS_H245UserInputIndication_EncryptedAlphanumeric& src)
	{

		O_CC(filled);
		O_C(algorithmOID);
		O_C(paramS);
		O_C(encrypted);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputIndication /////////////////////////
 	 VS_H245UserInputIndication::VS_H245UserInputIndication( void )
	:VS_AsnChoice(2 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245UserInputIndication::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_alphanumeric : return DecodeChoice( buffer , new VS_AsnGeneralString  );
		case e_userInputSupportIndication : return DecodeChoice( buffer , new VS_H245UserInputIndication_UserInputSupportIndication	 );
		case e_signal : return DecodeChoice( buffer , new VS_H245UserInputIndication_Signal	 );
		case e_signalUpdate : return DecodeChoice( buffer , new VS_H245UserInputIndication_SignalUpdate	 );
		case e_extendedAlphanumeric : return DecodeChoice( buffer , new VS_H245UserInputIndication_ExtendedAlphanumeric	 );
		case e_encryptedAlphanumeric : return DecodeChoice( buffer , new VS_H245UserInputIndication_EncryptedAlphanumeric	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245UserInputIndication::operator=(const VS_H245UserInputIndication & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_alphanumeric : CopyChoice<VS_AsnGeneralString   >(src,*this);  return;
		case e_userInputSupportIndication : CopyChoice<VS_H245UserInputIndication_UserInputSupportIndication	  >(src,*this);  return;
		case e_signal : CopyChoice<VS_H245UserInputIndication_Signal	  >(src,*this);  return;
		case e_signalUpdate : CopyChoice<VS_H245UserInputIndication_SignalUpdate	  >(src,*this);  return;
		case e_extendedAlphanumeric : CopyChoice<VS_H245UserInputIndication_ExtendedAlphanumeric	  >(src,*this);  return;
		case e_encryptedAlphanumeric : CopyChoice<VS_H245UserInputIndication_EncryptedAlphanumeric	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245UserInputIndication::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245UserInputIndication::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_alphanumeric :  dprint4("choice: VS_AsnGeneralString  ");return;
		case e_userInputSupportIndication :  dprint4("choice: VS_H245UserInputIndication_UserInputSupportIndication	 ");return;
		case e_signal :  dprint4("choice: VS_H245UserInputIndication_Signal	 ");return;
		case e_signalUpdate :  dprint4("choice: VS_H245UserInputIndication_SignalUpdate	 ");return;
		case e_extendedAlphanumeric :  dprint4("choice: VS_H245UserInputIndication_ExtendedAlphanumeric	 ");return;
		case e_encryptedAlphanumeric :  dprint4("choice: VS_H245UserInputIndication_EncryptedAlphanumeric	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245Params /////////////////////////
 	 VS_H245Params :: VS_H245Params( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&iv8,1);
		ref[1].Set(&iv16,1);
		ref[2].Set(&iv,1);
	}
	void VS_H245Params::operator=(const VS_H245Params& src)
	{

		O_CC(filled);
		O_C(iv8);
		O_C(iv16);
		O_C(iv);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H323Params /////////////////////////
	VS_H323Params::VS_H323Params(void)
		:VS_AsnSequence(2, ref, basic_root, e_ref, extension_root, 1)
	{
		ref[0].Set(&ranInt, 1);
		ref[1].Set(&iv8, 1);
		e_ref[0].Set(&iv16, 1);
		e_ref[1].Set(&iv, 1);
		e_ref[2].Set(&clearSalt, 1);
	}
    VS_H323Params::VS_H323Params(const VS_H323Params& other) = default;
	void VS_H323Params::operator=(const VS_H323Params& src)
	{

		O_CC(filled);
		O_C(ranInt);
		O_C(iv8);
		O_C(iv16);
		O_C(iv);
		O_C(clearSalt);
		O_CSA(ref, basic_root);
		O_CSA(e_ref, extension_root);
	}

/////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////CLASS VS_H323KeySyncMaterial /////////////////////////
	VS_H323KeySyncMaterial::VS_H323KeySyncMaterial(void)
		:VS_AsnSequence(0, ref, basic_root, nullptr, extension_root, 1)
	{
		ref[0].Set(&generalID, 0);
		ref[1].Set(&keyMaterial, 0);
	}
    VS_H323KeySyncMaterial::VS_H323KeySyncMaterial(const VS_H323KeySyncMaterial & src) = default;
	void VS_H323KeySyncMaterial::operator=(const VS_H323KeySyncMaterial& src)
	{
		O_CC(filled);
		O_C(generalID);
		O_C(keyMaterial);
		O_CSA(ref, basic_root)
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////CLASS VS_H323V3KeySyncMaterial /////////////////////////
	VS_H323V3KeySyncMaterial::VS_H323V3KeySyncMaterial(void)
		:VS_AsnSequence(7, ref, basic_root, nullptr, extension_root, 1)
	{
		ref[0].Set(&generalID, 1);
		ref[1].Set(&algorithmOID, 1);
		ref[2].Set(&paramS, 0);
		ref[3].Set(&encryptedSessionKey, 1);
		ref[4].Set(&encryptedSaltingKey, 1);
		ref[5].Set(&clearSaltingKey, 1);
		ref[6].Set(&paramSsalt, 1);
		ref[7].Set(&keyDerivationOID, 1);
	}
    VS_H323V3KeySyncMaterial::VS_H323V3KeySyncMaterial(const VS_H323V3KeySyncMaterial & src) = default;
	void VS_H323V3KeySyncMaterial::operator=(const VS_H323V3KeySyncMaterial& src)
	{

		O_CC(filled);
		O_C(generalID);
		O_C(algorithmOID);
		O_C(paramS);
		O_C(encryptedSessionKey);
		O_C(encryptedSaltingKey);
		O_C(clearSaltingKey);
		O_C(paramSsalt);
		O_C(keyDerivationOID);
		O_CSA(ref, basic_root)
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////CLASS VS_H323EncryptedEncodedKeySyncMaterial /////////////////////////
	VS_H323EncryptedEncodedKeySyncMaterial::VS_H323EncryptedEncodedKeySyncMaterial(void)
		:VS_AsnSequence(0, ref, basic_root, nullptr, extension_root, 0)
	{
		ref[0].Set(&algorithmOID, 0);
		ref[1].Set(&paramS, 0);
		ref[2].Set(&encryptedData, 0);
	}
    VS_H323EncryptedEncodedKeySyncMaterial::VS_H323EncryptedEncodedKeySyncMaterial(const VS_H323EncryptedEncodedKeySyncMaterial &) = default;
	void VS_H323EncryptedEncodedKeySyncMaterial::operator=(const VS_H323EncryptedEncodedKeySyncMaterial& src)
	{

		O_CC(filled);
		O_C(algorithmOID);
		O_C(paramS);
		O_C(encryptedData);
		O_CSA(ref, basic_root)
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////CLASS VS_H323KeySignedMaterial /////////////////////////
	VS_H323KeySignedMaterial::VS_H323KeySignedMaterial(void)
		:VS_AsnSequence(2, ref, basic_root, nullptr, extension_root, 0)
	{
		ref[0].Set(&generalId, 0);
		ref[1].Set(&mrandom, 0);
		ref[2].Set(&srandom, 1);
		ref[3].Set(&timeStamp, 1);
		ref[4].Set(&encrptval, 0);
	}
    VS_H323KeySignedMaterial::VS_H323KeySignedMaterial(const VS_H323KeySignedMaterial&) = default;
	void VS_H323KeySignedMaterial::operator=(const VS_H323KeySignedMaterial& src)
	{

		O_CC(filled);
		O_C(generalId);
		O_C(mrandom);
		O_C(srandom);
		O_C(timeStamp);
		O_C(encrptval);
		O_CSA(ref, basic_root)
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////CLASS VS_H323SignedEncodedKeySignedMaterial /////////////////////////
	VS_H323SignedEncodedKeySignedMaterial::VS_H323SignedEncodedKeySignedMaterial(void)
		:VS_AsnSequence(0, ref, basic_root, nullptr, extension_root, 0)
	{
		ref[0].Set(&toBeSigned, 0);
		ref[1].Set(&algorithmOID, 0);
		ref[2].Set(&paramS, 0);
		ref[3].Set(&signature, 0);
	}
    VS_H323SignedEncodedKeySignedMaterial::VS_H323SignedEncodedKeySignedMaterial(const VS_H323SignedEncodedKeySignedMaterial &) = default;
	void VS_H323SignedEncodedKeySignedMaterial::operator=(const VS_H323SignedEncodedKeySignedMaterial& src)
	{

		O_CC(filled);
		O_C(toBeSigned);
		O_C(algorithmOID);
		O_C(paramS);
		O_C(signature);
		O_CSA(ref, basic_root);
	}


	//////////////////////CLASS VS_H323H235Key /////////////////////////
	VS_H323H235Key::VS_H323H235Key(void)
		:VS_AsnChoice(3, 4, 1)
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H323H235Key::Decode(VS_PerBuffer &buffer)
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch (tag)
		{
		case e_secureChannel: return DecodeChoice(buffer, new VS_H323KeyMaterial);
		case e_sharedSecret: return DecodeChoice(buffer, new VS_H323EncryptedEncodedKeySyncMaterial);
		case e_certProtectedKey: return DecodeChoice(buffer, new VS_H323SignedEncodedKeySignedMaterial);
		case e_secureSharedSecret: return DecodeChoice(buffer, new VS_H323V3KeySyncMaterial);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H323H235Key::operator=(const VS_H323H235Key & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch (src.tag)
		{
		case e_secureChannel: CopyChoice< VS_H323KeyMaterial >(src, *this); return;
		case e_sharedSecret: CopyChoice< VS_H323EncryptedEncodedKeySyncMaterial >(src, *this); return;
		case e_certProtectedKey: CopyChoice< VS_H323SignedEncodedKeySignedMaterial >(src, *this); return;
		case e_secureSharedSecret: CopyChoice< VS_H323V3KeySyncMaterial >(src, *this); return;
		default:		return;
		}

		return;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	VS_H323H235Key::operator VS_H323KeyMaterial *(void)
	{
		return dynamic_cast< VS_H323KeyMaterial * >(choice);
	}

	VS_H323H235Key::operator VS_H323EncryptedEncodedKeySyncMaterial *(void)
	{
		return dynamic_cast< VS_H323EncryptedEncodedKeySyncMaterial * >(choice);
	}

	VS_H323H235Key::operator VS_H323SignedEncodedKeySignedMaterial *(void)
	{
		return dynamic_cast< VS_H323SignedEncodedKeySignedMaterial * >(choice);
	}

	VS_H323H235Key::operator VS_H323V3KeySyncMaterial *(void)
	{
		return dynamic_cast< VS_H323V3KeySyncMaterial * >(choice);
	}


	/////////////////////////////////////////////////////////////////////////////////////////
	void VS_H323H235Key::Show(void)
	{
		printf("\n\t----------- VS_H323H235Key::SHOW-----------");
		if (!filled) return;
		printf("Choice tag = %u ", tag);
		switch (tag)
		{
		case e_secureChannel:  printf("\n\t choice: VS_H323KeyMaterial "); return;
		case e_sharedSecret:  printf("\n\t choice: VS_H323EncryptedEncodedKeySyncMaterial "); return;
		case e_certProtectedKey:  printf("\n\t choice: VS_H323SignedEncodedKeySignedMaterial "); return;
		case e_secureSharedSecret:  printf("\n\t choice: VS_H323V3KeySyncMaterial "); return;
		default: printf("\n\t unknown choice: %u", tag); return;
		}

	}

//////////////////////CLASS VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery /////////////////////////
 	 VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery::VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nullClockRecovery : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_srtsClockRecovery : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_adaptiveClockRecovery : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery::operator=(const VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nullClockRecovery : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_srtsClockRecovery : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_adaptiveClockRecovery : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCIndication_Aal_Aal1_ClockRecovery::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nullClockRecovery :  dprint4("choice:  VS_AsnNull  ");return;
		case e_srtsClockRecovery :  dprint4("choice:  VS_AsnNull  ");return;
		case e_adaptiveClockRecovery :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection /////////////////////////
 	 VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection::VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nullErrorCorrection : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_longInterleaver : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_shortInterleaver : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_errorCorrectionOnly : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection::operator=(const VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nullErrorCorrection : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_longInterleaver : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_shortInterleaver : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_errorCorrectionOnly : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCIndication_Aal_Aal1_ErrorCorrection::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nullErrorCorrection :  dprint4("choice:  VS_AsnNull  ");return;
		case e_longInterleaver :  dprint4("choice:  VS_AsnNull  ");return;
		case e_shortInterleaver :  dprint4("choice:  VS_AsnNull  ");return;
		case e_errorCorrectionOnly :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCIndication_Aal_Aal1 /////////////////////////
 	 VS_H245NewATMVCIndication_Aal_Aal1 :: VS_H245NewATMVCIndication_Aal_Aal1( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&clockRecovery,0);
		ref[1].Set(&errorCorrection,0);
		ref[2].Set(&structuredDataTransfer,0);
		ref[3].Set(&partiallyFilledCells,0);
	}
	void VS_H245NewATMVCIndication_Aal_Aal1::operator=(const VS_H245NewATMVCIndication_Aal_Aal1& src)
	{

		O_CC(filled);
		O_C(clockRecovery);
		O_C(errorCorrection);
		O_C(structuredDataTransfer);
		O_C(partiallyFilledCells);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NewATMVCIndication_Aal_Aal5 /////////////////////////
 	 VS_H245NewATMVCIndication_Aal_Aal5 :: VS_H245NewATMVCIndication_Aal_Aal5( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&forwardMaximumSDUSize,0);
		ref[1].Set(&backwardMaximumSDUSize,0);
	}
	void VS_H245NewATMVCIndication_Aal_Aal5::operator=(const VS_H245NewATMVCIndication_Aal_Aal5& src)
	{

		O_CC(filled);
		O_C(forwardMaximumSDUSize);
		O_C(backwardMaximumSDUSize);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NewATMVCIndication_Aal /////////////////////////
 	 VS_H245NewATMVCIndication_Aal::VS_H245NewATMVCIndication_Aal( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCIndication_Aal::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_aal1 : return DecodeChoice( buffer , new VS_H245NewATMVCIndication_Aal_Aal1	 );
		case e_aal5 : return DecodeChoice( buffer , new VS_H245NewATMVCIndication_Aal_Aal5	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCIndication_Aal::operator=(const VS_H245NewATMVCIndication_Aal & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_aal1 : CopyChoice<VS_H245NewATMVCIndication_Aal_Aal1	  >(src,*this);  return;
		case e_aal5 : CopyChoice<VS_H245NewATMVCIndication_Aal_Aal5	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCIndication_Aal::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_aal1 :  dprint4("choice: VS_H245NewATMVCIndication_Aal_Aal1	 ");return;
		case e_aal5 :  dprint4("choice: VS_H245NewATMVCIndication_Aal_Aal5	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCIndication_Multiplex /////////////////////////
 	 VS_H245NewATMVCIndication_Multiplex::VS_H245NewATMVCIndication_Multiplex( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCIndication_Multiplex::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noMultiplex : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transportStream : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_programStream : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCIndication_Multiplex::operator=(const VS_H245NewATMVCIndication_Multiplex & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noMultiplex : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transportStream : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_programStream : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCIndication_Multiplex::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_noMultiplex :  dprint4("choice:  VS_AsnNull  ");return;
		case e_transportStream :  dprint4("choice:  VS_AsnNull  ");return;
		case e_programStream :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCIndication_ReverseParameters_Multiplex /////////////////////////
 	 VS_H245NewATMVCIndication_ReverseParameters_Multiplex::VS_H245NewATMVCIndication_ReverseParameters_Multiplex( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCIndication_ReverseParameters_Multiplex::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noMultiplex : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transportStream : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_programStream : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCIndication_ReverseParameters_Multiplex::operator=(const VS_H245NewATMVCIndication_ReverseParameters_Multiplex & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noMultiplex : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transportStream : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_programStream : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCIndication_ReverseParameters_Multiplex::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_noMultiplex :  dprint4("choice:  VS_AsnNull  ");return;
		case e_transportStream :  dprint4("choice:  VS_AsnNull  ");return;
		case e_programStream :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCIndication_ReverseParameters /////////////////////////
 	 VS_H245NewATMVCIndication_ReverseParameters :: VS_H245NewATMVCIndication_ReverseParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&bitRate,0);
		ref[1].Set(&bitRateLockedToPCRClock,0);
		ref[2].Set(&bitRateLockedToNetworkClock,0);
		ref[3].Set(&multiplex,0);
	}
	void VS_H245NewATMVCIndication_ReverseParameters::operator=(const VS_H245NewATMVCIndication_ReverseParameters& src)
	{

		O_CC(filled);
		O_C(bitRate);
		O_C(bitRateLockedToPCRClock);
		O_C(bitRateLockedToNetworkClock);
		O_C(multiplex);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NewATMVCIndication /////////////////////////
 	 VS_H245NewATMVCIndication :: VS_H245NewATMVCIndication( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&resourceID,0);
		ref[1].Set(&bitRate,0);
		ref[2].Set(&bitRateLockedToPCRClock,0);
		ref[3].Set(&bitRateLockedToNetworkClock,0);
		ref[4].Set(&aal,0);
		ref[5].Set(&multiplex,0);
		e_ref[0].Set(&reverseParameters,0);
	}
	void VS_H245NewATMVCIndication::operator=(const VS_H245NewATMVCIndication& src)
	{

		O_CC(filled);
		O_C(resourceID);
		O_C(bitRate);
		O_C(bitRateLockedToPCRClock);
		O_C(bitRateLockedToNetworkClock);
		O_C(aal);
		O_C(multiplex);
		O_C(reverseParameters);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VendorIdentification /////////////////////////
 	 VS_H245VendorIdentification :: VS_H245VendorIdentification( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&vendor,0);
		ref[1].Set(&productNumber,1);
		ref[2].Set(&versionNumber,1);
	}
	void VS_H245VendorIdentification::operator=(const VS_H245VendorIdentification& src)
	{

		O_CC(filled);
		O_C(vendor);
		O_C(productNumber);
		O_C(versionNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MCLocationIndication /////////////////////////
 	 VS_H245MCLocationIndication :: VS_H245MCLocationIndication( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&signalAddress,0);
	}
	void VS_H245MCLocationIndication::operator=(const VS_H245MCLocationIndication& src)
	{

		O_CC(filled);
		O_C(signalAddress);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H2250MaximumSkewIndication /////////////////////////
 	 VS_H245H2250MaximumSkewIndication :: VS_H245H2250MaximumSkewIndication( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&logicalChannelNumber1,0);
		ref[1].Set(&logicalChannelNumber2,0);
		ref[2].Set(&maximumSkew,0);
	}
	void VS_H245H2250MaximumSkewIndication::operator=(const VS_H245H2250MaximumSkewIndication& src)
	{

		O_CC(filled);
		O_C(logicalChannelNumber1);
		O_C(logicalChannelNumber2);
		O_C(maximumSkew);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223SkewIndication /////////////////////////
 	 VS_H245H223SkewIndication :: VS_H245H223SkewIndication( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&logicalChannelNumber1,0);
		ref[1].Set(&logicalChannelNumber2,0);
		ref[2].Set(&skew,0);
	}
	void VS_H245H223SkewIndication::operator=(const VS_H245H223SkewIndication& src)
	{

		O_CC(filled);
		O_C(logicalChannelNumber1);
		O_C(logicalChannelNumber2);
		O_C(skew);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245JitterIndication_Scope /////////////////////////
 	 VS_H245JitterIndication_Scope::VS_H245JitterIndication_Scope( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245JitterIndication_Scope::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_logicalChannelNumber : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_resourceID : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		case e_wholeMultiplex : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245JitterIndication_Scope::operator=(const VS_H245JitterIndication_Scope & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_logicalChannelNumber : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_resourceID : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_wholeMultiplex : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245JitterIndication_Scope::operator VS_H245LogicalChannelNumber *( void )
	{	return dynamic_cast< VS_H245LogicalChannelNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245JitterIndication_Scope::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_logicalChannelNumber :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_resourceID :  dprint4("choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_wholeMultiplex :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245JitterIndication /////////////////////////
 	 VS_H245JitterIndication :: VS_H245JitterIndication( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&scope,0);
		ref[1].Set(&estimatedReceivedJitterMantissa,0);
		ref[2].Set(&estimatedReceivedJitterExponent,0);
		ref[3].Set(&skippedFrameCount,1);
		ref[4].Set(&additionalDecoderBuffer,1);
	}
	void VS_H245JitterIndication::operator=(const VS_H245JitterIndication& src)
	{

		O_CC(filled);
		O_C(scope);
		O_C(estimatedReceivedJitterMantissa);
		O_C(estimatedReceivedJitterExponent);
		O_C(skippedFrameCount);
		O_C(additionalDecoderBuffer);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs /////////////////////////
 	 VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs :: VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&firstMB,0);
		ref[1].Set(&numberOfMBs,0);
		ref[2].Set(&temporalReference,0);
	}
	void VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs::operator=(const VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs& src)
	{

		O_CC(filled);
		O_C(firstMB);
		O_C(numberOfMBs);
		O_C(temporalReference);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousIndication_Type /////////////////////////
 	 VS_H245MiscellaneousIndication_Type::VS_H245MiscellaneousIndication_Type( void )
	:VS_AsnChoice(10 , 12 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MiscellaneousIndication_Type::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_logicalChannelActive : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_logicalChannelInactive : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_multipointConference : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cancelMultipointConference : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_multipointZeroComm : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cancelMultipointZeroComm : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_multipointSecondaryStatus : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cancelMultipointSecondaryStatus : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoIndicateReadyToActivate : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoTemporalSpatialTradeOff : return DecodeChoice( buffer , new TemplInteger<0,31,VS_Asn::FixedConstraint,0>  );
		case e_videoNotDecodedMBs : return DecodeChoice( buffer , new VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs	 );
		case e_transportCapability : return DecodeChoice( buffer , new VS_H245TransportCapability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MiscellaneousIndication_Type::operator=(const VS_H245MiscellaneousIndication_Type & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_logicalChannelActive : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_logicalChannelInactive : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_multipointConference : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cancelMultipointConference : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_multipointZeroComm : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cancelMultipointZeroComm : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_multipointSecondaryStatus : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cancelMultipointSecondaryStatus : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoIndicateReadyToActivate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoTemporalSpatialTradeOff : CopyChoice<TemplInteger<0,31,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_videoNotDecodedMBs : CopyChoice<VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs	  >(src,*this);  return;
		case e_transportCapability : CopyChoice< VS_H245TransportCapability >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MiscellaneousIndication_Type::operator VS_H245TransportCapability *( void )
	{	return dynamic_cast< VS_H245TransportCapability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MiscellaneousIndication_Type::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_logicalChannelActive :  dprint4("choice:  VS_AsnNull  ");return;
		case e_logicalChannelInactive :  dprint4("choice:  VS_AsnNull  ");return;
		case e_multipointConference :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cancelMultipointConference :  dprint4("choice:  VS_AsnNull  ");return;
		case e_multipointZeroComm :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cancelMultipointZeroComm :  dprint4("choice:  VS_AsnNull  ");return;
		case e_multipointSecondaryStatus :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cancelMultipointSecondaryStatus :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoIndicateReadyToActivate :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoTemporalSpatialTradeOff :  dprint4("choice: TemplInteger<0,31,VS_Asn::FixedConstraint,0>  ");return;
		case e_videoNotDecodedMBs :  dprint4("choice: VS_H245MiscellaneousIndication_Type_VideoNotDecodedMBs	 ");return;
		case e_transportCapability :  dprint4("choice: VS_H245TransportCapability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MiscellaneousIndication /////////////////////////
 	 VS_H245MiscellaneousIndication :: VS_H245MiscellaneousIndication( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&logicalChannelNumber,0);
		ref[1].Set(&type,0);
	}
	void VS_H245MiscellaneousIndication::operator=(const VS_H245MiscellaneousIndication& src)
	{

		O_CC(filled);
		O_C(logicalChannelNumber);
		O_C(type);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VideoIndicateCompose /////////////////////////
 	 VS_H245VideoIndicateCompose :: VS_H245VideoIndicateCompose( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&compositionNumber,0);
	}
	void VS_H245VideoIndicateCompose::operator=(const VS_H245VideoIndicateCompose& src)
	{

		O_CC(filled);
		O_C(compositionNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245TerminalYouAreSeeingInSubPictureNumber /////////////////////////
 	 VS_H245TerminalYouAreSeeingInSubPictureNumber :: VS_H245TerminalYouAreSeeingInSubPictureNumber( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&terminalNumber,0);
		ref[1].Set(&subPictureNumber,0);
	}
	void VS_H245TerminalYouAreSeeingInSubPictureNumber::operator=(const VS_H245TerminalYouAreSeeingInSubPictureNumber& src)
	{

		O_CC(filled);
		O_C(terminalNumber);
		O_C(subPictureNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceIndication /////////////////////////
 	 VS_H245ConferenceIndication::VS_H245ConferenceIndication( void )
	:VS_AsnChoice(10 , 14 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ConferenceIndication::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_sbeNumber : return DecodeChoice( buffer , new TemplInteger<0,9,VS_Asn::FixedConstraint,0>  );
		case e_terminalNumberAssign : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_terminalJoinedConference : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_terminalLeftConference : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_seenByAtLeastOneOther : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cancelSeenByAtLeastOneOther : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_seenByAll : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cancelSeenByAll : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_terminalYouAreSeeing : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_requestForFloor : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_withdrawChairToken : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_floorRequested : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_terminalYouAreSeeingInSubPictureNumber : return DecodeChoice( buffer , new VS_H245TerminalYouAreSeeingInSubPictureNumber);
		case e_videoIndicateCompose : return DecodeChoice( buffer , new VS_H245VideoIndicateCompose);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ConferenceIndication::operator=(const VS_H245ConferenceIndication & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_sbeNumber : CopyChoice<TemplInteger<0,9,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_terminalNumberAssign : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_terminalJoinedConference : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_terminalLeftConference : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_seenByAtLeastOneOther : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cancelSeenByAtLeastOneOther : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_seenByAll : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cancelSeenByAll : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_terminalYouAreSeeing : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_requestForFloor : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_withdrawChairToken : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_floorRequested : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_terminalYouAreSeeingInSubPictureNumber : CopyChoice< VS_H245TerminalYouAreSeeingInSubPictureNumber >(src,*this); return;
		case e_videoIndicateCompose : CopyChoice< VS_H245VideoIndicateCompose >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245ConferenceIndication::operator VS_H245TerminalLabel *( void )
	{	return dynamic_cast< VS_H245TerminalLabel * >(choice);    }

 	VS_H245ConferenceIndication::operator VS_H245TerminalYouAreSeeingInSubPictureNumber *( void )
	{	return dynamic_cast< VS_H245TerminalYouAreSeeingInSubPictureNumber * >(choice);    }

 	VS_H245ConferenceIndication::operator VS_H245VideoIndicateCompose *( void )
	{	return dynamic_cast< VS_H245VideoIndicateCompose * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ConferenceIndication::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_sbeNumber :  dprint4("choice: TemplInteger<0,9,VS_Asn::FixedConstraint,0>  ");return;
		case e_terminalNumberAssign :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_terminalJoinedConference :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_terminalLeftConference :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_seenByAtLeastOneOther :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cancelSeenByAtLeastOneOther :  dprint4("choice:  VS_AsnNull  ");return;
		case e_seenByAll :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cancelSeenByAll :  dprint4("choice:  VS_AsnNull  ");return;
		case e_terminalYouAreSeeing :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_requestForFloor :  dprint4("choice:  VS_AsnNull  ");return;
		case e_withdrawChairToken :  dprint4("choice:  VS_AsnNull  ");return;
		case e_floorRequested :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_terminalYouAreSeeingInSubPictureNumber :  dprint4("choice: VS_H245TerminalYouAreSeeingInSubPictureNumber ");return;
		case e_videoIndicateCompose :  dprint4("choice: VS_H245VideoIndicateCompose ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FunctionNotSupported_Cause /////////////////////////
 	 VS_H245FunctionNotSupported_Cause::VS_H245FunctionNotSupported_Cause( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FunctionNotSupported_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_syntaxError : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_semanticError : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_unknownFunction : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245FunctionNotSupported_Cause::operator=(const VS_H245FunctionNotSupported_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_syntaxError : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_semanticError : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_unknownFunction : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FunctionNotSupported_Cause::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_syntaxError :  dprint4("choice:  VS_AsnNull  ");return;
		case e_semanticError :  dprint4("choice:  VS_AsnNull  ");return;
		case e_unknownFunction :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FunctionNotSupported /////////////////////////
 	 VS_H245FunctionNotSupported :: VS_H245FunctionNotSupported( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&cause,0);
		ref[1].Set(&returnedFunction,1);
	}
	void VS_H245FunctionNotSupported::operator=(const VS_H245FunctionNotSupported& src)
	{

		O_CC(filled);
		O_C(cause);
		O_C(returnedFunction);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245FunctionNotUnderstood /////////////////////////
 	 VS_H245FunctionNotUnderstood::VS_H245FunctionNotUnderstood( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FunctionNotUnderstood::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_request : return DecodeChoice( buffer , new VS_H245RequestMessage);
		case e_response : return DecodeChoice( buffer , new VS_H245ResponseMessage);
		case e_command : return DecodeChoice( buffer , new VS_H245CommandMessage);
		default: return false;
		}

	}

	void VS_H245FunctionNotUnderstood::operator=(const VS_H245FunctionNotUnderstood & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_request : CopyChoice< VS_H245RequestMessage >(src,*this); return;
		case e_response : CopyChoice< VS_H245ResponseMessage >(src,*this); return;
		case e_command : CopyChoice< VS_H245CommandMessage >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245FunctionNotUnderstood::operator VS_H245RequestMessage *( void )
	{	return dynamic_cast< VS_H245RequestMessage * >(choice);    }

 	VS_H245FunctionNotUnderstood::operator VS_H245ResponseMessage *( void )
	{	return dynamic_cast< VS_H245ResponseMessage * >(choice);    }

 	VS_H245FunctionNotUnderstood::operator VS_H245CommandMessage *( void )
	{	return dynamic_cast< VS_H245CommandMessage * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FunctionNotUnderstood::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_request :  dprint4("choice: VS_H245RequestMessage ");return;
		case e_response :  dprint4("choice: VS_H245ResponseMessage ");return;
		case e_command :  dprint4("choice: VS_H245CommandMessage ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MobileMultilinkReconfigurationCommand_Status /////////////////////////
 	 VS_H245MobileMultilinkReconfigurationCommand_Status::VS_H245MobileMultilinkReconfigurationCommand_Status( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MobileMultilinkReconfigurationCommand_Status::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_synchronized : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_reconfiguration : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MobileMultilinkReconfigurationCommand_Status::operator=(const VS_H245MobileMultilinkReconfigurationCommand_Status & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_synchronized : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_reconfiguration : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MobileMultilinkReconfigurationCommand_Status::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_synchronized :  dprint4("choice:  VS_AsnNull  ");return;
		case e_reconfiguration :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MobileMultilinkReconfigurationCommand /////////////////////////
 	 VS_H245MobileMultilinkReconfigurationCommand :: VS_H245MobileMultilinkReconfigurationCommand( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&sampleSize,0);
		ref[1].Set(&samplesPerFrame,0);
		ref[2].Set(&status,0);
	}
	void VS_H245MobileMultilinkReconfigurationCommand::operator=(const VS_H245MobileMultilinkReconfigurationCommand& src)
	{

		O_CC(filled);
		O_C(sampleSize);
		O_C(samplesPerFrame);
		O_C(status);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery /////////////////////////
 	 VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery::VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nullClockRecovery : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_srtsClockRecovery : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_adaptiveClockRecovery : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery::operator=(const VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nullClockRecovery : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_srtsClockRecovery : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_adaptiveClockRecovery : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCCommand_Aal_Aal1_ClockRecovery::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_nullClockRecovery :  dprint4("choice:  VS_AsnNull  ");return;
		case e_srtsClockRecovery :  dprint4("choice:  VS_AsnNull  ");return;
		case e_adaptiveClockRecovery :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection /////////////////////////
 	 VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection::VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nullErrorCorrection : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_longInterleaver : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_shortInterleaver : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_errorCorrectionOnly : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection::operator=(const VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nullErrorCorrection : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_longInterleaver : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_shortInterleaver : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_errorCorrectionOnly : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCCommand_Aal_Aal1_ErrorCorrection::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_nullErrorCorrection :  dprint4("choice:  VS_AsnNull  ");return;
		case e_longInterleaver :  dprint4("choice:  VS_AsnNull  ");return;
		case e_shortInterleaver :  dprint4("choice:  VS_AsnNull  ");return;
		case e_errorCorrectionOnly :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCCommand_Aal_Aal1 /////////////////////////
 	 VS_H245NewATMVCCommand_Aal_Aal1 :: VS_H245NewATMVCCommand_Aal_Aal1( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&clockRecovery,0);
		ref[1].Set(&errorCorrection,0);
		ref[2].Set(&structuredDataTransfer,0);
		ref[3].Set(&partiallyFilledCells,0);
	}
	void VS_H245NewATMVCCommand_Aal_Aal1::operator=(const VS_H245NewATMVCCommand_Aal_Aal1& src)
	{

		O_CC(filled);
		O_C(clockRecovery);
		O_C(errorCorrection);
		O_C(structuredDataTransfer);
		O_C(partiallyFilledCells);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NewATMVCCommand_Aal_Aal5 /////////////////////////
 	 VS_H245NewATMVCCommand_Aal_Aal5 :: VS_H245NewATMVCCommand_Aal_Aal5( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&forwardMaximumSDUSize,0);
		ref[1].Set(&backwardMaximumSDUSize,0);
	}
	void VS_H245NewATMVCCommand_Aal_Aal5::operator=(const VS_H245NewATMVCCommand_Aal_Aal5& src)
	{

		O_CC(filled);
		O_C(forwardMaximumSDUSize);
		O_C(backwardMaximumSDUSize);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NewATMVCCommand_Aal /////////////////////////
 	 VS_H245NewATMVCCommand_Aal::VS_H245NewATMVCCommand_Aal( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCCommand_Aal::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_aal1 : return DecodeChoice( buffer , new VS_H245NewATMVCCommand_Aal_Aal1	 );
		case e_aal5 : return DecodeChoice( buffer , new VS_H245NewATMVCCommand_Aal_Aal5	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCCommand_Aal::operator=(const VS_H245NewATMVCCommand_Aal & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_aal1 : CopyChoice<VS_H245NewATMVCCommand_Aal_Aal1	  >(src,*this);  return;
		case e_aal5 : CopyChoice<VS_H245NewATMVCCommand_Aal_Aal5	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCCommand_Aal::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_aal1 :  dprint4("choice: VS_H245NewATMVCCommand_Aal_Aal1	 ");return;
		case e_aal5 :  dprint4("choice: VS_H245NewATMVCCommand_Aal_Aal5	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCCommand_Multiplex /////////////////////////
 	 VS_H245NewATMVCCommand_Multiplex::VS_H245NewATMVCCommand_Multiplex( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCCommand_Multiplex::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noMultiplex : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transportStream : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_programStream : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCCommand_Multiplex::operator=(const VS_H245NewATMVCCommand_Multiplex & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noMultiplex : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transportStream : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_programStream : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCCommand_Multiplex::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_noMultiplex :  dprint4("choice:  VS_AsnNull  ");return;
		case e_transportStream :  dprint4("choice:  VS_AsnNull  ");return;
		case e_programStream :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCCommand_ReverseParameters_Multiplex /////////////////////////
 	 VS_H245NewATMVCCommand_ReverseParameters_Multiplex::VS_H245NewATMVCCommand_ReverseParameters_Multiplex( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NewATMVCCommand_ReverseParameters_Multiplex::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noMultiplex : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transportStream : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_programStream : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NewATMVCCommand_ReverseParameters_Multiplex::operator=(const VS_H245NewATMVCCommand_ReverseParameters_Multiplex & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noMultiplex : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transportStream : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_programStream : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NewATMVCCommand_ReverseParameters_Multiplex::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_noMultiplex :  dprint4("choice:  VS_AsnNull  ");return;
		case e_transportStream :  dprint4("choice:  VS_AsnNull  ");return;
		case e_programStream :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NewATMVCCommand_ReverseParameters /////////////////////////
 	 VS_H245NewATMVCCommand_ReverseParameters :: VS_H245NewATMVCCommand_ReverseParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&bitRate,0);
		ref[1].Set(&bitRateLockedToPCRClock,0);
		ref[2].Set(&bitRateLockedToNetworkClock,0);
		ref[3].Set(&multiplex,0);
	}
	void VS_H245NewATMVCCommand_ReverseParameters::operator=(const VS_H245NewATMVCCommand_ReverseParameters& src)
	{

		O_CC(filled);
		O_C(bitRate);
		O_C(bitRateLockedToPCRClock);
		O_C(bitRateLockedToNetworkClock);
		O_C(multiplex);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NewATMVCCommand /////////////////////////
 	 VS_H245NewATMVCCommand :: VS_H245NewATMVCCommand( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&resourceID,0);
		ref[1].Set(&bitRate,0);
		ref[2].Set(&bitRateLockedToPCRClock,0);
		ref[3].Set(&bitRateLockedToNetworkClock,0);
		ref[4].Set(&aal,0);
		ref[5].Set(&multiplex,0);
		ref[6].Set(&reverseParameters,0);
	}
	void VS_H245NewATMVCCommand::operator=(const VS_H245NewATMVCCommand& src)
	{

		O_CC(filled);
		O_C(resourceID);
		O_C(bitRate);
		O_C(bitRateLockedToPCRClock);
		O_C(bitRateLockedToNetworkClock);
		O_C(aal);
		O_C(multiplex);
		O_C(reverseParameters);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223MultiplexReconfiguration_H223ModeChange /////////////////////////
 	 VS_H245H223MultiplexReconfiguration_H223ModeChange::VS_H245H223MultiplexReconfiguration_H223ModeChange( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223MultiplexReconfiguration_H223ModeChange::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_toLevel0 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_toLevel1 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_toLevel2 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_toLevel2withOptionalHeader : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223MultiplexReconfiguration_H223ModeChange::operator=(const VS_H245H223MultiplexReconfiguration_H223ModeChange & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_toLevel0 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_toLevel1 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_toLevel2 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_toLevel2withOptionalHeader : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223MultiplexReconfiguration_H223ModeChange::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_toLevel0 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_toLevel1 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_toLevel2 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_toLevel2withOptionalHeader :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag /////////////////////////
 	 VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag::VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_start : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_stop : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag::operator=(const VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_start : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_stop : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_start :  dprint4("choice:  VS_AsnNull  ");return;
		case e_stop :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223MultiplexReconfiguration /////////////////////////
 	 VS_H245H223MultiplexReconfiguration::VS_H245H223MultiplexReconfiguration( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223MultiplexReconfiguration::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_h223ModeChange : return DecodeChoice( buffer , new VS_H245H223MultiplexReconfiguration_H223ModeChange	 );
		case e_h223AnnexADoubleFlag : return DecodeChoice( buffer , new VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223MultiplexReconfiguration::operator=(const VS_H245H223MultiplexReconfiguration & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_h223ModeChange : CopyChoice<VS_H245H223MultiplexReconfiguration_H223ModeChange	  >(src,*this);  return;
		case e_h223AnnexADoubleFlag : CopyChoice<VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223MultiplexReconfiguration::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_h223ModeChange :  dprint4("choice: VS_H245H223MultiplexReconfiguration_H223ModeChange	 ");return;
		case e_h223AnnexADoubleFlag :  dprint4("choice: VS_H245H223MultiplexReconfiguration_H223AnnexADoubleFlag	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245PictureReference /////////////////////////
 	 VS_H245PictureReference::VS_H245PictureReference( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245PictureReference::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_pictureNumber : return DecodeChoice( buffer , new TemplInteger<0,1023,VS_Asn::FixedConstraint,0>  );
		case e_longTermPictureIndex : return DecodeChoice( buffer , new TemplInteger<0,255,VS_Asn::FixedConstraint,0>  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245PictureReference::operator=(const VS_H245PictureReference & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_pictureNumber : CopyChoice<TemplInteger<0,1023,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_longTermPictureIndex : CopyChoice<TemplInteger<0,255,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245PictureReference::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_pictureNumber :  dprint4("choice: TemplInteger<0,1023,VS_Asn::FixedConstraint,0>  ");return;
		case e_longTermPictureIndex :  dprint4("choice: TemplInteger<0,255,VS_Asn::FixedConstraint,0>  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245EncryptionUpdateRequest /////////////////////////
 	 VS_H245EncryptionUpdateRequest :: VS_H245EncryptionUpdateRequest( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&keyProtectionMethod,1);
		e_ref[0].Set(&synchFlag,1);
	}
	void VS_H245EncryptionUpdateRequest::operator=(const VS_H245EncryptionUpdateRequest& src)
	{

		O_CC(filled);
		O_C(keyProtectionMethod);
		O_C(synchFlag);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245KeyProtectionMethod /////////////////////////
 	 VS_H245KeyProtectionMethod :: VS_H245KeyProtectionMethod( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&secureChannel,0);
		ref[1].Set(&sharedSecret,0);
		ref[2].Set(&certProtectedKey,0);
	}
	void VS_H245KeyProtectionMethod::operator=(const VS_H245KeyProtectionMethod& src)
	{

		O_CC(filled);
		O_C(secureChannel);
		O_C(sharedSecret);
		O_C(certProtectedKey);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB /////////////////////////
 	 VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB :: VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 0 )
	{
		ref[0].Set(&firstGOB,0);
		ref[1].Set(&numberOfGOBs,0);
	}
	void VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB::operator=(const VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB& src)
	{

		O_CC(filled);
		O_C(firstGOB);
		O_C(numberOfGOBs);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB /////////////////////////
 	 VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB :: VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&firstGOB,1);
		ref[1].Set(&firstMB,1);
		ref[2].Set(&numberOfMBs,0);
	}
	void VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB::operator=(const VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB& src)
	{

		O_CC(filled);
		O_C(firstGOB);
		O_C(firstMB);
		O_C(numberOfMBs);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount /////////////////////////
 	 VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount::VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_doOneProgression : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_doContinuousProgressions : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_doOneIndependentProgression : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_doContinuousIndependentProgressions : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount::operator=(const VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_doOneProgression : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_doContinuousProgressions : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_doOneIndependentProgression : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_doContinuousIndependentProgressions : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart_RepeatCount::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_doOneProgression :  dprint4("choice:  VS_AsnNull  ");return;
		case e_doContinuousProgressions :  dprint4("choice:  VS_AsnNull  ");return;
		case e_doOneIndependentProgression :  dprint4("choice:  VS_AsnNull  ");return;
		case e_doContinuousIndependentProgressions :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart /////////////////////////
 	 VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart :: VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&repeatCount,0);
	}
	void VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart::operator=(const VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart& src)
	{

		O_CC(filled);
		O_C(repeatCount);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_VideoBadMBs /////////////////////////
 	 VS_H245MiscellaneousCommand_Type_VideoBadMBs :: VS_H245MiscellaneousCommand_Type_VideoBadMBs( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&firstMB,0);
		ref[1].Set(&numberOfMBs,0);
		ref[2].Set(&temporalReference,0);
	}
	void VS_H245MiscellaneousCommand_Type_VideoBadMBs::operator=(const VS_H245MiscellaneousCommand_Type_VideoBadMBs& src)
	{

		O_CC(filled);
		O_C(firstMB);
		O_C(numberOfMBs);
		O_C(temporalReference);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_LostPartialPicture /////////////////////////
 	 VS_H245MiscellaneousCommand_Type_LostPartialPicture :: VS_H245MiscellaneousCommand_Type_LostPartialPicture( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&pictureReference,0);
		ref[1].Set(&firstMB,0);
		ref[2].Set(&numberOfMBs,0);
	}
	void VS_H245MiscellaneousCommand_Type_LostPartialPicture::operator=(const VS_H245MiscellaneousCommand_Type_LostPartialPicture& src)
	{

		O_CC(filled);
		O_C(pictureReference);
		O_C(firstMB);
		O_C(numberOfMBs);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand /////////////////////////
 	 VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand :: VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&encryptionSync,0);
		ref[1].Set(&multiplePayloadStream,1);
	}
	void VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand::operator=(const VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand& src)
	{

		O_CC(filled);
		O_C(encryptionSync);
		O_C(multiplePayloadStream);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck /////////////////////////
 	 VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck :: VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&synchFlag,0);
	}
	void VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck::operator=(const VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck& src)
	{

		O_CC(filled);
		O_C(synchFlag);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MiscellaneousCommand_Type /////////////////////////
 	 VS_H245MiscellaneousCommand_Type::VS_H245MiscellaneousCommand_Type( void )
	:VS_AsnChoice(10 , 25 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MiscellaneousCommand_Type::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_equaliseDelay : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_zeroDelay : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_multipointModeCommand : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cancelMultipointModeCommand : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoFreezePicture : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoFastUpdatePicture : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoFastUpdateGOB : return DecodeChoice( buffer , new VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB	 );
		case e_videoTemporalSpatialTradeOff : return DecodeChoice( buffer , new TemplInteger<0,31,VS_Asn::FixedConstraint,0>  );
		case e_videoSendSyncEveryGOB : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoSendSyncEveryGOBCancel : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoFastUpdateMB : return DecodeChoice( buffer , new VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB	 );
		case e_maxH223MUXPDUsize : return DecodeChoice( buffer , new TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  );
		case e_encryptionUpdate : return DecodeChoice( buffer , new VS_H245EncryptionSync);
		case e_encryptionUpdateRequest : return DecodeChoice( buffer , new VS_H245EncryptionUpdateRequest);
		case e_switchReceiveMediaOff : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_switchReceiveMediaOn : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_progressiveRefinementStart : return DecodeChoice( buffer , new VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart	 );
		case e_progressiveRefinementAbortOne : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_progressiveRefinementAbortContinuous : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoBadMBs : return DecodeChoice( buffer , new VS_H245MiscellaneousCommand_Type_VideoBadMBs	 );
		case e_lostPicture : return DecodeChoice( buffer , new VS_H245PictureReference);
		case e_lostPartialPicture : return DecodeChoice( buffer , new VS_H245MiscellaneousCommand_Type_LostPartialPicture	 );
		case e_recoveryReferencePicture : return DecodeChoice( buffer , new VS_H245PictureReference);
		case e_encryptionUpdateCommand : return DecodeChoice( buffer , new VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand	 );
		case e_encryptionUpdateAck : return DecodeChoice( buffer , new VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MiscellaneousCommand_Type::operator=(const VS_H245MiscellaneousCommand_Type & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_equaliseDelay : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_zeroDelay : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_multipointModeCommand : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cancelMultipointModeCommand : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoFreezePicture : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoFastUpdatePicture : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoFastUpdateGOB : CopyChoice<VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB	  >(src,*this);  return;
		case e_videoTemporalSpatialTradeOff : CopyChoice<TemplInteger<0,31,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_videoSendSyncEveryGOB : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoSendSyncEveryGOBCancel : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoFastUpdateMB : CopyChoice<VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB	  >(src,*this);  return;
		case e_maxH223MUXPDUsize : CopyChoice<TemplInteger<1,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_encryptionUpdate : CopyChoice< VS_H245EncryptionSync >(src,*this); return;
		case e_encryptionUpdateRequest : CopyChoice< VS_H245EncryptionUpdateRequest >(src,*this); return;
		case e_switchReceiveMediaOff : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_switchReceiveMediaOn : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_progressiveRefinementStart : CopyChoice<VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart	  >(src,*this);  return;
		case e_progressiveRefinementAbortOne : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_progressiveRefinementAbortContinuous : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoBadMBs : CopyChoice<VS_H245MiscellaneousCommand_Type_VideoBadMBs	  >(src,*this);  return;
		case e_lostPicture : CopyChoice< VS_H245PictureReference >(src,*this); return;
		case e_lostPartialPicture : CopyChoice<VS_H245MiscellaneousCommand_Type_LostPartialPicture	  >(src,*this);  return;
		case e_recoveryReferencePicture : CopyChoice< VS_H245PictureReference >(src,*this); return;
		case e_encryptionUpdateCommand : CopyChoice<VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand	  >(src,*this);  return;
		case e_encryptionUpdateAck : CopyChoice<VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MiscellaneousCommand_Type::operator VS_H245EncryptionSync *( void )
	{	return dynamic_cast< VS_H245EncryptionSync * >(choice);    }

 	VS_H245MiscellaneousCommand_Type::operator VS_H245EncryptionUpdateRequest *( void )
	{	return dynamic_cast< VS_H245EncryptionUpdateRequest * >(choice);    }

 	VS_H245MiscellaneousCommand_Type::operator VS_H245PictureReference *( void )
	{	return dynamic_cast< VS_H245PictureReference * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MiscellaneousCommand_Type::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_equaliseDelay :  dprint4("choice:  VS_AsnNull  ");return;
		case e_zeroDelay :  dprint4("choice:  VS_AsnNull  ");return;
		case e_multipointModeCommand :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cancelMultipointModeCommand :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoFreezePicture :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoFastUpdatePicture :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoFastUpdateGOB :  dprint4("choice: VS_H245MiscellaneousCommand_Type_VideoFastUpdateGOB	 ");return;
		case e_videoTemporalSpatialTradeOff :  dprint4("choice: TemplInteger<0,31,VS_Asn::FixedConstraint,0>  ");return;
		case e_videoSendSyncEveryGOB :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoSendSyncEveryGOBCancel :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoFastUpdateMB :  dprint4("choice: VS_H245MiscellaneousCommand_Type_VideoFastUpdateMB	 ");return;
		case e_maxH223MUXPDUsize :  dprint4("choice: TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_encryptionUpdate :  dprint4("choice: VS_H245EncryptionSync ");return;
		case e_encryptionUpdateRequest :  dprint4("choice: VS_H245EncryptionUpdateRequest ");return;
		case e_switchReceiveMediaOff :  dprint4("choice:  VS_AsnNull  ");return;
		case e_switchReceiveMediaOn :  dprint4("choice:  VS_AsnNull  ");return;
		case e_progressiveRefinementStart :  dprint4("choice: VS_H245MiscellaneousCommand_Type_ProgressiveRefinementStart	 ");return;
		case e_progressiveRefinementAbortOne :  dprint4("choice:  VS_AsnNull  ");return;
		case e_progressiveRefinementAbortContinuous :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoBadMBs :  dprint4("choice: VS_H245MiscellaneousCommand_Type_VideoBadMBs	 ");return;
		case e_lostPicture :  dprint4("choice: VS_H245PictureReference ");return;
		case e_lostPartialPicture :  dprint4("choice: VS_H245MiscellaneousCommand_Type_LostPartialPicture	 ");return;
		case e_recoveryReferencePicture :  dprint4("choice: VS_H245PictureReference ");return;
		case e_encryptionUpdateCommand :  dprint4("choice: VS_H245MiscellaneousCommand_Type_EncryptionUpdateCommand	 ");return;
		case e_encryptionUpdateAck :  dprint4("choice: VS_H245MiscellaneousCommand_Type_EncryptionUpdateAck	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MiscellaneousCommand /////////////////////////
 	 VS_H245MiscellaneousCommand :: VS_H245MiscellaneousCommand( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&logicalChannelNumber,0);
		ref[1].Set(&type,0);
		e_ref[0].Set(&direction,1);
	}
	void VS_H245MiscellaneousCommand::operator=(const VS_H245MiscellaneousCommand& src)
	{

		O_CC(filled);
		O_C(logicalChannelNumber);
		O_C(type);
		O_C(direction);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245EncryptionUpdateDirection /////////////////////////
 	 VS_H245EncryptionUpdateDirection::VS_H245EncryptionUpdateDirection( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245EncryptionUpdateDirection::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_masterToSlave : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_slaveToMaster : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245EncryptionUpdateDirection::operator=(const VS_H245EncryptionUpdateDirection & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_masterToSlave : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_slaveToMaster : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245EncryptionUpdateDirection::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_masterToSlave :  dprint4("choice:  VS_AsnNull  ");return;
		case e_slaveToMaster :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245SubstituteConferenceIDCommand /////////////////////////
 	 VS_H245SubstituteConferenceIDCommand :: VS_H245SubstituteConferenceIDCommand( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&conferenceIdentifier,0);
	}
	void VS_H245SubstituteConferenceIDCommand::operator=(const VS_H245SubstituteConferenceIDCommand& src)
	{

		O_CC(filled);
		O_C(conferenceIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceCommand /////////////////////////
 	 VS_H245ConferenceCommand::VS_H245ConferenceCommand( void )
	:VS_AsnChoice(7 , 8 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ConferenceCommand::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_broadcastMyLogicalChannel : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_cancelBroadcastMyLogicalChannel : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_makeTerminalBroadcaster : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_cancelMakeTerminalBroadcaster : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_sendThisSource : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_cancelSendThisSource : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_dropConference : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_substituteConferenceIDCommand : return DecodeChoice( buffer , new VS_H245SubstituteConferenceIDCommand);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ConferenceCommand::operator=(const VS_H245ConferenceCommand & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_broadcastMyLogicalChannel : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_cancelBroadcastMyLogicalChannel : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_makeTerminalBroadcaster : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_cancelMakeTerminalBroadcaster : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_sendThisSource : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_cancelSendThisSource : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_dropConference : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_substituteConferenceIDCommand : CopyChoice< VS_H245SubstituteConferenceIDCommand >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245ConferenceCommand::operator VS_H245LogicalChannelNumber *( void )
	{	return dynamic_cast< VS_H245LogicalChannelNumber * >(choice);    }

 	VS_H245ConferenceCommand::operator VS_H245TerminalLabel *( void )
	{	return dynamic_cast< VS_H245TerminalLabel * >(choice);    }

 	VS_H245ConferenceCommand::operator VS_H245SubstituteConferenceIDCommand *( void )
	{	return dynamic_cast< VS_H245SubstituteConferenceIDCommand * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ConferenceCommand::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_broadcastMyLogicalChannel :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_cancelBroadcastMyLogicalChannel :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_makeTerminalBroadcaster :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_cancelMakeTerminalBroadcaster :  dprint4("choice:  VS_AsnNull  ");return;
		case e_sendThisSource :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_cancelSendThisSource :  dprint4("choice:  VS_AsnNull  ");return;
		case e_dropConference :  dprint4("choice:  VS_AsnNull  ");return;
		case e_substituteConferenceIDCommand :  dprint4("choice: VS_H245SubstituteConferenceIDCommand ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245EndSessionCommand_GstnOptions /////////////////////////
 	 VS_H245EndSessionCommand_GstnOptions::VS_H245EndSessionCommand_GstnOptions( void )
	:VS_AsnChoice(5 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245EndSessionCommand_GstnOptions::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_telephonyMode : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_v8bis : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_v34DSVD : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_v34DuplexFAX : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_v34H324 : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245EndSessionCommand_GstnOptions::operator=(const VS_H245EndSessionCommand_GstnOptions & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_telephonyMode : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_v8bis : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_v34DSVD : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_v34DuplexFAX : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_v34H324 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245EndSessionCommand_GstnOptions::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_telephonyMode :  dprint4("choice:  VS_AsnNull  ");return;
		case e_v8bis :  dprint4("choice:  VS_AsnNull  ");return;
		case e_v34DSVD :  dprint4("choice:  VS_AsnNull  ");return;
		case e_v34DuplexFAX :  dprint4("choice:  VS_AsnNull  ");return;
		case e_v34H324 :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245EndSessionCommand_IsdnOptions /////////////////////////
 	 VS_H245EndSessionCommand_IsdnOptions::VS_H245EndSessionCommand_IsdnOptions( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245EndSessionCommand_IsdnOptions::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_telephonyMode : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_v140 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_terminalOnHold : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245EndSessionCommand_IsdnOptions::operator=(const VS_H245EndSessionCommand_IsdnOptions & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_telephonyMode : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_v140 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_terminalOnHold : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245EndSessionCommand_IsdnOptions::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_telephonyMode :  dprint4("choice:  VS_AsnNull  ");return;
		case e_v140 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_terminalOnHold :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245EndSessionCommand /////////////////////////
 	 VS_H245EndSessionCommand::VS_H245EndSessionCommand( void )
	:VS_AsnChoice(3 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245EndSessionCommand::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_disconnect : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_gstnOptions : return DecodeChoice( buffer , new VS_H245EndSessionCommand_GstnOptions	 );
		case e_isdnOptions : return DecodeChoice( buffer , new VS_H245EndSessionCommand_IsdnOptions	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245EndSessionCommand::operator=(const VS_H245EndSessionCommand & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_disconnect : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_gstnOptions : CopyChoice<VS_H245EndSessionCommand_GstnOptions	  >(src,*this);  return;
		case e_isdnOptions : CopyChoice<VS_H245EndSessionCommand_IsdnOptions	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245EndSessionCommand::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245EndSessionCommand::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_disconnect :  dprint4("choice:  VS_AsnNull  ");return;
		case e_gstnOptions :  dprint4("choice: VS_H245EndSessionCommand_GstnOptions	 ");return;
		case e_isdnOptions :  dprint4("choice: VS_H245EndSessionCommand_IsdnOptions	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FlowControlCommand_Scope /////////////////////////
 	 VS_H245FlowControlCommand_Scope::VS_H245FlowControlCommand_Scope( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FlowControlCommand_Scope::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_logicalChannelNumber : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_resourceID : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		case e_wholeMultiplex : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245FlowControlCommand_Scope::operator=(const VS_H245FlowControlCommand_Scope & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_logicalChannelNumber : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_resourceID : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_wholeMultiplex : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245FlowControlCommand_Scope::operator VS_H245LogicalChannelNumber *( void )
	{	return dynamic_cast< VS_H245LogicalChannelNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FlowControlCommand_Scope::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_logicalChannelNumber :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_resourceID :  dprint4("choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_wholeMultiplex :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FlowControlCommand_Restriction /////////////////////////
 	 VS_H245FlowControlCommand_Restriction::VS_H245FlowControlCommand_Restriction( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FlowControlCommand_Restriction::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_maximumBitRate : return DecodeChoice( buffer , new TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>  );
		case e_noRestriction : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245FlowControlCommand_Restriction::operator=(const VS_H245FlowControlCommand_Restriction & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_maximumBitRate : CopyChoice<TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_noRestriction : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FlowControlCommand_Restriction::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_maximumBitRate :  dprint4("choice: TemplInteger<0,16777215,VS_Asn::FixedConstraint,0>  ");return;
		case e_noRestriction :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FlowControlCommand /////////////////////////
 	 VS_H245FlowControlCommand :: VS_H245FlowControlCommand( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&scope,0);
		ref[1].Set(&restriction,0);
	}
	void VS_H245FlowControlCommand::operator=(const VS_H245FlowControlCommand& src)
	{

		O_CC(filled);
		O_C(scope);
		O_C(restriction);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245EncryptionCommand_EncryptionAlgorithmID /////////////////////////
 	 VS_H245EncryptionCommand_EncryptionAlgorithmID :: VS_H245EncryptionCommand_EncryptionAlgorithmID( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 0 )
	{
		ref[0].Set(&h233AlgorithmIdentifier,0);
		ref[1].Set(&associatedAlgorithm,0);
	}
	void VS_H245EncryptionCommand_EncryptionAlgorithmID::operator=(const VS_H245EncryptionCommand_EncryptionAlgorithmID& src)
	{

		O_CC(filled);
		O_C(h233AlgorithmIdentifier);
		O_C(associatedAlgorithm);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245EncryptionCommand /////////////////////////
 	 VS_H245EncryptionCommand::VS_H245EncryptionCommand( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245EncryptionCommand::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_encryptionSE : return DecodeChoice( buffer , new TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  );
		case e_encryptionIVRequest : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_encryptionAlgorithmID : return DecodeChoice( buffer , new VS_H245EncryptionCommand_EncryptionAlgorithmID	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245EncryptionCommand::operator=(const VS_H245EncryptionCommand & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_encryptionSE : CopyChoice<TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>   >(src,*this);  return;
		case e_encryptionIVRequest : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_encryptionAlgorithmID : CopyChoice<VS_H245EncryptionCommand_EncryptionAlgorithmID	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245EncryptionCommand::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_encryptionSE :  dprint4("choice: TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
		case e_encryptionIVRequest :  dprint4("choice:  VS_AsnNull  ");return;
		case e_encryptionAlgorithmID :  dprint4("choice: VS_H245EncryptionCommand_EncryptionAlgorithmID	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245SendTerminalCapabilitySet_SpecificRequest /////////////////////////
 	 VS_H245SendTerminalCapabilitySet_SpecificRequest :: VS_H245SendTerminalCapabilitySet_SpecificRequest( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&multiplexCapability,0);
		ref[1].Set(&capabilityTableEntryNumbers,1);
		ref[2].Set(&capabilityDescriptorNumbers,1);
	}
	void VS_H245SendTerminalCapabilitySet_SpecificRequest::operator=(const VS_H245SendTerminalCapabilitySet_SpecificRequest& src)
	{

		O_CC(filled);
		O_C(multiplexCapability);
		O_C(capabilityTableEntryNumbers);
		O_C(capabilityDescriptorNumbers);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245SendTerminalCapabilitySet /////////////////////////
 	 VS_H245SendTerminalCapabilitySet::VS_H245SendTerminalCapabilitySet( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245SendTerminalCapabilitySet::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_specificRequest : return DecodeChoice( buffer , new VS_H245SendTerminalCapabilitySet_SpecificRequest	 );
		case e_genericRequest : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245SendTerminalCapabilitySet::operator=(const VS_H245SendTerminalCapabilitySet & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_specificRequest : CopyChoice<VS_H245SendTerminalCapabilitySet_SpecificRequest	  >(src,*this);  return;
		case e_genericRequest : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245SendTerminalCapabilitySet::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_specificRequest :  dprint4("choice: VS_H245SendTerminalCapabilitySet_SpecificRequest	 ");return;
		case e_genericRequest :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245LogicalChannelRateRelease /////////////////////////
 	 VS_H245LogicalChannelRateRelease :: VS_H245LogicalChannelRateRelease( void )
	:VS_AsnSequence(0 , nullptr, basic_root, nullptr, extension_root , 1 )
	{
	}
	void VS_H245LogicalChannelRateRelease::operator=(const VS_H245LogicalChannelRateRelease& src)
	{

		O_CC(filled);
		O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245LogicalChannelRateRejectReason /////////////////////////
 	 VS_H245LogicalChannelRateRejectReason::VS_H245LogicalChannelRateRejectReason( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245LogicalChannelRateRejectReason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_undefinedReason : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_insufficientResources : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245LogicalChannelRateRejectReason::operator=(const VS_H245LogicalChannelRateRejectReason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_undefinedReason : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_insufficientResources : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245LogicalChannelRateRejectReason::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_undefinedReason :  dprint4("choice:  VS_AsnNull  ");return;
		case e_insufficientResources :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245LogicalChannelRateReject /////////////////////////
 	 VS_H245LogicalChannelRateReject :: VS_H245LogicalChannelRateReject( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&logicalChannelNumber,0);
		ref[2].Set(&rejectReason,0);
		ref[3].Set(&currentMaximumBitRate,1);
	}
	void VS_H245LogicalChannelRateReject::operator=(const VS_H245LogicalChannelRateReject& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(logicalChannelNumber);
		O_C(rejectReason);
		O_C(currentMaximumBitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245LogicalChannelRateAcknowledge /////////////////////////
 	 VS_H245LogicalChannelRateAcknowledge :: VS_H245LogicalChannelRateAcknowledge( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&logicalChannelNumber,0);
		ref[2].Set(&maximumBitRate,0);
	}
	void VS_H245LogicalChannelRateAcknowledge::operator=(const VS_H245LogicalChannelRateAcknowledge& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(logicalChannelNumber);
		O_C(maximumBitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245LogicalChannelRateRequest /////////////////////////
 	 VS_H245LogicalChannelRateRequest :: VS_H245LogicalChannelRateRequest( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&logicalChannelNumber,0);
		ref[2].Set(&maximumBitRate,0);
	}
	void VS_H245LogicalChannelRateRequest::operator=(const VS_H245LogicalChannelRateRequest& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(logicalChannelNumber);
		O_C(maximumBitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConnectionIdentifier /////////////////////////
 	 VS_H245ConnectionIdentifier :: VS_H245ConnectionIdentifier( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&channelTag,0);
		ref[1].Set(&sequenceNumber,0);
	}
	void VS_H245ConnectionIdentifier::operator=(const VS_H245ConnectionIdentifier& src)
	{

		O_CC(filled);
		O_C(channelTag);
		O_C(sequenceNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DialingInformationNetworkType /////////////////////////
 	 VS_H245DialingInformationNetworkType::VS_H245DialingInformationNetworkType( void )
	:VS_AsnChoice(3 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DialingInformationNetworkType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_n_isdn : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_gstn : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_mobile : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DialingInformationNetworkType::operator=(const VS_H245DialingInformationNetworkType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_n_isdn : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_gstn : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_mobile : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245DialingInformationNetworkType::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DialingInformationNetworkType::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_n_isdn :  dprint4("choice:  VS_AsnNull  ");return;
		case e_gstn :  dprint4("choice:  VS_AsnNull  ");return;
		case e_mobile :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DialingInformationNumber /////////////////////////
 	 VS_H245DialingInformationNumber :: VS_H245DialingInformationNumber( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&networkAddress,0);
		ref[1].Set(&subAddress,1);
		ref[2].Set(&networkType,0);
	}
	void VS_H245DialingInformationNumber::operator=(const VS_H245DialingInformationNumber& src)
	{

		O_CC(filled);
		O_C(networkAddress);
		O_C(subAddress);
		O_C(networkType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DialingInformation /////////////////////////
 	 VS_H245DialingInformation::VS_H245DialingInformation( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DialingInformation::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_differential : return DecodeChoice( buffer , new VS_H245DialingInformationNumber);
		case e_infoNotAvailable : return DecodeChoice( buffer , new TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DialingInformation::operator=(const VS_H245DialingInformation & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_differential : CopyChoice< VS_H245DialingInformationNumber >(src,*this); return;
		case e_infoNotAvailable : CopyChoice<TemplInteger<1,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245DialingInformation::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }

 	VS_H245DialingInformation::operator VS_H245DialingInformationNumber *( void )
	{	return dynamic_cast< VS_H245DialingInformationNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DialingInformation::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_differential :  dprint4("choice: VS_H245DialingInformationNumber ");return;
		case e_infoNotAvailable :  dprint4("choice: TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultilinkIndication_CrcDesired /////////////////////////
 	 VS_H245MultilinkIndication_CrcDesired :: VS_H245MultilinkIndication_CrcDesired( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr, extension_root , 1 )
	{
	}
	void VS_H245MultilinkIndication_CrcDesired::operator=(const VS_H245MultilinkIndication_CrcDesired& src)
	{

		O_CC(filled);
		O_CP(e_ref);  O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkIndication_ExcessiveError /////////////////////////
 	 VS_H245MultilinkIndication_ExcessiveError :: VS_H245MultilinkIndication_ExcessiveError( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&connectionIdentifier,0);
	}
	void VS_H245MultilinkIndication_ExcessiveError::operator=(const VS_H245MultilinkIndication_ExcessiveError& src)
	{

		O_CC(filled);
		O_C(connectionIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkIndication /////////////////////////
 	 VS_H245MultilinkIndication::VS_H245MultilinkIndication( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultilinkIndication::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_crcDesired : return DecodeChoice( buffer , new VS_H245MultilinkIndication_CrcDesired	 );
		case e_excessiveError : return DecodeChoice( buffer , new VS_H245MultilinkIndication_ExcessiveError	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultilinkIndication::operator=(const VS_H245MultilinkIndication & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_crcDesired : CopyChoice<VS_H245MultilinkIndication_CrcDesired	  >(src,*this);  return;
		case e_excessiveError : CopyChoice<VS_H245MultilinkIndication_ExcessiveError	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MultilinkIndication::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultilinkIndication::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_crcDesired :  dprint4("choice: VS_H245MultilinkIndication_CrcDesired	 ");return;
		case e_excessiveError :  dprint4("choice: VS_H245MultilinkIndication_ExcessiveError	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultilinkResponse_CallInformation /////////////////////////
 	 VS_H245MultilinkResponse_CallInformation :: VS_H245MultilinkResponse_CallInformation( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&dialingInformation,0);
		ref[1].Set(&callAssociationNumber,0);
	}
	void VS_H245MultilinkResponse_CallInformation::operator=(const VS_H245MultilinkResponse_CallInformation& src)
	{

		O_CC(filled);
		O_C(dialingInformation);
		O_C(callAssociationNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected /////////////////////////
 	 VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected::VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_connectionsNotAvailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_userRejected : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected::operator=(const VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_connectionsNotAvailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_userRejected : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_connectionsNotAvailable :  dprint4("choice:  VS_AsnNull  ");return;
		case e_userRejected :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultilinkResponse_AddConnection_ResponseCode /////////////////////////
 	 VS_H245MultilinkResponse_AddConnection_ResponseCode::VS_H245MultilinkResponse_AddConnection_ResponseCode( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultilinkResponse_AddConnection_ResponseCode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_accepted : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_rejected : return DecodeChoice( buffer , new VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultilinkResponse_AddConnection_ResponseCode::operator=(const VS_H245MultilinkResponse_AddConnection_ResponseCode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_accepted : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_rejected : CopyChoice<VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultilinkResponse_AddConnection_ResponseCode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_accepted :  dprint4("choice:  VS_AsnNull  ");return;
		case e_rejected :  dprint4("choice: VS_H245MultilinkResponse_AddConnection_ResponseCode_Rejected	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultilinkResponse_AddConnection /////////////////////////
 	 VS_H245MultilinkResponse_AddConnection :: VS_H245MultilinkResponse_AddConnection( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&responseCode,0);
	}
	void VS_H245MultilinkResponse_AddConnection::operator=(const VS_H245MultilinkResponse_AddConnection& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(responseCode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkResponse_RemoveConnection /////////////////////////
 	 VS_H245MultilinkResponse_RemoveConnection :: VS_H245MultilinkResponse_RemoveConnection( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&connectionIdentifier,0);
	}
	void VS_H245MultilinkResponse_RemoveConnection::operator=(const VS_H245MultilinkResponse_RemoveConnection& src)
	{

		O_CC(filled);
		O_C(connectionIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkResponse_MaximumHeaderInterval /////////////////////////
 	 VS_H245MultilinkResponse_MaximumHeaderInterval :: VS_H245MultilinkResponse_MaximumHeaderInterval( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&currentInterval,0);
	}
	void VS_H245MultilinkResponse_MaximumHeaderInterval::operator=(const VS_H245MultilinkResponse_MaximumHeaderInterval& src)
	{

		O_CC(filled);
		O_C(currentInterval);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkResponse /////////////////////////
 	 VS_H245MultilinkResponse::VS_H245MultilinkResponse( void )
	:VS_AsnChoice(5 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultilinkResponse::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_callInformation : return DecodeChoice( buffer , new VS_H245MultilinkResponse_CallInformation	 );
		case e_addConnection : return DecodeChoice( buffer , new VS_H245MultilinkResponse_AddConnection	 );
		case e_removeConnection : return DecodeChoice( buffer , new VS_H245MultilinkResponse_RemoveConnection	 );
		case e_maximumHeaderInterval : return DecodeChoice( buffer , new VS_H245MultilinkResponse_MaximumHeaderInterval	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultilinkResponse::operator=(const VS_H245MultilinkResponse & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_callInformation : CopyChoice<VS_H245MultilinkResponse_CallInformation	  >(src,*this);  return;
		case e_addConnection : CopyChoice<VS_H245MultilinkResponse_AddConnection	  >(src,*this);  return;
		case e_removeConnection : CopyChoice<VS_H245MultilinkResponse_RemoveConnection	  >(src,*this);  return;
		case e_maximumHeaderInterval : CopyChoice<VS_H245MultilinkResponse_MaximumHeaderInterval	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MultilinkResponse::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultilinkResponse::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_callInformation :  dprint4("choice: VS_H245MultilinkResponse_CallInformation	 ");return;
		case e_addConnection :  dprint4("choice: VS_H245MultilinkResponse_AddConnection	 ");return;
		case e_removeConnection :  dprint4("choice: VS_H245MultilinkResponse_RemoveConnection	 ");return;
		case e_maximumHeaderInterval :  dprint4("choice: VS_H245MultilinkResponse_MaximumHeaderInterval	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultilinkRequest_CallInformation /////////////////////////
 	 VS_H245MultilinkRequest_CallInformation :: VS_H245MultilinkRequest_CallInformation( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&maxNumberOfAdditionalConnections,0);
	}
	void VS_H245MultilinkRequest_CallInformation::operator=(const VS_H245MultilinkRequest_CallInformation& src)
	{

		O_CC(filled);
		O_C(maxNumberOfAdditionalConnections);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkRequest_AddConnection /////////////////////////
 	 VS_H245MultilinkRequest_AddConnection :: VS_H245MultilinkRequest_AddConnection( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&dialingInformation,0);
	}
	void VS_H245MultilinkRequest_AddConnection::operator=(const VS_H245MultilinkRequest_AddConnection& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(dialingInformation);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkRequest_RemoveConnection /////////////////////////
 	 VS_H245MultilinkRequest_RemoveConnection :: VS_H245MultilinkRequest_RemoveConnection( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&connectionIdentifier,0);
	}
	void VS_H245MultilinkRequest_RemoveConnection::operator=(const VS_H245MultilinkRequest_RemoveConnection& src)
	{

		O_CC(filled);
		O_C(connectionIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType /////////////////////////
 	 VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType::VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_currentIntervalInformation : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_requestedInterval : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType::operator=(const VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_currentIntervalInformation : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_requestedInterval : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultilinkRequest_MaximumHeaderInterval_RequestType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_currentIntervalInformation :  dprint4("choice:  VS_AsnNull  ");return;
		case e_requestedInterval :  dprint4("choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultilinkRequest_MaximumHeaderInterval /////////////////////////
 	 VS_H245MultilinkRequest_MaximumHeaderInterval :: VS_H245MultilinkRequest_MaximumHeaderInterval( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&requestType,0);
	}
	void VS_H245MultilinkRequest_MaximumHeaderInterval::operator=(const VS_H245MultilinkRequest_MaximumHeaderInterval& src)
	{

		O_CC(filled);
		O_C(requestType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultilinkRequest /////////////////////////
 	 VS_H245MultilinkRequest::VS_H245MultilinkRequest( void )
	:VS_AsnChoice(5 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultilinkRequest::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_callInformation : return DecodeChoice( buffer , new VS_H245MultilinkRequest_CallInformation	 );
		case e_addConnection : return DecodeChoice( buffer , new VS_H245MultilinkRequest_AddConnection	 );
		case e_removeConnection : return DecodeChoice( buffer , new VS_H245MultilinkRequest_RemoveConnection	 );
		case e_maximumHeaderInterval : return DecodeChoice( buffer , new VS_H245MultilinkRequest_MaximumHeaderInterval	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultilinkRequest::operator=(const VS_H245MultilinkRequest & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_callInformation : CopyChoice<VS_H245MultilinkRequest_CallInformation	  >(src,*this);  return;
		case e_addConnection : CopyChoice<VS_H245MultilinkRequest_AddConnection	  >(src,*this);  return;
		case e_removeConnection : CopyChoice<VS_H245MultilinkRequest_RemoveConnection	  >(src,*this);  return;
		case e_maximumHeaderInterval : CopyChoice<VS_H245MultilinkRequest_MaximumHeaderInterval	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MultilinkRequest::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultilinkRequest::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_callInformation :  dprint4("choice: VS_H245MultilinkRequest_CallInformation	 ");return;
		case e_addConnection :  dprint4("choice: VS_H245MultilinkRequest_AddConnection	 ");return;
		case e_removeConnection :  dprint4("choice: VS_H245MultilinkRequest_RemoveConnection	 ");return;
		case e_maximumHeaderInterval :  dprint4("choice: VS_H245MultilinkRequest_MaximumHeaderInterval	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RemoteMCResponse_Reject /////////////////////////
 	 VS_H245RemoteMCResponse_Reject::VS_H245RemoteMCResponse_Reject( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RemoteMCResponse_Reject::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unspecified : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_functionNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RemoteMCResponse_Reject::operator=(const VS_H245RemoteMCResponse_Reject & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unspecified : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_functionNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RemoteMCResponse_Reject::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unspecified :  dprint4("choice:  VS_AsnNull  ");return;
		case e_functionNotSupported :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RemoteMCResponse /////////////////////////
 	 VS_H245RemoteMCResponse::VS_H245RemoteMCResponse( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RemoteMCResponse::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_accept : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_reject : return DecodeChoice( buffer , new VS_H245RemoteMCResponse_Reject	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RemoteMCResponse::operator=(const VS_H245RemoteMCResponse & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_accept : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_reject : CopyChoice<VS_H245RemoteMCResponse_Reject	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RemoteMCResponse::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_accept :  dprint4("choice:  VS_AsnNull  ");return;
		case e_reject :  dprint4("choice: VS_H245RemoteMCResponse_Reject	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RemoteMCRequest /////////////////////////
 	 VS_H245RemoteMCRequest::VS_H245RemoteMCRequest( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RemoteMCRequest::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_masterActivate : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_slaveActivate : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_deActivate : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RemoteMCRequest::operator=(const VS_H245RemoteMCRequest & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_masterActivate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_slaveActivate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_deActivate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RemoteMCRequest::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_masterActivate :  dprint4("choice:  VS_AsnNull  ");return;
		case e_slaveActivate :  dprint4("choice:  VS_AsnNull  ");return;
		case e_deActivate :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245TerminalInformation /////////////////////////
 	 VS_H245TerminalInformation :: VS_H245TerminalInformation( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&terminalLabel,0);
		ref[1].Set(&terminalID,0);
	}
	void VS_H245TerminalInformation::operator=(const VS_H245TerminalInformation& src)
	{

		O_CC(filled);
		O_C(terminalLabel);
		O_C(terminalID);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestAllTerminalIDsResponse /////////////////////////
 	 VS_H245RequestAllTerminalIDsResponse :: VS_H245RequestAllTerminalIDsResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&terminalInformation,0);
	}
	void VS_H245RequestAllTerminalIDsResponse::operator=(const VS_H245RequestAllTerminalIDsResponse& src)
	{

		O_CC(filled);
		O_C(terminalInformation);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceResponse_MCTerminalIDResponse /////////////////////////
 	 VS_H245ConferenceResponse_MCTerminalIDResponse :: VS_H245ConferenceResponse_MCTerminalIDResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&terminalLabel,0);
		ref[1].Set(&terminalID,0);
	}
	void VS_H245ConferenceResponse_MCTerminalIDResponse::operator=(const VS_H245ConferenceResponse_MCTerminalIDResponse& src)
	{

		O_CC(filled);
		O_C(terminalLabel);
		O_C(terminalID);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceResponse_TerminalIDResponse /////////////////////////
 	 VS_H245ConferenceResponse_TerminalIDResponse :: VS_H245ConferenceResponse_TerminalIDResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&terminalLabel,0);
		ref[1].Set(&terminalID,0);
	}
	void VS_H245ConferenceResponse_TerminalIDResponse::operator=(const VS_H245ConferenceResponse_TerminalIDResponse& src)
	{

		O_CC(filled);
		O_C(terminalLabel);
		O_C(terminalID);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceResponse_ConferenceIDResponse /////////////////////////
 	 VS_H245ConferenceResponse_ConferenceIDResponse :: VS_H245ConferenceResponse_ConferenceIDResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&terminalLabel,0);
		ref[1].Set(&conferenceID,0);
	}
	void VS_H245ConferenceResponse_ConferenceIDResponse::operator=(const VS_H245ConferenceResponse_ConferenceIDResponse& src)
	{

		O_CC(filled);
		O_C(terminalLabel);
		O_C(conferenceID);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceResponse_PasswordResponse /////////////////////////
 	 VS_H245ConferenceResponse_PasswordResponse :: VS_H245ConferenceResponse_PasswordResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&terminalLabel,0);
		ref[1].Set(&password,0);
	}
	void VS_H245ConferenceResponse_PasswordResponse::operator=(const VS_H245ConferenceResponse_PasswordResponse& src)
	{

		O_CC(filled);
		O_C(terminalLabel);
		O_C(password);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceResponse_MakeMeChairResponse /////////////////////////
 	 VS_H245ConferenceResponse_MakeMeChairResponse::VS_H245ConferenceResponse_MakeMeChairResponse( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ConferenceResponse_MakeMeChairResponse::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_grantedChairToken : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_deniedChairToken : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ConferenceResponse_MakeMeChairResponse::operator=(const VS_H245ConferenceResponse_MakeMeChairResponse & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_grantedChairToken : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_deniedChairToken : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ConferenceResponse_MakeMeChairResponse::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_grantedChairToken :  dprint4("choice:  VS_AsnNull  ");return;
		case e_deniedChairToken :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245ConferenceResponse_ExtensionAddressResponse /////////////////////////
 	 VS_H245ConferenceResponse_ExtensionAddressResponse :: VS_H245ConferenceResponse_ExtensionAddressResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr, extension_root , 1 )
	{
		ref[0].Set(&extensionAddress,0);
	}
	void VS_H245ConferenceResponse_ExtensionAddressResponse::operator=(const VS_H245ConferenceResponse_ExtensionAddressResponse& src)
	{

		O_CC(filled);
		O_C(extensionAddress);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceResponse_ChairTokenOwnerResponse /////////////////////////
 	 VS_H245ConferenceResponse_ChairTokenOwnerResponse :: VS_H245ConferenceResponse_ChairTokenOwnerResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&terminalLabel,0);
		ref[1].Set(&terminalID,0);
	}
	void VS_H245ConferenceResponse_ChairTokenOwnerResponse::operator=(const VS_H245ConferenceResponse_ChairTokenOwnerResponse& src)
	{

		O_CC(filled);
		O_C(terminalLabel);
		O_C(terminalID);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceResponse_TerminalCertificateResponse /////////////////////////
 	 VS_H245ConferenceResponse_TerminalCertificateResponse :: VS_H245ConferenceResponse_TerminalCertificateResponse( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&terminalLabel,1);
		ref[1].Set(&certificateResponse,1);
	}
	void VS_H245ConferenceResponse_TerminalCertificateResponse::operator=(const VS_H245ConferenceResponse_TerminalCertificateResponse& src)
	{

		O_CC(filled);
		O_C(terminalLabel);
		O_C(certificateResponse);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse /////////////////////////
 	 VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse::VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_grantedBroadcastMyLogicalChannel : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_deniedBroadcastMyLogicalChannel : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse::operator=(const VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_grantedBroadcastMyLogicalChannel : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_deniedBroadcastMyLogicalChannel : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_grantedBroadcastMyLogicalChannel :  dprint4("choice:  VS_AsnNull  ");return;
		case e_deniedBroadcastMyLogicalChannel :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse /////////////////////////
 	 VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse::VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_grantedMakeTerminalBroadcaster : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_deniedMakeTerminalBroadcaster : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse::operator=(const VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_grantedMakeTerminalBroadcaster : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_deniedMakeTerminalBroadcaster : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_grantedMakeTerminalBroadcaster :  dprint4("choice:  VS_AsnNull  ");return;
		case e_deniedMakeTerminalBroadcaster :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245ConferenceResponse_SendThisSourceResponse /////////////////////////
 	 VS_H245ConferenceResponse_SendThisSourceResponse::VS_H245ConferenceResponse_SendThisSourceResponse( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ConferenceResponse_SendThisSourceResponse::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_grantedSendThisSource : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_deniedSendThisSource : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ConferenceResponse_SendThisSourceResponse::operator=(const VS_H245ConferenceResponse_SendThisSourceResponse & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_grantedSendThisSource : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_deniedSendThisSource : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ConferenceResponse_SendThisSourceResponse::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_grantedSendThisSource :  dprint4("choice:  VS_AsnNull  ");return;
		case e_deniedSendThisSource :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245ConferenceResponse /////////////////////////
 	 VS_H245ConferenceResponse::VS_H245ConferenceResponse( void )
	:VS_AsnChoice(8 , 16 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ConferenceResponse::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_mCTerminalIDResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_MCTerminalIDResponse	 );
		case e_terminalIDResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_TerminalIDResponse	 );
		case e_conferenceIDResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_ConferenceIDResponse	 );
		case e_passwordResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_PasswordResponse	 );
		case e_terminalListResponse : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_videoCommandReject : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_terminalDropReject : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_makeMeChairResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_MakeMeChairResponse	 );
		case e_extensionAddressResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_ExtensionAddressResponse	 );
		case e_chairTokenOwnerResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_ChairTokenOwnerResponse	 );
		case e_terminalCertificateResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_TerminalCertificateResponse	 );
		case e_broadcastMyLogicalChannelResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse	 );
		case e_makeTerminalBroadcasterResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse	 );
		case e_sendThisSourceResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse_SendThisSourceResponse	 );
		case e_requestAllTerminalIDsResponse : return DecodeChoice( buffer , new VS_H245RequestAllTerminalIDsResponse);
		case e_remoteMCResponse : return DecodeChoice( buffer , new VS_H245RemoteMCResponse);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ConferenceResponse::operator=(const VS_H245ConferenceResponse & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_mCTerminalIDResponse : CopyChoice<VS_H245ConferenceResponse_MCTerminalIDResponse	  >(src,*this);  return;
		case e_terminalIDResponse : CopyChoice<VS_H245ConferenceResponse_TerminalIDResponse	  >(src,*this);  return;
		case e_conferenceIDResponse : CopyChoice<VS_H245ConferenceResponse_ConferenceIDResponse	  >(src,*this);  return;
		case e_passwordResponse : CopyChoice<VS_H245ConferenceResponse_PasswordResponse	  >(src,*this);  return;
		case e_terminalListResponse : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_videoCommandReject : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_terminalDropReject : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_makeMeChairResponse : CopyChoice<VS_H245ConferenceResponse_MakeMeChairResponse	  >(src,*this);  return;
		case e_extensionAddressResponse : CopyChoice<VS_H245ConferenceResponse_ExtensionAddressResponse	  >(src,*this);  return;
		case e_chairTokenOwnerResponse : CopyChoice<VS_H245ConferenceResponse_ChairTokenOwnerResponse	  >(src,*this);  return;
		case e_terminalCertificateResponse : CopyChoice<VS_H245ConferenceResponse_TerminalCertificateResponse	  >(src,*this);  return;
		case e_broadcastMyLogicalChannelResponse : CopyChoice<VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse	  >(src,*this);  return;
		case e_makeTerminalBroadcasterResponse : CopyChoice<VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse	  >(src,*this);  return;
		case e_sendThisSourceResponse : CopyChoice<VS_H245ConferenceResponse_SendThisSourceResponse	  >(src,*this);  return;
		case e_requestAllTerminalIDsResponse : CopyChoice< VS_H245RequestAllTerminalIDsResponse >(src,*this); return;
		case e_remoteMCResponse : CopyChoice< VS_H245RemoteMCResponse >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245ConferenceResponse::operator VS_H245TerminalLabel *( void )
	{	return dynamic_cast< VS_H245TerminalLabel * >(choice);    }

 	VS_H245ConferenceResponse::operator VS_H245RequestAllTerminalIDsResponse *( void )
	{	return dynamic_cast< VS_H245RequestAllTerminalIDsResponse * >(choice);    }

 	VS_H245ConferenceResponse::operator VS_H245RemoteMCResponse *( void )
	{	return dynamic_cast< VS_H245RemoteMCResponse * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ConferenceResponse::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_mCTerminalIDResponse :  dprint4("choice: VS_H245ConferenceResponse_MCTerminalIDResponse	 ");return;
		case e_terminalIDResponse :  dprint4("choice: VS_H245ConferenceResponse_TerminalIDResponse	 ");return;
		case e_conferenceIDResponse :  dprint4("choice: VS_H245ConferenceResponse_ConferenceIDResponse	 ");return;
		case e_passwordResponse :  dprint4("choice: VS_H245ConferenceResponse_PasswordResponse	 ");return;
		case e_terminalListResponse :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_videoCommandReject :  dprint4("choice:  VS_AsnNull  ");return;
		case e_terminalDropReject :  dprint4("choice:  VS_AsnNull  ");return;
		case e_makeMeChairResponse :  dprint4("choice: VS_H245ConferenceResponse_MakeMeChairResponse	 ");return;
		case e_extensionAddressResponse :  dprint4("choice: VS_H245ConferenceResponse_ExtensionAddressResponse	 ");return;
		case e_chairTokenOwnerResponse :  dprint4("choice: VS_H245ConferenceResponse_ChairTokenOwnerResponse	 ");return;
		case e_terminalCertificateResponse :  dprint4("choice: VS_H245ConferenceResponse_TerminalCertificateResponse	 ");return;
		case e_broadcastMyLogicalChannelResponse :  dprint4("choice: VS_H245ConferenceResponse_BroadcastMyLogicalChannelResponse	 ");return;
		case e_makeTerminalBroadcasterResponse :  dprint4("choice: VS_H245ConferenceResponse_MakeTerminalBroadcasterResponse	 ");return;
		case e_sendThisSourceResponse :  dprint4("choice: VS_H245ConferenceResponse_SendThisSourceResponse	 ");return;
		case e_requestAllTerminalIDsResponse :  dprint4("choice: VS_H245RequestAllTerminalIDsResponse ");return;
		case e_remoteMCResponse :  dprint4("choice: VS_H245RemoteMCResponse ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245TerminalLabel /////////////////////////
 	 VS_H245TerminalLabel :: VS_H245TerminalLabel( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&mcuNumber,0);
		ref[1].Set(&terminalNumber,0);
	}
	void VS_H245TerminalLabel::operator=(const VS_H245TerminalLabel& src)
	{

		O_CC(filled);
		O_C(mcuNumber);
		O_C(terminalNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245Criteria /////////////////////////
 	 VS_H245Criteria :: VS_H245Criteria( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&field,0);
		ref[1].Set(&value,0);
	}
	void VS_H245Criteria::operator=(const VS_H245Criteria& src)
	{

		O_CC(filled);
		O_C(field);
		O_C(value);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceRequest_RequestTerminalCertificate /////////////////////////
 	 VS_H245ConferenceRequest_RequestTerminalCertificate :: VS_H245ConferenceRequest_RequestTerminalCertificate( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&terminalLabel,1);
		ref[1].Set(&certSelectionCriteria,1);
		ref[2].Set(&sRandom,1);
	}
	void VS_H245ConferenceRequest_RequestTerminalCertificate::operator=(const VS_H245ConferenceRequest_RequestTerminalCertificate& src)
	{

		O_CC(filled);
		O_C(terminalLabel);
		O_C(certSelectionCriteria);
		O_C(sRandom);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceRequest /////////////////////////
 	 VS_H245ConferenceRequest::VS_H245ConferenceRequest( void )
	:VS_AsnChoice(8 , 16 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ConferenceRequest::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_terminalListRequest : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_makeMeChair : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cancelMakeMeChair : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_dropTerminal : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_requestTerminalID : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_enterH243Password : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_enterH243TerminalID : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_enterH243ConferenceID : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_enterExtensionAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_requestChairTokenOwner : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_requestTerminalCertificate : return DecodeChoice( buffer , new VS_H245ConferenceRequest_RequestTerminalCertificate	 );
		case e_broadcastMyLogicalChannel : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_makeTerminalBroadcaster : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_sendThisSource : return DecodeChoice( buffer , new VS_H245TerminalLabel);
		case e_requestAllTerminalIDs : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_remoteMCRequest : return DecodeChoice( buffer , new VS_H245RemoteMCRequest);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ConferenceRequest::operator=(const VS_H245ConferenceRequest & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_terminalListRequest : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_makeMeChair : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cancelMakeMeChair : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_dropTerminal : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_requestTerminalID : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_enterH243Password : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_enterH243TerminalID : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_enterH243ConferenceID : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_enterExtensionAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_requestChairTokenOwner : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_requestTerminalCertificate : CopyChoice<VS_H245ConferenceRequest_RequestTerminalCertificate	  >(src,*this);  return;
		case e_broadcastMyLogicalChannel : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_makeTerminalBroadcaster : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_sendThisSource : CopyChoice< VS_H245TerminalLabel >(src,*this); return;
		case e_requestAllTerminalIDs : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_remoteMCRequest : CopyChoice< VS_H245RemoteMCRequest >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245ConferenceRequest::operator VS_H245TerminalLabel *( void )
	{	return dynamic_cast< VS_H245TerminalLabel * >(choice);    }

 	VS_H245ConferenceRequest::operator VS_H245LogicalChannelNumber *( void )
	{	return dynamic_cast< VS_H245LogicalChannelNumber * >(choice);    }

 	VS_H245ConferenceRequest::operator VS_H245RemoteMCRequest *( void )
	{	return dynamic_cast< VS_H245RemoteMCRequest * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ConferenceRequest::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_terminalListRequest :  dprint4("choice:  VS_AsnNull  ");return;
		case e_makeMeChair :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cancelMakeMeChair :  dprint4("choice:  VS_AsnNull  ");return;
		case e_dropTerminal :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_requestTerminalID :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_enterH243Password :  dprint4("choice:  VS_AsnNull  ");return;
		case e_enterH243TerminalID :  dprint4("choice:  VS_AsnNull  ");return;
		case e_enterH243ConferenceID :  dprint4("choice:  VS_AsnNull  ");return;
		case e_enterExtensionAddress :  dprint4("choice:  VS_AsnNull  ");return;
		case e_requestChairTokenOwner :  dprint4("choice:  VS_AsnNull  ");return;
		case e_requestTerminalCertificate :  dprint4("choice: VS_H245ConferenceRequest_RequestTerminalCertificate	 ");return;
		case e_broadcastMyLogicalChannel :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_makeTerminalBroadcaster :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_sendThisSource :  dprint4("choice: VS_H245TerminalLabel ");return;
		case e_requestAllTerminalIDs :  dprint4("choice:  VS_AsnNull  ");return;
		case e_remoteMCRequest :  dprint4("choice: VS_H245RemoteMCRequest ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245CommunicationModeTableEntry_DataType /////////////////////////
 	 VS_H245CommunicationModeTableEntry_DataType::VS_H245CommunicationModeTableEntry_DataType( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CommunicationModeTableEntry_DataType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_videoData : return DecodeChoice( buffer , new VS_H245VideoCapability);
		case e_audioData : return DecodeChoice( buffer , new VS_H245AudioCapability);
		case e_data : return DecodeChoice( buffer , new VS_H245DataApplicationCapability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245CommunicationModeTableEntry_DataType::operator=(const VS_H245CommunicationModeTableEntry_DataType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_videoData : CopyChoice< VS_H245VideoCapability >(src,*this); return;
		case e_audioData : CopyChoice< VS_H245AudioCapability >(src,*this); return;
		case e_data : CopyChoice< VS_H245DataApplicationCapability >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245CommunicationModeTableEntry_DataType::operator VS_H245VideoCapability *( void )
	{	return dynamic_cast< VS_H245VideoCapability * >(choice);    }

 	VS_H245CommunicationModeTableEntry_DataType::operator VS_H245AudioCapability *( void )
	{	return dynamic_cast< VS_H245AudioCapability * >(choice);    }

 	VS_H245CommunicationModeTableEntry_DataType::operator VS_H245DataApplicationCapability *( void )
	{	return dynamic_cast< VS_H245DataApplicationCapability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CommunicationModeTableEntry_DataType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_videoData :  dprint4("choice: VS_H245VideoCapability ");return;
		case e_audioData :  dprint4("choice: VS_H245AudioCapability ");return;
		case e_data :  dprint4("choice: VS_H245DataApplicationCapability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245CommunicationModeTableEntry /////////////////////////
 	 VS_H245CommunicationModeTableEntry :: VS_H245CommunicationModeTableEntry( void )
	:VS_AsnSequence(7 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandard,1);
		ref[1].Set(&sessionID,0);
		ref[2].Set(&associatedSessionID,1);
		ref[3].Set(&terminalLabel,1);
		ref[4].Set(&sessionDescription,0);
		ref[5].Set(&dataType,0);
		ref[6].Set(&mediaChannel,1);
		ref[7].Set(&mediaGuaranteedDelivery,1);
		ref[8].Set(&mediaControlChannel,1);
		ref[9].Set(&mediaControlGuaranteedDelivery,1);
		e_ref[0].Set(&redundancyEncoding,1);
		e_ref[1].Set(&sessionDependency,1);
		e_ref[2].Set(&destination,1);
	}
	void VS_H245CommunicationModeTableEntry::operator=(const VS_H245CommunicationModeTableEntry& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_C(sessionID);
		O_C(associatedSessionID);
		O_C(terminalLabel);
		O_C(sessionDescription);
		O_C(dataType);
		O_C(mediaChannel);
		O_C(mediaGuaranteedDelivery);
		O_C(mediaControlChannel);
		O_C(mediaControlGuaranteedDelivery);
		O_C(redundancyEncoding);
		O_C(sessionDependency);
		O_C(destination);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CommunicationModeResponse /////////////////////////
 	 VS_H245CommunicationModeResponse::VS_H245CommunicationModeResponse( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CommunicationModeResponse::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_communicationModeTable : return DecodeChoice( buffer , new VS_H245CommunicationModeTableEntry);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245CommunicationModeResponse::operator=(const VS_H245CommunicationModeResponse & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_communicationModeTable : CopyChoice< VS_H245CommunicationModeTableEntry >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245CommunicationModeResponse::operator VS_H245CommunicationModeTableEntry *( void )
	{	return dynamic_cast< VS_H245CommunicationModeTableEntry * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CommunicationModeResponse::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_communicationModeTable :  dprint4("choice: VS_H245CommunicationModeTableEntry ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245CommunicationModeRequest /////////////////////////
 	 VS_H245CommunicationModeRequest :: VS_H245CommunicationModeRequest( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr , extension_root , 1 )
	{
	}
	void VS_H245CommunicationModeRequest::operator=(const VS_H245CommunicationModeRequest& src)
	{

		O_CC(filled);
		O_CP(e_ref);  O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CommunicationModeCommand /////////////////////////
 	 VS_H245CommunicationModeCommand :: VS_H245CommunicationModeCommand( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&communicationModeTable,0);
	}
	void VS_H245CommunicationModeCommand::operator=(const VS_H245CommunicationModeCommand& src)
	{

		O_CC(filled);
		O_C(communicationModeTable);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MaintenanceLoopOffCommand /////////////////////////
 	 VS_H245MaintenanceLoopOffCommand :: VS_H245MaintenanceLoopOffCommand( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr , extension_root , 1 )
	{
	}
	void VS_H245MaintenanceLoopOffCommand::operator=(const VS_H245MaintenanceLoopOffCommand& src)
	{

		O_CC(filled);
		O_CP(e_ref);  O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MaintenanceLoopReject_Type /////////////////////////
 	 VS_H245MaintenanceLoopReject_Type::VS_H245MaintenanceLoopReject_Type( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MaintenanceLoopReject_Type::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_systemLoop : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_mediaLoop : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_logicalChannelLoop : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MaintenanceLoopReject_Type::operator=(const VS_H245MaintenanceLoopReject_Type & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_systemLoop : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_mediaLoop : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_logicalChannelLoop : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MaintenanceLoopReject_Type::operator VS_H245LogicalChannelNumber *( void )
	{	return dynamic_cast< VS_H245LogicalChannelNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MaintenanceLoopReject_Type::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_systemLoop :  dprint4("choice:  VS_AsnNull  ");return;
		case e_mediaLoop :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_logicalChannelLoop :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MaintenanceLoopReject_Cause /////////////////////////
 	 VS_H245MaintenanceLoopReject_Cause::VS_H245MaintenanceLoopReject_Cause( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MaintenanceLoopReject_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_canNotPerformLoop : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MaintenanceLoopReject_Cause::operator=(const VS_H245MaintenanceLoopReject_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_canNotPerformLoop : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MaintenanceLoopReject_Cause::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_canNotPerformLoop :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MaintenanceLoopReject /////////////////////////
 	 VS_H245MaintenanceLoopReject :: VS_H245MaintenanceLoopReject( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&type,0);
		ref[1].Set(&cause,0);
	}
	void VS_H245MaintenanceLoopReject::operator=(const VS_H245MaintenanceLoopReject& src)
	{

		O_CC(filled);
		O_C(type);
		O_C(cause);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MaintenanceLoopAck_Type /////////////////////////
 	 VS_H245MaintenanceLoopAck_Type::VS_H245MaintenanceLoopAck_Type( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MaintenanceLoopAck_Type::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_systemLoop : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_mediaLoop : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_logicalChannelLoop : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MaintenanceLoopAck_Type::operator=(const VS_H245MaintenanceLoopAck_Type & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_systemLoop : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_mediaLoop : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_logicalChannelLoop : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MaintenanceLoopAck_Type::operator VS_H245LogicalChannelNumber *( void )
	{	return dynamic_cast< VS_H245LogicalChannelNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MaintenanceLoopAck_Type::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_systemLoop :  dprint4("choice:  VS_AsnNull  ");return;
		case e_mediaLoop :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_logicalChannelLoop :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MaintenanceLoopAck /////////////////////////
 	 VS_H245MaintenanceLoopAck :: VS_H245MaintenanceLoopAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&type,0);
	}
	void VS_H245MaintenanceLoopAck::operator=(const VS_H245MaintenanceLoopAck& src)
	{

		O_CC(filled);
		O_C(type);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MaintenanceLoopRequest_Type /////////////////////////
 	 VS_H245MaintenanceLoopRequest_Type::VS_H245MaintenanceLoopRequest_Type( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MaintenanceLoopRequest_Type::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_systemLoop : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_mediaLoop : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		case e_logicalChannelLoop : return DecodeChoice( buffer , new VS_H245LogicalChannelNumber);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MaintenanceLoopRequest_Type::operator=(const VS_H245MaintenanceLoopRequest_Type & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_systemLoop : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_mediaLoop : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		case e_logicalChannelLoop : CopyChoice< VS_H245LogicalChannelNumber >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MaintenanceLoopRequest_Type::operator VS_H245LogicalChannelNumber *( void )
	{	return dynamic_cast< VS_H245LogicalChannelNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MaintenanceLoopRequest_Type::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_systemLoop :  dprint4("choice:  VS_AsnNull  ");return;
		case e_mediaLoop :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		case e_logicalChannelLoop :  dprint4("choice: VS_H245LogicalChannelNumber ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MaintenanceLoopRequest /////////////////////////
 	 VS_H245MaintenanceLoopRequest :: VS_H245MaintenanceLoopRequest( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&type,0);
	}
	void VS_H245MaintenanceLoopRequest::operator=(const VS_H245MaintenanceLoopRequest& src)
	{

		O_CC(filled);
		O_C(type);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RoundTripDelayResponse /////////////////////////
 	 VS_H245RoundTripDelayResponse :: VS_H245RoundTripDelayResponse( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
	}
	void VS_H245RoundTripDelayResponse::operator=(const VS_H245RoundTripDelayResponse& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RoundTripDelayRequest /////////////////////////
 	 VS_H245RoundTripDelayRequest :: VS_H245RoundTripDelayRequest( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
	}
	void VS_H245RoundTripDelayRequest::operator=(const VS_H245RoundTripDelayRequest& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245EncryptionMode /////////////////////////
 	 VS_H245EncryptionMode::VS_H245EncryptionMode( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245EncryptionMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_h233Encryption : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245EncryptionMode::operator=(const VS_H245EncryptionMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_h233Encryption : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245EncryptionMode::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245EncryptionMode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_h233Encryption :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DataMode_Application_Nlpid /////////////////////////
 	 VS_H245DataMode_Application_Nlpid :: VS_H245DataMode_Application_Nlpid( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&nlpidProtocol,0);
		ref[1].Set(&nlpidData,0);
	}
	void VS_H245DataMode_Application_Nlpid::operator=(const VS_H245DataMode_Application_Nlpid& src)
	{

		O_CC(filled);
		O_C(nlpidProtocol);
		O_C(nlpidData);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DataMode_Application_T38fax /////////////////////////
 	 VS_H245DataMode_Application_T38fax :: VS_H245DataMode_Application_T38fax( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&t38FaxProtocol,0);
		ref[1].Set(&t38FaxProfile,0);
	}
	void VS_H245DataMode_Application_T38fax::operator=(const VS_H245DataMode_Application_T38fax& src)
	{

		O_CC(filled);
		O_C(t38FaxProtocol);
		O_C(t38FaxProfile);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DataMode_Application /////////////////////////
 	 VS_H245DataMode_Application::VS_H245DataMode_Application( void )
	:VS_AsnChoice(10 , 14 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DataMode_Application::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_t120 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_dsm_cc : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_userData : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t84 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t434 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_h224 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_nlpid : return DecodeChoice( buffer , new VS_H245DataMode_Application_Nlpid	 );
		case e_dsvdControl : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_h222DataPartitioning : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t30fax : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t140 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t38fax : return DecodeChoice( buffer , new VS_H245DataMode_Application_T38fax	 );
		case e_genericDataMode : return DecodeChoice( buffer , new VS_H245GenericCapability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DataMode_Application::operator=(const VS_H245DataMode_Application & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_t120 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_dsm_cc : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_userData : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t84 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t434 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_h224 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_nlpid : CopyChoice<VS_H245DataMode_Application_Nlpid	  >(src,*this);  return;
		case e_dsvdControl : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_h222DataPartitioning : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t30fax : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t140 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t38fax : CopyChoice<VS_H245DataMode_Application_T38fax	  >(src,*this);  return;
		case e_genericDataMode : CopyChoice< VS_H245GenericCapability >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245DataMode_Application::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245DataMode_Application::operator VS_H245DataProtocolCapability *( void )
	{	return dynamic_cast< VS_H245DataProtocolCapability * >(choice);    }

 	VS_H245DataMode_Application::operator VS_H245GenericCapability *( void )
	{	return dynamic_cast< VS_H245GenericCapability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DataMode_Application::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_t120 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_dsm_cc :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_userData :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t84 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t434 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_h224 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_nlpid :  dprint4("choice: VS_H245DataMode_Application_Nlpid	 ");return;
		case e_dsvdControl :  dprint4("choice:  VS_AsnNull  ");return;
		case e_h222DataPartitioning :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t30fax :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t140 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t38fax :  dprint4("choice: VS_H245DataMode_Application_T38fax	 ");return;
		case e_genericDataMode :  dprint4("choice: VS_H245GenericCapability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DataMode /////////////////////////
 	 VS_H245DataMode :: VS_H245DataMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&application,0);
		ref[1].Set(&bitRate,0);
	}
	void VS_H245DataMode::operator=(const VS_H245DataMode& src)
	{

		O_CC(filled);
		O_C(application);
		O_C(bitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VBDMode /////////////////////////
 	 VS_H245VBDMode :: VS_H245VBDMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&type,0);
	}
	void VS_H245VBDMode::operator=(const VS_H245VBDMode& src)
	{

		O_CC(filled);
		O_C(type);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245G7231AnnexCMode_G723AnnexCAudioMode /////////////////////////
 	 VS_H245G7231AnnexCMode_G723AnnexCAudioMode :: VS_H245G7231AnnexCMode_G723AnnexCAudioMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&highRateMode0,0);
		ref[1].Set(&highRateMode1,0);
		ref[2].Set(&lowRateMode0,0);
		ref[3].Set(&lowRateMode1,0);
		ref[4].Set(&sidMode0,0);
		ref[5].Set(&sidMode1,0);
	}
	void VS_H245G7231AnnexCMode_G723AnnexCAudioMode::operator=(const VS_H245G7231AnnexCMode_G723AnnexCAudioMode& src)
	{

		O_CC(filled);
		O_C(highRateMode0);
		O_C(highRateMode1);
		O_C(lowRateMode0);
		O_C(lowRateMode1);
		O_C(sidMode0);
		O_C(sidMode1);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245G7231AnnexCMode /////////////////////////
	VS_H245G7231AnnexCMode::VS_H245G7231AnnexCMode(void)
		:VS_AsnSequence(0, ref, basic_root, nullptr, extension_root, 1)
	{
		ref[0].Set(&maxAl_sduAudioFrames,0);
		ref[1].Set(&silenceSuppression,0);
		ref[2].Set(&g723AnnexCAudioMode,0);
	}
	void VS_H245G7231AnnexCMode::operator=(const VS_H245G7231AnnexCMode& src)
	{

		O_CC(filled);
		O_C(maxAl_sduAudioFrames);
		O_C(silenceSuppression);
		O_C(g723AnnexCAudioMode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245IS13818AudioMode_AudioLayer /////////////////////////
 	 VS_H245IS13818AudioMode_AudioLayer::VS_H245IS13818AudioMode_AudioLayer( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245IS13818AudioMode_AudioLayer::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_audioLayer1 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioLayer2 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioLayer3 : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245IS13818AudioMode_AudioLayer::operator=(const VS_H245IS13818AudioMode_AudioLayer & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_audioLayer1 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioLayer2 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioLayer3 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245IS13818AudioMode_AudioLayer::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_audioLayer1 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioLayer2 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioLayer3 :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IS13818AudioMode_AudioSampling /////////////////////////
 	 VS_H245IS13818AudioMode_AudioSampling::VS_H245IS13818AudioMode_AudioSampling( void )
	:VS_AsnChoice(6 , 6 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245IS13818AudioMode_AudioSampling::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_audioSampling16k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioSampling22k05 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioSampling24k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioSampling32k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioSampling44k1 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioSampling48k : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245IS13818AudioMode_AudioSampling::operator=(const VS_H245IS13818AudioMode_AudioSampling & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_audioSampling16k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioSampling22k05 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioSampling24k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioSampling32k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioSampling44k1 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioSampling48k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245IS13818AudioMode_AudioSampling::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_audioSampling16k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioSampling22k05 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioSampling24k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioSampling32k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioSampling44k1 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioSampling48k :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IS13818AudioMode_MultichannelType /////////////////////////
 	 VS_H245IS13818AudioMode_MultichannelType::VS_H245IS13818AudioMode_MultichannelType( void )
	:VS_AsnChoice(10 , 10 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245IS13818AudioMode_MultichannelType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_singleChannel : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_twoChannelStereo : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_twoChannelDual : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_threeChannels2_1 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_threeChannels3_0 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_fourChannels2_0_2_0 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_fourChannels2_2 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_fourChannels3_1 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_fiveChannels3_0_2_0 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_fiveChannels3_2 : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245IS13818AudioMode_MultichannelType::operator=(const VS_H245IS13818AudioMode_MultichannelType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_singleChannel : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_twoChannelStereo : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_twoChannelDual : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_threeChannels2_1 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_threeChannels3_0 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_fourChannels2_0_2_0 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_fourChannels2_2 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_fourChannels3_1 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_fiveChannels3_0_2_0 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_fiveChannels3_2 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245IS13818AudioMode_MultichannelType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_singleChannel :  dprint4("choice:  VS_AsnNull  ");return;
		case e_twoChannelStereo :  dprint4("choice:  VS_AsnNull  ");return;
		case e_twoChannelDual :  dprint4("choice:  VS_AsnNull  ");return;
		case e_threeChannels2_1 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_threeChannels3_0 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_fourChannels2_0_2_0 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_fourChannels2_2 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_fourChannels3_1 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_fiveChannels3_0_2_0 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_fiveChannels3_2 :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IS13818AudioMode /////////////////////////
 	 VS_H245IS13818AudioMode :: VS_H245IS13818AudioMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioLayer,0);
		ref[1].Set(&audioSampling,0);
		ref[2].Set(&multichannelType,0);
		ref[3].Set(&lowFrequencyEnhancement,0);
		ref[4].Set(&multilingual,0);
		ref[5].Set(&bitRate,0);
	}
	void VS_H245IS13818AudioMode::operator=(const VS_H245IS13818AudioMode& src)
	{

		O_CC(filled);
		O_C(audioLayer);
		O_C(audioSampling);
		O_C(multichannelType);
		O_C(lowFrequencyEnhancement);
		O_C(multilingual);
		O_C(bitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245IS11172AudioMode_AudioLayer /////////////////////////
 	 VS_H245IS11172AudioMode_AudioLayer::VS_H245IS11172AudioMode_AudioLayer( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245IS11172AudioMode_AudioLayer::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_audioLayer1 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioLayer2 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioLayer3 : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245IS11172AudioMode_AudioLayer::operator=(const VS_H245IS11172AudioMode_AudioLayer & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_audioLayer1 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioLayer2 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioLayer3 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245IS11172AudioMode_AudioLayer::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_audioLayer1 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioLayer2 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioLayer3 :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IS11172AudioMode_AudioSampling /////////////////////////
 	 VS_H245IS11172AudioMode_AudioSampling::VS_H245IS11172AudioMode_AudioSampling( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245IS11172AudioMode_AudioSampling::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_audioSampling32k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioSampling44k1 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_audioSampling48k : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245IS11172AudioMode_AudioSampling::operator=(const VS_H245IS11172AudioMode_AudioSampling & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_audioSampling32k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioSampling44k1 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_audioSampling48k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245IS11172AudioMode_AudioSampling::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_audioSampling32k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioSampling44k1 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_audioSampling48k :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IS11172AudioMode_MultichannelType /////////////////////////
 	 VS_H245IS11172AudioMode_MultichannelType::VS_H245IS11172AudioMode_MultichannelType( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245IS11172AudioMode_MultichannelType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_singleChannel : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_twoChannelStereo : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_twoChannelDual : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245IS11172AudioMode_MultichannelType::operator=(const VS_H245IS11172AudioMode_MultichannelType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_singleChannel : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_twoChannelStereo : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_twoChannelDual : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245IS11172AudioMode_MultichannelType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_singleChannel :  dprint4("choice:  VS_AsnNull  ");return;
		case e_twoChannelStereo :  dprint4("choice:  VS_AsnNull  ");return;
		case e_twoChannelDual :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IS11172AudioMode /////////////////////////
 	 VS_H245IS11172AudioMode :: VS_H245IS11172AudioMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioLayer,0);
		ref[1].Set(&audioSampling,0);
		ref[2].Set(&multichannelType,0);
		ref[3].Set(&bitRate,0);
	}
	void VS_H245IS11172AudioMode::operator=(const VS_H245IS11172AudioMode& src)
	{

		O_CC(filled);
		O_C(audioLayer);
		O_C(audioSampling);
		O_C(multichannelType);
		O_C(bitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245AudioMode_G7231 /////////////////////////
 	 VS_H245AudioMode_G7231::VS_H245AudioMode_G7231( void )
	:VS_AsnChoice(4 , 4 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245AudioMode_G7231::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noSilenceSuppressionLowRate : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_noSilenceSuppressionHighRate : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_silenceSuppressionLowRate : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_silenceSuppressionHighRate : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245AudioMode_G7231::operator=(const VS_H245AudioMode_G7231 & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noSilenceSuppressionLowRate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_noSilenceSuppressionHighRate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_silenceSuppressionLowRate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_silenceSuppressionHighRate : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245AudioMode_G7231::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_noSilenceSuppressionLowRate :  dprint4("choice:  VS_AsnNull  ");return;
		case e_noSilenceSuppressionHighRate :  dprint4("choice:  VS_AsnNull  ");return;
		case e_silenceSuppressionLowRate :  dprint4("choice:  VS_AsnNull  ");return;
		case e_silenceSuppressionHighRate :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245AudioMode /////////////////////////
 	 VS_H245AudioMode::VS_H245AudioMode( void )
	:VS_AsnChoice(14 , 23 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245AudioMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_g711Alaw64k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g711Alaw56k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g711Ulaw64k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g711Ulaw56k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g722_64k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g722_56k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g722_48k : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g728 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g729 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g729AnnexA : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_g7231 : return DecodeChoice( buffer , new VS_H245AudioMode_G7231	 );
		case e_is11172AudioMode : return DecodeChoice( buffer , new VS_H245IS11172AudioMode);
		case e_is13818AudioMode : return DecodeChoice( buffer , new VS_H245IS13818AudioMode);
		case e_g729wAnnexB : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g729AnnexAwAnnexB : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g7231AnnexCMode : return DecodeChoice( buffer , new VS_H245G7231AnnexCMode);
		case e_gsmFullRate : return DecodeChoice( buffer , new VS_H245GSMAudioCapability);
		case e_gsmHalfRate : return DecodeChoice( buffer , new VS_H245GSMAudioCapability);
		case e_gsmEnhancedFullRate : return DecodeChoice( buffer , new VS_H245GSMAudioCapability);
		case e_genericAudioMode : return DecodeChoice( buffer , new VS_H245GenericCapability);
		case e_g729Extensions : return DecodeChoice( buffer , new VS_H245G729Extensions);
		case e_vbd : return DecodeChoice( buffer , new VS_H245VBDMode);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245AudioMode::operator=(const VS_H245AudioMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_g711Alaw64k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g711Alaw56k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g711Ulaw64k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g711Ulaw56k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g722_64k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g722_56k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g722_48k : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g728 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g729 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g729AnnexA : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_g7231 : CopyChoice<VS_H245AudioMode_G7231	  >(src,*this);  return;
		case e_is11172AudioMode : CopyChoice< VS_H245IS11172AudioMode >(src,*this); return;
		case e_is13818AudioMode : CopyChoice< VS_H245IS13818AudioMode >(src,*this); return;
		case e_g729wAnnexB : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g729AnnexAwAnnexB : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g7231AnnexCMode : CopyChoice< VS_H245G7231AnnexCMode >(src,*this); return;
		case e_gsmFullRate : CopyChoice< VS_H245GSMAudioCapability >(src,*this); return;
		case e_gsmHalfRate : CopyChoice< VS_H245GSMAudioCapability >(src,*this); return;
		case e_gsmEnhancedFullRate : CopyChoice< VS_H245GSMAudioCapability >(src,*this); return;
		case e_genericAudioMode : CopyChoice< VS_H245GenericCapability >(src,*this); return;
		case e_g729Extensions : CopyChoice< VS_H245G729Extensions >(src,*this); return;
		case e_vbd : CopyChoice< VS_H245VBDMode >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245AudioMode::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245AudioMode::operator VS_H245IS11172AudioMode *( void )
	{	return dynamic_cast< VS_H245IS11172AudioMode * >(choice);    }

 	VS_H245AudioMode::operator VS_H245IS13818AudioMode *( void )
	{	return dynamic_cast< VS_H245IS13818AudioMode * >(choice);    }

 	VS_H245AudioMode::operator VS_H245G7231AnnexCMode *( void )
	{	return dynamic_cast< VS_H245G7231AnnexCMode * >(choice);    }

 	VS_H245AudioMode::operator VS_H245GSMAudioCapability *( void )
	{	return dynamic_cast< VS_H245GSMAudioCapability * >(choice);    }

 	VS_H245AudioMode::operator VS_H245GenericCapability *( void )
	{	return dynamic_cast< VS_H245GenericCapability * >(choice);    }

 	VS_H245AudioMode::operator VS_H245G729Extensions *( void )
	{	return dynamic_cast< VS_H245G729Extensions * >(choice);    }

 	VS_H245AudioMode::operator VS_H245VBDMode *( void )
	{	return dynamic_cast< VS_H245VBDMode * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245AudioMode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_g711Alaw64k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g711Alaw56k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g711Ulaw64k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g711Ulaw56k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g722_64k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g722_56k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g722_48k :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g728 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g729 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g729AnnexA :  dprint4("choice:  VS_AsnNull  ");return;
		case e_g7231 :  dprint4("choice: VS_H245AudioMode_G7231	 ");return;
		case e_is11172AudioMode :  dprint4("choice: VS_H245IS11172AudioMode ");return;
		case e_is13818AudioMode :  dprint4("choice: VS_H245IS13818AudioMode ");return;
		case e_g729wAnnexB :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g729AnnexAwAnnexB :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g7231AnnexCMode :  dprint4("choice: VS_H245G7231AnnexCMode ");return;
		case e_gsmFullRate :  dprint4("choice: VS_H245GSMAudioCapability ");return;
		case e_gsmHalfRate :  dprint4("choice: VS_H245GSMAudioCapability ");return;
		case e_gsmEnhancedFullRate :  dprint4("choice: VS_H245GSMAudioCapability ");return;
		case e_genericAudioMode :  dprint4("choice: VS_H245GenericCapability ");return;
		case e_g729Extensions :  dprint4("choice: VS_H245G729Extensions ");return;
		case e_vbd :  dprint4("choice: VS_H245VBDMode ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IS11172VideoMode /////////////////////////
 	 VS_H245IS11172VideoMode :: VS_H245IS11172VideoMode( void )
	:VS_AsnSequence(6 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&constrainedBitstream,0);
		ref[1].Set(&videoBitRate,1);
		ref[2].Set(&vbvBufferSize,1);
		ref[3].Set(&samplesPerLine,1);
		ref[4].Set(&linesPerFrame,1);
		ref[5].Set(&pictureRate,1);
		ref[6].Set(&luminanceSampleRate,1);
	}
	void VS_H245IS11172VideoMode::operator=(const VS_H245IS11172VideoMode& src)
	{

		O_CC(filled);
		O_C(constrainedBitstream);
		O_C(videoBitRate);
		O_C(vbvBufferSize);
		O_C(samplesPerLine);
		O_C(linesPerFrame);
		O_C(pictureRate);
		O_C(luminanceSampleRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H263VideoMode_Resolution /////////////////////////
 	 VS_H245H263VideoMode_Resolution::VS_H245H263VideoMode_Resolution( void )
	:VS_AsnChoice(5 , 6 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H263VideoMode_Resolution::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_sqcif : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_qcif : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cif : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cif4 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cif16 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_custom : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H263VideoMode_Resolution::operator=(const VS_H245H263VideoMode_Resolution & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_sqcif : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_qcif : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cif : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cif4 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cif16 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_custom : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H263VideoMode_Resolution::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_sqcif :  dprint4("choice:  VS_AsnNull  ");return;
		case e_qcif :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cif :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cif4 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cif16 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_custom :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H263VideoMode /////////////////////////
 	 VS_H245H263VideoMode :: VS_H245H263VideoMode( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&resolution,0);
		ref[1].Set(&bitRate,0);
		ref[2].Set(&unrestrictedVector,0);
		ref[3].Set(&arithmeticCoding,0);
		ref[4].Set(&advancedPrediction,0);
		ref[5].Set(&pbFrames,0);
		e_ref[0].Set(&errorCompensation,0);
		e_ref[1].Set(&enhancementLayerInfo,1);
		e_ref[2].Set(&h263Options,1);
	}
	void VS_H245H263VideoMode::operator=(const VS_H245H263VideoMode& src)
	{

		O_CC(filled);
		O_C(resolution);
		O_C(bitRate);
		O_C(unrestrictedVector);
		O_C(arithmeticCoding);
		O_C(advancedPrediction);
		O_C(pbFrames);
		O_C(errorCompensation);
		O_C(enhancementLayerInfo);
		O_C(h263Options);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H262VideoMode_ProfileAndLevel /////////////////////////
 	 VS_H245H262VideoMode_ProfileAndLevel::VS_H245H262VideoMode_ProfileAndLevel( void )
	:VS_AsnChoice(11 , 11 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H262VideoMode_ProfileAndLevel::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_profileAndLevel_SPatML : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_MPatLL : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_MPatML : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_MPatH_14 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_MPatHL : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_SNRatLL : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_SNRatML : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_SpatialatH_14 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_HPatML : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_HPatH_14 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_profileAndLevel_HPatHL : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H262VideoMode_ProfileAndLevel::operator=(const VS_H245H262VideoMode_ProfileAndLevel & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_profileAndLevel_SPatML : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_MPatLL : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_MPatML : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_MPatH_14 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_MPatHL : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_SNRatLL : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_SNRatML : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_SpatialatH_14 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_HPatML : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_HPatH_14 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_profileAndLevel_HPatHL : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H262VideoMode_ProfileAndLevel::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_profileAndLevel_SPatML :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_MPatLL :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_MPatML :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_MPatH_14 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_MPatHL :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_SNRatLL :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_SNRatML :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_SpatialatH_14 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_HPatML :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_HPatH_14 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_profileAndLevel_HPatHL :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H262VideoMode /////////////////////////
 	 VS_H245H262VideoMode :: VS_H245H262VideoMode( void )
	:VS_AsnSequence(6 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&profileAndLevel,0);
		ref[1].Set(&videoBitRate,1);
		ref[2].Set(&vbvBufferSize,1);
		ref[3].Set(&samplesPerLine,1);
		ref[4].Set(&linesPerFrame,1);
		ref[5].Set(&framesPerSecond,1);
		ref[6].Set(&luminanceSampleRate,1);
	}
	void VS_H245H262VideoMode::operator=(const VS_H245H262VideoMode& src)
	{

		O_CC(filled);
		O_C(profileAndLevel);
		O_C(videoBitRate);
		O_C(vbvBufferSize);
		O_C(samplesPerLine);
		O_C(linesPerFrame);
		O_C(framesPerSecond);
		O_C(luminanceSampleRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H261VideoMode_Resolution /////////////////////////
 	 VS_H245H261VideoMode_Resolution::VS_H245H261VideoMode_Resolution( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H261VideoMode_Resolution::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_qcif : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_cif : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245H261VideoMode_Resolution::operator=(const VS_H245H261VideoMode_Resolution & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_qcif : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_cif : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H261VideoMode_Resolution::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_qcif :  dprint4("choice:  VS_AsnNull  ");return;
		case e_cif :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H261VideoMode /////////////////////////
 	 VS_H245H261VideoMode :: VS_H245H261VideoMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&resolution,0);
		ref[1].Set(&bitRate,0);
		ref[2].Set(&stillImageTransmission,0);
	}
	void VS_H245H261VideoMode::operator=(const VS_H245H261VideoMode& src)
	{

		O_CC(filled);
		O_C(resolution);
		O_C(bitRate);
		O_C(stillImageTransmission);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VideoMode /////////////////////////
 	 VS_H245VideoMode::VS_H245VideoMode( void )
	:VS_AsnChoice(5 , 6 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245VideoMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_h261VideoMode : return DecodeChoice( buffer , new VS_H245H261VideoMode);
		case e_h262VideoMode : return DecodeChoice( buffer , new VS_H245H262VideoMode);
		case e_h263VideoMode : return DecodeChoice( buffer , new VS_H245H263VideoMode);
		case e_is11172VideoMode : return DecodeChoice( buffer , new VS_H245IS11172VideoMode);
		case e_genericVideoMode : return DecodeChoice( buffer , new VS_H245GenericCapability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245VideoMode::operator=(const VS_H245VideoMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_h261VideoMode : CopyChoice< VS_H245H261VideoMode >(src,*this); return;
		case e_h262VideoMode : CopyChoice< VS_H245H262VideoMode >(src,*this); return;
		case e_h263VideoMode : CopyChoice< VS_H245H263VideoMode >(src,*this); return;
		case e_is11172VideoMode : CopyChoice< VS_H245IS11172VideoMode >(src,*this); return;
		case e_genericVideoMode : CopyChoice< VS_H245GenericCapability >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245VideoMode::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245VideoMode::operator VS_H245H261VideoMode *( void )
	{	return dynamic_cast< VS_H245H261VideoMode * >(choice);    }

 	VS_H245VideoMode::operator VS_H245H262VideoMode *( void )
	{	return dynamic_cast< VS_H245H262VideoMode * >(choice);    }

 	VS_H245VideoMode::operator VS_H245H263VideoMode *( void )
	{	return dynamic_cast< VS_H245H263VideoMode * >(choice);    }

 	VS_H245VideoMode::operator VS_H245IS11172VideoMode *( void )
	{	return dynamic_cast< VS_H245IS11172VideoMode * >(choice);    }

 	VS_H245VideoMode::operator VS_H245GenericCapability *( void )
	{	return dynamic_cast< VS_H245GenericCapability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245VideoMode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_h261VideoMode :  dprint4("choice: VS_H245H261VideoMode ");return;
		case e_h262VideoMode :  dprint4("choice: VS_H245H262VideoMode ");return;
		case e_h263VideoMode :  dprint4("choice: VS_H245H263VideoMode ");return;
		case e_is11172VideoMode :  dprint4("choice: VS_H245IS11172VideoMode ");return;
		case e_genericVideoMode :  dprint4("choice: VS_H245GenericCapability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RedundancyEncodingMode_SecondaryEncoding /////////////////////////
 	 VS_H245RedundancyEncodingMode_SecondaryEncoding::VS_H245RedundancyEncodingMode_SecondaryEncoding( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RedundancyEncodingMode_SecondaryEncoding::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_audioData : return DecodeChoice( buffer , new VS_H245AudioMode);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RedundancyEncodingMode_SecondaryEncoding::operator=(const VS_H245RedundancyEncodingMode_SecondaryEncoding & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_audioData : CopyChoice< VS_H245AudioMode >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245RedundancyEncodingMode_SecondaryEncoding::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245RedundancyEncodingMode_SecondaryEncoding::operator VS_H245AudioMode *( void )
	{	return dynamic_cast< VS_H245AudioMode * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RedundancyEncodingMode_SecondaryEncoding::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_audioData :  dprint4("choice: VS_H245AudioMode ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RedundancyEncodingMode /////////////////////////
 	 VS_H245RedundancyEncodingMode :: VS_H245RedundancyEncodingMode( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&redundancyEncodingMethod,0);
		ref[1].Set(&secondaryEncoding,1);
	}
	void VS_H245RedundancyEncodingMode::operator=(const VS_H245RedundancyEncodingMode& src)
	{

		O_CC(filled);
		O_C(redundancyEncodingMethod);
		O_C(secondaryEncoding);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H2250ModeParameters /////////////////////////
 	 VS_H245H2250ModeParameters :: VS_H245H2250ModeParameters( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&redundancyEncodingMode,1);
	}
	void VS_H245H2250ModeParameters::operator=(const VS_H245H2250ModeParameters& src)
	{

		O_CC(filled);
		O_C(redundancyEncodingMode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245V76ModeParameters /////////////////////////
 	 VS_H245V76ModeParameters::VS_H245V76ModeParameters( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245V76ModeParameters::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_suspendResumewAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_suspendResumewoAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245V76ModeParameters::operator=(const VS_H245V76ModeParameters & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_suspendResumewAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_suspendResumewoAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245V76ModeParameters::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_suspendResumewAddress :  dprint4("choice:  VS_AsnNull  ");return;
		case e_suspendResumewoAddress :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223ModeParameters_AdaptationLayerType_Al3 /////////////////////////
 	 VS_H245H223ModeParameters_AdaptationLayerType_Al3 :: VS_H245H223ModeParameters_AdaptationLayerType_Al3( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&controlFieldOctets,0);
		ref[1].Set(&sendBufferSize,0);
	}
	void VS_H245H223ModeParameters_AdaptationLayerType_Al3::operator=(const VS_H245H223ModeParameters_AdaptationLayerType_Al3& src)
	{

		O_CC(filled);
		O_C(controlFieldOctets);
		O_C(sendBufferSize);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223ModeParameters_AdaptationLayerType /////////////////////////
 	 VS_H245H223ModeParameters_AdaptationLayerType::VS_H245H223ModeParameters_AdaptationLayerType( void )
	:VS_AsnChoice(6 , 9 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223ModeParameters_AdaptationLayerType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_al1Framed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_al1NotFramed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_al2WithoutSequenceNumbers : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_al2WithSequenceNumbers : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_al3 : return DecodeChoice( buffer , new VS_H245H223ModeParameters_AdaptationLayerType_Al3	 );
		case e_al1M : return DecodeChoice( buffer , new VS_H245H223AL1MParameters);
		case e_al2M : return DecodeChoice( buffer , new VS_H245H223AL2MParameters);
		case e_al3M : return DecodeChoice( buffer , new VS_H245H223AL3MParameters);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223ModeParameters_AdaptationLayerType::operator=(const VS_H245H223ModeParameters_AdaptationLayerType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_al1Framed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_al1NotFramed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_al2WithoutSequenceNumbers : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_al2WithSequenceNumbers : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_al3 : CopyChoice<VS_H245H223ModeParameters_AdaptationLayerType_Al3	  >(src,*this);  return;
		case e_al1M : CopyChoice< VS_H245H223AL1MParameters >(src,*this); return;
		case e_al2M : CopyChoice< VS_H245H223AL2MParameters >(src,*this); return;
		case e_al3M : CopyChoice< VS_H245H223AL3MParameters >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245H223ModeParameters_AdaptationLayerType::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245H223ModeParameters_AdaptationLayerType::operator VS_H245H223AL1MParameters *( void )
	{	return dynamic_cast< VS_H245H223AL1MParameters * >(choice);    }

 	VS_H245H223ModeParameters_AdaptationLayerType::operator VS_H245H223AL2MParameters *( void )
	{	return dynamic_cast< VS_H245H223AL2MParameters * >(choice);    }

 	VS_H245H223ModeParameters_AdaptationLayerType::operator VS_H245H223AL3MParameters *( void )
	{	return dynamic_cast< VS_H245H223AL3MParameters * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223ModeParameters_AdaptationLayerType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_al1Framed :  dprint4("choice:  VS_AsnNull  ");return;
		case e_al1NotFramed :  dprint4("choice:  VS_AsnNull  ");return;
		case e_al2WithoutSequenceNumbers :  dprint4("choice:  VS_AsnNull  ");return;
		case e_al2WithSequenceNumbers :  dprint4("choice:  VS_AsnNull  ");return;
		case e_al3 :  dprint4("choice: VS_H245H223ModeParameters_AdaptationLayerType_Al3	 ");return;
		case e_al1M :  dprint4("choice: VS_H245H223AL1MParameters ");return;
		case e_al2M :  dprint4("choice: VS_H245H223AL2MParameters ");return;
		case e_al3M :  dprint4("choice: VS_H245H223AL3MParameters ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223ModeParameters /////////////////////////
 	 VS_H245H223ModeParameters :: VS_H245H223ModeParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&adaptationLayerType,0);
		ref[1].Set(&segmentableFlag,0);
	}
	void VS_H245H223ModeParameters::operator=(const VS_H245H223ModeParameters& src)
	{

		O_CC(filled);
		O_C(adaptationLayerType);
		O_C(segmentableFlag);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245FECMode_Rfc2733Format /////////////////////////
 	 VS_H245FECMode_Rfc2733Format::VS_H245FECMode_Rfc2733Format( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FECMode_Rfc2733Format::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_rfc2733rfc2198 : return DecodeChoice( buffer , new VS_H245MaxRedundancy);
		case e_rfc2733sameport : return DecodeChoice( buffer , new VS_H245MaxRedundancy);
		case e_rfc2733diffport : return DecodeChoice( buffer , new VS_H245MaxRedundancy);
		default: return false;
		}

	}

	void VS_H245FECMode_Rfc2733Format::operator=(const VS_H245FECMode_Rfc2733Format & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_rfc2733rfc2198 : CopyChoice< VS_H245MaxRedundancy >(src,*this); return;
		case e_rfc2733sameport : CopyChoice< VS_H245MaxRedundancy >(src,*this); return;
		case e_rfc2733diffport : CopyChoice< VS_H245MaxRedundancy >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245FECMode_Rfc2733Format::operator VS_H245MaxRedundancy *( void )
	{	return dynamic_cast< VS_H245MaxRedundancy * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FECMode_Rfc2733Format::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_rfc2733rfc2198 :  dprint4("choice: VS_H245MaxRedundancy ");return;
		case e_rfc2733sameport :  dprint4("choice: VS_H245MaxRedundancy ");return;
		case e_rfc2733diffport :  dprint4("choice: VS_H245MaxRedundancy ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FECMode /////////////////////////
 	 VS_H245FECMode :: VS_H245FECMode( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&protectedElement,0);
		ref[1].Set(&fecScheme,1);
		ref[2].Set(&rfc2733Format,1);
	}
	void VS_H245FECMode::operator=(const VS_H245FECMode& src)
	{

		O_CC(filled);
		O_C(protectedElement);
		O_C(fecScheme);
		O_C(rfc2733Format);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort /////////////////////////
 	 VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort :: VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&protectedSessionID,0);
		ref[1].Set(&protectedPayloadType,1);
	}
	void VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort::operator=(const VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort& src)
	{

		O_CC(filled);
		O_C(protectedSessionID);
		O_C(protectedPayloadType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort /////////////////////////
 	 VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort :: VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&protectedType,0);
	}
	void VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort::operator=(const VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort& src)
	{

		O_CC(filled);
		O_C(protectedType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream /////////////////////////
 	 VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream::VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_differentPort : return DecodeChoice( buffer , new VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort	 );
		case e_samePort : return DecodeChoice( buffer , new VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream::operator=(const VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_differentPort : CopyChoice<VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort	  >(src,*this);  return;
		case e_samePort : CopyChoice<VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_differentPort :  dprint4("choice: VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_DifferentPort	 ");return;
		case e_samePort :  dprint4("choice: VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream_SamePort	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode_Mode /////////////////////////
 	 VS_H245DepFECMode_Rfc2733Mode_Mode::VS_H245DepFECMode_Rfc2733Mode_Mode( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DepFECMode_Rfc2733Mode_Mode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_redundancyEncoding : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_separateStream : return DecodeChoice( buffer , new VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DepFECMode_Rfc2733Mode_Mode::operator=(const VS_H245DepFECMode_Rfc2733Mode_Mode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_redundancyEncoding : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_separateStream : CopyChoice<VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DepFECMode_Rfc2733Mode_Mode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_redundancyEncoding :  dprint4("choice:  VS_AsnNull  ");return;
		case e_separateStream :  dprint4("choice: VS_H245DepFECMode_Rfc2733Mode_Mode_SeparateStream	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DepFECMode_Rfc2733Mode /////////////////////////
 	 VS_H245DepFECMode_Rfc2733Mode :: VS_H245DepFECMode_Rfc2733Mode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&mode,0);
	}
	void VS_H245DepFECMode_Rfc2733Mode::operator=(const VS_H245DepFECMode_Rfc2733Mode& src)
	{

		O_CC(filled);
		O_C(mode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECMode /////////////////////////
 	 VS_H245DepFECMode::VS_H245DepFECMode( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DepFECMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_rfc2733Mode : return DecodeChoice( buffer , new VS_H245DepFECMode_Rfc2733Mode	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DepFECMode::operator=(const VS_H245DepFECMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_rfc2733Mode : CopyChoice<VS_H245DepFECMode_Rfc2733Mode	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DepFECMode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_rfc2733Mode :  dprint4("choice: VS_H245DepFECMode_Rfc2733Mode	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultiplePayloadStreamElementMode /////////////////////////
 	 VS_H245MultiplePayloadStreamElementMode :: VS_H245MultiplePayloadStreamElementMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&type,0);
	}
	void VS_H245MultiplePayloadStreamElementMode::operator=(const VS_H245MultiplePayloadStreamElementMode& src)
	{

		O_CC(filled);
		O_C(type);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplePayloadStreamMode /////////////////////////
 	 VS_H245MultiplePayloadStreamMode :: VS_H245MultiplePayloadStreamMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&elements,0);
	}
	void VS_H245MultiplePayloadStreamMode::operator=(const VS_H245MultiplePayloadStreamMode& src)
	{

		O_CC(filled);
		O_C(elements);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RedundancyEncodingDTModeElement_Type /////////////////////////
 	 VS_H245RedundancyEncodingDTModeElement_Type::VS_H245RedundancyEncodingDTModeElement_Type( void )
	:VS_AsnChoice(6 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RedundancyEncodingDTModeElement_Type::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_videoMode : return DecodeChoice( buffer , new VS_H245VideoMode);
		case e_audioMode : return DecodeChoice( buffer , new VS_H245AudioMode);
		case e_dataMode : return DecodeChoice( buffer , new VS_H245DataMode);
		case e_encryptionMode : return DecodeChoice( buffer , new VS_H245EncryptionMode);
		case e_h235Mode : return DecodeChoice( buffer , new VS_H245H235Mode);
		case e_fecMode : return DecodeChoice( buffer , new VS_H245FECMode);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RedundancyEncodingDTModeElement_Type::operator=(const VS_H245RedundancyEncodingDTModeElement_Type & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_videoMode : CopyChoice< VS_H245VideoMode >(src,*this); return;
		case e_audioMode : CopyChoice< VS_H245AudioMode >(src,*this); return;
		case e_dataMode : CopyChoice< VS_H245DataMode >(src,*this); return;
		case e_encryptionMode : CopyChoice< VS_H245EncryptionMode >(src,*this); return;
		case e_h235Mode : CopyChoice< VS_H245H235Mode >(src,*this); return;
		case e_fecMode : CopyChoice< VS_H245FECMode >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245RedundancyEncodingDTModeElement_Type::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245RedundancyEncodingDTModeElement_Type::operator VS_H245VideoMode *( void )
	{	return dynamic_cast< VS_H245VideoMode * >(choice);    }

 	VS_H245RedundancyEncodingDTModeElement_Type::operator VS_H245AudioMode *( void )
	{	return dynamic_cast< VS_H245AudioMode * >(choice);    }

 	VS_H245RedundancyEncodingDTModeElement_Type::operator VS_H245DataMode *( void )
	{	return dynamic_cast< VS_H245DataMode * >(choice);    }

 	VS_H245RedundancyEncodingDTModeElement_Type::operator VS_H245EncryptionMode *( void )
	{	return dynamic_cast< VS_H245EncryptionMode * >(choice);    }

 	VS_H245RedundancyEncodingDTModeElement_Type::operator VS_H245H235Mode *( void )
	{	return dynamic_cast< VS_H245H235Mode * >(choice);    }

 	VS_H245RedundancyEncodingDTModeElement_Type::operator VS_H245FECMode *( void )
	{	return dynamic_cast< VS_H245FECMode * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RedundancyEncodingDTModeElement_Type::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_videoMode :  dprint4("choice: VS_H245VideoMode ");return;
		case e_audioMode :  dprint4("choice: VS_H245AudioMode ");return;
		case e_dataMode :  dprint4("choice: VS_H245DataMode ");return;
		case e_encryptionMode :  dprint4("choice: VS_H245EncryptionMode ");return;
		case e_h235Mode :  dprint4("choice: VS_H245H235Mode ");return;
		case e_fecMode :  dprint4("choice: VS_H245FECMode ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RedundancyEncodingDTModeElement /////////////////////////
 	 VS_H245RedundancyEncodingDTModeElement :: VS_H245RedundancyEncodingDTModeElement( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&type,0);
	}
	void VS_H245RedundancyEncodingDTModeElement::operator=(const VS_H245RedundancyEncodingDTModeElement& src)
	{

		O_CC(filled);
		O_C(type);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RedundancyEncodingDTMode /////////////////////////
 	 VS_H245RedundancyEncodingDTMode :: VS_H245RedundancyEncodingDTMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&redundancyEncodingMethod,0);
		ref[1].Set(&primary,0);
		ref[2].Set(&secondary,0);
	}
	void VS_H245RedundancyEncodingDTMode::operator=(const VS_H245RedundancyEncodingDTMode& src)
	{

		O_CC(filled);
		O_C(redundancyEncodingMethod);
		O_C(primary);
		O_C(secondary);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexedStreamModeParameters /////////////////////////
 	 VS_H245MultiplexedStreamModeParameters :: VS_H245MultiplexedStreamModeParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&logicalChannelNumber,0);
	}
	void VS_H245MultiplexedStreamModeParameters::operator=(const VS_H245MultiplexedStreamModeParameters& src)
	{

		O_CC(filled);
		O_C(logicalChannelNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H235Mode_MediaMode /////////////////////////
 	 VS_H245H235Mode_MediaMode::VS_H245H235Mode_MediaMode( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H235Mode_MediaMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_videoMode : return DecodeChoice( buffer , new VS_H245VideoMode);
		case e_audioMode : return DecodeChoice( buffer , new VS_H245AudioMode);
		case e_dataMode : return DecodeChoice( buffer , new VS_H245DataMode);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H235Mode_MediaMode::operator=(const VS_H245H235Mode_MediaMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_videoMode : CopyChoice< VS_H245VideoMode >(src,*this); return;
		case e_audioMode : CopyChoice< VS_H245AudioMode >(src,*this); return;
		case e_dataMode : CopyChoice< VS_H245DataMode >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245H235Mode_MediaMode::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245H235Mode_MediaMode::operator VS_H245VideoMode *( void )
	{	return dynamic_cast< VS_H245VideoMode * >(choice);    }

 	VS_H245H235Mode_MediaMode::operator VS_H245AudioMode *( void )
	{	return dynamic_cast< VS_H245AudioMode * >(choice);    }

 	VS_H245H235Mode_MediaMode::operator VS_H245DataMode *( void )
	{	return dynamic_cast< VS_H245DataMode * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H235Mode_MediaMode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_videoMode :  dprint4("choice: VS_H245VideoMode ");return;
		case e_audioMode :  dprint4("choice: VS_H245AudioMode ");return;
		case e_dataMode :  dprint4("choice: VS_H245DataMode ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H235Mode /////////////////////////
 	 VS_H245H235Mode :: VS_H245H235Mode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&encryptionAuthenticationAndIntegrity,0);
		ref[1].Set(&mediaMode,0);
	}
	void VS_H245H235Mode::operator=(const VS_H245H235Mode& src)
	{

		O_CC(filled);
		O_C(encryptionAuthenticationAndIntegrity);
		O_C(mediaMode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ModeElement /////////////////////////
 	 VS_H245ModeElement :: VS_H245ModeElement( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&type,0);
		ref[1].Set(&h223ModeParameters,1);
		e_ref[0].Set(&v76ModeParameters,1);
		e_ref[1].Set(&h2250ModeParameters,1);
		e_ref[2].Set(&genericModeParameters,1);
		e_ref[3].Set(&multiplexedStreamModeParameters,1);
		e_ref[4].Set(&logicalChannelNumber,1);
	}
	void VS_H245ModeElement::operator=(const VS_H245ModeElement& src)
	{

		O_CC(filled);
		O_C(type);
		O_C(h223ModeParameters);
		O_C(v76ModeParameters);
		O_C(h2250ModeParameters);
		O_C(genericModeParameters);
		O_C(multiplexedStreamModeParameters);
		O_C(logicalChannelNumber);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ModeElementType /////////////////////////
 	 VS_H245ModeElementType::VS_H245ModeElementType( void )
	:VS_AsnChoice(5 , 11 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ModeElementType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_videoMode : return DecodeChoice( buffer , new VS_H245VideoMode);
		case e_audioMode : return DecodeChoice( buffer , new VS_H245AudioMode);
		case e_dataMode : return DecodeChoice( buffer , new VS_H245DataMode);
		case e_encryptionMode : return DecodeChoice( buffer , new VS_H245EncryptionMode);
		case e_h235Mode : return DecodeChoice( buffer , new VS_H245H235Mode);
		case e_multiplexedStreamMode : return DecodeChoice( buffer , new VS_H245MultiplexedStreamParameter);
		case e_redundancyEncodingDTMode : return DecodeChoice( buffer , new VS_H245RedundancyEncodingDTMode);
		case e_multiplePayloadStreamMode : return DecodeChoice( buffer , new VS_H245MultiplePayloadStreamMode);
		case e_depFfecMode : return DecodeChoice( buffer , new VS_H245DepFECMode);
		case e_fecMode : return DecodeChoice( buffer , new VS_H245FECMode);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ModeElementType::operator=(const VS_H245ModeElementType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_videoMode : CopyChoice< VS_H245VideoMode >(src,*this); return;
		case e_audioMode : CopyChoice< VS_H245AudioMode >(src,*this); return;
		case e_dataMode : CopyChoice< VS_H245DataMode >(src,*this); return;
		case e_encryptionMode : CopyChoice< VS_H245EncryptionMode >(src,*this); return;
		case e_h235Mode : CopyChoice< VS_H245H235Mode >(src,*this); return;
		case e_multiplexedStreamMode : CopyChoice< VS_H245MultiplexedStreamParameter >(src,*this); return;
		case e_redundancyEncodingDTMode : CopyChoice< VS_H245RedundancyEncodingDTMode >(src,*this); return;
		case e_multiplePayloadStreamMode : CopyChoice< VS_H245MultiplePayloadStreamMode >(src,*this); return;
		case e_depFfecMode : CopyChoice< VS_H245DepFECMode >(src,*this); return;
		case e_fecMode : CopyChoice< VS_H245FECMode >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245ModeElementType::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245VideoMode *( void )
	{	return dynamic_cast< VS_H245VideoMode * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245AudioMode *( void )
	{	return dynamic_cast< VS_H245AudioMode * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245DataMode *( void )
	{	return dynamic_cast< VS_H245DataMode * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245EncryptionMode *( void )
	{	return dynamic_cast< VS_H245EncryptionMode * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245H235Mode *( void )
	{	return dynamic_cast< VS_H245H235Mode * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245MultiplexedStreamParameter *( void )
	{	return dynamic_cast< VS_H245MultiplexedStreamParameter * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245RedundancyEncodingDTMode *( void )
	{	return dynamic_cast< VS_H245RedundancyEncodingDTMode * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245MultiplePayloadStreamMode *( void )
	{	return dynamic_cast< VS_H245MultiplePayloadStreamMode * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245DepFECMode *( void )
	{	return dynamic_cast< VS_H245DepFECMode * >(choice);    }

 	VS_H245ModeElementType::operator VS_H245FECMode *( void )
	{	return dynamic_cast< VS_H245FECMode * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ModeElementType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_videoMode :  dprint4("choice: VS_H245VideoMode ");return;
		case e_audioMode :  dprint4("choice: VS_H245AudioMode ");return;
		case e_dataMode :  dprint4("choice: VS_H245DataMode ");return;
		case e_encryptionMode :  dprint4("choice: VS_H245EncryptionMode ");return;
		case e_h235Mode :  dprint4("choice: VS_H245H235Mode ");return;
		case e_multiplexedStreamMode :  dprint4("choice: VS_H245MultiplexedStreamParameter ");return;
		case e_redundancyEncodingDTMode :  dprint4("choice: VS_H245RedundancyEncodingDTMode ");return;
		case e_multiplePayloadStreamMode :  dprint4("choice: VS_H245MultiplePayloadStreamMode ");return;
		case e_depFfecMode :  dprint4("choice: VS_H245DepFECMode ");return;
		case e_fecMode :  dprint4("choice: VS_H245FECMode ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RequestModeRelease /////////////////////////
 	 VS_H245RequestModeRelease :: VS_H245RequestModeRelease( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr , extension_root , 1 )
	{

	}
	void VS_H245RequestModeRelease::operator=(const VS_H245RequestModeRelease& src)
	{

		O_CC(filled);
		O_CP(e_ref);  O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestModeReject_Cause /////////////////////////
 	 VS_H245RequestModeReject_Cause::VS_H245RequestModeReject_Cause( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RequestModeReject_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_modeUnavailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_multipointConstraint : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_requestDenied : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RequestModeReject_Cause::operator=(const VS_H245RequestModeReject_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_modeUnavailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_multipointConstraint : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_requestDenied : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RequestModeReject_Cause::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_modeUnavailable :  dprint4("choice:  VS_AsnNull  ");return;
		case e_multipointConstraint :  dprint4("choice:  VS_AsnNull  ");return;
		case e_requestDenied :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RequestModeReject /////////////////////////
 	 VS_H245RequestModeReject :: VS_H245RequestModeReject( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&cause,0);
	}
	void VS_H245RequestModeReject::operator=(const VS_H245RequestModeReject& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(cause);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestModeAck_Response /////////////////////////
 	 VS_H245RequestModeAck_Response::VS_H245RequestModeAck_Response( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RequestModeAck_Response::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_willTransmitMostPreferredMode : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_willTransmitLessPreferredMode : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RequestModeAck_Response::operator=(const VS_H245RequestModeAck_Response & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_willTransmitMostPreferredMode : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_willTransmitLessPreferredMode : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RequestModeAck_Response::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_willTransmitMostPreferredMode :  dprint4("choice:  VS_AsnNull  ");return;
		case e_willTransmitLessPreferredMode :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RequestModeAck /////////////////////////
 	 VS_H245RequestModeAck :: VS_H245RequestModeAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&response,0);
	}
	void VS_H245RequestModeAck::operator=(const VS_H245RequestModeAck& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(response);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestMode /////////////////////////
 	 VS_H245RequestMode :: VS_H245RequestMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&requestedModes,0);
	}
	void VS_H245RequestMode::operator=(const VS_H245RequestMode& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(requestedModes);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestMultiplexEntryRelease /////////////////////////
 	 VS_H245RequestMultiplexEntryRelease :: VS_H245RequestMultiplexEntryRelease( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&entryNumbers,0);
	}
	void VS_H245RequestMultiplexEntryRelease::operator=(const VS_H245RequestMultiplexEntryRelease& src)
	{

		O_CC(filled);
		O_C(entryNumbers);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestMultiplexEntryRejectionDescriptions_Cause /////////////////////////
 	 VS_H245RequestMultiplexEntryRejectionDescriptions_Cause::VS_H245RequestMultiplexEntryRejectionDescriptions_Cause( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RequestMultiplexEntryRejectionDescriptions_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unspecifiedCause : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RequestMultiplexEntryRejectionDescriptions_Cause::operator=(const VS_H245RequestMultiplexEntryRejectionDescriptions_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unspecifiedCause : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RequestMultiplexEntryRejectionDescriptions_Cause::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unspecifiedCause :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RequestMultiplexEntryRejectionDescriptions /////////////////////////
 	 VS_H245RequestMultiplexEntryRejectionDescriptions :: VS_H245RequestMultiplexEntryRejectionDescriptions( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&multiplexTableEntryNumber,0);
		ref[1].Set(&cause,0);
	}
	void VS_H245RequestMultiplexEntryRejectionDescriptions::operator=(const VS_H245RequestMultiplexEntryRejectionDescriptions& src)
	{

		O_CC(filled);
		O_C(multiplexTableEntryNumber);
		O_C(cause);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestMultiplexEntryReject /////////////////////////
 	 VS_H245RequestMultiplexEntryReject :: VS_H245RequestMultiplexEntryReject( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&entryNumbers,0);
		ref[1].Set(&rejectionDescriptions,0);
	}
	void VS_H245RequestMultiplexEntryReject::operator=(const VS_H245RequestMultiplexEntryReject& src)
	{

		O_CC(filled);
		O_C(entryNumbers);
		O_C(rejectionDescriptions);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestMultiplexEntryAck /////////////////////////
 	 VS_H245RequestMultiplexEntryAck :: VS_H245RequestMultiplexEntryAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&entryNumbers,0);
	}
	void VS_H245RequestMultiplexEntryAck::operator=(const VS_H245RequestMultiplexEntryAck& src)
	{

		O_CC(filled);
		O_C(entryNumbers);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestMultiplexEntry /////////////////////////
 	 VS_H245RequestMultiplexEntry :: VS_H245RequestMultiplexEntry( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&entryNumbers,0);
	}
	void VS_H245RequestMultiplexEntry::operator=(const VS_H245RequestMultiplexEntry& src)
	{

		O_CC(filled);
		O_C(entryNumbers);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexEntrySendRelease /////////////////////////
 	 VS_H245MultiplexEntrySendRelease :: VS_H245MultiplexEntrySendRelease( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&multiplexTableEntryNumber,0);
	}
	void VS_H245MultiplexEntrySendRelease::operator=(const VS_H245MultiplexEntrySendRelease& src)
	{

		O_CC(filled);
		O_C(multiplexTableEntryNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexEntryRejectionDescriptions_Cause /////////////////////////
 	 VS_H245MultiplexEntryRejectionDescriptions_Cause::VS_H245MultiplexEntryRejectionDescriptions_Cause( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultiplexEntryRejectionDescriptions_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unspecifiedCause : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_descriptorTooComplex : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultiplexEntryRejectionDescriptions_Cause::operator=(const VS_H245MultiplexEntryRejectionDescriptions_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unspecifiedCause : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_descriptorTooComplex : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultiplexEntryRejectionDescriptions_Cause::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unspecifiedCause :  dprint4("choice:  VS_AsnNull  ");return;
		case e_descriptorTooComplex :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultiplexEntryRejectionDescriptions /////////////////////////
 	 VS_H245MultiplexEntryRejectionDescriptions :: VS_H245MultiplexEntryRejectionDescriptions( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&multiplexTableEntryNumber,0);
		ref[1].Set(&cause,0);
	}
	void VS_H245MultiplexEntryRejectionDescriptions::operator=(const VS_H245MultiplexEntryRejectionDescriptions& src)
	{

		O_CC(filled);
		O_C(multiplexTableEntryNumber);
		O_C(cause);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexEntrySendReject /////////////////////////
 	 VS_H245MultiplexEntrySendReject :: VS_H245MultiplexEntrySendReject( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&rejectionDescriptions,0);
	}
	void VS_H245MultiplexEntrySendReject::operator=(const VS_H245MultiplexEntrySendReject& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(rejectionDescriptions);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexEntrySendAck /////////////////////////
 	 VS_H245MultiplexEntrySendAck :: VS_H245MultiplexEntrySendAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&multiplexTableEntryNumber,0);
	}
	void VS_H245MultiplexEntrySendAck::operator=(const VS_H245MultiplexEntrySendAck& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(multiplexTableEntryNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexElement_Type /////////////////////////
 	 VS_H245MultiplexElement_Type::VS_H245MultiplexElement_Type( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultiplexElement_Type::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_logicalChannelNumber : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		case e_subElementList : return DecodeChoice( buffer , new VS_H245MultiplexElement);
		default: return false;
		}

	}

	void VS_H245MultiplexElement_Type::operator=(const VS_H245MultiplexElement_Type & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_logicalChannelNumber : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_subElementList : CopyChoice< VS_H245MultiplexElement >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MultiplexElement_Type::operator VS_H245MultiplexElement *( void )
	{	return dynamic_cast< VS_H245MultiplexElement * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultiplexElement_Type::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_logicalChannelNumber :  dprint4("choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_subElementList :  dprint4("choice: VS_H245MultiplexElement ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultiplexElement_RepeatCount /////////////////////////
 	 VS_H245MultiplexElement_RepeatCount::VS_H245MultiplexElement_RepeatCount( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultiplexElement_RepeatCount::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_finite : return DecodeChoice( buffer , new TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  );
		case e_untilClosingFlag : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245MultiplexElement_RepeatCount::operator=(const VS_H245MultiplexElement_RepeatCount & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_finite : CopyChoice<TemplInteger<1,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_untilClosingFlag : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultiplexElement_RepeatCount::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_finite :  dprint4("choice: TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_untilClosingFlag :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultiplexElement /////////////////////////
 	 VS_H245MultiplexElement :: VS_H245MultiplexElement( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&type,0);
		ref[1].Set(&repeatCount,0);
	}
	void VS_H245MultiplexElement::operator=(const VS_H245MultiplexElement& src)
	{

		O_CC(filled);
		O_C(type);
		O_C(repeatCount);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexEntryDescriptor /////////////////////////
 	 VS_H245MultiplexEntryDescriptor :: VS_H245MultiplexEntryDescriptor( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&multiplexTableEntryNumber,0);
		ref[1].Set(&elementList,1);
	}
	void VS_H245MultiplexEntryDescriptor::operator=(const VS_H245MultiplexEntryDescriptor& src)
	{

		O_CC(filled);
		O_C(multiplexTableEntryNumber);
		O_C(elementList);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexEntrySend /////////////////////////
 	 VS_H245MultiplexEntrySend :: VS_H245MultiplexEntrySend( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&multiplexEntryDescriptors,0);
	}
	void VS_H245MultiplexEntrySend::operator=(const VS_H245MultiplexEntrySend& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(multiplexEntryDescriptors);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestChannelCloseRelease /////////////////////////
 	 VS_H245RequestChannelCloseRelease :: VS_H245RequestChannelCloseRelease( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
	}
	void VS_H245RequestChannelCloseRelease::operator=(const VS_H245RequestChannelCloseRelease& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestChannelCloseReject_Cause /////////////////////////
 	 VS_H245RequestChannelCloseReject_Cause::VS_H245RequestChannelCloseReject_Cause( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RequestChannelCloseReject_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unspecified : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RequestChannelCloseReject_Cause::operator=(const VS_H245RequestChannelCloseReject_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unspecified : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RequestChannelCloseReject_Cause::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unspecified :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RequestChannelCloseReject /////////////////////////
 	 VS_H245RequestChannelCloseReject :: VS_H245RequestChannelCloseReject( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
		ref[1].Set(&cause,0);
	}
	void VS_H245RequestChannelCloseReject::operator=(const VS_H245RequestChannelCloseReject& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_C(cause);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestChannelCloseAck /////////////////////////
 	 VS_H245RequestChannelCloseAck :: VS_H245RequestChannelCloseAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
	}
	void VS_H245RequestChannelCloseAck::operator=(const VS_H245RequestChannelCloseAck& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RequestChannelClose_Reason /////////////////////////
 	 VS_H245RequestChannelClose_Reason::VS_H245RequestChannelClose_Reason( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RequestChannelClose_Reason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unknown : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_normal : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_reopen : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_reservationFailure : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RequestChannelClose_Reason::operator=(const VS_H245RequestChannelClose_Reason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unknown : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_normal : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_reopen : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_reservationFailure : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RequestChannelClose_Reason::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unknown :  dprint4("choice:  VS_AsnNull  ");return;
		case e_normal :  dprint4("choice:  VS_AsnNull  ");return;
		case e_reopen :  dprint4("choice:  VS_AsnNull  ");return;
		case e_reservationFailure :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RequestChannelClose /////////////////////////
 	 VS_H245RequestChannelClose :: VS_H245RequestChannelClose( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
		e_ref[0].Set(&qosCapability,1);
		e_ref[1].Set(&reason,0);
	}
	void VS_H245RequestChannelClose::operator=(const VS_H245RequestChannelClose& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_C(qosCapability);
		O_C(reason);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CloseLogicalChannelAck /////////////////////////
 	 VS_H245CloseLogicalChannelAck :: VS_H245CloseLogicalChannelAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
	}
	void VS_H245CloseLogicalChannelAck::operator=(const VS_H245CloseLogicalChannelAck& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CloseLogicalChannel_Source /////////////////////////
 	 VS_H245CloseLogicalChannel_Source::VS_H245CloseLogicalChannel_Source( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CloseLogicalChannel_Source::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_user : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_lcse : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245CloseLogicalChannel_Source::operator=(const VS_H245CloseLogicalChannel_Source & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_user : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_lcse : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CloseLogicalChannel_Source::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_user :  dprint4("choice:  VS_AsnNull  ");return;
		case e_lcse :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245CloseLogicalChannel_Reason /////////////////////////
 	 VS_H245CloseLogicalChannel_Reason::VS_H245CloseLogicalChannel_Reason( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CloseLogicalChannel_Reason::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unknown : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_reopen : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_reservationFailure : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245CloseLogicalChannel_Reason::operator=(const VS_H245CloseLogicalChannel_Reason & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unknown : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_reopen : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_reservationFailure : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CloseLogicalChannel_Reason::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unknown :  dprint4("choice:  VS_AsnNull  ");return;
		case e_reopen :  dprint4("choice:  VS_AsnNull  ");return;
		case e_reservationFailure :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245CloseLogicalChannel /////////////////////////
 	 VS_H245CloseLogicalChannel :: VS_H245CloseLogicalChannel( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
		ref[1].Set(&source,0);
		e_ref[0].Set(&reason,0);
	}
	void VS_H245CloseLogicalChannel::operator=(const VS_H245CloseLogicalChannel& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_C(source);
		O_C(reason);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H2250LogicalChannelAckParameters /////////////////////////
 	 VS_H245H2250LogicalChannelAckParameters :: VS_H245H2250LogicalChannelAckParameters( void )
	:VS_AsnSequence(5 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandard,1);
		ref[1].Set(&sessionID,1);
		ref[2].Set(&mediaChannel,1);
		ref[3].Set(&mediaControlChannel,1);
		ref[4].Set(&dynamicRTPPayloadType,1);
		e_ref[0].Set(&flowControlToZero,0);
		e_ref[1].Set(&portNumber,1);
	}
	void VS_H245H2250LogicalChannelAckParameters::operator=(const VS_H245H2250LogicalChannelAckParameters& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_C(sessionID);
		O_C(mediaChannel);
		O_C(mediaControlChannel);
		O_C(dynamicRTPPayloadType);
		O_C(flowControlToZero);
		O_C(portNumber);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245OpenLogicalChannelConfirm /////////////////////////
 	 VS_H245OpenLogicalChannelConfirm :: VS_H245OpenLogicalChannelConfirm( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
	}
	void VS_H245OpenLogicalChannelConfirm::operator=(const VS_H245OpenLogicalChannelConfirm& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245OpenLogicalChannelReject_Cause /////////////////////////
 	 VS_H245OpenLogicalChannelReject_Cause::VS_H245OpenLogicalChannelReject_Cause( void )
	:VS_AsnChoice(6 , 15 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245OpenLogicalChannelReject_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unspecified : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_unsuitableReverseParameters : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_dataTypeNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_dataTypeNotAvailable : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_unknownDataType : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_dataTypeALCombinationNotSupported : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_multicastChannelNotAllowed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_insufficientBandwidth : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_separateStackEstablishmentFailed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidSessionID : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_masterSlaveConflict : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_waitForCommunicationMode : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_invalidDependentChannel : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_replacementForRejected : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_securityDenied : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245OpenLogicalChannelReject_Cause::operator=(const VS_H245OpenLogicalChannelReject_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unspecified : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_unsuitableReverseParameters : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_dataTypeNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_dataTypeNotAvailable : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_unknownDataType : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_dataTypeALCombinationNotSupported : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_multicastChannelNotAllowed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_insufficientBandwidth : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_separateStackEstablishmentFailed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidSessionID : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_masterSlaveConflict : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_waitForCommunicationMode : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_invalidDependentChannel : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_replacementForRejected : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_securityDenied : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245OpenLogicalChannelReject_Cause::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unspecified :  dprint4("choice:  VS_AsnNull  ");return;
		case e_unsuitableReverseParameters :  dprint4("choice:  VS_AsnNull  ");return;
		case e_dataTypeNotSupported :  dprint4("choice:  VS_AsnNull  ");return;
		case e_dataTypeNotAvailable :  dprint4("choice:  VS_AsnNull  ");return;
		case e_unknownDataType :  dprint4("choice:  VS_AsnNull  ");return;
		case e_dataTypeALCombinationNotSupported :  dprint4("choice:  VS_AsnNull  ");return;
		case e_multicastChannelNotAllowed :  dprint4("choice:  VS_AsnNull  ");return;
		case e_insufficientBandwidth :  dprint4("choice:  VS_AsnNull  ");return;
		case e_separateStackEstablishmentFailed :  dprint4("choice:  VS_AsnNull  ");return;
		case e_invalidSessionID :  dprint4("choice:  VS_AsnNull  ");return;
		case e_masterSlaveConflict :  dprint4("choice:  VS_AsnNull  ");return;
		case e_waitForCommunicationMode :  dprint4("choice:  VS_AsnNull  ");return;
		case e_invalidDependentChannel :  dprint4("choice:  VS_AsnNull  ");return;
		case e_replacementForRejected :  dprint4("choice:  VS_AsnNull  ");return;
		case e_securityDenied :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245OpenLogicalChannelReject /////////////////////////
 	 VS_H245OpenLogicalChannelReject :: VS_H245OpenLogicalChannelReject( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
		ref[1].Set(&cause,0);
	}
	void VS_H245OpenLogicalChannelReject::operator=(const VS_H245OpenLogicalChannelReject& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_C(cause);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters /////////////////////////
 	 VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters::VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters( void )
	:VS_AsnChoice(1 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_h222LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245H222LogicalChannelParameters);
		case e_h2250LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245H2250LogicalChannelParameters);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters::operator=(const VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_h222LogicalChannelParameters : CopyChoice< VS_H245H222LogicalChannelParameters >(src,*this); return;
		case e_h2250LogicalChannelParameters : CopyChoice< VS_H245H2250LogicalChannelParameters >(src,*this); return;
		default:		return;
		}
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters::operator VS_H245H222LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245H222LogicalChannelParameters * >(choice);    }

 	VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters::operator VS_H245H2250LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245H2250LogicalChannelParameters * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters_MultiplexParameters::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_h222LogicalChannelParameters :  dprint4("choice: VS_H245H222LogicalChannelParameters ");return;
		case e_h2250LogicalChannelParameters :  dprint4("choice: VS_H245H2250LogicalChannelParameters ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters /////////////////////////
 	 VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters :: VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&reverseLogicalChannelNumber,0);
		ref[1].Set(&portNumber,1);
		ref[2].Set(&multiplexParameters,1);
		e_ref[0].Set(&replacementFor,1);
	}
	void VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters::operator=(const VS_H245OpenLogicalChannelAck_ReverseLogicalChannelParameters& src)
	{

		O_CC(filled);
		O_C(reverseLogicalChannelNumber);
		O_C(portNumber);
		O_C(multiplexParameters);
		O_C(replacementFor);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters /////////////////////////
 	 VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters::VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_h2250LogicalChannelAckParameters : return DecodeChoice( buffer , new VS_H245H2250LogicalChannelAckParameters);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters::operator=(const VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_h2250LogicalChannelAckParameters : CopyChoice< VS_H245H2250LogicalChannelAckParameters >(src,*this); return;
		default:		return;
		}

		return;
	}
void VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters::operator=( VS_H245H2250LogicalChannelAckParameters *h225lcap )
{
	FreeChoice();
	choice = h225lcap;
	tag = e_h2250LogicalChannelAckParameters;
	filled = true;
}
 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters::operator VS_H245H2250LogicalChannelAckParameters *( void )
	{	return dynamic_cast< VS_H245H2250LogicalChannelAckParameters * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245OpenLogicalChannelAck_ForwardMultiplexAckParameters::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_h2250LogicalChannelAckParameters :  dprint4("choice: VS_H245H2250LogicalChannelAckParameters ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245OpenLogicalChannelAck /////////////////////////
 	 VS_H245OpenLogicalChannelAck :: VS_H245OpenLogicalChannelAck( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
		ref[1].Set(&reverseLogicalChannelParameters,1);
		e_ref[0].Set(&separateStack,1);
		e_ref[1].Set(&forwardMultiplexAckParameters,1);
		e_ref[2].Set(&encryptionSync,1);
	}
	void VS_H245OpenLogicalChannelAck::operator=(const VS_H245OpenLogicalChannelAck& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_C(reverseLogicalChannelParameters);
		O_C(separateStack);
		O_C(forwardMultiplexAckParameters);
		O_C(encryptionSync);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245EscrowData /////////////////////////
 	 VS_H245EscrowData :: VS_H245EscrowData( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&escrowID,0);
		ref[1].Set(&escrowValue,0);
	}
	void VS_H245EscrowData::operator=(const VS_H245EscrowData& src)
	{

		O_CC(filled);
		O_C(escrowID);
		O_C(escrowValue);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245EncryptionSync /////////////////////////
 	 VS_H245EncryptionSync :: VS_H245EncryptionSync( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref, extension_root , 1 )
	{
		ref[0].Set(&nonStandard,1);
		ref[1].Set(&synchFlag,0);
		ref[2].Set(&h235Key,0);
		ref[3].Set(&escrowentry,1);
		e_ref[0].Set(&genericParameter,1);
	}
	void VS_H245EncryptionSync::operator=(const VS_H245EncryptionSync& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_C(synchFlag);
		O_C(h235Key);
		O_C(escrowentry);
		O_C(genericParameter);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MulticastAddress_IPAddress /////////////////////////
 	 VS_H245MulticastAddress_IPAddress :: VS_H245MulticastAddress_IPAddress( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&network,0);
		ref[1].Set(&tsapIdentifier,0);
	}
	void VS_H245MulticastAddress_IPAddress::operator=(const VS_H245MulticastAddress_IPAddress& src)
	{

		O_CC(filled);
		O_C(network);
		O_C(tsapIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MulticastAddress_IP6Address /////////////////////////
 	 VS_H245MulticastAddress_IP6Address :: VS_H245MulticastAddress_IP6Address( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&network,0);
		ref[1].Set(&tsapIdentifier,0);
	}
	void VS_H245MulticastAddress_IP6Address::operator=(const VS_H245MulticastAddress_IP6Address& src)
	{

		O_CC(filled);
		O_C(network);
		O_C(tsapIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MulticastAddress /////////////////////////
 	 VS_H245MulticastAddress::VS_H245MulticastAddress( void )
	:VS_AsnChoice(2 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MulticastAddress::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_iPAddress : return DecodeChoice( buffer , new VS_H245MulticastAddress_IPAddress	 );
		case e_iP6Address : return DecodeChoice( buffer , new VS_H245MulticastAddress_IP6Address	 );
		case e_nsap : return DecodeChoice( buffer , new TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  );
		case e_nonStandardAddress : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MulticastAddress::operator=(const VS_H245MulticastAddress & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_iPAddress : CopyChoice<VS_H245MulticastAddress_IPAddress	  >(src,*this);  return;
		case e_iP6Address : CopyChoice<VS_H245MulticastAddress_IP6Address	  >(src,*this);  return;
		case e_nsap : CopyChoice<TemplOctetString<1,20,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_nonStandardAddress : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MulticastAddress::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MulticastAddress::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_iPAddress :  dprint4("choice: VS_H245MulticastAddress_IPAddress	 ");return;
		case e_iP6Address :  dprint4("choice: VS_H245MulticastAddress_IP6Address	 ");return;
		case e_nsap :  dprint4("choice: TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  ");return;
		case e_nonStandardAddress :  dprint4("choice: VS_H245NonStandardParameter ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245UnicastAddress_IPAddress /////////////////////////
 	 VS_H245UnicastAddress_IPAddress :: VS_H245UnicastAddress_IPAddress( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&network,0);
		ref[1].Set(&tsapIdentifier,0);
	}
	void VS_H245UnicastAddress_IPAddress::operator=(const VS_H245UnicastAddress_IPAddress& src)
	{

		O_CC(filled);
		O_C(network);
		O_C(tsapIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UnicastAddress_IPXAddress /////////////////////////
 	 VS_H245UnicastAddress_IPXAddress :: VS_H245UnicastAddress_IPXAddress( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&node,0);
		ref[1].Set(&netnum,0);
		ref[2].Set(&tsapIdentifier,0);
	}
	void VS_H245UnicastAddress_IPXAddress::operator=(const VS_H245UnicastAddress_IPXAddress& src)
	{

		O_CC(filled);
		O_C(node);
		O_C(netnum);
		O_C(tsapIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UnicastAddress_IP6Address /////////////////////////
 	 VS_H245UnicastAddress_IP6Address :: VS_H245UnicastAddress_IP6Address( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&network,0);
		ref[1].Set(&tsapIdentifier,0);
	}
	void VS_H245UnicastAddress_IP6Address::operator=(const VS_H245UnicastAddress_IP6Address& src)
	{

		O_CC(filled);
		O_C(network);
		O_C(tsapIdentifier);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UnicastAddress_IPSourceRouteAddress_Routing /////////////////////////
 	 VS_H245UnicastAddress_IPSourceRouteAddress_Routing::VS_H245UnicastAddress_IPSourceRouteAddress_Routing( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245UnicastAddress_IPSourceRouteAddress_Routing::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_strict : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_loose : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245UnicastAddress_IPSourceRouteAddress_Routing::operator=(const VS_H245UnicastAddress_IPSourceRouteAddress_Routing & src)
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
	void VS_H245UnicastAddress_IPSourceRouteAddress_Routing::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_strict :  dprint4("choice:  VS_AsnNull  ");return;
		case e_loose :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245UnicastAddress_IPSourceRouteAddress /////////////////////////
 	 VS_H245UnicastAddress_IPSourceRouteAddress :: VS_H245UnicastAddress_IPSourceRouteAddress( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&routing,0);
		ref[1].Set(&network,0);
		ref[2].Set(&tsapIdentifier,0);
		ref[3].Set(&route,0);
	}
	void VS_H245UnicastAddress_IPSourceRouteAddress::operator=(const VS_H245UnicastAddress_IPSourceRouteAddress& src)
	{

		O_CC(filled);
		O_C(routing);
		O_C(network);
		O_C(tsapIdentifier);
		O_C(route);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UnicastAddress /////////////////////////
 	 VS_H245UnicastAddress::VS_H245UnicastAddress( void )
	:VS_AsnChoice(5 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245UnicastAddress::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this))
			return false;
		switch(tag)
		{
		case e_iPAddress : return DecodeChoice( buffer , new VS_H245UnicastAddress_IPAddress	 );
		case e_iPXAddress : return DecodeChoice( buffer , new VS_H245UnicastAddress_IPXAddress	 );
		case e_iP6Address : return DecodeChoice( buffer , new VS_H245UnicastAddress_IP6Address	 );
		case e_netBios : return DecodeChoice( buffer , new TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  );
		case e_iPSourceRouteAddress : return DecodeChoice( buffer , new VS_H245UnicastAddress_IPSourceRouteAddress	 );
		case e_nsap : return DecodeChoice( buffer , new TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  );
		case e_nonStandardAddress : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245UnicastAddress::operator=(const VS_H245UnicastAddress & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_iPAddress : CopyChoice<VS_H245UnicastAddress_IPAddress	  >(src,*this);  return;
		case e_iPXAddress : CopyChoice<VS_H245UnicastAddress_IPXAddress	  >(src,*this);  return;
		case e_iP6Address : CopyChoice<VS_H245UnicastAddress_IP6Address	  >(src,*this);  return;
		case e_netBios : CopyChoice<TemplOctetString<16,16,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_iPSourceRouteAddress : CopyChoice<VS_H245UnicastAddress_IPSourceRouteAddress	  >(src,*this);  return;
		case e_nsap : CopyChoice<TemplOctetString<1,20,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_nonStandardAddress : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245UnicastAddress::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245UnicastAddress::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_iPAddress :  dprint4("choice: VS_H245UnicastAddress_IPAddress	 ");return;
		case e_iPXAddress :  dprint4("choice: VS_H245UnicastAddress_IPXAddress	 ");return;
		case e_iP6Address :  dprint4("choice: VS_H245UnicastAddress_IP6Address	 ");return;
		case e_netBios :  dprint4("choice: TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  ");return;
		case e_iPSourceRouteAddress :  dprint4("choice: VS_H245UnicastAddress_IPSourceRouteAddress	 ");return;
		case e_nsap :  dprint4("choice: TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  ");return;
		case e_nonStandardAddress :  dprint4("choice: VS_H245NonStandardParameter ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}
	VS_H245UnicastAddress::operator VS_H245UnicastAddress_IPAddress *(void)
	{
		return dynamic_cast<VS_H245UnicastAddress_IPAddress*>(choice);
	}

	VS_H245UnicastAddress::operator VS_H245UnicastAddress_IP6Address*(void)
	{
		return dynamic_cast<VS_H245UnicastAddress_IP6Address*>(choice);
	}

	void VS_H245UnicastAddress::operator =(VS_H245UnicastAddress_IPAddress *uma_ipa)
	{
		FreeChoice();
		choice = uma_ipa;
		tag = e_iPAddress;
		filled = true;
	}
	void VS_H245UnicastAddress::operator =(VS_H245UnicastAddress_IP6Address *uma_ipa)
	{
		FreeChoice();
		choice = uma_ipa;
		tag = e_iP6Address;
		filled = true;
	}
//////////////////////CLASS VS_H245TransportAddress /////////////////////////
 	 VS_H245TransportAddress::VS_H245TransportAddress( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245TransportAddress::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unicastAddress : return DecodeChoice( buffer , new VS_H245UnicastAddress);
		case e_multicastAddress : return DecodeChoice( buffer , new VS_H245MulticastAddress);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245TransportAddress::operator=(const VS_H245TransportAddress & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unicastAddress : CopyChoice< VS_H245UnicastAddress >(src,*this); return;
		case e_multicastAddress : CopyChoice< VS_H245MulticastAddress >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245TransportAddress::operator VS_H245UnicastAddress *( void )
	{	return dynamic_cast< VS_H245UnicastAddress * >(choice);    }

 	VS_H245TransportAddress::operator VS_H245MulticastAddress *( void )
	{	return dynamic_cast< VS_H245MulticastAddress * >(choice);    }

	void VS_H245TransportAddress::operator=( VS_H245UnicastAddress *ua )
	{
		FreeChoice();
		choice = ua;
		tag = e_unicastAddress;
		filled = true;
	}
 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245TransportAddress::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unicastAddress :  dprint4("choice: VS_H245UnicastAddress ");return;
		case e_multicastAddress :  dprint4("choice: VS_H245MulticastAddress ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport /////////////////////////
 	 VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport :: VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr , extension_root , 1 )
	{
	}
	void VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport::operator=(const VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport& src)
	{

		O_CC(filled);
		O_CP(e_ref);  O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport /////////////////////////
 	 VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport :: VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&protectedChannel,0);
	}
	void VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport::operator=(const VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport& src)
	{

		O_CC(filled);
		O_C(protectedChannel);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245FECData_Rfc2733_PktMode /////////////////////////
 	 VS_H245FECData_Rfc2733_PktMode::VS_H245FECData_Rfc2733_PktMode( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FECData_Rfc2733_PktMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_rfc2198coding : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_rfc2733sameport : return DecodeChoice( buffer , new VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport	 );
		case e_rfc2733diffport : return DecodeChoice( buffer , new VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245FECData_Rfc2733_PktMode::operator=(const VS_H245FECData_Rfc2733_PktMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_rfc2198coding : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_rfc2733sameport : CopyChoice<VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport	  >(src,*this);  return;
		case e_rfc2733diffport : CopyChoice<VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FECData_Rfc2733_PktMode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_rfc2198coding :  dprint4("choice:  VS_AsnNull  ");return;
		case e_rfc2733sameport :  dprint4("choice: VS_H245FECData_Rfc2733_PktMode_Rfc2733sameport	 ");return;
		case e_rfc2733diffport :  dprint4("choice: VS_H245FECData_Rfc2733_PktMode_Rfc2733diffport	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FECData_Rfc2733 /////////////////////////
 	 VS_H245FECData_Rfc2733 :: VS_H245FECData_Rfc2733( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&protectedPayloadType,0);
		ref[1].Set(&fecScheme,1);
		ref[2].Set(&pktMode,0);
	}
	void VS_H245FECData_Rfc2733::operator=(const VS_H245FECData_Rfc2733& src)
	{

		O_CC(filled);
		O_C(protectedPayloadType);
		O_C(fecScheme);
		O_C(pktMode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245FECData /////////////////////////
 	 VS_H245FECData::VS_H245FECData( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FECData::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_rfc2733 : return DecodeChoice( buffer , new VS_H245FECData_Rfc2733	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245FECData::operator=(const VS_H245FECData & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_rfc2733 : CopyChoice<VS_H245FECData_Rfc2733	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FECData::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_rfc2733 :  dprint4("choice: VS_H245FECData_Rfc2733	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort /////////////////////////
 	 VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort :: VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&protectedSessionID,0);
		ref[1].Set(&protectedPayloadType,1);
	}
	void VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort::operator=(const VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort& src)
	{

		O_CC(filled);
		O_C(protectedSessionID);
		O_C(protectedPayloadType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort /////////////////////////
 	 VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort :: VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&protectedPayloadType,0);
	}
	void VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort::operator=(const VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort& src)
	{

		O_CC(filled);
		O_C(protectedPayloadType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECData_Rfc2733_Mode_SeparateStream /////////////////////////
 	 VS_H245DepFECData_Rfc2733_Mode_SeparateStream::VS_H245DepFECData_Rfc2733_Mode_SeparateStream( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DepFECData_Rfc2733_Mode_SeparateStream::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_differentPort : return DecodeChoice( buffer , new VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort	 );
		case e_samePort : return DecodeChoice( buffer , new VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DepFECData_Rfc2733_Mode_SeparateStream::operator=(const VS_H245DepFECData_Rfc2733_Mode_SeparateStream & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_differentPort : CopyChoice<VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort	  >(src,*this);  return;
		case e_samePort : CopyChoice<VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DepFECData_Rfc2733_Mode_SeparateStream::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_differentPort :  dprint4("choice: VS_H245DepFECData_Rfc2733_Mode_SeparateStream_DifferentPort	 ");return;
		case e_samePort :  dprint4("choice: VS_H245DepFECData_Rfc2733_Mode_SeparateStream_SamePort	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DepFECData_Rfc2733_Mode /////////////////////////
 	 VS_H245DepFECData_Rfc2733_Mode::VS_H245DepFECData_Rfc2733_Mode( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DepFECData_Rfc2733_Mode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_redundancyEncoding : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_separateStream : return DecodeChoice( buffer , new VS_H245DepFECData_Rfc2733_Mode_SeparateStream	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DepFECData_Rfc2733_Mode::operator=(const VS_H245DepFECData_Rfc2733_Mode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_redundancyEncoding : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_separateStream : CopyChoice<VS_H245DepFECData_Rfc2733_Mode_SeparateStream	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DepFECData_Rfc2733_Mode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_redundancyEncoding :  dprint4("choice:  VS_AsnNull  ");return;
		case e_separateStream :  dprint4("choice: VS_H245DepFECData_Rfc2733_Mode_SeparateStream	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DepFECData_Rfc2733 /////////////////////////
 	 VS_H245DepFECData_Rfc2733 :: VS_H245DepFECData_Rfc2733( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&mode,0);
	}
	void VS_H245DepFECData_Rfc2733::operator=(const VS_H245DepFECData_Rfc2733& src)
	{

		O_CC(filled);
		O_C(mode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECData /////////////////////////
 	 VS_H245DepFECData::VS_H245DepFECData( void )
	:VS_AsnChoice(1 , 1 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DepFECData::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_rfc2733 : return DecodeChoice( buffer , new VS_H245DepFECData_Rfc2733	 );
		default: return false;
		}

	}

	void VS_H245DepFECData::operator=(const VS_H245DepFECData & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_rfc2733 : CopyChoice<VS_H245DepFECData_Rfc2733	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DepFECData::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_rfc2733 :  dprint4("choice: VS_H245DepFECData_Rfc2733	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultiplePayloadStreamElement /////////////////////////
 	 VS_H245MultiplePayloadStreamElement :: VS_H245MultiplePayloadStreamElement( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&dataType,0);
		ref[1].Set(&payloadType,1);
	}
	void VS_H245MultiplePayloadStreamElement::operator=(const VS_H245MultiplePayloadStreamElement& src)
	{

		O_CC(filled);
		O_C(dataType);
		O_C(payloadType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplePayloadStream /////////////////////////
 	 VS_H245MultiplePayloadStream :: VS_H245MultiplePayloadStream( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&elements,0);
	}
	void VS_H245MultiplePayloadStream::operator=(const VS_H245MultiplePayloadStream& src)
	{

		O_CC(filled);
		O_C(elements);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RedundancyEncodingElement /////////////////////////
 	 VS_H245RedundancyEncodingElement :: VS_H245RedundancyEncodingElement( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&dataType,0);
		ref[1].Set(&payloadType,1);
	}
	void VS_H245RedundancyEncodingElement::operator=(const VS_H245RedundancyEncodingElement& src)
	{

		O_CC(filled);
		O_C(dataType);
		O_C(payloadType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RedundancyEncoding_RtpRedundancyEncoding /////////////////////////
 	 VS_H245RedundancyEncoding_RtpRedundancyEncoding :: VS_H245RedundancyEncoding_RtpRedundancyEncoding( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&primary,1);
		ref[1].Set(&secondary,1);
	}
	void VS_H245RedundancyEncoding_RtpRedundancyEncoding::operator=(const VS_H245RedundancyEncoding_RtpRedundancyEncoding& src)
	{

		O_CC(filled);
		O_C(primary);
		O_C(secondary);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RedundancyEncoding /////////////////////////
 	 VS_H245RedundancyEncoding :: VS_H245RedundancyEncoding( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&redundancyEncodingMethod,0);
		ref[1].Set(&secondaryEncoding,1);
		e_ref[0].Set(&rtpRedundancyEncoding,1);
	}
	void VS_H245RedundancyEncoding::operator=(const VS_H245RedundancyEncoding& src)
	{

		O_CC(filled);
		O_C(redundancyEncodingMethod);
		O_C(secondaryEncoding);
		O_C(rtpRedundancyEncoding);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RTPPayloadType_PayloadDescriptor /////////////////////////
 	 VS_H245RTPPayloadType_PayloadDescriptor::VS_H245RTPPayloadType_PayloadDescriptor( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RTPPayloadType_PayloadDescriptor::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandardIdentifier : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_rfc_number : return DecodeChoice( buffer , new TemplInteger<1,32768,VS_Asn::FixedConstraint,1>  );
		case e_oid : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RTPPayloadType_PayloadDescriptor::operator=(const VS_H245RTPPayloadType_PayloadDescriptor & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandardIdentifier : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_rfc_number : CopyChoice<TemplInteger<1,32768,VS_Asn::FixedConstraint,1>   >(src,*this);  return;
		case e_oid : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245RTPPayloadType_PayloadDescriptor::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RTPPayloadType_PayloadDescriptor::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandardIdentifier :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_rfc_number :  dprint4("choice: TemplInteger<1,32768,VS_Asn::FixedConstraint,1>  ");return;
		case e_oid :  dprint4("choice:  VS_AsnObjectId  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RTPPayloadType /////////////////////////
 	 VS_H245RTPPayloadType :: VS_H245RTPPayloadType( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&payloadDescriptor,0);
		ref[1].Set(&payloadType,1);
	}
	void VS_H245RTPPayloadType::operator=(const VS_H245RTPPayloadType& src)
	{

		O_CC(filled);
		O_C(payloadDescriptor);
		O_C(payloadType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H2250LogicalChannelParameters_MediaPacketization /////////////////////////
 	 VS_H245H2250LogicalChannelParameters_MediaPacketization::VS_H245H2250LogicalChannelParameters_MediaPacketization( void )
	:VS_AsnChoice(1 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H2250LogicalChannelParameters_MediaPacketization::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_h261aVideoPacketization : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_rtpPayloadType : return DecodeChoice( buffer , new VS_H245RTPPayloadType);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H2250LogicalChannelParameters_MediaPacketization::operator=(const VS_H245H2250LogicalChannelParameters_MediaPacketization & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_h261aVideoPacketization : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_rtpPayloadType : CopyChoice< VS_H245RTPPayloadType >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245H2250LogicalChannelParameters_MediaPacketization::operator VS_H245RTPPayloadType *( void )
	{	return dynamic_cast< VS_H245RTPPayloadType * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H2250LogicalChannelParameters_MediaPacketization::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_h261aVideoPacketization :  dprint4("choice:  VS_AsnNull  ");return;
		case e_rtpPayloadType :  dprint4("choice: VS_H245RTPPayloadType ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H2250LogicalChannelParameters /////////////////////////
 	 VS_H245H2250LogicalChannelParameters :: VS_H245H2250LogicalChannelParameters( void )
	:VS_AsnSequence(10 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandard,1);
		ref[1].Set(&sessionID,0);
		ref[2].Set(&associatedSessionID,1);
		ref[3].Set(&mediaChannel,1);
		ref[4].Set(&mediaGuaranteedDelivery,1);
		ref[5].Set(&mediaControlChannel,1);
		ref[6].Set(&mediaControlGuaranteedDelivery,1);
		ref[7].Set(&silenceSuppression,1);
		ref[8].Set(&destination,1);
		ref[9].Set(&dynamicRTPPayloadType,1);
		ref[10].Set(&mediaPacketization,1);
		e_ref[0].Set(&transportCapability,1);
		e_ref[1].Set(&redundancyEncoding,1);
		e_ref[2].Set(&source,1);
	}
	void VS_H245H2250LogicalChannelParameters::operator=(const VS_H245H2250LogicalChannelParameters& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_C(sessionID);
		O_C(associatedSessionID);
		O_C(mediaChannel);
		O_C(mediaGuaranteedDelivery);
		O_C(mediaControlChannel);
		O_C(mediaControlGuaranteedDelivery);
		O_C(silenceSuppression);
		O_C(destination);
		O_C(dynamicRTPPayloadType);
		O_C(mediaPacketization);
		O_C(transportCapability);
		O_C(redundancyEncoding);
		O_C(source);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CRCLength /////////////////////////
 	 VS_H245CRCLength::VS_H245CRCLength( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CRCLength::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_crc8bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc16bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc32bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245CRCLength::operator=(const VS_H245CRCLength & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_crc8bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc16bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc32bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CRCLength::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_crc8bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc16bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc32bit :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245V76HDLCParameters /////////////////////////
 	 VS_H245V76HDLCParameters :: VS_H245V76HDLCParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&crcLength,0);
		ref[1].Set(&n401,0);
		ref[2].Set(&loopbackTestProcedure,0);
	}
	void VS_H245V76HDLCParameters::operator=(const VS_H245V76HDLCParameters& src)
	{

		O_CC(filled);
		O_C(crcLength);
		O_C(n401);
		O_C(loopbackTestProcedure);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245V76LogicalChannelParameters_SuspendResume /////////////////////////
 	 VS_H245V76LogicalChannelParameters_SuspendResume::VS_H245V76LogicalChannelParameters_SuspendResume( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245V76LogicalChannelParameters_SuspendResume::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noSuspendResume : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_suspendResumewAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_suspendResumewoAddress : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245V76LogicalChannelParameters_SuspendResume::operator=(const VS_H245V76LogicalChannelParameters_SuspendResume & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noSuspendResume : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_suspendResumewAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_suspendResumewoAddress : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245V76LogicalChannelParameters_SuspendResume::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_noSuspendResume :  dprint4("choice:  VS_AsnNull  ");return;
		case e_suspendResumewAddress :  dprint4("choice:  VS_AsnNull  ");return;
		case e_suspendResumewoAddress :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery /////////////////////////
 	 VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery::VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_rej : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_sREJ : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_mSREJ : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery::operator=(const VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_rej : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_sREJ : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_mSREJ : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245V76LogicalChannelParameters_Mode_ERM_Recovery::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_rej :  dprint4("choice:  VS_AsnNull  ");return;
		case e_sREJ :  dprint4("choice:  VS_AsnNull  ");return;
		case e_mSREJ :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245V76LogicalChannelParameters_Mode_ERM /////////////////////////
 	 VS_H245V76LogicalChannelParameters_Mode_ERM :: VS_H245V76LogicalChannelParameters_Mode_ERM( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&windowSize,0);
		ref[1].Set(&recovery,0);
	}
	void VS_H245V76LogicalChannelParameters_Mode_ERM::operator=(const VS_H245V76LogicalChannelParameters_Mode_ERM& src)
	{

		O_CC(filled);
		O_C(windowSize);
		O_C(recovery);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245V76LogicalChannelParameters_Mode /////////////////////////
 	 VS_H245V76LogicalChannelParameters_Mode::VS_H245V76LogicalChannelParameters_Mode( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245V76LogicalChannelParameters_Mode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_eRM : return DecodeChoice( buffer , new VS_H245V76LogicalChannelParameters_Mode_ERM	 );
		case e_uNERM : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245V76LogicalChannelParameters_Mode::operator=(const VS_H245V76LogicalChannelParameters_Mode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_eRM : CopyChoice<VS_H245V76LogicalChannelParameters_Mode_ERM	  >(src,*this);  return;
		case e_uNERM : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245V76LogicalChannelParameters_Mode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_eRM :  dprint4("choice: VS_H245V76LogicalChannelParameters_Mode_ERM	 ");return;
		case e_uNERM :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245V76LogicalChannelParameters /////////////////////////
 	 VS_H245V76LogicalChannelParameters :: VS_H245V76LogicalChannelParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&hdlcParameters,0);
		ref[1].Set(&suspendResume,0);
		ref[2].Set(&uIH,0);
		ref[3].Set(&mode,0);
		ref[4].Set(&v75Parameters,0);
	}
	void VS_H245V76LogicalChannelParameters::operator=(const VS_H245V76LogicalChannelParameters& src)
	{

		O_CC(filled);
		O_C(hdlcParameters);
		O_C(suspendResume);
		O_C(uIH);
		O_C(mode);
		O_C(v75Parameters);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223AnnexCArqParameters_NumberOfRetransmissions /////////////////////////
 	 VS_H245H223AnnexCArqParameters_NumberOfRetransmissions::VS_H245H223AnnexCArqParameters_NumberOfRetransmissions( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AnnexCArqParameters_NumberOfRetransmissions::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_finite : return DecodeChoice( buffer , new TemplInteger<0,16,VS_Asn::FixedConstraint,0>  );
		case e_infinite : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AnnexCArqParameters_NumberOfRetransmissions::operator=(const VS_H245H223AnnexCArqParameters_NumberOfRetransmissions & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_finite : CopyChoice<TemplInteger<0,16,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_infinite : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AnnexCArqParameters_NumberOfRetransmissions::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_finite :  dprint4("choice: TemplInteger<0,16,VS_Asn::FixedConstraint,0>  ");return;
		case e_infinite :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AnnexCArqParameters /////////////////////////
 	 VS_H245H223AnnexCArqParameters :: VS_H245H223AnnexCArqParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&numberOfRetransmissions,0);
		ref[1].Set(&sendBufferSize,0);
	}
	void VS_H245H223AnnexCArqParameters::operator=(const VS_H245H223AnnexCArqParameters& src)
	{

		O_CC(filled);
		O_C(numberOfRetransmissions);
		O_C(sendBufferSize);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223AL3MParameters_HeaderFormat /////////////////////////
 	 VS_H245H223AL3MParameters_HeaderFormat::VS_H245H223AL3MParameters_HeaderFormat( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AL3MParameters_HeaderFormat::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_sebch16_7 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_golay24_12 : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AL3MParameters_HeaderFormat::operator=(const VS_H245H223AL3MParameters_HeaderFormat & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_sebch16_7 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_golay24_12 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AL3MParameters_HeaderFormat::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_sebch16_7 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_golay24_12 :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AL3MParameters_CrcLength /////////////////////////
 	 VS_H245H223AL3MParameters_CrcLength::VS_H245H223AL3MParameters_CrcLength( void )
	:VS_AsnChoice(4 , 8 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AL3MParameters_CrcLength::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_crc4bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc12bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc20bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc28bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc8bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc16bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc32bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crcNotUsed : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AL3MParameters_CrcLength::operator=(const VS_H245H223AL3MParameters_CrcLength & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_crc4bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc12bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc20bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc28bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc8bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc16bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc32bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crcNotUsed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AL3MParameters_CrcLength::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_crc4bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc12bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc20bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc28bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc8bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc16bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc32bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crcNotUsed :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AL3MParameters_ArqType /////////////////////////
 	 VS_H245H223AL3MParameters_ArqType::VS_H245H223AL3MParameters_ArqType( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AL3MParameters_ArqType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noArq : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_typeIArq : return DecodeChoice( buffer , new VS_H245H223AnnexCArqParameters);
		case e_typeIIArq : return DecodeChoice( buffer , new VS_H245H223AnnexCArqParameters);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AL3MParameters_ArqType::operator=(const VS_H245H223AL3MParameters_ArqType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noArq : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_typeIArq : CopyChoice< VS_H245H223AnnexCArqParameters >(src,*this); return;
		case e_typeIIArq : CopyChoice< VS_H245H223AnnexCArqParameters >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245H223AL3MParameters_ArqType::operator VS_H245H223AnnexCArqParameters *( void )
	{	return dynamic_cast< VS_H245H223AnnexCArqParameters * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AL3MParameters_ArqType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_noArq :  dprint4("choice:  VS_AsnNull  ");return;
		case e_typeIArq :  dprint4("choice: VS_H245H223AnnexCArqParameters ");return;
		case e_typeIIArq :  dprint4("choice: VS_H245H223AnnexCArqParameters ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AL3MParameters /////////////////////////
 	 VS_H245H223AL3MParameters :: VS_H245H223AL3MParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&headerFormat,0);
		ref[1].Set(&crcLength,0);
		ref[2].Set(&rcpcCodeRate,0);
		ref[3].Set(&arqType,0);
		ref[4].Set(&alpduInterleaving,0);
		e_ref[0].Set(&rsCodeCorrection,1);
	}
	void VS_H245H223AL3MParameters::operator=(const VS_H245H223AL3MParameters& src)
	{

		O_CC(filled);
		O_C(headerFormat);
		O_C(crcLength);
		O_C(rcpcCodeRate);
		O_C(arqType);
		O_C(alpduInterleaving);
		O_C(rsCodeCorrection);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223AL2MParameters_HeaderFEC /////////////////////////
 	 VS_H245H223AL2MParameters_HeaderFEC::VS_H245H223AL2MParameters_HeaderFEC( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AL2MParameters_HeaderFEC::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_sebch16_5 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_golay24_12 : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AL2MParameters_HeaderFEC::operator=(const VS_H245H223AL2MParameters_HeaderFEC & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_sebch16_5 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_golay24_12 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AL2MParameters_HeaderFEC::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_sebch16_5 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_golay24_12 :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AL2MParameters /////////////////////////
 	 VS_H245H223AL2MParameters :: VS_H245H223AL2MParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&headerFEC,0);
		ref[1].Set(&alpduInterleaving,0);
	}
	void VS_H245H223AL2MParameters::operator=(const VS_H245H223AL2MParameters& src)
	{

		O_CC(filled);
		O_C(headerFEC);
		O_C(alpduInterleaving);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223AL1MParameters_TransferMode /////////////////////////
 	 VS_H245H223AL1MParameters_TransferMode::VS_H245H223AL1MParameters_TransferMode( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AL1MParameters_TransferMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_framed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_unframed : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AL1MParameters_TransferMode::operator=(const VS_H245H223AL1MParameters_TransferMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_framed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_unframed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AL1MParameters_TransferMode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_framed :  dprint4("choice:  VS_AsnNull  ");return;
		case e_unframed :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AL1MParameters_HeaderFEC /////////////////////////
 	 VS_H245H223AL1MParameters_HeaderFEC::VS_H245H223AL1MParameters_HeaderFEC( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AL1MParameters_HeaderFEC::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_sebch16_7 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_golay24_12 : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AL1MParameters_HeaderFEC::operator=(const VS_H245H223AL1MParameters_HeaderFEC & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_sebch16_7 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_golay24_12 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AL1MParameters_HeaderFEC::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_sebch16_7 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_golay24_12 :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AL1MParameters_CrcLength /////////////////////////
 	 VS_H245H223AL1MParameters_CrcLength::VS_H245H223AL1MParameters_CrcLength( void )
	:VS_AsnChoice(4 , 8 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AL1MParameters_CrcLength::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_crc4bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc12bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc20bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc28bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc8bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc16bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crc32bit : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_crcNotUsed : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AL1MParameters_CrcLength::operator=(const VS_H245H223AL1MParameters_CrcLength & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_crc4bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc12bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc20bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc28bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc8bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc16bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crc32bit : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_crcNotUsed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AL1MParameters_CrcLength::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_crc4bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc12bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc20bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc28bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc8bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc16bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crc32bit :  dprint4("choice:  VS_AsnNull  ");return;
		case e_crcNotUsed :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AL1MParameters_ArqType /////////////////////////
 	 VS_H245H223AL1MParameters_ArqType::VS_H245H223AL1MParameters_ArqType( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223AL1MParameters_ArqType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_noArq : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_typeIArq : return DecodeChoice( buffer , new VS_H245H223AnnexCArqParameters);
		case e_typeIIArq : return DecodeChoice( buffer , new VS_H245H223AnnexCArqParameters);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223AL1MParameters_ArqType::operator=(const VS_H245H223AL1MParameters_ArqType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_noArq : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_typeIArq : CopyChoice< VS_H245H223AnnexCArqParameters >(src,*this); return;
		case e_typeIIArq : CopyChoice< VS_H245H223AnnexCArqParameters >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245H223AL1MParameters_ArqType::operator VS_H245H223AnnexCArqParameters *( void )
	{	return dynamic_cast< VS_H245H223AnnexCArqParameters * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223AL1MParameters_ArqType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_noArq :  dprint4("choice:  VS_AsnNull  ");return;
		case e_typeIArq :  dprint4("choice: VS_H245H223AnnexCArqParameters ");return;
		case e_typeIIArq :  dprint4("choice: VS_H245H223AnnexCArqParameters ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223AL1MParameters /////////////////////////
 	 VS_H245H223AL1MParameters :: VS_H245H223AL1MParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&transferMode,0);
		ref[1].Set(&headerFEC,0);
		ref[2].Set(&crcLength,0);
		ref[3].Set(&rcpcCodeRate,0);
		ref[4].Set(&arqType,0);
		ref[5].Set(&alpduInterleaving,0);
		ref[6].Set(&alsduSplitting,0);
		e_ref[0].Set(&rsCodeCorrection,1);
	}
	void VS_H245H223AL1MParameters::operator=(const VS_H245H223AL1MParameters& src)
	{

		O_CC(filled);
		O_C(transferMode);
		O_C(headerFEC);
		O_C(crcLength);
		O_C(rcpcCodeRate);
		O_C(arqType);
		O_C(alpduInterleaving);
		O_C(alsduSplitting);
		O_C(rsCodeCorrection);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3 /////////////////////////
 	 VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3 :: VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&controlFieldOctets,0);
		ref[1].Set(&sendBufferSize,0);
	}
	void VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3::operator=(const VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3& src)
	{

		O_CC(filled);
		O_C(controlFieldOctets);
		O_C(sendBufferSize);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223LogicalChannelParameters_AdaptationLayerType /////////////////////////
 	 VS_H245H223LogicalChannelParameters_AdaptationLayerType::VS_H245H223LogicalChannelParameters_AdaptationLayerType( void )
	:VS_AsnChoice(6 , 9 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223LogicalChannelParameters_AdaptationLayerType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_al1Framed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_al1NotFramed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_al2WithoutSequenceNumbers : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_al2WithSequenceNumbers : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_al3 : return DecodeChoice( buffer , new VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3	 );
		case e_al1M : return DecodeChoice( buffer , new VS_H245H223AL1MParameters);
		case e_al2M : return DecodeChoice( buffer , new VS_H245H223AL2MParameters);
		case e_al3M : return DecodeChoice( buffer , new VS_H245H223AL3MParameters);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H223LogicalChannelParameters_AdaptationLayerType::operator=(const VS_H245H223LogicalChannelParameters_AdaptationLayerType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_al1Framed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_al1NotFramed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_al2WithoutSequenceNumbers : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_al2WithSequenceNumbers : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_al3 : CopyChoice<VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3	  >(src,*this);  return;
		case e_al1M : CopyChoice< VS_H245H223AL1MParameters >(src,*this); return;
		case e_al2M : CopyChoice< VS_H245H223AL2MParameters >(src,*this); return;
		case e_al3M : CopyChoice< VS_H245H223AL3MParameters >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245H223LogicalChannelParameters_AdaptationLayerType::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245H223LogicalChannelParameters_AdaptationLayerType::operator VS_H245H223AL1MParameters *( void )
	{	return dynamic_cast< VS_H245H223AL1MParameters * >(choice);    }

 	VS_H245H223LogicalChannelParameters_AdaptationLayerType::operator VS_H245H223AL2MParameters *( void )
	{	return dynamic_cast< VS_H245H223AL2MParameters * >(choice);    }

 	VS_H245H223LogicalChannelParameters_AdaptationLayerType::operator VS_H245H223AL3MParameters *( void )
	{	return dynamic_cast< VS_H245H223AL3MParameters * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223LogicalChannelParameters_AdaptationLayerType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_al1Framed :  dprint4("choice:  VS_AsnNull  ");return;
		case e_al1NotFramed :  dprint4("choice:  VS_AsnNull  ");return;
		case e_al2WithoutSequenceNumbers :  dprint4("choice:  VS_AsnNull  ");return;
		case e_al2WithSequenceNumbers :  dprint4("choice:  VS_AsnNull  ");return;
		case e_al3 :  dprint4("choice: VS_H245H223LogicalChannelParameters_AdaptationLayerType_Al3	 ");return;
		case e_al1M :  dprint4("choice: VS_H245H223AL1MParameters ");return;
		case e_al2M :  dprint4("choice: VS_H245H223AL2MParameters ");return;
		case e_al3M :  dprint4("choice: VS_H245H223AL3MParameters ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223LogicalChannelParameters /////////////////////////
 	 VS_H245H223LogicalChannelParameters :: VS_H245H223LogicalChannelParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&adaptationLayerType,0);
		ref[1].Set(&segmentableFlag,0);
	}
	void VS_H245H223LogicalChannelParameters::operator=(const VS_H245H223LogicalChannelParameters& src)
	{

		O_CC(filled);
		O_C(adaptationLayerType);
		O_C(segmentableFlag);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H222LogicalChannelParameters /////////////////////////
 	 VS_H245H222LogicalChannelParameters :: VS_H245H222LogicalChannelParameters( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&resourceID,0);
		ref[1].Set(&subChannelID,0);
		ref[2].Set(&pcr_pid,1);
		ref[3].Set(&programDescriptors,1);
		ref[4].Set(&streamDescriptors,1);
	}
	void VS_H245H222LogicalChannelParameters::operator=(const VS_H245H222LogicalChannelParameters& src)
	{

		O_CC(filled);
		O_C(resourceID);
		O_C(subChannelID);
		O_C(pcr_pid);
		O_C(programDescriptors);
		O_C(streamDescriptors);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexedStreamParameter /////////////////////////
 	 VS_H245MultiplexedStreamParameter :: VS_H245MultiplexedStreamParameter( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&multiplexFormat,0);
		ref[1].Set(&controlOnMuxStream,0);
	}
	void VS_H245MultiplexedStreamParameter::operator=(const VS_H245MultiplexedStreamParameter& src)
	{

		O_CC(filled);
		O_C(multiplexFormat);
		O_C(controlOnMuxStream);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H235Media_MediaType /////////////////////////
 	 VS_H245H235Media_MediaType::VS_H245H235Media_MediaType( void )
	:VS_AsnChoice(4 , 8 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H235Media_MediaType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_videoData : return DecodeChoice( buffer , new VS_H245VideoCapability);
		case e_audioData : return DecodeChoice( buffer , new VS_H245AudioCapability);
		case e_data : return DecodeChoice( buffer , new VS_H245DataApplicationCapability);
		case e_redundancyEncoding : return DecodeChoice( buffer , new VS_H245RedundancyEncoding);
		case e_multiplePayloadStream : return DecodeChoice( buffer , new VS_H245MultiplePayloadStream);
		case e_depFfec : return DecodeChoice( buffer , new VS_H245DepFECData);
		case e_fec : return DecodeChoice( buffer , new VS_H245FECData);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245H235Media_MediaType::operator=(const VS_H245H235Media_MediaType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_videoData : CopyChoice< VS_H245VideoCapability >(src,*this); return;
		case e_audioData : CopyChoice< VS_H245AudioCapability >(src,*this); return;
		case e_data : CopyChoice< VS_H245DataApplicationCapability >(src,*this); return;
		case e_redundancyEncoding : CopyChoice< VS_H245RedundancyEncoding >(src,*this); return;
		case e_multiplePayloadStream : CopyChoice< VS_H245MultiplePayloadStream >(src,*this); return;
		case e_depFfec : CopyChoice< VS_H245DepFECData >(src,*this); return;
		case e_fec : CopyChoice< VS_H245FECData >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245H235Media_MediaType::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245H235Media_MediaType::operator VS_H245VideoCapability *( void )
	{	return dynamic_cast< VS_H245VideoCapability * >(choice);    }

 	VS_H245H235Media_MediaType::operator VS_H245AudioCapability *( void )
	{	return dynamic_cast< VS_H245AudioCapability * >(choice);    }

 	VS_H245H235Media_MediaType::operator VS_H245DataApplicationCapability *( void )
	{	return dynamic_cast< VS_H245DataApplicationCapability * >(choice);    }

 	VS_H245H235Media_MediaType::operator VS_H245RedundancyEncoding *( void )
	{	return dynamic_cast< VS_H245RedundancyEncoding * >(choice);    }

 	VS_H245H235Media_MediaType::operator VS_H245MultiplePayloadStream *( void )
	{	return dynamic_cast< VS_H245MultiplePayloadStream * >(choice);    }

 	VS_H245H235Media_MediaType::operator VS_H245DepFECData *( void )
	{	return dynamic_cast< VS_H245DepFECData * >(choice);    }

 	VS_H245H235Media_MediaType::operator VS_H245FECData *( void )
	{	return dynamic_cast< VS_H245FECData * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H235Media_MediaType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_videoData :  dprint4("choice: VS_H245VideoCapability ");return;
		case e_audioData :  dprint4("choice: VS_H245AudioCapability ");return;
		case e_data :  dprint4("choice: VS_H245DataApplicationCapability ");return;
		case e_redundancyEncoding :  dprint4("choice: VS_H245RedundancyEncoding ");return;
		case e_multiplePayloadStream :  dprint4("choice: VS_H245MultiplePayloadStream ");return;
		case e_depFfec :  dprint4("choice: VS_H245DepFECData ");return;
		case e_fec :  dprint4("choice: VS_H245FECData ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H235Media /////////////////////////
 	 VS_H245H235Media :: VS_H245H235Media( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&encryptionAuthenticationAndIntegrity,0);
		ref[1].Set(&mediaType,0);
	}
	void VS_H245H235Media::operator=(const VS_H245H235Media& src)
	{

		O_CC(filled);
		O_C(encryptionAuthenticationAndIntegrity);
		O_C(mediaType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DataType /////////////////////////
 	 VS_H245DataType::VS_H245DataType( void )
	:VS_AsnChoice(6 , 13 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DataType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_nullData : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_videoData : return DecodeChoice( buffer , new VS_H245VideoCapability);
		case e_audioData : return DecodeChoice( buffer , new VS_H245AudioCapability);
		case e_data : return DecodeChoice( buffer , new VS_H245DataApplicationCapability);
		case e_encryptionData : return DecodeChoice( buffer , new VS_H245EncryptionMode);
		case e_h235Control : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_h235Media : return DecodeChoice( buffer , new VS_H245H235Media);
		case e_multiplexedStream : return DecodeChoice( buffer , new VS_H245MultiplexedStreamParameter);
		case e_redundancyEncoding : return DecodeChoice( buffer , new VS_H245RedundancyEncoding);
		case e_multiplePayloadStream : return DecodeChoice( buffer , new VS_H245MultiplePayloadStream);
		case e_depFfec : return DecodeChoice( buffer , new VS_H245DepFECData);
		case e_fec : return DecodeChoice( buffer , new VS_H245FECData);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}
	void VS_H245DataType::operator=( VS_H245VideoCapability *vc )
	{	Clear();	choice = vc;	tag = e_videoData;		filled = true;		}
	// end of VS_H245DataType::operator= VS_H245VideoCapability*

	void VS_H245DataType::operator=( VS_H245AudioCapability *ac )
	{	Clear();	choice = ac;	tag = e_audioData;		filled = true;		}

	void VS_H245DataType::operator=(const VS_H245DataType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_nullData : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_videoData : CopyChoice< VS_H245VideoCapability >(src,*this); return;
		case e_audioData : CopyChoice< VS_H245AudioCapability >(src,*this); return;
		case e_data : CopyChoice< VS_H245DataApplicationCapability >(src,*this); return;
		case e_encryptionData : CopyChoice< VS_H245EncryptionMode >(src,*this); return;
		case e_h235Control : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_h235Media : CopyChoice< VS_H245H235Media >(src,*this); return;
		case e_multiplexedStream : CopyChoice< VS_H245MultiplexedStreamParameter >(src,*this); return;
		case e_redundancyEncoding : CopyChoice< VS_H245RedundancyEncoding >(src,*this); return;
		case e_multiplePayloadStream : CopyChoice< VS_H245MultiplePayloadStream >(src,*this); return;
		case e_depFfec : CopyChoice< VS_H245DepFECData >(src,*this); return;
		case e_fec : CopyChoice< VS_H245FECData >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245DataType::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245DataType::operator VS_H245VideoCapability *( void )
	{	return dynamic_cast< VS_H245VideoCapability * >(choice);    }

 	VS_H245DataType::operator VS_H245AudioCapability *( void )
	{	return dynamic_cast< VS_H245AudioCapability * >(choice);    }

 	VS_H245DataType::operator VS_H245DataApplicationCapability *( void )
	{	return dynamic_cast< VS_H245DataApplicationCapability * >(choice);    }

 	VS_H245DataType::operator VS_H245EncryptionMode *( void )
	{	return dynamic_cast< VS_H245EncryptionMode * >(choice);    }

 	VS_H245DataType::operator VS_H245H235Media *( void )
	{	return dynamic_cast< VS_H245H235Media * >(choice);    }

 	VS_H245DataType::operator VS_H245MultiplexedStreamParameter *( void )
	{	return dynamic_cast< VS_H245MultiplexedStreamParameter * >(choice);    }

 	VS_H245DataType::operator VS_H245RedundancyEncoding *( void )
	{	return dynamic_cast< VS_H245RedundancyEncoding * >(choice);    }

 	VS_H245DataType::operator VS_H245MultiplePayloadStream *( void )
	{	return dynamic_cast< VS_H245MultiplePayloadStream * >(choice);    }

 	VS_H245DataType::operator VS_H245DepFECData *( void )
	{	return dynamic_cast< VS_H245DepFECData * >(choice);    }

 	VS_H245DataType::operator VS_H245FECData *( void )
	{	return dynamic_cast< VS_H245FECData * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DataType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_nullData :  dprint4("choice:  VS_AsnNull  ");return;
		case e_videoData :  dprint4("choice: VS_H245VideoCapability ");return;
		case e_audioData :  dprint4("choice: VS_H245AudioCapability ");return;
		case e_data :  dprint4("choice: VS_H245DataApplicationCapability ");return;
		case e_encryptionData :  dprint4("choice: VS_H245EncryptionMode ");return;
		case e_h235Control :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_h235Media :  dprint4("choice: VS_H245H235Media ");return;
		case e_multiplexedStream :  dprint4("choice: VS_H245MultiplexedStreamParameter ");return;
		case e_redundancyEncoding :  dprint4("choice: VS_H245RedundancyEncoding ");return;
		case e_multiplePayloadStream :  dprint4("choice: VS_H245MultiplePayloadStream ");return;
		case e_depFfec :  dprint4("choice: VS_H245DepFECData ");return;
		case e_fec :  dprint4("choice: VS_H245FECData ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245V75Parameters /////////////////////////
 	 VS_H245V75Parameters :: VS_H245V75Parameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioHeaderPresent,0);
	}
	void VS_H245V75Parameters::operator=(const VS_H245V75Parameters& src)
	{

		O_CC(filled);
		O_C(audioHeaderPresent);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245Q2931Address_Address /////////////////////////
 	 VS_H245Q2931Address_Address::VS_H245Q2931Address_Address( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245Q2931Address_Address::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_internationalNumber : return DecodeChoice( buffer , new TemplNumericString<1,16,VS_Asn::FixedConstraint,0>  );
		case e_nsapAddress : return DecodeChoice( buffer , new TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245Q2931Address_Address::operator=(const VS_H245Q2931Address_Address & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_internationalNumber : CopyChoice<TemplNumericString<1,16,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_nsapAddress : CopyChoice<TemplOctetString<1,20,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245Q2931Address_Address::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_internationalNumber :  dprint4("choice: TemplNumericString<1,16,VS_Asn::FixedConstraint,0>  ");return;
		case e_nsapAddress :  dprint4("choice: TemplOctetString<1,20,VS_Asn::FixedConstraint,0>  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245Q2931Address /////////////////////////
 	 VS_H245Q2931Address :: VS_H245Q2931Address( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&address,0);
		ref[1].Set(&subaddress,1);
	}
	void VS_H245Q2931Address::operator=(const VS_H245Q2931Address& src)
	{

		O_CC(filled);
		O_C(address);
		O_C(subaddress);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NetworkAccessParameters_Distribution /////////////////////////
 	 VS_H245NetworkAccessParameters_Distribution::VS_H245NetworkAccessParameters_Distribution( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NetworkAccessParameters_Distribution::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unicast : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_multicast : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NetworkAccessParameters_Distribution::operator=(const VS_H245NetworkAccessParameters_Distribution & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unicast : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_multicast : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NetworkAccessParameters_Distribution::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unicast :  dprint4("choice:  VS_AsnNull  ");return;
		case e_multicast :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NetworkAccessParameters_NetworkAddress /////////////////////////
 	 VS_H245NetworkAccessParameters_NetworkAddress::VS_H245NetworkAccessParameters_NetworkAddress( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

	 /////////////////////////////////////////////////////////////////////////////////////////
	unsigned char VS_H245NetworkAccessParameters_NetworkAddress::e164Address_alphabet[]=
	{'0','1','2','3','4','5','6','7','8','9','#','*',',' };
	unsigned  VS_H245NetworkAccessParameters_NetworkAddress::e164Address_alphabet_size=sizeof(VS_H245NetworkAccessParameters_NetworkAddress::e164Address_alphabet);
	unsigned char  VS_H245NetworkAccessParameters_NetworkAddress::e164Address_inverse_table[256]={0};
	 const bool VS_H245NetworkAccessParameters_NetworkAddress::e164Address_flag_set_table =
	 VS_AsnRestrictedString::MakeInverseCodeTable(
		 VS_H245NetworkAccessParameters_NetworkAddress::e164Address_inverse_table,
		 VS_H245NetworkAccessParameters_NetworkAddress::e164Address_alphabet,
		 VS_H245NetworkAccessParameters_NetworkAddress::e164Address_alphabet_size );

	 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NetworkAccessParameters_NetworkAddress::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_q2931Address : return DecodeChoice( buffer , new VS_H245Q2931Address);
		case e_e164Address : return DecodeChoice(buffer , new TemplAlphabeticString<e164Address_alphabet, sizeof(e164Address_alphabet), e164Address_inverse_table, 1, 128, VS_Asn::FixedConstraint, 0>);
		//case e_e164Address : return DecodeChoice( buffer , new TemplAlphabeticString< e164Address_alphabet, e164Address_alphabet_size,e164Address_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>  );
		case e_localAreaAddress : return DecodeChoice( buffer , new VS_H245TransportAddress);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NetworkAccessParameters_NetworkAddress::operator=(const VS_H245NetworkAccessParameters_NetworkAddress & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_q2931Address : CopyChoice< VS_H245Q2931Address >(src,*this); return;
		//case e_e164Address : CopyChoice<TemplAlphabeticString< e164Address_alphabet, e164Address_alphabet_size,e164Address_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>   >(src,*this);  return;
		case e_e164Address : CopyChoice<TemplAlphabeticString<e164Address_alphabet, sizeof(e164Address_alphabet), e164Address_inverse_table, 1, 128, VS_Asn::FixedConstraint, 0>>(src, *this);  return;
		case e_localAreaAddress : CopyChoice< VS_H245TransportAddress >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245NetworkAccessParameters_NetworkAddress::operator VS_H245Q2931Address *( void )
	{	return dynamic_cast< VS_H245Q2931Address * >(choice);    }

 	VS_H245NetworkAccessParameters_NetworkAddress::operator VS_H245TransportAddress *( void )
	{	return dynamic_cast< VS_H245TransportAddress * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NetworkAccessParameters_NetworkAddress::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_q2931Address :  dprint4("choice: VS_H245Q2931Address ");return;
//		case e_e164Address :  dprint4("choice: TemplAlphabeticString< e164Address_alphabet, e164Address_alphabet_size,e164Address_inverse_table,0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
		case e_e164Address :  dprint4("choice: TemplAlphabeticString< e164Address_alphabet, e164Address_alphabet_size,e164Address_inverse_table,1,128,VS_Asn::FixedConstraint,0>  ");return;
		case e_localAreaAddress :  dprint4("choice: VS_H245TransportAddress ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NetworkAccessParameters_T120SetupProcedure /////////////////////////
 	 VS_H245NetworkAccessParameters_T120SetupProcedure::VS_H245NetworkAccessParameters_T120SetupProcedure( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NetworkAccessParameters_T120SetupProcedure::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_originateCall : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_waitForCall : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_issueQuery : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245NetworkAccessParameters_T120SetupProcedure::operator=(const VS_H245NetworkAccessParameters_T120SetupProcedure & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_originateCall : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_waitForCall : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_issueQuery : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NetworkAccessParameters_T120SetupProcedure::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_originateCall :  dprint4("choice:  VS_AsnNull  ");return;
		case e_waitForCall :  dprint4("choice:  VS_AsnNull  ");return;
		case e_issueQuery :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NetworkAccessParameters /////////////////////////
 	 VS_H245NetworkAccessParameters :: VS_H245NetworkAccessParameters( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&distribution,1);
		ref[1].Set(&networkAddress,0);
		ref[2].Set(&associateConference,0);
		ref[3].Set(&externalReference,1);
		e_ref[0].Set(&t120SetupProcedure,1);
	}
	void VS_H245NetworkAccessParameters::operator=(const VS_H245NetworkAccessParameters& src)
	{

		O_CC(filled);
		O_C(distribution);
		O_C(networkAddress);
		O_C(associateConference);
		O_C(externalReference);
		O_C(t120SetupProcedure);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters /////////////////////////
 	 VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters( void )
	:VS_AsnChoice(3 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_h222LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245H222LogicalChannelParameters);
		case e_h223LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245H223LogicalChannelParameters);
		case e_v76LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245V76LogicalChannelParameters);
		case e_h2250LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245H2250LogicalChannelParameters);
		case e_none : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::operator=(const VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_h222LogicalChannelParameters : CopyChoice< VS_H245H222LogicalChannelParameters >(src,*this); return;
		case e_h223LogicalChannelParameters : CopyChoice< VS_H245H223LogicalChannelParameters >(src,*this); return;
		case e_v76LogicalChannelParameters : CopyChoice< VS_H245V76LogicalChannelParameters >(src,*this); return;
		case e_h2250LogicalChannelParameters : CopyChoice< VS_H245H2250LogicalChannelParameters >(src,*this); return;
		case e_none : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

	void VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::operator=( VS_H245H2250LogicalChannelParameters *h2250lcp )
	{	Clear();	choice = h2250lcp;	tag = e_h2250LogicalChannelParameters;	filled = true;		}


 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::operator VS_H245H222LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245H222LogicalChannelParameters * >(choice);    }

 	VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::operator VS_H245H223LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245H223LogicalChannelParameters * >(choice);    }

 	VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::operator VS_H245V76LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245V76LogicalChannelParameters * >(choice);    }

 	VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::operator VS_H245H2250LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245H2250LogicalChannelParameters * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters_MultiplexParameters::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_h222LogicalChannelParameters :  dprint4("choice: VS_H245H222LogicalChannelParameters ");return;
		case e_h223LogicalChannelParameters :  dprint4("choice: VS_H245H223LogicalChannelParameters ");return;
		case e_v76LogicalChannelParameters :  dprint4("choice: VS_H245V76LogicalChannelParameters ");return;
		case e_h2250LogicalChannelParameters :  dprint4("choice: VS_H245H2250LogicalChannelParameters ");return;
		case e_none :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters /////////////////////////
 	 VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters :: VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&portNumber,1);
		ref[1].Set(&dataType,0);
		ref[2].Set(&multiplexParameters,0);
		e_ref[0].Set(&forwardLogicalChannelDependency,1);
		e_ref[1].Set(&replacementFor,1);
	}
	void VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters::operator=(const VS_H245OpenLogicalChannel_ForwardLogicalChannelParameters& src)
	{

		O_CC(filled);
		O_C(portNumber);
		O_C(dataType);
		O_C(multiplexParameters);
		O_C(forwardLogicalChannelDependency);
		O_C(replacementFor);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters /////////////////////////
 	 VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters::VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters( void )
	:VS_AsnChoice(2 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_h223LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245H223LogicalChannelParameters);
		case e_v76LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245V76LogicalChannelParameters);
		case e_h2250LogicalChannelParameters : return DecodeChoice( buffer , new VS_H245H2250LogicalChannelParameters);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters::operator=(const VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_h223LogicalChannelParameters : CopyChoice< VS_H245H223LogicalChannelParameters >(src,*this); return;
		case e_v76LogicalChannelParameters : CopyChoice< VS_H245V76LogicalChannelParameters >(src,*this); return;
		case e_h2250LogicalChannelParameters : CopyChoice< VS_H245H2250LogicalChannelParameters >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters::operator VS_H245H223LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245H223LogicalChannelParameters * >(choice);    }

 	VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters::operator VS_H245V76LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245V76LogicalChannelParameters * >(choice);    }

 	VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters::operator VS_H245H2250LogicalChannelParameters *( void )
	{	return dynamic_cast< VS_H245H2250LogicalChannelParameters * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters_MultiplexParameters::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_h223LogicalChannelParameters :  dprint4("choice: VS_H245H223LogicalChannelParameters ");return;
		case e_v76LogicalChannelParameters :  dprint4("choice: VS_H245V76LogicalChannelParameters ");return;
		case e_h2250LogicalChannelParameters :  dprint4("choice: VS_H245H2250LogicalChannelParameters ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters /////////////////////////
 	 VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters :: VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&dataType,0);
		ref[1].Set(&multiplexParameters,1);
		e_ref[0].Set(&reverseLogicalChannelDependency,1);
		e_ref[1].Set(&replacementFor,1);
	}
	void VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters::operator=(const VS_H245OpenLogicalChannel_ReverseLogicalChannelParameters& src)
	{

		O_CC(filled);
		O_C(dataType);
		O_C(multiplexParameters);
		O_C(reverseLogicalChannelDependency);
		O_C(replacementFor);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245OpenLogicalChannel /////////////////////////
 	 VS_H245OpenLogicalChannel :: VS_H245OpenLogicalChannel( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&forwardLogicalChannelNumber,0);
		ref[1].Set(&forwardLogicalChannelParameters,0);
		ref[2].Set(&reverseLogicalChannelParameters,1);
		e_ref[0].Set(&separateStack,1);
		e_ref[1].Set(&encryptionSync,1);
	}
	void VS_H245OpenLogicalChannel::operator=(const VS_H245OpenLogicalChannel& src)
	{

		O_CC(filled);
		O_C(forwardLogicalChannelNumber);
		O_C(forwardLogicalChannelParameters);
		O_C(reverseLogicalChannelParameters);
		O_C(separateStack);
		O_C(encryptionSync);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245FECCapability_Rfc2733Format /////////////////////////
 	 VS_H245FECCapability_Rfc2733Format::VS_H245FECCapability_Rfc2733Format( void )
	:VS_AsnChoice(3 , 3 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245FECCapability_Rfc2733Format::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_rfc2733rfc2198 : return DecodeChoice( buffer , new VS_H245MaxRedundancy);
		case e_rfc2733sameport : return DecodeChoice( buffer , new VS_H245MaxRedundancy);
		case e_rfc2733diffport : return DecodeChoice( buffer , new VS_H245MaxRedundancy);
		default: return false;
		}

	}

	void VS_H245FECCapability_Rfc2733Format::operator=(const VS_H245FECCapability_Rfc2733Format & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_rfc2733rfc2198 : CopyChoice< VS_H245MaxRedundancy >(src,*this); return;
		case e_rfc2733sameport : CopyChoice< VS_H245MaxRedundancy >(src,*this); return;
		case e_rfc2733diffport : CopyChoice< VS_H245MaxRedundancy >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245FECCapability_Rfc2733Format::operator VS_H245MaxRedundancy *( void )
	{	return dynamic_cast< VS_H245MaxRedundancy * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245FECCapability_Rfc2733Format::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_rfc2733rfc2198 :  dprint4("choice: VS_H245MaxRedundancy ");return;
		case e_rfc2733sameport :  dprint4("choice: VS_H245MaxRedundancy ");return;
		case e_rfc2733diffport :  dprint4("choice: VS_H245MaxRedundancy ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245FECCapability /////////////////////////
 	 VS_H245FECCapability :: VS_H245FECCapability( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&protectedCapability,0);
		ref[1].Set(&fecScheme,1);
		ref[2].Set(&rfc2733Format,1);
	}
	void VS_H245FECCapability::operator=(const VS_H245FECCapability& src)
	{

		O_CC(filled);
		O_C(protectedCapability);
		O_C(fecScheme);
		O_C(rfc2733Format);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECCapability_Rfc2733_SeparateStream /////////////////////////
 	 VS_H245DepFECCapability_Rfc2733_SeparateStream :: VS_H245DepFECCapability_Rfc2733_SeparateStream( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&separatePort,0);
		ref[1].Set(&samePort,0);
	}
	void VS_H245DepFECCapability_Rfc2733_SeparateStream::operator=(const VS_H245DepFECCapability_Rfc2733_SeparateStream& src)
	{

		O_CC(filled);
		O_C(separatePort);
		O_C(samePort);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECCapability_Rfc2733 /////////////////////////
 	 VS_H245DepFECCapability_Rfc2733 :: VS_H245DepFECCapability_Rfc2733( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&redundancyEncoding,0);
		ref[1].Set(&separateStream,0);
	}
	void VS_H245DepFECCapability_Rfc2733::operator=(const VS_H245DepFECCapability_Rfc2733& src)
	{

		O_CC(filled);
		O_C(redundancyEncoding);
		O_C(separateStream);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DepFECCapability /////////////////////////
 	 VS_H245DepFECCapability::VS_H245DepFECCapability( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DepFECCapability::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_rfc2733 : return DecodeChoice( buffer , new VS_H245DepFECCapability_Rfc2733	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DepFECCapability::operator=(const VS_H245DepFECCapability & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_rfc2733 : CopyChoice<VS_H245DepFECCapability_Rfc2733	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DepFECCapability::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_rfc2733 :  dprint4("choice: VS_H245DepFECCapability_Rfc2733	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultiplePayloadStreamCapability /////////////////////////
 	 VS_H245MultiplePayloadStreamCapability :: VS_H245MultiplePayloadStreamCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&capabilities,0);
	}
	void VS_H245MultiplePayloadStreamCapability::operator=(const VS_H245MultiplePayloadStreamCapability& src)
	{

		O_CC(filled);
		O_C(capabilities);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NoPTAudioToneCapability /////////////////////////
 	 VS_H245NoPTAudioToneCapability :: VS_H245NoPTAudioToneCapability( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr , extension_root , 1 )
	{
	}
	void VS_H245NoPTAudioToneCapability::operator=(const VS_H245NoPTAudioToneCapability& src)
	{

		O_CC(filled);
		O_CP(e_ref);  O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NoPTAudioTelephonyEventCapability /////////////////////////
 	 VS_H245NoPTAudioTelephonyEventCapability :: VS_H245NoPTAudioTelephonyEventCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioTelephoneEvent,0);
	}
	void VS_H245NoPTAudioTelephonyEventCapability::operator=(const VS_H245NoPTAudioTelephonyEventCapability& src)
	{

		O_CC(filled);
		O_C(audioTelephoneEvent);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245AudioToneCapability /////////////////////////
 	 VS_H245AudioToneCapability :: VS_H245AudioToneCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&dynamicRTPPayloadType,0);
	}
	void VS_H245AudioToneCapability::operator=(const VS_H245AudioToneCapability& src)
	{

		O_CC(filled);
		O_C(dynamicRTPPayloadType);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245AudioTelephonyEventCapability /////////////////////////
 	 VS_H245AudioTelephonyEventCapability :: VS_H245AudioTelephonyEventCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&dynamicRTPPayloadType,0);
		ref[1].Set(&audioTelephoneEvent,0);
	}
	void VS_H245AudioTelephonyEventCapability::operator=(const VS_H245AudioTelephonyEventCapability& src)
	{

		O_CC(filled);
		O_C(dynamicRTPPayloadType);
		O_C(audioTelephoneEvent);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexFormat /////////////////////////
 	 VS_H245MultiplexFormat::VS_H245MultiplexFormat( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultiplexFormat::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_h222Capability : return DecodeChoice( buffer , new VS_H245H222Capability);
		case e_h223Capability : return DecodeChoice( buffer , new VS_H245H223Capability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultiplexFormat::operator=(const VS_H245MultiplexFormat & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_h222Capability : CopyChoice< VS_H245H222Capability >(src,*this); return;
		case e_h223Capability : CopyChoice< VS_H245H223Capability >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MultiplexFormat::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245MultiplexFormat::operator VS_H245H222Capability *( void )
	{	return dynamic_cast< VS_H245H222Capability * >(choice);    }

 	VS_H245MultiplexFormat::operator VS_H245H223Capability *( void )
	{	return dynamic_cast< VS_H245H223Capability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultiplexFormat::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_h222Capability :  dprint4("choice: VS_H245H222Capability ");return;
		case e_h223Capability :  dprint4("choice: VS_H245H223Capability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultiplexedStreamCapability /////////////////////////
 	 VS_H245MultiplexedStreamCapability :: VS_H245MultiplexedStreamCapability( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&multiplexFormat,0);
		ref[1].Set(&controlOnMuxStream,0);
		ref[2].Set(&capabilityOnMuxStream,1);
	}
	void VS_H245MultiplexedStreamCapability::operator=(const VS_H245MultiplexedStreamCapability& src)
	{

		O_CC(filled);
		O_C(multiplexFormat);
		O_C(controlOnMuxStream);
		O_C(capabilityOnMuxStream);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ParameterValue /////////////////////////
 	 VS_H245ParameterValue::VS_H245ParameterValue( void )
	:VS_AsnChoice(8 , 8 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ParameterValue::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_logical : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_booleanArray : return DecodeChoice( buffer , new TemplInteger<0,255,VS_Asn::FixedConstraint,0>  );
		case e_unsignedMin : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		case e_unsignedMax : return DecodeChoice( buffer , new TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  );
		case e_unsigned32Min : return DecodeChoice( buffer , new TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  );
		case e_unsigned32Max : return DecodeChoice( buffer , new TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  );
		case e_octetString : return DecodeChoice( buffer , new TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  );
		case e_genericParameter : return DecodeChoice( buffer , new VS_H245GenericParameter);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ParameterValue::operator=(const VS_H245ParameterValue & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_logical : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_booleanArray : CopyChoice<TemplInteger<0,255,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_unsignedMin : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_unsignedMax : CopyChoice<TemplInteger<0,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_unsigned32Min : CopyChoice<TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_unsigned32Max : CopyChoice<TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_octetString : CopyChoice<TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>   >(src,*this);  return;
		case e_genericParameter : CopyChoice< VS_H245GenericParameter >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245ParameterValue::operator VS_H245GenericParameter *( void )
	{	return dynamic_cast< VS_H245GenericParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ParameterValue::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_logical :  dprint4("choice:  VS_AsnNull  ");return;
		case e_booleanArray :  dprint4("choice: TemplInteger<0,255,VS_Asn::FixedConstraint,0>  ");return;
		case e_unsignedMin :  dprint4("choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_unsignedMax :  dprint4("choice: TemplInteger<0,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_unsigned32Min :  dprint4("choice: TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  ");return;
		case e_unsigned32Max :  dprint4("choice: TemplInteger<0,4294967295,VS_Asn::FixedConstraint,0>  ");return;
		case e_octetString :  dprint4("choice: TemplOctetString< 0,INT_MAX,VS_Asn::Unconstrained,false>  ");return;
		case e_genericParameter :  dprint4("choice: VS_H245GenericParameter ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245ParameterIdentifier /////////////////////////
 	 VS_H245ParameterIdentifier::VS_H245ParameterIdentifier( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ParameterIdentifier::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_standard : return DecodeChoice( buffer , new TemplInteger<0,127,VS_Asn::FixedConstraint,0>  );
		case e_h221NonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_uuid : return DecodeChoice( buffer , new TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  );
		case e_domainBased : return DecodeChoice( buffer , new TemplIA5String<1,64,VS_Asn::FixedConstraint,0>  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ParameterIdentifier::operator=(const VS_H245ParameterIdentifier & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_standard : CopyChoice<TemplInteger<0,127,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_h221NonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_uuid : CopyChoice<TemplOctetString<16,16,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_domainBased : CopyChoice<TemplIA5String<1,64,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245ParameterIdentifier::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ParameterIdentifier::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_standard :  dprint4("choice: TemplInteger<0,127,VS_Asn::FixedConstraint,0>  ");return;
		case e_h221NonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_uuid :  dprint4("choice: TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  ");return;
		case e_domainBased :  dprint4("choice: TemplIA5String<1,64,VS_Asn::FixedConstraint,0>  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245GenericParameter /////////////////////////
 	 VS_H245GenericParameter :: VS_H245GenericParameter( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&parameterIdentifier,0);
		ref[1].Set(&parameterValue,0);
		ref[2].Set(&supersedes,1);
	}
	void VS_H245GenericParameter::operator=(const VS_H245GenericParameter& src)
	{

		O_CC(filled);
		O_C(parameterIdentifier);
		O_C(parameterValue);
		O_C(supersedes);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CapabilityIdentifier /////////////////////////
 	 VS_H245CapabilityIdentifier::VS_H245CapabilityIdentifier( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CapabilityIdentifier::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_standard : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		case e_h221NonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_uuid : return DecodeChoice( buffer , new TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  );
		case e_domainBased : return DecodeChoice( buffer , new TemplIA5String<1,64,VS_Asn::FixedConstraint,0>  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245CapabilityIdentifier::operator=(const VS_H245CapabilityIdentifier & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_standard : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		case e_h221NonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_uuid : CopyChoice<TemplOctetString<16,16,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_domainBased : CopyChoice<TemplIA5String<1,64,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245CapabilityIdentifier::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CapabilityIdentifier::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_standard :  dprint4("choice:  VS_AsnObjectId  ");return;
		case e_h221NonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_uuid :  dprint4("choice: TemplOctetString<16,16,VS_Asn::FixedConstraint,0>  ");return;
		case e_domainBased :  dprint4("choice: TemplIA5String<1,64,VS_Asn::FixedConstraint,0>  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245GenericCapability /////////////////////////
 	 VS_H245GenericCapability :: VS_H245GenericCapability( void )
	:VS_AsnSequence(5 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&capabilityIdentifier,0);
		ref[1].Set(&maxBitRate,1);
		ref[2].Set(&collapsing,1);
		ref[3].Set(&nonCollapsing,1);
		ref[4].Set(&nonCollapsingRaw,1);
		ref[5].Set(&transport,1);
	}
	void VS_H245GenericCapability::operator=(const VS_H245GenericCapability& src)
	{

		O_CC(filled);
		O_C(capabilityIdentifier);
		O_C(maxBitRate);
		O_C(collapsing);
		O_C(nonCollapsing);
		O_C(nonCollapsingRaw);
		O_C(transport);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ConferenceCapability /////////////////////////
 	 VS_H245ConferenceCapability :: VS_H245ConferenceCapability( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&chairControlCapability,0);
		e_ref[0].Set(&videoIndicateMixingCapability,0);
		e_ref[1].Set(&multipointVisualizationCapability,1);
	}
	void VS_H245ConferenceCapability::operator=(const VS_H245ConferenceCapability& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(chairControlCapability);
		O_C(videoIndicateMixingCapability);
		O_C(multipointVisualizationCapability);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245UserInputCapability /////////////////////////
 	 VS_H245UserInputCapability::VS_H245UserInputCapability( void )
	:VS_AsnChoice(6 , 11 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245UserInputCapability::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_basicString : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_iA5String : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_generalString : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_dtmf : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_hookflash : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_extendedAlphanumeric : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_encryptedBasicString : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_encryptedIA5String : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_encryptedGeneralString : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_secureDTMF : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245UserInputCapability::operator=(const VS_H245UserInputCapability & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_basicString : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_iA5String : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_generalString : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_dtmf : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_hookflash : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_extendedAlphanumeric : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_encryptedBasicString : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_encryptedIA5String : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_encryptedGeneralString : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_secureDTMF : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245UserInputCapability::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245UserInputCapability::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_basicString :  dprint4("choice:  VS_AsnNull  ");return;
		case e_iA5String :  dprint4("choice:  VS_AsnNull  ");return;
		case e_generalString :  dprint4("choice:  VS_AsnNull  ");return;
		case e_dtmf :  dprint4("choice:  VS_AsnNull  ");return;
		case e_hookflash :  dprint4("choice:  VS_AsnNull  ");return;
		case e_extendedAlphanumeric :  dprint4("choice:  VS_AsnNull  ");return;
		case e_encryptedBasicString :  dprint4("choice:  VS_AsnNull  ");return;
		case e_encryptedIA5String :  dprint4("choice:  VS_AsnNull  ");return;
		case e_encryptedGeneralString :  dprint4("choice:  VS_AsnNull  ");return;
		case e_secureDTMF :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IntegrityCapability /////////////////////////
 	 VS_H245IntegrityCapability :: VS_H245IntegrityCapability( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandard,1);
	}
	void VS_H245IntegrityCapability::operator=(const VS_H245IntegrityCapability& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245AuthenticationCapability /////////////////////////
 	 VS_H245AuthenticationCapability :: VS_H245AuthenticationCapability( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&nonStandard,1);
		e_ref[0].Set(&antiSpamAlgorithm,1);
	}
	void VS_H245AuthenticationCapability::operator=(const VS_H245AuthenticationCapability& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_C(antiSpamAlgorithm);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MediaEncryptionAlgorithm /////////////////////////
 	 VS_H245MediaEncryptionAlgorithm::VS_H245MediaEncryptionAlgorithm( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MediaEncryptionAlgorithm::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_algorithm : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MediaEncryptionAlgorithm::operator=(const VS_H245MediaEncryptionAlgorithm & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_algorithm : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MediaEncryptionAlgorithm::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MediaEncryptionAlgorithm::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_algorithm :  dprint4("choice:  VS_AsnObjectId  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245EncryptionAuthenticationAndIntegrity /////////////////////////
 	 VS_H245EncryptionAuthenticationAndIntegrity :: VS_H245EncryptionAuthenticationAndIntegrity( void )
	:VS_AsnSequence(3 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&encryptionCapability,1);
		ref[1].Set(&authenticationCapability,1);
		ref[2].Set(&integrityCapability,1);
		e_ref[0].Set(&genericH235SecurityCapability,1);
	}
	void VS_H245EncryptionAuthenticationAndIntegrity::operator=(const VS_H245EncryptionAuthenticationAndIntegrity& src)
	{

		O_CC(filled);
		O_C(encryptionCapability);
		O_C(authenticationCapability);
		O_C(integrityCapability);
		O_C(genericH235SecurityCapability);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245T38FaxTcpOptions /////////////////////////
 	 VS_H245T38FaxTcpOptions :: VS_H245T38FaxTcpOptions( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&t38TCPBidirectionalMode,0);
	}
	void VS_H245T38FaxTcpOptions::operator=(const VS_H245T38FaxTcpOptions& src)
	{

		O_CC(filled);
		O_C(t38TCPBidirectionalMode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245T38FaxUdpOptions_T38FaxUdpEC /////////////////////////
 	 VS_H245T38FaxUdpOptions_T38FaxUdpEC::VS_H245T38FaxUdpOptions_T38FaxUdpEC( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245T38FaxUdpOptions_T38FaxUdpEC::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_t38UDPFEC : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_t38UDPRedundancy : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245T38FaxUdpOptions_T38FaxUdpEC::operator=(const VS_H245T38FaxUdpOptions_T38FaxUdpEC & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_t38UDPFEC : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_t38UDPRedundancy : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245T38FaxUdpOptions_T38FaxUdpEC::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_t38UDPFEC :  dprint4("choice:  VS_AsnNull  ");return;
		case e_t38UDPRedundancy :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245T38FaxUdpOptions /////////////////////////
 	 VS_H245T38FaxUdpOptions :: VS_H245T38FaxUdpOptions( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&t38FaxMaxBuffer,1);
		ref[1].Set(&t38FaxMaxDatagram,1);
		ref[2].Set(&t38FaxUdpEC,0);
	}
	void VS_H245T38FaxUdpOptions::operator=(const VS_H245T38FaxUdpOptions& src)
	{

		O_CC(filled);
		O_C(t38FaxMaxBuffer);
		O_C(t38FaxMaxDatagram);
		O_C(t38FaxUdpEC);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245T38FaxRateManagement /////////////////////////
 	 VS_H245T38FaxRateManagement::VS_H245T38FaxRateManagement( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245T38FaxRateManagement::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_localTCF : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transferredTCF : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245T38FaxRateManagement::operator=(const VS_H245T38FaxRateManagement & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_localTCF : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transferredTCF : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245T38FaxRateManagement::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_localTCF :  dprint4("choice:  VS_AsnNull  ");return;
		case e_transferredTCF :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245T38FaxProfile /////////////////////////
 	 VS_H245T38FaxProfile :: VS_H245T38FaxProfile( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&fillBitRemoval,0);
		ref[1].Set(&transcodingJBIG,0);
		ref[2].Set(&transcodingMMR,0);
		e_ref[0].Set(&version,0);
		e_ref[1].Set(&t38FaxRateManagement,0);
		e_ref[2].Set(&t38FaxUdpOptions,1);
		e_ref[3].Set(&t38FaxTcpOptions,1);
	}
	void VS_H245T38FaxProfile::operator=(const VS_H245T38FaxProfile& src)
	{

		O_CC(filled);
		O_C(fillBitRemoval);
		O_C(transcodingJBIG);
		O_C(transcodingMMR);
		O_C(version);
		O_C(t38FaxRateManagement);
		O_C(t38FaxUdpOptions);
		O_C(t38FaxTcpOptions);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245T84Profile_T84Restricted /////////////////////////
 	 VS_H245T84Profile_T84Restricted :: VS_H245T84Profile_T84Restricted( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&qcif,0);
		ref[1].Set(&cif,0);
		ref[2].Set(&ccir601Seq,0);
		ref[3].Set(&ccir601Prog,0);
		ref[4].Set(&hdtvSeq,0);
		ref[5].Set(&hdtvProg,0);
		ref[6].Set(&g3FacsMH200x100,0);
		ref[7].Set(&g3FacsMH200x200,0);
		ref[8].Set(&g4FacsMMR200x100,0);
		ref[9].Set(&g4FacsMMR200x200,0);
		ref[10].Set(&jbig200x200Seq,0);
		ref[11].Set(&jbig200x200Prog,0);
		ref[12].Set(&jbig300x300Seq,0);
		ref[13].Set(&jbig300x300Prog,0);
		ref[14].Set(&digPhotoLow,0);
		ref[15].Set(&digPhotoMedSeq,0);
		ref[16].Set(&digPhotoMedProg,0);
		ref[17].Set(&digPhotoHighSeq,0);
		ref[18].Set(&digPhotoHighProg,0);
	}
	void VS_H245T84Profile_T84Restricted::operator=(const VS_H245T84Profile_T84Restricted& src)
	{

		O_CC(filled);
		O_C(qcif);
		O_C(cif);
		O_C(ccir601Seq);
		O_C(ccir601Prog);
		O_C(hdtvSeq);
		O_C(hdtvProg);
		O_C(g3FacsMH200x100);
		O_C(g3FacsMH200x200);
		O_C(g4FacsMMR200x100);
		O_C(g4FacsMMR200x200);
		O_C(jbig200x200Seq);
		O_C(jbig200x200Prog);
		O_C(jbig300x300Seq);
		O_C(jbig300x300Prog);
		O_C(digPhotoLow);
		O_C(digPhotoMedSeq);
		O_C(digPhotoMedProg);
		O_C(digPhotoHighSeq);
		O_C(digPhotoHighProg);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245T84Profile /////////////////////////
 	 VS_H245T84Profile::VS_H245T84Profile( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245T84Profile::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_t84Unrestricted : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_t84Restricted : return DecodeChoice( buffer , new VS_H245T84Profile_T84Restricted	 );
		default: return false;
		}

	}

	void VS_H245T84Profile::operator=(const VS_H245T84Profile & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_t84Unrestricted : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_t84Restricted : CopyChoice<VS_H245T84Profile_T84Restricted	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245T84Profile::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_t84Unrestricted :  dprint4("choice:  VS_AsnNull  ");return;
		case e_t84Restricted :  dprint4("choice: VS_H245T84Profile_T84Restricted	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245V42bis /////////////////////////
 	 VS_H245V42bis :: VS_H245V42bis( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&numberOfCodewords,0);
		ref[1].Set(&maximumStringLength,0);
	}
	void VS_H245V42bis::operator=(const VS_H245V42bis& src)
	{

		O_CC(filled);
		O_C(numberOfCodewords);
		O_C(maximumStringLength);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CompressionType /////////////////////////
 	 VS_H245CompressionType::VS_H245CompressionType( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CompressionType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_v42bis : return DecodeChoice( buffer , new VS_H245V42bis);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245CompressionType::operator=(const VS_H245CompressionType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_v42bis : CopyChoice< VS_H245V42bis >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245CompressionType::operator VS_H245V42bis *( void )
	{	return dynamic_cast< VS_H245V42bis * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CompressionType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_v42bis :  dprint4("choice: VS_H245V42bis ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DataProtocolCapability_V76wCompression /////////////////////////
 	 VS_H245DataProtocolCapability_V76wCompression::VS_H245DataProtocolCapability_V76wCompression( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DataProtocolCapability_V76wCompression::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_transmitCompression : return DecodeChoice( buffer , new VS_H245CompressionType);
		case e_receiveCompression : return DecodeChoice( buffer , new VS_H245CompressionType);
		case e_transmitAndReceiveCompression : return DecodeChoice( buffer , new VS_H245CompressionType);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DataProtocolCapability_V76wCompression::operator=(const VS_H245DataProtocolCapability_V76wCompression & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_transmitCompression : CopyChoice< VS_H245CompressionType >(src,*this); return;
		case e_receiveCompression : CopyChoice< VS_H245CompressionType >(src,*this); return;
		case e_transmitAndReceiveCompression : CopyChoice< VS_H245CompressionType >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245DataProtocolCapability_V76wCompression::operator VS_H245CompressionType *( void )
	{	return dynamic_cast< VS_H245CompressionType * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DataProtocolCapability_V76wCompression::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_transmitCompression :  dprint4("choice: VS_H245CompressionType ");return;
		case e_receiveCompression :  dprint4("choice: VS_H245CompressionType ");return;
		case e_transmitAndReceiveCompression :  dprint4("choice: VS_H245CompressionType ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DataProtocolCapability /////////////////////////
 	 VS_H245DataProtocolCapability::VS_H245DataProtocolCapability( void )
	:VS_AsnChoice(7 , 14 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DataProtocolCapability::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_v14buffered : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_v42lapm : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_hdlcFrameTunnelling : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_h310SeparateVCStack : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_h310SingleVCStack : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_transparent : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_segmentationAndReassembly : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_hdlcFrameTunnelingwSAR : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_v120 : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_separateLANStack : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_v76wCompression : return DecodeChoice( buffer , new VS_H245DataProtocolCapability_V76wCompression	 );
		case e_tcp : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_udp : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DataProtocolCapability::operator=(const VS_H245DataProtocolCapability & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_v14buffered : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_v42lapm : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_hdlcFrameTunnelling : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_h310SeparateVCStack : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_h310SingleVCStack : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_transparent : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_segmentationAndReassembly : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_hdlcFrameTunnelingwSAR : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_v120 : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_separateLANStack : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_v76wCompression : CopyChoice<VS_H245DataProtocolCapability_V76wCompression	  >(src,*this);  return;
		case e_tcp : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_udp : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245DataProtocolCapability::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DataProtocolCapability::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_v14buffered :  dprint4("choice:  VS_AsnNull  ");return;
		case e_v42lapm :  dprint4("choice:  VS_AsnNull  ");return;
		case e_hdlcFrameTunnelling :  dprint4("choice:  VS_AsnNull  ");return;
		case e_h310SeparateVCStack :  dprint4("choice:  VS_AsnNull  ");return;
		case e_h310SingleVCStack :  dprint4("choice:  VS_AsnNull  ");return;
		case e_transparent :  dprint4("choice:  VS_AsnNull  ");return;
		case e_segmentationAndReassembly :  dprint4("choice:  VS_AsnNull  ");return;
		case e_hdlcFrameTunnelingwSAR :  dprint4("choice:  VS_AsnNull  ");return;
		case e_v120 :  dprint4("choice:  VS_AsnNull  ");return;
		case e_separateLANStack :  dprint4("choice:  VS_AsnNull  ");return;
		case e_v76wCompression :  dprint4("choice: VS_H245DataProtocolCapability_V76wCompression	 ");return;
		case e_tcp :  dprint4("choice:  VS_AsnNull  ");return;
		case e_udp :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DataApplicationCapability_Application_T84 /////////////////////////
 	 VS_H245DataApplicationCapability_Application_T84 :: VS_H245DataApplicationCapability_Application_T84( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&t84Protocol,0);
		ref[1].Set(&t84Profile,0);
	}
	void VS_H245DataApplicationCapability_Application_T84::operator=(const VS_H245DataApplicationCapability_Application_T84& src)
	{

		O_CC(filled);
		O_C(t84Protocol);
		O_C(t84Profile);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DataApplicationCapability_Application_Nlpid /////////////////////////
 	 VS_H245DataApplicationCapability_Application_Nlpid :: VS_H245DataApplicationCapability_Application_Nlpid( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&nlpidProtocol,0);
		ref[1].Set(&nlpidData,0);
	}
	void VS_H245DataApplicationCapability_Application_Nlpid::operator=(const VS_H245DataApplicationCapability_Application_Nlpid& src)
	{

		O_CC(filled);
		O_C(nlpidProtocol);
		O_C(nlpidData);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DataApplicationCapability_Application_T38fax /////////////////////////
 	 VS_H245DataApplicationCapability_Application_T38fax :: VS_H245DataApplicationCapability_Application_T38fax( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&t38FaxProtocol,0);
		ref[1].Set(&t38FaxProfile,0);
	}
	void VS_H245DataApplicationCapability_Application_T38fax::operator=(const VS_H245DataApplicationCapability_Application_T38fax& src)
	{

		O_CC(filled);
		O_C(t38FaxProtocol);
		O_C(t38FaxProfile);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245DataApplicationCapability_Application /////////////////////////
 	 VS_H245DataApplicationCapability_Application::VS_H245DataApplicationCapability_Application( void )
	:VS_AsnChoice(10 , 14 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245DataApplicationCapability_Application::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_t120 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_dsm_cc : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_userData : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t84 : return DecodeChoice( buffer , new VS_H245DataApplicationCapability_Application_T84	 );
		case e_t434 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_h224 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_nlpid : return DecodeChoice( buffer , new VS_H245DataApplicationCapability_Application_Nlpid	 );
		case e_dsvdControl : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_h222DataPartitioning : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t30fax : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t140 : return DecodeChoice( buffer , new VS_H245DataProtocolCapability);
		case e_t38fax : return DecodeChoice( buffer , new VS_H245DataApplicationCapability_Application_T38fax	 );
		case e_genericDataCapability : return DecodeChoice( buffer , new VS_H245GenericCapability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245DataApplicationCapability_Application::operator=(const VS_H245DataApplicationCapability_Application & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_t120 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_dsm_cc : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_userData : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t84 : CopyChoice<VS_H245DataApplicationCapability_Application_T84	  >(src,*this);  return;
		case e_t434 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_h224 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_nlpid : CopyChoice<VS_H245DataApplicationCapability_Application_Nlpid	  >(src,*this);  return;
		case e_dsvdControl : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_h222DataPartitioning : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t30fax : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t140 : CopyChoice< VS_H245DataProtocolCapability >(src,*this); return;
		case e_t38fax : CopyChoice<VS_H245DataApplicationCapability_Application_T38fax	  >(src,*this);  return;
		case e_genericDataCapability : CopyChoice< VS_H245GenericCapability >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245DataApplicationCapability_Application::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245DataApplicationCapability_Application::operator VS_H245DataProtocolCapability *( void )
	{	return dynamic_cast< VS_H245DataProtocolCapability * >(choice);    }

 	VS_H245DataApplicationCapability_Application::operator VS_H245GenericCapability *( void )
	{	return dynamic_cast< VS_H245GenericCapability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245DataApplicationCapability_Application::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_t120 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_dsm_cc :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_userData :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t84 :  dprint4("choice: VS_H245DataApplicationCapability_Application_T84	 ");return;
		case e_t434 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_h224 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_nlpid :  dprint4("choice: VS_H245DataApplicationCapability_Application_Nlpid	 ");return;
		case e_dsvdControl :  dprint4("choice:  VS_AsnNull  ");return;
		case e_h222DataPartitioning :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t30fax :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t140 :  dprint4("choice: VS_H245DataProtocolCapability ");return;
		case e_t38fax :  dprint4("choice: VS_H245DataApplicationCapability_Application_T38fax	 ");return;
		case e_genericDataCapability :  dprint4("choice: VS_H245GenericCapability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245DataApplicationCapability /////////////////////////
 	 VS_H245DataApplicationCapability :: VS_H245DataApplicationCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&application,0);
		ref[1].Set(&maxBitRate,0);
	}
	void VS_H245DataApplicationCapability::operator=(const VS_H245DataApplicationCapability& src)
	{

		O_CC(filled);
		O_C(application);
		O_C(maxBitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VBDCapability /////////////////////////
 	 VS_H245VBDCapability :: VS_H245VBDCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&type,0);
	}
	void VS_H245VBDCapability::operator=(const VS_H245VBDCapability& src)
	{

		O_CC(filled);
		O_C(type);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245GSMAudioCapability /////////////////////////
 	 VS_H245GSMAudioCapability :: VS_H245GSMAudioCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioUnitSize,0);
		ref[1].Set(&comfortNoise,0);
		ref[2].Set(&scrambled,0);
	}
	void VS_H245GSMAudioCapability::operator=(const VS_H245GSMAudioCapability& src)
	{

		O_CC(filled);
		O_C(audioUnitSize);
		O_C(comfortNoise);
		O_C(scrambled);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245IS13818AudioCapability /////////////////////////
 	 VS_H245IS13818AudioCapability :: VS_H245IS13818AudioCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioLayer1,0);
		ref[1].Set(&audioLayer2,0);
		ref[2].Set(&audioLayer3,0);
		ref[3].Set(&audioSampling16k,0);
		ref[4].Set(&audioSampling22k05,0);
		ref[5].Set(&audioSampling24k,0);
		ref[6].Set(&audioSampling32k,0);
		ref[7].Set(&audioSampling44k1,0);
		ref[8].Set(&audioSampling48k,0);
		ref[9].Set(&singleChannel,0);
		ref[10].Set(&twoChannels,0);
		ref[11].Set(&threeChannels2_1,0);
		ref[12].Set(&threeChannels3_0,0);
		ref[13].Set(&fourChannels2_0_2_0,0);
		ref[14].Set(&fourChannels2_2,0);
		ref[15].Set(&fourChannels3_1,0);
		ref[16].Set(&fiveChannels3_0_2_0,0);
		ref[17].Set(&fiveChannels3_2,0);
		ref[18].Set(&lowFrequencyEnhancement,0);
		ref[19].Set(&multilingual,0);
		ref[20].Set(&bitRate,0);
	}
	void VS_H245IS13818AudioCapability::operator=(const VS_H245IS13818AudioCapability& src)
	{

		O_CC(filled);
		O_C(audioLayer1);
		O_C(audioLayer2);
		O_C(audioLayer3);
		O_C(audioSampling16k);
		O_C(audioSampling22k05);
		O_C(audioSampling24k);
		O_C(audioSampling32k);
		O_C(audioSampling44k1);
		O_C(audioSampling48k);
		O_C(singleChannel);
		O_C(twoChannels);
		O_C(threeChannels2_1);
		O_C(threeChannels3_0);
		O_C(fourChannels2_0_2_0);
		O_C(fourChannels2_2);
		O_C(fourChannels3_1);
		O_C(fiveChannels3_0_2_0);
		O_C(fiveChannels3_2);
		O_C(lowFrequencyEnhancement);
		O_C(multilingual);
		O_C(bitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245IS11172AudioCapability /////////////////////////
 	 VS_H245IS11172AudioCapability :: VS_H245IS11172AudioCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioLayer1,0);
		ref[1].Set(&audioLayer2,0);
		ref[2].Set(&audioLayer3,0);
		ref[3].Set(&audioSampling32k,0);
		ref[4].Set(&audioSampling44k1,0);
		ref[5].Set(&audioSampling48k,0);
		ref[6].Set(&singleChannel,0);
		ref[7].Set(&twoChannels,0);
		ref[8].Set(&bitRate,0);
	}
	void VS_H245IS11172AudioCapability::operator=(const VS_H245IS11172AudioCapability& src)
	{

		O_CC(filled);
		O_C(audioLayer1);
		O_C(audioLayer2);
		O_C(audioLayer3);
		O_C(audioSampling32k);
		O_C(audioSampling44k1);
		O_C(audioSampling48k);
		O_C(singleChannel);
		O_C(twoChannels);
		O_C(bitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245G7231AnnexCCapability_G723AnnexCAudioMode /////////////////////////
 	 VS_H245G7231AnnexCCapability_G723AnnexCAudioMode :: VS_H245G7231AnnexCCapability_G723AnnexCAudioMode( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&highRateMode0,0);
		ref[1].Set(&highRateMode1,0);
		ref[2].Set(&lowRateMode0,0);
		ref[3].Set(&lowRateMode1,0);
		ref[4].Set(&sidMode0,0);
		ref[5].Set(&sidMode1,0);
	}
	void VS_H245G7231AnnexCCapability_G723AnnexCAudioMode::operator=(const VS_H245G7231AnnexCCapability_G723AnnexCAudioMode& src)
	{

		O_CC(filled);
		O_C(highRateMode0);
		O_C(highRateMode1);
		O_C(lowRateMode0);
		O_C(lowRateMode1);
		O_C(sidMode0);
		O_C(sidMode1);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245G7231AnnexCCapability /////////////////////////
 	 VS_H245G7231AnnexCCapability :: VS_H245G7231AnnexCCapability( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&maxAl_sduAudioFrames,0);
		ref[1].Set(&silenceSuppression,0);
		ref[2].Set(&g723AnnexCAudioMode,1);
	}
	void VS_H245G7231AnnexCCapability::operator=(const VS_H245G7231AnnexCCapability& src)
	{

		O_CC(filled);
		O_C(maxAl_sduAudioFrames);
		O_C(silenceSuppression);
		O_C(g723AnnexCAudioMode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245G729Extensions /////////////////////////
 	 VS_H245G729Extensions :: VS_H245G729Extensions( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioUnit,1);
		ref[1].Set(&annexA,0);
		ref[2].Set(&annexB,0);
		ref[3].Set(&annexD,0);
		ref[4].Set(&annexE,0);
		ref[5].Set(&annexF,0);
		ref[6].Set(&annexG,0);
		ref[7].Set(&annexH,0);
	}
	void VS_H245G729Extensions::operator=(const VS_H245G729Extensions& src)
	{

		O_CC(filled);
		O_C(audioUnit);
		O_C(annexA);
		O_C(annexB);
		O_C(annexD);
		O_C(annexE);
		O_C(annexF);
		O_C(annexG);
		O_C(annexH);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245AudioCapability_G7231 /////////////////////////
 	 VS_H245AudioCapability_G7231 :: VS_H245AudioCapability_G7231( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&maxAl_sduAudioFrames,0);
		ref[1].Set(&silenceSuppression,0);
	}
	void VS_H245AudioCapability_G7231::operator=(const VS_H245AudioCapability_G7231& src)
	{

		O_CC(filled);
		O_C(maxAl_sduAudioFrames);
		O_C(silenceSuppression);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245AudioCapability /////////////////////////
 	 VS_H245AudioCapability::VS_H245AudioCapability( void )
	:VS_AsnChoice(14 , 25 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245AudioCapability::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_g711Alaw64k : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g711Alaw56k : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g711Ulaw64k : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g711Ulaw56k : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g722_64k : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g722_56k : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g722_48k : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g7231 : return DecodeChoice( buffer , new VS_H245AudioCapability_G7231	 );
		case e_g728 : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g729 : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g729AnnexA : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_is11172AudioCapability : return DecodeChoice( buffer , new VS_H245IS11172AudioCapability);
		case e_is13818AudioCapability : return DecodeChoice( buffer , new VS_H245IS13818AudioCapability);
		case e_g729wAnnexB : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g729AnnexAwAnnexB : return DecodeChoice( buffer , new TemplInteger<1,256,VS_Asn::FixedConstraint,0>  );
		case e_g7231AnnexCCapability : return DecodeChoice( buffer , new VS_H245G7231AnnexCCapability);
		case e_gsmFullRate : return DecodeChoice( buffer , new VS_H245GSMAudioCapability);
		case e_gsmHalfRate : return DecodeChoice( buffer , new VS_H245GSMAudioCapability);
		case e_gsmEnhancedFullRate : return DecodeChoice( buffer , new VS_H245GSMAudioCapability);
		case e_genericAudioCapability : return DecodeChoice( buffer , new VS_H245GenericCapability);
		case e_g729Extensions : return DecodeChoice( buffer , new VS_H245G729Extensions);
		case e_vbd : return DecodeChoice( buffer , new VS_H245VBDCapability);
		case e_audioTelephonyEvent : return DecodeChoice( buffer , new VS_H245NoPTAudioTelephonyEventCapability);
		case e_audioTone : return DecodeChoice( buffer , new VS_H245NoPTAudioToneCapability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245AudioCapability::operator=(const VS_H245AudioCapability & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_g711Alaw64k : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g711Alaw56k : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g711Ulaw64k : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g711Ulaw56k : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g722_64k : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g722_56k : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g722_48k : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g7231 : CopyChoice<VS_H245AudioCapability_G7231	  >(src,*this);  return;
		case e_g728 : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g729 : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g729AnnexA : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_is11172AudioCapability : CopyChoice< VS_H245IS11172AudioCapability >(src,*this); return;
		case e_is13818AudioCapability : CopyChoice< VS_H245IS13818AudioCapability >(src,*this); return;
		case e_g729wAnnexB : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g729AnnexAwAnnexB : CopyChoice<TemplInteger<1,256,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_g7231AnnexCCapability : CopyChoice< VS_H245G7231AnnexCCapability >(src,*this); return;
		case e_gsmFullRate : CopyChoice< VS_H245GSMAudioCapability >(src,*this); return;
		case e_gsmHalfRate : CopyChoice< VS_H245GSMAudioCapability >(src,*this); return;
		case e_gsmEnhancedFullRate : CopyChoice< VS_H245GSMAudioCapability >(src,*this); return;
		case e_genericAudioCapability : CopyChoice< VS_H245GenericCapability >(src,*this); return;
		case e_g729Extensions : CopyChoice< VS_H245G729Extensions >(src,*this); return;
		case e_vbd : CopyChoice< VS_H245VBDCapability >(src,*this); return;
		case e_audioTelephonyEvent : CopyChoice< VS_H245NoPTAudioTelephonyEventCapability >(src,*this); return;
		case e_audioTone : CopyChoice< VS_H245NoPTAudioToneCapability >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245AudioCapability::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245IS11172AudioCapability *( void )
	{	return dynamic_cast< VS_H245IS11172AudioCapability * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245IS13818AudioCapability *( void )
	{	return dynamic_cast< VS_H245IS13818AudioCapability * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245G7231AnnexCCapability *( void )
	{	return dynamic_cast< VS_H245G7231AnnexCCapability * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245GSMAudioCapability *( void )
	{	return dynamic_cast< VS_H245GSMAudioCapability * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245GenericCapability *( void )
	{	return dynamic_cast< VS_H245GenericCapability * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245G729Extensions *( void )
	{	return dynamic_cast< VS_H245G729Extensions * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245VBDCapability *( void )
	{	return dynamic_cast< VS_H245VBDCapability * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245NoPTAudioTelephonyEventCapability *( void )
	{	return dynamic_cast< VS_H245NoPTAudioTelephonyEventCapability * >(choice);    }

 	VS_H245AudioCapability::operator VS_H245NoPTAudioToneCapability *( void )
	{	return dynamic_cast< VS_H245NoPTAudioToneCapability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245AudioCapability::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_g711Alaw64k :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g711Alaw56k :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g711Ulaw64k :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g711Ulaw56k :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g722_64k :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g722_56k :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g722_48k :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g7231 :  dprint4("choice: VS_H245AudioCapability_G7231	 ");return;
		case e_g728 :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g729 :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g729AnnexA :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_is11172AudioCapability :  dprint4("choice: VS_H245IS11172AudioCapability ");return;
		case e_is13818AudioCapability :  dprint4("choice: VS_H245IS13818AudioCapability ");return;
		case e_g729wAnnexB :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g729AnnexAwAnnexB :  dprint4("choice: TemplInteger<1,256,VS_Asn::FixedConstraint,0>  ");return;
		case e_g7231AnnexCCapability :  dprint4("choice: VS_H245G7231AnnexCCapability ");return;
		case e_gsmFullRate :  dprint4("choice: VS_H245GSMAudioCapability ");return;
		case e_gsmHalfRate :  dprint4("choice: VS_H245GSMAudioCapability ");return;
		case e_gsmEnhancedFullRate :  dprint4("choice: VS_H245GSMAudioCapability ");return;
		case e_genericAudioCapability :  dprint4("choice: VS_H245GenericCapability ");return;
		case e_g729Extensions :  dprint4("choice: VS_H245G729Extensions ");return;
		case e_vbd :  dprint4("choice: VS_H245VBDCapability ");return;
		case e_audioTelephonyEvent :  dprint4("choice: VS_H245NoPTAudioTelephonyEventCapability ");return;
		case e_audioTone :  dprint4("choice: VS_H245NoPTAudioToneCapability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245IS11172VideoCapability /////////////////////////
 	 VS_H245IS11172VideoCapability :: VS_H245IS11172VideoCapability( void )
	:VS_AsnSequence(6 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&constrainedBitstream,0);
		ref[1].Set(&videoBitRate,1);
		ref[2].Set(&vbvBufferSize,1);
		ref[3].Set(&samplesPerLine,1);
		ref[4].Set(&linesPerFrame,1);
		ref[5].Set(&pictureRate,1);
		ref[6].Set(&luminanceSampleRate,1);
		e_ref[0].Set(&videoBadMBsCap,0);
	}
	void VS_H245IS11172VideoCapability::operator=(const VS_H245IS11172VideoCapability& src)
	{

		O_CC(filled);
		O_C(constrainedBitstream);
		O_C(videoBitRate);
		O_C(vbvBufferSize);
		O_C(samplesPerLine);
		O_C(linesPerFrame);
		O_C(pictureRate);
		O_C(luminanceSampleRate);
		O_C(videoBadMBsCap);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H263Version3Options /////////////////////////
 	 VS_H245H263Version3Options :: VS_H245H263Version3Options( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&dataPartitionedSlices,0);
		ref[1].Set(&fixedPointIDCT0,0);
		ref[2].Set(&interlacedFields,0);
		ref[3].Set(&currentPictureHeaderRepetition,0);
		ref[4].Set(&previousPictureHeaderRepetition,0);
		ref[5].Set(&nextPictureHeaderRepetition,0);
		ref[6].Set(&pictureNumber,0);
		ref[7].Set(&spareReferencePictures,0);
	}
	void VS_H245H263Version3Options::operator=(const VS_H245H263Version3Options& src)
	{

		O_CC(filled);
		O_C(dataPartitionedSlices);
		O_C(fixedPointIDCT0);
		O_C(interlacedFields);
		O_C(currentPictureHeaderRepetition);
		O_C(previousPictureHeaderRepetition);
		O_C(nextPictureHeaderRepetition);
		O_C(pictureNumber);
		O_C(spareReferencePictures);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H263ModeComboFlags /////////////////////////
 	 VS_H245H263ModeComboFlags :: VS_H245H263ModeComboFlags( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&unrestrictedVector,0);
		ref[1].Set(&arithmeticCoding,0);
		ref[2].Set(&advancedPrediction,0);
		ref[3].Set(&pbFrames,0);
		ref[4].Set(&advancedIntraCodingMode,0);
		ref[5].Set(&deblockingFilterMode,0);
		ref[6].Set(&unlimitedMotionVectors,0);
		ref[7].Set(&slicesInOrder_NonRect,0);
		ref[8].Set(&slicesInOrder_Rect,0);
		ref[9].Set(&slicesNoOrder_NonRect,0);
		ref[10].Set(&slicesNoOrder_Rect,0);
		ref[11].Set(&improvedPBFramesMode,0);
		ref[12].Set(&referencePicSelect,0);
		ref[13].Set(&dynamicPictureResizingByFour,0);
		ref[14].Set(&dynamicPictureResizingSixteenthPel,0);
		ref[15].Set(&dynamicWarpingHalfPel,0);
		ref[16].Set(&dynamicWarpingSixteenthPel,0);
		ref[17].Set(&reducedResolutionUpdate,0);
		ref[18].Set(&independentSegmentDecoding,0);
		ref[19].Set(&alternateInterVLCMode,0);
		ref[20].Set(&modifiedQuantizationMode,0);
		e_ref[0].Set(&enhancedReferencePicSelect,0);
		e_ref[1].Set(&h263Version3Options,0);
	}
	void VS_H245H263ModeComboFlags::operator=(const VS_H245H263ModeComboFlags& src)
	{

		O_CC(filled);
		O_C(unrestrictedVector);
		O_C(arithmeticCoding);
		O_C(advancedPrediction);
		O_C(pbFrames);
		O_C(advancedIntraCodingMode);
		O_C(deblockingFilterMode);
		O_C(unlimitedMotionVectors);
		O_C(slicesInOrder_NonRect);
		O_C(slicesInOrder_Rect);
		O_C(slicesNoOrder_NonRect);
		O_C(slicesNoOrder_Rect);
		O_C(improvedPBFramesMode);
		O_C(referencePicSelect);
		O_C(dynamicPictureResizingByFour);
		O_C(dynamicPictureResizingSixteenthPel);
		O_C(dynamicWarpingHalfPel);
		O_C(dynamicWarpingSixteenthPel);
		O_C(reducedResolutionUpdate);
		O_C(independentSegmentDecoding);
		O_C(alternateInterVLCMode);
		O_C(modifiedQuantizationMode);
		O_C(enhancedReferencePicSelect);
		O_C(h263Version3Options);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H263VideoModeCombos /////////////////////////
 	 VS_H245H263VideoModeCombos :: VS_H245H263VideoModeCombos( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&h263VideoUncoupledModes,0);
		ref[1].Set(&h263VideoCoupledModes,0);
	}
	void VS_H245H263VideoModeCombos::operator=(const VS_H245H263VideoModeCombos& src)
	{

		O_CC(filled);
		O_C(h263VideoUncoupledModes);
		O_C(h263VideoCoupledModes);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CustomPCF /////////////////////////
 	 VS_H245CustomPCF :: VS_H245CustomPCF( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&clockConversionCode,0);
		ref[1].Set(&clockDivisor,0);
		ref[2].Set(&customMPI,0);
	}
	void VS_H245CustomPCF::operator=(const VS_H245CustomPCF& src)
	{

		O_CC(filled);
		O_C(clockConversionCode);
		O_C(clockDivisor);
		O_C(customMPI);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ExtendedPAR /////////////////////////
 	 VS_H245ExtendedPAR :: VS_H245ExtendedPAR( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&width,0);
		ref[1].Set(&height,0);
	}
	void VS_H245ExtendedPAR::operator=(const VS_H245ExtendedPAR& src)
	{

		O_CC(filled);
		O_C(width);
		O_C(height);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CustomPictureFormat_MPI /////////////////////////
 	 VS_H245CustomPictureFormat_MPI :: VS_H245CustomPictureFormat_MPI( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&standardMPI,1);
		ref[1].Set(&customPCF,1);
	}
	void VS_H245CustomPictureFormat_MPI::operator=(const VS_H245CustomPictureFormat_MPI& src)
	{

		O_CC(filled);
		O_C(standardMPI);
		O_C(customPCF);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CustomPictureFormat_PixelAspectInformation /////////////////////////
 	 VS_H245CustomPictureFormat_PixelAspectInformation::VS_H245CustomPictureFormat_PixelAspectInformation( void )
	:VS_AsnChoice(3 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CustomPictureFormat_PixelAspectInformation::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_anyPixelAspectRatio : return DecodeChoice( buffer , new  VS_AsnBoolean  );
		case e_pixelAspectCode : return DecodeChoice( buffer , new Constrained_array_of_type< TemplInteger<1,14,VS_Asn::FixedConstraint,0> ,1,14,VS_Asn::FixedConstraint,0  >  );
		case e_extendedPAR : return DecodeChoice( buffer , new VS_H245ExtendedPAR);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245CustomPictureFormat_PixelAspectInformation::operator=(const VS_H245CustomPictureFormat_PixelAspectInformation & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_anyPixelAspectRatio : CopyChoice< VS_AsnBoolean   >(src,*this);  return;
		case e_pixelAspectCode : CopyChoice<Constrained_array_of_type< TemplInteger<1,14,VS_Asn::FixedConstraint,0> ,1,14,VS_Asn::FixedConstraint,0  >   >(src,*this);  return;
		case e_extendedPAR : CopyChoice< VS_H245ExtendedPAR >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245CustomPictureFormat_PixelAspectInformation::operator VS_H245ExtendedPAR *( void )
	{	return dynamic_cast< VS_H245ExtendedPAR * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CustomPictureFormat_PixelAspectInformation::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_anyPixelAspectRatio :  dprint4("choice:  VS_AsnBoolean  ");return;
		case e_pixelAspectCode :  dprint4("choice: Constrained_array_of_type< TemplInteger<1,14,VS_Asn::FixedConstraint,0> ,1,14,VS_Asn::FixedConstraint,0  >  ");return;
		case e_extendedPAR :  dprint4("choice: VS_H245ExtendedPAR ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245CustomPictureFormat /////////////////////////
 	 VS_H245CustomPictureFormat :: VS_H245CustomPictureFormat( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&maxCustomPictureWidth,0);
		ref[1].Set(&maxCustomPictureHeight,0);
		ref[2].Set(&minCustomPictureWidth,0);
		ref[3].Set(&minCustomPictureHeight,0);
		ref[4].Set(&mPI,0);
		ref[5].Set(&pixelAspectInformation,0);
	}
	void VS_H245CustomPictureFormat::operator=(const VS_H245CustomPictureFormat& src)
	{

		O_CC(filled);
		O_C(maxCustomPictureWidth);
		O_C(maxCustomPictureHeight);
		O_C(minCustomPictureWidth);
		O_C(minCustomPictureHeight);
		O_C(mPI);
		O_C(pixelAspectInformation);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CustomPictureClockFrequency /////////////////////////
 	 VS_H245CustomPictureClockFrequency :: VS_H245CustomPictureClockFrequency( void )
	:VS_AsnSequence(5 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&clockConversionCode,0);
		ref[1].Set(&clockDivisor,0);
		ref[2].Set(&sqcifMPI,1);
		ref[3].Set(&qcifMPI,1);
		ref[4].Set(&cifMPI,1);
		ref[5].Set(&cif4MPI,1);
		ref[6].Set(&cif16MPI,1);
	}
	void VS_H245CustomPictureClockFrequency::operator=(const VS_H245CustomPictureClockFrequency& src)
	{

		O_CC(filled);
		O_C(clockConversionCode);
		O_C(clockDivisor);
		O_C(sqcifMPI);
		O_C(qcifMPI);
		O_C(cifMPI);
		O_C(cif4MPI);
		O_C(cif16MPI);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RefPictureSelection_AdditionalPictureMemory /////////////////////////
 	 VS_H245RefPictureSelection_AdditionalPictureMemory :: VS_H245RefPictureSelection_AdditionalPictureMemory( void )
	:VS_AsnSequence(6 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sqcifAdditionalPictureMemory,1);
		ref[1].Set(&qcifAdditionalPictureMemory,1);
		ref[2].Set(&cifAdditionalPictureMemory,1);
		ref[3].Set(&cif4AdditionalPictureMemory,1);
		ref[4].Set(&cif16AdditionalPictureMemory,1);
		ref[5].Set(&bigCpfAdditionalPictureMemory,1);
	}
	void VS_H245RefPictureSelection_AdditionalPictureMemory::operator=(const VS_H245RefPictureSelection_AdditionalPictureMemory& src)
	{

		O_CC(filled);
		O_C(sqcifAdditionalPictureMemory);
		O_C(qcifAdditionalPictureMemory);
		O_C(cifAdditionalPictureMemory);
		O_C(cif4AdditionalPictureMemory);
		O_C(cif16AdditionalPictureMemory);
		O_C(bigCpfAdditionalPictureMemory);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RefPictureSelection_VideoBackChannelSend /////////////////////////
 	 VS_H245RefPictureSelection_VideoBackChannelSend::VS_H245RefPictureSelection_VideoBackChannelSend( void )
	:VS_AsnChoice(5 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RefPictureSelection_VideoBackChannelSend::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_none : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_ackMessageOnly : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_nackMessageOnly : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_ackOrNackMessageOnly : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_ackAndNackMessage : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RefPictureSelection_VideoBackChannelSend::operator=(const VS_H245RefPictureSelection_VideoBackChannelSend & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_none : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_ackMessageOnly : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_nackMessageOnly : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_ackOrNackMessageOnly : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_ackAndNackMessage : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RefPictureSelection_VideoBackChannelSend::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_none :  dprint4("choice:  VS_AsnNull  ");return;
		case e_ackMessageOnly :  dprint4("choice:  VS_AsnNull  ");return;
		case e_nackMessageOnly :  dprint4("choice:  VS_AsnNull  ");return;
		case e_ackOrNackMessageOnly :  dprint4("choice:  VS_AsnNull  ");return;
		case e_ackAndNackMessage :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters /////////////////////////
 	 VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters :: VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&mpuHorizMBs,0);
		ref[1].Set(&mpuVertMBs,0);
		ref[2].Set(&mpuTotalNumber,0);
	}
	void VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters::operator=(const VS_H245RefPictureSelection_EnhancedReferencePicSelect_SubPictureRemovalParameters& src)
	{

		O_CC(filled);
		O_C(mpuHorizMBs);
		O_C(mpuVertMBs);
		O_C(mpuTotalNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RefPictureSelection_EnhancedReferencePicSelect /////////////////////////
 	 VS_H245RefPictureSelection_EnhancedReferencePicSelect :: VS_H245RefPictureSelection_EnhancedReferencePicSelect( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&subPictureRemovalParameters,1);
	}
	void VS_H245RefPictureSelection_EnhancedReferencePicSelect::operator=(const VS_H245RefPictureSelection_EnhancedReferencePicSelect& src)
	{

		O_CC(filled);
		O_C(subPictureRemovalParameters);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RefPictureSelection /////////////////////////
 	 VS_H245RefPictureSelection :: VS_H245RefPictureSelection( void )
	:VS_AsnSequence(1 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&additionalPictureMemory,1);
		ref[1].Set(&videoMux,0);
		ref[2].Set(&videoBackChannelSend,0);
		e_ref[0].Set(&enhancedReferencePicSelect,0);
	}
	void VS_H245RefPictureSelection::operator=(const VS_H245RefPictureSelection& src)
	{

		O_CC(filled);
		O_C(additionalPictureMemory);
		O_C(videoMux);
		O_C(videoBackChannelSend);
		O_C(enhancedReferencePicSelect);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245TransparencyParameters /////////////////////////
 	 VS_H245TransparencyParameters :: VS_H245TransparencyParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&presentationOrder,0);
		ref[1].Set(&offset_x,0);
		ref[2].Set(&offset_y,0);
		ref[3].Set(&scale_x,0);
		ref[4].Set(&scale_y,0);
	}
	void VS_H245TransparencyParameters::operator=(const VS_H245TransparencyParameters& src)
	{

		O_CC(filled);
		O_C(presentationOrder);
		O_C(offset_x);
		O_C(offset_y);
		O_C(scale_x);
		O_C(scale_y);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H263Options /////////////////////////
 	 VS_H245H263Options :: VS_H245H263Options( void )
	:VS_AsnSequence(5 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&advancedIntraCodingMode,0);
		ref[1].Set(&deblockingFilterMode,0);
		ref[2].Set(&improvedPBFramesMode,0);
		ref[3].Set(&unlimitedMotionVectors,0);
		ref[4].Set(&fullPictureFreeze,0);
		ref[5].Set(&partialPictureFreezeAndRelease,0);
		ref[6].Set(&resizingPartPicFreezeAndRelease,0);
		ref[7].Set(&fullPictureSnapshot,0);
		ref[8].Set(&partialPictureSnapshot,0);
		ref[9].Set(&videoSegmentTagging,0);
		ref[10].Set(&progressiveRefinement,0);
		ref[11].Set(&dynamicPictureResizingByFour,0);
		ref[12].Set(&dynamicPictureResizingSixteenthPel,0);
		ref[13].Set(&dynamicWarpingHalfPel,0);
		ref[14].Set(&dynamicWarpingSixteenthPel,0);
		ref[15].Set(&independentSegmentDecoding,0);
		ref[16].Set(&slicesInOrder_NonRect,0);
		ref[17].Set(&slicesInOrder_Rect,0);
		ref[18].Set(&slicesNoOrder_NonRect,0);
		ref[19].Set(&slicesNoOrder_Rect,0);
		ref[20].Set(&alternateInterVLCMode,0);
		ref[21].Set(&modifiedQuantizationMode,0);
		ref[22].Set(&reducedResolutionUpdate,0);
		ref[23].Set(&transparencyParameters,1);
		ref[24].Set(&separateVideoBackChannel,0);
		ref[25].Set(&refPictureSelection,1);
		ref[26].Set(&customPictureClockFrequency,1);
		ref[27].Set(&customPictureFormat,1);
		ref[28].Set(&modeCombos,1);
		e_ref[0].Set(&videoBadMBsCap,0);
		e_ref[1].Set(&h263Version3Options,0);
	}
	void VS_H245H263Options::operator=(const VS_H245H263Options& src)
	{

		O_CC(filled);
		O_C(advancedIntraCodingMode);
		O_C(deblockingFilterMode);
		O_C(improvedPBFramesMode);
		O_C(unlimitedMotionVectors);
		O_C(fullPictureFreeze);
		O_C(partialPictureFreezeAndRelease);
		O_C(resizingPartPicFreezeAndRelease);
		O_C(fullPictureSnapshot);
		O_C(partialPictureSnapshot);
		O_C(videoSegmentTagging);
		O_C(progressiveRefinement);
		O_C(dynamicPictureResizingByFour);
		O_C(dynamicPictureResizingSixteenthPel);
		O_C(dynamicWarpingHalfPel);
		O_C(dynamicWarpingSixteenthPel);
		O_C(independentSegmentDecoding);
		O_C(slicesInOrder_NonRect);
		O_C(slicesInOrder_Rect);
		O_C(slicesNoOrder_NonRect);
		O_C(slicesNoOrder_Rect);
		O_C(alternateInterVLCMode);
		O_C(modifiedQuantizationMode);
		O_C(reducedResolutionUpdate);
		O_C(transparencyParameters);
		O_C(separateVideoBackChannel);
		O_C(refPictureSelection);
		O_C(customPictureClockFrequency);
		O_C(customPictureFormat);
		O_C(modeCombos);
		O_C(videoBadMBsCap);
		O_C(h263Version3Options);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245EnhancementOptions /////////////////////////
 	 VS_H245EnhancementOptions :: VS_H245EnhancementOptions( void )
	:VS_AsnSequence(11 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sqcifMPI,1);
		ref[1].Set(&qcifMPI,1);
		ref[2].Set(&cifMPI,1);
		ref[3].Set(&cif4MPI,1);
		ref[4].Set(&cif16MPI,1);
		ref[5].Set(&maxBitRate,0);
		ref[6].Set(&unrestrictedVector,0);
		ref[7].Set(&arithmeticCoding,0);
		ref[8].Set(&temporalSpatialTradeOffCapability,0);
		ref[9].Set(&slowSqcifMPI,1);
		ref[10].Set(&slowQcifMPI,1);
		ref[11].Set(&slowCifMPI,1);
		ref[12].Set(&slowCif4MPI,1);
		ref[13].Set(&slowCif16MPI,1);
		ref[14].Set(&errorCompensation,0);
		ref[15].Set(&h263Options,1);
	}
	void VS_H245EnhancementOptions::operator=(const VS_H245EnhancementOptions& src)
	{

		O_CC(filled);
		O_C(sqcifMPI);
		O_C(qcifMPI);
		O_C(cifMPI);
		O_C(cif4MPI);
		O_C(cif16MPI);
		O_C(maxBitRate);
		O_C(unrestrictedVector);
		O_C(arithmeticCoding);
		O_C(temporalSpatialTradeOffCapability);
		O_C(slowSqcifMPI);
		O_C(slowQcifMPI);
		O_C(slowCifMPI);
		O_C(slowCif4MPI);
		O_C(slowCif16MPI);
		O_C(errorCompensation);
		O_C(h263Options);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245BEnhancementParameters /////////////////////////
 	 VS_H245BEnhancementParameters :: VS_H245BEnhancementParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&enhancementOptions,0);
		ref[1].Set(&numberOfBPictures,0);
	}
	void VS_H245BEnhancementParameters::operator=(const VS_H245BEnhancementParameters& src)
	{

		O_CC(filled);
		O_C(enhancementOptions);
		O_C(numberOfBPictures);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245EnhancementLayerInfo /////////////////////////
 	 VS_H245EnhancementLayerInfo :: VS_H245EnhancementLayerInfo( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&baseBitRateConstrained,0);
		ref[1].Set(&snrEnhancement,1);
		ref[2].Set(&spatialEnhancement,1);
		ref[3].Set(&bPictureEnhancement,1);
	}
	void VS_H245EnhancementLayerInfo::operator=(const VS_H245EnhancementLayerInfo& src)
	{

		O_CC(filled);
		O_C(baseBitRateConstrained);
		O_C(snrEnhancement);
		O_C(spatialEnhancement);
		O_C(bPictureEnhancement);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H263VideoCapability /////////////////////////
 	 VS_H245H263VideoCapability :: VS_H245H263VideoCapability( void )
	:VS_AsnSequence(7 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&sqcifMPI,1);
		ref[1].Set(&qcifMPI,1);
		ref[2].Set(&cifMPI,1);
		ref[3].Set(&cif4MPI,1);
		ref[4].Set(&cif16MPI,1);
		ref[5].Set(&maxBitRate,0);
		ref[6].Set(&unrestrictedVector,0);
		ref[7].Set(&arithmeticCoding,0);
		ref[8].Set(&advancedPrediction,0);
		ref[9].Set(&pbFrames,0);
		ref[10].Set(&temporalSpatialTradeOffCapability,0);
		ref[11].Set(&hrd_B,1);
		ref[12].Set(&bppMaxKb,1);
		e_ref[0].Set(&slowSqcifMPI,1);
		e_ref[1].Set(&slowQcifMPI,1);
		e_ref[2].Set(&slowCifMPI,1);
		e_ref[3].Set(&slowCif4MPI,1);
		e_ref[4].Set(&slowCif16MPI,1);
		e_ref[5].Set(&errorCompensation,0);
		e_ref[6].Set(&enhancementLayerInfo,1);
		e_ref[7].Set(&h263Options,1);
	}
	void VS_H245H263VideoCapability::operator=(const VS_H245H263VideoCapability& src)
	{

		O_CC(filled);
		O_C(sqcifMPI);
		O_C(qcifMPI);
		O_C(cifMPI);
		O_C(cif4MPI);
		O_C(cif16MPI);
		O_C(maxBitRate);
		O_C(unrestrictedVector);
		O_C(arithmeticCoding);
		O_C(advancedPrediction);
		O_C(pbFrames);
		O_C(temporalSpatialTradeOffCapability);
		O_C(hrd_B);
		O_C(bppMaxKb);
		O_C(slowSqcifMPI);
		O_C(slowQcifMPI);
		O_C(slowCifMPI);
		O_C(slowCif4MPI);
		O_C(slowCif16MPI);
		O_C(errorCompensation);
		O_C(enhancementLayerInfo);
		O_C(h263Options);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H262VideoCapability /////////////////////////
 	 VS_H245H262VideoCapability :: VS_H245H262VideoCapability( void )
	:VS_AsnSequence(6 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&profileAndLevel_SPatML,0);
		ref[1].Set(&profileAndLevel_MPatLL,0);
		ref[2].Set(&profileAndLevel_MPatML,0);
		ref[3].Set(&profileAndLevel_MPatH_14,0);
		ref[4].Set(&profileAndLevel_MPatHL,0);
		ref[5].Set(&profileAndLevel_SNRatLL,0);
		ref[6].Set(&profileAndLevel_SNRatML,0);
		ref[7].Set(&profileAndLevel_SpatialatH_14,0);
		ref[8].Set(&profileAndLevel_HPatML,0);
		ref[9].Set(&profileAndLevel_HPatH_14,0);
		ref[10].Set(&profileAndLevel_HPatHL,0);
		ref[11].Set(&videoBitRate,1);
		ref[12].Set(&vbvBufferSize,1);
		ref[13].Set(&samplesPerLine,1);
		ref[14].Set(&linesPerFrame,1);
		ref[15].Set(&framesPerSecond,1);
		ref[16].Set(&luminanceSampleRate,1);
		ref[17].Set(&videoBadMBsCap,0);
	}
	void VS_H245H262VideoCapability::operator=(const VS_H245H262VideoCapability& src)
	{

		O_CC(filled);
		O_C(profileAndLevel_SPatML);
		O_C(profileAndLevel_MPatLL);
		O_C(profileAndLevel_MPatML);
		O_C(profileAndLevel_MPatH_14);
		O_C(profileAndLevel_MPatHL);
		O_C(profileAndLevel_SNRatLL);
		O_C(profileAndLevel_SNRatML);
		O_C(profileAndLevel_SpatialatH_14);
		O_C(profileAndLevel_HPatML);
		O_C(profileAndLevel_HPatH_14);
		O_C(profileAndLevel_HPatHL);
		O_C(videoBitRate);
		O_C(vbvBufferSize);
		O_C(samplesPerLine);
		O_C(linesPerFrame);
		O_C(framesPerSecond);
		O_C(luminanceSampleRate);
		O_C(videoBadMBsCap);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H261VideoCapability /////////////////////////
 	 VS_H245H261VideoCapability :: VS_H245H261VideoCapability( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&qcifMPI,1);
		ref[1].Set(&cifMPI,1);
		ref[2].Set(&temporalSpatialTradeOffCapability,0);
		ref[3].Set(&maxBitRate,0);
		ref[4].Set(&stillImageTransmission,0);
		e_ref[0].Set(&videoBadMBsCap,0);
	}
	void VS_H245H261VideoCapability::operator=(const VS_H245H261VideoCapability& src)
	{

		O_CC(filled);
		O_C(qcifMPI);
		O_C(cifMPI);
		O_C(temporalSpatialTradeOffCapability);
		O_C(maxBitRate);
		O_C(stillImageTransmission);
		O_C(videoBadMBsCap);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ExtendedVideoCapability /////////////////////////
 	 VS_H245ExtendedVideoCapability :: VS_H245ExtendedVideoCapability( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&videoCapability,0);
		ref[1].Set(&videoCapabilityExtension,1);
	}
	void VS_H245ExtendedVideoCapability::operator=(const VS_H245ExtendedVideoCapability& src)
	{

		O_CC(filled);
		O_C(videoCapability);
		O_C(videoCapabilityExtension);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VideoCapability /////////////////////////
 	 VS_H245VideoCapability::VS_H245VideoCapability( void )
	:VS_AsnChoice(5 , 7 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245VideoCapability::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_h261VideoCapability : return DecodeChoice( buffer , new VS_H245H261VideoCapability);
		case e_h262VideoCapability : return DecodeChoice( buffer , new VS_H245H262VideoCapability);
		case e_h263VideoCapability : return DecodeChoice( buffer , new VS_H245H263VideoCapability);
		case e_is11172VideoCapability : return DecodeChoice( buffer , new VS_H245IS11172VideoCapability);
		case e_genericVideoCapability : return DecodeChoice( buffer , new VS_H245GenericCapability);
		case e_extendedVideoCapability : return DecodeChoice( buffer , new VS_H245ExtendedVideoCapability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245VideoCapability::operator=(const VS_H245VideoCapability & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_h261VideoCapability : CopyChoice< VS_H245H261VideoCapability >(src,*this); return;
		case e_h262VideoCapability : CopyChoice< VS_H245H262VideoCapability >(src,*this); return;
		case e_h263VideoCapability : CopyChoice< VS_H245H263VideoCapability >(src,*this); return;
		case e_is11172VideoCapability : CopyChoice< VS_H245IS11172VideoCapability >(src,*this); return;
		case e_genericVideoCapability : CopyChoice< VS_H245GenericCapability >(src,*this); return;
		case e_extendedVideoCapability : CopyChoice< VS_H245ExtendedVideoCapability >(src,*this); return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245VideoCapability::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245VideoCapability::operator VS_H245H261VideoCapability *( void )
	{	return dynamic_cast< VS_H245H261VideoCapability * >(choice);    }

 	VS_H245VideoCapability::operator VS_H245H262VideoCapability *( void )
	{	return dynamic_cast< VS_H245H262VideoCapability * >(choice);    }

 	VS_H245VideoCapability::operator VS_H245H263VideoCapability *( void )
	{	return dynamic_cast< VS_H245H263VideoCapability * >(choice);    }

 	VS_H245VideoCapability::operator VS_H245IS11172VideoCapability *( void )
	{	return dynamic_cast< VS_H245IS11172VideoCapability * >(choice);    }

 	VS_H245VideoCapability::operator VS_H245GenericCapability *( void )
	{	return dynamic_cast< VS_H245GenericCapability * >(choice);    }

 	VS_H245VideoCapability::operator VS_H245ExtendedVideoCapability *( void )
	{	return dynamic_cast< VS_H245ExtendedVideoCapability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245VideoCapability::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_h261VideoCapability :  dprint4("choice: VS_H245H261VideoCapability ");return;
		case e_h262VideoCapability :  dprint4("choice: VS_H245H262VideoCapability ");return;
		case e_h263VideoCapability :  dprint4("choice: VS_H245H263VideoCapability ");return;
		case e_is11172VideoCapability :  dprint4("choice: VS_H245IS11172VideoCapability ");return;
		case e_genericVideoCapability :  dprint4("choice: VS_H245GenericCapability ");return;
		case e_extendedVideoCapability :  dprint4("choice: VS_H245ExtendedVideoCapability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MediaDistributionCapability /////////////////////////
 	 VS_H245MediaDistributionCapability :: VS_H245MediaDistributionCapability( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&centralizedControl,0);
		ref[1].Set(&distributedControl,0);
		ref[2].Set(&centralizedAudio,0);
		ref[3].Set(&distributedAudio,0);
		ref[4].Set(&centralizedVideo,0);
		ref[5].Set(&distributedVideo,0);
		ref[6].Set(&centralizedData,1);
		ref[7].Set(&distributedData,1);
	}
	void VS_H245MediaDistributionCapability::operator=(const VS_H245MediaDistributionCapability& src)
	{

		O_CC(filled);
		O_C(centralizedControl);
		O_C(distributedControl);
		O_C(centralizedAudio);
		O_C(distributedAudio);
		O_C(centralizedVideo);
		O_C(distributedVideo);
		O_C(centralizedData);
		O_C(distributedData);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultipointCapability /////////////////////////
 	 VS_H245MultipointCapability :: VS_H245MultipointCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&multicastCapability,0);
		ref[1].Set(&multiUniCastConference,0);
		ref[2].Set(&mediaDistributionCapability,0);
	}
	void VS_H245MultipointCapability::operator=(const VS_H245MultipointCapability& src)
	{

		O_CC(filled);
		O_C(multicastCapability);
		O_C(multiUniCastConference);
		O_C(mediaDistributionCapability);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RTPH263VideoRedundancyFrameMapping /////////////////////////
 	 VS_H245RTPH263VideoRedundancyFrameMapping :: VS_H245RTPH263VideoRedundancyFrameMapping( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&threadNumber,0);
		ref[1].Set(&frameSequence,0);
	}
	void VS_H245RTPH263VideoRedundancyFrameMapping::operator=(const VS_H245RTPH263VideoRedundancyFrameMapping& src)
	{

		O_CC(filled);
		O_C(threadNumber);
		O_C(frameSequence);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping /////////////////////////
 	 VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping::VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_roundrobin : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_custom : return DecodeChoice( buffer , new VS_H245RTPH263VideoRedundancyFrameMapping);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping::operator=(const VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_roundrobin : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_custom : CopyChoice< VS_H245RTPH263VideoRedundancyFrameMapping >(src,*this); return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping::operator VS_H245RTPH263VideoRedundancyFrameMapping *( void )
	{	return dynamic_cast< VS_H245RTPH263VideoRedundancyFrameMapping * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RTPH263VideoRedundancyEncoding_FrameToThreadMapping::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_roundrobin :  dprint4("choice:  VS_AsnNull  ");return;
		case e_custom :  dprint4("choice: VS_H245RTPH263VideoRedundancyFrameMapping ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RTPH263VideoRedundancyEncoding /////////////////////////
 	 VS_H245RTPH263VideoRedundancyEncoding :: VS_H245RTPH263VideoRedundancyEncoding( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&numberOfThreads,0);
		ref[1].Set(&framesBetweenSyncPoints,0);
		ref[2].Set(&frameToThreadMapping,0);
		ref[3].Set(&containedThreads,1);
	}
	void VS_H245RTPH263VideoRedundancyEncoding::operator=(const VS_H245RTPH263VideoRedundancyEncoding& src)
	{

		O_CC(filled);
		O_C(numberOfThreads);
		O_C(framesBetweenSyncPoints);
		O_C(frameToThreadMapping);
		O_C(containedThreads);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245RedundancyEncodingMethod /////////////////////////
 	 VS_H245RedundancyEncodingMethod::VS_H245RedundancyEncodingMethod( void )
	:VS_AsnChoice(2 , 3 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RedundancyEncodingMethod::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_rtpAudioRedundancyEncoding : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_rtpH263VideoRedundancyEncoding : return DecodeChoice( buffer , new VS_H245RTPH263VideoRedundancyEncoding);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RedundancyEncodingMethod::operator=(const VS_H245RedundancyEncodingMethod & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_rtpAudioRedundancyEncoding : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_rtpH263VideoRedundancyEncoding : CopyChoice< VS_H245RTPH263VideoRedundancyEncoding >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245RedundancyEncodingMethod::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245RedundancyEncodingMethod::operator VS_H245RTPH263VideoRedundancyEncoding *( void )
	{	return dynamic_cast< VS_H245RTPH263VideoRedundancyEncoding * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RedundancyEncodingMethod::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_rtpAudioRedundancyEncoding :  dprint4("choice:  VS_AsnNull  ");return;
		case e_rtpH263VideoRedundancyEncoding :  dprint4("choice: VS_H245RTPH263VideoRedundancyEncoding ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RedundancyEncodingCapability /////////////////////////
 	 VS_H245RedundancyEncodingCapability :: VS_H245RedundancyEncodingCapability( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&redundancyEncodingMethod,0);
		ref[1].Set(&primaryEncoding,0);
		ref[2].Set(&secondaryEncoding,1);
	}
	void VS_H245RedundancyEncodingCapability::operator=(const VS_H245RedundancyEncodingCapability& src)
	{

		O_CC(filled);
		O_C(redundancyEncodingMethod);
		O_C(primaryEncoding);
		O_C(secondaryEncoding);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245TransportCapability /////////////////////////
 	 VS_H245TransportCapability :: VS_H245TransportCapability( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandard,1);
		ref[1].Set(&qOSCapabilities,1);
		ref[2].Set(&mediaChannelCapabilities,1);
	}
	void VS_H245TransportCapability::operator=(const VS_H245TransportCapability& src)
	{

		O_CC(filled);
		O_C(nonStandard);
		O_C(qOSCapabilities);
		O_C(mediaChannelCapabilities);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MediaChannelCapability /////////////////////////
 	 VS_H245MediaChannelCapability :: VS_H245MediaChannelCapability( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&mediaTransport,1);
	}
	void VS_H245MediaChannelCapability::operator=(const VS_H245MediaChannelCapability& src)
	{

		O_CC(filled);
		O_C(mediaTransport);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MediaTransportType_Atm_AAL5_compressed /////////////////////////
 	 VS_H245MediaTransportType_Atm_AAL5_compressed :: VS_H245MediaTransportType_Atm_AAL5_compressed( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&variable_delta,0);
	}
	void VS_H245MediaTransportType_Atm_AAL5_compressed::operator=(const VS_H245MediaTransportType_Atm_AAL5_compressed& src)
	{

		O_CC(filled);
		O_C(variable_delta);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MediaTransportType /////////////////////////
 	 VS_H245MediaTransportType::VS_H245MediaTransportType( void )
	:VS_AsnChoice(4 , 5 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MediaTransportType::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_ip_UDP : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_ip_TCP : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_atm_AAL5_UNIDIR : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_atm_AAL5_BIDIR : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_atm_AAL5_compressed : return DecodeChoice( buffer , new VS_H245MediaTransportType_Atm_AAL5_compressed	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MediaTransportType::operator=(const VS_H245MediaTransportType & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_ip_UDP : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_ip_TCP : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_atm_AAL5_UNIDIR : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_atm_AAL5_BIDIR : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_atm_AAL5_compressed : CopyChoice<VS_H245MediaTransportType_Atm_AAL5_compressed	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MediaTransportType::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_ip_UDP :  dprint4("choice:  VS_AsnNull  ");return;
		case e_ip_TCP :  dprint4("choice:  VS_AsnNull  ");return;
		case e_atm_AAL5_UNIDIR :  dprint4("choice:  VS_AsnNull  ");return;
		case e_atm_AAL5_BIDIR :  dprint4("choice:  VS_AsnNull  ");return;
		case e_atm_AAL5_compressed :  dprint4("choice: VS_H245MediaTransportType_Atm_AAL5_compressed	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245QOSCapability /////////////////////////
 	 VS_H245QOSCapability :: VS_H245QOSCapability( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,1);
		ref[1].Set(&rsvpParameters,1);
		ref[2].Set(&atmParameters,1);
	}
	void VS_H245QOSCapability::operator=(const VS_H245QOSCapability& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_C(rsvpParameters);
		O_C(atmParameters);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245ATMParameters /////////////////////////
 	 VS_H245ATMParameters :: VS_H245ATMParameters( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&maxNTUSize,0);
		ref[1].Set(&atmUBR,0);
		ref[2].Set(&atmrtVBR,0);
		ref[3].Set(&atmnrtVBR,0);
		ref[4].Set(&atmABR,0);
		ref[5].Set(&atmCBR,0);
	}
	void VS_H245ATMParameters::operator=(const VS_H245ATMParameters& src)
	{

		O_CC(filled);
		O_C(maxNTUSize);
		O_C(atmUBR);
		O_C(atmrtVBR);
		O_C(atmnrtVBR);
		O_C(atmABR);
		O_C(atmCBR);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245QOSMode /////////////////////////
 	 VS_H245QOSMode::VS_H245QOSMode( void )
	:VS_AsnChoice(2 , 2 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245QOSMode::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_guaranteedQOS : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_controlledLoad : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245QOSMode::operator=(const VS_H245QOSMode & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_guaranteedQOS : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_controlledLoad : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245QOSMode::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_guaranteedQOS :  dprint4("choice:  VS_AsnNull  ");return;
		case e_controlledLoad :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RSVPParameters /////////////////////////
 	 VS_H245RSVPParameters :: VS_H245RSVPParameters( void )
	:VS_AsnSequence(6 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&qosMode,1);
		ref[1].Set(&tokenRate,1);
		ref[2].Set(&bucketSize,1);
		ref[3].Set(&peakRate,1);
		ref[4].Set(&minPoliced,1);
		ref[5].Set(&maxPktSize,1);
	}
	void VS_H245RSVPParameters::operator=(const VS_H245RSVPParameters& src)
	{

		O_CC(filled);
		O_C(qosMode);
		O_C(tokenRate);
		O_C(bucketSize);
		O_C(peakRate);
		O_C(minPoliced);
		O_C(maxPktSize);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MediaPacketizationCapability /////////////////////////
 	 VS_H245MediaPacketizationCapability :: VS_H245MediaPacketizationCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&h261aVideoPacketization,0);
		e_ref[0].Set(&rtpPayloadType,1);
	}
	void VS_H245MediaPacketizationCapability::operator=(const VS_H245MediaPacketizationCapability& src)
	{

		O_CC(filled);
		O_C(h261aVideoPacketization);
		O_C(rtpPayloadType);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H2250Capability_McCapability /////////////////////////
 	 VS_H245H2250Capability_McCapability :: VS_H245H2250Capability_McCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&centralizedConferenceMC,0);
		ref[1].Set(&decentralizedConferenceMC,0);
	}
	void VS_H245H2250Capability_McCapability::operator=(const VS_H245H2250Capability_McCapability& src)
	{

		O_CC(filled);
		O_C(centralizedConferenceMC);
		O_C(decentralizedConferenceMC);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H2250Capability /////////////////////////
 	 VS_H245H2250Capability :: VS_H245H2250Capability( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&maximumAudioDelayJitter,0);
		ref[1].Set(&receiveMultipointCapability,0);
		ref[2].Set(&transmitMultipointCapability,0);
		ref[3].Set(&receiveAndTransmitMultipointCapability,0);
		ref[4].Set(&mcCapability,0);
		ref[5].Set(&rtcpVideoControlCapability,0);
		ref[6].Set(&mediaPacketizationCapability,0);
		e_ref[0].Set(&transportCapability,1);
		e_ref[1].Set(&redundancyEncodingCapability,1);
		e_ref[2].Set(&logicalChannelSwitchingCapability,0);
		e_ref[3].Set(&t120DynamicPortCapability,0);
	}
	void VS_H245H2250Capability::operator=(const VS_H245H2250Capability& src)
	{

		O_CC(filled);
		O_C(maximumAudioDelayJitter);
		O_C(receiveMultipointCapability);
		O_C(transmitMultipointCapability);
		O_C(receiveAndTransmitMultipointCapability);
		O_C(mcCapability);
		O_C(rtcpVideoControlCapability);
		O_C(mediaPacketizationCapability);
		O_C(transportCapability);
		O_C(redundancyEncodingCapability);
		O_C(logicalChannelSwitchingCapability);
		O_C(t120DynamicPortCapability);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245V75Capability /////////////////////////
 	 VS_H245V75Capability :: VS_H245V75Capability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&audioHeader,0);
	}
	void VS_H245V75Capability::operator=(const VS_H245V75Capability& src)
	{

		O_CC(filled);
		O_C(audioHeader);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245V76Capability /////////////////////////
 	 VS_H245V76Capability :: VS_H245V76Capability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&suspendResumeCapabilitywAddress,0);
		ref[1].Set(&suspendResumeCapabilitywoAddress,0);
		ref[2].Set(&rejCapability,0);
		ref[3].Set(&sREJCapability,0);
		ref[4].Set(&mREJCapability,0);
		ref[5].Set(&crc8bitCapability,0);
		ref[6].Set(&crc16bitCapability,0);
		ref[7].Set(&crc32bitCapability,0);
		ref[8].Set(&uihCapability,0);
		ref[9].Set(&numOfDLCS,0);
		ref[10].Set(&twoOctetAddressFieldCapability,0);
		ref[11].Set(&loopBackTestCapability,0);
		ref[12].Set(&n401Capability,0);
		ref[13].Set(&maxWindowSizeCapability,0);
		ref[14].Set(&v75Capability,0);
	}
	void VS_H245V76Capability::operator=(const VS_H245V76Capability& src)
	{

		O_CC(filled);
		O_C(suspendResumeCapabilitywAddress);
		O_C(suspendResumeCapabilitywoAddress);
		O_C(rejCapability);
		O_C(sREJCapability);
		O_C(mREJCapability);
		O_C(crc8bitCapability);
		O_C(crc16bitCapability);
		O_C(crc32bitCapability);
		O_C(uihCapability);
		O_C(numOfDLCS);
		O_C(twoOctetAddressFieldCapability);
		O_C(loopBackTestCapability);
		O_C(n401Capability);
		O_C(maxWindowSizeCapability);
		O_C(v75Capability);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223AnnexCCapability /////////////////////////
 	 VS_H245H223AnnexCCapability :: VS_H245H223AnnexCCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&videoWithAL1M,0);
		ref[1].Set(&videoWithAL2M,0);
		ref[2].Set(&videoWithAL3M,0);
		ref[3].Set(&audioWithAL1M,0);
		ref[4].Set(&audioWithAL2M,0);
		ref[5].Set(&audioWithAL3M,0);
		ref[6].Set(&dataWithAL1M,0);
		ref[7].Set(&dataWithAL2M,0);
		ref[8].Set(&dataWithAL3M,0);
		ref[9].Set(&alpduInterleaving,0);
		ref[10].Set(&maximumAL1MPDUSize,0);
		ref[11].Set(&maximumAL2MSDUSize,0);
		ref[12].Set(&maximumAL3MSDUSize,0);
		e_ref[0].Set(&rsCodeCapability,1);
	}
	void VS_H245H223AnnexCCapability::operator=(const VS_H245H223AnnexCCapability& src)
	{

		O_CC(filled);
		O_C(videoWithAL1M);
		O_C(videoWithAL2M);
		O_C(videoWithAL3M);
		O_C(audioWithAL1M);
		O_C(audioWithAL2M);
		O_C(audioWithAL3M);
		O_C(dataWithAL1M);
		O_C(dataWithAL2M);
		O_C(dataWithAL3M);
		O_C(alpduInterleaving);
		O_C(maximumAL1MPDUSize);
		O_C(maximumAL2MSDUSize);
		O_C(maximumAL3MSDUSize);
		O_C(rsCodeCapability);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223Capability_H223MultiplexTableCapability_Enhanced /////////////////////////
 	 VS_H245H223Capability_H223MultiplexTableCapability_Enhanced :: VS_H245H223Capability_H223MultiplexTableCapability_Enhanced( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&maximumNestingDepth,0);
		ref[1].Set(&maximumElementListSize,0);
		ref[2].Set(&maximumSubElementListSize,0);
	}
	void VS_H245H223Capability_H223MultiplexTableCapability_Enhanced::operator=(const VS_H245H223Capability_H223MultiplexTableCapability_Enhanced& src)
	{

		O_CC(filled);
		O_C(maximumNestingDepth);
		O_C(maximumElementListSize);
		O_C(maximumSubElementListSize);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223Capability_H223MultiplexTableCapability /////////////////////////
 	 VS_H245H223Capability_H223MultiplexTableCapability::VS_H245H223Capability_H223MultiplexTableCapability( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245H223Capability_H223MultiplexTableCapability::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_basic : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_enhanced : return DecodeChoice( buffer , new VS_H245H223Capability_H223MultiplexTableCapability_Enhanced	 );
		default: return false;
		}

	}

	void VS_H245H223Capability_H223MultiplexTableCapability::operator=(const VS_H245H223Capability_H223MultiplexTableCapability & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_basic : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_enhanced : CopyChoice<VS_H245H223Capability_H223MultiplexTableCapability_Enhanced	  >(src,*this);  return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245H223Capability_H223MultiplexTableCapability::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_basic :  dprint4("choice:  VS_AsnNull  ");return;
		case e_enhanced :  dprint4("choice: VS_H245H223Capability_H223MultiplexTableCapability_Enhanced	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H223Capability_MobileOperationTransmitCapability /////////////////////////
 	 VS_H245H223Capability_MobileOperationTransmitCapability :: VS_H245H223Capability_MobileOperationTransmitCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&modeChangeCapability,0);
		ref[1].Set(&h223AnnexA,0);
		ref[2].Set(&h223AnnexADoubleFlag,0);
		ref[3].Set(&h223AnnexB,0);
		ref[4].Set(&h223AnnexBwithHeader,0);
	}
	void VS_H245H223Capability_MobileOperationTransmitCapability::operator=(const VS_H245H223Capability_MobileOperationTransmitCapability& src)
	{

		O_CC(filled);
		O_C(modeChangeCapability);
		O_C(h223AnnexA);
		O_C(h223AnnexADoubleFlag);
		O_C(h223AnnexB);
		O_C(h223AnnexBwithHeader);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223Capability_MobileMultilinkFrameCapability /////////////////////////
 	 VS_H245H223Capability_MobileMultilinkFrameCapability :: VS_H245H223Capability_MobileMultilinkFrameCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&maximumSampleSize,0);
		ref[1].Set(&maximumPayloadLength,0);
	}
	void VS_H245H223Capability_MobileMultilinkFrameCapability::operator=(const VS_H245H223Capability_MobileMultilinkFrameCapability& src)
	{

		O_CC(filled);
		O_C(maximumSampleSize);
		O_C(maximumPayloadLength);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H223Capability /////////////////////////
 	 VS_H245H223Capability :: VS_H245H223Capability( void )
	:VS_AsnSequence(0 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&transportWithI_frames,0);
		ref[1].Set(&videoWithAL1,0);
		ref[2].Set(&videoWithAL2,0);
		ref[3].Set(&videoWithAL3,0);
		ref[4].Set(&audioWithAL1,0);
		ref[5].Set(&audioWithAL2,0);
		ref[6].Set(&audioWithAL3,0);
		ref[7].Set(&dataWithAL1,0);
		ref[8].Set(&dataWithAL2,0);
		ref[9].Set(&dataWithAL3,0);
		ref[10].Set(&maximumAl2SDUSize,0);
		ref[11].Set(&maximumAl3SDUSize,0);
		ref[12].Set(&maximumDelayJitter,0);
		ref[13].Set(&h223MultiplexTableCapability,0);
		e_ref[0].Set(&maxMUXPDUSizeCapability,0);
		e_ref[1].Set(&nsrpSupport,0);
		e_ref[2].Set(&mobileOperationTransmitCapability,1);
		e_ref[3].Set(&h223AnnexCCapability,1);
		e_ref[4].Set(&bitRate,1);
		e_ref[5].Set(&mobileMultilinkFrameCapability,1);
	}
	void VS_H245H223Capability::operator=(const VS_H245H223Capability& src)
	{

		O_CC(filled);
		O_C(transportWithI_frames);
		O_C(videoWithAL1);
		O_C(videoWithAL2);
		O_C(videoWithAL3);
		O_C(audioWithAL1);
		O_C(audioWithAL2);
		O_C(audioWithAL3);
		O_C(dataWithAL1);
		O_C(dataWithAL2);
		O_C(dataWithAL3);
		O_C(maximumAl2SDUSize);
		O_C(maximumAl3SDUSize);
		O_C(maximumDelayJitter);
		O_C(h223MultiplexTableCapability);
		O_C(maxMUXPDUSizeCapability);
		O_C(nsrpSupport);
		O_C(mobileOperationTransmitCapability);
		O_C(h223AnnexCCapability);
		O_C(bitRate);
		O_C(mobileMultilinkFrameCapability);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VCCapability_Aal1 /////////////////////////
 	 VS_H245VCCapability_Aal1 :: VS_H245VCCapability_Aal1( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nullClockRecovery,0);
		ref[1].Set(&srtsClockRecovery,0);
		ref[2].Set(&adaptiveClockRecovery,0);
		ref[3].Set(&nullErrorCorrection,0);
		ref[4].Set(&longInterleaver,0);
		ref[5].Set(&shortInterleaver,0);
		ref[6].Set(&errorCorrectionOnly,0);
		ref[7].Set(&structuredDataTransfer,0);
		ref[8].Set(&partiallyFilledCells,0);
	}
	void VS_H245VCCapability_Aal1::operator=(const VS_H245VCCapability_Aal1& src)
	{

		O_CC(filled);
		O_C(nullClockRecovery);
		O_C(srtsClockRecovery);
		O_C(adaptiveClockRecovery);
		O_C(nullErrorCorrection);
		O_C(longInterleaver);
		O_C(shortInterleaver);
		O_C(errorCorrectionOnly);
		O_C(structuredDataTransfer);
		O_C(partiallyFilledCells);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VCCapability_Aal5 /////////////////////////
 	 VS_H245VCCapability_Aal5 :: VS_H245VCCapability_Aal5( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&forwardMaximumSDUSize,0);
		ref[1].Set(&backwardMaximumSDUSize,0);
	}
	void VS_H245VCCapability_Aal5::operator=(const VS_H245VCCapability_Aal5& src)
	{

		O_CC(filled);
		O_C(forwardMaximumSDUSize);
		O_C(backwardMaximumSDUSize);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates /////////////////////////
 	 VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates :: VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&lowerBitRate,0);
		ref[1].Set(&higherBitRate,0);
	}
	void VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates::operator=(const VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates& src)
	{

		O_CC(filled);
		O_C(lowerBitRate);
		O_C(higherBitRate);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VCCapability_AvailableBitRates_Type /////////////////////////
 	 VS_H245VCCapability_AvailableBitRates_Type::VS_H245VCCapability_AvailableBitRates_Type( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245VCCapability_AvailableBitRates_Type::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_singleBitRate : return DecodeChoice( buffer , new TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  );
		case e_rangeOfBitRates : return DecodeChoice( buffer , new VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates	 );
		default: return false;
		}

	}

	void VS_H245VCCapability_AvailableBitRates_Type::operator=(const VS_H245VCCapability_AvailableBitRates_Type & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_singleBitRate : CopyChoice<TemplInteger<1,65535,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_rangeOfBitRates : CopyChoice<VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245VCCapability_AvailableBitRates_Type::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_singleBitRate :  dprint4("choice: TemplInteger<1,65535,VS_Asn::FixedConstraint,0>  ");return;
		case e_rangeOfBitRates :  dprint4("choice: VS_H245VCCapability_AvailableBitRates_Type_RangeOfBitRates	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245VCCapability_AvailableBitRates /////////////////////////
 	 VS_H245VCCapability_AvailableBitRates :: VS_H245VCCapability_AvailableBitRates( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&type,0);
	}
	void VS_H245VCCapability_AvailableBitRates::operator=(const VS_H245VCCapability_AvailableBitRates& src)
	{

		O_CC(filled);
		O_C(type);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VCCapability_Aal1ViaGateway /////////////////////////
 	 VS_H245VCCapability_Aal1ViaGateway :: VS_H245VCCapability_Aal1ViaGateway( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&gatewayAddress,0);
		ref[1].Set(&nullClockRecovery,0);
		ref[2].Set(&srtsClockRecovery,0);
		ref[3].Set(&adaptiveClockRecovery,0);
		ref[4].Set(&nullErrorCorrection,0);
		ref[5].Set(&longInterleaver,0);
		ref[6].Set(&shortInterleaver,0);
		ref[7].Set(&errorCorrectionOnly,0);
		ref[8].Set(&structuredDataTransfer,0);
		ref[9].Set(&partiallyFilledCells,0);
	}
	void VS_H245VCCapability_Aal1ViaGateway::operator=(const VS_H245VCCapability_Aal1ViaGateway& src)
	{

		O_CC(filled);
		O_C(gatewayAddress);
		O_C(nullClockRecovery);
		O_C(srtsClockRecovery);
		O_C(adaptiveClockRecovery);
		O_C(nullErrorCorrection);
		O_C(longInterleaver);
		O_C(shortInterleaver);
		O_C(errorCorrectionOnly);
		O_C(structuredDataTransfer);
		O_C(partiallyFilledCells);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245VCCapability /////////////////////////
 	 VS_H245VCCapability :: VS_H245VCCapability( void )
	:VS_AsnSequence(2 , ref , basic_root, e_ref , extension_root , 1 )
	{
		ref[0].Set(&aal1,1);
		ref[1].Set(&aal5,1);
		ref[2].Set(&transportStream,0);
		ref[3].Set(&programStream,0);
		ref[4].Set(&availableBitRates,0);
		e_ref[0].Set(&aal1ViaGateway,1);
	}
	void VS_H245VCCapability::operator=(const VS_H245VCCapability& src)
	{

		O_CC(filled);
		O_C(aal1);
		O_C(aal5);
		O_C(transportStream);
		O_C(programStream);
		O_C(availableBitRates);
		O_C(aal1ViaGateway);
		O_CSA(e_ref, extension_root);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245H222Capability /////////////////////////
 	 VS_H245H222Capability :: VS_H245H222Capability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&numberOfVCs,0);
		ref[1].Set(&vcCapability,0);
	}
	void VS_H245H222Capability::operator=(const VS_H245H222Capability& src)
	{

		O_CC(filled);
		O_C(numberOfVCs);
		O_C(vcCapability);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MultiplexCapability /////////////////////////
 	 VS_H245MultiplexCapability::VS_H245MultiplexCapability( void )
	:VS_AsnChoice(4 , 6 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultiplexCapability::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_h222Capability : return DecodeChoice( buffer , new VS_H245H222Capability);
		case e_h223Capability : return DecodeChoice( buffer , new VS_H245H223Capability);
		case e_v76Capability : return DecodeChoice( buffer , new VS_H245V76Capability);
		case e_h2250Capability : return DecodeChoice( buffer , new VS_H245H2250Capability);
		case e_genericMultiplexCapability : return DecodeChoice( buffer , new VS_H245GenericCapability);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultiplexCapability::operator=(const VS_H245MultiplexCapability & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_h222Capability : CopyChoice< VS_H245H222Capability >(src,*this); return;
		case e_h223Capability : CopyChoice< VS_H245H223Capability >(src,*this); return;
		case e_v76Capability : CopyChoice< VS_H245V76Capability >(src,*this); return;
		case e_h2250Capability : CopyChoice< VS_H245H2250Capability >(src,*this); return;
		case e_genericMultiplexCapability : CopyChoice< VS_H245GenericCapability >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MultiplexCapability::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245MultiplexCapability::operator VS_H245H222Capability *( void )
	{	return dynamic_cast< VS_H245H222Capability * >(choice);    }

 	VS_H245MultiplexCapability::operator VS_H245H223Capability *( void )
	{	return dynamic_cast< VS_H245H223Capability * >(choice);    }

 	VS_H245MultiplexCapability::operator VS_H245V76Capability *( void )
	{	return dynamic_cast< VS_H245V76Capability * >(choice);    }

 	VS_H245MultiplexCapability::operator VS_H245H2250Capability *( void )
	{	return dynamic_cast< VS_H245H2250Capability * >(choice);    }

 	VS_H245MultiplexCapability::operator VS_H245GenericCapability *( void )
	{	return dynamic_cast< VS_H245GenericCapability * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultiplexCapability::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_h222Capability :  dprint4("choice: VS_H245H222Capability ");return;
		case e_h223Capability :  dprint4("choice: VS_H245H223Capability ");return;
		case e_v76Capability :  dprint4("choice: VS_H245V76Capability ");return;
		case e_h2250Capability :  dprint4("choice: VS_H245H2250Capability ");return;
		case e_genericMultiplexCapability :  dprint4("choice: VS_H245GenericCapability ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245H235SecurityCapability /////////////////////////
 	 VS_H245H235SecurityCapability :: VS_H245H235SecurityCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&encryptionAuthenticationAndIntegrity,0);
		ref[1].Set(&mediaCapability,0);
	}
	void VS_H245H235SecurityCapability::operator=(const VS_H245H235SecurityCapability& src)
	{

		O_CC(filled);
		O_C(encryptionAuthenticationAndIntegrity);
		O_C(mediaCapability);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245Capability_H233EncryptionReceiveCapability /////////////////////////
 	 VS_H245Capability_H233EncryptionReceiveCapability :: VS_H245Capability_H233EncryptionReceiveCapability( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&h233IVResponseTime,0);
	}
	void VS_H245Capability_H233EncryptionReceiveCapability::operator=(const VS_H245Capability_H233EncryptionReceiveCapability& src)
	{

		O_CC(filled);
		O_C(h233IVResponseTime);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245Capability /////////////////////////
 	 VS_H245Capability::VS_H245Capability( void )
	:VS_AsnChoice(12 , 29 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245Capability::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardParameter);
		case e_receiveVideoCapability : return DecodeChoice( buffer , new VS_H245VideoCapability);
		case e_transmitVideoCapability : return DecodeChoice( buffer , new VS_H245VideoCapability);
		case e_receiveAndTransmitVideoCapability : return DecodeChoice( buffer , new VS_H245VideoCapability);
		case e_receiveAudioCapability : return DecodeChoice( buffer , new VS_H245AudioCapability);
		case e_transmitAudioCapability : return DecodeChoice( buffer , new VS_H245AudioCapability);
		case e_receiveAndTransmitAudioCapability : return DecodeChoice( buffer , new VS_H245AudioCapability);
		case e_receiveDataApplicationCapability : return DecodeChoice( buffer , new VS_H245DataApplicationCapability);
		case e_transmitDataApplicationCapability : return DecodeChoice( buffer , new VS_H245DataApplicationCapability);
		case e_receiveAndTransmitDataApplicationCapability : return DecodeChoice( buffer , new VS_H245DataApplicationCapability);
		case e_h233EncryptionTransmitCapability : return DecodeChoice( buffer , new  VS_AsnBoolean  );
		case e_h233EncryptionReceiveCapability : return DecodeChoice( buffer , new VS_H245Capability_H233EncryptionReceiveCapability	 );
		case e_conferenceCapability : return DecodeChoice( buffer , new VS_H245ConferenceCapability);
		case e_h235SecurityCapability : return DecodeChoice( buffer , new VS_H245H235SecurityCapability);
		case e_maxPendingReplacementFor : return DecodeChoice( buffer , new TemplInteger<0,255,VS_Asn::FixedConstraint,0>  );
		case e_receiveUserInputCapability : return DecodeChoice( buffer , new VS_H245UserInputCapability);
		case e_transmitUserInputCapability : return DecodeChoice( buffer , new VS_H245UserInputCapability);
		case e_receiveAndTransmitUserInputCapability : return DecodeChoice( buffer , new VS_H245UserInputCapability);
		case e_genericControlCapability : return DecodeChoice( buffer , new VS_H245GenericCapability);
		case e_receiveMultiplexedStreamCapability : return DecodeChoice( buffer , new VS_H245MultiplexedStreamCapability);
		case e_transmitMultiplexedStreamCapability : return DecodeChoice( buffer , new VS_H245MultiplexedStreamCapability);
		case e_receiveAndTransmitMultiplexedStreamCapability : return DecodeChoice( buffer , new VS_H245MultiplexedStreamCapability);
		case e_receiveRTPAudioTelephonyEventCapability : return DecodeChoice( buffer , new VS_H245AudioTelephonyEventCapability);
		case e_receiveRTPAudioToneCapability : return DecodeChoice( buffer , new VS_H245AudioToneCapability);
		case e_depFfecCapability : return DecodeChoice( buffer , new VS_H245DepFECCapability);
		case e_multiplePayloadStreamCapability : return DecodeChoice( buffer , new VS_H245MultiplePayloadStreamCapability);
		case e_fecCapability : return DecodeChoice( buffer , new VS_H245FECCapability);
		case e_redundancyEncodingCap : return DecodeChoice( buffer , new VS_H245RedundancyEncodingCapability);
		case e_oneOfCapabilities : return DecodeChoice( buffer , new VS_H245AlternativeCapabilitySet);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245Capability::operator=(const VS_H245Capability & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardParameter >(src,*this); return;
		case e_receiveVideoCapability : CopyChoice< VS_H245VideoCapability >(src,*this); return;
		case e_transmitVideoCapability : CopyChoice< VS_H245VideoCapability >(src,*this); return;
		case e_receiveAndTransmitVideoCapability : CopyChoice< VS_H245VideoCapability >(src,*this); return;
		case e_receiveAudioCapability : CopyChoice< VS_H245AudioCapability >(src,*this); return;
		case e_transmitAudioCapability : CopyChoice< VS_H245AudioCapability >(src,*this); return;
		case e_receiveAndTransmitAudioCapability : CopyChoice< VS_H245AudioCapability >(src,*this); return;
		case e_receiveDataApplicationCapability : CopyChoice< VS_H245DataApplicationCapability >(src,*this); return;
		case e_transmitDataApplicationCapability : CopyChoice< VS_H245DataApplicationCapability >(src,*this); return;
		case e_receiveAndTransmitDataApplicationCapability : CopyChoice< VS_H245DataApplicationCapability >(src,*this); return;
		case e_h233EncryptionTransmitCapability : CopyChoice< VS_AsnBoolean   >(src,*this);  return;
		case e_h233EncryptionReceiveCapability : CopyChoice<VS_H245Capability_H233EncryptionReceiveCapability	  >(src,*this);  return;
		case e_conferenceCapability : CopyChoice< VS_H245ConferenceCapability >(src,*this); return;
		case e_h235SecurityCapability : CopyChoice< VS_H245H235SecurityCapability >(src,*this); return;
		case e_maxPendingReplacementFor : CopyChoice<TemplInteger<0,255,VS_Asn::FixedConstraint,0>   >(src,*this);  return;
		case e_receiveUserInputCapability : CopyChoice< VS_H245UserInputCapability >(src,*this); return;
		case e_transmitUserInputCapability : CopyChoice< VS_H245UserInputCapability >(src,*this); return;
		case e_receiveAndTransmitUserInputCapability : CopyChoice< VS_H245UserInputCapability >(src,*this); return;
		case e_genericControlCapability : CopyChoice< VS_H245GenericCapability >(src,*this); return;
		case e_receiveMultiplexedStreamCapability : CopyChoice< VS_H245MultiplexedStreamCapability >(src,*this); return;
		case e_transmitMultiplexedStreamCapability : CopyChoice< VS_H245MultiplexedStreamCapability >(src,*this); return;
		case e_receiveAndTransmitMultiplexedStreamCapability : CopyChoice< VS_H245MultiplexedStreamCapability >(src,*this); return;
		case e_receiveRTPAudioTelephonyEventCapability : CopyChoice< VS_H245AudioTelephonyEventCapability >(src,*this); return;
		case e_receiveRTPAudioToneCapability : CopyChoice< VS_H245AudioToneCapability >(src,*this); return;
		case e_depFfecCapability : CopyChoice< VS_H245DepFECCapability >(src,*this); return;
		case e_multiplePayloadStreamCapability : CopyChoice< VS_H245MultiplePayloadStreamCapability >(src,*this); return;
		case e_fecCapability : CopyChoice< VS_H245FECCapability >(src,*this); return;
		case e_redundancyEncodingCap : CopyChoice< VS_H245RedundancyEncodingCapability >(src,*this); return;
		case e_oneOfCapabilities : CopyChoice< VS_H245AlternativeCapabilitySet >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245Capability::operator VS_H245NonStandardParameter *( void )
	{	return dynamic_cast< VS_H245NonStandardParameter * >(choice);    }

 	VS_H245Capability::operator VS_H245VideoCapability *( void )
	{	return dynamic_cast< VS_H245VideoCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245AudioCapability *( void )
	{	return dynamic_cast< VS_H245AudioCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245DataApplicationCapability *( void )
	{	return dynamic_cast< VS_H245DataApplicationCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245ConferenceCapability *( void )
	{	return dynamic_cast< VS_H245ConferenceCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245H235SecurityCapability *( void )
	{	return dynamic_cast< VS_H245H235SecurityCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245UserInputCapability *( void )
	{	return dynamic_cast< VS_H245UserInputCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245GenericCapability *( void )
	{	return dynamic_cast< VS_H245GenericCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245MultiplexedStreamCapability *( void )
	{	return dynamic_cast< VS_H245MultiplexedStreamCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245AudioTelephonyEventCapability *( void )
	{	return dynamic_cast< VS_H245AudioTelephonyEventCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245AudioToneCapability *( void )
	{	return dynamic_cast< VS_H245AudioToneCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245DepFECCapability *( void )
	{	return dynamic_cast< VS_H245DepFECCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245MultiplePayloadStreamCapability *( void )
	{	return dynamic_cast< VS_H245MultiplePayloadStreamCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245FECCapability *( void )
	{	return dynamic_cast< VS_H245FECCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245RedundancyEncodingCapability *( void )
	{	return dynamic_cast< VS_H245RedundancyEncodingCapability * >(choice);    }

 	VS_H245Capability::operator VS_H245AlternativeCapabilitySet *( void )
	{	return dynamic_cast< VS_H245AlternativeCapabilitySet * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245Capability::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardParameter ");return;
		case e_receiveVideoCapability :  dprint4("choice: VS_H245VideoCapability ");return;
		case e_transmitVideoCapability :  dprint4("choice: VS_H245VideoCapability ");return;
		case e_receiveAndTransmitVideoCapability :  dprint4("choice: VS_H245VideoCapability ");return;
		case e_receiveAudioCapability :  dprint4("choice: VS_H245AudioCapability ");return;
		case e_transmitAudioCapability :  dprint4("choice: VS_H245AudioCapability ");return;
		case e_receiveAndTransmitAudioCapability :  dprint4("choice: VS_H245AudioCapability ");return;
		case e_receiveDataApplicationCapability :  dprint4("choice: VS_H245DataApplicationCapability ");return;
		case e_transmitDataApplicationCapability :  dprint4("choice: VS_H245DataApplicationCapability ");return;
		case e_receiveAndTransmitDataApplicationCapability :  dprint4("choice: VS_H245DataApplicationCapability ");return;
		case e_h233EncryptionTransmitCapability :  dprint4("choice:  VS_AsnBoolean  ");return;
		case e_h233EncryptionReceiveCapability :  dprint4("choice: VS_H245Capability_H233EncryptionReceiveCapability	 ");return;
		case e_conferenceCapability :  dprint4("choice: VS_H245ConferenceCapability ");return;
		case e_h235SecurityCapability :  dprint4("choice: VS_H245H235SecurityCapability ");return;
		case e_maxPendingReplacementFor :  dprint4("choice: TemplInteger<0,255,VS_Asn::FixedConstraint,0>  ");return;
		case e_receiveUserInputCapability :  dprint4("choice: VS_H245UserInputCapability ");return;
		case e_transmitUserInputCapability :  dprint4("choice: VS_H245UserInputCapability ");return;
		case e_receiveAndTransmitUserInputCapability :  dprint4("choice: VS_H245UserInputCapability ");return;
		case e_genericControlCapability :  dprint4("choice: VS_H245GenericCapability ");return;
		case e_receiveMultiplexedStreamCapability :  dprint4("choice: VS_H245MultiplexedStreamCapability ");return;
		case e_transmitMultiplexedStreamCapability :  dprint4("choice: VS_H245MultiplexedStreamCapability ");return;
		case e_receiveAndTransmitMultiplexedStreamCapability :  dprint4("choice: VS_H245MultiplexedStreamCapability ");return;
		case e_receiveRTPAudioTelephonyEventCapability :  dprint4("choice: VS_H245AudioTelephonyEventCapability ");return;
		case e_receiveRTPAudioToneCapability :  dprint4("choice: VS_H245AudioToneCapability ");return;
		case e_depFfecCapability :  dprint4("choice: VS_H245DepFECCapability ");return;
		case e_multiplePayloadStreamCapability :  dprint4("choice: VS_H245MultiplePayloadStreamCapability ");return;
		case e_fecCapability :  dprint4("choice: VS_H245FECCapability ");return;
		case e_redundancyEncodingCap :  dprint4("choice: VS_H245RedundancyEncodingCapability ");return;
		case e_oneOfCapabilities :  dprint4("choice: VS_H245AlternativeCapabilitySet ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245TerminalCapabilitySetRelease /////////////////////////
 	 VS_H245TerminalCapabilitySetRelease :: VS_H245TerminalCapabilitySetRelease( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr , extension_root , 1 )
	{
	}
	void VS_H245TerminalCapabilitySetRelease::operator=(const VS_H245TerminalCapabilitySetRelease& src)
	{

		O_CC(filled);
		O_CP(e_ref);  O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded /////////////////////////
 	 VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded::VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_highestEntryNumberProcessed : return DecodeChoice( buffer , new VS_H245CapabilityTableEntryNumber);
		case e_noneProcessed : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded::operator=(const VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_highestEntryNumberProcessed : CopyChoice< VS_H245CapabilityTableEntryNumber >(src,*this); return;
		case e_noneProcessed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded::operator VS_H245CapabilityTableEntryNumber *( void )
	{	return dynamic_cast< VS_H245CapabilityTableEntryNumber * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_highestEntryNumberProcessed :  dprint4("choice: VS_H245CapabilityTableEntryNumber ");return;
		case e_noneProcessed :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245TerminalCapabilitySetReject_Cause /////////////////////////
 	 VS_H245TerminalCapabilitySetReject_Cause::VS_H245TerminalCapabilitySetReject_Cause( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245TerminalCapabilitySetReject_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_unspecified : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_undefinedTableEntryUsed : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_descriptorCapacityExceeded : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_tableEntryCapacityExceeded : return DecodeChoice( buffer , new VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded	 );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245TerminalCapabilitySetReject_Cause::operator=(const VS_H245TerminalCapabilitySetReject_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_unspecified : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_undefinedTableEntryUsed : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_descriptorCapacityExceeded : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_tableEntryCapacityExceeded : CopyChoice<VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded	  >(src,*this);  return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245TerminalCapabilitySetReject_Cause::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_unspecified :  dprint4("choice:  VS_AsnNull  ");return;
		case e_undefinedTableEntryUsed :  dprint4("choice:  VS_AsnNull  ");return;
		case e_descriptorCapacityExceeded :  dprint4("choice:  VS_AsnNull  ");return;
		case e_tableEntryCapacityExceeded :  dprint4("choice: VS_H245TerminalCapabilitySetReject_Cause_TableEntryCapacityExceeded	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245TerminalCapabilitySetReject /////////////////////////
 	 VS_H245TerminalCapabilitySetReject :: VS_H245TerminalCapabilitySetReject( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&cause,0);
	}
	void VS_H245TerminalCapabilitySetReject::operator=(const VS_H245TerminalCapabilitySetReject& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(cause);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245TerminalCapabilitySetAck /////////////////////////
 	 VS_H245TerminalCapabilitySetAck :: VS_H245TerminalCapabilitySetAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
	}
	void VS_H245TerminalCapabilitySetAck::operator=(const VS_H245TerminalCapabilitySetAck& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CapabilityDescriptor /////////////////////////
 	 VS_H245CapabilityDescriptor :: VS_H245CapabilityDescriptor( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&capabilityDescriptorNumber,0);
		ref[1].Set(&simultaneousCapabilities,1);
	}
	void VS_H245CapabilityDescriptor::operator=(const VS_H245CapabilityDescriptor& src)
	{

		O_CC(filled);
		O_C(capabilityDescriptorNumber);
		O_C(simultaneousCapabilities);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245CapabilityTableEntry /////////////////////////
 	 VS_H245CapabilityTableEntry :: VS_H245CapabilityTableEntry( void )
	:VS_AsnSequence(1 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&capabilityTableEntryNumber,0);
		ref[1].Set(&capability,1);
	}
	void VS_H245CapabilityTableEntry::operator=(const VS_H245CapabilityTableEntry& src)
	{

		O_CC(filled);
		O_C(capabilityTableEntryNumber);
		O_C(capability);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245TerminalCapabilitySet /////////////////////////
 	 VS_H245TerminalCapabilitySet :: VS_H245TerminalCapabilitySet( void )
	:VS_AsnSequence(3 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&sequenceNumber,0);
		ref[1].Set(&protocolIdentifier,0);
		ref[2].Set(&multiplexCapability,1);
		ref[3].Set(&capabilityTable,1);
		ref[4].Set(&capabilityDescriptors,1);
	}
	void VS_H245TerminalCapabilitySet::operator=(const VS_H245TerminalCapabilitySet& src)
	{

		O_CC(filled);
		O_C(sequenceNumber);
		O_C(protocolIdentifier);
		O_C(multiplexCapability);
		O_C(capabilityTable);
		O_C(capabilityDescriptors);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MasterSlaveDeterminationRelease /////////////////////////
 	 VS_H245MasterSlaveDeterminationRelease :: VS_H245MasterSlaveDeterminationRelease( void )
	:VS_AsnSequence(0 , nullptr , basic_root, nullptr , extension_root , 1 )
	{
	}
	void VS_H245MasterSlaveDeterminationRelease::operator=(const VS_H245MasterSlaveDeterminationRelease& src)
	{

		O_CC(filled);
		O_CP(e_ref);  O_CP(ref);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MasterSlaveDeterminationReject_Cause /////////////////////////
 	 VS_H245MasterSlaveDeterminationReject_Cause::VS_H245MasterSlaveDeterminationReject_Cause( void )
	:VS_AsnChoice(1 , 1 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MasterSlaveDeterminationReject_Cause::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_identicalNumbers : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MasterSlaveDeterminationReject_Cause::operator=(const VS_H245MasterSlaveDeterminationReject_Cause & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_identicalNumbers : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MasterSlaveDeterminationReject_Cause::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_identicalNumbers :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MasterSlaveDeterminationReject /////////////////////////
 	 VS_H245MasterSlaveDeterminationReject :: VS_H245MasterSlaveDeterminationReject( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&cause,0);
	}
	void VS_H245MasterSlaveDeterminationReject::operator=(const VS_H245MasterSlaveDeterminationReject& src)
	{

		O_CC(filled);
		O_C(cause);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MasterSlaveDeterminationAck_Decision /////////////////////////
 	 VS_H245MasterSlaveDeterminationAck_Decision::VS_H245MasterSlaveDeterminationAck_Decision( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MasterSlaveDeterminationAck_Decision::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_master : return DecodeChoice( buffer , new  VS_AsnNull  );
		case e_slave : return DecodeChoice( buffer , new  VS_AsnNull  );
		default: return false;
		}

	}

	void VS_H245MasterSlaveDeterminationAck_Decision::operator=(const VS_H245MasterSlaveDeterminationAck_Decision & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_master : CopyChoice< VS_AsnNull   >(src,*this);  return;
		case e_slave : CopyChoice< VS_AsnNull   >(src,*this);  return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MasterSlaveDeterminationAck_Decision::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_master :  dprint4("choice:  VS_AsnNull  ");return;
		case e_slave :  dprint4("choice:  VS_AsnNull  ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MasterSlaveDeterminationAck /////////////////////////
 	 VS_H245MasterSlaveDeterminationAck :: VS_H245MasterSlaveDeterminationAck( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&decision,0);
	}
	void VS_H245MasterSlaveDeterminationAck::operator=(const VS_H245MasterSlaveDeterminationAck& src)
	{

		O_CC(filled);
		O_C(decision);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245MasterSlaveDetermination /////////////////////////
 	 VS_H245MasterSlaveDetermination :: VS_H245MasterSlaveDetermination( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&terminalType,0);
		ref[1].Set(&statusDeterminationNumber,0);
	}
	void VS_H245MasterSlaveDetermination::operator=(const VS_H245MasterSlaveDetermination& src)
	{

		O_CC(filled);
		O_C(terminalType);
		O_C(statusDeterminationNumber);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NonStandardIdentifier_H221NonStandard /////////////////////////
 	 VS_H245NonStandardIdentifier_H221NonStandard :: VS_H245NonStandardIdentifier_H221NonStandard( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&t35CountryCode,0);
		ref[1].Set(&t35Extension,0);
		ref[2].Set(&manufacturerCode,0);
	}
	void VS_H245NonStandardIdentifier_H221NonStandard::operator=(const VS_H245NonStandardIdentifier_H221NonStandard& src)
	{

		O_CC(filled);
		O_C(t35CountryCode);
		O_C(t35Extension);
		O_C(manufacturerCode);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NonStandardIdentifier /////////////////////////
 	 VS_H245NonStandardIdentifier::VS_H245NonStandardIdentifier( void )
	:VS_AsnChoice(2 , 2 , 0 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245NonStandardIdentifier::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_object : return DecodeChoice( buffer , new  VS_AsnObjectId  );
		case e_h221NonStandard : return DecodeChoice( buffer , new VS_H245NonStandardIdentifier_H221NonStandard	 );
		default: return false;
		}

	}

	void VS_H245NonStandardIdentifier::operator=(const VS_H245NonStandardIdentifier & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_object : CopyChoice< VS_AsnObjectId   >(src,*this);  return;
		case e_h221NonStandard : CopyChoice<VS_H245NonStandardIdentifier_H221NonStandard	  >(src,*this);  return;
		default:		return;
		}

	}

 /////////////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245NonStandardIdentifier::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_object :  dprint4("choice:  VS_AsnObjectId  ");return;
		case e_h221NonStandard :  dprint4("choice: VS_H245NonStandardIdentifier_H221NonStandard	 ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245NonStandardParameter /////////////////////////
 	 VS_H245NonStandardParameter :: VS_H245NonStandardParameter( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 0 )
	{
		ref[0].Set(&nonStandardIdentifier,0);
		ref[1].Set(&data,0);
	}
	void VS_H245NonStandardParameter::operator=(const VS_H245NonStandardParameter& src)
	{

		O_CC(filled);
		O_C(nonStandardIdentifier);
		O_C(data);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245NonStandardMessage /////////////////////////
 	 VS_H245NonStandardMessage :: VS_H245NonStandardMessage( void )
	:VS_AsnSequence(0 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&nonStandardData,0);
	}
	void VS_H245NonStandardMessage::operator=(const VS_H245NonStandardMessage& src)
	{

		O_CC(filled);
		O_C(nonStandardData);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245GenericMessage /////////////////////////
 	 VS_H245GenericMessage :: VS_H245GenericMessage( void )
	:VS_AsnSequence(2 , ref , basic_root, nullptr , extension_root , 1 )
	{
		ref[0].Set(&messageIdentifier,0);
		ref[1].Set(&subMessageIdentifier,1);
		ref[2].Set(&messageContent,1);
	}
	void VS_H245GenericMessage::operator=(const VS_H245GenericMessage& src)
	{

		O_CC(filled);
		O_C(messageIdentifier);
		O_C(subMessageIdentifier);
		O_C(messageContent);
		O_CP(e_ref);  O_CSA(ref, basic_root);
	}

 /////////////////////////////////////////////////////////////////////////////////////////

//////////////////////CLASS VS_H245IndicationMessage /////////////////////////
 	 VS_H245IndicationMessage::VS_H245IndicationMessage( void )
	:VS_AsnChoice(14 , 24 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245IndicationMessage::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_functionNotUnderstood : return DecodeChoice( buffer , new VS_H245FunctionNotUnderstood);
		case e_masterSlaveDeterminationRelease : return DecodeChoice( buffer , new VS_H245MasterSlaveDeterminationRelease);
		case e_terminalCapabilitySetRelease : return DecodeChoice( buffer , new VS_H245TerminalCapabilitySetRelease);
		case e_openLogicalChannelConfirm : return DecodeChoice( buffer , new VS_H245OpenLogicalChannelConfirm);
		case e_requestChannelCloseRelease : return DecodeChoice( buffer , new VS_H245RequestChannelCloseRelease);
		case e_multiplexEntrySendRelease : return DecodeChoice( buffer , new VS_H245MultiplexEntrySendRelease);
		case e_requestMultiplexEntryRelease : return DecodeChoice( buffer , new VS_H245RequestMultiplexEntryRelease);
		case e_requestModeRelease : return DecodeChoice( buffer , new VS_H245RequestModeRelease);
		case e_miscellaneousIndication : return DecodeChoice( buffer , new VS_H245MiscellaneousIndication);
		case e_jitterIndication : return DecodeChoice( buffer , new VS_H245JitterIndication);
		case e_h223SkewIndication : return DecodeChoice( buffer , new VS_H245H223SkewIndication);
		case e_newATMVCIndication : return DecodeChoice( buffer , new VS_H245NewATMVCIndication);
		case e_userInput : return DecodeChoice( buffer , new VS_H245UserInputIndication);
		case e_h2250MaximumSkewIndication : return DecodeChoice( buffer , new VS_H245H2250MaximumSkewIndication);
		case e_mcLocationIndication : return DecodeChoice( buffer , new VS_H245MCLocationIndication);
		case e_conferenceIndication : return DecodeChoice( buffer , new VS_H245ConferenceIndication);
		case e_vendorIdentification : return DecodeChoice( buffer , new VS_H245VendorIdentification);
		case e_functionNotSupported : return DecodeChoice( buffer , new VS_H245FunctionNotSupported);
		case e_multilinkIndication : return DecodeChoice( buffer , new VS_H245MultilinkIndication);
		case e_logicalChannelRateRelease : return DecodeChoice( buffer , new VS_H245LogicalChannelRateRelease);
		case e_flowControlIndication : return DecodeChoice( buffer , new VS_H245FlowControlIndication);
		case e_mobileMultilinkReconfigurationIndication : return DecodeChoice( buffer , new VS_H245MobileMultilinkReconfigurationIndication);
		case e_genericIndication : return DecodeChoice( buffer , new VS_H245GenericMessage);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245IndicationMessage::operator=(const VS_H245IndicationMessage & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_functionNotUnderstood : CopyChoice< VS_H245FunctionNotUnderstood >(src,*this); return;
		case e_masterSlaveDeterminationRelease : CopyChoice< VS_H245MasterSlaveDeterminationRelease >(src,*this); return;
		case e_terminalCapabilitySetRelease : CopyChoice< VS_H245TerminalCapabilitySetRelease >(src,*this); return;
		case e_openLogicalChannelConfirm : CopyChoice< VS_H245OpenLogicalChannelConfirm >(src,*this); return;
		case e_requestChannelCloseRelease : CopyChoice< VS_H245RequestChannelCloseRelease >(src,*this); return;
		case e_multiplexEntrySendRelease : CopyChoice< VS_H245MultiplexEntrySendRelease >(src,*this); return;
		case e_requestMultiplexEntryRelease : CopyChoice< VS_H245RequestMultiplexEntryRelease >(src,*this); return;
		case e_requestModeRelease : CopyChoice< VS_H245RequestModeRelease >(src,*this); return;
		case e_miscellaneousIndication : CopyChoice< VS_H245MiscellaneousIndication >(src,*this); return;
		case e_jitterIndication : CopyChoice< VS_H245JitterIndication >(src,*this); return;
		case e_h223SkewIndication : CopyChoice< VS_H245H223SkewIndication >(src,*this); return;
		case e_newATMVCIndication : CopyChoice< VS_H245NewATMVCIndication >(src,*this); return;
		case e_userInput : CopyChoice< VS_H245UserInputIndication >(src,*this); return;
		case e_h2250MaximumSkewIndication : CopyChoice< VS_H245H2250MaximumSkewIndication >(src,*this); return;
		case e_mcLocationIndication : CopyChoice< VS_H245MCLocationIndication >(src,*this); return;
		case e_conferenceIndication : CopyChoice< VS_H245ConferenceIndication >(src,*this); return;
		case e_vendorIdentification : CopyChoice< VS_H245VendorIdentification >(src,*this); return;
		case e_functionNotSupported : CopyChoice< VS_H245FunctionNotSupported >(src,*this); return;
		case e_multilinkIndication : CopyChoice< VS_H245MultilinkIndication >(src,*this); return;
		case e_logicalChannelRateRelease : CopyChoice< VS_H245LogicalChannelRateRelease >(src,*this); return;
		case e_flowControlIndication : CopyChoice< VS_H245FlowControlIndication >(src,*this); return;
		case e_mobileMultilinkReconfigurationIndication : CopyChoice< VS_H245MobileMultilinkReconfigurationIndication >(src,*this); return;
		case e_genericIndication : CopyChoice< VS_H245GenericMessage >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245IndicationMessage::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245FunctionNotUnderstood *( void )
	{	return dynamic_cast< VS_H245FunctionNotUnderstood * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245MasterSlaveDeterminationRelease *( void )
	{	return dynamic_cast< VS_H245MasterSlaveDeterminationRelease * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245TerminalCapabilitySetRelease *( void )
	{	return dynamic_cast< VS_H245TerminalCapabilitySetRelease * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245OpenLogicalChannelConfirm *( void )
	{	return dynamic_cast< VS_H245OpenLogicalChannelConfirm * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245RequestChannelCloseRelease *( void )
	{	return dynamic_cast< VS_H245RequestChannelCloseRelease * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245MultiplexEntrySendRelease *( void )
	{	return dynamic_cast< VS_H245MultiplexEntrySendRelease * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245RequestMultiplexEntryRelease *( void )
	{	return dynamic_cast< VS_H245RequestMultiplexEntryRelease * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245RequestModeRelease *( void )
	{	return dynamic_cast< VS_H245RequestModeRelease * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245MiscellaneousIndication *( void )
	{	return dynamic_cast< VS_H245MiscellaneousIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245JitterIndication *( void )
	{	return dynamic_cast< VS_H245JitterIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245H223SkewIndication *( void )
	{	return dynamic_cast< VS_H245H223SkewIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245NewATMVCIndication *( void )
	{	return dynamic_cast< VS_H245NewATMVCIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245UserInputIndication *( void )
	{	return dynamic_cast< VS_H245UserInputIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245H2250MaximumSkewIndication *( void )
	{	return dynamic_cast< VS_H245H2250MaximumSkewIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245MCLocationIndication *( void )
	{	return dynamic_cast< VS_H245MCLocationIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245ConferenceIndication *( void )
	{	return dynamic_cast< VS_H245ConferenceIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245VendorIdentification *( void )
	{	return dynamic_cast< VS_H245VendorIdentification * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245FunctionNotSupported *( void )
	{	return dynamic_cast< VS_H245FunctionNotSupported * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245MultilinkIndication *( void )
	{	return dynamic_cast< VS_H245MultilinkIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245LogicalChannelRateRelease *( void )
	{	return dynamic_cast< VS_H245LogicalChannelRateRelease * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245FlowControlIndication *( void )
	{	return dynamic_cast< VS_H245FlowControlIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245MobileMultilinkReconfigurationIndication *( void )
	{	return dynamic_cast< VS_H245MobileMultilinkReconfigurationIndication * >(choice);    }

 	VS_H245IndicationMessage::operator VS_H245GenericMessage *( void )
	{	return dynamic_cast< VS_H245GenericMessage * >(choice);    }

	void VS_H245IndicationMessage::operator=(VS_H245MasterSlaveDeterminationRelease *msdrel)
	{
		FreeChoice();
		choice = msdrel;
		tag = e_masterSlaveDeterminationRelease;
		filled = true;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245IndicationMessage::Show( void ) const
	{
		if (!filled) return;
		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_functionNotUnderstood :  dprint4("choice: VS_H245FunctionNotUnderstood ");return;
		case e_masterSlaveDeterminationRelease :  dprint4("choice: VS_H245MasterSlaveDeterminationRelease ");return;
		case e_terminalCapabilitySetRelease :  dprint4("choice: VS_H245TerminalCapabilitySetRelease ");return;
		case e_openLogicalChannelConfirm :  dprint4("choice: VS_H245OpenLogicalChannelConfirm ");return;
		case e_requestChannelCloseRelease :  dprint4("choice: VS_H245RequestChannelCloseRelease ");return;
		case e_multiplexEntrySendRelease :  dprint4("choice: VS_H245MultiplexEntrySendRelease ");return;
		case e_requestMultiplexEntryRelease :  dprint4("choice: VS_H245RequestMultiplexEntryRelease ");return;
		case e_requestModeRelease :  dprint4("choice: VS_H245RequestModeRelease ");return;
		case e_miscellaneousIndication :  dprint4("choice: VS_H245MiscellaneousIndication ");return;
		case e_jitterIndication :  dprint4("choice: VS_H245JitterIndication ");return;
		case e_h223SkewIndication :  dprint4("choice: VS_H245H223SkewIndication ");return;
		case e_newATMVCIndication :  dprint4("choice: VS_H245NewATMVCIndication ");return;
		case e_userInput :  dprint4("choice: VS_H245UserInputIndication ");return;
		case e_h2250MaximumSkewIndication :  dprint4("choice: VS_H245H2250MaximumSkewIndication ");return;
		case e_mcLocationIndication :  dprint4("choice: VS_H245MCLocationIndication ");return;
		case e_conferenceIndication :  dprint4("choice: VS_H245ConferenceIndication ");return;
		case e_vendorIdentification :  dprint4("choice: VS_H245VendorIdentification ");return;
		case e_functionNotSupported :  dprint4("choice: VS_H245FunctionNotSupported ");return;
		case e_multilinkIndication :  dprint4("choice: VS_H245MultilinkIndication ");return;
		case e_logicalChannelRateRelease :  dprint4("choice: VS_H245LogicalChannelRateRelease ");return;
		case e_flowControlIndication :  dprint4("choice: VS_H245FlowControlIndication ");return;
		case e_mobileMultilinkReconfigurationIndication :  dprint4("choice: VS_H245MobileMultilinkReconfigurationIndication ");return;
		case e_genericIndication :  dprint4("choice: VS_H245GenericMessage ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245CommandMessage /////////////////////////
 	 VS_H245CommandMessage::VS_H245CommandMessage( void )
	:VS_AsnChoice(7 , 13 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245CommandMessage::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_maintenanceLoopOffCommand : return DecodeChoice( buffer , new VS_H245MaintenanceLoopOffCommand);
		case e_sendTerminalCapabilitySet : return DecodeChoice( buffer , new VS_H245SendTerminalCapabilitySet);
		case e_encryptionCommand : return DecodeChoice( buffer , new VS_H245EncryptionCommand);
		case e_flowControlCommand : return DecodeChoice( buffer , new VS_H245FlowControlCommand);
		case e_endSessionCommand : return DecodeChoice( buffer , new VS_H245EndSessionCommand);
		case e_miscellaneousCommand : return DecodeChoice( buffer , new VS_H245MiscellaneousCommand);
		case e_communicationModeCommand : return DecodeChoice( buffer , new VS_H245CommunicationModeCommand);
		case e_conferenceCommand : return DecodeChoice( buffer , new VS_H245ConferenceCommand);
		case e_h223MultiplexReconfiguration : return DecodeChoice( buffer , new VS_H245H223MultiplexReconfiguration);
		case e_newATMVCCommand : return DecodeChoice( buffer , new VS_H245NewATMVCCommand);
		case e_mobileMultilinkReconfigurationCommand : return DecodeChoice( buffer , new VS_H245MobileMultilinkReconfigurationCommand);
		case e_genericCommand : return DecodeChoice( buffer , new VS_H245GenericMessage);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245CommandMessage::operator=(const VS_H245CommandMessage & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_maintenanceLoopOffCommand : CopyChoice< VS_H245MaintenanceLoopOffCommand >(src,*this); return;
		case e_sendTerminalCapabilitySet : CopyChoice< VS_H245SendTerminalCapabilitySet >(src,*this); return;
		case e_encryptionCommand : CopyChoice< VS_H245EncryptionCommand >(src,*this); return;
		case e_flowControlCommand : CopyChoice< VS_H245FlowControlCommand >(src,*this); return;
		case e_endSessionCommand : CopyChoice< VS_H245EndSessionCommand >(src,*this); return;
		case e_miscellaneousCommand : CopyChoice< VS_H245MiscellaneousCommand >(src,*this); return;
		case e_communicationModeCommand : CopyChoice< VS_H245CommunicationModeCommand >(src,*this); return;
		case e_conferenceCommand : CopyChoice< VS_H245ConferenceCommand >(src,*this); return;
		case e_h223MultiplexReconfiguration : CopyChoice< VS_H245H223MultiplexReconfiguration >(src,*this); return;
		case e_newATMVCCommand : CopyChoice< VS_H245NewATMVCCommand >(src,*this); return;
		case e_mobileMultilinkReconfigurationCommand : CopyChoice< VS_H245MobileMultilinkReconfigurationCommand >(src,*this); return;
		case e_genericCommand : CopyChoice< VS_H245GenericMessage >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245CommandMessage::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245MaintenanceLoopOffCommand *( void )
	{	return dynamic_cast< VS_H245MaintenanceLoopOffCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245SendTerminalCapabilitySet *( void )
	{	return dynamic_cast< VS_H245SendTerminalCapabilitySet * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245EncryptionCommand *( void )
	{	return dynamic_cast< VS_H245EncryptionCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245FlowControlCommand *( void )
	{	return dynamic_cast< VS_H245FlowControlCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245EndSessionCommand *( void )
	{	return dynamic_cast< VS_H245EndSessionCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245MiscellaneousCommand *( void )
	{	return dynamic_cast< VS_H245MiscellaneousCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245CommunicationModeCommand *( void )
	{	return dynamic_cast< VS_H245CommunicationModeCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245ConferenceCommand *( void )
	{	return dynamic_cast< VS_H245ConferenceCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245H223MultiplexReconfiguration *( void )
	{	return dynamic_cast< VS_H245H223MultiplexReconfiguration * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245NewATMVCCommand *( void )
	{	return dynamic_cast< VS_H245NewATMVCCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245MobileMultilinkReconfigurationCommand *( void )
	{	return dynamic_cast< VS_H245MobileMultilinkReconfigurationCommand * >(choice);    }

 	VS_H245CommandMessage::operator VS_H245GenericMessage *( void )
	{	return dynamic_cast< VS_H245GenericMessage * >(choice);    }


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245CommandMessage::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_maintenanceLoopOffCommand :  dprint4("choice: VS_H245MaintenanceLoopOffCommand ");return;
		case e_sendTerminalCapabilitySet :  dprint4("choice: VS_H245SendTerminalCapabilitySet ");return;
		case e_encryptionCommand :  dprint4("choice: VS_H245EncryptionCommand ");return;
		case e_flowControlCommand :  dprint4("choice: VS_H245FlowControlCommand ");return;
		case e_endSessionCommand :  dprint4("choice: VS_H245EndSessionCommand ");return;
		case e_miscellaneousCommand :  dprint4("choice: VS_H245MiscellaneousCommand ");return;
		case e_communicationModeCommand :  dprint4("choice: VS_H245CommunicationModeCommand ");return;
		case e_conferenceCommand :  dprint4("choice: VS_H245ConferenceCommand ");return;
		case e_h223MultiplexReconfiguration :  dprint4("choice: VS_H245H223MultiplexReconfiguration ");return;
		case e_newATMVCCommand :  dprint4("choice: VS_H245NewATMVCCommand ");return;
		case e_mobileMultilinkReconfigurationCommand :  dprint4("choice: VS_H245MobileMultilinkReconfigurationCommand ");return;
		case e_genericCommand :  dprint4("choice: VS_H245GenericMessage ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245ResponseMessage /////////////////////////
 	 VS_H245ResponseMessage::VS_H245ResponseMessage( void )
	:VS_AsnChoice(19 , 25 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245ResponseMessage::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_masterSlaveDeterminationAck : return DecodeChoice( buffer , new VS_H245MasterSlaveDeterminationAck);
		case e_masterSlaveDeterminationReject : return DecodeChoice( buffer , new VS_H245MasterSlaveDeterminationReject);
		case e_terminalCapabilitySetAck : return DecodeChoice( buffer , new VS_H245TerminalCapabilitySetAck);
		case e_terminalCapabilitySetReject : return DecodeChoice( buffer , new VS_H245TerminalCapabilitySetReject);
		case e_openLogicalChannelAck : return DecodeChoice( buffer , new VS_H245OpenLogicalChannelAck);
		case e_openLogicalChannelReject : return DecodeChoice( buffer , new VS_H245OpenLogicalChannelReject);
		case e_closeLogicalChannelAck : return DecodeChoice( buffer , new VS_H245CloseLogicalChannelAck);
		case e_requestChannelCloseAck : return DecodeChoice( buffer , new VS_H245RequestChannelCloseAck);
		case e_requestChannelCloseReject : return DecodeChoice( buffer , new VS_H245RequestChannelCloseReject);
		case e_multiplexEntrySendAck : return DecodeChoice( buffer , new VS_H245MultiplexEntrySendAck);
		case e_multiplexEntrySendReject : return DecodeChoice( buffer , new VS_H245MultiplexEntrySendReject);
		case e_requestMultiplexEntryAck : return DecodeChoice( buffer , new VS_H245RequestMultiplexEntryAck);
		case e_requestMultiplexEntryReject : return DecodeChoice( buffer , new VS_H245RequestMultiplexEntryReject);
		case e_requestModeAck : return DecodeChoice( buffer , new VS_H245RequestModeAck);
		case e_requestModeReject : return DecodeChoice( buffer , new VS_H245RequestModeReject);
		case e_roundTripDelayResponse : return DecodeChoice( buffer , new VS_H245RoundTripDelayResponse);
		case e_maintenanceLoopAck : return DecodeChoice( buffer , new VS_H245MaintenanceLoopAck);
		case e_maintenanceLoopReject : return DecodeChoice( buffer , new VS_H245MaintenanceLoopReject);
		case e_communicationModeResponse : return DecodeChoice( buffer , new VS_H245CommunicationModeResponse);
		case e_conferenceResponse : return DecodeChoice( buffer , new VS_H245ConferenceResponse);
		case e_multilinkResponse : return DecodeChoice( buffer , new VS_H245MultilinkResponse);
		case e_logicalChannelRateAcknowledge : return DecodeChoice( buffer , new VS_H245LogicalChannelRateAcknowledge);
		case e_logicalChannelRateReject : return DecodeChoice( buffer , new VS_H245LogicalChannelRateReject);
		case e_genericResponse : return DecodeChoice( buffer , new VS_H245GenericMessage);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245ResponseMessage::operator=(const VS_H245ResponseMessage & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_masterSlaveDeterminationAck : CopyChoice< VS_H245MasterSlaveDeterminationAck >(src,*this); return;
		case e_masterSlaveDeterminationReject : CopyChoice< VS_H245MasterSlaveDeterminationReject >(src,*this); return;
		case e_terminalCapabilitySetAck : CopyChoice< VS_H245TerminalCapabilitySetAck >(src,*this); return;
		case e_terminalCapabilitySetReject : CopyChoice< VS_H245TerminalCapabilitySetReject >(src,*this); return;
		case e_openLogicalChannelAck : CopyChoice< VS_H245OpenLogicalChannelAck >(src,*this); return;
		case e_openLogicalChannelReject : CopyChoice< VS_H245OpenLogicalChannelReject >(src,*this); return;
		case e_closeLogicalChannelAck : CopyChoice< VS_H245CloseLogicalChannelAck >(src,*this); return;
		case e_requestChannelCloseAck : CopyChoice< VS_H245RequestChannelCloseAck >(src,*this); return;
		case e_requestChannelCloseReject : CopyChoice< VS_H245RequestChannelCloseReject >(src,*this); return;
		case e_multiplexEntrySendAck : CopyChoice< VS_H245MultiplexEntrySendAck >(src,*this); return;
		case e_multiplexEntrySendReject : CopyChoice< VS_H245MultiplexEntrySendReject >(src,*this); return;
		case e_requestMultiplexEntryAck : CopyChoice< VS_H245RequestMultiplexEntryAck >(src,*this); return;
		case e_requestMultiplexEntryReject : CopyChoice< VS_H245RequestMultiplexEntryReject >(src,*this); return;
		case e_requestModeAck : CopyChoice< VS_H245RequestModeAck >(src,*this); return;
		case e_requestModeReject : CopyChoice< VS_H245RequestModeReject >(src,*this); return;
		case e_roundTripDelayResponse : CopyChoice< VS_H245RoundTripDelayResponse >(src,*this); return;
		case e_maintenanceLoopAck : CopyChoice< VS_H245MaintenanceLoopAck >(src,*this); return;
		case e_maintenanceLoopReject : CopyChoice< VS_H245MaintenanceLoopReject >(src,*this); return;
		case e_communicationModeResponse : CopyChoice< VS_H245CommunicationModeResponse >(src,*this); return;
		case e_conferenceResponse : CopyChoice< VS_H245ConferenceResponse >(src,*this); return;
		case e_multilinkResponse : CopyChoice< VS_H245MultilinkResponse >(src,*this); return;
		case e_logicalChannelRateAcknowledge : CopyChoice< VS_H245LogicalChannelRateAcknowledge >(src,*this); return;
		case e_logicalChannelRateReject : CopyChoice< VS_H245LogicalChannelRateReject >(src,*this); return;
		case e_genericResponse : CopyChoice< VS_H245GenericMessage >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245ResponseMessage::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245MasterSlaveDeterminationAck *( void )
	{	return dynamic_cast< VS_H245MasterSlaveDeterminationAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245MasterSlaveDeterminationReject *( void )
	{	return dynamic_cast< VS_H245MasterSlaveDeterminationReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245TerminalCapabilitySetAck *( void )
	{	return dynamic_cast< VS_H245TerminalCapabilitySetAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245TerminalCapabilitySetReject *( void )
	{	return dynamic_cast< VS_H245TerminalCapabilitySetReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245OpenLogicalChannelAck *( void )
	{	return dynamic_cast< VS_H245OpenLogicalChannelAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245OpenLogicalChannelReject *( void )
	{	return dynamic_cast< VS_H245OpenLogicalChannelReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245CloseLogicalChannelAck *( void )
	{	return dynamic_cast< VS_H245CloseLogicalChannelAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245RequestChannelCloseAck *( void )
	{	return dynamic_cast< VS_H245RequestChannelCloseAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245RequestChannelCloseReject *( void )
	{	return dynamic_cast< VS_H245RequestChannelCloseReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245MultiplexEntrySendAck *( void )
	{	return dynamic_cast< VS_H245MultiplexEntrySendAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245MultiplexEntrySendReject *( void )
	{	return dynamic_cast< VS_H245MultiplexEntrySendReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245RequestMultiplexEntryAck *( void )
	{	return dynamic_cast< VS_H245RequestMultiplexEntryAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245RequestMultiplexEntryReject *( void )
	{	return dynamic_cast< VS_H245RequestMultiplexEntryReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245RequestModeAck *( void )
	{	return dynamic_cast< VS_H245RequestModeAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245RequestModeReject *( void )
	{	return dynamic_cast< VS_H245RequestModeReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245RoundTripDelayResponse *( void )
	{	return dynamic_cast< VS_H245RoundTripDelayResponse * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245MaintenanceLoopAck *( void )
	{	return dynamic_cast< VS_H245MaintenanceLoopAck * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245MaintenanceLoopReject *( void )
	{	return dynamic_cast< VS_H245MaintenanceLoopReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245CommunicationModeResponse *( void )
	{	return dynamic_cast< VS_H245CommunicationModeResponse * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245ConferenceResponse *( void )
	{	return dynamic_cast< VS_H245ConferenceResponse * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245MultilinkResponse *( void )
	{	return dynamic_cast< VS_H245MultilinkResponse * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245LogicalChannelRateAcknowledge *( void )
	{	return dynamic_cast< VS_H245LogicalChannelRateAcknowledge * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245LogicalChannelRateReject *( void )
	{	return dynamic_cast< VS_H245LogicalChannelRateReject * >(choice);    }

 	VS_H245ResponseMessage::operator VS_H245GenericMessage *( void )
	{	return dynamic_cast< VS_H245GenericMessage * >(choice);    }

	void VS_H245ResponseMessage::operator=( VS_H245TerminalCapabilitySetReject *tcsa )
	{
		FreeChoice();
		choice = tcsa;
		tag = e_terminalCapabilitySetReject;
		filled = true;
	}
	// end of VS_H245ResponseMessage::operator= VS_H245TerminalCapabilitySetAck *

	void VS_H245ResponseMessage::operator=( VS_H245TerminalCapabilitySetAck *tcsa )
	{
		FreeChoice();
		choice = tcsa;
		tag = e_terminalCapabilitySetAck;
		filled = true;
	}
	// end of VS_H245ResponseMessage::operator= VS_H245TerminalCapabilitySetAck *

	void VS_H245ResponseMessage::operator=( VS_H245MasterSlaveDeterminationAck *msda )
	{
		FreeChoice();
		choice = msda;
		tag = e_masterSlaveDeterminationAck;
		filled = true;
	}
	// end of VS_H245ResponseMessage::operator= VS_H245MasterSlaveDeterminationAck *

	void VS_H245ResponseMessage::operator=(VS_H245MasterSlaveDeterminationReject *msdr)
	{
		FreeChoice();
		choice = msdr;
		tag = e_masterSlaveDeterminationReject;
		filled = true;
	}
	// end of VS_H245ResponseMessage::operator= VS_H245MasterSlaveDeterminationReject *

	void VS_H245ResponseMessage::operator=( VS_H245OpenLogicalChannelAck *olca )
	{
		FreeChoice();
		choice = olca;
		tag = e_openLogicalChannelAck;
		filled = true;
	}

	void VS_H245ResponseMessage::operator=( VS_H245OpenLogicalChannelReject *olcRj )
	{
		FreeChoice();
		choice = olcRj;
		tag = e_openLogicalChannelReject;
		filled = true;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245ResponseMessage::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_masterSlaveDeterminationAck :  dprint4("choice: VS_H245MasterSlaveDeterminationAck ");return;
		case e_masterSlaveDeterminationReject :  dprint4("choice: VS_H245MasterSlaveDeterminationReject ");return;
		case e_terminalCapabilitySetAck :  dprint4("choice: VS_H245TerminalCapabilitySetAck ");return;
		case e_terminalCapabilitySetReject :  dprint4("choice: VS_H245TerminalCapabilitySetReject ");return;
		case e_openLogicalChannelAck :  dprint4("choice: VS_H245OpenLogicalChannelAck ");return;
		case e_openLogicalChannelReject :  dprint4("choice: VS_H245OpenLogicalChannelReject ");return;
		case e_closeLogicalChannelAck :  dprint4("choice: VS_H245CloseLogicalChannelAck ");return;
		case e_requestChannelCloseAck :  dprint4("choice: VS_H245RequestChannelCloseAck ");return;
		case e_requestChannelCloseReject :  dprint4("choice: VS_H245RequestChannelCloseReject ");return;
		case e_multiplexEntrySendAck :  dprint4("choice: VS_H245MultiplexEntrySendAck ");return;
		case e_multiplexEntrySendReject :  dprint4("choice: VS_H245MultiplexEntrySendReject ");return;
		case e_requestMultiplexEntryAck :  dprint4("choice: VS_H245RequestMultiplexEntryAck ");return;
		case e_requestMultiplexEntryReject :  dprint4("choice: VS_H245RequestMultiplexEntryReject ");return;
		case e_requestModeAck :  dprint4("choice: VS_H245RequestModeAck ");return;
		case e_requestModeReject :  dprint4("choice: VS_H245RequestModeReject ");return;
		case e_roundTripDelayResponse :  dprint4("choice: VS_H245RoundTripDelayResponse ");return;
		case e_maintenanceLoopAck :  dprint4("choice: VS_H245MaintenanceLoopAck ");return;
		case e_maintenanceLoopReject :  dprint4("choice: VS_H245MaintenanceLoopReject ");return;
		case e_communicationModeResponse :  dprint4("choice: VS_H245CommunicationModeResponse ");return;
		case e_conferenceResponse :  dprint4("choice: VS_H245ConferenceResponse ");return;
		case e_multilinkResponse :  dprint4("choice: VS_H245MultilinkResponse ");return;
		case e_logicalChannelRateAcknowledge :  dprint4("choice: VS_H245LogicalChannelRateAcknowledge ");return;
		case e_logicalChannelRateReject :  dprint4("choice: VS_H245LogicalChannelRateReject ");return;
		case e_genericResponse :  dprint4("choice: VS_H245GenericMessage ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245RequestMessage /////////////////////////
 	 VS_H245RequestMessage::VS_H245RequestMessage( void )
	:VS_AsnChoice(11 , 16 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245RequestMessage::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_nonStandard : return DecodeChoice( buffer , new VS_H245NonStandardMessage);
		case e_masterSlaveDetermination : return DecodeChoice( buffer , new VS_H245MasterSlaveDetermination);
		case e_terminalCapabilitySet : return DecodeChoice( buffer , new VS_H245TerminalCapabilitySet);
		case e_openLogicalChannel : return DecodeChoice( buffer , new VS_H245OpenLogicalChannel);
		case e_closeLogicalChannel : return DecodeChoice( buffer , new VS_H245CloseLogicalChannel);
		case e_requestChannelClose : return DecodeChoice( buffer , new VS_H245RequestChannelClose);
		case e_multiplexEntrySend : return DecodeChoice( buffer , new VS_H245MultiplexEntrySend);
		case e_requestMultiplexEntry : return DecodeChoice( buffer , new VS_H245RequestMultiplexEntry);
		case e_requestMode : return DecodeChoice( buffer , new VS_H245RequestMode);
		case e_roundTripDelayRequest : return DecodeChoice( buffer , new VS_H245RoundTripDelayRequest);
		case e_maintenanceLoopRequest : return DecodeChoice( buffer , new VS_H245MaintenanceLoopRequest);
		case e_communicationModeRequest : return DecodeChoice( buffer , new VS_H245CommunicationModeRequest);
		case e_conferenceRequest : return DecodeChoice( buffer , new VS_H245ConferenceRequest);
		case e_multilinkRequest : return DecodeChoice( buffer , new VS_H245MultilinkRequest);
		case e_logicalChannelRateRequest : return DecodeChoice( buffer , new VS_H245LogicalChannelRateRequest);
		case e_genericRequest : return DecodeChoice( buffer , new VS_H245GenericMessage);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245RequestMessage::operator=(const VS_H245RequestMessage & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_nonStandard : CopyChoice< VS_H245NonStandardMessage >(src,*this); return;
		case e_masterSlaveDetermination : CopyChoice< VS_H245MasterSlaveDetermination >(src,*this); return;
		case e_terminalCapabilitySet : CopyChoice< VS_H245TerminalCapabilitySet >(src,*this); return;
		case e_openLogicalChannel : CopyChoice< VS_H245OpenLogicalChannel >(src,*this); return;
		case e_closeLogicalChannel : CopyChoice< VS_H245CloseLogicalChannel >(src,*this); return;
		case e_requestChannelClose : CopyChoice< VS_H245RequestChannelClose >(src,*this); return;
		case e_multiplexEntrySend : CopyChoice< VS_H245MultiplexEntrySend >(src,*this); return;
		case e_requestMultiplexEntry : CopyChoice< VS_H245RequestMultiplexEntry >(src,*this); return;
		case e_requestMode : CopyChoice< VS_H245RequestMode >(src,*this); return;
		case e_roundTripDelayRequest : CopyChoice< VS_H245RoundTripDelayRequest >(src,*this); return;
		case e_maintenanceLoopRequest : CopyChoice< VS_H245MaintenanceLoopRequest >(src,*this); return;
		case e_communicationModeRequest : CopyChoice< VS_H245CommunicationModeRequest >(src,*this); return;
		case e_conferenceRequest : CopyChoice< VS_H245ConferenceRequest >(src,*this); return;
		case e_multilinkRequest : CopyChoice< VS_H245MultilinkRequest >(src,*this); return;
		case e_logicalChannelRateRequest : CopyChoice< VS_H245LogicalChannelRateRequest >(src,*this); return;
		case e_genericRequest : CopyChoice< VS_H245GenericMessage >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245RequestMessage::operator VS_H245NonStandardMessage *( void )
	{	return dynamic_cast< VS_H245NonStandardMessage * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245MasterSlaveDetermination *( void )
	{	return dynamic_cast< VS_H245MasterSlaveDetermination * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245TerminalCapabilitySet *( void )
	{	return dynamic_cast< VS_H245TerminalCapabilitySet * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245OpenLogicalChannel *( void )
	{	return dynamic_cast< VS_H245OpenLogicalChannel * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245CloseLogicalChannel *( void )
	{	return dynamic_cast< VS_H245CloseLogicalChannel * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245RequestChannelClose *( void )
	{	return dynamic_cast< VS_H245RequestChannelClose * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245MultiplexEntrySend *( void )
	{	return dynamic_cast< VS_H245MultiplexEntrySend * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245RequestMultiplexEntry *( void )
	{	return dynamic_cast< VS_H245RequestMultiplexEntry * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245RequestMode *( void )
	{	return dynamic_cast< VS_H245RequestMode * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245RoundTripDelayRequest *( void )
	{	return dynamic_cast< VS_H245RoundTripDelayRequest * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245MaintenanceLoopRequest *( void )
	{	return dynamic_cast< VS_H245MaintenanceLoopRequest * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245CommunicationModeRequest *( void )
	{	return dynamic_cast< VS_H245CommunicationModeRequest * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245ConferenceRequest *( void )
	{	return dynamic_cast< VS_H245ConferenceRequest * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245MultilinkRequest *( void )
	{	return dynamic_cast< VS_H245MultilinkRequest * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245LogicalChannelRateRequest *( void )
	{	return dynamic_cast< VS_H245LogicalChannelRateRequest * >(choice);    }

 	VS_H245RequestMessage::operator VS_H245GenericMessage *( void )
	{	return dynamic_cast< VS_H245GenericMessage * >(choice);    }


	void VS_H245RequestMessage::operator=( VS_H245OpenLogicalChannel *olc )
	{
		FreeChoice();
		choice = olc;
		tag = e_openLogicalChannel;
		filled = true;
	}
	// end of VS_H245RequestMessage::operator= VS_H245OpenLogicalChannel *

	void VS_H245RequestMessage::operator=( VS_H245MasterSlaveDetermination *msd )
	{
		FreeChoice();
		choice = msd;
		tag = e_masterSlaveDetermination;
		filled = true;
	}
	// end of VS_H245RequestMessage::operator= VS_H245MasterSlaveDetermination *

	void VS_H245RequestMessage::operator=( VS_H245TerminalCapabilitySet *tcs )
	{
		FreeChoice();
		choice = tcs;
		tag = e_terminalCapabilitySet;
		filled = true;
	}
		// end of VS_H245RequestMessage::operator= VS_H245TerminalCapabilitySet *

	void VS_H245RequestMessage::operator=( VS_H245RoundTripDelayRequest *tcs )
	{
		FreeChoice();
		choice = tcs;
		tag = e_roundTripDelayRequest;
		filled = true;
	}
 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245RequestMessage::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_nonStandard :  dprint4("choice: VS_H245NonStandardMessage ");return;
		case e_masterSlaveDetermination :  dprint4("choice: VS_H245MasterSlaveDetermination ");return;
		case e_terminalCapabilitySet :  dprint4("choice: VS_H245TerminalCapabilitySet ");return;
		case e_openLogicalChannel :  dprint4("choice: VS_H245OpenLogicalChannel ");return;
		case e_closeLogicalChannel :  dprint4("choice: VS_H245CloseLogicalChannel ");return;
		case e_requestChannelClose :  dprint4("choice: VS_H245RequestChannelClose ");return;
		case e_multiplexEntrySend :  dprint4("choice: VS_H245MultiplexEntrySend ");return;
		case e_requestMultiplexEntry :  dprint4("choice: VS_H245RequestMultiplexEntry ");return;
		case e_requestMode :  dprint4("choice: VS_H245RequestMode ");return;
		case e_roundTripDelayRequest :  dprint4("choice: VS_H245RoundTripDelayRequest ");return;
		case e_maintenanceLoopRequest :  dprint4("choice: VS_H245MaintenanceLoopRequest ");return;
		case e_communicationModeRequest :  dprint4("choice: VS_H245CommunicationModeRequest ");return;
		case e_conferenceRequest :  dprint4("choice: VS_H245ConferenceRequest ");return;
		case e_multilinkRequest :  dprint4("choice: VS_H245MultilinkRequest ");return;
		case e_logicalChannelRateRequest :  dprint4("choice: VS_H245LogicalChannelRateRequest ");return;
		case e_genericRequest :  dprint4("choice: VS_H245GenericMessage ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}

//////////////////////CLASS VS_H245MultimediaSystemControlMessage /////////////////////////
 	 VS_H245MultimediaSystemControlMessage::VS_H245MultimediaSystemControlMessage( void )
	:VS_AsnChoice(4 , 4 , 1 )
	{
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	bool VS_H245MultimediaSystemControlMessage::Decode( VS_PerBuffer &buffer )
	{
		if (!buffer.ChoiceDecode(*this)) return false;
		switch(tag)
		{
		case e_request : return DecodeChoice( buffer , new VS_H245RequestMessage);
		case e_response : return DecodeChoice( buffer , new VS_H245ResponseMessage);
		case e_command : return DecodeChoice( buffer , new VS_H245CommandMessage);
		case e_indication : return DecodeChoice( buffer , new VS_H245IndicationMessage);
		default: return buffer.ChoiceMissExtensionObject(*this);
		}

	}

	void VS_H245MultimediaSystemControlMessage::operator=(const VS_H245MultimediaSystemControlMessage & src)
	{
		FreeChoice();
		if (!src.filled) return;
		switch(src.tag)
		{
		case e_request : CopyChoice< VS_H245RequestMessage >(src,*this); return;
		case e_response : CopyChoice< VS_H245ResponseMessage >(src,*this); return;
		case e_command : CopyChoice< VS_H245CommandMessage >(src,*this); return;
		case e_indication : CopyChoice< VS_H245IndicationMessage >(src,*this); return;
		default:		return;
		}

		return;
	}

 /////////////////////////////////////////////////////////////////////////////////////////
	VS_H245MultimediaSystemControlMessage::operator VS_H245RequestMessage *( void )
	{	return dynamic_cast< VS_H245RequestMessage * >(choice);    }

 	VS_H245MultimediaSystemControlMessage::operator VS_H245ResponseMessage *( void )
	{	return dynamic_cast< VS_H245ResponseMessage * >(choice);    }

 	VS_H245MultimediaSystemControlMessage::operator VS_H245CommandMessage *( void )
	{	return dynamic_cast< VS_H245CommandMessage * >(choice);    }

 	VS_H245MultimediaSystemControlMessage::operator VS_H245IndicationMessage *( void )
	{	return dynamic_cast< VS_H245IndicationMessage * >(choice);    }


void VS_H245MultimediaSystemControlMessage::operator=(VS_H245RequestMessage *rm)
{
	FreeChoice();
	choice = rm;
	tag = e_request;
	filled = true;
}
// end of VS_H245MultimediaSystemControlMessage::operator= VS_H245RequestMessage *

void VS_H245MultimediaSystemControlMessage::operator=(VS_H245ResponseMessage *rm)
{
	FreeChoice();
	choice = rm;
	tag = e_response;
	filled = true;
}
// end of VS_H245MultimediaSystemControlMessage::operator= VS_H245ResponseMessage *

void VS_H245MultimediaSystemControlMessage::operator=(VS_H245CommandMessage *cm)
{
	FreeChoice();
	choice = cm;
	tag = e_command;
	filled = true;
}
// end of VS_H245MultimediaSystemControlMessage::operator= VS_H245CommandMessage *

void VS_H245MultimediaSystemControlMessage::operator=(VS_H245IndicationMessage *im)
{
	FreeChoice();
	choice = im;
	tag = e_indication;
	filled = true;
}
// end of VS_H245MultimediaSystemControlMessage::operator= VS_H245IndicationMessage *


 /////////////////////////////////////////////////////////////////////////////////////////
	void VS_H245MultimediaSystemControlMessage::Show( void ) const
	{

		if (!filled) return;

		switch(tag)
		{
		case e_request :  dprint4("choice: VS_H245RequestMessage ");return;
		case e_response :  dprint4("choice: VS_H245ResponseMessage ");return;
		case e_command :  dprint4("choice: VS_H245CommandMessage ");return;
		case e_indication :  dprint4("choice: VS_H245IndicationMessage ");return;
		default: dprint4("unknown choice: %d",tag); return ;
		}

	}
