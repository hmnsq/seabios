// smbios table generation (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "paravirt.h" // RamSize
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_findprefix
#ଅନ୍ତର୍ଭାଵ "std/smbios.h" // struct smbios_21_entry_point
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // MaxCountCPUs
#ଅନ୍ତର୍ଭାଵ "x86.h" // cpuid

ସ୍ଥିର ଶୂନ୍ୟ
smbios_21_entry_point_setup(u16 max_structure_size,
                         u16 structure_table_length,
                         ଶୂନ୍ୟ *structure_table_address,
                         u16 ସଂଖ୍ୟା_of_structures)
{
    ଶୂନ୍ୟ *finaltable;
    ଯଦି (structure_table_length <= BUILD_MAX_SMBIOS_FSEG)
        // Table is small enough for f-seg - allocate there.  This
        // works around a bug in JunOS (at least for small SMBIOS tables).
        finaltable = malloc_fseg(structure_table_length);
    ଅନ୍ୟଥା
        finaltable = malloc_ଉପର(structure_table_length);
    ଯଦି (!finaltable) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memcpy(finaltable, structure_table_address, structure_table_length);

    ସଂରଚନା smbios_21_entry_point ep;
    memset(&ep, 0, ଆକାର(ep));
    ep.signature = SMBIOS_21_SIGNATURE;
    ep.length = 0x1f;
    ep.smbios_major_version = 2;
    ep.smbios_minor_version = 4;
    ep.max_structure_size = max_structure_size;
    memcpy(ep.intermediate_anchor_string, "_DMI_", 5);

    ep.structure_table_length = structure_table_length;
    ep.structure_table_address = (u32)finaltable;
    ep.ସଂଖ୍ୟା_of_structures = ସଂଖ୍ୟା_of_structures;
    ep.smbios_bcd_revision = 0x24;

    ep.checksum -= checksum(&ep, 0x10);

    ep.intermediate_checksum -= checksum((ଶୂନ୍ୟ*)&ep + 0x10, ep.length - 0x10);

    copy_smbios_21(&ep);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_field(ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset, ଶୂନ୍ୟ *dest)
{
    ଅକ୍ଷର name[128];
    snprintf(name, ଆକାର(name), "smbios/field%d-%d", type, offset);
    ସଂରଚନା romfile_s *file = romfile_find(name);
    ଯଦି (!file)
        ଫେରନ୍ତୁ 0;
    file->copy(file, dest, file->size);
    ଫେରନ୍ତୁ file->size;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
get_external(ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଅକ୍ଷର **p, unsigned *nr_structs,
             unsigned *max_struct_size, ଅକ୍ଷର *end)
{
    ସ୍ଥିର u64 used_bitmap[4] = { 0 };
    ଅକ୍ଷର *start = *p;

    /* Check if we've already reported these tables */
    ଯଦି (used_bitmap[(type >> 6) & 0x3] & (1ULL << (type & 0x3f)))
        ଫେରନ୍ତୁ 1;

    /* Don't introduce spurious end markers */
    ଯଦି (type == 127)
        ଫେରନ୍ତୁ 0;

    ଅକ୍ଷର prefix[128];
    snprintf(prefix, ଆକାର(prefix), "smbios/table%d-", type);
    ସଂରଚନା romfile_s *file = ଖାଲି;
    ପାଇଁ (;;) {
        file = romfile_findprefix(prefix, file);
        ଯଦି (!file)
            ଭାଙ୍ଗ;

        ଯଦି (end - *p < file->size) {
            warn_noalloc();
            ଭାଙ୍ଗ;
        }

        ସଂରଚନା smbios_structure_header *header = (ଶୂନ୍ୟ*)*p;
        file->copy(file, header, file->size);
        *p += file->size;

        /* Entries end with a double NULL char, if there's a string at
         * the end (length is greater than formatted length), the string
         * terminator provides the first NULL. */
        *((u8*)*p) = 0;
        (*p)++;
        ଯଦି (header->length >= file->size) {
            *((u8*)*p) = 0;
            (*p)++;
        }

        (*nr_structs)++;
        ଯଦି (*p - (ଅକ୍ଷର*)header > *max_struct_size)
            *max_struct_size = *p - (ଅକ୍ଷର*)header;
    }

    ଯଦି (start == *p)
        ଫେରନ୍ତୁ 0;

    /* Mark that we've reported on this type */
    used_bitmap[(type >> 6) & 0x3] |= (1ULL << (type & 0x3f));
    ଫେରନ୍ତୁ 1;
}

#define load_str_field_with_default(type, field, def)                   \
    କର {                                                                \
        size = get_field(type, offsetof(ସଂରଚନା smbios_type_##type,      \
                                        field), end);                   \
        ଯଦି (size == 1) {                                                \
            /* zero-length string, skip to avoid bogus end marker */    \
            p->field = 0;                                               \
        } ଅନ୍ୟଥା ଯଦି (size > 1) {                                          \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } ଅନ୍ୟଥା {                                                        \
            memcpy(end, def, ଆକାର(def));                              \
            end += ଆକାର(def);                                         \
            p->field = ++str_index;                                     \
        }                                                               \
    } ଯେତେବେଳେ (0)

#define load_str_field_or_skip(type, field)                             \
    କର {                                                                \
        size = get_field(type, offsetof(ସଂରଚନା smbios_type_##type,      \
                                        field), end);                   \
        ଯଦି (size > 1) {                                                 \
            end += size;                                                \
            p->field = ++str_index;                                     \
        } ଅନ୍ୟଥା {                                                        \
            p->field = 0;                                               \
        }                                                               \
    } ଯେତେବେଳେ (0)

#define set_field_with_default(type, field, def)                        \
    କର {                                                                \
        ଯଦି (!get_field(type, offsetof(ସଂରଚନା smbios_type_##type,        \
                                      field), &p->field)) {             \
            p->field = def;                                             \
        }                                                               \
    } ଯେତେବେଳେ (0)

/* Type 0 -- BIOS Information */
#define RELEASE_DATE_STR "01/01/2011"
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_0(ଶୂନ୍ୟ *start)
{
    ସଂରଚନା smbios_type_0 *p = (ସଂରଚନା smbios_type_0 *)start;
    ଅକ୍ଷର *end = (ଅକ୍ଷର *)start + ଆକାର(ସଂରଚନା smbios_type_0);
    size_t size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_index = 0;

    p->header.type = 0;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_0);
    p->header.handle = 0;

    load_str_field_with_default(0, vendor_str, BUILD_APPNAME);
    load_str_field_with_default(0, bios_version_str, BUILD_APPNAME);

    p->bios_starting_address_segment = 0xe800;

    load_str_field_with_default(0, bios_release_date_str, RELEASE_DATE_STR);

    p->bios_rom_size = 0; /* FIXME */

    ଯଦି (!get_field(0, offsetof(ସଂରଚନା smbios_type_0, bios_characteristics),
                   &p->bios_characteristics)) {
        memset(p->bios_characteristics, 0, 8);
        /* BIOS characteristics not supported */
        p->bios_characteristics[0] = 0x08;
    }

    ଯଦି (!get_field(0, offsetof(ସଂରଚନା smbios_type_0,
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
    ଯଦି (!str_index) {
        *end = 0;
        end++;
    }

    ଫେରନ୍ତୁ end;
}

/* Type 1 -- System Information */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_1(ଶୂନ୍ୟ *start)
{
    ସଂରଚନା smbios_type_1 *p = (ସଂରଚନା smbios_type_1 *)start;
    ଅକ୍ଷର *end = (ଅକ୍ଷର *)start + ଆକାର(ସଂରଚନା smbios_type_1);
    size_t size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_index = 0;

    p->header.type = 1;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_1);
    p->header.handle = 0x100;

    load_str_field_with_default(1, manufacturer_str, BUILD_APPNAME);
    load_str_field_with_default(1, product_name_str, BUILD_APPNAME);
    load_str_field_or_skip(1, version_str);
    load_str_field_or_skip(1, serial_ସଂଖ୍ୟା_str);

    ଯଦି (!get_field(1, offsetof(ସଂରଚନା smbios_type_1, uuid), &p->uuid))
        memset(p->uuid, 0, 16);

    set_field_with_default(1, wake_up_type, 0x06); /* power switch */

    load_str_field_or_skip(1, sku_ସଂଖ୍ୟା_str);
    load_str_field_or_skip(1, family_str);

    *end = 0;
    end++;
    ଯଦି (!str_index) {
        *end = 0;
        end++;
    }

    ଫେରନ୍ତୁ end;
}

/* Type 3 -- System Enclosure */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_3(ଶୂନ୍ୟ *start)
{
    ସଂରଚନା smbios_type_3 *p = (ସଂରଚନା smbios_type_3 *)start;
    ଅକ୍ଷର *end = (ଅକ୍ଷର *)start + ଆକାର(ସଂରଚନା smbios_type_3);
    size_t size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_index = 0;

    p->header.type = 3;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_3);
    p->header.handle = 0x300;

    load_str_field_with_default(3, manufacturer_str, BUILD_APPNAME);
    set_field_with_default(3, type, 0x01); /* other */

    load_str_field_or_skip(3, version_str);
    load_str_field_or_skip(3, serial_ସଂଖ୍ୟା_str);
    load_str_field_or_skip(3, asset_tag_ସଂଖ୍ୟା_str);

    set_field_with_default(3, boot_up_state, 0x03); /* safe */
    set_field_with_default(3, power_supply_state, 0x03); /* safe */
    set_field_with_default(3, thermal_state, 0x03); /* safe */
    set_field_with_default(3, security_status, 0x02); /* unknown */

    set_field_with_default(3, oem_defined, 0);
    set_field_with_default(3, height, 0);
    set_field_with_default(3, ସଂଖ୍ୟା_of_power_cords, 0);
    set_field_with_default(3, contained_element_ଗଣନା, 0);

    *end = 0;
    end++;
    ଯଦି (!str_index) {
        *end = 0;
        end++;
    }

    ଫେରନ୍ତୁ end;
}

/* Type 4 -- Processor Information */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_4(ଶୂନ୍ୟ *start, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu_ସଂଖ୍ୟା)
{
    ସଂରଚନା smbios_type_4 *p = (ସଂରଚନା smbios_type_4 *)start;
    ଅକ୍ଷର *end = (ଅକ୍ଷର *)start + ଆକାର(ସଂରଚନା smbios_type_4);
    size_t size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_index = 0;
    ଅକ୍ଷର name[1024];

    p->header.type = 4;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_4);
    p->header.handle = 0x400 + cpu_ସଂଖ୍ୟା;

    size = get_field(4, offsetof(ସଂରଚନା smbios_type_4, socket_designation_str),
                     name);
    ଯଦି (size)
        snprintf(name + size - 1, ଆକାର(name) - size, "%2x", cpu_ସଂଖ୍ୟା);
    ଅନ୍ୟଥା
        snprintf(name, ଆକାର(name), "CPU%2x", cpu_ସଂଖ୍ୟା);

    memcpy(end, name, strlen(name) + 1);
    end += strlen(name) + 1;
    p->socket_designation_str = ++str_index;

    set_field_with_default(4, processor_type, 0x03); /* CPU */
    set_field_with_default(4, processor_family, 0x01); /* other */

    load_str_field_with_default(4, processor_manufacturer_str, BUILD_APPNAME);

    ଯଦି (!get_field(4, offsetof(ସଂରଚନା smbios_type_4, processor_id)
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
    ଯଦି (!str_index) {
        *end = 0;
        end++;
    }

    ଫେରନ୍ତୁ end;
}

/* Type 16 -- Physical Memory Array */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_16(ଶୂନ୍ୟ *start, u32 memory_size_mb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nr_mem_devs)
{
    ସଂରଚନା smbios_type_16 *p = (ସଂରଚନା smbios_type_16*)start;

    p->header.type = 16;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_16);
    p->header.handle = 0x1000;

    set_field_with_default(16, location, 0x01); /* other */
    set_field_with_default(16, use, 0x03); /* system memory */
    /* Multi-bit ECC to make Microsoft happy */
    set_field_with_default(16, error_correction, 0x06);
    /* 0x80000000 = unknown, accept sizes < 2TB - TODO multiple arrays */
    p->maximum_capacity = memory_size_mb < 2 << 20 ?
                          memory_size_mb << 10 : 0x80000000;
    p->memory_error_information_handle = 0xfffe; /* none provided */
    p->ସଂଖ୍ୟା_of_memory_devices = nr_mem_devs;

    start += ଆକାର(ସଂରଚନା smbios_type_16);
    *((u16 *)start) = 0;

    ଫେରନ୍ତୁ start + 2;
}

/* Type 17 -- Memory Device */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_17(ଶୂନ୍ୟ *start, u32 size_mb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance)
{
    ସଂରଚନା smbios_type_17 *p = (ସଂରଚନା smbios_type_17 *)start;
    ଅକ୍ଷର *end = (ଅକ୍ଷର *)start + ଆକାର(ସଂରଚନା smbios_type_17);
    size_t size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_index = 0;
    ଅକ୍ଷର name[1024];

    p->header.type = 17;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_17);
    p->header.handle = 0x1100 + instance;

    p->physical_memory_array_handle = 0x1000;
    set_field_with_default(17, total_width, 64);
    set_field_with_default(17, data_width, 64);
/* TODO: should assert in case something is wrong   ASSERT((memory_size_mb & ~0x7fff) == 0); */
    p->size = size_mb;
    set_field_with_default(17, form_factor, 0x09); /* DIMM */
    p->device_set = 0;

    size = get_field(17, offsetof(ସଂରଚନା smbios_type_17, device_locator_str),
                     name);
    ଯଦି (size)
        snprintf(name + size - 1, ଆକାର(name) - size, "%d", instance);
    ଅନ୍ୟଥା
        snprintf(name, ଆକାର(name), "DIMM %d", instance);

    memcpy(end, name, strlen(name) + 1);
    end += strlen(name) + 1;
    p->device_locator_str = ++str_index;

    load_str_field_or_skip(17, bank_locator_str);
    set_field_with_default(17, memory_type, 0x07); /* RAM */
    set_field_with_default(17, type_detail, 0);

    *end = 0;
    end++;
    ଯଦି (!str_index) {
        *end = 0;
        end++;
    }

    ଫେରନ୍ତୁ end;
}

/* Type 19 -- Memory Array Mapped Address */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_19(ଶୂନ୍ୟ *start, u32 start_mb, u32 size_mb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance)
{
    ସଂରଚନା smbios_type_19 *p = (ସଂରଚନା smbios_type_19 *)start;

    p->header.type = 19;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_19);
    p->header.handle = 0x1300 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (size_mb << 10) - 1;
    p->memory_array_handle = 0x1000;
    p->partition_width = 1;

    start += ଆକାର(ସଂରଚନା smbios_type_19);
    *((u16 *)start) = 0;

    ଫେରନ୍ତୁ start + 2;
}

/* Type 20 -- Memory Device Mapped Address */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_20(ଶୂନ୍ୟ *start, u32 start_mb, u32 size_mb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance,
                    ପୂର୍ଣ୍ଣସଂଖ୍ୟା dev_handle, ପୂର୍ଣ୍ଣସଂଖ୍ୟା array_handle)
{
    ସଂରଚନା smbios_type_20 *p = (ସଂରଚନା smbios_type_20 *)start;

    p->header.type = 20;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_20);
    p->header.handle = 0x1400 + instance;

    p->starting_address = start_mb << 10;
    p->ending_address = p->starting_address + (size_mb << 10) - 1;
    p->memory_device_handle = 0x1100 + dev_handle;
    p->memory_array_mapped_address_handle = 0x1300 + array_handle;
    p->partition_row_position = 1;
    p->interleave_position = 0;
    p->interleaved_data_depth = 0;

    start += ଆକାର(ସଂରଚନା smbios_type_20);

    *((u16 *)start) = 0;
    ଫେରନ୍ତୁ start+2;
}

/* Type 32 -- System Boot Information */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_32(ଶୂନ୍ୟ *start)
{
    ସଂରଚନା smbios_type_32 *p = (ସଂରଚନା smbios_type_32 *)start;

    p->header.type = 32;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_32);
    p->header.handle = 0x2000;
    memset(p->reserved, 0, 6);
    set_field_with_default(32, boot_status, 0); /* no errors detected */

    start += ଆକାର(ସଂରଚନା smbios_type_32);
    *((u16 *)start) = 0;

    ଫେରନ୍ତୁ start+2;
}

/* Type 127 -- End of Table */
ସ୍ଥିର ଶୂନ୍ୟ *
smbios_init_type_127(ଶୂନ୍ୟ *start)
{
    ସଂରଚନା smbios_type_127 *p = (ସଂରଚନା smbios_type_127 *)start;

    p->header.type = 127;
    p->header.length = ଆକାର(ସଂରଚନା smbios_type_127);
    p->header.handle = 0x7f00;

    start += ଆକାର(ସଂରଚନା smbios_type_127);
    *((u16 *)start) = 0;

    ଫେରନ୍ତୁ start + 2;
}

#define TEMPSMBIOSSIZE (32 * 1024)

ଶୂନ୍ୟ
smbios_legacy_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_SMBIOS)
        ଫେରନ୍ତୁ;

    dprintf(3, "init SMBIOS tables\n");

    ଅକ୍ଷର *start = malloc_tmphigh(TEMPSMBIOSSIZE);
    ଯଦି (! start) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(start, 0, TEMPSMBIOSSIZE);

    u32 nr_structs = 0, max_struct_size = 0;
    ଅକ୍ଷର *q, *p = start;
    ଅକ୍ଷର *end = start + TEMPSMBIOSSIZE - ଆକାର(ସଂରଚନା smbios_type_127);

#define add_struct(type, args...)                                       \
    କର {                                                                \
        ଯଦି (!get_external(type, &p, &nr_structs, &max_struct_size, end)) { \
            q = smbios_init_type_##type(args);                          \
            nr_structs++;                                               \
            ଯଦି ((q - p) > max_struct_size)                              \
                max_struct_size = q - p;                                \
            p = q;                                                      \
        }                                                               \
    } ଯେତେବେଳେ (0)

    add_struct(0, p);
    add_struct(1, p);
    add_struct(3, p);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu_ଅଙ୍କ;
    ପାଇଁ (cpu_ଅଙ୍କ = 1; cpu_ଅଙ୍କ <= Max_ଗଣନା_CPUs; cpu_ଅଙ୍କ++)
        add_struct(4, p, cpu_ଅଙ୍କ);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ram_mb = (RamSize + RamSizeOver4G) >> 20;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nr_mem_devs = (ram_mb + 0x3fff) >> 14;
    add_struct(16, p, ram_mb, nr_mem_devs);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
    ପାଇଁ (i = 0; i < nr_mem_devs; i++) {
        u32 dev_mb = ((i == (nr_mem_devs - 1))
                      ? (((ram_mb - 1) & 0x3fff) + 1)
                      : 16384);
        add_struct(17, p, dev_mb, i);
    }

    add_struct(19, p, 0, RamSize >> 20, 0);
    ଯଦି (RamSizeOver4G)
        add_struct(19, p, 4096, RamSizeOver4G >> 20, 1);

    add_struct(20, p, 0, RamSize >> 20, 0, 0, 0);
    ଯଦି (RamSizeOver4G) {
        u32 start_mb = 4096;
        ପାଇଁ (j = 1, i = 0; i < nr_mem_devs; i++, j++) {
            u32 dev_mb = ((i == (nr_mem_devs - 1))
                               ? (((ram_mb - 1) & 0x3fff) + 1)
                               : 16384);
            ଯଦି (i == 0)
                dev_mb -= RamSize >> 20;

            add_struct(20, p, start_mb, dev_mb, j, i, 1);
            start_mb += dev_mb;
        }
    }

    add_struct(32, p);
    /* Add any remaining provided entries before the end marker */
    ପାଇଁ (i = 0; i < 256; i++)
        get_external(i, &p, &nr_structs, &max_struct_size, end);
    add_struct(127, p);

#undef add_struct

    smbios_21_entry_point_setup(max_struct_size, p - start, start, nr_structs);
    ମୁକ୍ତ(start);
}
