#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "VS_H264Parser.h"
#include "std-generic/cpplib/hton.h"

struct h264Bitstream
{
	unsigned char *base;	///< pointer to the first byte of the buffer
	unsigned char *bs;		///< pointer to the current position of the buffer
	unsigned char *end;		///< pointer to the last byte of the buffer
	int			  bit_left;	///< indicates the number of available bits in the byte pointed by m_bs
	h264Bitstream(unsigned char *buffer, int buffer_size) : base(buffer), bs(buffer), end(buffer + buffer_size), bit_left(8) {};
};

// get n bit from bitstream
int GetBits(const unsigned int* &current_data, int &offset, unsigned int nbits);

// get 1 bit from bitstream
unsigned int GetBit(const unsigned int* &current_data, int& offset)
{
    unsigned int x;
	unsigned int cdata0 = vs_ntohl(current_data[0]);
	unsigned int cdata1 = vs_ntohl(current_data[1]);

    offset--;
    if(offset >= 0) {
        x = cdata0 >> (offset + 1);
    } else {
        offset += 32;

        x = cdata1 >> (offset);
        x >>= 1;
        x += cdata0 << (31 - offset);
        current_data++;
    }
    return (x & (((unsigned int)0x01 << 1) - 1));
}

// put 1 bit to h.264 bitstream
void PutBit(h264Bitstream *bs, unsigned int val)
{
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

// put n bits to h.264 bitstream
void PutBits(h264Bitstream *bs, int length, unsigned int val)
{
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
        } else {
            *bs->bs = (*bs->bs << bs->bit_left) | (val >> (length - bs->bit_left));
            length -= bs->bit_left;
            bs->bs++;
            bs->bit_left = 8;
        }
    }
}

void PutVLC_ue(h264Bitstream *bs, unsigned int val)
{
	unsigned int length = 0;
    int i = 0, n = 0,
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
	} else {
        info_length = (length - 1) >> 1;
        bits = val + 1 - (1 << info_length);
        PutBits(bs, info_length + 1, 1);
        PutBits(bs, info_length, bits);
    }
}

void PutVLC_se(h264Bitstream *bs, int val)
{
	PutVLC_ue(bs, val > 0 ? val * 2 - 1 : -val * 2);
}

void PutVLC_te(h264Bitstream *bs, int n, int val)
{
    if (n > 1) {
		PutVLC_ue(bs, val);
    } else if (n == 1) {
		PutBit(bs, 1&~val);
    }
}

// het vlc code from bitstream
unsigned int GetVLC(const unsigned int* &current_data, int& offset, bool isSigned)
{
	int leadingZeroBits = -1, b = 0, i = 0;
	unsigned int codeNum = 0;

	for (b = 0; !b; leadingZeroBits++)
		b = GetBit(current_data, offset);
	codeNum = (1 << leadingZeroBits) - 1;
	for (i = 0; i < leadingZeroBits; i++)
		codeNum = codeNum + (GetBit(current_data, offset) << (leadingZeroBits - i - 1));
	if (isSigned) {
		codeNum = (unsigned int)(ceil((double)codeNum / 2.0)) * (2 * (codeNum % 2) - 1);
	}

	return codeNum;
}

void SkipScalingListXxX(const unsigned int* &stream, int &offset, int X)
{
    unsigned int lastScale = 8;
    unsigned int nextScale = 8;
	int sizeList = X * X;
    int j;
    for (j = 0; j < sizeList; j++) {
        if (nextScale != 0) {
            int delta_scale = GetVLC(stream, offset, true);
            nextScale = (lastScale + delta_scale + 256) & 0xff;
        }
        lastScale = (nextScale == 0) ? (unsigned char)lastScale : (unsigned char)nextScale;
    }
}

// get frame resolution from NAL
// return:
// 0 - successfully decode frame resolution
// < 0 - try next NAL
int	ResolutionFromNAL_H264(const void *in, int insize, int& width, int& height)
{
	const unsigned char *pb = static_cast<const unsigned char*>(in);
	int offset = 31;
	unsigned int codeNum = 0, i = 0;
	int crop_l = 0, crop_r = 0, crop_t = 0, crop_b = 0;
	int frame_mbps_only_flag = 0;
	unsigned char profile_idc = 0, level_idc = 0;
	unsigned int chroma_format_idc = 0;
	int mbW = 0, mbH = 0;

    if (insize < 1)
        return -1;

	unsigned char typeNAL = (pb[0] & 0x1f);

	if (typeNAL == 0x01) return -1;
	if (typeNAL == 0x05) return -3;
	if (typeNAL != 0x07) return -2;

	pb++;

	// parse SPS
	const unsigned int *ipb = (unsigned int*)pb;
	unsigned int first = *(unsigned int*)ipb;
	// profile
	profile_idc = first & 0x000000ff;
	offset -= 8;
	//
	codeNum = GetBit(ipb, offset);
	codeNum = GetBit(ipb, offset);
	codeNum = GetBit(ipb, offset);
	codeNum = GetBit(ipb, offset);
	codeNum = GetBit(ipb, offset);
	codeNum = GetBit(ipb, offset);
	codeNum = GetBit(ipb, offset);
	codeNum = GetBit(ipb, offset);
	// level
	level_idc = ((first >> 16) & 0x000000ff);
	offset -= 8;
	// id
	codeNum = GetVLC(ipb, offset, false);
	// 7.3.2.1.1 "Sequence parameter set data syntax"
	// chapter of H264 standard for full list of profiles with chrominance
	if (profile_idc == 100 ||
		profile_idc == 110 ||
		profile_idc == 118 ||
		profile_idc == 122 ||
		profile_idc == 128 ||
		profile_idc == 244 ||
		profile_idc == 44  ||
		profile_idc == 83  ||
		profile_idc == 86)
	{
		chroma_format_idc = GetVLC(ipb, offset, false);
		if (chroma_format_idc == 3) {
			codeNum = GetBit(ipb, offset);
		}
		codeNum = GetVLC(ipb, offset, false) + 8;
		codeNum = GetVLC(ipb, offset, false) + 8;
		if (chroma_format_idc == 0) {

		}
		codeNum = GetBit(ipb, offset);
		codeNum = GetBit(ipb, offset);
		if (codeNum > 0) {
			// 0
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 1
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 2
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 3
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 4
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 5
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 0
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 8);
			}
			// 1
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 8);
			}
		} else {

		}
	} else {
		chroma_format_idc = 1;
	}
	// log2 max frame num
	codeNum = GetVLC(ipb, offset, false) + 4;
	// pic order cnt type (0..2)
	codeNum = GetVLC(ipb, offset, false);
	if (codeNum == 0) {
		codeNum = GetVLC(ipb, offset, false) + 4;
	} else if (codeNum == 1){
		codeNum = GetBit(ipb, offset);
		codeNum = GetVLC(ipb, offset, true);
		codeNum = GetVLC(ipb, offset, true);
		codeNum = GetVLC(ipb, offset, false);
		for (i = 0; i < codeNum; i++)
			GetVLC(ipb, offset, true);
	}
	// num reference frames
	codeNum = GetVLC(ipb, offset, false);
	codeNum = GetBit(ipb, offset);
	// get resolution frame
	mbW = GetVLC(ipb, offset, false) + 1;
	mbH = GetVLC(ipb, offset, false) + 1;
	// frame mbs only flag
	frame_mbps_only_flag = GetBit(ipb, offset);
	mbH = (2 - frame_mbps_only_flag) * mbH;
	width = mbW * 16;
	height = mbH * 16;
	if (frame_mbps_only_flag == 0) {
		codeNum = GetBit(ipb, offset);
	}
	// direct 8x8
	codeNum = GetBit(ipb, offset);
	// frame cropping flag
	codeNum = GetBit(ipb, offset);
	if (codeNum > 0) {
		crop_l = GetVLC(ipb, offset, false);
		crop_r = GetVLC(ipb, offset, false);
		crop_t = GetVLC(ipb, offset, false);
		crop_b = GetVLC(ipb, offset, false);
		// 14496-10, Table 6-1
		const int subWidthC[4]  = {1, 2, 2, 1};
		const int subHeightC[4] = {1, 2, 1, 1};
		int cropH = subHeightC[chroma_format_idc] * (2 - frame_mbps_only_flag) * (crop_t + crop_b);
		int cropW = subWidthC[chroma_format_idc] * (crop_r + crop_l);
		width -= cropW;
		height -= cropH;
	}

    return 0;
}

// get slice type from NAL
// return:
// 0 - successfully decode frame resolution
// < 0 - try next NAL
int	TypeSliceFromNAL_H264(const void *in, int insize)
{
	const unsigned char *pb = static_cast<const unsigned char*>(in);

    if (insize < 1)
        return -1;

	unsigned char typeNAL = (pb[0] & 0x1f);

	if (typeNAL == 0x01) return -1;
	if (typeNAL == 0x05) return 0;

	return -2;
}

// get frame resolution from bitstream
// return:
// 0 - successfully decode frame resolution
// < 0 - try next compressed frame
int	ResolutionFromBitstream_H264(const void *in, int insize, int& width, int& height)
{
	int res = -1;
	const unsigned char *pb = static_cast<const unsigned char*>(in);

    if (insize < 4)
        return -1;

    while (4 <= insize) {
		if ((0 == pb[0]) && (0 == pb[1]) && (1 == pb[2])) { // start code
			pb += 3;
			insize -= 3;
			res = ResolutionFromNAL_H264(pb, insize, width, height);
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
// 0 - successfully decode frame resolution
// < 0 - try next compressed frame
int	TypeSliceFromBitstream_H264(const void *in, int insize, int& bIdrSlice)
{
	int res = -1;
	const unsigned char *pb = static_cast<const unsigned char*>(in);
	bIdrSlice = 1;

    if (insize < 4)
        return -1;

    while (4 <= insize) {
		if ((0 == pb[0]) && (0 == pb[1]) && (1 == pb[2])) { // start code
			pb += 3;
			insize -= 3;
			res = TypeSliceFromNAL_H264(pb, insize);
			if (res == 0) {
				bIdrSlice = 0;
				return 0;
			}
			if (res == -1) return -1;
		}
        pb++;
        insize--;
    }

    return -1;
}

// get sps from bitstream
// return:
// 0 - successfully decode frame resolution
// < 0 - try next compressed frame
int	SPSFromBitstream_H264(const void *in, int insize, vs_sps *sps)
{
	int res = -1;
	const unsigned char *pb = static_cast<const unsigned char*>(in);

    if (insize < 4)
        return -1;

    while (4 <= insize) {
		if ((0 == pb[0]) && (0 == pb[1]) && (1 == pb[2])) { // start code
			pb += 3;
			insize -= 3;

			const unsigned char *src = pb;
			unsigned char *cutFrame = new unsigned char [insize];

			/// cutting rtp frame
			int nalSize = insize;
			int i = 0;
			for (; i < nalSize - 2; i++) {
				if (src[i] == 0x00 && src[i+1] == 0x00 && src[i+2] == 0x03) {
					cutFrame[i++] = 0x00;
					cutFrame[i++] = 0x00;
					src++;
					nalSize--;
				}
				cutFrame[i] = src[i];
			}
			cutFrame[i] = src[i];
			++i;
			if (src[i]) {
				cutFrame[i] = src[i];
				++i;
			}

			res = SPSFromNAL_H264(cutFrame, insize, sps);

			delete [] cutFrame;

			if (res == 0) return 0;
			if (res == -1) return -1;
			if (res == -3) return -2;
		}
        pb++;
        insize--;
    }

    return -1;
}

void ParseVUI_H264(vs_vui *vui, const unsigned int* &ipb, int& offset)
{
	vui->aspect_ratio_info_present = GetBit(ipb, offset);
	if (vui->aspect_ratio_info_present > 0) {
		vui->aspect_ratio_idc = GetBits(ipb, offset, 8);
		if (vui->aspect_ratio_idc == 255) {
			vui->sar_width = GetBits(ipb, offset, 16);
			vui->sar_height = GetBits(ipb, offset, 16);
		}
	}
	vui->overscan_info_present = GetBit(ipb, offset);
	if (vui->overscan_info_present > 0) {
		vui->overscan_info = GetBit(ipb, offset);
	}
	vui->signal_type_present = GetBit(ipb, offset);
	if (vui->signal_type_present > 0) {
		vui->vidformat = GetBits(ipb, offset, 3);
		vui->fullrange = GetBit(ipb, offset);
		vui->color_description_present = GetBit(ipb, offset);
		if (vui->color_description_present > 0) {
			vui->colorprim = GetBits(ipb, offset, 8);
			vui->transfer = GetBits(ipb, offset, 8);
			vui->colmatrix = GetBits(ipb, offset, 8);
		}
	}
	vui->chroma_loc_info_present = GetBit(ipb, offset);
	if (vui->chroma_loc_info_present > 0) {
		vui->chroma_loc_top = GetVLC(ipb, offset, false);
		vui->chroma_loc_bottom = GetVLC(ipb, offset, false);
	}
	vui->timing_info_present = GetBit(ipb, offset);
	if (vui->timing_info_present > 0) {
		vui->num_units_in_tick = GetBits(ipb, offset, 32);
		vui->time_scale = GetBits(ipb, offset, 32);
		vui->fixed_frame_rate = GetBit(ipb, offset);
	}
	vui->nal_hrd_parameters_present = GetBit(ipb, offset);
	if (vui->nal_hrd_parameters_present > 0) {
		vui->hrd.cpb_cnt_minus1 = GetVLC(ipb, offset, false);
		vui->hrd.bit_rate_scale = GetBits(ipb, offset, 4);
		vui->hrd.cpb_size_scale = GetBits(ipb, offset, 4);
		for (int i = 0; i <= vui->hrd.cpb_cnt_minus1; i++) {
			vui->hrd.bit_rate_value_minus1[i] = GetVLC(ipb, offset, false);
			vui->hrd.cpb_size_value_minus1[i] = GetVLC(ipb, offset, false);
			vui->hrd.cbr_flag[i] = GetBit(ipb, offset);
		}
		vui->hrd.initial_cpb_removal_delay_length_minus1 = GetBits(ipb, offset, 5);
		vui->hrd.cpb_removal_delay_length_minus1 = GetBits(ipb, offset, 5);
		vui->hrd.dpb_output_delay_length_minus1 = GetBits(ipb, offset, 5);
		vui->hrd.time_offset_length = GetBits(ipb, offset, 5);
	}
	vui->vcl_hrd_parameters_present = GetBit(ipb, offset);
	if (vui->vcl_hrd_parameters_present > 0) {
		vui->vcl_hrd.cpb_cnt_minus1 = GetVLC(ipb, offset, false);
		vui->vcl_hrd.bit_rate_scale = GetBits(ipb, offset, 4);
		vui->vcl_hrd.cpb_size_scale = GetBits(ipb, offset, 4);
		for (int i = 0; i <= vui->vcl_hrd.cpb_cnt_minus1; i++) {
			vui->vcl_hrd.bit_rate_value_minus1[i] = GetVLC(ipb, offset, false);
			vui->vcl_hrd.cpb_size_value_minus1[i] = GetVLC(ipb, offset, false);
			vui->vcl_hrd.cbr_flag[i] = GetBit(ipb, offset);
		}
		vui->vcl_hrd.initial_cpb_removal_delay_length_minus1 = GetBits(ipb, offset, 5);
		vui->vcl_hrd.cpb_removal_delay_length_minus1 = GetBits(ipb, offset, 5);
		vui->vcl_hrd.dpb_output_delay_length_minus1 = GetBits(ipb, offset, 5);
		vui->vcl_hrd.time_offset_length = GetBits(ipb, offset, 5);
	}
	if (vui->nal_hrd_parameters_present > 0 || vui->vcl_hrd_parameters_present > 0) {
		vui->low_delay_hrd = GetBit(ipb, offset);
	}
	vui->pic_struct_present = GetBit(ipb, offset);
	vui->bitstream_restriction = GetBit(ipb, offset);
	if (vui->bitstream_restriction > 0) {
		vui->motion_vectors_over_pic_boundaries = GetBit(ipb, offset);
		vui->max_bytes_per_pic_denom = GetVLC(ipb, offset, false);
		vui->max_bits_per_mb_denom = GetVLC(ipb, offset, false);
		vui->log2_max_mv_length_horizontal = GetVLC(ipb, offset, false);
		vui->log2_max_mv_length_vertical = GetVLC(ipb, offset, false);
		vui->num_reorder_frames = GetVLC(ipb, offset, false);
		vui->max_dec_frame_buffering = GetVLC(ipb, offset, false);
	}
}

int SPSFromNAL_H264(const void *in, int insize, vs_sps *sps)
{
	const unsigned char *pb = static_cast<const unsigned char*>(in);
	int offset = 31;

    if (insize < 1)
        return -1;

	unsigned char typeNAL = (pb[0] & 0x1f);

	if (typeNAL == 0x01) return -1;
	if (typeNAL == 0x05) return -3;
	if (typeNAL != 0x07) return -2;

	sps->type_nal = pb[0];

	pb++;

	// parse SPS
	const unsigned int *ipb = (unsigned int*)pb;
	unsigned int first = *(unsigned int*)ipb;
	// profile
	sps->profile_idc = first & 0x000000ff;
	offset -= 8;
	// constraint_set flags
	sps->constraint_set0 = GetBit(ipb, offset);
	sps->constraint_set1 = GetBit(ipb, offset);
	sps->constraint_set2 = GetBit(ipb, offset);
	sps->constraint_set3 = GetBit(ipb, offset);
	// 4 zero bits
	GetBits(ipb, offset, 4);
	// level
	sps->level_idc = ((first >> 16) & 0x000000ff);
	offset -= 8;
	// id
	sps->seq_parameter_set_id = GetVLC(ipb, offset, false);
	// 7.3.2.1.1 "Sequence parameter set data syntax"
	// chapter of H264 standard for full list of profiles with chrominance
	if (sps->profile_idc == 100 ||
		sps->profile_idc == 110 ||
		sps->profile_idc == 118 ||
		sps->profile_idc == 122 ||
		sps->profile_idc == 128 ||
		sps->profile_idc == 244 ||
		sps->profile_idc == 44  ||
		sps->profile_idc == 83  ||
		sps->profile_idc == 86)
	{
		sps->chroma_format_idc = GetVLC(ipb, offset, false);
		if (sps->chroma_format_idc == 3) {
			sps->residual_colour_transform_flag = GetBit(ipb, offset);
		}
		sps->bit_depth_luma = GetVLC(ipb, offset, false) + 8;
		sps->bit_depth_chroma = GetVLC(ipb, offset, false) + 8;
		if (sps->chroma_format_idc == 0) {

		}
		sps->qpprime_y_zero_transform_bypass_flag = GetBit(ipb, offset);
		sps->seq_scaling_matrix_present_flag = GetBit(ipb, offset);
		if (sps->seq_scaling_matrix_present_flag > 0) {
			int codeNum = 0;
			// 0
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 1
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 2
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 3
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 4
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 5
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 4);
			}
			// 0
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 8);
			}
			// 1
			codeNum = GetBit(ipb, offset);
			if (codeNum) {
				SkipScalingListXxX(ipb, offset, 8);
			}
		} else {

		}
	} else {
		sps->chroma_format_idc = 1;
	}
	// log2 max frame num
	sps->log2_max_frame_num = GetVLC(ipb, offset, false) + 4;
	// pic order cnt type (0..2)
	sps->pic_order_cnt_type = GetVLC(ipb, offset, false);
	if (sps->pic_order_cnt_type == 0) {
		sps->log2_max_pic_order_cnt_lsb = GetVLC(ipb, offset, false) + 4;
	} else if (sps->pic_order_cnt_type == 1){
		sps->delta_pic_order_always_zero_flag = GetBit(ipb, offset);
		sps->offset_for_non_ref_pic = GetVLC(ipb, offset, true);
		sps->offset_for_top_to_bottom_field = GetVLC(ipb, offset, true);
		sps->num_ref_frames_in_pic_order_cnt_cycle = GetVLC(ipb, offset, false);
		for (int i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
			sps->offset_for_ref_frame[i] = GetVLC(ipb, offset, true);
	}
	// num reference frames
	sps->num_ref_frames = GetVLC(ipb, offset, false);
	sps->gaps_in_frame_num_value_allowed_flag = GetBit(ipb, offset);
	// get resolution frame
	sps->frame_width_in_mbs = GetVLC(ipb, offset, false) + 1;
	sps->frame_height_in_mbs = GetVLC(ipb, offset, false) + 1;
	// frame mbs only flag
	sps->frame_mbs_only_flag = GetBit(ipb, offset);
	sps->frame_height_in_mbs = (2 - sps->frame_mbs_only_flag) * sps->frame_height_in_mbs;
	if (sps->frame_mbs_only_flag == 0) {
		sps->mb_adaptive_frame_field_flag = GetBit(ipb, offset);
	}
	// direct 8x8
	sps->direct_8x8_inference_flag = GetBit(ipb, offset);
	// frame cropping flag
	sps->frame_cropping_flag = GetBit(ipb, offset);
	if (sps->frame_cropping_flag > 0) {
		sps->frame_crop_left_offset = GetVLC(ipb, offset, false);
		sps->frame_crop_right_offset = GetVLC(ipb, offset, false);
		sps->frame_crop_top_offset = GetVLC(ipb, offset, false);
		sps->frame_crop_bottom_offset = GetVLC(ipb, offset, false);
	}
	// vui parameters
	sps->vui_parameters_present_flag = GetBit(ipb, offset);
	if (sps->vui_parameters_present_flag > 0) {
		ParseVUI_H264(&(sps->vui), ipb, offset);
	}
	return 0;
}

void WriteVUI_H264(vs_vui *vui, h264Bitstream *bs)
{
	PutBits(bs, 1, vui->aspect_ratio_info_present);
	if (vui->aspect_ratio_info_present > 0) {
		PutBits(bs, 8, vui->aspect_ratio_idc);
		if (vui->aspect_ratio_idc == 255) {
			PutBits(bs, 16, vui->sar_width);
			PutBits(bs, 16, vui->sar_height);
		}
	}
	PutBits(bs, 1, vui->overscan_info_present);
	if (vui->overscan_info_present > 0) {
		PutBits(bs, 1, vui->overscan_info);
	}
	PutBits(bs, 1, vui->signal_type_present);
	if (vui->signal_type_present > 0) {
		PutBits(bs, 3, vui->vidformat);
		PutBits(bs, 1, vui->fullrange);
		PutBits(bs, 1, vui->color_description_present);
		if (vui->color_description_present > 0) {
			PutBits(bs, 8, vui->colorprim);
			PutBits(bs, 8, vui->transfer);
			PutBits(bs, 8, vui->colmatrix);
		}
	}
	PutBits(bs, 1, vui->chroma_loc_info_present);
	if (vui->chroma_loc_info_present > 0) {
		PutVLC_ue(bs, vui->chroma_loc_top);
		PutVLC_ue(bs, vui->chroma_loc_bottom);
	}
	PutBits(bs, 1, vui->timing_info_present);
	if (vui->timing_info_present > 0) {
		PutBits(bs, 32, vui->num_units_in_tick);
		PutBits(bs, 32, vui->time_scale);
		PutBits(bs, 1, vui->fixed_frame_rate);
	}
	PutBits(bs, 1, vui->nal_hrd_parameters_present);
	if (vui->nal_hrd_parameters_present > 0) {
		PutVLC_ue(bs, vui->hrd.cpb_cnt_minus1);
		PutBits(bs, 4, vui->hrd.bit_rate_scale);
		PutBits(bs, 4, vui->hrd.cpb_size_scale);
		for (int i = 0; i <= vui->hrd.cpb_cnt_minus1; i++) {
			PutVLC_ue(bs, vui->hrd.bit_rate_value_minus1[i]);
			PutVLC_ue(bs, vui->hrd.cpb_size_value_minus1[i]);
			PutBits(bs, 1, vui->hrd.cbr_flag[i]);
		}
		PutBits(bs, 5, vui->hrd.initial_cpb_removal_delay_length_minus1);
		PutBits(bs, 5, vui->hrd.cpb_removal_delay_length_minus1);
		PutBits(bs, 5, vui->hrd.dpb_output_delay_length_minus1);
		PutBits(bs, 5, vui->hrd.time_offset_length);
	}
	PutBits(bs, 1, vui->vcl_hrd_parameters_present);
	if (vui->vcl_hrd_parameters_present > 0) {
		PutVLC_ue(bs, vui->vcl_hrd.cpb_cnt_minus1);
		PutBits(bs, 4, vui->vcl_hrd.bit_rate_scale);
		PutBits(bs, 4, vui->vcl_hrd.cpb_size_scale);
		for (int i = 0; i <= vui->vcl_hrd.cpb_cnt_minus1; i++) {
			PutVLC_ue(bs, vui->vcl_hrd.bit_rate_value_minus1[i]);
			PutVLC_ue(bs, vui->vcl_hrd.cpb_size_value_minus1[i]);
			PutBits(bs, 1, vui->vcl_hrd.cbr_flag[i]);
		}
		PutBits(bs, 5, vui->vcl_hrd.initial_cpb_removal_delay_length_minus1);
		PutBits(bs, 5, vui->vcl_hrd.cpb_removal_delay_length_minus1);
		PutBits(bs, 5, vui->vcl_hrd.dpb_output_delay_length_minus1);
		PutBits(bs, 5, vui->vcl_hrd.time_offset_length);
	}
	if (vui->nal_hrd_parameters_present > 0 || vui->vcl_hrd_parameters_present > 0) {
		PutBits(bs, 1, vui->low_delay_hrd);
	}
	PutBits(bs, 1, vui->pic_struct_present);
	PutBits(bs, 1, vui->bitstream_restriction);
	if (vui->bitstream_restriction > 0) {
		PutBits(bs, 1, vui->motion_vectors_over_pic_boundaries);
		PutVLC_ue(bs, vui->max_bytes_per_pic_denom);
		PutVLC_ue(bs, vui->max_bits_per_mb_denom);
		PutVLC_ue(bs, vui->log2_max_mv_length_horizontal);
		PutVLC_ue(bs, vui->log2_max_mv_length_vertical);
		PutVLC_ue(bs, vui->num_reorder_frames);
		PutVLC_ue(bs, vui->max_dec_frame_buffering);
	}
}

void ExtractRTPData(unsigned char *src, int insize, unsigned char *dst, int &outsize)
{
	unsigned char *in = src;
	unsigned char* const out = dst;

	unsigned int size, ExtraBytes;
	unsigned char *curPtr, *endPtr, *outPtr;

	// get current RBSP compressed size
	size = insize;
	ExtraBytes = 0;

	// Set Pointers
	endPtr = in + size - 1;	// Point at Last byte with data in it.
	curPtr = in;
	outPtr = out;

	*outPtr++ = 0;
	*outPtr++ = 0;
	*outPtr++ = 0;
	*outPtr++ = 1;
	*outPtr++ = *curPtr++;
	ExtraBytes += 5;

	while (curPtr < endPtr - 1) {	// Copy all but the last 2 bytes
		*outPtr++ = *curPtr;
		// Check for start code emulation
		if ((*curPtr++ == 0) && (*curPtr == 0) && (!(*(curPtr+1) & 0xfc))) {
			*outPtr++ = *curPtr++;
			*outPtr++ = 0x03;	// Emulation Prevention Byte
			ExtraBytes++;
		}
	}

	if (curPtr < endPtr) {
		*outPtr++ = *curPtr++;
	}
	// copy the last byte
	*outPtr = *curPtr;

	outsize = size + ExtraBytes;
}

int SPSToNAL_H264(void *in, int insize, vs_sps *sps)
{
	h264Bitstream bs(static_cast<unsigned char*>(in), insize);

	PutBits(&bs, 8, sps->type_nal);

	PutBits(&bs, 8, sps->profile_idc);

    PutBits(&bs, 1, sps->constraint_set0);
    PutBits(&bs, 1, sps->constraint_set1);
    PutBits(&bs, 1, sps->constraint_set2);
	PutBits(&bs, 1, sps->constraint_set3);
    PutBits(&bs, 4, 0);

	PutBits(&bs, 8, sps->level_idc);

	PutVLC_ue(&bs, sps->seq_parameter_set_id);

	if (sps->profile_idc == 100 ||
		sps->profile_idc == 110 ||
		sps->profile_idc == 118 ||
		sps->profile_idc == 122 ||
		sps->profile_idc == 128 ||
		sps->profile_idc == 244 ||
		sps->profile_idc == 44  ||
		sps->profile_idc == 83  ||
		sps->profile_idc == 86)
	{
		PutVLC_ue(&bs, sps->chroma_format_idc);
		if (sps->chroma_format_idc == 3) {
			PutBits(&bs, 1, sps->residual_colour_transform_flag);
		}
		PutVLC_ue(&bs, sps->bit_depth_luma - 8);
		PutVLC_ue(&bs, sps->bit_depth_chroma - 8);
		PutBits(&bs, 1, sps->qpprime_y_zero_transform_bypass_flag);
		PutBits(&bs, 1, sps->seq_scaling_matrix_present_flag);
		if (sps->seq_scaling_matrix_present_flag > 0) {
			/// TO DO : Put scaling list
		}
	}

	PutVLC_ue(&bs, sps->log2_max_frame_num - 4);
	PutVLC_ue(&bs, sps->pic_order_cnt_type);
	if (sps->pic_order_cnt_type == 0) {
		PutVLC_ue(&bs, sps->log2_max_pic_order_cnt_lsb - 4);
	} else if (sps->pic_order_cnt_type == 1){
		PutBits(&bs, 1, sps->delta_pic_order_always_zero_flag);
		PutVLC_se(&bs, sps->offset_for_non_ref_pic);
		PutVLC_se(&bs, sps->offset_for_top_to_bottom_field);
		PutVLC_ue(&bs, sps->num_ref_frames_in_pic_order_cnt_cycle);
		for (int i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
			PutVLC_se(&bs, sps->offset_for_ref_frame[i]);
	}

	PutVLC_ue(&bs, sps->num_ref_frames);
	PutBits(&bs, 1, sps->gaps_in_frame_num_value_allowed_flag);

	PutVLC_ue(&bs, sps->frame_width_in_mbs - 1);
	PutVLC_ue(&bs, sps->frame_height_in_mbs - 1);

	PutBits(&bs, 1, sps->frame_mbs_only_flag);
	if (sps->frame_mbs_only_flag == 0) {
		PutBits(&bs, 1, sps->mb_adaptive_frame_field_flag);
	}

	PutBits(&bs, 1, sps->direct_8x8_inference_flag);

	PutBits(&bs, 1, sps->frame_cropping_flag);
	if (sps->frame_cropping_flag > 0) {
		PutVLC_ue(&bs, sps->frame_crop_left_offset);
		PutVLC_ue(&bs, sps->frame_crop_right_offset);
		PutVLC_ue(&bs, sps->frame_crop_top_offset);
		PutVLC_ue(&bs, sps->frame_crop_bottom_offset);
	}

	PutBits(&bs, 1, sps->vui_parameters_present_flag);
	if (sps->vui_parameters_present_flag > 0) {
		WriteVUI_H264(&(sps->vui), &bs);
	}

    PutBits(&bs, 1, 1);
	if (bs.bit_left != 8) {
		PutBits(&bs, bs.bit_left, 0);
    }

	int nalSize = bs.bs - bs.base;
	int outSize = 0;
	unsigned char *out = new unsigned char [insize];

	ExtractRTPData(static_cast<unsigned char*>(in), nalSize, out, outSize);
	memcpy(in, out, outSize);

	delete [] out;

	return outSize;
}

const unsigned char* FindStartCode(const unsigned char *in, int insize, unsigned int& start_code_size)
{
	const unsigned char* const in_end = in + insize;
	for (const unsigned char* p = in; p < in_end; ++p)
		if      (p+3 < in_end && p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 1)
		{
			start_code_size = 4;
			return p;
		}
		else if (p+2 < in_end && p[0] == 0 && p[1] == 0 && p[2] == 1)
		{
			start_code_size = 3;
			return p;
		}
	return nullptr;
}

int NALFromBitstream_H264(const unsigned char *in, int insize, const unsigned char*& nal, const unsigned char*& nal_end, unsigned int& start_code_size)
{
	const unsigned char* const in_end = in + insize;
	nal = FindStartCode(in, in_end-in, start_code_size);
	if (!nal)
		return -1;
	unsigned int unused;
	nal_end = FindStartCode(nal+start_code_size, in_end-(nal+start_code_size), unused);
	if (!nal_end)
		nal_end = in_end;
	return 0;
}

int GetNALSize(const unsigned char * in, int insize)
{
	const unsigned char *pb = in;
	while (4 <= insize) {
		if ((0 == pb[0]) && (0 == pb[1]) && (1 == pb[2])) {
			return pb - in;
		}
		pb++;
		insize--;
	}
	return pb - in;
}

