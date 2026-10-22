#ifndef __BYTEORDER_H
#define __BYTEORDER_H

#अन्तर्भाव "types.h" // u32

स्थिर ओळीत u16 __swab16_constant(u16 val) {
    परत (val<<8) | (val>>8);
}
स्थिर ओळीत u32 __swab32_constant(u32 val) {
    परत (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
स्थिर ओळीत u64 __swab64_constant(u64 val) {
    परत ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
स्थिर ओळीत u32 __swab32(u32 val) {
    यंत्र("bswapl %0" : "+r"(val));
    परत val;
}
स्थिर ओळीत u64 __swab64(u64 val) {
    संघ u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    परत o.val;
}

#define swab16(x) __swab16_constant(x)
#define swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#define swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

स्थिर ओळीत u16 cpu_to_le16(u16 x) {
    परत x;
}
स्थिर ओळीत u32 cpu_to_le32(u32 x) {
    परत x;
}
स्थिर ओळीत u64 cpu_to_le64(u64 x) {
    परत x;
}
स्थिर ओळीत u16 le16_to_cpu(u16 x) {
    परत x;
}
स्थिर ओळीत u32 le32_to_cpu(u32 x) {
    परत x;
}
स्थिर ओळीत u64 le64_to_cpu(u64 x) {
    परत x;
}

स्थिर ओळीत u16 cpu_to_be16(u16 x) {
    परत swab16(x);
}
स्थिर ओळीत u32 cpu_to_be32(u32 x) {
    परत swab32(x);
}
स्थिर ओळीत u64 cpu_to_be64(u64 x) {
    परत swab64(x);
}
स्थिर ओळीत u16 be16_to_cpu(u16 x) {
    परत swab16(x);
}
स्थिर ओळीत u32 be32_to_cpu(u32 x) {
    परत swab32(x);
}
स्थिर ओळीत u64 be64_to_cpu(u64 x) {
    परत swab64(x);
}

#endif // byteorder.h
