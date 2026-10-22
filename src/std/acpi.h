#यदि_अपरिभाषितम् __ACPI_H
#परिभाषय __ACPI_H

#अन्तर्भाव "types.h" // u32

/*
 * ACPI 2.0 Generic Address Space definition.
 */
संरचना acpi_20_generic_स्थानाङ्कः {
    u8  स्थानाङ्कः_space_id;
    u8  पञ्जिकाकोशः_bit_विस्तारः;
    u8  पञ्जिकाकोशः_bit_विस्थापनम्;
    u8  reserved;
    u64 स्थानाङ्कः;
} PACKED;

#परिभाषय RSDP_SIGNATURE 0x2052545020445352LL // "RSD PTR "

संरचना rsdp_descriptor {        /* Root System Descriptor Pointer */
    u64 signature;              /* ACPI signature, contains "RSD PTR " */
    u8  checksum;               /* To make sum of struct == 0 */
    u8  oem_id [6];             /* OEM identification */
    u8  revision;               /* Must be 0 for 1.0, 2 for 2.0 */
    u32 rsdt_physical_स्थानाङ्कः;  /* 32-bit physical address of RSDT */
    u32 दीर्घता;                 /* XSDT Length in bytes including hdr */
    u64 xsdt_physical_स्थानाङ्कः;  /* 64-bit physical address of XSDT */
    u8  extended_checksum;      /* Checksum of entire table */
    u8  reserved [3];           /* Reserved field must be 0 */
};

/* Table structure from Linux kernel (the ACPI tables are under the
   BSD license) */

#परिभाषय ACPI_सारणी_शीर्षकम्_DEF   /* ACPI common table header */ \
    u32 signature;          /* ACPI signature (4 ASCII characters) */ \
    u32 दीर्घता;                 /* Length of table, in bytes, including header */ \
    u8  revision;               /* ACPI Specification minor version # */ \
    u8  checksum;               /* To make sum of entire table == 0 */ \
    u8  oem_id [6];             /* OEM identification */ \
    u8  oem_सारणी_id [8];       /* OEM table identification */ \
    u32 oem_revision;           /* OEM revision number */ \
    u8  asl_compiler_id [4];    /* ASL compiler vendor ID */ \
    u32 asl_compiler_revision;  /* ASL compiler revision number */

/*
 * Fixed ACPI Description Table Fixed Feature Flags
 */
#परिभाषय    ACPI_FADT_F_WBINVD            (1 << 0)
#परिभाषय    ACPI_FADT_F_WBINVD_FLUSH      (1 << 1)
#परिभाषय    ACPI_FADT_F_PROC_C1           (1 << 2)
#परिभाषय    ACPI_FADT_F_P_LVL2_ऊर्ध्वम्         (1 << 3)
#परिभाषय    ACPI_FADT_F_PWR_BUTTON        (1 << 4)
#परिभाषय    ACPI_FADT_F_SLP_BUTTON        (1 << 5)
#परिभाषय    ACPI_FADT_F_FIX_RTC           (1 << 6)
#परिभाषय    ACPI_FADT_F_RTC_S4            (1 << 7)
#परिभाषय    ACPI_FADT_F_TMR_VAL_EXT       (1 << 8)
#परिभाषय    ACPI_FADT_F_DCK_CAP           (1 << 9)
#परिभाषय    ACPI_FADT_F_RESET_REG_SUP     (1 << 10)
#परिभाषय    ACPI_FADT_F_SEALED_CASE       (1 << 11)
#परिभाषय    ACPI_FADT_F_HEADLESS          (1 << 12)
#परिभाषय    ACPI_FADT_F_CPU_SW_SLP        (1 << 13)
#परिभाषय    ACPI_FADT_F_PCI_EXP_WAK       (1 << 14)
#परिभाषय    ACPI_FADT_F_USE_PLATFORM_CLOCK (1 << 15)
#परिभाषय    ACPI_FADT_F_S4_RTC_STS_VALID   (1 << 16)
#परिभाषय    ACPI_FADT_F_REMOTE_POWER_ON_CAPABLE  (1 << 17)
#परिभाषय    ACPI_FADT_F_FORCE_APIC_CLUSTER_MODEL  (1 << 18)
#परिभाषय    ACPI_FADT_F_FORCE_APIC_PHYSICAL_DESTINATION_बहुलकम्  (1 << 19)
#परिभाषय    ACPI_FADT_F_HW_REDUCED_ACPI    (1 << 20)
#परिभाषय    ACPI_FADT_F_नीचः_POWER_S0_IDLE_CAPABLE  (1 << 21)

/*
 * ACPI 1.0 Fixed ACPI Description Table (FADT)
 */
#परिभाषय FACP_SIGNATURE 0x50434146 // FACP
संरचना fadt_descriptor_rev1
{
    ACPI_सारणी_शीर्षकम्_DEF     /* ACPI common table header */
    u32 firmware_ctrl;          /* Physical address of FACS */
    u32 dsdt;                   /* Physical address of DSDT */
    u8  model;                  /* System Interrupt Model */
    u8  reserved1;              /* Reserved */
    u16 sci_int;                /* System vector of SCI interrupt */
    u32 smi_cmd;                /* Port address of SMI command port */
    u8  acpi_enable;            /* Value to write to smi_cmd to enable ACPI */
    u8  acpi_disable;           /* Value to write to smi_cmd to disable ACPI */
    u8  S4bios_req;             /* Value to write to SMI CMD to enter S4BIOS state */
    u8  reserved2;              /* Reserved - must be zero */
    u32 pm1a_evt_blk;           /* Port address of Power Mgt 1a acpi_event Reg Blk */
    u32 pm1b_evt_blk;           /* Port address of Power Mgt 1b acpi_event Reg Blk */
    u32 pm1a_cnt_blk;           /* Port address of Power Mgt 1a Control Reg Blk */
    u32 pm1b_cnt_blk;           /* Port address of Power Mgt 1b Control Reg Blk */
    u32 pm2_cnt_blk;            /* Port address of Power Mgt 2 Control Reg Blk */
    u32 pm_tmr_blk;             /* Port address of Power Mgt Timer Ctrl Reg Blk */
    u32 gpe0_blk;               /* Port addr of General Purpose acpi_event 0 Reg Blk */
    u32 gpe1_blk;               /* Port addr of General Purpose acpi_event 1 Reg Blk */
    u8  pm1_evt_len;            /* Byte length of ports at pm1_x_evt_blk */
    u8  pm1_cnt_len;            /* Byte length of ports at pm1_x_cnt_blk */
    u8  pm2_cnt_len;            /* Byte Length of ports at pm2_cnt_blk */
    u8  pm_tmr_len;             /* Byte Length of ports at pm_tm_blk */
    u8  gpe0_blk_len;           /* Byte Length of ports at gpe0_blk */
    u8  gpe1_blk_len;           /* Byte Length of ports at gpe1_blk */
    u8  gpe1_आधारः;              /* Offset in gpe model where gpe1 events start */
    u8  reserved3;              /* Reserved */
    u16 plvl2_lat;              /* Worst case HW latency to enter/exit C2 state */
    u16 plvl3_lat;              /* Worst case HW latency to enter/exit C3 state */
    u16 flush_आकार;             /* Size of area read to flush caches */
    u16 flush_stride;           /* Stride used in flushing caches */
    u8  duty_विस्थापनम्;            /* Bit location of duty cycle field in p_cnt reg */
    u8  duty_विस्तारः;             /* Bit width of duty cycle field in p_cnt reg */
    u8  day_alrm;               /* Index to day-of-month alarm in RTC CMOS RAM */
    u8  mon_alrm;               /* Index to month-of-year alarm in RTC CMOS RAM */
    u8  century;                /* Index to century in RTC CMOS RAM */
    u8  reserved4;              /* Reserved */
    u8  reserved4a;             /* Reserved */
    u8  reserved4b;             /* Reserved */
    u32 flags;
} PACKED;

संरचना acpi_सारणी_शीर्षकम्         /* ACPI common table header */
{
    ACPI_सारणी_शीर्षकम्_DEF
} PACKED;

/*
 * ACPI 1.0 Root System Description Table (RSDT)
 */
#परिभाषय RSDT_SIGNATURE 0x54445352 // RSDT
संरचना rsdt_descriptor_rev1
{
    ACPI_सारणी_शीर्षकम्_DEF       /* ACPI common table header */
    u32 सारणी_विस्थापनम्_प्रविष्टिः[0];  /* Array of pointers to other */
    /* ACPI tables */
} PACKED;

/*
 * ACPI 2.0 eXtended System Description Table (XSDT)
 */
#परिभाषय XSDT_SIGNATURE 0x54445358 // XSDT
संरचना xsdt_descriptor_rev2
{
    ACPI_सारणी_शीर्षकम्_DEF       /* ACPI common table header */
    u64 सारणी_विस्थापनम्_प्रविष्टिः[0];  /* Array of pointers to other */
    /* ACPI tables */
} PACKED;

/*
 * ACPI 1.0 Firmware ACPI Control Structure (FACS)
 */
#परिभाषय FACS_SIGNATURE 0x53434146 // FACS
संरचना facs_descriptor_rev1
{
    u32 signature;           /* ACPI Signature */
    u32 दीर्घता;                 /* Length of structure, in bytes */
    u32 hardware_signature;     /* Hardware configuration signature */
    u32 firmware_waking_सदिशम्; /* ACPI OS waking vector */
    u32 global_तालकम्;            /* Global Lock */
    u32 flags;
    u8  resverved3 [40];        /* Reserved - must be zero */
} PACKED;

/*
 * Differentiated System Description Table (DSDT)
 */
#परिभाषय DSDT_SIGNATURE 0x54445344 // DSDT

/*
 * MADT values and structures
 */

/* Values for MADT PCATCompat */

#परिभाषय DUAL_PIC                0
#परिभाषय गुणजः_APIC           1

/* Master MADT */

#परिभाषय APIC_SIGNATURE 0x43495041 // APIC
संरचना गुणजः_apic_सारणी
{
    ACPI_सारणी_शीर्षकम्_DEF     /* ACPI common table header */
    u32 local_apic_स्थानाङ्कः;     /* Physical address of local APIC */
    u32 flags;
} PACKED;

/* Values for Type in APIC sub-headers */

#परिभाषय APIC_PROCESSOR          0
#परिभाषय APIC_IO                 1
#परिभाषय APIC_XRUPT_OVERRIDE     2
#परिभाषय APIC_NMI                3
#परिभाषय APIC_LOCAL_NMI          4
#परिभाषय APIC_स्थानाङ्कः_OVERRIDE   5
#परिभाषय APIC_IO_SAPIC           6
#परिभाषय APIC_LOCAL_SAPIC        7
#परिभाषय APIC_XRUPT_SOURCE       8
#परिभाषय APIC_RESERVED           9           /* 9 and greater are reserved */

/*
 * MADT sub-structures (Follow MULTIPLE_APIC_DESCRIPTION_TABLE)
 */
#परिभाषय ACPI_SUB_शीर्षकम्_DEF   /* Common ACPI sub-structure header */\
    u8  type;                               \
    u8  दीर्घता;

/* Sub-structures for MADT */

संरचना madt_processor_apic
{
    ACPI_SUB_शीर्षकम्_DEF
    u8  processor_id;           /* ACPI processor id */
    u8  local_apic_id;          /* Processor's local APIC id */
    u32 flags;
} PACKED;

संरचना madt_io_apic
{
    ACPI_SUB_शीर्षकम्_DEF
    u8  io_apic_id;             /* I/O APIC ID */
    u8  reserved;               /* Reserved - must be zero */
    u32 स्थानाङ्कः;                /* APIC physical address */
    u32 व्यवधानम्;              /* Global system interrupt where INTI
                                 * lines start */
} PACKED;

संरचना madt_intsrcovr {
    ACPI_SUB_शीर्षकम्_DEF
    u8  bus;
    u8  source;
    u32 gsi;
    u16 flags;
} PACKED;

संरचना madt_local_nmi {
    ACPI_SUB_शीर्षकम्_DEF
    u8  processor_id;           /* ACPI processor id */
    u16 flags;                  /* MPS INTI flags */
    u8  lint;                   /* Local APIC LINT# */
} PACKED;

/*
 * HPET Description Table
 */
#परिभाषय HPET_SIGNATURE 0x54455048 // HPET
संरचना acpi_20_hpet {
    ACPI_सारणी_शीर्षकम्_DEF                    /* ACPI common table header */
    u32           timer_खण्डम्_id;
    संरचना acpi_20_generic_स्थानाङ्कः addr;
    u8            hpet_सङ्ख्या;
    u16           min_tick;
    u8            page_protect;
} PACKED;

/*
 * SRAT (NUMA topology description) table
 */

#परिभाषय SRAT_SIGNATURE 0x54415253 // SRAT
संरचना system_resource_affinity_सारणी
{
    ACPI_सारणी_शीर्षकम्_DEF
    u32    reserved1;
    u32    reserved2[2];
} PACKED;

#परिभाषय SRAT_PROCESSOR          0
#परिभाषय SRAT_स्मृतिः             1

संरचना srat_processor_affinity
{
    ACPI_SUB_शीर्षकम्_DEF
    u8     proximity_lo;
    u8     local_apic_id;
    u32    flags;
    u8     local_sapic_eid;
    u8     proximity_hi[3];
    u32    reserved;
} PACKED;

संरचना srat_स्मृतिः_affinity
{
    ACPI_SUB_शीर्षकम्_DEF
    u8     proximity[4];
    u16    reserved1;
    u64    आधारः_addr;
    u64    range_दीर्घता;
    u32    reserved2;
    u32    flags;
    u32    reserved3[2];
} PACKED;

/* PCI fw r3.0 MCFG table. */
/* Subtable */
संरचना acpi_mcfg_आवण्टनम् {
    u64 स्थानाङ्कः;                /* Base address, processor-relative */
    u16 pci_segment;            /* PCI segment group number */
    u8 start_bus_सङ्ख्या;       /* Starting PCI Bus number */
    u8 अन्तः_bus_सङ्ख्या;         /* Final PCI Bus number */
    u32 reserved;
} PACKED;

#परिभाषय MCFG_SIGNATURE 0x4746434d       // MCFG
संरचना acpi_सारणी_mcfg {
    ACPI_सारणी_शीर्षकम्_DEF;
    u8 reserved[8];
    संरचना acpi_mcfg_आवण्टनम् आवण्टनम्[0];
} PACKED;

#परिभाषय TCPA_SIGNATURE 0x41504354
संरचना tcpa_descriptor_rev2
{
    ACPI_सारणी_शीर्षकम्_DEF
    u16  platform_class;
    u32  log_area_minimum_दीर्घता;
    u64  log_area_start_स्थानाङ्कः;
} PACKED;

/* TCPA ACPI definitions */
#परिभाषय TCPA_ACPI_CLASS_CLIENT          0
#परिभाषय TCPA_ACPI_CLASS_SERVER          1

#परिभाषय TPM2_SIGNATURE 0x324D5054
संरचना tpm2_descriptor_rev2
{
    ACPI_सारणी_शीर्षकम्_DEF
    u16  platform_class;
    u16  reserved;
    u64  स्थानाङ्कः_सम्बन्धि_control_area;
    u32  start_विधानम्;
    u8   start_विधानम्_params[12];
    u32  log_area_minimum_दीर्घता;
    u64  log_area_start_स्थानाङ्कः;
} PACKED;

#यदि_अन्तः // acpi.h
