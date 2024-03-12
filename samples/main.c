#include <stdio.h>
#include <stdlib.h>
#include "wrapper.h"
#include <string.h>

#define PATH_FILE_NAME "./send/%d.264"
#define PATH_AAC_SAMPLES "./aacSamples/%d.aac"
#define AAC_SAMPLES_NUM 2000

#define H264_SAMPLES_NUM 0x7fffffff

#define PATH_H265_SAMPLES "./h265Samples/%d.265"
//#define H265_SAMPLES_NUM 300
#define H265_SAMPLES_NUM 0x7fffffff

int record_h264()
{
    void *muxer = lc_mp4_muxer_open("./media/test_264.mp4", LC_MP4_CODEC_H264, 480, 640, 20, 1, 8000, 1, 16);
    if (!muxer) {
        printf("handle is null\n");
    }

    FILE *fp_in = NULL;
    int i = 1;
    char filename[64];
    char buf[64*1024];
    int len = 0;
    for (;i <= 90; i++) {
        memset(filename, 0, sizeof(filename));
        memset(buf, 0, sizeof(buf));
        sprintf(filename, PATH_FILE_NAME, i);
        fp_in = fopen(filename, "rb");
        if (!fp_in) {
            break;
        }

        len = fread(buf, 1, sizeof(buf), fp_in);
        fclose(fp_in);

        lc_mp4_muxer_write_frame(muxer, 1, (uint8_t*)buf, len, 0);
    }
    printf("close file\n");
    lc_mp4_muxer_close(muxer);
    return 0;
}

int record_h265()
{
    void *muxer = lc_mp4_muxer_open("./media/test_265.mp4", LC_MP4_CODEC_H265, 1280, 720, 24, LC_MP4_CODEC_AAC, 48000, 2, 16);
    if (!muxer) {
        printf("handle is null\n");
    }

    FILE *fp_in = NULL;
    int i = 0;
    char filename[64];
    char buf[64*1024];
    int len = 0;
    for (;i <= H265_SAMPLES_NUM; i++) {
        memset(filename, 0, sizeof(filename));
        memset(buf, 0, sizeof(buf));
        sprintf(filename, PATH_H265_SAMPLES, i);
        fp_in = fopen(filename, "rb");
        if (!fp_in) {
            break;
        }

        len = fread(buf, 1, sizeof(buf), fp_in);
        fclose(fp_in);

        lc_mp4_muxer_write_frame(muxer, 1, (uint8_t*)buf, len, 0);
    }
    printf("close file\n");
    lc_mp4_muxer_close(muxer);
    return 0;
}

int record_g711a()
{
    void *muxer = lc_mp4_muxer_open("test_g711a.mp4", LC_MP4_CODEC_H264, 480, 640, 20, 1, 8000, 1, 16);
    if (!muxer) {
        printf("handle is null\n");
    }

    FILE *fp_in = NULL;
    char buf[480];
    int len = 0;
    fp_in = fopen("audio.g711a", "rb");
    if (!fp_in) {
        printf("open file failed\n");
        return -1;
    }

    while ((len = fread(buf, 1, sizeof(buf), fp_in)) == 480) {
        //memset(buf, 0, sizeof(buf));
        lc_mp4_muxer_write_frame(muxer, 0, (uint8_t*)buf, len, 0);
    }
    printf("close file\n");
    if (fp_in)
        fclose(fp_in);

    lc_mp4_muxer_close(muxer);
    return 0;
}

int record_aac()
{
    void *muxer = lc_mp4_muxer_open("test_aac.mp4", LC_MP4_CODEC_H264, 480, 640, 20, 0, 48000, 2, 16);
    if (!muxer) {
        printf("handle is null\n");
    }

    FILE *fp_in = NULL;
    int i = 1;
    char filename[64];
    char buf[64*1024];
    int len = 0;
    for (;i <= AAC_SAMPLES_NUM; i++) {
        memset(filename, 0, sizeof(filename));
        memset(buf, 0, sizeof(buf));
        sprintf(filename, PATH_AAC_SAMPLES, i);
        fp_in = fopen(filename, "rb");
        if (!fp_in) {
            break;
        }

        len = fread(buf, 1, sizeof(buf), fp_in);
        fclose(fp_in);

        lc_mp4_muxer_write_frame(muxer, 0, (uint8_t*)buf, len, 0);
    }
    printf("close file\n");
    lc_mp4_muxer_close(muxer);
    return 0;
}

int read_file_264()
{
    void *demuxer = lc_mp4_demux_open("./media/264_alaw.mp4", 0); //1639472160000
    LC_MP4_DEMUXER_MOOV_t* info = lc_mp4_demux_get_info(demuxer);
    if (!info)
        return -1;

    LC_MP4_MUXER_FRAME_t* pframe;
    int i = 0;

    FILE *fp_out = NULL;
    fp_out = fopen("./media/264_alaw.264", "wb+");
    lc_mp4_demux_seek(demuxer, 10000);
    printf("cur_pts = %ld\n", lc_mp4_demux_get_cur_pts(demuxer));
    int index = 0;
    while ((pframe = lc_mp4_demux_read_frame(demuxer)) && index++ < H264_SAMPLES_NUM) {
        if (pframe->is_video) {
            //printf("cur_pts = %ld\n", lc_mp4_demux_get_cur_pts(demuxer));
            fwrite(pframe->buf, 1, pframe->size, fp_out);
        }
        //printf("[%d]pts = %lu,  payload_type = %d, is_video = %d, is_key_frame = %d, size = %d\n",
        //    i++, pframe->pts_ms, pframe->paylaod_type, pframe->is_video, pframe->is_key_frame, pframe->size);
    }
    fclose(fp_out);

    lc_mp4_demux_close(demuxer);
    return 0;
}

int read_file_265()
{
    void *demuxer = lc_mp4_demux_open("./media/junjie_265.mp4", 0); //1639472160000
    LC_MP4_DEMUXER_MOOV_t* info = lc_mp4_demux_get_info(demuxer);
    if (!info)
        return -1;

    LC_MP4_MUXER_FRAME_t* pframe;
    int i = 0;
#if 1
#if 1
    FILE *fp_out = NULL;
    fp_out = fopen("./media/junjie.265", "wb+");
    lc_mp4_demux_seek(demuxer, 235000);
    uint32_t index = 0;
    while ((pframe = lc_mp4_demux_read_frame(demuxer)) && index <= H265_SAMPLES_NUM) {
        if (pframe->is_video) {
            fwrite(pframe->buf, 1, pframe->size, fp_out);
            index ++;
        }
        //printf("[%d]pts = %lu,  payload_type = %d, is_video = %d, is_key_frame = %d, size = %d\n",
        //    i++, pframe->pts_ms, pframe->paylaod_type, pframe->is_video, pframe->is_key_frame, pframe->size);
    }
    fclose(fp_out);
#else
    FILE *fp_out = NULL;
    char filename[64];
    uint32_t index = 0;
    while ((pframe = lc_mp4_demux_read_frame(demuxer)) && index <= H265_SAMPLES_NUM) {
        if (pframe->is_video) {
            memset(filename, 0, sizeof(filename));
            sprintf(filename, PATH_H265_SAMPLES, index);
            fp_out = fopen(filename, "w+");
            fwrite(pframe->buf, 1, pframe->size, fp_out);
            fclose(fp_out);
            index++;
        }
    }
#endif
#endif
    lc_mp4_demux_close(demuxer);
    return 0;
}

int read_file_audio()
{
    void *demuxer = lc_mp4_demux_open("junjie_265.mp4", 0); //1639472160000
    LC_MP4_DEMUXER_MOOV_t* info = lc_mp4_demux_get_info(demuxer);
    if (!info)
        return -1;

    LC_MP4_MUXER_FRAME_t* pframe;
    int i = 0;

#if 1
    FILE *fp_out = NULL;
    fp_out = fopen("junjie.aac", "wb+");
    while (pframe = lc_mp4_demux_read_frame(demuxer)) {
        if (pframe->is_video == 0)
            fwrite(pframe->buf, 1, pframe->size, fp_out);
        //printf("[%d]pts = %lu,  payload_type = %d, is_video = %d, is_key_frame = %d, size = %d\n",
        //    i++, pframe->pts_ms, pframe->paylaod_type, pframe->is_video, pframe->is_key_frame, pframe->size);
    }
    fclose(fp_out);
#endif
    lc_mp4_demux_close(demuxer);
    return 0;
}

int read_file_audio_aac()
{
    void *demuxer = lc_mp4_demux_open("korean.mp4", 0);
    LC_MP4_DEMUXER_MOOV_t* info = lc_mp4_demux_get_info(demuxer);
    if (!info)
        return -1;

    LC_MP4_MUXER_FRAME_t* pframe;
    FILE *fp_out = NULL;
    char filename[64];
    uint32_t index = 0;
    while ((pframe = lc_mp4_demux_read_frame(demuxer)) && index <= AAC_SAMPLES_NUM) {
        if (pframe->is_video == 0) {
            memset(filename, 0, sizeof(filename));
            sprintf(filename, PATH_AAC_SAMPLES, index);
            fp_out = fopen(filename, "w+");
            fwrite(pframe->buf, 1, pframe->size, fp_out);
            fclose(fp_out);
            index++;
        } else {
            printf("video\n");
        }
        //printf("[%d]pts = %lu,  payload_type = %d, is_video = %d, is_key_frame = %d, size = %d\n",
        //    i++, pframe->pts_ms, pframe->paylaod_type, pframe->is_video, pframe->is_key_frame, pframe->size);
    }

    lc_mp4_demux_close(demuxer);
    return 0;
}


void parse_file_name()
{
    char path_file[] = "xxx_1640243008092_1614301_64000.mp4";

    char path[128];
    char prefix[128];
    uint64_t size = 0;
    uint64_t stamp_ms = 0;
    uint32_t du = 0;
    int n = 0;
    n = sscanf(path_file,"%*[^_]_%lu_%lu_%u.mp4", &stamp_ms, &size, &du);
    printf("%d n = %d\n", __LINE__, n);
    printf("%d size = %lu, stamp_ms = %lu, du = %u\n", __LINE__, size, stamp_ms, du);
}

int main(int argc, char* argv[])
{
    // first read form mp4 files, and create files of aac/g711a/264/265
    //record_h264();
    //record_h265();
    //record_g711a();
    read_file_264();
    //read_file_265();
    //read_file_audio();
    //read_file_audio_aac();
    //record_aac();
    //parse_file_name();
    return 0;
}
