// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "list.h"   // hlist_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "util.h"
#ಅನ್ತರ್ಭಾವ "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

ರಚನೆ acpi_device {
    ರಚನೆ hlist_ಗಂಟು ಗಂಟು;
    ಅಕ್ಷರ name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_ದತ್ತಾಂಶ;
    ಪೂರ್ಣಾಂಕ crs_ಗಾತ್ರ;
};
ಸ್ಥಿರ ರಚನೆ hlist_ತಲೆ acpi_devices VARVERIFY32INIT;
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ parse_dumpdevs = 0;

ರಚನೆ parse_state {
    ಅಕ್ಷರ name[32];
    ರಚನೆ acpi_device *dev;
    ಪೂರ್ಣಾಂಕ error;
    ಪೂರ್ಣಾಂಕ ಆಳ;
};

ಸ್ಥಿರ ಶೂನ್ಯ parse_termlist(ರಚನೆ parse_state *s,
                           u8 *ptr, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2, ಪೂರ್ಣಾಂಕ pkglength);

ಸ್ಥಿರ ಶೂನ್ಯ hex(ಸ್ಥಿರಾಂಕ u8 *ptr, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2, ಪೂರ್ಣಾಂಕ lvl, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಅಂಶ)
{
    ಪೂರ್ಣಾಂಕ l = 0, i;

    ಮಾಡು {
        dprintf(lvl, "%s: %04x:  ", ಅಂಶ, l);
        ಗಾಗಿ (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        ಗಾಗಿ (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } ಆಗ (l < ಎಣಿಕೆ_2);
}

ಸ್ಥಿರ u64 parse_resource_int(u8 *ptr, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
    u64 ಮೌಲ್ಯ = 0;
    ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = 0;

    ಗಾಗಿ (ಸೂಚ್ಯಂಕ = 0; ಸೂಚ್ಯಂಕ < ಎಣಿಕೆ_2; ಸೂಚ್ಯಂಕ++)
        ಮೌಲ್ಯ |= (u64)ptr[ಸೂಚ್ಯಂಕ] << (ಸೂಚ್ಯಂಕ * 8);
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_resource_ಬಿಟ್(u8 *ptr, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
    ಪೂರ್ಣಾಂಕ ಬಿಟ್;

    ಗಾಗಿ (ಬಿಟ್ = 0; ಬಿಟ್ < ಎಣಿಕೆ_2*8; ಬಿಟ್++)
        ಆದರೆ (ptr[ಬಿಟ್/8] & (1 << (ಬಿಟ್%8)))
            ಹಿಂತಿರುಗಿ ಬಿಟ್;
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_resource(u8 *ptr, ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ, ಪೂರ್ಣಾಂಕ *type, u64 *min, u64 *max)
{
    ಪೂರ್ಣಾಂಕ rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    ಆದರೆ (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        ಆಯ್ಕೆ (rname) {
        ಸ್ಥಿತಿ 0x04: /* irq */
            *min = parse_resource_ಬಿಟ್(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x0f: /* end marker */
            ಹಿಂತಿರುಗಿ 0;
        ಸ್ಥಿತಿ 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            ಆದರೆ (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            ನಿಲ್ಲಿಸು;
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        ಆಯ್ಕೆ (rname) {
        ಸ್ಥಿತಿ 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            ನಿಲ್ಲಿಸು;
        }
    }
    ಹಿಂತಿರುಗಿ rsize;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕಂಡುಹಿಡಿ_resource(u8 *ptr, ಪೂರ್ಣಾಂಕ len, ಪೂರ್ಣಾಂಕ ಬಗೆ, u64 *min, u64 *max)
{
    ಪೂರ್ಣಾಂಕ type, ಗಾತ್ರ_2, ಸ್ಥಾನಾಂತರ_2 = 0;

    ಮಾಡು {
        ಗಾತ್ರ_2 = parse_resource(ptr + ಸ್ಥಾನಾಂತರ_2, len - ಸ್ಥಾನಾಂತರ_2,
                              &type, min, max);
        ಆದರೆ (ಬಗೆ == type)
            ಹಿಂತಿರುಗಿ 0;
        ಸ್ಥಾನಾಂತರ_2 += ಗಾತ್ರ_2;
    } ಆಗ (ಗಾತ್ರ_2 > 0 && ಸ್ಥಾನಾಂತರ_2 < len);
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ print_resources(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, u8 *ptr, ಪೂರ್ಣಾಂಕ len)
{
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *typename[] = { "mem", "i/o", "bus" };
    ಪೂರ್ಣಾಂಕ type, ಗಾತ್ರ_2, ಸ್ಥಾನಾಂತರ_2 = 0;
    u64 min, max;

    ಮಾಡು {
        ಗಾತ್ರ_2 = parse_resource(ptr + ಸ್ಥಾನಾಂತರ_2, len - ಸ್ಥಾನಾಂತರ_2,
                              &type, &min, &max);
        ಆಯ್ಕೆ (type) {
        ಸ್ಥಿತಿ 0:
        ಸ್ಥಿತಿ 1:
        ಸ್ಥಿತಿ 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    ಪೂರ್ವಪ್ರತ್ಯಯ, typename[type], min, max);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 3:
            dprintf(1, "%sirq %lld\n", ಪೂರ್ವಪ್ರತ್ಯಯ, min);
            ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಾನಾಂತರ_2 += ಗಾತ್ರ_2;
    } ಆಗ (ಗಾತ್ರ_2 > 0 && ಸ್ಥಾನಾಂತರ_2 < len);
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_nameseg(u8 *ptr, ಅಕ್ಷರ **dst)
{
    ಆದರೆ (dst && *dst) {
        *(dst[0]++) = ptr[0];
        ಆದರೆ (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        ಆದರೆ (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        ಆದರೆ (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    ಹಿಂತಿರುಗಿ 4;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_namestring(ರಚನೆ parse_state *s,
                            u8 *ptr, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಅಂಶ)
{
    ಅಕ್ಷರ *dst = s->name;
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2 = 0;
    ಪೂರ್ಣಾಂಕ i, ಎಣಿಕೆ_2;

    ಗಾಗಿ (;;) {
        ಆಯ್ಕೆ (ptr[ಸ್ಥಾನಾಂತರ_2]) {
        ಸ್ಥಿತಿ 0: /* null name */
            ಸ್ಥಾನಾಂತರ_2++;
            *(dst++) = 0;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x2e:
            ಸ್ಥಾನಾಂತರ_2++;
            ಸ್ಥಾನಾಂತರ_2 += parse_nameseg(ptr + ಸ್ಥಾನಾಂತರ_2, &dst);
            *(dst++) = '.';
            ಸ್ಥಾನಾಂತರ_2 += parse_nameseg(ptr + ಸ್ಥಾನಾಂತರ_2, &dst);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x2f:
            ಸ್ಥಾನಾಂತರ_2++;
            ಎಣಿಕೆ_2 = ptr[ಸ್ಥಾನಾಂತರ_2];
            ಸ್ಥಾನಾಂತರ_2++;
            ಗಾಗಿ (i = 0; i < ಎಣಿಕೆ_2; i++) {
                ಆದರೆ (i)
                    *(dst++) = '.';
                ಸ್ಥಾನಾಂತರ_2 += parse_nameseg(ptr + ಸ್ಥಾನಾಂತರ_2, &dst);
            }
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ '\\':
            *(dst++) = '\\';
            ಸ್ಥಾನಾಂತರ_2++;
            ಮುಂದುವರಿ;
        ಸ್ಥಿತಿ '^':
            *(dst++) = '^';
            ಸ್ಥಾನಾಂತರ_2++;
            ಮುಂದುವರಿ;
        ಸ್ಥಿತಿ 'A' ... 'Z':
        ಸ್ಥಿತಿ '_':
            ಸ್ಥಾನಾಂತರ_2 += parse_nameseg(ptr, &dst);
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ನಿಲ್ಲಿಸು;
        }
        ನಿಲ್ಲಿಸು;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->ಆಳ,
            ಅಂಶ, s->name);
    ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_termarg_int(u8 *ptr, ಪೂರ್ಣಾಂಕ *error, u64 *dst)
{
    u64 ಮೌಲ್ಯ;
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2 = 1;

    ಆಯ್ಕೆ (ptr[0]) {
    ಸ್ಥಿತಿ 0x00: /* zero */
        ಮೌಲ್ಯ = 0;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: /* one */
        ಮೌಲ್ಯ = 1;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0a: /* byte prefix */
        ಮೌಲ್ಯ = ptr[1];
        ಸ್ಥಾನಾಂತರ_2++;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0b: /* word prefix */
        ಮೌಲ್ಯ = ptr[1] |
            ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ptr[2] << 8);
        ಸ್ಥಾನಾಂತರ_2 += 2;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0c: /* dword prefix */
        ಮೌಲ್ಯ = ptr[1] |
            ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ptr[2] << 8) |
            ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ptr[3] << 16) |
            ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ptr[4] << 24);
        ಸ್ಥಾನಾಂತರ_2 += 4;
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಮೌಲ್ಯ = 0;
        hex(ptr, 16, 3, __func__);
        ಆದರೆ (error)
            *error = 1;
        ನಿಲ್ಲಿಸು;
    }

    ಆದರೆ (dst)
        *dst = ಮೌಲ್ಯ;
    dprintf(5, "%s: 0x%llx\n", __func__, ಮೌಲ್ಯ);
    ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_pkglength(u8 *ptr, ಪೂರ್ಣಾಂಕ *pkglength)
{
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2 = 2;

    *pkglength = 0;
    ಆಯ್ಕೆ (ptr[0] >> 6) {
    ಸ್ಥಿತಿ 3:
        *pkglength |= ptr[3] << 20;
        ಸ್ಥಾನಾಂತರ_2++;
    ಸ್ಥಿತಿ 2:
        *pkglength |= ptr[2] << 12;
        ಸ್ಥಾನಾಂತರ_2++;
    ಸ್ಥಿತಿ 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ_2;
    ಸ್ಥಿತಿ 0:
    ಪೂರ್ವನಿಯೋಜಿತ:
        *pkglength |= ptr[0] & 0x3f;
        ಹಿಂತಿರುಗಿ 1;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_pkg_common(ರಚನೆ parse_state *s,
                            u8 *ptr, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಅಂಶ, ಪೂರ್ಣಾಂಕ *pkglength)
{
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2;

    ಸ್ಥಾನಾಂತರ_2 = parse_pkglength(ptr, pkglength);
    ಸ್ಥಾನಾಂತರ_2 += parse_namestring(s, ptr + ಸ್ಥಾನಾಂತರ_2, ಅಂಶ);
    ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_pkg_scope(ರಚನೆ parse_state *s,
                           u8 *ptr)
{
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2, pkglength;

    ಸ್ಥಾನಾಂತರ_2 = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, ಸ್ಥಾನಾಂತರ_2, pkglength);
    ಹಿಂತಿರುಗಿ pkglength;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_pkg_device(ರಚನೆ parse_state *s,
                            u8 *ptr)
{
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2, pkglength;

    ಸ್ಥಾನಾಂತರ_2 = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(ಗಾತ್ರ(ರಚನೆ acpi_device));
    ಆದರೆ (!s->dev) {
        warn_noalloc();
        s->error = 1;
        ಹಿಂತಿರುಗಿ pkglength;
    }

    memset(s->dev, 0, ಗಾತ್ರ(ರಚನೆ acpi_device));
    hlist_ಕೂಡಿಸು_ತಲೆ(&s->dev->ಗಂಟು, &acpi_devices);
    strtcpy(s->dev->name, s->name, ಗಾತ್ರ(s->name));
    parse_termlist(s, ptr, ಸ್ಥಾನಾಂತರ_2, pkglength);
    s->dev = ಖಾಲಿ;

    ಹಿಂತಿರುಗಿ pkglength;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_pkg_ಬಫರ್(ರಚನೆ parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    ಪೂರ್ಣಾಂಕ pkglength, ಸ್ಥಾನಾಂತರ_2;

    ಸ್ಥಾನಾಂತರ_2 = parse_pkglength(ptr, &pkglength);
    ಸ್ಥಾನಾಂತರ_2 += parse_termarg_int(ptr + ಸ್ಥಾನಾಂತರ_2, &s->error, &blen);
    ಆದರೆ (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_ದತ್ತಾಂಶ = ptr + ಸ್ಥಾನಾಂತರ_2;
        s->dev->crs_ಗಾತ್ರ = blen;
    }
    ಹಿಂತಿರುಗಿ pkglength;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_pkg_skip(ರಚನೆ parse_state *s,
                          u8 *ptr, ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ name)
{
    ಪೂರ್ಣಾಂಕ pkglength, ಸ್ಥಾನಾಂತರ_2;
    ಅಕ್ಷರ ಅಂಶ[8];

    snprintf(ಅಂಶ, ಗಾತ್ರ(ಅಂಶ), "op %x", op);
    ಸ್ಥಾನಾಂತರ_2 = parse_pkglength(ptr, &pkglength);
    ಆದರೆ (name) {
        parse_namestring(s, ptr + ಸ್ಥಾನಾಂತರ_2, ಅಂಶ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        dprintf(5, "%s: %s (%d)\n", __func__, ಅಂಶ, pkglength);
    }
    ಹಿಂತಿರುಗಿ pkglength;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_termobj(ರಚನೆ parse_state *s,
                         u8 *ptr)
{
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2 = 1;

    ಆದರೆ (s->ಆಳ == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ_2;
    }

    s->ಆಳ++;
    ಆಯ್ಕೆ (ptr[0]) {
    ಸ್ಥಿತಿ 0x00: /* zero */
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x01: /* one */
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x06: /* AliasOp */
        ಸ್ಥಾನಾಂತರ_2 += parse_namestring(s, ptr + ಸ್ಥಾನಾಂತರ_2, "SourceObject");
        ಸ್ಥಾನಾಂತರ_2 += parse_namestring(s, ptr + ಸ್ಥಾನಾಂತರ_2, "AliasObject");
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x08: /* name op */
        ಸ್ಥಾನಾಂತರ_2 += parse_namestring(s, ptr + ಸ್ಥಾನಾಂತರ_2, "name");
        ಸ್ಥಾನಾಂತರ_2 += parse_termobj(s, ptr + ಸ್ಥಾನಾಂತರ_2);
        ಆದರೆ (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        ಆದರೆ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0a: /* byte prefix */
        ಸ್ಥಾನಾಂತರ_2++;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0b: /* word prefix */
        ಸ್ಥಾನಾಂತರ_2 += 2;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0c: /* dword prefix */
        ಸ್ಥಾನಾಂತರ_2 += 4;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x0d: /* string prefix */
        ಆಗ (ptr[ಸ್ಥಾನಾಂತರ_2])
            ಸ್ಥಾನಾಂತರ_2++;
        ಸ್ಥಾನಾಂತರ_2++;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x10: /* scope op */
        ಸ್ಥಾನಾಂತರ_2 += parse_pkg_scope(s, ptr + ಸ್ಥಾನಾಂತರ_2);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x11: /* buffer op */
        ಸ್ಥಾನಾಂತರ_2 += parse_pkg_ಬಫರ್(s, ptr + ಸ್ಥಾನಾಂತರ_2);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x12: /* package op */
    ಸ್ಥಿತಿ 0x13: /* var package op */
        ಸ್ಥಾನಾಂತರ_2 += parse_pkg_skip(s, ptr + ಸ್ಥಾನಾಂತರ_2, ptr[0], 0);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x14: /* method op */
        ಸ್ಥಾನಾಂತರ_2 += parse_pkg_skip(s, ptr + ಸ್ಥಾನಾಂತರ_2, ptr[0], 1);
        ಆದರೆ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 0x5b: /* ext op prefix */
        ಸ್ಥಾನಾಂತರ_2++;
        ಆಯ್ಕೆ (ptr[1]) {
        ಸ್ಥಿತಿ 0x01: /* mutex op */
            ಸ್ಥಾನಾಂತರ_2 += parse_namestring(s, ptr + ಸ್ಥಾನಾಂತರ_2, "mutex");
            ಸ್ಥಾನಾಂತರ_2++; /* sync flags */
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x80: /* op region op */
            ಸ್ಥಾನಾಂತರ_2 += parse_namestring(s, ptr + ಸ್ಥಾನಾಂತರ_2, "op region");
            ಸ್ಥಾನಾಂತರ_2++; /* region space */
            ಸ್ಥಾನಾಂತರ_2 += parse_termarg_int(ptr + ಸ್ಥಾನಾಂತರ_2, &s->error, ಖಾಲಿ);
            ಸ್ಥಾನಾಂತರ_2 += parse_termarg_int(ptr + ಸ್ಥಾನಾಂತರ_2, &s->error, ಖಾಲಿ);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x81: /* field op */
        ಸ್ಥಿತಿ 0x83: /* processor op */
        ಸ್ಥಿತಿ 0x84: /* power resource op */
        ಸ್ಥಿತಿ 0x85: /* thermal zone op */
            ಸ್ಥಾನಾಂತರ_2 += parse_pkg_skip(s, ptr + ಸ್ಥಾನಾಂತರ_2, 0x5b00 | ptr[1], 1);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x82: /* device op */
            ಸ್ಥಾನಾಂತರ_2 += parse_pkg_device(s, ptr + ಸ್ಥಾನಾಂತರ_2);
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ನಿಲ್ಲಿಸು;
        }
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        ನಿಲ್ಲಿಸು;
    }
    s->ಆಳ--;

    ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ_2;
}

ಸ್ಥಿರ ಶೂನ್ಯ parse_termlist(ರಚನೆ parse_state *s,
                           u8 *ptr, ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_2, ಪೂರ್ಣಾಂಕ pkglength)
{
    ಗಾಗಿ (;;) {
        ಸ್ಥಾನಾಂತರ_2 += parse_termobj(s, ptr + ಸ್ಥಾನಾಂತರ_2);
        ಆದರೆ (ಸ್ಥಾನಾಂತರ_2 == pkglength)
            ಹಿಂತಿರುಗಿ;
        ಆದರೆ (ಸ್ಥಾನಾಂತರ_2 > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    ಸ್ಥಾನಾಂತರ_2, pkglength);
            s->error = 1;
            ಹಿಂತಿರುಗಿ;
        }
        ಆದರೆ (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    ಸ್ಥಾನಾಂತರ_2, pkglength);
            s->error = 0;
            ಹಿಂತಿರುಗಿ;
        }
    }
}

ಸ್ಥಿರ ರಚನೆ acpi_device *acpi_dsdt_ಕಂಡುಹಿಡಿ(ರಚನೆ acpi_device *prev,
                                          ಸ್ಥಿರಾಂಕ u8 *aml1, ಪೂರ್ಣಾಂಕ size1,
                                          ಸ್ಥಿರಾಂಕ u8 *aml2, ಪೂರ್ಣಾಂಕ size2)
{
    ರಚನೆ acpi_device *dev;
    ರಚನೆ hlist_ಗಂಟು *ಗಂಟು;

    ಆದರೆ (!prev)
        ಗಂಟು = acpi_devices.ಮೊದಲ;
    ಇಲ್ಲದಿದ್ದರೆ
        ಗಂಟು = prev->ಗಂಟು.ಮುಂದಿನ;

    ಗಾಗಿ (; ಗಂಟು != ಖಾಲಿ; ಗಂಟು = dev->ಗಂಟು.ಮುಂದಿನ) {
        dev = container_ದ(ಗಂಟು, ರಚನೆ acpi_device, ಗಂಟು);
        ಆದರೆ (!aml1 && !aml2)
            ಹಿಂತಿರುಗಿ dev;
        ಆದರೆ (!dev->hid_aml)
            ಮುಂದುವರಿ;
        ಆದರೆ (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            ಹಿಂತಿರುಗಿ dev;
        ಆದರೆ (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            ಹಿಂತಿರುಗಿ dev;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ acpi_dsdt_present(ರಚನೆ acpi_device *dev)
{
    ಆದರೆ (!dev)
        ಹಿಂತಿರುಗಿ 0; /* no */
    ಆದರೆ (!dev->sta_aml)
        ಹಿಂತಿರುಗಿ 1; /* yes */
    ಆದರೆ (dev->sta_aml[0] == 0x14)
        ಹಿಂತಿರುಗಿ -1; /* unknown (can't evaluate method) */
    ಆದರೆ (dev->sta_aml[0] == 0x08) {
        u64 ಮೌಲ್ಯ = 0;
        parse_termarg_int(dev->sta_aml + 5, ಖಾಲಿ, &ಮೌಲ್ಯ);
        ಆದರೆ (ಮೌಲ್ಯ == 0)
            ಹಿಂತಿರುಗಿ 0; /* no */
        ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ 1; /* yes */
    }
    ಹಿಂತಿರುಗಿ -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

ರಚನೆ acpi_device *acpi_dsdt_ಕಂಡುಹಿಡಿ_string(ರಚನೆ acpi_device *prev,
                                          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hid)
{
    ಆದರೆ (!CONFIG_ACPI_PARSE)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    u8 aml[10];
    ಪೂರ್ಣಾಂಕ len = snprintf((ಅಕ್ಷರ*)aml, ಗಾತ್ರ(aml), "\x0d%s", hid);
    ಹಿಂತಿರುಗಿ acpi_dsdt_ಕಂಡುಹಿಡಿ(prev, aml, len, ಖಾಲಿ, 0);
}

ರಚನೆ acpi_device *acpi_dsdt_ಕಂಡುಹಿಡಿ_eisaid(ರಚನೆ acpi_device *prev, u16 eisaid)
{
    ಆದರೆ (!CONFIG_ACPI_PARSE)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    ಪೂರ್ಣಾಂಕ len2 = snprintf((ಅಕ್ಷರ*)aml2, ಗಾತ್ರ(aml2), "\x0dPNP%04X", eisaid);
    ಹಿಂತಿರುಗಿ acpi_dsdt_ಕಂಡುಹಿಡಿ(prev, aml1, 5, aml2, len2);
}

ಅಕ್ಷರ *acpi_dsdt_name(ರಚನೆ acpi_device *dev)
{
    ಆದರೆ (!CONFIG_ACPI_PARSE || !dev)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ dev->name;
}

ಪೂರ್ಣಾಂಕ acpi_dsdt_ಕಂಡುಹಿಡಿ_io(ರಚನೆ acpi_device *dev, u64 *min, u64 *max)
{
    ಆದರೆ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ದತ್ತಾಂಶ)
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_resource(dev->crs_ದತ್ತಾಂಶ, dev->crs_ಗಾತ್ರ,
                         1 /* I/O */, min, max);
}

ಪೂರ್ಣಾಂಕ acpi_dsdt_ಕಂಡುಹಿಡಿ_mem(ರಚನೆ acpi_device *dev, u64 *min, u64 *max)
{
    ಆದರೆ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ದತ್ತಾಂಶ)
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_resource(dev->crs_ದತ್ತಾಂಶ, dev->crs_ಗಾತ್ರ,
                         0 /* mem */, min, max);
}

ಪೂರ್ಣಾಂಕ acpi_dsdt_ಕಂಡುಹಿಡಿ_irq(ರಚನೆ acpi_device *dev, u64 *irq)
{
    u64 max;
    ಆದರೆ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ದತ್ತಾಂಶ)
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_resource(dev->crs_ದತ್ತಾಂಶ, dev->crs_ಗಾತ್ರ,
                         3 /* irq */, irq, &max);
}

ಪೂರ್ಣಾಂಕ acpi_dsdt_present_eisaid(u16 eisaid)
{
    ಆದರೆ (!CONFIG_ACPI_PARSE)
        ಹಿಂತಿರುಗಿ -1; /* unknown */
    ಆದರೆ (hlist_ಖಾಲಿಗಳು(&acpi_devices))
        ಹಿಂತಿರುಗಿ -1; /* unknown (no dsdt table) */

    ರಚನೆ acpi_device *dev = acpi_dsdt_ಕಂಡುಹಿಡಿ_eisaid(ಖಾಲಿ, eisaid);
    ಹಿಂತಿರುಗಿ acpi_dsdt_present(dev);
}

ಶೂನ್ಯ acpi_dsdt_parse(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_ACPI_PARSE)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ fadt_descriptor_rev1 *fadt = ಕಂಡುಹಿಡಿ_acpi_ಕೋಷ್ಟಕ(FACP_SIGNATURE);
    ಆದರೆ (!fadt)
        ಹಿಂತಿರುಗಿ;
    u8 *dsdt = (ಶೂನ್ಯ*)(fadt->dsdt);
    ಆದರೆ (!dsdt)
        ಹಿಂತಿರುಗಿ;

    u32 ಉದ್ದಳತೆ = *(u32*)(dsdt + 4);
    u32 ಸ್ಥಾನಾಂತರ_2 = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, ಉದ್ದಳತೆ);

    ರಚನೆ parse_state s;
    memset(&s, 0, ಗಾತ್ರ(s));
    parse_termlist(&s, dsdt, ಸ್ಥಾನಾಂತರ_2, ಉದ್ದಳತೆ);

    ಆದರೆ (!parse_dumpdevs)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    ಗಾಗಿ (dev = acpi_dsdt_ಕಂಡುಹಿಡಿ(ಖಾಲಿ, ಖಾಲಿ, 0, ಖಾಲಿ, 0);
         dev != ಖಾಲಿ;
         dev = acpi_dsdt_ಕಂಡುಹಿಡಿ(dev, ಖಾಲಿ, 0, ಖಾಲಿ, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        ಆದರೆ (dev->hid_aml)
            dprintf(1, ", hid");
        ಆದರೆ (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        ಆದರೆ (dev->crs_ದತ್ತಾಂಶ)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        ಆದರೆ (dev->crs_ದತ್ತಾಂಶ)
            print_resources("        ", dev->crs_ದತ್ತಾಂಶ, dev->crs_ಗಾತ್ರ);
    }
}
