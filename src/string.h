// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#அந்தர்பாவ "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, வெறுமை *buf_far, u32 len);
u8 checksum(வெறுமை *buf, u32 len);
size_t strlen(மாறிலி எழுத்து *s);
முழு memcmp_far(u16 s1seg, மாறிலி வெறுமை *s1, u16 s2seg, மாறிலி வெறுமை *s2, size_t n);
முழு memcmp(மாறிலி வெறுமை *s1, மாறிலி வெறுமை *s2, size_t n);
முழு strcmp(மாறிலி எழுத்து *s1, மாறிலி எழுத்து *s2);
வெறுமை memset_far(u16 d_seg, வெறுமை *d_far, u8 c, size_t len);
வெறுமை memset16_far(u16 d_seg, வெறுமை *d_far, u16 c, size_t len);
வெறுமை *memset(வெறுமை *s, முழு c, size_t n);
வெறுமை memset_fl(வெறுமை *ptr, u8 val, size_t size);
வெறுமை memcpy_far(u16 d_seg, வெறுமை *d_far
                , u16 s_seg, மாறிலி வெறுமை *s_far, size_t len);
வெறுமை memcpy_fl(வெறுமை *d_fl, மாறிலி வெறுமை *s_fl, size_t len);
வெறுமை *memcpy(வெறுமை *d1, மாறிலி வெறுமை *s1, size_t len);
#எனில் MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
வெறுமை iomemcpy(வெறுமை *d, மாறிலி வெறுமை *s, u32 len);
வெறுமை *memmove(வெறுமை *d, மாறிலி வெறுமை *s, size_t len);
எழுத்து *strtcpy(எழுத்து *dest, மாறிலி எழுத்து *src, size_t len);
எழுத்து *strchr(மாறிலி எழுத்து *s, முழு c);
எழுத்து *nullTrailingSpace(எழுத்து *buf);

#endif // string.h
