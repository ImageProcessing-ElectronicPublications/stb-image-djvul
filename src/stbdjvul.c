#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "image-stb.h"
#include "djvul.h"

void djvul_usage(char* prog, unsigned int bgs, unsigned int level, int wbmode, float doverlay, float anisotropic)
{
    printf("StbDjVuL version %s.\n", DJVUL_VERSION);
    printf("usage: %s [options] image_in bw_mask_out.png [bg_out.png] [fg_out.png]\n", prog);
    printf("options:\n");
    printf("  -a N.N    factor anisortopic (default %f)\n", anisotropic);
    printf("  -b NUM    downsample FG and BG (default %d)\n", bgs);
    printf("  -l NUM    level of scale blocks (default %d)\n", level);
    printf("  -o N.N    part of overlay blocks (default %f)\n", doverlay);
    printf("  -w        white/black mode (default %d)\n", wbmode);
    printf("  -h        show this help message and exit\n");
}

int main(int argc, char **argv)
{
    unsigned int bgs = 3;
    unsigned int level = 0;
    int wbmode = 1;
    float doverlay = 0.5f;
    float anisotropic = 0.0f;
    int fhelp = 0;
    int opt;
    while ((opt = getopt(argc, argv, ":a:b:l:o:wh")) != -1)
    {
        switch(opt)
        {
        case 'a':
            anisotropic = atof(optarg);
            break;
        case 'b':
            bgs = atoi(optarg);
            if (bgs < 1)
            {
                fprintf(stderr, "Bad argument\n");
                fprintf(stderr, "bgs = %d\n", bgs);
                return 1;
            }
            break;
        case 'l':
            level = atoi(optarg);
            if (level < 0)
            {
                fprintf(stderr, "Bad argument\n");
                fprintf(stderr, "level = %d\n", level);
                return 1;
            }
            break;
        case 'o':
            doverlay = atof(optarg);
            if (doverlay < 0.0f)
            {
                fprintf(stderr, "Bad argument\n");
                fprintf(stderr, "overlay = %f\n", doverlay);
                return 1;
            }
            break;
        case 'w':
            wbmode = -wbmode;
            break;
        case 'h':
            fhelp = 1;
            break;
        case ':':
            fprintf(stderr, "ERROR: option needs a value\n");
            return 2;
            break;
        case '?':
            fprintf(stderr, "ERROR: unknown option: %c\n", optopt);
            return 3;
            break;
        }
    }
    if(optind + 2 > argc || fhelp)
    {
        djvul_usage(argv[0], bgs, level, wbmode, doverlay, anisotropic);
        return 0;
    }
    const char *src_name = argv[optind];
    const char *mask_name = argv[optind + 1];
    const char *bg_name = NULL;
    if(optind + 2 < argc) bg_name = argv[optind + 2];
    const char *fg_name = NULL;
    if(optind + 3 < argc) fg_name = argv[optind + 3];

    int height, width, channels;

    printf("Load: %s\n", src_name);
    stbi_uc* img = NULL;
    if (!(img = stbi_load(src_name, &width, &height, &channels, STBI_rgb_alpha)))
    {
        fprintf(stderr, "ERROR: not read image: %s\n", src_name);
        return 1;
    }
    printf("image: %dx%d:%d\n", width, height, channels);
    unsigned char* data = NULL;
    if (!(data = (unsigned char*)malloc(height * width * IMAGE_CHANNELS * sizeof(unsigned char))))
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 1;
    }
    size_t ki = 0, kd = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int d = 0; d < IMAGE_CHANNELS; d++)
            {
                data[kd + d] = (unsigned char)img[ki + d];
            }
            ki += STBI_rgb_alpha;
            kd += IMAGE_CHANNELS;
        }
    }
    stbi_image_free(img);

    int bg_width = (width + bgs - 1) / bgs;
    int bg_height = (height + bgs - 1) / bgs;
    printf("BG,FG: %dx%d:%d\n", bg_width, bg_height, IMAGE_CHANNELS);

    bool *mask_data = NULL;
    if (!(mask_data = (bool*)malloc(height * width * IMAGE_CHANNELS * sizeof(bool))))
    {
        fprintf(stderr, "ERROR: not memiory\n");
        return 2;
    }
    unsigned char *bg_data = NULL;
    if (!(bg_data = (unsigned char*)malloc(bg_height * bg_width * IMAGE_CHANNELS * sizeof(unsigned char))))
    {
        fprintf(stderr, "ERROR: not memiory\n");
        return 2;
    }
    unsigned char *fg_data = NULL;
    if (!(fg_data = (unsigned char*)malloc(bg_height * bg_width * IMAGE_CHANNELS * sizeof(unsigned char))))
    {
        fprintf(stderr, "ERROR: not memiory\n");
        return 2;
    }

    printf("DjVuL...");
    if(!(level = ImageDjvulThreshold(data, mask_data, bg_data, fg_data, width, height, bgs, level, wbmode, doverlay, anisotropic)))
    {
        fprintf(stderr, "ERROR: not complite DjVuL\n");
        return 3;
    }

    ki = 0;
    kd = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            unsigned char bw = (mask_data[ki]) ? 0 : 255;
            for (int d = 0; d < IMAGE_CHANNELS; d++)
            {
                data[kd] = bw;
                kd++;
            }
            ki++;
        }
    }
    printf(" %d level\n", level);

    printf("Save png: %s", mask_name);
    if (!(stbi_write_png(mask_name, width, height, IMAGE_CHANNELS, data, width * IMAGE_CHANNELS)))
    {
        fprintf(stderr, "ERROR: not write image: %s\n", mask_name);
        return 1;
    }
    if (bg_name)
    {
        printf(", %s", bg_name);
        if (!(stbi_write_png(bg_name, bg_width, bg_height, IMAGE_CHANNELS, bg_data, bg_width * IMAGE_CHANNELS)))
        {
            fprintf(stderr, "ERROR: not write image: %s\n", bg_name);
            return 1;
        }
    }
    if (fg_name)
    {
        printf(", %s", fg_name);
        if (!(stbi_write_png(fg_name, bg_width, bg_height, IMAGE_CHANNELS, fg_data, bg_width * IMAGE_CHANNELS)))
        {
            fprintf(stderr, "ERROR: not write image: %s\n", fg_name);
            return 1;
        }
    }
    printf(".\n");

    free(bg_data);
    free(fg_data);
    free(mask_data);
    free(data);

    return 0;
}
