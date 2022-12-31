# stb-image-djvul

DjVu Layered - image bundle on the mask + foreground + background using "Multi-scale binarization".

Degradation of images on layers mask, foreground and background by cluster analysis with a gradual decrease in block size.

## build

### load submodules

submodules:
- [stb](https://github.com/nothings/stb.git) -> [src/stb](src/stb)

```shell
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

The first parameter specifies the path to the image. The second parameter is the resulting BW mask. The third parameter is foreground. The fourth parameter is the background.
```shell
./stbdjvul $IMAGE_PATH $BWMASK_out.png $FG_out.png $BG_out.png
```

## structure

- `djvul.h` - DjVuL algoritm
- `image-stb.h` - API [stb](https://github.com/nothings/stb.git)
- `stb/` - [stb](https://github.com/nothings/stb.git)
- `stbdjvul.c` - CLI program.

## DjVu Layered

```shell
./stbdjvul lena.png lena.mask.png lena.fg.png lena.bg.png 
Load: lena.png
image: 512x512:3
BG,FG: 171x171:3
DjVuL... 8 level
Save png: lena.m.png, lena.fg.png, lena.bg.png
```

![lena](images/lena.png)  
![Mask](images/lena.mask.png) ![Fg](images/lena.fg.png) ![Bg](images/lena.bg.png)
