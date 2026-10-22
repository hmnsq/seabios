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
    بنية ٹیگ_RGBQUAD *quadp;
    بنانشانی حرف *datap;
    صحيح چوڑائی;
    صحيح اچائی;
    صحيح bpp;
};

#تعریف_کرو bmp_load4byte(addr) (*(u32 *)(addr))
#تعریف_کرو bmp_load2byte(addr) (*(u16 *)(addr))

تعريفنوع بنية ٹیگ_BITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_ناپ[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bf_Off_بٹ[4];
} BITMAPFILEHEADER, ٹیگ_BITMAPFILEHEADER;

تعريفنوع بنية ٹیگ_BITMAPINFOHEADER {
    u8 bi_ناپ[4];
    u8 bi_چوڑائی[4];
    u8 bi_اچائی[4];
    u8 biPlanes[2];
    u8 bi_ثنائی_ہندسہ_گنتی[2];
    u8 biCompression[4];
    u8 bi_ناپ_Image[4];
    u8 bi_XPels_فی_Meter[4];
    u8 bi_YPels_فی_Meter[4];
    u8 bi_Clr_ورتیا[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, ٹیگ_BITMAPINFOHEADER;

تعريفنوع بنية ٹیگ_RGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, ٹیگ_RGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
ساكن فراغ raw_انکڑے_format_adjust(u8 *src, u8 *dest, صحيح چوڑائی,
		صحيح اچائی, صحيح بائٹاں_فی_سطر_src, صحيح بائٹاں_فی_سطر_dest)
{
    صحيح i;
    لكل (i = 0 ; i < اچائی ; i++) {
        حافظہ_نقل(dest + i * بائٹاں_فی_سطر_dest,
           src + (اچائی - 1 - i) * بائٹاں_فی_سطر_src, بائٹاں_فی_سطر_src);
    }
}

/* allocate decdata struct */
بنية bmp_decdata *bmp_alloc(فراغ)
{
    بنية bmp_decdata *bmp = malloc_tmphigh(حجم(*bmp));
    أرجع bmp;
}

/* extract information from bmp file data */
صحيح bmp_decode(بنية bmp_decdata *bmp, بنانشانی حرف *انکڑے, صحيح انکڑے_ناپ)
{
    إذا (انکڑے_ناپ < 54)
        أرجع 1;

    u16 bmp_filehead = bmp_load2byte(انکڑے + 0);
    إذا (bmp_filehead != 0x4d42)
        أرجع 2;
    u32 bmp_recordsize = bmp_load4byte(انکڑے + 2);
    إذا (bmp_recordsize != انکڑے_ناپ)
        أرجع 3;
    u32 bmp_dataoffset = bmp_load4byte(انکڑے + 10);
    bmp->datap = (بنانشانی حرف *)انکڑے + bmp_dataoffset;
    bmp->چوڑائی = bmp_load4byte(انکڑے + 18);
    bmp->اچائی = bmp_load4byte(انکڑے + 22);
    bmp->bpp = bmp_load2byte(انکڑے + 28);
    أرجع 0;
}

/* get bmp properties */
فراغ bmp_get_info(بنية bmp_decdata *bmp, صحيح *چوڑائی, صحيح *اچائی, صحيح *bpp)
{
    *چوڑائی = bmp->چوڑائی;
    *اچائی = bmp->اچائی;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
صحيح bmp_show(بنية bmp_decdata *bmp, بنانشانی حرف *pic, صحيح چوڑائی,
             صحيح اچائی, صحيح ڈونگھائی, صحيح بائٹاں_فی_سطر_dest)
{
    إذا (bmp->datap == pic)
        أرجع 0;
    إذا ((ڈونگھائی == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_انکڑے_format_adjust(bmp->datap, pic, چوڑائی, اچائی,
			(bmp->bpp/8)*چوڑائی, بائٹاں_فی_سطر_dest);
        أرجع 0;
    }
    أرجع 1;
}
