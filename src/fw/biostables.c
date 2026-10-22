// Support for manipulating bios tables (pir, mptable, acpi, smbios).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "byteorder.h" // le32_to_cpu
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "hw/pci.h" // pci_config_writeb
#అన్తర్భావ "malloc.h" // malloc_fseg
#అన్తర్భావ "memmap.h" //చిహ్నం
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_find
#అన్తర్భావ "std/acpi.h" // struct rsdp_descriptor
#అన్తర్భావ "std/mptable.h" // MPTABLE_SIGNATURE
#అన్తర్భావ "std/pirtable.h" // struct pir_header
#అన్తర్భావ "std/smbios.h" // struct smbios_21_entry_point
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "util.h" // copy_table
#అన్తర్భావ "x86.h" // outb

నిర్మాణం pir_శీర్షిక *PirAddr VARFSEG;

స్థిర శూన్య *
copy_fseg_పట్టిక(స్థిరరాశి అక్షరం *name, శూన్య *pos, u32 కొలత)
{
    శూన్య *newpos = malloc_fseg(కొలత);
    ఒకవేళ (!newpos) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    dprintf(1, "Copying %s from %p to %p\n", name, pos, newpos);
    జ్ఞాపకం_నకలు(newpos, pos, కొలత);
    తిరిగి newpos;
}

శూన్య
copy_pir(శూన్య *pos)
{
    నిర్మాణం pir_శీర్షిక *p = pos;
    ఒకవేళ (p->signature != PIR_SIGNATURE)
        తిరిగి;
    ఒకవేళ (PirAddr)
        తిరిగి;
    ఒకవేళ (p->కొలత < పరిమాణం(*p))
        తిరిగి;
    ఒకవేళ (checksum(pos, p->కొలత) != 0)
        తిరిగి;
    PirAddr = copy_fseg_పట్టిక("PIR", pos, p->కొలత);
}

శూన్య
copy_mptable(శూన్య *pos)
{
    నిర్మాణం mptable_floating_s *p = pos;
    ఒకవేళ (p->signature != MPTABLE_SIGNATURE)
        తిరిగి;
    ఒకవేళ (!p->physaddr)
        తిరిగి;
    ఒకవేళ (checksum(pos, పరిమాణం(*p)) != 0)
        తిరిగి;
    u32 నిడివి = p->నిడివి * 16;
    u16 mpclength = ((నిర్మాణం mptable_config_s *)p->physaddr)->నిడివి;
    ఒకవేళ (నిడివి + mpclength > BUILD_MAX_MPTABLE_FSEG) {
        dprintf(1, "Skipping MPTABLE copy due to large size (%d bytes)\n"
                , నిడివి + mpclength);
        తిరిగి;
    }
    // Allocate final memory location.  (In theory the config
    // structure can go in high memory, but Linux kernels before
    // v2.6.30 crash with that.)
    నిర్మాణం mptable_floating_s *newpos = malloc_fseg(నిడివి + mpclength);
    ఒకవేళ (!newpos) {
        warn_noalloc();
        తిరిగి;
    }
    dprintf(1, "Copying MPTABLE from %p/%x to %p\n", pos, p->physaddr, newpos);
    జ్ఞాపకం_నకలు(newpos, pos, నిడివి);
    newpos->physaddr = (u32)newpos + నిడివి;
    newpos->checksum -= checksum(newpos, పరిమాణం(*newpos));
    జ్ఞాపకం_నకలు((శూన్య*)newpos + నిడివి, (శూన్య*)p->physaddr, mpclength);
}


/****************************************************************
 * ACPI
 ****************************************************************/

స్థిర పూర్ణాంకం
get_acpi_rsdp_నిడివి(శూన్య *pos, గుర్తులేని కొలత)
{
    నిర్మాణం rsdp_descriptor *p = pos;
    ఒకవేళ (p->signature != RSDP_SIGNATURE)
        తిరిగి -1;
    u32 నిడివి = 20;
    ఒకవేళ (నిడివి > కొలత)
        తిరిగి -1;
    ఒకవేళ (checksum(pos, నిడివి) != 0)
        తిరిగి -1;
    ఒకవేళ (p->revision > 1) {
        నిడివి = p->నిడివి;
        ఒకవేళ (నిడివి > కొలత)
            తిరిగి -1;
        ఒకవేళ (checksum(pos, నిడివి) != 0)
            తిరిగి -1;
    }
    తిరిగి నిడివి;
}

నిర్మాణం rsdp_descriptor *RsdpAddr;

శూన్య
copy_acpi_rsdp(శూన్య *pos)
{
    ఒకవేళ (RsdpAddr)
        తిరిగి;
    పూర్ణాంకం నిడివి = get_acpi_rsdp_నిడివి(pos, -1);
    ఒకవేళ (నిడివి < 0)
        తిరిగి;
    RsdpAddr = copy_fseg_పట్టిక("ACPI RSDP", pos, నిడివి);
}

శూన్య *కనుగొను_acpi_rsdp(శూన్య)
{
    గుర్తులేని దీర్ఘ ప్రారంభం = చిహ్నం(zonefseg_ప్రారంభం);
    గుర్తులేని దీర్ఘ చివర = చిహ్నం(zonefseg_చివర);
    గుర్తులేని దీర్ఘ pos;

    కొరకు (pos = ALIGN(ప్రారంభం, 0x10); pos <= ALIGN_కిందికి(చివర, 0x10); pos += 0x10)
        ఒకవేళ (get_acpi_rsdp_నిడివి((శూన్య *)pos, చివర - pos) >= 0)
            తిరిగి (శూన్య *)pos;

    తిరిగి ఖాళీ;
}

శూన్య *
కనుగొను_acpi_పట్టిక(u32 signature)
{
    dprintf(4, "rsdp=%p\n", RsdpAddr);
    ఒకవేళ (!RsdpAddr || RsdpAddr->signature != RSDP_SIGNATURE)
        తిరిగి ఖాళీ;
    నిర్మాణం rsdt_descriptor_rev1 *rsdt = (శూన్య*)RsdpAddr->rsdt_physical_చిరునామా;
    నిర్మాణం xsdt_descriptor_rev2 *xsdt =
        RsdpAddr->xsdt_physical_చిరునామా >= 0x100000000
        ? ఖాళీ : (శూన్య*)(u32)(RsdpAddr->xsdt_physical_చిరునామా);
    dprintf(4, "rsdt=%p\n", rsdt);
    dprintf(4, "xsdt=%p\n", xsdt);

    ఒకవేళ (xsdt && xsdt->signature == XSDT_SIGNATURE) {
        శూన్య *చివర = (శూన్య*)xsdt + xsdt->నిడివి;
        పూర్ణాంకం i;
        కొరకు (i=0; (శూన్య*)&xsdt->పట్టిక_స్థానభ్రంశం_నమోదు[i] < చివర; i++) {
            ఒకవేళ (xsdt->పట్టిక_స్థానభ్రంశం_నమోదు[i] >= 0x100000000)
                కొనసాగు; /* above 4G */
            నిర్మాణం acpi_పట్టిక_శీర్షిక *tbl = (శూన్య*)(u32)xsdt->పట్టిక_స్థానభ్రంశం_నమోదు[i];
            ఒకవేళ (!tbl || tbl->signature != signature)
                కొనసాగు;
            dprintf(1, "table(%x)=%p (via xsdt)\n", signature, tbl);
            తిరిగి tbl;
        }
    }

    ఒకవేళ (rsdt && rsdt->signature == RSDT_SIGNATURE) {
        శూన్య *చివర = (శూన్య*)rsdt + rsdt->నిడివి;
        పూర్ణాంకం i;
        కొరకు (i=0; (శూన్య*)&rsdt->పట్టిక_స్థానభ్రంశం_నమోదు[i] < చివర; i++) {
            నిర్మాణం acpi_పట్టిక_శీర్షిక *tbl = (శూన్య*)rsdt->పట్టిక_స్థానభ్రంశం_నమోదు[i];
            ఒకవేళ (!tbl || tbl->signature != signature)
                కొనసాగు;
            dprintf(1, "table(%x)=%p (via rsdt)\n", signature, tbl);
            తిరిగి tbl;
        }
    }

    dprintf(4, "no table %x found\n", signature);
    తిరిగి ఖాళీ;
}

u32
కనుగొను_resume_సదిశ(శూన్య)
{
    నిర్మాణం fadt_descriptor_rev1 *fadt = కనుగొను_acpi_పట్టిక(FACP_SIGNATURE);
    ఒకవేళ (!fadt)
        తిరిగి 0;
    నిర్మాణం facs_descriptor_rev1 *facs = (శూన్య*)fadt->firmware_ctrl;
    dprintf(4, "facs=%p\n", facs);
    ఒకవేళ (! facs || facs->signature != FACS_SIGNATURE)
        తిరిగి 0;
    // Found it.
    dprintf(4, "resume addr=%d\n", facs->firmware_waking_సదిశ);
    తిరిగి facs->firmware_waking_సదిశ;
}

స్థిర నిర్మాణం acpi_20_generic_చిరునామా acpi_reset_reg;
స్థిర u8 acpi_reset_val;
u32 acpi_pm1a_cnt VARFSEG;
u16 acpi_pm_పునాది = 0xb000;

#నిర్వచించు acpi_ga_గమ్యం_bdf(addr) pci_గమ్యం_bdf(0, (addr >> 32) & 0xffff, (addr >> 16) & 0xffff)

శూన్య
acpi_reboot(శూన్య)
{
    // Check it passed the sanity checks in acpi_set_reset_reg() and was set
    ఒకవేళ (acpi_reset_reg.రిజిస్టరు_బిట్టు_వెడల్పు != 8)
        తిరిగి;

    u64 addr = le64_గమ్యం_cpu(acpi_reset_reg.చిరునామా);

    dprintf(1, "ACPI hard reset %d:%llx (%x)\n",
            acpi_reset_reg.చిరునామా_space_id, addr, acpi_reset_val);

    ఎంపిక (acpi_reset_reg.చిరునామా_space_id) {
    స్థితి 0: // System Memory
        writeb((శూన్య *)(u32)addr, acpi_reset_val);
        ఆపు;
    స్థితి 1: // System I/O
        outb(acpi_reset_val, addr);
        ఆపు;
    స్థితి 2: // PCI config space
        pci_config_writeb(acpi_ga_గమ్యం_bdf(addr), addr & 0xffff, acpi_reset_val);
        ఆపు;
    }
}

స్థిర శూన్య
acpi_సమితి_reset_reg(నిర్మాణం acpi_20_generic_చిరునామా *reg, u8 val)
{
    ఒకవేళ (!reg || reg->చిరునామా_space_id > 2 ||
        reg->రిజిస్టరు_బిట్టు_వెడల్పు != 8 || reg->రిజిస్టరు_బిట్టు_స్థానభ్రంశం)
        తిరిగి;

    acpi_reset_reg = *reg;
    acpi_reset_val = val;
}

శూన్య
కనుగొను_acpi_features(శూన్య)
{
    నిర్మాణం fadt_descriptor_rev1 *fadt = కనుగొను_acpi_పట్టిక(FACP_SIGNATURE);
    ఒకవేళ (!fadt)
        తిరిగి;
    u32 pm_tmr = le32_గమ్యం_cpu(fadt->pm_tmr_blk);
    u32 pm1a_cnt = le32_గమ్యం_cpu(fadt->pm1a_cnt_blk);
    dprintf(4, "pm_tmr_blk=%x\n", pm_tmr);
    ఒకవేళ (pm_tmr)
        pmtimer_అమరిక(pm_tmr);
    ఒకవేళ (pm1a_cnt)
        acpi_pm1a_cnt = pm1a_cnt;

    // Theoretically we should check the 'reset_reg_sup' flag, but Windows
    // doesn't and thus nobody seems to *set* it. If the table is large enough
    // to include it, let the sanity checks in acpi_set_reset_reg() suffice.
    ఒకవేళ (fadt->నిడివి >= 129) {
        శూన్య *p = fadt;
        acpi_సమితి_reset_reg(p + 116, *(u8 *)(p + 128));
    }
    acpi_dsdt_parse();
}


/****************************************************************
 * SMBIOS
 ****************************************************************/

// Iterator for each sub-table in the smbios blob.
శూన్య *
smbios_తదుపరి(శూన్య *ప్రారంభం, u32 నిడివి, శూన్య *prev)
{
    ఒకవేళ (!ప్రారంభం)
        తిరిగి ఖాళీ;
    శూన్య *చివర = ప్రారంభం + నిడివి;

    ఒకవేళ (!prev) {
        prev = ప్రారంభం;
    } లేకపోతే {
        నిర్మాణం smbios_structure_శీర్షిక *hdr = prev;
        ఒకవేళ (prev + పరిమాణం(*hdr) > చివర)
            తిరిగి ఖాళీ;
        prev += hdr->నిడివి + 2;
        అయితే (prev < చివర && (*(u8*)(prev-1) != '\0' || *(u8*)(prev-2) != '\0'))
            prev++;
    }
    నిర్మాణం smbios_structure_శీర్షిక *hdr = prev;
    ఒకవేళ (prev >= చివర || prev + పరిమాణం(*hdr) >= చివర || prev + hdr->నిడివి >= చివర)
        తిరిగి ఖాళీ;
    తిరిగి prev;
}

శూన్య *
smbios_21_తదుపరి(నిర్మాణం smbios_21_నమోదు_బిందువు *smbios, శూన్య *prev)
{
    ఒకవేళ (!smbios)
        తిరిగి ఖాళీ;
    తిరిగి smbios_తదుపరి((శూన్య*)smbios->structure_పట్టిక_చిరునామా,
                       smbios->structure_పట్టిక_నిడివి, prev);
}

స్థిర నిర్మాణం smbios_21_నమోదు_బిందువు *SMBios21Addr;

శూన్య
copy_smbios_21(శూన్య *pos)
{
    ఒకవేళ (SMBios21Addr)
        తిరిగి;
    నిర్మాణం smbios_21_నమోదు_బిందువు *p = pos;
    ఒకవేళ (p->signature != SMBIOS_21_SIGNATURE)
        తిరిగి;
    ఒకవేళ (checksum(pos, 0x10) != 0)
        తిరిగి;
    ఒకవేళ (memcmp(p->intermediate_anchor_string, "_DMI_", 5))
        తిరిగి;
    ఒకవేళ (checksum(pos+0x10, p->నిడివి-0x10) != 0)
        తిరిగి;
    SMBios21Addr = copy_fseg_పట్టిక("SMBIOS", pos, p->నిడివి);
}

స్థిర నిర్మాణం smbios_30_నమోదు_బిందువు *SMBios30Addr;

స్థిర పూర్ణాంకం
valid_smbios_30_signature(నిర్మాణం smbios_30_నమోదు_బిందువు *p)
{
    తిరిగి !memcmp(p->signature, "_SM3_", 5);
}

శూన్య
copy_smbios_30(శూన్య *pos)
{
    ఒకవేళ (SMBios30Addr)
        తిరిగి;
    నిర్మాణం smbios_30_నమోదు_బిందువు *p = pos;
    ఒకవేళ (!valid_smbios_30_signature(p))
        తిరిగి;
    ఒకవేళ (checksum(pos, p->నిడివి) != 0)
        తిరిగి;
    SMBios30Addr = copy_fseg_పట్టిక("SMBIOS 3.0", pos, p->నిడివి);
}

శూన్య *smbios_get_tables(u32 *నిడివి)
{
    ఒకవేళ (SMBios30Addr) {
        u32 addr32 = SMBios30Addr->structure_పట్టిక_చిరునామా;
        ఒకవేళ (addr32 == SMBios30Addr->structure_పట్టిక_చిరునామా) {
            *నిడివి = SMBios30Addr->structure_పట్టిక_max_కొలత;
            తిరిగి (శూన్య *)addr32;
        }
    }
    ఒకవేళ (SMBios21Addr) {
        *నిడివి = SMBios21Addr->structure_పట్టిక_నిడివి;
        తిరిగి (శూన్య *)SMBios21Addr->structure_పట్టిక_చిరునామా;
    }
    తిరిగి ఖాళీ;
}

స్థిర పూర్ణాంకం
smbios_major_సంస్కరణ(శూన్య)
{
    ఒకవేళ (SMBios30Addr)
        తిరిగి SMBios30Addr->smbios_major_సంస్కరణ;
    లేకపోతే ఒకవేళ (SMBios21Addr)
        తిరిగి SMBios21Addr->smbios_major_సంస్కరణ;
    లేకపోతే
        తిరిగి 0;
}

స్థిర పూర్ణాంకం
smbios_minor_సంస్కరణ(శూన్య)
{
    ఒకవేళ (SMBios30Addr)
        తిరిగి SMBios30Addr->smbios_minor_సంస్కరణ;
    లేకపోతే ఒకవేళ (SMBios21Addr)
        తిరిగి SMBios21Addr->smbios_minor_సంస్కరణ;
    లేకపోతే
        తిరిగి 0;
}

శూన్య
display_uuid(శూన్య)
{
    u32 smbios_len = 0;
    శూన్య *smbios_tables = smbios_get_tables(&smbios_len);
    నిర్మాణం smbios_type_1 *tbl = smbios_తదుపరి(smbios_tables, smbios_len, ఖాళీ);
    పూర్ణాంకం minlen = offsetof(నిర్మాణం smbios_type_1, uuid) + పరిమాణం(tbl->uuid);
    కొరకు (; tbl; tbl = smbios_తదుపరి(smbios_tables, smbios_len, tbl))
        ఒకవేళ (tbl->శీర్షిక.type == 1 && tbl->శీర్షిక.నిడివి >= minlen) {
            u8 *uuid = tbl->uuid;
            u8 ఖాళీలు_uuid[పరిమాణం(tbl->uuid)] = { 0 };
            ఒకవేళ (memcmp(uuid, ఖాళీలు_uuid, పరిమాణం(ఖాళీలు_uuid)) == 0)
                తిరిగి;

            /*
             * According to SMBIOS v2.6 the first three fields are encoded in
             * little-endian format.  Versions prior to v2.6 did not specify
             * the encoding, but we follow dmidecode and assume big-endian
             * encoding.
             */
            ఒకవేళ (smbios_major_సంస్కరణ() > 2 ||
                (smbios_major_సంస్కరణ() == 2 &&
                 smbios_minor_సంస్కరణ() >= 6)) {
                రాయి("Machine UUID"
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
            } లేకపోతే {
                రాయి("Machine UUID"
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

            తిరిగి;
        }
}

#నిర్వచించు సమితి_str_క్షేత్రం_or_skip(type, క్షేత్రం, విలువ)                       \
    చేయి {                                                                \
        పూర్ణాంకం కొలత = (విలువ != ఖాళీ) ? అక్షరశ్రేణి_పొడవు(విలువ) + 1 : 0;             \
        ఒకవేళ (కొలత > 1) {                                                 \
            జ్ఞాపకం_నకలు(చివర, విలువ, కొలత);                                   \
            చివర += కొలత;                                                \
            p->క్షేత్రం = ++str_సూచీ;                                     \
        } లేకపోతే {                                                        \
            p->క్షేత్రం = 0;                                               \
        }                                                               \
    } అయితే (0)

స్థిర శూన్య *
smbios_new_type_0(శూన్య *ప్రారంభం,
                  స్థిరరాశి అక్షరం *vendor, స్థిరరాశి అక్షరం *సంస్కరణ_2, స్థిరరాశి అక్షరం *date)
{
    నిర్మాణం smbios_type_0 *p = (నిర్మాణం smbios_type_0 *)ప్రారంభం;
    అక్షరం *చివర = (అక్షరం *)ప్రారంభం + పరిమాణం(నిర్మాణం smbios_type_0);
    పూర్ణాంకం str_సూచీ = 0;

    p->శీర్షిక.type = 0;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_0);
    p->శీర్షిక.handle = 0;

    సమితి_str_క్షేత్రం_or_skip(0, vendor_str, vendor);
    సమితి_str_క్షేత్రం_or_skip(0, bios_సంస్కరణ_str, సంస్కరణ_2);
    p->bios_starting_చిరునామా_segment = 0xe800;
    సమితి_str_క్షేత్రం_or_skip(0, bios_release_date_str, date);

    p->bios_rom_కొలత = 0; /* FIXME */

    /* BIOS characteristics not supported */
    memset(p->bios_characteristics, 0, 8);
    p->bios_characteristics[0] = 0x08;

    /* Enable targeted content distribution (needed for SVVP) */
    p->bios_characteristics_extension_బైట్లు[0] = 0;
    p->bios_characteristics_extension_బైట్లు[1] = 4;

    p->system_bios_major_release = 0;
    p->system_bios_minor_release = 0;
    p->embedded_controller_major_release = 0xFF;
    p->embedded_controller_minor_release = 0xFF;

    *చివర = 0;
    చివర++;
    ఒకవేళ (!str_సూచీ) {
        *చివర = 0;
        చివర++;
    }

    తిరిగి చివర;
}

#నిర్వచించు BIOS_NAME "SeaBIOS"
#నిర్వచించు BIOS_DATE "04/01/2014"

/*
 * Build tables using qtables as input, adding additional type 0
 * table if necessary.
 *
 * @address and @length can't be NULL.  @max_structure_size and
 * @number_of_structures are optional and can be NULL.
 */
స్థిర పూర్ణాంకం
smbios_build_tables(నిర్మాణం romfile_s *f_tables,
                    u64 *చిరునామా, u32 *నిడివి,
                    u16 *max_structure_కొలత,
                    u16 *సంఖ్య_యొక్క_structures)
{
    నిర్మాణం smbios_type_0 *t0;
    u32 qtables_len, need_t0 = 1;
    u8 *qtables, *tables;

    ఒకవేళ (f_tables->కొలత != *నిడివి)
        తిరిగి 0;

    qtables = malloc_tmphigh(f_tables->కొలత);
    ఒకవేళ (!qtables) {
        warn_noalloc();
        తిరిగి 0;
    }
    f_tables->copy(f_tables, qtables, f_tables->కొలత);
    qtables_len = f_tables->కొలత;

    /* did we get a type 0 structure ? */
    కొరకు (t0 = smbios_తదుపరి(qtables, qtables_len, ఖాళీ); t0;
         t0 = smbios_తదుపరి(qtables, qtables_len, t0)) {
        ఒకవేళ (t0->శీర్షిక.type == 0) {
            need_t0 = 0;
            ఆపు;
        }
    }

    ఒకవేళ (need_t0) {
        /* common case: add our own type 0, with 3 strings and 4 '\0's */
        u16 t0_len = పరిమాణం(నిర్మాణం smbios_type_0) + అక్షరశ్రేణి_పొడవు(BIOS_NAME) +
                     అక్షరశ్రేణి_పొడవు(సంస్కరణ) + అక్షరశ్రేణి_పొడవు(BIOS_DATE) + 4;
        ఒకవేళ (t0_len > (0xffff - *నిడివి)) {
            dprintf(1, "Insufficient space (%d bytes) to add SMBIOS type 0 table (%d bytes)\n",
                    0xffff - *నిడివి, t0_len);
            need_t0 = 0;
        } లేకపోతే {
            *నిడివి += t0_len;
            ఒకవేళ (max_structure_కొలత && t0_len > *max_structure_కొలత)
                *max_structure_కొలత = t0_len;
            ఒకవేళ (సంఖ్య_యొక్క_structures)
                (*సంఖ్య_యొక్క_structures)++;
        }
    }

    /* allocate final blob and record its address in the entry point */
    ఒకవేళ (*నిడివి > BUILD_MAX_SMBIOS_FSEG)
        tables = malloc_ఎగువ(*నిడివి);
    లేకపోతే
        tables = malloc_fseg(*నిడివి);
    ఒకవేళ (!tables) {
        warn_noalloc();
        విడుదల(qtables);
        తిరిగి 0;
    }
    *చిరునామా = (u32)tables;

    /* populate final blob */
    ఒకవేళ (need_t0)
        tables = smbios_new_type_0(tables, BIOS_NAME, సంస్కరణ, BIOS_DATE);
    జ్ఞాపకం_నకలు(tables, qtables, qtables_len);
    విడుదల(qtables);
    తిరిగి 1;
}

స్థిర పూర్ణాంకం
smbios_21_అమరిక_నమోదు_బిందువు(నిర్మాణం romfile_s *f_tables,
                            నిర్మాణం smbios_21_నమోదు_బిందువు *ep)
{
    u64 చిరునామా = ep->structure_పట్టిక_చిరునామా;
    u32 నిడివి = ep->structure_పట్టిక_నిడివి;

    ఒకవేళ (!smbios_build_tables(f_tables,
                             &చిరునామా,
                             &నిడివి,
                             &ep->max_structure_కొలత,
                             &ep->సంఖ్య_యొక్క_structures))
        తిరిగి 0;

    ఒకవేళ ((u32)చిరునామా != చిరునామా || (u16)నిడివి != నిడివి) {
        warn_internalerror();
        తిరిగి 0;
    }

    /* finalize entry point */
    ep->structure_పట్టిక_చిరునామా = చిరునామా;
    ep->structure_పట్టిక_నిడివి = నిడివి;
    ep->checksum -= checksum(ep, 0x10);
    ep->intermediate_checksum -= checksum((శూన్య *)ep + 0x10, ep->నిడివి - 0x10);

    copy_smbios_21(ep);
    తిరిగి 1;
}

స్థిర పూర్ణాంకం
smbios_30_అమరిక_నమోదు_బిందువు(నిర్మాణం romfile_s *f_tables,
                            నిర్మాణం smbios_30_నమోదు_బిందువు *ep)
{
    ఒకవేళ (!smbios_build_tables(f_tables,
                             &ep->structure_పట్టిక_చిరునామా,
                             &ep->structure_పట్టిక_max_కొలత,
                             ఖాళీ, ఖాళీ))
        తిరిగి 0;

    ep->checksum -= checksum(ep, పరిమాణం(*ep));
    copy_smbios_30(ep);
    తిరిగి 1;
}

స్థిర పూర్ణాంకం
smbios_romfile_అమరిక(శూన్య)
{
    నిర్మాణం romfile_s *f_anchor = romfile_కనుగొను("etc/smbios/smbios-anchor");
    నిర్మాణం romfile_s *f_tables = romfile_కనుగొను("etc/smbios/smbios-tables");
    సంఘం {
        నిర్మాణం smbios_21_నమోదు_బిందువు ep21;
        నిర్మాణం smbios_30_నమోదు_బిందువు ep30;
    } ep;

    ఒకవేళ (!f_anchor || !f_tables || f_anchor->కొలత > పరిమాణం(ep))
        తిరిగి 0;

    f_anchor->copy(f_anchor, &ep, f_anchor->కొలత);

    ఒకవేళ (f_anchor->కొలత == పరిమాణం(ep.ep21) &&
        ep.ep21.signature == SMBIOS_21_SIGNATURE) {
        తిరిగి smbios_21_అమరిక_నమోదు_బిందువు(f_tables, &ep.ep21);
    } లేకపోతే ఒకవేళ (f_anchor->కొలత == పరిమాణం(ep.ep30) &&
               valid_smbios_30_signature(&ep.ep30)) {
        తిరిగి smbios_30_అమరిక_నమోదు_బిందువు(f_tables, &ep.ep30);
    } లేకపోతే {
        dprintf(1, "Invalid SMBIOS signature at etc/smbios/smbios-anchor\n");
        తిరిగి 0;
    }
}

శూన్య
smbios_అమరిక(శూన్య)
{
    ఒకవేళ (smbios_romfile_అమరిక())
        తిరిగి;
    smbios_legacy_అమరిక();
}

శూన్య
copy_పట్టిక(శూన్య *pos)
{
    copy_pir(pos);
    copy_mptable(pos);
    copy_acpi_rsdp(pos);
    copy_smbios_21(pos);
    copy_smbios_30(pos);
}
