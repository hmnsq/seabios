#ifndef __SHA_H
#define __SHA_H

#অন্তর্ভাব "types.h" // u32

শূন্য sha1(ধ্রুবক u8 *তথ্য, u32 দৈর্ঘ্য, u8 *hash);
শূন্য sha256(ধ্রুবক u8 *তথ্য, u32 দৈর্ঘ্য, u8 *hash);
শূন্য sha384(ধ্রুবক u8 *তথ্য, u32 দৈর্ঘ্য, u8 *hash);
শূন্য sha512(ধ্রুবক u8 *তথ্য, u32 দৈর্ঘ্য, u8 *hash);

#endif // sha.h
