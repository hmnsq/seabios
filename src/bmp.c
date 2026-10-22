/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#अन्तर्भाव "malloc.h" // malloc_tmphigh
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "util.h" // struct bmp_decdata

संरचना bmp_decdata {
    संरचना tagRGBQUAD *quadp;
    unsigned वर्ण *datap;
    पूर्णाङ्क width;
    पूर्णाङ्क height;
    पूर्णाङ्क bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

प्रकारनिर्देश संरचना tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

प्रकारनिर्देश संरचना tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_गणनम्[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

प्रकारनिर्देश संरचना tagRGBQUAD {
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
स्थिर शून्य raw_data_format_adjust(u8 *src, u8 *dest, पूर्णाङ्क width,
		पूर्णाङ्क height, पूर्णाङ्क bytes_per_पङ्क्तिः_src, पूर्णाङ्क bytes_per_पङ्क्तिः_dest)
{
    पूर्णाङ्क i;
    प्रति (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_पङ्क्तिः_dest,
           src + (height - 1 - i) * bytes_per_पङ्क्तिः_src, bytes_per_पङ्क्तिः_src);
    }
}

/* allocate decdata struct */
संरचना bmp_decdata *bmp_alloc(शून्य)
{
    संरचना bmp_decdata *bmp = malloc_tmphigh(परिमाण(*bmp));
    प्रत्यागम bmp;
}

/* extract information from bmp file data */
पूर्णाङ्क bmp_decode(संरचना bmp_decdata *bmp, unsigned वर्ण *data, पूर्णाङ्क data_size)
{
    जइ (data_size < 54)
        प्रत्यागम 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    जइ (bmp_filehead != 0x4d42)
        प्रत्यागम 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    जइ (bmp_recordsize != data_size)
        प्रत्यागम 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned वर्ण *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    प्रत्यागम 0;
}

/* get bmp properties */
शून्य bmp_get_info(संरचना bmp_decdata *bmp, पूर्णाङ्क *width, पूर्णाङ्क *height, पूर्णाङ्क *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
पूर्णाङ्क bmp_show(संरचना bmp_decdata *bmp, unsigned वर्ण *pic, पूर्णाङ्क width,
             पूर्णाङ्क height, पूर्णाङ्क depth, पूर्णाङ्क bytes_per_पङ्क्तिः_dest)
{
    जइ (bmp->datap == pic)
        प्रत्यागम 0;
    जइ ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_पङ्क्तिः_dest);
        प्रत्यागम 0;
    }
    प्रत्यागम 1;
}
