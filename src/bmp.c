/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#அந்தர்பாவ "malloc.h" // malloc_tmphigh
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "util.h" // struct bmp_decdata

கட்டமைப்பு bmp_decdata {
    கட்டமைப்பு tagRGBQUAD *quadp;
    unsigned எழுத்து *datap;
    முழு width;
    முழு height;
    முழு bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

வகைவரை கட்டமைப்பு tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

வகைவரை கட்டமைப்பு tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_மொத்தம்[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

வகைவரை கட்டமைப்பு tagRGBQUAD {
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
நிலையான வெறுமை raw_data_format_adjust(u8 *src, u8 *dest, முழு width,
		முழு height, முழு bytes_per_வரி_src, முழு bytes_per_வரி_dest)
{
    முழு i;
    க்கு (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_வரி_dest,
           src + (height - 1 - i) * bytes_per_வரி_src, bytes_per_வரி_src);
    }
}

/* allocate decdata struct */
கட்டமைப்பு bmp_decdata *bmp_alloc(வெறுமை)
{
    கட்டமைப்பு bmp_decdata *bmp = malloc_tmphigh(அளவு(*bmp));
    திரும்பு bmp;
}

/* extract information from bmp file data */
முழு bmp_decode(கட்டமைப்பு bmp_decdata *bmp, unsigned எழுத்து *data, முழு data_size)
{
    எனில் (data_size < 54)
        திரும்பு 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    எனில் (bmp_filehead != 0x4d42)
        திரும்பு 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    எனில் (bmp_recordsize != data_size)
        திரும்பு 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned எழுத்து *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    திரும்பு 0;
}

/* get bmp properties */
வெறுமை bmp_get_info(கட்டமைப்பு bmp_decdata *bmp, முழு *width, முழு *height, முழு *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
முழு bmp_show(கட்டமைப்பு bmp_decdata *bmp, unsigned எழுத்து *pic, முழு width,
             முழு height, முழு depth, முழு bytes_per_வரி_dest)
{
    எனில் (bmp->datap == pic)
        திரும்பு 0;
    எனில் ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_வரி_dest);
        திரும்பு 0;
    }
    திரும்பு 1;
}
