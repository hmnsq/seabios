// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "paravirt.h" // RamSize
#അന്തര്ഭാവ "romfile.h" // romfile_findprefix
#അന്തര്ഭാവ "std/smbios.h" // struct smbios_21_entry_point
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // MaxCountCPUs
#അന്തര്ഭാവ "x86.h" // cpuid

സ്ഥിതം ശൂന്യം
smbios_21_entry_point_setup(u16 max_structure_size,
                         u16 structure_table_length,
                         ശൂന്യം *structure_table_address,
                         u16 സംഖ്യ_of_structures)
{
    ശൂന്യം *finaltable;
    എങ്കിൽ (structure_table_length <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_table_length);
    അല്ലെങ്കിൽ
        finaltable = malloc_മുകളിലെ(structure_table_length);
    എങ്കിൽ (!finaltable) {
        warn_noalloc();
        തിരികെ;
    }
    memcpy(finaltable, structure_table_address, structure_table_length);

    ഘടന smbios_21_entry_point ep;
    memset(&ep, 0, വലുപ്പം(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.length = 0x1f;
    ep.smbios_major_version = 2;
    ep.smbios_minor_version = 4;
    ep.max_structure_size = max_structure_size;
    memcpy(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_table_length = structure_table_length;
    ep.structure_table_address = (u32)finaltable;
    ep.സംഖ്യ_of_structures = സംഖ്യ_of_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((ശൂന്യം*)&ep + 0x10, ep.length - 0x10);

    copy_smbios_21(&ep);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
get_field(പൂർണ്ണസംഖ്യ type, പൂർണ്ണസംഖ്യ offset, ശൂന്യം *dest)
{
    അക്ഷരം name[128];
    snprintf(name, വലുപ്പം(name), "smbios/field%d-%d", type, offset);
    ഘടന romfile_s *file = romfile_find(name);
    എങ്കിൽ (!file)
        തിരികെ 0;
    file->copy(file, dest, file->size);
    തിരികെ file->size;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
get_external(പൂർണ്ണസംഖ്യ type, അക്ഷരം **p, unsigned *nr_structs,
             unsigned *max_struct_size, അക്ഷരം *end)
{
    സ്ഥിതം u64 used_bitmap[4] = { 0 };
    അക്ഷരം *start = *p;

    /* Check if we've already reported these tables */
    എങ്കിൽ (used_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        തിരികെ 1;

    /* Don't introduce spurious end markers */
    എങ്കിൽ (type == 127)
        തിരികെ 0;

    അക്ഷരം prefix[128];
    snprintf(prefix, വലുപ്പം(prefix), "smbios/table%d-", type);
    ഘടന romfile_s *file = ശൂന്യ;
    വേണ്ടി (;;) {
        file = romfile_findprefix(prefix, file);
        എങ്കിൽ (!file)
            നിർത്തുക;

        എങ്കിൽ (end - *p < file->size) {
            warn_noalloc();
            നിർത്തുക;
        }

        ഘടന smbios_structure_header *header = (ശൂന്യം*)*p;
        file->copy(file, header, file->size);
        *p += file->size;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        എങ്കിൽ (header->length >= file->size) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        എങ്കിൽ (*p - (അക്ഷരം*)header > *max_struct_size)
            *max_struct_size = *p - (അക്ഷരം*)header;
    }

    എങ്കിൽ (start == *p)
        തിരികെ 0;

    /* Mark that we've reported on this type */
    used_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    തിരികെ 1;
}

#define load_str_field_with_default(type, field, def)                   \
    ചെയ്യുക {                                                                \
        size = get_field(type, offsetof(ഘടന smbios_type_##type,      \
                                        field), end);                   \
        എങ്കിൽ (size == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->field = 0;                                               \
        } അല്ലെങ്കിൽ എങ്കിൽ (size > 1) {                                          \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } അല്ലെങ്കിൽ {                                                        \
            memcpy(end, def, വലുപ്പം(def));                              \
            end += വലുപ്പം(def);                                         \
            p->field = ++str_index;                                     \
        }                                                               \
    } ആകുമ്പോൾ (0)

#define load_str_field_or_skip(type, field)                             \
    ചെയ്യുക {                                                                \
        size = get_field(type, offsetof(ഘടന smbios_type_##type,      \
                                        field), end);                   \
        എങ്കിൽ (size > 1) {                                                 \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } അല്ലെങ്കിൽ {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } ആകുമ്പോൾ (0)

#define set_field_with_default(type, field, def)                        \
    ചെയ്യുക {                                                                \
        എങ്കിൽ (!get_field(type, offsetof(ഘടന smbios_type_##type,        \
                                      field), &p->field)) {             \
            p->field = def;                                             \
        }                                                               \
    } ആകുമ്പോൾ (0)

/* Type 0 -- BIOS Information */
#define RELEASE_DATE_STR "01/01/2011"
സ്ഥിതം ശൂന്യം *
smbios_init_type_0(ശൂന്യം *start)
{
    ഘടന smbios_type_0 *p = (ഘടന smbios_type_0 *)start;
    അക്ഷരം *end = (അക്ഷരം *)start + വലുപ്പം(ഘടന smbios_type_0);
    size_t size;
    പൂർണ്ണസംഖ്യ str_index = 0;

    p->header.type = 0;
    p->header.length = വലുപ്പം(ഘടന smbios_type_0);
    p->header.handle = 0;

    load_str_field_with_default(0, vendor_str, BUILD_APPNAME);
    load_str_field_with_default(0, bios_version_str, BUILD_APPNAME);

    p->bios_starting_address_segment = 0xe800;

    load_str_field_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_size = 0; /* FIXME */

    എങ്കിൽ (!get_field(0, offsetof(ഘടന smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    എങ്കിൽ (!get_field(0, offsetof(ഘടന smbios_type_0,
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
    എങ്കിൽ (!str_index) {
        *end = 0;
        end++;
    }

    തിരികെ end;
}

/* Type 1 -- System Information */
സ്ഥിതം ശൂന്യം *
smbios_init_type_1(ശൂന്യം *start)
{
    ഘടന smbios_type_1 *p = (ഘടന smbios_type_1 *)start;
    അക്ഷരം *end = (അക്ഷരം *)start + വലുപ്പം(ഘടന smbios_type_1);
    size_t size;
    പൂർണ്ണസംഖ്യ str_index = 0;

    p->header.type = 1;
    p->header.length = വലുപ്പം(ഘടന smbios_type_1);
    p->header.handle = 0x100;

    load_str_field_with_default(1, manufacturer_str, BUILD_APPNAME);
    load_str_field_with_default(1, product_name_str, BUILD_APPNAME);
    load_str_field_or_skip(1, version_str);
    load_str_field_or_skip(1, serial_സംഖ്യ_str);

    എങ്കിൽ (!get_field(1, offsetof(ഘടന smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    set_field_with_default(1, wake_up_type, 0x06); /* power switch */

    load_str_field_or_skip(1, sku_സംഖ്യ_str);
    load_str_field_or_skip(1, family_str);

    *end = 0;
    end++;
    എങ്കിൽ (!str_index) {
        *end = 0;
        end++;
    }

    തിരികെ end;
}

/* Type 3 -- System Enclosure */
സ്ഥിതം ശൂന്യം *
smbios_init_type_3(ശൂന്യം *start)
{
    ഘടന smbios_type_3 *p = (ഘടന smbios_type_3 *)start;
    അക്ഷരം *end = (അക്ഷരം *)start + വലുപ്പം(ഘടന smbios_type_3);
    size_t size;
    പൂർണ്ണസംഖ്യ str_index = 0;

    p->header.type = 3;
    p->header.length = വലുപ്പം(ഘടന smbios_type_3);
    p->header.handle = 0x300;

    load_str_field_with_default(3, manufacturer_str, BUILD_APPNAME);
    set_field_with_default(3, type, 0x01); /* other */

    load_str_field_or_skip(3, version_str);
    load_str_field_or_skip(3, serial_സംഖ്യ_str);
    load_str_field_or_skip(3, asset_tag_സംഖ്യ_str);

    set_field_with_default(3, boot_up_state, 0x03); /* safe */
    set_field_with_default(3, power_supply_state, 0x03); /* safe */
    set_field_with_default(3, thermal_state, 0x03); /* safe */
    set_field_with_default(3, security_status, 0x02); /* unknown */

    set_field_with_default(3, oem_defined, 0);
    set_field_with_default(3, height, 0);
    set_field_with_default(3, സംഖ്യ_of_power_cords, 0);
    set_field_with_default(3, contained_element_എണ്ണം, 0);

    *end = 0;
    end++;
    എങ്കിൽ (!str_index) {
        *end = 0;
        end++;
    }

    തിരികെ end;
}

/* Type 4 -- Processor Information */
സ്ഥിതം ശൂന്യം *
smbios_init_type_4(ശൂന്യം *start, unsigned പൂർണ്ണസംഖ്യ cpu_സംഖ്യ)
{
    ഘടന smbios_type_4 *p = (ഘടന smbios_type_4 *)start;
    അക്ഷരം *end = (അക്ഷരം *)start + വലുപ്പം(ഘടന smbios_type_4);
    size_t size;
    പൂർണ്ണസംഖ്യ str_index = 0;
    അക്ഷരം name[1024];

    p->header.type = 4;
    p->header.length = വലുപ്പം(ഘടന smbios_type_4);
    p->header.handle = 0x400 + cpu_സംഖ്യ;

    size = get_field(4, offsetof(ഘടന smbios_type_4, socket_designation_str),
                     name);
    എങ്കിൽ (size)
        snprintf(name + size - 1, വലുപ്പം(name) - size, "%2x", cpu_സംഖ്യ);
    അല്ലെങ്കിൽ
        snprintf(name, വലുപ്പം(name), "CPU%2x", cpu_സംഖ്യ);

    memcpy(end, name, strlen(name) + 1);
    end += strlen(name) + 1;
    p->socket_designation_str = ++str_index;

    set_field_with_default(4, processor_type, 0x03); /* CPU */
    set_field_with_default(4, processor_family, 0x01); /* other */

    load_str_field_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    എങ്കിൽ (!get_field(4, offsetof(ഘടന smbios_type_4, processor_id)
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
    എങ്കിൽ (!str_index) {
        *end = 0;
        end++;
    }

    തിരികെ end;
}

/* Type 16 -- Physical Memory Array */
സ്ഥിതം ശൂന്യം *
smbios_init_type_16(ശൂന്യം *start, u32 memory_size_mb, പൂർണ്ണസംഖ്യ nr_mem_devs)
{
    ഘടന smbios_type_16 *p = (ഘടന smbios_type_16*)start;

    p->header.type = 16;
    p->header.length = വലുപ്പം(ഘടന smbios_type_16);
    p->header.handle = 0x1000;

    set_field_with_default(16, location, 0x01); /* other */
    set_field_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    set_field_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->maximum_capacity = memory_size_mb < 2 << 20 ?
                          memory_size_mb << 10 : 0x80000000;
    p->memory_error_information_handle = 0xfffe; /* none provided */
    p->സംഖ്യ_of_memory_devices = nr_mem_devs;

    start += വലുപ്പം(ഘടന smbios_type_16);
    *((u16 *)start) = 0;

    തിരികെ start + 2;
}

/* Type 17 -- Memory Device */
സ്ഥിതം ശൂന്യം *
smbios_init_type_17(ശൂന്യം *start, u32 size_mb, പൂർണ്ണസംഖ്യ instance)
{
    ഘടന smbios_type_17 *p = (ഘടന smbios_type_17 *)start;
    അക്ഷരം *end = (അക്ഷരം *)start + വലുപ്പം(ഘടന smbios_type_17);
    size_t size;
    പൂർണ്ണസംഖ്യ str_index = 0;
    അക്ഷരം name[1024];

    p->header.type = 17;
    p->header.length = വലുപ്പം(ഘടന smbios_type_17);
    p->header.handle = 0x1100 + instance;

    p->physical_memory_array_handle = 0x1000;
    set_field_with_default(17, total_width, 64);
    set_field_with_default(17, data_width, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->size = size_mb;
    set_field_with_default(17, form_factor, 0x09); /* DIMM */
    p->device_set = 0;

    size = get_field(17, offsetof(ഘടന smbios_type_17, device_locator_str),
                     name);
    എങ്കിൽ (size)
        snprintf(name + size - 1, വലുപ്പം(name) - size, "%d", instance);
    അല്ലെങ്കിൽ
        snprintf(name, വലുപ്പം(name), "DIMM %d", instance);

    memcpy(end, name, strlen(name) + 1);
    end += strlen(name) + 1;
    p->device_locator_str = ++str_index;

    load_str_field_or_skip(17, bank_locator_str);
    set_field_with_default(17, memory_type, 0x07); /* RAM */
    set_field_with_default(17, type_detail, 0);

    *end = 0;
    end++;
    എങ്കിൽ (!str_index) {
        *end = 0;
        end++;
    }

    തിരികെ end;
}

/* Type 19 -- Memory Array Mapped Address */
സ്ഥിതം ശൂന്യം *
smbios_init_type_19(ശൂന്യം *start, u32 start_mb, u32 size_mb, പൂർണ്ണസംഖ്യ instance)
{
    ഘടന smbios_type_19 *p = (ഘടന smbios_type_19 *)start;

    p->header.type = 19;
    p->header.length = വലുപ്പം(ഘടന smbios_type_19);
    p->header.handle = 0x1300 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (size_mb << 10) - 1;
    p->memory_array_handle = 0x1000;
    p->partition_width = 1;

    start += വലുപ്പം(ഘടന smbios_type_19);
    *((u16 *)start) = 0;

    തിരികെ start + 2;
}

/* Type 20 -- Memory Device Mapped Address */
സ്ഥിതം ശൂന്യം *
smbios_init_type_20(ശൂന്യം *start, u32 start_mb, u32 size_mb, പൂർണ്ണസംഖ്യ instance,
                    പൂർണ്ണസംഖ്യ dev_handle, പൂർണ്ണസംഖ്യ array_handle)
{
    ഘടന smbios_type_20 *p = (ഘടന smbios_type_20 *)start;

    p->header.type = 20;
    p->header.length = വലുപ്പം(ഘടന smbios_type_20);
    p->header.handle = 0x1400 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (size_mb << 10) - 1;
    p->memory_device_handle = 0x1100 + dev_handle;
    p->memory_array_mapped_address_handle = 0x1300 + array_handle;
    p->partition_row_position = 1;
    p->interleave_position = 0;
    p->interleaved_data_depth = 0;

    start += വലുപ്പം(ഘടന smbios_type_20);

    *((u16 *)start) = 0;
    തിരികെ start+2;
}

/* Type 32 -- System Boot Information */
സ്ഥിതം ശൂന്യം *
smbios_init_type_32(ശൂന്യം *start)
{
    ഘടന smbios_type_32 *p = (ഘടന smbios_type_32 *)start;

    p->header.type = 32;
    p->header.length = വലുപ്പം(ഘടന smbios_type_32);
    p->header.handle = 0x2000;
    memset(p->reserved, 0, 6);
    set_field_with_default(32, boot_status, 0); /* no errors detected */

    start += വലുപ്പം(ഘടന smbios_type_32);
    *((u16 *)start) = 0;

    തിരികെ start+2;
}

/* Type 127 -- End of Table */
സ്ഥിതം ശൂന്യം *
smbios_init_type_127(ശൂന്യം *start)
{
    ഘടന smbios_type_127 *p = (ഘടന smbios_type_127 *)start;

    p->header.type = 127;
    p->header.length = വലുപ്പം(ഘടന smbios_type_127);
    p->header.handle = 0x7f00;

    start += വലുപ്പം(ഘടന smbios_type_127);
    *((u16 *)start) = 0;

    തിരികെ start + 2;
}

#define TEMPSMBIOSSIZE (32 * 1024)

ശൂന്യം
smbios_legacy_setup(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_SMBIOS)
        തിരികെ;

    dprintf(3, "init SMBIOS tables\n");

    അക്ഷരം *start = malloc_tmphigh(TEMPSMBIOSSIZE);
    എങ്കിൽ (! start) {
        warn_noalloc();
        തിരികെ;
    }
    memset(start, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_size = 0;
    അക്ഷരം *q, *p = start;
    അക്ഷരം *end = start + TEMPSMBIOSSIZE - വലുപ്പം(ഘടന smbios_type_127);

#define add_struct(type, args...)                                       \
    ചെയ്യുക {                                                                \
        എങ്കിൽ (!get_external(type, &p, &nr_structs, &max_struct_size, end)) { \
            q = smbios_init_type_##type(args);                          \
            nr_structs++;                                               \
            എങ്കിൽ ((q - p) > max_struct_size)                              \
                max_struct_size = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } ആകുമ്പോൾ (0)

    add_struct(0, p);
    add_struct(1, p);
    add_struct(3, p);

    പൂർണ്ണസംഖ്യ cpu_അക്കം;
    വേണ്ടി (cpu_അക്കം = 1; cpu_അക്കം <= Max_എണ്ണം_CPUs; cpu_അക്കം++)
        add_struct(4, p, cpu_അക്കം);

    പൂർണ്ണസംഖ്യ ram_mb = (RamSize + RamSizeOver4G) >> 20;
    പൂർണ്ണസംഖ്യ nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    add_struct(16, p, ram_mb, nr_mem_devs);

    പൂർണ്ണസംഖ്യ i, j;
    വേണ്ടി (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        add_struct(17, p, dev_mb, i);
    }

    add_struct(19, p, 0, RamSize >> 20, 0);
    എങ്കിൽ (RamSizeOver4G)
        add_struct(19, p, 4096, RamSizeOver4G >> 20, 1);

    add_struct(20, p, 0, RamSize >> 20, 0, 0, 0);
    എങ്കിൽ (RamSizeOver4G) {
        u32 start_mb = 4096;
        വേണ്ടി (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            എങ്കിൽ (i == 0)
                dev_mb -= RamSize >> 20;

            add_struct(20, p, start_mb, dev_mb, j, i, 1);
            start_mb += dev_mb;
        }
    }

    add_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    വേണ്ടി (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_size, end);
    add_struct(127, p);

#undef add_struct

    smbios_21_entry_point_setup(max_struct_size, p - start, start, nr_structs);
    സ്വതന്ത്രം(start);
}
