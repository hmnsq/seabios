// String manipulation function defs.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __STRING_H
#ವ್ಯಾಖ್ಯಾನಿಸು __STRING_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, ಶೂನ್ಯ *buf_far, u32 len);
u8 checksum(ಶೂನ್ಯ *buf, u32 len);
ಗಾತ್ರ_t ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);
ಪೂರ್ಣಾಂಕ memcmp_far(u16 s1seg, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s1, u16 s2seg, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s2, ಗಾತ್ರ_t n);
ಪೂರ್ಣಾಂಕ memcmp(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s2, ಗಾತ್ರ_t n);
ಪೂರ್ಣಾಂಕ strcmp(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s2);
ಶೂನ್ಯ memset_far(u16 d_seg, ಶೂನ್ಯ *d_far, u8 c, ಗಾತ್ರ_t len);
ಶೂನ್ಯ memset16_far(u16 d_seg, ಶೂನ್ಯ *d_far, u16 c, ಗಾತ್ರ_t len);
ಶೂನ್ಯ *memset(ಶೂನ್ಯ *s, ಪೂರ್ಣಾಂಕ c, ಗಾತ್ರ_t n);
ಶೂನ್ಯ memset_fl(ಶೂನ್ಯ *ptr, u8 val, ಗಾತ್ರ_t ಗಾತ್ರ_2);
ಶೂನ್ಯ memcpy_far(u16 d_seg, ಶೂನ್ಯ *d_far
                , u16 s_seg, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s_far, ಗಾತ್ರ_t len);
ಶೂನ್ಯ memcpy_fl(ಶೂನ್ಯ *d_fl, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s_fl, ಗಾತ್ರ_t len);
ಶೂನ್ಯ *ಸ್ಮರಣೆ_ನಕಲು(ಶೂನ್ಯ *d1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s1, ಗಾತ್ರ_t len);
#ಆದರೆ MODESEGMENT == 0
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಮರಣೆ_ನಕಲು __builtin_memcpy
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ iomemcpy(ಶೂನ್ಯ *d, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s, u32 len);
ಶೂನ್ಯ *memmove(ಶೂನ್ಯ *d, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s, ಗಾತ್ರ_t len);
ಅಕ್ಷರ *strtcpy(ಅಕ್ಷರ *dest, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, ಗಾತ್ರ_t len);
ಅಕ್ಷರ *strchr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ c);
ಅಕ್ಷರ *nullTrailingSpace(ಅಕ್ಷರ *buf);

#ಆದರೆ_ಅಂತ್ಯ // string.h
