#pragma once

#include <memory>
#include <mutex>
#include <vector>

class VS_MediaFormat;
class VS_VideoCodecState;

class VS_VideoCodecManager
{

public:

	VS_VideoCodecManager();
	~VS_VideoCodecManager();
	bool	Init(VS_MediaFormat *mf, unsigned char senderLvl = 0);
	void	Release();
	bool	SetBitrate(int baseBitrate, int maxBitrate, int framerate);
	int		GetBitrate() {
		return m_baseBitrate;
	}
	int		Convert(unsigned char *in, unsigned char *out, bool *key);
	void	GetResolutionBaseLayer(int *width, int *height);
	std::vector<int> GetLayerFrameSizeMBs();
	uint32_t GetNumThreads() const;
	bool	IsValid() {
		return m_bValid;
	}

private:

	void UpdateBitrate();

public:

	enum eBitrateLimitSVC
	{
		BTR_LIMIT_BASE = 0,
		BTR_LIMIT_LOW = 1,
		BTR_LIMIT_HIGH,
		BTR_LIMIT_MAX
	};

	static const int MAX_SPATIAL_LAYER = 5;
	static const int slBitrateLimit[BTR_LIMIT_MAX][MAX_SPATIAL_LAYER];
	static const double slBitrateCoefs[MAX_SPATIAL_LAYER];

private:

	std::mutex m_mtxEncode;
	std::vector<std::unique_ptr<VS_VideoCodecState>> m_codecDesc;
	std::vector <int> m_layerFrameSizeMBs;
	int m_maxWidth, m_maxHeight;
	int m_baseWidth, m_baseHeight;
	int m_setFramerate, m_setFramerateSet;
	int m_baseBitrate, m_maxBitrate, m_baseBitrateSet, m_maxBitrateSet;
	int m_numSLayers, m_limitSLayers, m_maxSLayers;
	unsigned int m_supportMode;
	unsigned int m_currentMode;
	double m_maxBitrateK;
	uint32_t m_numThreads = 1;
	bool m_bValid;

};
