#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#include <stdint.h>

#include "mp4v2/mp4v2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LC_MP4_CODEC_H264 = 0,
    LC_MP4_CODEC_H265 = 1,
    LC_MP4_CODEC_AAC = 2,
    LC_MP4_CODEC_PCMA = 3,
    LC_MP4_CODEC_PCMU = 4
} LC_MP4_CODEC_TYPES_e;

typedef struct{
    int64_t pts_ms;

    int paylaod_type;
    int is_video;

    int channels;
    int bits;
    int sample_rate;

    int is_key_frame;

    int size;
    uint8_t* buf;
}LC_MP4_MUXER_FRAME_t;

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
    int64_t last_video_ts;
    uint64_t last_duration;
    LC_MP4_MUX_CODEC_INFO_t info;
}LC_MP4_MUXER_INFO_t;

typedef struct{
    int channels;
    int bits;
    uint32_t sample_rate;
    uint32_t sample_rate_index;

    uint32_t max_sample_size;
    uint32_t nalu_length_size;

    int8_t video_codec;
    int8_t audio_codec;

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
    LC_MP4_MUXER_FRAME_t frame;

    uint8_t *sample;

    uint8_t *buf;
    uint32_t buf_size;
    bool video_finished;
    bool audio_finished;

    MP4SampleId audio_sample_num;
    MP4SampleId video_sample_num;

    MP4SampleId audio_sample_id;
    MP4SampleId video_sample_id;

    int64_t cur_pts; // current time in ms
    MP4Timestamp video_abs_ts; // absolute time
    MP4Timestamp audio_abs_ts;
}LC_MP4_DEMUXER_INFO_t;

void* lc_mp4_muxer_open(const char *file, int vtype, int width, int height, 
    int fps, int atype, int samplerate, int channel,int bits);

void lc_mp4_muxer_close(void* muxer);

int64_t lc_mp4_muxer_get_mux_duration(void* muxer);

int lc_mp4_muxer_write_frame(void *muxer, int is_video, uint8_t *frame, int size, int64_t pts);

// demux

void* lc_mp4_demux_open(const char *file, uint64_t start_pts);

LC_MP4_DEMUXER_MOOV_t* lc_mp4_demux_get_info(void* demux);

int64_t lc_mp4_demux_get_cur_pts(void* demux);

LC_MP4_MUXER_FRAME_t* lc_mp4_demux_read_frame(void* demux);

int lc_mp4_demux_seek(void* demux, int64_t position_ms);

void lc_mp4_demux_close(void* demux);

#ifdef __cplusplus
}
#endif


#endif
