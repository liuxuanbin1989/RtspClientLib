/********************************************************************************
   * COPYRIGHT   : Copyright(C) 2014-2016  T&W  Co., Ltd. All Rights Reserved.
   * FILE NAME   : geth264videosize.h
   * DESCRIPTION : ����H264��ȡ��Ƶ���
   * AUTHOR      : liuxuanbin
   * VERSION     : 1.0
   * DATE        : 2015/03/05 
   * FIXED RECORD : 
   * DATE   VERSION     REVISIOR          ABSTRACT

   * NOTICE :
********************************************************************************/
#ifndef _get_h264_video_size_H_
#define _get_h264_video_size_H_

bool h264_decode_seq_parameter_set(unsigned char * buf,unsigned int nLen,unsigned int  &Width,unsigned int  &Height);

/*
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef int _int16_t;
typedef unsigned int uint16_t;

typedef long _int32_t;
typedef unsigned long uint32_t;

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

typedef _int16_t intptr_t;
typedef uint16_t uintptr_t;


#define Extended_SAR 255
typedef struct vui_parameters{
  int aspect_ratio_info_present_flag; //0 u(1) 
  int aspect_ratio_idc;               //0 u(8) 
  int sar_width;                      //0 u(16) 
  int sar_height;                     //0 u(16) 
  int overscan_info_present_flag;     //0 u(1) 
  int overscan_appropriate_flag;      //0 u(1) 
  int video_signal_type_present_flag; //0 u(1) 
  int video_format;                   //0 u(3) 
  int video_full_range_flag;          //0 u(1) 
  int colour_description_present_flag; //0 u(1) 
  int colour_primaries;                //0 u(8) 
  int transfer_characteristics;        //0 u(8) 
  int matrix_coefficients;             //0 u(8) 
  int chroma_loc_info_present_flag;     //0 u(1) 
  int chroma_sample_loc_type_top_field;  //0 ue(v) 
  int chroma_sample_loc_type_bottom_field; //0 ue(v) 
  int timing_info_present_flag;          //0 u(1) 
  uint32_t num_units_in_tick;           //0 u(32) 
  uint32_t time_scale;                 //0 u(32) 
  int fixed_frame_rate_flag;           //0 u(1) 
  int nal_hrd_parameters_present_flag; //0 u(1)
  int cpb_cnt_minus1;                 //0 ue(v)
  int bit_rate_scale;                 //0 u(4)
  int cpb_size_scale;                 //0 u(4)
  int bit_rate_value_minus1[16];      //0 ue(v)
  int cpb_size_value_minus1[16];      //0 ue(v)
  int cbr_flag[16];                   //0 u(1)
  int initial_cpb_removal_delay_length_minus1; //0 u(5)
  int cpb_removal_delay_length_minus1;         //0 u(5)
  int dpb_output_delay_length_minus1;         //0 u(5)
  int time_offset_length;                      //0 u(5)
  int vcl_hrd_parameters_present_flag;         //0 u(1)
  int low_delay_hrd_flag;                      //0 u(1)
  int pic_struct_present_flag;                 //0 u(1)
  int bitstream_restriction_flag;              //0 u(1)
  int motion_vectors_over_pic_boundaries_flag;  //0 ue(v)
  int max_bytes_per_pic_denom;                  //0 ue(v)
  int max_bits_per_mb_denom;                    //0 ue(v)
  int log2_max_mv_length_horizontal;            //0 ue(v)
  int log2_max_mv_length_vertical;              //0 ue(v)
  int num_reorder_frames;                       //0 ue(v)
  int max_dec_frame_buffering;                  //0 ue(v)
}vui_parameters_t;

typedef struct SPS{
	int profile_idc;
	int constraint_set0_flag;
	int constraint_set1_flag;
	int constraint_set2_flag;
	int constraint_set3_flag;
	int reserved_zero_4bits;
	int level_idc;
	int seq_parameter_set_id; //ue(v)
	int chroma_format_idc; //ue(v)
	int separate_colour_plane_flag; //u(1)
	int bit_depth_luma_minus8; //0 ue(v) 
	int bit_depth_chroma_minus8; //0 ue(v) 
	int qpprime_y_zero_transform_bypass_flag; //0 u(1) 
	int seq_scaling_matrix_present_flag; //0 u(1)
	int seq_scaling_list_present_flag[12];
	int UseDefaultScalingMatrix4x4Flag[6];
	int UseDefaultScalingMatrix8x8Flag[6];
	int ScalingList4x4[6][16];
	int ScalingList8x8[6][64];
	int log2_max_frame_num_minus4; //0 ue(v)
	int pic_order_cnt_type; //0 ue(v)
	int log2_max_pic_order_cnt_lsb_minus4; //
	int delta_pic_order_always_zero_flag;           //u(1)
	int offset_for_non_ref_pic;                     //se(v)
	int offset_for_top_to_bottom_field;            //se(v)
	int num_ref_frames_in_pic_order_cnt_cycle;    //ue(v) 
	int offset_for_ref_frame_array[16];           //se(v)
	int num_ref_frames;                           //ue(v)
	int gaps_in_frame_num_value_allowed_flag;    //u(1)
	int pic_width_in_mbs_minus1;                //ue(v)
	int pic_height_in_map_units_minus1;         //u(1)
	int frame_mbs_only_flag;                  //0 u(1) 
	int mb_adaptive_frame_field_flag;           //0 u(1) 
	int direct_8x8_inference_flag;              //0 u(1) 
	int frame_cropping_flag;                    //u(1)
	int frame_crop_left_offset;                //ue(v)
	int frame_crop_right_offset;                //ue(v)
	int frame_crop_top_offset;                  //ue(v)
	int frame_crop_bottom_offset;            //ue(v)
	int vui_parameters_present_flag;            //u(1)
	vui_parameters_t vui_parameters;
}SPS;

typedef struct PPS{
	int pic_parameter_set_id;
	int seq_parameter_set_id;
	int entropy_coding_mode_flag;
	int pic_order_present_flag;
	int num_slice_groups_minus1;
	int slice_group_map_type;
	int run_length_minus1[32];
	int top_left[32];
	int bottom_right[32];
	int slice_group_change_direction_flag;
	int slice_group_change_rate_minus1;
	int pic_size_in_map_units_minus1;
	int slice_group_id[32];
	int num_ref_idx_10_active_minus1;
	int num_ref_idx_11_active_minus1;
	int weighted_pred_flag;
	int weighted_bipred_idc;
	int pic_init_qp_minus26;
	int pic_init_qs_minus26;
	int chroma_qp_index_offset;
	int deblocking_filter_control_present_flag;
	int constrained_intra_pred_flag;
	int redundant_pic_cnt_present_flag;
	int transform_8x8_mode_flag;
	int pic_scaling_matrix_present_flag;
	int pic_scaling_list_present_flag[32];
	int second_chroma_qp_index_offset;
	int scaling_list_4x4[6][16];
	int scaling_list_8x8[2][64];
}PPS;
*/


#endif