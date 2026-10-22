#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __BYTEORDER_H
#ವ್ಯಾಖ್ಯಾನಿಸು __BYTEORDER_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 __swab16_constant(u16 val) {
    ಹಿಂತಿರುಗಿ (val<<8) | (val>>8);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 __swab32_constant(u32 val) {
    ಹಿಂತಿರುಗಿ (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 __swab64_constant(u64 val) {
    ಹಿಂತಿರುಗಿ ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 __swab32(u32 val) {
    asm("bswapl %0" : "+r"(val));
    ಹಿಂತಿರುಗಿ val;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 __swab64(u64 val) {
    ಒಕ್ಕೂಟ u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    ಹಿಂತಿರುಗಿ o.val;
}

#ವ್ಯಾಖ್ಯಾನಿಸು swab16(x) __swab16_constant(x)
#ವ್ಯಾಖ್ಯಾನಿಸು swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#ವ್ಯಾಖ್ಯಾನಿಸು swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 cpu_ಗಮ್ಯ_le16(u16 x) {
    ಹಿಂತಿರುಗಿ x;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 cpu_ಗಮ್ಯ_le32(u32 x) {
    ಹಿಂತಿರುಗಿ x;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 cpu_ಗಮ್ಯ_le64(u64 x) {
    ಹಿಂತಿರುಗಿ x;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 le16_ಗಮ್ಯ_cpu(u16 x) {
    ಹಿಂತಿರುಗಿ x;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 le32_ಗಮ್ಯ_cpu(u32 x) {
    ಹಿಂತಿರುಗಿ x;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 le64_ಗಮ್ಯ_cpu(u64 x) {
    ಹಿಂತಿರುಗಿ x;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 cpu_ಗಮ್ಯ_be16(u16 x) {
    ಹಿಂತಿರುಗಿ swab16(x);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 cpu_ಗಮ್ಯ_be32(u32 x) {
    ಹಿಂತಿರುಗಿ swab32(x);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 cpu_ಗಮ್ಯ_be64(u64 x) {
    ಹಿಂತಿರುಗಿ swab64(x);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 be16_ಗಮ್ಯ_cpu(u16 x) {
    ಹಿಂತಿರುಗಿ swab16(x);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 be32_ಗಮ್ಯ_cpu(u32 x) {
    ಹಿಂತಿರುಗಿ swab32(x);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 be64_ಗಮ್ಯ_cpu(u64 x) {
    ಹಿಂತಿರುಗಿ swab64(x);
}

#ಆದರೆ_ಅಂತ್ಯ // byteorder.h
