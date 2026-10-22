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

#وصف_ڪريو __LITTLE_ENDIAN
#ضمن "malloc.h"
#ضمن "string.h"
#ضمن "util.h"
#وصف_ڪريو ISHIFT 11

#وصف_ڪريو IFIX(a) ((صحيح)((a) * (1 << ISHIFT) + .5))
#وصف_ڪريو IMULT(a, b) (((a) * (b)) >> ISHIFT)
#وصف_ڪريو ITOINT(a) ((a) >> ISHIFT)

#جيڪڏهن_وصف_نه_ٿيل __P
# define __P(x) x
#جيڪڏهن_پورو

/* special markers */
#وصف_ڪريو M_BADHUFF        -1
#وصف_ڪريو M_EOF                0x80

بنية اندريون {
    بنانشان حرف *p;
    بنانشان صحيح بٽ;
    صحيح کاٻو;
    صحيح marker;

    صحيح (*func) __P((فراغ *));
    فراغ *معلومات;
};

/*********************************/
بنية dec_hufftbl;
بنية enc_hufftbl;

اتحاد hufftblp {
    بنية dec_hufftbl *dhuff;
    بنية enc_hufftbl *ehuff;
};

بنية جاچ {
    صحيح dc;               /* old dc value */

    اتحاد hufftblp hudc;
    اتحاد hufftblp huac;
    صحيح ايندڙ;             /* when to switch to next scan */

    صحيح cid;              /* component id */
    صحيح hv;               /* horiz/vert, copied from comp */
    صحيح tq;               /* quant tbl, copied from comp */
};

/*********************************/

#وصف_ڪريو DECBITS 10        /* seems to be the optimum */

بنية dec_hufftbl {
    صحيح maxcode[17];
    صحيح valptr[16];
    بنانشان حرف vals[256];
    بنانشان صحيح llvals[1 << DECBITS];
};

ساكن فراغ decode_mcus __P((بنية اندريون *, صحيح *, صحيح, بنية جاچ *, صحيح *));
ساكن صحيح dec_readmarker __P((بنية اندريون *));
ساكن فراغ dec_makehuff __P((بنية dec_hufftbl *, صحيح *, بنانشان حرف *));

ساكن فراغ setinput __P((بنية اندريون *, بنانشان حرف *));
/*********************************/

#وصف_رد PREC
#وصف_ڪريو PREC صحيح

ساكن فراغ idctqtab __P((بنانشان حرف *, PREC *));
ساكن فراغ idct __P((صحيح *, صحيح *, PREC *, PREC, صحيح));
ساكن فراغ scaleidctqtab __P((PREC *, PREC));

/*********************************/

ساكن فراغ initcol __P((PREC[][64]));

ساكن فراغ col221111 __P((صحيح *, بنانشان حرف *, صحيح));
ساكن فراغ col221111_16 __P((صحيح *, بنانشان حرف *, صحيح));
ساكن فراغ col221111_32 __P((صحيح *, بنانشان حرف *, صحيح));

/*********************************/

#وصف_ڪريو ERR_NO_SOI 1
#وصف_ڪريو ERR_NOT_8_ٻٽو_انگ 2
#وصف_ڪريو ERR_اوچائي_MISMATCH 3
#وصف_ڪريو ERR_ويڪر_MISMATCH 4
#وصف_ڪريو ERR_BAD_ويڪر_OR_اوچائي 5
#وصف_ڪريو ERR_TOO_MANY_COMPPS 6
#وصف_ڪريو ERR_ILLEGAL_HV 7
#وصف_ڪريو ERR_QUANT_جدول_SELECTOR 8
#وصف_ڪريو ERR_NOT_YCBCR_221111 9
#وصف_ڪريو ERR_UNKNOWN_CID_اندريون_جاچ 10
#وصف_ڪريو ERR_NOT_SEQUENTIAL_DCT 11
#وصف_ڪريو ERR_WRONG_MARKER 12
#وصف_ڪريو ERR_NO_EOI 13
#وصف_ڪريو ERR_BAD_TABLES 14
#وصف_ڪريو ERR_اونهائي_MISMATCH 15

/*********************************/

#وصف_ڪريو M_SOI   0xd8
#وصف_ڪريو M_APP0  0xe0
#وصف_ڪريو M_DQT   0xdb
#وصف_ڪريو M_SOF0  0xc0
#وصف_ڪريو M_DHT   0xc4
#وصف_ڪريو M_DRI   0xdd
#وصف_ڪريو M_SOS   0xda
#وصف_ڪريو M_RST0  0xd0
#وصف_ڪريو M_EOI   0xd9
#وصف_ڪريو M_COM   0xfe

بنية comp {
    صحيح cid;
    صحيح hv;
    صحيح tq;
};

#وصف_ڪريو MAXCOMP 4
بنية jpginfo {
    صحيح nc;   /* number of components */
    صحيح ns;   /* number of scans */
    صحيح dri;  /* restart interval */
    صحيح nm;   /* mcus til next marker */
    صحيح rm;   /* next restart marker */
};

بنية jpeg_decdata {
    صحيح dcts[6 * 64 + 16];
    صحيح ٻاهريون[64 * 6];
    صحيح dquant[3][64];

    بنانشان حرف *datap;
    بنية jpginfo info;
    بنية comp comps[MAXCOMP];
    بنية جاچ dscans[MAXCOMP];
    بنانشان حرف quant[4][64];
    بنية dec_hufftbl dhuff[4];
    بنية اندريون اندريون;

    صحيح اوچائي, ويڪر;
};

ساكن صحيح getbyte(بنية jpeg_decdata *jpeg)
{
    أرجع *jpeg->datap++;
}

ساكن صحيح getword(بنية jpeg_decdata *jpeg)
{
    صحيح c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    أرجع c1 << 8 | c2;
}

ساكن صحيح readtables(بنية jpeg_decdata *jpeg, صحيح till)
{
    صحيح m, l, i, j, lq, pq, tq;
    صحيح tc, th, tt;

    لكل (;;) {
        إذا (getbyte(jpeg) != 0xff)
            أرجع -1;
        إذا ((m = getbyte(jpeg)) == till)
            اكسر;

        بدل (m) {
        حالة 0xc2:
            أرجع 0;

        حالة M_DQT:
            lq = getword(jpeg);
            طالما (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                إذا (tq > 3)
                    أرجع -1;
                pq >>= 4;
                إذا (pq != 0)
                    أرجع -1;
                لكل (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            اكسر;

        حالة M_DHT:
            l = getword(jpeg);
            طالما (l > 2) {
                صحيح hufflen[16], k;
                بنانشان حرف huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                إذا (tc > 1 || th > 1)
                    أرجع -1;
                لكل (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                لكل (i = 0; i < 16; i++) {
                    لكل (j = 0; j < hufflen[i]; j++)
                        huffvals[k++] = getbyte(jpeg);
                    l -= hufflen[i];
                }
                dec_makehuff(jpeg->dhuff + tt, hufflen, huffvals);
            }
            اكسر;

        حالة M_DRI:
            l = getword(jpeg);
            jpeg->info.dri = getword(jpeg);
            اكسر;

        افتراضي:
            l = getword(jpeg);
            طالما (l-- > 2)
                getbyte(jpeg);
            اكسر;
        }
    }
    أرجع 0;
}

ساكن فراغ dec_initscans(بنية jpeg_decdata *jpeg)
{
    صحيح i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    لكل (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

ساكن صحيح dec_checkmarker(بنية jpeg_decdata *jpeg)
{
    صحيح i;

    إذا (dec_readmarker(&jpeg->اندريون) != jpeg->info.rm)
        أرجع -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    لكل (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    أرجع 0;
}

بنية jpeg_decdata *jpeg_alloc(فراغ)
{
    بنية jpeg_decdata *jpeg = malloc_tmphigh(حجم(*jpeg));
    أرجع jpeg;
}

صحيح jpeg_decode(بنية jpeg_decdata *jpeg, بنانشان حرف *buf)
{
    صحيح i, j, m, tac, tdc;

    إذا (!jpeg || !buf)
        أرجع -1;
    jpeg->datap = buf;
    إذا (getbyte(jpeg) != 0xff)
        أرجع ERR_NO_SOI;
    إذا (getbyte(jpeg) != M_SOI)
        أرجع ERR_NO_SOI;
    إذا (readtables(jpeg, M_SOF0))
        أرجع ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    إذا (i != 8)
        أرجع ERR_NOT_8_ٻٽو_انگ;
    jpeg->اوچائي = getword(jpeg);
    jpeg->ويڪر = getword(jpeg);
    إذا ((jpeg->اوچائي & 15) || (jpeg->ويڪر & 15))
        أرجع ERR_BAD_ويڪر_OR_اوچائي;
    jpeg->info.nc = getbyte(jpeg);
    إذا (jpeg->info.nc > MAXCOMP)
        أرجع ERR_TOO_MANY_COMPPS;
    لكل (i = 0; i < jpeg->info.nc; i++) {
        صحيح h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        إذا (h > 3 || v > 3)
            أرجع ERR_ILLEGAL_HV;
        إذا (jpeg->comps[i].tq > 3)
            أرجع ERR_QUANT_جدول_SELECTOR;
    }
    إذا (readtables(jpeg, M_SOS))
        أرجع ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    إذا (jpeg->info.ns != 3)
        أرجع ERR_NOT_YCBCR_221111;
    لكل (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        إذا (tdc > 1 || tac > 1)
            أرجع ERR_QUANT_جدول_SELECTOR;
        لكل (j = 0; j < jpeg->info.nc; j++)
            إذا (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                اكسر;
        إذا (j == jpeg->info.nc)
            أرجع ERR_UNKNOWN_CID_اندريون_جاچ;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    إذا (i != 0 || j != 63 || m != 0)
        أرجع ERR_NOT_SEQUENTIAL_DCT;

    إذا (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        أرجع ERR_NOT_YCBCR_221111;

    إذا (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        أرجع ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->اندريون, jpeg->datap);

#إذا 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#جيڪڏهن_پورو

    dec_initscans(jpeg);

    أرجع 0;
}

فراغ jpeg_get_سائيز(بنية jpeg_decdata *jpeg, صحيح *ويڪر, صحيح *اوچائي)
{
    *ويڪر = jpeg->ويڪر;
    *اوچائي = jpeg->اوچائي;
}

صحيح jpeg_show(بنية jpeg_decdata *jpeg, بنانشان حرف *pic, صحيح ويڪر
              , صحيح اوچائي, صحيح اونهائي, صحيح بائيٽون_في_سٽ_dest)
{
    صحيح m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    صحيح max[6];

    إذا (jpeg->اوچائي != اوچائي)
        أرجع ERR_اوچائي_MISMATCH;
    إذا (jpeg->ويڪر != ويڪر)
        أرجع ERR_ويڪر_MISMATCH;

    jpgbpl = ويڪر * اونهائي / 8;
    mloffset = بائيٽون_في_سٽ_dest > jpgbpl ? بائيٽون_في_سٽ_dest : jpgbpl;

    mcusx = jpeg->ويڪر >> 4;
    mcusy = jpeg->اوچائي >> 4;

    jpeg->dscans[0].ايندڙ = 6 - 4;
    jpeg->dscans[1].ايندڙ = 6 - 4 - 1;
    jpeg->dscans[2].ايندڙ = 6 - 4 - 1 - 1;        /* 411 encoding */
    لكل (my = 0; my < mcusy; my++) {
        لكل (mx = 0; mx < mcusx; mx++) {
            إذا (jpeg->info.dri && !--jpeg->info.nm)
                إذا (dec_checkmarker(jpeg))
                    أرجع ERR_WRONG_MARKER;

            decode_mcus(&jpeg->اندريون, jpeg->dcts, 6, jpeg->dscans, max);
            idct(jpeg->dcts, jpeg->ٻاهريون, jpeg->dquant[0],
                 IFIX(128.5), max[0]);
            idct(jpeg->dcts + 64, jpeg->ٻاهريون + 64, jpeg->dquant[0],
                 IFIX(128.5), max[1]);
            idct(jpeg->dcts + 128, jpeg->ٻاهريون + 128, jpeg->dquant[0],
                 IFIX(128.5), max[2]);
            idct(jpeg->dcts + 192, jpeg->ٻاهريون + 192, jpeg->dquant[0],
                 IFIX(128.5), max[3]);
            idct(jpeg->dcts + 256, jpeg->ٻاهريون + 256, jpeg->dquant[1],
                 IFIX(0.5), max[4]);
            idct(jpeg->dcts + 320, jpeg->ٻاهريون + 320, jpeg->dquant[2],
                 IFIX(0.5), max[5]);

            بدل (اونهائي) {
            حالة 32:
                col221111_32(jpeg->ٻاهريون,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                اكسر;
            حالة 24:
                col221111(jpeg->ٻاهريون,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                اكسر;
            حالة 16:
                col221111_16(jpeg->ٻاهريون,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                اكسر;
            افتراضي:
                أرجع ERR_اونهائي_MISMATCH;
                اكسر;
            }
        }
    }

    m = dec_readmarker(&jpeg->اندريون);
    إذا (m != M_EOI)
        أرجع ERR_NO_EOI;

    أرجع 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

ساكن صحيح fillbits __P((بنية اندريون *, صحيح, بنانشان صحيح));
ساكن صحيح dec_rec2 __P((بنية اندريون *, بنية dec_hufftbl *, صحيح *, صحيح, صحيح));

ساكن فراغ setinput(بنية اندريون *اندريون, بنانشان حرف *p)
{
    اندريون->p = p;
    اندريون->کاٻو = 0;
    اندريون->بٽ = 0;
    اندريون->marker = 0;
}

ساكن صحيح fillbits(بنية اندريون *اندريون, صحيح le, بنانشان صحيح bi)
{
    صحيح b, m;

    إذا (اندريون->marker) {
        إذا (le <= 16)
            اندريون->بٽ = bi << 16, le += 16;
        أرجع le;
    }
    طالما (le <= 24) {
        b = *اندريون->p++;
        إذا (b == 0xff && (m = *اندريون->p++) != 0) {
            إذا (m == M_EOF) {
                إذا (اندريون->func && (m = اندريون->func(اندريون->معلومات)) == 0)
                    استمر;
            }
            اندريون->marker = m;
            إذا (le <= 16)
                bi = bi << 16, le += 16;
            اكسر;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    اندريون->بٽ = bi;                /* tmp... 2 return values needed */
    أرجع le;
}

ساكن صحيح dec_readmarker(بنية اندريون *اندريون)
{
    صحيح m;

    اندريون->کاٻو = fillbits(اندريون, اندريون->کاٻو, اندريون->بٽ);
    إذا ((m = اندريون->marker) == 0)
        أرجع 0;
    اندريون->کاٻو = 0;
    اندريون->marker = 0;
    أرجع m;
}

#وصف_ڪريو LEBI_DCL       صحيح le, bi
#وصف_ڪريو LEBI_GET(اندريون)   (le = اندريون->کاٻو, bi = اندريون->بٽ)
#وصف_ڪريو LEBI_PUT(اندريون)   (اندريون->کاٻو = le, اندريون->بٽ = bi)

#وصف_ڪريو GETBITS(اندريون, n) (                                     \
  (le < (n) ? le = fillbits(اندريون, le, bi), bi = اندريون->بٽ : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#وصف_ڪريو UNGETBITS(اندريون, n) ( \
  le += (n)                \
)


ساكن صحيح dec_rec2(بنية اندريون *اندريون, بنية dec_hufftbl *hu, صحيح *runp,
                    صحيح c, صحيح i)
{
    LEBI_DCL;

    LEBI_GET(اندريون);
    إذا (i) {
        UNGETBITS(اندريون, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } وإلا {
        لكل (i = DECBITS;
             (c = ((c << 1) | GETBITS(اندريون, 1))) >= (hu->maxcode[i]); i++);
        إذا (i >= 16) {
            اندريون->marker = M_BADHUFF;
            أرجع 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    إذا (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(اندريون);
        أرجع 0;
    }
    /* receive part */
    c = GETBITS(اندريون, i);
    إذا (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(اندريون);
    أرجع c;
}

#وصف_ڪريو DEC_REC(اندريون, hu, r, i)         (  \
  r = GETBITS(اندريون, DECBITS),              \
  i = hu->llvals[r],                     \
  i & 128 ?                              \
    (                                    \
      UNGETBITS(اندريون, i & 127),            \
      r = i >> 8 & 15,                   \
      i >> 16                            \
    )                                    \
  :                                      \
    (                                    \
      LEBI_PUT(اندريون),                      \
      i = dec_rec2(اندريون, hu, &r, r, i),    \
      LEBI_GET(اندريون),                      \
      i                                  \
    )                                    \
)

ساكن فراغ decode_mcus(بنية اندريون *اندريون, صحيح *dct, صحيح n, بنية جاچ *sc,
                        صحيح *maxp)
{
    بنية dec_hufftbl *hu;
    صحيح i, r, t;
    LEBI_DCL;

    memset(dct, 0, n * 64 * حجم(*dct));
    LEBI_GET(اندريون);
    طالما (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(اندريون, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        طالما (i > 0) {
            t = DEC_REC(اندريون, hu, r, t);
            إذا (t == 0 && r == 0) {
                dct += i;
                اكسر;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        إذا (n == sc->ايندڙ)
            sc++;
    }
    LEBI_PUT(اندريون);
}

ساكن فراغ dec_makehuff(بنية dec_hufftbl *hu, صحيح *hufflen,
                         بنانشان حرف *huffvals)
{
    صحيح ڪوڊ, k, i, j, d, x, c, v;
    لكل (i = 0; i < (1 << DECBITS); i++)
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

    ڪوڊ = 0;
    k = 0;
    لكل (i = 0; i < 16; i++, ڪوڊ <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        لكل (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            إذا (i < DECBITS) {
                c = ڪوڊ << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /*سائيز*/
                لكل (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    إذا (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        إذا (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } وإلا
                        x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1));
                    hu->llvals[c | d] = x;
                }
            }
            ڪوڊ++;
            k++;
        }
        hu->maxcode[i] = ڪوڊ;
    }
    hu->maxcode[16] = 0x20000;        /* always terminate decode */
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

#وصف_ڪريو ONE ((PREC)IFIX(1.))
#وصف_ڪريو S2  ((PREC)IFIX(0.382683432))
#وصف_ڪريو C2  ((PREC)IFIX(0.923879532))
#وصف_ڪريو C4  ((PREC)IFIX(0.707106781))

#وصف_ڪريو S22 ((PREC)IFIX(2 * 0.382683432))
#وصف_ڪريو C22 ((PREC)IFIX(2 * 0.923879532))
#وصف_ڪريو IC4 ((PREC)IFIX(1 / 0.707106781))

#وصف_ڪريو C3IC1 ((PREC)IFIX(0.847759065))        /* c3/c1 */
#وصف_ڪريو C5IC1 ((PREC)IFIX(0.566454497))        /* c5/c1 */
#وصف_ڪريو C7IC1 ((PREC)IFIX(0.198912367))        /* c7/c1 */

#وصف_ڪريو XPP(a,b) (t = a + b, b = a - b, a = t)
#وصف_ڪريو XMP(a,b) (t = a - b, b = a + b, a = t)
#وصف_ڪريو XPM(a,b) (t = a + b, b = b - a, a = t)

#وصف_ڪريو ROT(a,b,s,c) (  t = IMULT(a + b, s),      \
                        a = IMULT(a, c - s) + t,  \
                        b = IMULT(b, c + s) - t)

#وصف_ڪريو IDCT                \
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

ساكن بنانشان حرف zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

ساكن فراغ idct(صحيح *اندريون, صحيح *ٻاهريون, PREC * quant, PREC off, صحيح max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    صحيح i, j;
    بنانشان حرف *zig2p;

    t0 = off;
    إذا (max == 1) {
        t0 += اندريون[0] * quant[0];
        لكل (i = 0; i < 64; i++)
            ٻاهريون[i] = ITOINT(t0);
        أرجع;
    }
    zig2p = zig2;
    tmpp = tmp;
    لكل (i = 0; i < 8; i++) {
        j = *zig2p++;
        t0 += اندريون[j] * quant[j];
        j = *zig2p++;
        t5 = اندريون[j] * quant[j];
        j = *zig2p++;
        t2 = اندريون[j] * quant[j];
        j = *zig2p++;
        t7 = اندريون[j] * quant[j];
        j = *zig2p++;
        t1 = اندريون[j] * quant[j];
        j = *zig2p++;
        t4 = اندريون[j] * quant[j];
        j = *zig2p++;
        t3 = اندريون[j] * quant[j];
        j = *zig2p++;
        t6 = اندريون[j] * quant[j];
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
    لكل (i = 0; i < 8; i++) {
        t0 = tmp[8 * i + 0];
        t1 = tmp[8 * i + 1];
        t2 = tmp[8 * i + 2];
        t3 = tmp[8 * i + 3];
        t4 = tmp[8 * i + 4];
        t5 = tmp[8 * i + 5];
        t6 = tmp[8 * i + 6];
        t7 = tmp[8 * i + 7];
        IDCT;
        ٻاهريون[8 * i + 0] = ITOINT(t0);
        ٻاهريون[8 * i + 1] = ITOINT(t1);
        ٻاهريون[8 * i + 2] = ITOINT(t2);
        ٻاهريون[8 * i + 3] = ITOINT(t3);
        ٻاهريون[8 * i + 4] = ITOINT(t4);
        ٻاهريون[8 * i + 5] = ITOINT(t5);
        ٻاهريون[8 * i + 6] = ITOINT(t6);
        ٻاهريون[8 * i + 7] = ITOINT(t7);
    }
}

ساكن بنانشان حرف zig[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

ساكن PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};


ساكن فراغ idctqtab(بنانشان حرف *qin, PREC * qout)
{
    صحيح i, j;

    لكل (i = 0; i < 8; i++)
        لكل (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

ساكن فراغ scaleidctqtab(PREC * q, PREC sc)
{
    صحيح i;

    لكل (i = 0; i < 64; i++)
        q[i] = IMULT(q[i], sc);
}

/****************************************************************/
/**************          color decoder            ***************/
/****************************************************************/

#وصف_ڪريو ڦيرو

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

ساكن فراغ initcol(PREC q[][64])
{
    scaleidctqtab(q[1], IFIX(1.77200));
    scaleidctqtab(q[2], IFIX(1.40200));
}

/* This is optimized for the stupid sun SUNWspro compiler. */
#وصف_ڪريو STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (بنانشان صحيح)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#وصف_ڪريو CLAMP(x) ((بنانشان صحيح)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#جيڪڏهن_وصف_ٿيل ڦيرو

#وصف_ڪريو CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#وإلا

#وصف_ڪريو CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (3 * cb + 8 * cr) >> 4                    \
)

#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __LITTLE_ENDIAN
#وصف_ڪريو PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 2], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 0], y + cb)          \
)
#وإلا
#وصف_ڪريو PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 0], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 2], y + cb)          \
)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __LITTLE_ENDIAN
#وصف_ڪريو PIC_16(yin, xin, p, xout, جوڙيو)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + جوڙيو*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + جوڙيو    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + جوڙيو*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y & 0xff,                  \
  p[(xout) * 2 + 1] = y >> 8                     \
)
#وإلا
#جيڪڏهن_وصف_ٿيل CONFIG_PPC
#وصف_ڪريو PIC_16(yin, xin, p, xout, جوڙيو)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + جوڙيو*2+1) & 0xf8) <<  7) | \
      ((CLAMP(y - cg + جوڙيو*2+1) & 0xf8) <<  2) | \
      ((CLAMP(y + cb + جوڙيو*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#وإلا
#وصف_ڪريو PIC_16(yin, xin, p, xout, جوڙيو)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + جوڙيو*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + جوڙيو    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + جوڙيو*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#جيڪڏهن_پورو
#جيڪڏهن_پورو

#وصف_ڪريو PIC_32(yin, xin, p, xout)               \
(                                               \
  y = outy[(yin) * 8 + xin],                    \
  STORECLAMP(p[(xout) * 4 + 0], y + cr),        \
  STORECLAMP(p[(xout) * 4 + 1], y - cg),        \
  STORECLAMP(p[(xout) * 4 + 2], y + cb),        \
  p[(xout) * 4 + 3] = 0                         \
)

#وصف_ڪريو PIC221111(xin)                                              \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)        \
)

#وصف_ڪريو PIC221111_16(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0, 3), \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1, 0), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0, 1), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1, 2)  \
)

#وصف_ڪريو PIC221111_32(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)     \
)

ساكن فراغ col221111(صحيح *ٻاهريون, بنانشان حرف *pic, صحيح ويڪر)
{
    صحيح i, j, k;
    بنانشان حرف *pic0, *pic1;
    صحيح *outy, *outc;
    صحيح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ويڪر;
    outy = ٻاهريون;
    outc = ٻاهريون + 64 * 4;
    لكل (i = 2; i > 0; i--) {
        لكل (j = 4; j > 0; j--) {
            لكل (k = 0; k < 8; k++) {
                PIC221111(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ويڪر;
            pic1 += 2 * ويڪر;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ساكن فراغ col221111_16(صحيح *ٻاهريون, بنانشان حرف *pic, صحيح ويڪر)
{
    صحيح i, j, k;
    بنانشان حرف *pic0, *pic1;
    صحيح *outy, *outc;
    صحيح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ويڪر;
    outy = ٻاهريون;
    outc = ٻاهريون + 64 * 4;
    لكل (i = 2; i > 0; i--) {
        لكل (j = 4; j > 0; j--) {
            لكل (k = 0; k < 8; k++) {
                PIC221111_16(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ويڪر;
            pic1 += 2 * ويڪر;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ساكن فراغ col221111_32(صحيح *ٻاهريون, بنانشان حرف *pic, صحيح ويڪر)
{
    صحيح i, j, k;
    بنانشان حرف *pic0, *pic1;
    صحيح *outy, *outc;
    صحيح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ويڪر;
    outy = ٻاهريون;
    outc = ٻاهريون + 64 * 4;
    لكل (i = 2; i > 0; i--) {
        لكل (j = 4; j > 0; j--) {
            لكل (k = 0; k < 8; k++) {
                PIC221111_32(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ويڪر;
            pic1 += 2 * ويڪر;
        }
        outy += 64 * 2 - 16 * 4;
    }
}
