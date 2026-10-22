// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, ಶೂನ್ಯ *buf_far, u32 len);
u8 checksum(ಶೂನ್ಯ *buf, u32 len);
size_t strlen(ಧ್ರುವ ವರ್ಣ *s);
ಪೂರ್ಣಾಙ್ಕ memcmp_far(u16 s1seg, ಧ್ರುವ ಶೂನ್ಯ *s1, u16 s2seg, ಧ್ರುವ ಶೂನ್ಯ *s2, size_t n);
ಪೂರ್ಣಾಙ್ಕ memcmp(ಧ್ರುವ ಶೂನ್ಯ *s1, ಧ್ರುವ ಶೂನ್ಯ *s2, size_t n);
ಪೂರ್ಣಾಙ್ಕ strcmp(ಧ್ರುವ ವರ್ಣ *s1, ಧ್ರುವ ವರ್ಣ *s2);
ಶೂನ್ಯ memset_far(u16 d_seg, ಶೂನ್ಯ *d_far, u8 c, size_t len);
ಶೂನ್ಯ memset16_far(u16 d_seg, ಶೂನ್ಯ *d_far, u16 c, size_t len);
ಶೂನ್ಯ *memset(ಶೂನ್ಯ *s, ಪೂರ್ಣಾಙ್ಕ c, size_t n);
ಶೂನ್ಯ memset_fl(ಶೂನ್ಯ *ptr, u8 val, size_t size);
ಶೂನ್ಯ memcpy_far(u16 d_seg, ಶೂನ್ಯ *d_far
                , u16 s_seg, ಧ್ರುವ ಶೂನ್ಯ *s_far, size_t len);
ಶೂನ್ಯ memcpy_fl(ಶೂನ್ಯ *d_fl, ಧ್ರುವ ಶೂನ್ಯ *s_fl, size_t len);
ಶೂನ್ಯ *memcpy(ಶೂನ್ಯ *d1, ಧ್ರುವ ಶೂನ್ಯ *s1, size_t len);
#ಯದಿ MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
ಶೂನ್ಯ iomemcpy(ಶೂನ್ಯ *d, ಧ್ರುವ ಶೂನ್ಯ *s, u32 len);
ಶೂನ್ಯ *memmove(ಶೂನ್ಯ *d, ಧ್ರುವ ಶೂನ್ಯ *s, size_t len);
ವರ್ಣ *strtcpy(ವರ್ಣ *dest, ಧ್ರುವ ವರ್ಣ *src, size_t len);
ವರ್ಣ *strchr(ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ c);
ವರ್ಣ *nullTrailingSpace(ವರ್ಣ *buf);

#endif // string.h
