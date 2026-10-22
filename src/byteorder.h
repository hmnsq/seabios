#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __BYTEORDER_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __BYTEORDER_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

ਸਥਿਰ ਸਤਰਅੰਦਰ u16 __swab16_constant(u16 val) {
    ਵਾਪਸ (val<<8) | (val>>8);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 __swab32_constant(u32 val) {
    ਵਾਪਸ (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u64 __swab64_constant(u64 val) {
    ਵਾਪਸ ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 __swab32(u32 val) {
    asm("bswapl %0" : "+r"(val));
    ਵਾਪਸ val;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u64 __swab64(u64 val) {
    ਸੰਘ u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    ਵਾਪਸ o.val;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ swab16(x) __swab16_constant(x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

ਸਥਿਰ ਸਤਰਅੰਦਰ u16 cpu_ਮੰਜ਼ਿਲ_le16(u16 x) {
    ਵਾਪਸ x;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 cpu_ਮੰਜ਼ਿਲ_le32(u32 x) {
    ਵਾਪਸ x;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u64 cpu_ਮੰਜ਼ਿਲ_le64(u64 x) {
    ਵਾਪਸ x;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 le16_ਮੰਜ਼ਿਲ_cpu(u16 x) {
    ਵਾਪਸ x;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 le32_ਮੰਜ਼ਿਲ_cpu(u32 x) {
    ਵਾਪਸ x;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u64 le64_ਮੰਜ਼ਿਲ_cpu(u64 x) {
    ਵਾਪਸ x;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u16 cpu_ਮੰਜ਼ਿਲ_be16(u16 x) {
    ਵਾਪਸ swab16(x);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 cpu_ਮੰਜ਼ਿਲ_be32(u32 x) {
    ਵਾਪਸ swab32(x);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u64 cpu_ਮੰਜ਼ਿਲ_be64(u64 x) {
    ਵਾਪਸ swab64(x);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 be16_ਮੰਜ਼ਿਲ_cpu(u16 x) {
    ਵਾਪਸ swab16(x);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 be32_ਮੰਜ਼ਿਲ_cpu(u32 x) {
    ਵਾਪਸ swab32(x);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u64 be64_ਮੰਜ਼ਿਲ_cpu(u64 x) {
    ਵਾਪਸ swab64(x);
}

#ਜੇ_ਅੰਤ // byteorder.h
