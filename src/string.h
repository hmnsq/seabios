// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#അന്തര്ഭാവ "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, ശൂന്യം *buf_far, u32 len);
u8 checksum(ശൂന്യം *buf, u32 len);
size_t strlen(സ്ഥിരം അക്ഷരം *s);
പൂർണ്ണസംഖ്യ memcmp_far(u16 s1seg, സ്ഥിരം ശൂന്യം *s1, u16 s2seg, സ്ഥിരം ശൂന്യം *s2, size_t n);
പൂർണ്ണസംഖ്യ memcmp(സ്ഥിരം ശൂന്യം *s1, സ്ഥിരം ശൂന്യം *s2, size_t n);
പൂർണ്ണസംഖ്യ strcmp(സ്ഥിരം അക്ഷരം *s1, സ്ഥിരം അക്ഷരം *s2);
ശൂന്യം memset_far(u16 d_seg, ശൂന്യം *d_far, u8 c, size_t len);
ശൂന്യം memset16_far(u16 d_seg, ശൂന്യം *d_far, u16 c, size_t len);
ശൂന്യം *memset(ശൂന്യം *s, പൂർണ്ണസംഖ്യ c, size_t n);
ശൂന്യം memset_fl(ശൂന്യം *ptr, u8 val, size_t size);
ശൂന്യം memcpy_far(u16 d_seg, ശൂന്യം *d_far
                , u16 s_seg, സ്ഥിരം ശൂന്യം *s_far, size_t len);
ശൂന്യം memcpy_fl(ശൂന്യം *d_fl, സ്ഥിരം ശൂന്യം *s_fl, size_t len);
ശൂന്യം *memcpy(ശൂന്യം *d1, സ്ഥിരം ശൂന്യം *s1, size_t len);
#എങ്കിൽ MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
ശൂന്യം iomemcpy(ശൂന്യം *d, സ്ഥിരം ശൂന്യം *s, u32 len);
ശൂന്യം *memmove(ശൂന്യം *d, സ്ഥിരം ശൂന്യം *s, size_t len);
അക്ഷരം *strtcpy(അക്ഷരം *dest, സ്ഥിരം അക്ഷരം *src, size_t len);
അക്ഷരം *strchr(സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ c);
അക്ഷരം *nullTrailingSpace(അക്ഷരം *buf);

#endif // string.h
