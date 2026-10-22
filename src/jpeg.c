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

#تعریف_کرو __LITTLE_ENDIAN
#شامل "malloc.h"
#شامل "string.h"
#شامل "util.h"
#تعریف_کرو ISHIFT 11

#تعریف_کرو IFIX(a) ((صحیح)((a) * (1 << ISHIFT) + .5))
#تعریف_کرو IMULT(a, b) (((a) * (b)) >> ISHIFT)
#تعریف_کرو ITOINT(a) ((a) >> ISHIFT)

#اگر_غیر_تعریف_شدہ __P
# define __P(x) x
#اگر_ختم

/* special markers */
#تعریف_کرو M_BADHUFF        -1
#تعریف_کرو M_EOF                0x80

ساخت اندر {
    غیرعلامتی حرف *p;
    غیرعلامتی صحیح بٹ;
    صحیح بایاں;
    صحیح marker;

    صحیح (*func) __P((خالی *));
    خالی *ڈیٹا;
};

/*********************************/
ساخت dec_hufftbl;
ساخت enc_hufftbl;

اتحاد hufftblp {
    ساخت dec_hufftbl *dhuff;
    ساخت enc_hufftbl *ehuff;
};

ساخت جائزہ {
    صحیح dc;               /* old dc value */

    اتحاد hufftblp hudc;
    اتحاد hufftblp huac;
    صحیح اگلا;             /* when to switch to next scan */

    صحیح cid;              /* component id */
    صحیح hv;               /* horiz/vert, copied from comp */
    صحیح tq;               /* quant tbl, copied from comp */
};

/*********************************/

#تعریف_کرو DECBITS 10        /* seems to be the optimum */

ساخت dec_hufftbl {
    صحیح maxcode[17];
    صحیح valptr[16];
    غیرعلامتی حرف vals[256];
    غیرعلامتی صحیح llvals[1 << DECBITS];
};

ساکن خالی decode_mcus __P((ساخت اندر *, صحیح *, صحیح, ساخت جائزہ *, صحیح *));
ساکن صحیح dec_readmarker __P((ساخت اندر *));
ساکن خالی dec_makehuff __P((ساخت dec_hufftbl *, صحیح *, غیرعلامتی حرف *));

ساکن خالی setinput __P((ساخت اندر *, غیرعلامتی حرف *));
/*********************************/

#تعریف_منسوخ PREC
#تعریف_کرو PREC صحیح

ساکن خالی idctqtab __P((غیرعلامتی حرف *, PREC *));
ساکن خالی idct __P((صحیح *, صحیح *, PREC *, PREC, صحیح));
ساکن خالی scaleidctqtab __P((PREC *, PREC));

/*********************************/

ساکن خالی initcol __P((PREC[][64]));

ساکن خالی col221111 __P((صحیح *, غیرعلامتی حرف *, صحیح));
ساکن خالی col221111_16 __P((صحیح *, غیرعلامتی حرف *, صحیح));
ساکن خالی col221111_32 __P((صحیح *, غیرعلامتی حرف *, صحیح));

/*********************************/

#تعریف_کرو ERR_NO_SOI 1
#تعریف_کرو ERR_NOT_8_ثنائی_ہندسہ 2
#تعریف_کرو ERR_اونچائی_MISMATCH 3
#تعریف_کرو ERR_چوڑائی_MISMATCH 4
#تعریف_کرو ERR_BAD_چوڑائی_OR_اونچائی 5
#تعریف_کرو ERR_TOO_MANY_COMPPS 6
#تعریف_کرو ERR_ILLEGAL_HV 7
#تعریف_کرو ERR_QUANT_جدول_SELECTOR 8
#تعریف_کرو ERR_NOT_YCBCR_221111 9
#تعریف_کرو ERR_UNKNOWN_CID_اندر_جائزہ 10
#تعریف_کرو ERR_NOT_SEQUENTIAL_DCT 11
#تعریف_کرو ERR_WRONG_MARKER 12
#تعریف_کرو ERR_NO_EOI 13
#تعریف_کرو ERR_BAD_TABLES 14
#تعریف_کرو ERR_گہرائی_MISMATCH 15

/*********************************/

#تعریف_کرو M_SOI   0xd8
#تعریف_کرو M_APP0  0xe0
#تعریف_کرو M_DQT   0xdb
#تعریف_کرو M_SOF0  0xc0
#تعریف_کرو M_DHT   0xc4
#تعریف_کرو M_DRI   0xdd
#تعریف_کرو M_SOS   0xda
#تعریف_کرو M_RST0  0xd0
#تعریف_کرو M_EOI   0xd9
#تعریف_کرو M_COM   0xfe

ساخت comp {
    صحیح cid;
    صحیح hv;
    صحیح tq;
};

#تعریف_کرو MAXCOMP 4
ساخت jpginfo {
    صحیح nc;   /* number of components */
    صحیح ns;   /* number of scans */
    صحیح dri;  /* restart interval */
    صحیح nm;   /* mcus til next marker */
    صحیح rm;   /* next restart marker */
};

ساخت jpeg_decdata {
    صحیح dcts[6 * 64 + 16];
    صحیح برآمد[64 * 6];
    صحیح dquant[3][64];

    غیرعلامتی حرف *datap;
    ساخت jpginfo info;
    ساخت comp comps[MAXCOMP];
    ساخت جائزہ dscans[MAXCOMP];
    غیرعلامتی حرف quant[4][64];
    ساخت dec_hufftbl dhuff[4];
    ساخت اندر اندر;

    صحیح اونچائی, چوڑائی;
};

ساکن صحیح getbyte(ساخت jpeg_decdata *jpeg)
{
    واپس *jpeg->datap++;
}

ساکن صحیح getword(ساخت jpeg_decdata *jpeg)
{
    صحیح c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    واپس c1 << 8 | c2;
}

ساکن صحیح readtables(ساخت jpeg_decdata *jpeg, صحیح till)
{
    صحیح m, l, i, j, lq, pq, tq;
    صحیح tc, th, tt;

    برائے (;;) {
        اگر (getbyte(jpeg) != 0xff)
            واپس -1;
        اگر ((m = getbyte(jpeg)) == till)
            توڑو;

        انتخاب (m) {
        حالت 0xc2:
            واپس 0;

        حالت M_DQT:
            lq = getword(jpeg);
            جب (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                اگر (tq > 3)
                    واپس -1;
                pq >>= 4;
                اگر (pq != 0)
                    واپس -1;
                برائے (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            توڑو;

        حالت M_DHT:
            l = getword(jpeg);
            جب (l > 2) {
                صحیح hufflen[16], k;
                غیرعلامتی حرف huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                اگر (tc > 1 || th > 1)
                    واپس -1;
                برائے (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                برائے (i = 0; i < 16; i++) {
                    برائے (j = 0; j < hufflen[i]; j++)
                        huffvals[k++] = getbyte(jpeg);
                    l -= hufflen[i];
                }
                dec_makehuff(jpeg->dhuff + tt, hufflen, huffvals);
            }
            توڑو;

        حالت M_DRI:
            l = getword(jpeg);
            jpeg->info.dri = getword(jpeg);
            توڑو;

        طےشدہ:
            l = getword(jpeg);
            جب (l-- > 2)
                getbyte(jpeg);
            توڑو;
        }
    }
    واپس 0;
}

ساکن خالی dec_initscans(ساخت jpeg_decdata *jpeg)
{
    صحیح i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    برائے (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

ساکن صحیح dec_checkmarker(ساخت jpeg_decdata *jpeg)
{
    صحیح i;

    اگر (dec_readmarker(&jpeg->اندر) != jpeg->info.rm)
        واپس -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    برائے (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    واپس 0;
}

ساخت jpeg_decdata *jpeg_alloc(خالی)
{
    ساخت jpeg_decdata *jpeg = malloc_tmphigh(حجم(*jpeg));
    واپس jpeg;
}

صحیح jpeg_decode(ساخت jpeg_decdata *jpeg, غیرعلامتی حرف *buf)
{
    صحیح i, j, m, tac, tdc;

    اگر (!jpeg || !buf)
        واپس -1;
    jpeg->datap = buf;
    اگر (getbyte(jpeg) != 0xff)
        واپس ERR_NO_SOI;
    اگر (getbyte(jpeg) != M_SOI)
        واپس ERR_NO_SOI;
    اگر (readtables(jpeg, M_SOF0))
        واپس ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    اگر (i != 8)
        واپس ERR_NOT_8_ثنائی_ہندسہ;
    jpeg->اونچائی = getword(jpeg);
    jpeg->چوڑائی = getword(jpeg);
    اگر ((jpeg->اونچائی & 15) || (jpeg->چوڑائی & 15))
        واپس ERR_BAD_چوڑائی_OR_اونچائی;
    jpeg->info.nc = getbyte(jpeg);
    اگر (jpeg->info.nc > MAXCOMP)
        واپس ERR_TOO_MANY_COMPPS;
    برائے (i = 0; i < jpeg->info.nc; i++) {
        صحیح h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        اگر (h > 3 || v > 3)
            واپس ERR_ILLEGAL_HV;
        اگر (jpeg->comps[i].tq > 3)
            واپس ERR_QUANT_جدول_SELECTOR;
    }
    اگر (readtables(jpeg, M_SOS))
        واپس ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    اگر (jpeg->info.ns != 3)
        واپس ERR_NOT_YCBCR_221111;
    برائے (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        اگر (tdc > 1 || tac > 1)
            واپس ERR_QUANT_جدول_SELECTOR;
        برائے (j = 0; j < jpeg->info.nc; j++)
            اگر (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                توڑو;
        اگر (j == jpeg->info.nc)
            واپس ERR_UNKNOWN_CID_اندر_جائزہ;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    اگر (i != 0 || j != 63 || m != 0)
        واپس ERR_NOT_SEQUENTIAL_DCT;

    اگر (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        واپس ERR_NOT_YCBCR_221111;

    اگر (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        واپس ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->اندر, jpeg->datap);

#اگر 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#اگر_ختم

    dec_initscans(jpeg);

    واپس 0;
}

خالی jpeg_get_سائز(ساخت jpeg_decdata *jpeg, صحیح *چوڑائی, صحیح *اونچائی)
{
    *چوڑائی = jpeg->چوڑائی;
    *اونچائی = jpeg->اونچائی;
}

صحیح jpeg_show(ساخت jpeg_decdata *jpeg, غیرعلامتی حرف *pic, صحیح چوڑائی
              , صحیح اونچائی, صحیح گہرائی, صحیح بائٹیں_فی_سطر_dest)
{
    صحیح m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    صحیح max[6];

    اگر (jpeg->اونچائی != اونچائی)
        واپس ERR_اونچائی_MISMATCH;
    اگر (jpeg->چوڑائی != چوڑائی)
        واپس ERR_چوڑائی_MISMATCH;

    jpgbpl = چوڑائی * گہرائی / 8;
    mloffset = بائٹیں_فی_سطر_dest > jpgbpl ? بائٹیں_فی_سطر_dest : jpgbpl;

    mcusx = jpeg->چوڑائی >> 4;
    mcusy = jpeg->اونچائی >> 4;

    jpeg->dscans[0].اگلا = 6 - 4;
    jpeg->dscans[1].اگلا = 6 - 4 - 1;
    jpeg->dscans[2].اگلا = 6 - 4 - 1 - 1;        /* 411 encoding */
    برائے (my = 0; my < mcusy; my++) {
        برائے (mx = 0; mx < mcusx; mx++) {
            اگر (jpeg->info.dri && !--jpeg->info.nm)
                اگر (dec_checkmarker(jpeg))
                    واپس ERR_WRONG_MARKER;

            decode_mcus(&jpeg->اندر, jpeg->dcts, 6, jpeg->dscans, max);
            idct(jpeg->dcts, jpeg->برآمد, jpeg->dquant[0],
                 IFIX(128.5), max[0]);
            idct(jpeg->dcts + 64, jpeg->برآمد + 64, jpeg->dquant[0],
                 IFIX(128.5), max[1]);
            idct(jpeg->dcts + 128, jpeg->برآمد + 128, jpeg->dquant[0],
                 IFIX(128.5), max[2]);
            idct(jpeg->dcts + 192, jpeg->برآمد + 192, jpeg->dquant[0],
                 IFIX(128.5), max[3]);
            idct(jpeg->dcts + 256, jpeg->برآمد + 256, jpeg->dquant[1],
                 IFIX(0.5), max[4]);
            idct(jpeg->dcts + 320, jpeg->برآمد + 320, jpeg->dquant[2],
                 IFIX(0.5), max[5]);

            انتخاب (گہرائی) {
            حالت 32:
                col221111_32(jpeg->برآمد,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                توڑو;
            حالت 24:
                col221111(jpeg->برآمد,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                توڑو;
            حالت 16:
                col221111_16(jpeg->برآمد,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                توڑو;
            طےشدہ:
                واپس ERR_گہرائی_MISMATCH;
                توڑو;
            }
        }
    }

    m = dec_readmarker(&jpeg->اندر);
    اگر (m != M_EOI)
        واپس ERR_NO_EOI;

    واپس 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

ساکن صحیح fillbits __P((ساخت اندر *, صحیح, غیرعلامتی صحیح));
ساکن صحیح dec_rec2 __P((ساخت اندر *, ساخت dec_hufftbl *, صحیح *, صحیح, صحیح));

ساکن خالی setinput(ساخت اندر *اندر, غیرعلامتی حرف *p)
{
    اندر->p = p;
    اندر->بایاں = 0;
    اندر->بٹ = 0;
    اندر->marker = 0;
}

ساکن صحیح fillbits(ساخت اندر *اندر, صحیح le, غیرعلامتی صحیح bi)
{
    صحیح b, m;

    اگر (اندر->marker) {
        اگر (le <= 16)
            اندر->بٹ = bi << 16, le += 16;
        واپس le;
    }
    جب (le <= 24) {
        b = *اندر->p++;
        اگر (b == 0xff && (m = *اندر->p++) != 0) {
            اگر (m == M_EOF) {
                اگر (اندر->func && (m = اندر->func(اندر->ڈیٹا)) == 0)
                    جاری;
            }
            اندر->marker = m;
            اگر (le <= 16)
                bi = bi << 16, le += 16;
            توڑو;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    اندر->بٹ = bi;                /* tmp... 2 return values needed */
    واپس le;
}

ساکن صحیح dec_readmarker(ساخت اندر *اندر)
{
    صحیح m;

    اندر->بایاں = fillbits(اندر, اندر->بایاں, اندر->بٹ);
    اگر ((m = اندر->marker) == 0)
        واپس 0;
    اندر->بایاں = 0;
    اندر->marker = 0;
    واپس m;
}

#تعریف_کرو LEBI_DCL       صحیح le, bi
#تعریف_کرو LEBI_GET(اندر)   (le = اندر->بایاں, bi = اندر->بٹ)
#تعریف_کرو LEBI_PUT(اندر)   (اندر->بایاں = le, اندر->بٹ = bi)

#تعریف_کرو GETBITS(اندر, n) (                                     \
  (le < (n) ? le = fillbits(اندر, le, bi), bi = اندر->بٹ : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#تعریف_کرو UNGETBITS(اندر, n) ( \
  le += (n)                \
)


ساکن صحیح dec_rec2(ساخت اندر *اندر, ساخت dec_hufftbl *hu, صحیح *runp,
                    صحیح c, صحیح i)
{
    LEBI_DCL;

    LEBI_GET(اندر);
    اگر (i) {
        UNGETBITS(اندر, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } ورنہ {
        برائے (i = DECBITS;
             (c = ((c << 1) | GETBITS(اندر, 1))) >= (hu->maxcode[i]); i++);
        اگر (i >= 16) {
            اندر->marker = M_BADHUFF;
            واپس 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    اگر (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(اندر);
        واپس 0;
    }
    /* receive part */
    c = GETBITS(اندر, i);
    اگر (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(اندر);
    واپس c;
}

#تعریف_کرو DEC_REC(اندر, hu, r, i)         (  \
  r = GETBITS(اندر, DECBITS),              \
  i = hu->llvals[r],                     \
  i & 128 ?                              \
    (                                    \
      UNGETBITS(اندر, i & 127),            \
      r = i >> 8 & 15,                   \
      i >> 16                            \
    )                                    \
  :                                      \
    (                                    \
      LEBI_PUT(اندر),                      \
      i = dec_rec2(اندر, hu, &r, r, i),    \
      LEBI_GET(اندر),                      \
      i                                  \
    )                                    \
)

ساکن خالی decode_mcus(ساخت اندر *اندر, صحیح *dct, صحیح n, ساخت جائزہ *sc,
                        صحیح *maxp)
{
    ساخت dec_hufftbl *hu;
    صحیح i, r, t;
    LEBI_DCL;

    memset(dct, 0, n * 64 * حجم(*dct));
    LEBI_GET(اندر);
    جب (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(اندر, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        جب (i > 0) {
            t = DEC_REC(اندر, hu, r, t);
            اگر (t == 0 && r == 0) {
                dct += i;
                توڑو;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        اگر (n == sc->اگلا)
            sc++;
    }
    LEBI_PUT(اندر);
}

ساکن خالی dec_makehuff(ساخت dec_hufftbl *hu, صحیح *hufflen,
                         غیرعلامتی حرف *huffvals)
{
    صحیح رمز, k, i, j, d, x, c, v;
    برائے (i = 0; i < (1 << DECBITS); i++)
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

    رمز = 0;
    k = 0;
    برائے (i = 0; i < 16; i++, رمز <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        برائے (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            اگر (i < DECBITS) {
                c = رمز << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /*سائز*/
                برائے (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    اگر (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        اگر (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } ورنہ
                        x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1));
                    hu->llvals[c | d] = x;
                }
            }
            رمز++;
            k++;
        }
        hu->maxcode[i] = رمز;
    }
    hu->maxcode[16] = 0x20000;        /* always terminate decode */
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

#تعریف_کرو ONE ((PREC)IFIX(1.))
#تعریف_کرو S2  ((PREC)IFIX(0.382683432))
#تعریف_کرو C2  ((PREC)IFIX(0.923879532))
#تعریف_کرو C4  ((PREC)IFIX(0.707106781))

#تعریف_کرو S22 ((PREC)IFIX(2 * 0.382683432))
#تعریف_کرو C22 ((PREC)IFIX(2 * 0.923879532))
#تعریف_کرو IC4 ((PREC)IFIX(1 / 0.707106781))

#تعریف_کرو C3IC1 ((PREC)IFIX(0.847759065))        /* c3/c1 */
#تعریف_کرو C5IC1 ((PREC)IFIX(0.566454497))        /* c5/c1 */
#تعریف_کرو C7IC1 ((PREC)IFIX(0.198912367))        /* c7/c1 */

#تعریف_کرو XPP(a,b) (t = a + b, b = a - b, a = t)
#تعریف_کرو XMP(a,b) (t = a - b, b = a + b, a = t)
#تعریف_کرو XPM(a,b) (t = a + b, b = b - a, a = t)

#تعریف_کرو ROT(a,b,s,c) (  t = IMULT(a + b, s),      \
                        a = IMULT(a, c - s) + t,  \
                        b = IMULT(b, c + s) - t)

#تعریف_کرو IDCT                \
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

ساکن غیرعلامتی حرف zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

ساکن خالی idct(صحیح *اندر, صحیح *برآمد, PREC * quant, PREC off, صحیح max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    صحیح i, j;
    غیرعلامتی حرف *zig2p;

    t0 = off;
    اگر (max == 1) {
        t0 += اندر[0] * quant[0];
        برائے (i = 0; i < 64; i++)
            برآمد[i] = ITOINT(t0);
        واپس;
    }
    zig2p = zig2;
    tmpp = tmp;
    برائے (i = 0; i < 8; i++) {
        j = *zig2p++;
        t0 += اندر[j] * quant[j];
        j = *zig2p++;
        t5 = اندر[j] * quant[j];
        j = *zig2p++;
        t2 = اندر[j] * quant[j];
        j = *zig2p++;
        t7 = اندر[j] * quant[j];
        j = *zig2p++;
        t1 = اندر[j] * quant[j];
        j = *zig2p++;
        t4 = اندر[j] * quant[j];
        j = *zig2p++;
        t3 = اندر[j] * quant[j];
        j = *zig2p++;
        t6 = اندر[j] * quant[j];
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
    برائے (i = 0; i < 8; i++) {
        t0 = tmp[8 * i + 0];
        t1 = tmp[8 * i + 1];
        t2 = tmp[8 * i + 2];
        t3 = tmp[8 * i + 3];
        t4 = tmp[8 * i + 4];
        t5 = tmp[8 * i + 5];
        t6 = tmp[8 * i + 6];
        t7 = tmp[8 * i + 7];
        IDCT;
        برآمد[8 * i + 0] = ITOINT(t0);
        برآمد[8 * i + 1] = ITOINT(t1);
        برآمد[8 * i + 2] = ITOINT(t2);
        برآمد[8 * i + 3] = ITOINT(t3);
        برآمد[8 * i + 4] = ITOINT(t4);
        برآمد[8 * i + 5] = ITOINT(t5);
        برآمد[8 * i + 6] = ITOINT(t6);
        برآمد[8 * i + 7] = ITOINT(t7);
    }
}

ساکن غیرعلامتی حرف zig[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

ساکن PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};


ساکن خالی idctqtab(غیرعلامتی حرف *qin, PREC * qout)
{
    صحیح i, j;

    برائے (i = 0; i < 8; i++)
        برائے (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

ساکن خالی scaleidctqtab(PREC * q, PREC sc)
{
    صحیح i;

    برائے (i = 0; i < 64; i++)
        q[i] = IMULT(q[i], sc);
}

/****************************************************************/
/**************          color decoder            ***************/
/****************************************************************/

#تعریف_کرو باری

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

ساکن خالی initcol(PREC q[][64])
{
    scaleidctqtab(q[1], IFIX(1.77200));
    scaleidctqtab(q[2], IFIX(1.40200));
}

/* This is optimized for the stupid sun SUNWspro compiler. */
#تعریف_کرو STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (غیرعلامتی صحیح)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#تعریف_کرو CLAMP(x) ((غیرعلامتی صحیح)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#اگر_تعریف_شدہ باری

#تعریف_کرو CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#ورنہ

#تعریف_کرو CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (3 * cb + 8 * cr) >> 4                    \
)

#اگر_ختم

#اگر_تعریف_شدہ __LITTLE_ENDIAN
#تعریف_کرو PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 2], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 0], y + cb)          \
)
#ورنہ
#تعریف_کرو PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 0], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 2], y + cb)          \
)
#اگر_ختم

#اگر_تعریف_شدہ __LITTLE_ENDIAN
#تعریف_کرو PIC_16(yin, xin, p, xout, جوڑو)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + جوڑو*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + جوڑو    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + جوڑو*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y & 0xff,                  \
  p[(xout) * 2 + 1] = y >> 8                     \
)
#ورنہ
#اگر_تعریف_شدہ CONFIG_PPC
#تعریف_کرو PIC_16(yin, xin, p, xout, جوڑو)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + جوڑو*2+1) & 0xf8) <<  7) | \
      ((CLAMP(y - cg + جوڑو*2+1) & 0xf8) <<  2) | \
      ((CLAMP(y + cb + جوڑو*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#ورنہ
#تعریف_کرو PIC_16(yin, xin, p, xout, جوڑو)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + جوڑو*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + جوڑو    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + جوڑو*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#اگر_ختم
#اگر_ختم

#تعریف_کرو PIC_32(yin, xin, p, xout)               \
(                                               \
  y = outy[(yin) * 8 + xin],                    \
  STORECLAMP(p[(xout) * 4 + 0], y + cr),        \
  STORECLAMP(p[(xout) * 4 + 1], y - cg),        \
  STORECLAMP(p[(xout) * 4 + 2], y + cb),        \
  p[(xout) * 4 + 3] = 0                         \
)

#تعریف_کرو PIC221111(xin)                                              \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)        \
)

#تعریف_کرو PIC221111_16(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0, 3), \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1, 0), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0, 1), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1, 2)  \
)

#تعریف_کرو PIC221111_32(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)     \
)

ساکن خالی col221111(صحیح *برآمد, غیرعلامتی حرف *pic, صحیح چوڑائی)
{
    صحیح i, j, k;
    غیرعلامتی حرف *pic0, *pic1;
    صحیح *outy, *outc;
    صحیح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + چوڑائی;
    outy = برآمد;
    outc = برآمد + 64 * 4;
    برائے (i = 2; i > 0; i--) {
        برائے (j = 4; j > 0; j--) {
            برائے (k = 0; k < 8; k++) {
                PIC221111(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * چوڑائی;
            pic1 += 2 * چوڑائی;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ساکن خالی col221111_16(صحیح *برآمد, غیرعلامتی حرف *pic, صحیح چوڑائی)
{
    صحیح i, j, k;
    غیرعلامتی حرف *pic0, *pic1;
    صحیح *outy, *outc;
    صحیح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + چوڑائی;
    outy = برآمد;
    outc = برآمد + 64 * 4;
    برائے (i = 2; i > 0; i--) {
        برائے (j = 4; j > 0; j--) {
            برائے (k = 0; k < 8; k++) {
                PIC221111_16(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * چوڑائی;
            pic1 += 2 * چوڑائی;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ساکن خالی col221111_32(صحیح *برآمد, غیرعلامتی حرف *pic, صحیح چوڑائی)
{
    صحیح i, j, k;
    غیرعلامتی حرف *pic0, *pic1;
    صحیح *outy, *outc;
    صحیح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + چوڑائی;
    outy = برآمد;
    outc = برآمد + 64 * 4;
    برائے (i = 2; i > 0; i--) {
        برائے (j = 4; j > 0; j--) {
            برائے (k = 0; k < 8; k++) {
                PIC221111_32(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * چوڑائی;
            pic1 += 2 * چوڑائی;
        }
        outy += 64 * 2 - 16 * 4;
    }
}
