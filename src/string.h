// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#অন্তর্ভাব "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, শূন্য *buf_far, u32 len);
u8 checksum(শূন্য *buf, u32 len);
size_t strlen(ধ্রুব বর্ণ *s);
পূর্ণাঙ্ক memcmp_far(u16 s1seg, ধ্রুব শূন্য *s1, u16 s2seg, ধ্রুব শূন্য *s2, size_t n);
পূর্ণাঙ্ক memcmp(ধ্রুব শূন্য *s1, ধ্রুব শূন্য *s2, size_t n);
পূর্ণাঙ্ক strcmp(ধ্রুব বর্ণ *s1, ধ্রুব বর্ণ *s2);
শূন্য memset_far(u16 d_seg, শূন্য *d_far, u8 c, size_t len);
শূন্য memset16_far(u16 d_seg, শূন্য *d_far, u16 c, size_t len);
শূন্য *memset(শূন্য *s, পূর্ণাঙ্ক c, size_t n);
শূন্য memset_fl(শূন্য *ptr, u8 val, size_t size);
শূন্য memcpy_far(u16 d_seg, শূন্য *d_far
                , u16 s_seg, ধ্রুব শূন্য *s_far, size_t len);
শূন্য memcpy_fl(শূন্য *d_fl, ধ্রুব শূন্য *s_fl, size_t len);
শূন্য *memcpy(শূন্য *d1, ধ্রুব শূন্য *s1, size_t len);
#যদি MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
শূন্য iomemcpy(শূন্য *d, ধ্রুব শূন্য *s, u32 len);
শূন্য *memmove(শূন্য *d, ধ্রুব শূন্য *s, size_t len);
বর্ণ *strtcpy(বর্ণ *dest, ধ্রুব বর্ণ *src, size_t len);
বর্ণ *strchr(ধ্রুব বর্ণ *s, পূর্ণাঙ্ক c);
বর্ণ *nullTrailingSpace(বর্ণ *buf);

#endif // string.h
