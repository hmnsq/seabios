#ifndef __SHA_H
#define __SHA_H

#अन्तर्भाव "types.h" // u32

शून्य sha1(ध्रुव u8 *data, u32 length, u8 *hash);
शून्य sha256(ध्रुव u8 *data, u32 length, u8 *hash);
शून्य sha384(ध्रुव u8 *data, u32 length, u8 *hash);
शून्य sha512(ध्रुव u8 *data, u32 length, u8 *hash);

#endif // sha.h
