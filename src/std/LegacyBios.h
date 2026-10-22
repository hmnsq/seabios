/** @file
  The EFI Legacy BIOS Protocol is used to abstract legacy Option ROM usage
  under EFI and Legacy OS boot.  This file also includes all the related
  COMPATIBILIY16 structures and defintions.

  Note: The names for EFI_IA32_REGISTER_SET elements were picked to follow
  well known naming conventions.

  Thunk is the code that switches from 32-bit protected environment into the 16-bit real-mode
	environment. Reverse thunk is the code that does the opposite.

Copyright (c) 2007 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Revision Reference:
  This protocol is defined in Framework for EFI Compatibility Support Module spec
  Version 0.97.

**/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _EFI_LEGACY_BIOS_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _EFI_LEGACY_BIOS_H_

///
/// 
///
#ਹਦਾਇਤ pack(1)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ UINT8                       SERIAL_ਬਹੁਲਕ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ UINT8                       PARALLEL_ਬਹੁਲਕ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EFI_COMPATIBILITY16_ਸਾਰਣੀ_SIGNATURE SIGNATURE_32 ('I', 'F', 'E', '$')

///
/// There is a table located within the traditional BIOS in either the 0xF000:xxxx or 0xE000:xxxx
/// physical address range. It is located on a 16-byte boundary and provides the physical address of the
/// entry point for the Compatibility16 functions. These functions provide the platform-specific
/// information that is required by the generic EfiCompatibility code. The functions are invoked via
/// thunking by using EFI_LEGACY_BIOS_PROTOCOL.FarCall86() with the 32-bit physical
/// entry point.
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ///
  /// The string "$EFI" denotes the start of the EfiCompatibility table. Byte 0 is "I," byte
  /// 1 is "F," byte 2 is "E," and byte 3 is "$" and is normally accessed as a DWORD or UINT32.
  ///
  UINT32                            Signature;
  
  ///
  /// The value required such that byte checksum of TableLength equals zero.
  ///
  UINT8                             ਸਾਰਣੀ_Checksum;
  
  ///
  /// The length of this table.
  ///
  UINT8                             ਸਾਰਣੀ_ਲੰਮਾਈ;
  
  ///
  /// The major EFI revision for which this table was generated.
  /// 
  UINT8                             EfiMajorRevision;
  
  ///
  /// The minor EFI revision for which this table was generated.
  ///
  UINT8                             EfiMinorRevision;
  
  ///
  /// The major revision of this table.
  ///
  UINT8                             ਸਾਰਣੀ_Major_Revision;
  
  ///
  /// The minor revision of this table.
  ///
  UINT8                             ਸਾਰਣੀ_Minor_Revision;
  
  ///
  /// Reserved for future usage.
  ///
  UINT16                            Reserved;
  
  ///
  /// The segment of the entry point within the traditional BIOS for Compatibility16 functions.
  ///
  UINT16                            Compatibility16CallSegment;
  
  ///
  /// The offset of the entry point within the traditional BIOS for Compatibility16 functions.
  ///
  UINT16                            Compatibility16_Call_ਵਿੱਥ;
  
  ///
  /// The segment of the entry point within the traditional BIOS for EfiCompatibility 
  /// to invoke the PnP installation check.
  ///
  UINT16                            PnPInstallationCheckSegment;
  
  ///
  /// The Offset of the entry point within the traditional BIOS for EfiCompatibility 
  /// to invoke the PnP installation check.
  ///
  UINT16                            Pn_PInstallation_Check_ਵਿੱਥ;
  
  ///
  /// EFI system resources table. Type EFI_SYSTEM_TABLE is defined in the IntelPlatform 
  ///Innovation Framework for EFI Driver Execution Environment Core Interface Specification (DXE CIS).
  ///
  UINT32                            Efi_System_ਸਾਰਣੀ; 
  
  ///
  /// The address of an OEM-provided identifier string. The string is null terminated.
  ///
  UINT32                            Oem_Id_String_ਸੂਚਕ;
  
  ///
  /// The 32-bit physical address where ACPI RSD PTR is stored within the traditional
  /// BIOS. The remained of the ACPI tables are located at their EFI addresses. The size
  /// reserved is the maximum for ACPI 2.0. The EfiCompatibility will fill in the ACPI
  /// RSD PTR with either the ACPI 1.0b or 2.0 values.
  ///
  UINT32                            Acpi_Rsd_Ptr_ਸੂਚਕ;
  
  ///
  /// The OEM revision number. Usage is undefined but provided for OEM module usage.
  ///
  UINT16                            OemRevision;
  
  ///
  /// The 32-bit physical address where INT15 E820 data is stored within the traditional
  /// BIOS. The EfiCompatibility code will fill in the E820Pointer value and copy the
  /// data to the indicated area.
  ///
  UINT32                            E820_ਸੂਚਕ;
  
  ///
  /// The length of the E820 data and is filled in by the EfiCompatibility code.
  ///
  UINT32                            E820_ਲੰਮਾਈ;
  
  ///
  /// The 32-bit physical address where the $PIR table is stored in the traditional BIOS.
  /// The EfiCompatibility code will fill in the IrqRoutingTablePointer value and
  /// copy the data to the indicated area.
  ///
  UINT32                            Irq_Routing_ਸਾਰਣੀ_ਸੂਚਕ;
  
  ///
  /// The length of the $PIR table and is filled in by the EfiCompatibility code.
  ///
  UINT32                            Irq_Routing_ਸਾਰਣੀ_ਲੰਮਾਈ;
  
  ///
  /// The 32-bit physical address where the MP table is stored in the traditional BIOS.
  /// The EfiCompatibility code will fill in the MpTablePtr value and copy the data 
  /// to the indicated area.
  ///
  UINT32                            Mp_ਸਾਰਣੀ_Ptr;
  
  ///
  /// The length of the MP table and is filled in by the EfiCompatibility code.
  ///
  UINT32                            Mp_ਸਾਰਣੀ_ਲੰਮਾਈ;
  
  ///
  /// The segment of the OEM-specific INT table/code.
  /// 
  UINT16                            OemIntSegment;
  
  ///
  /// The offset of the OEM-specific INT table/code.
  ///
  UINT16                            Oem_Int_ਵਿੱਥ;
  
  ///
  /// The segment of the OEM-specific 32-bit table/code.
  ///
  UINT16                            Oem32Segment;
  
  ///
  /// The offset of the OEM-specific 32-bit table/code.
  ///
  UINT16                            Oem32_ਵਿੱਥ;
  
  ///
  /// The segment of the OEM-specific 16-bit table/code.
  ///
  UINT16                            Oem16Segment;
  
  ///
  /// The offset of the OEM-specific 16-bit table/code.
  ///
  UINT16                            Oem16_ਵਿੱਥ;
  
  ///
  /// The segment of the TPM binary passed to 16-bit CSM.
  ///
  UINT16                            TpmSegment;
  
  ///
  /// The offset of the TPM binary passed to 16-bit CSM.
  ///
  UINT16                            Tpm_ਵਿੱਥ;
  
  ///
  /// A pointer to a string identifying the independent BIOS vendor.
  ///
  UINT32                            Ibv_ਸੂਚਕ;
  
  ///
  /// This field is NULL for all systems not supporting PCI Express. This field is the base
  /// value of the start of the PCI Express memory-mapped configuration registers and
  /// must be filled in prior to EfiCompatibility code issuing the Compatibility16 function
  /// Compatibility16InitializeYourself().
  /// Compatibility16InitializeYourself() is defined in Compatability16
  /// Functions.
  ///
  UINT32                            Pci_Express_ਅਧਾਰ;
  
  ///
  /// Maximum PCI bus number assigned.
  ///
  UINT8                             ਆਖਰੀ_Pci_Bus;

  ///
  /// Start address of UMB RAM
  ///
  UINT32                            Uma_ਪਤਾ;

  ///
  /// Size of UMB RAM
  ///
  UINT32                            Uma_ਮਾਪ;

  ///
  /// Start address of persistent allocation in high (>1MiB) memory
  ///
  UINT32                            Hi_Permanent_ਸਮ੍ਰਿਤੀ_ਪਤਾ;

  ///
  /// Size of persistent allocation in high (>1MiB) memory
  ///
  UINT32                            Hi_Permanent_ਸਮ੍ਰਿਤੀ_ਮਾਪ;
} EFI_COMPATIBILITY16_ਸਾਰਣੀ;

///
/// Functions provided by the CSM binary which communicate between the EfiCompatibility 
/// and Compatability16 code.
///
/// Inconsistent with the specification here: 
/// The member's name started with "Compatibility16" [defined in Intel Framework 
/// Compatibility Support Module Specification / 0.97 version] 
/// has been changed to "Legacy16" since keeping backward compatible.
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
  ///
  /// Causes the Compatibility16 code to do any internal initialization required.
  /// Input:
  ///   AX = Compatibility16InitializeYourself
  ///   ES:BX = Pointer to EFI_TO_COMPATIBILITY16_INIT_TABLE
  /// Return:
  ///   AX = Return Status codes
  ///
  Legacy16InitializeYourself    = 0x0000,
  
  ///
  /// Causes the Compatibility16 BIOS to perform any drive number translations to match the boot sequence.
  /// Input:
  ///   AX = Compatibility16UpdateBbs
  ///   ES:BX = Pointer to EFI_TO_COMPATIBILITY16_BOOT_TABLE
  /// Return:
  ///   AX = Returned status codes
  ///
  Legacy16UpdateBbs             = 0x0001,
  
  ///
  /// Allows the Compatibility16 code to perform any final actions before booting. The Compatibility16
  /// code is read/write.
  /// Input:
  ///   AX = Compatibility16PrepareToBoot
  ///   ES:BX = Pointer to EFI_TO_COMPATIBILITY16_BOOT_TABLE structure  
  /// Return:
  ///   AX = Returned status codes
  ///
  Legacy16_Prepare_ਮੰਜ਼ਿਲ_Boot         = 0x0002,
  
  ///
  /// Causes the Compatibility16 BIOS to boot. The Compatibility16 code is Read/Only.
  /// Input:
  ///   AX = Compatibility16Boot
  /// Output:
  ///   AX = Returned status codes
  ///
  Legacy16Boot                  = 0x0003,
  
  ///
  /// Allows the Compatibility16 code to get the last device from which a boot was attempted. This is
  /// stored in CMOS and is the priority number of the last attempted boot device.
  /// Input:
  ///   AX = Compatibility16RetrieveLastBootDevice
  /// Output:
  ///   AX = Returned status codes
  ///   BX = Priority number of the boot device.
  ///
  Legacy16_Retrieve_ਆਖਰੀ_Boot_Device = 0x0004,
  
  ///
  /// Allows the Compatibility16 code rehook INT13, INT18, and/or INT19 after dispatching a legacy OpROM.
  /// Input:
  ///   AX = Compatibility16DispatchOprom
  ///   ES:BX = Pointer to EFI_DISPATCH_OPROM_TABLE
  /// Output:
  ///   AX = Returned status codes
  ///   BX = Number of non-BBS-compliant devices found. Equals 0 if BBS compliant.
  ///
  Legacy16DispatchOprom         = 0x0005,
  
  ///
  /// Finds a free area in the 0xFxxxx or 0xExxxx region of the specified length and returns the address
  /// of that region.
  /// Input:
  ///   AX = Compatibility16GetTableAddress
  ///   BX = Allocation region
  ///       00 = Allocate from either 0xE0000 or 0xF0000 64 KB blocks.
  ///       Bit 0 = 1 Allocate from 0xF0000 64 KB block
  ///       Bit 1 = 1 Allocate from 0xE0000 64 KB block
  ///   CX = Requested length in bytes.
  ///   DX = Required address alignment. Bit mapped. First non-zero bit from the right is the alignment.
  /// Output:
  ///   AX = Returned status codes
  ///   DS:BX = Address of the region
  ///
  Legacy16_Get_ਸਾਰਣੀ_ਪਤਾ       = 0x0006,
  
  ///
  /// Enables the EfiCompatibility module to do any nonstandard processing of keyboard LEDs or state.
  /// Input:
  ///   AX = Compatibility16SetKeyboardLeds
  ///   CL = LED status.
  ///     Bit 0  Scroll Lock 0 = Off
  ///     Bit 1  NumLock
  ///     Bit 2  Caps Lock
  /// Output:
  ///     AX = Returned status codes
  ///
  Legacy16_ਸੈੱਟ_Keyboard_Leds       = 0x0007,
  
  ///
  /// Enables the EfiCompatibility module to install an interrupt handler for PCI mass media devices that
  /// do not have an OpROM associated with them. An example is SATA.
  /// Input:
  ///   AX = Compatibility16InstallPciHandler
  ///   ES:BX = Pointer to EFI_LEGACY_INSTALL_PCI_HANDLER structure
  /// Output:
  ///   AX = Returned status codes
  ///
  Legacy16InstallPciHandler     = 0x0008
} EFI_COMPATIBILITY_FUNCTIONS;


///
/// EFI_DISPATCH_OPROM_TABLE
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16  PnPInstallationCheckSegment;  ///< A pointer to the PnpInstallationCheck data structure.
  UINT16  Pn_PInstallation_Check_ਵਿੱਥ;   ///< A pointer to the PnpInstallationCheck data structure.
  UINT16  OpromSegment;                 ///< The segment where the OpROM was placed. Offset is assumed to be 3.
  UINT8   PciBus;                       ///< The PCI bus.
  UINT8   PciDeviceFunction;            ///< The PCI device * 0x08 | PCI function.
  UINT8   ਸੰਖਿਆ_Bbs_Entries;             ///< The number of valid BBS table entries upon entry and exit. The IBV code may
                                        ///< increase this number, if BBS-compliant devices also hook INTs in order to force the
                                        ///< OpROM BIOS Setup to be executed.
  UINT32  Bbs_ਸਾਰਣੀ_ਸੂਚਕ;              ///< A pointer to the BBS table.
  UINT16  RuntimeSegment;               ///< The segment where the OpROM can be relocated to. If this value is 0x0000, this
                                        ///< means that the relocation of this run time code is not supported.
                                        ///< Inconsistent with specification here: 
                                        ///< The member's name "OpromDestinationSegment" [defined in Intel Framework Compatibility Support Module Specification / 0.97 version] 
                                        ///< has been changed to "RuntimeSegment" since keeping backward compatible.

} EFI_DISPATCH_OPROM_ਸਾਰਣੀ;

///
/// EFI_TO_COMPATIBILITY16_INIT_TABLE
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ///
  /// Starting address of memory under 1 MB. The ending address is assumed to be 640 KB or 0x9FFFF.
  ///
  UINT32                            BiosLessThan1MB;
  
  ///
  /// The starting address of the high memory block.
  ///
  UINT32                            Hi_Pmm_ਸਮ੍ਰਿਤੀ;
  
  ///
  /// The length of high memory block.
  ///
  UINT32                            Hi_Pmm_ਸਮ੍ਰਿਤੀ_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ;
  
  ///
  /// The segment of the reverse thunk call code.
  ///
  UINT16                            ਉਲਟ_Thunk_Call_Segment;
  
  ///
  /// The offset of the reverse thunk call code.
  ///
  UINT16                            ਉਲਟ_Thunk_Call_ਵਿੱਥ;
  
  ///
  /// The number of E820 entries copied to the Compatibility16 BIOS.
  ///
  UINT32                            ਸੰਖਿਆ_E820_Entries;
  
  ///
  /// The amount of usable memory above 1 MB, e.g., E820 type 1 memory.
  ///
  UINT32                            Os_ਸਮ੍ਰਿਤੀ_Above1_Mb;
  
  ///
  /// The start of thunk code in main memory. Memory cannot be used by BIOS or PMM.
  ///
  UINT32                            Thunk_ਸ਼ੁਰੂ;
  
  ///
  /// The size of the thunk code.
  ///
  UINT32                            Thunk_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ;
  
  ///
  /// Starting address of memory under 1 MB.
  ///
  UINT32                            ਨੀਵਾਂ_Pmm_ਸਮ੍ਰਿਤੀ;
  
  ///
  /// The length of low Memory block.
  ///
  UINT32                            ਨੀਵਾਂ_Pmm_ਸਮ੍ਰਿਤੀ_ਮਾਪ_ਵਿੱਚ_ਬਾਈਟਾਂ;
} EFI_ਮੰਜ਼ਿਲ_COMPATIBILITY16_INIT_ਸਾਰਣੀ;

///
/// DEVICE_PRODUCER_SERIAL.
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16                            ਪਤਾ_2;    ///< I/O address assigned to the serial port.
  UINT8                             Irq;        ///< IRQ assigned to the serial port.
  SERIAL_ਬਹੁਲਕ                       ਬਹੁਲਕ_2;       ///< Mode of serial port. Values are defined below.
} DEVICE_ਉਤਪਾਦਕ_SERIAL;

///
/// DEVICE_PRODUCER_SERIAL's modes.
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_SERIAL_ਬਹੁਲਕ_ਸਾਧਾਰਨ               0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_SERIAL_ਬਹੁਲਕ_IRDA                 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_SERIAL_ਬਹੁਲਕ_ASK_IR               0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_SERIAL_ਬਹੁਲਕ_DUPLEX_HALF          0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_SERIAL_ਬਹੁਲਕ_DUPLEX_ਭਰਿਆ          0x10
///@)

///
/// DEVICE_PRODUCER_PARALLEL.
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16                            ਪਤਾ_2;  ///< I/O address assigned to the parallel port.
  UINT8                             Irq;      ///< IRQ assigned to the parallel port.
  UINT8                             Dma;      ///< DMA assigned to the parallel port.
  PARALLEL_ਬਹੁਲਕ                     ਬਹੁਲਕ_2;     ///< Mode of the parallel port. Values are defined below.
} DEVICE_ਉਤਪਾਦਕ_PARALLEL;

///
/// DEVICE_PRODUCER_PARALLEL's modes.
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_PARALLEL_ਬਹੁਲਕ_ਬਹੁਲਕ_OUTPUT_ONLY   0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_PARALLEL_ਬਹੁਲਕ_ਬਹੁਲਕ_BIDIRECTIONAL 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_PARALLEL_ਬਹੁਲਕ_ਬਹੁਲਕ_EPP           0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVICE_PARALLEL_ਬਹੁਲਕ_ਬਹੁਲਕ_ECP           0x03
///@}

///
/// DEVICE_PRODUCER_FLOPPY
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16                            ਪਤਾ_2;          ///< I/O address assigned to the floppy.
  UINT8                             Irq;              ///< IRQ assigned to the floppy.
  UINT8                             Dma;              ///< DMA assigned to the floppy.
  UINT8                             ਸੰਖਿਆ_ਦਾ_Floppy;   ///< Number of floppies in the system.
} DEVICE_ਉਤਪਾਦਕ_FLOPPY;

///
/// LEGACY_DEVICE_FLAGS
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT32                            A20Kybd : 1;      ///< A20 controller by keyboard controller.
  UINT32                            A20Port90 : 1;    ///< A20 controlled by port 0x92.
  UINT32                            Reserved : 30;    ///< Reserved for future usage.
} LEGACY_DEVICE_FLAGS;

///
/// DEVICE_PRODUCER_DATA_HEADER
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  DEVICE_ਉਤਪਾਦਕ_SERIAL            Serial[4];      ///< Data for serial port x. Type DEVICE_PRODUCER_SERIAL is defined below.
  DEVICE_ਉਤਪਾਦਕ_PARALLEL          Parallel[3];    ///< Data for parallel port x. Type DEVICE_PRODUCER_PARALLEL is defined below.
  DEVICE_ਉਤਪਾਦਕ_FLOPPY            Floppy;         ///< Data for floppy. Type DEVICE_PRODUCER_FLOPPY is defined below.
  UINT8                             MousePresent;   ///< Flag to indicate if mouse is present.
  LEGACY_DEVICE_FLAGS               Flags;          ///< Miscellaneous Boolean state information passed to CSM.
} DEVICE_ਉਤਪਾਦਕ_ਅੰਕੜੇ_ਸਿਰਲੇਖ;

///
/// ATAPI_IDENTIFY
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16                            Raw[256];     ///< Raw data from the IDE IdentifyDrive command.
} ATAPI_IDENTIFY;

///
/// HDD_INFO
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ///
  /// Status of IDE device. Values are defined below. There is one HDD_INFO structure
  /// per IDE controller. The IdentifyDrive is per drive. Index 0 is master and index
  /// 1 is slave.
  ///
  UINT16                            ਸਥਿਤੀ_2;   
  
  ///
  /// PCI bus of IDE controller.
  ///
  UINT32                            Bus;
  
  ///
  /// PCI device of IDE controller.
  ///
  UINT32                            Device;
  
  ///
  /// PCI function of IDE controller.
  ///
  UINT32                            Function;
  
  ///
  /// Command ports base address.
  ///
  UINT16                            Command_ਅਧਾਰ_ਪਤਾ;
  
  ///
  /// Control ports base address.
  ///
  UINT16                            Control_ਅਧਾਰ_ਪਤਾ;
  
  ///
  /// Bus master address.
  ///
  UINT16                            Bus_Master_ਪਤਾ;
  
  UINT8                             HddIrq;
  
  ///
  /// Data that identifies the drive data; one per possible attached drive.
  ///
  ATAPI_IDENTIFY                    IdentifyDrive[2];
} HDD_INFO;

///
/// HDD_INFO status bits
///
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HDD_PRIMARY               0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HDD_SECONDARY             0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HDD_MASTER_ATAPI_CDROM    0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HDD_SLAVE_ATAPI_CDROM     0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HDD_MASTER_IDE            0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HDD_SLAVE_IDE             0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HDD_MASTER_ATAPI_ZIPDISK  0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HDD_SLAVE_ATAPI_ZIPDISK   0x80

///
/// BBS_STATUS_FLAGS;\.
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16                            OldPosition : 4;    ///< Prior priority.
  UINT16                            Reserved1 : 4;      ///< Reserved for future use.
  UINT16                            Enabled : 1;        ///< If 0, ignore this entry.
  UINT16                            Failed : 1;         ///< 0 = Not known if boot failure occurred.
                                                        ///< 1 = Boot attempted failed.
  
  ///
  /// State of media present.
  ///   00 = No bootable media is present in the device.
  ///   01 = Unknown if a bootable media present.
  ///   10 = Media is present and appears bootable.
  ///   11 = Reserved.
  ///
  UINT16                            MediaPresent : 2;
  UINT16                            Reserved2 : 4;      ///< Reserved for future use.
} BBS_ਸਥਿਤੀ_FLAGS;

///
/// BBS_TABLE, device type values & boot priority values.
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ///
  /// The boot priority for this boot device. Values are defined below.
  ///
  UINT16                            Boot_ਤਰਜੀਹ;
  
  ///
  /// The PCI bus for this boot device.
  ///
  UINT32                            Bus;
  
  ///
  /// The PCI device for this boot device.
  ///
  UINT32                            Device;
  
  ///
  /// The PCI function for the boot device.
  ///
  UINT32                            Function;
  
  ///
  /// The PCI class for this boot device.
  ///
  UINT8                             Class;
  
  ///
  /// The PCI Subclass for this boot device.
  ///
  UINT8                             SubClass;
  
  ///
  /// Segment:offset address of an ASCIIZ description string describing the manufacturer.
  ///
  UINT16                            Mfg_String_ਵਿੱਥ;
  
  ///
  /// Segment:offset address of an ASCIIZ description string describing the manufacturer.
  ///  
  UINT16                            MfgStringSegment;
  
  ///
  /// BBS device type. BBS device types are defined below.
  ///
  UINT16                            DeviceType;
  
  ///
  /// Status of this boot device. Type BBS_STATUS_FLAGS is defined below.
  ///
  BBS_ਸਥਿਤੀ_FLAGS                  ਸਥਿਤੀ_Flags;
  
  ///
  /// Segment:Offset address of boot loader for IPL devices or install INT13 handler for
  /// BCV devices.
  ///
  UINT16                            Boot_Handler_ਵਿੱਥ;
  
  ///
  /// Segment:Offset address of boot loader for IPL devices or install INT13 handler for
  /// BCV devices.
  ///  
  UINT16                            BootHandlerSegment;
  
  ///
  /// Segment:offset address of an ASCIIZ description string describing this device.
  ///
  UINT16                            Desc_String_ਵਿੱਥ;

  ///
  /// Segment:offset address of an ASCIIZ description string describing this device.
  ///
  UINT16                            DescStringSegment;
  
  ///
  /// Reserved.
  ///
  UINT32                            Init_ਪ੍ਰਤੀ_Reserved;
  
  ///
  /// The use of these fields is IBV dependent. They can be used to flag that an OpROM
  /// has hooked the specified IRQ. The OpROM may be BBS compliant as some SCSI
  /// BBS-compliant OpROMs also hook IRQ vectors in order to run their BIOS Setup
  ///
  UINT32                            AdditionalIrq13Handler;
  
  ///
  /// The use of these fields is IBV dependent. They can be used to flag that an OpROM
  /// has hooked the specified IRQ. The OpROM may be BBS compliant as some SCSI
  /// BBS-compliant OpROMs also hook IRQ vectors in order to run their BIOS Setup
  ///  
  UINT32                            AdditionalIrq18Handler;
  
  ///
  /// The use of these fields is IBV dependent. They can be used to flag that an OpROM
  /// has hooked the specified IRQ. The OpROM may be BBS compliant as some SCSI
  /// BBS-compliant OpROMs also hook IRQ vectors in order to run their BIOS Setup
  ///  
  UINT32                            AdditionalIrq19Handler;
  
  ///
  /// The use of these fields is IBV dependent. They can be used to flag that an OpROM
  /// has hooked the specified IRQ. The OpROM may be BBS compliant as some SCSI
  /// BBS-compliant OpROMs also hook IRQ vectors in order to run their BIOS Setup
  ///  
  UINT32                            AdditionalIrq40Handler;
  UINT8                             Assigned_Drive_ਸੰਖਿਆ;
  UINT32                            AdditionalIrq41Handler;
  UINT32                            AdditionalIrq46Handler;
  UINT32                            IBV1;
  UINT32                            IBV2;
} BBS_ਸਾਰਣੀ;

///
/// BBS device type values
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_FLOPPY              0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_HARDDISK            0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_CDROM               0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_PCMCIA              0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_USB                 0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_EMBED_ਜਾਲ       0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_BEV_DEVICE          0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_UNKNOWN             0xff
///@}

///
/// BBS boot priority values
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_DO_NOT_BOOT_FROM    0xFFFC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_ਸਭ_ਤੋਂ_ਨੀਵਾਂ_ਤਰਜੀਹ     0xFFFD
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_UNPRIORITIZED_ਇੰਦਰਾਜ 0xFFFE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BBS_IGNORE_ਇੰਦਰਾਜ        0xFFFF
///@}

///
/// SMM_ATTRIBUTES
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ///
  /// Access mechanism used to generate the soft SMI. Defined types are below. The other
  /// values are reserved for future usage.
  ///
  UINT16                            Type : 3;
  
  ///
  /// The size of "port" in bits. Defined values are below.
  ///
  UINT16                            ਪੋਰਟ_Granularity : 3;
  
  ///
  /// The size of data in bits. Defined values are below.
  ///
  UINT16                            ਅੰਕੜੇ_Granularity : 3;
  
  ///
  /// Reserved for future use.
  ///
  UINT16                            Reserved : 7;
} SMM_ਗੁਣ;

///
/// SMM_ATTRIBUTES type values.
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STANDARD_IO       0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STANDARD_ਸਮ੍ਰਿਤੀ   0x01
///@}

///
/// SMM_ATTRIBUTES port size constants.
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_ਮਾਪ_8       0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_ਮਾਪ_16      0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_ਮਾਪ_32      0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_ਮਾਪ_64      0x03
///@}

///
/// SMM_ATTRIBUTES data size constants.
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕੜੇ_ਮਾਪ_8       0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕੜੇ_ਮਾਪ_16      0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕੜੇ_ਮਾਪ_32      0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕੜੇ_ਮਾਪ_64      0x03
///@}

///
/// SMM_FUNCTION & relating constants.
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16                            Function : 15;
  UINT16                            Owner : 1;
} SMM_FUNCTION;

///
/// SMM_FUNCTION Function constants.
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INT15_D042        0x0000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GET_USB_BOOT_INFO 0x0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DMI_PNP_50_57     0x0002
///@}

///
/// SMM_FUNCTION Owner constants.
///@{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STANDARD_OWNER    0x0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OEM_OWNER         0x1
///@}

///
/// This structure assumes both port and data sizes are 1. SmmAttribute must be
/// properly to reflect that assumption.
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ///
  /// Describes the access mechanism, SmmPort, and SmmData sizes. Type
  /// SMM_ATTRIBUTES is defined below.
  ///
  SMM_ਗੁਣ                    Smm_ਗੁਣ;
  
  ///
  /// Function Soft SMI is to perform. Type SMM_FUNCTION is defined below.
  ///
  SMM_FUNCTION                      SmmFunction;
  
  ///
  /// SmmPort size depends upon SmmAttributes and ranges from2 bytes to 16 bytes.
  ///
  UINT8                             Smm_ਪੋਰਟ;
  
  ///
  /// SmmData size depends upon SmmAttributes and ranges from2 bytes to 16 bytes.
  ///
  UINT8                             Smm_ਅੰਕੜੇ;
} SMM_ਇੰਦਰਾਜ;

///
/// SMM_TABLE
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16                            ਅੰਕ_Smm_Entries;    ///< Number of entries represented by SmmEntry.
  SMM_ਇੰਦਰਾਜ                         Smm_ਇੰਦਰਾਜ;         ///< One entry per function. Type SMM_ENTRY is defined below.
} SMM_ਸਾਰਣੀ;

///
/// UDC_ATTRIBUTES
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ///
  /// This bit set indicates that the ServiceAreaData is valid.
  ///
  UINT8                             DirectoryServiceValidity : 1;
  
  ///
  /// This bit set indicates to use the Reserve Area Boot Code Address (RACBA) only if
  /// DirectoryServiceValidity is 0.
  ///
  UINT8                             Rabca_ਵਰਤਿਆ_Flag : 1;
  
  ///
  /// This bit set indicates to execute hard disk diagnostics.
  ///
  UINT8                             ExecuteHddDiagnosticsFlag : 1;
  
  ///
  /// Reserved for future use. Set to 0.
  ///
  UINT8                             Reserved : 5;
} UDC_ਗੁਣ;

///
/// UD_TABLE
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  ///
  /// This field contains the bit-mapped attributes of the PARTIES information. Type
  /// UDC_ATTRIBUTES is defined below.
  ///
  UDC_ਗੁਣ                    ਗੁਣ;
  
  ///
  /// This field contains the zero-based device on which the selected
  /// ServiceDataArea is present. It is 0 for master and 1 for the slave device.  
  ///
  UINT8                             Device_ਸੰਖਿਆ;
  
  ///
  /// This field contains the zero-based index into the BbsTable for the parent device.
  /// This index allows the user to reference the parent device information such as PCI
  /// bus, device function.
  ///
  UINT8                             Bbs_ਸਾਰਣੀ_ਇੰਦਰਾਜ_ਸੰਖਿਆ_For_ਮਾਪਾ_Device;
  
  ///
  /// This field contains the zero-based index into the BbsTable for the boot entry.
  ///
  UINT8                             Bbs_ਸਾਰਣੀ_ਇੰਦਰਾਜ_ਸੰਖਿਆ_For_Boot;
  
  ///
  /// This field contains the zero-based index into the BbsTable for the HDD diagnostics entry.
  ///
  UINT8                             Bbs_ਸਾਰਣੀ_ਇੰਦਰਾਜ_ਸੰਖਿਆ_For_Hdd_Diag;
  
  ///
  /// The raw Beer data.
  ///
  UINT8                             Beer_ਅੰਕੜੇ[128];
  
  ///
  /// The raw data of selected service area.
  ///
  UINT8                             Service_Area_ਅੰਕੜੇ[64];
} UD_ਸਾਰਣੀ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EFI_ਮੰਜ਼ਿਲ_LEGACY_MAJOR_ਸੰਸਕਰਣ 0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EFI_ਮੰਜ਼ਿਲ_LEGACY_MINOR_ਸੰਸਕਰਣ 0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_IDE_CONTROLLER          8

///
/// EFI_TO_COMPATIBILITY16_BOOT_TABLE
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT16                            Major_ਸੰਸਕਰਣ;                 ///< The EfiCompatibility major version number.
  UINT16                            Minor_ਸੰਸਕਰਣ;                 ///< The EfiCompatibility minor version number.
  UINT32                            Acpi_ਸਾਰਣੀ;                    ///< The location of the RSDT ACPI table. < 4G range.
  UINT32                            Smbios_ਸਾਰਣੀ;                  ///< The location of the SMBIOS table in EFI memory. < 4G range.
  UINT32                            Smbios_ਸਾਰਣੀ_ਲੰਮਾਈ;
  //
  // Legacy SIO state
  //
  DEVICE_ਉਤਪਾਦਕ_ਅੰਕੜੇ_ਸਿਰਲੇਖ       Sio_ਅੰਕੜੇ;                      ///< Standard traditional device information.
  UINT16                            Device_ਰਸਤਾ_Type;               ///< The default boot type.
  UINT16                            Pci_Irq_ਨਕਾਬ;                   ///< Mask of which IRQs have been assigned to PCI.
  UINT32                            ਸੰਖਿਆ_E820_Entries;            ///< Number of E820 entries. The number can change from the
                                                                  ///< Compatibility16InitializeYourself() function.
  //
  // Controller & Drive Identify[2] per controller information
  //
  HDD_INFO                          HddInfo[MAX_IDE_CONTROLLER];  ///< Hard disk drive information, including raw Identify Drive data.
  UINT32                            ਸੰਖਿਆ_Bbs_Entries;             ///< Number of entries in the BBS table
  UINT32                            Bbs_ਸਾਰਣੀ;                     ///< A pointer to the BBS table. Type BBS_TABLE is defined below.
  UINT32                            Smm_ਸਾਰਣੀ;                     ///< A pointer to the SMM table. Type SMM_TABLE is defined below.
  UINT32                            Os_ਸਮ੍ਰਿਤੀ_Above1_Mb;             ///< The amount of usable memory above 1 MB, i.e. E820 type 1 memory. This value can
                                                                  ///< differ from the value in EFI_TO_COMPATIBILITY16_INIT_TABLE as more
                                                                  ///< memory may have been discovered.
  UINT32                            Unconventional_Device_ਸਾਰਣੀ;    ///< Information to boot off an unconventional device like a PARTIES partition. Type
                                                                  ///< UD_TABLE is defined below.
} EFI_ਮੰਜ਼ਿਲ_COMPATIBILITY16_BOOT_ਸਾਰਣੀ;

///
/// EFI_LEGACY_INSTALL_PCI_HANDLER
///
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  UINT8                             PciBus;             ///< The PCI bus of the device.
  UINT8                             PciDeviceFun;       ///< The PCI device in bits 7:3 and function in bits 2:0.
  UINT8                             PciSegment;         ///< The PCI segment of the device.
  UINT8                             PciClass;           ///< The PCI class code of the device.
  UINT8                             PciSubclass;        ///< The PCI subclass code of the device.
  UINT8                             PciInterface;       ///< The PCI interface code of the device.
  //
  // Primary section
  //
  UINT8                             PrimaryIrq;         ///< The primary device IRQ.
  UINT8                             PrimaryReserved;    ///< Reserved.
  UINT16                            PrimaryControl;     ///< The primary device control I/O base.
  UINT16                            Primary_ਅਧਾਰ;        ///< The primary device I/O base.
  UINT16                            PrimaryBusMaster;   ///< The primary device bus master I/O base.
  //
  // Secondary Section
  //
  UINT8                             SecondaryIrq;       ///< The secondary device IRQ.
  UINT8                             SecondaryReserved;  ///< Reserved.
  UINT16                            SecondaryControl;   ///< The secondary device control I/O base.
  UINT16                            Secondary_ਅਧਾਰ;      ///< The secondary device I/O base.
  UINT16                            SecondaryBusMaster; ///< The secondary device bus master I/O base.
} EFI_LEGACY_INSTALL_PCI_HANDLER;

#ਜੇ_ਅੰਤ
