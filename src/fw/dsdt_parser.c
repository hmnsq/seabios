// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "list.h"   // hlist_*
#అన్తర్భావ "malloc.h" // malloc_*
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "util.h"
#అన్తర్భావ "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

నిర్మాణం acpi_device {
    నిర్మాణం hlist_కణుపు కణుపు;
    అక్షరం name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_దత్తాంశం;
    పూర్ణాంకం crs_కొలత;
};
స్థిర నిర్మాణం hlist_తల acpi_devices VARVERIFY32INIT;
స్థిర స్థిరరాశి పూర్ణాంకం parse_dumpdevs = 0;

నిర్మాణం parse_state {
    అక్షరం name[32];
    నిర్మాణం acpi_device *dev;
    పూర్ణాంకం error;
    పూర్ణాంకం లోతు;
};

స్థిర శూన్య parse_termlist(నిర్మాణం parse_state *s,
                           u8 *ptr, పూర్ణాంకం స్థానభ్రంశం_2, పూర్ణాంకం pkglength);

స్థిర శూన్య hex(స్థిరరాశి u8 *ptr, పూర్ణాంకం లెక్క, పూర్ణాంకం lvl, స్థిరరాశి అక్షరం *అంశం)
{
    పూర్ణాంకం l = 0, i;

    చేయి {
        dprintf(lvl, "%s: %04x:  ", అంశం, l);
        కొరకు (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        కొరకు (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } అయితే (l < లెక్క);
}

స్థిర u64 parse_resource_int(u8 *ptr, పూర్ణాంకం లెక్క)
{
    u64 విలువ = 0;
    పూర్ణాంకం సూచీ = 0;

    కొరకు (సూచీ = 0; సూచీ < లెక్క; సూచీ++)
        విలువ |= (u64)ptr[సూచీ] << (సూచీ * 8);
    తిరిగి విలువ;
}

స్థిర పూర్ణాంకం parse_resource_బిట్టు(u8 *ptr, పూర్ణాంకం లెక్క)
{
    పూర్ణాంకం బిట్టు;

    కొరకు (బిట్టు = 0; బిట్టు < లెక్క*8; బిట్టు++)
        ఒకవేళ (ptr[బిట్టు/8] & (1 << (బిట్టు%8)))
            తిరిగి బిట్టు;
    తిరిగి 0;
}

స్థిర పూర్ణాంకం parse_resource(u8 *ptr, పూర్ణాంకం నిడివి, పూర్ణాంకం *type, u64 *min, u64 *max)
{
    పూర్ణాంకం rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    ఒకవేళ (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        ఎంపిక (rname) {
        స్థితి 0x04: /* irq */
            *min = parse_resource_బిట్టు(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            ఆపు;
        స్థితి 0x0f: /* end marker */
            తిరిగి 0;
        స్థితి 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            ఒకవేళ (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            ఆపు;
        స్థితి 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            ఆపు;
        అప్రమేయ:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            ఆపు;
        }
    } లేకపోతే {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        ఎంపిక (rname) {
        స్థితి 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            ఆపు;
        స్థితి 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            ఆపు;
        స్థితి 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            ఆపు;
        స్థితి 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            ఆపు;
        స్థితి 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            ఆపు;
        అప్రమేయ:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            ఆపు;
        }
    }
    తిరిగి rsize;
}

స్థిర పూర్ణాంకం కనుగొను_resource(u8 *ptr, పూర్ణాంకం len, పూర్ణాంకం రకం_2, u64 *min, u64 *max)
{
    పూర్ణాంకం type, కొలత, స్థానభ్రంశం_2 = 0;

    చేయి {
        కొలత = parse_resource(ptr + స్థానభ్రంశం_2, len - స్థానభ్రంశం_2,
                              &type, min, max);
        ఒకవేళ (రకం_2 == type)
            తిరిగి 0;
        స్థానభ్రంశం_2 += కొలత;
    } అయితే (కొలత > 0 && స్థానభ్రంశం_2 < len);
    తిరిగి -1;
}

స్థిర పూర్ణాంకం print_resources(స్థిరరాశి అక్షరం *ఉపసర్గ, u8 *ptr, పూర్ణాంకం len)
{
    స్థిర స్థిరరాశి అక్షరం *typename[] = { "mem", "i/o", "bus" };
    పూర్ణాంకం type, కొలత, స్థానభ్రంశం_2 = 0;
    u64 min, max;

    చేయి {
        కొలత = parse_resource(ptr + స్థానభ్రంశం_2, len - స్థానభ్రంశం_2,
                              &type, &min, &max);
        ఎంపిక (type) {
        స్థితి 0:
        స్థితి 1:
        స్థితి 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    ఉపసర్గ, typename[type], min, max);
            ఆపు;
        స్థితి 3:
            dprintf(1, "%sirq %lld\n", ఉపసర్గ, min);
            ఆపు;
        }
        స్థానభ్రంశం_2 += కొలత;
    } అయితే (కొలత > 0 && స్థానభ్రంశం_2 < len);
    తిరిగి -1;
}

స్థిర పూర్ణాంకం parse_nameseg(u8 *ptr, అక్షరం **dst)
{
    ఒకవేళ (dst && *dst) {
        *(dst[0]++) = ptr[0];
        ఒకవేళ (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        ఒకవేళ (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        ఒకవేళ (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    తిరిగి 4;
}

స్థిర పూర్ణాంకం parse_namestring(నిర్మాణం parse_state *s,
                            u8 *ptr, స్థిరరాశి అక్షరం *అంశం)
{
    అక్షరం *dst = s->name;
    పూర్ణాంకం స్థానభ్రంశం_2 = 0;
    పూర్ణాంకం i, లెక్క;

    కొరకు (;;) {
        ఎంపిక (ptr[స్థానభ్రంశం_2]) {
        స్థితి 0: /* null name */
            స్థానభ్రంశం_2++;
            *(dst++) = 0;
            ఆపు;
        స్థితి 0x2e:
            స్థానభ్రంశం_2++;
            స్థానభ్రంశం_2 += parse_nameseg(ptr + స్థానభ్రంశం_2, &dst);
            *(dst++) = '.';
            స్థానభ్రంశం_2 += parse_nameseg(ptr + స్థానభ్రంశం_2, &dst);
            ఆపు;
        స్థితి 0x2f:
            స్థానభ్రంశం_2++;
            లెక్క = ptr[స్థానభ్రంశం_2];
            స్థానభ్రంశం_2++;
            కొరకు (i = 0; i < లెక్క; i++) {
                ఒకవేళ (i)
                    *(dst++) = '.';
                స్థానభ్రంశం_2 += parse_nameseg(ptr + స్థానభ్రంశం_2, &dst);
            }
            ఆపు;
        స్థితి '\\':
            *(dst++) = '\\';
            స్థానభ్రంశం_2++;
            కొనసాగు;
        స్థితి '^':
            *(dst++) = '^';
            స్థానభ్రంశం_2++;
            కొనసాగు;
        స్థితి 'A' ... 'Z':
        స్థితి '_':
            స్థానభ్రంశం_2 += parse_nameseg(ptr, &dst);
            ఆపు;
        అప్రమేయ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ఆపు;
        }
        ఆపు;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->లోతు,
            అంశం, s->name);
    తిరిగి స్థానభ్రంశం_2;
}

స్థిర పూర్ణాంకం parse_termarg_int(u8 *ptr, పూర్ణాంకం *error, u64 *dst)
{
    u64 విలువ;
    పూర్ణాంకం స్థానభ్రంశం_2 = 1;

    ఎంపిక (ptr[0]) {
    స్థితి 0x00: /* zero */
        విలువ = 0;
        ఆపు;
    స్థితి 0x01: /* one */
        విలువ = 1;
        ఆపు;
    స్థితి 0x0a: /* byte prefix */
        విలువ = ptr[1];
        స్థానభ్రంశం_2++;
        ఆపు;
    స్థితి 0x0b: /* word prefix */
        విలువ = ptr[1] |
            ((గుర్తులేని దీర్ఘ)ptr[2] << 8);
        స్థానభ్రంశం_2 += 2;
        ఆపు;
    స్థితి 0x0c: /* dword prefix */
        విలువ = ptr[1] |
            ((గుర్తులేని దీర్ఘ)ptr[2] << 8) |
            ((గుర్తులేని దీర్ఘ)ptr[3] << 16) |
            ((గుర్తులేని దీర్ఘ)ptr[4] << 24);
        స్థానభ్రంశం_2 += 4;
        ఆపు;
    అప్రమేయ:
        విలువ = 0;
        hex(ptr, 16, 3, __func__);
        ఒకవేళ (error)
            *error = 1;
        ఆపు;
    }

    ఒకవేళ (dst)
        *dst = విలువ;
    dprintf(5, "%s: 0x%llx\n", __func__, విలువ);
    తిరిగి స్థానభ్రంశం_2;
}

స్థిర పూర్ణాంకం parse_pkglength(u8 *ptr, పూర్ణాంకం *pkglength)
{
    పూర్ణాంకం స్థానభ్రంశం_2 = 2;

    *pkglength = 0;
    ఎంపిక (ptr[0] >> 6) {
    స్థితి 3:
        *pkglength |= ptr[3] << 20;
        స్థానభ్రంశం_2++;
    స్థితి 2:
        *pkglength |= ptr[2] << 12;
        స్థానభ్రంశం_2++;
    స్థితి 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        తిరిగి స్థానభ్రంశం_2;
    స్థితి 0:
    అప్రమేయ:
        *pkglength |= ptr[0] & 0x3f;
        తిరిగి 1;
    }
}

స్థిర పూర్ణాంకం parse_pkg_common(నిర్మాణం parse_state *s,
                            u8 *ptr, స్థిరరాశి అక్షరం *అంశం, పూర్ణాంకం *pkglength)
{
    పూర్ణాంకం స్థానభ్రంశం_2;

    స్థానభ్రంశం_2 = parse_pkglength(ptr, pkglength);
    స్థానభ్రంశం_2 += parse_namestring(s, ptr + స్థానభ్రంశం_2, అంశం);
    తిరిగి స్థానభ్రంశం_2;
}

స్థిర పూర్ణాంకం parse_pkg_scope(నిర్మాణం parse_state *s,
                           u8 *ptr)
{
    పూర్ణాంకం స్థానభ్రంశం_2, pkglength;

    స్థానభ్రంశం_2 = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, స్థానభ్రంశం_2, pkglength);
    తిరిగి pkglength;
}

స్థిర పూర్ణాంకం parse_pkg_device(నిర్మాణం parse_state *s,
                            u8 *ptr)
{
    పూర్ణాంకం స్థానభ్రంశం_2, pkglength;

    స్థానభ్రంశం_2 = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(పరిమాణం(నిర్మాణం acpi_device));
    ఒకవేళ (!s->dev) {
        warn_noalloc();
        s->error = 1;
        తిరిగి pkglength;
    }

    memset(s->dev, 0, పరిమాణం(నిర్మాణం acpi_device));
    hlist_కూడు_తల(&s->dev->కణుపు, &acpi_devices);
    strtcpy(s->dev->name, s->name, పరిమాణం(s->name));
    parse_termlist(s, ptr, స్థానభ్రంశం_2, pkglength);
    s->dev = ఖాళీ;

    తిరిగి pkglength;
}

స్థిర పూర్ణాంకం parse_pkg_బఫరు(నిర్మాణం parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    పూర్ణాంకం pkglength, స్థానభ్రంశం_2;

    స్థానభ్రంశం_2 = parse_pkglength(ptr, &pkglength);
    స్థానభ్రంశం_2 += parse_termarg_int(ptr + స్థానభ్రంశం_2, &s->error, &blen);
    ఒకవేళ (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_దత్తాంశం = ptr + స్థానభ్రంశం_2;
        s->dev->crs_కొలత = blen;
    }
    తిరిగి pkglength;
}

స్థిర పూర్ణాంకం parse_pkg_skip(నిర్మాణం parse_state *s,
                          u8 *ptr, పూర్ణాంకం op, పూర్ణాంకం name)
{
    పూర్ణాంకం pkglength, స్థానభ్రంశం_2;
    అక్షరం అంశం[8];

    snprintf(అంశం, పరిమాణం(అంశం), "op %x", op);
    స్థానభ్రంశం_2 = parse_pkglength(ptr, &pkglength);
    ఒకవేళ (name) {
        parse_namestring(s, ptr + స్థానభ్రంశం_2, అంశం);
    } లేకపోతే {
        dprintf(5, "%s: %s (%d)\n", __func__, అంశం, pkglength);
    }
    తిరిగి pkglength;
}

స్థిర పూర్ణాంకం parse_termobj(నిర్మాణం parse_state *s,
                         u8 *ptr)
{
    పూర్ణాంకం స్థానభ్రంశం_2 = 1;

    ఒకవేళ (s->లోతు == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        తిరిగి స్థానభ్రంశం_2;
    }

    s->లోతు++;
    ఎంపిక (ptr[0]) {
    స్థితి 0x00: /* zero */
        ఆపు;
    స్థితి 0x01: /* one */
        ఆపు;
    స్థితి 0x06: /* AliasOp */
        స్థానభ్రంశం_2 += parse_namestring(s, ptr + స్థానభ్రంశం_2, "SourceObject");
        స్థానభ్రంశం_2 += parse_namestring(s, ptr + స్థానభ్రంశం_2, "AliasObject");
        ఆపు;
    స్థితి 0x08: /* name op */
        స్థానభ్రంశం_2 += parse_namestring(s, ptr + స్థానభ్రంశం_2, "name");
        స్థానభ్రంశం_2 += parse_termobj(s, ptr + స్థానభ్రంశం_2);
        ఒకవేళ (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        ఒకవేళ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ఆపు;
    స్థితి 0x0a: /* byte prefix */
        స్థానభ్రంశం_2++;
        ఆపు;
    స్థితి 0x0b: /* word prefix */
        స్థానభ్రంశం_2 += 2;
        ఆపు;
    స్థితి 0x0c: /* dword prefix */
        స్థానభ్రంశం_2 += 4;
        ఆపు;
    స్థితి 0x0d: /* string prefix */
        అయితే (ptr[స్థానభ్రంశం_2])
            స్థానభ్రంశం_2++;
        స్థానభ్రంశం_2++;
        ఆపు;
    స్థితి 0x10: /* scope op */
        స్థానభ్రంశం_2 += parse_pkg_scope(s, ptr + స్థానభ్రంశం_2);
        ఆపు;
    స్థితి 0x11: /* buffer op */
        స్థానభ్రంశం_2 += parse_pkg_బఫరు(s, ptr + స్థానభ్రంశం_2);
        ఆపు;
    స్థితి 0x12: /* package op */
    స్థితి 0x13: /* var package op */
        స్థానభ్రంశం_2 += parse_pkg_skip(s, ptr + స్థానభ్రంశం_2, ptr[0], 0);
        ఆపు;
    స్థితి 0x14: /* method op */
        స్థానభ్రంశం_2 += parse_pkg_skip(s, ptr + స్థానభ్రంశం_2, ptr[0], 1);
        ఒకవేళ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ఆపు;
    స్థితి 0x5b: /* ext op prefix */
        స్థానభ్రంశం_2++;
        ఎంపిక (ptr[1]) {
        స్థితి 0x01: /* mutex op */
            స్థానభ్రంశం_2 += parse_namestring(s, ptr + స్థానభ్రంశం_2, "mutex");
            స్థానభ్రంశం_2++; /* sync flags */
            ఆపు;
        స్థితి 0x80: /* op region op */
            స్థానభ్రంశం_2 += parse_namestring(s, ptr + స్థానభ్రంశం_2, "op region");
            స్థానభ్రంశం_2++; /* region space */
            స్థానభ్రంశం_2 += parse_termarg_int(ptr + స్థానభ్రంశం_2, &s->error, ఖాళీ);
            స్థానభ్రంశం_2 += parse_termarg_int(ptr + స్థానభ్రంశం_2, &s->error, ఖాళీ);
            ఆపు;
        స్థితి 0x81: /* field op */
        స్థితి 0x83: /* processor op */
        స్థితి 0x84: /* power resource op */
        స్థితి 0x85: /* thermal zone op */
            స్థానభ్రంశం_2 += parse_pkg_skip(s, ptr + స్థానభ్రంశం_2, 0x5b00 | ptr[1], 1);
            ఆపు;
        స్థితి 0x82: /* device op */
            స్థానభ్రంశం_2 += parse_pkg_device(s, ptr + స్థానభ్రంశం_2);
            ఆపు;
        అప్రమేయ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ఆపు;
        }
        ఆపు;
    అప్రమేయ:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        ఆపు;
    }
    s->లోతు--;

    తిరిగి స్థానభ్రంశం_2;
}

స్థిర శూన్య parse_termlist(నిర్మాణం parse_state *s,
                           u8 *ptr, పూర్ణాంకం స్థానభ్రంశం_2, పూర్ణాంకం pkglength)
{
    కొరకు (;;) {
        స్థానభ్రంశం_2 += parse_termobj(s, ptr + స్థానభ్రంశం_2);
        ఒకవేళ (స్థానభ్రంశం_2 == pkglength)
            తిరిగి;
        ఒకవేళ (స్థానభ్రంశం_2 > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    స్థానభ్రంశం_2, pkglength);
            s->error = 1;
            తిరిగి;
        }
        ఒకవేళ (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    స్థానభ్రంశం_2, pkglength);
            s->error = 0;
            తిరిగి;
        }
    }
}

స్థిర నిర్మాణం acpi_device *acpi_dsdt_కనుగొను(నిర్మాణం acpi_device *prev,
                                          స్థిరరాశి u8 *aml1, పూర్ణాంకం size1,
                                          స్థిరరాశి u8 *aml2, పూర్ణాంకం size2)
{
    నిర్మాణం acpi_device *dev;
    నిర్మాణం hlist_కణుపు *కణుపు;

    ఒకవేళ (!prev)
        కణుపు = acpi_devices.మొదటి;
    లేకపోతే
        కణుపు = prev->కణుపు.తదుపరి;

    కొరకు (; కణుపు != ఖాళీ; కణుపు = dev->కణుపు.తదుపరి) {
        dev = container_యొక్క(కణుపు, నిర్మాణం acpi_device, కణుపు);
        ఒకవేళ (!aml1 && !aml2)
            తిరిగి dev;
        ఒకవేళ (!dev->hid_aml)
            కొనసాగు;
        ఒకవేళ (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            తిరిగి dev;
        ఒకవేళ (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            తిరిగి dev;
    }
    తిరిగి ఖాళీ;
}

స్థిర పూర్ణాంకం acpi_dsdt_present(నిర్మాణం acpi_device *dev)
{
    ఒకవేళ (!dev)
        తిరిగి 0; /* no */
    ఒకవేళ (!dev->sta_aml)
        తిరిగి 1; /* yes */
    ఒకవేళ (dev->sta_aml[0] == 0x14)
        తిరిగి -1; /* unknown (can't evaluate method) */
    ఒకవేళ (dev->sta_aml[0] == 0x08) {
        u64 విలువ = 0;
        parse_termarg_int(dev->sta_aml + 5, ఖాళీ, &విలువ);
        ఒకవేళ (విలువ == 0)
            తిరిగి 0; /* no */
        లేకపోతే
            తిరిగి 1; /* yes */
    }
    తిరిగి -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

నిర్మాణం acpi_device *acpi_dsdt_కనుగొను_string(నిర్మాణం acpi_device *prev,
                                          స్థిరరాశి అక్షరం *hid)
{
    ఒకవేళ (!CONFIG_ACPI_PARSE)
        తిరిగి ఖాళీ;

    u8 aml[10];
    పూర్ణాంకం len = snprintf((అక్షరం*)aml, పరిమాణం(aml), "\x0d%s", hid);
    తిరిగి acpi_dsdt_కనుగొను(prev, aml, len, ఖాళీ, 0);
}

నిర్మాణం acpi_device *acpi_dsdt_కనుగొను_eisaid(నిర్మాణం acpi_device *prev, u16 eisaid)
{
    ఒకవేళ (!CONFIG_ACPI_PARSE)
        తిరిగి ఖాళీ;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    పూర్ణాంకం len2 = snprintf((అక్షరం*)aml2, పరిమాణం(aml2), "\x0dPNP%04X", eisaid);
    తిరిగి acpi_dsdt_కనుగొను(prev, aml1, 5, aml2, len2);
}

అక్షరం *acpi_dsdt_name(నిర్మాణం acpi_device *dev)
{
    ఒకవేళ (!CONFIG_ACPI_PARSE || !dev)
        తిరిగి ఖాళీ;
    తిరిగి dev->name;
}

పూర్ణాంకం acpi_dsdt_కనుగొను_io(నిర్మాణం acpi_device *dev, u64 *min, u64 *max)
{
    ఒకవేళ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_దత్తాంశం)
        తిరిగి -1;
    తిరిగి కనుగొను_resource(dev->crs_దత్తాంశం, dev->crs_కొలత,
                         1 /* I/O */, min, max);
}

పూర్ణాంకం acpi_dsdt_కనుగొను_mem(నిర్మాణం acpi_device *dev, u64 *min, u64 *max)
{
    ఒకవేళ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_దత్తాంశం)
        తిరిగి -1;
    తిరిగి కనుగొను_resource(dev->crs_దత్తాంశం, dev->crs_కొలత,
                         0 /* mem */, min, max);
}

పూర్ణాంకం acpi_dsdt_కనుగొను_irq(నిర్మాణం acpi_device *dev, u64 *irq)
{
    u64 max;
    ఒకవేళ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_దత్తాంశం)
        తిరిగి -1;
    తిరిగి కనుగొను_resource(dev->crs_దత్తాంశం, dev->crs_కొలత,
                         3 /* irq */, irq, &max);
}

పూర్ణాంకం acpi_dsdt_present_eisaid(u16 eisaid)
{
    ఒకవేళ (!CONFIG_ACPI_PARSE)
        తిరిగి -1; /* unknown */
    ఒకవేళ (hlist_ఖాళీలు(&acpi_devices))
        తిరిగి -1; /* unknown (no dsdt table) */

    నిర్మాణం acpi_device *dev = acpi_dsdt_కనుగొను_eisaid(ఖాళీ, eisaid);
    తిరిగి acpi_dsdt_present(dev);
}

శూన్య acpi_dsdt_parse(శూన్య)
{
    ఒకవేళ (!CONFIG_ACPI_PARSE)
        తిరిగి;

    నిర్మాణం fadt_descriptor_rev1 *fadt = కనుగొను_acpi_పట్టిక(FACP_SIGNATURE);
    ఒకవేళ (!fadt)
        తిరిగి;
    u8 *dsdt = (శూన్య*)(fadt->dsdt);
    ఒకవేళ (!dsdt)
        తిరిగి;

    u32 నిడివి = *(u32*)(dsdt + 4);
    u32 స్థానభ్రంశం_2 = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, నిడివి);

    నిర్మాణం parse_state s;
    memset(&s, 0, పరిమాణం(s));
    parse_termlist(&s, dsdt, స్థానభ్రంశం_2, నిడివి);

    ఒకవేళ (!parse_dumpdevs)
        తిరిగి;

    నిర్మాణం acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    కొరకు (dev = acpi_dsdt_కనుగొను(ఖాళీ, ఖాళీ, 0, ఖాళీ, 0);
         dev != ఖాళీ;
         dev = acpi_dsdt_కనుగొను(dev, ఖాళీ, 0, ఖాళీ, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        ఒకవేళ (dev->hid_aml)
            dprintf(1, ", hid");
        ఒకవేళ (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        ఒకవేళ (dev->crs_దత్తాంశం)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        ఒకవేళ (dev->crs_దత్తాంశం)
            print_resources("        ", dev->crs_దత్తాంశం, dev->crs_కొలత);
    }
}
