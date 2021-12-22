
#include "avc_hevc_parser.h"
#include <string.h>

int get_one_nalu_from_buf(const uint8_t *buffer, uint32_t size, uint32_t offset, LC_MP4_MUXER_Nalu_t* nalu)
{
    int i = offset;
    while (i + 4 < size) {
        if (buffer[i++] == 0x00 && buffer[i++] == 0x00 && buffer[i++] == 0x00 && buffer[i++] == 0x01) {
            int pos = i;
            while (pos < size) {
                if (buffer[pos++] == 0x00 && buffer[pos++] == 0x00 && buffer[pos++] == 0x00 &&buffer[pos++] == 0x01) {
                    break;
                }
            }
            if(pos == size) {
                nalu->nal_unit_size = pos - i;
            } else {
                nalu->nal_unit_size = (pos - 4) - i;
            }

            nalu->nal_unit =(uint8_t*) & buffer[i];
            return (nalu->nal_unit_size + i - offset);
        }
    }
    return 0;
}

int parse_h264_sps(uint8_t *sps, uint32_t sps_size, LC_MP4_AVC_SPS_INFO_t *sps_info)
{
    if (!sps_info)
        return -1;
    sps_info->profile_idc = sps[1];
    sps_info->profile_compat = sps[2];
    sps_info->level_idc = sps[3];
    sps_info->sample_len_field_size_minus_one = 3;
    return 0;
}

/*----------------------------------------------------------------------
|   ReadGolomb
+---------------------------------------------------------------------*/
static unsigned int
ReadGolomb(LC_MP4_BitReader& bits)
{
    unsigned int leading_zeros = 0;
    while (bits.ReadBit() == 0) {
        leading_zeros++;
        if (leading_zeros > 32) return 0; // safeguard
    }
    if (leading_zeros) {
        return (1<<leading_zeros)-1+bits.ReadBits(leading_zeros);
    } else {
        return 0;
    }
}

/*----------------------------------------------------------------------
|   scaling_list_data
+---------------------------------------------------------------------*/
static void scaling_list_data(LC_MP4_BitReader& bits)
{
    for (unsigned int sizeId = 0; sizeId < 4; sizeId++) {
        for (unsigned int matrixId = 0; matrixId < (unsigned int)((sizeId == 3)?2:6); matrixId++) {
            unsigned int flag = bits.ReadBit(); // scaling_list_pred_mode_flag[ sizeId ][ matrixId ]
            if (!flag) {
                ReadGolomb(bits); // scaling_list_pred_matrix_id_delta[ sizeId ][ matrixId ]
            } else {
                // nextCoef = 8;
                unsigned int coefNum = (1 << (4+(sizeId << 1)));
                if (coefNum > 64) coefNum = 64;
                if (sizeId > 1) {
                    ReadGolomb(bits); // scaling_list_dc_coef_minus8[ sizeId − 2 ][ matrixId ]
                    // nextCoef = scaling_list_dc_coef_minus8[ sizeId − 2 ][ matrixId ] + 8
                }
                for (unsigned i = 0; i < coefNum; i++) {
                    ReadGolomb(bits); // scaling_list_delta_coef
                    // nextCoef = ( nextCoef + scaling_list_delta_coef + 256 ) % 256
                    // ScalingList[ sizeId ][ matrixId ][ i ] = nextCoef
                }
            }
        }
    }
}

/*----------------------------------------------------------------------
|   st_ref_pic_set
+---------------------------------------------------------------------*/
static int parse_st_ref_pic_set(LC_MP4_HevcShortTermRefPicSet*         rps,
                     const LC_MP4_HevcSequenceParameterSet* sps,
                     unsigned int                           stRpsIdx,
                     unsigned int                           num_short_term_ref_pic_sets,
                     LC_MP4_BitReader&                      bits) {
    memset(rps, 0, sizeof(*rps));

    unsigned int inter_ref_pic_set_prediction_flag = 0;
    if (stRpsIdx != 0) {
        inter_ref_pic_set_prediction_flag = bits.ReadBit();
    }
    if (inter_ref_pic_set_prediction_flag) {
        unsigned int delta_idx_minus1 = 0;
        if (stRpsIdx == num_short_term_ref_pic_sets) {
            delta_idx_minus1 = ReadGolomb(bits);
        }
        /* delta_rps_sign = */ bits.ReadBit();
        /* abs_delta_rps_minus1 = */ ReadGolomb(bits);
        if (delta_idx_minus1+1 > stRpsIdx) return -10; // should not happen
        unsigned int RefRpsIdx = stRpsIdx - (delta_idx_minus1 + 1);
        unsigned int NumDeltaPocs = sps->short_term_ref_pic_sets[RefRpsIdx].num_delta_pocs;
        for (unsigned j=0; j<=NumDeltaPocs; j++) {
            unsigned int used_by_curr_pic_flag /*[j]*/ = bits.ReadBit();
            unsigned int use_delta_flag /*[j]*/ = 1;
            if (!used_by_curr_pic_flag /*[j]*/) {
                use_delta_flag /*[j]*/ = bits.ReadBit();
            }
            if (used_by_curr_pic_flag /*[j]*/ || use_delta_flag /*[j]*/) {
                rps->num_delta_pocs++;
            }
        }
    } else {
        rps->num_negative_pics = ReadGolomb(bits);
        rps->num_positive_pics = ReadGolomb(bits);
        if (rps->num_negative_pics > 16 || rps->num_positive_pics > 16) {
            return -10;
        }
        rps->num_delta_pocs = rps->num_negative_pics + rps->num_positive_pics;
        for (unsigned int i=0; i<rps->num_negative_pics; i++) {
            rps->delta_poc_s0_minus1[i] = ReadGolomb(bits);
            rps->used_by_curr_pic_s0_flag[i] = bits.ReadBit();
        }
        for (unsigned i=0; i<rps->num_positive_pics; i++) {
            rps->delta_poc_s1_minus1[i] = ReadGolomb(bits);
            rps->used_by_curr_pic_s1_flag[i] = bits.ReadBit();
        }
    }

    return 0;
}

/*----------------------------------------------------------------------
|   LC_MP4_HevcProfileTierLevel::LC_MP4_HevcProfileTierLevel
+---------------------------------------------------------------------*/
LC_MP4_HevcProfileTierLevel::LC_MP4_HevcProfileTierLevel() :
    general_profile_space(0),
    general_tier_flag(0),
    general_profile_idc(0),
    general_profile_compatibility_flags(0),
    general_constraint_indicator_flags(0),
    general_level_idc(0)
{
    memset(&sub_layer_info[0], 0, sizeof(sub_layer_info));
}

/*----------------------------------------------------------------------
|   LC_MP4_HevcProfileTierLevel::Parse
+---------------------------------------------------------------------*/
int LC_MP4_HevcProfileTierLevel::Parse(LC_MP4_BitReader& bits, uint32_t max_num_sub_layers_minus_1)
{
    // profile_tier_level
    general_profile_space               = bits.ReadBits(2);
    general_tier_flag                   = bits.ReadBit();
    general_profile_idc                 = bits.ReadBits(5);
    general_profile_compatibility_flags = bits.ReadBits(32);
    
    general_constraint_indicator_flags  = ((uint64_t)bits.ReadBits(16)) << 32;
    general_constraint_indicator_flags |= bits.ReadBits(32);
    
    general_level_idc                   = bits.ReadBits(8);
    for (unsigned int i = 0; i < max_num_sub_layers_minus_1; i++) {
        sub_layer_info[i].sub_layer_profile_present_flag = bits.ReadBit();
        sub_layer_info[i].sub_layer_level_present_flag   = bits.ReadBit();
    }
    if (max_num_sub_layers_minus_1) {
        for (unsigned int i = max_num_sub_layers_minus_1; i < 8; i++) {
            bits.ReadBits(2); // reserved_zero_2bits[i]
        }
    }
    for (unsigned int i = 0; i < max_num_sub_layers_minus_1; i++) {
        if (sub_layer_info[i].sub_layer_profile_present_flag) {
            sub_layer_info[i].sub_layer_profile_space               = bits.ReadBits(2);
            sub_layer_info[i].sub_layer_tier_flag                   = bits.ReadBit();
            sub_layer_info[i].sub_layer_profile_idc                 = bits.ReadBits(5);
            sub_layer_info[i].sub_layer_profile_compatibility_flags = bits.ReadBits(32);
            sub_layer_info[i].sub_layer_progressive_source_flag     = bits.ReadBit();
            sub_layer_info[i].sub_layer_interlaced_source_flag      = bits.ReadBit();
            sub_layer_info[i].sub_layer_non_packed_constraint_flag  = bits.ReadBit();
            sub_layer_info[i].sub_layer_frame_only_constraint_flag  = bits.ReadBit();
            bits.ReadBits(32); bits.ReadBits(12); // sub_layer_reserved_zero_44bits
        }
        if (sub_layer_info[i].sub_layer_level_present_flag) {
            sub_layer_info[i].sub_layer_level_idc = bits.ReadBits(8);
        }
    }
    
    return 0;
}

/*----------------------------------------------------------------------
|   LC_MP4_HevcSequenceParameterSet::LC_MP4_HevcSequenceParameterSet
+---------------------------------------------------------------------*/
LC_MP4_HevcSequenceParameterSet::LC_MP4_HevcSequenceParameterSet() :
    sps_video_parameter_set_id(0),
    sps_max_sub_layers_minus1(0),
    sps_temporal_id_nesting_flag(0),
    sps_seq_parameter_set_id(0),
    chroma_format_idc(0),
    separate_colour_plane_flag(0),
    pic_width_in_luma_samples(0),
    pic_height_in_luma_samples(0),
    conformance_window_flag(0),
    conf_win_left_offset(0),
    conf_win_right_offset(0),
    conf_win_top_offset(0),
    conf_win_bottom_offset(0),
    bit_depth_luma_minus8(0),
    bit_depth_chroma_minus8(0),
    log2_max_pic_order_cnt_lsb_minus4(0),
    sps_sub_layer_ordering_info_present_flag(0),
    log2_min_luma_coding_block_size_minus3(0),
    log2_diff_max_min_luma_coding_block_size(0),
    log2_min_transform_block_size_minus2(0),
    log2_diff_max_min_transform_block_size(0),
    max_transform_hierarchy_depth_inter(0),
    max_transform_hierarchy_depth_intra(0),
    scaling_list_enabled_flag(0),
    sps_scaling_list_data_present_flag(0),
    amp_enabled_flag(0),
    sample_adaptive_offset_enabled_flag(0),
    pcm_enabled_flag(0),
    pcm_sample_bit_depth_luma_minus1(0),
    pcm_sample_bit_depth_chroma_minus1(0),
    log2_min_pcm_luma_coding_block_size_minus3(0),
    log2_diff_max_min_pcm_luma_coding_block_size(0),
    pcm_loop_filter_disabled_flag(0),
    num_short_term_ref_pic_sets(0),
    long_term_ref_pics_present_flag(0),
    num_long_term_ref_pics_sps(0),
    sps_temporal_mvp_enabled_flag(0),
    strong_intra_smoothing_enabled_flag(0)
{
    memset(&profile_tier_level, 0, sizeof(profile_tier_level));
    for (unsigned int i=0; i<8; i++) {
        sps_max_dec_pic_buffering_minus1[i] = 0;
        sps_max_num_reorder_pics[i]         = 0;
        sps_max_latency_increase_plus1[i]   = 0;
    }
    memset(short_term_ref_pic_sets, 0, sizeof(short_term_ref_pic_sets));
}

/*----------------------------------------------------------------------
|   LC_MP4_HevcSequenceParameterSet::Parse
+---------------------------------------------------------------------*/
int LC_MP4_HevcSequenceParameterSet::Parse(const uint8_t* data, unsigned int data_size)
{
    LC_MP4_BitReader bits(data, data_size);

    bits.SkipBits(16); // NAL Unit Header

    sps_video_parameter_set_id   = bits.ReadBits(4);
    sps_max_sub_layers_minus1    = bits.ReadBits(3);
    sps_temporal_id_nesting_flag = bits.ReadBit();
    
    int result = profile_tier_level.Parse(bits, sps_max_sub_layers_minus1);
    if (result != 0) {
        return result;
    }
    
    sps_seq_parameter_set_id = ReadGolomb(bits);
    if (sps_seq_parameter_set_id > LC_MP4_HEVC_SPS_MAX_ID) {
        return -10;
    }

    chroma_format_idc = ReadGolomb(bits);
    if (chroma_format_idc == 3) {
        separate_colour_plane_flag = bits.ReadBit();
    }
    pic_width_in_luma_samples  = ReadGolomb(bits);
    pic_height_in_luma_samples = ReadGolomb(bits);
    conformance_window_flag    = bits.ReadBit();
    
    if (conformance_window_flag) {
        conf_win_left_offset    = ReadGolomb(bits);
        conf_win_right_offset   = ReadGolomb(bits);
        conf_win_top_offset     = ReadGolomb(bits);
        conf_win_bottom_offset  = ReadGolomb(bits);
    }
    bit_depth_luma_minus8                    = ReadGolomb(bits);
    bit_depth_chroma_minus8                  = ReadGolomb(bits);
    log2_max_pic_order_cnt_lsb_minus4        = ReadGolomb(bits);
    if (log2_max_pic_order_cnt_lsb_minus4 > 16) {
        return -10;
    }
    sps_sub_layer_ordering_info_present_flag = bits.ReadBit();
    for (unsigned int i = (sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1);
                      i <= sps_max_sub_layers_minus1;
                      i++) {
        sps_max_dec_pic_buffering_minus1[i] = ReadGolomb(bits);
        sps_max_num_reorder_pics[i]         = ReadGolomb(bits);
        sps_max_latency_increase_plus1[i]   = ReadGolomb(bits);
    }
    log2_min_luma_coding_block_size_minus3   = ReadGolomb(bits);
    log2_diff_max_min_luma_coding_block_size = ReadGolomb(bits);
    log2_min_transform_block_size_minus2     = ReadGolomb(bits);
    log2_diff_max_min_transform_block_size   = ReadGolomb(bits);
    max_transform_hierarchy_depth_inter      = ReadGolomb(bits);
    max_transform_hierarchy_depth_intra      = ReadGolomb(bits);
    scaling_list_enabled_flag                = bits.ReadBit();
    if (scaling_list_enabled_flag) {
        sps_scaling_list_data_present_flag = bits.ReadBit();
        if (sps_scaling_list_data_present_flag) {
            scaling_list_data(bits);
        }
    }
    amp_enabled_flag = bits.ReadBit();
    sample_adaptive_offset_enabled_flag = bits.ReadBit();
    pcm_enabled_flag = bits.ReadBit();
    if (pcm_enabled_flag) {
        pcm_sample_bit_depth_luma_minus1 = bits.ReadBits(4);
        pcm_sample_bit_depth_chroma_minus1 = bits.ReadBits(4);
        log2_min_pcm_luma_coding_block_size_minus3 = ReadGolomb(bits);
        log2_diff_max_min_pcm_luma_coding_block_size = ReadGolomb(bits);
        pcm_loop_filter_disabled_flag = bits.ReadBit();
    }
    num_short_term_ref_pic_sets = ReadGolomb(bits);
    if (num_short_term_ref_pic_sets > LC_MP4_HEVC_SPS_MAX_RPS) {
        return -10;
    }
    for (unsigned int i=0; i<num_short_term_ref_pic_sets; i++) {
        result = parse_st_ref_pic_set(&short_term_ref_pic_sets[i], this, i, num_short_term_ref_pic_sets, bits);
        if (result != 0) return result;
    }
    long_term_ref_pics_present_flag = bits.ReadBit();
    if (long_term_ref_pics_present_flag) {
        num_long_term_ref_pics_sps = ReadGolomb(bits);
        for (unsigned int i=0; i<num_long_term_ref_pics_sps; i++) {
            /* lt_ref_pic_poc_lsb_sps[i] = */ bits.ReadBits(log2_max_pic_order_cnt_lsb_minus4 + 4);
            /* used_by_curr_pic_lt_sps_flag[i] = */ bits.ReadBit();
        }
    }
    sps_temporal_mvp_enabled_flag  = bits.ReadBit();
    strong_intra_smoothing_enabled_flag = bits.ReadBit();

    return 0;
}

