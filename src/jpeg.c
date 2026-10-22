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
#अन्तर्भाव "malloc.h"
#अन्तर्भाव "string.h"
#अन्तर्भाव "util.h"
#define ISHIFT 11

#define IFIX(a) ((पूर्णांक)((a) * (1 << ISHIFT) + .5))
#define IMULT(a, b) (((a) * (b)) >> ISHIFT)
#define ITOINT(a) ((a) >> ISHIFT)

#ifndef __P
# define __P(x) x
#endif

/* special markers */
#define M_BADHUFF        -1
#define M_EOF                0x80

संरचना in {
    अचिन्हित अक्षर *p;
    अचिन्हित पूर्णांक bits;
    पूर्णांक डावा;
    पूर्णांक marker;

    पूर्णांक (*func) __P((शून्य *));
    शून्य *data;
};

/*********************************/
संरचना dec_hufftbl;
संरचना enc_hufftbl;

संघ hufftblp {
    संरचना dec_hufftbl *dhuff;
    संरचना enc_hufftbl *ehuff;
};

संरचना scan {
    पूर्णांक dc;               /* old dc value */

    संघ hufftblp hudc;
    संघ hufftblp huac;
    पूर्णांक next;             /* when to switch to next scan */

    पूर्णांक cid;              /* component id */
    पूर्णांक hv;               /* horiz/vert, copied from comp */
    पूर्णांक tq;               /* quant tbl, copied from comp */
};

/*********************************/

#define DECBITS 10        /* seems to be the optimum */

संरचना dec_hufftbl {
    पूर्णांक maxcode[17];
    पूर्णांक valptr[16];
    अचिन्हित अक्षर vals[256];
    अचिन्हित पूर्णांक llvals[1 << DECBITS];
};

स्थिर शून्य decode_mcus __P((संरचना in *, पूर्णांक *, पूर्णांक, संरचना scan *, पूर्णांक *));
स्थिर पूर्णांक dec_readmarker __P((संरचना in *));
स्थिर शून्य dec_makehuff __P((संरचना dec_hufftbl *, पूर्णांक *, अचिन्हित अक्षर *));

स्थिर शून्य setinput __P((संरचना in *, अचिन्हित अक्षर *));
/*********************************/

#undef PREC
#define PREC पूर्णांक

स्थिर शून्य idctqtab __P((अचिन्हित अक्षर *, PREC *));
स्थिर शून्य idct __P((पूर्णांक *, पूर्णांक *, PREC *, PREC, पूर्णांक));
स्थिर शून्य scaleidctqtab __P((PREC *, PREC));

/*********************************/

स्थिर शून्य initcol __P((PREC[][64]));

स्थिर शून्य col221111 __P((पूर्णांक *, अचिन्हित अक्षर *, पूर्णांक));
स्थिर शून्य col221111_16 __P((पूर्णांक *, अचिन्हित अक्षर *, पूर्णांक));
स्थिर शून्य col221111_32 __P((पूर्णांक *, अचिन्हित अक्षर *, पूर्णांक));

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

संरचना comp {
    पूर्णांक cid;
    पूर्णांक hv;
    पूर्णांक tq;
};

#define MAXCOMP 4
संरचना jpginfo {
    पूर्णांक nc;   /* number of components */
    पूर्णांक ns;   /* number of scans */
    पूर्णांक dri;  /* restart interval */
    पूर्णांक nm;   /* mcus til next marker */
    पूर्णांक rm;   /* next restart marker */
};

संरचना jpeg_decdata {
    पूर्णांक dcts[6 * 64 + 16];
    पूर्णांक out[64 * 6];
    पूर्णांक dquant[3][64];

    अचिन्हित अक्षर *datap;
    संरचना jpginfo info;
    संरचना comp comps[MAXCOMP];
    संरचना scan dscans[MAXCOMP];
    अचिन्हित अक्षर quant[4][64];
    संरचना dec_hufftbl dhuff[4];
    संरचना in in;

    पूर्णांक height, width;
};

स्थिर पूर्णांक getbyte(संरचना jpeg_decdata *jpeg)
{
    परत *jpeg->datap++;
}

स्थिर पूर्णांक getword(संरचना jpeg_decdata *jpeg)
{
    पूर्णांक c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    परत c1 << 8 | c2;
}

स्थिर पूर्णांक readtables(संरचना jpeg_decdata *jpeg, पूर्णांक till)
{
    पूर्णांक m, l, i, j, lq, pq, tq;
    पूर्णांक tc, th, tt;

    साठी (;;) {
        जर (getbyte(jpeg) != 0xff)
            परत -1;
        जर ((m = getbyte(jpeg)) == till)
            थांबा;

        निवड (m) {
        स्थिती 0xc2:
            परत 0;

        स्थिती M_DQT:
            lq = getword(jpeg);
            पर्यंत (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                जर (tq > 3)
                    परत -1;
                pq >>= 4;
                जर (pq != 0)
                    परत -1;
                साठी (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            थांबा;

        स्थिती M_DHT:
            l = getword(jpeg);
            पर्यंत (l > 2) {
                पूर्णांक hufflen[16], k;
                अचिन्हित अक्षर huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                जर (tc > 1 || th > 1)
                    परत -1;
                साठी (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                साठी (i = 0; i < 16; i++) {
                    साठी (j = 0; j < hufflen[i]; j++)
                        huffvals[k++] = getbyte(jpeg);
                    l -= hufflen[i];
                }
                dec_makehuff(jpeg->dhuff + tt, hufflen, huffvals);
            }
            थांबा;

        स्थिती M_DRI:
            l = getword(jpeg);
            jpeg->info.dri = getword(jpeg);
            थांबा;

        पूर्वनिर्धारित:
            l = getword(jpeg);
            पर्यंत (l-- > 2)
                getbyte(jpeg);
            थांबा;
        }
    }
    परत 0;
}

स्थिर शून्य dec_initscans(संरचना jpeg_decdata *jpeg)
{
    पूर्णांक i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    साठी (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

स्थिर पूर्णांक dec_checkmarker(संरचना jpeg_decdata *jpeg)
{
    पूर्णांक i;

    जर (dec_readmarker(&jpeg->in) != jpeg->info.rm)
        परत -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    साठी (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    परत 0;
}

संरचना jpeg_decdata *jpeg_alloc(शून्य)
{
    संरचना jpeg_decdata *jpeg = malloc_tmphigh(आकार(*jpeg));
    परत jpeg;
}

पूर्णांक jpeg_decode(संरचना jpeg_decdata *jpeg, अचिन्हित अक्षर *buf)
{
    पूर्णांक i, j, m, tac, tdc;

    जर (!jpeg || !buf)
        परत -1;
    jpeg->datap = buf;
    जर (getbyte(jpeg) != 0xff)
        परत ERR_NO_SOI;
    जर (getbyte(jpeg) != M_SOI)
        परत ERR_NO_SOI;
    जर (readtables(jpeg, M_SOF0))
        परत ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    जर (i != 8)
        परत ERR_NOT_8BIT;
    jpeg->height = getword(jpeg);
    jpeg->width = getword(jpeg);
    जर ((jpeg->height & 15) || (jpeg->width & 15))
        परत ERR_BAD_WIDTH_OR_HEIGHT;
    jpeg->info.nc = getbyte(jpeg);
    जर (jpeg->info.nc > MAXCOMP)
        परत ERR_TOO_MANY_COMPPS;
    साठी (i = 0; i < jpeg->info.nc; i++) {
        पूर्णांक h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        जर (h > 3 || v > 3)
            परत ERR_ILLEGAL_HV;
        जर (jpeg->comps[i].tq > 3)
            परत ERR_QUANT_TABLE_SELECTOR;
    }
    जर (readtables(jpeg, M_SOS))
        परत ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    जर (jpeg->info.ns != 3)
        परत ERR_NOT_YCBCR_221111;
    साठी (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        जर (tdc > 1 || tac > 1)
            परत ERR_QUANT_TABLE_SELECTOR;
        साठी (j = 0; j < jpeg->info.nc; j++)
            जर (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                थांबा;
        जर (j == jpeg->info.nc)
            परत ERR_UNKNOWN_CID_IN_SCAN;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    जर (i != 0 || j != 63 || m != 0)
        परत ERR_NOT_SEQUENTIAL_DCT;

    जर (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        परत ERR_NOT_YCBCR_221111;

    जर (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        परत ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->in, jpeg->datap);

#जर 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#endif

    dec_initscans(jpeg);

    परत 0;
}

शून्य jpeg_get_size(संरचना jpeg_decdata *jpeg, पूर्णांक *width, पूर्णांक *height)
{
    *width = jpeg->width;
    *height = jpeg->height;
}

पूर्णांक jpeg_show(संरचना jpeg_decdata *jpeg, अचिन्हित अक्षर *pic, पूर्णांक width
              , पूर्णांक height, पूर्णांक depth, पूर्णांक bytes_per_ओळ_dest)
{
    पूर्णांक m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    पूर्णांक max[6];

    जर (jpeg->height != height)
        परत ERR_HEIGHT_MISMATCH;
    जर (jpeg->width != width)
        परत ERR_WIDTH_MISMATCH;

    jpgbpl = width * depth / 8;
    mloffset = bytes_per_ओळ_dest > jpgbpl ? bytes_per_ओळ_dest : jpgbpl;

    mcusx = jpeg->width >> 4;
    mcusy = jpeg->height >> 4;

    jpeg->dscans[0].next = 6 - 4;
    jpeg->dscans[1].next = 6 - 4 - 1;
    jpeg->dscans[2].next = 6 - 4 - 1 - 1;        /* 411 encoding */
    साठी (my = 0; my < mcusy; my++) {
        साठी (mx = 0; mx < mcusx; mx++) {
            जर (jpeg->info.dri && !--jpeg->info.nm)
                जर (dec_checkmarker(jpeg))
                    परत ERR_WRONG_MARKER;

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

            निवड (depth) {
            स्थिती 32:
                col221111_32(jpeg->out,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                थांबा;
            स्थिती 24:
                col221111(jpeg->out,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                थांबा;
            स्थिती 16:
                col221111_16(jpeg->out,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                थांबा;
            पूर्वनिर्धारित:
                परत ERR_DEPTH_MISMATCH;
                थांबा;
            }
        }
    }

    m = dec_readmarker(&jpeg->in);
    जर (m != M_EOI)
        परत ERR_NO_EOI;

    परत 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

स्थिर पूर्णांक fillbits __P((संरचना in *, पूर्णांक, अचिन्हित पूर्णांक));
स्थिर पूर्णांक dec_rec2 __P((संरचना in *, संरचना dec_hufftbl *, पूर्णांक *, पूर्णांक, पूर्णांक));

स्थिर शून्य setinput(संरचना in *in, अचिन्हित अक्षर *p)
{
    in->p = p;
    in->डावा = 0;
    in->bits = 0;
    in->marker = 0;
}

स्थिर पूर्णांक fillbits(संरचना in *in, पूर्णांक le, अचिन्हित पूर्णांक bi)
{
    पूर्णांक b, m;

    जर (in->marker) {
        जर (le <= 16)
            in->bits = bi << 16, le += 16;
        परत le;
    }
    पर्यंत (le <= 24) {
        b = *in->p++;
        जर (b == 0xff && (m = *in->p++) != 0) {
            जर (m == M_EOF) {
                जर (in->func && (m = in->func(in->data)) == 0)
                    सुरूठेवा;
            }
            in->marker = m;
            जर (le <= 16)
                bi = bi << 16, le += 16;
            थांबा;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    in->bits = bi;                /* tmp... 2 return values needed */
    परत le;
}

स्थिर पूर्णांक dec_readmarker(संरचना in *in)
{
    पूर्णांक m;

    in->डावा = fillbits(in, in->डावा, in->bits);
    जर ((m = in->marker) == 0)
        परत 0;
    in->डावा = 0;
    in->marker = 0;
    परत m;
}

#define LEBI_DCL       पूर्णांक le, bi
#define LEBI_GET(in)   (le = in->डावा, bi = in->bits)
#define LEBI_PUT(in)   (in->डावा = le, in->bits = bi)

#define GETBITS(in, n) (                                     \
  (le < (n) ? le = fillbits(in, le, bi), bi = in->bits : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#define UNGETBITS(in, n) ( \
  le += (n)                \
)


स्थिर पूर्णांक dec_rec2(संरचना in *in, संरचना dec_hufftbl *hu, पूर्णांक *runp,
                    पूर्णांक c, पूर्णांक i)
{
    LEBI_DCL;

    LEBI_GET(in);
    जर (i) {
        UNGETBITS(in, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } अन्यथा {
        साठी (i = DECBITS;
             (c = ((c << 1) | GETBITS(in, 1))) >= (hu->maxcode[i]); i++);
        जर (i >= 16) {
            in->marker = M_BADHUFF;
            परत 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    जर (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(in);
        परत 0;
    }
    /* receive part */
    c = GETBITS(in, i);
    जर (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(in);
    परत c;
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

स्थिर शून्य decode_mcus(संरचना in *in, पूर्णांक *dct, पूर्णांक n, संरचना scan *sc,
                        पूर्णांक *maxp)
{
    संरचना dec_hufftbl *hu;
    पूर्णांक i, r, t;
    LEBI_DCL;

    स_रखो(dct, 0, n * 64 * आकार(*dct));
    LEBI_GET(in);
    पर्यंत (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(in, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        पर्यंत (i > 0) {
            t = DEC_REC(in, hu, r, t);
            जर (t == 0 && r == 0) {
                dct += i;
                थांबा;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        जर (n == sc->next)
            sc++;
    }
    LEBI_PUT(in);
}

स्थिर शून्य dec_makehuff(संरचना dec_hufftbl *hu, पूर्णांक *hufflen,
                         अचिन्हित अक्षर *huffvals)
{
    पूर्णांक code, k, i, j, d, x, c, v;
    साठी (i = 0; i < (1 << DECBITS); i++)
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
    साठी (i = 0; i < 16; i++, code <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        साठी (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            जर (i < DECBITS) {
                c = code << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /* size */
                साठी (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    जर (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        जर (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } अन्यथा
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

स्थिर अचिन्हित अक्षर zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

स्थिर शून्य idct(पूर्णांक *in, पूर्णांक *out, PREC * quant, PREC off, पूर्णांक max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    पूर्णांक i, j;
    अचिन्हित अक्षर *zig2p;

    t0 = off;
    जर (max == 1) {
        t0 += in[0] * quant[0];
        साठी (i = 0; i < 64; i++)
            out[i] = ITOINT(t0);
        परत;
    }
    zig2p = zig2;
    tmpp = tmp;
    साठी (i = 0; i < 8; i++) {
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
    साठी (i = 0; i < 8; i++) {
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

स्थिर अचिन्हित अक्षर zig[64] = {
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


स्थिर शून्य idctqtab(अचिन्हित अक्षर *qin, PREC * qout)
{
    पूर्णांक i, j;

    साठी (i = 0; i < 8; i++)
        साठी (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

स्थिर शून्य scaleidctqtab(PREC * q, PREC sc)
{
    पूर्णांक i;

    साठी (i = 0; i < 64; i++)
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

स्थिर शून्य initcol(PREC q[][64])
{
    scaleidctqtab(q[1], IFIX(1.77200));
    scaleidctqtab(q[2], IFIX(1.40200));
}

/* This is optimized for the stupid sun SUNWspro compiler. */
#define STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (अचिन्हित पूर्णांक)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#define CLAMP(x) ((अचिन्हित पूर्णांक)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#ifdef ROUND

#define CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#अन्यथा

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
#अन्यथा
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
#अन्यथा
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
#अन्यथा
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

स्थिर शून्य col221111(पूर्णांक *out, अचिन्हित अक्षर *pic, पूर्णांक width)
{
    पूर्णांक i, j, k;
    अचिन्हित अक्षर *pic0, *pic1;
    पूर्णांक *outy, *outc;
    पूर्णांक cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    साठी (i = 2; i > 0; i--) {
        साठी (j = 4; j > 0; j--) {
            साठी (k = 0; k < 8; k++) {
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

स्थिर शून्य col221111_16(पूर्णांक *out, अचिन्हित अक्षर *pic, पूर्णांक width)
{
    पूर्णांक i, j, k;
    अचिन्हित अक्षर *pic0, *pic1;
    पूर्णांक *outy, *outc;
    पूर्णांक cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    साठी (i = 2; i > 0; i--) {
        साठी (j = 4; j > 0; j--) {
            साठी (k = 0; k < 8; k++) {
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

स्थिर शून्य col221111_32(पूर्णांक *out, अचिन्हित अक्षर *pic, पूर्णांक width)
{
    पूर्णांक i, j, k;
    अचिन्हित अक्षर *pic0, *pic1;
    पूर्णांक *outy, *outc;
    पूर्णांक cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    साठी (i = 2; i > 0; i--) {
        साठी (j = 4; j > 0; j--) {
            साठी (k = 0; k < 8; k++) {
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
