#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "image-stb.h"
#include "djvul.h"

int main(int argc, char **argv)
{
    float ratio = 1.0f;
    unsigned int bgs = 3;
    unsigned int level = 0;
    int wbmode = 1;
    float doverlay = 0.5f;
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s image_in mask.png fg.png bg.png [bgs] [level] [wbmode] [overlay]\n", argv[0]);
        fprintf(stderr, "bgs = %d\n", bgs);
        fprintf(stderr, "level = %d\n", level);
        fprintf(stderr, "w/b = %d\n", wbmode);
        fprintf(stderr, "overlay = %f\n", doverlay);
        return 0;
    }
    if (argc > 5)
    {
        bgs = atoi(argv[5]);
        if (bgs < 1)
        {
            fprintf(stderr, "Bad argument\n");
            fprintf(stderr, "bgs = %d\n", bgs);
            return 1;
        }
    }
    if (argc > 6)
    {
        level = atoi(argv[6]);
        if (level < 0)
        {
            fprintf(stderr, "Bad argument\n");
            fprintf(stderr, "level = %d\n", level);
            return 1;
        }
    }
    if (argc > 7)
    {
        wbmode = atoi(argv[7]);
    }
    if (argc > 8)
    {
        doverlay = atof(argv[8]);
        if (doverlay < 0.0f)
        {
            fprintf(stderr, "Bad argument\n");
            fprintf(stderr, "overlay = %f\n", doverlay);
            return 1;
        }
    }
    const char *src_name = argv[1];
    const char *mask_name = argv[2];
    const char *fg_name = argv[3];
    const char *bg_name = argv[4];

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

    int bg_height = (width + bgs - 1) / bgs;
    int bg_width = (height + bgs - 1) / bgs;
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

    printf("DjVuL...\n");
    if(!ImageDjvulThreshold(data, mask_data, bg_data, fg_data, width, height, bgs, level, wbmode, doverlay))
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

    printf("Save png: %s", mask_name);
    if (!(stbi_write_png(mask_name, width, height, IMAGE_CHANNELS, data, width * IMAGE_CHANNELS)))
    {
        fprintf(stderr, "ERROR: not write image: %s\n", mask_name);
        return 1;
    }
    printf(", %s", fg_name);
    if (!(stbi_write_png(fg_name, bg_width, bg_height, IMAGE_CHANNELS, fg_data, bg_width * IMAGE_CHANNELS)))
    {
        fprintf(stderr, "ERROR: not write image: %s\n", fg_name);
        return 1;
    }
    printf(", %s\n", bg_name);
    if (!(stbi_write_png(bg_name, bg_width, bg_height, IMAGE_CHANNELS, bg_data, bg_width * IMAGE_CHANNELS)))
    {
        fprintf(stderr, "ERROR: not write image: %s\n", bg_name);
        return 1;
    }
    free(bg_data);
    free(fg_data);
    free(mask_data);
    free(data);    

    return 0;
}
