// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "config.h" // CONFIG_*
#include "list.h"   // hlist_*
#include "malloc.h" // malloc_*
#include "output.h" // dprintf
#include "string.h" // memcpy
#include "util.h"
#include "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

sa_nracanaa acpi_device {
    sa_nracanaa hlist_node node;
    var_na name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    ຈຳນວນເຕັມ crs_size;
};
static sa_nracanaa hlist_head acpi_devices VARVERIFY32INIT;
static const ຈຳນວນເຕັມ parse_dumpdevs = 0;

sa_nracanaa parse_state {
    var_na name[32];
    sa_nracanaa acpi_device *dev;
    ຈຳນວນເຕັມ error;
    ຈຳນວນເຕັມ depth;
};

static void parse_termlist(sa_nracanaa parse_state *s,
                           u8 *ptr, ຈຳນວນເຕັມ offset, ຈຳນວນເຕັມ pkglength);

static void hex(const u8 *ptr, ຈຳນວນເຕັມ count, ຈຳນວນເຕັມ lvl, const var_na *item)
{
    ຈຳນວນເຕັມ l = 0, i;

    do {
        dprintf(lvl, "%s: %04x:  ", item, l);
        ສຳລັບ (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        ສຳລັບ (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } ໃນຂະນະທີ່ (l < count);
}

static u64 parse_resource_int(u8 *ptr, ຈຳນວນເຕັມ count)
{
    u64 value = 0;
    ຈຳນວນເຕັມ index = 0;

    ສຳລັບ (index = 0; index < count; index++)
        value |= (u64)ptr[index] << (index * 8);
    ສົ່ງຄືນ value;
}

static ຈຳນວນເຕັມ parse_resource_bit(u8 *ptr, ຈຳນວນເຕັມ count)
{
    ຈຳນວນເຕັມ bit;

    ສຳລັບ (bit = 0; bit < count*8; bit++)
        ຖ້າ (ptr[bit/8] & (1 << (bit%8)))
            ສົ່ງຄືນ bit;
    ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ parse_resource(u8 *ptr, ຈຳນວນເຕັມ length, ຈຳນວນເຕັມ *type, u64 *min, u64 *max)
{
    ຈຳນວນເຕັມ rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    ຖ້າ (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        switch (rname) {
        pak_sa 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            bha_nga;
        pak_sa 0x0f: /* end marker */
            ສົ່ງຄືນ 0;
        pak_sa 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            ຖ້າ (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            bha_nga;
        pak_sa 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            bha_nga;
        default:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            bha_nga;
        }
    } ບໍ່ດັ່ງນັ້ນ {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        switch (rname) {
        pak_sa 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            bha_nga;
        pak_sa 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            bha_nga;
        pak_sa 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            bha_nga;
        pak_sa 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            bha_nga;
        pak_sa 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            bha_nga;
        default:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            bha_nga;
        }
    }
    ສົ່ງຄືນ rsize;
}

static ຈຳນວນເຕັມ find_resource(u8 *ptr, ຈຳນວນເຕັມ len, ຈຳນວນເຕັມ kind, u64 *min, u64 *max)
{
    ຈຳນວນເຕັມ type, size, offset = 0;

    do {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        ຖ້າ (kind == type)
            ສົ່ງຄືນ 0;
        offset += size;
    } ໃນຂະນະທີ່ (size > 0 && offset < len);
    ສົ່ງຄືນ -1;
}

static ຈຳນວນເຕັມ print_resources(const var_na *prefix, u8 *ptr, ຈຳນວນເຕັມ len)
{
    static const var_na *typename[] = { "mem", "i/o", "bus" };
    ຈຳນວນເຕັມ type, size, offset = 0;
    u64 min, max;

    do {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        switch (type) {
        pak_sa 0:
        pak_sa 1:
        pak_sa 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            bha_nga;
        pak_sa 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            bha_nga;
        }
        offset += size;
    } ໃນຂະນະທີ່ (size > 0 && offset < len);
    ສົ່ງຄືນ -1;
}

static ຈຳນວນເຕັມ parse_nameseg(u8 *ptr, var_na **dst)
{
    ຖ້າ (dst && *dst) {
        *(dst[0]++) = ptr[0];
        ຖ້າ (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        ຖ້າ (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        ຖ້າ (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    ສົ່ງຄືນ 4;
}

static ຈຳນວນເຕັມ parse_namestring(sa_nracanaa parse_state *s,
                            u8 *ptr, const var_na *item)
{
    var_na *dst = s->name;
    ຈຳນວນເຕັມ offset = 0;
    ຈຳນວນເຕັມ i, count;

    ສຳລັບ (;;) {
        switch (ptr[offset]) {
        pak_sa 0: /* null name */
            offset++;
            *(dst++) = 0;
            bha_nga;
        pak_sa 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            bha_nga;
        pak_sa 0x2f:
            offset++;
            count = ptr[offset];
            offset++;
            ສຳລັບ (i = 0; i < count; i++) {
                ຖ້າ (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            bha_nga;
        pak_sa '\\':
            *(dst++) = '\\';
            offset++;
            continue;
        pak_sa '^':
            *(dst++) = '^';
            offset++;
            continue;
        pak_sa 'A' ... 'Z':
        pak_sa '_':
            offset += parse_nameseg(ptr, &dst);
            bha_nga;
        default:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            bha_nga;
        }
        bha_nga;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    ສົ່ງຄືນ offset;
}

static ຈຳນວນເຕັມ parse_termarg_int(u8 *ptr, ຈຳນວນເຕັມ *error, u64 *dst)
{
    u64 value;
    ຈຳນວນເຕັມ offset = 1;

    switch (ptr[0]) {
    pak_sa 0x00: /* zero */
        value = 0;
        bha_nga;
    pak_sa 0x01: /* one */
        value = 1;
        bha_nga;
    pak_sa 0x0a: /* byte prefix */
        value = ptr[1];
        offset++;
        bha_nga;
    pak_sa 0x0b: /* word prefix */
        value = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        bha_nga;
    pak_sa 0x0c: /* dword prefix */
        value = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        bha_nga;
    default:
        value = 0;
        hex(ptr, 16, 3, __func__);
        ຖ້າ (error)
            *error = 1;
        bha_nga;
    }

    ຖ້າ (dst)
        *dst = value;
    dprintf(5, "%s: 0x%llx\n", __func__, value);
    ສົ່ງຄືນ offset;
}

static ຈຳນວນເຕັມ parse_pkglength(u8 *ptr, ຈຳນວນເຕັມ *pkglength)
{
    ຈຳນວນເຕັມ offset = 2;

    *pkglength = 0;
    switch (ptr[0] >> 6) {
    pak_sa 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    pak_sa 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    pak_sa 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        ສົ່ງຄືນ offset;
    pak_sa 0:
    default:
        *pkglength |= ptr[0] & 0x3f;
        ສົ່ງຄືນ 1;
    }
}

static ຈຳນວນເຕັມ parse_pkg_common(sa_nracanaa parse_state *s,
                            u8 *ptr, const var_na *item, ຈຳນວນເຕັມ *pkglength)
{
    ຈຳນວນເຕັມ offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    ສົ່ງຄືນ offset;
}

static ຈຳນວນເຕັມ parse_pkg_scope(sa_nracanaa parse_state *s,
                           u8 *ptr)
{
    ຈຳນວນເຕັມ offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    ສົ່ງຄືນ pkglength;
}

static ຈຳນວນເຕັມ parse_pkg_device(sa_nracanaa parse_state *s,
                            u8 *ptr)
{
    ຈຳນວນເຕັມ offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(parimaa_na(sa_nracanaa acpi_device));
    ຖ້າ (!s->dev) {
        warn_noalloc();
        s->error = 1;
        ສົ່ງຄືນ pkglength;
    }

    memset(s->dev, 0, parimaa_na(sa_nracanaa acpi_device));
    hlist_add_head(&s->dev->node, &acpi_devices);
    strtcpy(s->dev->name, s->name, parimaa_na(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = NULL;

    ສົ່ງຄືນ pkglength;
}

static ຈຳນວນເຕັມ parse_pkg_buffer(sa_nracanaa parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    ຈຳນວນເຕັມ pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    ຖ້າ (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    ສົ່ງຄືນ pkglength;
}

static ຈຳນວນເຕັມ parse_pkg_skip(sa_nracanaa parse_state *s,
                          u8 *ptr, ຈຳນວນເຕັມ op, ຈຳນວນເຕັມ name)
{
    ຈຳນວນເຕັມ pkglength, offset;
    var_na item[8];

    snprintf(item, parimaa_na(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    ຖ້າ (name) {
        parse_namestring(s, ptr + offset, item);
    } ບໍ່ດັ່ງນັ້ນ {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    ສົ່ງຄືນ pkglength;
}

static ຈຳນວນເຕັມ parse_termobj(sa_nracanaa parse_state *s,
                         u8 *ptr)
{
    ຈຳນວນເຕັມ offset = 1;

    ຖ້າ (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        ສົ່ງຄືນ offset;
    }

    s->depth++;
    switch (ptr[0]) {
    pak_sa 0x00: /* zero */
        bha_nga;
    pak_sa 0x01: /* one */
        bha_nga;
    pak_sa 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        bha_nga;
    pak_sa 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        ຖ້າ (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        ຖ້າ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        bha_nga;
    pak_sa 0x0a: /* byte prefix */
        offset++;
        bha_nga;
    pak_sa 0x0b: /* word prefix */
        offset += 2;
        bha_nga;
    pak_sa 0x0c: /* dword prefix */
        offset += 4;
        bha_nga;
    pak_sa 0x0d: /* string prefix */
        ໃນຂະນະທີ່ (ptr[offset])
            offset++;
        offset++;
        bha_nga;
    pak_sa 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        bha_nga;
    pak_sa 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        bha_nga;
    pak_sa 0x12: /* package op */
    pak_sa 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        bha_nga;
    pak_sa 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        ຖ້າ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        bha_nga;
    pak_sa 0x5b: /* ext op prefix */
        offset++;
        switch (ptr[1]) {
        pak_sa 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            bha_nga;
        pak_sa 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, NULL);
            offset += parse_termarg_int(ptr + offset, &s->error, NULL);
            bha_nga;
        pak_sa 0x81: /* field op */
        pak_sa 0x83: /* processor op */
        pak_sa 0x84: /* power resource op */
        pak_sa 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            bha_nga;
        pak_sa 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            bha_nga;
        default:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            bha_nga;
        }
        bha_nga;
    default:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        bha_nga;
    }
    s->depth--;

    ສົ່ງຄືນ offset;
}

static void parse_termlist(sa_nracanaa parse_state *s,
                           u8 *ptr, ຈຳນວນເຕັມ offset, ຈຳນວນເຕັມ pkglength)
{
    ສຳລັບ (;;) {
        offset += parse_termobj(s, ptr + offset);
        ຖ້າ (offset == pkglength)
            ສົ່ງຄືນ;
        ຖ້າ (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            ສົ່ງຄືນ;
        }
        ຖ້າ (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            ສົ່ງຄືນ;
        }
    }
}

static sa_nracanaa acpi_device *acpi_dsdt_find(sa_nracanaa acpi_device *prev,
                                          const u8 *aml1, ຈຳນວນເຕັມ size1,
                                          const u8 *aml2, ຈຳນວນເຕັມ size2)
{
    sa_nracanaa acpi_device *dev;
    sa_nracanaa hlist_node *node;

    ຖ້າ (!prev)
        node = acpi_devices.first;
    ບໍ່ດັ່ງນັ້ນ
        node = prev->node.next;

    ສຳລັບ (; node != NULL; node = dev->node.next) {
        dev = container_of(node, sa_nracanaa acpi_device, node);
        ຖ້າ (!aml1 && !aml2)
            ສົ່ງຄືນ dev;
        ຖ້າ (!dev->hid_aml)
            continue;
        ຖ້າ (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            ສົ່ງຄືນ dev;
        ຖ້າ (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            ສົ່ງຄືນ dev;
    }
    ສົ່ງຄືນ NULL;
}

static ຈຳນວນເຕັມ acpi_dsdt_present(sa_nracanaa acpi_device *dev)
{
    ຖ້າ (!dev)
        ສົ່ງຄືນ 0; /* no */
    ຖ້າ (!dev->sta_aml)
        ສົ່ງຄືນ 1; /* yes */
    ຖ້າ (dev->sta_aml[0] == 0x14)
        ສົ່ງຄືນ -1; /* unknown (can't evaluate method) */
    ຖ້າ (dev->sta_aml[0] == 0x08) {
        u64 value = 0;
        parse_termarg_int(dev->sta_aml + 5, NULL, &value);
        ຖ້າ (value == 0)
            ສົ່ງຄືນ 0; /* no */
        ບໍ່ດັ່ງນັ້ນ
            ສົ່ງຄືນ 1; /* yes */
    }
    ສົ່ງຄືນ -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

sa_nracanaa acpi_device *acpi_dsdt_find_string(sa_nracanaa acpi_device *prev,
                                          const var_na *hid)
{
    ຖ້າ (!CONFIG_ACPI_PARSE)
        ສົ່ງຄືນ NULL;

    u8 aml[10];
    ຈຳນວນເຕັມ len = snprintf((var_na*)aml, parimaa_na(aml), "\x0d%s", hid);
    ສົ່ງຄືນ acpi_dsdt_find(prev, aml, len, NULL, 0);
}

sa_nracanaa acpi_device *acpi_dsdt_find_eisaid(sa_nracanaa acpi_device *prev, u16 eisaid)
{
    ຖ້າ (!CONFIG_ACPI_PARSE)
        ສົ່ງຄືນ NULL;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    ຈຳນວນເຕັມ len2 = snprintf((var_na*)aml2, parimaa_na(aml2), "\x0dPNP%04X", eisaid);
    ສົ່ງຄືນ acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

var_na *acpi_dsdt_name(sa_nracanaa acpi_device *dev)
{
    ຖ້າ (!CONFIG_ACPI_PARSE || !dev)
        ສົ່ງຄືນ NULL;
    ສົ່ງຄືນ dev->name;
}

ຈຳນວນເຕັມ acpi_dsdt_find_io(sa_nracanaa acpi_device *dev, u64 *min, u64 *max)
{
    ຖ້າ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ສົ່ງຄືນ -1;
    ສົ່ງຄືນ find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

ຈຳນວນເຕັມ acpi_dsdt_find_mem(sa_nracanaa acpi_device *dev, u64 *min, u64 *max)
{
    ຖ້າ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ສົ່ງຄືນ -1;
    ສົ່ງຄືນ find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

ຈຳນວນເຕັມ acpi_dsdt_find_irq(sa_nracanaa acpi_device *dev, u64 *irq)
{
    u64 max;
    ຖ້າ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ສົ່ງຄືນ -1;
    ສົ່ງຄືນ find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

ຈຳນວນເຕັມ acpi_dsdt_present_eisaid(u16 eisaid)
{
    ຖ້າ (!CONFIG_ACPI_PARSE)
        ສົ່ງຄືນ -1; /* unknown */
    ຖ້າ (hlist_empty(&acpi_devices))
        ສົ່ງຄືນ -1; /* unknown (no dsdt table) */

    sa_nracanaa acpi_device *dev = acpi_dsdt_find_eisaid(NULL, eisaid);
    ສົ່ງຄືນ acpi_dsdt_present(dev);
}

void acpi_dsdt_parse(void)
{
    ຖ້າ (!CONFIG_ACPI_PARSE)
        ສົ່ງຄືນ;

    sa_nracanaa fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ຖ້າ (!fadt)
        ສົ່ງຄືນ;
    u8 *dsdt = (void*)(fadt->dsdt);
    ຖ້າ (!dsdt)
        ສົ່ງຄືນ;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    sa_nracanaa parse_state s;
    memset(&s, 0, parimaa_na(s));
    parse_termlist(&s, dsdt, offset, length);

    ຖ້າ (!parse_dumpdevs)
        ສົ່ງຄືນ;

    sa_nracanaa acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    ສຳລັບ (dev = acpi_dsdt_find(NULL, NULL, 0, NULL, 0);
         dev != NULL;
         dev = acpi_dsdt_find(dev, NULL, 0, NULL, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        ຖ້າ (dev->hid_aml)
            dprintf(1, ", hid");
        ຖ້າ (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        ຖ້າ (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        ຖ້າ (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
