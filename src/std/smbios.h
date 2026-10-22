#अगर_अघोषित __SMBIOS_H
#घोषणा __SMBIOS_H

#समावेश "types.h" // u32

#घोषणा SMBIOS_21_SIGNATURE 0x5f4d535f // "_SM_"

/* SMBIOS entry point -- must be written to a 16-bit aligned address
   between 0xf0000 and 0xfffff.
 */
काष्ठा smbios_21_entry_point {
    u32 signature;
    u8 checksum;
    u8 लंबाई;
    u8 smbios_major_version;
    u8 smbios_minor_version;
    u16 max_structure_आकार;
    u8 entry_point_revision;
    u8 formatted_area[5];
    अक्षर intermediate_anchor_string[5];
    u8 intermediate_checksum;
    u16 structure_table_लंबाई;
    u32 structure_table_address;
    u16 संख्या_of_structures;
    u8 smbios_bcd_revision;
} PACKED;

काष्ठा smbios_30_entry_point {
    अक्षर signature[5];
    u8 checksum;
    u8 लंबाई;
    u8 smbios_major_version;
    u8 smbios_minor_version;
    u8 smbios_docrev;
    u8 entry_point_revision;
    u8 reserved;
    u32 structure_table_max_आकार;
    u64 structure_table_address;
} PACKED;

/* This goes at the beginning of every SMBIOS structure. */
काष्ठा smbios_structure_header {
    u8 type;
    u8 लंबाई;
    u16 handle;
} PACKED;

/* SMBIOS type 0 - BIOS Information */
काष्ठा smbios_type_0 {
    काष्ठा smbios_structure_header header;
    u8 vendor_str;
    u8 bios_version_str;
    u16 bios_starting_address_segment;
    u8 bios_release_date_str;
    u8 bios_rom_आकार;
    u8 bios_characteristics[8];
    u8 bios_characteristics_extension_bytes[2];
    u8 system_bios_major_release;
    u8 system_bios_minor_release;
    u8 embedded_controller_major_release;
    u8 embedded_controller_minor_release;
} PACKED;

/* SMBIOS type 1 - System Information */
काष्ठा smbios_type_1 {
    काष्ठा smbios_structure_header header;
    u8 manufacturer_str;
    u8 product_name_str;
    u8 version_str;
    u8 serial_संख्या_str;
    u8 uuid[16];
    u8 wake_up_type;
    u8 sku_संख्या_str;
    u8 family_str;
} PACKED;

/* SMBIOS type 3 - System Enclosure (v2.3) */
काष्ठा smbios_type_3 {
    काष्ठा smbios_structure_header header;
    u8 manufacturer_str;
    u8 type;
    u8 version_str;
    u8 serial_संख्या_str;
    u8 asset_tag_संख्या_str;
    u8 boot_up_state;
    u8 power_supply_state;
    u8 thermal_state;
    u8 security_status;
    u32 oem_defined;
    u8 height;
    u8 संख्या_of_power_cords;
    u8 contained_element_गिनती;
    // contained elements follow
} PACKED;

/* SMBIOS type 4 - Processor Information (v2.0) */
काष्ठा smbios_type_4 {
    काष्ठा smbios_structure_header header;
    u8 socket_designation_str;
    u8 processor_type;
    u8 processor_family;
    u8 processor_manufacturer_str;
    u32 processor_id[2];
    u8 processor_version_str;
    u8 voltage;
    u16 external_clock;
    u16 max_speed;
    u16 current_speed;
    u8 status;
    u8 processor_upgrade;
    u16 l1_cache_handle;
    u16 l2_cache_handle;
    u16 l3_cache_handle;
} PACKED;

/* SMBIOS type 16 - Physical Memory Array
 *   Associated with one type 17 (Memory Device).
 */
काष्ठा smbios_type_16 {
    काष्ठा smbios_structure_header header;
    u8 location;
    u8 use;
    u8 error_correction;
    u32 maximum_capacity;
    u16 memory_error_information_handle;
    u16 संख्या_of_memory_devices;
} PACKED;

/* SMBIOS type 17 - Memory Device
 *   Associated with one type 19
 */
काष्ठा smbios_type_17 {
    काष्ठा smbios_structure_header header;
    u16 physical_memory_array_handle;
    u16 memory_error_information_handle;
    u16 योग_width;
    u16 तथ्य_width;
    u16 आकार;
    u8 form_factor;
    u8 device_set;
    u8 device_locator_str;
    u8 bank_locator_str;
    u8 memory_type;
    u16 type_detail;
} PACKED;

/* SMBIOS type 19 - Memory Array Mapped Address */
काष्ठा smbios_type_19 {
    काष्ठा smbios_structure_header header;
    u32 starting_address;
    u32 ending_address;
    u16 memory_array_handle;
    u8 partition_width;
} PACKED;

/* SMBIOS type 20 - Memory Device Mapped Address */
काष्ठा smbios_type_20 {
    काष्ठा smbios_structure_header header;
    u32 starting_address;
    u32 ending_address;
    u16 memory_device_handle;
    u16 memory_array_mapped_address_handle;
    u8 partition_row_position;
    u8 interleave_position;
    u8 interleaved_तथ्य_depth;
} PACKED;

/* SMBIOS type 32 - System Boot Information */
काष्ठा smbios_type_32 {
    काष्ठा smbios_structure_header header;
    u8 reserved[6];
    u8 boot_status;
} PACKED;

/* SMBIOS type 127 -- End-of-table */
काष्ठा smbios_type_127 {
    काष्ठा smbios_structure_header header;
} PACKED;

#पूर्ण_अगर // smbios.h
