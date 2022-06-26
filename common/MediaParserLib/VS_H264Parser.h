
#ifndef VS_H264PARSER_H
#define VS_H264PARSER_H

struct vs_hrd
{
	int cpb_cnt_minus1;
	int bit_rate_scale;
	int cpb_size_scale;
    unsigned int bit_rate_value_minus1[32];
    unsigned int cpb_size_value_minus1[32];
    int cbr_flag[32];
    int initial_cpb_removal_delay_length_minus1;
    int cpb_removal_delay_length_minus1;
    int dpb_output_delay_length_minus1;
    int time_offset_length;
};

struct vs_vui
{
    int aspect_ratio_info_present;
	int aspect_ratio_idc;
    int sar_width;
    int sar_height;

    int overscan_info_present;
    int overscan_info;

    int signal_type_present;
    int vidformat;
    int fullrange;
    int color_description_present;
    int colorprim;
    int transfer;
    int colmatrix;

    int chroma_loc_info_present;
    int chroma_loc_top;
    int chroma_loc_bottom;

    int timing_info_present;
    int num_units_in_tick;
    int time_scale;
    int fixed_frame_rate;

	int nal_hrd_parameters_present;
	vs_hrd hrd;

	int vcl_hrd_parameters_present;
	vs_hrd vcl_hrd;

	int low_delay_hrd;
	int pic_struct_present;
    int bitstream_restriction;
    int motion_vectors_over_pic_boundaries;
    int max_bytes_per_pic_denom;
    int max_bits_per_mb_denom;
    int log2_max_mv_length_horizontal;
    int log2_max_mv_length_vertical;
    int num_reorder_frames;
    int max_dec_frame_buffering;
};

struct vs_sps
{
	unsigned char type_nal;

    int profile_idc;

	int level_idc;
    int constraint_set0;
    int constraint_set1;
    int constraint_set2;
	int constraint_set3;
	int chroma_format_idc;

	int seq_parameter_set_id;
    int log2_max_frame_num;
	int pic_order_cnt_type;

    int bit_depth_luma;
    int bit_depth_chroma;

	int delta_pic_order_always_zero_flag;
	int frame_mbs_only_flag;
	int gaps_in_frame_num_value_allowed_flag;
	int mb_adaptive_frame_field_flag;
	int direct_8x8_inference_flag;

	int vui_parameters_present_flag;
	vs_vui vui;

    int frame_cropping_flag;
    int frame_crop_left_offset;
    int frame_crop_right_offset;
    int frame_crop_top_offset;
    int frame_crop_bottom_offset;

    int log2_max_pic_order_cnt_lsb;
    int offset_for_non_ref_pic;

    int offset_for_top_to_bottom_field;

    int num_ref_frames_in_pic_order_cnt_cycle;
    int offset_for_ref_frame[256];

    int num_ref_frames;
    int frame_width_in_mbs;
    int frame_height_in_mbs;

    int qpprime_y_zero_transform_bypass_flag;
	int seq_scaling_matrix_present_flag;
	int residual_colour_transform_flag;
};

int ResolutionFromNAL_H264(const void *in, int insize, int& width, int& height);
int ResolutionFromBitstream_H264(const void *in, int insize, int& width, int& height);
int TypeSliceFromNAL_H264(const void *in, int insize);
int TypeSliceFromBitstream_H264(const void *in, int insize, int& bIdrSlice);
int SPSFromNAL_H264(const void *in, int insize, vs_sps *sps);
int SPSFromBitstream_H264(const void *in, int insize, vs_sps *sps);
int SPSToNAL_H264(void *in, int insize, vs_sps *sps);
int NALFromBitstream_H264(const unsigned char *in, int insize, const unsigned char*& nal, const unsigned char*& nal_end, unsigned int& start_code_size);
int GetNALSize(const unsigned char *in, int insize);

#endif /* VS_H264PARSER_H */