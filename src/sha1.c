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

#समावेश "config.h"
#समावेश "byteorder.h" // cpu_to_*, __swab64
#समावेश "sha.h" // sha1
#समावेश "string.h" // memcpy
#समावेश "x86.h" // rol

प्रकार काष्ठा _sha1_ctx {
    u32 h[5];
} sha1_ctx;


अटल व्योम
sha1_block(u32 *w, sha1_ctx *ctx)
{
    u32 i;
    u32 a,b,c,d,e,f;
    u32 tmp;
    u32 idx;

    अटल स्थिर u32 sha_ko[4] = {
        0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

    /* change endianness of given data */
    क्रम (i = 0; i < 16; i++)
        w[i] = be32_to_cpu(w[i]);

    क्रम (i = 16; i <= 79; i++) {
        tmp = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
        w[i] = rol(tmp,1);
    }

    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];

    क्रम (i = 0; i <= 79; i++) {
        अगर (i <= 19) {
            f = (b & c) | ((b ^ 0xffffffff) & d);
            idx = 0;
        } अन्यथा अगर (i <= 39) {
            f = b ^ c ^ d;
            idx = 1;
        } अन्यथा अगर (i <= 59) {
            f = (b & c) | (b & d) | (c & d);
            idx = 2;
        } अन्यथा {
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


अटल व्योम
sha1_do(sha1_ctx *ctx, स्थिर u8 *data32, u32 लंबाई)
{
    u32 offset;
    u16 अंक;
    u32 bits = 0;
    u32 w[80];
    u64 tmp;

    /* treat data in 64-byte chunks */
    क्रम (offset = 0; लंबाई - offset >= 64; offset += 64) {
        स_नकल(w, data32 + offset, 64);
        sha1_block((u32 *)w, ctx);
        bits += (64 * 8);
    }

    /* last block with less than 64 bytes */
    अंक = लंबाई - offset;
    bits += (अंक << 3);

    स_नकल(w, data32 + offset, अंक);
    ((u8 *)w)[अंक] = 0x80;
    अगर (64 - (अंक + 1) > 0)
        स_रखो( &((u8 *)w)[अंक + 1], 0x0, 64 - (अंक + 1));

    अगर (अंक >= 56) {
        /* cannot append number of bits here */
        sha1_block((u32 *)w, ctx);
        स_रखो(w, 0x0, 60);
    }

    /* write number of bits to end of block */
    tmp = __swab64(bits);
    स_नकल(&w[14], &tmp, 8);

    sha1_block(w, ctx);

    /* need to switch result's endianness */
    क्रम (अंक = 0; अंक < 5; अंक++)
        ctx->h[अंक] = cpu_to_be32(ctx->h[अंक]);
}


व्योम
sha1(स्थिर u8 *तथ्य, u32 लंबाई, u8 *hash)
{
    अगर (!CONFIG_TCGBIOS)
        वापस;

    sha1_ctx ctx = {
        .h[0] = 0x67452301,
        .h[1] = 0xefcdab89,
        .h[2] = 0x98badcfe,
        .h[3] = 0x10325476,
        .h[4] = 0xc3d2e1f0,
    };

    sha1_do(&ctx, तथ्य, लंबाई);
    स_नकल(hash, &ctx.h[0], 20);

    वापस;
}
