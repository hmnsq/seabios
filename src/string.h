// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#include "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, void *buf_far, u32 len);
u8 checksum(void *buf, u32 len);
size_t strlen(const var_na *s);
จำนวนเต็ม memcmp_far(u16 s1seg, const void *s1, u16 s2seg, const void *s2, size_t n);
จำนวนเต็ม memcmp(const void *s1, const void *s2, size_t n);
จำนวนเต็ม strcmp(const var_na *s1, const var_na *s2);
void memset_far(u16 d_seg, void *d_far, u8 c, size_t len);
void memset16_far(u16 d_seg, void *d_far, u16 c, size_t len);
void *memset(void *s, จำนวนเต็ม c, size_t n);
void memset_fl(void *ptr, u8 val, size_t size);
void memcpy_far(u16 d_seg, void *d_far
                , u16 s_seg, const void *s_far, size_t len);
void memcpy_fl(void *d_fl, const void *s_fl, size_t len);
void *memcpy(void *d1, const void *s1, size_t len);
#ถ้า MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
void iomemcpy(void *d, const void *s, u32 len);
void *memmove(void *d, const void *s, size_t len);
var_na *strtcpy(var_na *dest, const var_na *src, size_t len);
var_na *strchr(const var_na *s, จำนวนเต็ม c);
var_na *nullTrailingSpace(var_na *buf);

#endif // string.h
