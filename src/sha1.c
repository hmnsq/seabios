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

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // cpu_to_*, __swab64
#ਅਨ੍ਤਰ੍ਭਾਵ "sha.h" // sha1
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // rol

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ _sha1_ctx {
    u32 h[5];
} sha1_ctx;


ਸਥਿਰ ਖਾਲੀ
sha1_ਖੰਡ(u32 *w, sha1_ctx *ctx)
{
    u32 i;
    u32 a,b,c,d,e,f;
    u32 tmp;
    u32 idx;

    ਸਥਿਰ ਅਚਲ u32 sha_ko[4] = {
        0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

    /* change endianness of given data */
    ਲਈ (i = 0; i < 16; i++)
        w[i] = be32_ਮੰਜ਼ਿਲ_cpu(w[i]);

    ਲਈ (i = 16; i <= 79; i++) {
        tmp = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
        w[i] = rol(tmp,1);
    }

    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];

    ਲਈ (i = 0; i <= 79; i++) {
        ਜੇ (i <= 19) {
            f = (b & c) | ((b ^ 0xffffffff) & d);
            idx = 0;
        } ਨਹੀਂਤਾਂ ਜੇ (i <= 39) {
            f = b ^ c ^ d;
            idx = 1;
        } ਨਹੀਂਤਾਂ ਜੇ (i <= 59) {
            f = (b & c) | (b & d) | (c & d);
            idx = 2;
        } ਨਹੀਂਤਾਂ {
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


ਸਥਿਰ ਖਾਲੀ
sha1_do(sha1_ctx *ctx, ਅਚਲ u8 *data32, u32 ਲੰਮਾਈ)
{
    u32 ਵਿੱਥ_2;
    u16 ਅੰਕ;
    u32 ਬਿੱਟ = 0;
    u32 w[80];
    u64 tmp;

    /* treat data in 64-byte chunks */
    ਲਈ (ਵਿੱਥ_2 = 0; ਲੰਮਾਈ - ਵਿੱਥ_2 >= 64; ਵਿੱਥ_2 += 64) {
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(w, data32 + ਵਿੱਥ_2, 64);
        sha1_ਖੰਡ((u32 *)w, ctx);
        ਬਿੱਟ += (64 * 8);
    }

    /* last block with less than 64 bytes */
    ਅੰਕ = ਲੰਮਾਈ - ਵਿੱਥ_2;
    ਬਿੱਟ += (ਅੰਕ << 3);

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(w, data32 + ਵਿੱਥ_2, ਅੰਕ);
    ((u8 *)w)[ਅੰਕ] = 0x80;
    ਜੇ (64 - (ਅੰਕ + 1) > 0)
        memset( &((u8 *)w)[ਅੰਕ + 1], 0x0, 64 - (ਅੰਕ + 1));

    ਜੇ (ਅੰਕ >= 56) {
        /* cannot append number of bits here */
        sha1_ਖੰਡ((u32 *)w, ctx);
        memset(w, 0x0, 60);
    }

    /* write number of bits to end of block */
    tmp = __swab64(ਬਿੱਟ);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&w[14], &tmp, 8);

    sha1_ਖੰਡ(w, ctx);

    /* need to switch result's endianness */
    ਲਈ (ਅੰਕ = 0; ਅੰਕ < 5; ਅੰਕ++)
        ctx->h[ਅੰਕ] = cpu_ਮੰਜ਼ਿਲ_be32(ctx->h[ਅੰਕ]);
}


ਖਾਲੀ
sha1(ਅਚਲ u8 *ਅੰਕੜੇ, u32 ਲੰਮਾਈ, u8 *hash)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ;

    sha1_ctx ctx = {
        .h[0] = 0x67452301,
        .h[1] = 0xefcdab89,
        .h[2] = 0x98badcfe,
        .h[3] = 0x10325476,
        .h[4] = 0xc3d2e1f0,
    };

    sha1_do(&ctx, ਅੰਕੜੇ, ਲੰਮਾਈ);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(hash, &ctx.h[0], 20);

    ਵਾਪਸ;
}
