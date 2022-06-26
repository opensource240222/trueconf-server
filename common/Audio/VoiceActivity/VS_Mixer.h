/**
 **************************************************************************
 * \file VS_Mixer.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Set Windows Mixer Features, combine VS_Mixer_Features or use VS_MIXER_ALL
 *
 * \b Project Audio
 * \author SMirnovK
 * \date 19.02.04
 *
 * $Revision: 2 $
 *
 * $History: VS_Mixer.h $
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 22.04.10   Time: 20:47
 * Updated in $/VSNA/Audio/VoiceActivity
 * - set boost option for microphone
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/Audio/VoiceActivity
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/Audio/VoiceActivity
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 17.12.04   Time: 14:21
 * Updated in $/VS/Audio/VoiceActivity
 * added file header comments
 ****************************************************************************/
#ifndef VS_MIXER_H
#define VS_MIXER_H

/**
****************************************************************************
* Features Enum
******************************************************************************/
enum VS_Mixer_Features
{
	VS_MIXER_NONE		=0x00,
	VS_MIXER_MUTE_MIC	=0x01,
	VS_MIXER_UNMUTE_ALL	=0x02,
	VS_MIXER_VOLUME_ALL =0x04,
	VS_MIXER_SELECT_MIC	=0x08,
	VS_MIXER_VOLUME_MIC	=0x10,
	VS_MIXER_BOOST_MIC	=0x20,
	VS_MIXER_UNBOOST_MIC =0x21,
	VS_MIXER_ALL		=0xff
};

/**
****************************************************************************
* \brief Set Windows Audio Mixer Features
******************************************************************************/
void VS_MixerSetFeature(int GlobalMode);

#endif
