// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // RamSize
#समावेश "romfile.h" // romfile_findprefix
#समावेश "std/smbios.h" // struct smbios_21_entry_point
#समावेश "string.h" // memset
#समावेश "util.h" // MaxCountCPUs
#समावेश "x86.h" // cpuid

अटल व्योम
smbios_21_entry_point_setup(u16 max_structure_आकार,
                         u16 structure_table_लंबाई,
                         व्योम *structure_table_address,
                         u16 संख्या_of_structures)
{
    व्योम *finaltable;
    अगर (structure_table_लंबाई <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_table_लंबाई);
    अन्यथा
        finaltable = malloc_ऊपरी(structure_table_लंबाई);
    अगर (!finaltable) {
        warn_noalloc();
        वापस;
    }
    स_नकल(finaltable, structure_table_address, structure_table_लंबाई);

    काष्ठा smbios_21_entry_point ep;
    स_रखो(&ep, 0, माप(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.लंबाई = 0x1f;
    ep.smbios_major_version = 2;
    ep.smbios_minor_version = 4;
    ep.max_structure_आकार = max_structure_आकार;
    स_नकल(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_table_लंबाई = structure_table_लंबाई;
    ep.structure_table_address = (u32)finaltable;
    ep.संख्या_of_structures = संख्या_of_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((व्योम*)&ep + 0x10, ep.लंबाई - 0x10);

    copy_smbios_21(&ep);
}

अटल पूर्णांक
get_field(पूर्णांक type, पूर्णांक offset, व्योम *dest)
{
    अक्षर name[128];
    snprintf(name, माप(name), "smbios/field%d-%d", type, offset);
    काष्ठा romfile_s *file = romfile_find(name);
    अगर (!file)
        वापस 0;
    file->copy(file, dest, file->आकार);
    वापस file->आकार;
}

अटल पूर्णांक
get_external(पूर्णांक type, अक्षर **p, अचिन्हित *nr_structs,
             अचिन्हित *max_struct_आकार, अक्षर *end)
{
    अटल u64 used_bitmap[4] = { 0 };
    अक्षर *start = *p;

    /* Check if we've already reported these tables */
    अगर (used_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        वापस 1;

    /* Don't introduce spurious end markers */
    अगर (type == 127)
        वापस 0;

    अक्षर prefix[128];
    snprintf(prefix, माप(prefix), "smbios/table%d-", type);
    काष्ठा romfile_s *file = शून्य;
    क्रम (;;) {
        file = romfile_findprefix(prefix, file);
        अगर (!file)
            अवरोध;

        अगर (end - *p < file->आकार) {
            warn_noalloc();
            अवरोध;
        }

        काष्ठा smbios_structure_header *header = (व्योम*)*p;
        file->copy(file, header, file->आकार);
        *p += file->आकार;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        अगर (header->लंबाई >= file->आकार) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        अगर (*p - (अक्षर*)header > *max_struct_आकार)
            *max_struct_आकार = *p - (अक्षर*)header;
    }

    अगर (start == *p)
        वापस 0;

    /* Mark that we've reported on this type */
    used_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    वापस 1;
}

#घोषणा load_str_field_with_default(type, field, def)                   \
    करो {                                                                \
        आकार = get_field(type, दुरत्व(काष्ठा smbios_type_##type,      \
                                        field), end);                   \
        अगर (आकार == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->field = 0;                                               \
        } अन्यथा अगर (आकार > 1) {                                          \
            end += आकार;                                                \
            p->field = ++str_सूचकांक;                                     \
        } अन्यथा {                                                        \
            स_नकल(end, def, माप(def));                              \
            end += माप(def);                                         \
            p->field = ++str_सूचकांक;                                     \
        }                                                               \
    } जबतक (0)

#घोषणा load_str_field_or_skip(type, field)                             \
    करो {                                                                \
        आकार = get_field(type, दुरत्व(काष्ठा smbios_type_##type,      \
                                        field), end);                   \
        अगर (आकार > 1) {                                                 \
            end += आकार;                                                \
            p->field = ++str_सूचकांक;                                     \
        } अन्यथा {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } जबतक (0)

#घोषणा set_field_with_default(type, field, def)                        \
    करो {                                                                \
        अगर (!get_field(type, दुरत्व(काष्ठा smbios_type_##type,        \
                                      field), &p->field)) {             \
            p->field = def;                                             \
        }                                                               \
    } जबतक (0)

/* Type 0 -- BIOS Information */
#घोषणा RELEASE_DATE_STR "01/01/2011"
अटल व्योम *
smbios_init_type_0(व्योम *start)
{
    काष्ठा smbios_type_0 *p = (काष्ठा smbios_type_0 *)start;
    अक्षर *end = (अक्षर *)start + माप(काष्ठा smbios_type_0);
    माप_प्रकार आकार;
    पूर्णांक str_सूचकांक = 0;

    p->header.type = 0;
    p->header.लंबाई = माप(काष्ठा smbios_type_0);
    p->header.handle = 0;

    load_str_field_with_default(0, vendor_str, BUILD_APPNAME);
    load_str_field_with_default(0, bios_version_str, BUILD_APPNAME);

    p->bios_starting_address_segment = 0xe800;

    load_str_field_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_आकार = 0; /* FIXME */

    अगर (!get_field(0, दुरत्व(काष्ठा smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        स_रखो(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    अगर (!get_field(0, दुरत्व(काष्ठा smbios_type_0,
                               bios_characteristics_extension_bytes),
                   &p->bios_characteristics_extension_bytes)) {
        p->bios_characteristics_extension_bytes[0] = 0;
        /* Enable targeted content distribution. Needed for SVVP */
        p->bios_characteristics_extension_bytes[1] = 4;
    }

    set_field_with_default(0, system_bios_major_release, 1);
    set_field_with_default(0, system_bios_minor_release, 0);
    set_field_with_default(0, embedded_controller_major_release, 0xff);
    set_field_with_default(0, embedded_controller_minor_release, 0xff);

    *end = 0;
    end++;
    अगर (!str_सूचकांक) {
        *end = 0;
        end++;
    }

    वापस end;
}

/* Type 1 -- System Information */
अटल व्योम *
smbios_init_type_1(व्योम *start)
{
    काष्ठा smbios_type_1 *p = (काष्ठा smbios_type_1 *)start;
    अक्षर *end = (अक्षर *)start + माप(काष्ठा smbios_type_1);
    माप_प्रकार आकार;
    पूर्णांक str_सूचकांक = 0;

    p->header.type = 1;
    p->header.लंबाई = माप(काष्ठा smbios_type_1);
    p->header.handle = 0x100;

    load_str_field_with_default(1, manufacturer_str, BUILD_APPNAME);
    load_str_field_with_default(1, product_name_str, BUILD_APPNAME);
    load_str_field_or_skip(1, version_str);
    load_str_field_or_skip(1, serial_संख्या_str);

    अगर (!get_field(1, दुरत्व(काष्ठा smbios_type_1, uuid), &p->uuid))
        स_रखो(p->uuid, 0, 16);

    set_field_with_default(1, wake_up_type, 0x06); /* power switch */

    load_str_field_or_skip(1, sku_संख्या_str);
    load_str_field_or_skip(1, family_str);

    *end = 0;
    end++;
    अगर (!str_सूचकांक) {
        *end = 0;
        end++;
    }

    वापस end;
}

/* Type 3 -- System Enclosure */
अटल व्योम *
smbios_init_type_3(व्योम *start)
{
    काष्ठा smbios_type_3 *p = (काष्ठा smbios_type_3 *)start;
    अक्षर *end = (अक्षर *)start + माप(काष्ठा smbios_type_3);
    माप_प्रकार आकार;
    पूर्णांक str_सूचकांक = 0;

    p->header.type = 3;
    p->header.लंबाई = माप(काष्ठा smbios_type_3);
    p->header.handle = 0x300;

    load_str_field_with_default(3, manufacturer_str, BUILD_APPNAME);
    set_field_with_default(3, type, 0x01); /* other */

    load_str_field_or_skip(3, version_str);
    load_str_field_or_skip(3, serial_संख्या_str);
    load_str_field_or_skip(3, asset_tag_संख्या_str);

    set_field_with_default(3, boot_up_state, 0x03); /* safe */
    set_field_with_default(3, power_supply_state, 0x03); /* safe */
    set_field_with_default(3, thermal_state, 0x03); /* safe */
    set_field_with_default(3, security_status, 0x02); /* unknown */

    set_field_with_default(3, oem_defined, 0);
    set_field_with_default(3, height, 0);
    set_field_with_default(3, संख्या_of_power_cords, 0);
    set_field_with_default(3, contained_element_गिनती, 0);

    *end = 0;
    end++;
    अगर (!str_सूचकांक) {
        *end = 0;
        end++;
    }

    वापस end;
}

/* Type 4 -- Processor Information */
अटल व्योम *
smbios_init_type_4(व्योम *start, अचिन्हित पूर्णांक cpu_संख्या)
{
    काष्ठा smbios_type_4 *p = (काष्ठा smbios_type_4 *)start;
    अक्षर *end = (अक्षर *)start + माप(काष्ठा smbios_type_4);
    माप_प्रकार आकार;
    पूर्णांक str_सूचकांक = 0;
    अक्षर name[1024];

    p->header.type = 4;
    p->header.लंबाई = माप(काष्ठा smbios_type_4);
    p->header.handle = 0x400 + cpu_संख्या;

    आकार = get_field(4, दुरत्व(काष्ठा smbios_type_4, socket_designation_str),
                     name);
    अगर (आकार)
        snprintf(name + आकार - 1, माप(name) - आकार, "%2x", cpu_संख्या);
    अन्यथा
        snprintf(name, माप(name), "CPU%2x", cpu_संख्या);

    स_नकल(end, name, म_माप(name) + 1);
    end += म_माप(name) + 1;
    p->socket_designation_str = ++str_सूचकांक;

    set_field_with_default(4, processor_type, 0x03); /* CPU */
    set_field_with_default(4, processor_family, 0x01); /* other */

    load_str_field_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    अगर (!get_field(4, दुरत्व(काष्ठा smbios_type_4, processor_id)
                   , p->processor_id)) {
        u32 cpuid_signature, ebx, ecx, cpuid_features;
        cpuid(1, &cpuid_signature, &ebx, &ecx, &cpuid_features);
        p->processor_id[0] = cpuid_signature;
        p->processor_id[1] = cpuid_features;
    }

    load_str_field_or_skip(4, processor_version_str);
    set_field_with_default(4, voltage, 0);
    set_field_with_default(4, external_clock, 0);

    set_field_with_default(4, max_speed, 2000);
    set_field_with_default(4, current_speed, 2000);

    set_field_with_default(4, status, 0x41); /* socket populated, CPU enabled */
    set_field_with_default(4, processor_upgrade, 0x01); /* other */

    /* cache information structure not provided */
    p->l1_cache_handle =  0xffff;
    p->l2_cache_handle =  0xffff;
    p->l3_cache_handle =  0xffff;

    *end = 0;
    end++;
    अगर (!str_सूचकांक) {
        *end = 0;
        end++;
    }

    वापस end;
}

/* Type 16 -- Physical Memory Array */
अटल व्योम *
smbios_init_type_16(व्योम *start, u32 memory_आकार_mb, पूर्णांक nr_mem_devs)
{
    काष्ठा smbios_type_16 *p = (काष्ठा smbios_type_16*)start;

    p->header.type = 16;
    p->header.लंबाई = माप(काष्ठा smbios_type_16);
    p->header.handle = 0x1000;

    set_field_with_default(16, location, 0x01); /* other */
    set_field_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    set_field_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->maximum_capacity = memory_आकार_mb < 2 << 20 ?
                          memory_आकार_mb << 10 : 0x80000000;
    p->memory_error_information_handle = 0xfffe; /* none provided */
    p->संख्या_of_memory_devices = nr_mem_devs;

    start += माप(काष्ठा smbios_type_16);
    *((u16 *)start) = 0;

    वापस start + 2;
}

/* Type 17 -- Memory Device */
अटल व्योम *
smbios_init_type_17(व्योम *start, u32 आकार_mb, पूर्णांक instance)
{
    काष्ठा smbios_type_17 *p = (काष्ठा smbios_type_17 *)start;
    अक्षर *end = (अक्षर *)start + माप(काष्ठा smbios_type_17);
    माप_प्रकार आकार;
    पूर्णांक str_सूचकांक = 0;
    अक्षर name[1024];

    p->header.type = 17;
    p->header.लंबाई = माप(काष्ठा smbios_type_17);
    p->header.handle = 0x1100 + instance;

    p->physical_memory_array_handle = 0x1000;
    set_field_with_default(17, योग_width, 64);
    set_field_with_default(17, तथ्य_width, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->आकार = आकार_mb;
    set_field_with_default(17, form_factor, 0x09); /* DIMM */
    p->device_set = 0;

    आकार = get_field(17, दुरत्व(काष्ठा smbios_type_17, device_locator_str),
                     name);
    अगर (आकार)
        snprintf(name + आकार - 1, माप(name) - आकार, "%d", instance);
    अन्यथा
        snprintf(name, माप(name), "DIMM %d", instance);

    स_नकल(end, name, म_माप(name) + 1);
    end += म_माप(name) + 1;
    p->device_locator_str = ++str_सूचकांक;

    load_str_field_or_skip(17, bank_locator_str);
    set_field_with_default(17, memory_type, 0x07); /* RAM */
    set_field_with_default(17, type_detail, 0);

    *end = 0;
    end++;
    अगर (!str_सूचकांक) {
        *end = 0;
        end++;
    }

    वापस end;
}

/* Type 19 -- Memory Array Mapped Address */
अटल व्योम *
smbios_init_type_19(व्योम *start, u32 start_mb, u32 आकार_mb, पूर्णांक instance)
{
    काष्ठा smbios_type_19 *p = (काष्ठा smbios_type_19 *)start;

    p->header.type = 19;
    p->header.लंबाई = माप(काष्ठा smbios_type_19);
    p->header.handle = 0x1300 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (आकार_mb << 10) - 1;
    p->memory_array_handle = 0x1000;
    p->partition_width = 1;

    start += माप(काष्ठा smbios_type_19);
    *((u16 *)start) = 0;

    वापस start + 2;
}

/* Type 20 -- Memory Device Mapped Address */
अटल व्योम *
smbios_init_type_20(व्योम *start, u32 start_mb, u32 आकार_mb, पूर्णांक instance,
                    पूर्णांक dev_handle, पूर्णांक array_handle)
{
    काष्ठा smbios_type_20 *p = (काष्ठा smbios_type_20 *)start;

    p->header.type = 20;
    p->header.लंबाई = माप(काष्ठा smbios_type_20);
    p->header.handle = 0x1400 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (आकार_mb << 10) - 1;
    p->memory_device_handle = 0x1100 + dev_handle;
    p->memory_array_mapped_address_handle = 0x1300 + array_handle;
    p->partition_row_position = 1;
    p->interleave_position = 0;
    p->interleaved_तथ्य_depth = 0;

    start += माप(काष्ठा smbios_type_20);

    *((u16 *)start) = 0;
    वापस start+2;
}

/* Type 32 -- System Boot Information */
अटल व्योम *
smbios_init_type_32(व्योम *start)
{
    काष्ठा smbios_type_32 *p = (काष्ठा smbios_type_32 *)start;

    p->header.type = 32;
    p->header.लंबाई = माप(काष्ठा smbios_type_32);
    p->header.handle = 0x2000;
    स_रखो(p->reserved, 0, 6);
    set_field_with_default(32, boot_status, 0); /* no errors detected */

    start += माप(काष्ठा smbios_type_32);
    *((u16 *)start) = 0;

    वापस start+2;
}

/* Type 127 -- End of Table */
अटल व्योम *
smbios_init_type_127(व्योम *start)
{
    काष्ठा smbios_type_127 *p = (काष्ठा smbios_type_127 *)start;

    p->header.type = 127;
    p->header.लंबाई = माप(काष्ठा smbios_type_127);
    p->header.handle = 0x7f00;

    start += माप(काष्ठा smbios_type_127);
    *((u16 *)start) = 0;

    वापस start + 2;
}

#घोषणा TEMPSMBIOSSIZE (32 * 1024)

व्योम
smbios_legacy_setup(व्योम)
{
    अगर (! CONFIG_SMBIOS)
        वापस;

    dprintf(3, "init SMBIOS tables\n");

    अक्षर *start = malloc_tmphigh(TEMPSMBIOSSIZE);
    अगर (! start) {
        warn_noalloc();
        वापस;
    }
    स_रखो(start, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_आकार = 0;
    अक्षर *q, *p = start;
    अक्षर *end = start + TEMPSMBIOSSIZE - माप(काष्ठा smbios_type_127);

#घोषणा add_struct(type, args...)                                       \
    करो {                                                                \
        अगर (!get_external(type, &p, &nr_structs, &max_struct_आकार, end)) { \
            q = smbios_init_type_##type(args);                          \
            nr_structs++;                                               \
            अगर ((q - p) > max_struct_आकार)                              \
                max_struct_आकार = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } जबतक (0)

    add_struct(0, p);
    add_struct(1, p);
    add_struct(3, p);

    पूर्णांक cpu_अंक;
    क्रम (cpu_अंक = 1; cpu_अंक <= Max_गिनती_CPUs; cpu_अंक++)
        add_struct(4, p, cpu_अंक);

    पूर्णांक ram_mb = (Ram_आकार + Ram_आकार_Over4_G) >> 20;
    पूर्णांक nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    add_struct(16, p, ram_mb, nr_mem_devs);

    पूर्णांक i, j;
    क्रम (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        add_struct(17, p, dev_mb, i);
    }

    add_struct(19, p, 0, Ram_आकार >> 20, 0);
    अगर (Ram_आकार_Over4_G)
        add_struct(19, p, 4096, Ram_आकार_Over4_G >> 20, 1);

    add_struct(20, p, 0, Ram_आकार >> 20, 0, 0, 0);
    अगर (Ram_आकार_Over4_G) {
        u32 start_mb = 4096;
        क्रम (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            अगर (i == 0)
                dev_mb -= Ram_आकार >> 20;

            add_struct(20, p, start_mb, dev_mb, j, i, 1);
            start_mb += dev_mb;
        }
    }

    add_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    क्रम (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_आकार, end);
    add_struct(127, p);

#अघोषित add_struct

    smbios_21_entry_point_setup(max_struct_आकार, p - start, start, nr_structs);
    मुक्त(start);
}
