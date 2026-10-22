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
    целое crs_size;
};
static struct hlist_head acpi_devices VARVERIFY32INIT;
static const целое parse_dumpdevs = 0;

struct parse_state {
    char name[32];
    struct acpi_device *dev;
    целое error;
    целое depth;
};

static void parse_termlist(struct parse_state *s,
                           u8 *ptr, целое offset, целое pkglength);

static void hex(const u8 *ptr, целое count, целое lvl, const char *item)
{
    целое l = 0, i;

    do {
        dprintf(lvl, "%s: %04x:  ", item, l);
        для (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        для (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } пока (l < count);
}

static u64 parse_resource_int(u8 *ptr, целое count)
{
    u64 value = 0;
    целое index = 0;

    для (index = 0; index < count; index++)
        value |= (u64)ptr[index] << (index * 8);
    вернуть value;
}

static целое parse_resource_bit(u8 *ptr, целое count)
{
    целое bit;

    для (bit = 0; bit < count*8; bit++)
        если (ptr[bit/8] & (1 << (bit%8)))
            вернуть bit;
    вернуть 0;
}

static целое parse_resource(u8 *ptr, целое length, целое *type, u64 *min, u64 *max)
{
    целое rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    если (!(ptr[0] & 0x80)) {
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
            вернуть 0;
        case 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            если (*min == *max) {
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
    } иначе {
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
    вернуть rsize;
}

static целое find_resource(u8 *ptr, целое len, целое kind, u64 *min, u64 *max)
{
    целое type, size, offset = 0;

    do {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        если (kind == type)
            вернуть 0;
        offset += size;
    } пока (size > 0 && offset < len);
    вернуть -1;
}

static целое print_resources(const char *prefix, u8 *ptr, целое len)
{
    static const char *typename[] = { "mem", "i/o", "bus" };
    целое type, size, offset = 0;
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
    } пока (size > 0 && offset < len);
    вернуть -1;
}

static целое parse_nameseg(u8 *ptr, char **dst)
{
    если (dst && *dst) {
        *(dst[0]++) = ptr[0];
        если (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        если (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        если (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    вернуть 4;
}

static целое parse_namestring(struct parse_state *s,
                            u8 *ptr, const char *item)
{
    char *dst = s->name;
    целое offset = 0;
    целое i, count;

    для (;;) {
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
            для (i = 0; i < count; i++) {
                если (i)
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
    вернуть offset;
}

static целое parse_termarg_int(u8 *ptr, целое *error, u64 *dst)
{
    u64 value;
    целое offset = 1;

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
        если (error)
            *error = 1;
        break;
    }

    если (dst)
        *dst = value;
    dprintf(5, "%s: 0x%llx\n", __func__, value);
    вернуть offset;
}

static целое parse_pkglength(u8 *ptr, целое *pkglength)
{
    целое offset = 2;

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
        вернуть offset;
    case 0:
    default:
        *pkglength |= ptr[0] & 0x3f;
        вернуть 1;
    }
}

static целое parse_pkg_common(struct parse_state *s,
                            u8 *ptr, const char *item, целое *pkglength)
{
    целое offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    вернуть offset;
}

static целое parse_pkg_scope(struct parse_state *s,
                           u8 *ptr)
{
    целое offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    вернуть pkglength;
}

static целое parse_pkg_device(struct parse_state *s,
                            u8 *ptr)
{
    целое offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(sizeof(struct acpi_device));
    если (!s->dev) {
        warn_noalloc();
        s->error = 1;
        вернуть pkglength;
    }

    memset(s->dev, 0, sizeof(struct acpi_device));
    hlist_add_head(&s->dev->node, &acpi_devices);
    strtcpy(s->dev->name, s->name, sizeof(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = NULL;

    вернуть pkglength;
}

static целое parse_pkg_buffer(struct parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    целое pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    если (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    вернуть pkglength;
}

static целое parse_pkg_skip(struct parse_state *s,
                          u8 *ptr, целое op, целое name)
{
    целое pkglength, offset;
    char item[8];

    snprintf(item, sizeof(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    если (name) {
        parse_namestring(s, ptr + offset, item);
    } иначе {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    вернуть pkglength;
}

static целое parse_termobj(struct parse_state *s,
                         u8 *ptr)
{
    целое offset = 1;

    если (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        вернуть offset;
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
        если (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        если (s->dev && strcmp(s->name, "_STA") == 0)
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
        пока (ptr[offset])
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
        если (s->dev && strcmp(s->name, "_STA") == 0)
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

    вернуть offset;
}

static void parse_termlist(struct parse_state *s,
                           u8 *ptr, целое offset, целое pkglength)
{
    для (;;) {
        offset += parse_termobj(s, ptr + offset);
        если (offset == pkglength)
            вернуть;
        если (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            вернуть;
        }
        если (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            вернуть;
        }
    }
}

static struct acpi_device *acpi_dsdt_find(struct acpi_device *prev,
                                          const u8 *aml1, целое size1,
                                          const u8 *aml2, целое size2)
{
    struct acpi_device *dev;
    struct hlist_node *node;

    если (!prev)
        node = acpi_devices.first;
    иначе
        node = prev->node.next;

    для (; node != NULL; node = dev->node.next) {
        dev = container_of(node, struct acpi_device, node);
        если (!aml1 && !aml2)
            вернуть dev;
        если (!dev->hid_aml)
            continue;
        если (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            вернуть dev;
        если (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            вернуть dev;
    }
    вернуть NULL;
}

static целое acpi_dsdt_present(struct acpi_device *dev)
{
    если (!dev)
        вернуть 0; /* no */
    если (!dev->sta_aml)
        вернуть 1; /* yes */
    если (dev->sta_aml[0] == 0x14)
        вернуть -1; /* unknown (can't evaluate method) */
    если (dev->sta_aml[0] == 0x08) {
        u64 value = 0;
        parse_termarg_int(dev->sta_aml + 5, NULL, &value);
        если (value == 0)
            вернуть 0; /* no */
        иначе
            вернуть 1; /* yes */
    }
    вернуть -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

struct acpi_device *acpi_dsdt_find_string(struct acpi_device *prev,
                                          const char *hid)
{
    если (!CONFIG_ACPI_PARSE)
        вернуть NULL;

    u8 aml[10];
    целое len = snprintf((char*)aml, sizeof(aml), "\x0d%s", hid);
    вернуть acpi_dsdt_find(prev, aml, len, NULL, 0);
}

struct acpi_device *acpi_dsdt_find_eisaid(struct acpi_device *prev, u16 eisaid)
{
    если (!CONFIG_ACPI_PARSE)
        вернуть NULL;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    целое len2 = snprintf((char*)aml2, sizeof(aml2), "\x0dPNP%04X", eisaid);
    вернуть acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

char *acpi_dsdt_name(struct acpi_device *dev)
{
    если (!CONFIG_ACPI_PARSE || !dev)
        вернуть NULL;
    вернуть dev->name;
}

целое acpi_dsdt_find_io(struct acpi_device *dev, u64 *min, u64 *max)
{
    если (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        вернуть -1;
    вернуть find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

целое acpi_dsdt_find_mem(struct acpi_device *dev, u64 *min, u64 *max)
{
    если (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        вернуть -1;
    вернуть find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

целое acpi_dsdt_find_irq(struct acpi_device *dev, u64 *irq)
{
    u64 max;
    если (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        вернуть -1;
    вернуть find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

целое acpi_dsdt_present_eisaid(u16 eisaid)
{
    если (!CONFIG_ACPI_PARSE)
        вернуть -1; /* unknown */
    если (hlist_empty(&acpi_devices))
        вернуть -1; /* unknown (no dsdt table) */

    struct acpi_device *dev = acpi_dsdt_find_eisaid(NULL, eisaid);
    вернуть acpi_dsdt_present(dev);
}

void acpi_dsdt_parse(void)
{
    если (!CONFIG_ACPI_PARSE)
        вернуть;

    struct fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    если (!fadt)
        вернуть;
    u8 *dsdt = (void*)(fadt->dsdt);
    если (!dsdt)
        вернуть;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    struct parse_state s;
    memset(&s, 0, sizeof(s));
    parse_termlist(&s, dsdt, offset, length);

    если (!parse_dumpdevs)
        вернуть;

    struct acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    для (dev = acpi_dsdt_find(NULL, NULL, 0, NULL, 0);
         dev != NULL;
         dev = acpi_dsdt_find(dev, NULL, 0, NULL, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        если (dev->hid_aml)
            dprintf(1, ", hid");
        если (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        если (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        если (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
