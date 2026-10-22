// String manipulation function defs.
#جيڪڏهن_وصف_نه_ٿيل __STRING_H
#وصف_ڪريو __STRING_H

#ضمن "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, فراغ *buf_far, u32 len);
u8 checksum(فراغ *buf, u32 len);
سائيز_t لڙي_ڊيگهه(ثابت حرف *s);
صحيح memcmp_far(u16 s1seg, ثابت فراغ *s1, u16 s2seg, ثابت فراغ *s2, سائيز_t n);
صحيح memcmp(ثابت فراغ *s1, ثابت فراغ *s2, سائيز_t n);
صحيح strcmp(ثابت حرف *s1, ثابت حرف *s2);
فراغ memset_far(u16 d_seg, فراغ *d_far, u8 c, سائيز_t len);
فراغ memset16_far(u16 d_seg, فراغ *d_far, u16 c, سائيز_t len);
فراغ *memset(فراغ *s, صحيح c, سائيز_t n);
فراغ memset_fl(فراغ *ptr, u8 val, سائيز_t سائيز);
فراغ memcpy_far(u16 d_seg, فراغ *d_far
                , u16 s_seg, ثابت فراغ *s_far, سائيز_t len);
فراغ memcpy_fl(فراغ *d_fl, ثابت فراغ *s_fl, سائيز_t len);
فراغ *يادگيري_نقل(فراغ *d1, ثابت فراغ *s1, سائيز_t len);
#إذا MODESEGMENT == 0
#وصف_ڪريو يادگيري_نقل __builtin_memcpy
#جيڪڏهن_پورو
فراغ iomemcpy(فراغ *d, ثابت فراغ *s, u32 len);
فراغ *memmove(فراغ *d, ثابت فراغ *s, سائيز_t len);
حرف *strtcpy(حرف *dest, ثابت حرف *src, سائيز_t len);
حرف *strchr(ثابت حرف *s, صحيح c);
حرف *nullTrailingSpace(حرف *buf);

#جيڪڏهن_پورو // string.h
