/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#شامل "malloc.h" // malloc_tmphigh
#شامل "string.h" // memcpy
#شامل "util.h" // struct bmp_decdata

ساخت bmp_decdata {
    ساخت لیبل_RGBQUAD *quadp;
    غیرعلامتی حرف *datap;
    صحیح چوڑائی;
    صحیح اونچائی;
    صحیح bpp;
};

#تعریف_کرو bmp_load4byte(addr) (*(u32 *)(addr))
#تعریف_کرو bmp_load2byte(addr) (*(u16 *)(addr))

قسمتعریف ساخت لیبل_BITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_سائز[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bf_Off_بٹ[4];
} BITMAPFILEHEADER, لیبل_BITMAPFILEHEADER;

قسمتعریف ساخت لیبل_BITMAPINFOHEADER {
    u8 bi_سائز[4];
    u8 bi_چوڑائی[4];
    u8 bi_اونچائی[4];
    u8 biPlanes[2];
    u8 bi_ثنائی_ہندسہ_گنتی[2];
    u8 biCompression[4];
    u8 bi_سائز_Image[4];
    u8 bi_XPels_فی_Meter[4];
    u8 bi_YPels_فی_Meter[4];
    u8 bi_Clr_مستعمل[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, لیبل_BITMAPINFOHEADER;

قسمتعریف ساخت لیبل_RGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, لیبل_RGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
ساکن خالی raw_ڈیٹا_format_adjust(u8 *src, u8 *dest, صحیح چوڑائی,
		صحیح اونچائی, صحیح بائٹیں_فی_سطر_src, صحیح بائٹیں_فی_سطر_dest)
{
    صحیح i;
    برائے (i = 0 ; i < اونچائی ; i++) {
        حافظہ_نقل(dest + i * بائٹیں_فی_سطر_dest,
           src + (اونچائی - 1 - i) * بائٹیں_فی_سطر_src, بائٹیں_فی_سطر_src);
    }
}

/* allocate decdata struct */
ساخت bmp_decdata *bmp_alloc(خالی)
{
    ساخت bmp_decdata *bmp = malloc_tmphigh(حجم(*bmp));
    واپس bmp;
}

/* extract information from bmp file data */
صحیح bmp_decode(ساخت bmp_decdata *bmp, غیرعلامتی حرف *ڈیٹا, صحیح ڈیٹا_سائز)
{
    اگر (ڈیٹا_سائز < 54)
        واپس 1;

    u16 bmp_filehead = bmp_load2byte(ڈیٹا + 0);
    اگر (bmp_filehead != 0x4d42)
        واپس 2;
    u32 bmp_recordsize = bmp_load4byte(ڈیٹا + 2);
    اگر (bmp_recordsize != ڈیٹا_سائز)
        واپس 3;
    u32 bmp_dataoffset = bmp_load4byte(ڈیٹا + 10);
    bmp->datap = (غیرعلامتی حرف *)ڈیٹا + bmp_dataoffset;
    bmp->چوڑائی = bmp_load4byte(ڈیٹا + 18);
    bmp->اونچائی = bmp_load4byte(ڈیٹا + 22);
    bmp->bpp = bmp_load2byte(ڈیٹا + 28);
    واپس 0;
}

/* get bmp properties */
خالی bmp_get_info(ساخت bmp_decdata *bmp, صحیح *چوڑائی, صحیح *اونچائی, صحیح *bpp)
{
    *چوڑائی = bmp->چوڑائی;
    *اونچائی = bmp->اونچائی;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
صحیح bmp_show(ساخت bmp_decdata *bmp, غیرعلامتی حرف *pic, صحیح چوڑائی,
             صحیح اونچائی, صحیح گہرائی, صحیح بائٹیں_فی_سطر_dest)
{
    اگر (bmp->datap == pic)
        واپس 0;
    اگر ((گہرائی == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_ڈیٹا_format_adjust(bmp->datap, pic, چوڑائی, اونچائی,
			(bmp->bpp/8)*چوڑائی, بائٹیں_فی_سطر_dest);
        واپس 0;
    }
    واپس 1;
}
