/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#અન્તર્ભાવ "malloc.h" // malloc_tmphigh
#અન્તર્ભાવ "string.h" // memcpy
#અન્તર્ભાવ "util.h" // struct bmp_decdata

સંરચના bmp_decdata {
    સંરચના tagRGBQUAD *quadp;
    unsigned અક્ષર *datap;
    પૂર્ણાંક width;
    પૂર્ણાંક height;
    પૂર્ણાંક bpp;
};

#define bmp_load4byte(addr) (*(u32 *)(addr))
#define bmp_load2byte(addr) (*(u16 *)(addr))

પ્રકારવ્યાખ્યા સંરચના tagBITMAPFILEHEADER {
    u8 bfType[2];
    u8 bfSize[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bfOffBits[4];
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

પ્રકારવ્યાખ્યા સંરચના tagBITMAPINFOHEADER {
    u8 biSize[4];
    u8 biWidth[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_ગણતરી[2];
    u8 biCompression[4];
    u8 biSizeImage[4];
    u8 biXPelsPerMeter[4];
    u8 biYPelsPerMeter[4];
    u8 biClrUsed[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

પ્રકારવ્યાખ્યા સંરચના tagRGBQUAD {
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
સ્થિર શૂન્ય raw_data_format_adjust(u8 *src, u8 *dest, પૂર્ણાંક width,
		પૂર્ણાંક height, પૂર્ણાંક bytes_per_લીટી_src, પૂર્ણાંક bytes_per_લીટી_dest)
{
    પૂર્ણાંક i;
    માટે (i = 0 ; i < height ; i++) {
        memcpy(dest + i * bytes_per_લીટી_dest,
           src + (height - 1 - i) * bytes_per_લીટી_src, bytes_per_લીટી_src);
    }
}

/* allocate decdata struct */
સંરચના bmp_decdata *bmp_alloc(શૂન્ય)
{
    સંરચના bmp_decdata *bmp = malloc_tmphigh(કદ(*bmp));
    પાછા bmp;
}

/* extract information from bmp file data */
પૂર્ણાંક bmp_decode(સંરચના bmp_decdata *bmp, unsigned અક્ષર *data, પૂર્ણાંક data_size)
{
    જો (data_size < 54)
        પાછા 1;

    u16 bmp_filehead = bmp_load2byte(data + 0);
    જો (bmp_filehead != 0x4d42)
        પાછા 2;
    u32 bmp_recordsize = bmp_load4byte(data + 2);
    જો (bmp_recordsize != data_size)
        પાછા 3;
    u32 bmp_dataoffset = bmp_load4byte(data + 10);
    bmp->datap = (unsigned અક્ષર *)data + bmp_dataoffset;
    bmp->width = bmp_load4byte(data + 18);
    bmp->height = bmp_load4byte(data + 22);
    bmp->bpp = bmp_load2byte(data + 28);
    પાછા 0;
}

/* get bmp properties */
શૂન્ય bmp_get_info(સંરચના bmp_decdata *bmp, પૂર્ણાંક *width, પૂર્ણાંક *height, પૂર્ણાંક *bpp)
{
    *width = bmp->width;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
પૂર્ણાંક bmp_show(સંરચના bmp_decdata *bmp, unsigned અક્ષર *pic, પૂર્ણાંક width,
             પૂર્ણાંક height, પૂર્ણાંક depth, પૂર્ણાંક bytes_per_લીટી_dest)
{
    જો (bmp->datap == pic)
        પાછા 0;
    જો ((depth == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_data_format_adjust(bmp->datap, pic, width, height,
			(bmp->bpp/8)*width, bytes_per_લીટી_dest);
        પાછા 0;
    }
    પાછા 1;
}
