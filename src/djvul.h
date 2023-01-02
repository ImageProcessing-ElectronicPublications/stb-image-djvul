/*
https://github.com/plzombie/depress/issues/2
*/

#ifndef DJVUL_H_
#define DJVUL_H_
#define DJVUL_VERSION "1.2"

#define IMAGE_CHANNELS 3

float exp256aprox(float x)
{
    x = 1.0 + x / 256.0;
    x *= x; x *= x; x *= x; x *= x;
    x *= x; x *= x; x *= x; x *= x;

    return x;
}

/*
ImageDjvulThreshold()

input:
buf - unsigned char* image (height * width * channels)
bgs = 3 // downscale BG and FG
level = 0 [auto]
wbmode = 1 [white]
doverlay = 0.5f [half]
anisotropic = 0.0f [off, regulator]
contrast = 0.0f [off, regulator]

output:
bufmask - bool* image mask (height * width)
bufbg, buffg - unsigned char* BG, FG (heightbg * widthbg * channels, heightbg = (height + bgs - 1) / bgs, widthbg = (width + bgs - 1) / bgs)

Use:
bool ok = ImageDjvulThreshold(buf, bufbg, buffg, width, height, bgs, level, wbmode, doverlay, anisotropic, contrast);
*/

int ImageDjvulThreshold(unsigned char* buf, bool* bufmask, unsigned char* bufbg, unsigned char* buffg, unsigned int width, unsigned int height, unsigned int bgs, unsigned int level, int wbmode, float doverlay, float anisotropic, float contrast)
{
    unsigned int y, x, d, i, j;
    unsigned int y0, x0, y1, x1, y0b, x0b, y1b, x1b, yb, xb;
    unsigned int widthbg, heightbg, whcp, blsz;
    unsigned long k, l, lm, n;
    unsigned char fgbase, bgbase;
    unsigned int cnth, cntw;
    int pim[IMAGE_CHANNELS], gim[IMAGE_CHANNELS], tim[IMAGE_CHANNELS];
    int fgim[IMAGE_CHANNELS], bgim[IMAGE_CHANNELS];
    int imd;
    float fgk, imx, parts, ims[IMAGE_CHANNELS];
    float fgdist, bgdist, fgdistf, bgdistf, kover, fgpart, bgpart;
    unsigned int maskbl, maskover, bgsover, fgnum, bgnum;
    unsigned int fgsum[IMAGE_CHANNELS], bgsum[IMAGE_CHANNELS];

    if (bgs > 0)
    {
        widthbg = (width + bgs - 1) / bgs;
        heightbg = (height + bgs - 1) / bgs;
    }
    else
    {
        return 0;
    }

    // level calculation
    whcp = height;
    whcp += width;
    whcp /= 2;
    blsz = 1;
    if (level == 0)
    {
        while (bgs * blsz < whcp)
        {
            level++;
            blsz <<= 1;
        }
    }
    else
    {
        for (l = 0; l < level; l++)
        {
            blsz <<= 1;
        }
    }
    if (doverlay < 0.0f)
    {
        doverlay = 0.0f;
    }
    kover = doverlay + 1.0;

    // w/b mode {1/-1}
    if (wbmode < 0)
    {
        fgbase = 255;
        bgbase = 0;
    }
    else
    {
        fgbase = 0;
        bgbase = 255;
    }
    k = 0;
    for (y = 0; y < heightbg; y++)
    {
        for (x = 0; x < widthbg; x++)
        {
            for (d = 0; d < IMAGE_CHANNELS; d++)
            {
                buffg[k] = fgbase;
                bufbg[k] = bgbase;
                k++;
            }
        }
    }

    // level blocks
    for (l = 0; l < level; l++)
    {
        cnth = (heightbg + blsz - 1) / blsz;
        cntw = (widthbg + blsz - 1) / blsz;
        maskbl = bgs * blsz;
        maskover = (kover * maskbl);
        bgsover = (kover * blsz);
        for (i = 0; i < cnth; i++)
        {
            y0 = i * maskbl;
            y1 = (((y0 + maskover) < height) ? (y0 + maskover) : height);
            y0b = i * blsz;
            y1b = (((y0b + bgsover) < heightbg) ? (y0b + bgsover) : heightbg);
            for (j = 0; j < cntw; j++)
            {
                x0 = j * maskbl;
                x1 = (((x0 + maskover) < width) ? (x0 + maskover) : width);
                x0b = j * blsz;
                x1b = (((x0b + bgsover) < widthbg) ? (x0b + bgsover) : widthbg);

                // mean region buf
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    ims[d] = 0.0f;
                }
                n = 0;
                k = 0;
                for (y = y0; y < y1; y++)
                {
                    for (x = x0; x < x1; x++)
                    {
                        k = (width * y + x) * IMAGE_CHANNELS;
                        for (d = 0; d < IMAGE_CHANNELS; d++)
                        {
                            ims[d] += (float)buf[k + d];
                        }
                        n++;
                    }
                }
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    if (n > 0)
                    {
                        ims[d] /= (float)n;
                    }
                    if (ims[d] > 255.0f) ims[d] = 255.0f;
                    gim[d] = (int)(ims[d] + 0.5f);
                }

                // mean region buffg
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    ims[d] = 0.0f;
                }
                n = 0;
                k = 0;
                for (y = y0b; y < y1b; y++)
                {
                    for (x = x0b; x < x1b; x++)
                    {
                        k = (widthbg * y + x) * IMAGE_CHANNELS;
                        for (d = 0; d < IMAGE_CHANNELS; d++)
                        {
                            ims[d] += (float)buffg[k + d];
                        }
                        n++;
                    }
                }
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    if (n > 0)
                    {
                        ims[d] /= (float)n;
                    }
                    if (ims[d] > 255.0f) ims[d] = 255.0f;
                    fgim[d] = (int)(ims[d] + 0.5f);
                }

                // mean region bufbg
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    ims[d] = 0.0f;
                }
                n = 0;
                k = 0;
                for (y = y0b; y < y1b; y++)
                {
                    for (x = x0b; x < x1b; x++)
                    {
                        k = (widthbg * y + x) * IMAGE_CHANNELS;
                        for (d = 0; d < IMAGE_CHANNELS; d++)
                        {
                            ims[d] += (float)bufbg[k + d];
                        }
                        n++;
                    }
                }
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    if (n > 0)
                    {
                        ims[d] /= (float)n;
                    }
                    if (ims[d] > 255.0f) ims[d] = 255.0f;
                    bgim[d] = (int)(ims[d] + 0.5f);
                }

                // distance buffg -> buf, bufbg -> buf
                fgdist = 0.0f;
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    imd = gim[d];
                    imd -= fgim[d];
                    if (imd < 0) imd = -imd;
                    fgdist += imd;
                }
                bgdist = 0.0f;
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    imd = gim[d];
                    imd -= bgim[d];
                    if (imd < 0) imd = -imd;
                    bgdist += imd;
                }

                // anisotropic regulator
                fgk = (fgdist + bgdist);
                if (fgk > 0)
                {
                    fgk = (bgdist - fgdist) / fgk;
                    fgk *= anisotropic;
                    fgk = exp256aprox(fgk);
                }
                else
                {
                    fgk = 1.0;
                }
                for (d = 0; d < 3; d++)
                {
                    fgsum[d] = 0;
                    bgsum[d] = 0;
                }
                fgnum = 0;
                bgnum = 0;

                // separate FG and BG
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    fgsum[d] = 0;
                    bgsum[d] = 0;
                }
                fgnum = 0;
                bgnum = 0;
                for (y = y0; y < y1; y++)
                {
                    for (x = x0; x < x1; x++)
                    {
                        k = (width * y + x) * IMAGE_CHANNELS;
                        for (d = 0; d < IMAGE_CHANNELS; d++)
                        {
                            pim[d] = (int)buf[k + d];
                            tim[d] = pim[d] +  contrast * (pim[d] - gim[d]);
                        }

                        fgdistf = 0.0f;
                        for (d = 0; d < IMAGE_CHANNELS; d++)
                        {
                            imd = tim[d];
                            imd -= fgim[d];
                            if (imd < 0) imd = -imd;
                            fgdistf += imd;
                        }
                        bgdistf = 0.0f;
                        for (d = 0; d < IMAGE_CHANNELS; d++)
                        {
                            imd = tim[d];
                            imd -= bgim[d];
                            if (imd < 0) imd = -imd;
                            bgdistf += imd;
                        }

                        if (fgdistf * fgk < bgdistf)
                        {
                            for (d = 0; d < IMAGE_CHANNELS; d++)
                            {
                                fgsum[d] += pim[d];
                            }
                            fgnum++;
                        }
                        else
                        {
                            for (d = 0; d < IMAGE_CHANNELS; d++)
                            {
                                bgsum[d] += pim[d];
                            }
                            bgnum++;
                        }
                    }
                }
                if (fgnum > 0)
                {
                    for (d = 0; d < IMAGE_CHANNELS; d++)
                    {
                        fgsum[d] /= (float)fgnum;
                        fgim[d] = (int)(fgsum[d] + 0.5f);
                    }
                }
                if (bgnum > 0)
                {
                    for (d = 0; d < IMAGE_CHANNELS; d++)
                    {
                        bgsum[d] /= (float)bgnum;
                        bgim[d] = (int)(bgsum[d] + 0.5f);
                    }
                }

                fgpart = 1.0f;
                bgpart = 1.0f;
                if ((fgdist + bgdist) > 0.0f)
                {
                    fgpart += (fgdist + fgdist) / (fgdist + bgdist);
                    bgpart += (bgdist + bgdist) / (fgdist + bgdist);
                }

                // average old and new FG
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    ims[d] = 0.0f;
                }
                n = 0;
                parts = 1.0f /((float)fgpart + 1.0f);
                for (y = y0b; y < y1b; y++)
                {
                    for (x = x0b; x < x1b; x++)
                    {
                        k = (widthbg * y + x) * IMAGE_CHANNELS;
                        for (d = 0; d < IMAGE_CHANNELS; d++)
                        {
                            imx = (float)buffg[k + d];
                            imx *= fgpart;
                            imx += (float)fgim[d];
                            imx *= parts;
                            imx += 0.5f;
                            if (imx > 255.0f) imx = 255.0f;
                            buffg[k + d] = (unsigned char)(imx + 0.5f);
                            ims[d] += imx;
                        }
                    }
                }

                // average old and new BG
                for (d = 0; d < IMAGE_CHANNELS; d++)
                {
                    ims[d] = 0.0f;
                }
                parts = 1.0f /((float)bgpart + 1.0f);
                for (y = y0b; y < y1b; y++)
                {
                    for (x = x0b; x < x1b; x++)
                    {
                        k = (widthbg * y + x) * IMAGE_CHANNELS;
                        for (d = 0; d < IMAGE_CHANNELS; d++)
                        {
                            imx = (float)bufbg[k + d];
                            imx *= bgpart;
                            imx += (float)bgim[d];
                            imx *= parts;
                            imx += 0.5f;
                            if (imx > 255.0f) imx = 255.0f;
                            bufbg[k + d] = (unsigned char)(imx + 0.5f);
                            ims[d] += imx;
                        }
                    }
                }
            }
        }
        blsz >>= 1;
    }

    // threshold mask
    l = 0;
    lm = 0;
    for (y = 0; y < height; y++)
    {
        yb = y / bgs;
        for (x = 0; x < width; x++)
        {
            xb = x / bgs;
            k = (widthbg * yb + xb) * IMAGE_CHANNELS;
            for (d = 0; d < IMAGE_CHANNELS; d++)
            {
                pim[d] = (int)buf[l];
                fgim[d] = (int)buffg[k + d];
                bgim[d] = (int)bufbg[k + d];
                l++;
            }

            // distance buffg -> buf, bufbg -> buf
            fgdist = 0.0f;
            for (d = 0; d < IMAGE_CHANNELS; d++)
            {
                imd = pim[d];
                imd -= fgim[d];
                if (imd < 0) imd = -imd;
                fgdist += (float)imd;
            }
            bgdist = 0.0f;
            for (d = 0; d < IMAGE_CHANNELS; d++)
            {
                imd = pim[d];
                imd -= bgim[d];
                if (imd < 0) imd = -imd;
                bgdist += (float)imd;
            }
            bufmask[lm] = (fgdist < bgdist);
            lm++;
        }
    }

    return level;
}

#endif /* DJVUL_H_ */
