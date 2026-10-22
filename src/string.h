// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#অন্তর্ভাব "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, শূন্য *buf_far, u32 len);
u8 checksum(শূন্য *buf, u32 len);
আকার_t strlen(ধ্রুবক অক্ষর *s);
পূর্ণসংখ্যা memcmp_far(u16 s1seg, ধ্রুবক শূন্য *s1, u16 s2seg, ধ্রুবক শূন্য *s2, আকার_t n);
পূর্ণসংখ্যা memcmp(ধ্রুবক শূন্য *s1, ধ্রুবক শূন্য *s2, আকার_t n);
পূর্ণসংখ্যা strcmp(ধ্রুবক অক্ষর *s1, ধ্রুবক অক্ষর *s2);
শূন্য memset_far(u16 d_seg, শূন্য *d_far, u8 c, আকার_t len);
শূন্য memset16_far(u16 d_seg, শূন্য *d_far, u16 c, আকার_t len);
শূন্য *memset(শূন্য *s, পূর্ণসংখ্যা c, আকার_t n);
শূন্য memset_fl(শূন্য *ptr, u8 val, আকার_t আকার_2);
শূন্য memcpy_far(u16 d_seg, শূন্য *d_far
                , u16 s_seg, ধ্রুবক শূন্য *s_far, আকার_t len);
শূন্য memcpy_fl(শূন্য *d_fl, ধ্রুবক শূন্য *s_fl, আকার_t len);
শূন্য *memcpy(শূন্য *d1, ধ্রুবক শূন্য *s1, আকার_t len);
#যদি MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
শূন্য iomemcpy(শূন্য *d, ধ্রুবক শূন্য *s, u32 len);
শূন্য *memmove(শূন্য *d, ধ্রুবক শূন্য *s, আকার_t len);
অক্ষর *strtcpy(অক্ষর *dest, ধ্রুবক অক্ষর *src, আকার_t len);
অক্ষর *strchr(ধ্রুবক অক্ষর *s, পূর্ণসংখ্যা c);
অক্ষর *nullTrailingSpace(অক্ষর *buf);

#endif // string.h
