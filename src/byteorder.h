#यदि_अपरिभाषितम् __BYTEORDER_H
#परिभाषय __BYTEORDER_H

#अन्तर्भाव "types.h" // u32

स्थिर अन्तःस्थ u16 __swab16_constant(u16 val) {
    प्रत्यागम (val<<8) | (val>>8);
}
स्थिर अन्तःस्थ u32 __swab32_constant(u32 val) {
    प्रत्यागम (val<<24) | ((val&0xff00)<<8) | ((val&0xff0000)>>8) | (val>>24);
}
स्थिर अन्तःस्थ u64 __swab64_constant(u64 val) {
    प्रत्यागम ((u64)__swab32_constant(val) << 32) | __swab32_constant(val>>32);
}
स्थिर अन्तःस्थ u32 __swab32(u32 val) {
    asm("bswapl %0" : "+r"(val));
    प्रत्यागम val;
}
स्थिर अन्तःस्थ u64 __swab64(u64 val) {
    सङ्घ u64_u32_u i, o;
    i.val = val;
    o.lo = __swab32(i.hi);
    o.hi = __swab32(i.lo);
    प्रत्यागम o.val;
}

#परिभाषय swab16(x) __swab16_constant(x)
#परिभाषय swab32(x) (__builtin_constant_p((u32)(x)) \
                   ? __swab32_constant(x) : __swab32(x))
#परिभाषय swab64(x) (__builtin_constant_p((u64)(x)) \
                   ? __swab64_constant(x) : __swab64(x))

स्थिर अन्तःस्थ u16 cpu_गन्तव्यम्_le16(u16 x) {
    प्रत्यागम x;
}
स्थिर अन्तःस्थ u32 cpu_गन्तव्यम्_le32(u32 x) {
    प्रत्यागम x;
}
स्थिर अन्तःस्थ u64 cpu_गन्तव्यम्_le64(u64 x) {
    प्रत्यागम x;
}
स्थिर अन्तःस्थ u16 le16_गन्तव्यम्_cpu(u16 x) {
    प्रत्यागम x;
}
स्थिर अन्तःस्थ u32 le32_गन्तव्यम्_cpu(u32 x) {
    प्रत्यागम x;
}
स्थिर अन्तःस्थ u64 le64_गन्तव्यम्_cpu(u64 x) {
    प्रत्यागम x;
}

स्थिर अन्तःस्थ u16 cpu_गन्तव्यम्_be16(u16 x) {
    प्रत्यागम swab16(x);
}
स्थिर अन्तःस्थ u32 cpu_गन्तव्यम्_be32(u32 x) {
    प्रत्यागम swab32(x);
}
स्थिर अन्तःस्थ u64 cpu_गन्तव्यम्_be64(u64 x) {
    प्रत्यागम swab64(x);
}
स्थिर अन्तःस्थ u16 be16_गन्तव्यम्_cpu(u16 x) {
    प्रत्यागम swab16(x);
}
स्थिर अन्तःस्थ u32 be32_गन्तव्यम्_cpu(u32 x) {
    प्रत्यागम swab32(x);
}
स्थिर अन्तःस्थ u64 be64_गन्तव्यम्_cpu(u64 x) {
    प्रत्यागम swab64(x);
}

#यदि_अन्तः // byteorder.h
