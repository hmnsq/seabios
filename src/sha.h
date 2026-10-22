#ifndef __SHA_H
#define __SHA_H

#અન્તર્ભાવ "types.h" // u32

શૂન્ય sha1(અચલ u8 *data, u32 length, u8 *hash);
શૂન્ય sha256(અચલ u8 *data, u32 length, u8 *hash);
શૂન્ય sha384(અચલ u8 *data, u32 length, u8 *hash);
શૂન્ય sha512(અચલ u8 *data, u32 length, u8 *hash);

#endif // sha.h
