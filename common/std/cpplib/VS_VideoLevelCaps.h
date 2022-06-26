#ifndef VS_VIDEOLEVELCAPS_H
#define VS_VIDEOLEVELCAPS_H

enum eLevelIdc
{
	VS_VIDEOLEVEL_10 = 10,
	VS_VIDEOLEVEL_11 = 11,
	VS_VIDEOLEVEL_12 = 12,
	VS_VIDEOLEVEL_13 = 13,
	VS_VIDEOLEVEL_15 = 15,
	VS_VIDEOLEVEL_17 = 17,
	VS_VIDEOLEVEL_21 = 21,
	VS_VIDEOLEVEL_22 = 22,
	VS_VIDEOLEVEL_23 = 23,
	VS_VIDEOLEVEL_24 = 24,
	VS_VIDEOLEVEL_25 = 25,
	VS_VIDEOLEVEL_26 = 26,
	VS_VIDEOLEVEL_28 = 28,
	VS_VIDEOLEVEL_29 = 29,
	VS_VIDEOLEVEL_30 = 30,
	VS_VIDEOLEVEL_31 = 31,
	VS_VIDEOLEVEL_32 = 32,
	VS_VIDEOLEVEL_33 = 33,
	VS_VIDEOLEVEL_35 = 35,
	VS_VIDEOLEVEL_36 = 36,
	VS_VIDEOLEVEL_38 = 38,
	VS_VIDEOLEVEL_43 = 43,
	VS_VIDEOLEVEL_47 = 47,
	VS_VIDEOLEVEL_51 = 51,
	VS_VIDEOLEVEL_54 = 54,
	VS_VIDEOLEVEL_56 = 56,
	VS_VIDEOLEVEL_58 = 58,
	VS_VIDEOLEVEL_60 = 60,
	VS_VIDEOLEVEL_61 = 61,
	VS_VIDEOLEVEL_62 = 62,
	VS_VIDEOLEVEL_63 = 63,
	VS_VIDEOLEVEL_65 = 65,
	VS_VIDEOLEVEL_66 = 66,
	VS_VIDEOLEVEL_67 = 67,
	VS_VIDEOLEVEL_68 = 68,
	VS_VIDEOLEVEL_69 = 69,
	VS_VIDEOLEVEL_72 = 72,
	VS_VIDEOLEVEL_75 = 75,
	VS_VIDEOLEVEL_81 = 81,
	VS_VIDEOLEVEL_83 = 83,
	VS_VIDEOLEVEL_85 = 85,
	VS_VIDEOLEVEL_87 = 87,
	VS_VIDEOLEVEL_89 = 89,
	VS_VIDEOLEVEL_91 = 91,
	VS_VIDEOLEVEL_93 = 93,
	VS_VIDEOLEVEL_96 = 96,
	VS_VIDEOLEVEL_98 = 98,
	VS_VIDEOLEVEL_101 = 101,
	VS_VIDEOLEVEL_108 = 108,
};

enum eAspectRatio
{
	VS_VIDEOAR_16_9		= 0, /// HD
	VS_VIDEOAR_4_3		= 1, /// VGA
	VS_VIDEOAR_5_3,			 /// WVGA
	VS_VIDEOAR_16_10,		 /// CGA & XGA
	VS_VIDEOAR_11_9,		 /// CIF
	VS_VIDEOAR_3_2,			 /// NTSC
	VS_VIDEOAR_NUM_MAX
};

enum eHardwareEncoder
{
	ENCODER_SOFTWARE		= 0,
	ENCODER_H264_LOGITECH	= 1,
	ENCODER_H264_INTEL		= 2,
	ENCODER_H264_INTEL_MSS	= 3,
	ENCODER_H264_NVIDIA		= 4,
	ENCODER_SLIDES			= 5,
	MAX_NUM_HW_ENCODERS
};

#ifndef _BUILD_CONFERENDO

#define VIDEO_LEVEL_MAX						(VS_VIDEOLEVEL_63)
#define VIDEO_LEVEL_4K_MAX					(VS_VIDEOLEVEL_101)
#define VIDEO_PERFECT_AR					(VS_VIDEOAR_16_9)
#define VIDEO_LEVEL_GCONF					(VS_VIDEOLEVEL_51)
#define VIDEO_LEVEL_GCONF_DS				(VS_VIDEOLEVEL_60)

#else

#define VIDEO_LEVEL_MAX						(VS_VIDEOLEVEL_38)
#define VIDEO_LEVEL_4K_MAX					(VS_VIDEOLEVEL_58)
#define VIDEO_PERFECT_AR					(VS_VIDEOAR_4_3)
#define VIDEO_LEVEL_GCONF					(VS_VIDEOLEVEL_33)
#define VIDEO_LEVEL_GCONF_DS				(VS_VIDEOLEVEL_51)

#endif

#define VIDEO_LEVEL_ED						(VS_VIDEOLEVEL_47)
#define VIDEO_HWLEVEL_MIN					(VS_VIDEOLEVEL_58)
#define VIDEO_FRAMERATE_MAX_LIMIT			(30)
#define VIDEO_FRAMERATE_MIN_LIMIT			(15)
#define VIDEO_FRAMERATE_EXTEND_LIMIT		(60)
#define VIDEO_SCREEN_FRAMERATE_MIN_LIMIT	(5)
#define VIDEO_NUMLEVELS_MAX					(256)

struct tc_levelVideo_t
{
	unsigned char level_idc;
	int			  maxMBps;
	int			  maxFrameSizeMB;
	char		  name[128];
};

struct tc_AspectRatio_t
{
	int arw;
	int arh;
};

struct tc_AutoModeDesc_t
{
	unsigned char		level_idc;
	eAspectRatio		ar;
	unsigned int		fps;
	unsigned int		btrLow;
	unsigned int		btrHigh;
	unsigned int		fourcc;
	int					defWidth;
	int					defHeight;
};

extern const char video_mode_name[10][128];
extern const tc_AspectRatio_t tc_AspectRatio[VS_VIDEOAR_NUM_MAX];

class tc_VideoLevelCaps
{
private:
	tc_levelVideo_t		*m_pLevels[VIDEO_NUMLEVELS_MAX];
	unsigned char		m_uMaxLevel;
	unsigned char		Rating2LevelRcv(int rating);
public:
	tc_VideoLevelCaps();
	~tc_VideoLevelCaps();
	unsigned char		GetMaxLevel();
	bool				GetLevelDesc(unsigned char level, tc_levelVideo_t *descLvl);
	unsigned char		CheckLevel(unsigned char level);
	unsigned char		MBps2Level(int maxMBps);
	int					LevelSnd2Rating(unsigned char level);
	unsigned char		MergeRatingVsLevel(int rating, unsigned char level);
};

class tc_AutoLevelCaps
{
private:
	tc_AutoModeDesc_t	*m_pAutoLevels[MAX_NUM_HW_ENCODERS][VIDEO_NUMLEVELS_MAX];
	unsigned char		m_uMinLevel;
	unsigned char		m_uMaxLevel;
	int					m_iLastIndex;
public:
	tc_AutoLevelCaps();
	~tc_AutoLevelCaps();
	unsigned char				GetMaxLevel();
	unsigned char				GetMinLevel();
	bool						GetLevelDesc(unsigned char level, int typeHW, tc_AutoModeDesc_t *descLvl);
	unsigned char				CheckLevel(unsigned char level);
	unsigned char				GetStartBitrateLevel();
	unsigned char				CheckBitrateLevel(int bitrate, bool bUpper, unsigned char uLastLvl);
};

#endif /* VS_VIDEOLEVELCAPS_H */