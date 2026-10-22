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
#അന്തര്ഭാവ "malloc.h"
#അന്തര്ഭാവ "string.h"
#അന്തര്ഭാവ "util.h"
#define ISHIFT 11

#define IFIX(a) ((പൂർണ്ണസംഖ്യ)((a) * (1 << ISHIFT) + .5))
#define IMULT(a, b) (((a) * (b)) >> ISHIFT)
#define ITOINT(a) ((a) >> ISHIFT)

#ifndef __P
# define __P(x) x
#endif

/* special markers */
#define M_BADHUFF        -1
#define M_EOF                0x80

ഘടന in {
    unsigned അക്ഷരം *p;
    unsigned പൂർണ്ണസംഖ്യ bits;
    പൂർണ്ണസംഖ്യ ഇടത്;
    പൂർണ്ണസംഖ്യ marker;

    പൂർണ്ണസംഖ്യ (*func) __P((ശൂന്യം *));
    ശൂന്യം *data;
};

/*********************************/
ഘടന dec_hufftbl;
ഘടന enc_hufftbl;

സംഘം hufftblp {
    ഘടന dec_hufftbl *dhuff;
    ഘടന enc_hufftbl *ehuff;
};

ഘടന scan {
    പൂർണ്ണസംഖ്യ dc;               /* old dc value */

    സംഘം hufftblp hudc;
    സംഘം hufftblp huac;
    പൂർണ്ണസംഖ്യ next;             /* when to switch to next scan */

    പൂർണ്ണസംഖ്യ cid;              /* component id */
    പൂർണ്ണസംഖ്യ hv;               /* horiz/vert, copied from comp */
    പൂർണ്ണസംഖ്യ tq;               /* quant tbl, copied from comp */
};

/*********************************/

#define DECBITS 10        /* seems to be the optimum */

ഘടന dec_hufftbl {
    പൂർണ്ണസംഖ്യ maxcode[17];
    പൂർണ്ണസംഖ്യ valptr[16];
    unsigned അക്ഷരം vals[256];
    unsigned പൂർണ്ണസംഖ്യ llvals[1 << DECBITS];
};

സ്ഥിതം ശൂന്യം decode_mcus __P((ഘടന in *, പൂർണ്ണസംഖ്യ *, പൂർണ്ണസംഖ്യ, ഘടന scan *, പൂർണ്ണസംഖ്യ *));
സ്ഥിതം പൂർണ്ണസംഖ്യ dec_readmarker __P((ഘടന in *));
സ്ഥിതം ശൂന്യം dec_makehuff __P((ഘടന dec_hufftbl *, പൂർണ്ണസംഖ്യ *, unsigned അക്ഷരം *));

സ്ഥിതം ശൂന്യം setinput __P((ഘടന in *, unsigned അക്ഷരം *));
/*********************************/

#undef PREC
#define PREC പൂർണ്ണസംഖ്യ

സ്ഥിതം ശൂന്യം idctqtab __P((unsigned അക്ഷരം *, PREC *));
സ്ഥിതം ശൂന്യം idct __P((പൂർണ്ണസംഖ്യ *, പൂർണ്ണസംഖ്യ *, PREC *, PREC, പൂർണ്ണസംഖ്യ));
സ്ഥിതം ശൂന്യം scaleidctqtab __P((PREC *, PREC));

/*********************************/

സ്ഥിതം ശൂന്യം initcol __P((PREC[][64]));

സ്ഥിതം ശൂന്യം col221111 __P((പൂർണ്ണസംഖ്യ *, unsigned അക്ഷരം *, പൂർണ്ണസംഖ്യ));
സ്ഥിതം ശൂന്യം col221111_16 __P((പൂർണ്ണസംഖ്യ *, unsigned അക്ഷരം *, പൂർണ്ണസംഖ്യ));
സ്ഥിതം ശൂന്യം col221111_32 __P((പൂർണ്ണസംഖ്യ *, unsigned അക്ഷരം *, പൂർണ്ണസംഖ്യ));

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

ഘടന comp {
    പൂർണ്ണസംഖ്യ cid;
    പൂർണ്ണസംഖ്യ hv;
    പൂർണ്ണസംഖ്യ tq;
};

#define MAXCOMP 4
ഘടന jpginfo {
    പൂർണ്ണസംഖ്യ nc;   /* number of components */
    പൂർണ്ണസംഖ്യ ns;   /* number of scans */
    പൂർണ്ണസംഖ്യ dri;  /* restart interval */
    പൂർണ്ണസംഖ്യ nm;   /* mcus til next marker */
    പൂർണ്ണസംഖ്യ rm;   /* next restart marker */
};

ഘടന jpeg_decdata {
    പൂർണ്ണസംഖ്യ dcts[6 * 64 + 16];
    പൂർണ്ണസംഖ്യ out[64 * 6];
    പൂർണ്ണസംഖ്യ dquant[3][64];

    unsigned അക്ഷരം *datap;
    ഘടന jpginfo info;
    ഘടന comp comps[MAXCOMP];
    ഘടന scan dscans[MAXCOMP];
    unsigned അക്ഷരം quant[4][64];
    ഘടന dec_hufftbl dhuff[4];
    ഘടന in in;

    പൂർണ്ണസംഖ്യ height, width;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ getbyte(ഘടന jpeg_decdata *jpeg)
{
    തിരികെ *jpeg->datap++;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ getword(ഘടന jpeg_decdata *jpeg)
{
    പൂർണ്ണസംഖ്യ c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    തിരികെ c1 << 8 | c2;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ readtables(ഘടന jpeg_decdata *jpeg, പൂർണ്ണസംഖ്യ till)
{
    പൂർണ്ണസംഖ്യ m, l, i, j, lq, pq, tq;
    പൂർണ്ണസംഖ്യ tc, th, tt;

    വേണ്ടി (;;) {
        എങ്കിൽ (getbyte(jpeg) != 0xff)
            തിരികെ -1;
        എങ്കിൽ ((m = getbyte(jpeg)) == till)
            നിർത്തുക;

        തിരഞ്ഞെടുപ്പ് (m) {
        അവസ്ഥ 0xc2:
            തിരികെ 0;

        അവസ്ഥ M_DQT:
            lq = getword(jpeg);
            ആകുമ്പോൾ (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                എങ്കിൽ (tq > 3)
                    തിരികെ -1;
                pq >>= 4;
                എങ്കിൽ (pq != 0)
                    തിരികെ -1;
                വേണ്ടി (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            നിർത്തുക;

        അവസ്ഥ M_DHT:
            l = getword(jpeg);
            ആകുമ്പോൾ (l > 2) {
                പൂർണ്ണസംഖ്യ hufflen[16], k;
                unsigned അക്ഷരം huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                എങ്കിൽ (tc > 1 || th > 1)
                    തിരികെ -1;
                വേണ്ടി (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                വേണ്ടി (i = 0; i < 16; i++) {
                    വേണ്ടി (j = 0; j < hufflen[i]; j++)
                        huffvals[k++] = getbyte(jpeg);
                    l -= hufflen[i];
                }
                dec_makehuff(jpeg->dhuff + tt, hufflen, huffvals);
            }
            നിർത്തുക;

        അവസ്ഥ M_DRI:
            l = getword(jpeg);
            jpeg->info.dri = getword(jpeg);
            നിർത്തുക;

        സ്ഥിരസ്ഥിതി:
            l = getword(jpeg);
            ആകുമ്പോൾ (l-- > 2)
                getbyte(jpeg);
            നിർത്തുക;
        }
    }
    തിരികെ 0;
}

സ്ഥിതം ശൂന്യം dec_initscans(ഘടന jpeg_decdata *jpeg)
{
    പൂർണ്ണസംഖ്യ i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    വേണ്ടി (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ dec_checkmarker(ഘടന jpeg_decdata *jpeg)
{
    പൂർണ്ണസംഖ്യ i;

    എങ്കിൽ (dec_readmarker(&jpeg->in) != jpeg->info.rm)
        തിരികെ -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    വേണ്ടി (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    തിരികെ 0;
}

ഘടന jpeg_decdata *jpeg_alloc(ശൂന്യം)
{
    ഘടന jpeg_decdata *jpeg = malloc_tmphigh(വലുപ്പം(*jpeg));
    തിരികെ jpeg;
}

പൂർണ്ണസംഖ്യ jpeg_decode(ഘടന jpeg_decdata *jpeg, unsigned അക്ഷരം *buf)
{
    പൂർണ്ണസംഖ്യ i, j, m, tac, tdc;

    എങ്കിൽ (!jpeg || !buf)
        തിരികെ -1;
    jpeg->datap = buf;
    എങ്കിൽ (getbyte(jpeg) != 0xff)
        തിരികെ ERR_NO_SOI;
    എങ്കിൽ (getbyte(jpeg) != M_SOI)
        തിരികെ ERR_NO_SOI;
    എങ്കിൽ (readtables(jpeg, M_SOF0))
        തിരികെ ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    എങ്കിൽ (i != 8)
        തിരികെ ERR_NOT_8BIT;
    jpeg->height = getword(jpeg);
    jpeg->width = getword(jpeg);
    എങ്കിൽ ((jpeg->height & 15) || (jpeg->width & 15))
        തിരികെ ERR_BAD_WIDTH_OR_HEIGHT;
    jpeg->info.nc = getbyte(jpeg);
    എങ്കിൽ (jpeg->info.nc > MAXCOMP)
        തിരികെ ERR_TOO_MANY_COMPPS;
    വേണ്ടി (i = 0; i < jpeg->info.nc; i++) {
        പൂർണ്ണസംഖ്യ h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        എങ്കിൽ (h > 3 || v > 3)
            തിരികെ ERR_ILLEGAL_HV;
        എങ്കിൽ (jpeg->comps[i].tq > 3)
            തിരികെ ERR_QUANT_TABLE_SELECTOR;
    }
    എങ്കിൽ (readtables(jpeg, M_SOS))
        തിരികെ ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    എങ്കിൽ (jpeg->info.ns != 3)
        തിരികെ ERR_NOT_YCBCR_221111;
    വേണ്ടി (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        എങ്കിൽ (tdc > 1 || tac > 1)
            തിരികെ ERR_QUANT_TABLE_SELECTOR;
        വേണ്ടി (j = 0; j < jpeg->info.nc; j++)
            എങ്കിൽ (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                നിർത്തുക;
        എങ്കിൽ (j == jpeg->info.nc)
            തിരികെ ERR_UNKNOWN_CID_IN_SCAN;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    എങ്കിൽ (i != 0 || j != 63 || m != 0)
        തിരികെ ERR_NOT_SEQUENTIAL_DCT;

    എങ്കിൽ (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        തിരികെ ERR_NOT_YCBCR_221111;

    എങ്കിൽ (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        തിരികെ ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->in, jpeg->datap);

#എങ്കിൽ 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#endif

    dec_initscans(jpeg);

    തിരികെ 0;
}

ശൂന്യം jpeg_get_size(ഘടന jpeg_decdata *jpeg, പൂർണ്ണസംഖ്യ *width, പൂർണ്ണസംഖ്യ *height)
{
    *width = jpeg->width;
    *height = jpeg->height;
}

പൂർണ്ണസംഖ്യ jpeg_show(ഘടന jpeg_decdata *jpeg, unsigned അക്ഷരം *pic, പൂർണ്ണസംഖ്യ width
              , പൂർണ്ണസംഖ്യ height, പൂർണ്ണസംഖ്യ depth, പൂർണ്ണസംഖ്യ bytes_per_വരി_dest)
{
    പൂർണ്ണസംഖ്യ m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    പൂർണ്ണസംഖ്യ max[6];

    എങ്കിൽ (jpeg->height != height)
        തിരികെ ERR_HEIGHT_MISMATCH;
    എങ്കിൽ (jpeg->width != width)
        തിരികെ ERR_WIDTH_MISMATCH;

    jpgbpl = width * depth / 8;
    mloffset = bytes_per_വരി_dest > jpgbpl ? bytes_per_വരി_dest : jpgbpl;

    mcusx = jpeg->width >> 4;
    mcusy = jpeg->height >> 4;

    jpeg->dscans[0].next = 6 - 4;
    jpeg->dscans[1].next = 6 - 4 - 1;
    jpeg->dscans[2].next = 6 - 4 - 1 - 1;        /* 411 encoding */
    വേണ്ടി (my = 0; my < mcusy; my++) {
        വേണ്ടി (mx = 0; mx < mcusx; mx++) {
            എങ്കിൽ (jpeg->info.dri && !--jpeg->info.nm)
                എങ്കിൽ (dec_checkmarker(jpeg))
                    തിരികെ ERR_WRONG_MARKER;

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

            തിരഞ്ഞെടുപ്പ് (depth) {
            അവസ്ഥ 32:
                col221111_32(jpeg->out,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                നിർത്തുക;
            അവസ്ഥ 24:
                col221111(jpeg->out,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                നിർത്തുക;
            അവസ്ഥ 16:
                col221111_16(jpeg->out,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                നിർത്തുക;
            സ്ഥിരസ്ഥിതി:
                തിരികെ ERR_DEPTH_MISMATCH;
                നിർത്തുക;
            }
        }
    }

    m = dec_readmarker(&jpeg->in);
    എങ്കിൽ (m != M_EOI)
        തിരികെ ERR_NO_EOI;

    തിരികെ 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

സ്ഥിതം പൂർണ്ണസംഖ്യ fillbits __P((ഘടന in *, പൂർണ്ണസംഖ്യ, unsigned പൂർണ്ണസംഖ്യ));
സ്ഥിതം പൂർണ്ണസംഖ്യ dec_rec2 __P((ഘടന in *, ഘടന dec_hufftbl *, പൂർണ്ണസംഖ്യ *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ));

സ്ഥിതം ശൂന്യം setinput(ഘടന in *in, unsigned അക്ഷരം *p)
{
    in->p = p;
    in->ഇടത് = 0;
    in->bits = 0;
    in->marker = 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fillbits(ഘടന in *in, പൂർണ്ണസംഖ്യ le, unsigned പൂർണ്ണസംഖ്യ bi)
{
    പൂർണ്ണസംഖ്യ b, m;

    എങ്കിൽ (in->marker) {
        എങ്കിൽ (le <= 16)
            in->bits = bi << 16, le += 16;
        തിരികെ le;
    }
    ആകുമ്പോൾ (le <= 24) {
        b = *in->p++;
        എങ്കിൽ (b == 0xff && (m = *in->p++) != 0) {
            എങ്കിൽ (m == M_EOF) {
                എങ്കിൽ (in->func && (m = in->func(in->data)) == 0)
                    തുടരുക;
            }
            in->marker = m;
            എങ്കിൽ (le <= 16)
                bi = bi << 16, le += 16;
            നിർത്തുക;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    in->bits = bi;                /* tmp... 2 return values needed */
    തിരികെ le;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ dec_readmarker(ഘടന in *in)
{
    പൂർണ്ണസംഖ്യ m;

    in->ഇടത് = fillbits(in, in->ഇടത്, in->bits);
    എങ്കിൽ ((m = in->marker) == 0)
        തിരികെ 0;
    in->ഇടത് = 0;
    in->marker = 0;
    തിരികെ m;
}

#define LEBI_DCL       പൂർണ്ണസംഖ്യ le, bi
#define LEBI_GET(in)   (le = in->ഇടത്, bi = in->bits)
#define LEBI_PUT(in)   (in->ഇടത് = le, in->bits = bi)

#define GETBITS(in, n) (                                     \
  (le < (n) ? le = fillbits(in, le, bi), bi = in->bits : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#define UNGETBITS(in, n) ( \
  le += (n)                \
)


സ്ഥിതം പൂർണ്ണസംഖ്യ dec_rec2(ഘടന in *in, ഘടന dec_hufftbl *hu, പൂർണ്ണസംഖ്യ *runp,
                    പൂർണ്ണസംഖ്യ c, പൂർണ്ണസംഖ്യ i)
{
    LEBI_DCL;

    LEBI_GET(in);
    എങ്കിൽ (i) {
        UNGETBITS(in, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } അല്ലെങ്കിൽ {
        വേണ്ടി (i = DECBITS;
             (c = ((c << 1) | GETBITS(in, 1))) >= (hu->maxcode[i]); i++);
        എങ്കിൽ (i >= 16) {
            in->marker = M_BADHUFF;
            തിരികെ 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    എങ്കിൽ (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(in);
        തിരികെ 0;
    }
    /* receive part */
    c = GETBITS(in, i);
    എങ്കിൽ (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(in);
    തിരികെ c;
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

സ്ഥിതം ശൂന്യം decode_mcus(ഘടന in *in, പൂർണ്ണസംഖ്യ *dct, പൂർണ്ണസംഖ്യ n, ഘടന scan *sc,
                        പൂർണ്ണസംഖ്യ *maxp)
{
    ഘടന dec_hufftbl *hu;
    പൂർണ്ണസംഖ്യ i, r, t;
    LEBI_DCL;

    memset(dct, 0, n * 64 * വലുപ്പം(*dct));
    LEBI_GET(in);
    ആകുമ്പോൾ (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(in, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        ആകുമ്പോൾ (i > 0) {
            t = DEC_REC(in, hu, r, t);
            എങ്കിൽ (t == 0 && r == 0) {
                dct += i;
                നിർത്തുക;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        എങ്കിൽ (n == sc->next)
            sc++;
    }
    LEBI_PUT(in);
}

സ്ഥിതം ശൂന്യം dec_makehuff(ഘടന dec_hufftbl *hu, പൂർണ്ണസംഖ്യ *hufflen,
                         unsigned അക്ഷരം *huffvals)
{
    പൂർണ്ണസംഖ്യ code, k, i, j, d, x, c, v;
    വേണ്ടി (i = 0; i < (1 << DECBITS); i++)
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
    വേണ്ടി (i = 0; i < 16; i++, code <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        വേണ്ടി (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            എങ്കിൽ (i < DECBITS) {
                c = code << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /* size */
                വേണ്ടി (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    എങ്കിൽ (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        എങ്കിൽ (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } അല്ലെങ്കിൽ
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

സ്ഥിതം unsigned അക്ഷരം zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

സ്ഥിതം ശൂന്യം idct(പൂർണ്ണസംഖ്യ *in, പൂർണ്ണസംഖ്യ *out, PREC * quant, PREC off, പൂർണ്ണസംഖ്യ max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    പൂർണ്ണസംഖ്യ i, j;
    unsigned അക്ഷരം *zig2p;

    t0 = off;
    എങ്കിൽ (max == 1) {
        t0 += in[0] * quant[0];
        വേണ്ടി (i = 0; i < 64; i++)
            out[i] = ITOINT(t0);
        തിരികെ;
    }
    zig2p = zig2;
    tmpp = tmp;
    വേണ്ടി (i = 0; i < 8; i++) {
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
    വേണ്ടി (i = 0; i < 8; i++) {
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

സ്ഥിതം unsigned അക്ഷരം zig[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

സ്ഥിതം PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};


സ്ഥിതം ശൂന്യം idctqtab(unsigned അക്ഷരം *qin, PREC * qout)
{
    പൂർണ്ണസംഖ്യ i, j;

    വേണ്ടി (i = 0; i < 8; i++)
        വേണ്ടി (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

സ്ഥിതം ശൂന്യം scaleidctqtab(PREC * q, PREC sc)
{
    പൂർണ്ണസംഖ്യ i;

    വേണ്ടി (i = 0; i < 64; i++)
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

സ്ഥിതം ശൂന്യം initcol(PREC q[][64])
{
    scaleidctqtab(q[1], IFIX(1.77200));
    scaleidctqtab(q[2], IFIX(1.40200));
}

/* This is optimized for the stupid sun SUNWspro compiler. */
#define STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (unsigned പൂർണ്ണസംഖ്യ)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#define CLAMP(x) ((unsigned പൂർണ്ണസംഖ്യ)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#ifdef ROUND

#define CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#അല്ലെങ്കിൽ

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
#അല്ലെങ്കിൽ
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
#അല്ലെങ്കിൽ
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
#അല്ലെങ്കിൽ
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

സ്ഥിതം ശൂന്യം col221111(പൂർണ്ണസംഖ്യ *out, unsigned അക്ഷരം *pic, പൂർണ്ണസംഖ്യ width)
{
    പൂർണ്ണസംഖ്യ i, j, k;
    unsigned അക്ഷരം *pic0, *pic1;
    പൂർണ്ണസംഖ്യ *outy, *outc;
    പൂർണ്ണസംഖ്യ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    വേണ്ടി (i = 2; i > 0; i--) {
        വേണ്ടി (j = 4; j > 0; j--) {
            വേണ്ടി (k = 0; k < 8; k++) {
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

സ്ഥിതം ശൂന്യം col221111_16(പൂർണ്ണസംഖ്യ *out, unsigned അക്ഷരം *pic, പൂർണ്ണസംഖ്യ width)
{
    പൂർണ്ണസംഖ്യ i, j, k;
    unsigned അക്ഷരം *pic0, *pic1;
    പൂർണ്ണസംഖ്യ *outy, *outc;
    പൂർണ്ണസംഖ്യ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    വേണ്ടി (i = 2; i > 0; i--) {
        വേണ്ടി (j = 4; j > 0; j--) {
            വേണ്ടി (k = 0; k < 8; k++) {
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

സ്ഥിതം ശൂന്യം col221111_32(പൂർണ്ണസംഖ്യ *out, unsigned അക്ഷരം *pic, പൂർണ്ണസംഖ്യ width)
{
    പൂർണ്ണസംഖ്യ i, j, k;
    unsigned അക്ഷരം *pic0, *pic1;
    പൂർണ്ണസംഖ്യ *outy, *outc;
    പൂർണ്ണസംഖ്യ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + width;
    outy = out;
    outc = out + 64 * 4;
    വേണ്ടി (i = 2; i > 0; i--) {
        വേണ്ടി (j = 4; j > 0; j--) {
            വേണ്ടി (k = 0; k < 8; k++) {
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
