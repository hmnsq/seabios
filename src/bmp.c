/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#समावेश "malloc.h" // malloc_tmphigh
#समावेश "string.h" // memcpy
#समावेश "util.h" // struct bmp_decdata

काष्ठा bmp_decdata {
    काष्ठा tagRGBQUAD *quadp;
    अचिन्हित अक्षर *datap;
    पूर्णांक width;
    पूर्णांक height;
    पूर्णांक bpp;
};

#घोषणा bmp_load4byte(addr) (*(u32 *)(addr))
#घोषणा bmp_load2byte(addr) (*(u16 *)(addr))

प्रकार काष्ठा tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_आकार[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

प्रकार काष्ठा tagBITMAPINFOHEADER {
    u8 bi_आकार[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_गिनती[2];
    u8 biCompression[4];
    u8 bi_आकार_Image[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

प्रकार काष्ठा tagRGBQUAD {
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
अटल व्योम raw_तथ्य_format_adjust(u8 *src, u8 *dest, पूर्णांक width,
		पूर्णांक height, पूर्णांक bytes_per_पंक्ति_src, पूर्णांक bytes_per_पंक्ति_dest)
{
    पूर्णांक i;
    क्रम (i = 0 ; i < height ; i++) {
        स_नकल(dest + i * bytes_per_पंक्ति_dest,
           src + (height - 1 - i) * bytes_per_पंक्ति_src, bytes_per_पंक्ति_src);
    }
}

/* allocate decdata struct */
काष्ठा bmp_decdata *bmp_alloc(व्योम)
{
    काष्ठा bmp_decdata *bmp = malloc_tmphigh(माप(*bmp));
    वापस bmp;
}

/* extract information from bmp file data */
पूर्णांक bmp_decode(काष्ठा bmp_decdata *bmp, अचिन्हित अक्षर *तथ्य, पूर्णांक तथ्य_आकार)
{
    अगर (तथ्य_आकार < 54)
        वापस 1;

    u16 bmp_filehead = bmp_load2byte(तथ्य + 0);
    अगर (bmp_filehead != 0x4d42)
        वापस 2;
    u32 bmp_recordsize = bmp_load4byte(तथ्य + 2);
    अगर (bmp_recordsize != तथ्य_आकार)
        वापस 3;
    u32 bmp_dataoffset = bmp_load4byte(तथ्य + 10);
    bmp->datap = (अचिन्हित अक्षर *)तथ्य + bmp_dataoffset;
    bmp->width = bmp_load4byte(तथ्य + 18);
    bmp->height = bmp_load4byte(तथ्य + 22);
    bmp->bpp = bmp_load2byte(तथ्य + 28);
    वापस 0;
}

/* get bmp properties */
व्योम bmp_get_info(काष्ठा bmp_decdata *bmp, पूर्णांक *width, पूर्णांक *height, पूर्णांक *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
पूर्णांक bmp_show(काष्ठा bmp_decdata *bmp, अचिन्हित अक्षर *pic, पूर्णांक width,
             पूर्णांक height, पूर्णांक depth, पूर्णांक bytes_per_पंक्ति_dest)
{
    अगर (bmp->datap == pic)
        वापस 0;
    अगर ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_तथ्य_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_पंक्ति_dest);
        वापस 0;
    }
    वापस 1;
}
