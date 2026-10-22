#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ACPI_H
#ವ್ಯಾಖ್ಯಾನಿಸು __ACPI_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

/*
 * ACPI 2.0 Generic Address Space definition.
 */
ರಚನೆ acpi_20_generic_ವಿಳಾಸ {
    u8  ವಿಳಾಸ_space_id;
    u8  ರಿಜಿಸ್ಟರ್_ಬಿಟ್_ಅಗಲ;
    u8  ರಿಜಿಸ್ಟರ್_ಬಿಟ್_ಸ್ಥಾನಾಂತರ;
    u8  reserved;
    u64 ವಿಳಾಸ;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು RSDP_SIGNATURE 0x2052545020445352LL // "RSD PTR "

ರಚನೆ rsdp_descriptor {        /* Root System Descriptor Pointer */
    u64 signature;              /* ACPI signature, contains "RSD PTR " */
    u8  checksum;               /* To make sum of struct == 0 */
    u8  oem_id [6];             /* OEM identification */
    u8  revision;               /* Must be 0 for 1.0, 2 for 2.0 */
    u32 rsdt_physical_ವಿಳಾಸ;  /* 32-bit physical address of RSDT */
    u32 ಉದ್ದಳತೆ;                 /* XSDT Length in bytes including hdr */
    u64 xsdt_physical_ವಿಳಾಸ;  /* 64-bit physical address of XSDT */
    u8  extended_checksum;      /* Checksum of entire table */
    u8  reserved [3];           /* Reserved field must be 0 */
};

/* Table structure from Linux kernel (the ACPI tables are under the
   BSD license) */

#ವ್ಯಾಖ್ಯಾನಿಸು ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF   /* ACPI common table header */ \
    u32 signature;          /* ACPI signature (4 ASCII characters) */ \
    u32 ಉದ್ದಳತೆ;                 /* Length of table, in bytes, including header */ \
    u8  revision;               /* ACPI Specification minor version # */ \
    u8  checksum;               /* To make sum of entire table == 0 */ \
    u8  oem_id [6];             /* OEM identification */ \
    u8  oem_ಕೋಷ್ಟಕ_id [8];       /* OEM table identification */ \
    u32 oem_revision;           /* OEM revision number */ \
    u8  asl_compiler_id [4];    /* ASL compiler vendor ID */ \
    u32 asl_compiler_revision;  /* ASL compiler revision number */

/*
 * Fixed ACPI Description Table Fixed Feature Flags
 */
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_WBINVD            (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_WBINVD_FLUSH      (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_PROC_C1           (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_P_LVL2_ಮೇಲೆ         (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_PWR_BUTTON        (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_SLP_BUTTON        (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_FIX_RTC           (1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_RTC_S4            (1 << 7)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_TMR_VAL_EXT       (1 << 8)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_DCK_CAP           (1 << 9)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_RESET_REG_SUP     (1 << 10)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_SEALED_CASE       (1 << 11)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_HEADLESS          (1 << 12)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_CPU_SW_SLP        (1 << 13)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_PCI_EXP_WAK       (1 << 14)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_USE_PLATFORM_CLOCK (1 << 15)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_S4_RTC_STS_VALID   (1 << 16)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_REMOTE_POWER_ON_CAPABLE  (1 << 17)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_FORCE_APIC_CLUSTER_MODEL  (1 << 18)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_FORCE_APIC_PHYSICAL_DESTINATION_ಬಹುಲಕ  (1 << 19)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_HW_REDUCED_ACPI    (1 << 20)
#ವ್ಯಾಖ್ಯಾನಿಸು    ACPI_FADT_F_ಕೆಳಗಿನ_POWER_S0_IDLE_CAPABLE  (1 << 21)

/*
 * ACPI 1.0 Fixed ACPI Description Table (FADT)
 */
#ವ್ಯಾಖ್ಯಾನಿಸು FACP_SIGNATURE 0x50434146 // FACP
ರಚನೆ fadt_descriptor_rev1
{
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF     /* ACPI common table header */
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
    u8  gpe1_ತಳ;              /* Offset in gpe model where gpe1 events start */
    u8  reserved3;              /* Reserved */
    u16 plvl2_lat;              /* Worst case HW latency to enter/exit C2 state */
    u16 plvl3_lat;              /* Worst case HW latency to enter/exit C3 state */
    u16 flush_ಗಾತ್ರ;             /* Size of area read to flush caches */
    u16 flush_stride;           /* Stride used in flushing caches */
    u8  duty_ಸ್ಥಾನಾಂತರ;            /* Bit location of duty cycle field in p_cnt reg */
    u8  duty_ಅಗಲ;             /* Bit width of duty cycle field in p_cnt reg */
    u8  day_alrm;               /* Index to day-of-month alarm in RTC CMOS RAM */
    u8  mon_alrm;               /* Index to month-of-year alarm in RTC CMOS RAM */
    u8  century;                /* Index to century in RTC CMOS RAM */
    u8  reserved4;              /* Reserved */
    u8  reserved4a;             /* Reserved */
    u8  reserved4b;             /* Reserved */
    u32 flags;
} PACKED;

ರಚನೆ acpi_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ         /* ACPI common table header */
{
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF
} PACKED;

/*
 * ACPI 1.0 Root System Description Table (RSDT)
 */
#ವ್ಯಾಖ್ಯಾನಿಸು RSDT_SIGNATURE 0x54445352 // RSDT
ರಚನೆ rsdt_descriptor_rev1
{
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF       /* ACPI common table header */
    u32 ಕೋಷ್ಟಕ_ಸ್ಥಾನಾಂತರ_ನಮೂದು[0];  /* Array of pointers to other */
    /* ACPI tables */
} PACKED;

/*
 * ACPI 2.0 eXtended System Description Table (XSDT)
 */
#ವ್ಯಾಖ್ಯಾನಿಸು XSDT_SIGNATURE 0x54445358 // XSDT
ರಚನೆ xsdt_descriptor_rev2
{
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF       /* ACPI common table header */
    u64 ಕೋಷ್ಟಕ_ಸ್ಥಾನಾಂತರ_ನಮೂದು[0];  /* Array of pointers to other */
    /* ACPI tables */
} PACKED;

/*
 * ACPI 1.0 Firmware ACPI Control Structure (FACS)
 */
#ವ್ಯಾಖ್ಯಾನಿಸು FACS_SIGNATURE 0x53434146 // FACS
ರಚನೆ facs_descriptor_rev1
{
    u32 signature;           /* ACPI Signature */
    u32 ಉದ್ದಳತೆ;                 /* Length of structure, in bytes */
    u32 hardware_signature;     /* Hardware configuration signature */
    u32 firmware_waking_ಸದಿಶ; /* ACPI OS waking vector */
    u32 global_ಬೀಗ;            /* Global Lock */
    u32 flags;
    u8  resverved3 [40];        /* Reserved - must be zero */
} PACKED;

/*
 * Differentiated System Description Table (DSDT)
 */
#ವ್ಯಾಖ್ಯಾನಿಸು DSDT_SIGNATURE 0x54445344 // DSDT

/*
 * MADT values and structures
 */

/* Values for MADT PCATCompat */

#ವ್ಯಾಖ್ಯಾನಿಸು DUAL_PIC                0
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಪವರ್ತ್ಯ_APIC           1

/* Master MADT */

#ವ್ಯಾಖ್ಯಾನಿಸು APIC_SIGNATURE 0x43495041 // APIC
ರಚನೆ ಅಪವರ್ತ್ಯ_apic_ಕೋಷ್ಟಕ
{
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF     /* ACPI common table header */
    u32 local_apic_ವಿಳಾಸ;     /* Physical address of local APIC */
    u32 flags;
} PACKED;

/* Values for Type in APIC sub-headers */

#ವ್ಯಾಖ್ಯಾನಿಸು APIC_PROCESSOR          0
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_IO                 1
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_XRUPT_OVERRIDE     2
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_NMI                3
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_LOCAL_NMI          4
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_ವಿಳಾಸ_OVERRIDE   5
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_IO_SAPIC           6
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_LOCAL_SAPIC        7
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_XRUPT_ಮೂಲ       8
#ವ್ಯಾಖ್ಯಾನಿಸು APIC_RESERVED           9           /* 9 and greater are reserved */

/*
 * MADT sub-structures (Follow MULTIPLE_APIC_DESCRIPTION_TABLE)
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ACPI_SUB_ಶೀರ್ಷಿಕೆ_DEF   /* Common ACPI sub-structure header */\
    u8  type;                               \
    u8  ಉದ್ದಳತೆ;

/* Sub-structures for MADT */

ರಚನೆ madt_processor_apic
{
    ACPI_SUB_ಶೀರ್ಷಿಕೆ_DEF
    u8  processor_id;           /* ACPI processor id */
    u8  local_apic_id;          /* Processor's local APIC id */
    u32 flags;
} PACKED;

ರಚನೆ madt_io_apic
{
    ACPI_SUB_ಶೀರ್ಷಿಕೆ_DEF
    u8  io_apic_id;             /* I/O APIC ID */
    u8  reserved;               /* Reserved - must be zero */
    u32 ವಿಳಾಸ;                /* APIC physical address */
    u32 ಅಡಚಣೆ;              /* Global system interrupt where INTI
                                 * lines start */
} PACKED;

ರಚನೆ madt_intsrcovr {
    ACPI_SUB_ಶೀರ್ಷಿಕೆ_DEF
    u8  bus;
    u8  ಮೂಲ;
    u32 gsi;
    u16 flags;
} PACKED;

ರಚನೆ madt_local_nmi {
    ACPI_SUB_ಶೀರ್ಷಿಕೆ_DEF
    u8  processor_id;           /* ACPI processor id */
    u16 flags;                  /* MPS INTI flags */
    u8  lint;                   /* Local APIC LINT# */
} PACKED;

/*
 * HPET Description Table
 */
#ವ್ಯಾಖ್ಯಾನಿಸು HPET_SIGNATURE 0x54455048 // HPET
ರಚನೆ acpi_20_hpet {
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF                    /* ACPI common table header */
    u32           timer_ಖಂಡ_id;
    ರಚನೆ acpi_20_generic_ವಿಳಾಸ addr;
    u8            hpet_ಸಂಖ್ಯೆ;
    u16           min_tick;
    u8            ಪುಟ_protect;
} PACKED;

/*
 * SRAT (NUMA topology description) table
 */

#ವ್ಯಾಖ್ಯಾನಿಸು SRAT_SIGNATURE 0x54415253 // SRAT
ರಚನೆ system_resource_affinity_ಕೋಷ್ಟಕ
{
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF
    u32    reserved1;
    u32    reserved2[2];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು SRAT_PROCESSOR          0
#ವ್ಯಾಖ್ಯಾನಿಸು SRAT_ಸ್ಮರಣೆ             1

ರಚನೆ srat_processor_affinity
{
    ACPI_SUB_ಶೀರ್ಷಿಕೆ_DEF
    u8     proximity_lo;
    u8     local_apic_id;
    u32    flags;
    u8     local_sapic_eid;
    u8     proximity_hi[3];
    u32    reserved;
} PACKED;

ರಚನೆ srat_ಸ್ಮರಣೆ_affinity
{
    ACPI_SUB_ಶೀರ್ಷಿಕೆ_DEF
    u8     proximity[4];
    u16    reserved1;
    u64    ತಳ_addr;
    u64    range_ಉದ್ದಳತೆ;
    u32    reserved2;
    u32    flags;
    u32    reserved3[2];
} PACKED;

/* PCI fw r3.0 MCFG table. */
/* Subtable */
ರಚನೆ acpi_mcfg_ಹಂಚಿಕೆ {
    u64 ವಿಳಾಸ;                /* Base address, processor-relative */
    u16 pci_segment;            /* PCI segment group number */
    u8 ಆರಂಭ_bus_ಸಂಖ್ಯೆ;       /* Starting PCI Bus number */
    u8 ಕೊನೆ_bus_ಸಂಖ್ಯೆ;         /* Final PCI Bus number */
    u32 reserved;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು MCFG_SIGNATURE 0x4746434d       // MCFG
ರಚನೆ acpi_ಕೋಷ್ಟಕ_mcfg {
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF;
    u8 reserved[8];
    ರಚನೆ acpi_mcfg_ಹಂಚಿಕೆ ಹಂಚಿಕೆ[0];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು TCPA_SIGNATURE 0x41504354
ರಚನೆ tcpa_descriptor_rev2
{
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF
    u16  platform_class;
    u32  log_area_minimum_ಉದ್ದಳತೆ;
    u64  log_area_ಆರಂಭ_ವಿಳಾಸ;
} PACKED;

/* TCPA ACPI definitions */
#ವ್ಯಾಖ್ಯಾನಿಸು TCPA_ACPI_CLASS_CLIENT          0
#ವ್ಯಾಖ್ಯಾನಿಸು TCPA_ACPI_CLASS_SERVER          1

#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_SIGNATURE 0x324D5054
ರಚನೆ tpm2_descriptor_rev2
{
    ACPI_ಕೋಷ್ಟಕ_ಶೀರ್ಷಿಕೆ_DEF
    u16  platform_class;
    u16  reserved;
    u64  ವಿಳಾಸ_ದ_control_area;
    u32  ಆರಂಭ_ವಿಧಾನ;
    u8   ಆರಂಭ_ವಿಧಾನ_params[12];
    u32  log_area_minimum_ಉದ್ದಳತೆ;
    u64  log_area_ಆರಂಭ_ವಿಳಾಸ;
} PACKED;

#ಆದರೆ_ಅಂತ್ಯ // acpi.h
