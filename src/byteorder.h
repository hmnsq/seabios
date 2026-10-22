#నిర్వచితం_కాకపోతే __BYTEORDER_H
#నిర్వచించు __BYTEORDER_H

#అన్తర్భావ "types.h" // u32

స్థిర అంతఃవరుస u16 __swab16_constant(u16 val) {
    తిరిగి (val<<8) | (val>>8);
}
స్థిర అంతఃవరుస u32 __swab32_constant(u32 val) {
    తిరిగి (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
స్థిర అంతఃవరుస u64 __swab64_constant(u64 val) {
    తిరిగి ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
స్థిర అంతఃవరుస u32 __swab32(u32 val) {
    asm("bswapl %0" : "+r"(val));
    తిరిగి val;
}
స్థిర అంతఃవరుస u64 __swab64(u64 val) {
    సంఘం u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    తిరిగి o.val;
}

#నిర్వచించు swab16(x) __swab16_constant(x)
#నిర్వచించు swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#నిర్వచించు swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

స్థిర అంతఃవరుస u16 cpu_గమ్యం_le16(u16 x) {
    తిరిగి x;
}
స్థిర అంతఃవరుస u32 cpu_గమ్యం_le32(u32 x) {
    తిరిగి x;
}
స్థిర అంతఃవరుస u64 cpu_గమ్యం_le64(u64 x) {
    తిరిగి x;
}
స్థిర అంతఃవరుస u16 le16_గమ్యం_cpu(u16 x) {
    తిరిగి x;
}
స్థిర అంతఃవరుస u32 le32_గమ్యం_cpu(u32 x) {
    తిరిగి x;
}
స్థిర అంతఃవరుస u64 le64_గమ్యం_cpu(u64 x) {
    తిరిగి x;
}

స్థిర అంతఃవరుస u16 cpu_గమ్యం_be16(u16 x) {
    తిరిగి swab16(x);
}
స్థిర అంతఃవరుస u32 cpu_గమ్యం_be32(u32 x) {
    తిరిగి swab32(x);
}
స్థిర అంతఃవరుస u64 cpu_గమ్యం_be64(u64 x) {
    తిరిగి swab64(x);
}
స్థిర అంతఃవరుస u16 be16_గమ్యం_cpu(u16 x) {
    తిరిగి swab16(x);
}
స్థిర అంతఃవరుస u32 be32_గమ్యం_cpu(u32 x) {
    తిరిగి swab32(x);
}
స్థిర అంతఃవరుస u64 be64_గమ్యం_cpu(u64 x) {
    తిరిగి swab64(x);
}

#అయితే_ముగింపు // byteorder.h
