// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "byteorder.h" // le32_to_cpu
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_writeb
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_fseg
#ಅನ್ತರ್ಭಾವ "memmap.h" //ಚಿಹ್ನೆ
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_find
#ಅನ್ತರ್ಭಾವ "std/acpi.h" // struct rsdp_descriptor
#ಅನ್ತರ್ಭಾವ "std/mptable.h" // MPTABLE_SIGNATURE
#ಅನ್ತರ್ಭಾವ "std/pirtable.h" // struct pir_header
#ಅನ್ತರ್ಭಾವ "std/smbios.h" // struct smbios_21_entry_point
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "util.h" // copy_table
#ಅನ್ತರ್ಭಾವ "x86.h" // outb

ರಚನೆ pir_ಶೀರ್ಷಿಕೆ *PirAddr VARFSEG;

ಸ್ಥಿರ ಶೂನ್ಯ *
copy_fseg_ಕೋಷ್ಟಕ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಶೂನ್ಯ *pos, u32 ಗಾತ್ರ_2)
{
    ಶೂನ್ಯ *newpos = malloc_fseg(ಗಾತ್ರ_2);
    ಆದರೆ (!newpos) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    ಸ್ಮರಣೆ_ನಕಲು(newpos, pos, ಗಾತ್ರ_2);
    ಹಿಂತಿರುಗಿ newpos;
}

ಶೂನ್ಯ
copy_pir(ಶೂನ್ಯ *pos)
{
    ರಚನೆ pir_ಶೀರ್ಷಿಕೆ *p = pos;
    ಆದರೆ (p->signature != PIR_SIGNATURE)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (PirAddr)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (p->ಗಾತ್ರ_2 < ಗಾತ್ರ(*p))
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (checksum(pos, p->ಗಾತ್ರ_2) != 0)
        ಹಿಂತಿರುಗಿ;
    PirAddr = copy_fseg_ಕೋಷ್ಟಕ("PIR", pos, p->ಗಾತ್ರ_2);
}

ಶೂನ್ಯ
copy_mptable(ಶೂನ್ಯ *pos)
{
    ರಚನೆ mptable_floating_s *p = pos;
    ಆದರೆ (p->signature != MPTABLE_SIGNATURE)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (!p->physaddr)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (checksum(pos, ಗಾತ್ರ(*p)) != 0)
        ಹಿಂತಿರುಗಿ;
    u32 ಉದ್ದಳತೆ = p->ಉದ್ದಳತೆ * 16;
    u16 mpclength = ((ರಚನೆ mptable_config_s *)p->physaddr)->ಉದ್ದಳತೆ;
    ಆದರೆ (ಉದ್ದಳತೆ + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , ಉದ್ದಳತೆ + mpclength);
        ಹಿಂತಿರುಗಿ;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    ರಚನೆ mptable_floating_s *newpos = malloc_fseg(ಉದ್ದಳತೆ + mpclength);
    ಆದರೆ (!newpos) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    ಸ್ಮರಣೆ_ನಕಲು(newpos, pos, ಉದ್ದಳತೆ);
    newpos->physaddr = (u32)newpos + ಉದ್ದಳತೆ;
    newpos->checksum -= checksum(newpos, ಗಾತ್ರ(*newpos));
    ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ*)newpos + ಉದ್ದಳತೆ, (ಶೂನ್ಯ*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_acpi_rsdp_ಉದ್ದಳತೆ(ಶೂನ್ಯ *pos, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2)
{
    ರಚನೆ rsdp_descriptor *p = pos;
    ಆದರೆ (p->signature != RSDP_SIGNATURE)
        ಹಿಂತಿರುಗಿ -1;
    u32 ಉದ್ದಳತೆ = 20;
    ಆದರೆ (ಉದ್ದಳತೆ > ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (checksum(pos, ಉದ್ದಳತೆ) != 0)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (p->revision > 1) {
        ಉದ್ದಳತೆ = p->ಉದ್ದಳತೆ;
        ಆದರೆ (ಉದ್ದಳತೆ > ಗಾತ್ರ_2)
            ಹಿಂತಿರುಗಿ -1;
        ಆದರೆ (checksum(pos, ಉದ್ದಳತೆ) != 0)
            ಹಿಂತಿರುಗಿ -1;
    }
    ಹಿಂತಿರುಗಿ ಉದ್ದಳತೆ;
}

ರಚನೆ rsdp_descriptor *RsdpAddr;

ಶೂನ್ಯ
copy_acpi_rsdp(ಶೂನ್ಯ *pos)
{
    ಆದರೆ (RsdpAddr)
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ = get_acpi_rsdp_ಉದ್ದಳತೆ(pos, -1);
    ಆದರೆ (ಉದ್ದಳತೆ < 0)
        ಹಿಂತಿರುಗಿ;
    RsdpAddr = copy_fseg_ಕೋಷ್ಟಕ("ACPI RSDP", pos, ಉದ್ದಳತೆ);
}

ಶೂನ್ಯ *ಕಂಡುಹಿಡಿ_acpi_rsdp(ಶೂನ್ಯ)
{
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಆರಂಭ = ಚಿಹ್ನೆ(zonefseg_ಆರಂಭ);
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಕೊನೆ = ಚಿಹ್ನೆ(zonefseg_ಕೊನೆ);
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ pos;

    ಗಾಗಿ (pos = ALIGN(ಆರಂಭ, 0x10); pos <= ALIGN_ಕೆಳಕ್ಕೆ(ಕೊನೆ, 0x10); pos += 0x10)
        ಆದರೆ (get_acpi_rsdp_ಉದ್ದಳತೆ((ಶೂನ್ಯ *)pos, ಕೊನೆ - pos) >= 0)
            ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)pos;

    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಶೂನ್ಯ *
ಕಂಡುಹಿಡಿ_acpi_ಕೋಷ್ಟಕ(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    ಆದರೆ (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ರಚನೆ rsdt_descriptor_rev1 *rsdt = (ಶೂನ್ಯ*)RsdpAddr->rsdt_physical_ವಿಳಾಸ;
    ರಚನೆ xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_ವಿಳಾಸ >= 0x100000000
        ? ಖಾಲಿ : (ಶೂನ್ಯ*)(u32)(RsdpAddr->xsdt_physical_ವಿಳಾಸ);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    ಆದರೆ (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        ಶೂನ್ಯ *ಕೊನೆ = (ಶೂನ್ಯ*)xsdt + xsdt->ಉದ್ದಳತೆ;
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i=0; (ಶೂನ್ಯ*)&xsdt->ಕೋಷ್ಟಕ_ಸ್ಥಾನಾಂತರ_ನಮೂದು[i] < ಕೊನೆ; i++) {
            ಆದರೆ (xsdt->ಕೋಷ್ಟಕ_ಸ್ಥಾನಾಂತರ_ನಮೂದು[i] >= 0x100000000)
                ಮುಂದುವರಿ; /* above 4G */
            ರಚನೆ acpi_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ *tbl = (ಶೂನ್ಯ*)(u32)xsdt->ಕೋಷ್ಟಕ_ಸ್ಥಾನಾಂತರ_ನಮೂದು[i];
            ಆದರೆ (!tbl || tbl->signature != signature)
                ಮುಂದುವರಿ;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            ಹಿಂತಿರುಗಿ tbl;
        }
    }

    ಆದರೆ (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        ಶೂನ್ಯ *ಕೊನೆ = (ಶೂನ್ಯ*)rsdt + rsdt->ಉದ್ದಳತೆ;
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i=0; (ಶೂನ್ಯ*)&rsdt->ಕೋಷ್ಟಕ_ಸ್ಥಾನಾಂತರ_ನಮೂದು[i] < ಕೊನೆ; i++) {
            ರಚನೆ acpi_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ *tbl = (ಶೂನ್ಯ*)rsdt->ಕೋಷ್ಟಕ_ಸ್ಥಾನಾಂತರ_ನಮೂದು[i];
            ಆದರೆ (!tbl || tbl->signature != signature)
                ಮುಂದುವರಿ;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            ಹಿಂತಿರುಗಿ tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

u32
ಕಂಡುಹಿಡಿ_resume_ಸದಿಶ(ಶೂನ್ಯ)
{
    ರಚನೆ fadt_descriptor_rev1 *fadt = ಕಂಡುಹಿಡಿ_acpi_ಕೋಷ್ಟಕ(FACP_SIGNATURE);
    ಆದರೆ (!fadt)
        ಹಿಂತಿರುಗಿ 0;
    ರಚನೆ facs_descriptor_rev1 *facs = (ಶೂನ್ಯ*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    ಆದರೆ (! facs || facs->signature != FACS_SIGNATURE)
        ಹಿಂತಿರುಗಿ 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_ಸದಿಶ);
    ಹಿಂತಿರುಗಿ facs->firmware_waking_ಸದಿಶ;
}

ಸ್ಥಿರ ರಚನೆ acpi_20_generic_ವಿಳಾಸ acpi_reset_reg;
ಸ್ಥಿರ u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_ತಳ = 0xb000;

#ವ್ಯಾಖ್ಯಾನಿಸು acpi_ga_ಗಮ್ಯ_bdf(addr) pci_ಗಮ್ಯ_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

ಶೂನ್ಯ
acpi_reboot(ಶೂನ್ಯ)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    ಆದರೆ (acpi_reset_reg.ರಿಜಿಸ್ಟರ್_ಬಿಟ್_ಅಗಲ != 8)
        ಹಿಂತಿರುಗಿ;

    u64 addr = le64_ಗಮ್ಯ_cpu(acpi_reset_reg.ವಿಳಾಸ);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.ವಿಳಾಸ_space_id, addr, acpi_reset_val);

    ಆಯ್ಕೆ (acpi_reset_reg.ವಿಳಾಸ_space_id) {
    ಸ್ಥಿತಿ 0: // System Memory
        writeb((ಶೂನ್ಯ *)(u32)addr, acpi_reset_val);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1: // System I/O
        outb(acpi_reset_val, addr);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 2: // PCI config space
        pci_config_writeb(acpi_ga_ಗಮ್ಯ_bdf(addr), addr & 0xffff, acpi_reset_val);
        ನಿಲ್ಲಿಸು;
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
acpi_ಗಣ_reset_reg(ರಚನೆ acpi_20_generic_ವಿಳಾಸ *reg, u8 val)
{
    ಆದರೆ (!reg || reg->ವಿಳಾಸ_space_id > 2 ||
        reg->ರಿಜಿಸ್ಟರ್_ಬಿಟ್_ಅಗಲ != 8 || reg->ರಿಜಿಸ್ಟರ್_ಬಿಟ್_ಸ್ಥಾನಾಂತರ)
        ಹಿಂತಿರುಗಿ;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

ಶೂನ್ಯ
ಕಂಡುಹಿಡಿ_acpi_features(ಶೂನ್ಯ)
{
    ರಚನೆ fadt_descriptor_rev1 *fadt = ಕಂಡುಹಿಡಿ_acpi_ಕೋಷ್ಟಕ(FACP_SIGNATURE);
    ಆದರೆ (!fadt)
        ಹಿಂತಿರುಗಿ;
    u32 pm_tmr = le32_ಗಮ್ಯ_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_ಗಮ್ಯ_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    ಆದರೆ (pm_tmr)
        pmtimer_ಸಿದ್ಧತೆ(pm_tmr);
    ಆದರೆ (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    ಆದರೆ (fadt->ಉದ್ದಳತೆ >= 129) {
        ಶೂನ್ಯ *p = fadt;
        acpi_ಗಣ_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
ಶೂನ್ಯ *
smbios_ಮುಂದಿನ(ಶೂನ್ಯ *ಆರಂಭ, u32 ಉದ್ದಳತೆ, ಶೂನ್ಯ *prev)
{
    ಆದರೆ (!ಆರಂಭ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಶೂನ್ಯ *ಕೊನೆ = ಆರಂಭ + ಉದ್ದಳತೆ;

    ಆದರೆ (!prev) {
        prev = ಆರಂಭ;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ *hdr = prev;
        ಆದರೆ (prev + ಗಾತ್ರ(*hdr) > ಕೊನೆ)
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        prev += hdr->ಉದ್ದಳತೆ + 2;
        ಆಗ (prev < ಕೊನೆ && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ *hdr = prev;
    ಆದರೆ (prev >= ಕೊನೆ || prev + ಗಾತ್ರ(*hdr) >= ಕೊನೆ || prev + hdr->ಉದ್ದಳತೆ >= ಕೊನೆ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ prev;
}

ಶೂನ್ಯ *
smbios_21_ಮುಂದಿನ(ರಚನೆ smbios_21_ನಮೂದು_ಬಿಂದು *smbios, ಶೂನ್ಯ *prev)
{
    ಆದರೆ (!smbios)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ smbios_ಮುಂದಿನ((ಶೂನ್ಯ*)smbios->structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ,
                       smbios->structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ, prev);
}

ಸ್ಥಿರ ರಚನೆ smbios_21_ನಮೂದು_ಬಿಂದು *SMBios21Addr;

ಶೂನ್ಯ
copy_smbios_21(ಶೂನ್ಯ *pos)
{
    ಆದರೆ (SMBios21Addr)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ smbios_21_ನಮೂದು_ಬಿಂದು *p = pos;
    ಆದರೆ (p->signature != SMBIOS_21_SIGNATURE)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (checksum(pos, 0x10) != 0)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (checksum(pos+0x10, p->ಉದ್ದಳತೆ-0x10) != 0)
        ಹಿಂತಿರುಗಿ;
    SMBios21Addr = copy_fseg_ಕೋಷ್ಟಕ("SMBIOS", pos, p->ಉದ್ದಳತೆ);
}

ಸ್ಥಿರ ರಚನೆ smbios_30_ನಮೂದು_ಬಿಂದು *SMBios30Addr;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
valid_smbios_30_signature(ರಚನೆ smbios_30_ನಮೂದು_ಬಿಂದು *p)
{
    ಹಿಂತಿರುಗಿ !memcmp(p->signature, "_SM3_", 5);
}

ಶೂನ್ಯ
copy_smbios_30(ಶೂನ್ಯ *pos)
{
    ಆದರೆ (SMBios30Addr)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ smbios_30_ನಮೂದು_ಬಿಂದು *p = pos;
    ಆದರೆ (!valid_smbios_30_signature(p))
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (checksum(pos, p->ಉದ್ದಳತೆ) != 0)
        ಹಿಂತಿರುಗಿ;
    SMBios30Addr = copy_fseg_ಕೋಷ್ಟಕ("SMBIOS 3.0", pos, p->ಉದ್ದಳತೆ);
}

ಶೂನ್ಯ *smbios_get_tables(u32 *ಉದ್ದಳತೆ)
{
    ಆದರೆ (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ;
        ಆದರೆ (addr32 == SMBios30Addr->structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ) {
            *ಉದ್ದಳತೆ = SMBios30Addr->structure_ಕೋಷ್ಟಕ_max_ಗಾತ್ರ;
            ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)addr32;
        }
    }
    ಆದರೆ (SMBios21Addr) {
        *ಉದ್ದಳತೆ = SMBios21Addr->structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ;
        ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)SMBios21Addr->structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
smbios_major_ಆವೃತ್ತಿ(ಶೂನ್ಯ)
{
    ಆದರೆ (SMBios30Addr)
        ಹಿಂತಿರುಗಿ SMBios30Addr->smbios_major_ಆವೃತ್ತಿ;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (SMBios21Addr)
        ಹಿಂತಿರುಗಿ SMBios21Addr->smbios_major_ಆವೃತ್ತಿ;
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
smbios_minor_ಆವೃತ್ತಿ(ಶೂನ್ಯ)
{
    ಆದರೆ (SMBios30Addr)
        ಹಿಂತಿರುಗಿ SMBios30Addr->smbios_minor_ಆವೃತ್ತಿ;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (SMBios21Addr)
        ಹಿಂತಿರುಗಿ SMBios21Addr->smbios_minor_ಆವೃತ್ತಿ;
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ
display_uuid(ಶೂನ್ಯ)
{
    u32 smbios_len = 0;
    ಶೂನ್ಯ *smbios_tables = smbios_get_tables(&smbios_len);
    ರಚನೆ smbios_type_1 *tbl = smbios_ಮುಂದಿನ(smbios_tables, smbios_len, ಖಾಲಿ);
    ಪೂರ್ಣಾಂಕ minlen = offsetof(ರಚನೆ smbios_type_1, uuid) + ಗಾತ್ರ(tbl->uuid);
    ಗಾಗಿ (; tbl; tbl = smbios_ಮುಂದಿನ(smbios_tables, smbios_len, tbl))
        ಆದರೆ (tbl->ಶೀರ್ಷಿಕೆ.type == 1 && tbl->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 ಖಾಲಿಗಳು_uuid[ಗಾತ್ರ(tbl->uuid)] = { 0 };
            ಆದರೆ (memcmp(uuid, ಖಾಲಿಗಳು_uuid, ಗಾತ್ರ(ಖಾಲಿಗಳು_uuid)) == 0)
                ಹಿಂತಿರುಗಿ;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            ಆದರೆ (smbios_major_ಆವೃತ್ತಿ() > 2 ||
                (smbios_major_ಆವೃತ್ತಿ() == 2 &&
                 smbios_minor_ಆವೃತ್ತಿ() >= 6)) {
                ಬರೆ("Machine UUID"
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
            } ಇಲ್ಲದಿದ್ದರೆ {
                ಬರೆ("Machine UUID"
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

            ಹಿಂತಿರುಗಿ;
        }
}

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_str_ಕ್ಷೇತ್ರ_or_skip(type, ಕ್ಷೇತ್ರ, ಮೌಲ್ಯ)                       \
    ಮಾಡು {                                                                \
        ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = (ಮೌಲ್ಯ != ಖಾಲಿ) ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ) + 1 : 0;             \
        ಆದರೆ (ಗಾತ್ರ_2 > 1) {                                                 \
            ಸ್ಮರಣೆ_ನಕಲು(ಕೊನೆ, ಮೌಲ್ಯ, ಗಾತ್ರ_2);                                   \
            ಕೊನೆ += ಗಾತ್ರ_2;                                                \
            p->ಕ್ಷೇತ್ರ = ++str_ಸೂಚ್ಯಂಕ;                                     \
        } ಇಲ್ಲದಿದ್ದರೆ {                                                        \
            p->ಕ್ಷೇತ್ರ = 0;                                               \
        }                                                               \
    } ಆಗ (0)

ಸ್ಥಿರ ಶೂನ್ಯ *
smbios_new_type_0(ಶೂನ್ಯ *ಆರಂಭ,
                  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *vendor, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *date)
{
    ರಚನೆ smbios_type_0 *p = (ರಚನೆ smbios_type_0 *)ಆರಂಭ;
    ಅಕ್ಷರ *ಕೊನೆ = (ಅಕ್ಷರ *)ಆರಂಭ + ಗಾತ್ರ(ರಚನೆ smbios_type_0);
    ಪೂರ್ಣಾಂಕ str_ಸೂಚ್ಯಂಕ = 0;

    p->ಶೀರ್ಷಿಕೆ.type = 0;
    p->ಶೀರ್ಷಿಕೆ.ಉದ್ದಳತೆ = ಗಾತ್ರ(ರಚನೆ smbios_type_0);
    p->ಶೀರ್ಷಿಕೆ.handle = 0;

    ಗಣ_str_ಕ್ಷೇತ್ರ_or_skip(0, vendor_str, vendor);
    ಗಣ_str_ಕ್ಷೇತ್ರ_or_skip(0, bios_ಆವೃತ್ತಿ_str, ಆವೃತ್ತಿ_2);
    p->bios_starting_ವಿಳಾಸ_segment = 0xe800;
    ಗಣ_str_ಕ್ಷೇತ್ರ_or_skip(0, bios_release_date_str, date);

    p->bios_rom_ಗಾತ್ರ = 0; /* FIXME */

    /* BIOS characteristics not supported */
    memset(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_ಬೈಟುಗಳು[0] = 0;
    p->bios_characteristics_extension_ಬೈಟುಗಳು[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *ಕೊನೆ = 0;
    ಕೊನೆ++;
    ಆದರೆ (!str_ಸೂಚ್ಯಂಕ) {
        *ಕೊನೆ = 0;
        ಕೊನೆ++;
    }

    ಹಿಂತಿರುಗಿ ಕೊನೆ;
}

#ವ್ಯಾಖ್ಯಾನಿಸು BIOS_NAME "SeaBIOS"
#ವ್ಯಾಖ್ಯಾನಿಸು BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
smbios_build_tables(ರಚನೆ romfile_s *f_tables,
                    u64 *ವಿಳಾಸ, u32 *ಉದ್ದಳತೆ,
                    u16 *max_structure_ಗಾತ್ರ,
                    u16 *ಸಂಖ್ಯೆ_ದ_structures)
{
    ರಚನೆ smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    ಆದರೆ (f_tables->ಗಾತ್ರ_2 != *ಉದ್ದಳತೆ)
        ಹಿಂತಿರುಗಿ 0;

    qtables = malloc_tmphigh(f_tables->ಗಾತ್ರ_2);
    ಆದರೆ (!qtables) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->ಗಾತ್ರ_2);
    qtables_len = f_tables->ಗಾತ್ರ_2;

    /* did we get a type 0 structure ? */
    ಗಾಗಿ (t0 = smbios_ಮುಂದಿನ(qtables, qtables_len, ಖಾಲಿ); t0;
         t0 = smbios_ಮುಂದಿನ(qtables, qtables_len, t0)) {
        ಆದರೆ (t0->ಶೀರ್ಷಿಕೆ.type == 0) {
            need_t0 = 0;
            ನಿಲ್ಲಿಸು;
        }
    }

    ಆದರೆ (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = ಗಾತ್ರ(ರಚನೆ smbios_type_0) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(BIOS_NAME) +
                     ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಆವೃತ್ತಿ) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(BIOS_DATE) + 4;
        ಆದರೆ (t0_len > (0xffff - *ಉದ್ದಳತೆ)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *ಉದ್ದಳತೆ, t0_len);
            need_t0 = 0;
        } ಇಲ್ಲದಿದ್ದರೆ {
            *ಉದ್ದಳತೆ += t0_len;
            ಆದರೆ (max_structure_ಗಾತ್ರ && t0_len > *max_structure_ಗಾತ್ರ)
                *max_structure_ಗಾತ್ರ = t0_len;
            ಆದರೆ (ಸಂಖ್ಯೆ_ದ_structures)
                (*ಸಂಖ್ಯೆ_ದ_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    ಆದರೆ (*ಉದ್ದಳತೆ > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_ಮೇಲಿನ(*ಉದ್ದಳತೆ);
    ಇಲ್ಲದಿದ್ದರೆ
        tables = malloc_fseg(*ಉದ್ದಳತೆ);
    ಆದರೆ (!tables) {
        warn_noalloc();
        ಬಿಡುಗಡೆ(qtables);
        ಹಿಂತಿರುಗಿ 0;
    }
    *ವಿಳಾಸ = (u32)tables;

    /* populate final blob */
    ಆದರೆ (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, ಆವೃತ್ತಿ, BIOS_DATE);
    ಸ್ಮರಣೆ_ನಕಲು(tables, qtables, qtables_len);
    ಬಿಡುಗಡೆ(qtables);
    ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
smbios_21_ಸಿದ್ಧತೆ_ನಮೂದು_ಬಿಂದು(ರಚನೆ romfile_s *f_tables,
                            ರಚನೆ smbios_21_ನಮೂದು_ಬಿಂದು *ep)
{
    u64 ವಿಳಾಸ = ep->structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ;
    u32 ಉದ್ದಳತೆ = ep->structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ;

    ಆದರೆ (!smbios_build_tables(f_tables,
                             &ವಿಳಾಸ,
                             &ಉದ್ದಳತೆ,
                             &ep->max_structure_ಗಾತ್ರ,
                             &ep->ಸಂಖ್ಯೆ_ದ_structures))
        ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ ((u32)ವಿಳಾಸ != ವಿಳಾಸ || (u16)ಉದ್ದಳತೆ != ಉದ್ದಳತೆ) {
        warn_internalerror();
        ಹಿಂತಿರುಗಿ 0;
    }

    /* finalize entry point */
    ep->structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ = ವಿಳಾಸ;
    ep->structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ = ಉದ್ದಳತೆ;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((ಶೂನ್ಯ *)ep + 0x10, ep->ಉದ್ದಳತೆ - 0x10);

    copy_smbios_21(ep);
    ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
smbios_30_ಸಿದ್ಧತೆ_ನಮೂದು_ಬಿಂದು(ರಚನೆ romfile_s *f_tables,
                            ರಚನೆ smbios_30_ನಮೂದು_ಬಿಂದು *ep)
{
    ಆದರೆ (!smbios_build_tables(f_tables,
                             &ep->structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ,
                             &ep->structure_ಕೋಷ್ಟಕ_max_ಗಾತ್ರ,
                             ಖಾಲಿ, ಖಾಲಿ))
        ಹಿಂತಿರುಗಿ 0;

    ep->checksum -= checksum(ep, ಗಾತ್ರ(*ep));
    copy_smbios_30(ep);
    ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
smbios_romfile_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ರಚನೆ romfile_s *f_anchor = romfile_ಕಂಡುಹಿಡಿ("etc/smbios/smbios-anchor");
    ರಚನೆ romfile_s *f_tables = romfile_ಕಂಡುಹಿಡಿ("etc/smbios/smbios-tables");
    ಒಕ್ಕೂಟ {
        ರಚನೆ smbios_21_ನಮೂದು_ಬಿಂದು ep21;
        ರಚನೆ smbios_30_ನಮೂದು_ಬಿಂದು ep30;
    } ep;

    ಆದರೆ (!f_anchor || !f_tables || f_anchor->ಗಾತ್ರ_2 > ಗಾತ್ರ(ep))
        ಹಿಂತಿರುಗಿ 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->ಗಾತ್ರ_2);

    ಆದರೆ (f_anchor->ಗಾತ್ರ_2 == ಗಾತ್ರ(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        ಹಿಂತಿರುಗಿ smbios_21_ಸಿದ್ಧತೆ_ನಮೂದು_ಬಿಂದು(f_tables, &ep.ep21);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (f_anchor->ಗಾತ್ರ_2 == ಗಾತ್ರ(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        ಹಿಂತಿರುಗಿ smbios_30_ಸಿದ್ಧತೆ_ನಮೂದು_ಬಿಂದು(f_tables, &ep.ep30);
    } ಇಲ್ಲದಿದ್ದರೆ {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        ಹಿಂತಿರುಗಿ 0;
    }
}

ಶೂನ್ಯ
smbios_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (smbios_romfile_ಸಿದ್ಧತೆ())
        ಹಿಂತಿರುಗಿ;
    smbios_legacy_ಸಿದ್ಧತೆ();
}

ಶೂನ್ಯ
copy_ಕೋಷ್ಟಕ(ಶೂನ್ಯ *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
