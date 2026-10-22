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

#define __LITTLE_ENDIAN
#ضمن "malloc.h"
#ضمن "string.h"
#ضمن "util.h"
#define ISHIFT 11

#define IFIX(a) ((صحيح)((a) * (1 << ISHIFT) + .5))
#define IMULT(a, b) (((a) * (b)) >> ISHIFT)
#define ITOINT(a) ((a) >> ISHIFT)

#ifndef __P
# define __P(x) x
#endif

/* special markers */
#define M_BADHUFF        -1
#define M_EOF                0x80

بنية in {
    unsigned حرف *p;
    unsigned صحيح bits;
    صحيح left;
    صحيح marker;

    صحيح (*func) __P((فراغ *));
    فراغ *data;
};

/*********************************/
بنية dec_hufftbl;
بنية enc_hufftbl;

اتحاد hufftblp {
    بنية dec_hufftbl *dhuff;
    بنية enc_hufftbl *ehuff;
};

بنية scan {
    صحيح dc;               /* old dc value */

    اتحاد hufftblp hudc;
    اتحاد hufftblp huac;
    صحيح next;             /* when to switch to next scan */

    صحيح cid;              /* component id */
    صحيح hv;               /* horiz/vert, copied from comp */
    صحيح tq;               /* quant tbl, copied from comp */
};

/*********************************/

#define DECBITS 10        /* seems to be the optimum */

بنية dec_hufftbl {
    صحيح maxcode[17];
    صحيح valptr[16];
    unsigned حرف vals[256];
    unsigned صحيح llvals[1 << DECBITS];
};

ساكن فراغ decode_mcus __P((بنية in *, صحيح *, صحيح, بنية scan *, صحيح *));
ساكن صحيح dec_readmarker __P((بنية in *));
ساكن فراغ dec_makehuff __P((بنية dec_hufftbl *, صحيح *, unsigned حرف *));

ساكن فراغ setinput __P((بنية in *, unsigned حرف *));
/*********************************/

#undef PREC
#define PREC صحيح

ساكن فراغ idctqtab __P((unsigned حرف *, PREC *));
ساكن فراغ idct __P((صحيح *, صحيح *, PREC *, PREC, صحيح));
ساكن فراغ scaleidctqtab __P((PREC *, PREC));

/*********************************/

ساكن فراغ initcol __P((PREC[][64]));

ساكن فراغ col221111 __P((صحيح *, unsigned حرف *, صحيح));
ساكن فراغ col221111_16 __P((صحيح *, unsigned حرف *, صحيح));
ساكن فراغ col221111_32 __P((صحيح *, unsigned حرف *, صحيح));

/*********************************/

#define ERR_NO_SOI 1
#define ERR_NOT_8BIT 2
#define ERR_HEIGHT_MISMATCH 3
#define ERR_WIDTH_MISMATCH 4
#define ERR_BAD_WIDTH_OR_HEIGHT 5
#define ERR_TOO_MANY_COMPPS 6
#define ERR_ILLEGAL_HV 7
#define ERR_QUANT_TABLE_SELECTOR 8
#define ERR_NOT_YCBCR_221111 9
#define ERR_UNKNOWN_CID_IN_SCAN 10
#define ERR_NOT_SEQUENTIAL_DCT 11
#define ERR_WRONG_MARKER 12
#define ERR_NO_EOI 13
#define ERR_BAD_TABLES 14
#define ERR_DEPTH_MISMATCH 15

/*********************************/

#define M_SOI   0xd8
#define M_APP0  0xe0
#define M_DQT   0xdb
#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_DRI   0xdd
#define M_SOS   0xda
#define M_RST0  0xd0
#define M_EOI   0xd9
#define M_COM   0xfe

بنية comp {
    صحيح cid;
    صحيح hv;
    صحيح tq;
};

#define MAXCOMP 4
بنية jpginfo {
    صحيح nc;   /* number of components */
    صحيح ns;   /* number of scans */
    صحيح dri;  /* restart interval */
    صحيح nm;   /* mcus til next marker */
    صحيح rm;   /* next restart marker */
};

بنية jpeg_decdata {
    صحيح dcts[6 * 64 + 16];
    صحيح out[64 * 6];
    صحيح dquant[3][64];

    unsigned حرف *datap;
    بنية jpginfo info;
    بنية comp comps[MAXCOMP];
    بنية scan dscans[MAXCOMP];
    unsigned حرف quant[4][64];
    بنية dec_hufftbl dhuff[4];
    بنية in in;

    صحيح height, width;
};

ساكن صحيح getbyte(بنية jpeg_decdata *jpeg)
{
    החזר *jpeg->datap++;
}

ساكن صحيح getword(بنية jpeg_decdata *jpeg)
{
    صحيح c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    החזר c1 << 8 | c2;
}

ساكن صحيح readtables(بنية jpeg_decdata *jpeg, صحيح till)
{
    صحيح m, l, i, j, lq, pq, tq;
    صحيح tc, th, tt;

    עבור (;;) {
        אם (getbyte(jpeg) != 0xff)
            החזר -1;
        אם ((m = getbyte(jpeg)) == till)
            اكسر;

        بدل (m) {
        حالة 0xc2:
            החזר 0;

        حالة M_DQT:
            lq = getword(jpeg);
            כלעוד (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                אם (tq > 3)
                    החזר -1;
                pq >>= 4;
                אם (pq != 0)
                    החזר -1;
                עבור (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            اكسر;

        حالة M_DHT:
            l = getword(jpeg);
            כלעוד (l > 2) {
                صحيح hufflen[16], k;
                unsigned حرف huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                אם (tc > 1 || th > 1)
                    החזר -1;
                עבור (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                עבור (i = 0; i < 16; i++) {
                    עבור (j = 0; j < hufflen[i]; j++)
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
            כלעוד (l-- > 2)
                getbyte(jpeg);
            اكسر;
        }
    }
    החזר 0;
}

ساكن فراغ dec_initscans(بنية jpeg_decdata *jpeg)
{
    صحيح i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    עבור (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

ساكن صحيح dec_checkmarker(بنية jpeg_decdata *jpeg)
{
    صحيح i;

    אם (dec_readmarker(&jpeg->in) != jpeg->info.rm)
        החזר -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    עבור (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    החזר 0;
}

بنية jpeg_decdata *jpeg_alloc(فراغ)
{
    بنية jpeg_decdata *jpeg = malloc_tmphigh(حجم(*jpeg));
    החזר jpeg;
}

صحيح jpeg_decode(بنية jpeg_decdata *jpeg, unsigned حرف *buf)
{
    صحيح i, j, m, tac, tdc;

    אם (!jpeg || !buf)
        החזר -1;
    jpeg->datap = buf;
    אם (getbyte(jpeg) != 0xff)
        החזר ERR_NO_SOI;
    אם (getbyte(jpeg) != M_SOI)
        החזר ERR_NO_SOI;
    אם (readtables(jpeg, M_SOF0))
        החזר ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    אם (i != 8)
        החזר ERR_NOT_8BIT;
    jpeg->height = getword(jpeg);
    jpeg->width = getword(jpeg);
    אם ((jpeg->height & 15) || (jpeg->width & 15))
        החזר ERR_BAD_WIDTH_OR_HEIGHT;
    jpeg->info.nc = getbyte(jpeg);
    אם (jpeg->info.nc > MAXCOMP)
        החזר ERR_TOO_MANY_COMPPS;
    עבור (i = 0; i < jpeg->info.nc; i++) {
        صحيح h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        אם (h > 3 || v > 3)
            החזר ERR_ILLEGAL_HV;
        אם (jpeg->comps[i].tq > 3)
            החזר ERR_QUANT_TABLE_SELECTOR;
    }
    אם (readtables(jpeg, M_SOS))
        החזר ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    אם (jpeg->info.ns != 3)
        החזר ERR_NOT_YCBCR_221111;
    עבור (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        אם (tdc > 1 || tac > 1)
            החזר ERR_QUANT_TABLE_SELECTOR;
        עבור (j = 0; j < jpeg->info.nc; j++)
            אם (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                اكسر;
        אם (j == jpeg->info.nc)
            החזר ERR_UNKNOWN_CID_IN_SCAN;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    אם (i != 0 || j != 63 || m != 0)
        החזר ERR_NOT_SEQUENTIAL_DCT;

    אם (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        החזר ERR_NOT_YCBCR_221111;

    אם (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        החזר ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->in, jpeg->datap);

#אם 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#endif

    dec_initscans(jpeg);

    החזר 0;
}

فراغ jpeg_get_size(بنية jpeg_decdata *jpeg, صحيح *width, صحيح *height)
{
    *width = jpeg->width;
    *height = jpeg->height;
}

صحيح jpeg_show(بنية jpeg_decdata *jpeg, unsigned حرف *pic, صحيح width
              , صحيح height, صحيح depth, صحيح bytes_per_line_dest)
{
    صحيح m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    صحيح max[6];

    אם (jpeg->height != height)
        החזר ERR_HEIGHT_MISMATCH;
    אם (jpeg->width != width)
        החזר ERR_WIDTH_MISMATCH;

    jpgbpl = width * depth / 8;
    mloffset = bytes_per_line_dest > jpgbpl ? bytes_per_line_dest : jpgbpl;

    mcusx = jpeg->width >> 4;
    mcusy = jpeg->height >> 4;

    jpeg->dscans[0].next = 6 - 4;
    jpeg->dscans[1].next = 6 - 4 - 1;
    jpeg->dscans[2].next = 6 - 4 - 1 - 1;        /* 411 encoding */
    עבור (my = 0; my < mcusy; my++) {
        עבור (mx = 0; mx < mcusx; mx++) {
            אם (jpeg->info.dri && !--jpeg->info.nm)
                אם (dec_checkmarker(jpeg))
                    החזר ERR_WRONG_MARKER;

            decode_mcus(&jpeg->in, jpeg->dcts, 6, jpeg->dscans, max);
            idct(jpeg->dcts, jpeg->out, jpeg->dquant[0],
                 IFIX(128.5), max[0]);
            idct(jpeg->dcts + 64, jpeg->out + 64, jpeg->dquant[0],
                 IFIX(128.5), max[1]);
            idct(jpeg->dcts + 128, jpeg->out + 128, jpeg->dquant[0],
                 IFIX(128.5), max[2]);
            idct(jpeg->dcts + 192, jpeg->out + 192, jpeg->dquant[0],
                 IFIX(128.5), max[3]);
            idct(jpeg->dcts + 256, jpeg->out + 256, jpeg->dquant[1],
                 IFIX(0.5), max[4]);
            idct(jpeg->dcts + 320, jpeg->out + 320, jpeg->dquant[2],
                 IFIX(0.5), max[5]);

            بدل (depth) {
            حالة 32:
                col221111_32(jpeg->out,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                اكسر;
            حالة 24:
                col221111(jpeg->out,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                اكسر;
            حالة 16:
                col221111_16(jpeg->out,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                اكسر;
            افتراضي:
                החזר ERR_DEPTH_MISMATCH;
                اكسر;
            }
        }
    }

    m = dec_readmarker(&jpeg->in);
    אם (m != M_EOI)
        החזר ERR_NO_EOI;

    החזר 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

ساكن صحيح fillbits __P((بنية in *, صحيح, unsigned صحيح));
ساكن صحيح dec_rec2 __P((بنية in *, بنية dec_hufftbl *, صحيح *, صحيح, صحيح));

ساكن فراغ setinput(بنية in *in, unsigned حرف *p)
{
    in->p = p;
    in->left = 0;
    in->bits = 0;
    in->marker = 0;
}

ساكن صحيح fillbits(بنية in *in, صحيح le, unsigned صحيح bi)
{
    صحيح b, m;

    אם (in->marker) {
        אם (le <= 16)
            in->bits = bi << 16, le += 16;
        החזר le;
    }
    כלעוד (le <= 24) {
        b = *in->p++;
        אם (b == 0xff && (m = *in->p++) != 0) {
            אם (m == M_EOF) {
                אם (in->func && (m = in->func(in->data)) == 0)
                    استمر;
            }
            in->marker = m;
            אם (le <= 16)
                bi = bi << 16, le += 16;
            اكسر;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    in->bits = bi;                /* tmp... 2 return values needed */
    החזר le;
}

ساكن صحيح dec_readmarker(بنية in *in)
{
    صحيح m;

    in->left = fillbits(in, in->left, in->bits);
    אם ((m = in->marker) == 0)
        החזר 0;
    in->left = 0;
    in->marker = 0;
    החזר m;
}

#define LEBI_DCL       صحيح le, bi
#define LEBI_GET(in)   (le = in->left, bi = in->bits)
#define LEBI_PUT(in)   (in->left = le, in->bits = bi)

#define GETBITS(in, n) (                                     \
  (le < (n) ? le = fillbits(in, le, bi), bi = in->bits : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#define UNGETBITS(in, n) ( \
  le += (n)                \
)


ساكن صحيح dec_rec2(بنية in *in, بنية dec_hufftbl *hu, صحيح *runp,
                    صحيح c, صحيح i)
{
    LEBI_DCL;

    LEBI_GET(in);
    אם (i) {
        UNGETBITS(in, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } אחרת {
        עבור (i = DECBITS;
             (c = ((c << 1) | GETBITS(in, 1))) >= (hu->maxcode[i]); i++);
        אם (i >= 16) {
            in->marker = M_BADHUFF;
            החזר 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    אם (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(in);
        החזר 0;
    }
    /* receive part */
    c = GETBITS(in, i);
    אם (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(in);
    החזר c;
}

#define DEC_REC(in, hu, r, i)         (  \
  r = GETBITS(in, DECBITS),              \
  i = hu->llvals[r],                     \
  i & 128 ?                              \
    (                                    \
      UNGETBITS(in, i & 127),            \
      r = i >> 8 & 15,                   \
      i >> 16                            \
    )                                    \
  :                                      \
    (                                    \
      LEBI_PUT(in),                      \
      i = dec_rec2(in, hu, &r, r, i),    \
      LEBI_GET(in),                      \
      i                                  \
    )                                    \
)

ساكن فراغ decode_mcus(بنية in *in, صحيح *dct, صحيح n, بنية scan *sc,
                        صحيح *maxp)
{
    بنية dec_hufftbl *hu;
    صحيح i, r, t;
    LEBI_DCL;

    memset(dct, 0, n * 64 * حجم(*dct));
    LEBI_GET(in);
    כלעוד (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(in, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        כלעוד (i > 0) {
            t = DEC_REC(in, hu, r, t);
            אם (t == 0 && r == 0) {
                dct += i;
                اكسر;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        אם (n == sc->next)
            sc++;
    }
    LEBI_PUT(in);
}

ساكن فراغ dec_makehuff(بنية dec_hufftbl *hu, صحيح *hufflen,
                         unsigned حرف *huffvals)
{
    صحيح code, k, i, j, d, x, c, v;
    עבור (i = 0; i < (1 << DECBITS); i++)
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

    code = 0;
    k = 0;
    עבור (i = 0; i < 16; i++, code <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        עבור (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            אם (i < DECBITS) {
                c = code << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /* size */
                עבור (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    אם (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        אם (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } אחרת
                        x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1));
                    hu->llvals[c | d] = x;
                }
            }
            code++;
            k++;
        }
        hu->maxcode[i] = code;
    }
    hu->maxcode[16] = 0x20000;        /* always terminate decode */
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

#define ONE ((PREC)IFIX(1.))
#define S2  ((PREC)IFIX(0.382683432))
#define C2  ((PREC)IFIX(0.923879532))
#define C4  ((PREC)IFIX(0.707106781))

#define S22 ((PREC)IFIX(2 * 0.382683432))
#define C22 ((PREC)IFIX(2 * 0.923879532))
#define IC4 ((PREC)IFIX(1 / 0.707106781))

#define C3IC1 ((PREC)IFIX(0.847759065))        /* c3/c1 */
#define C5IC1 ((PREC)IFIX(0.566454497))        /* c5/c1 */
#define C7IC1 ((PREC)IFIX(0.198912367))        /* c7/c1 */

#define XPP(a,b) (t = a + b, b = a - b, a = t)
#define XMP(a,b) (t = a - b, b = a + b, a = t)
#define XPM(a,b) (t = a + b, b = b - a, a = t)

#define ROT(a,b,s,c) (  t = IMULT(a + b, s),      \
                        a = IMULT(a, c - s) + t,  \
                        b = IMULT(b, c + s) - t)

#define IDCT                \
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

ساكن unsigned حرف zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

ساكن فراغ idct(صحيح *in, صحيح *out, PREC * quant, PREC off, صحيح max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    صحيح i, j;
    unsigned حرف *zig2p;

    t0 = off;
    אם (max == 1) {
        t0 += in[0] * quant[0];
        עבור (i = 0; i < 64; i++)
            out[i] = ITOINT(t0);
        החזר;
    }
    zig2p = zig2;
    tmpp = tmp;
    עבור (i = 0; i < 8; i++) {
        j = *zig2p++;
        t0 += in[j] * quant[j];
        j = *zig2p++;
        t5 = in[j] * quant[j];
        j = *zig2p++;
        t2 = in[j] * quant[j];
        j = *zig2p++;
        t7 = in[j] * quant[j];
        j = *zig2p++;
        t1 = in[j] * quant[j];
        j = *zig2p++;
        t4 = in[j] * quant[j];
        j = *zig2p++;
        t3 = in[j] * quant[j];
        j = *zig2p++;
        t6 = in[j] * quant[j];
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
    עבור (i = 0; i < 8; i++) {
        t0 = tmp[8 * i + 0];
        t1 = tmp[8 * i + 1];
        t2 = tmp[8 * i + 2];
        t3 = tmp[8 * i + 3];
        t4 = tmp[8 * i + 4];
        t5 = tmp[8 * i + 5];
        t6 = tmp[8 * i + 6];
        t7 = tmp[8 * i + 7];
        IDCT;
        out[8 * i + 0] = ITOINT(t0);
        out[8 * i + 1] = ITOINT(t1);
        out[8 * i + 2] = ITOINT(t2);
        out[8 * i + 3] = ITOINT(t3);
        out[8 * i + 4] = ITOINT(t4);
        out[8 * i + 5] = ITOINT(t5);
        out[8 * i + 6] = ITOINT(t6);
        out[8 * i + 7] = ITOINT(t7);
    }
}

ساكن unsigned حرف zig[64] = {
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


ساكن فراغ idctqtab(unsigned حرف *qin, PREC * qout)
{
    صحيح i, j;

    עבור (i = 0; i < 8; i++)
        עבור (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

ساكن فراغ scaleidctqtab(PREC * q, PREC sc)
{
    صحيح i;

    עבור (i = 0; i < 64; i++)
        q[i] = IMULT(q[i], sc);
}

/****************************************************************/
/**************          color decoder            ***************/
/****************************************************************/

#define ROUND

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
#define STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (unsigned صحيح)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#define CLAMP(x) ((unsigned صحيح)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#ifdef ROUND

#define CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#אחרת

#define CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (3 * cb + 8 * cr) >> 4                    \
)

#endif

#ifdef __LITTLE_ENDIAN
#define PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 2], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 0], y + cb)          \
)
#אחרת
#define PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 0], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 2], y + cb)          \
)
#endif

#ifdef __LITTLE_ENDIAN
#define PIC_16(yin, xin, p, xout, add)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + add*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + add    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + add*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y & 0xff,                  \
  p[(xout) * 2 + 1] = y >> 8                     \
)
#אחרת
#ifdef CONFIG_PPC
#define PIC_16(yin, xin, p, xout, add)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + add*2+1) & 0xf8) <<  7) | \
      ((CLAMP(y - cg + add*2+1) & 0xf8) <<  2) | \
      ((CLAMP(y + cb + add*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#אחרת
#define PIC_16(yin, xin, p, xout, add)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + add*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + add    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + add*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#endif
#endif

#define PIC_32(yin, xin, p, xout)               \
(                                               \
  y = outy[(yin) * 8 + xin],                    \
  STORECLAMP(p[(xout) * 4 + 0], y + cr),        \
  STORECLAMP(p[(xout) * 4 + 1], y - cg),        \
  STORECLAMP(p[(xout) * 4 + 2], y + cb),        \
  p[(xout) * 4 + 3] = 0                         \
)

#define PIC221111(xin)                                              \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)        \
)

#define PIC221111_16(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0, 3), \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1, 0), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0, 1), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1, 2)  \
)

#define PIC221111_32(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)     \
)

ساكن فراغ col221111(صحيح *out, unsigned حرف *pic, صحيح width)
{
    صحيح i, j, k;
    unsigned حرف *pic0, *pic1;
    صحيح *outy, *outc;
    صحيح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    עבור (i = 2; i > 0; i--) {
        עבור (j = 4; j > 0; j--) {
            עבור (k = 0; k < 8; k++) {
                PIC221111(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * width;
            pic1 += 2 * width;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ساكن فراغ col221111_16(صحيح *out, unsigned حرف *pic, صحيح width)
{
    صحيح i, j, k;
    unsigned حرف *pic0, *pic1;
    صحيح *outy, *outc;
    صحيح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    עבור (i = 2; i > 0; i--) {
        עבור (j = 4; j > 0; j--) {
            עבור (k = 0; k < 8; k++) {
                PIC221111_16(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * width;
            pic1 += 2 * width;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ساكن فراغ col221111_32(صحيح *out, unsigned حرف *pic, صحيح width)
{
    صحيح i, j, k;
    unsigned حرف *pic0, *pic1;
    صحيح *outy, *outc;
    صحيح cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    עבור (i = 2; i > 0; i--) {
        עבור (j = 4; j > 0; j--) {
            עבור (k = 0; k < 8; k++) {
                PIC221111_32(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * width;
            pic1 += 2 * width;
        }
        outy += 64 * 2 - 16 * 4;
    }
}
