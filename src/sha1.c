//  Support for Calculation of SHA1 in SW
//
//  Copyright (C) 2006-2011 IBM Corporation
//
//  Authors:
//      Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.
//
//  See: http://www.itl.nist.gov/fipspubs/fip180-1.htm
//       RFC3174, Wikipedia's SHA1 alogrithm description
//

#అన్తర్భావ "config.h"
#అన్తర్భావ "byteorder.h" // cpu_to_*, __swab64
#అన్తర్భావ "sha.h" // sha1
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "x86.h" // rol

రకనిర్వచనం నిర్మాణం _sha1_ctx {
    u32 h[5];
} sha1_ctx;


స్థిర శూన్య
sha1_ఖండం(u32 *w, sha1_ctx *ctx)
{
    u32 i;
    u32 a,b,c,d,e,f;
    u32 tmp;
    u32 idx;

    స్థిర స్థిరరాశి u32 sha_ko[4] = {
        0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

    /* change endianness of given data */
    కొరకు (i = 0; i < 16; i++)
        w[i] = be32_గమ్యం_cpu(w[i]);

    కొరకు (i = 16; i <= 79; i++) {
        tmp = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
        w[i] = rol(tmp,1);
    }

    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];

    కొరకు (i = 0; i <= 79; i++) {
        ఒకవేళ (i <= 19) {
            f = (b & c) | ((b ^ 0xffffffff) & d);
            idx = 0;
        } లేకపోతే ఒకవేళ (i <= 39) {
            f = b ^ c ^ d;
            idx = 1;
        } లేకపోతే ఒకవేళ (i <= 59) {
            f = (b & c) | (b & d) | (c & d);
            idx = 2;
        } లేకపోతే {
            f = b ^ c ^ d;
            idx = 3;
        }

        tmp = rol(a, 5) +
              f +
              e +
              sha_ko[idx] +
              w[i];
        e = d;
        d = c;
        c = rol(b, 30);
        b = a;
        a = tmp;
    }

    ctx->h[0] += a;
    ctx->h[1] += b;
    ctx->h[2] += c;
    ctx->h[3] += d;
    ctx->h[4] += e;
}


స్థిర శూన్య
sha1_do(sha1_ctx *ctx, స్థిరరాశి u8 *data32, u32 నిడివి)
{
    u32 స్థానభ్రంశం_2;
    u16 అంకె;
    u32 బిట్లు = 0;
    u32 w[80];
    u64 tmp;

    /* treat data in 64-byte chunks */
    కొరకు (స్థానభ్రంశం_2 = 0; నిడివి - స్థానభ్రంశం_2 >= 64; స్థానభ్రంశం_2 += 64) {
        జ్ఞాపకం_నకలు(w, data32 + స్థానభ్రంశం_2, 64);
        sha1_ఖండం((u32 *)w, ctx);
        బిట్లు += (64 * 8);
    }

    /* last block with less than 64 bytes */
    అంకె = నిడివి - స్థానభ్రంశం_2;
    బిట్లు += (అంకె << 3);

    జ్ఞాపకం_నకలు(w, data32 + స్థానభ్రంశం_2, అంకె);
    ((u8 *)w)[అంకె] = 0x80;
    ఒకవేళ (64 - (అంకె + 1) > 0)
        memset( &((u8 *)w)[అంకె + 1], 0x0, 64 - (అంకె + 1));

    ఒకవేళ (అంకె >= 56) {
        /* cannot append number of bits here */
        sha1_ఖండం((u32 *)w, ctx);
        memset(w, 0x0, 60);
    }

    /* write number of bits to end of block */
    tmp = __swab64(బిట్లు);
    జ్ఞాపకం_నకలు(&w[14], &tmp, 8);

    sha1_ఖండం(w, ctx);

    /* need to switch result's endianness */
    కొరకు (అంకె = 0; అంకె < 5; అంకె++)
        ctx->h[అంకె] = cpu_గమ్యం_be32(ctx->h[అంకె]);
}


శూన్య
sha1(స్థిరరాశి u8 *దత్తాంశం, u32 నిడివి, u8 *hash)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి;

    sha1_ctx ctx = {
        .h[0] = 0x67452301,
        .h[1] = 0xefcdab89,
        .h[2] = 0x98badcfe,
        .h[3] = 0x10325476,
        .h[4] = 0xc3d2e1f0,
    };

    sha1_do(&ctx, దత్తాంశం, నిడివి);
    జ్ఞాపకం_నకలు(hash, &ctx.h[0], 20);

    తిరిగి;
}
