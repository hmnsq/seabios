#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __SMBIOS_H
#ವ್ಯಾಖ್ಯಾನಿಸು __SMBIOS_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

#ವ್ಯಾಖ್ಯಾನಿಸು SMBIOS_21_SIGNATURE 0x5f4d535f // "_SM_"

/* SMBIOS entry point -- must be written to a 16-bit aligned address
   between 0xf0000 and 0xfffff.
 */
ರಚನೆ smbios_21_ನಮೂದು_ಬಿಂದು {
    u32 signature;
    u8 checksum;
    u8 ಉದ್ದಳತೆ;
    u8 smbios_major_ಆವೃತ್ತಿ;
    u8 smbios_minor_ಆವೃತ್ತಿ;
    u16 max_structure_ಗಾತ್ರ;
    u8 ನಮೂದು_ಬಿಂದು_revision;
    u8 formatted_area[5];
    ಅಕ್ಷರ intermediate_anchor_string[5];
    u8 intermediate_checksum;
    u16 structure_ಕೋಷ್ಟಕ_ಉದ್ದಳತೆ;
    u32 structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ;
    u16 ಸಂಖ್ಯೆ_ದ_structures;
    u8 smbios_bcd_revision;
} PACKED;

ರಚನೆ smbios_30_ನಮೂದು_ಬಿಂದು {
    ಅಕ್ಷರ signature[5];
    u8 checksum;
    u8 ಉದ್ದಳತೆ;
    u8 smbios_major_ಆವೃತ್ತಿ;
    u8 smbios_minor_ಆವೃತ್ತಿ;
    u8 smbios_docrev;
    u8 ನಮೂದು_ಬಿಂದು_revision;
    u8 reserved;
    u32 structure_ಕೋಷ್ಟಕ_max_ಗಾತ್ರ;
    u64 structure_ಕೋಷ್ಟಕ_ವಿಳಾಸ;
} PACKED;

/* This goes at the beginning of every SMBIOS structure. */
ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ {
    u8 type;
    u8 ಉದ್ದಳತೆ;
    u16 handle;
} PACKED;

/* SMBIOS type 0 - BIOS Information */
ರಚನೆ smbios_type_0 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u8 vendor_str;
    u8 bios_ಆವೃತ್ತಿ_str;
    u16 bios_starting_ವಿಳಾಸ_segment;
    u8 bios_release_date_str;
    u8 bios_rom_ಗಾತ್ರ;
    u8 bios_characteristics[8];
    u8 bios_characteristics_extension_ಬೈಟುಗಳು[2];
    u8 system_bios_major_release;
    u8 system_bios_minor_release;
    u8 embedded_controller_major_release;
    u8 embedded_controller_minor_release;
} PACKED;

/* SMBIOS type 1 - System Information */
ರಚನೆ smbios_type_1 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u8 manufacturer_str;
    u8 ಗುಣಲಬ್ಧ_name_str;
    u8 ಆವೃತ್ತಿ_str;
    u8 serial_ಸಂಖ್ಯೆ_str;
    u8 uuid[16];
    u8 wake_ಮೇಲೆ_type;
    u8 sku_ಸಂಖ್ಯೆ_str;
    u8 family_str;
} PACKED;

/* SMBIOS type 3 - System Enclosure (v2.3) */
ರಚನೆ smbios_type_3 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u8 manufacturer_str;
    u8 type;
    u8 ಆವೃತ್ತಿ_str;
    u8 serial_ಸಂಖ್ಯೆ_str;
    u8 asset_ಟ್ಯಾಗ್_ಸಂಖ್ಯೆ_str;
    u8 boot_ಮೇಲೆ_state;
    u8 power_supply_state;
    u8 thermal_state;
    u8 security_ಸ್ಥಿತಿ;
    u32 oem_defined;
    u8 ಎತ್ತರ;
    u8 ಸಂಖ್ಯೆ_ದ_power_cords;
    u8 contained_element_ಎಣಿಕೆ;
    // contained elements follow
} PACKED;

/* SMBIOS type 4 - Processor Information (v2.0) */
ರಚನೆ smbios_type_4 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u8 socket_designation_str;
    u8 processor_type;
    u8 processor_family;
    u8 processor_manufacturer_str;
    u32 processor_id[2];
    u8 processor_ಆವೃತ್ತಿ_str;
    u8 voltage;
    u16 external_clock;
    u16 max_ವೇಗ;
    u16 current_ವೇಗ;
    u8 ಸ್ಥಿತಿ_2;
    u8 processor_upgrade;
    u16 l1_cache_handle;
    u16 l2_cache_handle;
    u16 l3_cache_handle;
} PACKED;

/* SMBIOS type 16 - Physical Memory Array
 *   Associated with one type 17 (Memory Device).
 */
ರಚನೆ smbios_type_16 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u8 location;
    u8 use;
    u8 error_correction;
    u32 ಗರಿಷ್ಠ_ಸಾಮರ್ಥ್ಯ;
    u16 ಸ್ಮರಣೆ_error_information_handle;
    u16 ಸಂಖ್ಯೆ_ದ_ಸ್ಮರಣೆ_devices;
} PACKED;

/* SMBIOS type 17 - Memory Device
 *   Associated with one type 19
 */
ರಚನೆ smbios_type_17 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u16 physical_ಸ್ಮರಣೆ_array_handle;
    u16 ಸ್ಮರಣೆ_error_information_handle;
    u16 ಒಟ್ಟು_ಅಗಲ;
    u16 ದತ್ತಾಂಶ_ಅಗಲ;
    u16 ಗಾತ್ರ_2;
    u8 form_ಗುಣಕ;
    u8 device_ಗಣ;
    u8 device_locator_str;
    u8 bank_locator_str;
    u8 ಸ್ಮರಣೆ_type;
    u16 type_detail;
} PACKED;

/* SMBIOS type 19 - Memory Array Mapped Address */
ರಚನೆ smbios_type_19 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u32 starting_ವಿಳಾಸ;
    u32 ending_ವಿಳಾಸ;
    u16 ಸ್ಮರಣೆ_array_handle;
    u8 ವಿಭಜನೆ_ಅಗಲ;
} PACKED;

/* SMBIOS type 20 - Memory Device Mapped Address */
ರಚನೆ smbios_type_20 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u32 starting_ವಿಳಾಸ;
    u32 ending_ವಿಳಾಸ;
    u16 ಸ್ಮರಣೆ_device_handle;
    u16 ಸ್ಮರಣೆ_array_mapped_ವಿಳಾಸ_handle;
    u8 ವಿಭಜನೆ_ಅಡ್ಡಸಾಲು_position;
    u8 interleave_position;
    u8 interleaved_ದತ್ತಾಂಶ_ಆಳ;
} PACKED;

/* SMBIOS type 32 - System Boot Information */
ರಚನೆ smbios_type_32 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
    u8 reserved[6];
    u8 boot_ಸ್ಥಿತಿ;
} PACKED;

/* SMBIOS type 127 -- End-of-table */
ರಚನೆ smbios_type_127 {
    ರಚನೆ smbios_structure_ಶೀರ್ಷಿಕೆ ಶೀರ್ಷಿಕೆ;
} PACKED;

#ಆದರೆ_ಅಂತ್ಯ // smbios.h
