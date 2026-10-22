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
    정수 crs_size;
};
static struct hlist_head acpi_devices VARVERIFY32INIT;
static const 정수 parse_dumpdevs = 0;

struct parse_state {
    char name[32];
    struct acpi_device *dev;
    정수 error;
    정수 depth;
};

static void parse_termlist(struct parse_state *s,
                           u8 *ptr, 정수 offset, 정수 pkglength);

static void hex(const u8 *ptr, 정수 count, 정수 lvl, const char *item)
{
    정수 l = 0, i;

    do {
        dprintf(lvl, "%s: %04x:  ", item, l);
        위해 (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        위해 (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } 동안 (l < count);
}

static u64 parse_resource_int(u8 *ptr, 정수 count)
{
    u64 value = 0;
    정수 index = 0;

    위해 (index = 0; index < count; index++)
        value |= (u64)ptr[index] << (index * 8);
    반환 value;
}

static 정수 parse_resource_bit(u8 *ptr, 정수 count)
{
    정수 bit;

    위해 (bit = 0; bit < count*8; bit++)
        만약 (ptr[bit/8] & (1 << (bit%8)))
            반환 bit;
    반환 0;
}

static 정수 parse_resource(u8 *ptr, 정수 length, 정수 *type, u64 *min, u64 *max)
{
    정수 rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    만약 (!(ptr[0] & 0x80)) {
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
            반환 0;
        case 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            만약 (*min == *max) {
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
    } 아니면 {
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
    반환 rsize;
}

static 정수 find_resource(u8 *ptr, 정수 len, 정수 kind, u64 *min, u64 *max)
{
    정수 type, size, offset = 0;

    do {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        만약 (kind == type)
            반환 0;
        offset += size;
    } 동안 (size > 0 && offset < len);
    반환 -1;
}

static 정수 print_resources(const char *prefix, u8 *ptr, 정수 len)
{
    static const char *typename[] = { "mem", "i/o", "bus" };
    정수 type, size, offset = 0;
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
    } 동안 (size > 0 && offset < len);
    반환 -1;
}

static 정수 parse_nameseg(u8 *ptr, char **dst)
{
    만약 (dst && *dst) {
        *(dst[0]++) = ptr[0];
        만약 (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        만약 (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        만약 (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    반환 4;
}

static 정수 parse_namestring(struct parse_state *s,
                            u8 *ptr, const char *item)
{
    char *dst = s->name;
    정수 offset = 0;
    정수 i, count;

    위해 (;;) {
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
            위해 (i = 0; i < count; i++) {
                만약 (i)
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
    반환 offset;
}

static 정수 parse_termarg_int(u8 *ptr, 정수 *error, u64 *dst)
{
    u64 value;
    정수 offset = 1;

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
        만약 (error)
            *error = 1;
        break;
    }

    만약 (dst)
        *dst = value;
    dprintf(5, "%s: 0x%llx\n", __func__, value);
    반환 offset;
}

static 정수 parse_pkglength(u8 *ptr, 정수 *pkglength)
{
    정수 offset = 2;

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
        반환 offset;
    case 0:
    default:
        *pkglength |= ptr[0] & 0x3f;
        반환 1;
    }
}

static 정수 parse_pkg_common(struct parse_state *s,
                            u8 *ptr, const char *item, 정수 *pkglength)
{
    정수 offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    반환 offset;
}

static 정수 parse_pkg_scope(struct parse_state *s,
                           u8 *ptr)
{
    정수 offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    반환 pkglength;
}

static 정수 parse_pkg_device(struct parse_state *s,
                            u8 *ptr)
{
    정수 offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(sizeof(struct acpi_device));
    만약 (!s->dev) {
        warn_noalloc();
        s->error = 1;
        반환 pkglength;
    }

    memset(s->dev, 0, sizeof(struct acpi_device));
    hlist_add_head(&s->dev->node, &acpi_devices);
    strtcpy(s->dev->name, s->name, sizeof(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = NULL;

    반환 pkglength;
}

static 정수 parse_pkg_buffer(struct parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    정수 pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    만약 (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    반환 pkglength;
}

static 정수 parse_pkg_skip(struct parse_state *s,
                          u8 *ptr, 정수 op, 정수 name)
{
    정수 pkglength, offset;
    char item[8];

    snprintf(item, sizeof(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    만약 (name) {
        parse_namestring(s, ptr + offset, item);
    } 아니면 {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    반환 pkglength;
}

static 정수 parse_termobj(struct parse_state *s,
                         u8 *ptr)
{
    정수 offset = 1;

    만약 (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        반환 offset;
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
        만약 (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        만약 (s->dev && strcmp(s->name, "_STA") == 0)
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
        동안 (ptr[offset])
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
        만약 (s->dev && strcmp(s->name, "_STA") == 0)
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

    반환 offset;
}

static void parse_termlist(struct parse_state *s,
                           u8 *ptr, 정수 offset, 정수 pkglength)
{
    위해 (;;) {
        offset += parse_termobj(s, ptr + offset);
        만약 (offset == pkglength)
            반환;
        만약 (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            반환;
        }
        만약 (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            반환;
        }
    }
}

static struct acpi_device *acpi_dsdt_find(struct acpi_device *prev,
                                          const u8 *aml1, 정수 size1,
                                          const u8 *aml2, 정수 size2)
{
    struct acpi_device *dev;
    struct hlist_node *node;

    만약 (!prev)
        node = acpi_devices.first;
    아니면
        node = prev->node.next;

    위해 (; node != NULL; node = dev->node.next) {
        dev = container_of(node, struct acpi_device, node);
        만약 (!aml1 && !aml2)
            반환 dev;
        만약 (!dev->hid_aml)
            continue;
        만약 (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            반환 dev;
        만약 (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            반환 dev;
    }
    반환 NULL;
}

static 정수 acpi_dsdt_present(struct acpi_device *dev)
{
    만약 (!dev)
        반환 0; /* no */
    만약 (!dev->sta_aml)
        반환 1; /* yes */
    만약 (dev->sta_aml[0] == 0x14)
        반환 -1; /* unknown (can't evaluate method) */
    만약 (dev->sta_aml[0] == 0x08) {
        u64 value = 0;
        parse_termarg_int(dev->sta_aml + 5, NULL, &value);
        만약 (value == 0)
            반환 0; /* no */
        아니면
            반환 1; /* yes */
    }
    반환 -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

struct acpi_device *acpi_dsdt_find_string(struct acpi_device *prev,
                                          const char *hid)
{
    만약 (!CONFIG_ACPI_PARSE)
        반환 NULL;

    u8 aml[10];
    정수 len = snprintf((char*)aml, sizeof(aml), "\x0d%s", hid);
    반환 acpi_dsdt_find(prev, aml, len, NULL, 0);
}

struct acpi_device *acpi_dsdt_find_eisaid(struct acpi_device *prev, u16 eisaid)
{
    만약 (!CONFIG_ACPI_PARSE)
        반환 NULL;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    정수 len2 = snprintf((char*)aml2, sizeof(aml2), "\x0dPNP%04X", eisaid);
    반환 acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

char *acpi_dsdt_name(struct acpi_device *dev)
{
    만약 (!CONFIG_ACPI_PARSE || !dev)
        반환 NULL;
    반환 dev->name;
}

정수 acpi_dsdt_find_io(struct acpi_device *dev, u64 *min, u64 *max)
{
    만약 (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        반환 -1;
    반환 find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

정수 acpi_dsdt_find_mem(struct acpi_device *dev, u64 *min, u64 *max)
{
    만약 (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        반환 -1;
    반환 find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

정수 acpi_dsdt_find_irq(struct acpi_device *dev, u64 *irq)
{
    u64 max;
    만약 (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        반환 -1;
    반환 find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

정수 acpi_dsdt_present_eisaid(u16 eisaid)
{
    만약 (!CONFIG_ACPI_PARSE)
        반환 -1; /* unknown */
    만약 (hlist_empty(&acpi_devices))
        반환 -1; /* unknown (no dsdt table) */

    struct acpi_device *dev = acpi_dsdt_find_eisaid(NULL, eisaid);
    반환 acpi_dsdt_present(dev);
}

void acpi_dsdt_parse(void)
{
    만약 (!CONFIG_ACPI_PARSE)
        반환;

    struct fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    만약 (!fadt)
        반환;
    u8 *dsdt = (void*)(fadt->dsdt);
    만약 (!dsdt)
        반환;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    struct parse_state s;
    memset(&s, 0, sizeof(s));
    parse_termlist(&s, dsdt, offset, length);

    만약 (!parse_dumpdevs)
        반환;

    struct acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    위해 (dev = acpi_dsdt_find(NULL, NULL, 0, NULL, 0);
         dev != NULL;
         dev = acpi_dsdt_find(dev, NULL, 0, NULL, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        만약 (dev->hid_aml)
            dprintf(1, ", hid");
        만약 (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        만약 (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        만약 (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
