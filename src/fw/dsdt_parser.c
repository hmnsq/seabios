// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "list.h"   // hlist_*
#અન્તર્ભાવ "malloc.h" // malloc_*
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "string.h" // memcpy
#અન્તર્ભાવ "util.h"
#અન્તર્ભાવ "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

સંરચના acpi_device {
    સંરચના hlist_ગાંઠ ગાંઠ;
    અક્ષર name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    પૂર્ણાંક crs_size;
};
સ્થિર સંરચના hlist_head acpi_devices VARVERIFY32INIT;
સ્થિર અચલ પૂર્ણાંક parse_dumpdevs = 0;

સંરચના parse_state {
    અક્ષર name[32];
    સંરચના acpi_device *dev;
    પૂર્ણાંક error;
    પૂર્ણાંક depth;
};

સ્થિર શૂન્ય parse_termlist(સંરચના parse_state *s,
                           u8 *ptr, પૂર્ણાંક offset, પૂર્ણાંક pkglength);

સ્થિર શૂન્ય hex(અચલ u8 *ptr, પૂર્ણાંક ગણતરી, પૂર્ણાંક lvl, અચલ અક્ષર *item)
{
    પૂર્ણાંક l = 0, i;

    કરો {
        dprintf(lvl, "%s: %04x:  ", item, l);
        માટે (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        માટે (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } જ્યારે (l < ગણતરી);
}

સ્થિર u64 parse_resource_int(u8 *ptr, પૂર્ણાંક ગણતરી)
{
    u64 કિંમત = 0;
    પૂર્ણાંક index = 0;

    માટે (index = 0; index < ગણતરી; index++)
        કિંમત |= (u64)ptr[index] << (index * 8);
    પાછા કિંમત;
}

સ્થિર પૂર્ણાંક parse_resource_bit(u8 *ptr, પૂર્ણાંક ગણતરી)
{
    પૂર્ણાંક bit;

    માટે (bit = 0; bit < ગણતરી*8; bit++)
        જો (ptr[bit/8] & (1 << (bit%8)))
            પાછા bit;
    પાછા 0;
}

સ્થિર પૂર્ણાંક parse_resource(u8 *ptr, પૂર્ણાંક length, પૂર્ણાંક *type, u64 *min, u64 *max)
{
    પૂર્ણાંક rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    જો (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        પસંદગી (rname) {
        સ્થિતિ 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            તોડો;
        સ્થિતિ 0x0f: /* end marker */
            પાછા 0;
        સ્થિતિ 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            જો (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            તોડો;
        સ્થિતિ 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            તોડો;
        મૂળભૂત:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            તોડો;
        }
    } નહીંતર {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        પસંદગી (rname) {
        સ્થિતિ 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            તોડો;
        સ્થિતિ 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            તોડો;
        સ્થિતિ 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            તોડો;
        સ્થિતિ 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            તોડો;
        સ્થિતિ 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            તોડો;
        મૂળભૂત:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            તોડો;
        }
    }
    પાછા rsize;
}

સ્થિર પૂર્ણાંક find_resource(u8 *ptr, પૂર્ણાંક len, પૂર્ણાંક kind, u64 *min, u64 *max)
{
    પૂર્ણાંક type, size, offset = 0;

    કરો {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        જો (kind == type)
            પાછા 0;
        offset += size;
    } જ્યારે (size > 0 && offset < len);
    પાછા -1;
}

સ્થિર પૂર્ણાંક print_resources(અચલ અક્ષર *prefix, u8 *ptr, પૂર્ણાંક len)
{
    સ્થિર અચલ અક્ષર *typename[] = { "mem", "i/o", "bus" };
    પૂર્ણાંક type, size, offset = 0;
    u64 min, max;

    કરો {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        પસંદગી (type) {
        સ્થિતિ 0:
        સ્થિતિ 1:
        સ્થિતિ 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            તોડો;
        સ્થિતિ 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            તોડો;
        }
        offset += size;
    } જ્યારે (size > 0 && offset < len);
    પાછા -1;
}

સ્થિર પૂર્ણાંક parse_nameseg(u8 *ptr, અક્ષર **dst)
{
    જો (dst && *dst) {
        *(dst[0]++) = ptr[0];
        જો (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        જો (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        જો (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    પાછા 4;
}

સ્થિર પૂર્ણાંક parse_namestring(સંરચના parse_state *s,
                            u8 *ptr, અચલ અક્ષર *item)
{
    અક્ષર *dst = s->name;
    પૂર્ણાંક offset = 0;
    પૂર્ણાંક i, ગણતરી;

    માટે (;;) {
        પસંદગી (ptr[offset]) {
        સ્થિતિ 0: /* null name */
            offset++;
            *(dst++) = 0;
            તોડો;
        સ્થિતિ 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            તોડો;
        સ્થિતિ 0x2f:
            offset++;
            ગણતરી = ptr[offset];
            offset++;
            માટે (i = 0; i < ગણતરી; i++) {
                જો (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            તોડો;
        સ્થિતિ '\\':
            *(dst++) = '\\';
            offset++;
            ચાલુ;
        સ્થિતિ '^':
            *(dst++) = '^';
            offset++;
            ચાલુ;
        સ્થિતિ 'A' ... 'Z':
        સ્થિતિ '_':
            offset += parse_nameseg(ptr, &dst);
            તોડો;
        મૂળભૂત:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            તોડો;
        }
        તોડો;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    પાછા offset;
}

સ્થિર પૂર્ણાંક parse_termarg_int(u8 *ptr, પૂર્ણાંક *error, u64 *dst)
{
    u64 કિંમત;
    પૂર્ણાંક offset = 1;

    પસંદગી (ptr[0]) {
    સ્થિતિ 0x00: /* zero */
        કિંમત = 0;
        તોડો;
    સ્થિતિ 0x01: /* one */
        કિંમત = 1;
        તોડો;
    સ્થિતિ 0x0a: /* byte prefix */
        કિંમત = ptr[1];
        offset++;
        તોડો;
    સ્થિતિ 0x0b: /* word prefix */
        કિંમત = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        તોડો;
    સ્થિતિ 0x0c: /* dword prefix */
        કિંમત = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        તોડો;
    મૂળભૂત:
        કિંમત = 0;
        hex(ptr, 16, 3, __func__);
        જો (error)
            *error = 1;
        તોડો;
    }

    જો (dst)
        *dst = કિંમત;
    dprintf(5, "%s: 0x%llx\n", __func__, કિંમત);
    પાછા offset;
}

સ્થિર પૂર્ણાંક parse_pkglength(u8 *ptr, પૂર્ણાંક *pkglength)
{
    પૂર્ણાંક offset = 2;

    *pkglength = 0;
    પસંદગી (ptr[0] >> 6) {
    સ્થિતિ 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    સ્થિતિ 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    સ્થિતિ 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        પાછા offset;
    સ્થિતિ 0:
    મૂળભૂત:
        *pkglength |= ptr[0] & 0x3f;
        પાછા 1;
    }
}

સ્થિર પૂર્ણાંક parse_pkg_common(સંરચના parse_state *s,
                            u8 *ptr, અચલ અક્ષર *item, પૂર્ણાંક *pkglength)
{
    પૂર્ણાંક offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    પાછા offset;
}

સ્થિર પૂર્ણાંક parse_pkg_scope(સંરચના parse_state *s,
                           u8 *ptr)
{
    પૂર્ણાંક offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    પાછા pkglength;
}

સ્થિર પૂર્ણાંક parse_pkg_device(સંરચના parse_state *s,
                            u8 *ptr)
{
    પૂર્ણાંક offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(કદ(સંરચના acpi_device));
    જો (!s->dev) {
        warn_noalloc();
        s->error = 1;
        પાછા pkglength;
    }

    memset(s->dev, 0, કદ(સંરચના acpi_device));
    hlist_add_head(&s->dev->ગાંઠ, &acpi_devices);
    strtcpy(s->dev->name, s->name, કદ(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = ખાલી;

    પાછા pkglength;
}

સ્થિર પૂર્ણાંક parse_pkg_buffer(સંરચના parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    પૂર્ણાંક pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    જો (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    પાછા pkglength;
}

સ્થિર પૂર્ણાંક parse_pkg_skip(સંરચના parse_state *s,
                          u8 *ptr, પૂર્ણાંક op, પૂર્ણાંક name)
{
    પૂર્ણાંક pkglength, offset;
    અક્ષર item[8];

    snprintf(item, કદ(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    જો (name) {
        parse_namestring(s, ptr + offset, item);
    } નહીંતર {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    પાછા pkglength;
}

સ્થિર પૂર્ણાંક parse_termobj(સંરચના parse_state *s,
                         u8 *ptr)
{
    પૂર્ણાંક offset = 1;

    જો (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        પાછા offset;
    }

    s->depth++;
    પસંદગી (ptr[0]) {
    સ્થિતિ 0x00: /* zero */
        તોડો;
    સ્થિતિ 0x01: /* one */
        તોડો;
    સ્થિતિ 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        તોડો;
    સ્થિતિ 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        જો (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        જો (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        તોડો;
    સ્થિતિ 0x0a: /* byte prefix */
        offset++;
        તોડો;
    સ્થિતિ 0x0b: /* word prefix */
        offset += 2;
        તોડો;
    સ્થિતિ 0x0c: /* dword prefix */
        offset += 4;
        તોડો;
    સ્થિતિ 0x0d: /* string prefix */
        જ્યારે (ptr[offset])
            offset++;
        offset++;
        તોડો;
    સ્થિતિ 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        તોડો;
    સ્થિતિ 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        તોડો;
    સ્થિતિ 0x12: /* package op */
    સ્થિતિ 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        તોડો;
    સ્થિતિ 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        જો (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        તોડો;
    સ્થિતિ 0x5b: /* ext op prefix */
        offset++;
        પસંદગી (ptr[1]) {
        સ્થિતિ 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            તોડો;
        સ્થિતિ 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, ખાલી);
            offset += parse_termarg_int(ptr + offset, &s->error, ખાલી);
            તોડો;
        સ્થિતિ 0x81: /* field op */
        સ્થિતિ 0x83: /* processor op */
        સ્થિતિ 0x84: /* power resource op */
        સ્થિતિ 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            તોડો;
        સ્થિતિ 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            તોડો;
        મૂળભૂત:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            તોડો;
        }
        તોડો;
    મૂળભૂત:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        તોડો;
    }
    s->depth--;

    પાછા offset;
}

સ્થિર શૂન્ય parse_termlist(સંરચના parse_state *s,
                           u8 *ptr, પૂર્ણાંક offset, પૂર્ણાંક pkglength)
{
    માટે (;;) {
        offset += parse_termobj(s, ptr + offset);
        જો (offset == pkglength)
            પાછા;
        જો (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            પાછા;
        }
        જો (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            પાછા;
        }
    }
}

સ્થિર સંરચના acpi_device *acpi_dsdt_find(સંરચના acpi_device *prev,
                                          અચલ u8 *aml1, પૂર્ણાંક size1,
                                          અચલ u8 *aml2, પૂર્ણાંક size2)
{
    સંરચના acpi_device *dev;
    સંરચના hlist_ગાંઠ *ગાંઠ;

    જો (!prev)
        ગાંઠ = acpi_devices.first;
    નહીંતર
        ગાંઠ = prev->ગાંઠ.next;

    માટે (; ગાંઠ != ખાલી; ગાંઠ = dev->ગાંઠ.next) {
        dev = container_of(ગાંઠ, સંરચના acpi_device, ગાંઠ);
        જો (!aml1 && !aml2)
            પાછા dev;
        જો (!dev->hid_aml)
            ચાલુ;
        જો (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            પાછા dev;
        જો (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            પાછા dev;
    }
    પાછા ખાલી;
}

સ્થિર પૂર્ણાંક acpi_dsdt_present(સંરચના acpi_device *dev)
{
    જો (!dev)
        પાછા 0; /* no */
    જો (!dev->sta_aml)
        પાછા 1; /* yes */
    જો (dev->sta_aml[0] == 0x14)
        પાછા -1; /* unknown (can't evaluate method) */
    જો (dev->sta_aml[0] == 0x08) {
        u64 કિંમત = 0;
        parse_termarg_int(dev->sta_aml + 5, ખાલી, &કિંમત);
        જો (કિંમત == 0)
            પાછા 0; /* no */
        નહીંતર
            પાછા 1; /* yes */
    }
    પાછા -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

સંરચના acpi_device *acpi_dsdt_find_string(સંરચના acpi_device *prev,
                                          અચલ અક્ષર *hid)
{
    જો (!CONFIG_ACPI_PARSE)
        પાછા ખાલી;

    u8 aml[10];
    પૂર્ણાંક len = snprintf((અક્ષર*)aml, કદ(aml), "\x0d%s", hid);
    પાછા acpi_dsdt_find(prev, aml, len, ખાલી, 0);
}

સંરચના acpi_device *acpi_dsdt_find_eisaid(સંરચના acpi_device *prev, u16 eisaid)
{
    જો (!CONFIG_ACPI_PARSE)
        પાછા ખાલી;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    પૂર્ણાંક len2 = snprintf((અક્ષર*)aml2, કદ(aml2), "\x0dPNP%04X", eisaid);
    પાછા acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

અક્ષર *acpi_dsdt_name(સંરચના acpi_device *dev)
{
    જો (!CONFIG_ACPI_PARSE || !dev)
        પાછા ખાલી;
    પાછા dev->name;
}

પૂર્ણાંક acpi_dsdt_find_io(સંરચના acpi_device *dev, u64 *min, u64 *max)
{
    જો (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        પાછા -1;
    પાછા find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

પૂર્ણાંક acpi_dsdt_find_mem(સંરચના acpi_device *dev, u64 *min, u64 *max)
{
    જો (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        પાછા -1;
    પાછા find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

પૂર્ણાંક acpi_dsdt_find_irq(સંરચના acpi_device *dev, u64 *irq)
{
    u64 max;
    જો (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        પાછા -1;
    પાછા find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

પૂર્ણાંક acpi_dsdt_present_eisaid(u16 eisaid)
{
    જો (!CONFIG_ACPI_PARSE)
        પાછા -1; /* unknown */
    જો (hlist_empty(&acpi_devices))
        પાછા -1; /* unknown (no dsdt table) */

    સંરચના acpi_device *dev = acpi_dsdt_find_eisaid(ખાલી, eisaid);
    પાછા acpi_dsdt_present(dev);
}

શૂન્ય acpi_dsdt_parse(શૂન્ય)
{
    જો (!CONFIG_ACPI_PARSE)
        પાછા;

    સંરચના fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    જો (!fadt)
        પાછા;
    u8 *dsdt = (શૂન્ય*)(fadt->dsdt);
    જો (!dsdt)
        પાછા;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    સંરચના parse_state s;
    memset(&s, 0, કદ(s));
    parse_termlist(&s, dsdt, offset, length);

    જો (!parse_dumpdevs)
        પાછા;

    સંરચના acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    માટે (dev = acpi_dsdt_find(ખાલી, ખાલી, 0, ખાલી, 0);
         dev != ખાલી;
         dev = acpi_dsdt_find(dev, ખાલી, 0, ખાલી, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        જો (dev->hid_aml)
            dprintf(1, ", hid");
        જો (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        જો (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        જો (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
