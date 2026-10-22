#اگر_غیر_تعریف_شدہ __BYTEORDER_H
#تعریف_کرو __BYTEORDER_H

#شامل "types.h" // u32

ساکن اندرونی u16 __swab16_constant(u16 val) {
    واپس (val<<8) | (val>>8);
}
ساکن اندرونی u32 __swab32_constant(u32 val) {
    واپس (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
ساکن اندرونی u64 __swab64_constant(u64 val) {
    واپس ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
ساکن اندرونی u32 __swab32(u32 val) {
    asm("bswapl %0" : "+r"(val));
    واپس val;
}
ساکن اندرونی u64 __swab64(u64 val) {
    اتحاد u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    واپس o.val;
}

#تعریف_کرو swab16(x) __swab16_constant(x)
#تعریف_کرو swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#تعریف_کرو swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

ساکن اندرونی u16 cpu_مقصد_le16(u16 x) {
    واپس x;
}
ساکن اندرونی u32 cpu_مقصد_le32(u32 x) {
    واپس x;
}
ساکن اندرونی u64 cpu_مقصد_le64(u64 x) {
    واپس x;
}
ساکن اندرونی u16 le16_مقصد_cpu(u16 x) {
    واپس x;
}
ساکن اندرونی u32 le32_مقصد_cpu(u32 x) {
    واپس x;
}
ساکن اندرونی u64 le64_مقصد_cpu(u64 x) {
    واپس x;
}

ساکن اندرونی u16 cpu_مقصد_be16(u16 x) {
    واپس swab16(x);
}
ساکن اندرونی u32 cpu_مقصد_be32(u32 x) {
    واپس swab32(x);
}
ساکن اندرونی u64 cpu_مقصد_be64(u64 x) {
    واپس swab64(x);
}
ساکن اندرونی u16 be16_مقصد_cpu(u16 x) {
    واپس swab16(x);
}
ساکن اندرونی u32 be32_مقصد_cpu(u32 x) {
    واپس swab32(x);
}
ساکن اندرونی u64 be64_مقصد_cpu(u64 x) {
    واپس swab64(x);
}

#اگر_ختم // byteorder.h
