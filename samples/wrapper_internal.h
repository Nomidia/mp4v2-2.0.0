#ifndef __WRAPPER_INTERNAL_H__
#define __WRAPPER_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif


#define LC_MP4_TIMESCALE 44100

// #define LC_MP4_FREE(p) do { if (p) {free(p); p = NULL;}} while(0)

inline void LC_MP4_BytesFromUInt32BE(uint8_t* bytes, uint32_t value)
{
    bytes[0] = (uint8_t)((value >> 24)&0xFF);
    bytes[1] = (uint8_t)((value >> 16)&0xFF);
    bytes[2] = (uint8_t)((value >>  8)&0xFF);
    bytes[3] = (uint8_t)((value      )&0xFF);
}

void on_data_audio(LC_MP4_MUXER_INFO_t *mux, uint8_t *frame, int size, int64_t pts);

void on_data_video(LC_MP4_MUXER_INFO_t *mux, uint8_t *frame, int size, int64_t pts);

int handle_video_h264(LC_MP4_MUXER_INFO_t *mux, uint8_t *nal_unit, int nal_unit_size, uint64_t duration);

int handle_video_h265(LC_MP4_MUXER_INFO_t *mux, uint8_t *nal_unit, int nal_unit_size, uint64_t duration);

int parse_mp4info(LC_MP4_DEMUXER_INFO_t *dmux);

int make_video_prefix(LC_MP4_DEMUXER_MOOV_t &moov, std::string &prefix);

int write_adts_header(LC_MP4_DEMUXER_INFO_t *dmx, uint32_t frame_size);

bool read_one_frame(LC_MP4_DEMUXER_INFO_t *mux, bool is_video);


#ifdef __cplusplus
}
#endif


#endif
