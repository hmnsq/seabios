#నిర్వచితం_కాకపోతే __SMBIOS_H
#నిర్వచించు __SMBIOS_H

#అన్తర్భావ "types.h" // u32

#నిర్వచించు SMBIOS_21_SIGNATURE 0x5f4d535f // "_SM_"

/* SMBIOS entry point -- must be written to a 16-bit aligned address
   between 0xf0000 and 0xfffff.
 */
నిర్మాణం smbios_21_నమోదు_బిందువు {
    u32 signature;
    u8 checksum;
    u8 నిడివి;
    u8 smbios_major_సంస్కరణ;
    u8 smbios_minor_సంస్కరణ;
    u16 max_structure_కొలత;
    u8 నమోదు_బిందువు_revision;
    u8 formatted_area[5];
    అక్షరం intermediate_anchor_string[5];
    u8 intermediate_checksum;
    u16 structure_పట్టిక_నిడివి;
    u32 structure_పట్టిక_చిరునామా;
    u16 సంఖ్య_యొక్క_structures;
    u8 smbios_bcd_revision;
} PACKED;

నిర్మాణం smbios_30_నమోదు_బిందువు {
    అక్షరం signature[5];
    u8 checksum;
    u8 నిడివి;
    u8 smbios_major_సంస్కరణ;
    u8 smbios_minor_సంస్కరణ;
    u8 smbios_docrev;
    u8 నమోదు_బిందువు_revision;
    u8 reserved;
    u32 structure_పట్టిక_max_కొలత;
    u64 structure_పట్టిక_చిరునామా;
} PACKED;

/* This goes at the beginning of every SMBIOS structure. */
నిర్మాణం smbios_structure_శీర్షిక {
    u8 type;
    u8 నిడివి;
    u16 handle;
} PACKED;

/* SMBIOS type 0 - BIOS Information */
నిర్మాణం smbios_type_0 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u8 vendor_str;
    u8 bios_సంస్కరణ_str;
    u16 bios_starting_చిరునామా_segment;
    u8 bios_release_date_str;
    u8 bios_rom_కొలత;
    u8 bios_characteristics[8];
    u8 bios_characteristics_extension_బైట్లు[2];
    u8 system_bios_major_release;
    u8 system_bios_minor_release;
    u8 embedded_controller_major_release;
    u8 embedded_controller_minor_release;
} PACKED;

/* SMBIOS type 1 - System Information */
నిర్మాణం smbios_type_1 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u8 manufacturer_str;
    u8 లబ్ధం_name_str;
    u8 సంస్కరణ_str;
    u8 serial_సంఖ్య_str;
    u8 uuid[16];
    u8 wake_పైకి_type;
    u8 sku_సంఖ్య_str;
    u8 family_str;
} PACKED;

/* SMBIOS type 3 - System Enclosure (v2.3) */
నిర్మాణం smbios_type_3 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u8 manufacturer_str;
    u8 type;
    u8 సంస్కరణ_str;
    u8 serial_సంఖ్య_str;
    u8 asset_ట్యాగ్_సంఖ్య_str;
    u8 boot_పైకి_state;
    u8 power_supply_state;
    u8 thermal_state;
    u8 security_స్థితి;
    u32 oem_defined;
    u8 ఎత్తు;
    u8 సంఖ్య_యొక్క_power_cords;
    u8 contained_element_లెక్క;
    // contained elements follow
} PACKED;

/* SMBIOS type 4 - Processor Information (v2.0) */
నిర్మాణం smbios_type_4 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u8 socket_designation_str;
    u8 processor_type;
    u8 processor_family;
    u8 processor_manufacturer_str;
    u32 processor_id[2];
    u8 processor_సంస్కరణ_str;
    u8 voltage;
    u16 external_clock;
    u16 max_వేగం;
    u16 current_వేగం;
    u8 స్థితి_2;
    u8 processor_upgrade;
    u16 l1_cache_handle;
    u16 l2_cache_handle;
    u16 l3_cache_handle;
} PACKED;

/* SMBIOS type 16 - Physical Memory Array
 *   Associated with one type 17 (Memory Device).
 */
నిర్మాణం smbios_type_16 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u8 location;
    u8 use;
    u8 error_correction;
    u32 గరిష్ఠ_సామర్థ్యం;
    u16 జ్ఞాపకం_error_information_handle;
    u16 సంఖ్య_యొక్క_జ్ఞాపకం_devices;
} PACKED;

/* SMBIOS type 17 - Memory Device
 *   Associated with one type 19
 */
నిర్మాణం smbios_type_17 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u16 physical_జ్ఞాపకం_array_handle;
    u16 జ్ఞాపకం_error_information_handle;
    u16 మొత్తం_వెడల్పు;
    u16 దత్తాంశం_వెడల్పు;
    u16 కొలత;
    u8 form_గుణకం;
    u8 device_సమితి;
    u8 device_locator_str;
    u8 bank_locator_str;
    u8 జ్ఞాపకం_type;
    u16 type_detail;
} PACKED;

/* SMBIOS type 19 - Memory Array Mapped Address */
నిర్మాణం smbios_type_19 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u32 starting_చిరునామా;
    u32 ending_చిరునామా;
    u16 జ్ఞాపకం_array_handle;
    u8 విభజన_వెడల్పు;
} PACKED;

/* SMBIOS type 20 - Memory Device Mapped Address */
నిర్మాణం smbios_type_20 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u32 starting_చిరునామా;
    u32 ending_చిరునామా;
    u16 జ్ఞాపకం_device_handle;
    u16 జ్ఞాపకం_array_mapped_చిరునామా_handle;
    u8 విభజన_అడ్డువరుస_position;
    u8 interleave_position;
    u8 interleaved_దత్తాంశం_లోతు;
} PACKED;

/* SMBIOS type 32 - System Boot Information */
నిర్మాణం smbios_type_32 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
    u8 reserved[6];
    u8 boot_స్థితి;
} PACKED;

/* SMBIOS type 127 -- End-of-table */
నిర్మాణం smbios_type_127 {
    నిర్మాణం smbios_structure_శీర్షిక శీర్షిక;
} PACKED;

#అయితే_ముగింపు // smbios.h
