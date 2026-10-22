#ifndef __SHA_H
#define __SHA_H

#شامل "types.h" // u32

تهی sha1(ثابت u8 *data, u32 length, u8 *hash);
تهی sha256(ثابت u8 *data, u32 length, u8 *hash);
تهی sha384(ثابت u8 *data, u32 length, u8 *hash);
تهی sha512(ثابت u8 *data, u32 length, u8 *hash);

#endif // sha.h
