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
    بنية hlist_گرہ گرہ;
    حرف name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_انکڑے;
    صحيح crs_ناپ;
};
ساكن بنية hlist_سر acpi_devices VARVERIFY32INIT;
ساكن ثابت صحيح parse_dumpdevs = 0;

بنية parse_state {
    حرف name[32];
    بنية acpi_device *dev;
    صحيح error;
    صحيح ڈونگھائی;
};

ساكن فراغ parse_termlist(بنية parse_state *s,
                           u8 *ptr, صحيح وتھ_2, صحيح pkglength);

ساكن فراغ hex(ثابت u8 *ptr, صحيح گنتی, صحيح lvl, ثابت حرف *شے)
{
    صحيح l = 0, i;

    افعل {
        dprintf(lvl, "%s: %04x:  ", شے, l);
        لكل (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        لكل (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } طالما (l < گنتی);
}

ساكن u64 parse_resource_int(u8 *ptr, صحيح گنتی)
{
    u64 قدر = 0;
    صحيح اشاریہ = 0;

    لكل (اشاریہ = 0; اشاریہ < گنتی; اشاریہ++)
        قدر |= (u64)ptr[اشاریہ] << (اشاریہ * 8);
    أرجع قدر;
}

ساكن صحيح parse_resource_ثنائی_ہندسہ(u8 *ptr, صحيح گنتی)
{
    صحيح ثنائی_ہندسہ;

    لكل (ثنائی_ہندسہ = 0; ثنائی_ہندسہ < گنتی*8; ثنائی_ہندسہ++)
        إذا (ptr[ثنائی_ہندسہ/8] & (1 << (ثنائی_ہندسہ%8)))
            أرجع ثنائی_ہندسہ;
    أرجع 0;
}

ساكن صحيح parse_resource(u8 *ptr, صحيح لمائی, صحيح *type, u64 *min, u64 *max)
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
            *min = parse_resource_ثنائی_ہندسہ(ptr + 1, rsize);
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

ساكن صحيح لبھو_resource(u8 *ptr, صحيح len, صحيح قسم, u64 *min, u64 *max)
{
    صحيح type, ناپ, وتھ_2 = 0;

    افعل {
        ناپ = parse_resource(ptr + وتھ_2, len - وتھ_2,
                              &type, min, max);
        إذا (قسم == type)
            أرجع 0;
        وتھ_2 += ناپ;
    } طالما (ناپ > 0 && وتھ_2 < len);
    أرجع -1;
}

ساكن صحيح print_resources(ثابت حرف *اگیتر, u8 *ptr, صحيح len)
{
    ساكن ثابت حرف *typename[] = { "mem", "i/o", "bus" };
    صحيح type, ناپ, وتھ_2 = 0;
    u64 min, max;

    افعل {
        ناپ = parse_resource(ptr + وتھ_2, len - وتھ_2,
                              &type, &min, &max);
        بدل (type) {
        حالة 0:
        حالة 1:
        حالة 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    اگیتر, typename[type], min, max);
            اكسر;
        حالة 3:
            dprintf(1, "%sirq %lld\n", اگیتر, min);
            اكسر;
        }
        وتھ_2 += ناپ;
    } طالما (ناپ > 0 && وتھ_2 < len);
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
                            u8 *ptr, ثابت حرف *شے)
{
    حرف *dst = s->name;
    صحيح وتھ_2 = 0;
    صحيح i, گنتی;

    لكل (;;) {
        بدل (ptr[وتھ_2]) {
        حالة 0: /* null name */
            وتھ_2++;
            *(dst++) = 0;
            اكسر;
        حالة 0x2e:
            وتھ_2++;
            وتھ_2 += parse_nameseg(ptr + وتھ_2, &dst);
            *(dst++) = '.';
            وتھ_2 += parse_nameseg(ptr + وتھ_2, &dst);
            اكسر;
        حالة 0x2f:
            وتھ_2++;
            گنتی = ptr[وتھ_2];
            وتھ_2++;
            لكل (i = 0; i < گنتی; i++) {
                إذا (i)
                    *(dst++) = '.';
                وتھ_2 += parse_nameseg(ptr + وتھ_2, &dst);
            }
            اكسر;
        حالة '\\':
            *(dst++) = '\\';
            وتھ_2++;
            استمر;
        حالة '^':
            *(dst++) = '^';
            وتھ_2++;
            استمر;
        حالة 'A' ... 'Z':
        حالة '_':
            وتھ_2 += parse_nameseg(ptr, &dst);
            اكسر;
        افتراضي:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            اكسر;
        }
        اكسر;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->ڈونگھائی,
            شے, s->name);
    أرجع وتھ_2;
}

ساكن صحيح parse_termarg_int(u8 *ptr, صحيح *error, u64 *dst)
{
    u64 قدر;
    صحيح وتھ_2 = 1;

    بدل (ptr[0]) {
    حالة 0x00: /* zero */
        قدر = 0;
        اكسر;
    حالة 0x01: /* one */
        قدر = 1;
        اكسر;
    حالة 0x0a: /* byte prefix */
        قدر = ptr[1];
        وتھ_2++;
        اكسر;
    حالة 0x0b: /* word prefix */
        قدر = ptr[1] |
            ((بنانشانی لما)ptr[2] << 8);
        وتھ_2 += 2;
        اكسر;
    حالة 0x0c: /* dword prefix */
        قدر = ptr[1] |
            ((بنانشانی لما)ptr[2] << 8) |
            ((بنانشانی لما)ptr[3] << 16) |
            ((بنانشانی لما)ptr[4] << 24);
        وتھ_2 += 4;
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
    أرجع وتھ_2;
}

ساكن صحيح parse_pkglength(u8 *ptr, صحيح *pkglength)
{
    صحيح وتھ_2 = 2;

    *pkglength = 0;
    بدل (ptr[0] >> 6) {
    حالة 3:
        *pkglength |= ptr[3] << 20;
        وتھ_2++;
    حالة 2:
        *pkglength |= ptr[2] << 12;
        وتھ_2++;
    حالة 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        أرجع وتھ_2;
    حالة 0:
    افتراضي:
        *pkglength |= ptr[0] & 0x3f;
        أرجع 1;
    }
}

ساكن صحيح parse_pkg_common(بنية parse_state *s,
                            u8 *ptr, ثابت حرف *شے, صحيح *pkglength)
{
    صحيح وتھ_2;

    وتھ_2 = parse_pkglength(ptr, pkglength);
    وتھ_2 += parse_namestring(s, ptr + وتھ_2, شے);
    أرجع وتھ_2;
}

ساكن صحيح parse_pkg_scope(بنية parse_state *s,
                           u8 *ptr)
{
    صحيح وتھ_2, pkglength;

    وتھ_2 = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, وتھ_2, pkglength);
    أرجع pkglength;
}

ساكن صحيح parse_pkg_device(بنية parse_state *s,
                            u8 *ptr)
{
    صحيح وتھ_2, pkglength;

    وتھ_2 = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(حجم(بنية acpi_device));
    إذا (!s->dev) {
        warn_noalloc();
        s->error = 1;
        أرجع pkglength;
    }

    memset(s->dev, 0, حجم(بنية acpi_device));
    hlist_جوڑو_سر(&s->dev->گرہ, &acpi_devices);
    strtcpy(s->dev->name, s->name, حجم(s->name));
    parse_termlist(s, ptr, وتھ_2, pkglength);
    s->dev = عدم;

    أرجع pkglength;
}

ساكن صحيح parse_pkg_بفر(بنية parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    صحيح pkglength, وتھ_2;

    وتھ_2 = parse_pkglength(ptr, &pkglength);
    وتھ_2 += parse_termarg_int(ptr + وتھ_2, &s->error, &blen);
    إذا (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_انکڑے = ptr + وتھ_2;
        s->dev->crs_ناپ = blen;
    }
    أرجع pkglength;
}

ساكن صحيح parse_pkg_skip(بنية parse_state *s,
                          u8 *ptr, صحيح op, صحيح name)
{
    صحيح pkglength, وتھ_2;
    حرف شے[8];

    snprintf(شے, حجم(شے), "op %x", op);
    وتھ_2 = parse_pkglength(ptr, &pkglength);
    إذا (name) {
        parse_namestring(s, ptr + وتھ_2, شے);
    } وإلا {
        dprintf(5, "%s: %s (%d)\n", __func__, شے, pkglength);
    }
    أرجع pkglength;
}

ساكن صحيح parse_termobj(بنية parse_state *s,
                         u8 *ptr)
{
    صحيح وتھ_2 = 1;

    إذا (s->ڈونگھائی == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        أرجع وتھ_2;
    }

    s->ڈونگھائی++;
    بدل (ptr[0]) {
    حالة 0x00: /* zero */
        اكسر;
    حالة 0x01: /* one */
        اكسر;
    حالة 0x06: /* AliasOp */
        وتھ_2 += parse_namestring(s, ptr + وتھ_2, "SourceObject");
        وتھ_2 += parse_namestring(s, ptr + وتھ_2, "AliasObject");
        اكسر;
    حالة 0x08: /* name op */
        وتھ_2 += parse_namestring(s, ptr + وتھ_2, "name");
        وتھ_2 += parse_termobj(s, ptr + وتھ_2);
        إذا (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        إذا (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        اكسر;
    حالة 0x0a: /* byte prefix */
        وتھ_2++;
        اكسر;
    حالة 0x0b: /* word prefix */
        وتھ_2 += 2;
        اكسر;
    حالة 0x0c: /* dword prefix */
        وتھ_2 += 4;
        اكسر;
    حالة 0x0d: /* string prefix */
        طالما (ptr[وتھ_2])
            وتھ_2++;
        وتھ_2++;
        اكسر;
    حالة 0x10: /* scope op */
        وتھ_2 += parse_pkg_scope(s, ptr + وتھ_2);
        اكسر;
    حالة 0x11: /* buffer op */
        وتھ_2 += parse_pkg_بفر(s, ptr + وتھ_2);
        اكسر;
    حالة 0x12: /* package op */
    حالة 0x13: /* var package op */
        وتھ_2 += parse_pkg_skip(s, ptr + وتھ_2, ptr[0], 0);
        اكسر;
    حالة 0x14: /* method op */
        وتھ_2 += parse_pkg_skip(s, ptr + وتھ_2, ptr[0], 1);
        إذا (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        اكسر;
    حالة 0x5b: /* ext op prefix */
        وتھ_2++;
        بدل (ptr[1]) {
        حالة 0x01: /* mutex op */
            وتھ_2 += parse_namestring(s, ptr + وتھ_2, "mutex");
            وتھ_2++; /* sync flags */
            اكسر;
        حالة 0x80: /* op region op */
            وتھ_2 += parse_namestring(s, ptr + وتھ_2, "op region");
            وتھ_2++; /* region space */
            وتھ_2 += parse_termarg_int(ptr + وتھ_2, &s->error, عدم);
            وتھ_2 += parse_termarg_int(ptr + وتھ_2, &s->error, عدم);
            اكسر;
        حالة 0x81: /* field op */
        حالة 0x83: /* processor op */
        حالة 0x84: /* power resource op */
        حالة 0x85: /* thermal zone op */
            وتھ_2 += parse_pkg_skip(s, ptr + وتھ_2, 0x5b00 | ptr[1], 1);
            اكسر;
        حالة 0x82: /* device op */
            وتھ_2 += parse_pkg_device(s, ptr + وتھ_2);
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
    s->ڈونگھائی--;

    أرجع وتھ_2;
}

ساكن فراغ parse_termlist(بنية parse_state *s,
                           u8 *ptr, صحيح وتھ_2, صحيح pkglength)
{
    لكل (;;) {
        وتھ_2 += parse_termobj(s, ptr + وتھ_2);
        إذا (وتھ_2 == pkglength)
            أرجع;
        إذا (وتھ_2 > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    وتھ_2, pkglength);
            s->error = 1;
            أرجع;
        }
        إذا (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    وتھ_2, pkglength);
            s->error = 0;
            أرجع;
        }
    }
}

ساكن بنية acpi_device *acpi_dsdt_لبھو(بنية acpi_device *prev,
                                          ثابت u8 *aml1, صحيح size1,
                                          ثابت u8 *aml2, صحيح size2)
{
    بنية acpi_device *dev;
    بنية hlist_گرہ *گرہ;

    إذا (!prev)
        گرہ = acpi_devices.پہلا;
    وإلا
        گرہ = prev->گرہ.اگلا;

    لكل (; گرہ != عدم; گرہ = dev->گرہ.اگلا) {
        dev = container_دا(گرہ, بنية acpi_device, گرہ);
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

بنية acpi_device *acpi_dsdt_لبھو_string(بنية acpi_device *prev,
                                          ثابت حرف *hid)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع عدم;

    u8 aml[10];
    صحيح len = snprintf((حرف*)aml, حجم(aml), "\x0d%s", hid);
    أرجع acpi_dsdt_لبھو(prev, aml, len, عدم, 0);
}

بنية acpi_device *acpi_dsdt_لبھو_eisaid(بنية acpi_device *prev, u16 eisaid)
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
    أرجع acpi_dsdt_لبھو(prev, aml1, 5, aml2, len2);
}

حرف *acpi_dsdt_name(بنية acpi_device *dev)
{
    إذا (!CONFIG_ACPI_PARSE || !dev)
        أرجع عدم;
    أرجع dev->name;
}

صحيح acpi_dsdt_لبھو_io(بنية acpi_device *dev, u64 *min, u64 *max)
{
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_انکڑے)
        أرجع -1;
    أرجع لبھو_resource(dev->crs_انکڑے, dev->crs_ناپ,
                         1 /* I/O */, min, max);
}

صحيح acpi_dsdt_لبھو_mem(بنية acpi_device *dev, u64 *min, u64 *max)
{
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_انکڑے)
        أرجع -1;
    أرجع لبھو_resource(dev->crs_انکڑے, dev->crs_ناپ,
                         0 /* mem */, min, max);
}

صحيح acpi_dsdt_لبھو_irq(بنية acpi_device *dev, u64 *irq)
{
    u64 max;
    إذا (!CONFIG_ACPI_PARSE || !dev || !dev->crs_انکڑے)
        أرجع -1;
    أرجع لبھو_resource(dev->crs_انکڑے, dev->crs_ناپ,
                         3 /* irq */, irq, &max);
}

صحيح acpi_dsdt_present_eisaid(u16 eisaid)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع -1; /* unknown */
    إذا (hlist_خالی_تھاواں(&acpi_devices))
        أرجع -1; /* unknown (no dsdt table) */

    بنية acpi_device *dev = acpi_dsdt_لبھو_eisaid(عدم, eisaid);
    أرجع acpi_dsdt_present(dev);
}

فراغ acpi_dsdt_parse(فراغ)
{
    إذا (!CONFIG_ACPI_PARSE)
        أرجع;

    بنية fadt_descriptor_rev1 *fadt = لبھو_acpi_جدول(FACP_SIGNATURE);
    إذا (!fadt)
        أرجع;
    u8 *dsdt = (فراغ*)(fadt->dsdt);
    إذا (!dsdt)
        أرجع;

    u32 لمائی = *(u32*)(dsdt + 4);
    u32 وتھ_2 = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, لمائی);

    بنية parse_state s;
    memset(&s, 0, حجم(s));
    parse_termlist(&s, dsdt, وتھ_2, لمائی);

    إذا (!parse_dumpdevs)
        أرجع;

    بنية acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    لكل (dev = acpi_dsdt_لبھو(عدم, عدم, 0, عدم, 0);
         dev != عدم;
         dev = acpi_dsdt_لبھو(dev, عدم, 0, عدم, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        إذا (dev->hid_aml)
            dprintf(1, ", hid");
        إذا (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        إذا (dev->crs_انکڑے)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        إذا (dev->crs_انکڑے)
            print_resources("        ", dev->crs_انکڑے, dev->crs_ناپ);
    }
}
