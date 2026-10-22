// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // RamSize
#అన్తర్భావ "romfile.h" // romfile_findprefix
#అన్తర్భావ "std/smbios.h" // struct smbios_21_entry_point
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // MaxCountCPUs
#అన్తర్భావ "x86.h" // cpuid

స్థిర శూన్య
smbios_21_నమోదు_బిందువు_అమరిక(u16 max_structure_కొలత,
                         u16 structure_పట్టిక_నిడివి,
                         శూన్య *structure_పట్టిక_చిరునామా,
                         u16 సంఖ్య_యొక్క_structures)
{
    శూన్య *finaltable;
    ఒకవేళ (structure_పట్టిక_నిడివి <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_పట్టిక_నిడివి);
    లేకపోతే
        finaltable = malloc_ఎగువ(structure_పట్టిక_నిడివి);
    ఒకవేళ (!finaltable) {
        warn_noalloc();
        తిరిగి;
    }
    జ్ఞాపకం_నకలు(finaltable, structure_పట్టిక_చిరునామా, structure_పట్టిక_నిడివి);

    నిర్మాణం smbios_21_నమోదు_బిందువు ep;
    memset(&ep, 0, పరిమాణం(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.నిడివి = 0x1f;
    ep.smbios_major_సంస్కరణ = 2;
    ep.smbios_minor_సంస్కరణ = 4;
    ep.max_structure_కొలత = max_structure_కొలత;
    జ్ఞాపకం_నకలు(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_పట్టిక_నిడివి = structure_పట్టిక_నిడివి;
    ep.structure_పట్టిక_చిరునామా = (u32)finaltable;
    ep.సంఖ్య_యొక్క_structures = సంఖ్య_యొక్క_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((శూన్య*)&ep + 0x10, ep.నిడివి - 0x10);

    copy_smbios_21(&ep);
}

స్థిర పూర్ణాంకం
get_క్షేత్రం(పూర్ణాంకం type, పూర్ణాంకం స్థానభ్రంశం_2, శూన్య *dest)
{
    అక్షరం name[128];
    snprintf(name, పరిమాణం(name), "smbios/field%d-%d", type, స్థానభ్రంశం_2);
    నిర్మాణం romfile_s *file = romfile_కనుగొను(name);
    ఒకవేళ (!file)
        తిరిగి 0;
    file->copy(file, dest, file->కొలత);
    తిరిగి file->కొలత;
}

స్థిర పూర్ణాంకం
get_external(పూర్ణాంకం type, అక్షరం **p, గుర్తులేని *nr_structs,
             గుర్తులేని *max_struct_కొలత, అక్షరం *చివర)
{
    స్థిర u64 వాడిన_bitmap[4] = { 0 };
    అక్షరం *ప్రారంభం = *p;

    /* Check if we've already reported these tables */
    ఒకవేళ (వాడిన_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        తిరిగి 1;

    /* Don't introduce spurious end markers */
    ఒకవేళ (type == 127)
        తిరిగి 0;

    అక్షరం ఉపసర్గ[128];
    snprintf(ఉపసర్గ, పరిమాణం(ఉపసర్గ), "smbios/table%d-", type);
    నిర్మాణం romfile_s *file = ఖాళీ;
    కొరకు (;;) {
        file = romfile_findprefix(ఉపసర్గ, file);
        ఒకవేళ (!file)
            ఆపు;

        ఒకవేళ (చివర - *p < file->కొలత) {
            warn_noalloc();
            ఆపు;
        }

        నిర్మాణం smbios_structure_శీర్షిక *శీర్షిక = (శూన్య*)*p;
        file->copy(file, శీర్షిక, file->కొలత);
        *p += file->కొలత;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        ఒకవేళ (శీర్షిక->నిడివి >= file->కొలత) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        ఒకవేళ (*p - (అక్షరం*)శీర్షిక > *max_struct_కొలత)
            *max_struct_కొలత = *p - (అక్షరం*)శీర్షిక;
    }

    ఒకవేళ (ప్రారంభం == *p)
        తిరిగి 0;

    /* Mark that we've reported on this type */
    వాడిన_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    తిరిగి 1;
}

#నిర్వచించు ఎక్కించు_str_క్షేత్రం_with_default(type, క్షేత్రం, def)                   \
    చేయి {                                                                \
        కొలత = get_క్షేత్రం(type, offsetof(నిర్మాణం smbios_type_##type,      \
                                        క్షేత్రం), చివర);                   \
        ఒకవేళ (కొలత == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->క్షేత్రం = 0;                                               \
        } లేకపోతే ఒకవేళ (కొలత > 1) {                                          \
            చివర += కొలత;                                                \
            p->క్షేత్రం = ++str_సూచీ;                                     \
        } లేకపోతే {                                                        \
            జ్ఞాపకం_నకలు(చివర, def, పరిమాణం(def));                              \
            చివర += పరిమాణం(def);                                         \
            p->క్షేత్రం = ++str_సూచీ;                                     \
        }                                                               \
    } అయితే (0)

#నిర్వచించు ఎక్కించు_str_క్షేత్రం_or_skip(type, క్షేత్రం)                             \
    చేయి {                                                                \
        కొలత = get_క్షేత్రం(type, offsetof(నిర్మాణం smbios_type_##type,      \
                                        క్షేత్రం), చివర);                   \
        ఒకవేళ (కొలత > 1) {                                                 \
            చివర += కొలత;                                                \
            p->క్షేత్రం = ++str_సూచీ;                                     \
        } లేకపోతే {                                                        \
            p->క్షేత్రం = 0;                                               \
        }                                                               \
    } అయితే (0)

#నిర్వచించు సమితి_క్షేత్రం_with_default(type, క్షేత్రం, def)                        \
    చేయి {                                                                \
        ఒకవేళ (!get_క్షేత్రం(type, offsetof(నిర్మాణం smbios_type_##type,        \
                                      క్షేత్రం), &p->క్షేత్రం)) {             \
            p->క్షేత్రం = def;                                             \
        }                                                               \
    } అయితే (0)

/* Type 0 -- BIOS Information */
#నిర్వచించు RELEASE_DATE_STR "01/01/2011"
స్థిర శూన్య *
smbios_init_type_0(శూన్య *ప్రారంభం)
{
    నిర్మాణం smbios_type_0 *p = (నిర్మాణం smbios_type_0 *)ప్రారంభం;
    అక్షరం *చివర = (అక్షరం *)ప్రారంభం + పరిమాణం(నిర్మాణం smbios_type_0);
    కొలత_t కొలత;
    పూర్ణాంకం str_సూచీ = 0;

    p->శీర్షిక.type = 0;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_0);
    p->శీర్షిక.handle = 0;

    ఎక్కించు_str_క్షేత్రం_with_default(0, vendor_str, BUILD_APPNAME);
    ఎక్కించు_str_క్షేత్రం_with_default(0, bios_సంస్కరణ_str, BUILD_APPNAME);

    p->bios_starting_చిరునామా_segment = 0xe800;

    ఎక్కించు_str_క్షేత్రం_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_కొలత = 0; /* FIXME */

    ఒకవేళ (!get_క్షేత్రం(0, offsetof(నిర్మాణం smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    ఒకవేళ (!get_క్షేత్రం(0, offsetof(నిర్మాణం smbios_type_0,
                               bios_characteristics_extension_బైట్లు),
                   &p->bios_characteristics_extension_బైట్లు)) {
        p->bios_characteristics_extension_బైట్లు[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_బైట్లు[1] = 4;
    }

    సమితి_క్షేత్రం_with_default(0, system_bios_major_release, 1);
    సమితి_క్షేత్రం_with_default(0, system_bios_minor_release, 0);
    సమితి_క్షేత్రం_with_default(0, embedded_controller_major_release, 0xff);
    సమితి_క్షేత్రం_with_default(0, embedded_controller_minor_release, 0xff);

    *చివర = 0;
    చివర++;
    ఒకవేళ (!str_సూచీ) {
        *చివర = 0;
        చివర++;
    }

    తిరిగి చివర;
}

/* Type 1 -- System Information */
స్థిర శూన్య *
smbios_init_type_1(శూన్య *ప్రారంభం)
{
    నిర్మాణం smbios_type_1 *p = (నిర్మాణం smbios_type_1 *)ప్రారంభం;
    అక్షరం *చివర = (అక్షరం *)ప్రారంభం + పరిమాణం(నిర్మాణం smbios_type_1);
    కొలత_t కొలత;
    పూర్ణాంకం str_సూచీ = 0;

    p->శీర్షిక.type = 1;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_1);
    p->శీర్షిక.handle = 0x100;

    ఎక్కించు_str_క్షేత్రం_with_default(1, manufacturer_str, BUILD_APPNAME);
    ఎక్కించు_str_క్షేత్రం_with_default(1, లబ్ధం_name_str, BUILD_APPNAME);
    ఎక్కించు_str_క్షేత్రం_or_skip(1, సంస్కరణ_str);
    ఎక్కించు_str_క్షేత్రం_or_skip(1, serial_సంఖ్య_str);

    ఒకవేళ (!get_క్షేత్రం(1, offsetof(నిర్మాణం smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    సమితి_క్షేత్రం_with_default(1, wake_పైకి_type, 0x06); /* power switch */

    ఎక్కించు_str_క్షేత్రం_or_skip(1, sku_సంఖ్య_str);
    ఎక్కించు_str_క్షేత్రం_or_skip(1, family_str);

    *చివర = 0;
    చివర++;
    ఒకవేళ (!str_సూచీ) {
        *చివర = 0;
        చివర++;
    }

    తిరిగి చివర;
}

/* Type 3 -- System Enclosure */
స్థిర శూన్య *
smbios_init_type_3(శూన్య *ప్రారంభం)
{
    నిర్మాణం smbios_type_3 *p = (నిర్మాణం smbios_type_3 *)ప్రారంభం;
    అక్షరం *చివర = (అక్షరం *)ప్రారంభం + పరిమాణం(నిర్మాణం smbios_type_3);
    కొలత_t కొలత;
    పూర్ణాంకం str_సూచీ = 0;

    p->శీర్షిక.type = 3;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_3);
    p->శీర్షిక.handle = 0x300;

    ఎక్కించు_str_క్షేత్రం_with_default(3, manufacturer_str, BUILD_APPNAME);
    సమితి_క్షేత్రం_with_default(3, type, 0x01); /* other */

    ఎక్కించు_str_క్షేత్రం_or_skip(3, సంస్కరణ_str);
    ఎక్కించు_str_క్షేత్రం_or_skip(3, serial_సంఖ్య_str);
    ఎక్కించు_str_క్షేత్రం_or_skip(3, asset_ట్యాగ్_సంఖ్య_str);

    సమితి_క్షేత్రం_with_default(3, boot_పైకి_state, 0x03); /*సురక్షిత*/
    సమితి_క్షేత్రం_with_default(3, power_supply_state, 0x03); /*సురక్షిత*/
    సమితి_క్షేత్రం_with_default(3, thermal_state, 0x03); /*సురక్షిత*/
    సమితి_క్షేత్రం_with_default(3, security_స్థితి, 0x02); /* unknown */

    సమితి_క్షేత్రం_with_default(3, oem_defined, 0);
    సమితి_క్షేత్రం_with_default(3, ఎత్తు, 0);
    సమితి_క్షేత్రం_with_default(3, సంఖ్య_యొక్క_power_cords, 0);
    సమితి_క్షేత్రం_with_default(3, contained_element_లెక్క, 0);

    *చివర = 0;
    చివర++;
    ఒకవేళ (!str_సూచీ) {
        *చివర = 0;
        చివర++;
    }

    తిరిగి చివర;
}

/* Type 4 -- Processor Information */
స్థిర శూన్య *
smbios_init_type_4(శూన్య *ప్రారంభం, గుర్తులేని పూర్ణాంకం cpu_సంఖ్య)
{
    నిర్మాణం smbios_type_4 *p = (నిర్మాణం smbios_type_4 *)ప్రారంభం;
    అక్షరం *చివర = (అక్షరం *)ప్రారంభం + పరిమాణం(నిర్మాణం smbios_type_4);
    కొలత_t కొలత;
    పూర్ణాంకం str_సూచీ = 0;
    అక్షరం name[1024];

    p->శీర్షిక.type = 4;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_4);
    p->శీర్షిక.handle = 0x400 + cpu_సంఖ్య;

    కొలత = get_క్షేత్రం(4, offsetof(నిర్మాణం smbios_type_4, socket_designation_str),
                     name);
    ఒకవేళ (కొలత)
        snprintf(name + కొలత - 1, పరిమాణం(name) - కొలత, "%2x", cpu_సంఖ్య);
    లేకపోతే
        snprintf(name, పరిమాణం(name), "CPU%2x", cpu_సంఖ్య);

    జ్ఞాపకం_నకలు(చివర, name, అక్షరశ్రేణి_పొడవు(name) + 1);
    చివర += అక్షరశ్రేణి_పొడవు(name) + 1;
    p->socket_designation_str = ++str_సూచీ;

    సమితి_క్షేత్రం_with_default(4, processor_type, 0x03); /* CPU */
    సమితి_క్షేత్రం_with_default(4, processor_family, 0x01); /* other */

    ఎక్కించు_str_క్షేత్రం_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    ఒకవేళ (!get_క్షేత్రం(4, offsetof(నిర్మాణం smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    ఎక్కించు_str_క్షేత్రం_or_skip(4, processor_సంస్కరణ_str);
    సమితి_క్షేత్రం_with_default(4, voltage, 0);
    సమితి_క్షేత్రం_with_default(4, external_clock, 0);

    సమితి_క్షేత్రం_with_default(4, max_వేగం, 2000);
    సమితి_క్షేత్రం_with_default(4, current_వేగం, 2000);

    సమితి_క్షేత్రం_with_default(4, స్థితి_2, 0x41); /* socket populated, CPU enabled */
    సమితి_క్షేత్రం_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *చివర = 0;
    చివర++;
    ఒకవేళ (!str_సూచీ) {
        *చివర = 0;
        చివర++;
    }

    తిరిగి చివర;
}

/* Type 16 -- Physical Memory Array */
స్థిర శూన్య *
smbios_init_type_16(శూన్య *ప్రారంభం, u32 జ్ఞాపకం_కొలత_mb, పూర్ణాంకం nr_mem_devs)
{
    నిర్మాణం smbios_type_16 *p = (నిర్మాణం smbios_type_16*)ప్రారంభం;

    p->శీర్షిక.type = 16;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_16);
    p->శీర్షిక.handle = 0x1000;

    సమితి_క్షేత్రం_with_default(16, location, 0x01); /* other */
    సమితి_క్షేత్రం_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    సమితి_క్షేత్రం_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->గరిష్ఠ_సామర్థ్యం = జ్ఞాపకం_కొలత_mb < 2 << 20 ?
                          జ్ఞాపకం_కొలత_mb << 10 : 0x80000000;
    p->జ్ఞాపకం_error_information_handle = 0xfffe; /* none provided */
    p->సంఖ్య_యొక్క_జ్ఞాపకం_devices = nr_mem_devs;

    ప్రారంభం += పరిమాణం(నిర్మాణం smbios_type_16);
    *((u16 *)ప్రారంభం) = 0;

    తిరిగి ప్రారంభం + 2;
}

/* Type 17 -- Memory Device */
స్థిర శూన్య *
smbios_init_type_17(శూన్య *ప్రారంభం, u32 కొలత_mb, పూర్ణాంకం instance)
{
    నిర్మాణం smbios_type_17 *p = (నిర్మాణం smbios_type_17 *)ప్రారంభం;
    అక్షరం *చివర = (అక్షరం *)ప్రారంభం + పరిమాణం(నిర్మాణం smbios_type_17);
    కొలత_t కొలత;
    పూర్ణాంకం str_సూచీ = 0;
    అక్షరం name[1024];

    p->శీర్షిక.type = 17;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_17);
    p->శీర్షిక.handle = 0x1100 + instance;

    p->physical_జ్ఞాపకం_array_handle = 0x1000;
    సమితి_క్షేత్రం_with_default(17, మొత్తం_వెడల్పు, 64);
    సమితి_క్షేత్రం_with_default(17, దత్తాంశం_వెడల్పు, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->కొలత = కొలత_mb;
    సమితి_క్షేత్రం_with_default(17, form_గుణకం, 0x09); /* DIMM */
    p->device_సమితి = 0;

    కొలత = get_క్షేత్రం(17, offsetof(నిర్మాణం smbios_type_17, device_locator_str),
                     name);
    ఒకవేళ (కొలత)
        snprintf(name + కొలత - 1, పరిమాణం(name) - కొలత, "%d", instance);
    లేకపోతే
        snprintf(name, పరిమాణం(name), "DIMM %d", instance);

    జ్ఞాపకం_నకలు(చివర, name, అక్షరశ్రేణి_పొడవు(name) + 1);
    చివర += అక్షరశ్రేణి_పొడవు(name) + 1;
    p->device_locator_str = ++str_సూచీ;

    ఎక్కించు_str_క్షేత్రం_or_skip(17, bank_locator_str);
    సమితి_క్షేత్రం_with_default(17, జ్ఞాపకం_type, 0x07); /* RAM */
    సమితి_క్షేత్రం_with_default(17, type_detail, 0);

    *చివర = 0;
    చివర++;
    ఒకవేళ (!str_సూచీ) {
        *చివర = 0;
        చివర++;
    }

    తిరిగి చివర;
}

/* Type 19 -- Memory Array Mapped Address */
స్థిర శూన్య *
smbios_init_type_19(శూన్య *ప్రారంభం, u32 ప్రారంభం_mb, u32 కొలత_mb, పూర్ణాంకం instance)
{
    నిర్మాణం smbios_type_19 *p = (నిర్మాణం smbios_type_19 *)ప్రారంభం;

    p->శీర్షిక.type = 19;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_19);
    p->శీర్షిక.handle = 0x1300 + instance;

    p->starting_చిరునామా = ప్రారంభం_mb << 10;
    p->ending_చిరునామా = p->starting_చిరునామా + (కొలత_mb << 10) - 1;
    p->జ్ఞాపకం_array_handle = 0x1000;
    p->విభజన_వెడల్పు = 1;

    ప్రారంభం += పరిమాణం(నిర్మాణం smbios_type_19);
    *((u16 *)ప్రారంభం) = 0;

    తిరిగి ప్రారంభం + 2;
}

/* Type 20 -- Memory Device Mapped Address */
స్థిర శూన్య *
smbios_init_type_20(శూన్య *ప్రారంభం, u32 ప్రారంభం_mb, u32 కొలత_mb, పూర్ణాంకం instance,
                    పూర్ణాంకం dev_handle, పూర్ణాంకం array_handle)
{
    నిర్మాణం smbios_type_20 *p = (నిర్మాణం smbios_type_20 *)ప్రారంభం;

    p->శీర్షిక.type = 20;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_20);
    p->శీర్షిక.handle = 0x1400 + instance;

    p->starting_చిరునామా = ప్రారంభం_mb << 10;
    p->ending_చిరునామా = p->starting_చిరునామా + (కొలత_mb << 10) - 1;
    p->జ్ఞాపకం_device_handle = 0x1100 + dev_handle;
    p->జ్ఞాపకం_array_mapped_చిరునామా_handle = 0x1300 + array_handle;
    p->విభజన_అడ్డువరుస_position = 1;
    p->interleave_position = 0;
    p->interleaved_దత్తాంశం_లోతు = 0;

    ప్రారంభం += పరిమాణం(నిర్మాణం smbios_type_20);

    *((u16 *)ప్రారంభం) = 0;
    తిరిగి ప్రారంభం+2;
}

/* Type 32 -- System Boot Information */
స్థిర శూన్య *
smbios_init_type_32(శూన్య *ప్రారంభం)
{
    నిర్మాణం smbios_type_32 *p = (నిర్మాణం smbios_type_32 *)ప్రారంభం;

    p->శీర్షిక.type = 32;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_32);
    p->శీర్షిక.handle = 0x2000;
    memset(p->reserved, 0, 6);
    సమితి_క్షేత్రం_with_default(32, boot_స్థితి, 0); /* no errors detected */

    ప్రారంభం += పరిమాణం(నిర్మాణం smbios_type_32);
    *((u16 *)ప్రారంభం) = 0;

    తిరిగి ప్రారంభం+2;
}

/* Type 127 -- End of Table */
స్థిర శూన్య *
smbios_init_type_127(శూన్య *ప్రారంభం)
{
    నిర్మాణం smbios_type_127 *p = (నిర్మాణం smbios_type_127 *)ప్రారంభం;

    p->శీర్షిక.type = 127;
    p->శీర్షిక.నిడివి = పరిమాణం(నిర్మాణం smbios_type_127);
    p->శీర్షిక.handle = 0x7f00;

    ప్రారంభం += పరిమాణం(నిర్మాణం smbios_type_127);
    *((u16 *)ప్రారంభం) = 0;

    తిరిగి ప్రారంభం + 2;
}

#నిర్వచించు TEMPSMBIOSSIZE (32 * 1024)

శూన్య
smbios_legacy_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_SMBIOS)
        తిరిగి;

    dprintf(3, "init SMBIOS tables\n");

    అక్షరం *ప్రారంభం = malloc_tmphigh(TEMPSMBIOSSIZE);
    ఒకవేళ (! ప్రారంభం) {
        warn_noalloc();
        తిరిగి;
    }
    memset(ప్రారంభం, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_కొలత = 0;
    అక్షరం *q, *p = ప్రారంభం;
    అక్షరం *చివర = ప్రారంభం + TEMPSMBIOSSIZE - పరిమాణం(నిర్మాణం smbios_type_127);

#నిర్వచించు కూడు_struct(type, వాదనలు...)                                       \
    చేయి {                                                                \
        ఒకవేళ (!get_external(type, &p, &nr_structs, &max_struct_కొలత, చివర)) { \
            q = smbios_init_type_##type(వాదనలు);                          \
            nr_structs++;                                               \
            ఒకవేళ ((q - p) > max_struct_కొలత)                              \
                max_struct_కొలత = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } అయితే (0)

    కూడు_struct(0, p);
    కూడు_struct(1, p);
    కూడు_struct(3, p);

    పూర్ణాంకం cpu_అంకె;
    కొరకు (cpu_అంకె = 1; cpu_అంకె <= Max_లెక్క_CPUs; cpu_అంకె++)
        కూడు_struct(4, p, cpu_అంకె);

    పూర్ణాంకం ram_mb = (Ram_కొలత + Ram_కొలత_Over4_G) >> 20;
    పూర్ణాంకం nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    కూడు_struct(16, p, ram_mb, nr_mem_devs);

    పూర్ణాంకం i, j;
    కొరకు (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        కూడు_struct(17, p, dev_mb, i);
    }

    కూడు_struct(19, p, 0, Ram_కొలత >> 20, 0);
    ఒకవేళ (Ram_కొలత_Over4_G)
        కూడు_struct(19, p, 4096, Ram_కొలత_Over4_G >> 20, 1);

    కూడు_struct(20, p, 0, Ram_కొలత >> 20, 0, 0, 0);
    ఒకవేళ (Ram_కొలత_Over4_G) {
        u32 ప్రారంభం_mb = 4096;
        కొరకు (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            ఒకవేళ (i == 0)
                dev_mb -= Ram_కొలత >> 20;

            కూడు_struct(20, p, ప్రారంభం_mb, dev_mb, j, i, 1);
            ప్రారంభం_mb += dev_mb;
        }
    }

    కూడు_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    కొరకు (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_కొలత, చివర);
    కూడు_struct(127, p);

#నిర్వచనం_రద్దు కూడు_struct

    smbios_21_నమోదు_బిందువు_అమరిక(max_struct_కొలత, p - ప్రారంభం, ప్రారంభం, nr_structs);
    విడుదల(ప్రారంభం);
}
