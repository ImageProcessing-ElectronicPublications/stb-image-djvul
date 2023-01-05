# stb-image-djvul

DjVu Layered - image bundle on the mask + foreground + background using "Multi-scale binarization".

Degradation of images on layers mask, foreground and background by cluster analysis with a gradual decrease in block size.

## build

### load submodules

submodules:
- [stb](https://github.com/nothings/stb.git) -> [src/stb](src/stb)

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
./stbdjvul [options] image_in bw_mask_out.png [bg_out.png] [fg_out.png]
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
image: 512x512:3
BG,FG: 171x171:3
DjVuL... 8 level
Save png: lena.m.png, lena.bg.png, lena.fg.png
```

![Mask](images/lena.mask.png) ![Bg](images/lena.bg.png) ![Fg](images/lena.fg.png)

```shell
./stbdjvul -c 1 lena.png lena.mask.c1.png lena.bg.c1.png lena.fg.c1.png 
Load: lena.png
image: 512x512:3
BG,FG: 171x171:3
DjVuL... 8 level
Save png: lena.mask.c1.png, lena.bg.c1.png, lena.fg.c1.png.
```

![Mask](images/lena.mask.c1.png) ![Bg](images/lena.bg.c1.png) ![Fg](images/lena.fg.c1.png)

### DjVU Layered Ground

This utility splits an image into BG and FG according to an existing mask. It is possible to correct the mask.

```shell
stbdjvulground -r lena.png lena.mask.png lena.bg.g.png lena.fg.g.png 
Load: lena.png
image: 512x512:3
Load: lena.mask.png
mask: 512x512:3
BG,FG: 171x171:3
DjVuL ground... 8 level
Save png: lena.mask.png, lena.bg.g.png, lena.fg.g.png.
```

![Mask](images/lena.mask.png) -rewrite-> ![Mask](images/lena.mask.g.png) ![Bg](images/lena.bg.g.png) ![Fg](images/lena.fg.g.png)
