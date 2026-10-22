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

#నిర్వచించు __LITTLE_ENDIAN
#అన్తర్భావ "malloc.h"
#అన్తర్భావ "string.h"
#అన్తర్భావ "util.h"
#నిర్వచించు ISHIFT 11

#నిర్వచించు IFIX(a) ((పూర్ణాంకం)((a) * (1 << ISHIFT) + .5))
#నిర్వచించు IMULT(a, b) (((a) * (b)) >> ISHIFT)
#నిర్వచించు ITOINT(a) ((a) >> ISHIFT)

#నిర్వచితం_కాకపోతే __P
# define __P(x) x
#అయితే_ముగింపు

/* special markers */
#నిర్వచించు M_BADHUFF        -1
#నిర్వచించు M_EOF                0x80

నిర్మాణం లోపల {
    గుర్తులేని అక్షరం *p;
    గుర్తులేని పూర్ణాంకం బిట్లు;
    పూర్ణాంకం ఎడమ;
    పూర్ణాంకం marker;

    పూర్ణాంకం (*func) __P((శూన్య *));
    శూన్య *దత్తాంశం;
};

/*********************************/
నిర్మాణం dec_hufftbl;
నిర్మాణం enc_hufftbl;

సంఘం hufftblp {
    నిర్మాణం dec_hufftbl *dhuff;
    నిర్మాణం enc_hufftbl *ehuff;
};

నిర్మాణం పరిశీలన {
    పూర్ణాంకం dc;               /* old dc value */

    సంఘం hufftblp hudc;
    సంఘం hufftblp huac;
    పూర్ణాంకం తదుపరి;             /* when to switch to next scan */

    పూర్ణాంకం cid;              /* component id */
    పూర్ణాంకం hv;               /* horiz/vert, copied from comp */
    పూర్ణాంకం tq;               /* quant tbl, copied from comp */
};

/*********************************/

#నిర్వచించు DECBITS 10        /* seems to be the optimum */

నిర్మాణం dec_hufftbl {
    పూర్ణాంకం maxcode[17];
    పూర్ణాంకం valptr[16];
    గుర్తులేని అక్షరం vals[256];
    గుర్తులేని పూర్ణాంకం llvals[1 << DECBITS];
};

స్థిర శూన్య decode_mcus __P((నిర్మాణం లోపల *, పూర్ణాంకం *, పూర్ణాంకం, నిర్మాణం పరిశీలన *, పూర్ణాంకం *));
స్థిర పూర్ణాంకం dec_readmarker __P((నిర్మాణం లోపల *));
స్థిర శూన్య dec_makehuff __P((నిర్మాణం dec_hufftbl *, పూర్ణాంకం *, గుర్తులేని అక్షరం *));

స్థిర శూన్య setinput __P((నిర్మాణం లోపల *, గుర్తులేని అక్షరం *));
/*********************************/

#నిర్వచనం_రద్దు PREC
#నిర్వచించు PREC పూర్ణాంకం

స్థిర శూన్య idctqtab __P((గుర్తులేని అక్షరం *, PREC *));
స్థిర శూన్య idct __P((పూర్ణాంకం *, పూర్ణాంకం *, PREC *, PREC, పూర్ణాంకం));
స్థిర శూన్య scaleidctqtab __P((PREC *, PREC));

/*********************************/

స్థిర శూన్య initcol __P((PREC[][64]));

స్థిర శూన్య col221111 __P((పూర్ణాంకం *, గుర్తులేని అక్షరం *, పూర్ణాంకం));
స్థిర శూన్య col221111_16 __P((పూర్ణాంకం *, గుర్తులేని అక్షరం *, పూర్ణాంకం));
స్థిర శూన్య col221111_32 __P((పూర్ణాంకం *, గుర్తులేని అక్షరం *, పూర్ణాంకం));

/*********************************/

#నిర్వచించు ERR_NO_SOI 1
#నిర్వచించు ERR_NOT_8_బిట్టు 2
#నిర్వచించు ERR_ఎత్తు_MISMATCH 3
#నిర్వచించు ERR_వెడల్పు_MISMATCH 4
#నిర్వచించు ERR_BAD_వెడల్పు_OR_ఎత్తు 5
#నిర్వచించు ERR_TOO_MANY_COMPPS 6
#నిర్వచించు ERR_ILLEGAL_HV 7
#నిర్వచించు ERR_QUANT_పట్టిక_SELECTOR 8
#నిర్వచించు ERR_NOT_YCBCR_221111 9
#నిర్వచించు ERR_UNKNOWN_CID_లోపల_పరిశీలన 10
#నిర్వచించు ERR_NOT_SEQUENTIAL_DCT 11
#నిర్వచించు ERR_WRONG_MARKER 12
#నిర్వచించు ERR_NO_EOI 13
#నిర్వచించు ERR_BAD_TABLES 14
#నిర్వచించు ERR_లోతు_MISMATCH 15

/*********************************/

#నిర్వచించు M_SOI   0xd8
#నిర్వచించు M_APP0  0xe0
#నిర్వచించు M_DQT   0xdb
#నిర్వచించు M_SOF0  0xc0
#నిర్వచించు M_DHT   0xc4
#నిర్వచించు M_DRI   0xdd
#నిర్వచించు M_SOS   0xda
#నిర్వచించు M_RST0  0xd0
#నిర్వచించు M_EOI   0xd9
#నిర్వచించు M_COM   0xfe

నిర్మాణం comp {
    పూర్ణాంకం cid;
    పూర్ణాంకం hv;
    పూర్ణాంకం tq;
};

#నిర్వచించు MAXCOMP 4
నిర్మాణం jpginfo {
    పూర్ణాంకం nc;   /* number of components */
    పూర్ణాంకం ns;   /* number of scans */
    పూర్ణాంకం dri;  /* restart interval */
    పూర్ణాంకం nm;   /* mcus til next marker */
    పూర్ణాంకం rm;   /* next restart marker */
};

నిర్మాణం jpeg_decdata {
    పూర్ణాంకం dcts[6 * 64 + 16];
    పూర్ణాంకం బహిర్గతం[64 * 6];
    పూర్ణాంకం dquant[3][64];

    గుర్తులేని అక్షరం *datap;
    నిర్మాణం jpginfo info;
    నిర్మాణం comp comps[MAXCOMP];
    నిర్మాణం పరిశీలన dscans[MAXCOMP];
    గుర్తులేని అక్షరం quant[4][64];
    నిర్మాణం dec_hufftbl dhuff[4];
    నిర్మాణం లోపల లోపల;

    పూర్ణాంకం ఎత్తు, వెడల్పు;
};

స్థిర పూర్ణాంకం getbyte(నిర్మాణం jpeg_decdata *jpeg)
{
    తిరిగి *jpeg->datap++;
}

స్థిర పూర్ణాంకం getword(నిర్మాణం jpeg_decdata *jpeg)
{
    పూర్ణాంకం c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    తిరిగి c1 << 8 | c2;
}

స్థిర పూర్ణాంకం readtables(నిర్మాణం jpeg_decdata *jpeg, పూర్ణాంకం till)
{
    పూర్ణాంకం m, l, i, j, lq, pq, tq;
    పూర్ణాంకం tc, th, tt;

    కొరకు (;;) {
        ఒకవేళ (getbyte(jpeg) != 0xff)
            తిరిగి -1;
        ఒకవేళ ((m = getbyte(jpeg)) == till)
            ఆపు;

        ఎంపిక (m) {
        స్థితి 0xc2:
            తిరిగి 0;

        స్థితి M_DQT:
            lq = getword(jpeg);
            అయితే (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                ఒకవేళ (tq > 3)
                    తిరిగి -1;
                pq >>= 4;
                ఒకవేళ (pq != 0)
                    తిరిగి -1;
                కొరకు (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            ఆపు;

        స్థితి M_DHT:
            l = getword(jpeg);
            అయితే (l > 2) {
                పూర్ణాంకం hufflen[16], k;
                గుర్తులేని అక్షరం huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                ఒకవేళ (tc > 1 || th > 1)
                    తిరిగి -1;
                కొరకు (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                కొరకు (i = 0; i < 16; i++) {
                    కొరకు (j = 0; j < hufflen[i]; j++)
                        huffvals[k++] = getbyte(jpeg);
                    l -= hufflen[i];
                }
                dec_makehuff(jpeg->dhuff + tt, hufflen, huffvals);
            }
            ఆపు;

        స్థితి M_DRI:
            l = getword(jpeg);
            jpeg->info.dri = getword(jpeg);
            ఆపు;

        అప్రమేయ:
            l = getword(jpeg);
            అయితే (l-- > 2)
                getbyte(jpeg);
            ఆపు;
        }
    }
    తిరిగి 0;
}

స్థిర శూన్య dec_initscans(నిర్మాణం jpeg_decdata *jpeg)
{
    పూర్ణాంకం i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    కొరకు (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

స్థిర పూర్ణాంకం dec_checkmarker(నిర్మాణం jpeg_decdata *jpeg)
{
    పూర్ణాంకం i;

    ఒకవేళ (dec_readmarker(&jpeg->లోపల) != jpeg->info.rm)
        తిరిగి -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    కొరకు (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    తిరిగి 0;
}

నిర్మాణం jpeg_decdata *jpeg_alloc(శూన్య)
{
    నిర్మాణం jpeg_decdata *jpeg = malloc_tmphigh(పరిమాణం(*jpeg));
    తిరిగి jpeg;
}

పూర్ణాంకం jpeg_decode(నిర్మాణం jpeg_decdata *jpeg, గుర్తులేని అక్షరం *buf)
{
    పూర్ణాంకం i, j, m, tac, tdc;

    ఒకవేళ (!jpeg || !buf)
        తిరిగి -1;
    jpeg->datap = buf;
    ఒకవేళ (getbyte(jpeg) != 0xff)
        తిరిగి ERR_NO_SOI;
    ఒకవేళ (getbyte(jpeg) != M_SOI)
        తిరిగి ERR_NO_SOI;
    ఒకవేళ (readtables(jpeg, M_SOF0))
        తిరిగి ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    ఒకవేళ (i != 8)
        తిరిగి ERR_NOT_8_బిట్టు;
    jpeg->ఎత్తు = getword(jpeg);
    jpeg->వెడల్పు = getword(jpeg);
    ఒకవేళ ((jpeg->ఎత్తు & 15) || (jpeg->వెడల్పు & 15))
        తిరిగి ERR_BAD_వెడల్పు_OR_ఎత్తు;
    jpeg->info.nc = getbyte(jpeg);
    ఒకవేళ (jpeg->info.nc > MAXCOMP)
        తిరిగి ERR_TOO_MANY_COMPPS;
    కొరకు (i = 0; i < jpeg->info.nc; i++) {
        పూర్ణాంకం h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        ఒకవేళ (h > 3 || v > 3)
            తిరిగి ERR_ILLEGAL_HV;
        ఒకవేళ (jpeg->comps[i].tq > 3)
            తిరిగి ERR_QUANT_పట్టిక_SELECTOR;
    }
    ఒకవేళ (readtables(jpeg, M_SOS))
        తిరిగి ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    ఒకవేళ (jpeg->info.ns != 3)
        తిరిగి ERR_NOT_YCBCR_221111;
    కొరకు (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        ఒకవేళ (tdc > 1 || tac > 1)
            తిరిగి ERR_QUANT_పట్టిక_SELECTOR;
        కొరకు (j = 0; j < jpeg->info.nc; j++)
            ఒకవేళ (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                ఆపు;
        ఒకవేళ (j == jpeg->info.nc)
            తిరిగి ERR_UNKNOWN_CID_లోపల_పరిశీలన;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    ఒకవేళ (i != 0 || j != 63 || m != 0)
        తిరిగి ERR_NOT_SEQUENTIAL_DCT;

    ఒకవేళ (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        తిరిగి ERR_NOT_YCBCR_221111;

    ఒకవేళ (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        తిరిగి ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->లోపల, jpeg->datap);

#ఒకవేళ 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#అయితే_ముగింపు

    dec_initscans(jpeg);

    తిరిగి 0;
}

శూన్య jpeg_get_కొలత(నిర్మాణం jpeg_decdata *jpeg, పూర్ణాంకం *వెడల్పు, పూర్ణాంకం *ఎత్తు)
{
    *వెడల్పు = jpeg->వెడల్పు;
    *ఎత్తు = jpeg->ఎత్తు;
}

పూర్ణాంకం jpeg_show(నిర్మాణం jpeg_decdata *jpeg, గుర్తులేని అక్షరం *pic, పూర్ణాంకం వెడల్పు
              , పూర్ణాంకం ఎత్తు, పూర్ణాంకం లోతు, పూర్ణాంకం బైట్లు_చొప్పున_పంక్తి_dest)
{
    పూర్ణాంకం m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    పూర్ణాంకం max[6];

    ఒకవేళ (jpeg->ఎత్తు != ఎత్తు)
        తిరిగి ERR_ఎత్తు_MISMATCH;
    ఒకవేళ (jpeg->వెడల్పు != వెడల్పు)
        తిరిగి ERR_వెడల్పు_MISMATCH;

    jpgbpl = వెడల్పు * లోతు / 8;
    mloffset = బైట్లు_చొప్పున_పంక్తి_dest > jpgbpl ? బైట్లు_చొప్పున_పంక్తి_dest : jpgbpl;

    mcusx = jpeg->వెడల్పు >> 4;
    mcusy = jpeg->ఎత్తు >> 4;

    jpeg->dscans[0].తదుపరి = 6 - 4;
    jpeg->dscans[1].తదుపరి = 6 - 4 - 1;
    jpeg->dscans[2].తదుపరి = 6 - 4 - 1 - 1;        /* 411 encoding */
    కొరకు (my = 0; my < mcusy; my++) {
        కొరకు (mx = 0; mx < mcusx; mx++) {
            ఒకవేళ (jpeg->info.dri && !--jpeg->info.nm)
                ఒకవేళ (dec_checkmarker(jpeg))
                    తిరిగి ERR_WRONG_MARKER;

            decode_mcus(&jpeg->లోపల, jpeg->dcts, 6, jpeg->dscans, max);
            idct(jpeg->dcts, jpeg->బహిర్గతం, jpeg->dquant[0],
                 IFIX(128.5), max[0]);
            idct(jpeg->dcts + 64, jpeg->బహిర్గతం + 64, jpeg->dquant[0],
                 IFIX(128.5), max[1]);
            idct(jpeg->dcts + 128, jpeg->బహిర్గతం + 128, jpeg->dquant[0],
                 IFIX(128.5), max[2]);
            idct(jpeg->dcts + 192, jpeg->బహిర్గతం + 192, jpeg->dquant[0],
                 IFIX(128.5), max[3]);
            idct(jpeg->dcts + 256, jpeg->బహిర్గతం + 256, jpeg->dquant[1],
                 IFIX(0.5), max[4]);
            idct(jpeg->dcts + 320, jpeg->బహిర్గతం + 320, jpeg->dquant[2],
                 IFIX(0.5), max[5]);

            ఎంపిక (లోతు) {
            స్థితి 32:
                col221111_32(jpeg->బహిర్గతం,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                ఆపు;
            స్థితి 24:
                col221111(jpeg->బహిర్గతం,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                ఆపు;
            స్థితి 16:
                col221111_16(jpeg->బహిర్గతం,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                ఆపు;
            అప్రమేయ:
                తిరిగి ERR_లోతు_MISMATCH;
                ఆపు;
            }
        }
    }

    m = dec_readmarker(&jpeg->లోపల);
    ఒకవేళ (m != M_EOI)
        తిరిగి ERR_NO_EOI;

    తిరిగి 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

స్థిర పూర్ణాంకం fillbits __P((నిర్మాణం లోపల *, పూర్ణాంకం, గుర్తులేని పూర్ణాంకం));
స్థిర పూర్ణాంకం dec_rec2 __P((నిర్మాణం లోపల *, నిర్మాణం dec_hufftbl *, పూర్ణాంకం *, పూర్ణాంకం, పూర్ణాంకం));

స్థిర శూన్య setinput(నిర్మాణం లోపల *లోపల, గుర్తులేని అక్షరం *p)
{
    లోపల->p = p;
    లోపల->ఎడమ = 0;
    లోపల->బిట్లు = 0;
    లోపల->marker = 0;
}

స్థిర పూర్ణాంకం fillbits(నిర్మాణం లోపల *లోపల, పూర్ణాంకం le, గుర్తులేని పూర్ణాంకం bi)
{
    పూర్ణాంకం b, m;

    ఒకవేళ (లోపల->marker) {
        ఒకవేళ (le <= 16)
            లోపల->బిట్లు = bi << 16, le += 16;
        తిరిగి le;
    }
    అయితే (le <= 24) {
        b = *లోపల->p++;
        ఒకవేళ (b == 0xff && (m = *లోపల->p++) != 0) {
            ఒకవేళ (m == M_EOF) {
                ఒకవేళ (లోపల->func && (m = లోపల->func(లోపల->దత్తాంశం)) == 0)
                    కొనసాగు;
            }
            లోపల->marker = m;
            ఒకవేళ (le <= 16)
                bi = bi << 16, le += 16;
            ఆపు;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    లోపల->బిట్లు = bi;                /* tmp... 2 return values needed */
    తిరిగి le;
}

స్థిర పూర్ణాంకం dec_readmarker(నిర్మాణం లోపల *లోపల)
{
    పూర్ణాంకం m;

    లోపల->ఎడమ = fillbits(లోపల, లోపల->ఎడమ, లోపల->బిట్లు);
    ఒకవేళ ((m = లోపల->marker) == 0)
        తిరిగి 0;
    లోపల->ఎడమ = 0;
    లోపల->marker = 0;
    తిరిగి m;
}

#నిర్వచించు LEBI_DCL       పూర్ణాంకం le, bi
#నిర్వచించు LEBI_GET(లోపల)   (le = లోపల->ఎడమ, bi = లోపల->బిట్లు)
#నిర్వచించు LEBI_PUT(లోపల)   (లోపల->ఎడమ = le, లోపల->బిట్లు = bi)

#నిర్వచించు GETBITS(లోపల, n) (                                     \
  (le < (n) ? le = fillbits(లోపల, le, bi), bi = లోపల->బిట్లు : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#నిర్వచించు UNGETBITS(లోపల, n) ( \
  le += (n)                \
)


స్థిర పూర్ణాంకం dec_rec2(నిర్మాణం లోపల *లోపల, నిర్మాణం dec_hufftbl *hu, పూర్ణాంకం *runp,
                    పూర్ణాంకం c, పూర్ణాంకం i)
{
    LEBI_DCL;

    LEBI_GET(లోపల);
    ఒకవేళ (i) {
        UNGETBITS(లోపల, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } లేకపోతే {
        కొరకు (i = DECBITS;
             (c = ((c << 1) | GETBITS(లోపల, 1))) >= (hu->maxcode[i]); i++);
        ఒకవేళ (i >= 16) {
            లోపల->marker = M_BADHUFF;
            తిరిగి 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    ఒకవేళ (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(లోపల);
        తిరిగి 0;
    }
    /* receive part */
    c = GETBITS(లోపల, i);
    ఒకవేళ (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(లోపల);
    తిరిగి c;
}

#నిర్వచించు DEC_REC(లోపల, hu, r, i)         (  \
  r = GETBITS(లోపల, DECBITS),              \
  i = hu->llvals[r],                     \
  i & 128 ?                              \
    (                                    \
      UNGETBITS(లోపల, i & 127),            \
      r = i >> 8 & 15,                   \
      i >> 16                            \
    )                                    \
  :                                      \
    (                                    \
      LEBI_PUT(లోపల),                      \
      i = dec_rec2(లోపల, hu, &r, r, i),    \
      LEBI_GET(లోపల),                      \
      i                                  \
    )                                    \
)

స్థిర శూన్య decode_mcus(నిర్మాణం లోపల *లోపల, పూర్ణాంకం *dct, పూర్ణాంకం n, నిర్మాణం పరిశీలన *sc,
                        పూర్ణాంకం *maxp)
{
    నిర్మాణం dec_hufftbl *hu;
    పూర్ణాంకం i, r, t;
    LEBI_DCL;

    memset(dct, 0, n * 64 * పరిమాణం(*dct));
    LEBI_GET(లోపల);
    అయితే (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(లోపల, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        అయితే (i > 0) {
            t = DEC_REC(లోపల, hu, r, t);
            ఒకవేళ (t == 0 && r == 0) {
                dct += i;
                ఆపు;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        ఒకవేళ (n == sc->తదుపరి)
            sc++;
    }
    LEBI_PUT(లోపల);
}

స్థిర శూన్య dec_makehuff(నిర్మాణం dec_hufftbl *hu, పూర్ణాంకం *hufflen,
                         గుర్తులేని అక్షరం *huffvals)
{
    పూర్ణాంకం సంకేతం_2, k, i, j, d, x, c, v;
    కొరకు (i = 0; i < (1 << DECBITS); i++)
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

    సంకేతం_2 = 0;
    k = 0;
    కొరకు (i = 0; i < 16; i++, సంకేతం_2 <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        కొరకు (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            ఒకవేళ (i < DECBITS) {
                c = సంకేతం_2 << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /*కొలత*/
                కొరకు (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    ఒకవేళ (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        ఒకవేళ (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } లేకపోతే
                        x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1));
                    hu->llvals[c | d] = x;
                }
            }
            సంకేతం_2++;
            k++;
        }
        hu->maxcode[i] = సంకేతం_2;
    }
    hu->maxcode[16] = 0x20000;        /* always terminate decode */
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

#నిర్వచించు ONE ((PREC)IFIX(1.))
#నిర్వచించు S2  ((PREC)IFIX(0.382683432))
#నిర్వచించు C2  ((PREC)IFIX(0.923879532))
#నిర్వచించు C4  ((PREC)IFIX(0.707106781))

#నిర్వచించు S22 ((PREC)IFIX(2 * 0.382683432))
#నిర్వచించు C22 ((PREC)IFIX(2 * 0.923879532))
#నిర్వచించు IC4 ((PREC)IFIX(1 / 0.707106781))

#నిర్వచించు C3IC1 ((PREC)IFIX(0.847759065))        /* c3/c1 */
#నిర్వచించు C5IC1 ((PREC)IFIX(0.566454497))        /* c5/c1 */
#నిర్వచించు C7IC1 ((PREC)IFIX(0.198912367))        /* c7/c1 */

#నిర్వచించు XPP(a,b) (t = a + b, b = a - b, a = t)
#నిర్వచించు XMP(a,b) (t = a - b, b = a + b, a = t)
#నిర్వచించు XPM(a,b) (t = a + b, b = b - a, a = t)

#నిర్వచించు ROT(a,b,s,c) (  t = IMULT(a + b, s),      \
                        a = IMULT(a, c - s) + t,  \
                        b = IMULT(b, c + s) - t)

#నిర్వచించు IDCT                \
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

స్థిర గుర్తులేని అక్షరం zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

స్థిర శూన్య idct(పూర్ణాంకం *లోపల, పూర్ణాంకం *బహిర్గతం, PREC * quant, PREC off, పూర్ణాంకం max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    పూర్ణాంకం i, j;
    గుర్తులేని అక్షరం *zig2p;

    t0 = off;
    ఒకవేళ (max == 1) {
        t0 += లోపల[0] * quant[0];
        కొరకు (i = 0; i < 64; i++)
            బహిర్గతం[i] = ITOINT(t0);
        తిరిగి;
    }
    zig2p = zig2;
    tmpp = tmp;
    కొరకు (i = 0; i < 8; i++) {
        j = *zig2p++;
        t0 += లోపల[j] * quant[j];
        j = *zig2p++;
        t5 = లోపల[j] * quant[j];
        j = *zig2p++;
        t2 = లోపల[j] * quant[j];
        j = *zig2p++;
        t7 = లోపల[j] * quant[j];
        j = *zig2p++;
        t1 = లోపల[j] * quant[j];
        j = *zig2p++;
        t4 = లోపల[j] * quant[j];
        j = *zig2p++;
        t3 = లోపల[j] * quant[j];
        j = *zig2p++;
        t6 = లోపల[j] * quant[j];
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
    కొరకు (i = 0; i < 8; i++) {
        t0 = tmp[8 * i + 0];
        t1 = tmp[8 * i + 1];
        t2 = tmp[8 * i + 2];
        t3 = tmp[8 * i + 3];
        t4 = tmp[8 * i + 4];
        t5 = tmp[8 * i + 5];
        t6 = tmp[8 * i + 6];
        t7 = tmp[8 * i + 7];
        IDCT;
        బహిర్గతం[8 * i + 0] = ITOINT(t0);
        బహిర్గతం[8 * i + 1] = ITOINT(t1);
        బహిర్గతం[8 * i + 2] = ITOINT(t2);
        బహిర్గతం[8 * i + 3] = ITOINT(t3);
        బహిర్గతం[8 * i + 4] = ITOINT(t4);
        బహిర్గతం[8 * i + 5] = ITOINT(t5);
        బహిర్గతం[8 * i + 6] = ITOINT(t6);
        బహిర్గతం[8 * i + 7] = ITOINT(t7);
    }
}

స్థిర గుర్తులేని అక్షరం zig[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

స్థిర PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};


స్థిర శూన్య idctqtab(గుర్తులేని అక్షరం *qin, PREC * qout)
{
    పూర్ణాంకం i, j;

    కొరకు (i = 0; i < 8; i++)
        కొరకు (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

స్థిర శూన్య scaleidctqtab(PREC * q, PREC sc)
{
    పూర్ణాంకం i;

    కొరకు (i = 0; i < 64; i++)
        q[i] = IMULT(q[i], sc);
}

/****************************************************************/
/**************          color decoder            ***************/
/****************************************************************/

#నిర్వచించు విడత

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

స్థిర శూన్య initcol(PREC q[][64])
{
    scaleidctqtab(q[1], IFIX(1.77200));
    scaleidctqtab(q[2], IFIX(1.40200));
}

/* This is optimized for the stupid sun SUNWspro compiler. */
#నిర్వచించు STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (గుర్తులేని పూర్ణాంకం)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#నిర్వచించు CLAMP(x) ((గుర్తులేని పూర్ణాంకం)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#నిర్వచితమైతే విడత

#నిర్వచించు CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#లేకపోతే

#నిర్వచించు CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (3 * cb + 8 * cr) >> 4                    \
)

#అయితే_ముగింపు

#నిర్వచితమైతే __LITTLE_ENDIAN
#నిర్వచించు PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 2], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 0], y + cb)          \
)
#లేకపోతే
#నిర్వచించు PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 0], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 2], y + cb)          \
)
#అయితే_ముగింపు

#నిర్వచితమైతే __LITTLE_ENDIAN
#నిర్వచించు PIC_16(yin, xin, p, xout, కూడు)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + కూడు*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + కూడు    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + కూడు*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y & 0xff,                  \
  p[(xout) * 2 + 1] = y >> 8                     \
)
#లేకపోతే
#నిర్వచితమైతే CONFIG_PPC
#నిర్వచించు PIC_16(yin, xin, p, xout, కూడు)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + కూడు*2+1) & 0xf8) <<  7) | \
      ((CLAMP(y - cg + కూడు*2+1) & 0xf8) <<  2) | \
      ((CLAMP(y + cb + కూడు*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#లేకపోతే
#నిర్వచించు PIC_16(yin, xin, p, xout, కూడు)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + కూడు*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + కూడు    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + కూడు*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#అయితే_ముగింపు
#అయితే_ముగింపు

#నిర్వచించు PIC_32(yin, xin, p, xout)               \
(                                               \
  y = outy[(yin) * 8 + xin],                    \
  STORECLAMP(p[(xout) * 4 + 0], y + cr),        \
  STORECLAMP(p[(xout) * 4 + 1], y - cg),        \
  STORECLAMP(p[(xout) * 4 + 2], y + cb),        \
  p[(xout) * 4 + 3] = 0                         \
)

#నిర్వచించు PIC221111(xin)                                              \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)        \
)

#నిర్వచించు PIC221111_16(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0, 3), \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1, 0), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0, 1), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1, 2)  \
)

#నిర్వచించు PIC221111_32(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)     \
)

స్థిర శూన్య col221111(పూర్ణాంకం *బహిర్గతం, గుర్తులేని అక్షరం *pic, పూర్ణాంకం వెడల్పు)
{
    పూర్ణాంకం i, j, k;
    గుర్తులేని అక్షరం *pic0, *pic1;
    పూర్ణాంకం *outy, *outc;
    పూర్ణాంకం cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + వెడల్పు;
    outy = బహిర్గతం;
    outc = బహిర్గతం + 64 * 4;
    కొరకు (i = 2; i > 0; i--) {
        కొరకు (j = 4; j > 0; j--) {
            కొరకు (k = 0; k < 8; k++) {
                PIC221111(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * వెడల్పు;
            pic1 += 2 * వెడల్పు;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

స్థిర శూన్య col221111_16(పూర్ణాంకం *బహిర్గతం, గుర్తులేని అక్షరం *pic, పూర్ణాంకం వెడల్పు)
{
    పూర్ణాంకం i, j, k;
    గుర్తులేని అక్షరం *pic0, *pic1;
    పూర్ణాంకం *outy, *outc;
    పూర్ణాంకం cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + వెడల్పు;
    outy = బహిర్గతం;
    outc = బహిర్గతం + 64 * 4;
    కొరకు (i = 2; i > 0; i--) {
        కొరకు (j = 4; j > 0; j--) {
            కొరకు (k = 0; k < 8; k++) {
                PIC221111_16(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * వెడల్పు;
            pic1 += 2 * వెడల్పు;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

స్థిర శూన్య col221111_32(పూర్ణాంకం *బహిర్గతం, గుర్తులేని అక్షరం *pic, పూర్ణాంకం వెడల్పు)
{
    పూర్ణాంకం i, j, k;
    గుర్తులేని అక్షరం *pic0, *pic1;
    పూర్ణాంకం *outy, *outc;
    పూర్ణాంకం cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + వెడల్పు;
    outy = బహిర్గతం;
    outc = బహిర్గతం + 64 * 4;
    కొరకు (i = 2; i > 0; i--) {
        కొరకు (j = 4; j > 0; j--) {
            కొరకు (k = 0; k < 8; k++) {
                PIC221111_32(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * వెడల్పు;
            pic1 += 2 * వెడల్పు;
        }
        outy += 64 * 2 - 16 * 4;
    }
}
