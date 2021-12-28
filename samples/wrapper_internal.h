#ifndef __WRAPPER_INTERNAL_H__
#define __WRAPPER_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


#define LC_MP4_TIMESCALE 44100

#define LC_MP4_FREE(p) do { if (p) {free(p); p = NULL;}} while(0)

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
