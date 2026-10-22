#جيڪڏهن_وصف_نه_ٿيل __SMBIOS_H
#وصف_ڪريو __SMBIOS_H

#ضمن "types.h" // u32

#وصف_ڪريو SMBIOS_21_SIGNATURE 0x5f4d535f // "_SM_"

/* SMBIOS entry point -- must be written to a 16-bit aligned address
   between 0xf0000 and 0xfffff.
 */
بنية smbios_21_داخلا_نقطو {
    u32 signature;
    u8 checksum;
    u8 ڊيگهائي;
    u8 smbios_major_نسخو;
    u8 smbios_minor_نسخو;
    u16 max_structure_سائيز;
    u8 داخلا_نقطو_revision;
    u8 formatted_area[5];
    حرف intermediate_anchor_string[5];
    u8 intermediate_checksum;
    u16 structure_جدول_ڊيگهائي;
    u32 structure_جدول_پتو;
    u16 عدد_جو_structures;
    u8 smbios_bcd_revision;
} PACKED;

بنية smbios_30_داخلا_نقطو {
    حرف signature[5];
    u8 checksum;
    u8 ڊيگهائي;
    u8 smbios_major_نسخو;
    u8 smbios_minor_نسخو;
    u8 smbios_docrev;
    u8 داخلا_نقطو_revision;
    u8 reserved;
    u32 structure_جدول_max_سائيز;
    u64 structure_جدول_پتو;
} PACKED;

/* This goes at the beginning of every SMBIOS structure. */
بنية smbios_structure_سرو {
    u8 type;
    u8 ڊيگهائي;
    u16 handle;
} PACKED;

/* SMBIOS type 0 - BIOS Information */
بنية smbios_type_0 {
    بنية smbios_structure_سرو سرو;
    u8 vendor_str;
    u8 bios_نسخو_str;
    u16 bios_starting_پتو_segment;
    u8 bios_release_date_str;
    u8 bios_rom_سائيز;
    u8 bios_characteristics[8];
    u8 bios_characteristics_extension_بائيٽون[2];
    u8 system_bios_major_release;
    u8 system_bios_minor_release;
    u8 embedded_controller_major_release;
    u8 embedded_controller_minor_release;
} PACKED;

/* SMBIOS type 1 - System Information */
بنية smbios_type_1 {
    بنية smbios_structure_سرو سرو;
    u8 manufacturer_str;
    u8 حاصل_ضرب_name_str;
    u8 نسخو_str;
    u8 serial_عدد_str;
    u8 uuid[16];
    u8 wake_مٿي_type;
    u8 sku_عدد_str;
    u8 family_str;
} PACKED;

/* SMBIOS type 3 - System Enclosure (v2.3) */
بنية smbios_type_3 {
    بنية smbios_structure_سرو سرو;
    u8 manufacturer_str;
    u8 type;
    u8 نسخو_str;
    u8 serial_عدد_str;
    u8 asset_ٽيگ_عدد_str;
    u8 boot_مٿي_state;
    u8 power_supply_state;
    u8 thermal_state;
    u8 security_حالت;
    u32 oem_defined;
    u8 اوچائي;
    u8 عدد_جو_power_cords;
    u8 contained_element_ڳڻپ;
    // contained elements follow
} PACKED;

/* SMBIOS type 4 - Processor Information (v2.0) */
بنية smbios_type_4 {
    بنية smbios_structure_سرو سرو;
    u8 socket_designation_str;
    u8 processor_type;
    u8 processor_family;
    u8 processor_manufacturer_str;
    u32 processor_id[2];
    u8 processor_نسخو_str;
    u8 voltage;
    u16 external_clock;
    u16 max_رفتار;
    u16 current_رفتار;
    u8 حالت;
    u8 processor_upgrade;
    u16 l1_cache_handle;
    u16 l2_cache_handle;
    u16 l3_cache_handle;
} PACKED;

/* SMBIOS type 16 - Physical Memory Array
 *   Associated with one type 17 (Memory Device).
 */
بنية smbios_type_16 {
    بنية smbios_structure_سرو سرو;
    u8 location;
    u8 use;
    u8 error_correction;
    u32 وڌ_کان_وڌ_صلاحيت;
    u16 يادگيري_error_information_handle;
    u16 عدد_جو_يادگيري_devices;
} PACKED;

/* SMBIOS type 17 - Memory Device
 *   Associated with one type 19
 */
بنية smbios_type_17 {
    بنية smbios_structure_سرو سرو;
    u16 physical_يادگيري_array_handle;
    u16 يادگيري_error_information_handle;
    u16 ڪل_ويڪر;
    u16 معلومات_ويڪر;
    u16 سائيز;
    u8 form_ضرب_ڪندڙ;
    u8 device_سيٽ;
    u8 device_locator_str;
    u8 bank_locator_str;
    u8 يادگيري_type;
    u16 type_detail;
} PACKED;

/* SMBIOS type 19 - Memory Array Mapped Address */
بنية smbios_type_19 {
    بنية smbios_structure_سرو سرو;
    u32 starting_پتو;
    u32 ending_پتو;
    u16 يادگيري_array_handle;
    u8 حصا_ڪريو_ويڪر;
} PACKED;

/* SMBIOS type 20 - Memory Device Mapped Address */
بنية smbios_type_20 {
    بنية smbios_structure_سرو سرو;
    u32 starting_پتو;
    u32 ending_پتو;
    u16 يادگيري_device_handle;
    u16 يادگيري_array_mapped_پتو_handle;
    u8 حصا_ڪريو_صف_position;
    u8 interleave_position;
    u8 interleaved_معلومات_اونهائي;
} PACKED;

/* SMBIOS type 32 - System Boot Information */
بنية smbios_type_32 {
    بنية smbios_structure_سرو سرو;
    u8 reserved[6];
    u8 boot_حالت;
} PACKED;

/* SMBIOS type 127 -- End-of-table */
بنية smbios_type_127 {
    بنية smbios_structure_سرو سرو;
} PACKED;

#جيڪڏهن_پورو // smbios.h
