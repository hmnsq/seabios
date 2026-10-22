/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_tmphigh
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "util.h" // struct bmp_decdata

ସଂରଚନା bmp_decdata {
    ସଂରଚନା tagRGBQUAD *quadp;
    unsigned ଅକ୍ଷର *datap;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା height;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_ଗଣନା[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା tagRGBQUAD {
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
ସ୍ଥିର ଶୂନ୍ୟ raw_data_format_adjust(u8 *src, u8 *dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes_per_ଧାଡ଼ି_src, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes_per_ଧାଡ଼ି_dest)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_ଧାଡ଼ି_dest,
           src + (height - 1 - i) * bytes_per_ଧାଡ଼ି_src, bytes_per_ଧାଡ଼ି_src);
    }
}

/* allocate decdata struct */
ସଂରଚନା bmp_decdata *bmp_alloc(ଶୂନ୍ୟ)
{
    ସଂରଚନା bmp_decdata *bmp = malloc_tmphigh(ଆକାର(*bmp));
    ଫେରନ୍ତୁ bmp;
}

/* extract information from bmp file data */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bmp_decode(ସଂରଚନା bmp_decdata *bmp, unsigned ଅକ୍ଷର *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା data_size)
{
    ଯଦି (data_size < 54)
        ଫେରନ୍ତୁ 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    ଯଦି (bmp_filehead != 0x4d42)
        ଫେରନ୍ତୁ 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    ଯଦି (bmp_recordsize != data_size)
        ଫେରନ୍ତୁ 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned ଅକ୍ଷର *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    ଫେରନ୍ତୁ 0;
}

/* get bmp properties */
ଶୂନ୍ୟ bmp_get_info(ସଂରଚନା bmp_decdata *bmp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *height, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା bmp_show(ସଂରଚନା bmp_decdata *bmp, unsigned ଅକ୍ଷର *pic, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width,
             ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes_per_ଧାଡ଼ି_dest)
{
    ଯଦି (bmp->datap == pic)
        ଫେରନ୍ତୁ 0;
    ଯଦି ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_ଧାଡ଼ି_dest);
        ଫେରନ୍ତୁ 0;
    }
    ଫେରନ୍ତୁ 1;
}
