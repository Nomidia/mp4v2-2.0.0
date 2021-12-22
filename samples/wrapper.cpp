#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "mp4v2/mp4v2.h"
#include "wrapper_internal.h"
#include "wrapper.h"
#include "avc_hevc_parser.h"

void* lc_mp4_muxer_create_file(const char *file, int vtype, int width, int height, 
    int fps, int atype, int samplerate, int channel,int bits)
{
    if (!file) {
        printf("invalid file name");
        return NULL;
    }

    LC_MP4_MUXER_INFO_t* mux = NULL;
    mux = (LC_MP4_MUXER_INFO_t*)malloc(sizeof(LC_MP4_MUXER_INFO_t));
    if (!mux)
        return NULL;

    // for debug
    //MP4LogSetLevel(MP4_LOG_VERBOSE1);

    memset(mux,0,sizeof(LC_MP4_MUXER_INFO_t));
    mux->hFile = MP4Create(file);
    if (mux->hFile == MP4_INVALID_FILE_HANDLE) {
        free(mux);
        return NULL;
    }

    MP4SetTimeScale(mux->hFile, LC_MP4_TIMESCALE);

    mux->info.video_codec = vtype;
    mux->info.w = width;
    mux->info.h = height;
    mux->info.fps = fps > 0 ? fps : 12;
    mux->info.audio_codec = atype;
    mux->info.sample_rate = samplerate;
    mux->info.video_timescale = LC_MP4_TIMESCALE;
    mux->info.channels = channel;
    mux->info.bits = bits;
    mux->info.audio_track_id = MP4_INVALID_TRACK_ID;
    mux->info.video_track_id = MP4_INVALID_TRACK_ID;
    printf("open succeeded\n");

    return mux;
}

void lc_mp4_muxer_close(void* muxer)
{
    if (!muxer)
        return;

    printf("%s[%d]\n", __FUNCTION__, __LINE__);

    LC_MP4_MUXER_INFO_t* mux = (LC_MP4_MUXER_INFO_t*)muxer;
    MP4Close(mux->hFile);
    if (mux->buf) {
        free(mux->buf);
    }
    printf("%s[%d]\n", __FUNCTION__, __LINE__);
    free(mux);
    printf("close mux ok");
}

int lc_mp4_muxer_write_frame(void *muxer, int is_video, uint8_t *frame, int size, int64_t pts)
{
    if (!muxer)
        return -1;

    LC_MP4_MUXER_INFO_t* mux = (LC_MP4_MUXER_INFO_t*)muxer;
    if (is_video) {
        on_data_video(mux, frame, size, pts);
    } else {
        on_data_audio(mux, frame, size, pts);
    }
}

static uint32_t get_sampling_frequency_index(uint32_t sampling_frequency)
{
    switch (sampling_frequency) {
        case 96000: return 0;
        case 88200: return 1;
        case 64000: return 2;
        case 48000: return 3;
        case 44100: return 4;
        case 32000: return 5;
        case 24000: return 6;
        case 22050: return 7;
        case 16000: return 8;
        case 12000: return 9;
        case 11025: return 10;
        case 8000:  return 11;
        case 7350:  return 12;
        default:    return 0;
    }
}

bool find_adts_head(uint8_t *data, uint32_t size, uint8_t *header)
{
    if (size < LC_MP4_ADTS_HEADER_SIZE)
        return false;

    if ((((data[0] << 8) | data[1]) & LC_MP4_ADTS_SYNC_MASK) == LC_MP4_ADTS_SYNC_PATTERN) {
        if (header) {
            memcpy(header, data, LC_MP4_ADTS_HEADER_SIZE);
        }
        return true;
    }

    return false;
}

void make_dsi(uint32_t sampling_frequency_index,uint32_t channel_configuration, uint8_t* dsi)
{
    uint32_t object_type = LC_MP4_AAC_LC; // AAC LC by default
    dsi[0] = (object_type<<3) | (sampling_frequency_index>>1);
    dsi[1] = ((sampling_frequency_index&1)<<7) | (channel_configuration<<3);
}

void on_data_audio(LC_MP4_MUXER_INFO_t *mux, uint8_t *frame, int size, int64_t pts)
{
    if (mux->info.audio_codec == LC_MP4_CODEC_AAC) {
        //uint8_t raw_header[LC_MP4_ADTS_HEADER_SIZE];
        if (find_adts_head(frame, size, NULL)) {
            frame += LC_MP4_ADTS_HEADER_SIZE;
            size -= LC_MP4_ADTS_HEADER_SIZE;
        }
    }

    if (mux->info.audio_track_id == MP4_INVALID_TRACK_ID) {
        if (mux->info.audio_codec == LC_MP4_CODEC_PCMA || mux->info.audio_codec == LC_MP4_CODEC_PCMU) {
            // if the timescale is equal to  sample rate, duration is equal to size
            // sample_rate * bits / 8 / 2
#if 0
            uint64_t duration = (mux->info.sample_rate * mux->info.bits) >> 4; // total size per second : 8000
            duration = size * 1000 / duration; // sample duration : size / 8000
            duration = (duration * mux->info.sample_rate) / 1000; convert to timescale : equals to size
#else
            MP4Duration duration = size;
#endif
            mux->info.audio_duration = duration;

            printf("audio timescale = %u\n", mux->info.sample_rate);
            printf("audio duration = %lu\n", duration);

            if (mux->info.audio_codec == LC_MP4_CODEC_PCMA ) {
                mux->info.audio_track_id = MP4AddALawAudioTrack(mux->hFile, mux->info.sample_rate, duration);
            } else {
                mux->info.audio_track_id = MP4AddULawAudioTrack(mux->hFile, mux->info.sample_rate, duration);
            }

            if (mux->info.audio_track_id != MP4_INVALID_TRACK_ID) {
                MP4SetTrackIntegerProperty(mux->hFile, mux->info.audio_track_id,"mdia.minf.stbl.stsd.alaw.channels", mux->info.channels);
            } else {
                printf("add type[%u] audio track failed\n", mux->info.audio_codec);
            }
        } else if (mux->info.audio_codec == LC_MP4_CODEC_AAC) {
            MP4Duration duration = 1024; // 1024 samples for aac
            mux->info.audio_duration = duration;
            mux->info.audio_track_id = MP4AddAudioTrack(mux->hFile, mux->info.sample_rate, duration, MP4_MPEG4_AUDIO_TYPE);
            MP4SetTrackIntegerProperty(mux->hFile, mux->info.audio_track_id,"mdia.minf.stbl.stsd.mp4a.channels", mux->info.channels);
            if (mux->info.audio_track_id != MP4_INVALID_TRACK_ID) {
                //MP4SetAudioProfileLevel(mux->hFile, 0x02);
                uint8_t aac_dsi[2];
                uint32_t sampling_frequency_index = get_sampling_frequency_index(mux->info.sample_rate);
                printf("sampling_frequency_index = %u\n", sampling_frequency_index);
                make_dsi(sampling_frequency_index, mux->info.channels, aac_dsi);
                MP4SetTrackESConfiguration(mux->hFile, mux->info.audio_track_id, aac_dsi, 2);
            } else {
                printf("add aac audio track failed\n");
            }
        } else {
            printf("unknown audio codec\n");
        }
    }

    MP4WriteSample(mux->hFile, mux->info.audio_track_id, frame, size, mux->info.audio_duration);
}

void on_data_video(LC_MP4_MUXER_INFO_t *mux, uint8_t *frame, int size, int64_t pts)
{
    // parse nalu type
    int offset = 0, len = 0;
    LC_MP4_MUXER_Nalu_t nalu;
    while ((len = get_one_nalu_from_buf(frame, size, offset, &nalu)) > 0 ) {
        offset += len;
        if (mux->info.video_codec == LC_MP4_CODEC_H264) {
            handle_video_h264(mux, nalu.nal_unit , nalu.nal_unit_size, pts);
        } else if (mux->info.video_codec == LC_MP4_CODEC_H265){
            handle_video_h265(mux, nalu.nal_unit , nalu.nal_unit_size, pts);
        } else {
            printf("unknown video codec\n");
            break;
        }
    }
}

int handle_video_h264(LC_MP4_MUXER_INFO_t *mux, uint8_t *nal_unit, int nal_unit_size, int64_t pts)
{
    uint32_t nalu_type = nal_unit[0] & 0x1F;
    //printf("h264 nalu type = %02x\n", nal_unit[0]);
    bool is_sync = false;
    switch (nalu_type) {
        case LC_MP4_AVC_NAL_UNIT_TYPE_SPS:
            if (mux->info.video_track_id == MP4_INVALID_TRACK_ID) {
                LC_MP4_AVC_SPS_INFO_t sps_info;
                parse_h264_sps(nal_unit, nal_unit_size, &sps_info);
                mux->info.video_track_id = MP4AddH264VideoTrack(mux->hFile, mux->info.video_timescale, mux->info.video_timescale / mux->info.fps,
                    mux->info.w, mux->info.h, sps_info.profile_idc, sps_info.profile_compat, sps_info.level_idc, sps_info.sample_len_field_size_minus_one);

                MP4AddH264SequenceParameterSet(mux->hFile, mux->info.video_track_id, nal_unit, nal_unit_size);
            }
            break;
        case LC_MP4_AVC_NAL_UNIT_TYPE_PPS:
            if (mux->info.video_track_id == MP4_INVALID_TRACK_ID) {
                break;
            }
            MP4AddH264PictureParameterSet(mux->hFile, mux->info.video_track_id, nal_unit, nal_unit_size);
            break;
        case LC_MP4_AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE:
            is_sync = true;
        case LC_MP4_AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_NON_IDR_PICTURE:
            {
                if (mux->info.video_track_id == MP4_INVALID_TRACK_ID) {
                    break;
                }
                uint32_t total_size = nal_unit_size + 4;
                if (total_size > mux->buf_size) {
                    printf("new buf, size is %u\n", total_size);
                    mux->buf = (uint8_t*)realloc(mux->buf, total_size);
                    if (!mux->buf) {
                        return -1;
                    }
                    mux->buf_size = total_size;
                }
                memset(mux->buf, 0, mux->buf_size);
                LC_MP4_BytesFromUInt32BE(mux->buf, nal_unit_size);
                memcpy(mux->buf + 4, nal_unit, nal_unit_size);

                if (!MP4WriteSample(mux->hFile, mux->info.video_track_id, mux->buf, total_size, MP4_INVALID_DURATION, 0, is_sync)) {
                    return -1;
                }
            }
            break;
        case LC_MP4_AVC_NAL_UNIT_TYPE_SEI:
            //skip SEI
            break;
        default:
            break;
    }
    return 0;
}

int handle_video_h265(LC_MP4_MUXER_INFO_t *mux, uint8_t *nal_unit, int nal_unit_size, int64_t pts)
{
    uint32_t nalu_type = (nal_unit[0] >> 1) & 0x3F;
    //printf("h265 nalu[0] = %02x, nalu_type = %u\n", nal_unit[0], nalu_type);
    bool is_sync = false;
    // TODO fix if the order is not vps/sps/pps
    switch (nalu_type) {
        case LC_MP4_HEVC_NAL_UNIT_TYPE_VPS_NUT:
            if (mux->info.video_track_id == MP4_INVALID_TRACK_ID) {
                uint8_t nalue_size_minus_one = 3;
                mux->info.video_track_id = MP4AddH265VideoTrack(mux->hFile, mux->info.video_timescale, mux->info.video_timescale / mux->info.fps,
                    mux->info.w, mux->info.h, nalue_size_minus_one);
                MP4AddH265VideoParameterSet(mux->hFile, mux->info.video_track_id, nal_unit, nal_unit_size);
            }
            break;
        case LC_MP4_HEVC_NAL_UNIT_TYPE_SPS_NUT:
            {
                if (mux->info.video_track_id == MP4_INVALID_TRACK_ID) {
                    break;
                }
                //LC_MP4_AVC_SPS_INFO_t sps_info;
                //parse_h265_sps(nal_unit, nal_unit_size, &sps_info);
                LC_MP4_HevcSequenceParameterSet sps;
                sps.Parse(nal_unit, nal_unit_size);
                uint8_t general_profile_space = sps.profile_tier_level.general_profile_space;
                uint8_t general_tier_flag = sps.profile_tier_level.general_tier_flag;
                uint8_t general_profile = sps.profile_tier_level.general_profile_idc;
                uint32_t general_profile_compatibility_flags = sps.profile_tier_level.general_profile_compatibility_flags;
                uint64_t general_constraint_indicator_flags =  sps.profile_tier_level.general_constraint_indicator_flags;
                uint8_t general_level = sps.profile_tier_level.general_level_idc;
                uint32_t min_spatial_segmentation = 0; // TBD (should read from VUI if present)
                uint8_t parallelism_type = 0; // unknown
                uint8_t chroma_format = sps.chroma_format_idc;
                uint8_t luma_bit_depth = 8; // hardcoded temporarily, should be read from the bitstream
                uint8_t chroma_bit_depth = 8; // hardcoded temporarily, should be read from the bitstream
                uint16_t average_frame_rate =  0; // unknown
                uint8_t constant_frame_rate = 0; // unknown
                uint8_t num_temporal_layers = 0; // unknown
                uint8_t temporal_id_nested = 0; // unknown
                MP4AddH265SequenceParameterSet(mux->hFile, mux->info.video_track_id, nal_unit, nal_unit_size);
                MP4SetH265VideoConfig(mux->hFile, mux->info.video_track_id,
                                general_profile_space,
                                general_tier_flag,
                                general_profile,
                                general_profile_compatibility_flags,
                                general_constraint_indicator_flags,
                                general_level,
                                min_spatial_segmentation,
                                parallelism_type,
                                chroma_format,
                                luma_bit_depth,
                                chroma_bit_depth,
                                average_frame_rate,
                                constant_frame_rate,
                                num_temporal_layers,
                                temporal_id_nested);
            }
            break;
        case LC_MP4_HEVC_NAL_UNIT_TYPE_PPS_NUT:
            if (mux->info.video_track_id == MP4_INVALID_TRACK_ID) {
                break;
            }
            MP4AddH265PictureParameterSet(mux->hFile, mux->info.video_track_id, nal_unit, nal_unit_size);
            break;
        case LC_MP4_HEVC_NAL_UNIT_TYPE_IDR_W_RADL:
        case LC_MP4_HEVC_NAL_UNIT_TYPE_IDR_N_LP:
            is_sync = true;
        case LC_MP4_HEVC_NAL_UNIT_TYPE_TRAIL_N:
        case LC_MP4_HEVC_NAL_UNIT_TYPE_TRAIL_R:
            {
                if (mux->info.video_track_id == MP4_INVALID_TRACK_ID) {
                    break;
                }
                uint32_t total_size = nal_unit_size + 4;
                if (total_size > mux->buf_size) {
                    printf("new buf, nal_unit_size = %u, total_size = %u\n", nal_unit_size, total_size);
                    mux->buf = (uint8_t*)realloc(mux->buf, total_size);
                    if (!mux->buf) {
                        return -1;
                    }
                    mux->buf_size = total_size;
                }
                memset(mux->buf, 0, mux->buf_size);
                LC_MP4_BytesFromUInt32BE(mux->buf, nal_unit_size);
                memcpy(mux->buf + 4, nal_unit, nal_unit_size);

                if (!MP4WriteSample(mux->hFile, mux->info.video_track_id, mux->buf, total_size, MP4_INVALID_DURATION, 0, is_sync)) {
                    return -1;
                }
            }
            break;
        case LC_MP4_AVC_NAL_UNIT_TYPE_SEI:
            //skip SEI
            break;
        default:
            break;
    }
    return 0;
}

int parse_mp4info(LC_MP4_DEMUXER_INFO_t *dmux)
{
    if (dmux == NULL || dmux->hFile == MP4_INVALID_FILE_HANDLE)
        return -1;

    MP4FileHandle hFile = dmux->hFile;
    LC_MP4_DEMUXER_MOOV_t &moov = dmux->moov;

    uint32_t timescale = MP4GetTimeScale(hFile);
    MP4Duration duration = MP4GetDuration(hFile);
    moov.duration_ms = MP4ConvertFromMovieDuration(hFile, duration, MP4_MSECS_TIME_SCALE);
    printf("mp4 file timescale = %u\n", timescale);
    printf("total duration = %lu\n", duration);
    printf("total duration time = %lu ms\n", duration * 1000 / timescale);
    printf("moov.duration_ms time = %lu ms\n", moov.duration_ms);

    moov.video_track_id = MP4_INVALID_TRACK_ID;
    moov.audio_track_id = MP4_INVALID_TRACK_ID;
    std::string sps, pps, vps;
    int audio_rate_index = 0;
    int audio_channel_num = 0;
    int audio_type = 0;
    uint32_t audio_time_scale = 0;
    uint32_t video_time_scale = 0;

    uint32_t num_tracks = MP4GetNumberOfTracks(hFile);
    uint32_t i;
    for (i = 0; i < num_tracks; i++) {
        MP4TrackId track_id = MP4FindTrackId(hFile, i);
        const char* trackType = MP4GetTrackType(hFile, track_id);
        if (trackType == NULL) return -1;

        printf("trackType[%s]\n", trackType);
        if (MP4_IS_VIDEO_TRACK_TYPE(trackType)) {
            if (moov.video_track_id > 0)
                continue;
            moov.video_track_id = track_id;
            dmux->video_sample_id = 1;

            // for debug
            //char *info;
            //info = MP4Info(hFile, track_id);
            //free(info);

            video_time_scale = MP4GetTrackTimeScale(hFile, track_id);
            printf("video timescale = %u\n", video_time_scale);
            // for debug
            const char *video_name = MP4GetTrackMediaDataName(hFile, track_id);
            printf("video codec name[%s]\n", video_name);

            uint8_t  ** vps_header = NULL;
            uint8_t  ** sps_header = NULL;
            uint8_t  ** pps_header = NULL;
            uint32_t  * vps_size = NULL;
            uint32_t  * sps_size = NULL;
            uint32_t  * pps_size = NULL;

            if (strcasecmp(video_name, "avc1") == 0) {
                moov.video_codec = 0;
                // get sps/pps/vps
                if (!MP4GetTrackH264SeqPictHeaders(hFile, track_id, &sps_header, &sps_size, &pps_header, &pps_size)) {
                    continue;
                }
                uint32_t ix = 0;
                for(ix = 0; sps_size[ix] != 0; ++ix) {
                    uint8_t * lpSPS = sps_header[ix];
                    uint32_t  nSize = sps_size[ix];
                    if( sps.size() == 0 && nSize > 0 ) {
                        sps.assign((char*)lpSPS, nSize);
                    }
                    free(sps_header[ix]);
                    printf("===sps index = %d, size = %u ===\n", ix + 1, nSize);
                }
                free(sps_header);
                free(sps_size);
                for(ix = 0; pps_size[ix] != 0; ++ix) {
                    uint8_t * lpPPS = pps_header[ix];
                    uint32_t  nSize = pps_size[ix];
                    if( pps.size() == 0 && nSize > 0 ) {
                        pps.assign((char*)lpPPS, nSize);
                    }
                    free(pps_header[ix]);
                    printf("===pps index = %d, size = %u ===\n", ix+1, nSize);
                }
                free(pps_header);
                free(pps_size);
            } else if (strcasecmp(video_name, "hev1") == 0) {
                moov.video_codec = 1;
                // get sps/pps/vps
                if (!MP4GetTrackH265SeqPictHeaders(hFile, track_id, &vps_header, &vps_size, &sps_header, &sps_size, &pps_header, &pps_size))
                    continue;

                uint32_t ix = 0;
                for(ix = 0; vps_size[ix] != 0; ++ix) {
                    uint8_t * lpVPS = vps_header[ix];
                    uint32_t  nSize = vps_size[ix];
                    if( vps.size() == 0 && nSize > 0 ) {
                        vps.assign((char*)lpVPS, nSize);
                    }
                    free(vps_header[ix]);
                    printf("===vps index = %d, size = %u ===\n", ix + 1, nSize);
                }
                free(vps_header);
                free(vps_size);

                for(ix = 0; sps_size[ix] != 0; ++ix) {
                    uint8_t * lpSPS = sps_header[ix];
                    uint32_t  nSize = sps_size[ix];
                    if( sps.size() == 0 && nSize > 0 ) {
                        sps.assign((char*)lpSPS, nSize);
                    }
                    free(sps_header[ix]);
                    printf("===sps index = %d, size = %u ===\n", ix + 1, nSize);
                }
                free(sps_header);
                free(sps_size);

                for(ix = 0; pps_size[ix] != 0; ++ix) {
                    uint8_t * lpPPS = pps_header[ix];
                    uint32_t  nSize = pps_size[ix];
                    if( pps.size() == 0 && nSize > 0 ) {
                        pps.assign((char*)lpPPS, nSize);
                    }
                    free(pps_header[ix]);
                    printf("===pps index = %d, size = %u ===\n", ix + 1, nSize);
                }
                free(pps_header);
                free(pps_size);
            } else {
                continue;
            }

            MP4Duration track_duration = MP4GetTrackDuration(hFile, track_id);
            double msDuration =
                double(MP4ConvertFromTrackDuration(hFile, track_id,
                                                   track_duration, MP4_MSECS_TIME_SCALE));
            printf("track duration time = %u ms\n", (uint32_t)msDuration);

            // make video prefix
            if (moov.video_prefix) {
                free(moov.video_prefix);
                moov.video_prefix_size = 0;
            }
            make_video_prefix(moov, vps);
            make_video_prefix(moov, sps);
            make_video_prefix(moov, pps);
        } else if (MP4_IS_AUDIO_TRACK_TYPE(trackType)) {
            if (moov.audio_track_id > 0)
                continue;

            moov.bits = 16;
            moov.audio_track_id = track_id;
            dmux->audio_sample_id = 1;

            audio_channel_num = MP4GetTrackAudioChannels(hFile, track_id);
            printf("=== audio channels %d ===\n", audio_channel_num);
            moov.channels = audio_channel_num;
            const char *audio_name = MP4GetTrackMediaDataName(hFile, track_id);
            printf("=== audio codec name %s ===\n", audio_name);
            audio_time_scale = MP4GetTrackTimeScale(hFile, track_id);
            printf("=== audio time scale %u ===\n", audio_time_scale);
            moov.sample_rate = audio_time_scale;

            MP4Duration track_duration = MP4GetTrackDuration(hFile, track_id);
            double msDuration =
                double(MP4ConvertFromTrackDuration(hFile, track_id,
                                                   track_duration, MP4_MSECS_TIME_SCALE));
            printf("track duration time = %u ms\n", (uint32_t)msDuration);

            if (strcasecmp(audio_name, "mp4a") == 0) {
                moov.audio_codec = LC_MP4_CODEC_AAC;
                audio_type = MP4GetTrackAudioMpeg4Type(hFile, track_id);
                printf("=== audio type %d ===\n", audio_type);

                audio_rate_index = get_sampling_frequency_index(audio_time_scale);
                printf("=== audio sample rate index %d ===\n", audio_rate_index);
                moov.sample_rate_index = audio_rate_index;
            } else if (strcasecmp(audio_name, "alaw") == 0) {
                moov.audio_codec = LC_MP4_CODEC_PCMA;
            } else if (strcasecmp(audio_name, "ulaw") == 0) {
                moov.audio_codec = LC_MP4_CODEC_PCMU;
            }
        }
    }

    return 0;
}

void* lc_mp4_demuxer_open(const char *file, uint64_t start_pts)
{
    // for debug
    //MP4LogSetLevel(MP4_LOG_VERBOSE3);

    LC_MP4_DEMUXER_INFO_t* demux = NULL;
    if (!file) {
        printf("invalid file name\n");
        return NULL;
    }

    demux = (LC_MP4_DEMUXER_INFO_t*)malloc(sizeof(LC_MP4_DEMUXER_INFO_t));
    if (!demux)
        return NULL;

    memset(demux,0,sizeof(LC_MP4_DEMUXER_INFO_t));
    demux->hFile = MP4Read(file);
    if (demux->hFile == MP4_INVALID_FILE_HANDLE) {
        free(demux);
        return NULL;
    }

    // get the moov
    parse_mp4info(demux);
    // TODO for test
    //demux->audio_finished = true;
    //demux->video_finished = true;

    demux->video_pts = start_pts;
    demux->audio_pts = start_pts;

    demux->cur_pts = start_pts;
    demux->moov.start_pts = start_pts;
    demux->moov.end_pts = start_pts + demux->moov.duration_ms;
    printf("<jiang> start_pts = %lu, end_pts = %lu\n", demux->moov.start_pts, demux->moov.end_pts);

    LC_MP4_MUXER_FRAME_t *frame = (LC_MP4_MUXER_FRAME_t *)malloc(sizeof(LC_MP4_MUXER_FRAME_t));
    memset(frame, 0, sizeof(LC_MP4_MUXER_FRAME_t));
    frame->bits = demux->moov.bits;
    frame->channels = demux->moov.channels;
    frame->sample_rate = demux->moov.sample_rate;
    frame->pts_ms = demux->cur_pts;
    printf("<jiang> bits = %d, channels = %d, sample rate= %d, frame->pts_ms = %lu\n",
        frame->bits, frame->channels, frame->sample_rate, frame->pts_ms);


    demux->frame = frame;;

    return demux;
}

void lc_mp4_demuxer_close(void* demux)
{
    if (!demux)
        return;

    LC_MP4_DEMUXER_INFO_t* dmux = (LC_MP4_DEMUXER_INFO_t*)demux;
    MP4Close(dmux->hFile);
    if (dmux->buf) {
        LC_MP4_FREE(dmux->buf);
    }
    if (dmux->frame) {
        LC_MP4_FREE(dmux->frame);
    }
    free(dmux);
    printf("close demuxer ok\n");
}

LC_MP4_DEMUXER_CODEC_t* lc_mp4_demux_get_info(void* demux)
{
    LC_MP4_DEMUXER_CODEC_t* info = NULL;
    LC_MP4_DEMUXER_INFO_t* dmx = (LC_MP4_DEMUXER_INFO_t*)demux;
    if (dmx) {
        info = (LC_MP4_DEMUXER_CODEC_t*)malloc(sizeof(LC_MP4_DEMUXER_CODEC_t));
        if (info) {
            info->channels = dmx->moov.channels;
            info->bits = dmx->moov.bits;
            info->sample_rate = dmx->moov.sample_rate;
            info->video_codec = dmx->moov.video_codec;
            info->audio_codec = dmx->moov.audio_codec;
            info->duration = dmx->moov.duration_ms;
        }
    }

    return info;
}

int64_t lc_mp4_demux_get_cur_pts(void* demux)
{
    int64_t pts = 0;
    LC_MP4_DEMUXER_INFO_t* dmx = (LC_MP4_DEMUXER_INFO_t*)demux;
    if (dmx && dmx->frame) {
        LC_MP4_DEMUXER_INFO_t* pframe = (LC_MP4_DEMUXER_INFO_t*)dmx->frame;
        pts = pframe->cur_pts;
    }

    return pts;
}

LC_MP4_MUXER_FRAME_t* lc_mp4_demux_read_frame(void* demux)
{
    if (!demux) {
        return NULL;
    }

    LC_MP4_DEMUXER_INFO_t* dmx = (LC_MP4_DEMUXER_INFO_t*)demux;

    if (dmx->video_sample_id == MP4_INVALID_SAMPLE_ID) {
        dmx->video_finished = true;
    }
    if (dmx->audio_sample_id == MP4_INVALID_SAMPLE_ID) {
        dmx->audio_finished = true;
    }

    bool is_video = false;
    while (true) {
        if (dmx->audio_finished && dmx->video_finished) {
            printf("file over\n");
            return NULL;
        } else if (dmx->audio_finished) {
            is_video = true;
        } else if (dmx->video_finished) {
            is_video = false;
        } else {
            // audio and video synchronization
            if (dmx->video_pts < dmx->audio_pts) {
                is_video = true;
            }
        }

        if (!read_one_frame(dmx, is_video)) {
            if (is_video) {
                dmx->video_finished = true;
            } else {
                dmx->audio_finished = true;
            }
            continue;
        }

        // sync time after seek, when abs(video_pts - audio_pts) >= 500
        if (!is_video && !dmx->video_finished && (dmx->audio_pts + 500 < dmx->video_pts)) {
            printf("sync video_pts = %lu, audio_pts = %lu\n", dmx->video_pts, dmx->audio_pts);
            printf("sync video and audio, span = %lu\n", dmx->video_pts - dmx->audio_pts);
            continue;
        }

        //printf("video ts = %lu, audio ts = %lu, cur ts = %lu\n", dmx->video_pts, dmx->audio_pts, dmx->cur_pts);
        break;
    }

    return (LC_MP4_MUXER_FRAME_t*)dmx->frame;
}

int write_adts_header(LC_MP4_DEMUXER_INFO_t *dmx, uint32_t frame_size)
{
    uint8_t *p = dmx->buf;
    p[0] = 0xFF;
    p[1] = 0xF1; // MPEG-4, 0xF9 (MPEG2)
    p[2] = 0x40 | (dmx->moov.sample_rate_index << 2) | (dmx->moov.channels >> 2);
    p[3] = ((dmx->moov.channels&0x3)<<6) | ((frame_size+LC_MP4_ADTS_HEADER_SIZE) >> 11);
    p[4] = ((frame_size+LC_MP4_ADTS_HEADER_SIZE) >> 3)&0xFF;
    p[5] = (((frame_size+LC_MP4_ADTS_HEADER_SIZE) << 5)&0xFF) | 0x1F;
    p[6] = 0xFC;

    return 0;
}

bool read_one_frame(LC_MP4_DEMUXER_INFO_t *dmx, bool is_video)
{
    MP4TrackId tid = MP4_INVALID_TRACK_ID;
    uint32_t sid = 0;
    uint8_t *sample_bytes = NULL;
    uint32_t num_bytes = 0;
    MP4Timestamp starttime = 0;
    MP4Duration duration = 0;
    bool is_sync = false;
    if (is_video) {
        tid = dmx->moov.video_track_id;
        sid = dmx->video_sample_id++;
    } else {
        tid = dmx->moov.audio_track_id;
        sid = dmx->audio_sample_id++;
    }

    if (!MP4ReadSample(dmx->hFile, tid, sid, &sample_bytes, &num_bytes, &starttime, &duration, NULL, &is_sync))
        return false;

    //printf("%s num_bytes = %u, starttime = %lu, duration = %lu, is_sync = %d\n", __FUNCTION__, num_bytes, starttime, duration, is_sync);

    // sample start time in ms
    starttime = MP4ConvertFromTrackTimestamp(dmx->hFile, tid, starttime, MP4_MSECS_TIME_SCALE);
    //printf("starttime = %lu\n", starttime);

    LC_MP4_MUXER_FRAME_t* pframe = (LC_MP4_MUXER_FRAME_t*)dmx->frame;
    pframe->pts_ms = dmx->moov.start_pts + starttime;
    pframe->is_key_frame = is_sync;
    pframe->is_video = is_video;
    pframe->paylaod_type = is_video ? dmx->moov.video_codec : dmx->moov.audio_codec;

    // TODO remain a fixed buffer in dmx by getting max sample size from all tracks, when the file is opened
    uint32_t expect_size = num_bytes;
    uint32_t offset = 0;
    if (is_video) {
        if (is_sync && dmx->moov.video_prefix_size > 0) {
            expect_size += dmx->moov.video_prefix_size;
            offset += dmx->moov.video_prefix_size;
        }
        if (dmx->buf_size < expect_size) {
            dmx->buf = (uint8_t*)realloc(dmx->buf, expect_size);
            dmx->buf_size = expect_size;
        }

        // add prefix, vps/sps/pps
        if (is_sync && dmx->moov.video_prefix_size > 0) {
            memcpy(dmx->buf, dmx->moov.video_prefix, dmx->moov.video_prefix_size);
        }
        uint8_t *p = dmx->buf + offset;
        memcpy(p, sample_bytes, num_bytes);
        p[0] = 0;
        p[1] = 0;
        p[2] = 0;
        p[3] = 1;
    } else {
        bool add_adts_head = false;
        // check if it is needed to add adts for every aac frame
        if (dmx->moov.audio_codec == LC_MP4_CODEC_AAC) {
            add_adts_head = !find_adts_head(sample_bytes, num_bytes, NULL);
            if (add_adts_head) {
                expect_size += LC_MP4_ADTS_HEADER_SIZE;
            }
        }
        if (dmx->buf_size < expect_size) {
            dmx->buf = (uint8_t*)realloc(dmx->buf, expect_size);
            dmx->buf_size = expect_size;
        }
        // add adts
        if (add_adts_head) {
            write_adts_header(dmx, num_bytes);
            offset += LC_MP4_ADTS_HEADER_SIZE;
        }
        memcpy(dmx->buf + offset, sample_bytes, num_bytes);
    }
    free(sample_bytes);

    pframe->buf = dmx->buf;
    pframe->size = expect_size;
    //printf("pframe->pts_ms = %lu\n", pframe->pts_ms);
    if (is_video) {
        //dmx->video_pts = starttime + MP4ConvertFromTrackDuration(dmx->hFile, tid, duration, MP4_MSECS_TIME_SCALE);
        dmx->video_pts = starttime;
        dmx->cur_pts = dmx->moov.start_pts + dmx->video_pts;
    } else {
        //dmx->audio_pts = starttime + MP4ConvertFromTrackDuration(dmx->hFile, tid, duration, MP4_MSECS_TIME_SCALE);
        dmx->audio_pts = starttime;
        if (dmx->moov.video_track_id == MP4_INVALID_TRACK_ID)
            dmx->cur_pts = dmx->moov.start_pts + dmx->audio_pts;
    }

    return true;
}

int make_video_prefix(LC_MP4_DEMUXER_MOOV_t &moov, std::string &prefix)
{
    if (prefix.empty())
        return -1;

    uint32_t pre_prefix_size = moov.video_prefix_size;
    moov.video_prefix_size += (4 + prefix.size());
    moov.video_prefix = (uint8_t*)realloc(moov.video_prefix , moov.video_prefix_size);
    uint8_t *p = moov.video_prefix + pre_prefix_size;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 1;
    memcpy(p, prefix.c_str(), prefix.size());
    return 0;
}

int lc_mp4_demux_seek(void* demux, int64_t start_pts)
{
    if (!demux) {
        return -1;
    }

    LC_MP4_DEMUXER_INFO_t* dmx = (LC_MP4_DEMUXER_INFO_t*)demux;
    if (start_pts < 0 || start_pts >= dmx->moov.duration_ms) {
        printf("invalid position: %ld, duration = %lu\n", start_pts, dmx->moov.duration_ms);
    }

    if (dmx->moov.video_track_id == MP4_INVALID_TRACK_ID && dmx->moov.audio_track_id == MP4_INVALID_TRACK_ID) {
        printf("seek invalid index, no track\n");
        return -1;
    }

    MP4TrackId tid = MP4_INVALID_TRACK_ID;
    bool want_sync = true;
    MP4Timestamp when = 0;
    MP4Timestamp video_start_ms, audio_start_ms;
    MP4TrackId video_sample_id = MP4_INVALID_TRACK_ID;
    MP4TrackId audio_sample_id = MP4_INVALID_TRACK_ID;
    if (dmx->moov.video_track_id != MP4_INVALID_TRACK_ID) {
        tid = dmx->moov.video_track_id;
        // whether it needs to be the nearest sync sample, before or next
        when = MP4ConvertToTrackTimestamp(dmx->hFile, tid, start_pts, MP4_MSECS_TIME_SCALE);
        video_sample_id = MP4GetSampleIdFromTime(dmx->hFile, tid, when, want_sync);
        if (video_sample_id == MP4_INVALID_SAMPLE_ID) {
            printf("find video sample id failed\n");
        } else {
            video_start_ms = MP4GetSampleTime(dmx->hFile, tid, video_sample_id);
            video_start_ms = MP4ConvertFromTrackTimestamp(dmx->hFile, tid, video_start_ms, MP4_MSECS_TIME_SCALE);
            // reset video_start_ms according to video start time
            start_pts = video_start_ms;
            printf("seek video_sample_id = %u, video_start_ms = %lu\n", video_sample_id, video_start_ms);
        }
    }

    if (dmx->moov.audio_track_id != MP4_INVALID_TRACK_ID) {
        tid = dmx->moov.audio_track_id;
        when = MP4ConvertToTrackTimestamp(dmx->hFile, tid, start_pts, MP4_MSECS_TIME_SCALE);
        audio_sample_id = MP4GetSampleIdFromTime(dmx->hFile, tid, when, want_sync);
        if (audio_sample_id == MP4_INVALID_SAMPLE_ID) {
            printf("find audio sample id failed\n");
        } else {
            audio_start_ms = MP4GetSampleTime(dmx->hFile, tid, audio_sample_id);
            audio_start_ms = MP4ConvertFromTrackTimestamp(dmx->hFile, tid, audio_start_ms, MP4_MSECS_TIME_SCALE);
            printf("seek audio_sample_id = %u, audio_start_ms = %lu\n", audio_sample_id, audio_start_ms);
        }
    }

    if (video_sample_id == MP4_INVALID_SAMPLE_ID && audio_sample_id == MP4_INVALID_SAMPLE_ID) {
        return -1;
    }

    dmx->video_sample_id = video_sample_id;
    dmx->audio_sample_id = audio_sample_id;

    dmx->video_pts = video_start_ms;
    dmx->audio_pts = audio_start_ms;
    if (audio_sample_id != MP4_INVALID_SAMPLE_ID) {
        dmx->cur_pts = dmx->moov.start_pts + dmx->video_pts;
    } else {
        dmx->cur_pts = dmx->moov.start_pts + dmx->audio_pts;
    }

    return 0;
}

