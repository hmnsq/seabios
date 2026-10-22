#ifndef __SHA_H
#define __SHA_H

#अन्तर्भाव "types.h" // u32

शून्य sha1(स्थिरांक u8 *data, u32 length, u8 *hash);
शून्य sha256(स्थिरांक u8 *data, u32 length, u8 *hash);
शून्य sha384(स्थिरांक u8 *data, u32 length, u8 *hash);
शून्य sha512(स्थिरांक u8 *data, u32 length, u8 *hash);

#endif // sha.h
