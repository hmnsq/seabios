#ifndef __BYTEORDER_H
#define __BYTEORDER_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 __swab16_constant(u16 val) {
    ଫେରନ୍ତୁ (val<<8) | (val>>8);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 __swab32_constant(u32 val) {
    ଫେରନ୍ତୁ (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 __swab64_constant(u64 val) {
    ଫେରନ୍ତୁ ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 __swab32(u32 val) {
    asm("bswapl %0" : "+r"(val));
    ଫେରନ୍ତୁ val;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 __swab64(u64 val) {
    ସଂଘ u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    ଫେରନ୍ତୁ o.val;
}

#define swab16(x) __swab16_constant(x)
#define swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#define swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 cpu_to_le16(u16 x) {
    ଫେରନ୍ତୁ x;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 cpu_to_le32(u32 x) {
    ଫେରନ୍ତୁ x;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 cpu_to_le64(u64 x) {
    ଫେରନ୍ତୁ x;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 le16_to_cpu(u16 x) {
    ଫେରନ୍ତୁ x;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 le32_to_cpu(u32 x) {
    ଫେରନ୍ତୁ x;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 le64_to_cpu(u64 x) {
    ଫେରନ୍ତୁ x;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 cpu_to_be16(u16 x) {
    ଫେରନ୍ତୁ swab16(x);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 cpu_to_be32(u32 x) {
    ଫେରନ୍ତୁ swab32(x);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 cpu_to_be64(u64 x) {
    ଫେରନ୍ତୁ swab64(x);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 be16_to_cpu(u16 x) {
    ଫେରନ୍ତୁ swab16(x);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 be32_to_cpu(u32 x) {
    ଫେରନ୍ତୁ swab32(x);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 be64_to_cpu(u64 x) {
    ଫେରନ୍ତୁ swab64(x);
}

#endif // byteorder.h
