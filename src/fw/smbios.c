// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "paravirt.h" // RamSize
#شامل "romfile.h" // romfile_findprefix
#شامل "std/smbios.h" // struct smbios_21_entry_point
#شامل "string.h" // memset
#شامل "util.h" // MaxCountCPUs
#شامل "x86.h" // cpuid

ایستا تهی
smbios_21_entry_point_setup(u16 max_structure_size,
                         u16 structure_table_length,
                         تهی *structure_table_address,
                         u16 عدد_of_structures)
{
    تهی *finaltable;
    اگر (structure_table_length <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_table_length);
    وگرنه
        finaltable = malloc_بالا(structure_table_length);
    اگر (!finaltable) {
        warn_noalloc();
        بازگشت;
    }
    memcpy(finaltable, structure_table_address, structure_table_length);

    ساختار smbios_21_entry_point ep;
    memset(&ep, 0, اندازه(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.length = 0x1f;
    ep.smbios_major_version = 2;
    ep.smbios_minor_version = 4;
    ep.max_structure_size = max_structure_size;
    memcpy(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_table_length = structure_table_length;
    ep.structure_table_address = (u32)finaltable;
    ep.عدد_of_structures = عدد_of_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((تهی*)&ep + 0x10, ep.length - 0x10);

    copy_smbios_21(&ep);
}

ایستا صحیح
get_field(صحیح type, صحیح offset, تهی *dest)
{
    نویسه name[128];
    snprintf(name, اندازه(name), "smbios/field%d-%d", type, offset);
    ساختار romfile_s *file = romfile_find(name);
    اگر (!file)
        بازگشت 0;
    file->copy(file, dest, file->size);
    بازگشت file->size;
}

ایستا صحیح
get_external(صحیح type, نویسه **p, unsigned *nr_structs,
             unsigned *max_struct_size, نویسه *end)
{
    ایستا u64 used_bitmap[4] = { 0 };
    نویسه *start = *p;

    /* Check if we've already reported these tables */
    اگر (used_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        بازگشت 1;

    /* Don't introduce spurious end markers */
    اگر (type == 127)
        بازگشت 0;

    نویسه prefix[128];
    snprintf(prefix, اندازه(prefix), "smbios/table%d-", type);
    ساختار romfile_s *file = پوچ;
    برای (;;) {
        file = romfile_findprefix(prefix, file);
        اگر (!file)
            بشکن;

        اگر (end - *p < file->size) {
            warn_noalloc();
            بشکن;
        }

        ساختار smbios_structure_header *header = (تهی*)*p;
        file->copy(file, header, file->size);
        *p += file->size;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        اگر (header->length >= file->size) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        اگر (*p - (نویسه*)header > *max_struct_size)
            *max_struct_size = *p - (نویسه*)header;
    }

    اگر (start == *p)
        بازگشت 0;

    /* Mark that we've reported on this type */
    used_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    بازگشت 1;
}

#define load_str_field_with_default(type, field, def)                   \
    انجامبده {                                                                \
        size = get_field(type, offsetof(ساختار smbios_type_##type,      \
                                        field), end);                   \
        اگر (size == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->field = 0;                                               \
        } وگرنه اگر (size > 1) {                                          \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } وگرنه {                                                        \
            memcpy(end, def, اندازه(def));                              \
            end += اندازه(def);                                         \
            p->field = ++str_index;                                     \
        }                                                               \
    } وقتی (0)

#define load_str_field_or_skip(type, field)                             \
    انجامبده {                                                                \
        size = get_field(type, offsetof(ساختار smbios_type_##type,      \
                                        field), end);                   \
        اگر (size > 1) {                                                 \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } وگرنه {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } وقتی (0)

#define set_field_with_default(type, field, def)                        \
    انجامبده {                                                                \
        اگر (!get_field(type, offsetof(ساختار smbios_type_##type,        \
                                      field), &p->field)) {             \
            p->field = def;                                             \
        }                                                               \
    } وقتی (0)

/* Type 0 -- BIOS Information */
#define RELEASE_DATE_STR "01/01/2011"
ایستا تهی *
smbios_init_type_0(تهی *start)
{
    ساختار smbios_type_0 *p = (ساختار smbios_type_0 *)start;
    نویسه *end = (نویسه *)start + اندازه(ساختار smbios_type_0);
    size_t size;
    صحیح str_index = 0;

    p->header.type = 0;
    p->header.length = اندازه(ساختار smbios_type_0);
    p->header.handle = 0;

    load_str_field_with_default(0, vendor_str, BUILD_APPNAME);
    load_str_field_with_default(0, bios_version_str, BUILD_APPNAME);

    p->bios_starting_address_segment = 0xe800;

    load_str_field_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_size = 0; /* FIXME */

    اگر (!get_field(0, offsetof(ساختار smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    اگر (!get_field(0, offsetof(ساختار smbios_type_0,
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
    اگر (!str_index) {
        *end = 0;
        end++;
    }

    بازگشت end;
}

/* Type 1 -- System Information */
ایستا تهی *
smbios_init_type_1(تهی *start)
{
    ساختار smbios_type_1 *p = (ساختار smbios_type_1 *)start;
    نویسه *end = (نویسه *)start + اندازه(ساختار smbios_type_1);
    size_t size;
    صحیح str_index = 0;

    p->header.type = 1;
    p->header.length = اندازه(ساختار smbios_type_1);
    p->header.handle = 0x100;

    load_str_field_with_default(1, manufacturer_str, BUILD_APPNAME);
    load_str_field_with_default(1, product_name_str, BUILD_APPNAME);
    load_str_field_or_skip(1, version_str);
    load_str_field_or_skip(1, serial_عدد_str);

    اگر (!get_field(1, offsetof(ساختار smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    set_field_with_default(1, wake_up_type, 0x06); /* power switch */

    load_str_field_or_skip(1, sku_عدد_str);
    load_str_field_or_skip(1, family_str);

    *end = 0;
    end++;
    اگر (!str_index) {
        *end = 0;
        end++;
    }

    بازگشت end;
}

/* Type 3 -- System Enclosure */
ایستا تهی *
smbios_init_type_3(تهی *start)
{
    ساختار smbios_type_3 *p = (ساختار smbios_type_3 *)start;
    نویسه *end = (نویسه *)start + اندازه(ساختار smbios_type_3);
    size_t size;
    صحیح str_index = 0;

    p->header.type = 3;
    p->header.length = اندازه(ساختار smbios_type_3);
    p->header.handle = 0x300;

    load_str_field_with_default(3, manufacturer_str, BUILD_APPNAME);
    set_field_with_default(3, type, 0x01); /* other */

    load_str_field_or_skip(3, version_str);
    load_str_field_or_skip(3, serial_عدد_str);
    load_str_field_or_skip(3, asset_tag_عدد_str);

    set_field_with_default(3, boot_up_state, 0x03); /* safe */
    set_field_with_default(3, power_supply_state, 0x03); /* safe */
    set_field_with_default(3, thermal_state, 0x03); /* safe */
    set_field_with_default(3, security_status, 0x02); /* unknown */

    set_field_with_default(3, oem_defined, 0);
    set_field_with_default(3, height, 0);
    set_field_with_default(3, عدد_of_power_cords, 0);
    set_field_with_default(3, contained_element_شمار, 0);

    *end = 0;
    end++;
    اگر (!str_index) {
        *end = 0;
        end++;
    }

    بازگشت end;
}

/* Type 4 -- Processor Information */
ایستا تهی *
smbios_init_type_4(تهی *start, unsigned صحیح cpu_عدد)
{
    ساختار smbios_type_4 *p = (ساختار smbios_type_4 *)start;
    نویسه *end = (نویسه *)start + اندازه(ساختار smbios_type_4);
    size_t size;
    صحیح str_index = 0;
    نویسه name[1024];

    p->header.type = 4;
    p->header.length = اندازه(ساختار smbios_type_4);
    p->header.handle = 0x400 + cpu_عدد;

    size = get_field(4, offsetof(ساختار smbios_type_4, socket_designation_str),
                     name);
    اگر (size)
        snprintf(name + size - 1, اندازه(name) - size, "%2x", cpu_عدد);
    وگرنه
        snprintf(name, اندازه(name), "CPU%2x", cpu_عدد);

    memcpy(end, name, strlen(name) + 1);
    end += strlen(name) + 1;
    p->socket_designation_str = ++str_index;

    set_field_with_default(4, processor_type, 0x03); /* CPU */
    set_field_with_default(4, processor_family, 0x01); /* other */

    load_str_field_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    اگر (!get_field(4, offsetof(ساختار smbios_type_4, processor_id)
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
    اگر (!str_index) {
        *end = 0;
        end++;
    }

    بازگشت end;
}

/* Type 16 -- Physical Memory Array */
ایستا تهی *
smbios_init_type_16(تهی *start, u32 memory_size_mb, صحیح nr_mem_devs)
{
    ساختار smbios_type_16 *p = (ساختار smbios_type_16*)start;

    p->header.type = 16;
    p->header.length = اندازه(ساختار smbios_type_16);
    p->header.handle = 0x1000;

    set_field_with_default(16, location, 0x01); /* other */
    set_field_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    set_field_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->maximum_capacity = memory_size_mb < 2 << 20 ?
                          memory_size_mb << 10 : 0x80000000;
    p->memory_error_information_handle = 0xfffe; /* none provided */
    p->عدد_of_memory_devices = nr_mem_devs;

    start += اندازه(ساختار smbios_type_16);
    *((u16 *)start) = 0;

    بازگشت start + 2;
}

/* Type 17 -- Memory Device */
ایستا تهی *
smbios_init_type_17(تهی *start, u32 size_mb, صحیح instance)
{
    ساختار smbios_type_17 *p = (ساختار smbios_type_17 *)start;
    نویسه *end = (نویسه *)start + اندازه(ساختار smbios_type_17);
    size_t size;
    صحیح str_index = 0;
    نویسه name[1024];

    p->header.type = 17;
    p->header.length = اندازه(ساختار smbios_type_17);
    p->header.handle = 0x1100 + instance;

    p->physical_memory_array_handle = 0x1000;
    set_field_with_default(17, total_width, 64);
    set_field_with_default(17, data_width, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->size = size_mb;
    set_field_with_default(17, form_factor, 0x09); /* DIMM */
    p->device_set = 0;

    size = get_field(17, offsetof(ساختار smbios_type_17, device_locator_str),
                     name);
    اگر (size)
        snprintf(name + size - 1, اندازه(name) - size, "%d", instance);
    وگرنه
        snprintf(name, اندازه(name), "DIMM %d", instance);

    memcpy(end, name, strlen(name) + 1);
    end += strlen(name) + 1;
    p->device_locator_str = ++str_index;

    load_str_field_or_skip(17, bank_locator_str);
    set_field_with_default(17, memory_type, 0x07); /* RAM */
    set_field_with_default(17, type_detail, 0);

    *end = 0;
    end++;
    اگر (!str_index) {
        *end = 0;
        end++;
    }

    بازگشت end;
}

/* Type 19 -- Memory Array Mapped Address */
ایستا تهی *
smbios_init_type_19(تهی *start, u32 start_mb, u32 size_mb, صحیح instance)
{
    ساختار smbios_type_19 *p = (ساختار smbios_type_19 *)start;

    p->header.type = 19;
    p->header.length = اندازه(ساختار smbios_type_19);
    p->header.handle = 0x1300 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (size_mb << 10) - 1;
    p->memory_array_handle = 0x1000;
    p->partition_width = 1;

    start += اندازه(ساختار smbios_type_19);
    *((u16 *)start) = 0;

    بازگشت start + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ایستا تهی *
smbios_init_type_20(تهی *start, u32 start_mb, u32 size_mb, صحیح instance,
                    صحیح dev_handle, صحیح array_handle)
{
    ساختار smbios_type_20 *p = (ساختار smbios_type_20 *)start;

    p->header.type = 20;
    p->header.length = اندازه(ساختار smbios_type_20);
    p->header.handle = 0x1400 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (size_mb << 10) - 1;
    p->memory_device_handle = 0x1100 + dev_handle;
    p->memory_array_mapped_address_handle = 0x1300 + array_handle;
    p->partition_row_position = 1;
    p->interleave_position = 0;
    p->interleaved_data_depth = 0;

    start += اندازه(ساختار smbios_type_20);

    *((u16 *)start) = 0;
    بازگشت start+2;
}

/* Type 32 -- System Boot Information */
ایستا تهی *
smbios_init_type_32(تهی *start)
{
    ساختار smbios_type_32 *p = (ساختار smbios_type_32 *)start;

    p->header.type = 32;
    p->header.length = اندازه(ساختار smbios_type_32);
    p->header.handle = 0x2000;
    memset(p->reserved, 0, 6);
    set_field_with_default(32, boot_status, 0); /* no errors detected */

    start += اندازه(ساختار smbios_type_32);
    *((u16 *)start) = 0;

    بازگشت start+2;
}

/* Type 127 -- End of Table */
ایستا تهی *
smbios_init_type_127(تهی *start)
{
    ساختار smbios_type_127 *p = (ساختار smbios_type_127 *)start;

    p->header.type = 127;
    p->header.length = اندازه(ساختار smbios_type_127);
    p->header.handle = 0x7f00;

    start += اندازه(ساختار smbios_type_127);
    *((u16 *)start) = 0;

    بازگشت start + 2;
}

#define TEMPSMBIOSSIZE (32 * 1024)

تهی
smbios_legacy_setup(تهی)
{
    اگر (! CONFIG_SMBIOS)
        بازگشت;

    dprintf(3, "init SMBIOS tables\n");

    نویسه *start = malloc_tmphigh(TEMPSMBIOSSIZE);
    اگر (! start) {
        warn_noalloc();
        بازگشت;
    }
    memset(start, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_size = 0;
    نویسه *q, *p = start;
    نویسه *end = start + TEMPSMBIOSSIZE - اندازه(ساختار smbios_type_127);

#define add_struct(type, args...)                                       \
    انجامبده {                                                                \
        اگر (!get_external(type, &p, &nr_structs, &max_struct_size, end)) { \
            q = smbios_init_type_##type(args);                          \
            nr_structs++;                                               \
            اگر ((q - p) > max_struct_size)                              \
                max_struct_size = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } وقتی (0)

    add_struct(0, p);
    add_struct(1, p);
    add_struct(3, p);

    صحیح cpu_رقم;
    برای (cpu_رقم = 1; cpu_رقم <= Max_شمار_CPUs; cpu_رقم++)
        add_struct(4, p, cpu_رقم);

    صحیح ram_mb = (RamSize + RamSizeOver4G) >> 20;
    صحیح nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    add_struct(16, p, ram_mb, nr_mem_devs);

    صحیح i, j;
    برای (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        add_struct(17, p, dev_mb, i);
    }

    add_struct(19, p, 0, RamSize >> 20, 0);
    اگر (RamSizeOver4G)
        add_struct(19, p, 4096, RamSizeOver4G >> 20, 1);

    add_struct(20, p, 0, RamSize >> 20, 0, 0, 0);
    اگر (RamSizeOver4G) {
        u32 start_mb = 4096;
        برای (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            اگر (i == 0)
                dev_mb -= RamSize >> 20;

            add_struct(20, p, start_mb, dev_mb, j, i, 1);
            start_mb += dev_mb;
        }
    }

    add_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    برای (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_size, end);
    add_struct(127, p);

#undef add_struct

    smbios_21_entry_point_setup(max_struct_size, p - start, start, nr_structs);
    آزاد(start);
}
