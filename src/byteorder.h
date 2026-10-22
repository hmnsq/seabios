#ifndef __BYTEORDER_H
#define __BYTEORDER_H

#അന്തര്ഭാവ "types.h" // u32

സ്ഥിതം വരിയിൽ u16 __swab16_constant(u16 val) {
    തിരികെ (val<<8) | (val>>8);
}
സ്ഥിതം വരിയിൽ u32 __swab32_constant(u32 val) {
    തിരികെ (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
സ്ഥിതം വരിയിൽ u64 __swab64_constant(u64 val) {
    തിരികെ ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
സ്ഥിതം വരിയിൽ u32 __swab32(u32 val) {
    asm("bswapl %0" : "+r"(val));
    തിരികെ val;
}
സ്ഥിതം വരിയിൽ u64 __swab64(u64 val) {
    സംഘം u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    തിരികെ o.val;
}

#define swab16(x) __swab16_constant(x)
#define swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#define swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

സ്ഥിതം വരിയിൽ u16 cpu_to_le16(u16 x) {
    തിരികെ x;
}
സ്ഥിതം വരിയിൽ u32 cpu_to_le32(u32 x) {
    തിരികെ x;
}
സ്ഥിതം വരിയിൽ u64 cpu_to_le64(u64 x) {
    തിരികെ x;
}
സ്ഥിതം വരിയിൽ u16 le16_to_cpu(u16 x) {
    തിരികെ x;
}
സ്ഥിതം വരിയിൽ u32 le32_to_cpu(u32 x) {
    തിരികെ x;
}
സ്ഥിതം വരിയിൽ u64 le64_to_cpu(u64 x) {
    തിരികെ x;
}

സ്ഥിതം വരിയിൽ u16 cpu_to_be16(u16 x) {
    തിരികെ swab16(x);
}
സ്ഥിതം വരിയിൽ u32 cpu_to_be32(u32 x) {
    തിരികെ swab32(x);
}
സ്ഥിതം വരിയിൽ u64 cpu_to_be64(u64 x) {
    തിരികെ swab64(x);
}
സ്ഥിതം വരിയിൽ u16 be16_to_cpu(u16 x) {
    തിരികെ swab16(x);
}
സ്ഥിതം വരിയിൽ u32 be32_to_cpu(u32 x) {
    തിരികെ swab32(x);
}
സ്ഥിതം വരിയിൽ u64 be64_to_cpu(u64 x) {
    തിരികെ swab64(x);
}

#endif // byteorder.h
