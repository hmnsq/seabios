/*****************************************************************************
 * Copyright (c) 2015-2020 IBM Corporation
 * All rights reserved.
 * This program and the accompanying materials
 * are made available under the terms of the BSD License
 * which accompanies this distribution, and is available at
 * http://www.opensource.org/licenses/bsd-license.php
 *
 * Contributors:
 *     IBM Corporation - initial implementation
 *****************************************************************************/

/*
 *  See: NIST standard for SHA-256 in FIPS PUB 180-4
 */

#ضمن "config.h"
#ضمن "byteorder.h"
#ضمن "sha.h"
#ضمن "string.h"
#ضمن "x86.h"

تعريفنوع بنية _sha256_ctx {
    u32 h[8];
} sha256_ctx;

ساكن مضمن u32 Ch(u32 x, u32 y, u32 z)
{
    أرجع (x & y) | ((x ^ 0xffffffff) & z);
}

ساكن مضمن u32 Maj(u32 x, u32 y, u32 z)
{
    أرجع (x & y) | (x & z) | (y & z);
}

ساكن مضمن u32 sum0(u32 x)
{
    أرجع ror(x, 2) ^ ror(x, 13) ^ ror(x, 22);
}

ساكن مضمن u32 sum1(u32 x)
{
    أرجع ror(x, 6) ^ ror(x, 11) ^ ror(x, 25);
}

ساكن مضمن u32 sigma0(u32 x)
{
    أرجع ror(x, 7) ^ ror(x, 18) ^ (x >> 3);
}

ساكن مضمن u32 sigma1(u32 x)
{
    أرجع ror(x, 17) ^ ror(x, 19) ^ (x >> 10);
}

ساكن فراغ sha256_block(u32 *w, sha256_ctx *ctx)
{
    u32 t;
    u32 a, b, c, d, e, f, g, h;
    u32 T1, T2;

    /*
     * FIPS 180-4 4.2.2: SHA256 Constants
     */
    ساكن ثابت u32 sha_ko[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    /*
     * FIPS 180-4 6.2.2: step 1
     *
     *  0 <= i <= 15:
     *    W(t) = M(t)
     * 16 <= i <= 63:
     *    W(t) = sigma1(W(t-2)) + W(t-7) + sigma0(W(t-15)) + W(t-16)
     */

    /* w(0)..w(15) are in big endian format */
    لكل (t = 0; t <= 15; t++)
        w[t] = be32_to_cpu(w[t]);

    لكل (t = 16; t <= 63; t++)
        w[t] = sigma1(w[t-2]) + w[t-7] + sigma0(w[t-15]) + w[t-16];

    /*
     * step 2: a = H0, b = H1, c = H2, d = H3, e = H4, f = H5, g = H6, h = H7
     */
    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];
    f = ctx->h[5];
    g = ctx->h[6];
    h = ctx->h[7];

    /*
     * step 3: For i = 0 to 63:
     *    T1 = h + sum1(e) + Ch(e,f,g) + K(t) + W(t);
     *    T2 = sum0(a) + Maj(a,b,c)
     *    h = g; g = f; f = e; e = d + T1; d = c; c = b; b = a; a + T1 + T2
     */
    لكل (t = 0; t <= 63; t++) {
        T1 = h + sum1(e) + Ch(e, f, g) + sha_ko[t] + w[t];
        T2 = sum0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    /*
     * step 4:
     *    H0 = a + H0, H1 = b + H1, H2 = c + H2, H3 = d + H3, H4 = e + H4
     */
    ctx->h[0] += a;
    ctx->h[1] += b;
    ctx->h[2] += c;
    ctx->h[3] += d;
    ctx->h[4] += e;
    ctx->h[5] += f;
    ctx->h[6] += g;
    ctx->h[7] += h;
}

ساكن فراغ sha256_do(sha256_ctx *ctx, ثابت u8 *data32, u32 length)
{
    u32 offset;
    u16 رقم;
    u32 bits = 0;
    u32 w[64];
    u64 tmp;

    /* treat data in 64-byte chunks */
    لكل (offset = 0; length - offset >= 64; offset += 64) {
        memcpy(w, data32 + offset, 64);
        sha256_block((u32 *)w, ctx);
        bits += (64 * 8);
    }

    /* last block with less than 64 bytes */
    رقم = length - offset;
    bits += (رقم << 3);

    memcpy(w, data32 + offset, رقم);
    /*
     * FIPS 180-4 5.1: Padding the Message
     */
    ((u8 *)w)[رقم] = 0x80;
    إذا (64 - (رقم + 1) > 0)
        memset( &((u8 *)w)[رقم + 1], 0, 64 - (رقم + 1));

    إذا (رقم >= 56) {
        /* cannot append number of bits here */
        sha256_block((u32 *)w, ctx);
        memset(w, 0, 60);
    }

    /* write number of bits to end of block */
    tmp = cpu_to_be64(bits);
    memcpy(&w[14], &tmp, 8);

    sha256_block(w, ctx);

    /* need to switch result's endianness */
    لكل (رقم = 0; رقم < 8; رقم++)
        ctx->h[رقم] = cpu_to_be32(ctx->h[رقم]);
}

فراغ sha256(ثابت u8 *data, u32 length, u8 *hash)
{
    sha256_ctx ctx = {
        .h = {
            /*
             * FIPS 180-4: 6.2.1
             *   -> 5.3.3: initial hash value
             */
            0x6a09e667,
            0xbb67ae85,
            0x3c6ef372,
            0xa54ff53a,
            0x510e527f,
            0x9b05688c,
            0x1f83d9ab,
            0x5be0cd19
        }
    };

    sha256_do(&ctx, data, length);
    memcpy(hash, ctx.h, حجم(ctx.h));
}
