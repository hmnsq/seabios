/*
 * Copyright (C) 2001, Novell Inc.
 * Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Novell nor the names of the contributors may 
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*
 * a tiny jpeg decoder.
 *
 * written in August 2001 by Michael Schroeder <mls@suse.de>
 *
 */

#परिभाषय __LITTLE_ENDIAN
#अन्तर्भाव "malloc.h"
#अन्तर्भाव "string.h"
#अन्तर्भाव "util.h"
#परिभाषय ISHIFT 11

#परिभाषय IFIX(a) ((पूर्णाङ्क)((a) * (1 << ISHIFT) + .5))
#परिभाषय IMULT(a, b) (((a) * (b)) >> ISHIFT)
#परिभाषय ITOINT(a) ((a) >> ISHIFT)

#यदि_अपरिभाषितम् __P
# define __P(x) x
#यदि_अन्तः

/* special markers */
#परिभाषय M_BADHUFF        -1
#परिभाषय M_EOF                0x80

संरचना अन्तर् {
    अचिह्नित वर्ण *p;
    अचिह्नित पूर्णाङ्क द्वयङ्काः;
    पूर्णाङ्क वामः;
    पूर्णाङ्क marker;

    पूर्णाङ्क (*func) __P((शून्य *));
    शून्य *दत्त;
};

/*********************************/
संरचना dec_hufftbl;
संरचना enc_hufftbl;

सङ्घ hufftblp {
    संरचना dec_hufftbl *dhuff;
    संरचना enc_hufftbl *ehuff;
};

संरचना अवलोकनम् {
    पूर्णाङ्क dc;               /* old dc value */

    सङ्घ hufftblp hudc;
    सङ्घ hufftblp huac;
    पूर्णाङ्क अग्र;             /* when to switch to next scan */

    पूर्णाङ्क cid;              /* component id */
    पूर्णाङ्क hv;               /* horiz/vert, copied from comp */
    पूर्णाङ्क tq;               /* quant tbl, copied from comp */
};

/*********************************/

#परिभाषय DECBITS 10        /* seems to be the optimum */

संरचना dec_hufftbl {
    पूर्णाङ्क maxcode[17];
    पूर्णाङ्क valptr[16];
    अचिह्नित वर्ण vals[256];
    अचिह्नित पूर्णाङ्क llvals[1 << DECBITS];
};

स्थिर शून्य decode_mcus __P((संरचना अन्तर् *, पूर्णाङ्क *, पूर्णाङ्क, संरचना अवलोकनम् *, पूर्णाङ्क *));
स्थिर पूर्णाङ्क dec_readmarker __P((संरचना अन्तर् *));
स्थिर शून्य dec_makehuff __P((संरचना dec_hufftbl *, पूर्णाङ्क *, अचिह्नित वर्ण *));

स्थिर शून्य setinput __P((संरचना अन्तर् *, अचिह्नित वर्ण *));
/*********************************/

#अपरिभाषय PREC
#परिभाषय PREC पूर्णाङ्क

स्थिर शून्य idctqtab __P((अचिह्नित वर्ण *, PREC *));
स्थिर शून्य idct __P((पूर्णाङ्क *, पूर्णाङ्क *, PREC *, PREC, पूर्णाङ्क));
स्थिर शून्य scaleidctqtab __P((PREC *, PREC));

/*********************************/

स्थिर शून्य initcol __P((PREC[][64]));

स्थिर शून्य col221111 __P((पूर्णाङ्क *, अचिह्नित वर्ण *, पूर्णाङ्क));
स्थिर शून्य col221111_16 __P((पूर्णाङ्क *, अचिह्नित वर्ण *, पूर्णाङ्क));
स्थिर शून्य col221111_32 __P((पूर्णाङ्क *, अचिह्नित वर्ण *, पूर्णाङ्क));

/*********************************/

#परिभाषय ERR_NO_SOI 1
#परिभाषय ERR_NOT_8BIT 2
#परिभाषय ERR_HEIGHT_MISMATCH 3
#परिभाषय ERR_विस्तारः_MISMATCH 4
#परिभाषय ERR_BAD_विस्तारः_OR_HEIGHT 5
#परिभाषय ERR_TOO_MANY_COMPPS 6
#परिभाषय ERR_ILLEGAL_HV 7
#परिभाषय ERR_QUANT_सारणी_SELECTOR 8
#परिभाषय ERR_NOT_YCBCR_221111 9
#परिभाषय ERR_UNKNOWN_CID_अन्तर्_अवलोकनम् 10
#परिभाषय ERR_NOT_SEQUENTIAL_DCT 11
#परिभाषय ERR_WRONG_MARKER 12
#परिभाषय ERR_NO_EOI 13
#परिभाषय ERR_BAD_TABLES 14
#परिभाषय ERR_गभीरता_MISMATCH 15

/*********************************/

#परिभाषय M_SOI   0xd8
#परिभाषय M_APP0  0xe0
#परिभाषय M_DQT   0xdb
#परिभाषय M_SOF0  0xc0
#परिभाषय M_DHT   0xc4
#परिभाषय M_DRI   0xdd
#परिभाषय M_SOS   0xda
#परिभाषय M_RST0  0xd0
#परिभाषय M_EOI   0xd9
#परिभाषय M_COM   0xfe

संरचना comp {
    पूर्णाङ्क cid;
    पूर्णाङ्क hv;
    पूर्णाङ्क tq;
};

#परिभाषय MAXCOMP 4
संरचना jpginfo {
    पूर्णाङ्क nc;   /* number of components */
    पूर्णाङ्क ns;   /* number of scans */
    पूर्णाङ्क dri;  /* restart interval */
    पूर्णाङ्क nm;   /* mcus til next marker */
    पूर्णाङ्क rm;   /* next restart marker */
};

संरचना jpeg_decdata {
    पूर्णाङ्क dcts[6 * 64 + 16];
    पूर्णाङ्क बहिर्गमनम्[64 * 6];
    पूर्णाङ्क dquant[3][64];

    अचिह्नित वर्ण *datap;
    संरचना jpginfo info;
    संरचना comp comps[MAXCOMP];
    संरचना अवलोकनम् dscans[MAXCOMP];
    अचिह्नित वर्ण quant[4][64];
    संरचना dec_hufftbl dhuff[4];
    संरचना अन्तर् अन्तर्;

    पूर्णाङ्क height, विस्तारः;
};

स्थिर पूर्णाङ्क getbyte(संरचना jpeg_decdata *jpeg)
{
    प्रत्यागम *jpeg->datap++;
}

स्थिर पूर्णाङ्क getword(संरचना jpeg_decdata *jpeg)
{
    पूर्णाङ्क c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    प्रत्यागम c1 << 8 | c2;
}

स्थिर पूर्णाङ्क readtables(संरचना jpeg_decdata *jpeg, पूर्णाङ्क till)
{
    पूर्णाङ्क m, l, i, j, lq, pq, tq;
    पूर्णाङ्क tc, th, tt;

    प्रति (;;) {
        यदि (getbyte(jpeg) != 0xff)
            प्रत्यागम -1;
        यदि ((m = getbyte(jpeg)) == till)
            भङ्ग;

        विकल्प (m) {
        पक्ष 0xc2:
            प्रत्यागम 0;

        पक्ष M_DQT:
            lq = getword(jpeg);
            यावत् (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                यदि (tq > 3)
                    प्रत्यागम -1;
                pq >>= 4;
                यदि (pq != 0)
                    प्रत्यागम -1;
                प्रति (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            भङ्ग;

        पक्ष M_DHT:
            l = getword(jpeg);
            यावत् (l > 2) {
                पूर्णाङ्क hufflen[16], k;
                अचिह्नित वर्ण huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                यदि (tc > 1 || th > 1)
                    प्रत्यागम -1;
                प्रति (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                प्रति (i = 0; i < 16; i++) {
                    प्रति (j = 0; j < hufflen[i]; j++)
                        huffvals[k++] = getbyte(jpeg);
                    l -= hufflen[i];
                }
                dec_makehuff(jpeg->dhuff + tt, hufflen, huffvals);
            }
            भङ्ग;

        पक्ष M_DRI:
            l = getword(jpeg);
            jpeg->info.dri = getword(jpeg);
            भङ्ग;

        स्वभाव:
            l = getword(jpeg);
            यावत् (l-- > 2)
                getbyte(jpeg);
            भङ्ग;
        }
    }
    प्रत्यागम 0;
}

स्थिर शून्य dec_initscans(संरचना jpeg_decdata *jpeg)
{
    पूर्णाङ्क i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    प्रति (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

स्थिर पूर्णाङ्क dec_checkmarker(संरचना jpeg_decdata *jpeg)
{
    पूर्णाङ्क i;

    यदि (dec_readmarker(&jpeg->अन्तर्) != jpeg->info.rm)
        प्रत्यागम -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    प्रति (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    प्रत्यागम 0;
}

संरचना jpeg_decdata *jpeg_alloc(शून्य)
{
    संरचना jpeg_decdata *jpeg = malloc_tmphigh(परिमाण(*jpeg));
    प्रत्यागम jpeg;
}

पूर्णाङ्क jpeg_decode(संरचना jpeg_decdata *jpeg, अचिह्नित वर्ण *buf)
{
    पूर्णाङ्क i, j, m, tac, tdc;

    यदि (!jpeg || !buf)
        प्रत्यागम -1;
    jpeg->datap = buf;
    यदि (getbyte(jpeg) != 0xff)
        प्रत्यागम ERR_NO_SOI;
    यदि (getbyte(jpeg) != M_SOI)
        प्रत्यागम ERR_NO_SOI;
    यदि (readtables(jpeg, M_SOF0))
        प्रत्यागम ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    यदि (i != 8)
        प्रत्यागम ERR_NOT_8BIT;
    jpeg->height = getword(jpeg);
    jpeg->विस्तारः = getword(jpeg);
    यदि ((jpeg->height & 15) || (jpeg->विस्तारः & 15))
        प्रत्यागम ERR_BAD_विस्तारः_OR_HEIGHT;
    jpeg->info.nc = getbyte(jpeg);
    यदि (jpeg->info.nc > MAXCOMP)
        प्रत्यागम ERR_TOO_MANY_COMPPS;
    प्रति (i = 0; i < jpeg->info.nc; i++) {
        पूर्णाङ्क h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        यदि (h > 3 || v > 3)
            प्रत्यागम ERR_ILLEGAL_HV;
        यदि (jpeg->comps[i].tq > 3)
            प्रत्यागम ERR_QUANT_सारणी_SELECTOR;
    }
    यदि (readtables(jpeg, M_SOS))
        प्रत्यागम ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    यदि (jpeg->info.ns != 3)
        प्रत्यागम ERR_NOT_YCBCR_221111;
    प्रति (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        यदि (tdc > 1 || tac > 1)
            प्रत्यागम ERR_QUANT_सारणी_SELECTOR;
        प्रति (j = 0; j < jpeg->info.nc; j++)
            यदि (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                भङ्ग;
        यदि (j == jpeg->info.nc)
            प्रत्यागम ERR_UNKNOWN_CID_अन्तर्_अवलोकनम्;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    यदि (i != 0 || j != 63 || m != 0)
        प्रत्यागम ERR_NOT_SEQUENTIAL_DCT;

    यदि (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        प्रत्यागम ERR_NOT_YCBCR_221111;

    यदि (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        प्रत्यागम ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->अन्तर्, jpeg->datap);

#यदि 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#यदि_अन्तः

    dec_initscans(jpeg);

    प्रत्यागम 0;
}

शून्य jpeg_get_आकार(संरचना jpeg_decdata *jpeg, पूर्णाङ्क *विस्तारः, पूर्णाङ्क *height)
{
    *विस्तारः = jpeg->विस्तारः;
    *height = jpeg->height;
}

पूर्णाङ्क jpeg_show(संरचना jpeg_decdata *jpeg, अचिह्नित वर्ण *pic, पूर्णाङ्क विस्तारः
              , पूर्णाङ्क height, पूर्णाङ्क गभीरता, पूर्णाङ्क अष्टकानि_प्रत्येकम्_पङ्क्तिः_dest)
{
    पूर्णाङ्क m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    पूर्णाङ्क max[6];

    यदि (jpeg->height != height)
        प्रत्यागम ERR_HEIGHT_MISMATCH;
    यदि (jpeg->विस्तारः != विस्तारः)
        प्रत्यागम ERR_विस्तारः_MISMATCH;

    jpgbpl = विस्तारः * गभीरता / 8;
    mloffset = अष्टकानि_प्रत्येकम्_पङ्क्तिः_dest > jpgbpl ? अष्टकानि_प्रत्येकम्_पङ्क्तिः_dest : jpgbpl;

    mcusx = jpeg->विस्तारः >> 4;
    mcusy = jpeg->height >> 4;

    jpeg->dscans[0].अग्र = 6 - 4;
    jpeg->dscans[1].अग्र = 6 - 4 - 1;
    jpeg->dscans[2].अग्र = 6 - 4 - 1 - 1;        /* 411 encoding */
    प्रति (my = 0; my < mcusy; my++) {
        प्रति (mx = 0; mx < mcusx; mx++) {
            यदि (jpeg->info.dri && !--jpeg->info.nm)
                यदि (dec_checkmarker(jpeg))
                    प्रत्यागम ERR_WRONG_MARKER;

            decode_mcus(&jpeg->अन्तर्, jpeg->dcts, 6, jpeg->dscans, max);
            idct(jpeg->dcts, jpeg->बहिर्गमनम्, jpeg->dquant[0],
                 IFIX(128.5), max[0]);
            idct(jpeg->dcts + 64, jpeg->बहिर्गमनम् + 64, jpeg->dquant[0],
                 IFIX(128.5), max[1]);
            idct(jpeg->dcts + 128, jpeg->बहिर्गमनम् + 128, jpeg->dquant[0],
                 IFIX(128.5), max[2]);
            idct(jpeg->dcts + 192, jpeg->बहिर्गमनम् + 192, jpeg->dquant[0],
                 IFIX(128.5), max[3]);
            idct(jpeg->dcts + 256, jpeg->बहिर्गमनम् + 256, jpeg->dquant[1],
                 IFIX(0.5), max[4]);
            idct(jpeg->dcts + 320, jpeg->बहिर्गमनम् + 320, jpeg->dquant[2],
                 IFIX(0.5), max[5]);

            विकल्प (गभीरता) {
            पक्ष 32:
                col221111_32(jpeg->बहिर्गमनम्,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                भङ्ग;
            पक्ष 24:
                col221111(jpeg->बहिर्गमनम्,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                भङ्ग;
            पक्ष 16:
                col221111_16(jpeg->बहिर्गमनम्,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                भङ्ग;
            स्वभाव:
                प्रत्यागम ERR_गभीरता_MISMATCH;
                भङ्ग;
            }
        }
    }

    m = dec_readmarker(&jpeg->अन्तर्);
    यदि (m != M_EOI)
        प्रत्यागम ERR_NO_EOI;

    प्रत्यागम 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

स्थिर पूर्णाङ्क fillbits __P((संरचना अन्तर् *, पूर्णाङ्क, अचिह्नित पूर्णाङ्क));
स्थिर पूर्णाङ्क dec_rec2 __P((संरचना अन्तर् *, संरचना dec_hufftbl *, पूर्णाङ्क *, पूर्णाङ्क, पूर्णाङ्क));

स्थिर शून्य setinput(संरचना अन्तर् *अन्तर्, अचिह्नित वर्ण *p)
{
    अन्तर्->p = p;
    अन्तर्->वामः = 0;
    अन्तर्->द्वयङ्काः = 0;
    अन्तर्->marker = 0;
}

स्थिर पूर्णाङ्क fillbits(संरचना अन्तर् *अन्तर्, पूर्णाङ्क le, अचिह्नित पूर्णाङ्क bi)
{
    पूर्णाङ्क b, m;

    यदि (अन्तर्->marker) {
        यदि (le <= 16)
            अन्तर्->द्वयङ्काः = bi << 16, le += 16;
        प्रत्यागम le;
    }
    यावत् (le <= 24) {
        b = *अन्तर्->p++;
        यदि (b == 0xff && (m = *अन्तर्->p++) != 0) {
            यदि (m == M_EOF) {
                यदि (अन्तर्->func && (m = अन्तर्->func(अन्तर्->दत्त)) == 0)
                    अनुवर्त;
            }
            अन्तर्->marker = m;
            यदि (le <= 16)
                bi = bi << 16, le += 16;
            भङ्ग;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    अन्तर्->द्वयङ्काः = bi;                /* tmp... 2 return values needed */
    प्रत्यागम le;
}

स्थिर पूर्णाङ्क dec_readmarker(संरचना अन्तर् *अन्तर्)
{
    पूर्णाङ्क m;

    अन्तर्->वामः = fillbits(अन्तर्, अन्तर्->वामः, अन्तर्->द्वयङ्काः);
    यदि ((m = अन्तर्->marker) == 0)
        प्रत्यागम 0;
    अन्तर्->वामः = 0;
    अन्तर्->marker = 0;
    प्रत्यागम m;
}

#परिभाषय LEBI_DCL       पूर्णाङ्क le, bi
#परिभाषय LEBI_GET(अन्तर्)   (le = अन्तर्->वामः, bi = अन्तर्->द्वयङ्काः)
#परिभाषय LEBI_PUT(अन्तर्)   (अन्तर्->वामः = le, अन्तर्->द्वयङ्काः = bi)

#परिभाषय GETBITS(अन्तर्, n) (                                     \
  (le < (n) ? le = fillbits(अन्तर्, le, bi), bi = अन्तर्->द्वयङ्काः : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#परिभाषय UNGETBITS(अन्तर्, n) ( \
  le += (n)                \
)


स्थिर पूर्णाङ्क dec_rec2(संरचना अन्तर् *अन्तर्, संरचना dec_hufftbl *hu, पूर्णाङ्क *runp,
                    पूर्णाङ्क c, पूर्णाङ्क i)
{
    LEBI_DCL;

    LEBI_GET(अन्तर्);
    यदि (i) {
        UNGETBITS(अन्तर्, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } अन्यथा {
        प्रति (i = DECBITS;
             (c = ((c << 1) | GETBITS(अन्तर्, 1))) >= (hu->maxcode[i]); i++);
        यदि (i >= 16) {
            अन्तर्->marker = M_BADHUFF;
            प्रत्यागम 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    यदि (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(अन्तर्);
        प्रत्यागम 0;
    }
    /* receive part */
    c = GETBITS(अन्तर्, i);
    यदि (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(अन्तर्);
    प्रत्यागम c;
}

#परिभाषय DEC_REC(अन्तर्, hu, r, i)         (  \
  r = GETBITS(अन्तर्, DECBITS),              \
  i = hu->llvals[r],                     \
  i & 128 ?                              \
    (                                    \
      UNGETBITS(अन्तर्, i & 127),            \
      r = i >> 8 & 15,                   \
      i >> 16                            \
    )                                    \
  :                                      \
    (                                    \
      LEBI_PUT(अन्तर्),                      \
      i = dec_rec2(अन्तर्, hu, &r, r, i),    \
      LEBI_GET(अन्तर्),                      \
      i                                  \
    )                                    \
)

स्थिर शून्य decode_mcus(संरचना अन्तर् *अन्तर्, पूर्णाङ्क *dct, पूर्णाङ्क n, संरचना अवलोकनम् *sc,
                        पूर्णाङ्क *maxp)
{
    संरचना dec_hufftbl *hu;
    पूर्णाङ्क i, r, t;
    LEBI_DCL;

    स्मृतिं_पूरय(dct, 0, n * 64 * परिमाण(*dct));
    LEBI_GET(अन्तर्);
    यावत् (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(अन्तर्, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        यावत् (i > 0) {
            t = DEC_REC(अन्तर्, hu, r, t);
            यदि (t == 0 && r == 0) {
                dct += i;
                भङ्ग;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        यदि (n == sc->अग्र)
            sc++;
    }
    LEBI_PUT(अन्तर्);
}

स्थिर शून्य dec_makehuff(संरचना dec_hufftbl *hu, पूर्णाङ्क *hufflen,
                         अचिह्नित वर्ण *huffvals)
{
    पूर्णाङ्क संहिता, k, i, j, d, x, c, v;
    प्रति (i = 0; i < (1 << DECBITS); i++)
        hu->llvals[i] = 0;

    /*
     * llvals layout:
     *
     * value v already known, run r, backup u bits:
     *  vvvvvvvvvvvvvvvv 0000 rrrr 1 uuuuuuu
     * value unknown, size b bits, run r, backup u bits:
     *  000000000000bbbb 0000 rrrr 0 uuuuuuu
     * value and size unknown:
     *  0000000000000000 0000 0000 0 0000000
     */

    संहिता = 0;
    k = 0;
    प्रति (i = 0; i < 16; i++, संहिता <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        प्रति (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            यदि (i < DECBITS) {
                c = संहिता << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /*आकार*/
                प्रति (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    यदि (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        यदि (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } अन्यथा
                        x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1));
                    hu->llvals[c | d] = x;
                }
            }
            संहिता++;
            k++;
        }
        hu->maxcode[i] = संहिता;
    }
    hu->maxcode[16] = 0x20000;        /* always terminate decode */
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

#परिभाषय ONE ((PREC)IFIX(1.))
#परिभाषय S2  ((PREC)IFIX(0.382683432))
#परिभाषय C2  ((PREC)IFIX(0.923879532))
#परिभाषय C4  ((PREC)IFIX(0.707106781))

#परिभाषय S22 ((PREC)IFIX(2 * 0.382683432))
#परिभाषय C22 ((PREC)IFIX(2 * 0.923879532))
#परिभाषय IC4 ((PREC)IFIX(1 / 0.707106781))

#परिभाषय C3IC1 ((PREC)IFIX(0.847759065))        /* c3/c1 */
#परिभाषय C5IC1 ((PREC)IFIX(0.566454497))        /* c5/c1 */
#परिभाषय C7IC1 ((PREC)IFIX(0.198912367))        /* c7/c1 */

#परिभाषय XPP(a,b) (t = a + b, b = a - b, a = t)
#परिभाषय XMP(a,b) (t = a - b, b = a + b, a = t)
#परिभाषय XPM(a,b) (t = a + b, b = b - a, a = t)

#परिभाषय ROT(a,b,s,c) (  t = IMULT(a + b, s),      \
                        a = IMULT(a, c - s) + t,  \
                        b = IMULT(b, c + s) - t)

#परिभाषय IDCT                \
(                           \
  XPP(t0, t1),              \
  XMP(t2, t3),              \
  t2 = IMULT(t2, IC4) - t3, \
  XPP(t0, t3),              \
  XPP(t1, t2),              \
  XMP(t4, t7),              \
  XPP(t5, t6),              \
  XMP(t5, t7),              \
  t5 = IMULT(t5, IC4),      \
  ROT(t4, t6, S22, C22),    \
  t6 -= t7,                 \
  t5 -= t6,                 \
  t4 -= t5,                 \
  XPP(t0, t7),              \
  XPP(t1, t6),              \
  XPP(t2, t5),              \
  XPP(t3, t4)               \
)

स्थिर अचिह्नित वर्ण zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

स्थिर शून्य idct(पूर्णाङ्क *अन्तर्, पूर्णाङ्क *बहिर्गमनम्, PREC * quant, PREC off, पूर्णाङ्क max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    पूर्णाङ्क i, j;
    अचिह्नित वर्ण *zig2p;

    t0 = off;
    यदि (max == 1) {
        t0 += अन्तर्[0] * quant[0];
        प्रति (i = 0; i < 64; i++)
            बहिर्गमनम्[i] = ITOINT(t0);
        प्रत्यागम;
    }
    zig2p = zig2;
    tmpp = tmp;
    प्रति (i = 0; i < 8; i++) {
        j = *zig2p++;
        t0 += अन्तर्[j] * quant[j];
        j = *zig2p++;
        t5 = अन्तर्[j] * quant[j];
        j = *zig2p++;
        t2 = अन्तर्[j] * quant[j];
        j = *zig2p++;
        t7 = अन्तर्[j] * quant[j];
        j = *zig2p++;
        t1 = अन्तर्[j] * quant[j];
        j = *zig2p++;
        t4 = अन्तर्[j] * quant[j];
        j = *zig2p++;
        t3 = अन्तर्[j] * quant[j];
        j = *zig2p++;
        t6 = अन्तर्[j] * quant[j];
        IDCT;
        tmpp[0 * 8] = t0;
        tmpp[1 * 8] = t1;
        tmpp[2 * 8] = t2;
        tmpp[3 * 8] = t3;
        tmpp[4 * 8] = t4;
        tmpp[5 * 8] = t5;
        tmpp[6 * 8] = t6;
        tmpp[7 * 8] = t7;
        tmpp++;
        t0 = 0;
    }
    प्रति (i = 0; i < 8; i++) {
        t0 = tmp[8 * i + 0];
        t1 = tmp[8 * i + 1];
        t2 = tmp[8 * i + 2];
        t3 = tmp[8 * i + 3];
        t4 = tmp[8 * i + 4];
        t5 = tmp[8 * i + 5];
        t6 = tmp[8 * i + 6];
        t7 = tmp[8 * i + 7];
        IDCT;
        बहिर्गमनम्[8 * i + 0] = ITOINT(t0);
        बहिर्गमनम्[8 * i + 1] = ITOINT(t1);
        बहिर्गमनम्[8 * i + 2] = ITOINT(t2);
        बहिर्गमनम्[8 * i + 3] = ITOINT(t3);
        बहिर्गमनम्[8 * i + 4] = ITOINT(t4);
        बहिर्गमनम्[8 * i + 5] = ITOINT(t5);
        बहिर्गमनम्[8 * i + 6] = ITOINT(t6);
        बहिर्गमनम्[8 * i + 7] = ITOINT(t7);
    }
}

स्थिर अचिह्नित वर्ण zig[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

स्थिर PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};


स्थिर शून्य idctqtab(अचिह्नित वर्ण *qin, PREC * qout)
{
    पूर्णाङ्क i, j;

    प्रति (i = 0; i < 8; i++)
        प्रति (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

स्थिर शून्य scaleidctqtab(PREC * q, PREC sc)
{
    पूर्णाङ्क i;

    प्रति (i = 0; i < 64; i++)
        q[i] = IMULT(q[i], sc);
}

/****************************************************************/
/**************          color decoder            ***************/
/****************************************************************/

#परिभाषय आवर्तनम्

/*
 * YCbCr Color transformation:
 *
 * y:0..255   Cb:-128..127   Cr:-128..127
 *
 *      R = Y                + 1.40200 * Cr
 *      G = Y - 0.34414 * Cb - 0.71414 * Cr
 *      B = Y + 1.77200 * Cb
 *
 * =>
 *      Cr *= 1.40200;
 *      Cb *= 1.77200;
 *      Cg = 0.19421 * Cb + .50937 * Cr;
 *      R = Y + Cr;
 *      G = Y - Cg;
 *      B = Y + Cb;
 *
 * =>
 *      Cg = (50 * Cb + 130 * Cr + 128) >> 8;
 */

स्थिर शून्य initcol(PREC q[][64])
{
    scaleidctqtab(q[1], IFIX(1.77200));
    scaleidctqtab(q[2], IFIX(1.40200));
}

/* This is optimized for the stupid sun SUNWspro compiler. */
#परिभाषय STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (अचिह्नित पूर्णाङ्क)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#परिभाषय CLAMP(x) ((अचिह्नित पूर्णाङ्क)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#यदि_परिभाषितम् आवर्तनम्

#परिभाषय CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#अन्यथा

#परिभाषय CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (3 * cb + 8 * cr) >> 4                    \
)

#यदि_अन्तः

#यदि_परिभाषितम् __LITTLE_ENDIAN
#परिभाषय PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 2], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 0], y + cb)          \
)
#अन्यथा
#परिभाषय PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 0], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 2], y + cb)          \
)
#यदि_अन्तः

#यदि_परिभाषितम् __LITTLE_ENDIAN
#परिभाषय PIC_16(yin, xin, p, xout, सङ्कलय)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + सङ्कलय*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + सङ्कलय    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + सङ्कलय*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y & 0xff,                  \
  p[(xout) * 2 + 1] = y >> 8                     \
)
#अन्यथा
#यदि_परिभाषितम् CONFIG_PPC
#परिभाषय PIC_16(yin, xin, p, xout, सङ्कलय)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + सङ्कलय*2+1) & 0xf8) <<  7) | \
      ((CLAMP(y - cg + सङ्कलय*2+1) & 0xf8) <<  2) | \
      ((CLAMP(y + cb + सङ्कलय*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#अन्यथा
#परिभाषय PIC_16(yin, xin, p, xout, सङ्कलय)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + सङ्कलय*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + सङ्कलय    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + सङ्कलय*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#यदि_अन्तः
#यदि_अन्तः

#परिभाषय PIC_32(yin, xin, p, xout)               \
(                                               \
  y = outy[(yin) * 8 + xin],                    \
  STORECLAMP(p[(xout) * 4 + 0], y + cr),        \
  STORECLAMP(p[(xout) * 4 + 1], y - cg),        \
  STORECLAMP(p[(xout) * 4 + 2], y + cb),        \
  p[(xout) * 4 + 3] = 0                         \
)

#परिभाषय PIC221111(xin)                                              \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)        \
)

#परिभाषय PIC221111_16(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0, 3), \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1, 0), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0, 1), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1, 2)  \
)

#परिभाषय PIC221111_32(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)     \
)

स्थिर शून्य col221111(पूर्णाङ्क *बहिर्गमनम्, अचिह्नित वर्ण *pic, पूर्णाङ्क विस्तारः)
{
    पूर्णाङ्क i, j, k;
    अचिह्नित वर्ण *pic0, *pic1;
    पूर्णाङ्क *outy, *outc;
    पूर्णाङ्क cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + विस्तारः;
    outy = बहिर्गमनम्;
    outc = बहिर्गमनम् + 64 * 4;
    प्रति (i = 2; i > 0; i--) {
        प्रति (j = 4; j > 0; j--) {
            प्रति (k = 0; k < 8; k++) {
                PIC221111(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * विस्तारः;
            pic1 += 2 * विस्तारः;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

स्थिर शून्य col221111_16(पूर्णाङ्क *बहिर्गमनम्, अचिह्नित वर्ण *pic, पूर्णाङ्क विस्तारः)
{
    पूर्णाङ्क i, j, k;
    अचिह्नित वर्ण *pic0, *pic1;
    पूर्णाङ्क *outy, *outc;
    पूर्णाङ्क cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + विस्तारः;
    outy = बहिर्गमनम्;
    outc = बहिर्गमनम् + 64 * 4;
    प्रति (i = 2; i > 0; i--) {
        प्रति (j = 4; j > 0; j--) {
            प्रति (k = 0; k < 8; k++) {
                PIC221111_16(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * विस्तारः;
            pic1 += 2 * विस्तारः;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

स्थिर शून्य col221111_32(पूर्णाङ्क *बहिर्गमनम्, अचिह्नित वर्ण *pic, पूर्णाङ्क विस्तारः)
{
    पूर्णाङ्क i, j, k;
    अचिह्नित वर्ण *pic0, *pic1;
    पूर्णाङ्क *outy, *outc;
    पूर्णाङ्क cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + विस्तारः;
    outy = बहिर्गमनम्;
    outc = बहिर्गमनम् + 64 * 4;
    प्रति (i = 2; i > 0; i--) {
        प्रति (j = 4; j > 0; j--) {
            प्रति (k = 0; k < 8; k++) {
                PIC221111_32(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * विस्तारः;
            pic1 += 2 * विस्तारः;
        }
        outy += 64 * 2 - 16 * 4;
    }
}
