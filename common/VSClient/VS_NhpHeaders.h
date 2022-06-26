/**
 **************************************************************************
 * \file VS_NhpHeaders.h
 * (c) 2002-2006 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Contain structures and interfaces to use with NHP
 * \b Project Client
 * \author SMirnovK
 * \author AnufrievS
 * \date 01.11.2006
 *
 * $Revision: 1 $
 *
 * $History: VS_NhpHeaders.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 10.11.06   Time: 19:34
 * Updated in $/VS/vsclient
 *  - unsugned members
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 8.11.06    Time: 16:53
 * Created in $/VS/VSClient
 * - NHP headers added
 *
 ****************************************************************************/
#ifndef _VS_NHP_HEADERS_H_
#define _VS_NHP_HEADERS_H_

#pragma pack(push, 1)

/// NHP stream version
#define NHPH_VER_ALFA	(0)

/*@{ \name Four data types come by NHP */
#define NHPH_DT_CMND	(0)		/// command
#define NHPH_DT_AUDIO	(1)		/// audio data
#define NHPH_DT_VIDEO	(2)		/// video data
#define NHPH_DT_DATA	(3)		/// any other data
/*@}*/

/*@{ \name Four parts of (video) frame */
#define NHPH_TP_INBOUND	(0)		/// inner part
#define NHPH_TP_LAST	(1)		/// last part
#define NHPH_TP_FIRST	(2)		/// first part
#define NHPH_TP_ONEONLY	(3)		/// first and last part
/*@}*/

/**
 **************************************************************************
 * \brief First byte of data come from NHP stream
 ****************************************************************************/
struct VS_NhpFirstHeader
{
	unsigned char	typeOfPart:2;	/// 0..3
	unsigned char	resendNum:2;	/// 0..3
	unsigned char	dataType:2;		/// 0..3
	unsigned char	version:2;		/// = NHPH_VER_ALFA
	unsigned char	DataFormat;		/// data format
	unsigned short	SeqId;			/// increment by one for created packet
};


/*@{ \name Type of compressed video frame */
#define NHPVH_FT_IFRAME	(0)		/// I-frame
#define NHPVH_FT_PFRAME	(1)		/// P-frame
#define NHPVH_FT_BFRAME	(2)		/// B-frame
/*@}*/

/**
 **************************************************************************
 * \brief Second header of data type of video
 ****************************************************************************/
struct VS_NhpVideoHeader
{
	unsigned char	FrameId;	/// ++ by video frame
	unsigned char	FrameType;	/// I, P, B ...ect/
	unsigned int	TimeStamp;	/// stored capture time
};

/**
 **************************************************************************
 * \brief Second header of data type of audio
 ****************************************************************************/
struct VS_NhpAudioHeader
{
	unsigned int	TimeStamp;	/// stored capture time
};


/**
 **************************************************************************
 * \brief Statictic per one data type
 ****************************************************************************/
struct VS_NhpTypeStat
{
	unsigned short	rcv_pkts;
	unsigned short	loss_pkts;
	unsigned int	rcv_bytes;
};

/**
 **************************************************************************
 * \brief Total Statistic and some flags for control module usage
 ****************************************************************************/
struct VS_NhpTotalStat
{
	unsigned char	lossByBitrate;
	unsigned char	reserved;
	VS_NhpTypeStat	typeStat[4];
};

#pragma pack(pop)

#endif /*_VS_NHP_HEADERS_H_*/