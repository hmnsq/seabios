#ifndef __SHA_H
#define __SHA_H

#അന്തര്ഭാവ "types.h" // u32

ശൂന്യം sha1(സ്ഥിരം u8 *data, u32 length, u8 *hash);
ശൂന്യം sha256(സ്ഥിരം u8 *data, u32 length, u8 *hash);
ശൂന്യം sha384(സ്ഥിരം u8 *data, u32 length, u8 *hash);
ശൂന്യം sha512(സ്ഥിരം u8 *data, u32 length, u8 *hash);

#endif // sha.h
