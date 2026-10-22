// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "list.h"   // hlist_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "util.h"
#ಅನ್ತರ್ಭಾವ "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

ಸಂರಚನಾ acpi_device {
    ಸಂರಚನಾ hlist_ಗಂಟು ಗಂಟು;
    ವರ್ಣ name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    ಪೂರ್ಣಾಙ್ಕ crs_size;
};
ಸ್ಥಿರ ಸಂರಚನಾ hlist_head acpi_devices VARVERIFY32INIT;
ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ parse_dumpdevs = 0;

ಸಂರಚನಾ parse_state {
    ವರ್ಣ name[32];
    ಸಂರಚನಾ acpi_device *dev;
    ಪೂರ್ಣಾಙ್ಕ error;
    ಪೂರ್ಣಾಙ್ಕ depth;
};

ಸ್ಥಿರ ಶೂನ್ಯ parse_termlist(ಸಂರಚನಾ parse_state *s,
                           u8 *ptr, ಪೂರ್ಣಾಙ್ಕ offset, ಪೂರ್ಣಾಙ್ಕ pkglength);

ಸ್ಥಿರ ಶೂನ್ಯ hex(ಧ್ರುವ u8 *ptr, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ, ಪೂರ್ಣಾಙ್ಕ lvl, ಧ್ರುವ ವರ್ಣ *item)
{
    ಪೂರ್ಣಾಙ್ಕ l = 0, i;

    ಕುರು {
        dprintf(lvl, "%s: %04x:  ", item, l);
        ಪ್ರತಿ (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        ಪ್ರತಿ (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } ಯಾವತ್ (l < ಎಣಿಕೆ);
}

ಸ್ಥಿರ u64 parse_resource_int(u8 *ptr, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
    u64 ಮೌಲ್ಯ = 0;
    ಪೂರ್ಣಾಙ್ಕ index = 0;

    ಪ್ರತಿ (index = 0; index < ಎಣಿಕೆ; index++)
        ಮೌಲ್ಯ |= (u64)ptr[index] << (index * 8);
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_resource_bit(u8 *ptr, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
    ಪೂರ್ಣಾಙ್ಕ bit;

    ಪ್ರತಿ (bit = 0; bit < ಎಣಿಕೆ*8; bit++)
        ಯದಿ (ptr[bit/8] & (1 << (bit%8)))
            ಪ್ರತ್ಯಾಗಮ bit;
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_resource(u8 *ptr, ಪೂರ್ಣಾಙ್ಕ length, ಪೂರ್ಣಾಙ್ಕ *type, u64 *min, u64 *max)
{
    ಪೂರ್ಣಾಙ್ಕ rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    ಯದಿ (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        ವಿಕಲ್ಪ (rname) {
        ಪಕ್ಷ 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x0f: /* end marker */
            ಪ್ರತ್ಯಾಗಮ 0;
        ಪಕ್ಷ 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            ಯದಿ (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            ಭಙ್ಗ;
        }
    } ಅನ್ಯಥಾ {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        ವಿಕಲ್ಪ (rname) {
        ಪಕ್ಷ 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            ಭಙ್ಗ;
        }
    }
    ಪ್ರತ್ಯಾಗಮ rsize;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ find_resource(u8 *ptr, ಪೂರ್ಣಾಙ್ಕ len, ಪೂರ್ಣಾಙ್ಕ kind, u64 *min, u64 *max)
{
    ಪೂರ್ಣಾಙ್ಕ type, size, offset = 0;

    ಕುರು {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        ಯದಿ (kind == type)
            ಪ್ರತ್ಯಾಗಮ 0;
        offset += size;
    } ಯಾವತ್ (size > 0 && offset < len);
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ print_resources(ಧ್ರುವ ವರ್ಣ *prefix, u8 *ptr, ಪೂರ್ಣಾಙ್ಕ len)
{
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *typename[] = { "mem", "i/o", "bus" };
    ಪೂರ್ಣಾಙ್ಕ type, size, offset = 0;
    u64 min, max;

    ಕುರು {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        ವಿಕಲ್ಪ (type) {
        ಪಕ್ಷ 0:
        ಪಕ್ಷ 1:
        ಪಕ್ಷ 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            ಭಙ್ಗ;
        ಪಕ್ಷ 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            ಭಙ್ಗ;
        }
        offset += size;
    } ಯಾವತ್ (size > 0 && offset < len);
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_nameseg(u8 *ptr, ವರ್ಣ **dst)
{
    ಯದಿ (dst && *dst) {
        *(dst[0]++) = ptr[0];
        ಯದಿ (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        ಯದಿ (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        ಯದಿ (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    ಪ್ರತ್ಯಾಗಮ 4;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_namestring(ಸಂರಚನಾ parse_state *s,
                            u8 *ptr, ಧ್ರುವ ವರ್ಣ *item)
{
    ವರ್ಣ *dst = s->name;
    ಪೂರ್ಣಾಙ್ಕ offset = 0;
    ಪೂರ್ಣಾಙ್ಕ i, ಎಣಿಕೆ;

    ಪ್ರತಿ (;;) {
        ವಿಕಲ್ಪ (ptr[offset]) {
        ಪಕ್ಷ 0: /* null name */
            offset++;
            *(dst++) = 0;
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x2f:
            offset++;
            ಎಣಿಕೆ = ptr[offset];
            offset++;
            ಪ್ರತಿ (i = 0; i < ಎಣಿಕೆ; i++) {
                ಯದಿ (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            ಭಙ್ಗ;
        ಪಕ್ಷ '\\':
            *(dst++) = '\\';
            offset++;
            ಅನುವರ್ತ;
        ಪಕ್ಷ '^':
            *(dst++) = '^';
            offset++;
            ಅನುವರ್ತ;
        ಪಕ್ಷ 'A' ... 'Z':
        ಪಕ್ಷ '_':
            offset += parse_nameseg(ptr, &dst);
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ಭಙ್ಗ;
        }
        ಭಙ್ಗ;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    ಪ್ರತ್ಯಾಗಮ offset;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_termarg_int(u8 *ptr, ಪೂರ್ಣಾಙ್ಕ *error, u64 *dst)
{
    u64 ಮೌಲ್ಯ;
    ಪೂರ್ಣಾಙ್ಕ offset = 1;

    ವಿಕಲ್ಪ (ptr[0]) {
    ಪಕ್ಷ 0x00: /* zero */
        ಮೌಲ್ಯ = 0;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: /* one */
        ಮೌಲ್ಯ = 1;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x0a: /* byte prefix */
        ಮೌಲ್ಯ = ptr[1];
        offset++;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x0b: /* word prefix */
        ಮೌಲ್ಯ = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x0c: /* dword prefix */
        ಮೌಲ್ಯ = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        ಮೌಲ್ಯ = 0;
        hex(ptr, 16, 3, __func__);
        ಯದಿ (error)
            *error = 1;
        ಭಙ್ಗ;
    }

    ಯದಿ (dst)
        *dst = ಮೌಲ್ಯ;
    dprintf(5, "%s: 0x%llx\n", __func__, ಮೌಲ್ಯ);
    ಪ್ರತ್ಯಾಗಮ offset;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_pkglength(u8 *ptr, ಪೂರ್ಣಾಙ್ಕ *pkglength)
{
    ಪೂರ್ಣಾಙ್ಕ offset = 2;

    *pkglength = 0;
    ವಿಕಲ್ಪ (ptr[0] >> 6) {
    ಪಕ್ಷ 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    ಪಕ್ಷ 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    ಪಕ್ಷ 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        ಪ್ರತ್ಯಾಗಮ offset;
    ಪಕ್ಷ 0:
    ಸ್ವಭಾವ:
        *pkglength |= ptr[0] & 0x3f;
        ಪ್ರತ್ಯಾಗಮ 1;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_pkg_common(ಸಂರಚನಾ parse_state *s,
                            u8 *ptr, ಧ್ರುವ ವರ್ಣ *item, ಪೂರ್ಣಾಙ್ಕ *pkglength)
{
    ಪೂರ್ಣಾಙ್ಕ offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    ಪ್ರತ್ಯಾಗಮ offset;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_pkg_scope(ಸಂರಚನಾ parse_state *s,
                           u8 *ptr)
{
    ಪೂರ್ಣಾಙ್ಕ offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    ಪ್ರತ್ಯಾಗಮ pkglength;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_pkg_device(ಸಂರಚನಾ parse_state *s,
                            u8 *ptr)
{
    ಪೂರ್ಣಾಙ್ಕ offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(ಪರಿಮಾಣ(ಸಂರಚನಾ acpi_device));
    ಯದಿ (!s->dev) {
        warn_noalloc();
        s->error = 1;
        ಪ್ರತ್ಯಾಗಮ pkglength;
    }

    memset(s->dev, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ acpi_device));
    hlist_add_head(&s->dev->ಗಂಟು, &acpi_devices);
    strtcpy(s->dev->name, s->name, ಪರಿಮಾಣ(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = ರಿಕ್ತ;

    ಪ್ರತ್ಯಾಗಮ pkglength;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_pkg_buffer(ಸಂರಚನಾ parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    ಪೂರ್ಣಾಙ್ಕ pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    ಯದಿ (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    ಪ್ರತ್ಯಾಗಮ pkglength;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_pkg_skip(ಸಂರಚನಾ parse_state *s,
                          u8 *ptr, ಪೂರ್ಣಾಙ್ಕ op, ಪೂರ್ಣಾಙ್ಕ name)
{
    ಪೂರ್ಣಾಙ್ಕ pkglength, offset;
    ವರ್ಣ item[8];

    snprintf(item, ಪರಿಮಾಣ(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    ಯದಿ (name) {
        parse_namestring(s, ptr + offset, item);
    } ಅನ್ಯಥಾ {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    ಪ್ರತ್ಯಾಗಮ pkglength;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_termobj(ಸಂರಚನಾ parse_state *s,
                         u8 *ptr)
{
    ಪೂರ್ಣಾಙ್ಕ offset = 1;

    ಯದಿ (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        ಪ್ರತ್ಯಾಗಮ offset;
    }

    s->depth++;
    ವಿಕಲ್ಪ (ptr[0]) {
    ಪಕ್ಷ 0x00: /* zero */
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x01: /* one */
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        ಯದಿ (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        ಯದಿ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x0a: /* byte prefix */
        offset++;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x0b: /* word prefix */
        offset += 2;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x0c: /* dword prefix */
        offset += 4;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x0d: /* string prefix */
        ಯಾವತ್ (ptr[offset])
            offset++;
        offset++;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x12: /* package op */
    ಪಕ್ಷ 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        ಯದಿ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ಭಙ್ಗ;
    ಪಕ್ಷ 0x5b: /* ext op prefix */
        offset++;
        ವಿಕಲ್ಪ (ptr[1]) {
        ಪಕ್ಷ 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, ರಿಕ್ತ);
            offset += parse_termarg_int(ptr + offset, &s->error, ರಿಕ್ತ);
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x81: /* field op */
        ಪಕ್ಷ 0x83: /* processor op */
        ಪಕ್ಷ 0x84: /* power resource op */
        ಪಕ್ಷ 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ಭಙ್ಗ;
        }
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        ಭಙ್ಗ;
    }
    s->depth--;

    ಪ್ರತ್ಯಾಗಮ offset;
}

ಸ್ಥಿರ ಶೂನ್ಯ parse_termlist(ಸಂರಚನಾ parse_state *s,
                           u8 *ptr, ಪೂರ್ಣಾಙ್ಕ offset, ಪೂರ್ಣಾಙ್ಕ pkglength)
{
    ಪ್ರತಿ (;;) {
        offset += parse_termobj(s, ptr + offset);
        ಯದಿ (offset == pkglength)
            ಪ್ರತ್ಯಾಗಮ;
        ಯದಿ (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            ಪ್ರತ್ಯಾಗಮ;
        }
        ಯದಿ (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            ಪ್ರತ್ಯಾಗಮ;
        }
    }
}

ಸ್ಥಿರ ಸಂರಚನಾ acpi_device *acpi_dsdt_find(ಸಂರಚನಾ acpi_device *prev,
                                          ಧ್ರುವ u8 *aml1, ಪೂರ್ಣಾಙ್ಕ size1,
                                          ಧ್ರುವ u8 *aml2, ಪೂರ್ಣಾಙ್ಕ size2)
{
    ಸಂರಚನಾ acpi_device *dev;
    ಸಂರಚನಾ hlist_ಗಂಟು *ಗಂಟು;

    ಯದಿ (!prev)
        ಗಂಟು = acpi_devices.first;
    ಅನ್ಯಥಾ
        ಗಂಟು = prev->ಗಂಟು.next;

    ಪ್ರತಿ (; ಗಂಟು != ರಿಕ್ತ; ಗಂಟು = dev->ಗಂಟು.next) {
        dev = container_of(ಗಂಟು, ಸಂರಚನಾ acpi_device, ಗಂಟು);
        ಯದಿ (!aml1 && !aml2)
            ಪ್ರತ್ಯಾಗಮ dev;
        ಯದಿ (!dev->hid_aml)
            ಅನುವರ್ತ;
        ಯದಿ (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            ಪ್ರತ್ಯಾಗಮ dev;
        ಯದಿ (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            ಪ್ರತ್ಯಾಗಮ dev;
    }
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_present(ಸಂರಚನಾ acpi_device *dev)
{
    ಯದಿ (!dev)
        ಪ್ರತ್ಯಾಗಮ 0; /* no */
    ಯದಿ (!dev->sta_aml)
        ಪ್ರತ್ಯಾಗಮ 1; /* yes */
    ಯದಿ (dev->sta_aml[0] == 0x14)
        ಪ್ರತ್ಯಾಗಮ -1; /* unknown (can't evaluate method) */
    ಯದಿ (dev->sta_aml[0] == 0x08) {
        u64 ಮೌಲ್ಯ = 0;
        parse_termarg_int(dev->sta_aml + 5, ರಿಕ್ತ, &ಮೌಲ್ಯ);
        ಯದಿ (ಮೌಲ್ಯ == 0)
            ಪ್ರತ್ಯಾಗಮ 0; /* no */
        ಅನ್ಯಥಾ
            ಪ್ರತ್ಯಾಗಮ 1; /* yes */
    }
    ಪ್ರತ್ಯಾಗಮ -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

ಸಂರಚನಾ acpi_device *acpi_dsdt_find_string(ಸಂರಚನಾ acpi_device *prev,
                                          ಧ್ರುವ ವರ್ಣ *hid)
{
    ಯದಿ (!CONFIG_ACPI_PARSE)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    u8 aml[10];
    ಪೂರ್ಣಾಙ್ಕ len = snprintf((ವರ್ಣ*)aml, ಪರಿಮಾಣ(aml), "\x0d%s", hid);
    ಪ್ರತ್ಯಾಗಮ acpi_dsdt_find(prev, aml, len, ರಿಕ್ತ, 0);
}

ಸಂರಚನಾ acpi_device *acpi_dsdt_find_eisaid(ಸಂರಚನಾ acpi_device *prev, u16 eisaid)
{
    ಯದಿ (!CONFIG_ACPI_PARSE)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    ಪೂರ್ಣಾಙ್ಕ len2 = snprintf((ವರ್ಣ*)aml2, ಪರಿಮಾಣ(aml2), "\x0dPNP%04X", eisaid);
    ಪ್ರತ್ಯಾಗಮ acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

ವರ್ಣ *acpi_dsdt_name(ಸಂರಚನಾ acpi_device *dev)
{
    ಯದಿ (!CONFIG_ACPI_PARSE || !dev)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ dev->name;
}

ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_find_io(ಸಂರಚನಾ acpi_device *dev, u64 *min, u64 *max)
{
    ಯದಿ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಪ್ರತ್ಯಾಗಮ find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_find_mem(ಸಂರಚನಾ acpi_device *dev, u64 *min, u64 *max)
{
    ಯದಿ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಪ್ರತ್ಯಾಗಮ find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_find_irq(ಸಂರಚನಾ acpi_device *dev, u64 *irq)
{
    u64 max;
    ಯದಿ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಪ್ರತ್ಯಾಗಮ find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

ಪೂರ್ಣಾಙ್ಕ acpi_dsdt_present_eisaid(u16 eisaid)
{
    ಯದಿ (!CONFIG_ACPI_PARSE)
        ಪ್ರತ್ಯಾಗಮ -1; /* unknown */
    ಯದಿ (hlist_empty(&acpi_devices))
        ಪ್ರತ್ಯಾಗಮ -1; /* unknown (no dsdt table) */

    ಸಂರಚನಾ acpi_device *dev = acpi_dsdt_find_eisaid(ರಿಕ್ತ, eisaid);
    ಪ್ರತ್ಯಾಗಮ acpi_dsdt_present(dev);
}

ಶೂನ್ಯ acpi_dsdt_parse(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_ACPI_PARSE)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    ಯದಿ (!fadt)
        ಪ್ರತ್ಯಾಗಮ;
    u8 *dsdt = (ಶೂನ್ಯ*)(fadt->dsdt);
    ಯದಿ (!dsdt)
        ಪ್ರತ್ಯಾಗಮ;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    ಸಂರಚನಾ parse_state s;
    memset(&s, 0, ಪರಿಮಾಣ(s));
    parse_termlist(&s, dsdt, offset, length);

    ಯದಿ (!parse_dumpdevs)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    ಪ್ರತಿ (dev = acpi_dsdt_find(ರಿಕ್ತ, ರಿಕ್ತ, 0, ರಿಕ್ತ, 0);
         dev != ರಿಕ್ತ;
         dev = acpi_dsdt_find(dev, ರಿಕ್ತ, 0, ರಿಕ್ತ, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        ಯದಿ (dev->hid_aml)
            dprintf(1, ", hid");
        ಯದಿ (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        ಯದಿ (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        ಯದಿ (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
