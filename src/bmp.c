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
    संरचना चिह्नकम्_RGBQUAD *quadp;
    अचिह्नित वर्ण *datap;
    पूर्णाङ्क विस्तारः;
    पूर्णाङ्क height;
    पूर्णाङ्क bpp;
};

#परिभाषय bmp_load4byte(addr) (*(u32 *)(addr))
#परिभाषय bmp_load2byte(addr) (*(u16 *)(addr))

प्रकारनिर्देश संरचना चिह्नकम्_BITMAPFILEHEADER {
    u8 bfType[2];
    u8 bf_आकार[4];
    u8 bfReserved1[2];
    u8 bfReserved2[2];
    u8 bf_Off_द्वयङ्काः[4];
} BITMAPFILEHEADER, चिह्नकम्_BITMAPFILEHEADER;

प्रकारनिर्देश संरचना चिह्नकम्_BITMAPINFOHEADER {
    u8 bi_आकार[4];
    u8 bi_विस्तारः[4];
    u8 biHeight[4];
    u8 biPlanes[2];
    u8 bi_Bit_गणनम्[2];
    u8 biCompression[4];
    u8 bi_आकार_Image[4];
    u8 bi_XPels_प्रत्येकम्_Meter[4];
    u8 bi_YPels_प्रत्येकम्_Meter[4];
    u8 bi_Clr_प्रयुक्तम्[4];
    u8 biClrImportant[4];
} BITMAPINFOHEADER, चिह्नकम्_BITMAPINFOHEADER;

प्रकारनिर्देश संरचना चिह्नकम्_RGBQUAD {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD, चिह्नकम्_RGBQUAD;

/* flat picture data adjusting function
* description:
*   switch the vertical line sequence
*   arrange horizontal pixel data, add extra space in the dest buffer
*       for every line
*/
स्थिर शून्य raw_दत्त_format_adjust(u8 *src, u8 *dest, पूर्णाङ्क विस्तारः,
		पूर्णाङ्क height, पूर्णाङ्क अष्टकानि_प्रत्येकम्_पङ्क्तिः_src, पूर्णाङ्क अष्टकानि_प्रत्येकम्_पङ्क्तिः_dest)
{
    पूर्णाङ्क i;
    प्रति (i = 0 ; i < height ; i++) {
        स्मृतिं_प्रतिलिख(dest + i * अष्टकानि_प्रत्येकम्_पङ्क्तिः_dest,
           src + (height - 1 - i) * अष्टकानि_प्रत्येकम्_पङ्क्तिः_src, अष्टकानि_प्रत्येकम्_पङ्क्तिः_src);
    }
}

/* allocate decdata struct */
संरचना bmp_decdata *bmp_alloc(शून्य)
{
    संरचना bmp_decdata *bmp = malloc_tmphigh(परिमाण(*bmp));
    प्रत्यागम bmp;
}

/* extract information from bmp file data */
पूर्णाङ्क bmp_decode(संरचना bmp_decdata *bmp, अचिह्नित वर्ण *दत्त, पूर्णाङ्क दत्त_आकार)
{
    यदि (दत्त_आकार < 54)
        प्रत्यागम 1;

    u16 bmp_filehead = bmp_load2byte(दत्त + 0);
    यदि (bmp_filehead != 0x4d42)
        प्रत्यागम 2;
    u32 bmp_recordsize = bmp_load4byte(दत्त + 2);
    यदि (bmp_recordsize != दत्त_आकार)
        प्रत्यागम 3;
    u32 bmp_dataoffset = bmp_load4byte(दत्त + 10);
    bmp->datap = (अचिह्नित वर्ण *)दत्त + bmp_dataoffset;
    bmp->विस्तारः = bmp_load4byte(दत्त + 18);
    bmp->height = bmp_load4byte(दत्त + 22);
    bmp->bpp = bmp_load2byte(दत्त + 28);
    प्रत्यागम 0;
}

/* get bmp properties */
शून्य bmp_get_info(संरचना bmp_decdata *bmp, पूर्णाङ्क *विस्तारः, पूर्णाङ्क *height, पूर्णाङ्क *bpp)
{
    *विस्तारः = bmp->विस्तारः;
    *height = bmp->height;
    *bpp = bmp->bpp;
}

/* flush flat picture data to *pc */
पूर्णाङ्क bmp_show(संरचना bmp_decdata *bmp, अचिह्नित वर्ण *pic, पूर्णाङ्क विस्तारः,
             पूर्णाङ्क height, पूर्णाङ्क गभीरता, पूर्णाङ्क अष्टकानि_प्रत्येकम्_पङ्क्तिः_dest)
{
    यदि (bmp->datap == pic)
        प्रत्यागम 0;
    यदि ((गभीरता == bmp->bpp) && (bmp->bpp%8 == 0)) {
        raw_दत्त_format_adjust(bmp->datap, pic, विस्तारः, height,
			(bmp->bpp/8)*विस्तारः, अष्टकानि_प्रत्येकम्_पङ्क्तिः_dest);
        प्रत्यागम 0;
    }
    प्रत्यागम 1;
}
