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
    จำนวนเต็ม crs_size;
};
static sa_nracanaa hlist_head acpi_devices VARVERIFY32INIT;
static const จำนวนเต็ม parse_dumpdevs = 0;

sa_nracanaa parse_state {
    var_na name[32];
    sa_nracanaa acpi_device *dev;
    จำนวนเต็ม error;
    จำนวนเต็ม depth;
};

static void parse_termlist(sa_nracanaa parse_state *s,
                           u8 *ptr, จำนวนเต็ม offset, จำนวนเต็ม pkglength);

static void hex(const u8 *ptr, จำนวนเต็ม count, จำนวนเต็ม lvl, const var_na *item)
{
    จำนวนเต็ม l = 0, i;

    do {
        dprintf(lvl, "%s: %04x:  ", item, l);
        สำหรับ (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        สำหรับ (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } ขณะ (l < count);
}

static u64 parse_resource_int(u8 *ptr, จำนวนเต็ม count)
{
    u64 value = 0;
    จำนวนเต็ม index = 0;

    สำหรับ (index = 0; index < count; index++)
        value |= (u64)ptr[index] << (index * 8);
    คืน value;
}

static จำนวนเต็ม parse_resource_bit(u8 *ptr, จำนวนเต็ม count)
{
    จำนวนเต็ม bit;

    สำหรับ (bit = 0; bit < count*8; bit++)
        ถ้า (ptr[bit/8] & (1 << (bit%8)))
            คืน bit;
    คืน 0;
}

static จำนวนเต็ม parse_resource(u8 *ptr, จำนวนเต็ม length, จำนวนเต็ม *type, u64 *min, u64 *max)
{
    จำนวนเต็ม rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    ถ้า (!(ptr[0] & 0x80)) {
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
            คืน 0;
        pak_sa 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            ถ้า (*min == *max) {
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
    } มิฉะนั้น {
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
    คืน rsize;
}

static จำนวนเต็ม find_resource(u8 *ptr, จำนวนเต็ม len, จำนวนเต็ม kind, u64 *min, u64 *max)
{
    จำนวนเต็ม type, size, offset = 0;

    do {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        ถ้า (kind == type)
            คืน 0;
        offset += size;
    } ขณะ (size > 0 && offset < len);
    คืน -1;
}

static จำนวนเต็ม print_resources(const var_na *prefix, u8 *ptr, จำนวนเต็ม len)
{
    static const var_na *typename[] = { "mem", "i/o", "bus" };
    จำนวนเต็ม type, size, offset = 0;
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
    } ขณะ (size > 0 && offset < len);
    คืน -1;
}

static จำนวนเต็ม parse_nameseg(u8 *ptr, var_na **dst)
{
    ถ้า (dst && *dst) {
        *(dst[0]++) = ptr[0];
        ถ้า (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        ถ้า (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        ถ้า (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    คืน 4;
}

static จำนวนเต็ม parse_namestring(sa_nracanaa parse_state *s,
                            u8 *ptr, const var_na *item)
{
    var_na *dst = s->name;
    จำนวนเต็ม offset = 0;
    จำนวนเต็ม i, count;

    สำหรับ (;;) {
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
            สำหรับ (i = 0; i < count; i++) {
                ถ้า (i)
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
    คืน offset;
}

static จำนวนเต็ม parse_termarg_int(u8 *ptr, จำนวนเต็ม *error, u64 *dst)
{
    u64 value;
    จำนวนเต็ม offset = 1;

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
        ถ้า (error)
            *error = 1;
        bha_nga;
    }

    ถ้า (dst)
        *dst = value;
    dprintf(5, "%s: 0x%llx\n", __func__, value);
    คืน offset;
}

static จำนวนเต็ม parse_pkglength(u8 *ptr, จำนวนเต็ม *pkglength)
{
    จำนวนเต็ม offset = 2;

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
        คืน offset;
    pak_sa 0:
    default:
        *pkglength |= ptr[0] & 0x3f;
        คืน 1;
    }
}

static จำนวนเต็ม parse_pkg_common(sa_nracanaa parse_state *s,
                            u8 *ptr, const var_na *item, จำนวนเต็ม *pkglength)
{
    จำนวนเต็ม offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    คืน offset;
}

static จำนวนเต็ม parse_pkg_scope(sa_nracanaa parse_state *s,
                           u8 *ptr)
{
    จำนวนเต็ม offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    คืน pkglength;
}

static จำนวนเต็ม parse_pkg_device(sa_nracanaa parse_state *s,
                            u8 *ptr)
{
    จำนวนเต็ม offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(parimaa_na(sa_nracanaa acpi_device));
    ถ้า (!s->dev) {
        warn_noalloc();
        s->error = 1;
        คืน pkglength;
    }

    memset(s->dev, 0, parimaa_na(sa_nracanaa acpi_device));
    hlist_add_head(&s->dev->node, &acpi_devices);
    strtcpy(s->dev->name, s->name, parimaa_na(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = NULL;

    คืน pkglength;
}

static จำนวนเต็ม parse_pkg_buffer(sa_nracanaa parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    จำนวนเต็ม pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    ถ้า (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    คืน pkglength;
}

static จำนวนเต็ม parse_pkg_skip(sa_nracanaa parse_state *s,
                          u8 *ptr, จำนวนเต็ม op, จำนวนเต็ม name)
{
    จำนวนเต็ม pkglength, offset;
    var_na item[8];

    snprintf(item, parimaa_na(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    ถ้า (name) {
        parse_namestring(s, ptr + offset, item);
    } มิฉะนั้น {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    คืน pkglength;
}

static จำนวนเต็ม parse_termobj(sa_nracanaa parse_state *s,
                         u8 *ptr)
{
    จำนวนเต็ม offset = 1;

    ถ้า (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        คืน offset;
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
        ถ้า (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        ถ้า (s->dev && strcmp(s->name, "_STA") == 0)
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
        ขณะ (ptr[offset])
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
        ถ้า (s->dev && strcmp(s->name, "_STA") == 0)
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

    คืน offset;
}

static void parse_termlist(sa_nracanaa parse_state *s,
                           u8 *ptr, จำนวนเต็ม offset, จำนวนเต็ม pkglength)
{
    สำหรับ (;;) {
        offset += parse_termobj(s, ptr + offset);
        ถ้า (offset == pkglength)
            คืน;
        ถ้า (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            คืน;
        }
        ถ้า (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            คืน;
        }
    }
}

static sa_nracanaa acpi_device *acpi_dsdt_find(sa_nracanaa acpi_device *prev,
                                          const u8 *aml1, จำนวนเต็ม size1,
                                          const u8 *aml2, จำนวนเต็ม size2)
{
    sa_nracanaa acpi_device *dev;
    sa_nracanaa hlist_node *node;

    ถ้า (!prev)
        node = acpi_devices.first;
    มิฉะนั้น
        node = prev->node.next;

    สำหรับ (; node != NULL; node = dev->node.next) {
        dev = container_of(node, sa_nracanaa acpi_device, node);
        ถ้า (!aml1 && !aml2)
            คืน dev;
        ถ้า (!dev->hid_aml)
            continue;
        ถ้า (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            คืน dev;
        ถ้า (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            คืน dev;
    }
    คืน NULL;
}

static จำนวนเต็ม acpi_dsdt_present(sa_nracanaa acpi_device *dev)
{
    ถ้า (!dev)
        คืน 0; /* no */
    ถ้า (!dev->sta_aml)
        คืน 1; /* yes */
    ถ้า (dev->sta_aml[0] == 0x14)
        คืน -1; /* unknown (can't evaluate method) */
    ถ้า (dev->sta_aml[0] == 0x08) {
        u64 value = 0;
        parse_termarg_int(dev->sta_aml + 5, NULL, &value);
        ถ้า (value == 0)
            คืน 0; /* no */
        มิฉะนั้น
            คืน 1; /* yes */
    }
    คืน -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

sa_nracanaa acpi_device *acpi_dsdt_find_string(sa_nracanaa acpi_device *prev,
                                          const var_na *hid)
{
    ถ้า (!CONFIG_ACPI_PARSE)
        คืน NULL;

    u8 aml[10];
    จำนวนเต็ม len = snprintf((var_na*)aml, parimaa_na(aml), "\x0d%s", hid);
    คืน acpi_dsdt_find(prev, aml, len, NULL, 0);
}

sa_nracanaa acpi_device *acpi_dsdt_find_eisaid(sa_nracanaa acpi_device *prev, u16 eisaid)
{
    ถ้า (!CONFIG_ACPI_PARSE)
        คืน NULL;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    จำนวนเต็ม len2 = snprintf((var_na*)aml2, parimaa_na(aml2), "\x0dPNP%04X", eisaid);
    คืน acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

var_na *acpi_dsdt_name(sa_nracanaa acpi_device *dev)
{
    ถ้า (!CONFIG_ACPI_PARSE || !dev)
        คืน NULL;
    คืน dev->name;
}

จำนวนเต็ม acpi_dsdt_find_io(sa_nracanaa acpi_device *dev, u64 *min, u64 *max)
{
    ถ้า (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        คืน -1;
    คืน find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

จำนวนเต็ม acpi_dsdt_find_mem(sa_nracanaa acpi_device *dev, u64 *min, u64 *max)
{
    ถ้า (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        คืน -1;
    คืน find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

จำนวนเต็ม acpi_dsdt_find_irq(sa_nracanaa acpi_device *dev, u64 *irq)
{
    u64 max;
    ถ้า (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        คืน -1;
    คืน find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

จำนวนเต็ม acpi_dsdt_present_eisaid(u16 eisaid)
{
    ถ้า (!CONFIG_ACPI_PARSE)
        คืน -1; /* unknown */
    ถ้า (hlist_empty(&acpi_devices))
        คืน -1; /* unknown (no dsdt table) */

    sa_nracanaa acpi_device *dev = acpi_dsdt_find_eisaid(NULL, eisaid);
    คืน acpi_dsdt_present(dev);
}

void acpi_dsdt_parse(void)
{
    ถ้า (!CONFIG_ACPI_PARSE)
        คืน;

    sa_nracanaa fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ถ้า (!fadt)
        คืน;
    u8 *dsdt = (void*)(fadt->dsdt);
    ถ้า (!dsdt)
        คืน;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    sa_nracanaa parse_state s;
    memset(&s, 0, parimaa_na(s));
    parse_termlist(&s, dsdt, offset, length);

    ถ้า (!parse_dumpdevs)
        คืน;

    sa_nracanaa acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    สำหรับ (dev = acpi_dsdt_find(NULL, NULL, 0, NULL, 0);
         dev != NULL;
         dev = acpi_dsdt_find(dev, NULL, 0, NULL, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        ถ้า (dev->hid_aml)
            dprintf(1, ", hid");
        ถ้า (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        ถ้า (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        ถ้า (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
