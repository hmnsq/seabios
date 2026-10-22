#यदि_अपरिभाषितम् __SMBIOS_H
#परिभाषय __SMBIOS_H

#अन्तर्भाव "types.h" // u32

#परिभाषय SMBIOS_21_SIGNATURE 0x5f4d535f // "_SM_"

/* SMBIOS entry point -- must be written to a 16-bit aligned address
   between 0xf0000 and 0xfffff.
 */
संरचना smbios_21_प्रविष्टिः_point {
    u32 signature;
    u8 checksum;
    u8 दीर्घता;
    u8 smbios_major_संस्करणम्;
    u8 smbios_minor_संस्करणम्;
    u16 max_structure_आकार;
    u8 प्रविष्टिः_point_revision;
    u8 formatted_area[5];
    वर्ण intermediate_anchor_string[5];
    u8 intermediate_checksum;
    u16 structure_सारणी_दीर्घता;
    u32 structure_सारणी_स्थानाङ्कः;
    u16 सङ्ख्या_सम्बन्धि_structures;
    u8 smbios_bcd_revision;
} PACKED;

संरचना smbios_30_प्रविष्टिः_point {
    वर्ण signature[5];
    u8 checksum;
    u8 दीर्घता;
    u8 smbios_major_संस्करणम्;
    u8 smbios_minor_संस्करणम्;
    u8 smbios_docrev;
    u8 प्रविष्टिः_point_revision;
    u8 reserved;
    u32 structure_सारणी_max_आकार;
    u64 structure_सारणी_स्थानाङ्कः;
} PACKED;

/* This goes at the beginning of every SMBIOS structure. */
संरचना smbios_structure_शीर्षकम् {
    u8 type;
    u8 दीर्घता;
    u16 handle;
} PACKED;

/* SMBIOS type 0 - BIOS Information */
संरचना smbios_type_0 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u8 vendor_str;
    u8 bios_संस्करणम्_str;
    u16 bios_starting_स्थानाङ्कः_segment;
    u8 bios_release_date_str;
    u8 bios_rom_आकार;
    u8 bios_characteristics[8];
    u8 bios_characteristics_extension_अष्टकानि[2];
    u8 system_bios_major_release;
    u8 system_bios_minor_release;
    u8 embedded_controller_major_release;
    u8 embedded_controller_minor_release;
} PACKED;

/* SMBIOS type 1 - System Information */
संरचना smbios_type_1 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u8 manufacturer_str;
    u8 गुणनफलम्_name_str;
    u8 संस्करणम्_str;
    u8 serial_सङ्ख्या_str;
    u8 uuid[16];
    u8 wake_ऊर्ध्वम्_type;
    u8 sku_सङ्ख्या_str;
    u8 family_str;
} PACKED;

/* SMBIOS type 3 - System Enclosure (v2.3) */
संरचना smbios_type_3 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u8 manufacturer_str;
    u8 type;
    u8 संस्करणम्_str;
    u8 serial_सङ्ख्या_str;
    u8 asset_चिह्नकम्_सङ्ख्या_str;
    u8 boot_ऊर्ध्वम्_state;
    u8 power_supply_state;
    u8 thermal_state;
    u8 security_स्थितिः;
    u32 oem_defined;
    u8 height;
    u8 सङ्ख्या_सम्बन्धि_power_cords;
    u8 contained_element_गणनम्;
    // contained elements follow
} PACKED;

/* SMBIOS type 4 - Processor Information (v2.0) */
संरचना smbios_type_4 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u8 socket_designation_str;
    u8 processor_type;
    u8 processor_family;
    u8 processor_manufacturer_str;
    u32 processor_id[2];
    u8 processor_संस्करणम्_str;
    u8 voltage;
    u16 external_clock;
    u16 max_जवः;
    u16 current_जवः;
    u8 स्थितिः;
    u8 processor_upgrade;
    u16 l1_cache_handle;
    u16 l2_cache_handle;
    u16 l3_cache_handle;
} PACKED;

/* SMBIOS type 16 - Physical Memory Array
 *   Associated with one type 17 (Memory Device).
 */
संरचना smbios_type_16 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u8 location;
    u8 use;
    u8 error_correction;
    u32 अधिकतमम्_क्षमता;
    u16 स्मृतिः_error_information_handle;
    u16 सङ्ख्या_सम्बन्धि_स्मृतिः_devices;
} PACKED;

/* SMBIOS type 17 - Memory Device
 *   Associated with one type 19
 */
संरचना smbios_type_17 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u16 physical_स्मृतिः_array_handle;
    u16 स्मृतिः_error_information_handle;
    u16 योग_विस्तारः;
    u16 दत्त_विस्तारः;
    u16 आकार;
    u8 form_गुणकः;
    u8 device_समुच्चयः;
    u8 device_locator_str;
    u8 bank_locator_str;
    u8 स्मृतिः_type;
    u16 type_detail;
} PACKED;

/* SMBIOS type 19 - Memory Array Mapped Address */
संरचना smbios_type_19 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u32 starting_स्थानाङ्कः;
    u32 ending_स्थानाङ्कः;
    u16 स्मृतिः_array_handle;
    u8 partition_विस्तारः;
} PACKED;

/* SMBIOS type 20 - Memory Device Mapped Address */
संरचना smbios_type_20 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u32 starting_स्थानाङ्कः;
    u32 ending_स्थानाङ्कः;
    u16 स्मृतिः_device_handle;
    u16 स्मृतिः_array_mapped_स्थानाङ्कः_handle;
    u8 partition_row_position;
    u8 interleave_position;
    u8 interleaved_दत्त_गभीरता;
} PACKED;

/* SMBIOS type 32 - System Boot Information */
संरचना smbios_type_32 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
    u8 reserved[6];
    u8 boot_स्थितिः;
} PACKED;

/* SMBIOS type 127 -- End-of-table */
संरचना smbios_type_127 {
    संरचना smbios_structure_शीर्षकम् शीर्षकम्;
} PACKED;

#यदि_अन्तः // smbios.h
