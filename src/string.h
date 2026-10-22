// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#شامل "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, تهی *buf_far, u32 len);
u8 checksum(تهی *buf, u32 len);
size_t strlen(ثابت نویسه *s);
صحیح memcmp_far(u16 s1seg, ثابت تهی *s1, u16 s2seg, ثابت تهی *s2, size_t n);
صحیح memcmp(ثابت تهی *s1, ثابت تهی *s2, size_t n);
صحیح strcmp(ثابت نویسه *s1, ثابت نویسه *s2);
تهی memset_far(u16 d_seg, تهی *d_far, u8 c, size_t len);
تهی memset16_far(u16 d_seg, تهی *d_far, u16 c, size_t len);
تهی *memset(تهی *s, صحیح c, size_t n);
تهی memset_fl(تهی *ptr, u8 val, size_t size);
تهی memcpy_far(u16 d_seg, تهی *d_far
                , u16 s_seg, ثابت تهی *s_far, size_t len);
تهی memcpy_fl(تهی *d_fl, ثابت تهی *s_fl, size_t len);
تهی *memcpy(تهی *d1, ثابت تهی *s1, size_t len);
#اگر MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
تهی iomemcpy(تهی *d, ثابت تهی *s, u32 len);
تهی *memmove(تهی *d, ثابت تهی *s, size_t len);
نویسه *strtcpy(نویسه *dest, ثابت نویسه *src, size_t len);
نویسه *strchr(ثابت نویسه *s, صحیح c);
نویسه *nullTrailingSpace(نویسه *buf);

#endif // string.h
