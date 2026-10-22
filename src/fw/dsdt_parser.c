// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "list.h"   // hlist_*
#অন্তর্ভাব "malloc.h" // malloc_*
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "string.h" // memcpy
#অন্তর্ভাব "util.h"
#অন্তর্ভাব "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

কাঠামো acpi_device {
    কাঠামো hlist_গ্রন্থি গ্রন্থি;
    অক্ষর name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_তথ্য;
    পূর্ণসংখ্যা crs_আকার;
};
স্থির কাঠামো hlist_শীর্ষ acpi_devices VARVERIFY32INIT;
স্থির ধ্রুবক পূর্ণসংখ্যা parse_dumpdevs = 0;

কাঠামো parse_state {
    অক্ষর name[32];
    কাঠামো acpi_device *dev;
    পূর্ণসংখ্যা error;
    পূর্ণসংখ্যা depth;
};

স্থির শূন্য parse_termlist(কাঠামো parse_state *s,
                           u8 *ptr, পূর্ণসংখ্যা offset, পূর্ণসংখ্যা pkglength);

স্থির শূন্য hex(ধ্রুবক u8 *ptr, পূর্ণসংখ্যা গুনতি, পূর্ণসংখ্যা lvl, ধ্রুবক অক্ষর *item)
{
    পূর্ণসংখ্যা l = 0, i;

    করো {
        dprintf(lvl, "%s: %04x:  ", item, l);
        জন্য (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        জন্য (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } যখন (l < গুনতি);
}

স্থির u64 parse_resource_int(u8 *ptr, পূর্ণসংখ্যা গুনতি)
{
    u64 মান = 0;
    পূর্ণসংখ্যা সূচক = 0;

    জন্য (সূচক = 0; সূচক < গুনতি; সূচক++)
        মান |= (u64)ptr[সূচক] << (সূচক * 8);
    ফেরত মান;
}

স্থির পূর্ণসংখ্যা parse_resource_bit(u8 *ptr, পূর্ণসংখ্যা গুনতি)
{
    পূর্ণসংখ্যা bit;

    জন্য (bit = 0; bit < গুনতি*8; bit++)
        যদি (ptr[bit/8] & (1 << (bit%8)))
            ফেরত bit;
    ফেরত 0;
}

স্থির পূর্ণসংখ্যা parse_resource(u8 *ptr, পূর্ণসংখ্যা দৈর্ঘ্য, পূর্ণসংখ্যা *type, u64 *min, u64 *max)
{
    পূর্ণসংখ্যা rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    যদি (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        নির্বাচন (rname) {
        ক্ষেত্র 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            বিরতি;
        ক্ষেত্র 0x0f: /* end marker */
            ফেরত 0;
        ক্ষেত্র 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            যদি (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            বিরতি;
        ক্ষেত্র 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            বিরতি;
        পূর্বনির্ধারিত:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            বিরতি;
        }
    } অন্যথা {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        নির্বাচন (rname) {
        ক্ষেত্র 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            বিরতি;
        ক্ষেত্র 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            বিরতি;
        ক্ষেত্র 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            বিরতি;
        ক্ষেত্র 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            বিরতি;
        ক্ষেত্র 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            বিরতি;
        পূর্বনির্ধারিত:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            বিরতি;
        }
    }
    ফেরত rsize;
}

স্থির পূর্ণসংখ্যা find_resource(u8 *ptr, পূর্ণসংখ্যা len, পূর্ণসংখ্যা kind, u64 *min, u64 *max)
{
    পূর্ণসংখ্যা type, আকার_2, offset = 0;

    করো {
        আকার_2 = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        যদি (kind == type)
            ফেরত 0;
        offset += আকার_2;
    } যখন (আকার_2 > 0 && offset < len);
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা print_resources(ধ্রুবক অক্ষর *prefix, u8 *ptr, পূর্ণসংখ্যা len)
{
    স্থির ধ্রুবক অক্ষর *typename[] = { "mem", "i/o", "bus" };
    পূর্ণসংখ্যা type, আকার_2, offset = 0;
    u64 min, max;

    করো {
        আকার_2 = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        নির্বাচন (type) {
        ক্ষেত্র 0:
        ক্ষেত্র 1:
        ক্ষেত্র 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            বিরতি;
        ক্ষেত্র 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            বিরতি;
        }
        offset += আকার_2;
    } যখন (আকার_2 > 0 && offset < len);
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা parse_nameseg(u8 *ptr, অক্ষর **dst)
{
    যদি (dst && *dst) {
        *(dst[0]++) = ptr[0];
        যদি (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        যদি (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        যদি (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    ফেরত 4;
}

স্থির পূর্ণসংখ্যা parse_namestring(কাঠামো parse_state *s,
                            u8 *ptr, ধ্রুবক অক্ষর *item)
{
    অক্ষর *dst = s->name;
    পূর্ণসংখ্যা offset = 0;
    পূর্ণসংখ্যা i, গুনতি;

    জন্য (;;) {
        নির্বাচন (ptr[offset]) {
        ক্ষেত্র 0: /* null name */
            offset++;
            *(dst++) = 0;
            বিরতি;
        ক্ষেত্র 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            বিরতি;
        ক্ষেত্র 0x2f:
            offset++;
            গুনতি = ptr[offset];
            offset++;
            জন্য (i = 0; i < গুনতি; i++) {
                যদি (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            বিরতি;
        ক্ষেত্র '\\':
            *(dst++) = '\\';
            offset++;
            চালু;
        ক্ষেত্র '^':
            *(dst++) = '^';
            offset++;
            চালু;
        ক্ষেত্র 'A' ... 'Z':
        ক্ষেত্র '_':
            offset += parse_nameseg(ptr, &dst);
            বিরতি;
        পূর্বনির্ধারিত:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            বিরতি;
        }
        বিরতি;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    ফেরত offset;
}

স্থির পূর্ণসংখ্যা parse_termarg_int(u8 *ptr, পূর্ণসংখ্যা *error, u64 *dst)
{
    u64 মান;
    পূর্ণসংখ্যা offset = 1;

    নির্বাচন (ptr[0]) {
    ক্ষেত্র 0x00: /* zero */
        মান = 0;
        বিরতি;
    ক্ষেত্র 0x01: /* one */
        মান = 1;
        বিরতি;
    ক্ষেত্র 0x0a: /* byte prefix */
        মান = ptr[1];
        offset++;
        বিরতি;
    ক্ষেত্র 0x0b: /* word prefix */
        মান = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        বিরতি;
    ক্ষেত্র 0x0c: /* dword prefix */
        মান = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        বিরতি;
    পূর্বনির্ধারিত:
        মান = 0;
        hex(ptr, 16, 3, __func__);
        যদি (error)
            *error = 1;
        বিরতি;
    }

    যদি (dst)
        *dst = মান;
    dprintf(5, "%s: 0x%llx\n", __func__, মান);
    ফেরত offset;
}

স্থির পূর্ণসংখ্যা parse_pkglength(u8 *ptr, পূর্ণসংখ্যা *pkglength)
{
    পূর্ণসংখ্যা offset = 2;

    *pkglength = 0;
    নির্বাচন (ptr[0] >> 6) {
    ক্ষেত্র 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    ক্ষেত্র 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    ক্ষেত্র 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        ফেরত offset;
    ক্ষেত্র 0:
    পূর্বনির্ধারিত:
        *pkglength |= ptr[0] & 0x3f;
        ফেরত 1;
    }
}

স্থির পূর্ণসংখ্যা parse_pkg_common(কাঠামো parse_state *s,
                            u8 *ptr, ধ্রুবক অক্ষর *item, পূর্ণসংখ্যা *pkglength)
{
    পূর্ণসংখ্যা offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    ফেরত offset;
}

স্থির পূর্ণসংখ্যা parse_pkg_scope(কাঠামো parse_state *s,
                           u8 *ptr)
{
    পূর্ণসংখ্যা offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    ফেরত pkglength;
}

স্থির পূর্ণসংখ্যা parse_pkg_device(কাঠামো parse_state *s,
                            u8 *ptr)
{
    পূর্ণসংখ্যা offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(আকার(কাঠামো acpi_device));
    যদি (!s->dev) {
        warn_noalloc();
        s->error = 1;
        ফেরত pkglength;
    }

    memset(s->dev, 0, আকার(কাঠামো acpi_device));
    hlist_add_শীর্ষ(&s->dev->গ্রন্থি, &acpi_devices);
    strtcpy(s->dev->name, s->name, আকার(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = ফাঁকা;

    ফেরত pkglength;
}

স্থির পূর্ণসংখ্যা parse_pkg_বাফার(কাঠামো parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    পূর্ণসংখ্যা pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    যদি (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_তথ্য = ptr + offset;
        s->dev->crs_আকার = blen;
    }
    ফেরত pkglength;
}

স্থির পূর্ণসংখ্যা parse_pkg_skip(কাঠামো parse_state *s,
                          u8 *ptr, পূর্ণসংখ্যা op, পূর্ণসংখ্যা name)
{
    পূর্ণসংখ্যা pkglength, offset;
    অক্ষর item[8];

    snprintf(item, আকার(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    যদি (name) {
        parse_namestring(s, ptr + offset, item);
    } অন্যথা {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    ফেরত pkglength;
}

স্থির পূর্ণসংখ্যা parse_termobj(কাঠামো parse_state *s,
                         u8 *ptr)
{
    পূর্ণসংখ্যা offset = 1;

    যদি (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        ফেরত offset;
    }

    s->depth++;
    নির্বাচন (ptr[0]) {
    ক্ষেত্র 0x00: /* zero */
        বিরতি;
    ক্ষেত্র 0x01: /* one */
        বিরতি;
    ক্ষেত্র 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        বিরতি;
    ক্ষেত্র 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        যদি (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        যদি (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        বিরতি;
    ক্ষেত্র 0x0a: /* byte prefix */
        offset++;
        বিরতি;
    ক্ষেত্র 0x0b: /* word prefix */
        offset += 2;
        বিরতি;
    ক্ষেত্র 0x0c: /* dword prefix */
        offset += 4;
        বিরতি;
    ক্ষেত্র 0x0d: /* string prefix */
        যখন (ptr[offset])
            offset++;
        offset++;
        বিরতি;
    ক্ষেত্র 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        বিরতি;
    ক্ষেত্র 0x11: /* buffer op */
        offset += parse_pkg_বাফার(s, ptr + offset);
        বিরতি;
    ক্ষেত্র 0x12: /* package op */
    ক্ষেত্র 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        বিরতি;
    ক্ষেত্র 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        যদি (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        বিরতি;
    ক্ষেত্র 0x5b: /* ext op prefix */
        offset++;
        নির্বাচন (ptr[1]) {
        ক্ষেত্র 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            বিরতি;
        ক্ষেত্র 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, ফাঁকা);
            offset += parse_termarg_int(ptr + offset, &s->error, ফাঁকা);
            বিরতি;
        ক্ষেত্র 0x81: /* field op */
        ক্ষেত্র 0x83: /* processor op */
        ক্ষেত্র 0x84: /* power resource op */
        ক্ষেত্র 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            বিরতি;
        ক্ষেত্র 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            বিরতি;
        পূর্বনির্ধারিত:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            বিরতি;
        }
        বিরতি;
    পূর্বনির্ধারিত:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        বিরতি;
    }
    s->depth--;

    ফেরত offset;
}

স্থির শূন্য parse_termlist(কাঠামো parse_state *s,
                           u8 *ptr, পূর্ণসংখ্যা offset, পূর্ণসংখ্যা pkglength)
{
    জন্য (;;) {
        offset += parse_termobj(s, ptr + offset);
        যদি (offset == pkglength)
            ফেরত;
        যদি (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            ফেরত;
        }
        যদি (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            ফেরত;
        }
    }
}

স্থির কাঠামো acpi_device *acpi_dsdt_find(কাঠামো acpi_device *পূর্ব,
                                          ধ্রুবক u8 *aml1, পূর্ণসংখ্যা size1,
                                          ধ্রুবক u8 *aml2, পূর্ণসংখ্যা size2)
{
    কাঠামো acpi_device *dev;
    কাঠামো hlist_গ্রন্থি *গ্রন্থি;

    যদি (!পূর্ব)
        গ্রন্থি = acpi_devices.first;
    অন্যথা
        গ্রন্থি = পূর্ব->গ্রন্থি.পরবর্তী;

    জন্য (; গ্রন্থি != ফাঁকা; গ্রন্থি = dev->গ্রন্থি.পরবর্তী) {
        dev = container_of(গ্রন্থি, কাঠামো acpi_device, গ্রন্থি);
        যদি (!aml1 && !aml2)
            ফেরত dev;
        যদি (!dev->hid_aml)
            চালু;
        যদি (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            ফেরত dev;
        যদি (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            ফেরত dev;
    }
    ফেরত ফাঁকা;
}

স্থির পূর্ণসংখ্যা acpi_dsdt_present(কাঠামো acpi_device *dev)
{
    যদি (!dev)
        ফেরত 0; /* no */
    যদি (!dev->sta_aml)
        ফেরত 1; /* yes */
    যদি (dev->sta_aml[0] == 0x14)
        ফেরত -1; /* unknown (can't evaluate method) */
    যদি (dev->sta_aml[0] == 0x08) {
        u64 মান = 0;
        parse_termarg_int(dev->sta_aml + 5, ফাঁকা, &মান);
        যদি (মান == 0)
            ফেরত 0; /* no */
        অন্যথা
            ফেরত 1; /* yes */
    }
    ফেরত -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

কাঠামো acpi_device *acpi_dsdt_find_string(কাঠামো acpi_device *পূর্ব,
                                          ধ্রুবক অক্ষর *hid)
{
    যদি (!CONFIG_ACPI_PARSE)
        ফেরত ফাঁকা;

    u8 aml[10];
    পূর্ণসংখ্যা len = snprintf((অক্ষর*)aml, আকার(aml), "\x0d%s", hid);
    ফেরত acpi_dsdt_find(পূর্ব, aml, len, ফাঁকা, 0);
}

কাঠামো acpi_device *acpi_dsdt_find_eisaid(কাঠামো acpi_device *পূর্ব, u16 eisaid)
{
    যদি (!CONFIG_ACPI_PARSE)
        ফেরত ফাঁকা;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    পূর্ণসংখ্যা len2 = snprintf((অক্ষর*)aml2, আকার(aml2), "\x0dPNP%04X", eisaid);
    ফেরত acpi_dsdt_find(পূর্ব, aml1, 5, aml2, len2);
}

অক্ষর *acpi_dsdt_name(কাঠামো acpi_device *dev)
{
    যদি (!CONFIG_ACPI_PARSE || !dev)
        ফেরত ফাঁকা;
    ফেরত dev->name;
}

পূর্ণসংখ্যা acpi_dsdt_find_io(কাঠামো acpi_device *dev, u64 *min, u64 *max)
{
    যদি (!CONFIG_ACPI_PARSE || !dev || !dev->crs_তথ্য)
        ফেরত -1;
    ফেরত find_resource(dev->crs_তথ্য, dev->crs_আকার,
                         1 /* I/O */, min, max);
}

পূর্ণসংখ্যা acpi_dsdt_find_mem(কাঠামো acpi_device *dev, u64 *min, u64 *max)
{
    যদি (!CONFIG_ACPI_PARSE || !dev || !dev->crs_তথ্য)
        ফেরত -1;
    ফেরত find_resource(dev->crs_তথ্য, dev->crs_আকার,
                         0 /* mem */, min, max);
}

পূর্ণসংখ্যা acpi_dsdt_find_irq(কাঠামো acpi_device *dev, u64 *irq)
{
    u64 max;
    যদি (!CONFIG_ACPI_PARSE || !dev || !dev->crs_তথ্য)
        ফেরত -1;
    ফেরত find_resource(dev->crs_তথ্য, dev->crs_আকার,
                         3 /* irq */, irq, &max);
}

পূর্ণসংখ্যা acpi_dsdt_present_eisaid(u16 eisaid)
{
    যদি (!CONFIG_ACPI_PARSE)
        ফেরত -1; /* unknown */
    যদি (hlist_empty(&acpi_devices))
        ফেরত -1; /* unknown (no dsdt table) */

    কাঠামো acpi_device *dev = acpi_dsdt_find_eisaid(ফাঁকা, eisaid);
    ফেরত acpi_dsdt_present(dev);
}

শূন্য acpi_dsdt_parse(শূন্য)
{
    যদি (!CONFIG_ACPI_PARSE)
        ফেরত;

    কাঠামো fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    যদি (!fadt)
        ফেরত;
    u8 *dsdt = (শূন্য*)(fadt->dsdt);
    যদি (!dsdt)
        ফেরত;

    u32 দৈর্ঘ্য = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, দৈর্ঘ্য);

    কাঠামো parse_state s;
    memset(&s, 0, আকার(s));
    parse_termlist(&s, dsdt, offset, দৈর্ঘ্য);

    যদি (!parse_dumpdevs)
        ফেরত;

    কাঠামো acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    জন্য (dev = acpi_dsdt_find(ফাঁকা, ফাঁকা, 0, ফাঁকা, 0);
         dev != ফাঁকা;
         dev = acpi_dsdt_find(dev, ফাঁকা, 0, ফাঁকা, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        যদি (dev->hid_aml)
            dprintf(1, ", hid");
        যদি (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        যদি (dev->crs_তথ্য)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        যদি (dev->crs_তথ্য)
            print_resources("        ", dev->crs_তথ্য, dev->crs_আকার);
    }
}
