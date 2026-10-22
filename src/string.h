// String manipulation function defs.
#جے_بنا_تعریف __STRING_H
#تعریف_کرو __STRING_H

#ضمن "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, فراغ *buf_far, u32 len);
u8 checksum(فراغ *buf, u32 len);
ناپ_t لڑی_لمبائی(ثابت حرف *s);
صحيح memcmp_far(u16 s1seg, ثابت فراغ *s1, u16 s2seg, ثابت فراغ *s2, ناپ_t n);
صحيح memcmp(ثابت فراغ *s1, ثابت فراغ *s2, ناپ_t n);
صحيح strcmp(ثابت حرف *s1, ثابت حرف *s2);
فراغ memset_far(u16 d_seg, فراغ *d_far, u8 c, ناپ_t len);
فراغ memset16_far(u16 d_seg, فراغ *d_far, u16 c, ناپ_t len);
فراغ *memset(فراغ *s, صحيح c, ناپ_t n);
فراغ memset_fl(فراغ *ptr, u8 val, ناپ_t ناپ);
فراغ memcpy_far(u16 d_seg, فراغ *d_far
                , u16 s_seg, ثابت فراغ *s_far, ناپ_t len);
فراغ memcpy_fl(فراغ *d_fl, ثابت فراغ *s_fl, ناپ_t len);
فراغ *حافظہ_نقل(فراغ *d1, ثابت فراغ *s1, ناپ_t len);
#إذا MODESEGMENT == 0
#تعریف_کرو حافظہ_نقل __builtin_memcpy
#جے_انت
فراغ iomemcpy(فراغ *d, ثابت فراغ *s, u32 len);
فراغ *memmove(فراغ *d, ثابت فراغ *s, ناپ_t len);
حرف *strtcpy(حرف *dest, ثابت حرف *src, ناپ_t len);
حرف *strchr(ثابت حرف *s, صحيح c);
حرف *nullTrailingSpace(حرف *buf);

#جے_انت // string.h
