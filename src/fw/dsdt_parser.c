// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "list.h"   // hlist_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_*
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "util.h"
#ଅନ୍ତର୍ଭାଵ "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

ସଂରଚନା acpi_device {
    ସଂରଚନା hlist_ଗଣ୍ଠି ଗଣ୍ଠି;
    ଅକ୍ଷର name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା crs_size;
};
ସ୍ଥିର ସଂରଚନା hlist_head acpi_devices VARVERIFY32INIT;
ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_dumpdevs = 0;

ସଂରଚନା parse_state {
    ଅକ୍ଷର name[32];
    ସଂରଚନା acpi_device *dev;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା error;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth;
};

ସ୍ଥିର ଶୂନ୍ୟ parse_termlist(ସଂରଚନା parse_state *s,
                           u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pkglength);

ସ୍ଥିର ଶୂନ୍ୟ hex(ଅଚଳ u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lvl, ଅଚଳ ଅକ୍ଷର *item)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା l = 0, i;

    କର {
        dprintf(lvl, "%s: %04x:  ", item, l);
        ପାଇଁ (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        ପାଇଁ (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } ଯେତେବେଳେ (l < ଗଣନା_2);
}

ସ୍ଥିର u64 parse_resource_int(u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
    u64 ମୂଲ୍ୟ = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = 0;

    ପାଇଁ (index = 0; index < ଗଣନା_2; index++)
        ମୂଲ୍ୟ |= (u64)ptr[index] << (index * 8);
    ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_resource_bit(u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bit;

    ପାଇଁ (bit = 0; bit < ଗଣନା_2*8; bit++)
        ଯଦି (ptr[bit/8] & (1 << (bit%8)))
            ଫେରନ୍ତୁ bit;
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_resource(u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା length, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *type, u64 *min, u64 *max)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    ଯଦି (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        ବାଛ (rname) {
        ସ୍ଥିତି 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x0f: /* end marker */
            ଫେରନ୍ତୁ 0;
        ସ୍ଥିତି 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            ଯଦି (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            ଭାଙ୍ଗ;
        }
    } ଅନ୍ୟଥା {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        ବାଛ (rname) {
        ସ୍ଥିତି 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            ଭାଙ୍ଗ;
        }
    }
    ଫେରନ୍ତୁ rsize;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା find_resource(u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା kind, u64 *min, u64 *max)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, size, offset = 0;

    କର {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        ଯଦି (kind == type)
            ଫେରନ୍ତୁ 0;
        offset += size;
    } ଯେତେବେଳେ (size > 0 && offset < len);
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା print_resources(ଅଚଳ ଅକ୍ଷର *prefix, u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *typename[] = { "mem", "i/o", "bus" };
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, size, offset = 0;
    u64 min, max;

    କର {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        ବାଛ (type) {
        ସ୍ଥିତି 0:
        ସ୍ଥିତି 1:
        ସ୍ଥିତି 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            ଭାଙ୍ଗ;
        }
        offset += size;
    } ଯେତେବେଳେ (size > 0 && offset < len);
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_nameseg(u8 *ptr, ଅକ୍ଷର **dst)
{
    ଯଦି (dst && *dst) {
        *(dst[0]++) = ptr[0];
        ଯଦି (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        ଯଦି (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        ଯଦି (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    ଫେରନ୍ତୁ 4;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_namestring(ସଂରଚନା parse_state *s,
                            u8 *ptr, ଅଚଳ ଅକ୍ଷର *item)
{
    ଅକ୍ଷର *dst = s->name;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ଗଣନା_2;

    ପାଇଁ (;;) {
        ବାଛ (ptr[offset]) {
        ସ୍ଥିତି 0: /* null name */
            offset++;
            *(dst++) = 0;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x2f:
            offset++;
            ଗଣନା_2 = ptr[offset];
            offset++;
            ପାଇଁ (i = 0; i < ଗଣନା_2; i++) {
                ଯଦି (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            ଭାଙ୍ଗ;
        ସ୍ଥିତି '\\':
            *(dst++) = '\\';
            offset++;
            ଜାରି;
        ସ୍ଥିତି '^':
            *(dst++) = '^';
            offset++;
            ଜାରି;
        ସ୍ଥିତି 'A' ... 'Z':
        ସ୍ଥିତି '_':
            offset += parse_nameseg(ptr, &dst);
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ଭାଙ୍ଗ;
        }
        ଭାଙ୍ଗ;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    ଫେରନ୍ତୁ offset;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_termarg_int(u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *error, u64 *dst)
{
    u64 ମୂଲ୍ୟ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset = 1;

    ବାଛ (ptr[0]) {
    ସ୍ଥିତି 0x00: /* zero */
        ମୂଲ୍ୟ = 0;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: /* one */
        ମୂଲ୍ୟ = 1;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0a: /* byte prefix */
        ମୂଲ୍ୟ = ptr[1];
        offset++;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0b: /* word prefix */
        ମୂଲ୍ୟ = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0c: /* dword prefix */
        ମୂଲ୍ୟ = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ମୂଲ୍ୟ = 0;
        hex(ptr, 16, 3, __func__);
        ଯଦି (error)
            *error = 1;
        ଭାଙ୍ଗ;
    }

    ଯଦି (dst)
        *dst = ମୂଲ୍ୟ;
    dprintf(5, "%s: 0x%llx\n", __func__, ମୂଲ୍ୟ);
    ଫେରନ୍ତୁ offset;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_pkglength(u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pkglength)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset = 2;

    *pkglength = 0;
    ବାଛ (ptr[0] >> 6) {
    ସ୍ଥିତି 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    ସ୍ଥିତି 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    ସ୍ଥିତି 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        ଫେରନ୍ତୁ offset;
    ସ୍ଥିତି 0:
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        *pkglength |= ptr[0] & 0x3f;
        ଫେରନ୍ତୁ 1;
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_pkg_common(ସଂରଚନା parse_state *s,
                            u8 *ptr, ଅଚଳ ଅକ୍ଷର *item, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pkglength)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    ଫେରନ୍ତୁ offset;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_pkg_scope(ସଂରଚନା parse_state *s,
                           u8 *ptr)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    ଫେରନ୍ତୁ pkglength;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_pkg_device(ସଂରଚନା parse_state *s,
                            u8 *ptr)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(ଆକାର(ସଂରଚନା acpi_device));
    ଯଦି (!s->dev) {
        warn_noalloc();
        s->error = 1;
        ଫେରନ୍ତୁ pkglength;
    }

    memset(s->dev, 0, ଆକାର(ସଂରଚନା acpi_device));
    hlist_add_head(&s->dev->ଗଣ୍ଠି, &acpi_devices);
    strtcpy(s->dev->name, s->name, ଆକାର(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = ଖାଲି;

    ଫେରନ୍ତୁ pkglength;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_pkg_buffer(ସଂରଚନା parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    ଯଦି (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    ଫେରନ୍ତୁ pkglength;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_pkg_skip(ସଂରଚନା parse_state *s,
                          u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା name)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pkglength, offset;
    ଅକ୍ଷର item[8];

    snprintf(item, ଆକାର(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    ଯଦି (name) {
        parse_namestring(s, ptr + offset, item);
    } ଅନ୍ୟଥା {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    ଫେରନ୍ତୁ pkglength;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_termobj(ସଂରଚନା parse_state *s,
                         u8 *ptr)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset = 1;

    ଯଦି (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        ଫେରନ୍ତୁ offset;
    }

    s->depth++;
    ବାଛ (ptr[0]) {
    ସ୍ଥିତି 0x00: /* zero */
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x01: /* one */
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        ଯଦି (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        ଯଦି (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0a: /* byte prefix */
        offset++;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0b: /* word prefix */
        offset += 2;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0c: /* dword prefix */
        offset += 4;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x0d: /* string prefix */
        ଯେତେବେଳେ (ptr[offset])
            offset++;
        offset++;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x12: /* package op */
    ସ୍ଥିତି 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        ଯଦି (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 0x5b: /* ext op prefix */
        offset++;
        ବାଛ (ptr[1]) {
        ସ୍ଥିତି 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, ଖାଲି);
            offset += parse_termarg_int(ptr + offset, &s->error, ଖାଲି);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x81: /* field op */
        ସ୍ଥିତି 0x83: /* processor op */
        ସ୍ଥିତି 0x84: /* power resource op */
        ସ୍ଥିତି 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ଭାଙ୍ଗ;
        }
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        ଭାଙ୍ଗ;
    }
    s->depth--;

    ଫେରନ୍ତୁ offset;
}

ସ୍ଥିର ଶୂନ୍ୟ parse_termlist(ସଂରଚନା parse_state *s,
                           u8 *ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pkglength)
{
    ପାଇଁ (;;) {
        offset += parse_termobj(s, ptr + offset);
        ଯଦି (offset == pkglength)
            ଫେରନ୍ତୁ;
        ଯଦି (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            ଫେରନ୍ତୁ;
        }
        ଯଦି (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            ଫେରନ୍ତୁ;
        }
    }
}

ସ୍ଥିର ସଂରଚନା acpi_device *acpi_dsdt_find(ସଂରଚନା acpi_device *prev,
                                          ଅଚଳ u8 *aml1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size1,
                                          ଅଚଳ u8 *aml2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size2)
{
    ସଂରଚନା acpi_device *dev;
    ସଂରଚନା hlist_ଗଣ୍ଠି *ଗଣ୍ଠି;

    ଯଦି (!prev)
        ଗଣ୍ଠି = acpi_devices.first;
    ଅନ୍ୟଥା
        ଗଣ୍ଠି = prev->ଗଣ୍ଠି.next;

    ପାଇଁ (; ଗଣ୍ଠି != ଖାଲି; ଗଣ୍ଠି = dev->ଗଣ୍ଠି.next) {
        dev = container_of(ଗଣ୍ଠି, ସଂରଚନା acpi_device, ଗଣ୍ଠି);
        ଯଦି (!aml1 && !aml2)
            ଫେରନ୍ତୁ dev;
        ଯଦି (!dev->hid_aml)
            ଜାରି;
        ଯଦି (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            ଫେରନ୍ତୁ dev;
        ଯଦି (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            ଫେରନ୍ତୁ dev;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_present(ସଂରଚନା acpi_device *dev)
{
    ଯଦି (!dev)
        ଫେରନ୍ତୁ 0; /* no */
    ଯଦି (!dev->sta_aml)
        ଫେରନ୍ତୁ 1; /* yes */
    ଯଦି (dev->sta_aml[0] == 0x14)
        ଫେରନ୍ତୁ -1; /* unknown (can't evaluate method) */
    ଯଦି (dev->sta_aml[0] == 0x08) {
        u64 ମୂଲ୍ୟ = 0;
        parse_termarg_int(dev->sta_aml + 5, ଖାଲି, &ମୂଲ୍ୟ);
        ଯଦି (ମୂଲ୍ୟ == 0)
            ଫେରନ୍ତୁ 0; /* no */
        ଅନ୍ୟଥା
            ଫେରନ୍ତୁ 1; /* yes */
    }
    ଫେରନ୍ତୁ -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

ସଂରଚନା acpi_device *acpi_dsdt_find_string(ସଂରଚନା acpi_device *prev,
                                          ଅଚଳ ଅକ୍ଷର *hid)
{
    ଯଦି (!CONFIG_ACPI_PARSE)
        ଫେରନ୍ତୁ ଖାଲି;

    u8 aml[10];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = snprintf((ଅକ୍ଷର*)aml, ଆକାର(aml), "\x0d%s", hid);
    ଫେରନ୍ତୁ acpi_dsdt_find(prev, aml, len, ଖାଲି, 0);
}

ସଂରଚନା acpi_device *acpi_dsdt_find_eisaid(ସଂରଚନା acpi_device *prev, u16 eisaid)
{
    ଯଦି (!CONFIG_ACPI_PARSE)
        ଫେରନ୍ତୁ ଖାଲି;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len2 = snprintf((ଅକ୍ଷର*)aml2, ଆକାର(aml2), "\x0dPNP%04X", eisaid);
    ଫେରନ୍ତୁ acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

ଅକ୍ଷର *acpi_dsdt_name(ସଂରଚନା acpi_device *dev)
{
    ଯଦି (!CONFIG_ACPI_PARSE || !dev)
        ଫେରନ୍ତୁ ଖାଲି;
    ଫେରନ୍ତୁ dev->name;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_find_io(ସଂରଚନା acpi_device *dev, u64 *min, u64 *max)
{
    ଯଦି (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ଫେରନ୍ତୁ -1;
    ଫେରନ୍ତୁ find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_find_mem(ସଂରଚନା acpi_device *dev, u64 *min, u64 *max)
{
    ଯଦି (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ଫେରନ୍ତୁ -1;
    ଫେରନ୍ତୁ find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_find_irq(ସଂରଚନା acpi_device *dev, u64 *irq)
{
    u64 max;
    ଯଦି (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ଫେରନ୍ତୁ -1;
    ଫେରନ୍ତୁ find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା acpi_dsdt_present_eisaid(u16 eisaid)
{
    ଯଦି (!CONFIG_ACPI_PARSE)
        ଫେରନ୍ତୁ -1; /* unknown */
    ଯଦି (hlist_empty(&acpi_devices))
        ଫେରନ୍ତୁ -1; /* unknown (no dsdt table) */

    ସଂରଚନା acpi_device *dev = acpi_dsdt_find_eisaid(ଖାଲି, eisaid);
    ଫେରନ୍ତୁ acpi_dsdt_present(dev);
}

ଶୂନ୍ୟ acpi_dsdt_parse(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_ACPI_PARSE)
        ଫେରନ୍ତୁ;

    ସଂରଚନା fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ଯଦି (!fadt)
        ଫେରନ୍ତୁ;
    u8 *dsdt = (ଶୂନ୍ୟ*)(fadt->dsdt);
    ଯଦି (!dsdt)
        ଫେରନ୍ତୁ;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    ସଂରଚନା parse_state s;
    memset(&s, 0, ଆକାର(s));
    parse_termlist(&s, dsdt, offset, length);

    ଯଦି (!parse_dumpdevs)
        ଫେରନ୍ତୁ;

    ସଂରଚନା acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    ପାଇଁ (dev = acpi_dsdt_find(ଖାଲି, ଖାଲି, 0, ଖାଲି, 0);
         dev != ଖାଲି;
         dev = acpi_dsdt_find(dev, ଖାଲି, 0, ଖାଲି, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        ଯଦି (dev->hid_aml)
            dprintf(1, ", hid");
        ଯଦି (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        ଯଦି (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        ଯଦି (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
