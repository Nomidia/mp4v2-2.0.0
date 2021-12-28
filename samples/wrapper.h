#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LC_MP4_CODEC_H264 0
#define LC_MP4_CODEC_H265 1

#define LC_MP4_CODEC_AAC 0
#define LC_MP4_CODEC_PCMA 1
#define LC_MP4_CODEC_PCMU 2

typedef struct{
    int channels;
    int bits;
    uint32_t sample_rate;

    int8_t video_codec; // -1 : none, 0 :H264, 1: H265
    int8_t audio_codec; // -1 : none, 0: AAC , 1: pcma, 2: pcmu

    uint64_t duration;
}LC_MP4_DEMUXER_CODEC_t;

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


void* lc_mp4_muxer_create_file(const char *file, int vtype, int width, int height, 
    int fps, int atype, int samplerate, int channel,int bits);

void lc_mp4_muxer_close(void* muxer);

int lc_mp4_muxer_write_frame(void *muxer, int is_video, uint8_t *frame, int size, int64_t pts);

void* lc_mp4_demuxer_open(const char *file, uint64_t start_pts);

void lc_mp4_demuxer_close(void* demux);

LC_MP4_DEMUXER_CODEC_t* lc_mp4_demux_get_info(void* demux);

int64_t lc_mp4_demux_get_cur_pts(void* demux);

LC_MP4_MUXER_FRAME_t* lc_mp4_demux_read_frame(void* demux);

int lc_mp4_demux_seek(void* demux, int64_t start_pts);


#ifdef __cplusplus
}
#endif


#endif
