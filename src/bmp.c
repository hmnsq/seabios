/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#অন্তর্ভাব "malloc.h" // malloc_tmphigh
#অন্তর্ভাব "string.h" // memcpy
#অন্তর্ভাব "util.h" // struct bmp_decdata

কাঠামো bmp_decdata {
    কাঠামো tagRGBQUAD *quadp;
    unsigned অক্ষর *datap;
    পূর্ণসংখ্যা width;
    পূর্ণসংখ্যা height;
    পূর্ণসংখ্যা bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

প্রকারলেখ কাঠামো tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_আকার[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

প্রকারলেখ কাঠামো tagBITMAPINFOHEADER {
    u8 bi_আকার[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_গুনতি[2];
    u8 biCompression[4];
    u8 bi_আকার_Image[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

প্রকারলেখ কাঠামো tagRGBQUAD {
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
স্থির শূন্য raw_তথ্য_format_adjust(u8 *src, u8 *dest, পূর্ণসংখ্যা width,
		পূর্ণসংখ্যা height, পূর্ণসংখ্যা bytes_per_সারি_src, পূর্ণসংখ্যা bytes_per_সারি_dest)
{
    পূর্ণসংখ্যা i;
    জন্য (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_সারি_dest,
           src + (height - 1 - i) * bytes_per_সারি_src, bytes_per_সারি_src);
    }
}

/* allocate decdata struct */
কাঠামো bmp_decdata *bmp_alloc(শূন্য)
{
    কাঠামো bmp_decdata *bmp = malloc_tmphigh(আকার(*bmp));
    ফেরত bmp;
}

/* extract information from bmp file data */
পূর্ণসংখ্যা bmp_decode(কাঠামো bmp_decdata *bmp, unsigned অক্ষর *তথ্য, পূর্ণসংখ্যা তথ্য_আকার)
{
    যদি (তথ্য_আকার < 54)
        ফেরত 1;

    u16 bmp_filehead = bmp_load2byte(তথ্য + 0);
    যদি (bmp_filehead != 0x4d42)
        ফেরত 2;
    u32 bmp_recordsize = bmp_load4byte(তথ্য + 2);
    যদি (bmp_recordsize != তথ্য_আকার)
        ফেরত 3;
    u32 bmp_dataoffset = bmp_load4byte(তথ্য + 10);
    bmp->datap = (unsigned অক্ষর *)তথ্য + bmp_dataoffset;
    bmp->width = bmp_load4byte(তথ্য + 18);
    bmp->height = bmp_load4byte(তথ্য + 22);
    bmp->bpp = bmp_load2byte(তথ্য + 28);
    ফেরত 0;
}

/* get bmp properties */
শূন্য bmp_get_info(কাঠামো bmp_decdata *bmp, পূর্ণসংখ্যা *width, পূর্ণসংখ্যা *height, পূর্ণসংখ্যা *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
পূর্ণসংখ্যা bmp_show(কাঠামো bmp_decdata *bmp, unsigned অক্ষর *pic, পূর্ণসংখ্যা width,
             পূর্ণসংখ্যা height, পূর্ণসংখ্যা depth, পূর্ণসংখ্যা bytes_per_সারি_dest)
{
    যদি (bmp->datap == pic)
        ফেরত 0;
    যদি ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_তথ্য_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_সারি_dest);
        ফেরত 0;
    }
    ফেরত 1;
}
