#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include "djvul.h"

void djvul_usage(char* prog, unsigned int bgs, unsigned int fgs, unsigned int level, int wbmode, int pmode, int tmode, float doverlay, float anisotropic, float contrast, float fbscale, float delta, float sensitivity)
{
    printf("StbDjVuL version %s.\n", DJVUL_VERSION);
    printf("usage: %s [options] image_in bw_mask_out.png [bg_out.png] [fg_out.png] [bgmask_out.png] [fgmask_out.png]\n", prog);
    printf("options:\n");
    printf("  -a N.N    factor anisortopic (regulator, default %f)\n", anisotropic);
    printf("  -b NUM    downsample FG and BG (default %d)\n", bgs);
    printf("  -c N.N    factor contrast (regulator, default %f)\n", contrast);
    printf("  -d N.N    factor delta (regulator, default %f)\n", delta);
    printf("  -e N.N    factor FG/BG scale (regulator, default %f)\n", fbscale);
    printf("  -f NUM    downsample FG (default %d)\n", fgs);
    printf("  -l NUM    level of scale blocks (default %d)\n", level);
    printf("  -m NUM    mode: 0 - threshold, 1 - ground, 2 - recontruct (default %d)\n", pmode);
    printf("  -o N.N    part of overlay blocks (default %f)\n", doverlay);
    printf("  -r        rewrite maks in ground mode\n");
    printf("  -s N.N    sensitivity Sauvola and Blur threshold (default %f)\n", sensitivity);
    printf("  -t NUM    threshold: 0 - DjVuL, 1 - BiMod, 2 - Sauvola, 3 - Blur, 4 - EdgePlus (default %d)\n", tmode);
    printf("  -w        white/black mode (default %d)\n", wbmode);
    printf("  -h        show this help message and exit\n");
}

int main(int argc, char **argv)
{
    unsigned int channels_out = DJVUL_IMAGE_CHANNELS;
    unsigned int bgs = 3;
    unsigned int fgs = 2;
    unsigned int level = 0;
    int wbmode = 1;
    float doverlay = 0.5f;
    float anisotropic = 0.0f;
    float contrast = 0.0f;
    float fbscale = 1.0f;
    float delta = 0.0f;
    float sensitivity = 0.2f;
    int pmode = 0;
    int tmode = 0;
    int remask = 0;
    int fhelp = 0;
    int opt;
    while ((opt = getopt(argc, argv, ":a:b:c:d:e:f:l:m:o:rs:t:wh")) != -1)
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
        case 'c':
            contrast = atof(optarg);
            break;
        case 'd':
            delta = atof(optarg);
            break;
        case 'e':
            fbscale = atof(optarg);
            break;
        case 'f':
            fgs = atoi(optarg);
            if (fgs < 1)
            {
                fprintf(stderr, "Bad argument\n");
                fprintf(stderr, "fgs = %d\n", fgs);
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
        case 'm':
            pmode = atoi(optarg);
            if ((pmode < 0) || (pmode > 2))
            {
                fprintf(stderr, "Bad argument\n");
                fprintf(stderr, "mode = %d\n", pmode);
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
        case 'r':
            remask = 1;
            break;
        case 's':
            sensitivity = atof(optarg);
            break;
        case 't':
            tmode = atoi(optarg);
            if ((tmode < 0) || (tmode > 4))
            {
                fprintf(stderr, "Bad argument\n");
                fprintf(stderr, "threshold = %d\n", tmode);
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
        djvul_usage(argv[0], bgs, fgs, level, wbmode, pmode, tmode, doverlay, anisotropic, contrast, fbscale, delta, sensitivity);
        return 0;
    }
    const char *src_name = argv[optind];
    const char *mask_name = argv[optind + 1];
    const char *bg_name = NULL;
    if(optind + 2 < argc) bg_name = argv[optind + 2];
    const char *fg_name = NULL;
    if(optind + 3 < argc) fg_name = argv[optind + 3];
    const char *bgmask_name = NULL;
    if(optind + 4 < argc) bgmask_name = argv[optind + 4];
    const char *fgmask_name = NULL;
    if(optind + 5 < argc) fgmask_name = argv[optind + 5];

    int height, width, channels, heightm, widthm;

    stbi_uc* img = NULL;
    unsigned char* data = NULL;
    bool *mask_data = NULL;
    unsigned char *bg_data = NULL;
    unsigned char *fg_data = NULL;
    int bg_width, bg_height, fg_width, fg_height;
    size_t ki = 0, kd = 0;

    if (pmode < 2)
    {
        printf("Load: %s\n", src_name);
        img = NULL;
        if (!(img = stbi_load(src_name, &width, &height, &channels, STBI_rgb_alpha)))
        {
            fprintf(stderr, "ERROR: not read image: %s\n", src_name);
            return 1;
        }
        printf("image: %dx%d:%d\n", width, height, channels);
        if (!(data = (unsigned char*)malloc(height * width * channels_out * sizeof(unsigned char))))
        {
            fprintf(stderr, "ERROR: not use memmory\n");
            return 1;
        }
        ki = 0;
        kd = 0;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                for (int d = 0; d < channels_out; d++)
                {
                    data[kd + d] = (unsigned char)img[ki + d];
                }
                ki += STBI_rgb_alpha;
                kd += channels_out;
            }
        }
        stbi_image_free(img);
    }

    if (pmode < 1)
    {
        if (!(mask_data = (bool*)malloc(height * width * sizeof(bool))))
        {
            fprintf(stderr, "ERROR: not memiory\n");
            return 2;
        }
    }
    else
    {
        printf("Load: %s\n", mask_name);
        img = NULL;
        if (!(img = stbi_load(mask_name, &widthm, &heightm, &channels, STBI_rgb_alpha)))
        {
            fprintf(stderr, "ERROR: not read image: %s\n", mask_name);
            return 1;
        }
        printf("mask: %dx%d:%d\n", widthm, heightm, channels);
        if (pmode < 2)
        {
            if ((width != widthm) || (height != height))
            {
                fprintf(stderr, "ERROR: sizes do not match: %s\n", mask_name);
                return 1;
            }
        }
        else
        {
            width = widthm;
            height = heightm;
            if (!(data = (unsigned char*)malloc(height * width * channels_out * sizeof(unsigned char))))
            {
                fprintf(stderr, "ERROR: not use memmory\n");
                return 1;
            }
        }
        if (!(mask_data = (bool*)malloc(height * width * sizeof(bool))))
        {
            fprintf(stderr, "ERROR: not memiory\n");
            return 2;
        }
        ki = 0;
        kd = 0;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int mt = 0;
                for (int d = 0; d < channels_out; d++)
                {
                    mt += img[ki + d];
                }
                mask_data[kd] = (mt < 383);
                ki += STBI_rgb_alpha;
                kd ++;
            }
        }
        stbi_image_free(img);
    }
    bg_width = (width + bgs - 1) / bgs;
    bg_height = (height + bgs - 1) / bgs;
    fg_width = (bg_width + fgs - 1) / fgs;
    fg_height = (bg_height + fgs - 1) / fgs;

    if (pmode < 2)
    {
        if (!(bg_data = (unsigned char*)malloc(bg_height * bg_width * channels_out * sizeof(unsigned char))))
        {
            fprintf(stderr, "ERROR: not memiory\n");
            return 2;
        }
    }
    else
    {
        if (bg_name)
        {
            printf("Load: %s\n", bg_name);
            img = NULL;
            if (!(img = stbi_load(bg_name, &bg_width, &bg_height, &channels, STBI_rgb_alpha)))
            {
                fprintf(stderr, "ERROR: not read image: %s\n", src_name);
                return 1;
            }
            if (!(bg_data = (unsigned char*)malloc(bg_height * bg_width * channels_out * sizeof(unsigned char))))
            {
                fprintf(stderr, "ERROR: not use memmory\n");
                return 1;
            }
            size_t ki = 0, kd = 0;
            for (int y = 0; y < bg_height; y++)
            {
                for (int x = 0; x < bg_width; x++)
                {
                    for (int d = 0; d < channels_out; d++)
                    {
                        bg_data[kd + d] = (unsigned char)img[ki + d];
                    }
                    ki += STBI_rgb_alpha;
                    kd += channels_out;
                }
            }
            stbi_image_free(img);
        }
    }
    printf("BG: %dx%d:%d\n", bg_width, bg_height, channels_out);

    if (pmode < 2)
    {
        if (!(fg_data = (unsigned char*)malloc(bg_height * bg_width * channels_out * sizeof(unsigned char))))
        {
            fprintf(stderr, "ERROR: not memiory\n");
            return 2;
        }
    }
    else
    {
        if (fg_name)
        {
            printf("Load: %s\n", fg_name);
            img = NULL;
            if (!(img = stbi_load(fg_name, &fg_width, &fg_height, &channels, STBI_rgb_alpha)))
            {
                fprintf(stderr, "ERROR: not read image: %s\n", src_name);
                return 1;
            }
            if (!(fg_data = (unsigned char*)malloc(fg_height * fg_width * channels_out * sizeof(unsigned char))))
            {
                fprintf(stderr, "ERROR: not use memmory\n");
                return 1;
            }
            size_t ki = 0, kd = 0;
            for (int y = 0; y < fg_height; y++)
            {
                for (int x = 0; x < fg_width; x++)
                {
                    for (int d = 0; d < channels_out; d++)
                    {
                        fg_data[kd + d] = (unsigned char)img[ki + d];
                    }
                    ki += STBI_rgb_alpha;
                    kd += channels_out;
                }
            }
            stbi_image_free(img);
        }
    }
    printf("FG: %dx%d:%d\n", fg_width, fg_height, channels_out);

    if (pmode == 1)
    {
        printf("DjVuL ground...");
        if(!(level = ImageDjvulGround(data, mask_data, bg_data, fg_data, width, height, channels_out, bgs, level, doverlay)))
        {
            fprintf(stderr, "ERROR: not complite DjVuL ground\n");
            return 3;
        }
    }
    else if (pmode == 2)
    {
        printf("DjVuL reconstruct...");
        if((level = ImageDjvuReconstruct(data, mask_data, bg_data, fg_data, width, height, channels_out, bg_width, bg_height, fg_width, fg_height)) < 0)
        {
            fprintf(stderr, "ERROR: not complite DjVuL reconstruct\n");
            return 3;
        }
    }
    else
    {
        switch(tmode)
        {
        case TBIMOD:
            printf("threshold: BiMod\n");
            break;
        case TSAUVOLA:
            printf("threshold: Sauvola\n");
            break;
        case TBLUR:
            printf("threshold: Blur\n");
            break;
        case TEDGEPLUS:
            printf("threshold: EdgePlus\n");
            break;
        default:
            printf("threshold: DjVuL\n");
            break;
        }
        printf("DjVuL...");
        if(!(level = ImageDjvulSelect(data, mask_data, bg_data, fg_data, width, height, channels_out, bgs, level, wbmode, doverlay, anisotropic, contrast, fbscale, delta, (float)(bgs * fgs), sensitivity, tmode)))
        {
            fprintf(stderr, "ERROR: not complite DjVuL\n");
            return 3;
        }
    }

    if (pmode < 2)
    {
        ki = 0;
        kd = 0;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                unsigned char bw = (mask_data[ki]) ? 0 : 255;
                for (int d = 0; d < channels_out; d++)
                {
                    data[kd] = bw;
                    kd++;
                }
                ki++;
            }
        }
        if (fgs > 1)
        {
            fgs = ImageFGdownsample(fg_data, bg_width, bg_height, channels_out, fgs);
        }
    }
    printf(" %d level\n", level);

    printf("Save png:");
    if (pmode < 2)
    {
        if ((pmode != 1) || remask)
        {
            printf(" %s", mask_name);
            if (!(stbi_write_png(mask_name, width, height, channels_out, data, width * channels_out)))
            {
                fprintf(stderr, "ERROR: not write image: %s\n", mask_name);
                return 1;
            }
        }
        else
        {
            printf(" none");
        }
        if (bg_name)
        {
            printf(", %s", bg_name);
            if (!(stbi_write_png(bg_name, bg_width, bg_height, channels_out, bg_data, bg_width * channels_out)))
            {
                fprintf(stderr, "ERROR: not write image: %s\n", bg_name);
                return 1;
            }
        }
        if (fg_name)
        {
            printf(", %s", fg_name);
            if (!(stbi_write_png(fg_name, fg_width, fg_height, channels_out, fg_data, fg_width * channels_out)))
            {
                fprintf(stderr, "ERROR: not write image: %s\n", fg_name);
                return 1;
            }
        }
        if (bgmask_name)
        {
            printf(", %s", bgmask_name);
            kd = 0;
            for (int y = 0; y < bg_height; y++)
            {
                for (int x = 0; x < bg_width; x++)
                {
                    for (int d = 0; d < channels_out; d++)
                    {
                        bg_data[kd] = 0;
                        kd++;
                    }
                }
            }
            ki = 0;
            kd = 0;
            for (int y = 0; y < height; y++)
            {
                int yb = y / bgs;
                for (int x = 0; x < width; x++)
                {
                    int xb = x / bgs;
                    if (!mask_data[ki])
                    {
                        kd = (yb * bg_width + xb) * channels_out;
                        for (int d = 0; d < channels_out; d++)
                        {
                            bg_data[kd + d] = 255;
                        }
                    }
                    ki++;
                }
            }
            if (!(stbi_write_png(bgmask_name, bg_width, bg_height, channels_out, bg_data, bg_width * channels_out)))
            {
                fprintf(stderr, "ERROR: not write image: %s\n", bgmask_name);
                return 1;
            }
        }
        if (fgmask_name)
        {
            printf(", %s", fgmask_name);
            kd = 0;
            for (int y = 0; y < fg_height; y++)
            {
                for (int x = 0; x < fg_width; x++)
                {
                    for (int d = 0; d < channels_out; d++)
                    {
                        fg_data[kd] = 0;
                        kd++;
                    }
                }
            }
            ki = 0;
            kd = 0;
            for (int y = 0; y < height; y++)
            {
                int yf = y / bgs / fgs;
                for (int x = 0; x < width; x++)
                {
                    int xf = x / bgs / fgs;
                    if (mask_data[ki])
                    {
                        kd = (yf * fg_width + xf) * channels_out;
                        for (int d = 0; d < channels_out; d++)
                        {
                            fg_data[kd + d] = 255;
                        }
                    }
                    ki++;
                }
            }
            printf(", %s", fgmask_name);
            if (!(stbi_write_png(fgmask_name, fg_width, fg_height, channels_out, fg_data, fg_width * channels_out)))
            {
                fprintf(stderr, "ERROR: not write image: %s\n", fgmask_name);
                return 1;
            }
        }
    }
    else
    {
        printf(" %s", src_name);
        if (!(stbi_write_png(src_name, width, height, channels_out, data, width * channels_out)))
        {
            fprintf(stderr, "ERROR: not write image: %s\n", mask_name);
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
