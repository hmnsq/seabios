// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "list.h"   // hlist_*
#समावेश "malloc.h" // malloc_*
#समावेश "output.h" // dprintf
#समावेश "string.h" // memcpy
#समावेश "util.h"
#समावेश "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

काष्ठा acpi_device {
    काष्ठा hlist_गाँठ गाँठ;
    अक्षर name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_तथ्य;
    पूर्णांक crs_आकार;
};
अटल काष्ठा hlist_शीर्ष acpi_devices VARVERIFY32INIT;
अटल स्थिर पूर्णांक parse_dumpdevs = 0;

काष्ठा parse_state {
    अक्षर name[32];
    काष्ठा acpi_device *dev;
    पूर्णांक error;
    पूर्णांक depth;
};

अटल व्योम parse_termlist(काष्ठा parse_state *s,
                           u8 *ptr, पूर्णांक offset, पूर्णांक pkglength);

अटल व्योम hex(स्थिर u8 *ptr, पूर्णांक गिनती, पूर्णांक lvl, स्थिर अक्षर *item)
{
    पूर्णांक l = 0, i;

    करो {
        dprintf(lvl, "%s: %04x:  ", item, l);
        क्रम (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        क्रम (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } जबतक (l < गिनती);
}

अटल u64 parse_resource_int(u8 *ptr, पूर्णांक गिनती)
{
    u64 मूल्य = 0;
    पूर्णांक सूचकांक = 0;

    क्रम (सूचकांक = 0; सूचकांक < गिनती; सूचकांक++)
        मूल्य |= (u64)ptr[सूचकांक] << (सूचकांक * 8);
    वापस मूल्य;
}

अटल पूर्णांक parse_resource_bit(u8 *ptr, पूर्णांक गिनती)
{
    पूर्णांक bit;

    क्रम (bit = 0; bit < गिनती*8; bit++)
        अगर (ptr[bit/8] & (1 << (bit%8)))
            वापस bit;
    वापस 0;
}

अटल पूर्णांक parse_resource(u8 *ptr, पूर्णांक लंबाई, पूर्णांक *type, u64 *min, u64 *max)
{
    पूर्णांक rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    अगर (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        चयन (rname) {
        हाल 0x04: /* irq */
            *min = parse_resource_bit(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            अवरोध;
        हाल 0x0f: /* end marker */
            वापस 0;
        हाल 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            अगर (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            अवरोध;
        हाल 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            अवरोध;
        शेष:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            अवरोध;
        }
    } अन्यथा {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        चयन (rname) {
        हाल 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            अवरोध;
        हाल 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            अवरोध;
        हाल 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            अवरोध;
        हाल 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            अवरोध;
        हाल 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            अवरोध;
        शेष:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            अवरोध;
        }
    }
    वापस rsize;
}

अटल पूर्णांक find_resource(u8 *ptr, पूर्णांक len, पूर्णांक kind, u64 *min, u64 *max)
{
    पूर्णांक type, आकार, offset = 0;

    करो {
        आकार = parse_resource(ptr + offset, len - offset,
                              &type, min, max);
        अगर (kind == type)
            वापस 0;
        offset += आकार;
    } जबतक (आकार > 0 && offset < len);
    वापस -1;
}

अटल पूर्णांक print_resources(स्थिर अक्षर *prefix, u8 *ptr, पूर्णांक len)
{
    अटल स्थिर अक्षर *typename[] = { "mem", "i/o", "bus" };
    पूर्णांक type, आकार, offset = 0;
    u64 min, max;

    करो {
        आकार = parse_resource(ptr + offset, len - offset,
                              &type, &min, &max);
        चयन (type) {
        हाल 0:
        हाल 1:
        हाल 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    prefix, typename[type], min, max);
            अवरोध;
        हाल 3:
            dprintf(1, "%sirq %lld\n", prefix, min);
            अवरोध;
        }
        offset += आकार;
    } जबतक (आकार > 0 && offset < len);
    वापस -1;
}

अटल पूर्णांक parse_nameseg(u8 *ptr, अक्षर **dst)
{
    अगर (dst && *dst) {
        *(dst[0]++) = ptr[0];
        अगर (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        अगर (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        अगर (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    वापस 4;
}

अटल पूर्णांक parse_namestring(काष्ठा parse_state *s,
                            u8 *ptr, स्थिर अक्षर *item)
{
    अक्षर *dst = s->name;
    पूर्णांक offset = 0;
    पूर्णांक i, गिनती;

    क्रम (;;) {
        चयन (ptr[offset]) {
        हाल 0: /* null name */
            offset++;
            *(dst++) = 0;
            अवरोध;
        हाल 0x2e:
            offset++;
            offset += parse_nameseg(ptr + offset, &dst);
            *(dst++) = '.';
            offset += parse_nameseg(ptr + offset, &dst);
            अवरोध;
        हाल 0x2f:
            offset++;
            गिनती = ptr[offset];
            offset++;
            क्रम (i = 0; i < गिनती; i++) {
                अगर (i)
                    *(dst++) = '.';
                offset += parse_nameseg(ptr + offset, &dst);
            }
            अवरोध;
        हाल '\\':
            *(dst++) = '\\';
            offset++;
            जारी;
        हाल '^':
            *(dst++) = '^';
            offset++;
            जारी;
        हाल 'A' ... 'Z':
        हाल '_':
            offset += parse_nameseg(ptr, &dst);
            अवरोध;
        शेष:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            अवरोध;
        }
        अवरोध;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->depth,
            item, s->name);
    वापस offset;
}

अटल पूर्णांक parse_termarg_int(u8 *ptr, पूर्णांक *error, u64 *dst)
{
    u64 मूल्य;
    पूर्णांक offset = 1;

    चयन (ptr[0]) {
    हाल 0x00: /* zero */
        मूल्य = 0;
        अवरोध;
    हाल 0x01: /* one */
        मूल्य = 1;
        अवरोध;
    हाल 0x0a: /* byte prefix */
        मूल्य = ptr[1];
        offset++;
        अवरोध;
    हाल 0x0b: /* word prefix */
        मूल्य = ptr[1] |
            ((अचिन्हित दीर्घ)ptr[2] << 8);
        offset += 2;
        अवरोध;
    हाल 0x0c: /* dword prefix */
        मूल्य = ptr[1] |
            ((अचिन्हित दीर्घ)ptr[2] << 8) |
            ((अचिन्हित दीर्घ)ptr[3] << 16) |
            ((अचिन्हित दीर्घ)ptr[4] << 24);
        offset += 4;
        अवरोध;
    शेष:
        मूल्य = 0;
        hex(ptr, 16, 3, __func__);
        अगर (error)
            *error = 1;
        अवरोध;
    }

    अगर (dst)
        *dst = मूल्य;
    dprintf(5, "%s: 0x%llx\n", __func__, मूल्य);
    वापस offset;
}

अटल पूर्णांक parse_pkglength(u8 *ptr, पूर्णांक *pkglength)
{
    पूर्णांक offset = 2;

    *pkglength = 0;
    चयन (ptr[0] >> 6) {
    हाल 3:
        *pkglength |= ptr[3] << 20;
        offset++;
    हाल 2:
        *pkglength |= ptr[2] << 12;
        offset++;
    हाल 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        वापस offset;
    हाल 0:
    शेष:
        *pkglength |= ptr[0] & 0x3f;
        वापस 1;
    }
}

अटल पूर्णांक parse_pkg_common(काष्ठा parse_state *s,
                            u8 *ptr, स्थिर अक्षर *item, पूर्णांक *pkglength)
{
    पूर्णांक offset;

    offset = parse_pkglength(ptr, pkglength);
    offset += parse_namestring(s, ptr + offset, item);
    वापस offset;
}

अटल पूर्णांक parse_pkg_scope(काष्ठा parse_state *s,
                           u8 *ptr)
{
    पूर्णांक offset, pkglength;

    offset = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, offset, pkglength);
    वापस pkglength;
}

अटल पूर्णांक parse_pkg_device(काष्ठा parse_state *s,
                            u8 *ptr)
{
    पूर्णांक offset, pkglength;

    offset = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(माप(काष्ठा acpi_device));
    अगर (!s->dev) {
        warn_noalloc();
        s->error = 1;
        वापस pkglength;
    }

    स_रखो(s->dev, 0, माप(काष्ठा acpi_device));
    hlist_add_शीर्ष(&s->dev->गाँठ, &acpi_devices);
    strtcpy(s->dev->name, s->name, माप(s->name));
    parse_termlist(s, ptr, offset, pkglength);
    s->dev = शून्य;

    वापस pkglength;
}

अटल पूर्णांक parse_pkg_बफर(काष्ठा parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    पूर्णांक pkglength, offset;

    offset = parse_pkglength(ptr, &pkglength);
    offset += parse_termarg_int(ptr + offset, &s->error, &blen);
    अगर (s->dev && म_भेद(s->name, "_CRS") == 0) {
        s->dev->crs_तथ्य = ptr + offset;
        s->dev->crs_आकार = blen;
    }
    वापस pkglength;
}

अटल पूर्णांक parse_pkg_skip(काष्ठा parse_state *s,
                          u8 *ptr, पूर्णांक op, पूर्णांक name)
{
    पूर्णांक pkglength, offset;
    अक्षर item[8];

    snprintf(item, माप(item), "op %x", op);
    offset = parse_pkglength(ptr, &pkglength);
    अगर (name) {
        parse_namestring(s, ptr + offset, item);
    } अन्यथा {
        dprintf(5, "%s: %s (%d)\n", __func__, item, pkglength);
    }
    वापस pkglength;
}

अटल पूर्णांक parse_termobj(काष्ठा parse_state *s,
                         u8 *ptr)
{
    पूर्णांक offset = 1;

    अगर (s->depth == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        वापस offset;
    }

    s->depth++;
    चयन (ptr[0]) {
    हाल 0x00: /* zero */
        अवरोध;
    हाल 0x01: /* one */
        अवरोध;
    हाल 0x06: /* AliasOp */
        offset += parse_namestring(s, ptr + offset, "SourceObject");
        offset += parse_namestring(s, ptr + offset, "AliasObject");
        अवरोध;
    हाल 0x08: /* name op */
        offset += parse_namestring(s, ptr + offset, "name");
        offset += parse_termobj(s, ptr + offset);
        अगर (s->dev && म_भेद(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        अगर (s->dev && म_भेद(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        अवरोध;
    हाल 0x0a: /* byte prefix */
        offset++;
        अवरोध;
    हाल 0x0b: /* word prefix */
        offset += 2;
        अवरोध;
    हाल 0x0c: /* dword prefix */
        offset += 4;
        अवरोध;
    हाल 0x0d: /* string prefix */
        जबतक (ptr[offset])
            offset++;
        offset++;
        अवरोध;
    हाल 0x10: /* scope op */
        offset += parse_pkg_scope(s, ptr + offset);
        अवरोध;
    हाल 0x11: /* buffer op */
        offset += parse_pkg_बफर(s, ptr + offset);
        अवरोध;
    हाल 0x12: /* package op */
    हाल 0x13: /* var package op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 0);
        अवरोध;
    हाल 0x14: /* method op */
        offset += parse_pkg_skip(s, ptr + offset, ptr[0], 1);
        अगर (s->dev && म_भेद(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        अवरोध;
    हाल 0x5b: /* ext op prefix */
        offset++;
        चयन (ptr[1]) {
        हाल 0x01: /* mutex op */
            offset += parse_namestring(s, ptr + offset, "mutex");
            offset++; /* sync flags */
            अवरोध;
        हाल 0x80: /* op region op */
            offset += parse_namestring(s, ptr + offset, "op region");
            offset++; /* region space */
            offset += parse_termarg_int(ptr + offset, &s->error, शून्य);
            offset += parse_termarg_int(ptr + offset, &s->error, शून्य);
            अवरोध;
        हाल 0x81: /* field op */
        हाल 0x83: /* processor op */
        हाल 0x84: /* power resource op */
        हाल 0x85: /* thermal zone op */
            offset += parse_pkg_skip(s, ptr + offset, 0x5b00 | ptr[1], 1);
            अवरोध;
        हाल 0x82: /* device op */
            offset += parse_pkg_device(s, ptr + offset);
            अवरोध;
        शेष:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            अवरोध;
        }
        अवरोध;
    शेष:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        अवरोध;
    }
    s->depth--;

    वापस offset;
}

अटल व्योम parse_termlist(काष्ठा parse_state *s,
                           u8 *ptr, पूर्णांक offset, पूर्णांक pkglength)
{
    क्रम (;;) {
        offset += parse_termobj(s, ptr + offset);
        अगर (offset == pkglength)
            वापस;
        अगर (offset > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 1;
            वापस;
        }
        अगर (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    offset, pkglength);
            s->error = 0;
            वापस;
        }
    }
}

अटल काष्ठा acpi_device *acpi_dsdt_find(काष्ठा acpi_device *पूर्व,
                                          स्थिर u8 *aml1, पूर्णांक size1,
                                          स्थिर u8 *aml2, पूर्णांक size2)
{
    काष्ठा acpi_device *dev;
    काष्ठा hlist_गाँठ *गाँठ;

    अगर (!पूर्व)
        गाँठ = acpi_devices.first;
    अन्यथा
        गाँठ = पूर्व->गाँठ.अग्र;

    क्रम (; गाँठ != शून्य; गाँठ = dev->गाँठ.अग्र) {
        dev = container_of(गाँठ, काष्ठा acpi_device, गाँठ);
        अगर (!aml1 && !aml2)
            वापस dev;
        अगर (!dev->hid_aml)
            जारी;
        अगर (aml1 && स_भेद(dev->hid_aml + 5, aml1, size1) == 0)
            वापस dev;
        अगर (aml2 && स_भेद(dev->hid_aml + 5, aml2, size2) == 0)
            वापस dev;
    }
    वापस शून्य;
}

अटल पूर्णांक acpi_dsdt_present(काष्ठा acpi_device *dev)
{
    अगर (!dev)
        वापस 0; /* no */
    अगर (!dev->sta_aml)
        वापस 1; /* yes */
    अगर (dev->sta_aml[0] == 0x14)
        वापस -1; /* unknown (can't evaluate method) */
    अगर (dev->sta_aml[0] == 0x08) {
        u64 मूल्य = 0;
        parse_termarg_int(dev->sta_aml + 5, शून्य, &मूल्य);
        अगर (मूल्य == 0)
            वापस 0; /* no */
        अन्यथा
            वापस 1; /* yes */
    }
    वापस -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

काष्ठा acpi_device *acpi_dsdt_find_string(काष्ठा acpi_device *पूर्व,
                                          स्थिर अक्षर *hid)
{
    अगर (!CONFIG_ACPI_PARSE)
        वापस शून्य;

    u8 aml[10];
    पूर्णांक len = snprintf((अक्षर*)aml, माप(aml), "\x0d%s", hid);
    वापस acpi_dsdt_find(पूर्व, aml, len, शून्य, 0);
}

काष्ठा acpi_device *acpi_dsdt_find_eisaid(काष्ठा acpi_device *पूर्व, u16 eisaid)
{
    अगर (!CONFIG_ACPI_PARSE)
        वापस शून्य;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    पूर्णांक len2 = snprintf((अक्षर*)aml2, माप(aml2), "\x0dPNP%04X", eisaid);
    वापस acpi_dsdt_find(पूर्व, aml1, 5, aml2, len2);
}

अक्षर *acpi_dsdt_name(काष्ठा acpi_device *dev)
{
    अगर (!CONFIG_ACPI_PARSE || !dev)
        वापस शून्य;
    वापस dev->name;
}

पूर्णांक acpi_dsdt_find_io(काष्ठा acpi_device *dev, u64 *min, u64 *max)
{
    अगर (!CONFIG_ACPI_PARSE || !dev || !dev->crs_तथ्य)
        वापस -1;
    वापस find_resource(dev->crs_तथ्य, dev->crs_आकार,
                         1 /* I/O */, min, max);
}

पूर्णांक acpi_dsdt_find_mem(काष्ठा acpi_device *dev, u64 *min, u64 *max)
{
    अगर (!CONFIG_ACPI_PARSE || !dev || !dev->crs_तथ्य)
        वापस -1;
    वापस find_resource(dev->crs_तथ्य, dev->crs_आकार,
                         0 /* mem */, min, max);
}

पूर्णांक acpi_dsdt_find_irq(काष्ठा acpi_device *dev, u64 *irq)
{
    u64 max;
    अगर (!CONFIG_ACPI_PARSE || !dev || !dev->crs_तथ्य)
        वापस -1;
    वापस find_resource(dev->crs_तथ्य, dev->crs_आकार,
                         3 /* irq */, irq, &max);
}

पूर्णांक acpi_dsdt_present_eisaid(u16 eisaid)
{
    अगर (!CONFIG_ACPI_PARSE)
        वापस -1; /* unknown */
    अगर (hlist_empty(&acpi_devices))
        वापस -1; /* unknown (no dsdt table) */

    काष्ठा acpi_device *dev = acpi_dsdt_find_eisaid(शून्य, eisaid);
    वापस acpi_dsdt_present(dev);
}

व्योम acpi_dsdt_parse(व्योम)
{
    अगर (!CONFIG_ACPI_PARSE)
        वापस;

    काष्ठा fadt_descriptor_rev1 *fadt = find_acpi_table(FACP_SIGNATURE);
    अगर (!fadt)
        वापस;
    u8 *dsdt = (व्योम*)(fadt->dsdt);
    अगर (!dsdt)
        वापस;

    u32 लंबाई = *(u32*)(dsdt + 4);
    u32 offset = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, लंबाई);

    काष्ठा parse_state s;
    स_रखो(&s, 0, माप(s));
    parse_termlist(&s, dsdt, offset, लंबाई);

    अगर (!parse_dumpdevs)
        वापस;

    काष्ठा acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    क्रम (dev = acpi_dsdt_find(शून्य, शून्य, 0, शून्य, 0);
         dev != शून्य;
         dev = acpi_dsdt_find(dev, शून्य, 0, शून्य, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        अगर (dev->hid_aml)
            dprintf(1, ", hid");
        अगर (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        अगर (dev->crs_तथ्य)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        अगर (dev->crs_तथ्य)
            print_resources("        ", dev->crs_तथ्य, dev->crs_आकार);
    }
}
