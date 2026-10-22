/*
* Basic BMP data process and Raw picture data handle functions.
* Could be used to adjust pixel data format, get information, etc.
*
* Copyright (C) 2011 Wayne Xia <xiawenc@cn.ibm.com>
*
* This work is licensed under the terms of the GNU LGPLv3.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_tmphigh
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // struct bmp_decdata

ਬਣਤਰ bmp_decdata {
    ਬਣਤਰ ਟੈਗ_RGBQUAD *quadp;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *datap;
    ਪੂਰਨ ਚੌੜਾਈ;
    ਪੂਰਨ ਉਚਾਈ;
    ਪੂਰਨ bpp;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ bmp_load4byte(addr) (*(u32 *)(addr))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ bmp_load2byte(addr) (*(u16 *)(addr))

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਟੈਗ_BITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_ਮਾਪ[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bf_Off_ਬਿੱਟ[4];
} BITMAPFILEHEADER, ਟੈਗ_BITMAPFILEHEADER;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਟੈਗ_BITMAPINFOHEADER {
    u8 bi_ਮਾਪ[4];
    u8 bi_ਚੌੜਾਈ[4];
    u8 bi_ਉਚਾਈ[4];
    u8 biPlanes[2];
    u8 bi_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਣਤੀ[2];
    u8 biCompression[4];
    u8 bi_ਮਾਪ_Image[4];
    u8 bi_XPels_ਪ੍ਰਤੀ_Meter[4];
    u8 bi_YPels_ਪ੍ਰਤੀ_Meter[4];
    u8 bi_Clr_ਵਰਤਿਆ[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, ਟੈਗ_BITMAPINFOHEADER;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਟੈਗ_RGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, ਟੈਗ_RGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
ਸਥਿਰ ਖਾਲੀ raw_ਅੰਕੜੇ_format_adjust(u8 *src, u8 *dest, ਪੂਰਨ ਚੌੜਾਈ,
		ਪੂਰਨ ਉਚਾਈ, ਪੂਰਨ ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_src, ਪੂਰਨ ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest)
{
    ਪੂਰਨ i;
    ਲਈ (i = 0 ; i < ਉਚਾਈ ; i++) {
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest + i * ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest,
           src + (ਉਚਾਈ - 1 - i) * ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_src, ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_src);
    }
}

/* allocate decdata struct */
ਬਣਤਰ bmp_decdata *bmp_alloc(ਖਾਲੀ)
{
    ਬਣਤਰ bmp_decdata *bmp = malloc_tmphigh(ਆਕਾਰ(*bmp));
    ਵਾਪਸ bmp;
}

/* extract information from bmp file data */
ਪੂਰਨ bmp_decode(ਬਣਤਰ bmp_decdata *bmp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅੰਕੜੇ, ਪੂਰਨ ਅੰਕੜੇ_ਮਾਪ)
{
    ਜੇ (ਅੰਕੜੇ_ਮਾਪ < 54)
        ਵਾਪਸ 1;

    u16 bmp_filehead = bmp_load2byte(ਅੰਕੜੇ + 0);
    ਜੇ (bmp_filehead != 0x4d42)
        ਵਾਪਸ 2;
    u32 bmp_recordsize = bmp_load4byte(ਅੰਕੜੇ + 2);
    ਜੇ (bmp_recordsize != ਅੰਕੜੇ_ਮਾਪ)
        ਵਾਪਸ 3;
    u32 bmp_dataoffset = bmp_load4byte(ਅੰਕੜੇ + 10);
    bmp->datap = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ਅੰਕੜੇ + bmp_dataoffset;
    bmp->ਚੌੜਾਈ = bmp_load4byte(ਅੰਕੜੇ + 18);
    bmp->ਉਚਾਈ = bmp_load4byte(ਅੰਕੜੇ + 22);
    bmp->bpp = bmp_load2byte(ਅੰਕੜੇ + 28);
    ਵਾਪਸ 0;
}

/* get bmp properties */
ਖਾਲੀ bmp_get_info(ਬਣਤਰ bmp_decdata *bmp, ਪੂਰਨ *ਚੌੜਾਈ, ਪੂਰਨ *ਉਚਾਈ, ਪੂਰਨ *bpp)
{
    *ਚੌੜਾਈ = bmp->ਚੌੜਾਈ;
    *ਉਚਾਈ = bmp->ਉਚਾਈ;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
ਪੂਰਨ bmp_show(ਬਣਤਰ bmp_decdata *bmp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic, ਪੂਰਨ ਚੌੜਾਈ,
             ਪੂਰਨ ਉਚਾਈ, ਪੂਰਨ ਡੂੰਘਾਈ, ਪੂਰਨ ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest)
{
    ਜੇ (bmp->datap == pic)
        ਵਾਪਸ 0;
    ਜੇ ((ਡੂੰਘਾਈ == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_ਅੰਕੜੇ_format_adjust(bmp->datap, pic, ਚੌੜਾਈ, ਉਚਾਈ,
			(bmp->bpp/8)*ਚੌੜਾਈ, ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest);
        ਵਾਪਸ 0;
    }
    ਵਾਪਸ 1;
}
