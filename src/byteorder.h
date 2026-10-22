#ifndef __BYTEORDER_H
#define __BYTEORDER_H

#अन्तर्भाव "types.h" // u32

स्थिर अन्तःस्थ u16 __swab16_constant(u16 val) {
    फर्काऊ (val<<8) | (val>>8);
}
स्थिर अन्तःस्थ u32 __swab32_constant(u32 val) {
    फर्काऊ (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
स्थिर अन्तःस्थ u64 __swab64_constant(u64 val) {
    फर्काऊ ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
स्थिर अन्तःस्थ u32 __swab32(u32 val) {
    यंत्र("bswapl %0" : "+r"(val));
    फर्काऊ val;
}
स्थिर अन्तःस्थ u64 __swab64(u64 val) {
    सङ्घ u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    फर्काऊ o.val;
}

#define swab16(x) __swab16_constant(x)
#define swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#define swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

स्थिर अन्तःस्थ u16 cpu_to_le16(u16 x) {
    फर्काऊ x;
}
स्थिर अन्तःस्थ u32 cpu_to_le32(u32 x) {
    फर्काऊ x;
}
स्थिर अन्तःस्थ u64 cpu_to_le64(u64 x) {
    फर्काऊ x;
}
स्थिर अन्तःस्थ u16 le16_to_cpu(u16 x) {
    फर्काऊ x;
}
स्थिर अन्तःस्थ u32 le32_to_cpu(u32 x) {
    फर्काऊ x;
}
स्थिर अन्तःस्थ u64 le64_to_cpu(u64 x) {
    फर्काऊ x;
}

स्थिर अन्तःस्थ u16 cpu_to_be16(u16 x) {
    फर्काऊ swab16(x);
}
स्थिर अन्तःस्थ u32 cpu_to_be32(u32 x) {
    फर्काऊ swab32(x);
}
स्थिर अन्तःस्थ u64 cpu_to_be64(u64 x) {
    फर्काऊ swab64(x);
}
स्थिर अन्तःस्थ u16 be16_to_cpu(u16 x) {
    फर्काऊ swab16(x);
}
स्थिर अन्तःस्थ u32 be32_to_cpu(u32 x) {
    फर्काऊ swab32(x);
}
स्थिर अन्तःस्थ u64 be64_to_cpu(u64 x) {
    फर्काऊ swab64(x);
}

#endif // byteorder.h
