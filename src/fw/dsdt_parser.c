// Support for parsing dsdt acpi tables
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h"   // hlist_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_*
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "std/acpi.h" // struct rsdp_descriptor

/****************************************************************
 * DSDT parser
 ****************************************************************/

ਬਣਤਰ acpi_device {
    ਬਣਤਰ hlist_ਗੰਢ ਗੰਢ;
    ਅੱਖਰ name[16];
    u8 *hid_aml;
    u8 *sta_aml;
    u8 *crs_ਅੰਕੜੇ;
    ਪੂਰਨ crs_ਮਾਪ;
};
ਸਥਿਰ ਬਣਤਰ hlist_ਸਿਰ acpi_devices VARVERIFY32INIT;
ਸਥਿਰ ਅਚਲ ਪੂਰਨ parse_dumpdevs = 0;

ਬਣਤਰ parse_state {
    ਅੱਖਰ name[32];
    ਬਣਤਰ acpi_device *dev;
    ਪੂਰਨ error;
    ਪੂਰਨ ਡੂੰਘਾਈ;
};

ਸਥਿਰ ਖਾਲੀ parse_termlist(ਬਣਤਰ parse_state *s,
                           u8 *ptr, ਪੂਰਨ ਵਿੱਥ_2, ਪੂਰਨ pkglength);

ਸਥਿਰ ਖਾਲੀ hex(ਅਚਲ u8 *ptr, ਪੂਰਨ ਗਿਣਤੀ_2, ਪੂਰਨ lvl, ਅਚਲ ਅੱਖਰ *ਵਸਤੂ)
{
    ਪੂਰਨ l = 0, i;

    ਕਰੋ {
        dprintf(lvl, "%s: %04x:  ", ਵਸਤੂ, l);
        ਲਈ (i = l; i < l+16; i += 4)
            dprintf(lvl, "%02x %02x %02x %02x  ",
                    ptr[i+0], ptr[i+1], ptr[i+2], ptr[i+3]);
        ਲਈ (i = l; i < l+16; i++)
            dprintf(lvl, "%c", (ptr[i] > 0x20 && ptr[i] < 0x80)
                    ? ptr[i] : '.');
        dprintf(lvl, "\n");
        l += 16;
    } ਜਦੋਂ (l < ਗਿਣਤੀ_2);
}

ਸਥਿਰ u64 parse_resource_int(u8 *ptr, ਪੂਰਨ ਗਿਣਤੀ_2)
{
    u64 ਮੁੱਲ = 0;
    ਪੂਰਨ ਸੂਚਕਾਂਕ = 0;

    ਲਈ (ਸੂਚਕਾਂਕ = 0; ਸੂਚਕਾਂਕ < ਗਿਣਤੀ_2; ਸੂਚਕਾਂਕ++)
        ਮੁੱਲ |= (u64)ptr[ਸੂਚਕਾਂਕ] << (ਸੂਚਕਾਂਕ * 8);
    ਵਾਪਸ ਮੁੱਲ;
}

ਸਥਿਰ ਪੂਰਨ parse_resource_ਦੋਆਧਾਰੀ_ਅੰਕ(u8 *ptr, ਪੂਰਨ ਗਿਣਤੀ_2)
{
    ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ;

    ਲਈ (ਦੋਆਧਾਰੀ_ਅੰਕ = 0; ਦੋਆਧਾਰੀ_ਅੰਕ < ਗਿਣਤੀ_2*8; ਦੋਆਧਾਰੀ_ਅੰਕ++)
        ਜੇ (ptr[ਦੋਆਧਾਰੀ_ਅੰਕ/8] & (1 << (ਦੋਆਧਾਰੀ_ਅੰਕ%8)))
            ਵਾਪਸ ਦੋਆਧਾਰੀ_ਅੰਕ;
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ parse_resource(u8 *ptr, ਪੂਰਨ ਲੰਮਾਈ, ਪੂਰਨ *type, u64 *min, u64 *max)
{
    ਪੂਰਨ rname, rsize;
    u64 len;

    *type = -1;
    *min = 0;
    *max = 0;
    len = 0;
    ਜੇ (!(ptr[0] & 0x80)) {
        /* small resource */
        rname = (ptr[0] >> 3) & 0x0f;
        rsize = ptr[0] & 0x07;
        rsize++;
        ਚੋਣ (rname) {
        ਹਾਲਤ 0x04: /* irq */
            *min = parse_resource_ਦੋਆਧਾਰੀ_ਅੰਕ(ptr + 1, rsize);
            *max = *min;
            *type = 3;
            ਰੋਕੋ;
        ਹਾਲਤ 0x0f: /* end marker */
            ਵਾਪਸ 0;
        ਹਾਲਤ 0x08: /* io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = parse_resource_int(ptr + 4, 2);
            ਜੇ (*min == *max) {
                *max = *min + ptr[7] - 1;
                *type = 1;
            }
            ਰੋਕੋ;
        ਹਾਲਤ 0x09: /* fixed io */
            *min = parse_resource_int(ptr + 2, 2);
            *max = *min + ptr[4] - 1;
            *type = 1;
            ਰੋਕੋ;
        ਮੂਲ:
            dprintf(3, "%s: small: 0x%x (len %d)\n",
                    __func__, rname, rsize);
            ਰੋਕੋ;
        }
    } ਨਹੀਂਤਾਂ {
        /* large resource */
        rname = ptr[0] & 0x7f;
        rsize = ptr[2] << 8 | ptr[1];
        rsize += 3;
        ਚੋਣ (rname) {
        ਹਾਲਤ 0x06: /* 32-bit Fixed Location Memory Range Descriptor */
            *min = parse_resource_int(ptr + 4, 4);
            len = parse_resource_int(ptr + 8, 4);
            *max = *min + len - 1;
            *type = 0;
            ਰੋਕੋ;
        ਹਾਲਤ 0x07: /* DWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 10, 4);
            *max = parse_resource_int(ptr + 14, 4);
            *type = ptr[3];
            ਰੋਕੋ;
        ਹਾਲਤ 0x08: /* WORD Address Space Descriptor */
            *min = parse_resource_int(ptr +  8, 2);
            *max = parse_resource_int(ptr + 10, 2);
            *type = ptr[3];
            ਰੋਕੋ;
        ਹਾਲਤ 0x09: /* irq */
            *min = parse_resource_int(ptr +  5, 4);
            *max = *min;
            *type = 3;
            ਰੋਕੋ;
        ਹਾਲਤ 0x0a: /* QWORD Address Space Descriptor */
            *min = parse_resource_int(ptr + 14, 8);
            *max = parse_resource_int(ptr + 22, 8);
            *type = ptr[3];
            ਰੋਕੋ;
        ਮੂਲ:
            dprintf(3, "%s: large: 0x%x (len %d)\n", __func__, rname, rsize);
            ਰੋਕੋ;
        }
    }
    ਵਾਪਸ rsize;
}

ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_resource(u8 *ptr, ਪੂਰਨ len, ਪੂਰਨ ਕਿਸਮ_2, u64 *min, u64 *max)
{
    ਪੂਰਨ type, ਮਾਪ, ਵਿੱਥ_2 = 0;

    ਕਰੋ {
        ਮਾਪ = parse_resource(ptr + ਵਿੱਥ_2, len - ਵਿੱਥ_2,
                              &type, min, max);
        ਜੇ (ਕਿਸਮ_2 == type)
            ਵਾਪਸ 0;
        ਵਿੱਥ_2 += ਮਾਪ;
    } ਜਦੋਂ (ਮਾਪ > 0 && ਵਿੱਥ_2 < len);
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ print_resources(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, u8 *ptr, ਪੂਰਨ len)
{
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *typename[] = { "mem", "i/o", "bus" };
    ਪੂਰਨ type, ਮਾਪ, ਵਿੱਥ_2 = 0;
    u64 min, max;

    ਕਰੋ {
        ਮਾਪ = parse_resource(ptr + ਵਿੱਥ_2, len - ਵਿੱਥ_2,
                              &type, &min, &max);
        ਚੋਣ (type) {
        ਹਾਲਤ 0:
        ਹਾਲਤ 1:
        ਹਾਲਤ 2:
            dprintf(1, "%s%s 0x%llx -> 0x%llx\n",
                    ਅਗੇਤਰ, typename[type], min, max);
            ਰੋਕੋ;
        ਹਾਲਤ 3:
            dprintf(1, "%sirq %lld\n", ਅਗੇਤਰ, min);
            ਰੋਕੋ;
        }
        ਵਿੱਥ_2 += ਮਾਪ;
    } ਜਦੋਂ (ਮਾਪ > 0 && ਵਿੱਥ_2 < len);
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ parse_nameseg(u8 *ptr, ਅੱਖਰ **dst)
{
    ਜੇ (dst && *dst) {
        *(dst[0]++) = ptr[0];
        ਜੇ (ptr[1] != '_')
            *(dst[0]++) = ptr[1];
        ਜੇ (ptr[2] != '_')
            *(dst[0]++) = ptr[2];
        ਜੇ (ptr[3] != '_')
            *(dst[0]++) = ptr[3];
        *(dst[0]) = 0;
    }
    ਵਾਪਸ 4;
}

ਸਥਿਰ ਪੂਰਨ parse_namestring(ਬਣਤਰ parse_state *s,
                            u8 *ptr, ਅਚਲ ਅੱਖਰ *ਵਸਤੂ)
{
    ਅੱਖਰ *dst = s->name;
    ਪੂਰਨ ਵਿੱਥ_2 = 0;
    ਪੂਰਨ i, ਗਿਣਤੀ_2;

    ਲਈ (;;) {
        ਚੋਣ (ptr[ਵਿੱਥ_2]) {
        ਹਾਲਤ 0: /* null name */
            ਵਿੱਥ_2++;
            *(dst++) = 0;
            ਰੋਕੋ;
        ਹਾਲਤ 0x2e:
            ਵਿੱਥ_2++;
            ਵਿੱਥ_2 += parse_nameseg(ptr + ਵਿੱਥ_2, &dst);
            *(dst++) = '.';
            ਵਿੱਥ_2 += parse_nameseg(ptr + ਵਿੱਥ_2, &dst);
            ਰੋਕੋ;
        ਹਾਲਤ 0x2f:
            ਵਿੱਥ_2++;
            ਗਿਣਤੀ_2 = ptr[ਵਿੱਥ_2];
            ਵਿੱਥ_2++;
            ਲਈ (i = 0; i < ਗਿਣਤੀ_2; i++) {
                ਜੇ (i)
                    *(dst++) = '.';
                ਵਿੱਥ_2 += parse_nameseg(ptr + ਵਿੱਥ_2, &dst);
            }
            ਰੋਕੋ;
        ਹਾਲਤ '\\':
            *(dst++) = '\\';
            ਵਿੱਥ_2++;
            ਜਾਰੀ;
        ਹਾਲਤ '^':
            *(dst++) = '^';
            ਵਿੱਥ_2++;
            ਜਾਰੀ;
        ਹਾਲਤ 'A' ... 'Z':
        ਹਾਲਤ '_':
            ਵਿੱਥ_2 += parse_nameseg(ptr, &dst);
            ਰੋਕੋ;
        ਮੂਲ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ਰੋਕੋ;
        }
        ਰੋਕੋ;
    }
    dprintf(5, "%s: %d %s '%s'\n", __func__, s->ਡੂੰਘਾਈ,
            ਵਸਤੂ, s->name);
    ਵਾਪਸ ਵਿੱਥ_2;
}

ਸਥਿਰ ਪੂਰਨ parse_termarg_int(u8 *ptr, ਪੂਰਨ *error, u64 *dst)
{
    u64 ਮੁੱਲ;
    ਪੂਰਨ ਵਿੱਥ_2 = 1;

    ਚੋਣ (ptr[0]) {
    ਹਾਲਤ 0x00: /* zero */
        ਮੁੱਲ = 0;
        ਰੋਕੋ;
    ਹਾਲਤ 0x01: /* one */
        ਮੁੱਲ = 1;
        ਰੋਕੋ;
    ਹਾਲਤ 0x0a: /* byte prefix */
        ਮੁੱਲ = ptr[1];
        ਵਿੱਥ_2++;
        ਰੋਕੋ;
    ਹਾਲਤ 0x0b: /* word prefix */
        ਮੁੱਲ = ptr[1] |
            ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ptr[2] << 8);
        ਵਿੱਥ_2 += 2;
        ਰੋਕੋ;
    ਹਾਲਤ 0x0c: /* dword prefix */
        ਮੁੱਲ = ptr[1] |
            ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ptr[2] << 8) |
            ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ptr[3] << 16) |
            ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ptr[4] << 24);
        ਵਿੱਥ_2 += 4;
        ਰੋਕੋ;
    ਮੂਲ:
        ਮੁੱਲ = 0;
        hex(ptr, 16, 3, __func__);
        ਜੇ (error)
            *error = 1;
        ਰੋਕੋ;
    }

    ਜੇ (dst)
        *dst = ਮੁੱਲ;
    dprintf(5, "%s: 0x%llx\n", __func__, ਮੁੱਲ);
    ਵਾਪਸ ਵਿੱਥ_2;
}

ਸਥਿਰ ਪੂਰਨ parse_pkglength(u8 *ptr, ਪੂਰਨ *pkglength)
{
    ਪੂਰਨ ਵਿੱਥ_2 = 2;

    *pkglength = 0;
    ਚੋਣ (ptr[0] >> 6) {
    ਹਾਲਤ 3:
        *pkglength |= ptr[3] << 20;
        ਵਿੱਥ_2++;
    ਹਾਲਤ 2:
        *pkglength |= ptr[2] << 12;
        ਵਿੱਥ_2++;
    ਹਾਲਤ 1:
        *pkglength |= ptr[1] << 4;
        *pkglength |= ptr[0] & 0x0f;
        ਵਾਪਸ ਵਿੱਥ_2;
    ਹਾਲਤ 0:
    ਮੂਲ:
        *pkglength |= ptr[0] & 0x3f;
        ਵਾਪਸ 1;
    }
}

ਸਥਿਰ ਪੂਰਨ parse_pkg_common(ਬਣਤਰ parse_state *s,
                            u8 *ptr, ਅਚਲ ਅੱਖਰ *ਵਸਤੂ, ਪੂਰਨ *pkglength)
{
    ਪੂਰਨ ਵਿੱਥ_2;

    ਵਿੱਥ_2 = parse_pkglength(ptr, pkglength);
    ਵਿੱਥ_2 += parse_namestring(s, ptr + ਵਿੱਥ_2, ਵਸਤੂ);
    ਵਾਪਸ ਵਿੱਥ_2;
}

ਸਥਿਰ ਪੂਰਨ parse_pkg_scope(ਬਣਤਰ parse_state *s,
                           u8 *ptr)
{
    ਪੂਰਨ ਵਿੱਥ_2, pkglength;

    ਵਿੱਥ_2 = parse_pkg_common(s, ptr, "scope", &pkglength);
    parse_termlist(s, ptr, ਵਿੱਥ_2, pkglength);
    ਵਾਪਸ pkglength;
}

ਸਥਿਰ ਪੂਰਨ parse_pkg_device(ਬਣਤਰ parse_state *s,
                            u8 *ptr)
{
    ਪੂਰਨ ਵਿੱਥ_2, pkglength;

    ਵਿੱਥ_2 = parse_pkg_common(s, ptr, "device", &pkglength);

    s->dev = malloc_tmp(ਆਕਾਰ(ਬਣਤਰ acpi_device));
    ਜੇ (!s->dev) {
        warn_noalloc();
        s->error = 1;
        ਵਾਪਸ pkglength;
    }

    memset(s->dev, 0, ਆਕਾਰ(ਬਣਤਰ acpi_device));
    hlist_ਜੋੜੋ_ਸਿਰ(&s->dev->ਗੰਢ, &acpi_devices);
    strtcpy(s->dev->name, s->name, ਆਕਾਰ(s->name));
    parse_termlist(s, ptr, ਵਿੱਥ_2, pkglength);
    s->dev = ਨਿਰਾ;

    ਵਾਪਸ pkglength;
}

ਸਥਿਰ ਪੂਰਨ parse_pkg_ਬਫਰ(ਬਣਤਰ parse_state *s,
                            u8 *ptr)
{
    u64 blen;
    ਪੂਰਨ pkglength, ਵਿੱਥ_2;

    ਵਿੱਥ_2 = parse_pkglength(ptr, &pkglength);
    ਵਿੱਥ_2 += parse_termarg_int(ptr + ਵਿੱਥ_2, &s->error, &blen);
    ਜੇ (s->dev && strcmp(s->name, "_CRS") == 0) {
        s->dev->crs_ਅੰਕੜੇ = ptr + ਵਿੱਥ_2;
        s->dev->crs_ਮਾਪ = blen;
    }
    ਵਾਪਸ pkglength;
}

ਸਥਿਰ ਪੂਰਨ parse_pkg_skip(ਬਣਤਰ parse_state *s,
                          u8 *ptr, ਪੂਰਨ op, ਪੂਰਨ name)
{
    ਪੂਰਨ pkglength, ਵਿੱਥ_2;
    ਅੱਖਰ ਵਸਤੂ[8];

    snprintf(ਵਸਤੂ, ਆਕਾਰ(ਵਸਤੂ), "op %x", op);
    ਵਿੱਥ_2 = parse_pkglength(ptr, &pkglength);
    ਜੇ (name) {
        parse_namestring(s, ptr + ਵਿੱਥ_2, ਵਸਤੂ);
    } ਨਹੀਂਤਾਂ {
        dprintf(5, "%s: %s (%d)\n", __func__, ਵਸਤੂ, pkglength);
    }
    ਵਾਪਸ pkglength;
}

ਸਥਿਰ ਪੂਰਨ parse_termobj(ਬਣਤਰ parse_state *s,
                         u8 *ptr)
{
    ਪੂਰਨ ਵਿੱਥ_2 = 1;

    ਜੇ (s->ਡੂੰਘਾਈ == 16) {
        dprintf(1, "%s: deep recursion\n", __func__);
        s->error = 1;
        ਵਾਪਸ ਵਿੱਥ_2;
    }

    s->ਡੂੰਘਾਈ++;
    ਚੋਣ (ptr[0]) {
    ਹਾਲਤ 0x00: /* zero */
        ਰੋਕੋ;
    ਹਾਲਤ 0x01: /* one */
        ਰੋਕੋ;
    ਹਾਲਤ 0x06: /* AliasOp */
        ਵਿੱਥ_2 += parse_namestring(s, ptr + ਵਿੱਥ_2, "SourceObject");
        ਵਿੱਥ_2 += parse_namestring(s, ptr + ਵਿੱਥ_2, "AliasObject");
        ਰੋਕੋ;
    ਹਾਲਤ 0x08: /* name op */
        ਵਿੱਥ_2 += parse_namestring(s, ptr + ਵਿੱਥ_2, "name");
        ਵਿੱਥ_2 += parse_termobj(s, ptr + ਵਿੱਥ_2);
        ਜੇ (s->dev && strcmp(s->name, "_HID") == 0)
            s->dev->hid_aml = ptr;
        ਜੇ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ਰੋਕੋ;
    ਹਾਲਤ 0x0a: /* byte prefix */
        ਵਿੱਥ_2++;
        ਰੋਕੋ;
    ਹਾਲਤ 0x0b: /* word prefix */
        ਵਿੱਥ_2 += 2;
        ਰੋਕੋ;
    ਹਾਲਤ 0x0c: /* dword prefix */
        ਵਿੱਥ_2 += 4;
        ਰੋਕੋ;
    ਹਾਲਤ 0x0d: /* string prefix */
        ਜਦੋਂ (ptr[ਵਿੱਥ_2])
            ਵਿੱਥ_2++;
        ਵਿੱਥ_2++;
        ਰੋਕੋ;
    ਹਾਲਤ 0x10: /* scope op */
        ਵਿੱਥ_2 += parse_pkg_scope(s, ptr + ਵਿੱਥ_2);
        ਰੋਕੋ;
    ਹਾਲਤ 0x11: /* buffer op */
        ਵਿੱਥ_2 += parse_pkg_ਬਫਰ(s, ptr + ਵਿੱਥ_2);
        ਰੋਕੋ;
    ਹਾਲਤ 0x12: /* package op */
    ਹਾਲਤ 0x13: /* var package op */
        ਵਿੱਥ_2 += parse_pkg_skip(s, ptr + ਵਿੱਥ_2, ptr[0], 0);
        ਰੋਕੋ;
    ਹਾਲਤ 0x14: /* method op */
        ਵਿੱਥ_2 += parse_pkg_skip(s, ptr + ਵਿੱਥ_2, ptr[0], 1);
        ਜੇ (s->dev && strcmp(s->name, "_STA") == 0)
            s->dev->sta_aml = ptr;
        ਰੋਕੋ;
    ਹਾਲਤ 0x5b: /* ext op prefix */
        ਵਿੱਥ_2++;
        ਚੋਣ (ptr[1]) {
        ਹਾਲਤ 0x01: /* mutex op */
            ਵਿੱਥ_2 += parse_namestring(s, ptr + ਵਿੱਥ_2, "mutex");
            ਵਿੱਥ_2++; /* sync flags */
            ਰੋਕੋ;
        ਹਾਲਤ 0x80: /* op region op */
            ਵਿੱਥ_2 += parse_namestring(s, ptr + ਵਿੱਥ_2, "op region");
            ਵਿੱਥ_2++; /* region space */
            ਵਿੱਥ_2 += parse_termarg_int(ptr + ਵਿੱਥ_2, &s->error, ਨਿਰਾ);
            ਵਿੱਥ_2 += parse_termarg_int(ptr + ਵਿੱਥ_2, &s->error, ਨਿਰਾ);
            ਰੋਕੋ;
        ਹਾਲਤ 0x81: /* field op */
        ਹਾਲਤ 0x83: /* processor op */
        ਹਾਲਤ 0x84: /* power resource op */
        ਹਾਲਤ 0x85: /* thermal zone op */
            ਵਿੱਥ_2 += parse_pkg_skip(s, ptr + ਵਿੱਥ_2, 0x5b00 | ptr[1], 1);
            ਰੋਕੋ;
        ਹਾਲਤ 0x82: /* device op */
            ਵਿੱਥ_2 += parse_pkg_device(s, ptr + ਵਿੱਥ_2);
            ਰੋਕੋ;
        ਮੂਲ:
            hex(ptr, 16, 3, __func__);
            s->error = 1;
            ਰੋਕੋ;
        }
        ਰੋਕੋ;
    ਮੂਲ:
        hex(ptr, 16, 3, __func__);
        s->error = 1;
        ਰੋਕੋ;
    }
    s->ਡੂੰਘਾਈ--;

    ਵਾਪਸ ਵਿੱਥ_2;
}

ਸਥਿਰ ਖਾਲੀ parse_termlist(ਬਣਤਰ parse_state *s,
                           u8 *ptr, ਪੂਰਨ ਵਿੱਥ_2, ਪੂਰਨ pkglength)
{
    ਲਈ (;;) {
        ਵਿੱਥ_2 += parse_termobj(s, ptr + ਵਿੱਥ_2);
        ਜੇ (ਵਿੱਥ_2 == pkglength)
            ਵਾਪਸ;
        ਜੇ (ਵਿੱਥ_2 > pkglength) {
            dprintf(1, "%s: overrun: %d/%d\n", __func__,
                    ਵਿੱਥ_2, pkglength);
            s->error = 1;
            ਵਾਪਸ;
        }
        ਜੇ (s->error) {
            dprintf(1, "%s: parse error, skip from %d/%d\n", __func__,
                    ਵਿੱਥ_2, pkglength);
            s->error = 0;
            ਵਾਪਸ;
        }
    }
}

ਸਥਿਰ ਬਣਤਰ acpi_device *acpi_dsdt_ਲੱਭੋ(ਬਣਤਰ acpi_device *prev,
                                          ਅਚਲ u8 *aml1, ਪੂਰਨ size1,
                                          ਅਚਲ u8 *aml2, ਪੂਰਨ size2)
{
    ਬਣਤਰ acpi_device *dev;
    ਬਣਤਰ hlist_ਗੰਢ *ਗੰਢ;

    ਜੇ (!prev)
        ਗੰਢ = acpi_devices.ਪਹਿਲਾ;
    ਨਹੀਂਤਾਂ
        ਗੰਢ = prev->ਗੰਢ.ਅਗਲਾ;

    ਲਈ (; ਗੰਢ != ਨਿਰਾ; ਗੰਢ = dev->ਗੰਢ.ਅਗਲਾ) {
        dev = container_ਦਾ(ਗੰਢ, ਬਣਤਰ acpi_device, ਗੰਢ);
        ਜੇ (!aml1 && !aml2)
            ਵਾਪਸ dev;
        ਜੇ (!dev->hid_aml)
            ਜਾਰੀ;
        ਜੇ (aml1 && memcmp(dev->hid_aml + 5, aml1, size1) == 0)
            ਵਾਪਸ dev;
        ਜੇ (aml2 && memcmp(dev->hid_aml + 5, aml2, size2) == 0)
            ਵਾਪਸ dev;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਪੂਰਨ acpi_dsdt_present(ਬਣਤਰ acpi_device *dev)
{
    ਜੇ (!dev)
        ਵਾਪਸ 0; /* no */
    ਜੇ (!dev->sta_aml)
        ਵਾਪਸ 1; /* yes */
    ਜੇ (dev->sta_aml[0] == 0x14)
        ਵਾਪਸ -1; /* unknown (can't evaluate method) */
    ਜੇ (dev->sta_aml[0] == 0x08) {
        u64 ਮੁੱਲ = 0;
        parse_termarg_int(dev->sta_aml + 5, ਨਿਰਾ, &ਮੁੱਲ);
        ਜੇ (ਮੁੱਲ == 0)
            ਵਾਪਸ 0; /* no */
        ਨਹੀਂਤਾਂ
            ਵਾਪਸ 1; /* yes */
    }
    ਵਾਪਸ -1; /* unknown (should not happen) */
}

/****************************************************************
 * DSDT parser, public interface
 ****************************************************************/

ਬਣਤਰ acpi_device *acpi_dsdt_ਲੱਭੋ_string(ਬਣਤਰ acpi_device *prev,
                                          ਅਚਲ ਅੱਖਰ *hid)
{
    ਜੇ (!CONFIG_ACPI_PARSE)
        ਵਾਪਸ ਨਿਰਾ;

    u8 aml[10];
    ਪੂਰਨ len = snprintf((ਅੱਖਰ*)aml, ਆਕਾਰ(aml), "\x0d%s", hid);
    ਵਾਪਸ acpi_dsdt_ਲੱਭੋ(prev, aml, len, ਨਿਰਾ, 0);
}

ਬਣਤਰ acpi_device *acpi_dsdt_ਲੱਭੋ_eisaid(ਬਣਤਰ acpi_device *prev, u16 eisaid)
{
    ਜੇ (!CONFIG_ACPI_PARSE)
        ਵਾਪਸ ਨਿਰਾ;
    u8 aml1[] = {
        0x0c, 0x41, 0xd0,
        eisaid >> 8,
        eisaid & 0xff
    };
    u8 aml2[10];
    ਪੂਰਨ len2 = snprintf((ਅੱਖਰ*)aml2, ਆਕਾਰ(aml2), "\x0dPNP%04X", eisaid);
    ਵਾਪਸ acpi_dsdt_ਲੱਭੋ(prev, aml1, 5, aml2, len2);
}

ਅੱਖਰ *acpi_dsdt_name(ਬਣਤਰ acpi_device *dev)
{
    ਜੇ (!CONFIG_ACPI_PARSE || !dev)
        ਵਾਪਸ ਨਿਰਾ;
    ਵਾਪਸ dev->name;
}

ਪੂਰਨ acpi_dsdt_ਲੱਭੋ_io(ਬਣਤਰ acpi_device *dev, u64 *min, u64 *max)
{
    ਜੇ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ਅੰਕੜੇ)
        ਵਾਪਸ -1;
    ਵਾਪਸ ਲੱਭੋ_resource(dev->crs_ਅੰਕੜੇ, dev->crs_ਮਾਪ,
                         1 /* I/O */, min, max);
}

ਪੂਰਨ acpi_dsdt_ਲੱਭੋ_mem(ਬਣਤਰ acpi_device *dev, u64 *min, u64 *max)
{
    ਜੇ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ਅੰਕੜੇ)
        ਵਾਪਸ -1;
    ਵਾਪਸ ਲੱਭੋ_resource(dev->crs_ਅੰਕੜੇ, dev->crs_ਮਾਪ,
                         0 /* mem */, min, max);
}

ਪੂਰਨ acpi_dsdt_ਲੱਭੋ_irq(ਬਣਤਰ acpi_device *dev, u64 *irq)
{
    u64 max;
    ਜੇ (!CONFIG_ACPI_PARSE || !dev || !dev->crs_ਅੰਕੜੇ)
        ਵਾਪਸ -1;
    ਵਾਪਸ ਲੱਭੋ_resource(dev->crs_ਅੰਕੜੇ, dev->crs_ਮਾਪ,
                         3 /* irq */, irq, &max);
}

ਪੂਰਨ acpi_dsdt_present_eisaid(u16 eisaid)
{
    ਜੇ (!CONFIG_ACPI_PARSE)
        ਵਾਪਸ -1; /* unknown */
    ਜੇ (hlist_ਖਾਲੀ_ਥਾਵਾਂ(&acpi_devices))
        ਵਾਪਸ -1; /* unknown (no dsdt table) */

    ਬਣਤਰ acpi_device *dev = acpi_dsdt_ਲੱਭੋ_eisaid(ਨਿਰਾ, eisaid);
    ਵਾਪਸ acpi_dsdt_present(dev);
}

ਖਾਲੀ acpi_dsdt_parse(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_ACPI_PARSE)
        ਵਾਪਸ;

    ਬਣਤਰ fadt_descriptor_rev1 *fadt = ਲੱਭੋ_acpi_ਸਾਰਣੀ(FACP_SIGNATURE);
    ਜੇ (!fadt)
        ਵਾਪਸ;
    u8 *dsdt = (ਖਾਲੀ*)(fadt->dsdt);
    ਜੇ (!dsdt)
        ਵਾਪਸ;

    u32 ਲੰਮਾਈ = *(u32*)(dsdt + 4);
    u32 ਵਿੱਥ_2 = 0x24;
    dprintf(1, "ACPI: parse DSDT at %p (len %d)\n", dsdt, ਲੰਮਾਈ);

    ਬਣਤਰ parse_state s;
    memset(&s, 0, ਆਕਾਰ(s));
    parse_termlist(&s, dsdt, ਵਿੱਥ_2, ਲੰਮਾਈ);

    ਜੇ (!parse_dumpdevs)
        ਵਾਪਸ;

    ਬਣਤਰ acpi_device *dev;
    dprintf(1, "ACPI: dumping dsdt devices\n");
    ਲਈ (dev = acpi_dsdt_ਲੱਭੋ(ਨਿਰਾ, ਨਿਰਾ, 0, ਨਿਰਾ, 0);
         dev != ਨਿਰਾ;
         dev = acpi_dsdt_ਲੱਭੋ(dev, ਨਿਰਾ, 0, ਨਿਰਾ, 0)) {
        dprintf(1, "    %s", acpi_dsdt_name(dev));
        ਜੇ (dev->hid_aml)
            dprintf(1, ", hid");
        ਜੇ (dev->sta_aml)
            dprintf(1, ", sta (0x%x)", dev->sta_aml[0]);
        ਜੇ (dev->crs_ਅੰਕੜੇ)
            dprintf(1, ", crs");
        dprintf(1, "\n");
        ਜੇ (dev->crs_ਅੰਕੜੇ)
            print_resources("        ", dev->crs_ਅੰਕੜੇ, dev->crs_ਮਾਪ);
    }
}
