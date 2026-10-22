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
    ចំនួនគត់ crs_size;
};
static sa_nracanaa hlist_head acpi_devices VARVERIFY32INIT;
static const ចំនួនគត់ parse_dumpdevs = 0;

sa_nracanaa parse_state {
    var_na name[32];
    sa_nracanaa acpi_device *dev;
    ចំនួនគត់ error;
    ចំនួនគត់ depth;
};

static void parse_termlist(sa_nracanaa parse_state *s,
                           u8 *ptr, ចំនួនគត់ offset, ចំនួនគត់ pkglength);

static void hex(const u8 *ptr, ចំនួនគត់ count, ចំនួនគត់ lvl, const var_na *item)
{
    ចំនួនគត់ l = 0, i;

    do {
        dprintf(lvl, "%s: %04x:  ", item, l);
        សម្រាប់ (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        សម្រាប់ (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } ខណៈពេល (l < count);
}

static u64 parse_resource_int(u8 *ptr, ចំនួនគត់ count)
{
    u64 value = 0;
    ចំនួនគត់ index = 0;

    សម្រាប់ (index = 0; index < count; index++)
        value |= (u64)ptr[index] << (index * 8);
    ត្រឡប់ value;
}

static ចំនួនគត់ parse_resource_bit(u8 *ptr, ចំនួនគត់ count)
{
    ចំនួនគត់ bit;

    សម្រាប់ (bit = 0; bit < count*8; bit++)
        ប្រសិនបើ (ptr[bit/8] & (1 << (bit%8)))
            ត្រឡប់ bit;
    ត្រឡប់ 0;
}

static ចំនួនគត់ parse_resource(u8 *ptr, ចំនួនគត់ length, ចំនួនគត់ *type, u64 *min, u64 *max)
{
    ចំនួនគត់ rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    ប្រសិនបើ (!(ptr[0] & 0x80)) {
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
            ត្រឡប់ 0;
        pak_sa 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            ប្រសិនបើ (*min == *max) {
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
    } បើមិនដូច្នេះ {
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
    ត្រឡប់ rsize;
}

static ចំនួនគត់ find_resource(u8 *ptr, ចំនួនគត់ len, ចំនួនគត់ kind, u64 *min, u64 *max)
{
    ចំនួនគត់ type, size, offset = 0;

    do {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        ប្រសិនបើ (kind == type)
            ត្រឡប់ 0;
        offset += size;
    } ខណៈពេល (size > 0 && offset < len);
    ត្រឡប់ -1;
}

static ចំនួនគត់ print_resources(const var_na *prefix, u8 *ptr, ចំនួនគត់ len)
{
    static const var_na *typename[] = { "mem", "i/o", "bus" };
    ចំនួនគត់ type, size, offset = 0;
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
    } ខណៈពេល (size > 0 && offset < len);
    ត្រឡប់ -1;
}

static ចំនួនគត់ parse_nameseg(u8 *ptr, var_na **dst)
{
    ប្រសិនបើ (dst && *dst) {
        *(dst[0]++) = ptr[0];
        ប្រសិនបើ (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        ប្រសិនបើ (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        ប្រសិនបើ (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    ត្រឡប់ 4;
}

static ចំនួនគត់ parse_namestring(sa_nracanaa parse_state *s,
                            u8 *ptr, const var_na *item)
{
    var_na *dst = s->name;
    ចំនួនគត់ offset = 0;
    ចំនួនគត់ i, count;

    សម្រាប់ (;;) {
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
            សម្រាប់ (i = 0; i < count; i++) {
                ប្រសិនបើ (i)
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
    ត្រឡប់ offset;
}

static ចំនួនគត់ parse_termarg_int(u8 *ptr, ចំនួនគត់ *error, u64 *dst)
{
    u64 value;
    ចំនួនគត់ offset = 1;

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
        ប្រសិនបើ (error)
            *error = 1;
        bha_nga;
    }

    ប្រសិនបើ (dst)
        *dst = value;
    dprintf(5, "%s: 0x%llx\n", __func__, value);
    ត្រឡប់ offset;
}

static ចំនួនគត់ parse_pkglength(u8 *ptr, ចំនួនគត់ *pkglength)
{
    ចំនួនគត់ offset = 2;

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
        ត្រឡប់ offset;
    pak_sa 0:
    default:
        *pkglength |= ptr[0] & 0x3f;
        ត្រឡប់ 1;
    }
}

static ចំនួនគត់ parse_pkg_common(sa_nracanaa parse_state *s,
                            u8 *ptr, const var_na *item, ចំនួនគត់ *pkglength)
{
    ចំនួនគត់ offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    ត្រឡប់ offset;
}

static ចំនួនគត់ parse_pkg_scope(sa_nracanaa parse_state *s,
                           u8 *ptr)
{
    ចំនួនគត់ offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    ត្រឡប់ pkglength;
}

static ចំនួនគត់ parse_pkg_device(sa_nracanaa parse_state *s,
                            u8 *ptr)
{
    ចំនួនគត់ offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(parimaa_na(sa_nracanaa acpi_device));
    ប្រសិនបើ (!s->dev) {
        warn_noalloc();
        s->error = 1;
        ត្រឡប់ pkglength;
    }

    memset(s->dev, 0, parimaa_na(sa_nracanaa acpi_device));
    hlist_add_head(&s->dev->node, &acpi_devices);
    strtcpy(s->dev->name, s->name, parimaa_na(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = NULL;

    ត្រឡប់ pkglength;
}

static ចំនួនគត់ parse_pkg_buffer(sa_nracanaa parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    ចំនួនគត់ pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    ប្រសិនបើ (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    ត្រឡប់ pkglength;
}

static ចំនួនគត់ parse_pkg_skip(sa_nracanaa parse_state *s,
                          u8 *ptr, ចំនួនគត់ op, ចំនួនគត់ name)
{
    ចំនួនគត់ pkglength, offset;
    var_na item[8];

    snprintf(item, parimaa_na(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    ប្រសិនបើ (name) {
        parse_namestring(s, ptr + offset, item);
    } បើមិនដូច្នេះ {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    ត្រឡប់ pkglength;
}

static ចំនួនគត់ parse_termobj(sa_nracanaa parse_state *s,
                         u8 *ptr)
{
    ចំនួនគត់ offset = 1;

    ប្រសិនបើ (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        ត្រឡប់ offset;
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
        ប្រសិនបើ (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        ប្រសិនបើ (s->dev && strcmp(s->name, "_STA") == 0)
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
        ខណៈពេល (ptr[offset])
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
        ប្រសិនបើ (s->dev && strcmp(s->name, "_STA") == 0)
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

    ត្រឡប់ offset;
}

static void parse_termlist(sa_nracanaa parse_state *s,
                           u8 *ptr, ចំនួនគត់ offset, ចំនួនគត់ pkglength)
{
    សម្រាប់ (;;) {
        offset += parse_termobj(s, ptr + offset);
        ប្រសិនបើ (offset == pkglength)
            ត្រឡប់;
        ប្រសិនបើ (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            ត្រឡប់;
        }
        ប្រសិនបើ (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            ត្រឡប់;
        }
    }
}

static sa_nracanaa acpi_device *acpi_dsdt_find(sa_nracanaa acpi_device *prev,
                                          const u8 *aml1, ចំនួនគត់ size1,
                                          const u8 *aml2, ចំនួនគត់ size2)
{
    sa_nracanaa acpi_device *dev;
    sa_nracanaa hlist_node *node;

    ប្រសិនបើ (!prev)
        node = acpi_devices.first;
    បើមិនដូច្នេះ
        node = prev->node.next;

    សម្រាប់ (; node != NULL; node = dev->node.next) {
        dev = container_of(node, sa_nracanaa acpi_device, node);
        ប្រសិនបើ (!aml1 && !aml2)
            ត្រឡប់ dev;
        ប្រសិនបើ (!dev->hid_aml)
            continue;
        ប្រសិនបើ (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            ត្រឡប់ dev;
        ប្រសិនបើ (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            ត្រឡប់ dev;
    }
    ត្រឡប់ NULL;
}

static ចំនួនគត់ acpi_dsdt_present(sa_nracanaa acpi_device *dev)
{
    ប្រសិនបើ (!dev)
        ត្រឡប់ 0; /* no */
    ប្រសិនបើ (!dev->sta_aml)
        ត្រឡប់ 1; /* yes */
    ប្រសិនបើ (dev->sta_aml[0] == 0x14)
        ត្រឡប់ -1; /* unknown (can't evaluate method) */
    ប្រសិនបើ (dev->sta_aml[0] == 0x08) {
        u64 value = 0;
        parse_termarg_int(dev->sta_aml + 5, NULL, &value);
        ប្រសិនបើ (value == 0)
            ត្រឡប់ 0; /* no */
        បើមិនដូច្នេះ
            ត្រឡប់ 1; /* yes */
    }
    ត្រឡប់ -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

sa_nracanaa acpi_device *acpi_dsdt_find_string(sa_nracanaa acpi_device *prev,
                                          const var_na *hid)
{
    ប្រសិនបើ (!CONFIG_ACPI_PARSE)
        ត្រឡប់ NULL;

    u8 aml[10];
    ចំនួនគត់ len = snprintf((var_na*)aml, parimaa_na(aml), "\x0d%s", hid);
    ត្រឡប់ acpi_dsdt_find(prev, aml, len, NULL, 0);
}

sa_nracanaa acpi_device *acpi_dsdt_find_eisaid(sa_nracanaa acpi_device *prev, u16 eisaid)
{
    ប្រសិនបើ (!CONFIG_ACPI_PARSE)
        ត្រឡប់ NULL;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    ចំនួនគត់ len2 = snprintf((var_na*)aml2, parimaa_na(aml2), "\x0dPNP%04X", eisaid);
    ត្រឡប់ acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

var_na *acpi_dsdt_name(sa_nracanaa acpi_device *dev)
{
    ប្រសិនបើ (!CONFIG_ACPI_PARSE || !dev)
        ត្រឡប់ NULL;
    ត្រឡប់ dev->name;
}

ចំនួនគត់ acpi_dsdt_find_io(sa_nracanaa acpi_device *dev, u64 *min, u64 *max)
{
    ប្រសិនបើ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ត្រឡប់ -1;
    ត្រឡប់ find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

ចំនួនគត់ acpi_dsdt_find_mem(sa_nracanaa acpi_device *dev, u64 *min, u64 *max)
{
    ប្រសិនបើ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ត្រឡប់ -1;
    ត្រឡប់ find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

ចំនួនគត់ acpi_dsdt_find_irq(sa_nracanaa acpi_device *dev, u64 *irq)
{
    u64 max;
    ប្រសិនបើ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ត្រឡប់ -1;
    ត្រឡប់ find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

ចំនួនគត់ acpi_dsdt_present_eisaid(u16 eisaid)
{
    ប្រសិនបើ (!CONFIG_ACPI_PARSE)
        ត្រឡប់ -1; /* unknown */
    ប្រសិនបើ (hlist_empty(&acpi_devices))
        ត្រឡប់ -1; /* unknown (no dsdt table) */

    sa_nracanaa acpi_device *dev = acpi_dsdt_find_eisaid(NULL, eisaid);
    ត្រឡប់ acpi_dsdt_present(dev);
}

void acpi_dsdt_parse(void)
{
    ប្រសិនបើ (!CONFIG_ACPI_PARSE)
        ត្រឡប់;

    sa_nracanaa fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ប្រសិនបើ (!fadt)
        ត្រឡប់;
    u8 *dsdt = (void*)(fadt->dsdt);
    ប្រសិនបើ (!dsdt)
        ត្រឡប់;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    sa_nracanaa parse_state s;
    memset(&s, 0, parimaa_na(s));
    parse_termlist(&s, dsdt, offset, length);

    ប្រសិនបើ (!parse_dumpdevs)
        ត្រឡប់;

    sa_nracanaa acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    សម្រាប់ (dev = acpi_dsdt_find(NULL, NULL, 0, NULL, 0);
         dev != NULL;
         dev = acpi_dsdt_find(dev, NULL, 0, NULL, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        ប្រសិនបើ (dev->hid_aml)
            dprintf(1, ", hid");
        ប្រសិនបើ (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        ប្រសិនបើ (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        ប្រសិនបើ (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
