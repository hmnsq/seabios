// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "list.h"   // hlist_*
#ضمن "malloc.h" // malloc_*
#ضمن "output.h" // dprintf
#ضمن "string.h" // memcpy
#ضمن "util.h"
#ضمن "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

بنية acpi_device {
    بنية hlist_گره گره;
    حرف name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_داده;
    صحيح crs_اندازه;
};
ساكن بنية hlist_سر acpi_devices VARVERIFY32INIT;
ساكن ثابت صحيح parse_dumpdevs = 0;

بنية parse_state {
    حرف name[32];
    بنية acpi_device *dev;
    صحيح error;
    صحيح عمق;
};

ساكن فراغ parse_termlist(بنية parse_state *s,
                           u8 *ptr, صحيح جابه_جایی_2, صحيح pkglength);

ساكن فراغ hex(ثابت u8 *ptr, صحيح شمار, صحيح lvl, ثابت حرف *قلم)
{
    صحيح l = 0, i;

    افعل {
        dprintf(lvl, "%s: %04x:  ", قلم, l);
        لكل (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        لكل (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } طالما (l < شمار);
}

ساكن u64 parse_resource_int(u8 *ptr, صحيح شمار)
{
    u64 مقدار = 0;
    صحيح شاخص = 0;

    لكل (شاخص = 0; شاخص < شمار; شاخص++)
        مقدار |= (u64)ptr[شاخص] << (شاخص * 8);
    أرجع مقدار;
}

ساكن صحيح parse_resource_بیت(u8 *ptr, صحيح شمار)
{
    صحيح بیت;

    لكل (بیت = 0; بیت < شمار*8; بیت++)
        إذا (ptr[بیت/8] & (1 << (بیت%8)))
            أرجع بیت;
    أرجع 0;
}

ساكن صحيح parse_resource(u8 *ptr, صحيح درازی, صحيح *type, u64 *min, u64 *max)
{
    صحيح rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    إذا (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        بدل (rname) {
        حالة 0x04: /* irq */
            *min = parse_resource_بیت(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            اكسر;
        حالة 0x0f: /* end marker */
            أرجع 0;
        حالة 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            إذا (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            اكسر;
        حالة 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            اكسر;
        افتراضي:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            اكسر;
        }
    } وإلا {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        بدل (rname) {
        حالة 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            اكسر;
        حالة 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            اكسر;
        حالة 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            اكسر;
        حالة 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            اكسر;
        حالة 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            اكسر;
        افتراضي:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            اكسر;
        }
    }
    أرجع rsize;
}

ساكن صحيح پیدا_کن_resource(u8 *ptr, صحيح len, صحيح نوع_2, u64 *min, u64 *max)
{
    صحيح type, اندازه, جابه_جایی_2 = 0;

    افعل {
        اندازه = parse_resource(ptr + جابه_جایی_2, len - جابه_جایی_2,
                              &type, min, max);
        إذا (نوع_2 == type)
            أرجع 0;
        جابه_جایی_2 += اندازه;
    } طالما (اندازه > 0 && جابه_جایی_2 < len);
    أرجع -1;
}

ساكن صحيح print_resources(ثابت حرف *پیشوند, u8 *ptr, صحيح len)
{
    ساكن ثابت حرف *typename[] = { "mem", "i/o", "bus" };
    صحيح type, اندازه, جابه_جایی_2 = 0;
    u64 min, max;

    افعل {
        اندازه = parse_resource(ptr + جابه_جایی_2, len - جابه_جایی_2,
                              &type, &min, &max);
        بدل (type) {
        حالة 0:
        حالة 1:
        حالة 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    پیشوند, typename[type], min, max);
            اكسر;
        حالة 3:
            dprintf(1, "%sirq %lld\n", پیشوند, min);
            اكسر;
        }
        جابه_جایی_2 += اندازه;
    } طالما (اندازه > 0 && جابه_جایی_2 < len);
    أرجع -1;
}

ساكن صحيح parse_nameseg(u8 *ptr, حرف **dst)
{
    إذا (dst && *dst) {
        *(dst[0]++) = ptr[0];
        إذا (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        إذا (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        إذا (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    أرجع 4;
}

ساكن صحيح parse_namestring(بنية parse_state *s,
                            u8 *ptr, ثابت حرف *قلم)
{
    حرف *dst = s->name;
    صحيح جابه_جایی_2 = 0;
    صحيح i, شمار;

    لكل (;;) {
        بدل (ptr[جابه_جایی_2]) {
        حالة 0: /* null name */
            جابه_جایی_2++;
            *(dst++) = 0;
            اكسر;
        حالة 0x2e:
            جابه_جایی_2++;
            جابه_جایی_2 += parse_nameseg(ptr + جابه_جایی_2, &dst);
            *(dst++) = '.';
            جابه_جایی_2 += parse_nameseg(ptr + جابه_جایی_2, &dst);
            اكسر;
        حالة 0x2f:
            جابه_جایی_2++;
            شمار = ptr[جابه_جایی_2];
            جابه_جایی_2++;
            لكل (i = 0; i < شمار; i++) {
                إذا (i)
                    *(dst++) = '.';
                جابه_جایی_2 += parse_nameseg(ptr + جابه_جایی_2, &dst);
            }
            اكسر;
        حالة '\\':
            *(dst++) = '\\';
            جابه_جایی_2++;
            استمر;
        حالة '^':
            *(dst++) = '^';
            جابه_جایی_2++;
            استمر;
        حالة 'A' ... 'Z':
        حالة '_':
            جابه_جایی_2 += parse_nameseg(ptr, &dst);
            اكسر;
        افتراضي:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            اكسر;
        }
        اكسر;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->عمق,
            قلم, s->name);
    أرجع جابه_جایی_2;
}

ساكن صحيح parse_termarg_int(u8 *ptr, صحيح *error, u64 *dst)
{
    u64 مقدار;
    صحيح جابه_جایی_2 = 1;

    بدل (ptr[0]) {
    حالة 0x00: /* zero */
        مقدار = 0;
        اكسر;
    حالة 0x01: /* one */
        مقدار = 1;
        اكسر;
    حالة 0x0a: /* byte prefix */
        مقدار = ptr[1];
        جابه_جایی_2++;
        اكسر;
    حالة 0x0b: /* word prefix */
        مقدار = ptr[1] |
            ((بیعلامت دراز)ptr[2] << 8);
        جابه_جایی_2 += 2;
        اكسر;
    حالة 0x0c: /* dword prefix */
        مقدار = ptr[1] |
            ((بیعلامت دراز)ptr[2] << 8) |
            ((بیعلامت دراز)ptr[3] << 16) |
            ((بیعلامت دراز)ptr[4] << 24);
        جابه_جایی_2 += 4;
        اكسر;
    افتراضي:
        مقدار = 0;
        hex(ptr, 16, 3, __func__);
        إذا (error)
            *error = 1;
        اكسر;
    }

    إذا (dst)
        *dst = مقدار;
    dprintf(5, "%s: 0x%llx\n", __func__, مقدار);
    أرجع جابه_جایی_2;
}

ساكن صحيح parse_pkglength(u8 *ptr, صحيح *pkglength)
{
    صحيح جابه_جایی_2 = 2;

    *pkglength = 0;
    بدل (ptr[0] >> 6) {
    حالة 3:
        *pkglength |= ptr[3] << 20;
        جابه_جایی_2++;
    حالة 2:
        *pkglength |= ptr[2] << 12;
        جابه_جایی_2++;
    حالة 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        أرجع جابه_جایی_2;
    حالة 0:
    افتراضي:
        *pkglength |= ptr[0] & 0x3f;
        أرجع 1;
    }
}

ساكن صحيح parse_pkg_common(بنية parse_state *s,
                            u8 *ptr, ثابت حرف *قلم, صحيح *pkglength)
{
    صحيح جابه_جایی_2;

    جابه_جایی_2 = parse_pkglength(ptr, pkglength);
    جابه_جایی_2 += parse_namestring(s, ptr + جابه_جایی_2, قلم);
    أرجع جابه_جایی_2;
}

ساكن صحيح parse_pkg_scope(بنية parse_state *s,
                           u8 *ptr)
{
    صحيح جابه_جایی_2, pkglength;

    جابه_جایی_2 = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, جابه_جایی_2, pkglength);
    أرجع pkglength;
}

ساكن صحيح parse_pkg_device(بنية parse_state *s,
                            u8 *ptr)
{
    صحيح جابه_جایی_2, pkglength;

    جابه_جایی_2 = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(حجم(بنية acpi_device));
    إذا (!s->dev) {
        warn_noalloc();
        s->error = 1;
        أرجع pkglength;
    }

    memset(s->dev, 0, حجم(بنية acpi_device));
    hlist_جمع_کن_سر(&s->dev->گره, &acpi_devices);
    strtcpy(s->dev->name, s->name, حجم(s->name));
    parse_termlist(s, ptr, جابه_جایی_2, pkglength);
    s->dev = عدم;

    أرجع pkglength;
}

ساكن صحيح parse_pkg_میانگیر(بنية parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    صحيح pkglength, جابه_جایی_2;

    جابه_جایی_2 = parse_pkglength(ptr, &pkglength);
    جابه_جایی_2 += parse_termarg_int(ptr + جابه_جایی_2, &s->error, &blen);
    إذا (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_داده = ptr + جابه_جایی_2;
        s->dev->crs_اندازه = blen;
    }
    أرجع pkglength;
}

ساكن صحيح parse_pkg_skip(بنية parse_state *s,
                          u8 *ptr, صحيح op, صحيح name)
{
    صحيح pkglength, جابه_جایی_2;
    حرف قلم[8];

    snprintf(قلم, حجم(قلم), "op %x", op);
    جابه_جایی_2 = parse_pkglength(ptr, &pkglength);
    إذا (name) {
        parse_namestring(s, ptr + جابه_جایی_2, قلم);
    } وإلا {
        dprintf(5, "%s: %s (%d)\n", __func__, قلم, pkglength);
    }
    أرجع pkglength;
}

ساكن صحيح parse_termobj(بنية parse_state *s,
                         u8 *ptr)
{
    صحيح جابه_جایی_2 = 1;

    إذا (s->عمق == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        أرجع جابه_جایی_2;
    }

    s->عمق++;
    بدل (ptr[0]) {
    حالة 0x00: /* zero */
        اكسر;
    حالة 0x01: /* one */
        اكسر;
    حالة 0x06: /* AliasOp */
        جابه_جایی_2 += parse_namestring(s, ptr + جابه_جایی_2, "SourceObject");
        جابه_جایی_2 += parse_namestring(s, ptr + جابه_جایی_2, "AliasObject");
        اكسر;
    حالة 0x08: /* name op */
        جابه_جایی_2 += parse_namestring(s, ptr + جابه_جایی_2, "name");
        جابه_جایی_2 += parse_termobj(s, ptr + جابه_جایی_2);
        إذا (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        إذا (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        اكسر;
    حالة 0x0a: /* byte prefix */
        جابه_جایی_2++;
        اكسر;
    حالة 0x0b: /* word prefix */
        جابه_جایی_2 += 2;
        اكسر;
    حالة 0x0c: /* dword prefix */
        جابه_جایی_2 += 4;
        اكسر;
    حالة 0x0d: /* string prefix */
        طالما (ptr[جابه_جایی_2])
            جابه_جایی_2++;
        جابه_جایی_2++;
        اكسر;
    حالة 0x10: /* scope op */
        جابه_جایی_2 += parse_pkg_scope(s, ptr + جابه_جایی_2);
        اكسر;
    حالة 0x11: /* buffer op */
        جابه_جایی_2 += parse_pkg_میانگیر(s, ptr + جابه_جایی_2);
        اكسر;
    حالة 0x12: /* package op */
    حالة 0x13: /* var package op */
        جابه_جایی_2 += parse_pkg_skip(s, ptr + جابه_جایی_2, ptr[0], 0);
        اكسر;
    حالة 0x14: /* method op */
        جابه_جایی_2 += parse_pkg_skip(s, ptr + جابه_جایی_2, ptr[0], 1);
        إذا (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        اكسر;
    حالة 0x5b: /* ext op prefix */
        جابه_جایی_2++;
        بدل (ptr[1]) {
        حالة 0x01: /* mutex op */
            جابه_جایی_2 += parse_namestring(s, ptr + جابه_جایی_2, "mutex");
            جابه_جایی_2++; /* sync flags */
            اكسر;
        حالة 0x80: /* op region op */
            جابه_جایی_2 += parse_namestring(s, ptr + جابه_جایی_2, "op region");
            جابه_جایی_2++; /* region space */
            جابه_جایی_2 += parse_termarg_int(ptr + جابه_جایی_2, &s->error, عدم);
            جابه_جایی_2 += parse_termarg_int(ptr + جابه_جایی_2, &s->error, عدم);
            اكسر;
        حالة 0x81: /* field op */
        حالة 0x83: /* processor op */
        حالة 0x84: /* power resource op */
        حالة 0x85: /* thermal zone op */
            جابه_جایی_2 += parse_pkg_skip(s, ptr + جابه_جایی_2, 0x5b00 | ptr[1], 1);
            اكسر;
        حالة 0x82: /* device op */
            جابه_جایی_2 += parse_pkg_device(s, ptr + جابه_جایی_2);
            اكسر;
        افتراضي:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            اكسر;
        }
        اكسر;
    افتراضي:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        اكسر;
    }
    s->عمق--;

    أرجع جابه_جایی_2;
}

ساكن فراغ parse_termlist(بنية parse_state *s,
                           u8 *ptr, صحيح جابه_جایی_2, صحيح pkglength)
{
    لكل (;;) {
        جابه_جایی_2 += parse_termobj(s, ptr + جابه_جایی_2);
        إذا (جابه_جایی_2 == pkglength)
            أرجع;
        إذا (جابه_جایی_2 > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    جابه_جایی_2, pkglength);
            s->error = 1;
            أرجع;
        }
        إذا (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    جابه_جایی_2, pkglength);
            s->error = 0;
            أرجع;
        }
    }
}

ساكن بنية acpi_device *acpi_dsdt_پیدا_کن(بنية acpi_device *prev,
                                          ثابت u8 *aml1, صحيح size1,
                                          ثابت u8 *aml2, صحيح size2)
{
    بنية acpi_device *dev;
    بنية hlist_گره *گره;

    إذا (!prev)
        گره = acpi_devices.اولی;
    وإلا
        گره = prev->گره.بعدی;

    لكل (; گره != عدم; گره = dev->گره.بعدی) {
        dev = container_از(گره, بنية acpi_device, گره);
        إذا (!aml1 && !aml2)
            أرجع dev;
        إذا (!dev->hid_aml)
            استمر;
        إذا (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            أرجع dev;
        إذا (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            أرجع dev;
    }
    أرجع عدم;
}

ساكن صحيح acpi_dsdt_present(بنية acpi_device *dev)
{
    إذا (!dev)
        أرجع 0; /* no */
    إذا (!dev->sta_aml)
        أرجع 1; /* yes */
    إذا (dev->sta_aml[0] == 0x14)
        أرجع -1; /* unknown (can't evaluate method) */
    إذا (dev->sta_aml[0] == 0x08) {
        u64 مقدار = 0;
        parse_termarg_int(dev->sta_aml + 5, عدم, &مقدار);
        إذا (مقدار == 0)
            أرجع 0; /* no */
        وإلا
            أرجع 1; /* yes */
    }
    أرجع -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

بنية acpi_device *acpi_dsdt_پیدا_کن_string(بنية acpi_device *prev,
                                          ثابت حرف *hid)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع عدم;

    u8 aml[10];
    صحيح len = snprintf((حرف*)aml, حجم(aml), "\x0d%s", hid);
    أرجع acpi_dsdt_پیدا_کن(prev, aml, len, عدم, 0);
}

بنية acpi_device *acpi_dsdt_پیدا_کن_eisaid(بنية acpi_device *prev, u16 eisaid)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع عدم;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    صحيح len2 = snprintf((حرف*)aml2, حجم(aml2), "\x0dPNP%04X", eisaid);
    أرجع acpi_dsdt_پیدا_کن(prev, aml1, 5, aml2, len2);
}

حرف *acpi_dsdt_name(بنية acpi_device *dev)
{
    إذا (!CONFIG_ACPI_PARSE || !dev)
        أرجع عدم;
    أرجع dev->name;
}

صحيح acpi_dsdt_پیدا_کن_io(بنية acpi_device *dev, u64 *min, u64 *max)
{
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_داده)
        أرجع -1;
    أرجع پیدا_کن_resource(dev->crs_داده, dev->crs_اندازه,
                         1 /* I/O */, min, max);
}

صحيح acpi_dsdt_پیدا_کن_mem(بنية acpi_device *dev, u64 *min, u64 *max)
{
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_داده)
        أرجع -1;
    أرجع پیدا_کن_resource(dev->crs_داده, dev->crs_اندازه,
                         0 /* mem */, min, max);
}

صحيح acpi_dsdt_پیدا_کن_irq(بنية acpi_device *dev, u64 *irq)
{
    u64 max;
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_داده)
        أرجع -1;
    أرجع پیدا_کن_resource(dev->crs_داده, dev->crs_اندازه,
                         3 /* irq */, irq, &max);
}

صحيح acpi_dsdt_present_eisaid(u16 eisaid)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع -1; /* unknown */
    إذا (hlist_خالی(&acpi_devices))
        أرجع -1; /* unknown (no dsdt table) */

    بنية acpi_device *dev = acpi_dsdt_پیدا_کن_eisaid(عدم, eisaid);
    أرجع acpi_dsdt_present(dev);
}

فراغ acpi_dsdt_parse(فراغ)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع;

    بنية fadt_descriptor_rev1 *fadt = پیدا_کن_acpi_جدول(FACP_SIGNATURE);
    إذا (!fadt)
        أرجع;
    u8 *dsdt = (فراغ*)(fadt->dsdt);
    إذا (!dsdt)
        أرجع;

    u32 درازی = *(u32*)(dsdt + 4);
    u32 جابه_جایی_2 = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, درازی);

    بنية parse_state s;
    memset(&s, 0, حجم(s));
    parse_termlist(&s, dsdt, جابه_جایی_2, درازی);

    إذا (!parse_dumpdevs)
        أرجع;

    بنية acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    لكل (dev = acpi_dsdt_پیدا_کن(عدم, عدم, 0, عدم, 0);
         dev != عدم;
         dev = acpi_dsdt_پیدا_کن(dev, عدم, 0, عدم, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        إذا (dev->hid_aml)
            dprintf(1, ", hid");
        إذا (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        إذا (dev->crs_داده)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        إذا (dev->crs_داده)
            print_resources("        ", dev->crs_داده, dev->crs_اندازه);
    }
}
