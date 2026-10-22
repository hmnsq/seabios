// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#अन्तर्भाव "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, शून्य *buf_far, u32 len);
u8 checksum(शून्य *buf, u32 len);
size_t strlen(ध्रुव वर्ण *s);
पूर्णाङ्क memcmp_far(u16 s1seg, ध्रुव शून्य *s1, u16 s2seg, ध्रुव शून्य *s2, size_t n);
पूर्णाङ्क memcmp(ध्रुव शून्य *s1, ध्रुव शून्य *s2, size_t n);
पूर्णाङ्क strcmp(ध्रुव वर्ण *s1, ध्रुव वर्ण *s2);
शून्य memset_far(u16 d_seg, शून्य *d_far, u8 c, size_t len);
शून्य memset16_far(u16 d_seg, शून्य *d_far, u16 c, size_t len);
शून्य *memset(शून्य *s, पूर्णाङ्क c, size_t n);
शून्य memset_fl(शून्य *ptr, u8 val, size_t size);
शून्य memcpy_far(u16 d_seg, शून्य *d_far
                , u16 s_seg, ध्रुव शून्य *s_far, size_t len);
शून्य memcpy_fl(शून्य *d_fl, ध्रुव शून्य *s_fl, size_t len);
शून्य *memcpy(शून्य *d1, ध्रुव शून्य *s1, size_t len);
#सचे MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
शून्य iomemcpy(शून्य *d, ध्रुव शून्य *s, u32 len);
शून्य *memmove(शून्य *d, ध्रुव शून्य *s, size_t len);
वर्ण *strtcpy(वर्ण *dest, ध्रुव वर्ण *src, size_t len);
वर्ण *strchr(ध्रुव वर्ण *s, पूर्णाङ्क c);
वर्ण *nullTrailingSpace(वर्ण *buf);

#endif // string.h
