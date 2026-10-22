// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "list.h"   // hlist_*
#شامل "malloc.h" // malloc_*
#شامل "output.h" // dprintf
#شامل "string.h" // memcpy
#شامل "util.h"
#شامل "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

ساخت acpi_device {
    ساخت hlist_گرہ گرہ;
    حرف name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_ڈیٹا;
    صحیح crs_سائز;
};
ساکن ساخت hlist_سر acpi_devices VARVERIFY32INIT;
ساکن ثابت صحیح parse_dumpdevs = 0;

ساخت parse_state {
    حرف name[32];
    ساخت acpi_device *dev;
    صحیح error;
    صحیح گہرائی;
};

ساکن خالی parse_termlist(ساخت parse_state *s,
                           u8 *ptr, صحیح انحراف_2, صحیح pkglength);

ساکن خالی hex(ثابت u8 *ptr, صحیح گنتی, صحیح lvl, ثابت حرف *شے)
{
    صحیح l = 0, i;

    کرو {
        dprintf(lvl, "%s: %04x:  ", شے, l);
        برائے (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        برائے (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } جب (l < گنتی);
}

ساکن u64 parse_resource_int(u8 *ptr, صحیح گنتی)
{
    u64 قدر = 0;
    صحیح اشاریہ = 0;

    برائے (اشاریہ = 0; اشاریہ < گنتی; اشاریہ++)
        قدر |= (u64)ptr[اشاریہ] << (اشاریہ * 8);
    واپس قدر;
}

ساکن صحیح parse_resource_ثنائی_ہندسہ(u8 *ptr, صحیح گنتی)
{
    صحیح ثنائی_ہندسہ;

    برائے (ثنائی_ہندسہ = 0; ثنائی_ہندسہ < گنتی*8; ثنائی_ہندسہ++)
        اگر (ptr[ثنائی_ہندسہ/8] & (1 << (ثنائی_ہندسہ%8)))
            واپس ثنائی_ہندسہ;
    واپس 0;
}

ساکن صحیح parse_resource(u8 *ptr, صحیح لمبائی, صحیح *type, u64 *min, u64 *max)
{
    صحیح rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    اگر (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        انتخاب (rname) {
        حالت 0x04: /* irq */
            *min = parse_resource_ثنائی_ہندسہ(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            توڑو;
        حالت 0x0f: /* end marker */
            واپس 0;
        حالت 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            اگر (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            توڑو;
        حالت 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            توڑو;
        طےشدہ:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            توڑو;
        }
    } ورنہ {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        انتخاب (rname) {
        حالت 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            توڑو;
        حالت 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            توڑو;
        حالت 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            توڑو;
        حالت 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            توڑو;
        حالت 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            توڑو;
        طےشدہ:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            توڑو;
        }
    }
    واپس rsize;
}

ساکن صحیح ڈھونڈو_resource(u8 *ptr, صحیح len, صحیح قسم_2, u64 *min, u64 *max)
{
    صحیح type, سائز, انحراف_2 = 0;

    کرو {
        سائز = parse_resource(ptr + انحراف_2, len - انحراف_2,
                              &type, min, max);
        اگر (قسم_2 == type)
            واپس 0;
        انحراف_2 += سائز;
    } جب (سائز > 0 && انحراف_2 < len);
    واپس -1;
}

ساکن صحیح print_resources(ثابت حرف *سابقہ, u8 *ptr, صحیح len)
{
    ساکن ثابت حرف *typename[] = { "mem", "i/o", "bus" };
    صحیح type, سائز, انحراف_2 = 0;
    u64 min, max;

    کرو {
        سائز = parse_resource(ptr + انحراف_2, len - انحراف_2,
                              &type, &min, &max);
        انتخاب (type) {
        حالت 0:
        حالت 1:
        حالت 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    سابقہ, typename[type], min, max);
            توڑو;
        حالت 3:
            dprintf(1, "%sirq %lld\n", سابقہ, min);
            توڑو;
        }
        انحراف_2 += سائز;
    } جب (سائز > 0 && انحراف_2 < len);
    واپس -1;
}

ساکن صحیح parse_nameseg(u8 *ptr, حرف **dst)
{
    اگر (dst && *dst) {
        *(dst[0]++) = ptr[0];
        اگر (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        اگر (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        اگر (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    واپس 4;
}

ساکن صحیح parse_namestring(ساخت parse_state *s,
                            u8 *ptr, ثابت حرف *شے)
{
    حرف *dst = s->name;
    صحیح انحراف_2 = 0;
    صحیح i, گنتی;

    برائے (;;) {
        انتخاب (ptr[انحراف_2]) {
        حالت 0: /* null name */
            انحراف_2++;
            *(dst++) = 0;
            توڑو;
        حالت 0x2e:
            انحراف_2++;
            انحراف_2 += parse_nameseg(ptr + انحراف_2, &dst);
            *(dst++) = '.';
            انحراف_2 += parse_nameseg(ptr + انحراف_2, &dst);
            توڑو;
        حالت 0x2f:
            انحراف_2++;
            گنتی = ptr[انحراف_2];
            انحراف_2++;
            برائے (i = 0; i < گنتی; i++) {
                اگر (i)
                    *(dst++) = '.';
                انحراف_2 += parse_nameseg(ptr + انحراف_2, &dst);
            }
            توڑو;
        حالت '\\':
            *(dst++) = '\\';
            انحراف_2++;
            جاری;
        حالت '^':
            *(dst++) = '^';
            انحراف_2++;
            جاری;
        حالت 'A' ... 'Z':
        حالت '_':
            انحراف_2 += parse_nameseg(ptr, &dst);
            توڑو;
        طےشدہ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            توڑو;
        }
        توڑو;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->گہرائی,
            شے, s->name);
    واپس انحراف_2;
}

ساکن صحیح parse_termarg_int(u8 *ptr, صحیح *error, u64 *dst)
{
    u64 قدر;
    صحیح انحراف_2 = 1;

    انتخاب (ptr[0]) {
    حالت 0x00: /* zero */
        قدر = 0;
        توڑو;
    حالت 0x01: /* one */
        قدر = 1;
        توڑو;
    حالت 0x0a: /* byte prefix */
        قدر = ptr[1];
        انحراف_2++;
        توڑو;
    حالت 0x0b: /* word prefix */
        قدر = ptr[1] |
            ((غیرعلامتی طویل)ptr[2] << 8);
        انحراف_2 += 2;
        توڑو;
    حالت 0x0c: /* dword prefix */
        قدر = ptr[1] |
            ((غیرعلامتی طویل)ptr[2] << 8) |
            ((غیرعلامتی طویل)ptr[3] << 16) |
            ((غیرعلامتی طویل)ptr[4] << 24);
        انحراف_2 += 4;
        توڑو;
    طےشدہ:
        قدر = 0;
        hex(ptr, 16, 3, __func__);
        اگر (error)
            *error = 1;
        توڑو;
    }

    اگر (dst)
        *dst = قدر;
    dprintf(5, "%s: 0x%llx\n", __func__, قدر);
    واپس انحراف_2;
}

ساکن صحیح parse_pkglength(u8 *ptr, صحیح *pkglength)
{
    صحیح انحراف_2 = 2;

    *pkglength = 0;
    انتخاب (ptr[0] >> 6) {
    حالت 3:
        *pkglength |= ptr[3] << 20;
        انحراف_2++;
    حالت 2:
        *pkglength |= ptr[2] << 12;
        انحراف_2++;
    حالت 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        واپس انحراف_2;
    حالت 0:
    طےشدہ:
        *pkglength |= ptr[0] & 0x3f;
        واپس 1;
    }
}

ساکن صحیح parse_pkg_common(ساخت parse_state *s,
                            u8 *ptr, ثابت حرف *شے, صحیح *pkglength)
{
    صحیح انحراف_2;

    انحراف_2 = parse_pkglength(ptr, pkglength);
    انحراف_2 += parse_namestring(s, ptr + انحراف_2, شے);
    واپس انحراف_2;
}

ساکن صحیح parse_pkg_scope(ساخت parse_state *s,
                           u8 *ptr)
{
    صحیح انحراف_2, pkglength;

    انحراف_2 = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, انحراف_2, pkglength);
    واپس pkglength;
}

ساکن صحیح parse_pkg_device(ساخت parse_state *s,
                            u8 *ptr)
{
    صحیح انحراف_2, pkglength;

    انحراف_2 = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(حجم(ساخت acpi_device));
    اگر (!s->dev) {
        warn_noalloc();
        s->error = 1;
        واپس pkglength;
    }

    memset(s->dev, 0, حجم(ساخت acpi_device));
    hlist_جوڑو_سر(&s->dev->گرہ, &acpi_devices);
    strtcpy(s->dev->name, s->name, حجم(s->name));
    parse_termlist(s, ptr, انحراف_2, pkglength);
    s->dev = تہی;

    واپس pkglength;
}

ساکن صحیح parse_pkg_بفر(ساخت parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    صحیح pkglength, انحراف_2;

    انحراف_2 = parse_pkglength(ptr, &pkglength);
    انحراف_2 += parse_termarg_int(ptr + انحراف_2, &s->error, &blen);
    اگر (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_ڈیٹا = ptr + انحراف_2;
        s->dev->crs_سائز = blen;
    }
    واپس pkglength;
}

ساکن صحیح parse_pkg_skip(ساخت parse_state *s,
                          u8 *ptr, صحیح op, صحیح name)
{
    صحیح pkglength, انحراف_2;
    حرف شے[8];

    snprintf(شے, حجم(شے), "op %x", op);
    انحراف_2 = parse_pkglength(ptr, &pkglength);
    اگر (name) {
        parse_namestring(s, ptr + انحراف_2, شے);
    } ورنہ {
        dprintf(5, "%s: %s (%d)\n", __func__, شے, pkglength);
    }
    واپس pkglength;
}

ساکن صحیح parse_termobj(ساخت parse_state *s,
                         u8 *ptr)
{
    صحیح انحراف_2 = 1;

    اگر (s->گہرائی == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        واپس انحراف_2;
    }

    s->گہرائی++;
    انتخاب (ptr[0]) {
    حالت 0x00: /* zero */
        توڑو;
    حالت 0x01: /* one */
        توڑو;
    حالت 0x06: /* AliasOp */
        انحراف_2 += parse_namestring(s, ptr + انحراف_2, "SourceObject");
        انحراف_2 += parse_namestring(s, ptr + انحراف_2, "AliasObject");
        توڑو;
    حالت 0x08: /* name op */
        انحراف_2 += parse_namestring(s, ptr + انحراف_2, "name");
        انحراف_2 += parse_termobj(s, ptr + انحراف_2);
        اگر (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        اگر (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        توڑو;
    حالت 0x0a: /* byte prefix */
        انحراف_2++;
        توڑو;
    حالت 0x0b: /* word prefix */
        انحراف_2 += 2;
        توڑو;
    حالت 0x0c: /* dword prefix */
        انحراف_2 += 4;
        توڑو;
    حالت 0x0d: /* string prefix */
        جب (ptr[انحراف_2])
            انحراف_2++;
        انحراف_2++;
        توڑو;
    حالت 0x10: /* scope op */
        انحراف_2 += parse_pkg_scope(s, ptr + انحراف_2);
        توڑو;
    حالت 0x11: /* buffer op */
        انحراف_2 += parse_pkg_بفر(s, ptr + انحراف_2);
        توڑو;
    حالت 0x12: /* package op */
    حالت 0x13: /* var package op */
        انحراف_2 += parse_pkg_skip(s, ptr + انحراف_2, ptr[0], 0);
        توڑو;
    حالت 0x14: /* method op */
        انحراف_2 += parse_pkg_skip(s, ptr + انحراف_2, ptr[0], 1);
        اگر (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        توڑو;
    حالت 0x5b: /* ext op prefix */
        انحراف_2++;
        انتخاب (ptr[1]) {
        حالت 0x01: /* mutex op */
            انحراف_2 += parse_namestring(s, ptr + انحراف_2, "mutex");
            انحراف_2++; /* sync flags */
            توڑو;
        حالت 0x80: /* op region op */
            انحراف_2 += parse_namestring(s, ptr + انحراف_2, "op region");
            انحراف_2++; /* region space */
            انحراف_2 += parse_termarg_int(ptr + انحراف_2, &s->error, تہی);
            انحراف_2 += parse_termarg_int(ptr + انحراف_2, &s->error, تہی);
            توڑو;
        حالت 0x81: /* field op */
        حالت 0x83: /* processor op */
        حالت 0x84: /* power resource op */
        حالت 0x85: /* thermal zone op */
            انحراف_2 += parse_pkg_skip(s, ptr + انحراف_2, 0x5b00 | ptr[1], 1);
            توڑو;
        حالت 0x82: /* device op */
            انحراف_2 += parse_pkg_device(s, ptr + انحراف_2);
            توڑو;
        طےشدہ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            توڑو;
        }
        توڑو;
    طےشدہ:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        توڑو;
    }
    s->گہرائی--;

    واپس انحراف_2;
}

ساکن خالی parse_termlist(ساخت parse_state *s,
                           u8 *ptr, صحیح انحراف_2, صحیح pkglength)
{
    برائے (;;) {
        انحراف_2 += parse_termobj(s, ptr + انحراف_2);
        اگر (انحراف_2 == pkglength)
            واپس;
        اگر (انحراف_2 > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    انحراف_2, pkglength);
            s->error = 1;
            واپس;
        }
        اگر (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    انحراف_2, pkglength);
            s->error = 0;
            واپس;
        }
    }
}

ساکن ساخت acpi_device *acpi_dsdt_ڈھونڈو(ساخت acpi_device *پچھلا,
                                          ثابت u8 *aml1, صحیح size1,
                                          ثابت u8 *aml2, صحیح size2)
{
    ساخت acpi_device *dev;
    ساخت hlist_گرہ *گرہ;

    اگر (!پچھلا)
        گرہ = acpi_devices.پہلا;
    ورنہ
        گرہ = پچھلا->گرہ.اگلا;

    برائے (; گرہ != تہی; گرہ = dev->گرہ.اگلا) {
        dev = container_کا(گرہ, ساخت acpi_device, گرہ);
        اگر (!aml1 && !aml2)
            واپس dev;
        اگر (!dev->hid_aml)
            جاری;
        اگر (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            واپس dev;
        اگر (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            واپس dev;
    }
    واپس تہی;
}

ساکن صحیح acpi_dsdt_present(ساخت acpi_device *dev)
{
    اگر (!dev)
        واپس 0; /* no */
    اگر (!dev->sta_aml)
        واپس 1; /* yes */
    اگر (dev->sta_aml[0] == 0x14)
        واپس -1; /* unknown (can't evaluate method) */
    اگر (dev->sta_aml[0] == 0x08) {
        u64 قدر = 0;
        parse_termarg_int(dev->sta_aml + 5, تہی, &قدر);
        اگر (قدر == 0)
            واپس 0; /* no */
        ورنہ
            واپس 1; /* yes */
    }
    واپس -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

ساخت acpi_device *acpi_dsdt_ڈھونڈو_string(ساخت acpi_device *پچھلا,
                                          ثابت حرف *hid)
{
    اگر (!CONFIG_ACPI_PARSE)
        واپس تہی;

    u8 aml[10];
    صحیح len = snprintf((حرف*)aml, حجم(aml), "\x0d%s", hid);
    واپس acpi_dsdt_ڈھونڈو(پچھلا, aml, len, تہی, 0);
}

ساخت acpi_device *acpi_dsdt_ڈھونڈو_eisaid(ساخت acpi_device *پچھلا, u16 eisaid)
{
    اگر (!CONFIG_ACPI_PARSE)
        واپس تہی;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    صحیح len2 = snprintf((حرف*)aml2, حجم(aml2), "\x0dPNP%04X", eisaid);
    واپس acpi_dsdt_ڈھونڈو(پچھلا, aml1, 5, aml2, len2);
}

حرف *acpi_dsdt_name(ساخت acpi_device *dev)
{
    اگر (!CONFIG_ACPI_PARSE || !dev)
        واپس تہی;
    واپس dev->name;
}

صحیح acpi_dsdt_ڈھونڈو_io(ساخت acpi_device *dev, u64 *min, u64 *max)
{
    اگر (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ڈیٹا)
        واپس -1;
    واپس ڈھونڈو_resource(dev->crs_ڈیٹا, dev->crs_سائز,
                         1 /* I/O */, min, max);
}

صحیح acpi_dsdt_ڈھونڈو_mem(ساخت acpi_device *dev, u64 *min, u64 *max)
{
    اگر (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ڈیٹا)
        واپس -1;
    واپس ڈھونڈو_resource(dev->crs_ڈیٹا, dev->crs_سائز,
                         0 /* mem */, min, max);
}

صحیح acpi_dsdt_ڈھونڈو_irq(ساخت acpi_device *dev, u64 *irq)
{
    u64 max;
    اگر (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ڈیٹا)
        واپس -1;
    واپس ڈھونڈو_resource(dev->crs_ڈیٹا, dev->crs_سائز,
                         3 /* irq */, irq, &max);
}

صحیح acpi_dsdt_present_eisaid(u16 eisaid)
{
    اگر (!CONFIG_ACPI_PARSE)
        واپس -1; /* unknown */
    اگر (hlist_خالی_خانے(&acpi_devices))
        واپس -1; /* unknown (no dsdt table) */

    ساخت acpi_device *dev = acpi_dsdt_ڈھونڈو_eisaid(تہی, eisaid);
    واپس acpi_dsdt_present(dev);
}

خالی acpi_dsdt_parse(خالی)
{
    اگر (!CONFIG_ACPI_PARSE)
        واپس;

    ساخت fadt_descriptor_rev1 *fadt = ڈھونڈو_acpi_جدول(FACP_SIGNATURE);
    اگر (!fadt)
        واپس;
    u8 *dsdt = (خالی*)(fadt->dsdt);
    اگر (!dsdt)
        واپس;

    u32 لمبائی = *(u32*)(dsdt + 4);
    u32 انحراف_2 = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, لمبائی);

    ساخت parse_state s;
    memset(&s, 0, حجم(s));
    parse_termlist(&s, dsdt, انحراف_2, لمبائی);

    اگر (!parse_dumpdevs)
        واپس;

    ساخت acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    برائے (dev = acpi_dsdt_ڈھونڈو(تہی, تہی, 0, تہی, 0);
         dev != تہی;
         dev = acpi_dsdt_ڈھونڈو(dev, تہی, 0, تہی, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        اگر (dev->hid_aml)
            dprintf(1, ", hid");
        اگر (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        اگر (dev->crs_ڈیٹا)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        اگر (dev->crs_ڈیٹا)
            print_resources("        ", dev->crs_ڈیٹا, dev->crs_سائز);
    }
}
