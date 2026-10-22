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
    بنية نښان_RGBQUAD *quadp;
    بینښه حرف *datap;
    صحيح پلنوالی;
    صحيح لوړوالی;
    صحيح bpp;
};

#تعریف_کړه bmp_load4byte(addr) (*(u32 *)(addr))
#تعریف_کړه bmp_load2byte(addr) (*(u16 *)(addr))

تعريفنوع بنية نښان_BITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_اندازه[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bf_Off_بټونه[4];
} BITMAPFILEHEADER, نښان_BITMAPFILEHEADER;

تعريفنوع بنية نښان_BITMAPINFOHEADER {
    u8 bi_اندازه[4];
    u8 bi_پلنوالی[4];
    u8 bi_لوړوالی[4];
    u8 biPlanes[2];
    u8 bi_بټ_شمېر[2];
    u8 biCompression[4];
    u8 bi_اندازه_Image[4];
    u8 bi_XPels_فی_Meter[4];
    u8 bi_YPels_فی_Meter[4];
    u8 bi_Clr_کارول_شوی[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, نښان_BITMAPINFOHEADER;

تعريفنوع بنية نښان_RGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, نښان_RGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
ساكن فراغ raw_معلومات_format_adjust(u8 *src, u8 *dest, صحيح پلنوالی,
		صحيح لوړوالی, صحيح بایټونه_فی_کرښه_src, صحيح بایټونه_فی_کرښه_dest)
{
    صحيح i;
    لكل (i = 0 ; i < لوړوالی ; i++) {
        حافظه_نقل(dest + i * بایټونه_فی_کرښه_dest,
           src + (لوړوالی - 1 - i) * بایټونه_فی_کرښه_src, بایټونه_فی_کرښه_src);
    }
}

/* allocate decdata struct */
بنية bmp_decdata *bmp_alloc(فراغ)
{
    بنية bmp_decdata *bmp = malloc_tmphigh(حجم(*bmp));
    أرجع bmp;
}

/* extract information from bmp file data */
صحيح bmp_decode(بنية bmp_decdata *bmp, بینښه حرف *معلومات, صحيح معلومات_اندازه)
{
    إذا (معلومات_اندازه < 54)
        أرجع 1;

    u16 bmp_filehead = bmp_load2byte(معلومات + 0);
    إذا (bmp_filehead != 0x4d42)
        أرجع 2;
    u32 bmp_recordsize = bmp_load4byte(معلومات + 2);
    إذا (bmp_recordsize != معلومات_اندازه)
        أرجع 3;
    u32 bmp_dataoffset = bmp_load4byte(معلومات + 10);
    bmp->datap = (بینښه حرف *)معلومات + bmp_dataoffset;
    bmp->پلنوالی = bmp_load4byte(معلومات + 18);
    bmp->لوړوالی = bmp_load4byte(معلومات + 22);
    bmp->bpp = bmp_load2byte(معلومات + 28);
    أرجع 0;
}

/* get bmp properties */
فراغ bmp_get_info(بنية bmp_decdata *bmp, صحيح *پلنوالی, صحيح *لوړوالی, صحيح *bpp)
{
    *پلنوالی = bmp->پلنوالی;
    *لوړوالی = bmp->لوړوالی;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
صحيح bmp_show(بنية bmp_decdata *bmp, بینښه حرف *pic, صحيح پلنوالی,
             صحيح لوړوالی, صحيح ژوروالی, صحيح بایټونه_فی_کرښه_dest)
{
    إذا (bmp->datap == pic)
        أرجع 0;
    إذا ((ژوروالی == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_معلومات_format_adjust(bmp->datap, pic, پلنوالی, لوړوالی,
			(bmp->bpp/8)*پلنوالی, بایټونه_فی_کرښه_dest);
        أرجع 0;
    }
    أرجع 1;
}
