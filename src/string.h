// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, ଶୂନ୍ୟ *buf_far, u32 len);
u8 checksum(ଶୂନ୍ୟ *buf, u32 len);
size_t strlen(ଅଚଳ ଅକ୍ଷର *s);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା memcmp_far(u16 s1seg, ଅଚଳ ଶୂନ୍ୟ *s1, u16 s2seg, ଅଚଳ ଶୂନ୍ୟ *s2, size_t n);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା memcmp(ଅଚଳ ଶୂନ୍ୟ *s1, ଅଚଳ ଶୂନ୍ୟ *s2, size_t n);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା strcmp(ଅଚଳ ଅକ୍ଷର *s1, ଅଚଳ ଅକ୍ଷର *s2);
ଶୂନ୍ୟ memset_far(u16 d_seg, ଶୂନ୍ୟ *d_far, u8 c, size_t len);
ଶୂନ୍ୟ memset16_far(u16 d_seg, ଶୂନ୍ୟ *d_far, u16 c, size_t len);
ଶୂନ୍ୟ *memset(ଶୂନ୍ୟ *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, size_t n);
ଶୂନ୍ୟ memset_fl(ଶୂନ୍ୟ *ptr, u8 val, size_t size);
ଶୂନ୍ୟ memcpy_far(u16 d_seg, ଶୂନ୍ୟ *d_far
                , u16 s_seg, ଅଚଳ ଶୂନ୍ୟ *s_far, size_t len);
ଶୂନ୍ୟ memcpy_fl(ଶୂନ୍ୟ *d_fl, ଅଚଳ ଶୂନ୍ୟ *s_fl, size_t len);
ଶୂନ୍ୟ *memcpy(ଶୂନ୍ୟ *d1, ଅଚଳ ଶୂନ୍ୟ *s1, size_t len);
#ଯଦି MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
ଶୂନ୍ୟ iomemcpy(ଶୂନ୍ୟ *d, ଅଚଳ ଶୂନ୍ୟ *s, u32 len);
ଶୂନ୍ୟ *memmove(ଶୂନ୍ୟ *d, ଅଚଳ ଶୂନ୍ୟ *s, size_t len);
ଅକ୍ଷର *strtcpy(ଅକ୍ଷର *dest, ଅଚଳ ଅକ୍ଷର *src, size_t len);
ଅକ୍ଷର *strchr(ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c);
ଅକ୍ଷର *nullTrailingSpace(ଅକ୍ଷର *buf);

#endif // string.h
