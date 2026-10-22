// String manipulation function defs.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __STRING_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __STRING_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, ਖਾਲੀ *buf_far, u32 len);
u8 checksum(ਖਾਲੀ *buf, u32 len);
ਮਾਪ_t ਲੜੀ_ਲੰਬਾਈ(ਅਚਲ ਅੱਖਰ *s);
ਪੂਰਨ memcmp_far(u16 s1seg, ਅਚਲ ਖਾਲੀ *s1, u16 s2seg, ਅਚਲ ਖਾਲੀ *s2, ਮਾਪ_t n);
ਪੂਰਨ memcmp(ਅਚਲ ਖਾਲੀ *s1, ਅਚਲ ਖਾਲੀ *s2, ਮਾਪ_t n);
ਪੂਰਨ strcmp(ਅਚਲ ਅੱਖਰ *s1, ਅਚਲ ਅੱਖਰ *s2);
ਖਾਲੀ memset_far(u16 d_seg, ਖਾਲੀ *d_far, u8 c, ਮਾਪ_t len);
ਖਾਲੀ memset16_far(u16 d_seg, ਖਾਲੀ *d_far, u16 c, ਮਾਪ_t len);
ਖਾਲੀ *memset(ਖਾਲੀ *s, ਪੂਰਨ c, ਮਾਪ_t n);
ਖਾਲੀ memset_fl(ਖਾਲੀ *ptr, u8 val, ਮਾਪ_t ਮਾਪ);
ਖਾਲੀ memcpy_far(u16 d_seg, ਖਾਲੀ *d_far
                , u16 s_seg, ਅਚਲ ਖਾਲੀ *s_far, ਮਾਪ_t len);
ਖਾਲੀ memcpy_fl(ਖਾਲੀ *d_fl, ਅਚਲ ਖਾਲੀ *s_fl, ਮਾਪ_t len);
ਖਾਲੀ *ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਖਾਲੀ *d1, ਅਚਲ ਖਾਲੀ *s1, ਮਾਪ_t len);
#ਜੇ MODESEGMENT == 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮ੍ਰਿਤੀ_ਨਕਲ __builtin_memcpy
#ਜੇ_ਅੰਤ
ਖਾਲੀ iomemcpy(ਖਾਲੀ *d, ਅਚਲ ਖਾਲੀ *s, u32 len);
ਖਾਲੀ *memmove(ਖਾਲੀ *d, ਅਚਲ ਖਾਲੀ *s, ਮਾਪ_t len);
ਅੱਖਰ *strtcpy(ਅੱਖਰ *dest, ਅਚਲ ਅੱਖਰ *src, ਮਾਪ_t len);
ਅੱਖਰ *strchr(ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ c);
ਅੱਖਰ *nullTrailingSpace(ਅੱਖਰ *buf);

#ਜੇ_ਅੰਤ // string.h
