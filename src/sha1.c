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

#அந்தர்பாவ "config.h"
#அந்தர்பாவ "byteorder.h" // cpu_to_*, __swab64
#அந்தர்பாவ "sha.h" // sha1
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "x86.h" // rol

வகைவரை கட்டமைப்பு _sha1_ctx {
    u32 h[5];
} sha1_ctx;


நிலையான வெறுமை
sha1_block(u32 *w, sha1_ctx *ctx)
{
    u32 i;
    u32 a,b,c,d,e,f;
    u32 tmp;
    u32 idx;

    நிலையான மாறிலி u32 sha_ko[4] = {
        0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

    /* change endianness of given data */
    க்கு (i = 0; i < 16; i++)
        w[i] = be32_to_cpu(w[i]);

    க்கு (i = 16; i <= 79; i++) {
        tmp = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
        w[i] = rol(tmp,1);
    }

    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];

    க்கு (i = 0; i <= 79; i++) {
        எனில் (i <= 19) {
            f = (b & c) | ((b ^ 0xffffffff) & d);
            idx = 0;
        } இல்லையெனில் எனில் (i <= 39) {
            f = b ^ c ^ d;
            idx = 1;
        } இல்லையெனில் எனில் (i <= 59) {
            f = (b & c) | (b & d) | (c & d);
            idx = 2;
        } இல்லையெனில் {
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


நிலையான வெறுமை
sha1_do(sha1_ctx *ctx, மாறிலி u8 *data32, u32 length)
{
    u32 offset;
    u16 இலக்கம்;
    u32 bits = 0;
    u32 w[80];
    u64 tmp;

    /* treat data in 64-byte chunks */
    க்கு (offset = 0; length - offset >= 64; offset += 64) {
        memcpy(w, data32 + offset, 64);
        sha1_block((u32 *)w, ctx);
        bits += (64 * 8);
    }

    /* last block with less than 64 bytes */
    இலக்கம் = length - offset;
    bits += (இலக்கம் << 3);

    memcpy(w, data32 + offset, இலக்கம்);
    ((u8 *)w)[இலக்கம்] = 0x80;
    எனில் (64 - (இலக்கம் + 1) > 0)
        memset( &((u8 *)w)[இலக்கம் + 1], 0x0, 64 - (இலக்கம் + 1));

    எனில் (இலக்கம் >= 56) {
        /* cannot append number of bits here */
        sha1_block((u32 *)w, ctx);
        memset(w, 0x0, 60);
    }

    /* write number of bits to end of block */
    tmp = __swab64(bits);
    memcpy(&w[14], &tmp, 8);

    sha1_block(w, ctx);

    /* need to switch result's endianness */
    க்கு (இலக்கம் = 0; இலக்கம் < 5; இலக்கம்++)
        ctx->h[இலக்கம்] = cpu_to_be32(ctx->h[இலக்கம்]);
}


வெறுமை
sha1(மாறிலி u8 *data, u32 length, u8 *hash)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு;

    sha1_ctx ctx = {
        .h[0] = 0x67452301,
        .h[1] = 0xefcdab89,
        .h[2] = 0x98badcfe,
        .h[3] = 0x10325476,
        .h[4] = 0xc3d2e1f0,
    };

    sha1_do(&ctx, data, length);
    memcpy(hash, &ctx.h[0], 20);

    திரும்பு;
}
