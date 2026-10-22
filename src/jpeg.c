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

#ವ್ಯಾಖ್ಯಾನಿಸು __LITTLE_ENDIAN
#ಅನ್ತರ್ಭಾವ "malloc.h"
#ಅನ್ತರ್ಭಾವ "string.h"
#ಅನ್ತರ್ಭಾವ "util.h"
#ವ್ಯಾಖ್ಯಾನಿಸು ISHIFT 11

#ವ್ಯಾಖ್ಯಾನಿಸು IFIX(a) ((ಪೂರ್ಣಾಂಕ)((a) * (1 << ISHIFT) + .5))
#ವ್ಯಾಖ್ಯಾನಿಸು IMULT(a, b) (((a) * (b)) >> ISHIFT)
#ವ್ಯಾಖ್ಯಾನಿಸು ITOINT(a) ((a) >> ISHIFT)

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __P
# define __P(x) x
#ಆದರೆ_ಅಂತ್ಯ

/* special markers */
#ವ್ಯಾಖ್ಯಾನಿಸು M_BADHUFF        -1
#ವ್ಯಾಖ್ಯಾನಿಸು M_EOF                0x80

ರಚನೆ ಒಳ {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು;
    ಪೂರ್ಣಾಂಕ ಎಡ;
    ಪೂರ್ಣಾಂಕ marker;

    ಪೂರ್ಣಾಂಕ (*func) __P((ಶೂನ್ಯ *));
    ಶೂನ್ಯ *ದತ್ತಾಂಶ;
};

/*********************************/
ರಚನೆ dec_hufftbl;
ರಚನೆ enc_hufftbl;

ಒಕ್ಕೂಟ hufftblp {
    ರಚನೆ dec_hufftbl *dhuff;
    ರಚನೆ enc_hufftbl *ehuff;
};

ರಚನೆ ಪರಿಶೀಲನೆ {
    ಪೂರ್ಣಾಂಕ dc;               /* old dc value */

    ಒಕ್ಕೂಟ hufftblp hudc;
    ಒಕ್ಕೂಟ hufftblp huac;
    ಪೂರ್ಣಾಂಕ ಮುಂದಿನ;             /* when to switch to next scan */

    ಪೂರ್ಣಾಂಕ cid;              /* component id */
    ಪೂರ್ಣಾಂಕ hv;               /* horiz/vert, copied from comp */
    ಪೂರ್ಣಾಂಕ tq;               /* quant tbl, copied from comp */
};

/*********************************/

#ವ್ಯಾಖ್ಯಾನಿಸು DECBITS 10        /* seems to be the optimum */

ರಚನೆ dec_hufftbl {
    ಪೂರ್ಣಾಂಕ maxcode[17];
    ಪೂರ್ಣಾಂಕ valptr[16];
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ vals[256];
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ llvals[1 << DECBITS];
};

ಸ್ಥಿರ ಶೂನ್ಯ decode_mcus __P((ರಚನೆ ಒಳ *, ಪೂರ್ಣಾಂಕ *, ಪೂರ್ಣಾಂಕ, ರಚನೆ ಪರಿಶೀಲನೆ *, ಪೂರ್ಣಾಂಕ *));
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dec_readmarker __P((ರಚನೆ ಒಳ *));
ಸ್ಥಿರ ಶೂನ್ಯ dec_makehuff __P((ರಚನೆ dec_hufftbl *, ಪೂರ್ಣಾಂಕ *, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *));

ಸ್ಥಿರ ಶೂನ್ಯ setinput __P((ರಚನೆ ಒಳ *, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *));
/*********************************/

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು PREC
#ವ್ಯಾಖ್ಯಾನಿಸು PREC ಪೂರ್ಣಾಂಕ

ಸ್ಥಿರ ಶೂನ್ಯ idctqtab __P((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *, PREC *));
ಸ್ಥಿರ ಶೂನ್ಯ idct __P((ಪೂರ್ಣಾಂಕ *, ಪೂರ್ಣಾಂಕ *, PREC *, PREC, ಪೂರ್ಣಾಂಕ));
ಸ್ಥಿರ ಶೂನ್ಯ scaleidctqtab __P((PREC *, PREC));

/*********************************/

ಸ್ಥಿರ ಶೂನ್ಯ initcol __P((PREC[][64]));

ಸ್ಥಿರ ಶೂನ್ಯ col221111 __P((ಪೂರ್ಣಾಂಕ *, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ));
ಸ್ಥಿರ ಶೂನ್ಯ col221111_16 __P((ಪೂರ್ಣಾಂಕ *, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ));
ಸ್ಥಿರ ಶೂನ್ಯ col221111_32 __P((ಪೂರ್ಣಾಂಕ *, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ));

/*********************************/

#ವ್ಯಾಖ್ಯಾನಿಸು ERR_NO_SOI 1
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_NOT_8_ಬಿಟ್ 2
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_ಎತ್ತರ_MISMATCH 3
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_ಅಗಲ_MISMATCH 4
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_BAD_ಅಗಲ_OR_ಎತ್ತರ 5
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_TOO_MANY_COMPPS 6
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_ILLEGAL_HV 7
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_QUANT_ಕೋಷ್ಟಕ_SELECTOR 8
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_NOT_YCBCR_221111 9
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_UNKNOWN_CID_ಒಳ_ಪರಿಶೀಲನೆ 10
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_NOT_SEQUENTIAL_DCT 11
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_WRONG_MARKER 12
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_NO_EOI 13
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_BAD_TABLES 14
#ವ್ಯಾಖ್ಯಾನಿಸು ERR_ಆಳ_MISMATCH 15

/*********************************/

#ವ್ಯಾಖ್ಯಾನಿಸು M_SOI   0xd8
#ವ್ಯಾಖ್ಯಾನಿಸು M_APP0  0xe0
#ವ್ಯಾಖ್ಯಾನಿಸು M_DQT   0xdb
#ವ್ಯಾಖ್ಯಾನಿಸು M_SOF0  0xc0
#ವ್ಯಾಖ್ಯಾನಿಸು M_DHT   0xc4
#ವ್ಯಾಖ್ಯಾನಿಸು M_DRI   0xdd
#ವ್ಯಾಖ್ಯಾನಿಸು M_SOS   0xda
#ವ್ಯಾಖ್ಯಾನಿಸು M_RST0  0xd0
#ವ್ಯಾಖ್ಯಾನಿಸು M_EOI   0xd9
#ವ್ಯಾಖ್ಯಾನಿಸು M_COM   0xfe

ರಚನೆ comp {
    ಪೂರ್ಣಾಂಕ cid;
    ಪೂರ್ಣಾಂಕ hv;
    ಪೂರ್ಣಾಂಕ tq;
};

#ವ್ಯಾಖ್ಯಾನಿಸು MAXCOMP 4
ರಚನೆ jpginfo {
    ಪೂರ್ಣಾಂಕ nc;   /* number of components */
    ಪೂರ್ಣಾಂಕ ns;   /* number of scans */
    ಪೂರ್ಣಾಂಕ dri;  /* restart interval */
    ಪೂರ್ಣಾಂಕ nm;   /* mcus til next marker */
    ಪೂರ್ಣಾಂಕ rm;   /* next restart marker */
};

ರಚನೆ jpeg_decdata {
    ಪೂರ್ಣಾಂಕ dcts[6 * 64 + 16];
    ಪೂರ್ಣಾಂಕ ಹೊರಹೋಗು[64 * 6];
    ಪೂರ್ಣಾಂಕ dquant[3][64];

    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *datap;
    ರಚನೆ jpginfo info;
    ರಚನೆ comp comps[MAXCOMP];
    ರಚನೆ ಪರಿಶೀಲನೆ dscans[MAXCOMP];
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ quant[4][64];
    ರಚನೆ dec_hufftbl dhuff[4];
    ರಚನೆ ಒಳ ಒಳ;

    ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಅಗಲ;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getbyte(ರಚನೆ jpeg_decdata *jpeg)
{
    ಹಿಂತಿರುಗಿ *jpeg->datap++;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getword(ರಚನೆ jpeg_decdata *jpeg)
{
    ಪೂರ್ಣಾಂಕ c1, c2;
    c1 = *jpeg->datap++;
    c2 = *jpeg->datap++;
    ಹಿಂತಿರುಗಿ c1 << 8 | c2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ readtables(ರಚನೆ jpeg_decdata *jpeg, ಪೂರ್ಣಾಂಕ till)
{
    ಪೂರ್ಣಾಂಕ m, l, i, j, lq, pq, tq;
    ಪೂರ್ಣಾಂಕ tc, th, tt;

    ಗಾಗಿ (;;) {
        ಆದರೆ (getbyte(jpeg) != 0xff)
            ಹಿಂತಿರುಗಿ -1;
        ಆದರೆ ((m = getbyte(jpeg)) == till)
            ನಿಲ್ಲಿಸು;

        ಆಯ್ಕೆ (m) {
        ಸ್ಥಿತಿ 0xc2:
            ಹಿಂತಿರುಗಿ 0;

        ಸ್ಥಿತಿ M_DQT:
            lq = getword(jpeg);
            ಆಗ (lq > 2) {
                pq = getbyte(jpeg);
                tq = pq & 15;
                ಆದರೆ (tq > 3)
                    ಹಿಂತಿರುಗಿ -1;
                pq >>= 4;
                ಆದರೆ (pq != 0)
                    ಹಿಂತಿರುಗಿ -1;
                ಗಾಗಿ (i = 0; i < 64; i++)
                    jpeg->quant[tq][i] = getbyte(jpeg);
                lq -= 64 + 1;
            }
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ M_DHT:
            l = getword(jpeg);
            ಆಗ (l > 2) {
                ಪೂರ್ಣಾಂಕ hufflen[16], k;
                ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ huffvals[256];

                tc = getbyte(jpeg);
                th = tc & 15;
                tc >>= 4;
                tt = tc * 2 + th;
                ಆದರೆ (tc > 1 || th > 1)
                    ಹಿಂತಿರುಗಿ -1;
                ಗಾಗಿ (i = 0; i < 16; i++)
                    hufflen[i] = getbyte(jpeg);
                l -= 1 + 16;
                k = 0;
                ಗಾಗಿ (i = 0; i < 16; i++) {
                    ಗಾಗಿ (j = 0; j < hufflen[i]; j++)
                        huffvals[k++] = getbyte(jpeg);
                    l -= hufflen[i];
                }
                dec_makehuff(jpeg->dhuff + tt, hufflen, huffvals);
            }
            ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ M_DRI:
            l = getword(jpeg);
            jpeg->info.dri = getword(jpeg);
            ನಿಲ್ಲಿಸು;

        ಪೂರ್ವನಿಯೋಜಿತ:
            l = getword(jpeg);
            ಆಗ (l-- > 2)
                getbyte(jpeg);
            ನಿಲ್ಲಿಸು;
        }
    }
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ dec_initscans(ರಚನೆ jpeg_decdata *jpeg)
{
    ಪೂರ್ಣಾಂಕ i;

    jpeg->info.nm = jpeg->info.dri + 1;
    jpeg->info.rm = M_RST0;
    ಗಾಗಿ (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dec_checkmarker(ರಚನೆ jpeg_decdata *jpeg)
{
    ಪೂರ್ಣಾಂಕ i;

    ಆದರೆ (dec_readmarker(&jpeg->ಒಳ) != jpeg->info.rm)
        ಹಿಂತಿರುಗಿ -1;
    jpeg->info.nm = jpeg->info.dri;
    jpeg->info.rm = (jpeg->info.rm + 1) & ~0x08;
    ಗಾಗಿ (i = 0; i < jpeg->info.ns; i++)
        jpeg->dscans[i].dc = 0;
    ಹಿಂತಿರುಗಿ 0;
}

ರಚನೆ jpeg_decdata *jpeg_alloc(ಶೂನ್ಯ)
{
    ರಚನೆ jpeg_decdata *jpeg = malloc_tmphigh(ಗಾತ್ರ(*jpeg));
    ಹಿಂತಿರುಗಿ jpeg;
}

ಪೂರ್ಣಾಂಕ jpeg_decode(ರಚನೆ jpeg_decdata *jpeg, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf)
{
    ಪೂರ್ಣಾಂಕ i, j, m, tac, tdc;

    ಆದರೆ (!jpeg || !buf)
        ಹಿಂತಿರುಗಿ -1;
    jpeg->datap = buf;
    ಆದರೆ (getbyte(jpeg) != 0xff)
        ಹಿಂತಿರುಗಿ ERR_NO_SOI;
    ಆದರೆ (getbyte(jpeg) != M_SOI)
        ಹಿಂತಿರುಗಿ ERR_NO_SOI;
    ಆದರೆ (readtables(jpeg, M_SOF0))
        ಹಿಂತಿರುಗಿ ERR_BAD_TABLES;
    getword(jpeg);
    i = getbyte(jpeg);
    ಆದರೆ (i != 8)
        ಹಿಂತಿರುಗಿ ERR_NOT_8_ಬಿಟ್;
    jpeg->ಎತ್ತರ = getword(jpeg);
    jpeg->ಅಗಲ = getword(jpeg);
    ಆದರೆ ((jpeg->ಎತ್ತರ & 15) || (jpeg->ಅಗಲ & 15))
        ಹಿಂತಿರುಗಿ ERR_BAD_ಅಗಲ_OR_ಎತ್ತರ;
    jpeg->info.nc = getbyte(jpeg);
    ಆದರೆ (jpeg->info.nc > MAXCOMP)
        ಹಿಂತಿರುಗಿ ERR_TOO_MANY_COMPPS;
    ಗಾಗಿ (i = 0; i < jpeg->info.nc; i++) {
        ಪೂರ್ಣಾಂಕ h, v;
        jpeg->comps[i].cid = getbyte(jpeg);
        jpeg->comps[i].hv = getbyte(jpeg);
        v = jpeg->comps[i].hv & 15;
        h = jpeg->comps[i].hv >> 4;
        jpeg->comps[i].tq = getbyte(jpeg);
        ಆದರೆ (h > 3 || v > 3)
            ಹಿಂತಿರುಗಿ ERR_ILLEGAL_HV;
        ಆದರೆ (jpeg->comps[i].tq > 3)
            ಹಿಂತಿರುಗಿ ERR_QUANT_ಕೋಷ್ಟಕ_SELECTOR;
    }
    ಆದರೆ (readtables(jpeg, M_SOS))
        ಹಿಂತಿರುಗಿ ERR_BAD_TABLES;
    getword(jpeg);
    jpeg->info.ns = getbyte(jpeg);
    ಆದರೆ (jpeg->info.ns != 3)
        ಹಿಂತಿರುಗಿ ERR_NOT_YCBCR_221111;
    ಗಾಗಿ (i = 0; i < 3; i++) {
        jpeg->dscans[i].cid = getbyte(jpeg);
        tdc = getbyte(jpeg);
        tac = tdc & 15;
        tdc >>= 4;
        ಆದರೆ (tdc > 1 || tac > 1)
            ಹಿಂತಿರುಗಿ ERR_QUANT_ಕೋಷ್ಟಕ_SELECTOR;
        ಗಾಗಿ (j = 0; j < jpeg->info.nc; j++)
            ಆದರೆ (jpeg->comps[j].cid == jpeg->dscans[i].cid)
                ನಿಲ್ಲಿಸು;
        ಆದರೆ (j == jpeg->info.nc)
            ಹಿಂತಿರುಗಿ ERR_UNKNOWN_CID_ಒಳ_ಪರಿಶೀಲನೆ;
        jpeg->dscans[i].hv = jpeg->comps[j].hv;
        jpeg->dscans[i].tq = jpeg->comps[j].tq;
        jpeg->dscans[i].hudc.dhuff = &jpeg->dhuff[tdc];
        jpeg->dscans[i].huac.dhuff = &jpeg->dhuff[2 + tac];
    }

    i = getbyte(jpeg);
    j = getbyte(jpeg);
    m = getbyte(jpeg);

    ಆದರೆ (i != 0 || j != 63 || m != 0)
        ಹಿಂತಿರುಗಿ ERR_NOT_SEQUENTIAL_DCT;

    ಆದರೆ (jpeg->dscans[0].cid != 1 || jpeg->dscans[1].cid != 2
        || jpeg->dscans[2].cid != 3)
        ಹಿಂತಿರುಗಿ ERR_NOT_YCBCR_221111;

    ಆದರೆ (jpeg->dscans[0].hv != 0x22 || jpeg->dscans[1].hv != 0x11
        || jpeg->dscans[2].hv != 0x11)
        ಹಿಂತಿರುಗಿ ERR_NOT_YCBCR_221111;

    idctqtab(jpeg->quant[jpeg->dscans[0].tq], jpeg->dquant[0]);
    idctqtab(jpeg->quant[jpeg->dscans[1].tq], jpeg->dquant[1]);
    idctqtab(jpeg->quant[jpeg->dscans[2].tq], jpeg->dquant[2]);
    initcol(jpeg->dquant);
    setinput(&jpeg->ಒಳ, jpeg->datap);

#ಆದರೆ 0
    /* landing zone */
    img[len] = 0;
    img[len + 1] = 0xff;
    img[len + 2] = M_EOF;
#ಆದರೆ_ಅಂತ್ಯ

    dec_initscans(jpeg);

    ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ jpeg_get_ಗಾತ್ರ(ರಚನೆ jpeg_decdata *jpeg, ಪೂರ್ಣಾಂಕ *ಅಗಲ, ಪೂರ್ಣಾಂಕ *ಎತ್ತರ)
{
    *ಅಗಲ = jpeg->ಅಗಲ;
    *ಎತ್ತರ = jpeg->ಎತ್ತರ;
}

ಪೂರ್ಣಾಂಕ jpeg_show(ರಚನೆ jpeg_decdata *jpeg, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic, ಪೂರ್ಣಾಂಕ ಅಗಲ
              , ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಪೂರ್ಣಾಂಕ ಆಳ, ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಾಲು_dest)
{
    ಪೂರ್ಣಾಂಕ m, mcusx, mcusy, mx, my, mloffset, jpgbpl;
    ಪೂರ್ಣಾಂಕ max[6];

    ಆದರೆ (jpeg->ಎತ್ತರ != ಎತ್ತರ)
        ಹಿಂತಿರುಗಿ ERR_ಎತ್ತರ_MISMATCH;
    ಆದರೆ (jpeg->ಅಗಲ != ಅಗಲ)
        ಹಿಂತಿರುಗಿ ERR_ಅಗಲ_MISMATCH;

    jpgbpl = ಅಗಲ * ಆಳ / 8;
    mloffset = ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಾಲು_dest > jpgbpl ? ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಾಲು_dest : jpgbpl;

    mcusx = jpeg->ಅಗಲ >> 4;
    mcusy = jpeg->ಎತ್ತರ >> 4;

    jpeg->dscans[0].ಮುಂದಿನ = 6 - 4;
    jpeg->dscans[1].ಮುಂದಿನ = 6 - 4 - 1;
    jpeg->dscans[2].ಮುಂದಿನ = 6 - 4 - 1 - 1;        /* 411 encoding */
    ಗಾಗಿ (my = 0; my < mcusy; my++) {
        ಗಾಗಿ (mx = 0; mx < mcusx; mx++) {
            ಆದರೆ (jpeg->info.dri && !--jpeg->info.nm)
                ಆದರೆ (dec_checkmarker(jpeg))
                    ಹಿಂತಿರುಗಿ ERR_WRONG_MARKER;

            decode_mcus(&jpeg->ಒಳ, jpeg->dcts, 6, jpeg->dscans, max);
            idct(jpeg->dcts, jpeg->ಹೊರಹೋಗು, jpeg->dquant[0],
                 IFIX(128.5), max[0]);
            idct(jpeg->dcts + 64, jpeg->ಹೊರಹೋಗು + 64, jpeg->dquant[0],
                 IFIX(128.5), max[1]);
            idct(jpeg->dcts + 128, jpeg->ಹೊರಹೋಗು + 128, jpeg->dquant[0],
                 IFIX(128.5), max[2]);
            idct(jpeg->dcts + 192, jpeg->ಹೊರಹೋಗು + 192, jpeg->dquant[0],
                 IFIX(128.5), max[3]);
            idct(jpeg->dcts + 256, jpeg->ಹೊರಹೋಗು + 256, jpeg->dquant[1],
                 IFIX(0.5), max[4]);
            idct(jpeg->dcts + 320, jpeg->ಹೊರಹೋಗು + 320, jpeg->dquant[2],
                 IFIX(0.5), max[5]);

            ಆಯ್ಕೆ (ಆಳ) {
            ಸ್ಥಿತಿ 32:
                col221111_32(jpeg->ಹೊರಹೋಗು,
                             pic + (my * 16 * mloffset + mx * 16 * 4),
                             mloffset);
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 24:
                col221111(jpeg->ಹೊರಹೋಗು,
                          pic + (my * 16 * mloffset + mx * 16 * 3),
                          mloffset);
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ 16:
                col221111_16(jpeg->ಹೊರಹೋಗು,
                             pic + (my * 16 * mloffset + mx * 16 * 2),
                             mloffset);
                ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:
                ಹಿಂತಿರುಗಿ ERR_ಆಳ_MISMATCH;
                ನಿಲ್ಲಿಸು;
            }
        }
    }

    m = dec_readmarker(&jpeg->ಒಳ);
    ಆದರೆ (m != M_EOI)
        ಹಿಂತಿರುಗಿ ERR_NO_EOI;

    ಹಿಂತಿರುಗಿ 0;
}

/****************************************************************/
/**************       huffman decoder             ***************/
/****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fillbits __P((ರಚನೆ ಒಳ *, ಪೂರ್ಣಾಂಕ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ));
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dec_rec2 __P((ರಚನೆ ಒಳ *, ರಚನೆ dec_hufftbl *, ಪೂರ್ಣಾಂಕ *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ));

ಸ್ಥಿರ ಶೂನ್ಯ setinput(ರಚನೆ ಒಳ *ಒಳ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p)
{
    ಒಳ->p = p;
    ಒಳ->ಎಡ = 0;
    ಒಳ->ಬಿಟ್ಟುಗಳು = 0;
    ಒಳ->marker = 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fillbits(ರಚನೆ ಒಳ *ಒಳ, ಪೂರ್ಣಾಂಕ le, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ bi)
{
    ಪೂರ್ಣಾಂಕ b, m;

    ಆದರೆ (ಒಳ->marker) {
        ಆದರೆ (le <= 16)
            ಒಳ->ಬಿಟ್ಟುಗಳು = bi << 16, le += 16;
        ಹಿಂತಿರುಗಿ le;
    }
    ಆಗ (le <= 24) {
        b = *ಒಳ->p++;
        ಆದರೆ (b == 0xff && (m = *ಒಳ->p++) != 0) {
            ಆದರೆ (m == M_EOF) {
                ಆದರೆ (ಒಳ->func && (m = ಒಳ->func(ಒಳ->ದತ್ತಾಂಶ)) == 0)
                    ಮುಂದುವರಿ;
            }
            ಒಳ->marker = m;
            ಆದರೆ (le <= 16)
                bi = bi << 16, le += 16;
            ನಿಲ್ಲಿಸು;
        }
        bi = bi << 8 | b;
        le += 8;
    }
    ಒಳ->ಬಿಟ್ಟುಗಳು = bi;                /* tmp... 2 return values needed */
    ಹಿಂತಿರುಗಿ le;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dec_readmarker(ರಚನೆ ಒಳ *ಒಳ)
{
    ಪೂರ್ಣಾಂಕ m;

    ಒಳ->ಎಡ = fillbits(ಒಳ, ಒಳ->ಎಡ, ಒಳ->ಬಿಟ್ಟುಗಳು);
    ಆದರೆ ((m = ಒಳ->marker) == 0)
        ಹಿಂತಿರುಗಿ 0;
    ಒಳ->ಎಡ = 0;
    ಒಳ->marker = 0;
    ಹಿಂತಿರುಗಿ m;
}

#ವ್ಯಾಖ್ಯಾನಿಸು LEBI_DCL       ಪೂರ್ಣಾಂಕ le, bi
#ವ್ಯಾಖ್ಯಾನಿಸು LEBI_GET(ಒಳ)   (le = ಒಳ->ಎಡ, bi = ಒಳ->ಬಿಟ್ಟುಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು LEBI_PUT(ಒಳ)   (ಒಳ->ಎಡ = le, ಒಳ->ಬಿಟ್ಟುಗಳು = bi)

#ವ್ಯಾಖ್ಯಾನಿಸು GETBITS(ಒಳ, n) (                                     \
  (le < (n) ? le = fillbits(ಒಳ, le, bi), bi = ಒಳ->ಬಿಟ್ಟುಗಳು : 0), \
  (le -= (n)),                                               \
  bi >> le & ((1 << (n)) - 1)                                \
)

#ವ್ಯಾಖ್ಯಾನಿಸು UNGETBITS(ಒಳ, n) ( \
  le += (n)                \
)


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dec_rec2(ರಚನೆ ಒಳ *ಒಳ, ರಚನೆ dec_hufftbl *hu, ಪೂರ್ಣಾಂಕ *runp,
                    ಪೂರ್ಣಾಂಕ c, ಪೂರ್ಣಾಂಕ i)
{
    LEBI_DCL;

    LEBI_GET(ಒಳ);
    ಆದರೆ (i) {
        UNGETBITS(ಒಳ, i & 127);
        *runp = i >> 8 & 15;
        i >>= 16;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಗಾಗಿ (i = DECBITS;
             (c = ((c << 1) | GETBITS(ಒಳ, 1))) >= (hu->maxcode[i]); i++);
        ಆದರೆ (i >= 16) {
            ಒಳ->marker = M_BADHUFF;
            ಹಿಂತಿರುಗಿ 0;
        }
        i = hu->vals[hu->valptr[i] + c - hu->maxcode[i - 1] * 2];
        *runp = i >> 4;
        i &= 15;
    }
    ಆದರೆ (i == 0) {                /* sigh, 0xf0 is 11 bit */
        LEBI_PUT(ಒಳ);
        ಹಿಂತಿರುಗಿ 0;
    }
    /* receive part */
    c = GETBITS(ಒಳ, i);
    ಆದರೆ (c < (1 << (i - 1)))
        c += (-1 << i) + 1;
    LEBI_PUT(ಒಳ);
    ಹಿಂತಿರುಗಿ c;
}

#ವ್ಯಾಖ್ಯಾನಿಸು DEC_REC(ಒಳ, hu, r, i)         (  \
  r = GETBITS(ಒಳ, DECBITS),              \
  i = hu->llvals[r],                     \
  i & 128 ?                              \
    (                                    \
      UNGETBITS(ಒಳ, i & 127),            \
      r = i >> 8 & 15,                   \
      i >> 16                            \
    )                                    \
  :                                      \
    (                                    \
      LEBI_PUT(ಒಳ),                      \
      i = dec_rec2(ಒಳ, hu, &r, r, i),    \
      LEBI_GET(ಒಳ),                      \
      i                                  \
    )                                    \
)

ಸ್ಥಿರ ಶೂನ್ಯ decode_mcus(ರಚನೆ ಒಳ *ಒಳ, ಪೂರ್ಣಾಂಕ *dct, ಪೂರ್ಣಾಂಕ n, ರಚನೆ ಪರಿಶೀಲನೆ *sc,
                        ಪೂರ್ಣಾಂಕ *maxp)
{
    ರಚನೆ dec_hufftbl *hu;
    ಪೂರ್ಣಾಂಕ i, r, t;
    LEBI_DCL;

    memset(dct, 0, n * 64 * ಗಾತ್ರ(*dct));
    LEBI_GET(ಒಳ);
    ಆಗ (n-- > 0) {
        hu = sc->hudc.dhuff;
        *dct++ = (sc->dc += DEC_REC(ಒಳ, hu, r, t));

        hu = sc->huac.dhuff;
        i = 63;
        ಆಗ (i > 0) {
            t = DEC_REC(ಒಳ, hu, r, t);
            ಆದರೆ (t == 0 && r == 0) {
                dct += i;
                ನಿಲ್ಲಿಸು;
            }
            dct += r;
            *dct++ = t;
            i -= r + 1;
        }
        *maxp++ = 64 - i;
        ಆದರೆ (n == sc->ಮುಂದಿನ)
            sc++;
    }
    LEBI_PUT(ಒಳ);
}

ಸ್ಥಿರ ಶೂನ್ಯ dec_makehuff(ರಚನೆ dec_hufftbl *hu, ಪೂರ್ಣಾಂಕ *hufflen,
                         ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *huffvals)
{
    ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2, k, i, j, d, x, c, v;
    ಗಾಗಿ (i = 0; i < (1 << DECBITS); i++)
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

    ಸಂಕೇತ_2 = 0;
    k = 0;
    ಗಾಗಿ (i = 0; i < 16; i++, ಸಂಕೇತ_2 <<= 1) {        /* sizes */
        hu->valptr[i] = k;
        ಗಾಗಿ (j = 0; j < hufflen[i]; j++) {
            hu->vals[k] = *huffvals++;
            ಆದರೆ (i < DECBITS) {
                c = ಸಂಕೇತ_2 << (DECBITS - 1 - i);
                v = hu->vals[k] & 0x0f;        /*ಗಾತ್ರ*/
                ಗಾಗಿ (d = 1 << (DECBITS - 1 - i); --d >= 0;) {
                    ಆದರೆ (v + i < DECBITS) {        /* both fit in table */
                        x = d >> (DECBITS - 1 - v - i);
                        ಆದರೆ (v && x < (1 << (v - 1)))
                            x += (-1 << v) + 1;
                        x = x << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1 + v)) | 128;
                    } ಇಲ್ಲದಿದ್ದರೆ
                        x = v << 16 | (hu->vals[k] & 0xf0) << 4 |
                            (DECBITS - (i + 1));
                    hu->llvals[c | d] = x;
                }
            }
            ಸಂಕೇತ_2++;
            k++;
        }
        hu->maxcode[i] = ಸಂಕೇತ_2;
    }
    hu->maxcode[16] = 0x20000;        /* always terminate decode */
}

/****************************************************************/
/**************             idct                  ***************/
/****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು ONE ((PREC)IFIX(1.))
#ವ್ಯಾಖ್ಯಾನಿಸು S2  ((PREC)IFIX(0.382683432))
#ವ್ಯಾಖ್ಯಾನಿಸು C2  ((PREC)IFIX(0.923879532))
#ವ್ಯಾಖ್ಯಾನಿಸು C4  ((PREC)IFIX(0.707106781))

#ವ್ಯಾಖ್ಯಾನಿಸು S22 ((PREC)IFIX(2 * 0.382683432))
#ವ್ಯಾಖ್ಯಾನಿಸು C22 ((PREC)IFIX(2 * 0.923879532))
#ವ್ಯಾಖ್ಯಾನಿಸು IC4 ((PREC)IFIX(1 / 0.707106781))

#ವ್ಯಾಖ್ಯಾನಿಸು C3IC1 ((PREC)IFIX(0.847759065))        /* c3/c1 */
#ವ್ಯಾಖ್ಯಾನಿಸು C5IC1 ((PREC)IFIX(0.566454497))        /* c5/c1 */
#ವ್ಯಾಖ್ಯಾನಿಸು C7IC1 ((PREC)IFIX(0.198912367))        /* c7/c1 */

#ವ್ಯಾಖ್ಯಾನಿಸು XPP(a,b) (t = a + b, b = a - b, a = t)
#ವ್ಯಾಖ್ಯಾನಿಸು XMP(a,b) (t = a - b, b = a + b, a = t)
#ವ್ಯಾಖ್ಯಾನಿಸು XPM(a,b) (t = a + b, b = b - a, a = t)

#ವ್ಯಾಖ್ಯಾನಿಸು ROT(a,b,s,c) (  t = IMULT(a + b, s),      \
                        a = IMULT(a, c - s) + t,  \
                        b = IMULT(b, c + s) - t)

#ವ್ಯಾಖ್ಯಾನಿಸು IDCT                \
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

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zig2[64] = {
     0,  2,  3,  9, 10, 20, 21, 35,
    14, 16, 25, 31, 39, 46, 50, 57,
     5,  7, 12, 18, 23, 33, 37, 48,
    27, 29, 41, 44, 52, 55, 59, 62,
    15, 26, 30, 40, 45, 51, 56, 58,
     1,  4,  8, 11, 19, 22, 34, 36,
    28, 42, 43, 53, 54, 60, 61, 63,
     6, 13, 17, 24, 32, 38, 47, 49
};

ಸ್ಥಿರ ಶೂನ್ಯ idct(ಪೂರ್ಣಾಂಕ *ಒಳ, ಪೂರ್ಣಾಂಕ *ಹೊರಹೋಗು, PREC * quant, PREC off, ಪೂರ್ಣಾಂಕ max)
{
    PREC t0, t1, t2, t3, t4, t5, t6, t7, t;
    PREC tmp[64], *tmpp;
    ಪೂರ್ಣಾಂಕ i, j;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zig2p;

    t0 = off;
    ಆದರೆ (max == 1) {
        t0 += ಒಳ[0] * quant[0];
        ಗಾಗಿ (i = 0; i < 64; i++)
            ಹೊರಹೋಗು[i] = ITOINT(t0);
        ಹಿಂತಿರುಗಿ;
    }
    zig2p = zig2;
    tmpp = tmp;
    ಗಾಗಿ (i = 0; i < 8; i++) {
        j = *zig2p++;
        t0 += ಒಳ[j] * quant[j];
        j = *zig2p++;
        t5 = ಒಳ[j] * quant[j];
        j = *zig2p++;
        t2 = ಒಳ[j] * quant[j];
        j = *zig2p++;
        t7 = ಒಳ[j] * quant[j];
        j = *zig2p++;
        t1 = ಒಳ[j] * quant[j];
        j = *zig2p++;
        t4 = ಒಳ[j] * quant[j];
        j = *zig2p++;
        t3 = ಒಳ[j] * quant[j];
        j = *zig2p++;
        t6 = ಒಳ[j] * quant[j];
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
    ಗಾಗಿ (i = 0; i < 8; i++) {
        t0 = tmp[8 * i + 0];
        t1 = tmp[8 * i + 1];
        t2 = tmp[8 * i + 2];
        t3 = tmp[8 * i + 3];
        t4 = tmp[8 * i + 4];
        t5 = tmp[8 * i + 5];
        t6 = tmp[8 * i + 6];
        t7 = tmp[8 * i + 7];
        IDCT;
        ಹೊರಹೋಗು[8 * i + 0] = ITOINT(t0);
        ಹೊರಹೋಗು[8 * i + 1] = ITOINT(t1);
        ಹೊರಹೋಗು[8 * i + 2] = ITOINT(t2);
        ಹೊರಹೋಗು[8 * i + 3] = ITOINT(t3);
        ಹೊರಹೋಗು[8 * i + 4] = ITOINT(t4);
        ಹೊರಹೋಗು[8 * i + 5] = ITOINT(t5);
        ಹೊರಹೋಗು[8 * i + 6] = ITOINT(t6);
        ಹೊರಹೋಗು[8 * i + 7] = ITOINT(t7);
    }
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zig[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

ಸ್ಥಿರ PREC aaidct[8] = {
    IFIX(0.3535533906), IFIX(0.4903926402),
    IFIX(0.4619397663), IFIX(0.4157348062),
    IFIX(0.3535533906), IFIX(0.2777851165),
    IFIX(0.1913417162), IFIX(0.0975451610)
};


ಸ್ಥಿರ ಶೂನ್ಯ idctqtab(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *qin, PREC * qout)
{
    ಪೂರ್ಣಾಂಕ i, j;

    ಗಾಗಿ (i = 0; i < 8; i++)
        ಗಾಗಿ (j = 0; j < 8; j++)
            qout[zig[i * 8 + j]] = qin[zig[i * 8 + j]] *
                IMULT(aaidct[i], aaidct[j]);
}

ಸ್ಥಿರ ಶೂನ್ಯ scaleidctqtab(PREC * q, PREC sc)
{
    ಪೂರ್ಣಾಂಕ i;

    ಗಾಗಿ (i = 0; i < 64; i++)
        q[i] = IMULT(q[i], sc);
}

/****************************************************************/
/**************          color decoder            ***************/
/****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು ಸುತ್ತು

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

ಸ್ಥಿರ ಶೂನ್ಯ initcol(PREC q[][64])
{
    scaleidctqtab(q[1], IFIX(1.77200));
    scaleidctqtab(q[2], IFIX(1.40200));
}

/* This is optimized for the stupid sun SUNWspro compiler. */
#ವ್ಯಾಖ್ಯಾನಿಸು STORECLAMP(a,x)                          \
(                                                \
  (a) = (x),                                     \
  (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(x) >= 256 ?                     \
    ((a) = (x) < 0 ? 0 : 255)                    \
  :                                              \
    0                                            \
)

#ವ್ಯಾಖ್ಯಾನಿಸು CLAMP(x) ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(x) >= 256 ? ((x) < 0 ? 0 : 255) : (x))

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಸುತ್ತು

#ವ್ಯಾಖ್ಯಾನಿಸು CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (50 * cb + 130 * cr + 128) >> 8           \
)

#ಇಲ್ಲದಿದ್ದರೆ

#ವ್ಯಾಖ್ಯಾನಿಸು CBCRCG(yin, xin)                         \
(                                                \
  cb = outc[0 +yin*8+xin],                       \
  cr = outc[64+yin*8+xin],                       \
  cg = (3 * cb + 8 * cr) >> 4                    \
)

#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN
#ವ್ಯಾಖ್ಯಾನಿಸು PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 2], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 0], y + cb)          \
)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು PIC(yin, xin, p, xout)                   \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  STORECLAMP(p[(xout) * 3 + 0], y + cr),         \
  STORECLAMP(p[(xout) * 3 + 1], y - cg),         \
  STORECLAMP(p[(xout) * 3 + 2], y + cb)          \
)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN
#ವ್ಯಾಖ್ಯಾನಿಸು PIC_16(yin, xin, p, xout, ಕೂಡಿಸು)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + ಕೂಡಿಸು*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + ಕೂಡಿಸು    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + ಕೂಡಿಸು*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y & 0xff,                  \
  p[(xout) * 2 + 1] = y >> 8                     \
)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_PPC
#ವ್ಯಾಖ್ಯಾನಿಸು PIC_16(yin, xin, p, xout, ಕೂಡಿಸು)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + ಕೂಡಿಸು*2+1) & 0xf8) <<  7) | \
      ((CLAMP(y - cg + ಕೂಡಿಸು*2+1) & 0xf8) <<  2) | \
      ((CLAMP(y + cb + ಕೂಡಿಸು*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು PIC_16(yin, xin, p, xout, ಕೂಡಿಸು)           \
(                                                \
  y = outy[(yin) * 8 + xin],                     \
  y = ((CLAMP(y + cr + ಕೂಡಿಸು*2+1) & 0xf8) <<  8) | \
      ((CLAMP(y - cg + ಕೂಡಿಸು    ) & 0xfc) <<  3) | \
      ((CLAMP(y + cb + ಕೂಡಿಸು*2+1)       ) >>  3),  \
  p[(xout) * 2 + 0] = y >> 8,                    \
  p[(xout) * 2 + 1] = y & 0xff                   \
)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು PIC_32(yin, xin, p, xout)               \
(                                               \
  y = outy[(yin) * 8 + xin],                    \
  STORECLAMP(p[(xout) * 4 + 0], y + cr),        \
  STORECLAMP(p[(xout) * 4 + 1], y - cg),        \
  STORECLAMP(p[(xout) * 4 + 2], y + cb),        \
  p[(xout) * 4 + 3] = 0                         \
)

#ವ್ಯಾಖ್ಯಾನಿಸು PIC221111(xin)                                              \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),       \
  PIC(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)        \
)

#ವ್ಯಾಖ್ಯಾನಿಸು PIC221111_16(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0, 3), \
  PIC_16(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1, 0), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0, 1), \
  PIC_16(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1, 2)  \
)

#ವ್ಯಾಖ್ಯಾನಿಸು PIC221111_32(xin)                                           \
(                                                                   \
  CBCRCG(0, xin),                                                   \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 0, pic0, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 0, (xin & 3) * 2 + 1, pic0, xin * 2 + 1),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 0, pic1, xin * 2 + 0),    \
  PIC_32(xin / 4 * 8 + 1, (xin & 3) * 2 + 1, pic1, xin * 2 + 1)     \
)

ಸ್ಥಿರ ಶೂನ್ಯ col221111(ಪೂರ್ಣಾಂಕ *ಹೊರಹೋಗು, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic, ಪೂರ್ಣಾಂಕ ಅಗಲ)
{
    ಪೂರ್ಣಾಂಕ i, j, k;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic0, *pic1;
    ಪೂರ್ಣಾಂಕ *outy, *outc;
    ಪೂರ್ಣಾಂಕ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ಅಗಲ;
    outy = ಹೊರಹೋಗು;
    outc = ಹೊರಹೋಗು + 64 * 4;
    ಗಾಗಿ (i = 2; i > 0; i--) {
        ಗಾಗಿ (j = 4; j > 0; j--) {
            ಗಾಗಿ (k = 0; k < 8; k++) {
                PIC221111(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ಅಗಲ;
            pic1 += 2 * ಅಗಲ;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ col221111_16(ಪೂರ್ಣಾಂಕ *ಹೊರಹೋಗು, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic, ಪೂರ್ಣಾಂಕ ಅಗಲ)
{
    ಪೂರ್ಣಾಂಕ i, j, k;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic0, *pic1;
    ಪೂರ್ಣಾಂಕ *outy, *outc;
    ಪೂರ್ಣಾಂಕ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ಅಗಲ;
    outy = ಹೊರಹೋಗು;
    outc = ಹೊರಹೋಗು + 64 * 4;
    ಗಾಗಿ (i = 2; i > 0; i--) {
        ಗಾಗಿ (j = 4; j > 0; j--) {
            ಗಾಗಿ (k = 0; k < 8; k++) {
                PIC221111_16(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ಅಗಲ;
            pic1 += 2 * ಅಗಲ;
        }
        outy += 64 * 2 - 16 * 4;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ col221111_32(ಪೂರ್ಣಾಂಕ *ಹೊರಹೋಗು, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic, ಪೂರ್ಣಾಂಕ ಅಗಲ)
{
    ಪೂರ್ಣಾಂಕ i, j, k;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pic0, *pic1;
    ಪೂರ್ಣಾಂಕ *outy, *outc;
    ಪೂರ್ಣಾಂಕ cr, cg, cb, y;

    pic0 = pic;
    pic1 = pic + ಅಗಲ;
    outy = ಹೊರಹೋಗು;
    outc = ಹೊರಹೋಗು + 64 * 4;
    ಗಾಗಿ (i = 2; i > 0; i--) {
        ಗಾಗಿ (j = 4; j > 0; j--) {
            ಗಾಗಿ (k = 0; k < 8; k++) {
                PIC221111_32(k);
            }
            outc += 8;
            outy += 16;
            pic0 += 2 * ಅಗಲ;
            pic1 += 2 * ಅಗಲ;
        }
        outy += 64 * 2 - 16 * 4;
    }
}
