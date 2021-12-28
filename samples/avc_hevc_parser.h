#ifndef __AVC_HEVC_PARSER_H__
#define __AVC_HEVC_PARSER_H__

#include "bit_reader.h"


#define LC_MP4_ADTS_HEADER_SIZE 7
#define LC_MP4_ADTS_SYNC_MASK     0xFFF6 /* 12 sync bits plus 2 layer bits */
#define LC_MP4_ADTS_SYNC_PATTERN  0xFFF0 /* 12 sync bits=1 layer=0         */

typedef enum {
    LC_MP4_AAC_MAIN = 1,
    LC_MP4_AAC_LC = 2,
    LC_MP4_AAC_SSR = 3,
    LC_MP4_AAC_LTP = 4,
    LC_MP4_SBR = 5,
    LC_MP4_AAC_SCALABLE = 6,
} LC_MP4_MPEG4_AUDIO_OBJECT_TYPES;

uint32_t get_sampling_frequency_index(uint32_t sampling_frequency);

bool find_adts_head(uint8_t *data, uint32_t size, uint8_t *header);

void make_dsi(uint32_t sampling_frequency_index,uint32_t channel_configuration, uint8_t* dsi);

typedef struct
{
    uint32_t nal_unit_size;
    uint8_t *nal_unit;
}LC_MP4_MUXER_Nalu_t;

int get_one_nalu_from_buf(const uint8_t *buffer, uint32_t size, uint32_t offset, LC_MP4_MUXER_Nalu_t* nalu);

// video

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_UNSPECIFIED                       = 0;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_NON_IDR_PICTURE    = 1;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A      = 2;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B      = 3;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C      = 4;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE        = 5;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_SEI                               = 6;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_SPS                               = 7;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_PPS                               = 8;
const uint32_t LC_MP4_AVC_NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER             = 9;

const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_TRAIL_N        = 0;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_TRAIL_R        = 1;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_TSA_N          = 2;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_TSA_R          = 3;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_STSA_N         = 4;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_STSA_R         = 5;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RADL_N         = 6;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RADL_R         = 7;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RASL_N         = 8;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RASL_R         = 9;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL_N10    = 10;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL_R11    = 11;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL_N12    = 12;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL_R13    = 13;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL_N14    = 14;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL_R15    = 15;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_BLA_W_LP       = 16;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_BLA_W_RADL     = 17;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_BLA_N_LP       = 18;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_IDR_W_RADL     = 19;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_IDR_N_LP       = 20;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_CRA_NUT        = 21;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_IRAP_VCL22 = 22;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_IRAP_VCL23 = 23;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL24      = 24;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL25      = 25;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL26      = 26;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL27      = 27;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL28      = 28;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL29      = 29;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL30      = 30;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_RSV_VCL31      = 31;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_VPS_NUT        = 32;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_SPS_NUT        = 33;
const uint32_t LC_MP4_HEVC_NAL_UNIT_TYPE_PPS_NUT        = 34;


// avc
struct LC_MP4_AvcSequenceParameterSet {
    LC_MP4_AvcSequenceParameterSet();
    // methods
    int Parse(const uint8_t* data, uint32_t data_size);
    uint32_t profile_idc;
    uint32_t constraint_set0_flag;
    uint32_t constraint_set1_flag;
    uint32_t constraint_set2_flag;
    uint32_t constraint_set3_flag;
    uint32_t level_idc;
    uint32_t seq_parameter_set_id;
};

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

