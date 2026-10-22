// String manipulation function defs.
#اگر_تعریف_نشده __STRING_H
#تعریف_کن __STRING_H

#ضمن "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, فراغ *buf_far, u32 len);
u8 checksum(فراغ *buf, u32 len);
اندازه_t طول_رشته(ثابت حرف *s);
صحيح memcmp_far(u16 s1seg, ثابت فراغ *s1, u16 s2seg, ثابت فراغ *s2, اندازه_t n);
صحيح memcmp(ثابت فراغ *s1, ثابت فراغ *s2, اندازه_t n);
صحيح strcmp(ثابت حرف *s1, ثابت حرف *s2);
فراغ memset_far(u16 d_seg, فراغ *d_far, u8 c, اندازه_t len);
فراغ memset16_far(u16 d_seg, فراغ *d_far, u16 c, اندازه_t len);
فراغ *memset(فراغ *s, صحيح c, اندازه_t n);
فراغ memset_fl(فراغ *ptr, u8 val, اندازه_t اندازه);
فراغ memcpy_far(u16 d_seg, فراغ *d_far
                , u16 s_seg, ثابت فراغ *s_far, اندازه_t len);
فراغ memcpy_fl(فراغ *d_fl, ثابت فراغ *s_fl, اندازه_t len);
فراغ *کپی_حافظه(فراغ *d1, ثابت فراغ *s1, اندازه_t len);
#إذا MODESEGMENT == 0
#تعریف_کن کپی_حافظه __builtin_memcpy
#پایان_اگر
فراغ iomemcpy(فراغ *d, ثابت فراغ *s, u32 len);
فراغ *memmove(فراغ *d, ثابت فراغ *s, اندازه_t len);
حرف *strtcpy(حرف *dest, ثابت حرف *src, اندازه_t len);
حرف *strchr(ثابت حرف *s, صحيح c);
حرف *nullTrailingSpace(حرف *buf);

#پایان_اگر // string.h
