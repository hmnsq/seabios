#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __SMBIOS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __SMBIOS_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMBIOS_21_SIGNATURE 0x5f4d535f // "_SM_"

/* SMBIOS entry point -- must be written to a 16-bit aligned address
   between 0xf0000 and 0xfffff.
 */
ਬਣਤਰ smbios_21_ਇੰਦਰਾਜ_ਬਿੰਦੂ {
    u32 signature;
    u8 checksum;
    u8 ਲੰਮਾਈ;
    u8 smbios_major_ਸੰਸਕਰਣ;
    u8 smbios_minor_ਸੰਸਕਰਣ;
    u16 max_structure_ਮਾਪ;
    u8 ਇੰਦਰਾਜ_ਬਿੰਦੂ_revision;
    u8 formatted_area[5];
    ਅੱਖਰ intermediate_anchor_string[5];
    u8 intermediate_checksum;
    u16 structure_ਸਾਰਣੀ_ਲੰਮਾਈ;
    u32 structure_ਸਾਰਣੀ_ਪਤਾ;
    u16 ਸੰਖਿਆ_ਦਾ_structures;
    u8 smbios_bcd_revision;
} PACKED;

ਬਣਤਰ smbios_30_ਇੰਦਰਾਜ_ਬਿੰਦੂ {
    ਅੱਖਰ signature[5];
    u8 checksum;
    u8 ਲੰਮਾਈ;
    u8 smbios_major_ਸੰਸਕਰਣ;
    u8 smbios_minor_ਸੰਸਕਰਣ;
    u8 smbios_docrev;
    u8 ਇੰਦਰਾਜ_ਬਿੰਦੂ_revision;
    u8 reserved;
    u32 structure_ਸਾਰਣੀ_max_ਮਾਪ;
    u64 structure_ਸਾਰਣੀ_ਪਤਾ;
} PACKED;

/* This goes at the beginning of every SMBIOS structure. */
ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ {
    u8 type;
    u8 ਲੰਮਾਈ;
    u16 handle;
} PACKED;

/* SMBIOS type 0 - BIOS Information */
ਬਣਤਰ smbios_type_0 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u8 vendor_str;
    u8 bios_ਸੰਸਕਰਣ_str;
    u16 bios_starting_ਪਤਾ_segment;
    u8 bios_release_date_str;
    u8 bios_rom_ਮਾਪ;
    u8 bios_characteristics[8];
    u8 bios_characteristics_extension_ਬਾਈਟਾਂ[2];
    u8 system_bios_major_release;
    u8 system_bios_minor_release;
    u8 embedded_controller_major_release;
    u8 embedded_controller_minor_release;
} PACKED;

/* SMBIOS type 1 - System Information */
ਬਣਤਰ smbios_type_1 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u8 manufacturer_str;
    u8 ਗੁਣਨਫਲ_name_str;
    u8 ਸੰਸਕਰਣ_str;
    u8 serial_ਸੰਖਿਆ_str;
    u8 uuid[16];
    u8 wake_ਉੱਪਰ_type;
    u8 sku_ਸੰਖਿਆ_str;
    u8 family_str;
} PACKED;

/* SMBIOS type 3 - System Enclosure (v2.3) */
ਬਣਤਰ smbios_type_3 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u8 manufacturer_str;
    u8 type;
    u8 ਸੰਸਕਰਣ_str;
    u8 serial_ਸੰਖਿਆ_str;
    u8 asset_ਟੈਗ_ਸੰਖਿਆ_str;
    u8 boot_ਉੱਪਰ_state;
    u8 power_supply_state;
    u8 thermal_state;
    u8 security_ਸਥਿਤੀ;
    u32 oem_defined;
    u8 ਉਚਾਈ;
    u8 ਸੰਖਿਆ_ਦਾ_power_cords;
    u8 contained_element_ਗਿਣਤੀ;
    // contained elements follow
} PACKED;

/* SMBIOS type 4 - Processor Information (v2.0) */
ਬਣਤਰ smbios_type_4 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u8 socket_designation_str;
    u8 processor_type;
    u8 processor_family;
    u8 processor_manufacturer_str;
    u32 processor_id[2];
    u8 processor_ਸੰਸਕਰਣ_str;
    u8 voltage;
    u16 external_clock;
    u16 max_ਰਫਤਾਰ;
    u16 current_ਰਫਤਾਰ;
    u8 ਸਥਿਤੀ;
    u8 processor_upgrade;
    u16 l1_cache_handle;
    u16 l2_cache_handle;
    u16 l3_cache_handle;
} PACKED;

/* SMBIOS type 16 - Physical Memory Array
 *   Associated with one type 17 (Memory Device).
 */
ਬਣਤਰ smbios_type_16 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u8 location;
    u8 use;
    u8 error_correction;
    u32 ਵੱਧ_ਤੋਂ_ਵੱਧ_ਸਮਰੱਥਾ;
    u16 ਸਮ੍ਰਿਤੀ_error_information_handle;
    u16 ਸੰਖਿਆ_ਦਾ_ਸਮ੍ਰਿਤੀ_devices;
} PACKED;

/* SMBIOS type 17 - Memory Device
 *   Associated with one type 19
 */
ਬਣਤਰ smbios_type_17 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u16 physical_ਸਮ੍ਰਿਤੀ_array_handle;
    u16 ਸਮ੍ਰਿਤੀ_error_information_handle;
    u16 ਕੁੱਲ_ਚੌੜਾਈ;
    u16 ਅੰਕੜੇ_ਚੌੜਾਈ;
    u16 ਮਾਪ;
    u8 form_ਗੁਣਕ;
    u8 device_ਸੈੱਟ;
    u8 device_locator_str;
    u8 bank_locator_str;
    u8 ਸਮ੍ਰਿਤੀ_type;
    u16 type_detail;
} PACKED;

/* SMBIOS type 19 - Memory Array Mapped Address */
ਬਣਤਰ smbios_type_19 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u32 starting_ਪਤਾ;
    u32 ending_ਪਤਾ;
    u16 ਸਮ੍ਰਿਤੀ_array_handle;
    u8 ਹਿੱਸੇ_ਕਰੋ_ਚੌੜਾਈ;
} PACKED;

/* SMBIOS type 20 - Memory Device Mapped Address */
ਬਣਤਰ smbios_type_20 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u32 starting_ਪਤਾ;
    u32 ending_ਪਤਾ;
    u16 ਸਮ੍ਰਿਤੀ_device_handle;
    u16 ਸਮ੍ਰਿਤੀ_array_mapped_ਪਤਾ_handle;
    u8 ਹਿੱਸੇ_ਕਰੋ_ਪੰਕਤੀ_position;
    u8 interleave_position;
    u8 interleaved_ਅੰਕੜੇ_ਡੂੰਘਾਈ;
} PACKED;

/* SMBIOS type 32 - System Boot Information */
ਬਣਤਰ smbios_type_32 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
    u8 reserved[6];
    u8 boot_ਸਥਿਤੀ;
} PACKED;

/* SMBIOS type 127 -- End-of-table */
ਬਣਤਰ smbios_type_127 {
    ਬਣਤਰ smbios_structure_ਸਿਰਲੇਖ ਸਿਰਲੇਖ;
} PACKED;

#ਜੇ_ਅੰਤ // smbios.h
