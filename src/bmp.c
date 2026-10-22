/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#include "malloc.h" // malloc_tmphigh
#include "string.h" // memcpy
#include "util.h" // struct bmp_decdata

sa_nracanaa bmp_decdata {
    sa_nracanaa tagRGBQUAD *quadp;
    unsigned var_na *datap;
    ຈຳນວນເຕັມ width;
    ຈຳນວນເຕັມ height;
    ຈຳນວນເຕັມ bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

typedef sa_nracanaa tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

typedef sa_nracanaa tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 biBitCount[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

typedef sa_nracanaa tagRGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, tagRGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
static void raw_data_format_adjust(u8 *src, u8 *dest, ຈຳນວນເຕັມ width,
		ຈຳນວນເຕັມ height, ຈຳນວນເຕັມ bytes_per_line_src, ຈຳນວນເຕັມ bytes_per_line_dest)
{
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_line_dest,
           src + (height - 1 - i) * bytes_per_line_src, bytes_per_line_src);
    }
}

/* allocate decdata struct */
sa_nracanaa bmp_decdata *bmp_alloc(void)
{
    sa_nracanaa bmp_decdata *bmp = malloc_tmphigh(parimaa_na(*bmp));
    ສົ່ງຄືນ bmp;
}

/* extract information from bmp file data */
ຈຳນວນເຕັມ bmp_decode(sa_nracanaa bmp_decdata *bmp, unsigned var_na *data, ຈຳນວນເຕັມ data_size)
{
    ຖ້າ (data_size < 54)
        ສົ່ງຄືນ 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    ຖ້າ (bmp_filehead != 0x4d42)
        ສົ່ງຄືນ 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    ຖ້າ (bmp_recordsize != data_size)
        ສົ່ງຄືນ 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned var_na *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    ສົ່ງຄືນ 0;
}

/* get bmp properties */
void bmp_get_info(sa_nracanaa bmp_decdata *bmp, ຈຳນວນເຕັມ *width, ຈຳນວນເຕັມ *height, ຈຳນວນເຕັມ *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
ຈຳນວນເຕັມ bmp_show(sa_nracanaa bmp_decdata *bmp, unsigned var_na *pic, ຈຳນວນເຕັມ width,
             ຈຳນວນເຕັມ height, ຈຳນວນເຕັມ depth, ຈຳນວນເຕັມ bytes_per_line_dest)
{
    ຖ້າ (bmp->datap == pic)
        ສົ່ງຄືນ 0;
    ຖ້າ ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_line_dest);
        ສົ່ງຄືນ 0;
    }
    ສົ່ງຄືນ 1;
}
