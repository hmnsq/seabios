#که_نه_تعریف_شوی __SMBIOS_H
#تعریف_کړه __SMBIOS_H

#ضمن "types.h" // u32

#تعریف_کړه SMBIOS_21_SIGNATURE 0x5f4d535f // "_SM_"

/* SMBIOS entry point -- must be written to a 16-bit aligned address
   between 0xf0000 and 0xfffff.
 */
بنية smbios_21_ثبت_ټکی {
    u32 signature;
    u8 checksum;
    u8 اوږدتیا;
    u8 smbios_major_نسخه;
    u8 smbios_minor_نسخه;
    u16 max_structure_اندازه;
    u8 ثبت_ټکی_revision;
    u8 formatted_area[5];
    حرف intermediate_anchor_string[5];
    u8 intermediate_checksum;
    u16 structure_جدول_اوږدتیا;
    u32 structure_جدول_پته;
    u16 شمېره_د_structures;
    u8 smbios_bcd_revision;
} PACKED;

بنية smbios_30_ثبت_ټکی {
    حرف signature[5];
    u8 checksum;
    u8 اوږدتیا;
    u8 smbios_major_نسخه;
    u8 smbios_minor_نسخه;
    u8 smbios_docrev;
    u8 ثبت_ټکی_revision;
    u8 reserved;
    u32 structure_جدول_max_اندازه;
    u64 structure_جدول_پته;
} PACKED;

/* This goes at the beginning of every SMBIOS structure. */
بنية smbios_structure_سرلیک {
    u8 type;
    u8 اوږدتیا;
    u16 handle;
} PACKED;

/* SMBIOS type 0 - BIOS Information */
بنية smbios_type_0 {
    بنية smbios_structure_سرلیک سرلیک;
    u8 vendor_str;
    u8 bios_نسخه_str;
    u16 bios_starting_پته_segment;
    u8 bios_release_date_str;
    u8 bios_rom_اندازه;
    u8 bios_characteristics[8];
    u8 bios_characteristics_extension_بایټونه[2];
    u8 system_bios_major_release;
    u8 system_bios_minor_release;
    u8 embedded_controller_major_release;
    u8 embedded_controller_minor_release;
} PACKED;

/* SMBIOS type 1 - System Information */
بنية smbios_type_1 {
    بنية smbios_structure_سرلیک سرلیک;
    u8 manufacturer_str;
    u8 حاصل_ضرب_name_str;
    u8 نسخه_str;
    u8 serial_شمېره_str;
    u8 uuid[16];
    u8 wake_پورته_type;
    u8 sku_شمېره_str;
    u8 family_str;
} PACKED;

/* SMBIOS type 3 - System Enclosure (v2.3) */
بنية smbios_type_3 {
    بنية smbios_structure_سرلیک سرلیک;
    u8 manufacturer_str;
    u8 type;
    u8 نسخه_str;
    u8 serial_شمېره_str;
    u8 asset_نښان_شمېره_str;
    u8 boot_پورته_state;
    u8 power_supply_state;
    u8 thermal_state;
    u8 security_حالت;
    u32 oem_defined;
    u8 لوړوالی;
    u8 شمېره_د_power_cords;
    u8 contained_element_شمېر;
    // contained elements follow
} PACKED;

/* SMBIOS type 4 - Processor Information (v2.0) */
بنية smbios_type_4 {
    بنية smbios_structure_سرلیک سرلیک;
    u8 socket_designation_str;
    u8 processor_type;
    u8 processor_family;
    u8 processor_manufacturer_str;
    u32 processor_id[2];
    u8 processor_نسخه_str;
    u8 voltage;
    u16 external_clock;
    u16 max_چټکتیا;
    u16 current_چټکتیا;
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
    بنية smbios_structure_سرلیک سرلیک;
    u8 location;
    u8 use;
    u8 error_correction;
    u32 اعظمي_ظرفیت;
    u16 حافظه_error_information_handle;
    u16 شمېره_د_حافظه_devices;
} PACKED;

/* SMBIOS type 17 - Memory Device
 *   Associated with one type 19
 */
بنية smbios_type_17 {
    بنية smbios_structure_سرلیک سرلیک;
    u16 physical_حافظه_array_handle;
    u16 حافظه_error_information_handle;
    u16 ټول_پلنوالی;
    u16 معلومات_پلنوالی;
    u16 اندازه;
    u8 form_ضریب;
    u8 device_سېټ;
    u8 device_locator_str;
    u8 bank_locator_str;
    u8 حافظه_type;
    u16 type_detail;
} PACKED;

/* SMBIOS type 19 - Memory Array Mapped Address */
بنية smbios_type_19 {
    بنية smbios_structure_سرلیک سرلیک;
    u32 starting_پته;
    u32 ending_پته;
    u16 حافظه_array_handle;
    u8 برخه_کول_پلنوالی;
} PACKED;

/* SMBIOS type 20 - Memory Device Mapped Address */
بنية smbios_type_20 {
    بنية smbios_structure_سرلیک سرلیک;
    u32 starting_پته;
    u32 ending_پته;
    u16 حافظه_device_handle;
    u16 حافظه_array_mapped_پته_handle;
    u8 برخه_کول_لیکه_position;
    u8 interleave_position;
    u8 interleaved_معلومات_ژوروالی;
} PACKED;

/* SMBIOS type 32 - System Boot Information */
بنية smbios_type_32 {
    بنية smbios_structure_سرلیک سرلیک;
    u8 reserved[6];
    u8 boot_حالت;
} PACKED;

/* SMBIOS type 127 -- End-of-table */
بنية smbios_type_127 {
    بنية smbios_structure_سرلیک سرلیک;
} PACKED;

#د_که_پای // smbios.h
