
#include <math.h>
#include "VS_H265Parser.h"

enum NalUnitType
{
	/// Table 7 - 1 - NAL unit type codes and NAL unit type classes
	NAL_TRAIL_N = 0,
	NAL_TRAIL_R = 1,
	NAL_TSA_N,
	NAL_TSA_R,
	NAL_STSA_N,
	NAL_STSA_R,
	NAL_RADL_N,
	NAL_RADL_R,
	NAL_RASL_N,
	NAL_RASL_R,
	NAL_BLA_W_LP = 16,
	NAL_BLA_W_RADL = 17,
	NAL_BLA_N_LP,
	NAL_IDR_W_RADL,
	NAL_IDR_N_LP,
	NAL_CRA,
	NAL_VPS = 32,
	NAL_SPS = 33,
	NAL_PPS = 34,
	NAL_ACCESS_UNIT_DELIMITER,
	NAL_EOS,
	NAL_EOB,
	NAL_FILLER_DATA,
	NAL_PREFIX_SEI,
	NAL_SUFFIX_SEI,
	NAL_INVALID = 64
};

struct h265Bitstream {
unsigned char *base; ///< point32_ter to the first byte of the buffer
unsigned char *bs; ///< point32_ter to the current position of the buffer
unsigned char *end; ///< point32_ter to the last byte of the buffer
int32_t bit_left; ///< indicates the number of available bits in the byte point32_ted by m_bs

h265Bitstream(unsigned char *buffer, int32_t buffer_size) : base(buffer), bs(buffer), end(buffer + buffer_size), bit_left(8) {
};
};

// get n bit from bitstream
int GetBits(const unsigned int* &current_data, int &offset, unsigned int nbits);
// get 1 bit from bitstream
unsigned int GetBit(const unsigned int * &current_data, int & offset);
// get vlc code from bitstream
unsigned int GetVLC(const unsigned int * &current_data, int & offset, bool isSigned);
// get nal size
int GetNALSize(const unsigned char *in, int insize);

// put 1 bit to h.265 bitstream
void PutBit(h265Bitstream *bs, uint32_t val) {
	if (bs->bs < bs->end) {
		*bs->bs <<= 1;
		*bs->bs |= val;
		bs->bit_left--;
		if (bs->bit_left == 0) {
			bs->bs++;
			bs->bit_left = 8;
		}
	}
}

// put n bits to h.265 bitstream
void PutBits(h265Bitstream *bs, int32_t length, uint32_t val) {
	if (bs->end - bs->bs <= 4)
		return;

	while (length > 0) {
		if (length < 32) {
			val &= (1u << length) - 1;
		}
		if (length < bs->bit_left) {
			*bs->bs = (*bs->bs << length) | val;
			bs->bit_left -= length;
			break;
		}
		else {
			*bs->bs = (*bs->bs << bs->bit_left) | (val >> (length - bs->bit_left));
			length -= bs->bit_left;
			bs->bs++;
			bs->bit_left = 8;
		}
	}
}

void PutVLC_ue(h265Bitstream *bs, uint32_t val) {
	uint32_t length = 0;
	int32_t i = 0, n = 0,
		info_length = 0, bits = 0;

	n = val + 1;
	i = -1;

	while (n) {
		n >>= 1;
		i++;
	}

	length = 1 + (i << 1);

	if (length == 1) {
		PutBit(bs, 1);
	}
	else {
		info_length = (length - 1) >> 1;
		bits = val + 1 - (1 << info_length);
		PutBits(bs, info_length + 1, 1);
		PutBits(bs, info_length, bits);
	}
}

void PutVLC_se(h265Bitstream *bs, int32_t val) {
	PutVLC_ue(bs, val > 0 ? val * 2 - 1 : -val * 2);
}

void PutVLC_te(h265Bitstream *bs, int32_t n, int32_t val) {
	if (n > 1) {
		PutVLC_ue(bs, val);
	}
	else if (n == 1) {
		PutBit(bs, 1 & ~val);
	}
}

void profile_tier_level(int profilePresentFlag, uint8_t maxNumSubLayersMinus1, const uint32_t *&ipb, int32_t &offset)
{
	if (profilePresentFlag) {
		uint8_t general_profile_compatibility_flag[32] = { 0 };
		GetBits(ipb, offset, 2);
		GetBits(ipb, offset, 1);
		uint8_t general_profile_idc = GetBits(ipb, offset, 5);
		for (int j = 0; j < 32; j++) {
			general_profile_compatibility_flag[j] = GetBit(ipb, offset);
		}
		GetBit(ipb, offset);
		GetBit(ipb, offset);
		GetBit(ipb, offset);
		GetBit(ipb, offset);
		if (general_profile_idc == 4 || general_profile_compatibility_flag[4] ||
			general_profile_idc == 5 || general_profile_compatibility_flag[5] ||
			general_profile_idc == 6 || general_profile_compatibility_flag[6] ||
			general_profile_idc == 7 || general_profile_compatibility_flag[7]) {
			/* The number of bits in this syntax structure is not affected by this condition */
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBits(ipb, offset, 34);
		}
		else {
			GetBits(ipb, offset, 43);
		}
		if ((general_profile_idc >= 1 && general_profile_idc <= 5) ||
			general_profile_compatibility_flag[1] || general_profile_compatibility_flag[2] ||
			general_profile_compatibility_flag[3] || general_profile_compatibility_flag[4] ||
			general_profile_compatibility_flag[5]) {
			/* The number of bits in this syntax structure is not affected by this condition */
			GetBit(ipb, offset);
		}
		else {
			GetBit(ipb, offset);
		}
	}
	GetBits(ipb, offset, 8);
	uint8_t sub_layer_profile_present_flag[9] = { 0 };
	uint8_t sub_layer_level_present_flag[9] = { 0 };
	for (int j = 0; j < maxNumSubLayersMinus1; j++) {
		sub_layer_profile_present_flag[j] = GetBit(ipb, offset);
		sub_layer_level_present_flag[j] = GetBit(ipb, offset);
	}
	if (maxNumSubLayersMinus1 > 0) {
		for (int i = maxNumSubLayersMinus1; i < 8; i++) {
			GetBits(ipb, offset, 2);
		}
	}
	uint8_t sub_layer_profile_compatibility_flag[9][32] = {};
	uint8_t sub_layer_profile_idc[9] = { 0 };
	for (int i = 0; i < maxNumSubLayersMinus1; i++) {
		if (sub_layer_profile_present_flag[i]) {
			GetBits(ipb, offset, 2);
			GetBits(ipb, offset, 1);
			sub_layer_profile_idc[i] = GetBits(ipb, offset, 5);
			for (int j = 0; j < 32; j++) {
				sub_layer_profile_compatibility_flag[i][j] = GetBit(ipb, offset);
			}
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			GetBit(ipb, offset);
			if (sub_layer_profile_idc[i] == 4 || sub_layer_profile_compatibility_flag[i][4] ||
				sub_layer_profile_idc[i] == 5 || sub_layer_profile_compatibility_flag[i][5] ||
				sub_layer_profile_idc[i] == 6 || sub_layer_profile_compatibility_flag[i][6] ||
				sub_layer_profile_idc[i] == 7 || sub_layer_profile_compatibility_flag[i][7]) {
				/* The number of bits in this syntax structure is not affected by this condition */
				GetBit(ipb, offset);
				GetBit(ipb, offset);
				GetBit(ipb, offset);
				GetBit(ipb, offset);
				GetBit(ipb, offset);
				GetBit(ipb, offset);
				GetBit(ipb, offset);
				GetBit(ipb, offset);
				GetBit(ipb, offset);
				GetBits(ipb, offset, 34);
			}
			else {
				GetBits(ipb, offset, 43);
			}
			if ((sub_layer_profile_idc[i] >= 1 && sub_layer_profile_idc[i] <= 5) ||
				sub_layer_profile_compatibility_flag[1] || sub_layer_profile_compatibility_flag[2] ||
				sub_layer_profile_compatibility_flag[3] || sub_layer_profile_compatibility_flag[4] ||
				sub_layer_profile_compatibility_flag[5]) {
				/* The number of bits in this syntax structure is not affected by this condition */
				GetBit(ipb, offset);
			}
			else {
				GetBit(ipb, offset);
			}
		}
		if (sub_layer_level_present_flag[i]) {
			GetBits(ipb, offset, 8);
		}
	}
}

// get frame resolution from NAL
// return:
// 0 - successfully decode frame resolution (sps)
// -1 - inter frame (trail, tsa, stsa, rasl, radl)
// -2 - other nal, try next nal
// -3 - irap frame (idr, cra, bla)

int32_t ResolutionFromNAL_H265(const unsigned char *in, int32_t insize, int32_t& width, int32_t& height) {
	const unsigned char *pb = in;
	int32_t offset(31);
	if (insize < 1) {
		return -1;
	}
	if ((pb[0] & 0x80) != 0) {
		return -2;
	}
	unsigned char typeNAL = (pb[0] & 0x7e) >> 1;
	if (typeNAL == NAL_TRAIL_N ||
		typeNAL == NAL_TRAIL_R ||
		typeNAL == NAL_TSA_N ||
		typeNAL == NAL_TSA_R ||
		typeNAL == NAL_STSA_N ||
		typeNAL == NAL_STSA_R ||
		typeNAL == NAL_RADL_N ||
		typeNAL == NAL_RADL_R ||
		typeNAL == NAL_RASL_N ||
		typeNAL == NAL_RASL_R)
	{
		return -1;
	}
	if (typeNAL == NAL_BLA_W_LP ||
		typeNAL == NAL_BLA_W_RADL ||
		typeNAL == NAL_BLA_N_LP ||
		typeNAL == NAL_IDR_W_RADL ||
		typeNAL == NAL_IDR_N_LP ||
		typeNAL == NAL_CRA)
	{
		return -3;
	}
	if (typeNAL != NAL_SPS) {
		return -2;
	}
	pb += 2;
	// delete prevention bytes before parsing (0x03)
	int nalSize = GetNALSize(in, insize), k(0), i(0);
	uint8_t *extract0x03 = new uint8_t[nalSize];
	for (i = 0; i < nalSize - 2; ) {
		if (pb[i] == 0x00 && pb[i + 1] == 0x00 && pb[i + 2] == 0x03) {
			extract0x03[k++] = pb[i++];
			extract0x03[k++] = pb[i++];
			i++;
		}
		else {
			extract0x03[k++] = pb[i++];
		}
	}
	while (i < nalSize) {
		extract0x03[k++] = pb[i++];
	}
	// parse SPS
	const uint32_t *ipb = (uint32_t*)extract0x03;
	uint8_t sps_video_parameter_set_id = GetBits(ipb, offset, 4);
	uint8_t sps_max_sub_layers_minus1 = GetBits(ipb, offset, 3);
	uint8_t sps_temporal_id_nesting_flag = GetBit(ipb, offset);
	profile_tier_level(1, sps_max_sub_layers_minus1, ipb, offset);
	uint32_t sps_seq_parameter_set_id = GetVLC(ipb, offset, false);
	uint32_t chroma_format_idc = GetVLC(ipb, offset, false);
	if (chroma_format_idc == 3) {
		uint8_t separate_colour_plane_flag = GetBit(ipb, offset);
	}
	width = GetVLC(ipb, offset, false);
	height = GetVLC(ipb, offset, false);
	uint8_t conformance_window_flag = GetBit(ipb, offset);
	if (conformance_window_flag) {
		uint32_t conf_win_left_offset = GetVLC(ipb, offset, false);
		uint32_t conf_win_right_offset = GetVLC(ipb, offset, false);
		uint32_t conf_win_top_offset = GetVLC(ipb, offset, false);
		uint32_t conf_win_bottom_offset = GetVLC(ipb, offset, false);
		// Rec. ITU-T H.265, Table 6-1
		const int32_t subWidthC[4] = { 1, 2, 2, 1 };
		const int32_t subHeightC[4] = { 1, 2, 1, 1 };
		int32_t cropH = subHeightC[chroma_format_idc] * (conf_win_top_offset + conf_win_bottom_offset);
		int32_t cropW = subWidthC[chroma_format_idc] * (conf_win_right_offset + conf_win_left_offset);
		width -= cropW;
		height -= cropH;
	}
	delete[] extract0x03;
    return 0;
}

// get slice type from NAL
// return:
// 0 - irap frame (idr, cra, bla)
// -1 - inter frame (trail, tsa, stsa, rasl, radl)
// -2 - other nal, try next nal

int32_t TypeSliceFromNAL_H265(unsigned char *in, int32_t insize) {
	unsigned char *pb = in;
	if (insize < 1) {
		return -1;
	}
	if ((pb[0] & 0x80) != 0) {
		return -2;
	}
	unsigned char typeNAL = (pb[0] & 0x7e) >> 1;
	if (typeNAL == NAL_TRAIL_N ||
		typeNAL == NAL_TRAIL_R ||
		typeNAL == NAL_TSA_N ||
		typeNAL == NAL_TSA_R ||
		typeNAL == NAL_STSA_N ||
		typeNAL == NAL_STSA_R ||
		typeNAL == NAL_RADL_N ||
		typeNAL == NAL_RADL_R ||
		typeNAL == NAL_RASL_N ||
		typeNAL == NAL_RASL_R)
	{
		return -1;
	}
	if (typeNAL == NAL_BLA_W_LP ||
		typeNAL == NAL_BLA_W_RADL ||
		typeNAL == NAL_BLA_N_LP ||
		typeNAL == NAL_IDR_W_RADL ||
		typeNAL == NAL_IDR_N_LP ||
		typeNAL == NAL_CRA)
	{
		return 0;
	}
    return -2;
}

// get frame resolution from bitstream
// return:
// 0 - successfully decode frame resolution (sps)
// -2 - irap frame (idr, cra, bla)
// -1 - inter frame or incorrect, try next compressed frame

int32_t ResolutionFromBitstream_H265(const unsigned char *in, int32_t insize, int32_t& width, int32_t& height) {
    int32_t res = -1;
    const unsigned char *pb = in;

    if (insize < 4)
        return -1;

    while (4 <= insize) {
        if ((0 == pb[0]) && (0 == pb[1]) && (1 == pb[2])) { // start code
            pb += 3;
            res = ResolutionFromNAL_H265(pb, insize, width, height);
            if (res == 0) return 0;
            if (res == -1) return -1;
            if (res == -3) return -2;
        }
        pb++;
        insize--;
    }

    return -1;
}

// get type slice from bitstream
// return:
// 0 - successfully decode type slice
// < 0 - try next compressed frame

int32_t TypeSliceFromBitstream_H265(unsigned char *in, int32_t insize, int32_t& irap) {
    int32_t res = -1;
    unsigned char *pb = in;
	irap = 1;

    if (insize < 4)
        return -1;

    while (4 <= insize) {
        if ((0 == pb[0]) && (0 == pb[1]) && (1 == pb[2])) { // start code
            pb += 3;
            res = TypeSliceFromNAL_H265(pb, insize);
            if (res == 0) {
				irap = 0;
                return 0;
            }
            if (res == -1) return -1;
        }
        pb++;
        insize--;
    }

    return -1;
}
