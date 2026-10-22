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
    संरचना hlist_गाठ गाठ;
    अक्षर name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_data;
    पूर्णांक crs_size;
};
स्थिर संरचना hlist_head acpi_devices VARVERIFY32INIT;
स्थिर स्थिरांक पूर्णांक parse_dumpdevs = 0;

संरचना parse_state {
    अक्षर name[32];
    संरचना acpi_device *dev;
    पूर्णांक error;
    पूर्णांक depth;
};

स्थिर शून्य parse_termlist(संरचना parse_state *s,
                           u8 *ptr, पूर्णांक offset, पूर्णांक pkglength);

स्थिर शून्य hex(स्थिरांक u8 *ptr, पूर्णांक मोजणी, पूर्णांक lvl, स्थिरांक अक्षर *item)
{
    पूर्णांक l = 0, i;

    करा {
        dprintf(lvl, "%s: %04x:  ", item, l);
        साठी (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        साठी (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } पर्यंत (l < मोजणी);
}

स्थिर u64 parse_resource_int(u8 *ptr, पूर्णांक मोजणी)
{
    u64 मूल्य = 0;
    पूर्णांक index = 0;

    साठी (index = 0; index < मोजणी; index++)
        मूल्य |= (u64)ptr[index] << (index * 8);
    परत मूल्य;
}

स्थिर पूर्णांक parse_resource_bit(u8 *ptr, पूर्णांक मोजणी)
{
    पूर्णांक bit;

    साठी (bit = 0; bit < मोजणी*8; bit++)
        जर (ptr[bit/8] & (1 << (bit%8)))
            परत bit;
    परत 0;
}

स्थिर पूर्णांक parse_resource(u8 *ptr, पूर्णांक length, पूर्णांक *type, u64 *min, u64 *max)
{
    पूर्णांक rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    जर (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        निवड (rname) {
        स्थिती 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            थांबा;
        स्थिती 0x0f: /* end marker */
            परत 0;
        स्थिती 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            जर (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            थांबा;
        स्थिती 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            थांबा;
        पूर्वनिर्धारित:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            थांबा;
        }
    } अन्यथा {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        निवड (rname) {
        स्थिती 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            थांबा;
        स्थिती 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            थांबा;
        स्थिती 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            थांबा;
        स्थिती 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            थांबा;
        स्थिती 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            थांबा;
        पूर्वनिर्धारित:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            थांबा;
        }
    }
    परत rsize;
}

स्थिर पूर्णांक find_resource(u8 *ptr, पूर्णांक len, पूर्णांक kind, u64 *min, u64 *max)
{
    पूर्णांक type, size, offset = 0;

    करा {
        size = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        जर (kind == type)
            परत 0;
        offset += size;
    } पर्यंत (size > 0 && offset < len);
    परत -1;
}

स्थिर पूर्णांक print_resources(स्थिरांक अक्षर *prefix, u8 *ptr, पूर्णांक len)
{
    स्थिर स्थिरांक अक्षर *typename[] = { "mem", "i/o", "bus" };
    पूर्णांक type, size, offset = 0;
    u64 min, max;

    करा {
        size = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        निवड (type) {
        स्थिती 0:
        स्थिती 1:
        स्थिती 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            थांबा;
        स्थिती 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            थांबा;
        }
        offset += size;
    } पर्यंत (size > 0 && offset < len);
    परत -1;
}

स्थिर पूर्णांक parse_nameseg(u8 *ptr, अक्षर **dst)
{
    जर (dst && *dst) {
        *(dst[0]++) = ptr[0];
        जर (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        जर (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        जर (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    परत 4;
}

स्थिर पूर्णांक parse_namestring(संरचना parse_state *s,
                            u8 *ptr, स्थिरांक अक्षर *item)
{
    अक्षर *dst = s->name;
    पूर्णांक offset = 0;
    पूर्णांक i, मोजणी;

    साठी (;;) {
        निवड (ptr[offset]) {
        स्थिती 0: /* null name */
            offset++;
            *(dst++) = 0;
            थांबा;
        स्थिती 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            थांबा;
        स्थिती 0x2f:
            offset++;
            मोजणी = ptr[offset];
            offset++;
            साठी (i = 0; i < मोजणी; i++) {
                जर (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            थांबा;
        स्थिती '\\':
            *(dst++) = '\\';
            offset++;
            सुरूठेवा;
        स्थिती '^':
            *(dst++) = '^';
            offset++;
            सुरूठेवा;
        स्थिती 'A' ... 'Z':
        स्थिती '_':
            offset += parse_nameseg(ptr, &dst);
            थांबा;
        पूर्वनिर्धारित:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            थांबा;
        }
        थांबा;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    परत offset;
}

स्थिर पूर्णांक parse_termarg_int(u8 *ptr, पूर्णांक *error, u64 *dst)
{
    u64 मूल्य;
    पूर्णांक offset = 1;

    निवड (ptr[0]) {
    स्थिती 0x00: /* zero */
        मूल्य = 0;
        थांबा;
    स्थिती 0x01: /* one */
        मूल्य = 1;
        थांबा;
    स्थिती 0x0a: /* byte prefix */
        मूल्य = ptr[1];
        offset++;
        थांबा;
    स्थिती 0x0b: /* word prefix */
        मूल्य = ptr[1] |
            ((अचिन्हित दीर्घ)ptr[2] << 8);
        offset += 2;
        थांबा;
    स्थिती 0x0c: /* dword prefix */
        मूल्य = ptr[1] |
            ((अचिन्हित दीर्घ)ptr[2] << 8) |
            ((अचिन्हित दीर्घ)ptr[3] << 16) |
            ((अचिन्हित दीर्घ)ptr[4] << 24);
        offset += 4;
        थांबा;
    पूर्वनिर्धारित:
        मूल्य = 0;
        hex(ptr, 16, 3, __func__);
        जर (error)
            *error = 1;
        थांबा;
    }

    जर (dst)
        *dst = मूल्य;
    dprintf(5, "%s: 0x%llx\n", __func__, मूल्य);
    परत offset;
}

स्थिर पूर्णांक parse_pkglength(u8 *ptr, पूर्णांक *pkglength)
{
    पूर्णांक offset = 2;

    *pkglength = 0;
    निवड (ptr[0] >> 6) {
    स्थिती 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    स्थिती 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    स्थिती 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        परत offset;
    स्थिती 0:
    पूर्वनिर्धारित:
        *pkglength |= ptr[0] & 0x3f;
        परत 1;
    }
}

स्थिर पूर्णांक parse_pkg_common(संरचना parse_state *s,
                            u8 *ptr, स्थिरांक अक्षर *item, पूर्णांक *pkglength)
{
    पूर्णांक offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    परत offset;
}

स्थिर पूर्णांक parse_pkg_scope(संरचना parse_state *s,
                           u8 *ptr)
{
    पूर्णांक offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    परत pkglength;
}

स्थिर पूर्णांक parse_pkg_device(संरचना parse_state *s,
                            u8 *ptr)
{
    पूर्णांक offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(आकार(संरचना acpi_device));
    जर (!s->dev) {
        warn_noalloc();
        s->error = 1;
        परत pkglength;
    }

    स_रखो(s->dev, 0, आकार(संरचना acpi_device));
    hlist_add_head(&s->dev->गाठ, &acpi_devices);
    strtcpy(s->dev->name, s->name, आकार(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = रिक्त;

    परत pkglength;
}

स्थिर पूर्णांक parse_pkg_buffer(संरचना parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    पूर्णांक pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    जर (s->dev && म_भेद(s->name, "_CRS") == 0) {
        s->dev->crs_data = ptr + offset;
        s->dev->crs_size = blen;
    }
    परत pkglength;
}

स्थिर पूर्णांक parse_pkg_skip(संरचना parse_state *s,
                          u8 *ptr, पूर्णांक op, पूर्णांक name)
{
    पूर्णांक pkglength, offset;
    अक्षर item[8];

    snprintf(item, आकार(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    जर (name) {
        parse_namestring(s, ptr + offset, item);
    } अन्यथा {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    परत pkglength;
}

स्थिर पूर्णांक parse_termobj(संरचना parse_state *s,
                         u8 *ptr)
{
    पूर्णांक offset = 1;

    जर (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        परत offset;
    }

    s->depth++;
    निवड (ptr[0]) {
    स्थिती 0x00: /* zero */
        थांबा;
    स्थिती 0x01: /* one */
        थांबा;
    स्थिती 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        थांबा;
    स्थिती 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        जर (s->dev && म_भेद(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        जर (s->dev && म_भेद(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        थांबा;
    स्थिती 0x0a: /* byte prefix */
        offset++;
        थांबा;
    स्थिती 0x0b: /* word prefix */
        offset += 2;
        थांबा;
    स्थिती 0x0c: /* dword prefix */
        offset += 4;
        थांबा;
    स्थिती 0x0d: /* string prefix */
        पर्यंत (ptr[offset])
            offset++;
        offset++;
        थांबा;
    स्थिती 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        थांबा;
    स्थिती 0x11: /* buffer op */
        offset += parse_pkg_buffer(s, ptr + offset);
        थांबा;
    स्थिती 0x12: /* package op */
    स्थिती 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        थांबा;
    स्थिती 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        जर (s->dev && म_भेद(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        थांबा;
    स्थिती 0x5b: /* ext op prefix */
        offset++;
        निवड (ptr[1]) {
        स्थिती 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            थांबा;
        स्थिती 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, रिक्त);
            offset += parse_termarg_int(ptr + offset, &s->error, रिक्त);
            थांबा;
        स्थिती 0x81: /* field op */
        स्थिती 0x83: /* processor op */
        स्थिती 0x84: /* power resource op */
        स्थिती 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            थांबा;
        स्थिती 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            थांबा;
        पूर्वनिर्धारित:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            थांबा;
        }
        थांबा;
    पूर्वनिर्धारित:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        थांबा;
    }
    s->depth--;

    परत offset;
}

स्थिर शून्य parse_termlist(संरचना parse_state *s,
                           u8 *ptr, पूर्णांक offset, पूर्णांक pkglength)
{
    साठी (;;) {
        offset += parse_termobj(s, ptr + offset);
        जर (offset == pkglength)
            परत;
        जर (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            परत;
        }
        जर (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            परत;
        }
    }
}

स्थिर संरचना acpi_device *acpi_dsdt_find(संरचना acpi_device *prev,
                                          स्थिरांक u8 *aml1, पूर्णांक size1,
                                          स्थिरांक u8 *aml2, पूर्णांक size2)
{
    संरचना acpi_device *dev;
    संरचना hlist_गाठ *गाठ;

    जर (!prev)
        गाठ = acpi_devices.first;
    अन्यथा
        गाठ = prev->गाठ.next;

    साठी (; गाठ != रिक्त; गाठ = dev->गाठ.next) {
        dev = container_of(गाठ, संरचना acpi_device, गाठ);
        जर (!aml1 && !aml2)
            परत dev;
        जर (!dev->hid_aml)
            सुरूठेवा;
        जर (aml1 && स_भेद(dev->hid_aml + 5, aml1, size1) == 0)
            परत dev;
        जर (aml2 && स_भेद(dev->hid_aml + 5, aml2, size2) == 0)
            परत dev;
    }
    परत रिक्त;
}

स्थिर पूर्णांक acpi_dsdt_present(संरचना acpi_device *dev)
{
    जर (!dev)
        परत 0; /* no */
    जर (!dev->sta_aml)
        परत 1; /* yes */
    जर (dev->sta_aml[0] == 0x14)
        परत -1; /* unknown (can't evaluate method) */
    जर (dev->sta_aml[0] == 0x08) {
        u64 मूल्य = 0;
        parse_termarg_int(dev->sta_aml + 5, रिक्त, &मूल्य);
        जर (मूल्य == 0)
            परत 0; /* no */
        अन्यथा
            परत 1; /* yes */
    }
    परत -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

संरचना acpi_device *acpi_dsdt_find_string(संरचना acpi_device *prev,
                                          स्थिरांक अक्षर *hid)
{
    जर (!CONFIG_ACPI_PARSE)
        परत रिक्त;

    u8 aml[10];
    पूर्णांक len = snprintf((अक्षर*)aml, आकार(aml), "\x0d%s", hid);
    परत acpi_dsdt_find(prev, aml, len, रिक्त, 0);
}

संरचना acpi_device *acpi_dsdt_find_eisaid(संरचना acpi_device *prev, u16 eisaid)
{
    जर (!CONFIG_ACPI_PARSE)
        परत रिक्त;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    पूर्णांक len2 = snprintf((अक्षर*)aml2, आकार(aml2), "\x0dPNP%04X", eisaid);
    परत acpi_dsdt_find(prev, aml1, 5, aml2, len2);
}

अक्षर *acpi_dsdt_name(संरचना acpi_device *dev)
{
    जर (!CONFIG_ACPI_PARSE || !dev)
        परत रिक्त;
    परत dev->name;
}

पूर्णांक acpi_dsdt_find_io(संरचना acpi_device *dev, u64 *min, u64 *max)
{
    जर (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        परत -1;
    परत find_resource(dev->crs_data, dev->crs_size,
                         1 /* I/O */, min, max);
}

पूर्णांक acpi_dsdt_find_mem(संरचना acpi_device *dev, u64 *min, u64 *max)
{
    जर (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        परत -1;
    परत find_resource(dev->crs_data, dev->crs_size,
                         0 /* mem */, min, max);
}

पूर्णांक acpi_dsdt_find_irq(संरचना acpi_device *dev, u64 *irq)
{
    u64 max;
    जर (!CONFIG_ACPI_PARSE || !dev || !dev->crs_data)
        परत -1;
    परत find_resource(dev->crs_data, dev->crs_size,
                         3 /* irq */, irq, &max);
}

पूर्णांक acpi_dsdt_present_eisaid(u16 eisaid)
{
    जर (!CONFIG_ACPI_PARSE)
        परत -1; /* unknown */
    जर (hlist_empty(&acpi_devices))
        परत -1; /* unknown (no dsdt table) */

    संरचना acpi_device *dev = acpi_dsdt_find_eisaid(रिक्त, eisaid);
    परत acpi_dsdt_present(dev);
}

शून्य acpi_dsdt_parse(शून्य)
{
    जर (!CONFIG_ACPI_PARSE)
        परत;

    संरचना fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    जर (!fadt)
        परत;
    u8 *dsdt = (शून्य*)(fadt->dsdt);
    जर (!dsdt)
        परत;

    u32 length = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, length);

    संरचना parse_state s;
    स_रखो(&s, 0, आकार(s));
    parse_termlist(&s, dsdt, offset, length);

    जर (!parse_dumpdevs)
        परत;

    संरचना acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    साठी (dev = acpi_dsdt_find(रिक्त, रिक्त, 0, रिक्त, 0);
         dev != रिक्त;
         dev = acpi_dsdt_find(dev, रिक्त, 0, रिक्त, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        जर (dev->hid_aml)
            dprintf(1, ", hid");
        जर (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        जर (dev->crs_data)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        जर (dev->crs_data)
            print_resources("        ", dev->crs_data, dev->crs_size);
    }
}
