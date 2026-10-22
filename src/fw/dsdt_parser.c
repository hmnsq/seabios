// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "list.h"   // hlist_*
#அந்தர்பாவ "malloc.h" // malloc_*
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "util.h"
#அந்தர்பாவ "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

கட்டமைப்பு acpi_device {
    கட்டமைப்பு hlist_முனை முனை;
    எழுத்து name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    முழு crs_size;
};
நிலையான கட்டமைப்பு hlist_head acpi_devices VARVERIFY32INIT;
நிலையான மாறிலி முழு parse_dumpdevs = 0;

கட்டமைப்பு parse_state {
    எழுத்து name[32];
    கட்டமைப்பு acpi_device *dev;
    முழு error;
    முழு depth;
};

நிலையான வெறுமை parse_termlist(கட்டமைப்பு parse_state *s,
                           u8 *ptr, முழு offset, முழு pkglength);

நிலையான வெறுமை hex(மாறிலி u8 *ptr, முழு மொத்தம், முழு lvl, மாறிலி எழுத்து *item)
{
    முழு l = 0, i;

    செய் {
        dprintf(lvl, "%s: %04x:  ", item, l);
        க்கு (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        க்கு (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } போது (l < மொத்தம்);
}

நிலையான u64 parse_resource_int(u8 *ptr, முழு மொத்தம்)
{
    u64 மதிப்பு = 0;
    முழு index = 0;

    க்கு (index = 0; index < மொத்தம்; index++)
        மதிப்பு |= (u64)ptr[index] << (index * 8);
    திரும்பு மதிப்பு;
}

நிலையான முழு parse_resource_bit(u8 *ptr, முழு மொத்தம்)
{
    முழு bit;

    க்கு (bit = 0; bit < மொத்தம்*8; bit++)
        எனில் (ptr[bit/8] & (1 << (bit%8)))
            திரும்பு bit;
    திரும்பு 0;
}

நிலையான முழு parse_resource(u8 *ptr, முழு length, முழு *type, u64 *min, u64 *max)
{
    முழு rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    எனில் (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        தேர்வு (rname) {
        நிலை 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            நிறுத்து;
        நிலை 0x0f: /* end marker */
            திரும்பு 0;
        நிலை 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            எனில் (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            நிறுத்து;
        நிலை 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            நிறுத்து;
        இயல்பு:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            நிறுத்து;
        }
    } இல்லையெனில் {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        தேர்வு (rname) {
        நிலை 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            நிறுத்து;
        நிலை 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            நிறுத்து;
        நிலை 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            நிறுத்து;
        நிலை 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            நிறுத்து;
        நிலை 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            நிறுத்து;
        இயல்பு:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            நிறுத்து;
        }
    }
    திரும்பு rsize;
}

நிலையான முழு find_resource(u8 *ptr, முழு len, முழு kind, u64 *min, u64 *max)
{
    முழு type, size, offset = 0;

    செய் {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        எனில் (kind == type)
            திரும்பு 0;
        offset += size;
    } போது (size > 0 && offset < len);
    திரும்பு -1;
}

நிலையான முழு print_resources(மாறிலி எழுத்து *prefix, u8 *ptr, முழு len)
{
    நிலையான மாறிலி எழுத்து *typename[] = { "mem", "i/o", "bus" };
    முழு type, size, offset = 0;
    u64 min, max;

    செய் {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        தேர்வு (type) {
        நிலை 0:
        நிலை 1:
        நிலை 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            நிறுத்து;
        நிலை 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            நிறுத்து;
        }
        offset += size;
    } போது (size > 0 && offset < len);
    திரும்பு -1;
}

நிலையான முழு parse_nameseg(u8 *ptr, எழுத்து **dst)
{
    எனில் (dst && *dst) {
        *(dst[0]++) = ptr[0];
        எனில் (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        எனில் (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        எனில் (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    திரும்பு 4;
}

நிலையான முழு parse_namestring(கட்டமைப்பு parse_state *s,
                            u8 *ptr, மாறிலி எழுத்து *item)
{
    எழுத்து *dst = s->name;
    முழு offset = 0;
    முழு i, மொத்தம்;

    க்கு (;;) {
        தேர்வு (ptr[offset]) {
        நிலை 0: /* null name */
            offset++;
            *(dst++) = 0;
            நிறுத்து;
        நிலை 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            நிறுத்து;
        நிலை 0x2f:
            offset++;
            மொத்தம் = ptr[offset];
            offset++;
            க்கு (i = 0; i < மொத்தம்; i++) {
                எனில் (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            நிறுத்து;
        நிலை '\\':
            *(dst++) = '\\';
            offset++;
            தொடர்;
        நிலை '^':
            *(dst++) = '^';
            offset++;
            தொடர்;
        நிலை 'A' ... 'Z':
        நிலை '_':
            offset += parse_nameseg(ptr, &dst);
            நிறுத்து;
        இயல்பு:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            நிறுத்து;
        }
        நிறுத்து;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    திரும்பு offset;
}

நிலையான முழு parse_termarg_int(u8 *ptr, முழு *error, u64 *dst)
{
    u64 மதிப்பு;
    முழு offset = 1;

    தேர்வு (ptr[0]) {
    நிலை 0x00: /* zero */
        மதிப்பு = 0;
        நிறுத்து;
    நிலை 0x01: /* one */
        மதிப்பு = 1;
        நிறுத்து;
    நிலை 0x0a: /* byte prefix */
        மதிப்பு = ptr[1];
        offset++;
        நிறுத்து;
    நிலை 0x0b: /* word prefix */
        மதிப்பு = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        நிறுத்து;
    நிலை 0x0c: /* dword prefix */
        மதிப்பு = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        நிறுத்து;
    இயல்பு:
        மதிப்பு = 0;
        hex(ptr, 16, 3, __func__);
        எனில் (error)
            *error = 1;
        நிறுத்து;
    }

    எனில் (dst)
        *dst = மதிப்பு;
    dprintf(5, "%s: 0x%llx\n", __func__, மதிப்பு);
    திரும்பு offset;
}

நிலையான முழு parse_pkglength(u8 *ptr, முழு *pkglength)
{
    முழு offset = 2;

    *pkglength = 0;
    தேர்வு (ptr[0] >> 6) {
    நிலை 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    நிலை 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    நிலை 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        திரும்பு offset;
    நிலை 0:
    இயல்பு:
        *pkglength |= ptr[0] & 0x3f;
        திரும்பு 1;
    }
}

நிலையான முழு parse_pkg_common(கட்டமைப்பு parse_state *s,
                            u8 *ptr, மாறிலி எழுத்து *item, முழு *pkglength)
{
    முழு offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    திரும்பு offset;
}

நிலையான முழு parse_pkg_scope(கட்டமைப்பு parse_state *s,
                           u8 *ptr)
{
    முழு offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    திரும்பு pkglength;
}

நிலையான முழு parse_pkg_device(கட்டமைப்பு parse_state *s,
                            u8 *ptr)
{
    முழு offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(அளவு(கட்டமைப்பு acpi_device));
    எனில் (!s->dev) {
        warn_noalloc();
        s->error = 1;
        திரும்பு pkglength;
    }

    memset(s->dev, 0, அளவு(கட்டமைப்பு acpi_device));
    hlist_add_head(&s->dev->முனை, &acpi_devices);
    strtcpy(s->dev->name, s->name, அளவு(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = வெற்று;

    திரும்பு pkglength;
}

நிலையான முழு parse_pkg_buffer(கட்டமைப்பு parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    முழு pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    எனில் (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    திரும்பு pkglength;
}

நிலையான முழு parse_pkg_skip(கட்டமைப்பு parse_state *s,
                          u8 *ptr, முழு op, முழு name)
{
    முழு pkglength, offset;
    எழுத்து item[8];

    snprintf(item, அளவு(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    எனில் (name) {
        parse_namestring(s, ptr + offset, item);
    } இல்லையெனில் {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    திரும்பு pkglength;
}

நிலையான முழு parse_termobj(கட்டமைப்பு parse_state *s,
                         u8 *ptr)
{
    முழு offset = 1;

    எனில் (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        திரும்பு offset;
    }

    s->depth++;
    தேர்வு (ptr[0]) {
    நிலை 0x00: /* zero */
        நிறுத்து;
    நிலை 0x01: /* one */
        நிறுத்து;
    நிலை 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        நிறுத்து;
    நிலை 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        எனில் (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        எனில் (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        நிறுத்து;
    நிலை 0x0a: /* byte prefix */
        offset++;
        நிறுத்து;
    நிலை 0x0b: /* word prefix */
        offset += 2;
        நிறுத்து;
    நிலை 0x0c: /* dword prefix */
        offset += 4;
        நிறுத்து;
    நிலை 0x0d: /* string prefix */
        போது (ptr[offset])
            offset++;
        offset++;
        நிறுத்து;
    நிலை 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        நிறுத்து;
    நிலை 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        நிறுத்து;
    நிலை 0x12: /* package op */
    நிலை 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        நிறுத்து;
    நிலை 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        எனில் (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        நிறுத்து;
    நிலை 0x5b: /* ext op prefix */
        offset++;
        தேர்வு (ptr[1]) {
        நிலை 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            நிறுத்து;
        நிலை 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, வெற்று);
            offset += parse_termarg_int(ptr + offset, &s->error, வெற்று);
            நிறுத்து;
        நிலை 0x81: /* field op */
        நிலை 0x83: /* processor op */
        நிலை 0x84: /* power resource op */
        நிலை 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            நிறுத்து;
        நிலை 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            நிறுத்து;
        இயல்பு:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            நிறுத்து;
        }
        நிறுத்து;
    இயல்பு:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        நிறுத்து;
    }
    s->depth--;

    திரும்பு offset;
}

நிலையான வெறுமை parse_termlist(கட்டமைப்பு parse_state *s,
                           u8 *ptr, முழு offset, முழு pkglength)
{
    க்கு (;;) {
        offset += parse_termobj(s, ptr + offset);
        எனில் (offset == pkglength)
            திரும்பு;
        எனில் (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            திரும்பு;
        }
        எனில் (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            திரும்பு;
        }
    }
}

நிலையான கட்டமைப்பு acpi_device *acpi_dsdt_find(கட்டமைப்பு acpi_device *prev,
                                          மாறிலி u8 *aml1, முழு size1,
                                          மாறிலி u8 *aml2, முழு size2)
{
    கட்டமைப்பு acpi_device *dev;
    கட்டமைப்பு hlist_முனை *முனை;

    எனில் (!prev)
        முனை = acpi_devices.first;
    இல்லையெனில்
        முனை = prev->முனை.next;

    க்கு (; முனை != வெற்று; முனை = dev->முனை.next) {
        dev = container_of(முனை, கட்டமைப்பு acpi_device, முனை);
        எனில் (!aml1 && !aml2)
            திரும்பு dev;
        எனில் (!dev->hid_aml)
            தொடர்;
        எனில் (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            திரும்பு dev;
        எனில் (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            திரும்பு dev;
    }
    திரும்பு வெற்று;
}

நிலையான முழு acpi_dsdt_present(கட்டமைப்பு acpi_device *dev)
{
    எனில் (!dev)
        திரும்பு 0; /* no */
    எனில் (!dev->sta_aml)
        திரும்பு 1; /* yes */
    எனில் (dev->sta_aml[0] == 0x14)
        திரும்பு -1; /* unknown (can't evaluate method) */
    எனில் (dev->sta_aml[0] == 0x08) {
        u64 மதிப்பு = 0;
        parse_termarg_int(dev->sta_aml + 5, வெற்று, &மதிப்பு);
        எனில் (மதிப்பு == 0)
            திரும்பு 0; /* no */
        இல்லையெனில்
            திரும்பு 1; /* yes */
    }
    திரும்பு -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

கட்டமைப்பு acpi_device *acpi_dsdt_find_string(கட்டமைப்பு acpi_device *prev,
                                          மாறிலி எழுத்து *hid)
{
    எனில் (!CONFIG_ACPI_PARSE)
        திரும்பு வெற்று;

    u8 aml[10];
    முழு len = snprintf((எழுத்து*)aml, அளவு(aml), "\x0d%s", hid);
    திரும்பு acpi_dsdt_find(prev, aml, len, வெற்று, 0);
}

கட்டமைப்பு acpi_device *acpi_dsdt_find_eisaid(கட்டமைப்பு acpi_device *prev, u16 eisaid)
{
    எனில் (!CONFIG_ACPI_PARSE)
        திரும்பு வெற்று;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    முழு len2 = snprintf((எழுத்து*)aml2, அளவு(aml2), "\x0dPNP%04X", eisaid);
    திரும்பு acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

எழுத்து *acpi_dsdt_name(கட்டமைப்பு acpi_device *dev)
{
    எனில் (!CONFIG_ACPI_PARSE || !dev)
        திரும்பு வெற்று;
    திரும்பு dev->name;
}

முழு acpi_dsdt_find_io(கட்டமைப்பு acpi_device *dev, u64 *min, u64 *max)
{
    எனில் (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        திரும்பு -1;
    திரும்பு find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

முழு acpi_dsdt_find_mem(கட்டமைப்பு acpi_device *dev, u64 *min, u64 *max)
{
    எனில் (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        திரும்பு -1;
    திரும்பு find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

முழு acpi_dsdt_find_irq(கட்டமைப்பு acpi_device *dev, u64 *irq)
{
    u64 max;
    எனில் (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        திரும்பு -1;
    திரும்பு find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

முழு acpi_dsdt_present_eisaid(u16 eisaid)
{
    எனில் (!CONFIG_ACPI_PARSE)
        திரும்பு -1; /* unknown */
    எனில் (hlist_empty(&acpi_devices))
        திரும்பு -1; /* unknown (no dsdt table) */

    கட்டமைப்பு acpi_device *dev = acpi_dsdt_find_eisaid(வெற்று, eisaid);
    திரும்பு acpi_dsdt_present(dev);
}

வெறுமை acpi_dsdt_parse(வெறுமை)
{
    எனில் (!CONFIG_ACPI_PARSE)
        திரும்பு;

    கட்டமைப்பு fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    எனில் (!fadt)
        திரும்பு;
    u8 *dsdt = (வெறுமை*)(fadt->dsdt);
    எனில் (!dsdt)
        திரும்பு;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    கட்டமைப்பு parse_state s;
    memset(&s, 0, அளவு(s));
    parse_termlist(&s, dsdt, offset, length);

    எனில் (!parse_dumpdevs)
        திரும்பு;

    கட்டமைப்பு acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    க்கு (dev = acpi_dsdt_find(வெற்று, வெற்று, 0, வெற்று, 0);
         dev != வெற்று;
         dev = acpi_dsdt_find(dev, வெற்று, 0, வெற்று, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        எனில் (dev->hid_aml)
            dprintf(1, ", hid");
        எனில் (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        எனில் (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        எனில் (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
