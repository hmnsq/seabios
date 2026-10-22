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

struct acpi_device {
    struct hlist_node node;
    char name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    整数 crs_size;
};
static struct hlist_head acpi_devices VARVERIFY32INIT;
static const 整数 parse_dumpdevs = 0;

struct parse_state {
    char name[32];
    struct acpi_device *dev;
    整数 error;
    整数 depth;
};

static void parse_termlist(struct parse_state *s,
                           u8 *ptr, 整数 offset, 整数 pkglength);

static void hex(const u8 *ptr, 整数 count, 整数 lvl, const char *item)
{
    整数 l = 0, i;

    do {
        dprintf(lvl, "%s: %04x:  ", item, l);
        为 (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        为 (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } 当 (l < count);
}

static u64 parse_resource_int(u8 *ptr, 整数 count)
{
    u64 value = 0;
    整数 index = 0;

    为 (index = 0; index < count; index++)
        value |= (u64)ptr[index] << (index * 8);
    返回 value;
}

static 整数 parse_resource_bit(u8 *ptr, 整数 count)
{
    整数 bit;

    为 (bit = 0; bit < count*8; bit++)
        如果 (ptr[bit/8] & (1 << (bit%8)))
            返回 bit;
    返回 0;
}

static 整数 parse_resource(u8 *ptr, 整数 length, 整数 *type, u64 *min, u64 *max)
{
    整数 rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    如果 (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        switch (rname) {
        case 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            break;
        case 0x0f: /* end marker */
            返回 0;
        case 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            如果 (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            break;
        case 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            break;
        default:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            break;
        }
    } 否则 {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        switch (rname) {
        case 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            break;
        case 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            break;
        case 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            break;
        case 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            break;
        case 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            break;
        default:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            break;
        }
    }
    返回 rsize;
}

static 整数 find_resource(u8 *ptr, 整数 len, 整数 kind, u64 *min, u64 *max)
{
    整数 type, size, offset = 0;

    do {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        如果 (kind == type)
            返回 0;
        offset += size;
    } 当 (size > 0 && offset < len);
    返回 -1;
}

static 整数 print_resources(const char *prefix, u8 *ptr, 整数 len)
{
    static const char *typename[] = { "mem", "i/o", "bus" };
    整数 type, size, offset = 0;
    u64 min, max;

    do {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        switch (type) {
        case 0:
        case 1:
        case 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            break;
        case 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            break;
        }
        offset += size;
    } 当 (size > 0 && offset < len);
    返回 -1;
}

static 整数 parse_nameseg(u8 *ptr, char **dst)
{
    如果 (dst && *dst) {
        *(dst[0]++) = ptr[0];
        如果 (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        如果 (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        如果 (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    返回 4;
}

static 整数 parse_namestring(struct parse_state *s,
                            u8 *ptr, const char *item)
{
    char *dst = s->name;
    整数 offset = 0;
    整数 i, count;

    为 (;;) {
        switch (ptr[offset]) {
        case 0: /* null name */
            offset++;
            *(dst++) = 0;
            break;
        case 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            break;
        case 0x2f:
            offset++;
            count = ptr[offset];
            offset++;
            为 (i = 0; i < count; i++) {
                如果 (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            break;
        case '\\':
            *(dst++) = '\\';
            offset++;
            continue;
        case '^':
            *(dst++) = '^';
            offset++;
            continue;
        case 'A' ... 'Z':
        case '_':
            offset += parse_nameseg(ptr, &dst);
            break;
        default:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            break;
        }
        break;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    返回 offset;
}

static 整数 parse_termarg_int(u8 *ptr, 整数 *error, u64 *dst)
{
    u64 value;
    整数 offset = 1;

    switch (ptr[0]) {
    case 0x00: /* zero */
        value = 0;
        break;
    case 0x01: /* one */
        value = 1;
        break;
    case 0x0a: /* byte prefix */
        value = ptr[1];
        offset++;
        break;
    case 0x0b: /* word prefix */
        value = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        break;
    case 0x0c: /* dword prefix */
        value = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        break;
    default:
        value = 0;
        hex(ptr, 16, 3, __func__);
        如果 (error)
            *error = 1;
        break;
    }

    如果 (dst)
        *dst = value;
    dprintf(5, "%s: 0x%llx\n", __func__, value);
    返回 offset;
}

static 整数 parse_pkglength(u8 *ptr, 整数 *pkglength)
{
    整数 offset = 2;

    *pkglength = 0;
    switch (ptr[0] >> 6) {
    case 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    case 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    case 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        返回 offset;
    case 0:
    default:
        *pkglength |= ptr[0] & 0x3f;
        返回 1;
    }
}

static 整数 parse_pkg_common(struct parse_state *s,
                            u8 *ptr, const char *item, 整数 *pkglength)
{
    整数 offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    返回 offset;
}

static 整数 parse_pkg_scope(struct parse_state *s,
                           u8 *ptr)
{
    整数 offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    返回 pkglength;
}

static 整数 parse_pkg_device(struct parse_state *s,
                            u8 *ptr)
{
    整数 offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(sizeof(struct acpi_device));
    如果 (!s->dev) {
        warn_noalloc();
        s->error = 1;
        返回 pkglength;
    }

    memset(s->dev, 0, sizeof(struct acpi_device));
    hlist_add_head(&s->dev->node, &acpi_devices);
    strtcpy(s->dev->name, s->name, sizeof(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = NULL;

    返回 pkglength;
}

static 整数 parse_pkg_buffer(struct parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    整数 pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    如果 (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    返回 pkglength;
}

static 整数 parse_pkg_skip(struct parse_state *s,
                          u8 *ptr, 整数 op, 整数 name)
{
    整数 pkglength, offset;
    char item[8];

    snprintf(item, sizeof(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    如果 (name) {
        parse_namestring(s, ptr + offset, item);
    } 否则 {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    返回 pkglength;
}

static 整数 parse_termobj(struct parse_state *s,
                         u8 *ptr)
{
    整数 offset = 1;

    如果 (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        返回 offset;
    }

    s->depth++;
    switch (ptr[0]) {
    case 0x00: /* zero */
        break;
    case 0x01: /* one */
        break;
    case 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        break;
    case 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        如果 (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        如果 (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        break;
    case 0x0a: /* byte prefix */
        offset++;
        break;
    case 0x0b: /* word prefix */
        offset += 2;
        break;
    case 0x0c: /* dword prefix */
        offset += 4;
        break;
    case 0x0d: /* string prefix */
        当 (ptr[offset])
            offset++;
        offset++;
        break;
    case 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        break;
    case 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        break;
    case 0x12: /* package op */
    case 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        break;
    case 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        如果 (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        break;
    case 0x5b: /* ext op prefix */
        offset++;
        switch (ptr[1]) {
        case 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            break;
        case 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, NULL);
            offset += parse_termarg_int(ptr + offset, &s->error, NULL);
            break;
        case 0x81: /* field op */
        case 0x83: /* processor op */
        case 0x84: /* power resource op */
        case 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            break;
        case 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            break;
        default:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            break;
        }
        break;
    default:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        break;
    }
    s->depth--;

    返回 offset;
}

static void parse_termlist(struct parse_state *s,
                           u8 *ptr, 整数 offset, 整数 pkglength)
{
    为 (;;) {
        offset += parse_termobj(s, ptr + offset);
        如果 (offset == pkglength)
            返回;
        如果 (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            返回;
        }
        如果 (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            返回;
        }
    }
}

static struct acpi_device *acpi_dsdt_find(struct acpi_device *prev,
                                          const u8 *aml1, 整数 size1,
                                          const u8 *aml2, 整数 size2)
{
    struct acpi_device *dev;
    struct hlist_node *node;

    如果 (!prev)
        node = acpi_devices.first;
    否则
        node = prev->node.next;

    为 (; node != NULL; node = dev->node.next) {
        dev = container_of(node, struct acpi_device, node);
        如果 (!aml1 && !aml2)
            返回 dev;
        如果 (!dev->hid_aml)
            continue;
        如果 (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            返回 dev;
        如果 (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            返回 dev;
    }
    返回 NULL;
}

static 整数 acpi_dsdt_present(struct acpi_device *dev)
{
    如果 (!dev)
        返回 0; /* no */
    如果 (!dev->sta_aml)
        返回 1; /* yes */
    如果 (dev->sta_aml[0] == 0x14)
        返回 -1; /* unknown (can't evaluate method) */
    如果 (dev->sta_aml[0] == 0x08) {
        u64 value = 0;
        parse_termarg_int(dev->sta_aml + 5, NULL, &value);
        如果 (value == 0)
            返回 0; /* no */
        否则
            返回 1; /* yes */
    }
    返回 -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

struct acpi_device *acpi_dsdt_find_string(struct acpi_device *prev,
                                          const char *hid)
{
    如果 (!CONFIG_ACPI_PARSE)
        返回 NULL;

    u8 aml[10];
    整数 len = snprintf((char*)aml, sizeof(aml), "\x0d%s", hid);
    返回 acpi_dsdt_find(prev, aml, len, NULL, 0);
}

struct acpi_device *acpi_dsdt_find_eisaid(struct acpi_device *prev, u16 eisaid)
{
    如果 (!CONFIG_ACPI_PARSE)
        返回 NULL;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    整数 len2 = snprintf((char*)aml2, sizeof(aml2), "\x0dPNP%04X", eisaid);
    返回 acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

char *acpi_dsdt_name(struct acpi_device *dev)
{
    如果 (!CONFIG_ACPI_PARSE || !dev)
        返回 NULL;
    返回 dev->name;
}

整数 acpi_dsdt_find_io(struct acpi_device *dev, u64 *min, u64 *max)
{
    如果 (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        返回 -1;
    返回 find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

整数 acpi_dsdt_find_mem(struct acpi_device *dev, u64 *min, u64 *max)
{
    如果 (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        返回 -1;
    返回 find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

整数 acpi_dsdt_find_irq(struct acpi_device *dev, u64 *irq)
{
    u64 max;
    如果 (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        返回 -1;
    返回 find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

整数 acpi_dsdt_present_eisaid(u16 eisaid)
{
    如果 (!CONFIG_ACPI_PARSE)
        返回 -1; /* unknown */
    如果 (hlist_empty(&acpi_devices))
        返回 -1; /* unknown (no dsdt table) */

    struct acpi_device *dev = acpi_dsdt_find_eisaid(NULL, eisaid);
    返回 acpi_dsdt_present(dev);
}

void acpi_dsdt_parse(void)
{
    如果 (!CONFIG_ACPI_PARSE)
        返回;

    struct fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    如果 (!fadt)
        返回;
    u8 *dsdt = (void*)(fadt->dsdt);
    如果 (!dsdt)
        返回;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    struct parse_state s;
    memset(&s, 0, sizeof(s));
    parse_termlist(&s, dsdt, offset, length);

    如果 (!parse_dumpdevs)
        返回;

    struct acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    为 (dev = acpi_dsdt_find(NULL, NULL, 0, NULL, 0);
         dev != NULL;
         dev = acpi_dsdt_find(dev, NULL, 0, NULL, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        如果 (dev->hid_aml)
            dprintf(1, ", hid");
        如果 (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        如果 (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        如果 (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
