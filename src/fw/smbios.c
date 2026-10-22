// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "paravirt.h" // RamSize
#অন্তর্ভাব "romfile.h" // romfile_findprefix
#অন্তর্ভাব "std/smbios.h" // struct smbios_21_entry_point
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // MaxCountCPUs
#অন্তর্ভাব "x86.h" // cpuid

স্থির শূন্য
smbios_21_entry_point_setup(u16 max_structure_আকার,
                         u16 structure_table_দৈর্ঘ্য,
                         শূন্য *structure_table_address,
                         u16 সংখ্যা_of_structures)
{
    শূন্য *finaltable;
    যদি (structure_table_দৈর্ঘ্য <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_table_দৈর্ঘ্য);
    অন্যথা
        finaltable = malloc_উপরের(structure_table_দৈর্ঘ্য);
    যদি (!finaltable) {
        warn_noalloc();
        ফেরত;
    }
    memcpy(finaltable, structure_table_address, structure_table_দৈর্ঘ্য);

    কাঠামো smbios_21_entry_point ep;
    memset(&ep, 0, আকার(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.দৈর্ঘ্য = 0x1f;
    ep.smbios_major_version = 2;
    ep.smbios_minor_version = 4;
    ep.max_structure_আকার = max_structure_আকার;
    memcpy(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_table_দৈর্ঘ্য = structure_table_দৈর্ঘ্য;
    ep.structure_table_address = (u32)finaltable;
    ep.সংখ্যা_of_structures = সংখ্যা_of_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((শূন্য*)&ep + 0x10, ep.দৈর্ঘ্য - 0x10);

    copy_smbios_21(&ep);
}

স্থির পূর্ণসংখ্যা
get_field(পূর্ণসংখ্যা type, পূর্ণসংখ্যা offset, শূন্য *dest)
{
    অক্ষর name[128];
    snprintf(name, আকার(name), "smbios/field%d-%d", type, offset);
    কাঠামো romfile_s *file = romfile_find(name);
    যদি (!file)
        ফেরত 0;
    file->copy(file, dest, file->আকার_2);
    ফেরত file->আকার_2;
}

স্থির পূর্ণসংখ্যা
get_external(পূর্ণসংখ্যা type, অক্ষর **p, unsigned *nr_structs,
             unsigned *max_struct_আকার, অক্ষর *end)
{
    স্থির u64 used_bitmap[4] = { 0 };
    অক্ষর *start = *p;

    /* Check if we've already reported these tables */
    যদি (used_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        ফেরত 1;

    /* Don't introduce spurious end markers */
    যদি (type == 127)
        ফেরত 0;

    অক্ষর prefix[128];
    snprintf(prefix, আকার(prefix), "smbios/table%d-", type);
    কাঠামো romfile_s *file = ফাঁকা;
    জন্য (;;) {
        file = romfile_findprefix(prefix, file);
        যদি (!file)
            বিরতি;

        যদি (end - *p < file->আকার_2) {
            warn_noalloc();
            বিরতি;
        }

        কাঠামো smbios_structure_header *header = (শূন্য*)*p;
        file->copy(file, header, file->আকার_2);
        *p += file->আকার_2;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        যদি (header->দৈর্ঘ্য >= file->আকার_2) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        যদি (*p - (অক্ষর*)header > *max_struct_আকার)
            *max_struct_আকার = *p - (অক্ষর*)header;
    }

    যদি (start == *p)
        ফেরত 0;

    /* Mark that we've reported on this type */
    used_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    ফেরত 1;
}

#define load_str_field_with_default(type, field, def)                   \
    করো {                                                                \
        আকার_2 = get_field(type, offsetof(কাঠামো smbios_type_##type,      \
                                        field), end);                   \
        যদি (আকার_2 == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->field = 0;                                               \
        } অন্যথা যদি (আকার_2 > 1) {                                          \
            end += আকার_2;                                                \
            p->field = ++str_সূচক;                                     \
        } অন্যথা {                                                        \
            memcpy(end, def, আকার(def));                              \
            end += আকার(def);                                         \
            p->field = ++str_সূচক;                                     \
        }                                                               \
    } যখন (0)

#define load_str_field_or_skip(type, field)                             \
    করো {                                                                \
        আকার_2 = get_field(type, offsetof(কাঠামো smbios_type_##type,      \
                                        field), end);                   \
        যদি (আকার_2 > 1) {                                                 \
            end += আকার_2;                                                \
            p->field = ++str_সূচক;                                     \
        } অন্যথা {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } যখন (0)

#define set_field_with_default(type, field, def)                        \
    করো {                                                                \
        যদি (!get_field(type, offsetof(কাঠামো smbios_type_##type,        \
                                      field), &p->field)) {             \
            p->field = def;                                             \
        }                                                               \
    } যখন (0)

/* Type 0 -- BIOS Information */
#define RELEASE_DATE_STR "01/01/2011"
স্থির শূন্য *
smbios_init_type_0(শূন্য *start)
{
    কাঠামো smbios_type_0 *p = (কাঠামো smbios_type_0 *)start;
    অক্ষর *end = (অক্ষর *)start + আকার(কাঠামো smbios_type_0);
    আকার_t আকার_2;
    পূর্ণসংখ্যা str_সূচক = 0;

    p->header.type = 0;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_0);
    p->header.handle = 0;

    load_str_field_with_default(0, vendor_str, BUILD_APPNAME);
    load_str_field_with_default(0, bios_version_str, BUILD_APPNAME);

    p->bios_starting_address_segment = 0xe800;

    load_str_field_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_আকার = 0; /* FIXME */

    যদি (!get_field(0, offsetof(কাঠামো smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    যদি (!get_field(0, offsetof(কাঠামো smbios_type_0,
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
    যদি (!str_সূচক) {
        *end = 0;
        end++;
    }

    ফেরত end;
}

/* Type 1 -- System Information */
স্থির শূন্য *
smbios_init_type_1(শূন্য *start)
{
    কাঠামো smbios_type_1 *p = (কাঠামো smbios_type_1 *)start;
    অক্ষর *end = (অক্ষর *)start + আকার(কাঠামো smbios_type_1);
    আকার_t আকার_2;
    পূর্ণসংখ্যা str_সূচক = 0;

    p->header.type = 1;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_1);
    p->header.handle = 0x100;

    load_str_field_with_default(1, manufacturer_str, BUILD_APPNAME);
    load_str_field_with_default(1, product_name_str, BUILD_APPNAME);
    load_str_field_or_skip(1, version_str);
    load_str_field_or_skip(1, serial_সংখ্যা_str);

    যদি (!get_field(1, offsetof(কাঠামো smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    set_field_with_default(1, wake_up_type, 0x06); /* power switch */

    load_str_field_or_skip(1, sku_সংখ্যা_str);
    load_str_field_or_skip(1, family_str);

    *end = 0;
    end++;
    যদি (!str_সূচক) {
        *end = 0;
        end++;
    }

    ফেরত end;
}

/* Type 3 -- System Enclosure */
স্থির শূন্য *
smbios_init_type_3(শূন্য *start)
{
    কাঠামো smbios_type_3 *p = (কাঠামো smbios_type_3 *)start;
    অক্ষর *end = (অক্ষর *)start + আকার(কাঠামো smbios_type_3);
    আকার_t আকার_2;
    পূর্ণসংখ্যা str_সূচক = 0;

    p->header.type = 3;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_3);
    p->header.handle = 0x300;

    load_str_field_with_default(3, manufacturer_str, BUILD_APPNAME);
    set_field_with_default(3, type, 0x01); /* other */

    load_str_field_or_skip(3, version_str);
    load_str_field_or_skip(3, serial_সংখ্যা_str);
    load_str_field_or_skip(3, asset_tag_সংখ্যা_str);

    set_field_with_default(3, boot_up_state, 0x03); /* safe */
    set_field_with_default(3, power_supply_state, 0x03); /* safe */
    set_field_with_default(3, thermal_state, 0x03); /* safe */
    set_field_with_default(3, security_status, 0x02); /* unknown */

    set_field_with_default(3, oem_defined, 0);
    set_field_with_default(3, height, 0);
    set_field_with_default(3, সংখ্যা_of_power_cords, 0);
    set_field_with_default(3, contained_element_গুনতি, 0);

    *end = 0;
    end++;
    যদি (!str_সূচক) {
        *end = 0;
        end++;
    }

    ফেরত end;
}

/* Type 4 -- Processor Information */
স্থির শূন্য *
smbios_init_type_4(শূন্য *start, unsigned পূর্ণসংখ্যা cpu_সংখ্যা)
{
    কাঠামো smbios_type_4 *p = (কাঠামো smbios_type_4 *)start;
    অক্ষর *end = (অক্ষর *)start + আকার(কাঠামো smbios_type_4);
    আকার_t আকার_2;
    পূর্ণসংখ্যা str_সূচক = 0;
    অক্ষর name[1024];

    p->header.type = 4;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_4);
    p->header.handle = 0x400 + cpu_সংখ্যা;

    আকার_2 = get_field(4, offsetof(কাঠামো smbios_type_4, socket_designation_str),
                     name);
    যদি (আকার_2)
        snprintf(name + আকার_2 - 1, আকার(name) - আকার_2, "%2x", cpu_সংখ্যা);
    অন্যথা
        snprintf(name, আকার(name), "CPU%2x", cpu_সংখ্যা);

    memcpy(end, name, strlen(name) + 1);
    end += strlen(name) + 1;
    p->socket_designation_str = ++str_সূচক;

    set_field_with_default(4, processor_type, 0x03); /* CPU */
    set_field_with_default(4, processor_family, 0x01); /* other */

    load_str_field_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    যদি (!get_field(4, offsetof(কাঠামো smbios_type_4, processor_id)
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
    যদি (!str_সূচক) {
        *end = 0;
        end++;
    }

    ফেরত end;
}

/* Type 16 -- Physical Memory Array */
স্থির শূন্য *
smbios_init_type_16(শূন্য *start, u32 memory_আকার_mb, পূর্ণসংখ্যা nr_mem_devs)
{
    কাঠামো smbios_type_16 *p = (কাঠামো smbios_type_16*)start;

    p->header.type = 16;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_16);
    p->header.handle = 0x1000;

    set_field_with_default(16, location, 0x01); /* other */
    set_field_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    set_field_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->maximum_capacity = memory_আকার_mb < 2 << 20 ?
                          memory_আকার_mb << 10 : 0x80000000;
    p->memory_error_information_handle = 0xfffe; /* none provided */
    p->সংখ্যা_of_memory_devices = nr_mem_devs;

    start += আকার(কাঠামো smbios_type_16);
    *((u16 *)start) = 0;

    ফেরত start + 2;
}

/* Type 17 -- Memory Device */
স্থির শূন্য *
smbios_init_type_17(শূন্য *start, u32 আকার_mb, পূর্ণসংখ্যা instance)
{
    কাঠামো smbios_type_17 *p = (কাঠামো smbios_type_17 *)start;
    অক্ষর *end = (অক্ষর *)start + আকার(কাঠামো smbios_type_17);
    আকার_t আকার_2;
    পূর্ণসংখ্যা str_সূচক = 0;
    অক্ষর name[1024];

    p->header.type = 17;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_17);
    p->header.handle = 0x1100 + instance;

    p->physical_memory_array_handle = 0x1000;
    set_field_with_default(17, যোগ_width, 64);
    set_field_with_default(17, তথ্য_width, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->আকার_2 = আকার_mb;
    set_field_with_default(17, form_factor, 0x09); /* DIMM */
    p->device_set = 0;

    আকার_2 = get_field(17, offsetof(কাঠামো smbios_type_17, device_locator_str),
                     name);
    যদি (আকার_2)
        snprintf(name + আকার_2 - 1, আকার(name) - আকার_2, "%d", instance);
    অন্যথা
        snprintf(name, আকার(name), "DIMM %d", instance);

    memcpy(end, name, strlen(name) + 1);
    end += strlen(name) + 1;
    p->device_locator_str = ++str_সূচক;

    load_str_field_or_skip(17, bank_locator_str);
    set_field_with_default(17, memory_type, 0x07); /* RAM */
    set_field_with_default(17, type_detail, 0);

    *end = 0;
    end++;
    যদি (!str_সূচক) {
        *end = 0;
        end++;
    }

    ফেরত end;
}

/* Type 19 -- Memory Array Mapped Address */
স্থির শূন্য *
smbios_init_type_19(শূন্য *start, u32 start_mb, u32 আকার_mb, পূর্ণসংখ্যা instance)
{
    কাঠামো smbios_type_19 *p = (কাঠামো smbios_type_19 *)start;

    p->header.type = 19;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_19);
    p->header.handle = 0x1300 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (আকার_mb << 10) - 1;
    p->memory_array_handle = 0x1000;
    p->partition_width = 1;

    start += আকার(কাঠামো smbios_type_19);
    *((u16 *)start) = 0;

    ফেরত start + 2;
}

/* Type 20 -- Memory Device Mapped Address */
স্থির শূন্য *
smbios_init_type_20(শূন্য *start, u32 start_mb, u32 আকার_mb, পূর্ণসংখ্যা instance,
                    পূর্ণসংখ্যা dev_handle, পূর্ণসংখ্যা array_handle)
{
    কাঠামো smbios_type_20 *p = (কাঠামো smbios_type_20 *)start;

    p->header.type = 20;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_20);
    p->header.handle = 0x1400 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (আকার_mb << 10) - 1;
    p->memory_device_handle = 0x1100 + dev_handle;
    p->memory_array_mapped_address_handle = 0x1300 + array_handle;
    p->partition_row_position = 1;
    p->interleave_position = 0;
    p->interleaved_তথ্য_depth = 0;

    start += আকার(কাঠামো smbios_type_20);

    *((u16 *)start) = 0;
    ফেরত start+2;
}

/* Type 32 -- System Boot Information */
স্থির শূন্য *
smbios_init_type_32(শূন্য *start)
{
    কাঠামো smbios_type_32 *p = (কাঠামো smbios_type_32 *)start;

    p->header.type = 32;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_32);
    p->header.handle = 0x2000;
    memset(p->reserved, 0, 6);
    set_field_with_default(32, boot_status, 0); /* no errors detected */

    start += আকার(কাঠামো smbios_type_32);
    *((u16 *)start) = 0;

    ফেরত start+2;
}

/* Type 127 -- End of Table */
স্থির শূন্য *
smbios_init_type_127(শূন্য *start)
{
    কাঠামো smbios_type_127 *p = (কাঠামো smbios_type_127 *)start;

    p->header.type = 127;
    p->header.দৈর্ঘ্য = আকার(কাঠামো smbios_type_127);
    p->header.handle = 0x7f00;

    start += আকার(কাঠামো smbios_type_127);
    *((u16 *)start) = 0;

    ফেরত start + 2;
}

#define TEMPSMBIOSSIZE (32 * 1024)

শূন্য
smbios_legacy_setup(শূন্য)
{
    যদি (! CONFIG_SMBIOS)
        ফেরত;

    dprintf(3, "init SMBIOS tables\n");

    অক্ষর *start = malloc_tmphigh(TEMPSMBIOSSIZE);
    যদি (! start) {
        warn_noalloc();
        ফেরত;
    }
    memset(start, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_আকার = 0;
    অক্ষর *q, *p = start;
    অক্ষর *end = start + TEMPSMBIOSSIZE - আকার(কাঠামো smbios_type_127);

#define add_struct(type, args...)                                       \
    করো {                                                                \
        যদি (!get_external(type, &p, &nr_structs, &max_struct_আকার, end)) { \
            q = smbios_init_type_##type(args);                          \
            nr_structs++;                                               \
            যদি ((q - p) > max_struct_আকার)                              \
                max_struct_আকার = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } যখন (0)

    add_struct(0, p);
    add_struct(1, p);
    add_struct(3, p);

    পূর্ণসংখ্যা cpu_অঙ্ক;
    জন্য (cpu_অঙ্ক = 1; cpu_অঙ্ক <= Max_গুনতি_CPUs; cpu_অঙ্ক++)
        add_struct(4, p, cpu_অঙ্ক);

    পূর্ণসংখ্যা ram_mb = (Ram_আকার + Ram_আকার_Over4_G) >> 20;
    পূর্ণসংখ্যা nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    add_struct(16, p, ram_mb, nr_mem_devs);

    পূর্ণসংখ্যা i, j;
    জন্য (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        add_struct(17, p, dev_mb, i);
    }

    add_struct(19, p, 0, Ram_আকার >> 20, 0);
    যদি (Ram_আকার_Over4_G)
        add_struct(19, p, 4096, Ram_আকার_Over4_G >> 20, 1);

    add_struct(20, p, 0, Ram_আকার >> 20, 0, 0, 0);
    যদি (Ram_আকার_Over4_G) {
        u32 start_mb = 4096;
        জন্য (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            যদি (i == 0)
                dev_mb -= Ram_আকার >> 20;

            add_struct(20, p, start_mb, dev_mb, j, i, 1);
            start_mb += dev_mb;
        }
    }

    add_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    জন্য (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_আকার, end);
    add_struct(127, p);

#undef add_struct

    smbios_21_entry_point_setup(max_struct_আকার, p - start, start, nr_structs);
    মুক্ত(start);
}
