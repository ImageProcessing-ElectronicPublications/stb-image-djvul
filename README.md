# stb-image-djvul

DjVu Layered - image bundle on the mask + foreground + background using "Multi-scale binarization".

Degradation of images on layers mask, foreground and background by cluster analysis with a gradual decrease in block size.

In addition to DjVuL, several more threshold methods are supported: BiMod, Sauvola, Blur threshold.

## build

### load submodules

submodules:

- [stb](https://github.com/nothings/stb.git) -> [src](src)

```shell
$ git submodule init
$ git submodule update
```

### install dependencies

build dependencies:

- build-essential
- cmake

```shell
$ sudo apt-get install build-essential cmake
```

### compilation
```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```
## use

The first parameter specifies the path to the image. The second parameter is the resulting BW mask. [optional: The third parameter is the background. The fourth parameter is foreground.]

```shell
./stbdjvul [options] image_in bw_mask_out.png [bg_out.png] [fg_out.png] [bgmask_out.png] [fgmask_out.png]
```

## structure

- `dependencies.c` - API [stb](https://github.com/nothings/stb.git)
- `djvul.h` - DjVuL algoritm
- `stb/` - [stb](https://github.com/nothings/stb.git)
- `stbdjvul.c` - CLI program.

## DjVu Layered

![lena](images/lena.png)  

```shell
./stbdjvul lena.png lena.mask.png lena.bg.png lena.fg.png
Load: lena.png
image: 256x256:3
BG: 86x86:3
FG: 43x43:3
DjVuL... 7 level
Save png: lena.mask.png, lena.bg.png, lena.fg.png.
```

![Mask](images/lena.mask.png) ![Bg](images/lena.bg.png) ![Fg](images/lena.fg.png)

```shell
./stbdjvul -c 1 lena.png lena.mask.c1.png lena.bg.c1.png lena.fg.c1.png 
Load: lena.png
image: 256x256:3
BG: 86x86:3
FG: 43x43:3
DjVuL... 7 level
Save png: lena.mask.c1.png, lena.bg.c1.png, lena.fg.c1.png.
```

![Mask](images/lena.mask.c1.png) ![Bg](images/lena.bg.c1.png) ![Fg](images/lena.fg.c1.png)

### DjVu Layered ground

This utility includes a mode for splitting an image into a BG and a FG based on an existing mask (ground). The mask can be corrected.

```shell
./stbdjvul -m 1 -r lena.png lena.mask.png lena.bg.g.png lena.fg.g.png 
Load: lena.png
image: 256x256:3
Load: lena.mask.png
mask: 256x256:3
BG: 86x86:3
FG: 43x43:3
DjVuL ground... 7 level
Save png: lena.mask.png, lena.bg.g.png, lena.fg.g.png.
```

![Mask](images/lena.mask.png) -rewrite-> ![Mask](images/lena.mask.g.png) ![Bg](images/lena.bg.g.png) ![Fg](images/lena.fg.g.png)

### DjVu Layered reconstruct

This utility includes a mode for reconstruct an image on an existing mask, BG and FG (reconstruct).

```shell
./stbdjvul -m 2 lena.r.png lena.mask.png lena.bg.png lena.fg.png 
Load: lena.mask.png
mask: 256x256:3
Load: lena.bg.png
BG: 86x86:3
Load: lena.fg.png
FG: 43x43:3
DjVuL reconstruct... 2 level
Save png: lena.r.png.
```

![Rec](images/lena.r.png)

---

See [demo of stbDjVuL](https://github.com/ImageProcessing-ElectronicPublications/stb-image-djvul-demo).

---

## DjVuL description.

The [base of the algorithm](https://sourceforge.net/p/imthreshold/wiki/DjVuL/?version=3) was obtained in 2016 by studying the works [monday2000](http://djvu-soft.narod.ru/) and adapting them to Linux.
The prerequisite was the [BookScanLib](http://djvu-soft.narod.ru/bookscanlib/) project  and the algorithm [DjVu Thresholding Binarization](http://djvu-soft.narod.ru/bookscanlib/034.htm).
This algorithm embodied good ideas, but had a recursive structure, was a "function with discontinuities" and had a hard color limit.
The result of this algorithm, due to the indicated shortcomings and the absence of regulators, was doubtful.
After careful study, all the foundations of the specified algorithm were rejected.
The new algorithm is based on levels instead of recursion, a smooth weight function is used instead of a "discontinuous" one, no color restriction, BG/FG selection controls are enabled.
The new algorithm allowed not only to obtain a much more adequate result, but also gave derivative functions: image division into BG/FG according to the existing mask.

## Links

* [djvulibre](http://djvu.sourceforge.net/)
* [mfbdjvu](https://github.com/ImageProcessing-ElectronicPublications/mfbdjvu)
* [simpledjvu](https://github.com/mihaild/simpledjvu)
* [depress](https://github.com/plzombie/depress)
* [tesseract](https://github.com/tesseract-ocr/tesseract)
* [hocr-tools](https://github.com/ocropus/hocr-tools)
* [imthreshold](https://github.com/ImageProcessing-ElectronicPublications/imthreshold)
* [aithreshold](https://github.com/ImageProcessing-ElectronicPublications/aithreshold)
