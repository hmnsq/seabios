#که_نه_تعریف_شوی __BYTEORDER_H
#تعریف_کړه __BYTEORDER_H

#ضمن "types.h" // u32

ساكن مضمن u16 __swab16_constant(u16 val) {
    أرجع (val<<8) | (val>>8);
}
ساكن مضمن u32 __swab32_constant(u32 val) {
    أرجع (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
ساكن مضمن u64 __swab64_constant(u64 val) {
    أرجع ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
ساكن مضمن u32 __swab32(u32 val) {
    asm("bswapl %0" : "+r"(val));
    أرجع val;
}
ساكن مضمن u64 __swab64(u64 val) {
    اتحاد u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    أرجع o.val;
}

#تعریف_کړه swab16(x) __swab16_constant(x)
#تعریف_کړه swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#تعریف_کړه swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

ساكن مضمن u16 cpu_منزل_le16(u16 x) {
    أرجع x;
}
ساكن مضمن u32 cpu_منزل_le32(u32 x) {
    أرجع x;
}
ساكن مضمن u64 cpu_منزل_le64(u64 x) {
    أرجع x;
}
ساكن مضمن u16 le16_منزل_cpu(u16 x) {
    أرجع x;
}
ساكن مضمن u32 le32_منزل_cpu(u32 x) {
    أرجع x;
}
ساكن مضمن u64 le64_منزل_cpu(u64 x) {
    أرجع x;
}

ساكن مضمن u16 cpu_منزل_be16(u16 x) {
    أرجع swab16(x);
}
ساكن مضمن u32 cpu_منزل_be32(u32 x) {
    أرجع swab32(x);
}
ساكن مضمن u64 cpu_منزل_be64(u64 x) {
    أرجع swab64(x);
}
ساكن مضمن u16 be16_منزل_cpu(u16 x) {
    أرجع swab16(x);
}
ساكن مضمن u32 be32_منزل_cpu(u32 x) {
    أرجع swab32(x);
}
ساكن مضمن u64 be64_منزل_cpu(u64 x) {
    أرجع swab64(x);
}

#د_که_پای // byteorder.h
