/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#അന്തര്ഭാവ "malloc.h" // malloc_tmphigh
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "util.h" // struct bmp_decdata

ഘടന bmp_decdata {
    ഘടന tagRGBQUAD *quadp;
    unsigned അക്ഷരം *datap;
    പൂർണ്ണസംഖ്യ width;
    പൂർണ്ണസംഖ്യ height;
    പൂർണ്ണസംഖ്യ bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

തരനിർവചനം ഘടന tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

തരനിർവചനം ഘടന tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_എണ്ണം[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

തരനിർവചനം ഘടന tagRGBQUAD {
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
സ്ഥിതം ശൂന്യം raw_data_format_adjust(u8 *src, u8 *dest, പൂർണ്ണസംഖ്യ width,
		പൂർണ്ണസംഖ്യ height, പൂർണ്ണസംഖ്യ bytes_per_വരി_src, പൂർണ്ണസംഖ്യ bytes_per_വരി_dest)
{
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_വരി_dest,
           src + (height - 1 - i) * bytes_per_വരി_src, bytes_per_വരി_src);
    }
}

/* allocate decdata struct */
ഘടന bmp_decdata *bmp_alloc(ശൂന്യം)
{
    ഘടന bmp_decdata *bmp = malloc_tmphigh(വലുപ്പം(*bmp));
    തിരികെ bmp;
}

/* extract information from bmp file data */
പൂർണ്ണസംഖ്യ bmp_decode(ഘടന bmp_decdata *bmp, unsigned അക്ഷരം *data, പൂർണ്ണസംഖ്യ data_size)
{
    എങ്കിൽ (data_size < 54)
        തിരികെ 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    എങ്കിൽ (bmp_filehead != 0x4d42)
        തിരികെ 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    എങ്കിൽ (bmp_recordsize != data_size)
        തിരികെ 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned അക്ഷരം *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    തിരികെ 0;
}

/* get bmp properties */
ശൂന്യം bmp_get_info(ഘടന bmp_decdata *bmp, പൂർണ്ണസംഖ്യ *width, പൂർണ്ണസംഖ്യ *height, പൂർണ്ണസംഖ്യ *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
പൂർണ്ണസംഖ്യ bmp_show(ഘടന bmp_decdata *bmp, unsigned അക്ഷരം *pic, പൂർണ്ണസംഖ്യ width,
             പൂർണ്ണസംഖ്യ height, പൂർണ്ണസംഖ്യ depth, പൂർണ്ണസംഖ്യ bytes_per_വരി_dest)
{
    എങ്കിൽ (bmp->datap == pic)
        തിരികെ 0;
    എങ്കിൽ ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_വരി_dest);
        തിരികെ 0;
    }
    തിരികെ 1;
}
