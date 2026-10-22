/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#ضمن "malloc.h" // malloc_tmphigh
#ضمن "string.h" // memcpy
#ضمن "util.h" // struct bmp_decdata

بنية bmp_decdata {
    بنية برچسب_RGBQUAD *quadp;
    بیعلامت حرف *datap;
    صحيح عرض;
    صحيح ارتفاع;
    صحيح bpp;
};

#تعریف_کن bmp_load4byte(addr) (*(u32 *)(addr))
#تعریف_کن bmp_load2byte(addr) (*(u16 *)(addr))

تعريفنوع بنية برچسب_BITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_اندازه[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bf_Off_بیتها[4];
} BITMAPFILEHEADER, برچسب_BITMAPFILEHEADER;

تعريفنوع بنية برچسب_BITMAPINFOHEADER {
    u8 bi_اندازه[4];
    u8 bi_عرض[4];
    u8 bi_ارتفاع[4];
    u8 biPlanes[2];
    u8 bi_بیت_شمار[2];
    u8 biCompression[4];
    u8 bi_اندازه_Image[4];
    u8 bi_XPels_در_هر_Meter[4];
    u8 bi_YPels_در_هر_Meter[4];
    u8 bi_Clr_استفاده_شده[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, برچسب_BITMAPINFOHEADER;

تعريفنوع بنية برچسب_RGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, برچسب_RGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
ساكن فراغ raw_داده_format_adjust(u8 *src, u8 *dest, صحيح عرض,
		صحيح ارتفاع, صحيح بایتها_در_هر_سطر_src, صحيح بایتها_در_هر_سطر_dest)
{
    صحيح i;
    لكل (i = 0 ; i < ارتفاع ; i++) {
        کپی_حافظه(dest + i * بایتها_در_هر_سطر_dest,
           src + (ارتفاع - 1 - i) * بایتها_در_هر_سطر_src, بایتها_در_هر_سطر_src);
    }
}

/* allocate decdata struct */
بنية bmp_decdata *bmp_alloc(فراغ)
{
    بنية bmp_decdata *bmp = malloc_tmphigh(حجم(*bmp));
    أرجع bmp;
}

/* extract information from bmp file data */
صحيح bmp_decode(بنية bmp_decdata *bmp, بیعلامت حرف *داده, صحيح داده_اندازه)
{
    إذا (داده_اندازه < 54)
        أرجع 1;

    u16 bmp_filehead = bmp_load2byte(داده + 0);
    إذا (bmp_filehead != 0x4d42)
        أرجع 2;
    u32 bmp_recordsize = bmp_load4byte(داده + 2);
    إذا (bmp_recordsize != داده_اندازه)
        أرجع 3;
    u32 bmp_dataoffset = bmp_load4byte(داده + 10);
    bmp->datap = (بیعلامت حرف *)داده + bmp_dataoffset;
    bmp->عرض = bmp_load4byte(داده + 18);
    bmp->ارتفاع = bmp_load4byte(داده + 22);
    bmp->bpp = bmp_load2byte(داده + 28);
    أرجع 0;
}

/* get bmp properties */
فراغ bmp_get_info(بنية bmp_decdata *bmp, صحيح *عرض, صحيح *ارتفاع, صحيح *bpp)
{
    *عرض = bmp->عرض;
    *ارتفاع = bmp->ارتفاع;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
صحيح bmp_show(بنية bmp_decdata *bmp, بیعلامت حرف *pic, صحيح عرض,
             صحيح ارتفاع, صحيح عمق, صحيح بایتها_در_هر_سطر_dest)
{
    إذا (bmp->datap == pic)
        أرجع 0;
    إذا ((عمق == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_داده_format_adjust(bmp->datap, pic, عرض, ارتفاع,
			(bmp->bpp/8)*عرض, بایتها_در_هر_سطر_dest);
        أرجع 0;
    }
    أرجع 1;
}
