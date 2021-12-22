#include <stdio.h>
#include <string.h>

#define PATH_FILE_NAME "./send/%d.264"
#define PATH_FILE_AAC_NAME "./aacSampleFrames/sample-%03d.aac"

int compose_264()
{
    FILE *fp_in = NULL, *fp_out = NULL;
    int i = 1;
    char filename[64];
    char buf[64*1024];
    int len = 0;
    for (;i <= 338; i++) {
        memset(filename, 0, sizeof(filename));
        memset(buf, 0, sizeof(buf));
        sprintf(filename, PATH_FILE_NAME, i);
        fp_in = fopen(filename, "rb");
        //printf("%s\n", filename);
        if (!fp_in) {
            break;
        }

        len = fread(buf, 1, sizeof(buf), fp_in);
        fclose(fp_in);
        if (!fp_out) {
            fp_out = fopen("video.264", "w+");
            if (!fp_out) {
                break;
            }
        }
        fwrite(buf, 1, len, fp_out);
        
    }

    if (fp_out) {
        fclose(fp_out);
    }
    return 0;
}

int compose_aac()
{
    FILE *fp_in = NULL, *fp_out = NULL;
    int i = 1;
    char filename[64];
    char buf[64*1024];
    int len = 0;
    for (;i <= 200; i++) {
        memset(filename, 0, sizeof(filename));
        memset(buf, 0, sizeof(buf));
        sprintf(filename, PATH_FILE_AAC_NAME, i);
        fp_in = fopen(filename, "rb");
        //printf("%s\n", filename);
        if (!fp_in) {
            break;
        }

        len = fread(buf, 1, sizeof(buf), fp_in);
        fclose(fp_in);
        if (!fp_out) {
            fp_out = fopen("audio.aac", "w+");
            if (!fp_out) {
                break;
            }
        }
        fwrite(buf, 1, len, fp_out);
        
    }

    if (fp_out) {
        fclose(fp_out);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    compose_aac();

    return 0;
}
