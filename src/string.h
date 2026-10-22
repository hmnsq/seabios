// String manipulation function defs.
#اگر_غیر_تعریف_شدہ __STRING_H
#تعریف_کرو __STRING_H

#شامل "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, خالی *buf_far, u32 len);
u8 checksum(خالی *buf, u32 len);
سائز_t لڑی_لمبائی(ثابت حرف *s);
صحیح memcmp_far(u16 s1seg, ثابت خالی *s1, u16 s2seg, ثابت خالی *s2, سائز_t n);
صحیح memcmp(ثابت خالی *s1, ثابت خالی *s2, سائز_t n);
صحیح strcmp(ثابت حرف *s1, ثابت حرف *s2);
خالی memset_far(u16 d_seg, خالی *d_far, u8 c, سائز_t len);
خالی memset16_far(u16 d_seg, خالی *d_far, u16 c, سائز_t len);
خالی *memset(خالی *s, صحیح c, سائز_t n);
خالی memset_fl(خالی *ptr, u8 val, سائز_t سائز);
خالی memcpy_far(u16 d_seg, خالی *d_far
                , u16 s_seg, ثابت خالی *s_far, سائز_t len);
خالی memcpy_fl(خالی *d_fl, ثابت خالی *s_fl, سائز_t len);
خالی *حافظہ_نقل(خالی *d1, ثابت خالی *s1, سائز_t len);
#اگر MODESEGMENT == 0
#تعریف_کرو حافظہ_نقل __builtin_memcpy
#اگر_ختم
خالی iomemcpy(خالی *d, ثابت خالی *s, u32 len);
خالی *memmove(خالی *d, ثابت خالی *s, سائز_t len);
حرف *strtcpy(حرف *dest, ثابت حرف *src, سائز_t len);
حرف *strchr(ثابت حرف *s, صحیح c);
حرف *nullTrailingSpace(حرف *buf);

#اگر_ختم // string.h
