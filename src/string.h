// String manipulation function defs.
#यदि_अपरिभाषितम् __STRING_H
#परिभाषय __STRING_H

#अन्तर्भाव "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, शून्य *buf_far, u32 len);
u8 checksum(शून्य *buf, u32 len);
आकार_t वाक्यदैर्घ्यम्(ध्रुव वर्ण *s);
पूर्णाङ्क memcmp_far(u16 s1seg, ध्रुव शून्य *s1, u16 s2seg, ध्रुव शून्य *s2, आकार_t n);
पूर्णाङ्क memcmp(ध्रुव शून्य *s1, ध्रुव शून्य *s2, आकार_t n);
पूर्णाङ्क वाक्यं_तुलय(ध्रुव वर्ण *s1, ध्रुव वर्ण *s2);
शून्य memset_far(u16 d_seg, शून्य *d_far, u8 c, आकार_t len);
शून्य memset16_far(u16 d_seg, शून्य *d_far, u16 c, आकार_t len);
शून्य *स्मृतिं_पूरय(शून्य *s, पूर्णाङ्क c, आकार_t n);
शून्य memset_fl(शून्य *ptr, u8 val, आकार_t आकार);
शून्य memcpy_far(u16 d_seg, शून्य *d_far
                , u16 s_seg, ध्रुव शून्य *s_far, आकार_t len);
शून्य memcpy_fl(शून्य *d_fl, ध्रुव शून्य *s_fl, आकार_t len);
शून्य *स्मृतिं_प्रतिलिख(शून्य *d1, ध्रुव शून्य *s1, आकार_t len);
#यदि MODESEGMENT == 0
#परिभाषय स्मृतिं_प्रतिलिख __builtin_memcpy
#यदि_अन्तः
शून्य iomemcpy(शून्य *d, ध्रुव शून्य *s, u32 len);
शून्य *memmove(शून्य *d, ध्रुव शून्य *s, आकार_t len);
वर्ण *strtcpy(वर्ण *dest, ध्रुव वर्ण *src, आकार_t len);
वर्ण *strchr(ध्रुव वर्ण *s, पूर्णाङ्क c);
वर्ण *nullTrailingSpace(वर्ण *buf);

#यदि_अन्तः // string.h
