#اگر_غیر_تعریف_شدہ __SMBIOS_H
#تعریف_کرو __SMBIOS_H

#شامل "types.h" // u32

#تعریف_کرو SMBIOS_21_SIGNATURE 0x5f4d535f // "_SM_"

/* SMBIOS entry point -- must be written to a 16-bit aligned address
   between 0xf0000 and 0xfffff.
 */
ساخت smbios_21_اندراج_نقطہ {
    u32 signature;
    u8 checksum;
    u8 لمبائی;
    u8 smbios_major_نسخہ;
    u8 smbios_minor_نسخہ;
    u16 max_structure_سائز;
    u8 اندراج_نقطہ_revision;
    u8 formatted_area[5];
    حرف intermediate_anchor_string[5];
    u8 intermediate_checksum;
    u16 structure_جدول_لمبائی;
    u32 structure_جدول_پتہ;
    u16 نمبر_کا_structures;
    u8 smbios_bcd_revision;
} PACKED;

ساخت smbios_30_اندراج_نقطہ {
    حرف signature[5];
    u8 checksum;
    u8 لمبائی;
    u8 smbios_major_نسخہ;
    u8 smbios_minor_نسخہ;
    u8 smbios_docrev;
    u8 اندراج_نقطہ_revision;
    u8 reserved;
    u32 structure_جدول_max_سائز;
    u64 structure_جدول_پتہ;
} PACKED;

/* This goes at the beginning of every SMBIOS structure. */
ساخت smbios_structure_سرخی {
    u8 type;
    u8 لمبائی;
    u16 handle;
} PACKED;

/* SMBIOS type 0 - BIOS Information */
ساخت smbios_type_0 {
    ساخت smbios_structure_سرخی سرخی;
    u8 vendor_str;
    u8 bios_نسخہ_str;
    u16 bios_starting_پتہ_segment;
    u8 bios_release_date_str;
    u8 bios_rom_سائز;
    u8 bios_characteristics[8];
    u8 bios_characteristics_extension_بائٹیں[2];
    u8 system_bios_major_release;
    u8 system_bios_minor_release;
    u8 embedded_controller_major_release;
    u8 embedded_controller_minor_release;
} PACKED;

/* SMBIOS type 1 - System Information */
ساخت smbios_type_1 {
    ساخت smbios_structure_سرخی سرخی;
    u8 manufacturer_str;
    u8 حاصل_ضرب_name_str;
    u8 نسخہ_str;
    u8 serial_نمبر_str;
    u8 uuid[16];
    u8 wake_اوپر_type;
    u8 sku_نمبر_str;
    u8 family_str;
} PACKED;

/* SMBIOS type 3 - System Enclosure (v2.3) */
ساخت smbios_type_3 {
    ساخت smbios_structure_سرخی سرخی;
    u8 manufacturer_str;
    u8 type;
    u8 نسخہ_str;
    u8 serial_نمبر_str;
    u8 asset_لیبل_نمبر_str;
    u8 boot_اوپر_state;
    u8 power_supply_state;
    u8 thermal_state;
    u8 security_حالت;
    u32 oem_defined;
    u8 اونچائی;
    u8 نمبر_کا_power_cords;
    u8 contained_element_گنتی;
    // contained elements follow
} PACKED;

/* SMBIOS type 4 - Processor Information (v2.0) */
ساخت smbios_type_4 {
    ساخت smbios_structure_سرخی سرخی;
    u8 socket_designation_str;
    u8 processor_type;
    u8 processor_family;
    u8 processor_manufacturer_str;
    u32 processor_id[2];
    u8 processor_نسخہ_str;
    u8 voltage;
    u16 external_clock;
    u16 max_رفتار;
    u16 current_رفتار;
    u8 حالت_2;
    u8 processor_upgrade;
    u16 l1_cache_handle;
    u16 l2_cache_handle;
    u16 l3_cache_handle;
} PACKED;

/* SMBIOS type 16 - Physical Memory Array
 *   Associated with one type 17 (Memory Device).
 */
ساخت smbios_type_16 {
    ساخت smbios_structure_سرخی سرخی;
    u8 location;
    u8 use;
    u8 error_correction;
    u32 زیادہ_سے_زیادہ_استعداد;
    u16 حافظہ_error_information_handle;
    u16 نمبر_کا_حافظہ_devices;
} PACKED;

/* SMBIOS type 17 - Memory Device
 *   Associated with one type 19
 */
ساخت smbios_type_17 {
    ساخت smbios_structure_سرخی سرخی;
    u16 physical_حافظہ_array_handle;
    u16 حافظہ_error_information_handle;
    u16 کل_چوڑائی;
    u16 ڈیٹا_چوڑائی;
    u16 سائز;
    u8 form_ضربی_عدد;
    u8 device_سیٹ;
    u8 device_locator_str;
    u8 bank_locator_str;
    u8 حافظہ_type;
    u16 type_detail;
} PACKED;

/* SMBIOS type 19 - Memory Array Mapped Address */
ساخت smbios_type_19 {
    ساخت smbios_structure_سرخی سرخی;
    u32 starting_پتہ;
    u32 ending_پتہ;
    u16 حافظہ_array_handle;
    u8 تقسیم_چوڑائی;
} PACKED;

/* SMBIOS type 20 - Memory Device Mapped Address */
ساخت smbios_type_20 {
    ساخت smbios_structure_سرخی سرخی;
    u32 starting_پتہ;
    u32 ending_پتہ;
    u16 حافظہ_device_handle;
    u16 حافظہ_array_mapped_پتہ_handle;
    u8 تقسیم_صف_position;
    u8 interleave_position;
    u8 interleaved_ڈیٹا_گہرائی;
} PACKED;

/* SMBIOS type 32 - System Boot Information */
ساخت smbios_type_32 {
    ساخت smbios_structure_سرخی سرخی;
    u8 reserved[6];
    u8 boot_حالت;
} PACKED;

/* SMBIOS type 127 -- End-of-table */
ساخت smbios_type_127 {
    ساخت smbios_structure_سرخی سرخی;
} PACKED;

#اگر_ختم // smbios.h
