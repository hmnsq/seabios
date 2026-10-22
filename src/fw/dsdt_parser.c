// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "list.h"   // hlist_*
#अन्तर्भाव "malloc.h" // malloc_*
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "util.h"
#अन्तर्भाव "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

संरचना acpi_device {
    संरचना hlist_गाँठ गाँठ;
    वर्ण name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    पूर्णाङ्क crs_size;
};
स्थिर संरचना hlist_head acpi_devices VARVERIFY32INIT;
स्थिर ध्रुव पूर्णाङ्क parse_dumpdevs = 0;

संरचना parse_state {
    वर्ण name[32];
    संरचना acpi_device *dev;
    पूर्णाङ्क error;
    पूर्णाङ्क depth;
};

स्थिर शून्य parse_termlist(संरचना parse_state *s,
                           u8 *ptr, पूर्णाङ्क offset, पूर्णाङ्क pkglength);

स्थिर शून्य hex(ध्रुव u8 *ptr, पूर्णाङ्क गिनती, पूर्णाङ्क lvl, ध्रुव वर्ण *item)
{
    पूर्णाङ्क l = 0, i;

    कुरु {
        dprintf(lvl, "%s: %04x:  ", item, l);
        प्रति (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        प्रति (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } यावत् (l < गिनती);
}

स्थिर u64 parse_resource_int(u8 *ptr, पूर्णाङ्क गिनती)
{
    u64 मूल्य = 0;
    पूर्णाङ्क index = 0;

    प्रति (index = 0; index < गिनती; index++)
        मूल्य |= (u64)ptr[index] << (index * 8);
    प्रत्यागम मूल्य;
}

स्थिर पूर्णाङ्क parse_resource_bit(u8 *ptr, पूर्णाङ्क गिनती)
{
    पूर्णाङ्क bit;

    प्रति (bit = 0; bit < गिनती*8; bit++)
        यदि (ptr[bit/8] & (1 << (bit%8)))
            प्रत्यागम bit;
    प्रत्यागम 0;
}

स्थिर पूर्णाङ्क parse_resource(u8 *ptr, पूर्णाङ्क length, पूर्णाङ्क *type, u64 *min, u64 *max)
{
    पूर्णाङ्क rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    यदि (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        विकल्प (rname) {
        पक्ष 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            भङ्ग;
        पक्ष 0x0f: /* end marker */
            प्रत्यागम 0;
        पक्ष 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            यदि (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            भङ्ग;
        पक्ष 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            भङ्ग;
        स्वभाव:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            भङ्ग;
        }
    } अन्यथा {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        विकल्प (rname) {
        पक्ष 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            भङ्ग;
        पक्ष 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            भङ्ग;
        पक्ष 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            भङ्ग;
        पक्ष 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            भङ्ग;
        पक्ष 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            भङ्ग;
        स्वभाव:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            भङ्ग;
        }
    }
    प्रत्यागम rsize;
}

स्थिर पूर्णाङ्क find_resource(u8 *ptr, पूर्णाङ्क len, पूर्णाङ्क kind, u64 *min, u64 *max)
{
    पूर्णाङ्क type, size, offset = 0;

    कुरु {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        यदि (kind == type)
            प्रत्यागम 0;
        offset += size;
    } यावत् (size > 0 && offset < len);
    प्रत्यागम -1;
}

स्थिर पूर्णाङ्क print_resources(ध्रुव वर्ण *prefix, u8 *ptr, पूर्णाङ्क len)
{
    स्थिर ध्रुव वर्ण *typename[] = { "mem", "i/o", "bus" };
    पूर्णाङ्क type, size, offset = 0;
    u64 min, max;

    कुरु {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        विकल्प (type) {
        पक्ष 0:
        पक्ष 1:
        पक्ष 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            भङ्ग;
        पक्ष 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            भङ्ग;
        }
        offset += size;
    } यावत् (size > 0 && offset < len);
    प्रत्यागम -1;
}

स्थिर पूर्णाङ्क parse_nameseg(u8 *ptr, वर्ण **dst)
{
    यदि (dst && *dst) {
        *(dst[0]++) = ptr[0];
        यदि (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        यदि (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        यदि (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    प्रत्यागम 4;
}

स्थिर पूर्णाङ्क parse_namestring(संरचना parse_state *s,
                            u8 *ptr, ध्रुव वर्ण *item)
{
    वर्ण *dst = s->name;
    पूर्णाङ्क offset = 0;
    पूर्णाङ्क i, गिनती;

    प्रति (;;) {
        विकल्प (ptr[offset]) {
        पक्ष 0: /* null name */
            offset++;
            *(dst++) = 0;
            भङ्ग;
        पक्ष 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            भङ्ग;
        पक्ष 0x2f:
            offset++;
            गिनती = ptr[offset];
            offset++;
            प्रति (i = 0; i < गिनती; i++) {
                यदि (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            भङ्ग;
        पक्ष '\\':
            *(dst++) = '\\';
            offset++;
            अनुवर्त;
        पक्ष '^':
            *(dst++) = '^';
            offset++;
            अनुवर्त;
        पक्ष 'A' ... 'Z':
        पक्ष '_':
            offset += parse_nameseg(ptr, &dst);
            भङ्ग;
        स्वभाव:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            भङ्ग;
        }
        भङ्ग;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    प्रत्यागम offset;
}

स्थिर पूर्णाङ्क parse_termarg_int(u8 *ptr, पूर्णाङ्क *error, u64 *dst)
{
    u64 मूल्य;
    पूर्णाङ्क offset = 1;

    विकल्प (ptr[0]) {
    पक्ष 0x00: /* zero */
        मूल्य = 0;
        भङ्ग;
    पक्ष 0x01: /* one */
        मूल्य = 1;
        भङ्ग;
    पक्ष 0x0a: /* byte prefix */
        मूल्य = ptr[1];
        offset++;
        भङ्ग;
    पक्ष 0x0b: /* word prefix */
        मूल्य = ptr[1] |
            ((अचिन्हित दीर्घ)ptr[2] << 8);
        offset += 2;
        भङ्ग;
    पक्ष 0x0c: /* dword prefix */
        मूल्य = ptr[1] |
            ((अचिन्हित दीर्घ)ptr[2] << 8) |
            ((अचिन्हित दीर्घ)ptr[3] << 16) |
            ((अचिन्हित दीर्घ)ptr[4] << 24);
        offset += 4;
        भङ्ग;
    स्वभाव:
        मूल्य = 0;
        hex(ptr, 16, 3, __func__);
        यदि (error)
            *error = 1;
        भङ्ग;
    }

    यदि (dst)
        *dst = मूल्य;
    dprintf(5, "%s: 0x%llx\n", __func__, मूल्य);
    प्रत्यागम offset;
}

स्थिर पूर्णाङ्क parse_pkglength(u8 *ptr, पूर्णाङ्क *pkglength)
{
    पूर्णाङ्क offset = 2;

    *pkglength = 0;
    विकल्प (ptr[0] >> 6) {
    पक्ष 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    पक्ष 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    पक्ष 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        प्रत्यागम offset;
    पक्ष 0:
    स्वभाव:
        *pkglength |= ptr[0] & 0x3f;
        प्रत्यागम 1;
    }
}

स्थिर पूर्णाङ्क parse_pkg_common(संरचना parse_state *s,
                            u8 *ptr, ध्रुव वर्ण *item, पूर्णाङ्क *pkglength)
{
    पूर्णाङ्क offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    प्रत्यागम offset;
}

स्थिर पूर्णाङ्क parse_pkg_scope(संरचना parse_state *s,
                           u8 *ptr)
{
    पूर्णाङ्क offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    प्रत्यागम pkglength;
}

स्थिर पूर्णाङ्क parse_pkg_device(संरचना parse_state *s,
                            u8 *ptr)
{
    पूर्णाङ्क offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(परिमाण(संरचना acpi_device));
    यदि (!s->dev) {
        warn_noalloc();
        s->error = 1;
        प्रत्यागम pkglength;
    }

    स_रखो(s->dev, 0, परिमाण(संरचना acpi_device));
    hlist_add_head(&s->dev->गाँठ, &acpi_devices);
    strtcpy(s->dev->name, s->name, परिमाण(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = रिक्त;

    प्रत्यागम pkglength;
}

स्थिर पूर्णाङ्क parse_pkg_buffer(संरचना parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    पूर्णाङ्क pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    यदि (s->dev && म_भेद(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    प्रत्यागम pkglength;
}

स्थिर पूर्णाङ्क parse_pkg_skip(संरचना parse_state *s,
                          u8 *ptr, पूर्णाङ्क op, पूर्णाङ्क name)
{
    पूर्णाङ्क pkglength, offset;
    वर्ण item[8];

    snprintf(item, परिमाण(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    यदि (name) {
        parse_namestring(s, ptr + offset, item);
    } अन्यथा {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    प्रत्यागम pkglength;
}

स्थिर पूर्णाङ्क parse_termobj(संरचना parse_state *s,
                         u8 *ptr)
{
    पूर्णाङ्क offset = 1;

    यदि (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        प्रत्यागम offset;
    }

    s->depth++;
    विकल्प (ptr[0]) {
    पक्ष 0x00: /* zero */
        भङ्ग;
    पक्ष 0x01: /* one */
        भङ्ग;
    पक्ष 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        भङ्ग;
    पक्ष 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        यदि (s->dev && म_भेद(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        यदि (s->dev && म_भेद(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        भङ्ग;
    पक्ष 0x0a: /* byte prefix */
        offset++;
        भङ्ग;
    पक्ष 0x0b: /* word prefix */
        offset += 2;
        भङ्ग;
    पक्ष 0x0c: /* dword prefix */
        offset += 4;
        भङ्ग;
    पक्ष 0x0d: /* string prefix */
        यावत् (ptr[offset])
            offset++;
        offset++;
        भङ्ग;
    पक्ष 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        भङ्ग;
    पक्ष 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        भङ्ग;
    पक्ष 0x12: /* package op */
    पक्ष 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        भङ्ग;
    पक्ष 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        यदि (s->dev && म_भेद(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        भङ्ग;
    पक्ष 0x5b: /* ext op prefix */
        offset++;
        विकल्प (ptr[1]) {
        पक्ष 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            भङ्ग;
        पक्ष 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, रिक्त);
            offset += parse_termarg_int(ptr + offset, &s->error, रिक्त);
            भङ्ग;
        पक्ष 0x81: /* field op */
        पक्ष 0x83: /* processor op */
        पक्ष 0x84: /* power resource op */
        पक्ष 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            भङ्ग;
        पक्ष 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            भङ्ग;
        स्वभाव:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            भङ्ग;
        }
        भङ्ग;
    स्वभाव:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        भङ्ग;
    }
    s->depth--;

    प्रत्यागम offset;
}

स्थिर शून्य parse_termlist(संरचना parse_state *s,
                           u8 *ptr, पूर्णाङ्क offset, पूर्णाङ्क pkglength)
{
    प्रति (;;) {
        offset += parse_termobj(s, ptr + offset);
        यदि (offset == pkglength)
            प्रत्यागम;
        यदि (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            प्रत्यागम;
        }
        यदि (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            प्रत्यागम;
        }
    }
}

स्थिर संरचना acpi_device *acpi_dsdt_find(संरचना acpi_device *prev,
                                          ध्रुव u8 *aml1, पूर्णाङ्क size1,
                                          ध्रुव u8 *aml2, पूर्णाङ्क size2)
{
    संरचना acpi_device *dev;
    संरचना hlist_गाँठ *गाँठ;

    यदि (!prev)
        गाँठ = acpi_devices.first;
    अन्यथा
        गाँठ = prev->गाँठ.next;

    प्रति (; गाँठ != रिक्त; गाँठ = dev->गाँठ.next) {
        dev = container_of(गाँठ, संरचना acpi_device, गाँठ);
        यदि (!aml1 && !aml2)
            प्रत्यागम dev;
        यदि (!dev->hid_aml)
            अनुवर्त;
        यदि (aml1 && स_भेद(dev->hid_aml + 5, aml1, size1) == 0)
            प्रत्यागम dev;
        यदि (aml2 && स_भेद(dev->hid_aml + 5, aml2, size2) == 0)
            प्रत्यागम dev;
    }
    प्रत्यागम रिक्त;
}

स्थिर पूर्णाङ्क acpi_dsdt_present(संरचना acpi_device *dev)
{
    यदि (!dev)
        प्रत्यागम 0; /* no */
    यदि (!dev->sta_aml)
        प्रत्यागम 1; /* yes */
    यदि (dev->sta_aml[0] == 0x14)
        प्रत्यागम -1; /* unknown (can't evaluate method) */
    यदि (dev->sta_aml[0] == 0x08) {
        u64 मूल्य = 0;
        parse_termarg_int(dev->sta_aml + 5, रिक्त, &मूल्य);
        यदि (मूल्य == 0)
            प्रत्यागम 0; /* no */
        अन्यथा
            प्रत्यागम 1; /* yes */
    }
    प्रत्यागम -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

संरचना acpi_device *acpi_dsdt_find_string(संरचना acpi_device *prev,
                                          ध्रुव वर्ण *hid)
{
    यदि (!CONFIG_ACPI_PARSE)
        प्रत्यागम रिक्त;

    u8 aml[10];
    पूर्णाङ्क len = snprintf((वर्ण*)aml, परिमाण(aml), "\x0d%s", hid);
    प्रत्यागम acpi_dsdt_find(prev, aml, len, रिक्त, 0);
}

संरचना acpi_device *acpi_dsdt_find_eisaid(संरचना acpi_device *prev, u16 eisaid)
{
    यदि (!CONFIG_ACPI_PARSE)
        प्रत्यागम रिक्त;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    पूर्णाङ्क len2 = snprintf((वर्ण*)aml2, परिमाण(aml2), "\x0dPNP%04X", eisaid);
    प्रत्यागम acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

वर्ण *acpi_dsdt_name(संरचना acpi_device *dev)
{
    यदि (!CONFIG_ACPI_PARSE || !dev)
        प्रत्यागम रिक्त;
    प्रत्यागम dev->name;
}

पूर्णाङ्क acpi_dsdt_find_io(संरचना acpi_device *dev, u64 *min, u64 *max)
{
    यदि (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        प्रत्यागम -1;
    प्रत्यागम find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

पूर्णाङ्क acpi_dsdt_find_mem(संरचना acpi_device *dev, u64 *min, u64 *max)
{
    यदि (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        प्रत्यागम -1;
    प्रत्यागम find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

पूर्णाङ्क acpi_dsdt_find_irq(संरचना acpi_device *dev, u64 *irq)
{
    u64 max;
    यदि (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        प्रत्यागम -1;
    प्रत्यागम find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

पूर्णाङ्क acpi_dsdt_present_eisaid(u16 eisaid)
{
    यदि (!CONFIG_ACPI_PARSE)
        प्रत्यागम -1; /* unknown */
    यदि (hlist_empty(&acpi_devices))
        प्रत्यागम -1; /* unknown (no dsdt table) */

    संरचना acpi_device *dev = acpi_dsdt_find_eisaid(रिक्त, eisaid);
    प्रत्यागम acpi_dsdt_present(dev);
}

शून्य acpi_dsdt_parse(शून्य)
{
    यदि (!CONFIG_ACPI_PARSE)
        प्रत्यागम;

    संरचना fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    यदि (!fadt)
        प्रत्यागम;
    u8 *dsdt = (शून्य*)(fadt->dsdt);
    यदि (!dsdt)
        प्रत्यागम;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    संरचना parse_state s;
    स_रखो(&s, 0, परिमाण(s));
    parse_termlist(&s, dsdt, offset, length);

    यदि (!parse_dumpdevs)
        प्रत्यागम;

    संरचना acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    प्रति (dev = acpi_dsdt_find(रिक्त, रिक्त, 0, रिक्त, 0);
         dev != रिक्त;
         dev = acpi_dsdt_find(dev, रिक्त, 0, रिक्त, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        यदि (dev->hid_aml)
            dprintf(1, ", hid");
        यदि (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        यदि (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        यदि (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
