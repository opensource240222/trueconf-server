
#include "VS_MediaFormat.h"
#include "VS_VideoLevelCaps.h"

#include <algorithm>
#include <cstring>

const char video_mode_name[10][128] =
{
	"Standard Definition Video",
	"Standard Definition Video",
	"Standard Definition Video",
	"High Quality Video",
	"High Quality Video",
	"Enchanced High Quality Video",
	"High Definition Video",
	"High Definition Video",
	"Full High Definition Video",
	"Full High Definition Video",
};

const tc_AspectRatio_t tc_AspectRatio[VS_VIDEOAR_NUM_MAX] =
{
	{16, 9}, {4, 3}, {5, 3}, {16, 10}, {11, 9}, {3, 2}
};

const tc_AutoModeDesc_t tc_AutoVideoModeDescVPX[VS_VIDEOAR_NUM_MAX+1][7] =
{
	{
		{VS_VIDEOLEVEL_28,	VS_VIDEOAR_16_9,  30,    0,   96, VS_VCODEC_VPX,  320,  176}, /// 320x176 @ 30 fps
		{VS_VIDEOLEVEL_32,	VS_VIDEOAR_16_9,  15,   64,  192, VS_VCODEC_VPX,  640,  360}, /// 640x360 @ 15 fps
		{VS_VIDEOLEVEL_36,	VS_VIDEOAR_16_9,  30,  154,  500, VS_VCODEC_VPX,  640,  360}, /// 640x360 @ 30 fps
		{VS_VIDEOLEVEL_47,	VS_VIDEOAR_16_9,  30,  400, 1000, VS_VCODEC_VPX,  864,  480}, /// 864x480 @ 30 fps
		{VS_VIDEOLEVEL_58,	VS_VIDEOAR_16_9,  30,  800, 2000, VS_VCODEC_VPX, 1280,  720}, ///      HD @ 30 fps
		{VS_VIDEOLEVEL_63,	VS_VIDEOAR_16_9,  30, 1600, 4000, VS_VCODEC_VPX, 1920, 1080}, ///     FHD @ 30 fps
		{				0,	VS_VIDEOAR_16_9,   0,	 0,	   0,			  0,    0,    0}, ///     end
	},
	{
		{VS_VIDEOLEVEL_29,	VS_VIDEOAR_4_3,  30,    0,   96, VS_VCODEC_VPX,  320,  240}, /// 320x240 @ 30 fps
		{VS_VIDEOLEVEL_33,	VS_VIDEOAR_4_3,  15,   64,  192, VS_VCODEC_VPX,  640,  480}, /// 640x480 @ 15 fps
		{VS_VIDEOLEVEL_38,	VS_VIDEOAR_4_3,  30,  154,  500, VS_VCODEC_VPX,  640,  480}, /// 640x480 @ 30 fps
		{				0,	VS_VIDEOAR_4_3,   0,	0,	  0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_4_3,   0,	0,	  0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_4_3,   0,	0,	  0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_4_3,   0,	0,	  0,			 0,    0,    0}, ///     end
	},
	{
		{				0,  VS_VIDEOAR_5_3,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,  VS_VIDEOAR_5_3,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,  VS_VIDEOAR_5_3,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,  VS_VIDEOAR_5_3,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,  VS_VIDEOAR_5_3,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,  VS_VIDEOAR_5_3,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,  VS_VIDEOAR_5_3,   0,	   0,	 0,			 0,    0,    0}, ///     end

	},
	{
		{				0, VS_VIDEOAR_16_10,  0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_16_10,  0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_16_10,  0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_16_10,  0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_16_10,  0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_16_10,  0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_16_10,  0,	   0,	 0,			 0,    0,    0}, ///     end
	},
	{
		{				0, VS_VIDEOAR_11_9,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_11_9,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_11_9,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_11_9,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_11_9,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_11_9,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0, VS_VIDEOAR_11_9,   0,	   0,	 0,			 0,    0,    0}, ///     end
	},
	{
		{				0,	VS_VIDEOAR_3_2,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_3_2,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_3_2,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_3_2,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_3_2,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_3_2,   0,	   0,	 0,			 0,    0,    0}, ///     end
		{				0,	VS_VIDEOAR_3_2,   0,	   0,	 0,			 0,    0,    0}, ///     end
	},
};

const tc_AutoModeDesc_t tc_AutoVideoModeDescHardwareH264[] =
{
	{VS_VIDEOLEVEL_28, VS_VIDEOAR_16_9,  30,    0,   96,  VS_VCODEC_VPX,  320,  176},  /// 320x176 @ 30 fps
	{VS_VIDEOLEVEL_32, VS_VIDEOAR_16_9,  15,   64,  192,  VS_VCODEC_VPX,  640,  360},  /// 640x360 @ 15 fps
	{VS_VIDEOLEVEL_36, VS_VIDEOAR_16_9,  30,  154,  500,  VS_VCODEC_VPX,  640,  360},  /// 640x360 @ 30 fps
	{VS_VIDEOLEVEL_47, VS_VIDEOAR_16_9,  30,  400, 1000,  VS_VCODEC_VPX,  864,  480},  /// 864x480 @ 30 fps
	{VS_VIDEOLEVEL_58, VS_VIDEOAR_16_9,  30,  800, 2000, VS_VCODEC_H264, 1280,  720},  ///      HD @ 30 fps
	{VS_VIDEOLEVEL_63, VS_VIDEOAR_16_9,  30, 1600, 4000, VS_VCODEC_H264, 1920, 1080},  ///     FHD @ 30 fps
	{				0, VS_VIDEOAR_16_9,   0,	0,	  0,			  0,    0,    0},  ///	   end
};

#define VIDEO_AUTOLEVEL_SHIFT		(1)

const tc_levelVideo_t tc_VideoLevels[] =
{
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	/// levels 1.x
	{ VS_VIDEOLEVEL_10,      693,    99, " 1.0"}, /// max qCIF	  @  7
	{ VS_VIDEOLEVEL_11,      756,   108, " 1.1"}, /// max 192x144 @  7
	{ VS_VIDEOLEVEL_12,      990,    99, " 1.2"}, /// max qCIF	  @ 10
	{ VS_VIDEOLEVEL_13,     1080,   108, " 1.3"}, /// max 192x144 @ 10
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_15,     1485,    99, " 1.5"}, /// max qCIF	  @ 15
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_17,     1620,   108, " 1.7"}, /// max 192x144 @ 15
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	/// levels 2.x
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_21,     1760,   220, " 2.1"}, /// max 320x176 @  8
	{ VS_VIDEOLEVEL_22,     2100,   300, " 2.2"}, /// max qVGA    @  7
	{ VS_VIDEOLEVEL_23,     2200,   220, " 2.3"}, /// max 320x176 @ 10
	{ VS_VIDEOLEVEL_24,     3000,   300, " 2.4"}, /// max qVGA    @ 10
	{ VS_VIDEOLEVEL_25,     3300,   220, " 2.5"}, /// max 320x176 @ 15
	{ VS_VIDEOLEVEL_26,     4500,   300, " 2.6"}, /// max qVGA    @ 15
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_28,     6600,   220, " 2.8"}, /// max 320x176 @ 30
	{ VS_VIDEOLEVEL_29,     9000,   300, " 2.9"}, /// max qVGA    @ 30
	/// levels 3.x
	{ VS_VIDEOLEVEL_30,     9900,   900, " 3.0"}, /// max 640x360	@ 11
	{ VS_VIDEOLEVEL_31,    12000,  1200, " 3.1"}, /// max VGA		@ 10
	{ VS_VIDEOLEVEL_32,    13500,   900, " 3.2"}, /// max 640x360	@ 15
	{ VS_VIDEOLEVEL_33,    18000,  1200, " 3.3"}, /// max VGA		@ 15 = 4 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_35,    22800,  1200, " 3.5"}, /// max VGA		@ 19 = 5 x (qVGA @ 15)
	{ VS_VIDEOLEVEL_36,    27000,   900, " 3.6"}, /// max 640x360	@ 30 = 6 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_38,    36000,  1200, " 3.8"}, /// max VGA		@ 30 = 8 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	/// levels 4.x
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_43,    40500,  1620, " 4.3"}, /// max 864x480 @ 25 =  9 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_47,    48600,  1620, " 4.7"}, /// max 864x480 @ 30 = 10 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	/// levels 5.x
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_51,    54000,  3600, " 5.1"}, /// max HD @ 15 fps = 12 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_54,    72000,  3600, " 5.4"}, /// max HD @ 20 fps = 16 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_56,    90000,  3600, " 5.6"}, /// max HD @ 25 fps = 20 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_58,   108000,  3600, " 5.8"}, /// max HD @ 30 fps = 24 x (qVGA @ 15)
	{ 0, 0, 0, {} },
	/// levels 6.x
	{ VS_VIDEOLEVEL_60,   121500,  8100, " 6.0"}, /// max FHD @ 15 fps
	{ VS_VIDEOLEVEL_61,   145800,  8100, " 6.1"}, /// max FHD @ 16 fps =    8 x (VGA @ 15)
	{ VS_VIDEOLEVEL_62,   186300,  8100, " 6.2"}, /// max FHD @ 16 fps =   10 x (VGA @ 15)
	{ VS_VIDEOLEVEL_63,   243000,  8100, " 6.3"}, /// max FHD @ 30 fps = 13.5 x (VGA @ 15)
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_65,   288000,  8100, " 6.5"}, /// max FHD @ 35 fps =   16 x (VGA @ 15)
	{ VS_VIDEOLEVEL_66,   324000,  8100, " 6.6"}, /// max FHD @ 40 fps =   18 x (VGA @ 15)
	{ VS_VIDEOLEVEL_67,   405000,  8100, " 6.7"}, /// max FHD @ 50 fps =   22 x (VGA @ 15)
	{ VS_VIDEOLEVEL_68,   437400,  8100, " 6.8"}, /// max FHD @ 54 fps =   24 x (VGA @ 15)
	{ VS_VIDEOLEVEL_69,   486000,  8100, " 6.9"}, /// max FHD @ 60 fps =   27 x (VGA @ 15)
	/// levels 7.x
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_72,   516096, 12288, " 7.2"}, /// max 4XGA	@ 42 fps
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_75,   540672, 12288, " 7.5"}, /// max 4XGA	@ 44 fps
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	/// levels 8.x
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_81,   576000, 19200, " 8.1"}, /// max 16VGA	@ 30 fps
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_83,   614400, 19200, " 8.3"}, /// max 16VGA	@ 32 fps
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_85,   652800, 19200, " 8.5"}, /// max 16VGA	@ 34 fps
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_87,   729600, 19200, " 8.7"}, /// max 16VGA	@ 38 fps
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_89,   806400, 19200, " 8.9"}, /// max 16VGA	@ 42 fps
	/// levels 9.x
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_91,   816000, 24000, " 9.1"}, /// max 20VGA	@ 34 fps
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_93,   912000, 24000, " 9.3"}, /// max 20VGA	@ 36 fps
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_96,   936000, 24000, " 9.6"}, /// max 20VGA	@ 39 fps
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_98,   960000, 24000, " 9.8"}, /// max 20VGA	@ 41 fps
	{ 0, 0, 0, {} },
	/// level 10.x
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_101, 1036800, 34560, "10.1"}, /// max 4K	@ 30 fps
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ 0, 0, 0, {} },
	{ VS_VIDEOLEVEL_108, 2073600, 34560, "10.1"}, /// max 4K	@ 60 fps
	{255, 0, 0, ""}
};

tc_VideoLevelCaps::tc_VideoLevelCaps()
{
	memset(m_pLevels, 0, sizeof(m_pLevels));
	const tc_levelVideo_t *l = tc_VideoLevels;
	while (l->level_idc != 255) {
		if (l->level_idc != 0) {
			m_pLevels[l->level_idc] = new tc_levelVideo_t;
			*m_pLevels[l->level_idc] = *l;
			m_uMaxLevel = l->level_idc;
		}
		l++;
	}
}

tc_VideoLevelCaps::~tc_VideoLevelCaps()
{
	for (unsigned char lvl = 0; lvl <= m_uMaxLevel; lvl++) delete m_pLevels[lvl];
}

unsigned char tc_VideoLevelCaps::GetMaxLevel()
{
	return m_uMaxLevel;
}

bool tc_VideoLevelCaps::GetLevelDesc(unsigned char level, tc_levelVideo_t *descLvl)
{
	if (m_pLevels[level] == 0) return false;
	*descLvl = *m_pLevels[level];
	return true;
}

unsigned char tc_VideoLevelCaps::CheckLevel(unsigned char level)
{
	unsigned char lvl_check = VS_VIDEOLEVEL_10, lvl;
	for (lvl = VS_VIDEOLEVEL_10; lvl <= m_uMaxLevel; lvl++) {
		if (m_pLevels[lvl] == 0) continue;
		if (level < lvl) break;
		lvl_check = lvl;
	}
	return lvl_check;
}

int	tc_VideoLevelCaps::LevelSnd2Rating(unsigned char level)
{
	int rating = 0;

	if (level >= VS_VIDEOLEVEL_63) rating = 870;
	else if (level >= VS_VIDEOLEVEL_60) rating = 830;
	else if (level >= VS_VIDEOLEVEL_58) rating = 790;
	else if (level >= VS_VIDEOLEVEL_51) rating = 750;
	else if (level >= VS_VIDEOLEVEL_47) rating = 690;
	else if (level >= VS_VIDEOLEVEL_38) rating = 600;
	else if (level >= VS_VIDEOLEVEL_33) rating = 530;
	else if (level >= VS_VIDEOLEVEL_28) rating = 430;
	else if (level >= VS_VIDEOLEVEL_26) rating = 350;
	else if (level >= VS_VIDEOLEVEL_24) rating = 140;
	else if (level >= VS_VIDEOLEVEL_15) rating = 100;
	else if (level >= VS_VIDEOLEVEL_12) rating =  50;

	return rating;
}

unsigned char tc_VideoLevelCaps::Rating2LevelRcv(int rating)
{
	unsigned char level = VS_VIDEOLEVEL_24;

	if (rating >= 855)		level = VS_VIDEOLEVEL_89;
	else if (rating >= 807) level = VS_VIDEOLEVEL_85;
	else if (rating >= 782) level = VS_VIDEOLEVEL_83;
	else if (rating >= 710) level = VS_VIDEOLEVEL_75;
	else if (rating >= 662) level = VS_VIDEOLEVEL_65;
	else if (rating >= 600) level = VS_VIDEOLEVEL_58;
	else if (rating >= 566) level = VS_VIDEOLEVEL_47;
	else if (rating >= 496) level = VS_VIDEOLEVEL_43;
	else if (rating >= 383) level = VS_VIDEOLEVEL_38;
	else if (rating >= 141) level = VS_VIDEOLEVEL_33;
	else if (rating >= 100) level = VS_VIDEOLEVEL_28;
	else if (rating >=  70) level = VS_VIDEOLEVEL_26;

	return level;
}

unsigned char tc_VideoLevelCaps::MergeRatingVsLevel(int rating, unsigned char level)
{
	if (level == 0) level = Rating2LevelRcv(rating);
	return level;
}

unsigned char tc_VideoLevelCaps::MBps2Level(int maxMBps)
{
	unsigned char lvl_check = VS_VIDEOLEVEL_10, lvl;
	for (lvl = VS_VIDEOLEVEL_10; lvl <= m_uMaxLevel; lvl++) {
		if (m_pLevels[lvl] == 0) continue;
		if (maxMBps < m_pLevels[lvl]->maxMBps) break;
		lvl_check = lvl;
	}
	return lvl_check;
}

/// AutoLevelCaps

tc_AutoLevelCaps::tc_AutoLevelCaps()
{
	int i;
	m_iLastIndex = 0;
	memset(m_pAutoLevels, 0, sizeof(m_pAutoLevels));
	for (i = 0; i < MAX_NUM_HW_ENCODERS; i++) {
		const tc_AutoModeDesc_t *al = 0;
		if (i == ENCODER_SOFTWARE) {
			al = tc_AutoVideoModeDescVPX[VIDEO_PERFECT_AR];
			m_uMinLevel = al->level_idc;
		} else {
			al = tc_AutoVideoModeDescHardwareH264;
		}
		while (al->level_idc != 0) {
			m_pAutoLevels[i][al->level_idc] = new tc_AutoModeDesc_t;
			*m_pAutoLevels[i][al->level_idc] = *al;
			if (i == ENCODER_SOFTWARE) {
				m_uMaxLevel = std::min<unsigned char>(al->level_idc, VIDEO_LEVEL_MAX);
				m_iLastIndex++;
			}
			al++;
		}
	}
}

tc_AutoLevelCaps::~tc_AutoLevelCaps()
{
	unsigned char lvl;
	int i;
	for (lvl = m_uMinLevel; lvl <= m_uMaxLevel; lvl++) {
		for (i = 0; i < MAX_NUM_HW_ENCODERS; i++) delete m_pAutoLevels[i][lvl];
	}
}

unsigned char tc_AutoLevelCaps::GetMinLevel()
{
	return m_uMinLevel;
}

unsigned char tc_AutoLevelCaps::GetMaxLevel()
{
	return m_uMaxLevel;
}

bool tc_AutoLevelCaps::GetLevelDesc(unsigned char level, int typeHW, tc_AutoModeDesc_t *descLvl)
{
	if (level < m_uMinLevel) {
		if (tc_VideoLevels[level].level_idc == 0) return false;
		*descLvl = *m_pAutoLevels[typeHW][m_uMinLevel];
		descLvl->fps = tc_VideoLevels[level].maxMBps / tc_VideoLevels[level].maxFrameSizeMB;
	} else {
		if (m_pAutoLevels[typeHW][level] == 0) return false;
		*descLvl = *m_pAutoLevels[typeHW][level];
	}
	return true;
}

unsigned char tc_AutoLevelCaps::CheckLevel(unsigned char level)
{
	if (level < m_uMinLevel) return level;
	unsigned char lvl_check = m_uMinLevel, lvl;
	for (lvl = m_uMinLevel; lvl <= m_uMaxLevel; lvl++) {
		if (m_pAutoLevels[ENCODER_SOFTWARE][lvl] == 0) continue;
		if (level < lvl) break;
		lvl_check = lvl;
	}
	return lvl_check;
}

unsigned char tc_AutoLevelCaps::GetStartBitrateLevel()
{
	return tc_AutoVideoModeDescVPX[VIDEO_PERFECT_AR][VIDEO_AUTOLEVEL_SHIFT].level_idc;
}

unsigned char tc_AutoLevelCaps::CheckBitrateLevel(int bitrate, bool bUpper, unsigned char uLastLvl)
{
	const tc_AutoModeDesc_t *al = tc_AutoVideoModeDescVPX[VIDEO_PERFECT_AR];
	if (bUpper) {
		while (al->level_idc < m_uMaxLevel) {
			if (((unsigned int)bitrate < al->btrHigh) && (uLastLvl <= al->level_idc)) break;
			al++;
		}
	} else {
		al = &tc_AutoVideoModeDescVPX[VIDEO_PERFECT_AR][m_iLastIndex-1];
		while (al->level_idc >= m_uMinLevel) {
			if (((unsigned int)bitrate > al->btrLow) && (uLastLvl >= al->level_idc)) break;
			al--;
		}
	}
	return al->level_idc;
}

