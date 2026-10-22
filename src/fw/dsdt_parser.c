// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "list.h"   // hlist_*
#شامل "malloc.h" // malloc_*
#شامل "output.h" // dprintf
#شامل "string.h" // memcpy
#شامل "util.h"
#شامل "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

ساختار acpi_device {
    ساختار hlist_گره گره;
    نویسه name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    صحیح crs_size;
};
ایستا ساختار hlist_head acpi_devices VARVERIFY32INIT;
ایستا ثابت صحیح parse_dumpdevs = 0;

ساختار parse_state {
    نویسه name[32];
    ساختار acpi_device *dev;
    صحیح error;
    صحیح depth;
};

ایستا تهی parse_termlist(ساختار parse_state *s,
                           u8 *ptr, صحیح offset, صحیح pkglength);

ایستا تهی hex(ثابت u8 *ptr, صحیح شمار, صحیح lvl, ثابت نویسه *item)
{
    صحیح l = 0, i;

    انجامبده {
        dprintf(lvl, "%s: %04x:  ", item, l);
        برای (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        برای (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } وقتی (l < شمار);
}

ایستا u64 parse_resource_int(u8 *ptr, صحیح شمار)
{
    u64 مقدار = 0;
    صحیح index = 0;

    برای (index = 0; index < شمار; index++)
        مقدار |= (u64)ptr[index] << (index * 8);
    بازگشت مقدار;
}

ایستا صحیح parse_resource_bit(u8 *ptr, صحیح شمار)
{
    صحیح bit;

    برای (bit = 0; bit < شمار*8; bit++)
        اگر (ptr[bit/8] & (1 << (bit%8)))
            بازگشت bit;
    بازگشت 0;
}

ایستا صحیح parse_resource(u8 *ptr, صحیح length, صحیح *type, u64 *min, u64 *max)
{
    صحیح rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    اگر (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        انتخاب (rname) {
        حالت 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            بشکن;
        حالت 0x0f: /* end marker */
            بازگشت 0;
        حالت 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            اگر (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            بشکن;
        حالت 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            بشکن;
        پیشفرض:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            بشکن;
        }
    } وگرنه {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        انتخاب (rname) {
        حالت 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            بشکن;
        حالت 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            بشکن;
        حالت 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            بشکن;
        حالت 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            بشکن;
        حالت 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            بشکن;
        پیشفرض:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            بشکن;
        }
    }
    بازگشت rsize;
}

ایستا صحیح find_resource(u8 *ptr, صحیح len, صحیح kind, u64 *min, u64 *max)
{
    صحیح type, size, offset = 0;

    انجامبده {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        اگر (kind == type)
            بازگشت 0;
        offset += size;
    } وقتی (size > 0 && offset < len);
    بازگشت -1;
}

ایستا صحیح print_resources(ثابت نویسه *prefix, u8 *ptr, صحیح len)
{
    ایستا ثابت نویسه *typename[] = { "mem", "i/o", "bus" };
    صحیح type, size, offset = 0;
    u64 min, max;

    انجامبده {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        انتخاب (type) {
        حالت 0:
        حالت 1:
        حالت 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            بشکن;
        حالت 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            بشکن;
        }
        offset += size;
    } وقتی (size > 0 && offset < len);
    بازگشت -1;
}

ایستا صحیح parse_nameseg(u8 *ptr, نویسه **dst)
{
    اگر (dst && *dst) {
        *(dst[0]++) = ptr[0];
        اگر (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        اگر (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        اگر (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    بازگشت 4;
}

ایستا صحیح parse_namestring(ساختار parse_state *s,
                            u8 *ptr, ثابت نویسه *item)
{
    نویسه *dst = s->name;
    صحیح offset = 0;
    صحیح i, شمار;

    برای (;;) {
        انتخاب (ptr[offset]) {
        حالت 0: /* null name */
            offset++;
            *(dst++) = 0;
            بشکن;
        حالت 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            بشکن;
        حالت 0x2f:
            offset++;
            شمار = ptr[offset];
            offset++;
            برای (i = 0; i < شمار; i++) {
                اگر (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            بشکن;
        حالت '\\':
            *(dst++) = '\\';
            offset++;
            ادامه;
        حالت '^':
            *(dst++) = '^';
            offset++;
            ادامه;
        حالت 'A' ... 'Z':
        حالت '_':
            offset += parse_nameseg(ptr, &dst);
            بشکن;
        پیشفرض:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            بشکن;
        }
        بشکن;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    بازگشت offset;
}

ایستا صحیح parse_termarg_int(u8 *ptr, صحیح *error, u64 *dst)
{
    u64 مقدار;
    صحیح offset = 1;

    انتخاب (ptr[0]) {
    حالت 0x00: /* zero */
        مقدار = 0;
        بشکن;
    حالت 0x01: /* one */
        مقدار = 1;
        بشکن;
    حالت 0x0a: /* byte prefix */
        مقدار = ptr[1];
        offset++;
        بشکن;
    حالت 0x0b: /* word prefix */
        مقدار = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        بشکن;
    حالت 0x0c: /* dword prefix */
        مقدار = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        بشکن;
    پیشفرض:
        مقدار = 0;
        hex(ptr, 16, 3, __func__);
        اگر (error)
            *error = 1;
        بشکن;
    }

    اگر (dst)
        *dst = مقدار;
    dprintf(5, "%s: 0x%llx\n", __func__, مقدار);
    بازگشت offset;
}

ایستا صحیح parse_pkglength(u8 *ptr, صحیح *pkglength)
{
    صحیح offset = 2;

    *pkglength = 0;
    انتخاب (ptr[0] >> 6) {
    حالت 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    حالت 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    حالت 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        بازگشت offset;
    حالت 0:
    پیشفرض:
        *pkglength |= ptr[0] & 0x3f;
        بازگشت 1;
    }
}

ایستا صحیح parse_pkg_common(ساختار parse_state *s,
                            u8 *ptr, ثابت نویسه *item, صحیح *pkglength)
{
    صحیح offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    بازگشت offset;
}

ایستا صحیح parse_pkg_scope(ساختار parse_state *s,
                           u8 *ptr)
{
    صحیح offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    بازگشت pkglength;
}

ایستا صحیح parse_pkg_device(ساختار parse_state *s,
                            u8 *ptr)
{
    صحیح offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(اندازه(ساختار acpi_device));
    اگر (!s->dev) {
        warn_noalloc();
        s->error = 1;
        بازگشت pkglength;
    }

    memset(s->dev, 0, اندازه(ساختار acpi_device));
    hlist_add_head(&s->dev->گره, &acpi_devices);
    strtcpy(s->dev->name, s->name, اندازه(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = پوچ;

    بازگشت pkglength;
}

ایستا صحیح parse_pkg_buffer(ساختار parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    صحیح pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    اگر (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    بازگشت pkglength;
}

ایستا صحیح parse_pkg_skip(ساختار parse_state *s,
                          u8 *ptr, صحیح op, صحیح name)
{
    صحیح pkglength, offset;
    نویسه item[8];

    snprintf(item, اندازه(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    اگر (name) {
        parse_namestring(s, ptr + offset, item);
    } وگرنه {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    بازگشت pkglength;
}

ایستا صحیح parse_termobj(ساختار parse_state *s,
                         u8 *ptr)
{
    صحیح offset = 1;

    اگر (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        بازگشت offset;
    }

    s->depth++;
    انتخاب (ptr[0]) {
    حالت 0x00: /* zero */
        بشکن;
    حالت 0x01: /* one */
        بشکن;
    حالت 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        بشکن;
    حالت 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        اگر (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        اگر (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        بشکن;
    حالت 0x0a: /* byte prefix */
        offset++;
        بشکن;
    حالت 0x0b: /* word prefix */
        offset += 2;
        بشکن;
    حالت 0x0c: /* dword prefix */
        offset += 4;
        بشکن;
    حالت 0x0d: /* string prefix */
        وقتی (ptr[offset])
            offset++;
        offset++;
        بشکن;
    حالت 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        بشکن;
    حالت 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        بشکن;
    حالت 0x12: /* package op */
    حالت 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        بشکن;
    حالت 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        اگر (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        بشکن;
    حالت 0x5b: /* ext op prefix */
        offset++;
        انتخاب (ptr[1]) {
        حالت 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            بشکن;
        حالت 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, پوچ);
            offset += parse_termarg_int(ptr + offset, &s->error, پوچ);
            بشکن;
        حالت 0x81: /* field op */
        حالت 0x83: /* processor op */
        حالت 0x84: /* power resource op */
        حالت 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            بشکن;
        حالت 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            بشکن;
        پیشفرض:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            بشکن;
        }
        بشکن;
    پیشفرض:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        بشکن;
    }
    s->depth--;

    بازگشت offset;
}

ایستا تهی parse_termlist(ساختار parse_state *s,
                           u8 *ptr, صحیح offset, صحیح pkglength)
{
    برای (;;) {
        offset += parse_termobj(s, ptr + offset);
        اگر (offset == pkglength)
            بازگشت;
        اگر (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            بازگشت;
        }
        اگر (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            بازگشت;
        }
    }
}

ایستا ساختار acpi_device *acpi_dsdt_find(ساختار acpi_device *prev,
                                          ثابت u8 *aml1, صحیح size1,
                                          ثابت u8 *aml2, صحیح size2)
{
    ساختار acpi_device *dev;
    ساختار hlist_گره *گره;

    اگر (!prev)
        گره = acpi_devices.first;
    وگرنه
        گره = prev->گره.next;

    برای (; گره != پوچ; گره = dev->گره.next) {
        dev = container_of(گره, ساختار acpi_device, گره);
        اگر (!aml1 && !aml2)
            بازگشت dev;
        اگر (!dev->hid_aml)
            ادامه;
        اگر (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            بازگشت dev;
        اگر (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            بازگشت dev;
    }
    بازگشت پوچ;
}

ایستا صحیح acpi_dsdt_present(ساختار acpi_device *dev)
{
    اگر (!dev)
        بازگشت 0; /* no */
    اگر (!dev->sta_aml)
        بازگشت 1; /* yes */
    اگر (dev->sta_aml[0] == 0x14)
        بازگشت -1; /* unknown (can't evaluate method) */
    اگر (dev->sta_aml[0] == 0x08) {
        u64 مقدار = 0;
        parse_termarg_int(dev->sta_aml + 5, پوچ, &مقدار);
        اگر (مقدار == 0)
            بازگشت 0; /* no */
        وگرنه
            بازگشت 1; /* yes */
    }
    بازگشت -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

ساختار acpi_device *acpi_dsdt_find_string(ساختار acpi_device *prev,
                                          ثابت نویسه *hid)
{
    اگر (!CONFIG_ACPI_PARSE)
        بازگشت پوچ;

    u8 aml[10];
    صحیح len = snprintf((نویسه*)aml, اندازه(aml), "\x0d%s", hid);
    بازگشت acpi_dsdt_find(prev, aml, len, پوچ, 0);
}

ساختار acpi_device *acpi_dsdt_find_eisaid(ساختار acpi_device *prev, u16 eisaid)
{
    اگر (!CONFIG_ACPI_PARSE)
        بازگشت پوچ;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    صحیح len2 = snprintf((نویسه*)aml2, اندازه(aml2), "\x0dPNP%04X", eisaid);
    بازگشت acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

نویسه *acpi_dsdt_name(ساختار acpi_device *dev)
{
    اگر (!CONFIG_ACPI_PARSE || !dev)
        بازگشت پوچ;
    بازگشت dev->name;
}

صحیح acpi_dsdt_find_io(ساختار acpi_device *dev, u64 *min, u64 *max)
{
    اگر (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        بازگشت -1;
    بازگشت find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

صحیح acpi_dsdt_find_mem(ساختار acpi_device *dev, u64 *min, u64 *max)
{
    اگر (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        بازگشت -1;
    بازگشت find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

صحیح acpi_dsdt_find_irq(ساختار acpi_device *dev, u64 *irq)
{
    u64 max;
    اگر (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        بازگشت -1;
    بازگشت find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

صحیح acpi_dsdt_present_eisaid(u16 eisaid)
{
    اگر (!CONFIG_ACPI_PARSE)
        بازگشت -1; /* unknown */
    اگر (hlist_empty(&acpi_devices))
        بازگشت -1; /* unknown (no dsdt table) */

    ساختار acpi_device *dev = acpi_dsdt_find_eisaid(پوچ, eisaid);
    بازگشت acpi_dsdt_present(dev);
}

تهی acpi_dsdt_parse(تهی)
{
    اگر (!CONFIG_ACPI_PARSE)
        بازگشت;

    ساختار fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    اگر (!fadt)
        بازگشت;
    u8 *dsdt = (تهی*)(fadt->dsdt);
    اگر (!dsdt)
        بازگشت;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    ساختار parse_state s;
    memset(&s, 0, اندازه(s));
    parse_termlist(&s, dsdt, offset, length);

    اگر (!parse_dumpdevs)
        بازگشت;

    ساختار acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    برای (dev = acpi_dsdt_find(پوچ, پوچ, 0, پوچ, 0);
         dev != پوچ;
         dev = acpi_dsdt_find(dev, پوچ, 0, پوچ, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        اگر (dev->hid_aml)
            dprintf(1, ", hid");
        اگر (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        اگر (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        اگر (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
