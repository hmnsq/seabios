#ifndef __SHA_H
#define __SHA_H

#ضمن "types.h" // u32

فراغ sha1(ثابت u8 *data, u32 length, u8 *hash);
فراغ sha256(ثابت u8 *data, u32 length, u8 *hash);
فراغ sha384(ثابت u8 *data, u32 length, u8 *hash);
فراغ sha512(ثابت u8 *data, u32 length, u8 *hash);

#endif // sha.h
