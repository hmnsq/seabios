// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "list.h"   // hlist_*
#അന്തര്ഭാവ "malloc.h" // malloc_*
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "util.h"
#അന്തര്ഭാവ "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

ഘടന acpi_device {
    ഘടന hlist_കണ്ണി കണ്ണി;
    അക്ഷരം name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    പൂർണ്ണസംഖ്യ crs_size;
};
സ്ഥിതം ഘടന hlist_head acpi_devices VARVERIFY32INIT;
സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ parse_dumpdevs = 0;

ഘടന parse_state {
    അക്ഷരം name[32];
    ഘടന acpi_device *dev;
    പൂർണ്ണസംഖ്യ error;
    പൂർണ്ണസംഖ്യ depth;
};

സ്ഥിതം ശൂന്യം parse_termlist(ഘടന parse_state *s,
                           u8 *ptr, പൂർണ്ണസംഖ്യ offset, പൂർണ്ണസംഖ്യ pkglength);

സ്ഥിതം ശൂന്യം hex(സ്ഥിരം u8 *ptr, പൂർണ്ണസംഖ്യ എണ്ണം, പൂർണ്ണസംഖ്യ lvl, സ്ഥിരം അക്ഷരം *item)
{
    പൂർണ്ണസംഖ്യ l = 0, i;

    ചെയ്യുക {
        dprintf(lvl, "%s: %04x:  ", item, l);
        വേണ്ടി (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        വേണ്ടി (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } ആകുമ്പോൾ (l < എണ്ണം);
}

സ്ഥിതം u64 parse_resource_int(u8 *ptr, പൂർണ്ണസംഖ്യ എണ്ണം)
{
    u64 മൂല്യം = 0;
    പൂർണ്ണസംഖ്യ index = 0;

    വേണ്ടി (index = 0; index < എണ്ണം; index++)
        മൂല്യം |= (u64)ptr[index] << (index * 8);
    തിരികെ മൂല്യം;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_resource_bit(u8 *ptr, പൂർണ്ണസംഖ്യ എണ്ണം)
{
    പൂർണ്ണസംഖ്യ bit;

    വേണ്ടി (bit = 0; bit < എണ്ണം*8; bit++)
        എങ്കിൽ (ptr[bit/8] & (1 << (bit%8)))
            തിരികെ bit;
    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_resource(u8 *ptr, പൂർണ്ണസംഖ്യ length, പൂർണ്ണസംഖ്യ *type, u64 *min, u64 *max)
{
    പൂർണ്ണസംഖ്യ rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    എങ്കിൽ (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        തിരഞ്ഞെടുപ്പ് (rname) {
        അവസ്ഥ 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            നിർത്തുക;
        അവസ്ഥ 0x0f: /* end marker */
            തിരികെ 0;
        അവസ്ഥ 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            എങ്കിൽ (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            നിർത്തുക;
        അവസ്ഥ 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            നിർത്തുക;
        }
    } അല്ലെങ്കിൽ {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        തിരഞ്ഞെടുപ്പ് (rname) {
        അവസ്ഥ 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            നിർത്തുക;
        അവസ്ഥ 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            നിർത്തുക;
        അവസ്ഥ 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            നിർത്തുക;
        അവസ്ഥ 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            നിർത്തുക;
        അവസ്ഥ 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            നിർത്തുക;
        }
    }
    തിരികെ rsize;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ find_resource(u8 *ptr, പൂർണ്ണസംഖ്യ len, പൂർണ്ണസംഖ്യ kind, u64 *min, u64 *max)
{
    പൂർണ്ണസംഖ്യ type, size, offset = 0;

    ചെയ്യുക {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        എങ്കിൽ (kind == type)
            തിരികെ 0;
        offset += size;
    } ആകുമ്പോൾ (size > 0 && offset < len);
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ print_resources(സ്ഥിരം അക്ഷരം *prefix, u8 *ptr, പൂർണ്ണസംഖ്യ len)
{
    സ്ഥിതം സ്ഥിരം അക്ഷരം *typename[] = { "mem", "i/o", "bus" };
    പൂർണ്ണസംഖ്യ type, size, offset = 0;
    u64 min, max;

    ചെയ്യുക {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        തിരഞ്ഞെടുപ്പ് (type) {
        അവസ്ഥ 0:
        അവസ്ഥ 1:
        അവസ്ഥ 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            നിർത്തുക;
        അവസ്ഥ 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            നിർത്തുക;
        }
        offset += size;
    } ആകുമ്പോൾ (size > 0 && offset < len);
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_nameseg(u8 *ptr, അക്ഷരം **dst)
{
    എങ്കിൽ (dst && *dst) {
        *(dst[0]++) = ptr[0];
        എങ്കിൽ (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        എങ്കിൽ (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        എങ്കിൽ (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    തിരികെ 4;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_namestring(ഘടന parse_state *s,
                            u8 *ptr, സ്ഥിരം അക്ഷരം *item)
{
    അക്ഷരം *dst = s->name;
    പൂർണ്ണസംഖ്യ offset = 0;
    പൂർണ്ണസംഖ്യ i, എണ്ണം;

    വേണ്ടി (;;) {
        തിരഞ്ഞെടുപ്പ് (ptr[offset]) {
        അവസ്ഥ 0: /* null name */
            offset++;
            *(dst++) = 0;
            നിർത്തുക;
        അവസ്ഥ 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            നിർത്തുക;
        അവസ്ഥ 0x2f:
            offset++;
            എണ്ണം = ptr[offset];
            offset++;
            വേണ്ടി (i = 0; i < എണ്ണം; i++) {
                എങ്കിൽ (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            നിർത്തുക;
        അവസ്ഥ '\\':
            *(dst++) = '\\';
            offset++;
            തുടരുക;
        അവസ്ഥ '^':
            *(dst++) = '^';
            offset++;
            തുടരുക;
        അവസ്ഥ 'A' ... 'Z':
        അവസ്ഥ '_':
            offset += parse_nameseg(ptr, &dst);
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            നിർത്തുക;
        }
        നിർത്തുക;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    തിരികെ offset;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_termarg_int(u8 *ptr, പൂർണ്ണസംഖ്യ *error, u64 *dst)
{
    u64 മൂല്യം;
    പൂർണ്ണസംഖ്യ offset = 1;

    തിരഞ്ഞെടുപ്പ് (ptr[0]) {
    അവസ്ഥ 0x00: /* zero */
        മൂല്യം = 0;
        നിർത്തുക;
    അവസ്ഥ 0x01: /* one */
        മൂല്യം = 1;
        നിർത്തുക;
    അവസ്ഥ 0x0a: /* byte prefix */
        മൂല്യം = ptr[1];
        offset++;
        നിർത്തുക;
    അവസ്ഥ 0x0b: /* word prefix */
        മൂല്യം = ptr[1] |
            ((unsigned long)ptr[2] << 8);
        offset += 2;
        നിർത്തുക;
    അവസ്ഥ 0x0c: /* dword prefix */
        മൂല്യം = ptr[1] |
            ((unsigned long)ptr[2] << 8) |
            ((unsigned long)ptr[3] << 16) |
            ((unsigned long)ptr[4] << 24);
        offset += 4;
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        മൂല്യം = 0;
        hex(ptr, 16, 3, __func__);
        എങ്കിൽ (error)
            *error = 1;
        നിർത്തുക;
    }

    എങ്കിൽ (dst)
        *dst = മൂല്യം;
    dprintf(5, "%s: 0x%llx\n", __func__, മൂല്യം);
    തിരികെ offset;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_pkglength(u8 *ptr, പൂർണ്ണസംഖ്യ *pkglength)
{
    പൂർണ്ണസംഖ്യ offset = 2;

    *pkglength = 0;
    തിരഞ്ഞെടുപ്പ് (ptr[0] >> 6) {
    അവസ്ഥ 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    അവസ്ഥ 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    അവസ്ഥ 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        തിരികെ offset;
    അവസ്ഥ 0:
    സ്ഥിരസ്ഥിതി:
        *pkglength |= ptr[0] & 0x3f;
        തിരികെ 1;
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_pkg_common(ഘടന parse_state *s,
                            u8 *ptr, സ്ഥിരം അക്ഷരം *item, പൂർണ്ണസംഖ്യ *pkglength)
{
    പൂർണ്ണസംഖ്യ offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    തിരികെ offset;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_pkg_scope(ഘടന parse_state *s,
                           u8 *ptr)
{
    പൂർണ്ണസംഖ്യ offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    തിരികെ pkglength;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_pkg_device(ഘടന parse_state *s,
                            u8 *ptr)
{
    പൂർണ്ണസംഖ്യ offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(വലുപ്പം(ഘടന acpi_device));
    എങ്കിൽ (!s->dev) {
        warn_noalloc();
        s->error = 1;
        തിരികെ pkglength;
    }

    memset(s->dev, 0, വലുപ്പം(ഘടന acpi_device));
    hlist_add_head(&s->dev->കണ്ണി, &acpi_devices);
    strtcpy(s->dev->name, s->name, വലുപ്പം(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = ശൂന്യ;

    തിരികെ pkglength;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_pkg_buffer(ഘടന parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    പൂർണ്ണസംഖ്യ pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    എങ്കിൽ (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    തിരികെ pkglength;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_pkg_skip(ഘടന parse_state *s,
                          u8 *ptr, പൂർണ്ണസംഖ്യ op, പൂർണ്ണസംഖ്യ name)
{
    പൂർണ്ണസംഖ്യ pkglength, offset;
    അക്ഷരം item[8];

    snprintf(item, വലുപ്പം(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    എങ്കിൽ (name) {
        parse_namestring(s, ptr + offset, item);
    } അല്ലെങ്കിൽ {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    തിരികെ pkglength;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_termobj(ഘടന parse_state *s,
                         u8 *ptr)
{
    പൂർണ്ണസംഖ്യ offset = 1;

    എങ്കിൽ (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        തിരികെ offset;
    }

    s->depth++;
    തിരഞ്ഞെടുപ്പ് (ptr[0]) {
    അവസ്ഥ 0x00: /* zero */
        നിർത്തുക;
    അവസ്ഥ 0x01: /* one */
        നിർത്തുക;
    അവസ്ഥ 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        നിർത്തുക;
    അവസ്ഥ 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        എങ്കിൽ (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        എങ്കിൽ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        നിർത്തുക;
    അവസ്ഥ 0x0a: /* byte prefix */
        offset++;
        നിർത്തുക;
    അവസ്ഥ 0x0b: /* word prefix */
        offset += 2;
        നിർത്തുക;
    അവസ്ഥ 0x0c: /* dword prefix */
        offset += 4;
        നിർത്തുക;
    അവസ്ഥ 0x0d: /* string prefix */
        ആകുമ്പോൾ (ptr[offset])
            offset++;
        offset++;
        നിർത്തുക;
    അവസ്ഥ 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        നിർത്തുക;
    അവസ്ഥ 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        നിർത്തുക;
    അവസ്ഥ 0x12: /* package op */
    അവസ്ഥ 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        നിർത്തുക;
    അവസ്ഥ 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        എങ്കിൽ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        നിർത്തുക;
    അവസ്ഥ 0x5b: /* ext op prefix */
        offset++;
        തിരഞ്ഞെടുപ്പ് (ptr[1]) {
        അവസ്ഥ 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            നിർത്തുക;
        അവസ്ഥ 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, ശൂന്യ);
            offset += parse_termarg_int(ptr + offset, &s->error, ശൂന്യ);
            നിർത്തുക;
        അവസ്ഥ 0x81: /* field op */
        അവസ്ഥ 0x83: /* processor op */
        അവസ്ഥ 0x84: /* power resource op */
        അവസ്ഥ 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            നിർത്തുക;
        അവസ്ഥ 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            നിർത്തുക;
        }
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        നിർത്തുക;
    }
    s->depth--;

    തിരികെ offset;
}

സ്ഥിതം ശൂന്യം parse_termlist(ഘടന parse_state *s,
                           u8 *ptr, പൂർണ്ണസംഖ്യ offset, പൂർണ്ണസംഖ്യ pkglength)
{
    വേണ്ടി (;;) {
        offset += parse_termobj(s, ptr + offset);
        എങ്കിൽ (offset == pkglength)
            തിരികെ;
        എങ്കിൽ (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            തിരികെ;
        }
        എങ്കിൽ (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            തിരികെ;
        }
    }
}

സ്ഥിതം ഘടന acpi_device *acpi_dsdt_find(ഘടന acpi_device *prev,
                                          സ്ഥിരം u8 *aml1, പൂർണ്ണസംഖ്യ size1,
                                          സ്ഥിരം u8 *aml2, പൂർണ്ണസംഖ്യ size2)
{
    ഘടന acpi_device *dev;
    ഘടന hlist_കണ്ണി *കണ്ണി;

    എങ്കിൽ (!prev)
        കണ്ണി = acpi_devices.first;
    അല്ലെങ്കിൽ
        കണ്ണി = prev->കണ്ണി.next;

    വേണ്ടി (; കണ്ണി != ശൂന്യ; കണ്ണി = dev->കണ്ണി.next) {
        dev = container_of(കണ്ണി, ഘടന acpi_device, കണ്ണി);
        എങ്കിൽ (!aml1 && !aml2)
            തിരികെ dev;
        എങ്കിൽ (!dev->hid_aml)
            തുടരുക;
        എങ്കിൽ (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            തിരികെ dev;
        എങ്കിൽ (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            തിരികെ dev;
    }
    തിരികെ ശൂന്യ;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ acpi_dsdt_present(ഘടന acpi_device *dev)
{
    എങ്കിൽ (!dev)
        തിരികെ 0; /* no */
    എങ്കിൽ (!dev->sta_aml)
        തിരികെ 1; /* yes */
    എങ്കിൽ (dev->sta_aml[0] == 0x14)
        തിരികെ -1; /* unknown (can't evaluate method) */
    എങ്കിൽ (dev->sta_aml[0] == 0x08) {
        u64 മൂല്യം = 0;
        parse_termarg_int(dev->sta_aml + 5, ശൂന്യ, &മൂല്യം);
        എങ്കിൽ (മൂല്യം == 0)
            തിരികെ 0; /* no */
        അല്ലെങ്കിൽ
            തിരികെ 1; /* yes */
    }
    തിരികെ -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

ഘടന acpi_device *acpi_dsdt_find_string(ഘടന acpi_device *prev,
                                          സ്ഥിരം അക്ഷരം *hid)
{
    എങ്കിൽ (!CONFIG_ACPI_PARSE)
        തിരികെ ശൂന്യ;

    u8 aml[10];
    പൂർണ്ണസംഖ്യ len = snprintf((അക്ഷരം*)aml, വലുപ്പം(aml), "\x0d%s", hid);
    തിരികെ acpi_dsdt_find(prev, aml, len, ശൂന്യ, 0);
}

ഘടന acpi_device *acpi_dsdt_find_eisaid(ഘടന acpi_device *prev, u16 eisaid)
{
    എങ്കിൽ (!CONFIG_ACPI_PARSE)
        തിരികെ ശൂന്യ;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    പൂർണ്ണസംഖ്യ len2 = snprintf((അക്ഷരം*)aml2, വലുപ്പം(aml2), "\x0dPNP%04X", eisaid);
    തിരികെ acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

അക്ഷരം *acpi_dsdt_name(ഘടന acpi_device *dev)
{
    എങ്കിൽ (!CONFIG_ACPI_PARSE || !dev)
        തിരികെ ശൂന്യ;
    തിരികെ dev->name;
}

പൂർണ്ണസംഖ്യ acpi_dsdt_find_io(ഘടന acpi_device *dev, u64 *min, u64 *max)
{
    എങ്കിൽ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        തിരികെ -1;
    തിരികെ find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

പൂർണ്ണസംഖ്യ acpi_dsdt_find_mem(ഘടന acpi_device *dev, u64 *min, u64 *max)
{
    എങ്കിൽ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        തിരികെ -1;
    തിരികെ find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

പൂർണ്ണസംഖ്യ acpi_dsdt_find_irq(ഘടന acpi_device *dev, u64 *irq)
{
    u64 max;
    എങ്കിൽ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        തിരികെ -1;
    തിരികെ find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

പൂർണ്ണസംഖ്യ acpi_dsdt_present_eisaid(u16 eisaid)
{
    എങ്കിൽ (!CONFIG_ACPI_PARSE)
        തിരികെ -1; /* unknown */
    എങ്കിൽ (hlist_empty(&acpi_devices))
        തിരികെ -1; /* unknown (no dsdt table) */

    ഘടന acpi_device *dev = acpi_dsdt_find_eisaid(ശൂന്യ, eisaid);
    തിരികെ acpi_dsdt_present(dev);
}

ശൂന്യം acpi_dsdt_parse(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_ACPI_PARSE)
        തിരികെ;

    ഘടന fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    എങ്കിൽ (!fadt)
        തിരികെ;
    u8 *dsdt = (ശൂന്യം*)(fadt->dsdt);
    എങ്കിൽ (!dsdt)
        തിരികെ;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    ഘടന parse_state s;
    memset(&s, 0, വലുപ്പം(s));
    parse_termlist(&s, dsdt, offset, length);

    എങ്കിൽ (!parse_dumpdevs)
        തിരികെ;

    ഘടന acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    വേണ്ടി (dev = acpi_dsdt_find(ശൂന്യ, ശൂന്യ, 0, ശൂന്യ, 0);
         dev != ശൂന്യ;
         dev = acpi_dsdt_find(dev, ശൂന്യ, 0, ശൂന്യ, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        എങ്കിൽ (dev->hid_aml)
            dprintf(1, ", hid");
        എങ്കിൽ (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        എങ്കിൽ (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        എങ്കിൽ (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
