/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#అన్తర్భావ "malloc.h" // malloc_tmphigh
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "util.h" // struct bmp_decdata

నిర్మాణం bmp_decdata {
    నిర్మాణం ట్యాగ్_RGBQUAD *quadp;
    గుర్తులేని అక్షరం *datap;
    పూర్ణాంకం వెడల్పు;
    పూర్ణాంకం ఎత్తు;
    పూర్ణాంకం bpp;
};

#నిర్వచించు bmp_load4byte(addr) (*(u32 *)(addr))
#నిర్వచించు bmp_load2byte(addr) (*(u16 *)(addr))

రకనిర్వచనం నిర్మాణం ట్యాగ్_BITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_కొలత[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bf_Off_బిట్లు[4];
} BITMAPFILEHEADER, ట్యాగ్_BITMAPFILEHEADER;

రకనిర్వచనం నిర్మాణం ట్యాగ్_BITMAPINFOHEADER {
    u8 bi_కొలత[4];
    u8 bi_వెడల్పు[4];
    u8 bi_ఎత్తు[4];
    u8 biPlanes[2];
    u8 bi_బిట్టు_లెక్క[2];
    u8 biCompression[4];
    u8 bi_కొలత_Image[4];
    u8 bi_XPels_చొప్పున_Meter[4];
    u8 bi_YPels_చొప్పున_Meter[4];
    u8 bi_Clr_వాడిన[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, ట్యాగ్_BITMAPINFOHEADER;

రకనిర్వచనం నిర్మాణం ట్యాగ్_RGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, ట్యాగ్_RGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
స్థిర శూన్య raw_దత్తాంశం_format_adjust(u8 *src, u8 *dest, పూర్ణాంకం వెడల్పు,
		పూర్ణాంకం ఎత్తు, పూర్ణాంకం బైట్లు_చొప్పున_పంక్తి_src, పూర్ణాంకం బైట్లు_చొప్పున_పంక్తి_dest)
{
    పూర్ణాంకం i;
    కొరకు (i = 0 ; i < ఎత్తు ; i++) {
        జ్ఞాపకం_నకలు(dest + i * బైట్లు_చొప్పున_పంక్తి_dest,
           src + (ఎత్తు - 1 - i) * బైట్లు_చొప్పున_పంక్తి_src, బైట్లు_చొప్పున_పంక్తి_src);
    }
}

/* allocate decdata struct */
నిర్మాణం bmp_decdata *bmp_alloc(శూన్య)
{
    నిర్మాణం bmp_decdata *bmp = malloc_tmphigh(పరిమాణం(*bmp));
    తిరిగి bmp;
}

/* extract information from bmp file data */
పూర్ణాంకం bmp_decode(నిర్మాణం bmp_decdata *bmp, గుర్తులేని అక్షరం *దత్తాంశం, పూర్ణాంకం దత్తాంశం_కొలత)
{
    ఒకవేళ (దత్తాంశం_కొలత < 54)
        తిరిగి 1;

    u16 bmp_filehead = bmp_load2byte(దత్తాంశం + 0);
    ఒకవేళ (bmp_filehead != 0x4d42)
        తిరిగి 2;
    u32 bmp_recordsize = bmp_load4byte(దత్తాంశం + 2);
    ఒకవేళ (bmp_recordsize != దత్తాంశం_కొలత)
        తిరిగి 3;
    u32 bmp_dataoffset = bmp_load4byte(దత్తాంశం + 10);
    bmp->datap = (గుర్తులేని అక్షరం *)దత్తాంశం + bmp_dataoffset;
    bmp->వెడల్పు = bmp_load4byte(దత్తాంశం + 18);
    bmp->ఎత్తు = bmp_load4byte(దత్తాంశం + 22);
    bmp->bpp = bmp_load2byte(దత్తాంశం + 28);
    తిరిగి 0;
}

/* get bmp properties */
శూన్య bmp_get_info(నిర్మాణం bmp_decdata *bmp, పూర్ణాంకం *వెడల్పు, పూర్ణాంకం *ఎత్తు, పూర్ణాంకం *bpp)
{
    *వెడల్పు = bmp->వెడల్పు;
    *ఎత్తు = bmp->ఎత్తు;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
పూర్ణాంకం bmp_show(నిర్మాణం bmp_decdata *bmp, గుర్తులేని అక్షరం *pic, పూర్ణాంకం వెడల్పు,
             పూర్ణాంకం ఎత్తు, పూర్ణాంకం లోతు, పూర్ణాంకం బైట్లు_చొప్పున_పంక్తి_dest)
{
    ఒకవేళ (bmp->datap == pic)
        తిరిగి 0;
    ఒకవేళ ((లోతు == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_దత్తాంశం_format_adjust(bmp->datap, pic, వెడల్పు, ఎత్తు,
			(bmp->bpp/8)*వెడల్పు, బైట్లు_చొప్పున_పంక్తి_dest);
        తిరిగి 0;
    }
    తిరిగి 1;
}
