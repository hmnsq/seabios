// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "list.h"   // hlist_*
#ضمن "malloc.h" // malloc_*
#ضمن "output.h" // dprintf
#ضمن "string.h" // memcpy
#ضمن "util.h"
#ضمن "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

بنية acpi_device {
    بنية hlist_node node;
    حرف name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    صحيح crs_size;
};
ساكن بنية hlist_head acpi_devices VARVERIFY32INIT;
ساكن ثابت صحيح parse_dumpdevs = 0;

بنية parse_state {
    حرف name[32];
    بنية acpi_device *dev;
    صحيح error;
    صحيح depth;
};

ساكن فراغ parse_termlist(بنية parse_state *s,
                           u8 *ptr, صحيح offset, صحيح pkglength);

ساكن فراغ hex(ثابت u8 *ptr, صحيح count, صحيح lvl, ثابت حرف *item)
{
    صحيح l = 0, i;

    עשה {
        dprintf(lvl, "%s: %04x:  ", item, l);
        עבור (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        עבור (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } כלעוד (l < count);
}

ساكن u64 parse_resource_int(u8 *ptr, صحيح count)
{
    u64 value = 0;
    صحيح index = 0;

    עבור (index = 0; index < count; index++)
        value |= (u64)ptr[index] << (index * 8);
    החזר value;
}

ساكن صحيح parse_resource_bit(u8 *ptr, صحيح count)
{
    صحيح bit;

    עבור (bit = 0; bit < count*8; bit++)
        אם (ptr[bit/8] & (1 << (bit%8)))
            החזר bit;
    החזר 0;
}

ساكن صحيح parse_resource(u8 *ptr, صحيح length, صحيح *type, u64 *min, u64 *max)
{
    صحيح rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    אם (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        بدل (rname) {
        حالة 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            اكسر;
        حالة 0x0f: /* end marker */
            החזר 0;
        حالة 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            אם (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            اكسر;
        حالة 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            اكسر;
        افتراضي:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            اكسر;
        }
    } אחרת {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        بدل (rname) {
        حالة 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            اكسر;
        حالة 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            اكسر;
        حالة 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            اكسر;
        حالة 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            اكسر;
        حالة 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            اكسر;
        افتراضي:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            اكسر;
        }
    }
    החזר rsize;
}

ساكن صحيح find_resource(u8 *ptr, صحيح len, صحيح kind, u64 *min, u64 *max)
{
    صحيح type, size, offset = 0;

    עשה {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        אם (kind == type)
            החזר 0;
        offset += size;
    } כלעוד (size > 0 && offset < len);
    החזר -1;
}

ساكن صحيح print_resources(ثابت حرف *prefix, u8 *ptr, صحيح len)
{
    ساكن ثابت حرف *typename[] = { "mem", "i/o", "bus" };
    صحيح type, size, offset = 0;
    u64 min, max;

    עשה {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        بدل (type) {
        حالة 0:
        حالة 1:
        حالة 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            اكسر;
        حالة 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            اكسر;
        }
        offset += size;
    } כלעוד (size > 0 && offset < len);
    החזר -1;
}

ساكن صحيح parse_nameseg(u8 *ptr, حرف **dst)
{
    אם (dst && *dst) {
        *(dst[0]++) = ptr[0];
        אם (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        אם (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        אם (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    החזר 4;
}

ساكن صحيح parse_namestring(بنية parse_state *s,
                            u8 *ptr, ثابت حرف *item)
{
    حرف *dst = s->name;
    صحيح offset = 0;
    صحيح i, count;

    עבור (;;) {
        بدل (ptr[offset]) {
        حالة 0: /* null name */
            offset++;
            *(dst++) = 0;
            اكسر;
        حالة 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            اكسر;
        حالة 0x2f:
            offset++;
            count = ptr[offset];
            offset++;
            עבור (i = 0; i < count; i++) {
                אם (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            اكسر;
        حالة '\\':
            *(dst++) = '\\';
            offset++;
            استمر;
        حالة '^':
            *(dst++) = '^';
            offset++;
            استمر;
        حالة 'A' ... 'Z':
        حالة '_':
            offset += parse_nameseg(ptr, &dst);
            اكسر;
        افتراضي:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            اكسر;
        }
        اكسر;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    החזר offset;
}

ساكن صحيح parse_termarg_int(u8 *ptr, صحيح *error, u64 *dst)
{
    u64 value;
    صحيح offset = 1;

    بدل (ptr[0]) {
    حالة 0x00: /* zero */
        value = 0;
        اكسر;
    حالة 0x01: /* one */
        value = 1;
        اكسر;
    حالة 0x0a: /* byte prefix */
        value = ptr[1];
        offset++;
        اكسر;
    حالة 0x0b: /* word prefix */
        value = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        اكسر;
    حالة 0x0c: /* dword prefix */
        value = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        اكسر;
    افتراضي:
        value = 0;
        hex(ptr, 16, 3, __func__);
        אם (error)
            *error = 1;
        اكسر;
    }

    אם (dst)
        *dst = value;
    dprintf(5, "%s: 0x%llx\n", __func__, value);
    החזר offset;
}

ساكن صحيح parse_pkglength(u8 *ptr, صحيح *pkglength)
{
    صحيح offset = 2;

    *pkglength = 0;
    بدل (ptr[0] >> 6) {
    حالة 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    حالة 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    حالة 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        החזר offset;
    حالة 0:
    افتراضي:
        *pkglength |= ptr[0] & 0x3f;
        החזר 1;
    }
}

ساكن صحيح parse_pkg_common(بنية parse_state *s,
                            u8 *ptr, ثابت حرف *item, صحيح *pkglength)
{
    صحيح offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    החזר offset;
}

ساكن صحيح parse_pkg_scope(بنية parse_state *s,
                           u8 *ptr)
{
    صحيح offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    החזר pkglength;
}

ساكن صحيح parse_pkg_device(بنية parse_state *s,
                            u8 *ptr)
{
    صحيح offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(حجم(بنية acpi_device));
    אם (!s->dev) {
        warn_noalloc();
        s->error = 1;
        החזר pkglength;
    }

    memset(s->dev, 0, حجم(بنية acpi_device));
    hlist_add_head(&s->dev->node, &acpi_devices);
    strtcpy(s->dev->name, s->name, حجم(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = عدم;

    החזר pkglength;
}

ساكن صحيح parse_pkg_buffer(بنية parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    صحيح pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    אם (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    החזר pkglength;
}

ساكن صحيح parse_pkg_skip(بنية parse_state *s,
                          u8 *ptr, صحيح op, صحيح name)
{
    صحيح pkglength, offset;
    حرف item[8];

    snprintf(item, حجم(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    אם (name) {
        parse_namestring(s, ptr + offset, item);
    } אחרת {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    החזר pkglength;
}

ساكن صحيح parse_termobj(بنية parse_state *s,
                         u8 *ptr)
{
    صحيح offset = 1;

    אם (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        החזר offset;
    }

    s->depth++;
    بدل (ptr[0]) {
    حالة 0x00: /* zero */
        اكسر;
    حالة 0x01: /* one */
        اكسر;
    حالة 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        اكسر;
    حالة 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        אם (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        אם (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        اكسر;
    حالة 0x0a: /* byte prefix */
        offset++;
        اكسر;
    حالة 0x0b: /* word prefix */
        offset += 2;
        اكسر;
    حالة 0x0c: /* dword prefix */
        offset += 4;
        اكسر;
    حالة 0x0d: /* string prefix */
        כלעוד (ptr[offset])
            offset++;
        offset++;
        اكسر;
    حالة 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        اكسر;
    حالة 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        اكسر;
    حالة 0x12: /* package op */
    حالة 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        اكسر;
    حالة 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        אם (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        اكسر;
    حالة 0x5b: /* ext op prefix */
        offset++;
        بدل (ptr[1]) {
        حالة 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            اكسر;
        حالة 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, عدم);
            offset += parse_termarg_int(ptr + offset, &s->error, عدم);
            اكسر;
        حالة 0x81: /* field op */
        حالة 0x83: /* processor op */
        حالة 0x84: /* power resource op */
        حالة 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            اكسر;
        حالة 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            اكسر;
        افتراضي:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            اكسر;
        }
        اكسر;
    افتراضي:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        اكسر;
    }
    s->depth--;

    החזר offset;
}

ساكن فراغ parse_termlist(بنية parse_state *s,
                           u8 *ptr, صحيح offset, صحيح pkglength)
{
    עבור (;;) {
        offset += parse_termobj(s, ptr + offset);
        אם (offset == pkglength)
            החזר;
        אם (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            החזר;
        }
        אם (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            החזר;
        }
    }
}

ساكن بنية acpi_device *acpi_dsdt_find(بنية acpi_device *prev,
                                          ثابت u8 *aml1, صحيح size1,
                                          ثابت u8 *aml2, صحيح size2)
{
    بنية acpi_device *dev;
    بنية hlist_node *node;

    אם (!prev)
        node = acpi_devices.first;
    אחרת
        node = prev->node.next;

    עבור (; node != عدم; node = dev->node.next) {
        dev = container_of(node, بنية acpi_device, node);
        אם (!aml1 && !aml2)
            החזר dev;
        אם (!dev->hid_aml)
            استمر;
        אם (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            החזר dev;
        אם (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            החזר dev;
    }
    החזר عدم;
}

ساكن صحيح acpi_dsdt_present(بنية acpi_device *dev)
{
    אם (!dev)
        החזר 0; /* no */
    אם (!dev->sta_aml)
        החזר 1; /* yes */
    אם (dev->sta_aml[0] == 0x14)
        החזר -1; /* unknown (can't evaluate method) */
    אם (dev->sta_aml[0] == 0x08) {
        u64 value = 0;
        parse_termarg_int(dev->sta_aml + 5, عدم, &value);
        אם (value == 0)
            החזר 0; /* no */
        אחרת
            החזר 1; /* yes */
    }
    החזר -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

بنية acpi_device *acpi_dsdt_find_string(بنية acpi_device *prev,
                                          ثابت حرف *hid)
{
    אם (!CONFIG_ACPI_PARSE)
        החזר عدم;

    u8 aml[10];
    صحيح len = snprintf((حرف*)aml, حجم(aml), "\x0d%s", hid);
    החזר acpi_dsdt_find(prev, aml, len, عدم, 0);
}

بنية acpi_device *acpi_dsdt_find_eisaid(بنية acpi_device *prev, u16 eisaid)
{
    אם (!CONFIG_ACPI_PARSE)
        החזר عدم;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    صحيح len2 = snprintf((حرف*)aml2, حجم(aml2), "\x0dPNP%04X", eisaid);
    החזר acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

حرف *acpi_dsdt_name(بنية acpi_device *dev)
{
    אם (!CONFIG_ACPI_PARSE || !dev)
        החזר عدم;
    החזר dev->name;
}

صحيح acpi_dsdt_find_io(بنية acpi_device *dev, u64 *min, u64 *max)
{
    אם (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        החזר -1;
    החזר find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

صحيح acpi_dsdt_find_mem(بنية acpi_device *dev, u64 *min, u64 *max)
{
    אם (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        החזר -1;
    החזר find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

صحيح acpi_dsdt_find_irq(بنية acpi_device *dev, u64 *irq)
{
    u64 max;
    אם (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        החזר -1;
    החזר find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

صحيح acpi_dsdt_present_eisaid(u16 eisaid)
{
    אם (!CONFIG_ACPI_PARSE)
        החזר -1; /* unknown */
    אם (hlist_empty(&acpi_devices))
        החזר -1; /* unknown (no dsdt table) */

    بنية acpi_device *dev = acpi_dsdt_find_eisaid(عدم, eisaid);
    החזר acpi_dsdt_present(dev);
}

فراغ acpi_dsdt_parse(فراغ)
{
    אם (!CONFIG_ACPI_PARSE)
        החזר;

    بنية fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    אם (!fadt)
        החזר;
    u8 *dsdt = (فراغ*)(fadt->dsdt);
    אם (!dsdt)
        החזר;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    بنية parse_state s;
    memset(&s, 0, حجم(s));
    parse_termlist(&s, dsdt, offset, length);

    אם (!parse_dumpdevs)
        החזר;

    بنية acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    עבור (dev = acpi_dsdt_find(عدم, عدم, 0, عدم, 0);
         dev != عدم;
         dev = acpi_dsdt_find(dev, عدم, 0, عدم, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        אם (dev->hid_aml)
            dprintf(1, ", hid");
        אם (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        אם (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        אם (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
