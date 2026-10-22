#ifndef __SHA_H
#define __SHA_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

ଶୂନ୍ୟ sha1(ଅଚଳ u8 *data, u32 length, u8 *hash);
ଶୂନ୍ୟ sha256(ଅଚଳ u8 *data, u32 length, u8 *hash);
ଶୂନ୍ୟ sha384(ଅଚଳ u8 *data, u32 length, u8 *hash);
ଶୂନ୍ୟ sha512(ଅଚଳ u8 *data, u32 length, u8 *hash);

#endif // sha.h
