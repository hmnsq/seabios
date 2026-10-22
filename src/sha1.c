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

#ضمن "config.h"
#ضمن "byteorder.h" // cpu_to_*, __swab64
#ضمن "sha.h" // sha1
#ضمن "string.h" // memcpy
#ضمن "x86.h" // rol

تعريفنوع بنية _sha1_ctx {
    u32 h[5];
} sha1_ctx;


ساكن فراغ
sha1_بلوک(u32 *w, sha1_ctx *ctx)
{
    u32 i;
    u32 a,b,c,d,e,f;
    u32 tmp;
    u32 idx;

    ساكن ثابت u32 sha_ko[4] = {
        0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

    /* change endianness of given data */
    لكل (i = 0; i < 16; i++)
        w[i] = be32_مقصد_cpu(w[i]);

    لكل (i = 16; i <= 79; i++) {
        tmp = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
        w[i] = rol(tmp,1);
    }

    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];

    لكل (i = 0; i <= 79; i++) {
        إذا (i <= 19) {
            f = (b & c) | ((b ^ 0xffffffff) & d);
            idx = 0;
        } وإلا إذا (i <= 39) {
            f = b ^ c ^ d;
            idx = 1;
        } وإلا إذا (i <= 59) {
            f = (b & c) | (b & d) | (c & d);
            idx = 2;
        } وإلا {
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


ساكن فراغ
sha1_do(sha1_ctx *ctx, ثابت u8 *data32, u32 درازی)
{
    u32 جابه_جایی_2;
    u16 رقم;
    u32 بیتها = 0;
    u32 w[80];
    u64 tmp;

    /* treat data in 64-byte chunks */
    لكل (جابه_جایی_2 = 0; درازی - جابه_جایی_2 >= 64; جابه_جایی_2 += 64) {
        کپی_حافظه(w, data32 + جابه_جایی_2, 64);
        sha1_بلوک((u32 *)w, ctx);
        بیتها += (64 * 8);
    }

    /* last block with less than 64 bytes */
    رقم = درازی - جابه_جایی_2;
    بیتها += (رقم << 3);

    کپی_حافظه(w, data32 + جابه_جایی_2, رقم);
    ((u8 *)w)[رقم] = 0x80;
    إذا (64 - (رقم + 1) > 0)
        memset( &((u8 *)w)[رقم + 1], 0x0, 64 - (رقم + 1));

    إذا (رقم >= 56) {
        /* cannot append number of bits here */
        sha1_بلوک((u32 *)w, ctx);
        memset(w, 0x0, 60);
    }

    /* write number of bits to end of block */
    tmp = __swab64(بیتها);
    کپی_حافظه(&w[14], &tmp, 8);

    sha1_بلوک(w, ctx);

    /* need to switch result's endianness */
    لكل (رقم = 0; رقم < 5; رقم++)
        ctx->h[رقم] = cpu_مقصد_be32(ctx->h[رقم]);
}


فراغ
sha1(ثابت u8 *داده, u32 درازی, u8 *hash)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع;

    sha1_ctx ctx = {
        .h[0] = 0x67452301,
        .h[1] = 0xefcdab89,
        .h[2] = 0x98badcfe,
        .h[3] = 0x10325476,
        .h[4] = 0xc3d2e1f0,
    };

    sha1_do(&ctx, داده, درازی);
    کپی_حافظه(hash, &ctx.h[0], 20);

    أرجع;
}
