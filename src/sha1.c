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

#ಅನ್ತರ್ಭಾವ "config.h"
#ಅನ್ತರ್ಭಾವ "byteorder.h" // cpu_to_*, __swab64
#ಅನ್ತರ್ಭಾವ "sha.h" // sha1
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "x86.h" // rol

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ _sha1_ctx {
    u32 h[5];
} sha1_ctx;


ಸ್ಥಿರ ಶೂನ್ಯ
sha1_ಖಂಡ(u32 *w, sha1_ctx *ctx)
{
    u32 i;
    u32 a,b,c,d,e,f;
    u32 tmp;
    u32 idx;

    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u32 sha_ko[4] = {
        0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

    /* change endianness of given data */
    ಗಾಗಿ (i = 0; i < 16; i++)
        w[i] = be32_ಗಮ್ಯ_cpu(w[i]);

    ಗಾಗಿ (i = 16; i <= 79; i++) {
        tmp = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
        w[i] = rol(tmp,1);
    }

    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];

    ಗಾಗಿ (i = 0; i <= 79; i++) {
        ಆದರೆ (i <= 19) {
            f = (b & c) | ((b ^ 0xffffffff) & d);
            idx = 0;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (i <= 39) {
            f = b ^ c ^ d;
            idx = 1;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (i <= 59) {
            f = (b & c) | (b & d) | (c & d);
            idx = 2;
        } ಇಲ್ಲದಿದ್ದರೆ {
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


ಸ್ಥಿರ ಶೂನ್ಯ
sha1_do(sha1_ctx *ctx, ಸ್ಥಿರಾಂಕ u8 *data32, u32 ಉದ್ದಳತೆ)
{
    u32 ಸ್ಥಾನಾಂತರ_2;
    u16 ಅಂಕೆ;
    u32 ಬಿಟ್ಟುಗಳು = 0;
    u32 w[80];
    u64 tmp;

    /* treat data in 64-byte chunks */
    ಗಾಗಿ (ಸ್ಥಾನಾಂತರ_2 = 0; ಉದ್ದಳತೆ - ಸ್ಥಾನಾಂತರ_2 >= 64; ಸ್ಥಾನಾಂತರ_2 += 64) {
        ಸ್ಮರಣೆ_ನಕಲು(w, data32 + ಸ್ಥಾನಾಂತರ_2, 64);
        sha1_ಖಂಡ((u32 *)w, ctx);
        ಬಿಟ್ಟುಗಳು += (64 * 8);
    }

    /* last block with less than 64 bytes */
    ಅಂಕೆ = ಉದ್ದಳತೆ - ಸ್ಥಾನಾಂತರ_2;
    ಬಿಟ್ಟುಗಳು += (ಅಂಕೆ << 3);

    ಸ್ಮರಣೆ_ನಕಲು(w, data32 + ಸ್ಥಾನಾಂತರ_2, ಅಂಕೆ);
    ((u8 *)w)[ಅಂಕೆ] = 0x80;
    ಆದರೆ (64 - (ಅಂಕೆ + 1) > 0)
        memset( &((u8 *)w)[ಅಂಕೆ + 1], 0x0, 64 - (ಅಂಕೆ + 1));

    ಆದರೆ (ಅಂಕೆ >= 56) {
        /* cannot append number of bits here */
        sha1_ಖಂಡ((u32 *)w, ctx);
        memset(w, 0x0, 60);
    }

    /* write number of bits to end of block */
    tmp = __swab64(ಬಿಟ್ಟುಗಳು);
    ಸ್ಮರಣೆ_ನಕಲು(&w[14], &tmp, 8);

    sha1_ಖಂಡ(w, ctx);

    /* need to switch result's endianness */
    ಗಾಗಿ (ಅಂಕೆ = 0; ಅಂಕೆ < 5; ಅಂಕೆ++)
        ctx->h[ಅಂಕೆ] = cpu_ಗಮ್ಯ_be32(ctx->h[ಅಂಕೆ]);
}


ಶೂನ್ಯ
sha1(ಸ್ಥಿರಾಂಕ u8 *ದತ್ತಾಂಶ, u32 ಉದ್ದಳತೆ, u8 *hash)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ;

    sha1_ctx ctx = {
        .h[0] = 0x67452301,
        .h[1] = 0xefcdab89,
        .h[2] = 0x98badcfe,
        .h[3] = 0x10325476,
        .h[4] = 0xc3d2e1f0,
    };

    sha1_do(&ctx, ದತ್ತಾಂಶ, ಉದ್ದಳತೆ);
    ಸ್ಮರಣೆ_ನಕಲು(hash, &ctx.h[0], 20);

    ಹಿಂತಿರುಗಿ;
}
