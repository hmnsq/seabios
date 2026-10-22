/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#অন্তর্ভাৱ "malloc.h" // malloc_tmphigh
#অন্তর্ভাৱ "string.h" // memcpy
#অন্তর্ভাৱ "util.h" // struct bmp_decdata

সংরচনা bmp_decdata {
    সংরচনা tagRGBQUAD *quadp;
    unsigned ৱর্ণ *datap;
    পূর্ণাঙ্ক width;
    পূর্ণাঙ্ক height;
    পূর্ণাঙ্ক bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

প্রকারনির্দেশ সংরচনা tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

প্রকারনির্দেশ সংরচনা tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_গুনতি[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

প্রকারনির্দেশ সংরচনা tagRGBQUAD {
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
স্থির শূন্য raw_data_format_adjust(u8 *src, u8 *dest, পূর্ণাঙ্ক width,
		পূর্ণাঙ্ক height, পূর্ণাঙ্ক bytes_per_সাৰি_src, পূর্ণাঙ্ক bytes_per_সাৰি_dest)
{
    পূর্ণাঙ্ক i;
    বাবে (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_সাৰি_dest,
           src + (height - 1 - i) * bytes_per_সাৰি_src, bytes_per_সাৰি_src);
    }
}

/* allocate decdata struct */
সংরচনা bmp_decdata *bmp_alloc(শূন্য)
{
    সংরচনা bmp_decdata *bmp = malloc_tmphigh(পরিমাণ(*bmp));
    ঘূৰাই bmp;
}

/* extract information from bmp file data */
পূর্ণাঙ্ক bmp_decode(সংরচনা bmp_decdata *bmp, unsigned ৱর্ণ *data, পূর্ণাঙ্ক data_size)
{
    যদি (data_size < 54)
        ঘূৰাই 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    যদি (bmp_filehead != 0x4d42)
        ঘূৰাই 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    যদি (bmp_recordsize != data_size)
        ঘূৰাই 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned ৱর্ণ *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    ঘূৰাই 0;
}

/* get bmp properties */
শূন্য bmp_get_info(সংরচনা bmp_decdata *bmp, পূর্ণাঙ্ক *width, পূর্ণাঙ্ক *height, পূর্ণাঙ্ক *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
পূর্ণাঙ্ক bmp_show(সংরচনা bmp_decdata *bmp, unsigned ৱর্ণ *pic, পূর্ণাঙ্ক width,
             পূর্ণাঙ্ক height, পূর্ণাঙ্ক depth, পূর্ণাঙ্ক bytes_per_সাৰি_dest)
{
    যদি (bmp->datap == pic)
        ঘূৰাই 0;
    যদি ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_সাৰি_dest);
        ঘূৰাই 0;
    }
    ঘূৰাই 1;
}
