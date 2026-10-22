// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#અન્તર્ભાવ "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, શૂન્ય *buf_far, u32 len);
u8 checksum(શૂન્ય *buf, u32 len);
size_t strlen(અચલ અક્ષર *s);
પૂર્ણાંક memcmp_far(u16 s1seg, અચલ શૂન્ય *s1, u16 s2seg, અચલ શૂન્ય *s2, size_t n);
પૂર્ણાંક memcmp(અચલ શૂન્ય *s1, અચલ શૂન્ય *s2, size_t n);
પૂર્ણાંક strcmp(અચલ અક્ષર *s1, અચલ અક્ષર *s2);
શૂન્ય memset_far(u16 d_seg, શૂન્ય *d_far, u8 c, size_t len);
શૂન્ય memset16_far(u16 d_seg, શૂન્ય *d_far, u16 c, size_t len);
શૂન્ય *memset(શૂન્ય *s, પૂર્ણાંક c, size_t n);
શૂન્ય memset_fl(શૂન્ય *ptr, u8 val, size_t size);
શૂન્ય memcpy_far(u16 d_seg, શૂન્ય *d_far
                , u16 s_seg, અચલ શૂન્ય *s_far, size_t len);
શૂન્ય memcpy_fl(શૂન્ય *d_fl, અચલ શૂન્ય *s_fl, size_t len);
શૂન્ય *memcpy(શૂન્ય *d1, અચલ શૂન્ય *s1, size_t len);
#જો MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
શૂન્ય iomemcpy(શૂન્ય *d, અચલ શૂન્ય *s, u32 len);
શૂન્ય *memmove(શૂન્ય *d, અચલ શૂન્ય *s, size_t len);
અક્ષર *strtcpy(અક્ષર *dest, અચલ અક્ષર *src, size_t len);
અક્ષર *strchr(અચલ અક્ષર *s, પૂર્ણાંક c);
અક્ષર *nullTrailingSpace(અક્ષર *buf);

#endif // string.h
