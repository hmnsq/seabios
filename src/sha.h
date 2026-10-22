#ifndef __SHA_H
#define __SHA_H

#অন্তর্ভাব "types.h" // u32

শূন্য sha1(ধ্রুব u8 *data, u32 length, u8 *hash);
শূন্য sha256(ধ্রুব u8 *data, u32 length, u8 *hash);
শূন্য sha384(ধ্রুব u8 *data, u32 length, u8 *hash);
শূন্য sha512(ধ্রুব u8 *data, u32 length, u8 *hash);

#endif // sha.h
