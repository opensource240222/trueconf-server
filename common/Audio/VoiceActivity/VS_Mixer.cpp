/**
 **************************************************************************
 * \file VS_Mixer.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Set Windows Mixer Features, combine VS_Mixer_Features or use VS_MIXER_ALL
 *
 * \b Project Audio
 * \author SMirnovK
 * \date 19.02.04
 *
 * $Revision: 3 $
 *
 * $History: VS_Mixer.cpp $
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 22.04.10   Time: 20:47
 * Updated in $/VSNA/Audio/VoiceActivity
 * - set boost option for microphone
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/Audio/VoiceActivity
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 9.11.07    Time: 18:54
 * Updated in $/VS2005/Audio/VoiceActivity
 * - bugfix #3535
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

/******************************************************************************
* Includes
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Mmsystem.h>
#include <fstream>
#include "VS_Mixer.h"
#include "../../std/cpplib/VS_SimpleStr.h"


/******************************************************************************
* Defines
******************************************************************************/
#ifdef _DEBUG
#define dprint printf
#else
#define dprint (void)
#endif

#define  pr_err(func, ret) dprint(#func ": %s\n", formEr(ret))

/**
****************************************************************************
* Explain MMRESULT as string
******************************************************************************/
static const char* formEr(MMRESULT mmres)
{
	switch(mmres)
	{
	case MMSYSERR_ALLOCATED:
		return "The specified resource is already allocated by the maximum number of clients possible.";
	case MMSYSERR_BADDEVICEID:
		return "The uMxId parameter specifies an invalid device identifier.";
	case MMSYSERR_INVALFLAG:
		return "One or more flags are invalid. ";
	case MMSYSERR_INVALHANDLE:
		return "The uMxId parameter specifies an invalid handle. ";
	case MMSYSERR_INVALPARAM:
		return "One or more parameters are invalid. ";
	case MMSYSERR_NODRIVER:
		return "No mixer device is available for the object specified by uMxId. Note that the location referenced by uMxId will also contain the value Ц 1. ";
	case MMSYSERR_NOMEM:
		return "Unable to allocate resources. ";
	default :
		return "Unknown error";
	}
}

/**
****************************************************************************
* Set Control of given type by given value
******************************************************************************/
bool VS_MixerSetControl(HMIXEROBJ mixer, DWORD id, DWORD type, DWORD val, DWORD channels, DWORD mUi = 0)
{
	MMRESULT mmres = -1;
	MIXERCONTROLDETAILS mcd;
	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.dwControlID = id;
	mcd.cChannels = channels;
	mcd.cMultipleItems = mUi;
	DWORD i, l;

	if		((type&0xffff0000) == MIXERCONTROL_CONTROLTYPE_BOOLEAN) {
		MIXERCONTROLDETAILS_BOOLEAN *control = new MIXERCONTROLDETAILS_BOOLEAN[channels];
		mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		mcd.paDetails = control;
		for (i =0; i< channels; i++)
			control[i].fValue = val;
		mmres = mixerSetControlDetails(mixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
		if (mmres!= MMSYSERR_NOERROR)
			pr_err(mixerSetControlDetails_bool, mmres);
		else {
			dprint("          BOOL val = %ld set \n", val);
		}
		delete[] control;
	}
	else if ((type&0xffff0000) == MIXERCONTROL_CONTROLTYPE_FADER) {
		MIXERCONTROLDETAILS_UNSIGNED *control = new MIXERCONTROLDETAILS_UNSIGNED[channels];
		mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
		mcd.paDetails = control;
		for (i =0; i< channels; i++)
			control[i].dwValue = val;
		mmres = mixerSetControlDetails(mixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
		if (mmres!= MMSYSERR_NOERROR)
			pr_err(mixerSetControlDetails_unsigned, mmres);
		else {
			dprint("          UINT val = %ld set \n", val);
		}
		delete[] control;
	}
	else if ((type&MIXERCONTROL_CT_CLASS_MASK) == MIXERCONTROL_CT_CLASS_LIST) {
		bool sigleSelect = (type&0xffff0000)==MIXERCONTROL_CONTROLTYPE_SINGLESELECT;
		DWORD num = -1;

		MIXERCONTROLDETAILS_BOOLEAN *controls = new MIXERCONTROLDETAILS_BOOLEAN[channels*mUi];
		MIXERCONTROLDETAILS_LISTTEXT *list = new MIXERCONTROLDETAILS_LISTTEXT[channels*mUi];

		mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
		mcd.paDetails = list;
		mmres = mixerGetControlDetails((HMIXEROBJ)mixer, &mcd, MIXER_GETCONTROLDETAILSF_LISTTEXT);
		if (mmres!= MMSYSERR_NOERROR)
			pr_err(mixerGetControlDetails_List, mmres);

		mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		mcd.paDetails = controls;
		mmres = mixerGetControlDetails((HMIXEROBJ)mixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);
		if (mmres!= MMSYSERR_NOERROR)
			pr_err(mixerGetControlDetails_Val, mmres);

		if (mmres== MMSYSERR_NOERROR) {
			for (i =0; i< channels; i++) {
				for (l = 0; l< mUi; l++) {
					dprint("          %16s = %ld retrived \n", list[i*mUi+l].szName, controls[i*mUi+l].fValue);
					if (list[i*mUi+l].dwParam2 == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE) {
						controls[i*mUi+l].fValue = 1;
						num = i*mUi+l;
					}
					else if (sigleSelect){ // clear other if Single Select Mux
						controls[i*mUi+l].fValue = 0;
					}
				}
			}

			mmres = mixerSetControlDetails((HMIXEROBJ)mixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
			if (mmres!= MMSYSERR_NOERROR)
				pr_err(mixerSetControlDetails_val, mmres);
			else {
				dprint("          %s selected \n", num != -1? list[num].szName : "Mic was not found, not ");
			}
		}
		delete[] controls;
		delete[] list;
	}
	return mmres==MMSYSERR_NOERROR;
}




/**
****************************************************************************
* Search Controls that have given features (options)
******************************************************************************/
bool VS_MixerSearchOpt(HMIXEROBJ mixer, MIXERLINE *pml, MIXERCONTROL *pmc, int opt)
{
	bool ret = true;
	if (pml->cControls==0) return false;
	// check uniform state
	DWORD channels = (pmc->fdwControl&MIXERCONTROL_CONTROLF_UNIFORM) ? 1 : pml->cChannels;
	// go throw options
	if (opt&VS_MIXER_MUTE_MIC)												// - Mute Mic
		if (pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)	// Mic
			if (pml->cControls>=2)											// Mic belong OUT ???
				if (pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE)	// Mute
					ret&= VS_MixerSetControl(mixer, pmc->dwControlID, pmc->dwControlType, 1, channels);
	if (opt&VS_MIXER_UNMUTE_ALL)											// - UnMute All dev
		if (pml->dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS ||	// Master Volume Control???
			pml->dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN ||	// Record Master Control
			pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT)	// WaveOut Control
				if (pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE)	// Mute
					ret&= VS_MixerSetControl(mixer, pmc->dwControlID, pmc->dwControlType, 0, channels);

	if (opt&VS_MIXER_VOLUME_ALL)											// - Set Master Volume
		if (pml->dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS ||	// Master Volume Control???
			pml->dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN ||	// Record Master Control
			pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT)	// WaveOut Control
				if (pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)	// Volume
					ret&= VS_MixerSetControl(mixer, pmc->dwControlID, pmc->dwControlType,
					(pmc->Bounds.dwMinimum + pmc->Bounds.dwMaximum)*3/4, channels);

	if (opt&VS_MIXER_SELECT_MIC)											// - Select Mic
		if (pml->dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)		// Record Master Control
			if ((pmc->dwControlType&MIXERCONTROL_CT_CLASS_MASK) == MIXERCONTROL_CT_CLASS_LIST)	// Mixer
				ret&= VS_MixerSetControl(mixer, pmc->dwControlID, pmc->dwControlType, -1, channels, pmc->cMultipleItems);

	if (opt&VS_MIXER_VOLUME_MIC)											// - Set Mic Volume
		if (pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)	// Mic
			if (pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)		// Volume
				ret&= VS_MixerSetControl(mixer, pmc->dwControlID, pmc->dwControlType,
				(pmc->Bounds.dwMinimum + pmc->Bounds.dwMaximum)*3/4, channels);

	if ((opt&VS_MIXER_BOOST_MIC)||(opt&VS_MIXER_UNBOOST_MIC))													// - boost mic
		if (pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE ||	// exactly Mic
			pml->dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_ANALOG)			// possible Mic
			if (pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_ONOFF ||			// some switch
				pmc->dwControlType == MIXERCONTROL_CONTROLTYPE_LOUDNESS) {		// real boost
					VS_SimpleStr name(pmc->szName);
					_strlwr(name);
					if (strstr(name, "boost") ||strstr(name, "усил")||strstr(name, "”сил"))
					{
						if(opt==VS_MIXER_BOOST_MIC)
							ret&= VS_MixerSetControl(mixer, pmc->dwControlID, pmc->dwControlType, 1, channels);
						if(opt==VS_MIXER_UNBOOST_MIC)
							ret&= VS_MixerSetControl(mixer, pmc->dwControlID, pmc->dwControlType, 0, channels);


					}
			}

	return ret;
}

/**
****************************************************************************
* Set Audio Mixer Features
******************************************************************************/
void VS_MixerSetFeature(int GlobalMode)
{

	UINT numdev = mixerGetNumDevs();
	dprint("Found %d mix \n", numdev);
	MIXERCAPS mcaps;
	MMRESULT mmres;

	for (UINT i = 0; i< numdev; i++) {
		mixerGetDevCaps(i, &mcaps, sizeof(MIXERCAPS));
		dprint("  %s  --- %d %d , lines = %ld \n", mcaps.szPname, mcaps.wMid, mcaps.wPid, mcaps.cDestinations);

		HMIXER mixer;
		mmres = mixerOpen(&mixer, i, 0, 0, MIXER_OBJECTF_HMIXER);
		if (mmres!= MMSYSERR_NOERROR)
			pr_err(mixerOpen, mmres);
		else {
			for (DWORD j = 0; j< mcaps.cDestinations; j++) {
				MIXERLINE mline;
				mline.cbStruct = sizeof(MIXERLINE);
				mline.dwDestination = j;
				mmres = mixerGetLineInfo((HMIXEROBJ)mixer, &mline, MIXER_GETLINEINFOF_DESTINATION);
				if (mmres!=MMSYSERR_NOERROR)
					pr_err(mixerGetLineInfo_Dst, mmres);
				else {
					dprint("    --------- new dest inf ---------------\n");
					DWORD ConnNum = mline.cConnections;
					for (DWORD m = 0; m< (ConnNum+1); m++) {
						mline.dwSource = m-1;

						dprint("    --------- line inf ---------------\n");
						mmres = mixerGetLineInfo((HMIXEROBJ)mixer, &mline, m==0? MIXER_GETLINEINFOF_DESTINATION : MIXER_GETLINEINFOF_SOURCE);
						if (mmres!=MMSYSERR_NOERROR)
							pr_err(mixerGetLineInfo_src, mmres);
						else
						{
							dprint("     id = %lx, status = %lx, ctype = %lx, ch = %lx, conn = %lx, contr = %lx \n\
								   \r     SN = %s, LN = %s \n",
								   mline.dwLineID, mline.fdwLine, mline.dwComponentType, mline.cChannels,
								   mline.cConnections, mline.cControls, mline.szShortName, mline.szName);

							if (mline.cControls!=0) {
								MIXERLINECONTROLS mlc;
								MIXERCONTROL * pmcs = new MIXERCONTROL[mline.cControls];
								mlc.cbStruct = sizeof(MIXERLINECONTROLS);
								mlc.dwLineID = mline.dwLineID;
								mlc.cControls = mline.cControls;
								mlc.cbmxctrl = sizeof(MIXERCONTROL);
								mlc.pamxctrl = pmcs;
								mmres = mixerGetLineControls((HMIXEROBJ)mixer, &mlc, MIXER_GETLINECONTROLSF_ALL);
								if (mmres!= MMSYSERR_NOERROR)
									pr_err(mixerGetLineControls, mmres);
								else {
									for (DWORD k = 0; k<mline.cControls; k++) {
										MIXERCONTROL * pp = &pmcs[k];
										dprint("       SN = %16s, id = %lx, type = %lx, st = %lx, mUi = %lx \n",
											pp->szShortName, pp->dwControlID, pp->dwControlType, pp->fdwControl, pp->cMultipleItems);
										VS_MixerSearchOpt((HMIXEROBJ)mixer, &mline, pp, GlobalMode);
									}
								}
								delete[] pmcs;
							}
						}
					}
				}
			}
			mixerClose(mixer);
		}
	}
}
