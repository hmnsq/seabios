// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#अन्तर्भाव "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, शून्य *buf_far, u32 len);
u8 checksum(शून्य *buf, u32 len);
माप_प्रकार म_माप(ध्रुव वर्ण *s);
पूर्णाङ्क memcmp_far(u16 s1seg, ध्रुव शून्य *s1, u16 s2seg, ध्रुव शून्य *s2, माप_प्रकार n);
पूर्णाङ्क स_भेद(ध्रुव शून्य *s1, ध्रुव शून्य *s2, माप_प्रकार n);
पूर्णाङ्क म_भेद(ध्रुव वर्ण *s1, ध्रुव वर्ण *s2);
शून्य memset_far(u16 d_seg, शून्य *d_far, u8 c, माप_प्रकार len);
शून्य memset16_far(u16 d_seg, शून्य *d_far, u16 c, माप_प्रकार len);
शून्य *स_रखो(शून्य *s, पूर्णाङ्क c, माप_प्रकार n);
शून्य memset_fl(शून्य *ptr, u8 val, माप_प्रकार size);
शून्य memcpy_far(u16 d_seg, शून्य *d_far
                , u16 s_seg, ध्रुव शून्य *s_far, माप_प्रकार len);
शून्य memcpy_fl(शून्य *d_fl, ध्रुव शून्य *s_fl, माप_प्रकार len);
शून्य *स_नकल(शून्य *d1, ध्रुव शून्य *s1, माप_प्रकार len);
#यदि MODESEGMENT == 0
#define स_नकल __builtin_memcpy
#endif
शून्य iomemcpy(शून्य *d, ध्रुव शून्य *s, u32 len);
शून्य *स_हटाओ(शून्य *d, ध्रुव शून्य *s, माप_प्रकार len);
वर्ण *strtcpy(वर्ण *dest, ध्रुव वर्ण *src, माप_प्रकार len);
वर्ण *म_अक्षर(ध्रुव वर्ण *s, पूर्णाङ्क c);
वर्ण *nullTrailingSpace(वर्ण *buf);

#endif // string.h
