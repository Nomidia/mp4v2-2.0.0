#ifndef __AVC_HEVC_PARSER_H__
#define __AVC_HEVC_PARSER_H__

#include "bit_reader.h"

typedef struct
{
    uint32_t nal_unit_size;
    uint8_t *nal_unit;
}LC_MP4_MUXER_Nalu_t;

int get_one_nalu_from_buf(const uint8_t *buffer, uint32_t size, uint32_t offset, LC_MP4_MUXER_Nalu_t* nalu);

// avc
typedef struct{
    uint8_t profile_idc;
    uint8_t profile_compat;
    uint8_t level_idc;
    uint8_t sample_len_field_size_minus_one;
}LC_MP4_AVC_SPS_INFO_t;

int parse_h264_sps(uint8_t *sps, uint32_t sps_size, LC_MP4_AVC_SPS_INFO_t *sps_info);

// hevc
const uint32_t LC_MP4_HEVC_PPS_MAX_ID               = 63;
const uint32_t LC_MP4_HEVC_SPS_MAX_ID               = 15;
const uint32_t LC_MP4_HEVC_VPS_MAX_ID               = 15;
const uint32_t LC_MP4_HEVC_SPS_MAX_RPS              = 64;

typedef struct {
    uint32_t delta_poc_s0_minus1[16];
    uint32_t delta_poc_s1_minus1[16];
    uint32_t used_by_curr_pic_s0_flag[16];
    uint32_t used_by_curr_pic_s1_flag[16];
    uint32_t num_negative_pics;
    uint32_t num_positive_pics;
    uint32_t num_delta_pocs;
} LC_MP4_HevcShortTermRefPicSet;

struct LC_MP4_HevcProfileTierLevel {
    LC_MP4_HevcProfileTierLevel();

    // methods
    int Parse(LC_MP4_BitReader& bits, uint32_t max_num_sub_layers_minus_1);

    uint32_t general_profile_space;
    uint32_t general_tier_flag;
    uint32_t general_profile_idc;
    uint32_t     general_profile_compatibility_flags;

    // this is a synthetic field that includes 48 bits starting with:
    // general_progressive_source_flag    (1 bit)
    // general_interlaced_source_flag     (1 bit)
    // general_non_packed_constraint_flag (1 bit)
    // general_frame_only_constraint_flag (1 bit)
    // general_reserved_zero_44bits       (44 bits)
    uint64_t     general_constraint_indicator_flags;

    uint32_t general_level_idc;
    struct {
        uint8_t sub_layer_profile_present_flag;
        uint8_t sub_layer_level_present_flag;
        uint8_t sub_layer_profile_space;
        uint8_t sub_layer_tier_flag;
        uint8_t sub_layer_profile_idc;
        uint32_t sub_layer_profile_compatibility_flags;
        uint8_t sub_layer_progressive_source_flag;
        uint8_t sub_layer_interlaced_source_flag;
        uint8_t sub_layer_non_packed_constraint_flag;
        uint8_t sub_layer_frame_only_constraint_flag;
        uint8_t sub_layer_level_idc;
    } sub_layer_info[8];
};

struct LC_MP4_HevcSequenceParameterSet {
    LC_MP4_HevcSequenceParameterSet();
    
    // methods
    int Parse(const uint8_t* data, uint32_t data_size);
    void GetInfo(uint32_t& width, uint32_t& height);

    uint32_t             sps_video_parameter_set_id;
    uint32_t             sps_max_sub_layers_minus1;
    uint32_t             sps_temporal_id_nesting_flag;
    LC_MP4_HevcProfileTierLevel profile_tier_level;
    uint32_t             sps_seq_parameter_set_id;
    uint32_t             chroma_format_idc;
    uint32_t             separate_colour_plane_flag;
    uint32_t             pic_width_in_luma_samples;
    uint32_t             pic_height_in_luma_samples;
    uint32_t             conformance_window_flag;
    uint32_t             conf_win_left_offset;
    uint32_t             conf_win_right_offset;
    uint32_t             conf_win_top_offset;
    uint32_t             conf_win_bottom_offset;
    uint32_t             bit_depth_luma_minus8;
    uint32_t             bit_depth_chroma_minus8;
    uint32_t             sps_max_dec_pic_buffering_minus1[8];
    uint32_t             sps_max_num_reorder_pics[8];
    uint32_t             sps_max_latency_increase_plus1[8];
    uint32_t             log2_max_pic_order_cnt_lsb_minus4;
    uint32_t             sps_sub_layer_ordering_info_present_flag;
    uint32_t             log2_min_luma_coding_block_size_minus3;
    uint32_t             log2_diff_max_min_luma_coding_block_size;
    uint32_t             log2_min_transform_block_size_minus2;
    uint32_t             log2_diff_max_min_transform_block_size;
    uint32_t             max_transform_hierarchy_depth_inter;
    uint32_t             max_transform_hierarchy_depth_intra;
    uint32_t             scaling_list_enabled_flag;
    uint32_t             sps_scaling_list_data_present_flag;
                         // skipped scaling list data
    uint32_t             amp_enabled_flag;
    uint32_t             sample_adaptive_offset_enabled_flag;
    uint32_t             pcm_enabled_flag;
    uint32_t             pcm_sample_bit_depth_luma_minus1;
    uint32_t             pcm_sample_bit_depth_chroma_minus1;
    uint32_t             log2_min_pcm_luma_coding_block_size_minus3;
    uint32_t             log2_diff_max_min_pcm_luma_coding_block_size;
    uint32_t             pcm_loop_filter_disabled_flag;
    uint32_t             num_short_term_ref_pic_sets;
    uint32_t             long_term_ref_pics_present_flag;
    uint32_t             num_long_term_ref_pics_sps;
    uint32_t             sps_temporal_mvp_enabled_flag;
    uint32_t             strong_intra_smoothing_enabled_flag;

    LC_MP4_HevcShortTermRefPicSet short_term_ref_pic_sets[64];
};


#endif

