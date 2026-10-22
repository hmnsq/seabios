// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "paravirt.h" // RamSize
#अन्तर्भाव "romfile.h" // romfile_findprefix
#अन्तर्भाव "std/smbios.h" // struct smbios_21_entry_point
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // MaxCountCPUs
#अन्तर्भाव "x86.h" // cpuid

स्थिर शून्य
smbios_21_प्रविष्टिः_point_सज्जता(u16 max_structure_आकार,
                         u16 structure_सारणी_दीर्घता,
                         शून्य *structure_सारणी_स्थानाङ्कः,
                         u16 सङ्ख्या_सम्बन्धि_structures)
{
    शून्य *finaltable;
    यदि (structure_सारणी_दीर्घता <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_सारणी_दीर्घता);
    अन्यथा
        finaltable = malloc_उच्चः(structure_सारणी_दीर्घता);
    यदि (!finaltable) {
        warn_noalloc();
        प्रत्यागम;
    }
    स्मृतिं_प्रतिलिख(finaltable, structure_सारणी_स्थानाङ्कः, structure_सारणी_दीर्घता);

    संरचना smbios_21_प्रविष्टिः_point ep;
    स्मृतिं_पूरय(&ep, 0, परिमाण(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.दीर्घता = 0x1f;
    ep.smbios_major_संस्करणम् = 2;
    ep.smbios_minor_संस्करणम् = 4;
    ep.max_structure_आकार = max_structure_आकार;
    स्मृतिं_प्रतिलिख(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_सारणी_दीर्घता = structure_सारणी_दीर्घता;
    ep.structure_सारणी_स्थानाङ्कः = (u32)finaltable;
    ep.सङ्ख्या_सम्बन्धि_structures = सङ्ख्या_सम्बन्धि_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((शून्य*)&ep + 0x10, ep.दीर्घता - 0x10);

    copy_smbios_21(&ep);
}

स्थिर पूर्णाङ्क
get_क्षेत्रम्(पूर्णाङ्क type, पूर्णाङ्क विस्थापनम्_2, शून्य *dest)
{
    वर्ण name[128];
    snprintf(name, परिमाण(name), "smbios/field%d-%d", type, विस्थापनम्_2);
    संरचना romfile_s *file = romfile_find(name);
    यदि (!file)
        प्रत्यागम 0;
    file->copy(file, dest, file->आकार);
    प्रत्यागम file->आकार;
}

स्थिर पूर्णाङ्क
get_external(पूर्णाङ्क type, वर्ण **p, अचिह्नित *nr_structs,
             अचिह्नित *max_struct_आकार, वर्ण *अन्तः)
{
    स्थिर u64 प्रयुक्तम्_bitmap[4] = { 0 };
    वर्ण *start = *p;

    /* Check if we've already reported these tables */
    यदि (प्रयुक्तम्_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        प्रत्यागम 1;

    /* Don't introduce spurious end markers */
    यदि (type == 127)
        प्रत्यागम 0;

    वर्ण उपसर्गः[128];
    snprintf(उपसर्गः, परिमाण(उपसर्गः), "smbios/table%d-", type);
    संरचना romfile_s *file = रिक्त;
    प्रति (;;) {
        file = romfile_findprefix(उपसर्गः, file);
        यदि (!file)
            भङ्ग;

        यदि (अन्तः - *p < file->आकार) {
            warn_noalloc();
            भङ्ग;
        }

        संरचना smbios_structure_शीर्षकम् *शीर्षकम् = (शून्य*)*p;
        file->copy(file, शीर्षकम्, file->आकार);
        *p += file->आकार;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        यदि (शीर्षकम्->दीर्घता >= file->आकार) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        यदि (*p - (वर्ण*)शीर्षकम् > *max_struct_आकार)
            *max_struct_आकार = *p - (वर्ण*)शीर्षकम्;
    }

    यदि (start == *p)
        प्रत्यागम 0;

    /* Mark that we've reported on this type */
    प्रयुक्तम्_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    प्रत्यागम 1;
}

#परिभाषय आरोपय_str_क्षेत्रम्_with_default(type, क्षेत्रम्, def)                   \
    कुरु {                                                                \
        आकार = get_क्षेत्रम्(type, offsetof(संरचना smbios_type_##type,      \
                                        क्षेत्रम्), अन्तः);                   \
        यदि (आकार == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->क्षेत्रम् = 0;                                               \
        } अन्यथा यदि (आकार > 1) {                                          \
            अन्तः += आकार;                                                \
            p->क्षेत्रम् = ++str_सूचकांक;                                     \
        } अन्यथा {                                                        \
            स्मृतिं_प्रतिलिख(अन्तः, def, परिमाण(def));                              \
            अन्तः += परिमाण(def);                                         \
            p->क्षेत्रम् = ++str_सूचकांक;                                     \
        }                                                               \
    } यावत् (0)

#परिभाषय आरोपय_str_क्षेत्रम्_or_skip(type, क्षेत्रम्)                             \
    कुरु {                                                                \
        आकार = get_क्षेत्रम्(type, offsetof(संरचना smbios_type_##type,      \
                                        क्षेत्रम्), अन्तः);                   \
        यदि (आकार > 1) {                                                 \
            अन्तः += आकार;                                                \
            p->क्षेत्रम् = ++str_सूचकांक;                                     \
        } अन्यथा {                                                        \
            p->क्षेत्रम् = 0;                                               \
        }                                                               \
    } यावत् (0)

#परिभाषय समुच्चयः_क्षेत्रम्_with_default(type, क्षेत्रम्, def)                        \
    कुरु {                                                                \
        यदि (!get_क्षेत्रम्(type, offsetof(संरचना smbios_type_##type,        \
                                      क्षेत्रम्), &p->क्षेत्रम्)) {             \
            p->क्षेत्रम् = def;                                             \
        }                                                               \
    } यावत् (0)

/* Type 0 -- BIOS Information */
#परिभाषय RELEASE_DATE_STR "01/01/2011"
स्थिर शून्य *
smbios_init_type_0(शून्य *start)
{
    संरचना smbios_type_0 *p = (संरचना smbios_type_0 *)start;
    वर्ण *अन्तः = (वर्ण *)start + परिमाण(संरचना smbios_type_0);
    आकार_t आकार;
    पूर्णाङ्क str_सूचकांक = 0;

    p->शीर्षकम्.type = 0;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_0);
    p->शीर्षकम्.handle = 0;

    आरोपय_str_क्षेत्रम्_with_default(0, vendor_str, BUILD_APPNAME);
    आरोपय_str_क्षेत्रम्_with_default(0, bios_संस्करणम्_str, BUILD_APPNAME);

    p->bios_starting_स्थानाङ्कः_segment = 0xe800;

    आरोपय_str_क्षेत्रम्_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_आकार = 0; /* FIXME */

    यदि (!get_क्षेत्रम्(0, offsetof(संरचना smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        स्मृतिं_पूरय(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    यदि (!get_क्षेत्रम्(0, offsetof(संरचना smbios_type_0,
                               bios_characteristics_extension_अष्टकानि),
                   &p->bios_characteristics_extension_अष्टकानि)) {
        p->bios_characteristics_extension_अष्टकानि[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_अष्टकानि[1] = 4;
    }

    समुच्चयः_क्षेत्रम्_with_default(0, system_bios_major_release, 1);
    समुच्चयः_क्षेत्रम्_with_default(0, system_bios_minor_release, 0);
    समुच्चयः_क्षेत्रम्_with_default(0, embedded_controller_major_release, 0xff);
    समुच्चयः_क्षेत्रम्_with_default(0, embedded_controller_minor_release, 0xff);

    *अन्तः = 0;
    अन्तः++;
    यदि (!str_सूचकांक) {
        *अन्तः = 0;
        अन्तः++;
    }

    प्रत्यागम अन्तः;
}

/* Type 1 -- System Information */
स्थिर शून्य *
smbios_init_type_1(शून्य *start)
{
    संरचना smbios_type_1 *p = (संरचना smbios_type_1 *)start;
    वर्ण *अन्तः = (वर्ण *)start + परिमाण(संरचना smbios_type_1);
    आकार_t आकार;
    पूर्णाङ्क str_सूचकांक = 0;

    p->शीर्षकम्.type = 1;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_1);
    p->शीर्षकम्.handle = 0x100;

    आरोपय_str_क्षेत्रम्_with_default(1, manufacturer_str, BUILD_APPNAME);
    आरोपय_str_क्षेत्रम्_with_default(1, गुणनफलम्_name_str, BUILD_APPNAME);
    आरोपय_str_क्षेत्रम्_or_skip(1, संस्करणम्_str);
    आरोपय_str_क्षेत्रम्_or_skip(1, serial_सङ्ख्या_str);

    यदि (!get_क्षेत्रम्(1, offsetof(संरचना smbios_type_1, uuid), &p->uuid))
        स्मृतिं_पूरय(p->uuid, 0, 16);

    समुच्चयः_क्षेत्रम्_with_default(1, wake_ऊर्ध्वम्_type, 0x06); /* power switch */

    आरोपय_str_क्षेत्रम्_or_skip(1, sku_सङ्ख्या_str);
    आरोपय_str_क्षेत्रम्_or_skip(1, family_str);

    *अन्तः = 0;
    अन्तः++;
    यदि (!str_सूचकांक) {
        *अन्तः = 0;
        अन्तः++;
    }

    प्रत्यागम अन्तः;
}

/* Type 3 -- System Enclosure */
स्थिर शून्य *
smbios_init_type_3(शून्य *start)
{
    संरचना smbios_type_3 *p = (संरचना smbios_type_3 *)start;
    वर्ण *अन्तः = (वर्ण *)start + परिमाण(संरचना smbios_type_3);
    आकार_t आकार;
    पूर्णाङ्क str_सूचकांक = 0;

    p->शीर्षकम्.type = 3;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_3);
    p->शीर्षकम्.handle = 0x300;

    आरोपय_str_क्षेत्रम्_with_default(3, manufacturer_str, BUILD_APPNAME);
    समुच्चयः_क्षेत्रम्_with_default(3, type, 0x01); /* other */

    आरोपय_str_क्षेत्रम्_or_skip(3, संस्करणम्_str);
    आरोपय_str_क्षेत्रम्_or_skip(3, serial_सङ्ख्या_str);
    आरोपय_str_क्षेत्रम्_or_skip(3, asset_चिह्नकम्_सङ्ख्या_str);

    समुच्चयः_क्षेत्रम्_with_default(3, boot_ऊर्ध्वम्_state, 0x03); /* safe */
    समुच्चयः_क्षेत्रम्_with_default(3, power_supply_state, 0x03); /* safe */
    समुच्चयः_क्षेत्रम्_with_default(3, thermal_state, 0x03); /* safe */
    समुच्चयः_क्षेत्रम्_with_default(3, security_स्थितिः, 0x02); /* unknown */

    समुच्चयः_क्षेत्रम्_with_default(3, oem_defined, 0);
    समुच्चयः_क्षेत्रम्_with_default(3, height, 0);
    समुच्चयः_क्षेत्रम्_with_default(3, सङ्ख्या_सम्बन्धि_power_cords, 0);
    समुच्चयः_क्षेत्रम्_with_default(3, contained_element_गणनम्, 0);

    *अन्तः = 0;
    अन्तः++;
    यदि (!str_सूचकांक) {
        *अन्तः = 0;
        अन्तः++;
    }

    प्रत्यागम अन्तः;
}

/* Type 4 -- Processor Information */
स्थिर शून्य *
smbios_init_type_4(शून्य *start, अचिह्नित पूर्णाङ्क cpu_सङ्ख्या)
{
    संरचना smbios_type_4 *p = (संरचना smbios_type_4 *)start;
    वर्ण *अन्तः = (वर्ण *)start + परिमाण(संरचना smbios_type_4);
    आकार_t आकार;
    पूर्णाङ्क str_सूचकांक = 0;
    वर्ण name[1024];

    p->शीर्षकम्.type = 4;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_4);
    p->शीर्षकम्.handle = 0x400 + cpu_सङ्ख्या;

    आकार = get_क्षेत्रम्(4, offsetof(संरचना smbios_type_4, socket_designation_str),
                     name);
    यदि (आकार)
        snprintf(name + आकार - 1, परिमाण(name) - आकार, "%2x", cpu_सङ्ख्या);
    अन्यथा
        snprintf(name, परिमाण(name), "CPU%2x", cpu_सङ्ख्या);

    स्मृतिं_प्रतिलिख(अन्तः, name, वाक्यदैर्घ्यम्(name) + 1);
    अन्तः += वाक्यदैर्घ्यम्(name) + 1;
    p->socket_designation_str = ++str_सूचकांक;

    समुच्चयः_क्षेत्रम्_with_default(4, processor_type, 0x03); /* CPU */
    समुच्चयः_क्षेत्रम्_with_default(4, processor_family, 0x01); /* other */

    आरोपय_str_क्षेत्रम्_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    यदि (!get_क्षेत्रम्(4, offsetof(संरचना smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    आरोपय_str_क्षेत्रम्_or_skip(4, processor_संस्करणम्_str);
    समुच्चयः_क्षेत्रम्_with_default(4, voltage, 0);
    समुच्चयः_क्षेत्रम्_with_default(4, external_clock, 0);

    समुच्चयः_क्षेत्रम्_with_default(4, max_जवः, 2000);
    समुच्चयः_क्षेत्रम्_with_default(4, current_जवः, 2000);

    समुच्चयः_क्षेत्रम्_with_default(4, स्थितिः, 0x41); /* socket populated, CPU enabled */
    समुच्चयः_क्षेत्रम्_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *अन्तः = 0;
    अन्तः++;
    यदि (!str_सूचकांक) {
        *अन्तः = 0;
        अन्तः++;
    }

    प्रत्यागम अन्तः;
}

/* Type 16 -- Physical Memory Array */
स्थिर शून्य *
smbios_init_type_16(शून्य *start, u32 स्मृतिः_आकार_mb, पूर्णाङ्क nr_mem_devs)
{
    संरचना smbios_type_16 *p = (संरचना smbios_type_16*)start;

    p->शीर्षकम्.type = 16;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_16);
    p->शीर्षकम्.handle = 0x1000;

    समुच्चयः_क्षेत्रम्_with_default(16, location, 0x01); /* other */
    समुच्चयः_क्षेत्रम्_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    समुच्चयः_क्षेत्रम्_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->अधिकतमम्_क्षमता = स्मृतिः_आकार_mb < 2 << 20 ?
                          स्मृतिः_आकार_mb << 10 : 0x80000000;
    p->स्मृतिः_error_information_handle = 0xfffe; /* none provided */
    p->सङ्ख्या_सम्बन्धि_स्मृतिः_devices = nr_mem_devs;

    start += परिमाण(संरचना smbios_type_16);
    *((u16 *)start) = 0;

    प्रत्यागम start + 2;
}

/* Type 17 -- Memory Device */
स्थिर शून्य *
smbios_init_type_17(शून्य *start, u32 आकार_mb, पूर्णाङ्क instance)
{
    संरचना smbios_type_17 *p = (संरचना smbios_type_17 *)start;
    वर्ण *अन्तः = (वर्ण *)start + परिमाण(संरचना smbios_type_17);
    आकार_t आकार;
    पूर्णाङ्क str_सूचकांक = 0;
    वर्ण name[1024];

    p->शीर्षकम्.type = 17;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_17);
    p->शीर्षकम्.handle = 0x1100 + instance;

    p->physical_स्मृतिः_array_handle = 0x1000;
    समुच्चयः_क्षेत्रम्_with_default(17, योग_विस्तारः, 64);
    समुच्चयः_क्षेत्रम्_with_default(17, दत्त_विस्तारः, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->आकार = आकार_mb;
    समुच्चयः_क्षेत्रम्_with_default(17, form_गुणकः, 0x09); /* DIMM */
    p->device_समुच्चयः = 0;

    आकार = get_क्षेत्रम्(17, offsetof(संरचना smbios_type_17, device_locator_str),
                     name);
    यदि (आकार)
        snprintf(name + आकार - 1, परिमाण(name) - आकार, "%d", instance);
    अन्यथा
        snprintf(name, परिमाण(name), "DIMM %d", instance);

    स्मृतिं_प्रतिलिख(अन्तः, name, वाक्यदैर्घ्यम्(name) + 1);
    अन्तः += वाक्यदैर्घ्यम्(name) + 1;
    p->device_locator_str = ++str_सूचकांक;

    आरोपय_str_क्षेत्रम्_or_skip(17, bank_locator_str);
    समुच्चयः_क्षेत्रम्_with_default(17, स्मृतिः_type, 0x07); /* RAM */
    समुच्चयः_क्षेत्रम्_with_default(17, type_detail, 0);

    *अन्तः = 0;
    अन्तः++;
    यदि (!str_सूचकांक) {
        *अन्तः = 0;
        अन्तः++;
    }

    प्रत्यागम अन्तः;
}

/* Type 19 -- Memory Array Mapped Address */
स्थिर शून्य *
smbios_init_type_19(शून्य *start, u32 start_mb, u32 आकार_mb, पूर्णाङ्क instance)
{
    संरचना smbios_type_19 *p = (संरचना smbios_type_19 *)start;

    p->शीर्षकम्.type = 19;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_19);
    p->शीर्षकम्.handle = 0x1300 + instance;

    p->starting_स्थानाङ्कः = start_mb << 10;
    p->ending_स्थानाङ्कः = p->starting_स्थानाङ्कः + (आकार_mb << 10) - 1;
    p->स्मृतिः_array_handle = 0x1000;
    p->partition_विस्तारः = 1;

    start += परिमाण(संरचना smbios_type_19);
    *((u16 *)start) = 0;

    प्रत्यागम start + 2;
}

/* Type 20 -- Memory Device Mapped Address */
स्थिर शून्य *
smbios_init_type_20(शून्य *start, u32 start_mb, u32 आकार_mb, पूर्णाङ्क instance,
                    पूर्णाङ्क dev_handle, पूर्णाङ्क array_handle)
{
    संरचना smbios_type_20 *p = (संरचना smbios_type_20 *)start;

    p->शीर्षकम्.type = 20;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_20);
    p->शीर्षकम्.handle = 0x1400 + instance;

    p->starting_स्थानाङ्कः = start_mb << 10;
    p->ending_स्थानाङ्कः = p->starting_स्थानाङ्कः + (आकार_mb << 10) - 1;
    p->स्मृतिः_device_handle = 0x1100 + dev_handle;
    p->स्मृतिः_array_mapped_स्थानाङ्कः_handle = 0x1300 + array_handle;
    p->partition_row_position = 1;
    p->interleave_position = 0;
    p->interleaved_दत्त_गभीरता = 0;

    start += परिमाण(संरचना smbios_type_20);

    *((u16 *)start) = 0;
    प्रत्यागम start+2;
}

/* Type 32 -- System Boot Information */
स्थिर शून्य *
smbios_init_type_32(शून्य *start)
{
    संरचना smbios_type_32 *p = (संरचना smbios_type_32 *)start;

    p->शीर्षकम्.type = 32;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_32);
    p->शीर्षकम्.handle = 0x2000;
    स्मृतिं_पूरय(p->reserved, 0, 6);
    समुच्चयः_क्षेत्रम्_with_default(32, boot_स्थितिः, 0); /* no errors detected */

    start += परिमाण(संरचना smbios_type_32);
    *((u16 *)start) = 0;

    प्रत्यागम start+2;
}

/* Type 127 -- End of Table */
स्थिर शून्य *
smbios_init_type_127(शून्य *start)
{
    संरचना smbios_type_127 *p = (संरचना smbios_type_127 *)start;

    p->शीर्षकम्.type = 127;
    p->शीर्षकम्.दीर्घता = परिमाण(संरचना smbios_type_127);
    p->शीर्षकम्.handle = 0x7f00;

    start += परिमाण(संरचना smbios_type_127);
    *((u16 *)start) = 0;

    प्रत्यागम start + 2;
}

#परिभाषय TEMPSMBIOSSIZE (32 * 1024)

शून्य
smbios_legacy_सज्जता(शून्य)
{
    यदि (! CONFIG_SMBIOS)
        प्रत्यागम;

    dprintf(3, "init SMBIOS tables\n");

    वर्ण *start = malloc_tmphigh(TEMPSMBIOSSIZE);
    यदि (! start) {
        warn_noalloc();
        प्रत्यागम;
    }
    स्मृतिं_पूरय(start, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_आकार = 0;
    वर्ण *q, *p = start;
    वर्ण *अन्तः = start + TEMPSMBIOSSIZE - परिमाण(संरचना smbios_type_127);

#परिभाषय सङ्कलय_struct(type, तर्काः...)                                       \
    कुरु {                                                                \
        यदि (!get_external(type, &p, &nr_structs, &max_struct_आकार, अन्तः)) { \
            q = smbios_init_type_##type(तर्काः);                          \
            nr_structs++;                                               \
            यदि ((q - p) > max_struct_आकार)                              \
                max_struct_आकार = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } यावत् (0)

    सङ्कलय_struct(0, p);
    सङ्कलय_struct(1, p);
    सङ्कलय_struct(3, p);

    पूर्णाङ्क cpu_अङ्कः;
    प्रति (cpu_अङ्कः = 1; cpu_अङ्कः <= Max_गणनम्_CPUs; cpu_अङ्कः++)
        सङ्कलय_struct(4, p, cpu_अङ्कः);

    पूर्णाङ्क ram_mb = (Ram_आकार + Ram_आकार_Over4_G) >> 20;
    पूर्णाङ्क nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    सङ्कलय_struct(16, p, ram_mb, nr_mem_devs);

    पूर्णाङ्क i, j;
    प्रति (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        सङ्कलय_struct(17, p, dev_mb, i);
    }

    सङ्कलय_struct(19, p, 0, Ram_आकार >> 20, 0);
    यदि (Ram_आकार_Over4_G)
        सङ्कलय_struct(19, p, 4096, Ram_आकार_Over4_G >> 20, 1);

    सङ्कलय_struct(20, p, 0, Ram_आकार >> 20, 0, 0, 0);
    यदि (Ram_आकार_Over4_G) {
        u32 start_mb = 4096;
        प्रति (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            यदि (i == 0)
                dev_mb -= Ram_आकार >> 20;

            सङ्कलय_struct(20, p, start_mb, dev_mb, j, i, 1);
            start_mb += dev_mb;
        }
    }

    सङ्कलय_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    प्रति (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_आकार, अन्तः);
    सङ्कलय_struct(127, p);

#अपरिभाषय सङ्कलय_struct

    smbios_21_प्रविष्टिः_point_सज्जता(max_struct_आकार, p - start, start, nr_structs);
    मुक्त(start);
}
