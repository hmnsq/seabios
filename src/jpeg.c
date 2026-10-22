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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __LITTLE_ENDIAN
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ISHIFT 11

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IFIX(a) ((ਪੂਰਨ)((a) * (1 << ISHIFT) + .5))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IMULT(a, b) (((a) * (b)) >> ISHIFT)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ITOINT(a) ((a) >> ISHIFT)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __P
# define __P(x) x
#ਜੇ_ਅੰਤ

/* special markers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_BADHUFF        -1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_EOF                0x80

ਬਣਤਰ ਵਿੱਚ_2 {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਿੱਟ;
    ਪੂਰਨ ਖੱਬਾ;
    ਪੂਰਨ marker;

    ਪੂਰਨ (*func) __P((ਖਾਲੀ *));
    ਖਾਲੀ *ਅੰਕੜੇ;
};

/*********************************/
ਬਣਤਰ dec_hufftbl;
ਬਣਤਰ enc_hufftbl;

ਸੰਘ hufftblp {
    ਬਣਤਰ dec_hufftbl *dhuff;
    ਬਣਤਰ enc_hufftbl *ehuff;
};

ਬਣਤਰ ਜਾਂਚ {
    ਪੂਰਨ dc;               /* old dc value */

    ਸੰਘ hufftblp hudc;
    ਸੰਘ hufftblp huac;
    ਪੂਰਨ ਅਗਲਾ;             /* when to switch to next scan */

    ਪੂਰਨ cid;              /* component id */
    ਪੂਰਨ hv;               /* horiz/vert, copied from comp */
    ਪੂਰਨ tq;               /* quant tbl, copied from comp */
};

/*********************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DECBITS 10        /* seems to be the optimum */

ਬਣਤਰ dec_hufftbl {
    ਪੂਰਨ maxcode[17];
    ਪੂਰਨ valptr[16];
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ vals[256];
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ llvals[1 << DECBITS];
};

ਸਥਿਰ ਖਾਲੀ decode_mcus __P((ਬਣਤਰ ਵਿੱਚ_2 *, ਪੂਰਨ *, ਪੂਰਨ, ਬਣਤਰ ਜਾਂਚ *, ਪੂਰਨ *));
ਸਥਿਰ ਪੂਰਨ dec_readmarker __P((ਬਣਤਰ ਵਿੱਚ_2 *));
ਸਥਿਰ ਖਾਲੀ dec_makehuff __P((ਬਣਤਰ dec_hufftbl *, ਪੂਰਨ *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *));

ਸਥਿਰ ਖਾਲੀ setinput __P((ਬਣਤਰ ਵਿੱਚ_2 *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *));
/*********************************/

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ PREC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PREC ਪੂਰਨ

ਸਥਿਰ ਖਾਲੀ idctqtab __P((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *, PREC *));
ਸਥਿਰ ਖਾਲੀ idct __P((ਪੂਰਨ *, ਪੂਰਨ *, PREC *, PREC, ਪੂਰਨ));
ਸਥਿਰ ਖਾਲੀ scaleidctqtab __P((PREC *, PREC));

/*********************************/

ਸਥਿਰ ਖਾਲੀ initcol __P((PREC[][64]));

ਸਥਿਰ ਖਾਲੀ col221111 __P((ਪੂਰਨ *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *, ਪੂਰਨ));
ਸਥਿਰ ਖਾਲੀ col221111_16 __P((ਪੂਰਨ *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *, ਪੂਰਨ));
ਸਥਿਰ ਖਾਲੀ col221111_32 __P((ਪੂਰਨ *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *, ਪੂਰਨ));

/*********************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_NO_SOI 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_NOT_8_ਦੋਆਧਾਰੀ_ਅੰਕ 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_ਉਚਾਈ_MISMATCH 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_ਚੌੜਾਈ_MISMATCH 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_BAD_ਚੌੜਾਈ_OR_ਉਚਾਈ 5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_TOO_MANY_COMPPS 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_ILLEGAL_HV 7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_QUANT_ਸਾਰਣੀ_SELECTOR 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_NOT_YCBCR_221111 9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_UNKNOWN_CID_ਵਿੱਚ_ਜਾਂਚ 10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_NOT_SEQUENTIAL_DCT 11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_WRONG_MARKER 12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_NO_EOI 13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_BAD_TABLES 14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_ਡੂੰਘਾਈ_MISMATCH 15

/*********************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_SOI   0xd8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_APP0  0xe0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_DQT   0xdb
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_SOF0  0xc0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_DHT   0xc4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_DRI   0xdd
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_SOS   0xda
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_RST0  0xd0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_EOI   0xd9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_COM   0xfe

ਬਣਤਰ comp {
    ਪੂਰਨ cid;
    ਪੂਰਨ hv;
    ਪੂਰਨ tq;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXCOMP 4
ਬਣਤਰ jpginfo {
    ਪੂਰਨ nc;   /* number of components */
    ਪੂਰਨ ns;   /* number of scans */
    ਪੂਰਨ dri;  /* restart interval */
    ਪੂਰਨ nm;   /* mcus til next marker */
    ਪੂਰਨ rm;   /* next restart marker */
};

ਬਣਤਰ jpeg_decdata {
    ਪੂਰਨ dcts[6 * 64 + 16];
    ਪੂਰਨ ਬਾਹਰਲਾ[64 * 6];
    ਪੂਰਨ dquant[3][64];

    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *datap;
    ਬਣਤਰ jpginfo info;
    ਬਣਤਰ comp comps[MAXCOMP];
    ਬਣਤਰ ਜਾਂਚ dscans[MAXCOMP];
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ quant[4][64];
    ਬਣਤਰ dec_hufftbl dhuff[4];
    ਬਣਤਰ ਵਿੱਚ_2 ਵਿੱਚ_2;

    ਪੂਰਨ ਉਚਾਈ, ਚੌੜਾਈ;
};

ਸਥਿਰ ਪੂਰਨ getbyte(ਬਣਤਰ jpeg_decdata *jpeg)
{
    ਵਾਪਸ *jpeg->datap++;
}

ਸਥਿਰ ਪੂਰਨ getword(ਬਣਤਰ jpeg_decdata *jpeg)
{
    ਪੂਰਨ c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    ਵਾਪਸ c1 << 8 | c2;
}

ਸਥਿਰ ਪੂਰਨ readtables(ਬਣਤਰ jpeg_decdata *jpeg, ਪੂਰਨ till)
{
    ਪੂਰਨ m, l, i, j, lq, pq, tq;
    ਪੂਰਨ tc, th, tt;

    ਲਈ (;;) {
        ਜੇ (getbyte(jpeg) != 0xff)
            ਵਾਪਸ -1;
        ਜੇ ((m = getbyte(jpeg)) == till)
            ਰੋਕੋ;

        ਚੋਣ (m) {
        ਹਾਲਤ 0xc2:
            ਵਾਪਸ 0;

        ਹਾਲਤ M_DQT:
            lq = getword(jpeg);
            ਜਦੋਂ (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                ਜੇ (tq > 3)
                    ਵਾਪਸ -1;
                pq >>= 4;
                ਜੇ (pq != 0)
                    ਵਾਪਸ -1;
                ਲਈ (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            ਰੋਕੋ;

        ਹਾਲਤ M_DHT:
            l = getword(jpeg);
            ਜਦੋਂ (l > 2) {
                ਪੂਰਨ hufflen[16], k;
                ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                ਜੇ (tc > 1 || th > 1)
                    ਵਾਪਸ -1;
                ਲਈ (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                ਲਈ (i = 0; i < 16; i++) {
                    ਲਈ (j = 0; j < hufflen[i]; j++)
                        huffvals[k++] = getbyte(jpeg);
                    l -= hufflen[i];
                }
                dec_makehuff(jpeg->dhuff + tt, hufflen, huffvals);
            }
            ਰੋਕੋ;

        ਹਾਲਤ M_DRI:
            l = getword(jpeg);
            jpeg->info.dri = getword(jpeg);
            ਰੋਕੋ;

        ਮੂਲ:
            l = getword(jpeg);
            ਜਦੋਂ (l-- > 2)
                getbyte(jpeg);
            ਰੋਕੋ;
        }
    }
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ dec_initscans(ਬਣਤਰ jpeg_decdata *jpeg)
{
    ਪੂਰਨ i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    ਲਈ (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

ਸਥਿਰ ਪੂਰਨ dec_checkmarker(ਬਣਤਰ jpeg_decdata *jpeg)
{
    ਪੂਰਨ i;

    ਜੇ (dec_readmarker(&jpeg->ਵਿੱਚ_2) != jpeg->info.rm)
        ਵਾਪਸ -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    ਲਈ (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    ਵਾਪਸ 0;
}

ਬਣਤਰ jpeg_decdata *jpeg_alloc(ਖਾਲੀ)
{
    ਬਣਤਰ jpeg_decdata *jpeg = malloc_tmphigh(ਆਕਾਰ(*jpeg));
    ਵਾਪਸ jpeg;
}

ਪੂਰਨ jpeg_decode(ਬਣਤਰ jpeg_decdata *jpeg, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf)
{
    ਪੂਰਨ i, j, m, tac, tdc;

    ਜੇ (!jpeg || !buf)
        ਵਾਪਸ -1;
    jpeg->datap = buf;
    ਜੇ (getbyte(jpeg) != 0xff)
        ਵਾਪਸ ERR_NO_SOI;
    ਜੇ (getbyte(jpeg) != M_SOI)
        ਵਾਪਸ ERR_NO_SOI;
    ਜੇ (readtables(jpeg, M_SOF0))
        ਵਾਪਸ ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    ਜੇ (i != 8)
        ਵਾਪਸ ERR_NOT_8_ਦੋਆਧਾਰੀ_ਅੰਕ;
    jpeg->ਉਚਾਈ = getword(jpeg);
    jpeg->ਚੌੜਾਈ = getword(jpeg);
    ਜੇ ((jpeg->ਉਚਾਈ & 15) || (jpeg->ਚੌੜਾਈ & 15))
        ਵਾਪਸ ERR_BAD_ਚੌੜਾਈ_OR_ਉਚਾਈ;
    jpeg->info.nc = getbyte(jpeg);
    ਜੇ (jpeg->info.nc > MAXCOMP)
        ਵਾਪਸ ERR_TOO_MANY_COMPPS;
    ਲਈ (i = 0; i < jpeg->info.nc; i++) {
        ਪੂਰਨ h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        ਜੇ (h > 3 || v > 3)
            ਵਾਪਸ ERR_ILLEGAL_HV;
        ਜੇ (jpeg->comps[i].tq > 3)
            ਵਾਪਸ ERR_QUANT_ਸਾਰਣੀ_SELECTOR;
    }
    ਜੇ (readtables(jpeg, M_SOS))
        ਵਾਪਸ ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    ਜੇ (jpeg->info.ns != 3)
        ਵਾਪਸ ERR_NOT_YCBCR_221111;
    ਲਈ (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        ਜੇ (tdc > 1 || tac > 1)
            ਵਾਪਸ ERR_QUANT_ਸਾਰਣੀ_SELECTOR;
        ਲਈ (j = 0; j < jpeg->info.nc; j++)
            ਜੇ (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                ਰੋਕੋ;
        ਜੇ (j == jpeg->info.nc)
            ਵਾਪਸ ERR_UNKNOWN_CID_ਵਿੱਚ_ਜਾਂਚ;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    ਜੇ (i != 0 || j != 63 || m != 0)
        ਵਾਪਸ ERR_NOT_SEQUENTIAL_DCT;

    ਜੇ (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        ਵਾਪਸ ERR_NOT_YCBCR_221111;

    ਜੇ (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        ਵਾਪਸ ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->ਵਿੱਚ_2, jpeg->datap);

#ਜੇ 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#ਜੇ_ਅੰਤ

    dec_initscans(jpeg);

    ਵਾਪਸ 0;
}

ਖਾਲੀ jpeg_get_ਮਾਪ(ਬਣਤਰ jpeg_decdata *jpeg, ਪੂਰਨ *ਚੌੜਾਈ, ਪੂਰਨ *ਉਚਾਈ)
{
    *ਚੌੜਾਈ = jpeg->ਚੌੜਾਈ;
    *ਉਚਾਈ = jpeg->ਉਚਾਈ;
}

ਪੂਰਨ jpeg_show(ਬਣਤਰ jpeg_decdata *jpeg, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic, ਪੂਰਨ ਚੌੜਾਈ
              , ਪੂਰਨ ਉਚਾਈ, ਪੂਰਨ ਡੂੰਘਾਈ, ਪੂਰਨ ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest)
{
    ਪੂਰਨ m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    ਪੂਰਨ max[6];

    ਜੇ (jpeg->ਉਚਾਈ != ਉਚਾਈ)
        ਵਾਪਸ ERR_ਉਚਾਈ_MISMATCH;
    ਜੇ (jpeg->ਚੌੜਾਈ != ਚੌੜਾਈ)
        ਵਾਪਸ ERR_ਚੌੜਾਈ_MISMATCH;

    jpgbpl = ਚੌੜਾਈ * ਡੂੰਘਾਈ / 8;
    mloffset = ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest > jpgbpl ? ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਸਤਰ_dest : jpgbpl;

    mcusx = jpeg->ਚੌੜਾਈ >> 4;
    mcusy = jpeg->ਉਚਾਈ >> 4;

    jpeg->dscans[0].ਅਗਲਾ = 6 - 4;
    jpeg->dscans[1].ਅਗਲਾ = 6 - 4 - 1;
    jpeg->dscans[2].ਅਗਲਾ = 6 - 4 - 1 - 1;        /* 411 encoding */
    ਲਈ (my = 0; my < mcusy; my++) {
        ਲਈ (mx = 0; mx < mcusx; mx++) {
            ਜੇ (jpeg->info.dri && !--jpeg->info.nm)
                ਜੇ (dec_checkmarker(jpeg))
                    ਵਾਪਸ ERR_WRONG_MARKER;

            decode_mcus(&jpeg->ਵਿੱਚ_2, jpeg->dcts, 6, jpeg->dscans, max);
            idct(jpeg->dcts, jpeg->ਬਾਹਰਲਾ, jpeg->dquant[0],
                 IFIX(128.5), max[0]);
            idct(jpeg->dcts + 64, jpeg->ਬਾਹਰਲਾ + 64, jpeg->dquant[0],
                 IFIX(128.5), max[1]);
            idct(jpeg->dcts + 128, jpeg->ਬਾਹਰਲਾ + 128, jpeg->dquant[0],
                 IFIX(128.5), max[2]);
            idct(jpeg->dcts + 192, jpeg->ਬਾਹਰਲਾ + 192, jpeg->dquant[0],
                 IFIX(128.5), max[3]);
            idct(jpeg->dcts + 256, jpeg->ਬਾਹਰਲਾ + 256, jpeg->dquant[1],
                 IFIX(0.5), max[4]);
            idct(jpeg->dcts + 320, jpeg->ਬਾਹਰਲਾ + 320, jpeg->dquant[2],
                 IFIX(0.5), max[5]);

            ਚੋਣ (ਡੂੰਘਾਈ) {
            ਹਾਲਤ 32:
                col221111_32(jpeg->ਬਾਹਰਲਾ,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                ਰੋਕੋ;
            ਹਾਲਤ 24:
                col221111(jpeg->ਬਾਹਰਲਾ,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                ਰੋਕੋ;
            ਹਾਲਤ 16:
                col221111_16(jpeg->ਬਾਹਰਲਾ,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                ਰੋਕੋ;
            ਮੂਲ:
                ਵਾਪਸ ERR_ਡੂੰਘਾਈ_MISMATCH;
                ਰੋਕੋ;
            }
        }
    }

    m = dec_readmarker(&jpeg->ਵਿੱਚ_2);
    ਜੇ (m != M_EOI)
        ਵਾਪਸ ERR_NO_EOI;

    ਵਾਪਸ 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

ਸਥਿਰ ਪੂਰਨ fillbits __P((ਬਣਤਰ ਵਿੱਚ_2 *, ਪੂਰਨ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ));
ਸਥਿਰ ਪੂਰਨ dec_rec2 __P((ਬਣਤਰ ਵਿੱਚ_2 *, ਬਣਤਰ dec_hufftbl *, ਪੂਰਨ *, ਪੂਰਨ, ਪੂਰਨ));

ਸਥਿਰ ਖਾਲੀ setinput(ਬਣਤਰ ਵਿੱਚ_2 *ਵਿੱਚ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p)
{
    ਵਿੱਚ_2->p = p;
    ਵਿੱਚ_2->ਖੱਬਾ = 0;
    ਵਿੱਚ_2->ਬਿੱਟ = 0;
    ਵਿੱਚ_2->marker = 0;
}

ਸਥਿਰ ਪੂਰਨ fillbits(ਬਣਤਰ ਵਿੱਚ_2 *ਵਿੱਚ_2, ਪੂਰਨ le, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ bi)
{
    ਪੂਰਨ b, m;

    ਜੇ (ਵਿੱਚ_2->marker) {
        ਜੇ (le <= 16)
            ਵਿੱਚ_2->ਬਿੱਟ = bi << 16, le += 16;
        ਵਾਪਸ le;
    }
    ਜਦੋਂ (le <= 24) {
        b = *ਵਿੱਚ_2->p++;
        ਜੇ (b == 0xff && (m = *ਵਿੱਚ_2->p++) != 0) {
            ਜੇ (m == M_EOF) {
                ਜੇ (ਵਿੱਚ_2->func && (m = ਵਿੱਚ_2->func(ਵਿੱਚ_2->ਅੰਕੜੇ)) == 0)
                    ਜਾਰੀ;
            }
            ਵਿੱਚ_2->marker = m;
            ਜੇ (le <= 16)
                bi = bi << 16, le += 16;
            ਰੋਕੋ;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    ਵਿੱਚ_2->ਬਿੱਟ = bi;                /* tmp... 2 return values needed */
    ਵਾਪਸ le;
}

ਸਥਿਰ ਪੂਰਨ dec_readmarker(ਬਣਤਰ ਵਿੱਚ_2 *ਵਿੱਚ_2)
{
    ਪੂਰਨ m;

    ਵਿੱਚ_2->ਖੱਬਾ = fillbits(ਵਿੱਚ_2, ਵਿੱਚ_2->ਖੱਬਾ, ਵਿੱਚ_2->ਬਿੱਟ);
    ਜੇ ((m = ਵਿੱਚ_2->marker) == 0)
        ਵਾਪਸ 0;
    ਵਿੱਚ_2->ਖੱਬਾ = 0;
    ਵਿੱਚ_2->marker = 0;
    ਵਾਪਸ m;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEBI_DCL       ਪੂਰਨ le, bi
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEBI_GET(ਵਿੱਚ_2)   (le = ਵਿੱਚ_2->ਖੱਬਾ, bi = ਵਿੱਚ_2->ਬਿੱਟ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEBI_PUT(ਵਿੱਚ_2)   (ਵਿੱਚ_2->ਖੱਬਾ = le, ਵਿੱਚ_2->ਬਿੱਟ = bi)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GETBITS(ਵਿੱਚ_2, n) (                                     \
  (le < (n) ? le = fillbits(ਵਿੱਚ_2, le, bi), bi = ਵਿੱਚ_2->ਬਿੱਟ : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNGETBITS(ਵਿੱਚ_2, n) ( \
  le += (n)                \
)


ਸਥਿਰ ਪੂਰਨ dec_rec2(ਬਣਤਰ ਵਿੱਚ_2 *ਵਿੱਚ_2, ਬਣਤਰ dec_hufftbl *hu, ਪੂਰਨ *runp,
                    ਪੂਰਨ c, ਪੂਰਨ i)
{
    LEBI_DCL;

    LEBI_GET(ਵਿੱਚ_2);
    ਜੇ (i) {
        UNGETBITS(ਵਿੱਚ_2, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } ਨਹੀਂਤਾਂ {
        ਲਈ (i = DECBITS;
             (c = ((c << 1) | GETBITS(ਵਿੱਚ_2, 1))) >= (hu->maxcode[i]); i++);
        ਜੇ (i >= 16) {
            ਵਿੱਚ_2->marker = M_BADHUFF;
            ਵਾਪਸ 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    ਜੇ (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(ਵਿੱਚ_2);
        ਵਾਪਸ 0;
    }
    /* receive part */
    c = GETBITS(ਵਿੱਚ_2, i);
    ਜੇ (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(ਵਿੱਚ_2);
    ਵਾਪਸ c;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEC_REC(ਵਿੱਚ_2, hu, r, i)         (  \
  r = GETBITS(ਵਿੱਚ_2, DECBITS),              \
  i = hu->llvals[r],                     \
  i & 128 ?                              \
    (                                    \
      UNGETBITS(ਵਿੱਚ_2, i & 127),            \
      r = i >> 8 & 15,                   \
      i >> 16                            \
    )                                    \
  :                                      \
    (                                    \
      LEBI_PUT(ਵਿੱਚ_2),                      \
      i = dec_rec2(ਵਿੱਚ_2, hu, &r, r, i),    \
      LEBI_GET(ਵਿੱਚ_2),                      \
      i                                  \
    )                                    \
)

ਸਥਿਰ ਖਾਲੀ decode_mcus(ਬਣਤਰ ਵਿੱਚ_2 *ਵਿੱਚ_2, ਪੂਰਨ *dct, ਪੂਰਨ n, ਬਣਤਰ ਜਾਂਚ *sc,
                        ਪੂਰਨ *maxp)
{
    ਬਣਤਰ dec_hufftbl *hu;
    ਪੂਰਨ i, r, t;
    LEBI_DCL;

    memset(dct, 0, n * 64 * ਆਕਾਰ(*dct));
    LEBI_GET(ਵਿੱਚ_2);
    ਜਦੋਂ (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(ਵਿੱਚ_2, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        ਜਦੋਂ (i > 0) {
            t = DEC_REC(ਵਿੱਚ_2, hu, r, t);
            ਜੇ (t == 0 && r == 0) {
                dct += i;
                ਰੋਕੋ;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        ਜੇ (n == sc->ਅਗਲਾ)
            sc++;
    }
    LEBI_PUT(ਵਿੱਚ_2);
}

ਸਥਿਰ ਖਾਲੀ dec_makehuff(ਬਣਤਰ dec_hufftbl *hu, ਪੂਰਨ *hufflen,
                         ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *huffvals)
{
    ਪੂਰਨ ਕੋਡ, k, i, j, d, x, c, v;
    ਲਈ (i = 0; i < (1 << DECBITS); i++)
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

    ਕੋਡ = 0;
    k = 0;
    ਲਈ (i = 0; i < 16; i++, ਕੋਡ <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        ਲਈ (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            ਜੇ (i < DECBITS) {
                c = ਕੋਡ << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /*ਮਾਪ*/
                ਲਈ (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    ਜੇ (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        ਜੇ (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } ਨਹੀਂਤਾਂ
                        x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1));
                    hu->llvals[c | d] = x;
                }
            }
            ਕੋਡ++;
            k++;
        }
        hu->maxcode[i] = ਕੋਡ;
    }
    hu->maxcode[16] = 0x20000;        /* always terminate decode */
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONE ((PREC)IFIX(1.))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ S2  ((PREC)IFIX(0.382683432))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C2  ((PREC)IFIX(0.923879532))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C4  ((PREC)IFIX(0.707106781))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ S22 ((PREC)IFIX(2 * 0.382683432))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C22 ((PREC)IFIX(2 * 0.923879532))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IC4 ((PREC)IFIX(1 / 0.707106781))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C3IC1 ((PREC)IFIX(0.847759065))        /* c3/c1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C5IC1 ((PREC)IFIX(0.566454497))        /* c5/c1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ C7IC1 ((PREC)IFIX(0.198912367))        /* c7/c1 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XPP(a,b) (t = a + b, b = a - b, a = t)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XMP(a,b) (t = a - b, b = a + b, a = t)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XPM(a,b) (t = a + b, b = b - a, a = t)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROT(a,b,s,c) (  t = IMULT(a + b, s),      \
                        a = IMULT(a, c - s) + t,  \
                        b = IMULT(b, c + s) - t)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IDCT                \
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

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

ਸਥਿਰ ਖਾਲੀ idct(ਪੂਰਨ *ਵਿੱਚ_2, ਪੂਰਨ *ਬਾਹਰਲਾ, PREC * quant, PREC off, ਪੂਰਨ max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    ਪੂਰਨ i, j;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zig2p;

    t0 = off;
    ਜੇ (max == 1) {
        t0 += ਵਿੱਚ_2[0] * quant[0];
        ਲਈ (i = 0; i < 64; i++)
            ਬਾਹਰਲਾ[i] = ITOINT(t0);
        ਵਾਪਸ;
    }
    zig2p = zig2;
    tmpp = tmp;
    ਲਈ (i = 0; i < 8; i++) {
        j = *zig2p++;
        t0 += ਵਿੱਚ_2[j] * quant[j];
        j = *zig2p++;
        t5 = ਵਿੱਚ_2[j] * quant[j];
        j = *zig2p++;
        t2 = ਵਿੱਚ_2[j] * quant[j];
        j = *zig2p++;
        t7 = ਵਿੱਚ_2[j] * quant[j];
        j = *zig2p++;
        t1 = ਵਿੱਚ_2[j] * quant[j];
        j = *zig2p++;
        t4 = ਵਿੱਚ_2[j] * quant[j];
        j = *zig2p++;
        t3 = ਵਿੱਚ_2[j] * quant[j];
        j = *zig2p++;
        t6 = ਵਿੱਚ_2[j] * quant[j];
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
    ਲਈ (i = 0; i < 8; i++) {
        t0 = tmp[8 * i + 0];
        t1 = tmp[8 * i + 1];
        t2 = tmp[8 * i + 2];
        t3 = tmp[8 * i + 3];
        t4 = tmp[8 * i + 4];
        t5 = tmp[8 * i + 5];
        t6 = tmp[8 * i + 6];
        t7 = tmp[8 * i + 7];
        IDCT;
        ਬਾਹਰਲਾ[8 * i + 0] = ITOINT(t0);
        ਬਾਹਰਲਾ[8 * i + 1] = ITOINT(t1);
        ਬਾਹਰਲਾ[8 * i + 2] = ITOINT(t2);
        ਬਾਹਰਲਾ[8 * i + 3] = ITOINT(t3);
        ਬਾਹਰਲਾ[8 * i + 4] = ITOINT(t4);
        ਬਾਹਰਲਾ[8 * i + 5] = ITOINT(t5);
        ਬਾਹਰਲਾ[8 * i + 6] = ITOINT(t6);
        ਬਾਹਰਲਾ[8 * i + 7] = ITOINT(t7);
    }
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zig[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

ਸਥਿਰ PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};


ਸਥਿਰ ਖਾਲੀ idctqtab(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *qin, PREC * qout)
{
    ਪੂਰਨ i, j;

    ਲਈ (i = 0; i < 8; i++)
        ਲਈ (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

ਸਥਿਰ ਖਾਲੀ scaleidctqtab(PREC * q, PREC sc)
{
    ਪੂਰਨ i;

    ਲਈ (i = 0; i < 64; i++)
        q[i] = IMULT(q[i], sc);
}

/****************************************************************/
/**************          color decoder            ***************/
/****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗੇੜਾ

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

ਸਥਿਰ ਖਾਲੀ initcol(PREC q[][64])
{
    scaleidctqtab(q[1], IFIX(1.77200));
    scaleidctqtab(q[2], IFIX(1.40200));
}

/* This is optimized for the stupid sun SUNWspro compiler. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CLAMP(x) ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਗੇੜਾ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#ਨਹੀਂਤਾਂ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (3 * cb + 8 * cr) >> 4                    \
)

#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 2], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 0], y + cb)          \
)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 0], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 2], y + cb)          \
)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC_16(yin, xin, p, xout, ਜੋੜੋ)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + ਜੋੜੋ*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + ਜੋੜੋ    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + ਜੋੜੋ*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y & 0xff,                  \
  p[(xout) * 2 + 1] = y >> 8                     \
)
#ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PPC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC_16(yin, xin, p, xout, ਜੋੜੋ)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + ਜੋੜੋ*2+1) & 0xf8) <<  7) | \
      ((CLAMP(y - cg + ਜੋੜੋ*2+1) & 0xf8) <<  2) | \
      ((CLAMP(y + cb + ਜੋੜੋ*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC_16(yin, xin, p, xout, ਜੋੜੋ)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + ਜੋੜੋ*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + ਜੋੜੋ    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + ਜੋੜੋ*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC_32(yin, xin, p, xout)               \
(                                               \
  y = outy[(yin) * 8 + xin],                    \
  STORECLAMP(p[(xout) * 4 + 0], y + cr),        \
  STORECLAMP(p[(xout) * 4 + 1], y - cg),        \
  STORECLAMP(p[(xout) * 4 + 2], y + cb),        \
  p[(xout) * 4 + 3] = 0                         \
)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC221111(xin)                                              \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)        \
)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC221111_16(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0, 3), \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1, 0), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0, 1), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1, 2)  \
)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIC221111_32(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)     \
)

ਸਥਿਰ ਖਾਲੀ col221111(ਪੂਰਨ *ਬਾਹਰਲਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic, ਪੂਰਨ ਚੌੜਾਈ)
{
    ਪੂਰਨ i, j, k;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic0, *pic1;
    ਪੂਰਨ *outy, *outc;
    ਪੂਰਨ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ਚੌੜਾਈ;
    outy = ਬਾਹਰਲਾ;
    outc = ਬਾਹਰਲਾ + 64 * 4;
    ਲਈ (i = 2; i > 0; i--) {
        ਲਈ (j = 4; j > 0; j--) {
            ਲਈ (k = 0; k < 8; k++) {
                PIC221111(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ਚੌੜਾਈ;
            pic1 += 2 * ਚੌੜਾਈ;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ਸਥਿਰ ਖਾਲੀ col221111_16(ਪੂਰਨ *ਬਾਹਰਲਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic, ਪੂਰਨ ਚੌੜਾਈ)
{
    ਪੂਰਨ i, j, k;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic0, *pic1;
    ਪੂਰਨ *outy, *outc;
    ਪੂਰਨ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ਚੌੜਾਈ;
    outy = ਬਾਹਰਲਾ;
    outc = ਬਾਹਰਲਾ + 64 * 4;
    ਲਈ (i = 2; i > 0; i--) {
        ਲਈ (j = 4; j > 0; j--) {
            ਲਈ (k = 0; k < 8; k++) {
                PIC221111_16(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ਚੌੜਾਈ;
            pic1 += 2 * ਚੌੜਾਈ;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ਸਥਿਰ ਖਾਲੀ col221111_32(ਪੂਰਨ *ਬਾਹਰਲਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic, ਪੂਰਨ ਚੌੜਾਈ)
{
    ਪੂਰਨ i, j, k;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pic0, *pic1;
    ਪੂਰਨ *outy, *outc;
    ਪੂਰਨ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ਚੌੜਾਈ;
    outy = ਬਾਹਰਲਾ;
    outc = ਬਾਹਰਲਾ + 64 * 4;
    ਲਈ (i = 2; i > 0; i--) {
        ਲਈ (j = 4; j > 0; j--) {
            ਲਈ (k = 0; k < 8; k++) {
                PIC221111_32(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ਚੌੜਾਈ;
            pic1 += 2 * ਚੌੜਾਈ;
        }
        outy += 64 * 2 - 16 * 4;
    }
}
