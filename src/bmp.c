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
    بنية ٽيگ_RGBQUAD *quadp;
    بنانشان حرف *datap;
    صحيح ويڪر;
    صحيح اوچائي;
    صحيح bpp;
};

#وصف_ڪريو bmp_load4byte(addr) (*(u32 *)(addr))
#وصف_ڪريو bmp_load2byte(addr) (*(u16 *)(addr))

تعريفنوع بنية ٽيگ_BITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_سائيز[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bf_Off_بٽ[4];
} BITMAPFILEHEADER, ٽيگ_BITMAPFILEHEADER;

تعريفنوع بنية ٽيگ_BITMAPINFOHEADER {
    u8 bi_سائيز[4];
    u8 bi_ويڪر[4];
    u8 bi_اوچائي[4];
    u8 biPlanes[2];
    u8 bi_ٻٽو_انگ_ڳڻپ[2];
    u8 biCompression[4];
    u8 bi_سائيز_Image[4];
    u8 bi_XPels_في_Meter[4];
    u8 bi_YPels_في_Meter[4];
    u8 bi_Clr_استعمال_ٿيل[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, ٽيگ_BITMAPINFOHEADER;

تعريفنوع بنية ٽيگ_RGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, ٽيگ_RGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
ساكن فراغ raw_معلومات_format_adjust(u8 *src, u8 *dest, صحيح ويڪر,
		صحيح اوچائي, صحيح بائيٽون_في_سٽ_src, صحيح بائيٽون_في_سٽ_dest)
{
    صحيح i;
    لكل (i = 0 ; i < اوچائي ; i++) {
        يادگيري_نقل(dest + i * بائيٽون_في_سٽ_dest,
           src + (اوچائي - 1 - i) * بائيٽون_في_سٽ_src, بائيٽون_في_سٽ_src);
    }
}

/* allocate decdata struct */
بنية bmp_decdata *bmp_alloc(فراغ)
{
    بنية bmp_decdata *bmp = malloc_tmphigh(حجم(*bmp));
    أرجع bmp;
}

/* extract information from bmp file data */
صحيح bmp_decode(بنية bmp_decdata *bmp, بنانشان حرف *معلومات, صحيح معلومات_سائيز)
{
    إذا (معلومات_سائيز < 54)
        أرجع 1;

    u16 bmp_filehead = bmp_load2byte(معلومات + 0);
    إذا (bmp_filehead != 0x4d42)
        أرجع 2;
    u32 bmp_recordsize = bmp_load4byte(معلومات + 2);
    إذا (bmp_recordsize != معلومات_سائيز)
        أرجع 3;
    u32 bmp_dataoffset = bmp_load4byte(معلومات + 10);
    bmp->datap = (بنانشان حرف *)معلومات + bmp_dataoffset;
    bmp->ويڪر = bmp_load4byte(معلومات + 18);
    bmp->اوچائي = bmp_load4byte(معلومات + 22);
    bmp->bpp = bmp_load2byte(معلومات + 28);
    أرجع 0;
}

/* get bmp properties */
فراغ bmp_get_info(بنية bmp_decdata *bmp, صحيح *ويڪر, صحيح *اوچائي, صحيح *bpp)
{
    *ويڪر = bmp->ويڪر;
    *اوچائي = bmp->اوچائي;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
صحيح bmp_show(بنية bmp_decdata *bmp, بنانشان حرف *pic, صحيح ويڪر,
             صحيح اوچائي, صحيح اونهائي, صحيح بائيٽون_في_سٽ_dest)
{
    إذا (bmp->datap == pic)
        أرجع 0;
    إذا ((اونهائي == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_معلومات_format_adjust(bmp->datap, pic, ويڪر, اوچائي,
			(bmp->bpp/8)*ويڪر, بائيٽون_في_سٽ_dest);
        أرجع 0;
    }
    أرجع 1;
}
