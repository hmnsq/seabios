/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#ضمن "malloc.h" // malloc_tmphigh
#ضمن "string.h" // memcpy
#ضمن "util.h" // struct bmp_decdata

بنية bmp_decdata {
    بنية tagRGBQUAD *quadp;
    unsigned حرف *datap;
    صحيح width;
    صحيح height;
    صحيح bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

تعريفنوع بنية tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

تعريفنوع بنية tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_ژماردن[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

تعريفنوع بنية tagRGBQUAD {
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
ساكن فراغ raw_data_format_adjust(u8 *src, u8 *dest, صحيح width,
		صحيح height, صحيح bytes_per_هێڵ_src, صحيح bytes_per_هێڵ_dest)
{
    صحيح i;
    لكل (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_هێڵ_dest,
           src + (height - 1 - i) * bytes_per_هێڵ_src, bytes_per_هێڵ_src);
    }
}

/* allocate decdata struct */
بنية bmp_decdata *bmp_alloc(فراغ)
{
    بنية bmp_decdata *bmp = malloc_tmphigh(حجم(*bmp));
    أرجع bmp;
}

/* extract information from bmp file data */
صحيح bmp_decode(بنية bmp_decdata *bmp, unsigned حرف *data, صحيح data_size)
{
    إذا (data_size < 54)
        أرجع 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    إذا (bmp_filehead != 0x4d42)
        أرجع 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    إذا (bmp_recordsize != data_size)
        أرجع 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned حرف *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    أرجع 0;
}

/* get bmp properties */
فراغ bmp_get_info(بنية bmp_decdata *bmp, صحيح *width, صحيح *height, صحيح *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
صحيح bmp_show(بنية bmp_decdata *bmp, unsigned حرف *pic, صحيح width,
             صحيح height, صحيح depth, صحيح bytes_per_هێڵ_dest)
{
    إذا (bmp->datap == pic)
        أرجع 0;
    إذا ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_هێڵ_dest);
        أرجع 0;
    }
    أرجع 1;
}
