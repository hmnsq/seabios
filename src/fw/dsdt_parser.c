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

সংরচনা acpi_device {
    সংরচনা hlist_গ্রন্থি গ্রন্থি;
    বর্ণ name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    পূর্ণাঙ্ক crs_size;
};
স্থির সংরচনা hlist_head acpi_devices VARVERIFY32INIT;
স্থির ধ্রুব পূর্ণাঙ্ক parse_dumpdevs = 0;

সংরচনা parse_state {
    বর্ণ name[32];
    সংরচনা acpi_device *dev;
    পূর্ণাঙ্ক error;
    পূর্ণাঙ্ক depth;
};

স্থির শূন্য parse_termlist(সংরচনা parse_state *s,
                           u8 *ptr, পূর্ণাঙ্ক offset, পূর্ণাঙ্ক pkglength);

স্থির শূন্য hex(ধ্রুব u8 *ptr, পূর্ণাঙ্ক গুনতি, পূর্ণাঙ্ক lvl, ধ্রুব বর্ণ *item)
{
    পূর্ণাঙ্ক l = 0, i;

    কুরু {
        dprintf(lvl, "%s: %04x:  ", item, l);
        প্রতি (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        প্রতি (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } যাবত্ (l < গুনতি);
}

স্থির u64 parse_resource_int(u8 *ptr, পূর্ণাঙ্ক গুনতি)
{
    u64 মান = 0;
    পূর্ণাঙ্ক index = 0;

    প্রতি (index = 0; index < গুনতি; index++)
        মান |= (u64)ptr[index] << (index * 8);
    প্রত্যাগম মান;
}

স্থির পূর্ণাঙ্ক parse_resource_bit(u8 *ptr, পূর্ণাঙ্ক গুনতি)
{
    পূর্ণাঙ্ক bit;

    প্রতি (bit = 0; bit < গুনতি*8; bit++)
        যদি (ptr[bit/8] & (1 << (bit%8)))
            প্রত্যাগম bit;
    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক parse_resource(u8 *ptr, পূর্ণাঙ্ক length, পূর্ণাঙ্ক *type, u64 *min, u64 *max)
{
    পূর্ণাঙ্ক rname, rsize;
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
        বিকল্প (rname) {
        পক্ষ 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            ভঙ্গ;
        পক্ষ 0x0f: /* end marker */
            প্রত্যাগম 0;
        পক্ষ 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            যদি (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            ভঙ্গ;
        পক্ষ 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            ভঙ্গ;
        স্বভাব:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            ভঙ্গ;
        }
    } অন্যথা {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        বিকল্প (rname) {
        পক্ষ 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            ভঙ্গ;
        পক্ষ 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            ভঙ্গ;
        পক্ষ 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            ভঙ্গ;
        পক্ষ 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            ভঙ্গ;
        পক্ষ 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            ভঙ্গ;
        স্বভাব:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            ভঙ্গ;
        }
    }
    প্রত্যাগম rsize;
}

স্থির পূর্ণাঙ্ক find_resource(u8 *ptr, পূর্ণাঙ্ক len, পূর্ণাঙ্ক kind, u64 *min, u64 *max)
{
    পূর্ণাঙ্ক type, size, offset = 0;

    কুরু {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        যদি (kind == type)
            প্রত্যাগম 0;
        offset += size;
    } যাবত্ (size > 0 && offset < len);
    প্রত্যাগম -1;
}

স্থির পূর্ণাঙ্ক print_resources(ধ্রুব বর্ণ *prefix, u8 *ptr, পূর্ণাঙ্ক len)
{
    স্থির ধ্রুব বর্ণ *typename[] = { "mem", "i/o", "bus" };
    পূর্ণাঙ্ক type, size, offset = 0;
    u64 min, max;

    কুরু {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        বিকল্প (type) {
        পক্ষ 0:
        পক্ষ 1:
        পক্ষ 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            ভঙ্গ;
        পক্ষ 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            ভঙ্গ;
        }
        offset += size;
    } যাবত্ (size > 0 && offset < len);
    প্রত্যাগম -1;
}

স্থির পূর্ণাঙ্ক parse_nameseg(u8 *ptr, বর্ণ **dst)
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
    প্রত্যাগম 4;
}

স্থির পূর্ণাঙ্ক parse_namestring(সংরচনা parse_state *s,
                            u8 *ptr, ধ্রুব বর্ণ *item)
{
    বর্ণ *dst = s->name;
    পূর্ণাঙ্ক offset = 0;
    পূর্ণাঙ্ক i, গুনতি;

    প্রতি (;;) {
        বিকল্প (ptr[offset]) {
        পক্ষ 0: /* null name */
            offset++;
            *(dst++) = 0;
            ভঙ্গ;
        পক্ষ 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            ভঙ্গ;
        পক্ষ 0x2f:
            offset++;
            গুনতি = ptr[offset];
            offset++;
            প্রতি (i = 0; i < গুনতি; i++) {
                যদি (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            ভঙ্গ;
        পক্ষ '\\':
            *(dst++) = '\\';
            offset++;
            অনুবর্ত;
        পক্ষ '^':
            *(dst++) = '^';
            offset++;
            অনুবর্ত;
        পক্ষ 'A' ... 'Z':
        পক্ষ '_':
            offset += parse_nameseg(ptr, &dst);
            ভঙ্গ;
        স্বভাব:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ভঙ্গ;
        }
        ভঙ্গ;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    প্রত্যাগম offset;
}

স্থির পূর্ণাঙ্ক parse_termarg_int(u8 *ptr, পূর্ণাঙ্ক *error, u64 *dst)
{
    u64 মান;
    পূর্ণাঙ্ক offset = 1;

    বিকল্প (ptr[0]) {
    পক্ষ 0x00: /* zero */
        মান = 0;
        ভঙ্গ;
    পক্ষ 0x01: /* one */
        মান = 1;
        ভঙ্গ;
    পক্ষ 0x0a: /* byte prefix */
        মান = ptr[1];
        offset++;
        ভঙ্গ;
    পক্ষ 0x0b: /* word prefix */
        মান = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        ভঙ্গ;
    পক্ষ 0x0c: /* dword prefix */
        মান = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        ভঙ্গ;
    স্বভাব:
        মান = 0;
        hex(ptr, 16, 3, __func__);
        যদি (error)
            *error = 1;
        ভঙ্গ;
    }

    যদি (dst)
        *dst = মান;
    dprintf(5, "%s: 0x%llx\n", __func__, মান);
    প্রত্যাগম offset;
}

স্থির পূর্ণাঙ্ক parse_pkglength(u8 *ptr, পূর্ণাঙ্ক *pkglength)
{
    পূর্ণাঙ্ক offset = 2;

    *pkglength = 0;
    বিকল্প (ptr[0] >> 6) {
    পক্ষ 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    পক্ষ 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    পক্ষ 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        প্রত্যাগম offset;
    পক্ষ 0:
    স্বভাব:
        *pkglength |= ptr[0] & 0x3f;
        প্রত্যাগম 1;
    }
}

স্থির পূর্ণাঙ্ক parse_pkg_common(সংরচনা parse_state *s,
                            u8 *ptr, ধ্রুব বর্ণ *item, পূর্ণাঙ্ক *pkglength)
{
    পূর্ণাঙ্ক offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    প্রত্যাগম offset;
}

স্থির পূর্ণাঙ্ক parse_pkg_scope(সংরচনা parse_state *s,
                           u8 *ptr)
{
    পূর্ণাঙ্ক offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    প্রত্যাগম pkglength;
}

স্থির পূর্ণাঙ্ক parse_pkg_device(সংরচনা parse_state *s,
                            u8 *ptr)
{
    পূর্ণাঙ্ক offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(পরিমাণ(সংরচনা acpi_device));
    যদি (!s->dev) {
        warn_noalloc();
        s->error = 1;
        প্রত্যাগম pkglength;
    }

    memset(s->dev, 0, পরিমাণ(সংরচনা acpi_device));
    hlist_add_head(&s->dev->গ্রন্থি, &acpi_devices);
    strtcpy(s->dev->name, s->name, পরিমাণ(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = রিক্ত;

    প্রত্যাগম pkglength;
}

স্থির পূর্ণাঙ্ক parse_pkg_buffer(সংরচনা parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    পূর্ণাঙ্ক pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    যদি (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    প্রত্যাগম pkglength;
}

স্থির পূর্ণাঙ্ক parse_pkg_skip(সংরচনা parse_state *s,
                          u8 *ptr, পূর্ণাঙ্ক op, পূর্ণাঙ্ক name)
{
    পূর্ণাঙ্ক pkglength, offset;
    বর্ণ item[8];

    snprintf(item, পরিমাণ(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    যদি (name) {
        parse_namestring(s, ptr + offset, item);
    } অন্যথা {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    প্রত্যাগম pkglength;
}

স্থির পূর্ণাঙ্ক parse_termobj(সংরচনা parse_state *s,
                         u8 *ptr)
{
    পূর্ণাঙ্ক offset = 1;

    যদি (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        প্রত্যাগম offset;
    }

    s->depth++;
    বিকল্প (ptr[0]) {
    পক্ষ 0x00: /* zero */
        ভঙ্গ;
    পক্ষ 0x01: /* one */
        ভঙ্গ;
    পক্ষ 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        ভঙ্গ;
    পক্ষ 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        যদি (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        যদি (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ভঙ্গ;
    পক্ষ 0x0a: /* byte prefix */
        offset++;
        ভঙ্গ;
    পক্ষ 0x0b: /* word prefix */
        offset += 2;
        ভঙ্গ;
    পক্ষ 0x0c: /* dword prefix */
        offset += 4;
        ভঙ্গ;
    পক্ষ 0x0d: /* string prefix */
        যাবত্ (ptr[offset])
            offset++;
        offset++;
        ভঙ্গ;
    পক্ষ 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        ভঙ্গ;
    পক্ষ 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        ভঙ্গ;
    পক্ষ 0x12: /* package op */
    পক্ষ 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        ভঙ্গ;
    পক্ষ 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        যদি (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ভঙ্গ;
    পক্ষ 0x5b: /* ext op prefix */
        offset++;
        বিকল্প (ptr[1]) {
        পক্ষ 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            ভঙ্গ;
        পক্ষ 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, রিক্ত);
            offset += parse_termarg_int(ptr + offset, &s->error, রিক্ত);
            ভঙ্গ;
        পক্ষ 0x81: /* field op */
        পক্ষ 0x83: /* processor op */
        পক্ষ 0x84: /* power resource op */
        পক্ষ 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            ভঙ্গ;
        পক্ষ 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            ভঙ্গ;
        স্বভাব:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ভঙ্গ;
        }
        ভঙ্গ;
    স্বভাব:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        ভঙ্গ;
    }
    s->depth--;

    প্রত্যাগম offset;
}

স্থির শূন্য parse_termlist(সংরচনা parse_state *s,
                           u8 *ptr, পূর্ণাঙ্ক offset, পূর্ণাঙ্ক pkglength)
{
    প্রতি (;;) {
        offset += parse_termobj(s, ptr + offset);
        যদি (offset == pkglength)
            প্রত্যাগম;
        যদি (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            প্রত্যাগম;
        }
        যদি (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            প্রত্যাগম;
        }
    }
}

স্থির সংরচনা acpi_device *acpi_dsdt_find(সংরচনা acpi_device *prev,
                                          ধ্রুব u8 *aml1, পূর্ণাঙ্ক size1,
                                          ধ্রুব u8 *aml2, পূর্ণাঙ্ক size2)
{
    সংরচনা acpi_device *dev;
    সংরচনা hlist_গ্রন্থি *গ্রন্থি;

    যদি (!prev)
        গ্রন্থি = acpi_devices.first;
    অন্যথা
        গ্রন্থি = prev->গ্রন্থি.next;

    প্রতি (; গ্রন্থি != রিক্ত; গ্রন্থি = dev->গ্রন্থি.next) {
        dev = container_of(গ্রন্থি, সংরচনা acpi_device, গ্রন্থি);
        যদি (!aml1 && !aml2)
            প্রত্যাগম dev;
        যদি (!dev->hid_aml)
            অনুবর্ত;
        যদি (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            প্রত্যাগম dev;
        যদি (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            প্রত্যাগম dev;
    }
    প্রত্যাগম রিক্ত;
}

স্থির পূর্ণাঙ্ক acpi_dsdt_present(সংরচনা acpi_device *dev)
{
    যদি (!dev)
        প্রত্যাগম 0; /* no */
    যদি (!dev->sta_aml)
        প্রত্যাগম 1; /* yes */
    যদি (dev->sta_aml[0] == 0x14)
        প্রত্যাগম -1; /* unknown (can't evaluate method) */
    যদি (dev->sta_aml[0] == 0x08) {
        u64 মান = 0;
        parse_termarg_int(dev->sta_aml + 5, রিক্ত, &মান);
        যদি (মান == 0)
            প্রত্যাগম 0; /* no */
        অন্যথা
            প্রত্যাগম 1; /* yes */
    }
    প্রত্যাগম -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

সংরচনা acpi_device *acpi_dsdt_find_string(সংরচনা acpi_device *prev,
                                          ধ্রুব বর্ণ *hid)
{
    যদি (!CONFIG_ACPI_PARSE)
        প্রত্যাগম রিক্ত;

    u8 aml[10];
    পূর্ণাঙ্ক len = snprintf((বর্ণ*)aml, পরিমাণ(aml), "\x0d%s", hid);
    প্রত্যাগম acpi_dsdt_find(prev, aml, len, রিক্ত, 0);
}

সংরচনা acpi_device *acpi_dsdt_find_eisaid(সংরচনা acpi_device *prev, u16 eisaid)
{
    যদি (!CONFIG_ACPI_PARSE)
        প্রত্যাগম রিক্ত;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    পূর্ণাঙ্ক len2 = snprintf((বর্ণ*)aml2, পরিমাণ(aml2), "\x0dPNP%04X", eisaid);
    প্রত্যাগম acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

বর্ণ *acpi_dsdt_name(সংরচনা acpi_device *dev)
{
    যদি (!CONFIG_ACPI_PARSE || !dev)
        প্রত্যাগম রিক্ত;
    প্রত্যাগম dev->name;
}

পূর্ণাঙ্ক acpi_dsdt_find_io(সংরচনা acpi_device *dev, u64 *min, u64 *max)
{
    যদি (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        প্রত্যাগম -1;
    প্রত্যাগম find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

পূর্ণাঙ্ক acpi_dsdt_find_mem(সংরচনা acpi_device *dev, u64 *min, u64 *max)
{
    যদি (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        প্রত্যাগম -1;
    প্রত্যাগম find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

পূর্ণাঙ্ক acpi_dsdt_find_irq(সংরচনা acpi_device *dev, u64 *irq)
{
    u64 max;
    যদি (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        প্রত্যাগম -1;
    প্রত্যাগম find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

পূর্ণাঙ্ক acpi_dsdt_present_eisaid(u16 eisaid)
{
    যদি (!CONFIG_ACPI_PARSE)
        প্রত্যাগম -1; /* unknown */
    যদি (hlist_empty(&acpi_devices))
        প্রত্যাগম -1; /* unknown (no dsdt table) */

    সংরচনা acpi_device *dev = acpi_dsdt_find_eisaid(রিক্ত, eisaid);
    প্রত্যাগম acpi_dsdt_present(dev);
}

শূন্য acpi_dsdt_parse(শূন্য)
{
    যদি (!CONFIG_ACPI_PARSE)
        প্রত্যাগম;

    সংরচনা fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    যদি (!fadt)
        প্রত্যাগম;
    u8 *dsdt = (শূন্য*)(fadt->dsdt);
    যদি (!dsdt)
        প্রত্যাগম;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    সংরচনা parse_state s;
    memset(&s, 0, পরিমাণ(s));
    parse_termlist(&s, dsdt, offset, length);

    যদি (!parse_dumpdevs)
        প্রত্যাগম;

    সংরচনা acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    প্রতি (dev = acpi_dsdt_find(রিক্ত, রিক্ত, 0, রিক্ত, 0);
         dev != রিক্ত;
         dev = acpi_dsdt_find(dev, রিক্ত, 0, রিক্ত, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        যদি (dev->hid_aml)
            dprintf(1, ", hid");
        যদি (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        যদি (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        যদি (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
