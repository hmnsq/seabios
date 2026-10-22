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
    بنية hlist_ڳنڍ ڳنڍ;
    حرف name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_معلومات;
    صحيح crs_سائيز;
};
ساكن بنية hlist_سر acpi_devices VARVERIFY32INIT;
ساكن ثابت صحيح parse_dumpdevs = 0;

بنية parse_state {
    حرف name[32];
    بنية acpi_device *dev;
    صحيح error;
    صحيح اونهائي;
};

ساكن فراغ parse_termlist(بنية parse_state *s,
                           u8 *ptr, صحيح وٿي_2, صحيح pkglength);

ساكن فراغ hex(ثابت u8 *ptr, صحيح ڳڻپ, صحيح lvl, ثابت حرف *شي)
{
    صحيح l = 0, i;

    افعل {
        dprintf(lvl, "%s: %04x:  ", شي, l);
        لكل (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        لكل (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } طالما (l < ڳڻپ);
}

ساكن u64 parse_resource_int(u8 *ptr, صحيح ڳڻپ)
{
    u64 قدر = 0;
    صحيح اشاريو = 0;

    لكل (اشاريو = 0; اشاريو < ڳڻپ; اشاريو++)
        قدر |= (u64)ptr[اشاريو] << (اشاريو * 8);
    أرجع قدر;
}

ساكن صحيح parse_resource_ٻٽو_انگ(u8 *ptr, صحيح ڳڻپ)
{
    صحيح ٻٽو_انگ;

    لكل (ٻٽو_انگ = 0; ٻٽو_انگ < ڳڻپ*8; ٻٽو_انگ++)
        إذا (ptr[ٻٽو_انگ/8] & (1 << (ٻٽو_انگ%8)))
            أرجع ٻٽو_انگ;
    أرجع 0;
}

ساكن صحيح parse_resource(u8 *ptr, صحيح ڊيگهائي, صحيح *type, u64 *min, u64 *max)
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
            *min = parse_resource_ٻٽو_انگ(ptr + 1, rsize);
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

ساكن صحيح ڳوليو_resource(u8 *ptr, صحيح len, صحيح قسم, u64 *min, u64 *max)
{
    صحيح type, سائيز, وٿي_2 = 0;

    افعل {
        سائيز = parse_resource(ptr + وٿي_2, len - وٿي_2,
                              &type, min, max);
        إذا (قسم == type)
            أرجع 0;
        وٿي_2 += سائيز;
    } طالما (سائيز > 0 && وٿي_2 < len);
    أرجع -1;
}

ساكن صحيح print_resources(ثابت حرف *اڳياڙي, u8 *ptr, صحيح len)
{
    ساكن ثابت حرف *typename[] = { "mem", "i/o", "bus" };
    صحيح type, سائيز, وٿي_2 = 0;
    u64 min, max;

    افعل {
        سائيز = parse_resource(ptr + وٿي_2, len - وٿي_2,
                              &type, &min, &max);
        بدل (type) {
        حالة 0:
        حالة 1:
        حالة 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    اڳياڙي, typename[type], min, max);
            اكسر;
        حالة 3:
            dprintf(1, "%sirq %lld\n", اڳياڙي, min);
            اكسر;
        }
        وٿي_2 += سائيز;
    } طالما (سائيز > 0 && وٿي_2 < len);
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
                            u8 *ptr, ثابت حرف *شي)
{
    حرف *dst = s->name;
    صحيح وٿي_2 = 0;
    صحيح i, ڳڻپ;

    لكل (;;) {
        بدل (ptr[وٿي_2]) {
        حالة 0: /* null name */
            وٿي_2++;
            *(dst++) = 0;
            اكسر;
        حالة 0x2e:
            وٿي_2++;
            وٿي_2 += parse_nameseg(ptr + وٿي_2, &dst);
            *(dst++) = '.';
            وٿي_2 += parse_nameseg(ptr + وٿي_2, &dst);
            اكسر;
        حالة 0x2f:
            وٿي_2++;
            ڳڻپ = ptr[وٿي_2];
            وٿي_2++;
            لكل (i = 0; i < ڳڻپ; i++) {
                إذا (i)
                    *(dst++) = '.';
                وٿي_2 += parse_nameseg(ptr + وٿي_2, &dst);
            }
            اكسر;
        حالة '\\':
            *(dst++) = '\\';
            وٿي_2++;
            استمر;
        حالة '^':
            *(dst++) = '^';
            وٿي_2++;
            استمر;
        حالة 'A' ... 'Z':
        حالة '_':
            وٿي_2 += parse_nameseg(ptr, &dst);
            اكسر;
        افتراضي:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            اكسر;
        }
        اكسر;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->اونهائي,
            شي, s->name);
    أرجع وٿي_2;
}

ساكن صحيح parse_termarg_int(u8 *ptr, صحيح *error, u64 *dst)
{
    u64 قدر;
    صحيح وٿي_2 = 1;

    بدل (ptr[0]) {
    حالة 0x00: /* zero */
        قدر = 0;
        اكسر;
    حالة 0x01: /* one */
        قدر = 1;
        اكسر;
    حالة 0x0a: /* byte prefix */
        قدر = ptr[1];
        وٿي_2++;
        اكسر;
    حالة 0x0b: /* word prefix */
        قدر = ptr[1] |
            ((بنانشان ڊگهو)ptr[2] << 8);
        وٿي_2 += 2;
        اكسر;
    حالة 0x0c: /* dword prefix */
        قدر = ptr[1] |
            ((بنانشان ڊگهو)ptr[2] << 8) |
            ((بنانشان ڊگهو)ptr[3] << 16) |
            ((بنانشان ڊگهو)ptr[4] << 24);
        وٿي_2 += 4;
        اكسر;
    افتراضي:
        قدر = 0;
        hex(ptr, 16, 3, __func__);
        إذا (error)
            *error = 1;
        اكسر;
    }

    إذا (dst)
        *dst = قدر;
    dprintf(5, "%s: 0x%llx\n", __func__, قدر);
    أرجع وٿي_2;
}

ساكن صحيح parse_pkglength(u8 *ptr, صحيح *pkglength)
{
    صحيح وٿي_2 = 2;

    *pkglength = 0;
    بدل (ptr[0] >> 6) {
    حالة 3:
        *pkglength |= ptr[3] << 20;
        وٿي_2++;
    حالة 2:
        *pkglength |= ptr[2] << 12;
        وٿي_2++;
    حالة 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        أرجع وٿي_2;
    حالة 0:
    افتراضي:
        *pkglength |= ptr[0] & 0x3f;
        أرجع 1;
    }
}

ساكن صحيح parse_pkg_common(بنية parse_state *s,
                            u8 *ptr, ثابت حرف *شي, صحيح *pkglength)
{
    صحيح وٿي_2;

    وٿي_2 = parse_pkglength(ptr, pkglength);
    وٿي_2 += parse_namestring(s, ptr + وٿي_2, شي);
    أرجع وٿي_2;
}

ساكن صحيح parse_pkg_scope(بنية parse_state *s,
                           u8 *ptr)
{
    صحيح وٿي_2, pkglength;

    وٿي_2 = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, وٿي_2, pkglength);
    أرجع pkglength;
}

ساكن صحيح parse_pkg_device(بنية parse_state *s,
                            u8 *ptr)
{
    صحيح وٿي_2, pkglength;

    وٿي_2 = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(حجم(بنية acpi_device));
    إذا (!s->dev) {
        warn_noalloc();
        s->error = 1;
        أرجع pkglength;
    }

    memset(s->dev, 0, حجم(بنية acpi_device));
    hlist_جوڙيو_سر(&s->dev->ڳنڍ, &acpi_devices);
    strtcpy(s->dev->name, s->name, حجم(s->name));
    parse_termlist(s, ptr, وٿي_2, pkglength);
    s->dev = عدم;

    أرجع pkglength;
}

ساكن صحيح parse_pkg_بفر(بنية parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    صحيح pkglength, وٿي_2;

    وٿي_2 = parse_pkglength(ptr, &pkglength);
    وٿي_2 += parse_termarg_int(ptr + وٿي_2, &s->error, &blen);
    إذا (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_معلومات = ptr + وٿي_2;
        s->dev->crs_سائيز = blen;
    }
    أرجع pkglength;
}

ساكن صحيح parse_pkg_skip(بنية parse_state *s,
                          u8 *ptr, صحيح op, صحيح name)
{
    صحيح pkglength, وٿي_2;
    حرف شي[8];

    snprintf(شي, حجم(شي), "op %x", op);
    وٿي_2 = parse_pkglength(ptr, &pkglength);
    إذا (name) {
        parse_namestring(s, ptr + وٿي_2, شي);
    } وإلا {
        dprintf(5, "%s: %s (%d)\n", __func__, شي, pkglength);
    }
    أرجع pkglength;
}

ساكن صحيح parse_termobj(بنية parse_state *s,
                         u8 *ptr)
{
    صحيح وٿي_2 = 1;

    إذا (s->اونهائي == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        أرجع وٿي_2;
    }

    s->اونهائي++;
    بدل (ptr[0]) {
    حالة 0x00: /* zero */
        اكسر;
    حالة 0x01: /* one */
        اكسر;
    حالة 0x06: /* AliasOp */
        وٿي_2 += parse_namestring(s, ptr + وٿي_2, "SourceObject");
        وٿي_2 += parse_namestring(s, ptr + وٿي_2, "AliasObject");
        اكسر;
    حالة 0x08: /* name op */
        وٿي_2 += parse_namestring(s, ptr + وٿي_2, "name");
        وٿي_2 += parse_termobj(s, ptr + وٿي_2);
        إذا (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        إذا (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        اكسر;
    حالة 0x0a: /* byte prefix */
        وٿي_2++;
        اكسر;
    حالة 0x0b: /* word prefix */
        وٿي_2 += 2;
        اكسر;
    حالة 0x0c: /* dword prefix */
        وٿي_2 += 4;
        اكسر;
    حالة 0x0d: /* string prefix */
        طالما (ptr[وٿي_2])
            وٿي_2++;
        وٿي_2++;
        اكسر;
    حالة 0x10: /* scope op */
        وٿي_2 += parse_pkg_scope(s, ptr + وٿي_2);
        اكسر;
    حالة 0x11: /* buffer op */
        وٿي_2 += parse_pkg_بفر(s, ptr + وٿي_2);
        اكسر;
    حالة 0x12: /* package op */
    حالة 0x13: /* var package op */
        وٿي_2 += parse_pkg_skip(s, ptr + وٿي_2, ptr[0], 0);
        اكسر;
    حالة 0x14: /* method op */
        وٿي_2 += parse_pkg_skip(s, ptr + وٿي_2, ptr[0], 1);
        إذا (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        اكسر;
    حالة 0x5b: /* ext op prefix */
        وٿي_2++;
        بدل (ptr[1]) {
        حالة 0x01: /* mutex op */
            وٿي_2 += parse_namestring(s, ptr + وٿي_2, "mutex");
            وٿي_2++; /* sync flags */
            اكسر;
        حالة 0x80: /* op region op */
            وٿي_2 += parse_namestring(s, ptr + وٿي_2, "op region");
            وٿي_2++; /* region space */
            وٿي_2 += parse_termarg_int(ptr + وٿي_2, &s->error, عدم);
            وٿي_2 += parse_termarg_int(ptr + وٿي_2, &s->error, عدم);
            اكسر;
        حالة 0x81: /* field op */
        حالة 0x83: /* processor op */
        حالة 0x84: /* power resource op */
        حالة 0x85: /* thermal zone op */
            وٿي_2 += parse_pkg_skip(s, ptr + وٿي_2, 0x5b00 | ptr[1], 1);
            اكسر;
        حالة 0x82: /* device op */
            وٿي_2 += parse_pkg_device(s, ptr + وٿي_2);
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
    s->اونهائي--;

    أرجع وٿي_2;
}

ساكن فراغ parse_termlist(بنية parse_state *s,
                           u8 *ptr, صحيح وٿي_2, صحيح pkglength)
{
    لكل (;;) {
        وٿي_2 += parse_termobj(s, ptr + وٿي_2);
        إذا (وٿي_2 == pkglength)
            أرجع;
        إذا (وٿي_2 > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    وٿي_2, pkglength);
            s->error = 1;
            أرجع;
        }
        إذا (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    وٿي_2, pkglength);
            s->error = 0;
            أرجع;
        }
    }
}

ساكن بنية acpi_device *acpi_dsdt_ڳوليو(بنية acpi_device *prev,
                                          ثابت u8 *aml1, صحيح size1,
                                          ثابت u8 *aml2, صحيح size2)
{
    بنية acpi_device *dev;
    بنية hlist_ڳنڍ *ڳنڍ;

    إذا (!prev)
        ڳنڍ = acpi_devices.پهريون;
    وإلا
        ڳنڍ = prev->ڳنڍ.ايندڙ;

    لكل (; ڳنڍ != عدم; ڳنڍ = dev->ڳنڍ.ايندڙ) {
        dev = container_جو(ڳنڍ, بنية acpi_device, ڳنڍ);
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
        u64 قدر = 0;
        parse_termarg_int(dev->sta_aml + 5, عدم, &قدر);
        إذا (قدر == 0)
            أرجع 0; /* no */
        وإلا
            أرجع 1; /* yes */
    }
    أرجع -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

بنية acpi_device *acpi_dsdt_ڳوليو_string(بنية acpi_device *prev,
                                          ثابت حرف *hid)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع عدم;

    u8 aml[10];
    صحيح len = snprintf((حرف*)aml, حجم(aml), "\x0d%s", hid);
    أرجع acpi_dsdt_ڳوليو(prev, aml, len, عدم, 0);
}

بنية acpi_device *acpi_dsdt_ڳوليو_eisaid(بنية acpi_device *prev, u16 eisaid)
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
    أرجع acpi_dsdt_ڳوليو(prev, aml1, 5, aml2, len2);
}

حرف *acpi_dsdt_name(بنية acpi_device *dev)
{
    إذا (!CONFIG_ACPI_PARSE || !dev)
        أرجع عدم;
    أرجع dev->name;
}

صحيح acpi_dsdt_ڳوليو_io(بنية acpi_device *dev, u64 *min, u64 *max)
{
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_معلومات)
        أرجع -1;
    أرجع ڳوليو_resource(dev->crs_معلومات, dev->crs_سائيز,
                         1 /* I/O */, min, max);
}

صحيح acpi_dsdt_ڳوليو_mem(بنية acpi_device *dev, u64 *min, u64 *max)
{
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_معلومات)
        أرجع -1;
    أرجع ڳوليو_resource(dev->crs_معلومات, dev->crs_سائيز,
                         0 /* mem */, min, max);
}

صحيح acpi_dsdt_ڳوليو_irq(بنية acpi_device *dev, u64 *irq)
{
    u64 max;
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_معلومات)
        أرجع -1;
    أرجع ڳوليو_resource(dev->crs_معلومات, dev->crs_سائيز,
                         3 /* irq */, irq, &max);
}

صحيح acpi_dsdt_present_eisaid(u16 eisaid)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع -1; /* unknown */
    إذا (hlist_خالي_جايون(&acpi_devices))
        أرجع -1; /* unknown (no dsdt table) */

    بنية acpi_device *dev = acpi_dsdt_ڳوليو_eisaid(عدم, eisaid);
    أرجع acpi_dsdt_present(dev);
}

فراغ acpi_dsdt_parse(فراغ)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع;

    بنية fadt_descriptor_rev1 *fadt = ڳوليو_acpi_جدول(FACP_SIGNATURE);
    إذا (!fadt)
        أرجع;
    u8 *dsdt = (فراغ*)(fadt->dsdt);
    إذا (!dsdt)
        أرجع;

    u32 ڊيگهائي = *(u32*)(dsdt + 4);
    u32 وٿي_2 = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, ڊيگهائي);

    بنية parse_state s;
    memset(&s, 0, حجم(s));
    parse_termlist(&s, dsdt, وٿي_2, ڊيگهائي);

    إذا (!parse_dumpdevs)
        أرجع;

    بنية acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    لكل (dev = acpi_dsdt_ڳوليو(عدم, عدم, 0, عدم, 0);
         dev != عدم;
         dev = acpi_dsdt_ڳوليو(dev, عدم, 0, عدم, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        إذا (dev->hid_aml)
            dprintf(1, ", hid");
        إذا (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        إذا (dev->crs_معلومات)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        إذا (dev->crs_معلومات)
            print_resources("        ", dev->crs_معلومات, dev->crs_سائيز);
    }
}
