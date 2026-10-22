/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#شامل "malloc.h" // malloc_tmphigh
#شامل "string.h" // memcpy
#شامل "util.h" // struct bmp_decdata

ساختار bmp_decdata {
    ساختار tagRGBQUAD *quadp;
    unsigned نویسه *datap;
    صحیح width;
    صحیح height;
    صحیح bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

تعریفنوع ساختار tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

تعریفنوع ساختار tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_شمار[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

تعریفنوع ساختار tagRGBQUAD {
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
ایستا تهی raw_data_format_adjust(u8 *src, u8 *dest, صحیح width,
		صحیح height, صحیح bytes_per_سطر_src, صحیح bytes_per_سطر_dest)
{
    صحیح i;
    برای (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_سطر_dest,
           src + (height - 1 - i) * bytes_per_سطر_src, bytes_per_سطر_src);
    }
}

/* allocate decdata struct */
ساختار bmp_decdata *bmp_alloc(تهی)
{
    ساختار bmp_decdata *bmp = malloc_tmphigh(اندازه(*bmp));
    بازگشت bmp;
}

/* extract information from bmp file data */
صحیح bmp_decode(ساختار bmp_decdata *bmp, unsigned نویسه *data, صحیح data_size)
{
    اگر (data_size < 54)
        بازگشت 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    اگر (bmp_filehead != 0x4d42)
        بازگشت 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    اگر (bmp_recordsize != data_size)
        بازگشت 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned نویسه *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    بازگشت 0;
}

/* get bmp properties */
تهی bmp_get_info(ساختار bmp_decdata *bmp, صحیح *width, صحیح *height, صحیح *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
صحیح bmp_show(ساختار bmp_decdata *bmp, unsigned نویسه *pic, صحیح width,
             صحیح height, صحیح depth, صحیح bytes_per_سطر_dest)
{
    اگر (bmp->datap == pic)
        بازگشت 0;
    اگر ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_سطر_dest);
        بازگشت 0;
    }
    بازگشت 1;
}
