// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // le32_to_cpu
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_config_writeb
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_fseg
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" //ਚਿੰਨ੍ਹ
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_find
#ਅਨ੍ਤਰ੍ਭਾਵ "std/acpi.h" // struct rsdp_descriptor
#ਅਨ੍ਤਰ੍ਭਾਵ "std/mptable.h" // MPTABLE_SIGNATURE
#ਅਨ੍ਤਰ੍ਭਾਵ "std/pirtable.h" // struct pir_header
#ਅਨ੍ਤਰ੍ਭਾਵ "std/smbios.h" // struct smbios_21_entry_point
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // copy_table
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // outb

ਬਣਤਰ pir_ਸਿਰਲੇਖ *PirAddr VARFSEG;

ਸਥਿਰ ਖਾਲੀ *
copy_fseg_ਸਾਰਣੀ(ਅਚਲ ਅੱਖਰ *name, ਖਾਲੀ *pos, u32 ਮਾਪ)
{
    ਖਾਲੀ *newpos = malloc_fseg(ਮਾਪ);
    ਜੇ (!newpos) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(newpos, pos, ਮਾਪ);
    ਵਾਪਸ newpos;
}

ਖਾਲੀ
copy_pir(ਖਾਲੀ *pos)
{
    ਬਣਤਰ pir_ਸਿਰਲੇਖ *p = pos;
    ਜੇ (p->signature != PIR_SIGNATURE)
        ਵਾਪਸ;
    ਜੇ (PirAddr)
        ਵਾਪਸ;
    ਜੇ (p->ਮਾਪ < ਆਕਾਰ(*p))
        ਵਾਪਸ;
    ਜੇ (checksum(pos, p->ਮਾਪ) != 0)
        ਵਾਪਸ;
    PirAddr = copy_fseg_ਸਾਰਣੀ("PIR", pos, p->ਮਾਪ);
}

ਖਾਲੀ
copy_mptable(ਖਾਲੀ *pos)
{
    ਬਣਤਰ mptable_floating_s *p = pos;
    ਜੇ (p->signature != MPTABLE_SIGNATURE)
        ਵਾਪਸ;
    ਜੇ (!p->physaddr)
        ਵਾਪਸ;
    ਜੇ (checksum(pos, ਆਕਾਰ(*p)) != 0)
        ਵਾਪਸ;
    u32 ਲੰਮਾਈ = p->ਲੰਮਾਈ * 16;
    u16 mpclength = ((ਬਣਤਰ mptable_config_s *)p->physaddr)->ਲੰਮਾਈ;
    ਜੇ (ਲੰਮਾਈ + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , ਲੰਮਾਈ + mpclength);
        ਵਾਪਸ;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    ਬਣਤਰ mptable_floating_s *newpos = malloc_fseg(ਲੰਮਾਈ + mpclength);
    ਜੇ (!newpos) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(newpos, pos, ਲੰਮਾਈ);
    newpos->physaddr = (u32)newpos + ਲੰਮਾਈ;
    newpos->checksum -= checksum(newpos, ਆਕਾਰ(*newpos));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ*)newpos + ਲੰਮਾਈ, (ਖਾਲੀ*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ਸਥਿਰ ਪੂਰਨ
get_acpi_rsdp_ਲੰਮਾਈ(ਖਾਲੀ *pos, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ)
{
    ਬਣਤਰ rsdp_descriptor *p = pos;
    ਜੇ (p->signature != RSDP_SIGNATURE)
        ਵਾਪਸ -1;
    u32 ਲੰਮਾਈ = 20;
    ਜੇ (ਲੰਮਾਈ > ਮਾਪ)
        ਵਾਪਸ -1;
    ਜੇ (checksum(pos, ਲੰਮਾਈ) != 0)
        ਵਾਪਸ -1;
    ਜੇ (p->revision > 1) {
        ਲੰਮਾਈ = p->ਲੰਮਾਈ;
        ਜੇ (ਲੰਮਾਈ > ਮਾਪ)
            ਵਾਪਸ -1;
        ਜੇ (checksum(pos, ਲੰਮਾਈ) != 0)
            ਵਾਪਸ -1;
    }
    ਵਾਪਸ ਲੰਮਾਈ;
}

ਬਣਤਰ rsdp_descriptor *RsdpAddr;

ਖਾਲੀ
copy_acpi_rsdp(ਖਾਲੀ *pos)
{
    ਜੇ (RsdpAddr)
        ਵਾਪਸ;
    ਪੂਰਨ ਲੰਮਾਈ = get_acpi_rsdp_ਲੰਮਾਈ(pos, -1);
    ਜੇ (ਲੰਮਾਈ < 0)
        ਵਾਪਸ;
    RsdpAddr = copy_fseg_ਸਾਰਣੀ("ACPI RSDP", pos, ਲੰਮਾਈ);
}

ਖਾਲੀ *ਲੱਭੋ_acpi_rsdp(ਖਾਲੀ)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸ਼ੁਰੂ = ਚਿੰਨ੍ਹ(zonefseg_ਸ਼ੁਰੂ);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸਿਰਾ = ਚਿੰਨ੍ਹ(zonefseg_ਸਿਰਾ);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ pos;

    ਲਈ (pos = ALIGN(ਸ਼ੁਰੂ, 0x10); pos <= ALIGN_ਥੱਲੇ(ਸਿਰਾ, 0x10); pos += 0x10)
        ਜੇ (get_acpi_rsdp_ਲੰਮਾਈ((ਖਾਲੀ *)pos, ਸਿਰਾ - pos) >= 0)
            ਵਾਪਸ (ਖਾਲੀ *)pos;

    ਵਾਪਸ ਨਿਰਾ;
}

ਖਾਲੀ *
ਲੱਭੋ_acpi_ਸਾਰਣੀ(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    ਜੇ (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        ਵਾਪਸ ਨਿਰਾ;
    ਬਣਤਰ rsdt_descriptor_rev1 *rsdt = (ਖਾਲੀ*)RsdpAddr->rsdt_physical_ਪਤਾ;
    ਬਣਤਰ xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_ਪਤਾ >= 0x100000000
        ? ਨਿਰਾ : (ਖਾਲੀ*)(u32)(RsdpAddr->xsdt_physical_ਪਤਾ);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    ਜੇ (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        ਖਾਲੀ *ਸਿਰਾ = (ਖਾਲੀ*)xsdt + xsdt->ਲੰਮਾਈ;
        ਪੂਰਨ i;
        ਲਈ (i=0; (ਖਾਲੀ*)&xsdt->ਸਾਰਣੀ_ਵਿੱਥ_ਇੰਦਰਾਜ[i] < ਸਿਰਾ; i++) {
            ਜੇ (xsdt->ਸਾਰਣੀ_ਵਿੱਥ_ਇੰਦਰਾਜ[i] >= 0x100000000)
                ਜਾਰੀ; /* above 4G */
            ਬਣਤਰ acpi_ਸਾਰਣੀ_ਸਿਰਲੇਖ *tbl = (ਖਾਲੀ*)(u32)xsdt->ਸਾਰਣੀ_ਵਿੱਥ_ਇੰਦਰਾਜ[i];
            ਜੇ (!tbl || tbl->signature != signature)
                ਜਾਰੀ;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            ਵਾਪਸ tbl;
        }
    }

    ਜੇ (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        ਖਾਲੀ *ਸਿਰਾ = (ਖਾਲੀ*)rsdt + rsdt->ਲੰਮਾਈ;
        ਪੂਰਨ i;
        ਲਈ (i=0; (ਖਾਲੀ*)&rsdt->ਸਾਰਣੀ_ਵਿੱਥ_ਇੰਦਰਾਜ[i] < ਸਿਰਾ; i++) {
            ਬਣਤਰ acpi_ਸਾਰਣੀ_ਸਿਰਲੇਖ *tbl = (ਖਾਲੀ*)rsdt->ਸਾਰਣੀ_ਵਿੱਥ_ਇੰਦਰਾਜ[i];
            ਜੇ (!tbl || tbl->signature != signature)
                ਜਾਰੀ;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            ਵਾਪਸ tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    ਵਾਪਸ ਨਿਰਾ;
}

u32
ਲੱਭੋ_resume_ਸਦਿਸ਼(ਖਾਲੀ)
{
    ਬਣਤਰ fadt_descriptor_rev1 *fadt = ਲੱਭੋ_acpi_ਸਾਰਣੀ(FACP_SIGNATURE);
    ਜੇ (!fadt)
        ਵਾਪਸ 0;
    ਬਣਤਰ facs_descriptor_rev1 *facs = (ਖਾਲੀ*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    ਜੇ (! facs || facs->signature != FACS_SIGNATURE)
        ਵਾਪਸ 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_ਸਦਿਸ਼);
    ਵਾਪਸ facs->firmware_waking_ਸਦਿਸ਼;
}

ਸਥਿਰ ਬਣਤਰ acpi_20_generic_ਪਤਾ acpi_reset_reg;
ਸਥਿਰ u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_ਅਧਾਰ = 0xb000;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ acpi_ga_ਮੰਜ਼ਿਲ_bdf(addr) pci_ਮੰਜ਼ਿਲ_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

ਖਾਲੀ
acpi_reboot(ਖਾਲੀ)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    ਜੇ (acpi_reset_reg.ਰਜਿਸਟਰ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ != 8)
        ਵਾਪਸ;

    u64 addr = le64_ਮੰਜ਼ਿਲ_cpu(acpi_reset_reg.ਪਤਾ);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.ਪਤਾ_space_id, addr, acpi_reset_val);

    ਚੋਣ (acpi_reset_reg.ਪਤਾ_space_id) {
    ਹਾਲਤ 0: // System Memory
        writeb((ਖਾਲੀ *)(u32)addr, acpi_reset_val);
        ਰੋਕੋ;
    ਹਾਲਤ 1: // System I/O
        outb(acpi_reset_val, addr);
        ਰੋਕੋ;
    ਹਾਲਤ 2: // PCI config space
        pci_config_writeb(acpi_ga_ਮੰਜ਼ਿਲ_bdf(addr), addr & 0xffff, acpi_reset_val);
        ਰੋਕੋ;
    }
}

ਸਥਿਰ ਖਾਲੀ
acpi_ਸੈੱਟ_reset_reg(ਬਣਤਰ acpi_20_generic_ਪਤਾ *reg, u8 val)
{
    ਜੇ (!reg || reg->ਪਤਾ_space_id > 2 ||
        reg->ਰਜਿਸਟਰ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ != 8 || reg->ਰਜਿਸਟਰ_ਦੋਆਧਾਰੀ_ਅੰਕ_ਵਿੱਥ)
        ਵਾਪਸ;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

ਖਾਲੀ
ਲੱਭੋ_acpi_features(ਖਾਲੀ)
{
    ਬਣਤਰ fadt_descriptor_rev1 *fadt = ਲੱਭੋ_acpi_ਸਾਰਣੀ(FACP_SIGNATURE);
    ਜੇ (!fadt)
        ਵਾਪਸ;
    u32 pm_tmr = le32_ਮੰਜ਼ਿਲ_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_ਮੰਜ਼ਿਲ_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    ਜੇ (pm_tmr)
        pmtimer_ਤਿਆਰੀ(pm_tmr);
    ਜੇ (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    ਜੇ (fadt->ਲੰਮਾਈ >= 129) {
        ਖਾਲੀ *p = fadt;
        acpi_ਸੈੱਟ_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
ਖਾਲੀ *
smbios_ਅਗਲਾ(ਖਾਲੀ *ਸ਼ੁਰੂ, u32 ਲੰਮਾਈ, ਖਾਲੀ *prev)
{
    ਜੇ (!ਸ਼ੁਰੂ)
        ਵਾਪਸ ਨਿਰਾ;
    ਖਾਲੀ *ਸਿਰਾ = ਸ਼ੁਰੂ + ਲੰਮਾਈ;

    ਜੇ (!prev) {
        prev = ਸ਼ੁਰੂ;
    } ਨਹੀਂਤਾਂ {
        ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ *hdr = prev;
        ਜੇ (prev + ਆਕਾਰ(*hdr) > ਸਿਰਾ)
            ਵਾਪਸ ਨਿਰਾ;
        prev += hdr->ਲੰਮਾਈ + 2;
        ਜਦੋਂ (prev < ਸਿਰਾ && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ *hdr = prev;
    ਜੇ (prev >= ਸਿਰਾ || prev + ਆਕਾਰ(*hdr) >= ਸਿਰਾ || prev + hdr->ਲੰਮਾਈ >= ਸਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    ਵਾਪਸ prev;
}

ਖਾਲੀ *
smbios_21_ਅਗਲਾ(ਬਣਤਰ smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ *smbios, ਖਾਲੀ *prev)
{
    ਜੇ (!smbios)
        ਵਾਪਸ ਨਿਰਾ;
    ਵਾਪਸ smbios_ਅਗਲਾ((ਖਾਲੀ*)smbios->structure_ਸਾਰਣੀ_ਪਤਾ,
                       smbios->structure_ਸਾਰਣੀ_ਲੰਮਾਈ, prev);
}

ਸਥਿਰ ਬਣਤਰ smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ *SMBios21Addr;

ਖਾਲੀ
copy_smbios_21(ਖਾਲੀ *pos)
{
    ਜੇ (SMBios21Addr)
        ਵਾਪਸ;
    ਬਣਤਰ smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ *p = pos;
    ਜੇ (p->signature != SMBIOS_21_SIGNATURE)
        ਵਾਪਸ;
    ਜੇ (checksum(pos, 0x10) != 0)
        ਵਾਪਸ;
    ਜੇ (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        ਵਾਪਸ;
    ਜੇ (checksum(pos+0x10, p->ਲੰਮਾਈ-0x10) != 0)
        ਵਾਪਸ;
    SMBios21Addr = copy_fseg_ਸਾਰਣੀ("SMBIOS", pos, p->ਲੰਮਾਈ);
}

ਸਥਿਰ ਬਣਤਰ smbios_30_ਇੰਦਰਾਜ_ਬਿੰਦੂ *SMBios30Addr;

ਸਥਿਰ ਪੂਰਨ
valid_smbios_30_signature(ਬਣਤਰ smbios_30_ਇੰਦਰਾਜ_ਬਿੰਦੂ *p)
{
    ਵਾਪਸ !memcmp(p->signature, "_SM3_", 5);
}

ਖਾਲੀ
copy_smbios_30(ਖਾਲੀ *pos)
{
    ਜੇ (SMBios30Addr)
        ਵਾਪਸ;
    ਬਣਤਰ smbios_30_ਇੰਦਰਾਜ_ਬਿੰਦੂ *p = pos;
    ਜੇ (!valid_smbios_30_signature(p))
        ਵਾਪਸ;
    ਜੇ (checksum(pos, p->ਲੰਮਾਈ) != 0)
        ਵਾਪਸ;
    SMBios30Addr = copy_fseg_ਸਾਰਣੀ("SMBIOS 3.0", pos, p->ਲੰਮਾਈ);
}

ਖਾਲੀ *smbios_get_tables(u32 *ਲੰਮਾਈ)
{
    ਜੇ (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_ਸਾਰਣੀ_ਪਤਾ;
        ਜੇ (addr32 == SMBios30Addr->structure_ਸਾਰਣੀ_ਪਤਾ) {
            *ਲੰਮਾਈ = SMBios30Addr->structure_ਸਾਰਣੀ_max_ਮਾਪ;
            ਵਾਪਸ (ਖਾਲੀ *)addr32;
        }
    }
    ਜੇ (SMBios21Addr) {
        *ਲੰਮਾਈ = SMBios21Addr->structure_ਸਾਰਣੀ_ਲੰਮਾਈ;
        ਵਾਪਸ (ਖਾਲੀ *)SMBios21Addr->structure_ਸਾਰਣੀ_ਪਤਾ;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਪੂਰਨ
smbios_major_ਸੰਸਕਰਣ(ਖਾਲੀ)
{
    ਜੇ (SMBios30Addr)
        ਵਾਪਸ SMBios30Addr->smbios_major_ਸੰਸਕਰਣ;
    ਨਹੀਂਤਾਂ ਜੇ (SMBios21Addr)
        ਵਾਪਸ SMBios21Addr->smbios_major_ਸੰਸਕਰਣ;
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
smbios_minor_ਸੰਸਕਰਣ(ਖਾਲੀ)
{
    ਜੇ (SMBios30Addr)
        ਵਾਪਸ SMBios30Addr->smbios_minor_ਸੰਸਕਰਣ;
    ਨਹੀਂਤਾਂ ਜੇ (SMBios21Addr)
        ਵਾਪਸ SMBios21Addr->smbios_minor_ਸੰਸਕਰਣ;
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ 0;
}

ਖਾਲੀ
display_uuid(ਖਾਲੀ)
{
    u32 smbios_len = 0;
    ਖਾਲੀ *smbios_tables = smbios_get_tables(&smbios_len);
    ਬਣਤਰ smbios_type_1 *tbl = smbios_ਅਗਲਾ(smbios_tables, smbios_len, ਨਿਰਾ);
    ਪੂਰਨ minlen = offsetof(ਬਣਤਰ smbios_type_1, uuid) + ਆਕਾਰ(tbl->uuid);
    ਲਈ (; tbl; tbl = smbios_ਅਗਲਾ(smbios_tables, smbios_len, tbl))
        ਜੇ (tbl->ਸਿਰਲੇਖ.type == 1 && tbl->ਸਿਰਲੇਖ.ਲੰਮਾਈ >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 ਖਾਲੀ_ਥਾਵਾਂ_uuid[ਆਕਾਰ(tbl->uuid)] = { 0 };
            ਜੇ (memcmp(uuid, ਖਾਲੀ_ਥਾਵਾਂ_uuid, ਆਕਾਰ(ਖਾਲੀ_ਥਾਵਾਂ_uuid)) == 0)
                ਵਾਪਸ;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            ਜੇ (smbios_major_ਸੰਸਕਰਣ() > 2 ||
                (smbios_major_ਸੰਸਕਰਣ() == 2 &&
                 smbios_minor_ਸੰਸਕਰਣ() >= 6)) {
                ਲਿਖੋ("Machine UUID"
                       " %02x%02x%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x%02x%02x%02x%02x\n"
                       , uuid[ 3], uuid[ 2], uuid[ 1], uuid[ 0]
                       , uuid[ 5], uuid[ 4]
                       , uuid[ 7], uuid[ 6]
                       , uuid[ 8], uuid[ 9]
                       , uuid[10], uuid[11], uuid[12]
                       , uuid[13], uuid[14], uuid[15]);
            } ਨਹੀਂਤਾਂ {
                ਲਿਖੋ("Machine UUID"
                       " %02x%02x%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x"
                       "-%02x%02x%02x%02x%02x%02x\n"
                       , uuid[ 0], uuid[ 1], uuid[ 2], uuid[ 3]
                       , uuid[ 4], uuid[ 5]
                       , uuid[ 6], uuid[ 7]
                       , uuid[ 8], uuid[ 9]
                       , uuid[10], uuid[11], uuid[12]
                       , uuid[13], uuid[14], uuid[15]);
            }

            ਵਾਪਸ;
        }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_str_ਖੇਤਰ_or_skip(type, ਖੇਤਰ, ਮੁੱਲ)                       \
    ਕਰੋ {                                                                \
        ਪੂਰਨ ਮਾਪ = (ਮੁੱਲ != ਨਿਰਾ) ? ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ) + 1 : 0;             \
        ਜੇ (ਮਾਪ > 1) {                                                 \
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਸਿਰਾ, ਮੁੱਲ, ਮਾਪ);                                   \
            ਸਿਰਾ += ਮਾਪ;                                                \
            p->ਖੇਤਰ = ++str_ਸੂਚਕਾਂਕ;                                     \
        } ਨਹੀਂਤਾਂ {                                                        \
            p->ਖੇਤਰ = 0;                                               \
        }                                                               \
    } ਜਦੋਂ (0)

ਸਥਿਰ ਖਾਲੀ *
smbios_new_type_0(ਖਾਲੀ *ਸ਼ੁਰੂ,
                  ਅਚਲ ਅੱਖਰ *vendor, ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ_2, ਅਚਲ ਅੱਖਰ *date)
{
    ਬਣਤਰ smbios_type_0 *p = (ਬਣਤਰ smbios_type_0 *)ਸ਼ੁਰੂ;
    ਅੱਖਰ *ਸਿਰਾ = (ਅੱਖਰ *)ਸ਼ੁਰੂ + ਆਕਾਰ(ਬਣਤਰ smbios_type_0);
    ਪੂਰਨ str_ਸੂਚਕਾਂਕ = 0;

    p->ਸਿਰਲੇਖ.type = 0;
    p->ਸਿਰਲੇਖ.ਲੰਮਾਈ = ਆਕਾਰ(ਬਣਤਰ smbios_type_0);
    p->ਸਿਰਲੇਖ.handle = 0;

    ਸੈੱਟ_str_ਖੇਤਰ_or_skip(0, vendor_str, vendor);
    ਸੈੱਟ_str_ਖੇਤਰ_or_skip(0, bios_ਸੰਸਕਰਣ_str, ਸੰਸਕਰਣ_2);
    p->bios_starting_ਪਤਾ_segment = 0xe800;
    ਸੈੱਟ_str_ਖੇਤਰ_or_skip(0, bios_release_date_str, date);

    p->bios_rom_ਮਾਪ = 0; /* FIXME */

    /* BIOS characteristics not supported */
    memset(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_ਬਾਈਟਾਂ[0] = 0;
    p->bios_characteristics_extension_ਬਾਈਟਾਂ[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *ਸਿਰਾ = 0;
    ਸਿਰਾ++;
    ਜੇ (!str_ਸੂਚਕਾਂਕ) {
        *ਸਿਰਾ = 0;
        ਸਿਰਾ++;
    }

    ਵਾਪਸ ਸਿਰਾ;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BIOS_NAME "SeaBIOS"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
ਸਥਿਰ ਪੂਰਨ
smbios_build_tables(ਬਣਤਰ romfile_s *f_tables,
                    u64 *ਪਤਾ, u32 *ਲੰਮਾਈ,
                    u16 *max_structure_ਮਾਪ,
                    u16 *ਸੰਖਿਆ_ਦਾ_structures)
{
    ਬਣਤਰ smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    ਜੇ (f_tables->ਮਾਪ != *ਲੰਮਾਈ)
        ਵਾਪਸ 0;

    qtables = malloc_tmphigh(f_tables->ਮਾਪ);
    ਜੇ (!qtables) {
        warn_noalloc();
        ਵਾਪਸ 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->ਮਾਪ);
    qtables_len = f_tables->ਮਾਪ;

    /* did we get a type 0 structure ? */
    ਲਈ (t0 = smbios_ਅਗਲਾ(qtables, qtables_len, ਨਿਰਾ); t0;
         t0 = smbios_ਅਗਲਾ(qtables, qtables_len, t0)) {
        ਜੇ (t0->ਸਿਰਲੇਖ.type == 0) {
            need_t0 = 0;
            ਰੋਕੋ;
        }
    }

    ਜੇ (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = ਆਕਾਰ(ਬਣਤਰ smbios_type_0) + ਲੜੀ_ਲੰਬਾਈ(BIOS_NAME) +
                     ਲੜੀ_ਲੰਬਾਈ(ਸੰਸਕਰਣ) + ਲੜੀ_ਲੰਬਾਈ(BIOS_DATE) + 4;
        ਜੇ (t0_len > (0xffff - *ਲੰਮਾਈ)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *ਲੰਮਾਈ, t0_len);
            need_t0 = 0;
        } ਨਹੀਂਤਾਂ {
            *ਲੰਮਾਈ += t0_len;
            ਜੇ (max_structure_ਮਾਪ && t0_len > *max_structure_ਮਾਪ)
                *max_structure_ਮਾਪ = t0_len;
            ਜੇ (ਸੰਖਿਆ_ਦਾ_structures)
                (*ਸੰਖਿਆ_ਦਾ_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    ਜੇ (*ਲੰਮਾਈ > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_ਉੱਚਾ(*ਲੰਮਾਈ);
    ਨਹੀਂਤਾਂ
        tables = malloc_fseg(*ਲੰਮਾਈ);
    ਜੇ (!tables) {
        warn_noalloc();
        ਮੁਕਤ(qtables);
        ਵਾਪਸ 0;
    }
    *ਪਤਾ = (u32)tables;

    /* populate final blob */
    ਜੇ (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, ਸੰਸਕਰਣ, BIOS_DATE);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(tables, qtables, qtables_len);
    ਮੁਕਤ(qtables);
    ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ
smbios_21_ਤਿਆਰੀ_ਇੰਦਰਾਜ_ਬਿੰਦੂ(ਬਣਤਰ romfile_s *f_tables,
                            ਬਣਤਰ smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ *ep)
{
    u64 ਪਤਾ = ep->structure_ਸਾਰਣੀ_ਪਤਾ;
    u32 ਲੰਮਾਈ = ep->structure_ਸਾਰਣੀ_ਲੰਮਾਈ;

    ਜੇ (!smbios_build_tables(f_tables,
                             &ਪਤਾ,
                             &ਲੰਮਾਈ,
                             &ep->max_structure_ਮਾਪ,
                             &ep->ਸੰਖਿਆ_ਦਾ_structures))
        ਵਾਪਸ 0;

    ਜੇ ((u32)ਪਤਾ != ਪਤਾ || (u16)ਲੰਮਾਈ != ਲੰਮਾਈ) {
        warn_internalerror();
        ਵਾਪਸ 0;
    }

    /* finalize entry point */
    ep->structure_ਸਾਰਣੀ_ਪਤਾ = ਪਤਾ;
    ep->structure_ਸਾਰਣੀ_ਲੰਮਾਈ = ਲੰਮਾਈ;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((ਖਾਲੀ *)ep + 0x10, ep->ਲੰਮਾਈ - 0x10);

    copy_smbios_21(ep);
    ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ
smbios_30_ਤਿਆਰੀ_ਇੰਦਰਾਜ_ਬਿੰਦੂ(ਬਣਤਰ romfile_s *f_tables,
                            ਬਣਤਰ smbios_30_ਇੰਦਰਾਜ_ਬਿੰਦੂ *ep)
{
    ਜੇ (!smbios_build_tables(f_tables,
                             &ep->structure_ਸਾਰਣੀ_ਪਤਾ,
                             &ep->structure_ਸਾਰਣੀ_max_ਮਾਪ,
                             ਨਿਰਾ, ਨਿਰਾ))
        ਵਾਪਸ 0;

    ep->checksum -= checksum(ep, ਆਕਾਰ(*ep));
    copy_smbios_30(ep);
    ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ
smbios_romfile_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਬਣਤਰ romfile_s *f_anchor = romfile_ਲੱਭੋ("etc/smbios/smbios-anchor");
    ਬਣਤਰ romfile_s *f_tables = romfile_ਲੱਭੋ("etc/smbios/smbios-tables");
    ਸੰਘ {
        ਬਣਤਰ smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ ep21;
        ਬਣਤਰ smbios_30_ਇੰਦਰਾਜ_ਬਿੰਦੂ ep30;
    } ep;

    ਜੇ (!f_anchor || !f_tables || f_anchor->ਮਾਪ > ਆਕਾਰ(ep))
        ਵਾਪਸ 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->ਮਾਪ);

    ਜੇ (f_anchor->ਮਾਪ == ਆਕਾਰ(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        ਵਾਪਸ smbios_21_ਤਿਆਰੀ_ਇੰਦਰਾਜ_ਬਿੰਦੂ(f_tables, &ep.ep21);
    } ਨਹੀਂਤਾਂ ਜੇ (f_anchor->ਮਾਪ == ਆਕਾਰ(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        ਵਾਪਸ smbios_30_ਤਿਆਰੀ_ਇੰਦਰਾਜ_ਬਿੰਦੂ(f_tables, &ep.ep30);
    } ਨਹੀਂਤਾਂ {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        ਵਾਪਸ 0;
    }
}

ਖਾਲੀ
smbios_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (smbios_romfile_ਤਿਆਰੀ())
        ਵਾਪਸ;
    smbios_legacy_ਤਿਆਰੀ();
}

ਖਾਲੀ
copy_ਸਾਰਣੀ(ਖਾਲੀ *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
