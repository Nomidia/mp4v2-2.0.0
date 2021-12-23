#ifndef __WRAPPER_INTERNAL_H__
#define __WRAPPER_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


#define LC_MP4_TIMESCALE 44100
#define LC_MP4_ADTS_HEADER_SIZE 7
#define LC_MP4_ADTS_SYNC_MASK     0xFFF6 /* 12 sync bits plus 2 layer bits */
#define LC_MP4_ADTS_SYNC_PATTERN  0xFFF0 /* 12 sync bits=1 layer=0         */


#define LC_MP4_FREE(p) do { if (p) {free(p); p = NULL;}} while(0)

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


typedef struct{
    int32_t w;
    int32_t h;

    int32_t fps;

    int8_t video_codec; // -1 : none, 0 :H264, 1: H265
    int8_t audio_codec; // -1 : none, 0: AAC , 1: pcma, 2: pcmu
    int8_t channels;
    int8_t bits;

    uint32_t sample_rate;
    uint32_t video_timescale;

    uint64_t audio_duration;
    MP4TrackId audio_track_id;
    MP4TrackId video_track_id;
}LC_MP4_MUX_CODEC_INFO_t;

typedef struct{
    MP4FileHandle hFile;
    uint8_t *buf;
    uint32_t buf_size;
    LC_MP4_MUX_CODEC_INFO_t info;
}LC_MP4_MUXER_INFO_t;

typedef struct{
    int channels;
    int bits;
    uint32_t sample_rate;
    uint32_t sample_rate_index;

    int8_t video_codec; // -1 : none, 0 :H264, 1: H265
    int8_t audio_codec; // -1 : none, 0: AAC , 1: pcma, 2: pcmu

    MP4TrackId audio_track_id;
    MP4TrackId video_track_id;

    uint64_t duration_ms;

    int64_t start_pts;
    int64_t end_pts;

    uint8_t *video_prefix;
    uint32_t video_prefix_size;
    uint8_t *audio_aes;
    uint32_t audio_aes_size;
}LC_MP4_DEMUXER_MOOV_t;

typedef struct{
    MP4FileHandle hFile;
    LC_MP4_DEMUXER_MOOV_t moov;
    void* frame;

    uint8_t *buf;
    uint32_t buf_size;
    bool video_finished;
    bool audio_finished;

    int32_t audio_sample_id;
    int32_t video_sample_id;

    int64_t cur_pts; // current time in ms
    MP4Timestamp video_abs_ts; // absolute time
    MP4Timestamp audio_abs_ts;
}LC_MP4_DEMUXER_INFO_t;

inline void LC_MP4_BytesFromUInt32BE(uint8_t* bytes, uint32_t value)
{
    bytes[0] = (uint8_t)((value >> 24)&0xFF);
    bytes[1] = (uint8_t)((value >> 16)&0xFF);
    bytes[2] = (uint8_t)((value >>  8)&0xFF);
    bytes[3] = (uint8_t)((value      )&0xFF);
}

bool find_adts_head(uint8_t *data, uint32_t size, uint8_t *header);

void make_dsi(uint32_t sampling_frequency_index,uint32_t channel_configuration, uint8_t* dsi);

void on_data_audio(LC_MP4_MUXER_INFO_t *mux, uint8_t *frame, int size, int64_t pts);

void on_data_video(LC_MP4_MUXER_INFO_t *mux, uint8_t *frame, int size, int64_t pts);

int handle_video_h264(LC_MP4_MUXER_INFO_t *mux, uint8_t *nal_unit, int nal_unit_size, int64_t pts);

int handle_video_h265(LC_MP4_MUXER_INFO_t *mux, uint8_t *nal_unit, int nal_unit_size, int64_t pts);

int parse_mp4info(LC_MP4_DEMUXER_INFO_t *dmux);

int make_video_prefix(LC_MP4_DEMUXER_MOOV_t &moov, std::string &prefix);

int write_adts_header(LC_MP4_DEMUXER_INFO_t *dmx, uint32_t frame_size);

bool read_one_frame(LC_MP4_DEMUXER_INFO_t *mux, bool is_video);


#ifdef __cplusplus
}
#endif


#endif
