/*
 *	pci_regs.h
 *
 *	PCI standard defines
 *	Copyright 1994, Drew Eckhardt
 *	Copyright 1997--1999 Martin Mares <mj@ucw.cz>
 *
 *	For more information, please consult the following manuals (look at
 *	http://www.pcisig.com/ for how to get them):
 *
 *	PCI BIOS Specification
 *	PCI Local Bus Specification
 *	PCI to PCI Bridge Specification
 *	PCI System Design Guide
 *
 * 	For hypertransport information, please consult the following manuals
 * 	from http://www.hypertransport.org
 *
 *	The Hypertransport I/O Link Specification
 */

#यदि_अपरिभाषितम् LINUX_PCI_REGS_H
#परिभाषय LINUX_PCI_REGS_H

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#परिभाषय PCI_VENDOR_ID		0x00	/* 16 bits */
#परिभाषय PCI_DEVICE_ID		0x02	/* 16 bits */
#परिभाषय PCI_COMMAND		0x04	/* 16 bits */
#परिभाषय  PCI_COMMAND_IO		0x1	/* Enable response in I/O space */
#परिभाषय  PCI_COMMAND_स्मृतिः	0x2	/* Enable response in Memory space */
#परिभाषय  PCI_COMMAND_MASTER	0x4	/* Enable bus mastering */
#परिभाषय  PCI_COMMAND_SPECIAL	0x8	/* Enable response to special cycles */
#परिभाषय  PCI_COMMAND_INVALIDATE	0x10	/* Use memory write and invalidate */
#परिभाषय  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#परिभाषय  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#परिभाषय  PCI_COMMAND_WAIT 	0x80	/* Enable address/data stepping */
#परिभाषय  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#परिभाषय  PCI_COMMAND_FAST_पृष्ठभागः	0x200	/* Enable back-to-back writes */
#परिभाषय  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

#परिभाषय PCI_स्थितिः		0x06	/* 16 bits */
#परिभाषय  PCI_स्थितिः_CAP_LIST	0x10	/* Support Capability List */
#परिभाषय  PCI_स्थितिः_66_MHZ	0x20	/* Support 66 Mhz PCI 2.1 bus */
#परिभाषय  PCI_स्थितिः_UDF		0x40	/* Support User Definable Features [obsolete] */
#परिभाषय  PCI_स्थितिः_FAST_पृष्ठभागः	0x80	/* Accept fast-back to back */
#परिभाषय  PCI_स्थितिः_PARITY	0x100	/* Detected parity error */
#परिभाषय  PCI_स्थितिः_DEVSEL_आवरणम्	0x600	/* DEVSEL timing */
#परिभाषय  PCI_स्थितिः_DEVSEL_FAST		0x000
#परिभाषय  PCI_स्थितिः_DEVSEL_MEDIUM	0x200
#परिभाषय  PCI_स्थितिः_DEVSEL_SLOW		0x400
#परिभाषय  PCI_स्थितिः_SIG_TARGET_ABORT	0x800 /* Set on target abort */
#परिभाषय  PCI_स्थितिः_REC_TARGET_ABORT	0x1000 /* Master ack of " */
#परिभाषय  PCI_स्थितिः_REC_MASTER_ABORT	0x2000 /* Set on master abort */
#परिभाषय  PCI_स्थितिः_SIG_SYSTEM_ERROR	0x4000 /* Set when we drive SERR */
#परिभाषय  PCI_स्थितिः_DETECTED_PARITY	0x8000 /* Set on parity error */

#परिभाषय PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8 revision */
#परिभाषय PCI_REVISION_ID		0x08	/* Revision ID */
#परिभाषय PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#परिभाषय PCI_CLASS_DEVICE	0x0a	/* Device class */

#परिभाषय PCI_CACHE_पङ्क्तिः_आकार	0x0c	/* 8 bits */
#परिभाषय PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#परिभाषय PCI_शीर्षकम्_TYPE		0x0e	/* 8 bits */
#परिभाषय  PCI_शीर्षकम्_TYPE_प्रसामान्यम्		0
#परिभाषय  PCI_शीर्षकम्_TYPE_BRIDGE		1
#परिभाषय  PCI_शीर्षकम्_TYPE_CARDBUS	2

#परिभाषय PCI_BIST		0x0f	/* 8 bits */
#परिभाषय  PCI_BIST_संहिता_आवरणम्	0x0f	/* Return result */
#परिभाषय  PCI_BIST_START		0x40	/* 1 to start BIST, 2 secs or less */
#परिभाषय  PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#परिभाषय PCI_आधारः_स्थानाङ्कः_0	0x10	/* 32 bits */
#परिभाषय PCI_आधारः_स्थानाङ्कः_1	0x14	/* 32 bits [htype 0,1 only] */
#परिभाषय PCI_आधारः_स्थानाङ्कः_2	0x18	/* 32 bits [htype 0 only] */
#परिभाषय PCI_आधारः_स्थानाङ्कः_3	0x1c	/* 32 bits */
#परिभाषय PCI_आधारः_स्थानाङ्कः_4	0x20	/* 32 bits */
#परिभाषय PCI_आधारः_स्थानाङ्कः_5	0x24	/* 32 bits */
#परिभाषय  PCI_आधारः_स्थानाङ्कः_SPACE		0x01	/* 0 = memory, 1 = I/O */
#परिभाषय  PCI_आधारः_स्थानाङ्कः_SPACE_IO	0x01
#परिभाषय  PCI_आधारः_स्थानाङ्कः_SPACE_स्मृतिः	0x00
#परिभाषय  PCI_आधारः_स्थानाङ्कः_MEM_TYPE_आवरणम्	0x06
#परिभाषय  PCI_आधारः_स्थानाङ्कः_MEM_TYPE_32	0x00	/* 32 bit address */
#परिभाषय  PCI_आधारः_स्थानाङ्कः_MEM_TYPE_1_M	0x02	/* Below 1M [obsolete] */
#परिभाषय  PCI_आधारः_स्थानाङ्कः_MEM_TYPE_64	0x04	/* 64 bit address */
#परिभाषय  PCI_आधारः_स्थानाङ्कः_MEM_PREFETCH	0x08	/* prefetchable? */
#परिभाषय  PCI_आधारः_स्थानाङ्कः_MEM_आवरणम्	(~0x0fUL)
#परिभाषय  PCI_आधारः_स्थानाङ्कः_IO_आवरणम्	(~0x03UL)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#परिभाषय PCI_CARDBUS_CIS		0x28
#परिभाषय PCI_SUBSYSTEM_VENDOR_ID	0x2c
#परिभाषय PCI_SUBSYSTEM_ID	0x2e
#परिभाषय PCI_ROM_स्थानाङ्कः		0x30	/* Bits 31..11 are address, 10..1 reserved */
#परिभाषय  PCI_ROM_स्थानाङ्कः_ENABLE	0x01
#परिभाषय PCI_ROM_स्थानाङ्कः_आवरणम्	(~0x7ffUL)

#परिभाषय PCI_CAPABILITY_LIST	0x34	/* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#परिभाषय PCI_व्यवधानम्_पङ्क्तिः	0x3c	/* 8 bits */
#परिभाषय PCI_व्यवधानम्_PIN	0x3d	/* 8 bits */
#परिभाषय PCI_MIN_GNT		0x3e	/* 8 bits */
#परिभाषय PCI_MAX_LAT		0x3f	/* 8 bits */

/* Header type 1 (PCI-to-PCI bridges) */
#परिभाषय PCI_PRIMARY_BUS		0x18	/* Primary bus number */
#परिभाषय PCI_SECONDARY_BUS	0x19	/* Secondary bus number */
#परिभाषय PCI_SUBORDINATE_BUS	0x1a	/* Highest bus number behind the bridge */
#परिभाषय PCI_SEC_LATENCY_TIMER	0x1b	/* Latency timer for secondary interface */
#परिभाषय PCI_IO_आधारः		0x1c	/* I/O range behind the bridge */
#परिभाषय PCI_IO_सीमा		0x1d
#परिभाषय  PCI_IO_RANGE_TYPE_आवरणम्	0x0fUL	/* I/O bridging type */
#परिभाषय  PCI_IO_RANGE_TYPE_16	0x00
#परिभाषय  PCI_IO_RANGE_TYPE_32	0x01
#परिभाषय  PCI_IO_RANGE_आवरणम्	(~0x0fUL)
#परिभाषय PCI_SEC_स्थितिः		0x1e	/* Secondary status register, only bit 14 used */
#परिभाषय PCI_स्मृतिः_आधारः		0x20	/* Memory range behind */
#परिभाषय PCI_स्मृतिः_सीमा	0x22
#परिभाषय  PCI_स्मृतिः_RANGE_TYPE_आवरणम् 0x0fUL
#परिभाषय  PCI_स्मृतिः_RANGE_आवरणम्	(~0x0fUL)
#परिभाषय PCI_PREF_स्मृतिः_आधारः	0x24	/* Prefetchable memory range behind */
#परिभाषय PCI_PREF_स्मृतिः_सीमा	0x26
#परिभाषय  PCI_PREF_RANGE_TYPE_आवरणम् 0x0fUL
#परिभाषय  PCI_PREF_RANGE_TYPE_32	0x00
#परिभाषय  PCI_PREF_RANGE_TYPE_64	0x01
#परिभाषय  PCI_PREF_RANGE_आवरणम्	(~0x0fUL)
#परिभाषय PCI_PREF_आधारः_UPPER32	0x28	/* Upper half of prefetchable memory range */
#परिभाषय PCI_PREF_सीमा_UPPER32	0x2c
#परिभाषय PCI_IO_आधारः_UPPER16	0x30	/* Upper half of I/O addresses */
#परिभाषय PCI_IO_सीमा_UPPER16	0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#परिभाषय PCI_ROM_ADDRESS1	0x38	/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#परिभाषय PCI_BRIDGE_CONTROL	0x3e
#परिभाषय  PCI_BRIDGE_CTL_PARITY	0x01	/* Enable parity detection on secondary interface */
#परिभाषय  PCI_BRIDGE_CTL_SERR	0x02	/* The same for SERR forwarding */
#परिभाषय  PCI_BRIDGE_CTL_ISA	0x04	/* Enable ISA mode */
#परिभाषय  PCI_BRIDGE_CTL_VGA	0x08	/* Forward VGA addresses */
#परिभाषय  PCI_BRIDGE_CTL_MASTER_ABORT	0x20  /* Report master aborts */
#परिभाषय  PCI_BRIDGE_CTL_BUS_RESET	0x40	/* Secondary bus reset */
#परिभाषय  PCI_BRIDGE_CTL_FAST_पृष्ठभागः	0x80	/* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#परिभाषय PCI_CB_CAPABILITY_LIST	0x14
/* 0x15 reserved */
#परिभाषय PCI_CB_SEC_स्थितिः	0x16	/* Secondary status */
#परिभाषय PCI_CB_PRIMARY_BUS	0x18	/* PCI bus number */
#परिभाषय PCI_CB_CARD_BUS		0x19	/* CardBus bus number */
#परिभाषय PCI_CB_SUBORDINATE_BUS	0x1a	/* Subordinate bus number */
#परिभाषय PCI_CB_LATENCY_TIMER	0x1b	/* CardBus latency timer */
#परिभाषय PCI_CB_स्मृतिः_आधारः_0	0x1c
#परिभाषय PCI_CB_स्मृतिः_सीमा_0	0x20
#परिभाषय PCI_CB_स्मृतिः_आधारः_1	0x24
#परिभाषय PCI_CB_स्मृतिः_सीमा_1	0x28
#परिभाषय PCI_CB_IO_आधारः_0	0x2c
#परिभाषय PCI_CB_IO_आधारः_0_HI	0x2e
#परिभाषय PCI_CB_IO_सीमा_0	0x30
#परिभाषय PCI_CB_IO_सीमा_0_HI	0x32
#परिभाषय PCI_CB_IO_आधारः_1	0x34
#परिभाषय PCI_CB_IO_आधारः_1_HI	0x36
#परिभाषय PCI_CB_IO_सीमा_1	0x38
#परिभाषय PCI_CB_IO_सीमा_1_HI	0x3a
#परिभाषय  PCI_CB_IO_RANGE_आवरणम्	(~0x03UL)
/* 0x3c-0x3d are same as for htype 0 */
#परिभाषय PCI_CB_BRIDGE_CONTROL	0x3e
#परिभाषय  PCI_CB_BRIDGE_CTL_PARITY	0x01	/* Similar to standard bridge control register */
#परिभाषय  PCI_CB_BRIDGE_CTL_SERR		0x02
#परिभाषय  PCI_CB_BRIDGE_CTL_ISA		0x04
#परिभाषय  PCI_CB_BRIDGE_CTL_VGA		0x08
#परिभाषय  PCI_CB_BRIDGE_CTL_MASTER_ABORT	0x20
#परिभाषय  PCI_CB_BRIDGE_CTL_CB_RESET	0x40	/* CardBus reset */
#परिभाषय  PCI_CB_BRIDGE_CTL_16BIT_INT	0x80	/* Enable interrupt for 16-bit cards */
#परिभाषय  PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 0x100	/* Prefetch enable for both memory regions */
#परिभाषय  PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 0x200
#परिभाषय  PCI_CB_BRIDGE_CTL_POST_WRITES	0x400
#परिभाषय PCI_CB_SUBSYSTEM_VENDOR_ID	0x40
#परिभाषय PCI_CB_SUBSYSTEM_ID		0x42
#परिभाषय PCI_CB_LEGACY_बहुलकम्_आधारः		0x44	/* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability lists */

#परिभाषय PCI_CAP_LIST_ID		0	/* Capability ID */
#परिभाषय  PCI_CAP_ID_PM		0x01	/* Power Management */
#परिभाषय  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#परिभाषय  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#परिभाषय  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#परिभाषय  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#परिभाषय  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#परिभाषय  PCI_CAP_ID_PCIX	0x07	/* PCI-X */
#परिभाषय  PCI_CAP_ID_HT		0x08	/* HyperTransport */
#परिभाषय  PCI_CAP_ID_VNDR	0x09	/* Vendor specific */
#परिभाषय  PCI_CAP_ID_DBG		0x0A	/* Debug port */
#परिभाषय  PCI_CAP_ID_CCRC	0x0B	/* CompactPCI Central Resource Control */
#परिभाषय  PCI_CAP_ID_SHPC 	0x0C	/* PCI Standard Hot-Plug Controller */
#परिभाषय  PCI_CAP_ID_SSVID	0x0D	/* Bridge subsystem vendor/device ID */
#परिभाषय  PCI_CAP_ID_AGP3	0x0E	/* AGP Target PCI-PCI bridge */
#परिभाषय  PCI_CAP_ID_EXP 	0x10	/* PCI Express */
#परिभाषय  PCI_CAP_ID_MSIX	0x11	/* MSI-X */
#परिभाषय PCI_CAP_LIST_अग्र	1	/* Next capability in the list */
#परिभाषय PCI_CAP_FLAGS		2	/* Capability defined flags (16 bits) */
#परिभाषय PCI_CAP_SIZEOF		4

/* Power Management Registers */

#परिभाषय PCI_PM_PMC		2	/* PM Capabilities Register */
#परिभाषय  PCI_PM_CAP_VER_आवरणम्	0x0007	/* Version */
#परिभाषय  PCI_PM_CAP_PME_CLOCK	0x0008	/* PME clock required */
#परिभाषय  PCI_PM_CAP_RESERVED    0x0010  /* Reserved field */
#परिभाषय  PCI_PM_CAP_DSI		0x0020	/* Device specific initialization */
#परिभाषय  PCI_PM_CAP_AUX_POWER	0x01C0	/* Auxilliary power support mask */
#परिभाषय  PCI_PM_CAP_D1		0x0200	/* D1 power state support */
#परिभाषय  PCI_PM_CAP_D2		0x0400	/* D2 power state support */
#परिभाषय  PCI_PM_CAP_PME		0x0800	/* PME pin supported */
#परिभाषय  PCI_PM_CAP_PME_आवरणम्	0xF800	/* PME Mask of all supported states */
#परिभाषय  PCI_PM_CAP_PME_D0	0x0800	/* PME# from D0 */
#परिभाषय  PCI_PM_CAP_PME_D1	0x1000	/* PME# from D1 */
#परिभाषय  PCI_PM_CAP_PME_D2	0x2000	/* PME# from D2 */
#परिभाषय  PCI_PM_CAP_PME_D3	0x4000	/* PME# from D3 (hot) */
#परिभाषय  PCI_PM_CAP_PME_D3cold	0x8000	/* PME# from D3 (cold) */
#परिभाषय  PCI_PM_CAP_PME_सरणम्	11	/* Start of the PME Mask in PMC */
#परिभाषय PCI_PM_CTRL		4	/* PM control and status register */
#परिभाषय  PCI_PM_CTRL_STATE_आवरणम्	0x0003	/* Current power state (D0 to D3) */
#परिभाषय  PCI_PM_CTRL_NO_SOFT_RESET	0x0004	/* No reset for D3hot->D0 */
#परिभाषय  PCI_PM_CTRL_PME_ENABLE	0x0100	/* PME pin enable */
#परिभाषय  PCI_PM_CTRL_दत्त_SEL_आवरणम्	0x1e00	/* Data select (??) */
#परिभाषय  PCI_PM_CTRL_दत्त_SCALE_आवरणम्	0x6000	/* Data scale (??) */
#परिभाषय  PCI_PM_CTRL_PME_स्थितिः	0x8000	/* PME pin status */
#परिभाषय PCI_PM_PPB_EXTENSIONS	6	/* PPB support extensions (??) */
#परिभाषय  PCI_PM_PPB_B2_B3	0x40	/* Stop clock when in D3hot (??) */
#परिभाषय  PCI_PM_BPCC_ENABLE	0x80	/* Bus power/clock control enable (??) */
#परिभाषय PCI_PM_दत्त_पञ्जिकाकोशः	7	/* (??) */
#परिभाषय PCI_PM_SIZEOF		8

/* AGP registers */

#परिभाषय PCI_AGP_संस्करणम्		2	/* BCD version number */
#परिभाषय PCI_AGP_RFU		3	/* Rest of capability flags */
#परिभाषय PCI_AGP_स्थितिः		4	/* Status register */
#परिभाषय  PCI_AGP_स्थितिः_RQ_आवरणम्	0xff000000	/* Maximum number of requests - 1 */
#परिभाषय  PCI_AGP_स्थितिः_SBA	0x0200	/* Sideband addressing supported */
#परिभाषय  PCI_AGP_स्थितिः_64_BIT	0x0020	/* 64-bit addressing supported */
#परिभाषय  PCI_AGP_स्थितिः_FW	0x0010	/* FW transfers supported */
#परिभाषय  PCI_AGP_स्थितिः_RATE4	0x0004	/* 4x transfer rate supported */
#परिभाषय  PCI_AGP_स्थितिः_RATE2	0x0002	/* 2x transfer rate supported */
#परिभाषय  PCI_AGP_स्थितिः_RATE1	0x0001	/* 1x transfer rate supported */
#परिभाषय PCI_AGP_COMMAND		8	/* Control register */
#परिभाषय  PCI_AGP_COMMAND_RQ_आवरणम् 0xff000000  /* Master: Maximum number of requests */
#परिभाषय  PCI_AGP_COMMAND_SBA	0x0200	/* Sideband addressing enabled */
#परिभाषय  PCI_AGP_COMMAND_AGP	0x0100	/* Allow processing of AGP transactions */
#परिभाषय  PCI_AGP_COMMAND_64BIT	0x0020 	/* Allow processing of 64-bit addresses */
#परिभाषय  PCI_AGP_COMMAND_FW	0x0010 	/* Force FW transfers */
#परिभाषय  PCI_AGP_COMMAND_RATE4	0x0004	/* Use 4x rate */
#परिभाषय  PCI_AGP_COMMAND_RATE2	0x0002	/* Use 2x rate */
#परिभाषय  PCI_AGP_COMMAND_RATE1	0x0001	/* Use 1x rate */
#परिभाषय PCI_AGP_SIZEOF		12

/* Vital Product Data */

#परिभाषय PCI_VPD_ADDR		2	/* Address to access (15 bits!) */
#परिभाषय  PCI_VPD_ADDR_आवरणम्	0x7fff	/* Address mask */
#परिभाषय  PCI_VPD_ADDR_F		0x8000	/* Write 0, 1 indicates completion */
#परिभाषय PCI_VPD_दत्त		4	/* 32-bits of data returned here */

/* Slot Identification */

#परिभाषय PCI_SID_ESR		2	/* Expansion Slot Register */
#परिभाषय  PCI_SID_ESR_NSLOTS	0x1f	/* Number of expansion slots available */
#परिभाषय  PCI_SID_ESR_FIC	0x20	/* First In Chassis Flag */
#परिभाषय PCI_SID_CHASSIS_NR	3	/* Chassis Number */

/* Message Signalled Interrupts registers */

#परिभाषय PCI_MSI_FLAGS		2	/* Various flags */
#परिभाषय  PCI_MSI_FLAGS_64BIT	0x80	/* 64-bit addresses allowed */
#परिभाषय  PCI_MSI_FLAGS_QSIZE	0x70	/* Message queue size configured */
#परिभाषय  PCI_MSI_FLAGS_QMASK	0x0e	/* Maximum queue size available */
#परिभाषय  PCI_MSI_FLAGS_ENABLE	0x01	/* MSI feature enabled */
#परिभाषय  PCI_MSI_FLAGS_MASKBIT	0x100	/* 64-bit mask bits allowed */
#परिभाषय PCI_MSI_RFU		3	/* Rest of capability flags */
#परिभाषय PCI_MSI_स्थानाङ्कः_LO	4	/* Lower 32 bits */
#परिभाषय PCI_MSI_स्थानाङ्कः_HI	8	/* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#परिभाषय PCI_MSI_दत्त_32		8	/* 16 bits of data for 32-bit devices */
#परिभाषय PCI_MSI_दत्त_64		12	/* 16 bits of data for 64-bit devices */
#परिभाषय PCI_MSI_आवरणम्_BIT	16	/* Mask bits register */

/* MSI-X registers (these are at offset PCI_MSIX_FLAGS) */
#परिभाषय PCI_MSIX_FLAGS		2
#परिभाषय  PCI_MSIX_FLAGS_QSIZE	0x7FF
#परिभाषय  PCI_MSIX_FLAGS_ENABLE	(1 << 15)
#परिभाषय  PCI_MSIX_FLAGS_MASKALL	(1 << 14)
#परिभाषय PCI_MSIX_FLAGS_BIRMASK	(7 << 0)
#परिभाषय PCI_MSIX_FLAGS_BITMASK	(1 << 0)

/* CompactPCI Hotswap Register */

#परिभाषय PCI_CHSWP_CSR		2	/* Control and Status Register */
#परिभाषय  PCI_CHSWP_DHA		0x01	/* Device Hiding Arm */
#परिभाषय  PCI_CHSWP_EIM		0x02	/* ENUM# Signal Mask */
#परिभाषय  PCI_CHSWP_PIE		0x04	/* Pending Insert or Extract */
#परिभाषय  PCI_CHSWP_LOO		0x08	/* LED On / Off */
#परिभाषय  PCI_CHSWP_PI		0x30	/* Programming Interface */
#परिभाषय  PCI_CHSWP_EXT		0x40	/* ENUM# status - extraction */
#परिभाषय  PCI_CHSWP_INS		0x80	/* ENUM# status - insertion */

/* PCI-X registers */

#परिभाषय PCI_X_CMD		2	/* Modes & Features */
#परिभाषय  PCI_X_CMD_DPERR_E	0x0001	/* Data Parity Error Recovery Enable */
#परिभाषय  PCI_X_CMD_ERO		0x0002	/* Enable Relaxed Ordering */
#परिभाषय  PCI_X_CMD_READ_512	0x0000	/* 512 byte maximum read byte count */
#परिभाषय  PCI_X_CMD_READ_1K	0x0004	/* 1Kbyte maximum read byte count */
#परिभाषय  PCI_X_CMD_READ_2K	0x0008	/* 2Kbyte maximum read byte count */
#परिभाषय  PCI_X_CMD_READ_4K	0x000c	/* 4Kbyte maximum read byte count */
#परिभाषय  PCI_X_CMD_MAX_READ	0x000c	/* Max Memory Read Byte Count */
				/* Max # of outstanding split transactions */
#परिभाषय  PCI_X_CMD_विभज_1	0x0000	/* Max 1 */
#परिभाषय  PCI_X_CMD_विभज_2	0x0010	/* Max 2 */
#परिभाषय  PCI_X_CMD_विभज_3	0x0020	/* Max 3 */
#परिभाषय  PCI_X_CMD_विभज_4	0x0030	/* Max 4 */
#परिभाषय  PCI_X_CMD_विभज_8	0x0040	/* Max 8 */
#परिभाषय  PCI_X_CMD_विभज_12	0x0050	/* Max 12 */
#परिभाषय  PCI_X_CMD_विभज_16	0x0060	/* Max 16 */
#परिभाषय  PCI_X_CMD_विभज_32	0x0070	/* Max 32 */
#परिभाषय  PCI_X_CMD_MAX_विभज	0x0070	/* Max Outstanding Split Transactions */
#परिभाषय  PCI_X_CMD_संस्करणम्(x) 	(((x) >> 12) & 3) /* Version */
#परिभाषय PCI_X_स्थितिः		4	/* PCI-X capabilities */
#परिभाषय  PCI_X_स्थितिः_DEVFN	0x000000ff	/* A copy of devfn */
#परिभाषय  PCI_X_स्थितिः_BUS	0x0000ff00	/* A copy of bus nr */
#परिभाषय  PCI_X_स्थितिः_64_BIT	0x00010000	/* 64-bit device */
#परिभाषय  PCI_X_स्थितिः_133_MHZ	0x00020000	/* 133 MHz capable */
#परिभाषय  PCI_X_स्थितिः_SPL_DISC	0x00040000	/* Split Completion Discarded */
#परिभाषय  PCI_X_स्थितिः_UNX_SPL	0x00080000	/* Unexpected Split Completion */
#परिभाषय  PCI_X_स्थितिः_COMPLEX	0x00100000	/* Device Complexity */
#परिभाषय  PCI_X_स्थितिः_MAX_READ	0x00600000	/* Designed Max Memory Read Count */
#परिभाषय  PCI_X_स्थितिः_MAX_विभज	0x03800000	/* Designed Max Outstanding Split Transactions */
#परिभाषय  PCI_X_स्थितिः_MAX_CUM	0x1c000000	/* Designed Max Cumulative Read Size */
#परिभाषय  PCI_X_स्थितिः_SPL_ERR	0x20000000	/* Rcvd Split Completion Error Msg */
#परिभाषय  PCI_X_स्थितिः_266_MHZ	0x40000000	/* 266 MHz capable */
#परिभाषय  PCI_X_स्थितिः_533_MHZ	0x80000000	/* 533 MHz capable */

/* PCI Express capability registers */

#परिभाषय PCI_EXP_FLAGS		2	/* Capabilities register */
#परिभाषय PCI_EXP_FLAGS_VERS	0x000f	/* Capability version */
#परिभाषय PCI_EXP_FLAGS_TYPE	0x00f0	/* Device/Port type */
#परिभाषय  PCI_EXP_TYPE_ENDPOINT	0x0	/* Express Endpoint */
#परिभाषय  PCI_EXP_TYPE_LEG_अन्तः	0x1	/* Legacy Endpoint */
#परिभाषय  PCI_EXP_TYPE_मूलम्_पत्तनम् 0x4	/* Root Port */
#परिभाषय  PCI_EXP_TYPE_UPSTREAM	0x5	/* Upstream Port */
#परिभाषय  PCI_EXP_TYPE_DOWNSTREAM 0x6	/* Downstream Port */
#परिभाषय  PCI_EXP_TYPE_PCI_BRIDGE 0x7	/* PCI/PCI-X Bridge */
#परिभाषय PCI_EXP_FLAGS_SLOT	0x0100	/* Slot implemented */
#परिभाषय PCI_EXP_FLAGS_IRQ	0x3e00	/* Interrupt message number */
#परिभाषय PCI_EXP_DEVCAP		4	/* Device capabilities */
#परिभाषय  PCI_EXP_DEVCAP_PAYLOAD	0x07	/* Max_Payload_Size */
#परिभाषय  PCI_EXP_DEVCAP_PHANTOM	0x18	/* Phantom functions */
#परिभाषय  PCI_EXP_DEVCAP_EXT_चिह्नकम्	0x20	/* Extended tags */
#परिभाषय  PCI_EXP_DEVCAP_L0S	0x1c0	/* L0s Acceptable Latency */
#परिभाषय  PCI_EXP_DEVCAP_L1	0xe00	/* L1 Acceptable Latency */
#परिभाषय  PCI_EXP_DEVCAP_ATN_BUT	0x1000	/* Attention Button Present */
#परिभाषय  PCI_EXP_DEVCAP_ATN_IND	0x2000	/* Attention Indicator Present */
#परिभाषय  PCI_EXP_DEVCAP_PWR_IND	0x4000	/* Power Indicator Present */
#परिभाषय  PCI_EXP_DEVCAP_RBER	0x8000	/* Role-Based Error Reporting */
#परिभाषय  PCI_EXP_DEVCAP_PWR_VAL	0x3fc0000 /* Slot Power Limit Value */
#परिभाषय  PCI_EXP_DEVCAP_PWR_SCL	0xc000000 /* Slot Power Limit Scale */
#परिभाषय  PCI_EXP_DEVCAP_FLR     0x10000000 /* Function Level Reset */
#परिभाषय PCI_EXP_DEVCTL		8	/* Device Control */
#परिभाषय  PCI_EXP_DEVCTL_CERE	0x0001	/* Correctable Error Reporting En. */
#परिभाषय  PCI_EXP_DEVCTL_NFERE	0x0002	/* Non-Fatal Error Reporting Enable */
#परिभाषय  PCI_EXP_DEVCTL_FERE	0x0004	/* Fatal Error Reporting Enable */
#परिभाषय  PCI_EXP_DEVCTL_URRE	0x0008	/* Unsupported Request Reporting En. */
#परिभाषय  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#परिभाषय  PCI_EXP_DEVCTL_PAYLOAD	0x00e0	/* Max_Payload_Size */
#परिभाषय  PCI_EXP_DEVCTL_EXT_चिह्नकम्	0x0100	/* Extended Tag Field Enable */
#परिभाषय  PCI_EXP_DEVCTL_PHANTOM	0x0200	/* Phantom Functions Enable */
#परिभाषय  PCI_EXP_DEVCTL_AUX_PME	0x0400	/* Auxiliary Power PM Enable */
#परिभाषय  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#परिभाषय  PCI_EXP_DEVCTL_READRQ	0x7000	/* Max_Read_Request_Size */
#परिभाषय  PCI_EXP_DEVCTL_BCR_FLR 0x8000  /* Bridge Configuration Retry / FLR */
#परिभाषय PCI_EXP_DEVSTA		10	/* Device Status */
#परिभाषय  PCI_EXP_DEVSTA_CED	0x01	/* Correctable Error Detected */
#परिभाषय  PCI_EXP_DEVSTA_NFED	0x02	/* Non-Fatal Error Detected */
#परिभाषय  PCI_EXP_DEVSTA_FED	0x04	/* Fatal Error Detected */
#परिभाषय  PCI_EXP_DEVSTA_URD	0x08	/* Unsupported Request Detected */
#परिभाषय  PCI_EXP_DEVSTA_AUXPD	0x10	/* AUX Power Detected */
#परिभाषय  PCI_EXP_DEVSTA_TRPND	0x20	/* Transactions Pending */
#परिभाषय PCI_EXP_LNKCAP		12	/* Link Capabilities */
#परिभाषय  PCI_EXP_LNKCAP_ASPMS	0xc00	/* ASPM Support */
#परिभाषय  PCI_EXP_LNKCAP_L0SEL	0x7000	/* L0s Exit Latency */
#परिभाषय  PCI_EXP_LNKCAP_L1EL	0x38000	/* L1 Exit Latency */
#परिभाषय  PCI_EXP_LNKCAP_CLKPM	0x40000	/* L1 Clock Power Management */
#परिभाषय PCI_EXP_LNKCTL		16	/* Link Control */
#परिभाषय  PCI_EXP_LNKCTL_RL	0x20	/* Retrain Link */
#परिभाषय  PCI_EXP_LNKCTL_CCC	0x40	/* Common Clock COnfiguration */
#परिभाषय  PCI_EXP_LNKCTL_CLKREQ_EN 0x100	/* Enable clkreq */
#परिभाषय PCI_EXP_LNKSTA		18	/* Link Status */
#परिभाषय  PCI_EXP_LNKSTA_LT	0x800	/* Link Training */
#परिभाषय  PCI_EXP_LNKSTA_SLC	0x1000	/* Slot Clock Configuration */
#परिभाषय PCI_EXP_SLTCAP		20	/* Slot Capabilities */
#परिभाषय PCI_EXP_SLTCTL		24	/* Slot Control */
#परिभाषय PCI_EXP_SLTSTA		26	/* Slot Status */
#परिभाषय PCI_EXP_RTCTL		28	/* Root Control */
#परिभाषय  PCI_EXP_RTCTL_SECEE	0x01	/* System Error on Correctable Error */
#परिभाषय  PCI_EXP_RTCTL_SENFEE	0x02	/* System Error on Non-Fatal Error */
#परिभाषय  PCI_EXP_RTCTL_SEFEE	0x04	/* System Error on Fatal Error */
#परिभाषय  PCI_EXP_RTCTL_PMEIE	0x08	/* PME Interrupt Enable */
#परिभाषय  PCI_EXP_RTCTL_CRSSVE	0x10	/* CRS Software Visibility Enable */
#परिभाषय PCI_EXP_RTCAP		30	/* Root Capabilities */
#परिभाषय PCI_EXP_RTSTA		32	/* Root Status */
#परिभाषय PCI_EXP_DEVCAP2		36	/* Device Capabilities 2 */
#परिभाषय  PCI_EXP_DEVCAP2_ARI	0x20	/* Alternative Routing-ID */
#परिभाषय PCI_EXP_DEVCTL2		40	/* Device Control 2 */
#परिभाषय  PCI_EXP_DEVCTL2_ARI	0x20	/* Alternative Routing-ID */

/* Extended Capabilities (PCI-X 2.0 and Express) */
#परिभाषय PCI_EXT_CAP_ID(शीर्षकम्)		(शीर्षकम् & 0x0000ffff)
#परिभाषय PCI_EXT_CAP_VER(शीर्षकम्)		((शीर्षकम् >> 16) & 0xf)
#परिभाषय PCI_EXT_CAP_अग्र(शीर्षकम्)	((शीर्षकम् >> 20) & 0xffc)

#परिभाषय PCI_EXT_CAP_ID_ERR	1
#परिभाषय PCI_EXT_CAP_ID_VC	2
#परिभाषय PCI_EXT_CAP_ID_DSN	3
#परिभाषय PCI_EXT_CAP_ID_PWR	4
#परिभाषय PCI_EXT_CAP_ID_ARI	14

/* Advanced Error Reporting */
#परिभाषय PCI_ERR_UNCOR_स्थितिः	4	/* Uncorrectable Error Status */
#परिभाषय  PCI_ERR_UNC_TRAIN	0x00000001	/* Training */
#परिभाषय  PCI_ERR_UNC_DLP	0x00000010	/* Data Link Protocol */
#परिभाषय  PCI_ERR_UNC_POISON_TLP	0x00001000	/* Poisoned TLP */
#परिभाषय  PCI_ERR_UNC_FCP	0x00002000	/* Flow Control Protocol */
#परिभाषय  PCI_ERR_UNC_COMP_TIME	0x00004000	/* Completion Timeout */
#परिभाषय  PCI_ERR_UNC_COMP_ABORT	0x00008000	/* Completer Abort */
#परिभाषय  PCI_ERR_UNC_UNX_COMP	0x00010000	/* Unexpected Completion */
#परिभाषय  PCI_ERR_UNC_RX_OVER	0x00020000	/* Receiver Overflow */
#परिभाषय  PCI_ERR_UNC_MALF_TLP	0x00040000	/* Malformed TLP */
#परिभाषय  PCI_ERR_UNC_ECRC	0x00080000	/* ECRC Error Status */
#परिभाषय  PCI_ERR_UNC_UNSUP	0x00100000	/* Unsupported Request */
#परिभाषय PCI_ERR_UNCOR_आवरणम्	8	/* Uncorrectable Error Mask */
	/* Same bits as above */
#परिभाषय PCI_ERR_UNCOR_SEVER	12	/* Uncorrectable Error Severity */
	/* Same bits as above */
#परिभाषय PCI_ERR_COR_स्थितिः	16	/* Correctable Error Status */
#परिभाषय  PCI_ERR_COR_RCVR	0x00000001	/* Receiver Error Status */
#परिभाषय  PCI_ERR_COR_BAD_TLP	0x00000040	/* Bad TLP Status */
#परिभाषय  PCI_ERR_COR_BAD_DLLP	0x00000080	/* Bad DLLP Status */
#परिभाषय  PCI_ERR_COR_REP_ROLL	0x00000100	/* REPLAY_NUM Rollover */
#परिभाषय  PCI_ERR_COR_REP_TIMER	0x00001000	/* Replay Timer Timeout */
#परिभाषय PCI_ERR_COR_आवरणम्	20	/* Correctable Error Mask */
	/* Same bits as above */
#परिभाषय PCI_ERR_CAP		24	/* Advanced Error Capabilities */
#परिभाषय  PCI_ERR_CAP_FEP(x)	((x) & 31)	/* First Error Pointer */
#परिभाषय  PCI_ERR_CAP_ECRC_GENC	0x00000020	/* ECRC Generation Capable */
#परिभाषय  PCI_ERR_CAP_ECRC_GENE	0x00000040	/* ECRC Generation Enable */
#परिभाषय  PCI_ERR_CAP_ECRC_CHKC	0x00000080	/* ECRC Check Capable */
#परिभाषय  PCI_ERR_CAP_ECRC_CHKE	0x00000100	/* ECRC Check Enable */
#परिभाषय PCI_ERR_शीर्षकम्_LOG	28	/* Header Log Register (16 bytes) */
#परिभाषय PCI_ERR_मूलम्_COMMAND	44	/* Root Error Command */
/* Correctable Err Reporting Enable */
#परिभाषय PCI_ERR_मूलम्_CMD_COR_EN		0x00000001
/* Non-fatal Err Reporting Enable */
#परिभाषय PCI_ERR_मूलम्_CMD_NONFATAL_EN	0x00000002
/* Fatal Err Reporting Enable */
#परिभाषय PCI_ERR_मूलम्_CMD_FATAL_EN	0x00000004
#परिभाषय PCI_ERR_मूलम्_स्थितिः	48
#परिभाषय PCI_ERR_मूलम्_COR_RCV		0x00000001	/* ERR_COR Received */
/* Multi ERR_COR Received */
#परिभाषय PCI_ERR_मूलम्_MULTI_COR_RCV	0x00000002
/* ERR_FATAL/NONFATAL Recevied */
#परिभाषय PCI_ERR_मूलम्_UNCOR_RCV		0x00000004
/* Multi ERR_FATAL/NONFATAL Recevied */
#परिभाषय PCI_ERR_मूलम्_MULTI_UNCOR_RCV	0x00000008
#परिभाषय PCI_ERR_मूलम्_FIRST_FATAL	0x00000010	/* First Fatal */
#परिभाषय PCI_ERR_मूलम्_NONFATAL_RCV	0x00000020	/* Non-Fatal Received */
#परिभाषय PCI_ERR_मूलम्_FATAL_RCV		0x00000040	/* Fatal Received */
#परिभाषय PCI_ERR_मूलम्_COR_SRC	52
#परिभाषय PCI_ERR_मूलम्_SRC	54

/* Virtual Channel */
#परिभाषय PCI_VC_पत्तनम्_REG1	4
#परिभाषय PCI_VC_पत्तनम्_REG2	8
#परिभाषय PCI_VC_पत्तनम्_CTRL	12
#परिभाषय PCI_VC_पत्तनम्_स्थितिः	14
#परिभाषय PCI_VC_RES_CAP		16
#परिभाषय PCI_VC_RES_CTRL		20
#परिभाषय PCI_VC_RES_स्थितिः	26

/* Power Budgeting */
#परिभाषय PCI_PWR_DSR		4	/* Data Select Register */
#परिभाषय PCI_PWR_दत्त		8	/* Data Register */
#परिभाषय  PCI_PWR_दत्त_आधारः(x)	((x) & 0xff)	    /* Base Power */
#परिभाषय  PCI_PWR_दत्त_SCALE(x)	(((x) >> 8) & 3)    /* Data Scale */
#परिभाषय  PCI_PWR_दत्त_PM_SUB(x)	(((x) >> 10) & 7)   /* PM Sub State */
#परिभाषय  PCI_PWR_दत्त_PM_STATE(x) (((x) >> 13) & 3) /* PM State */
#परिभाषय  PCI_PWR_दत्त_TYPE(x)	(((x) >> 15) & 7)   /* Type */
#परिभाषय  PCI_PWR_दत्त_RAIL(x)	(((x) >> 18) & 7)   /* Power Rail */
#परिभाषय PCI_PWR_CAP		12	/* Capability */
#परिभाषय  PCI_PWR_CAP_BUDGET(x)	((x) & 1)	/* Included in system budget */

/*
 * Hypertransport sub capability types
 *
 * Unfortunately there are both 3 bit and 5 bit capability types defined
 * in the HT spec, catering for that is a little messy. You probably don't
 * want to use these directly, just use pci_find_ht_capability() and it
 * will do the right thing for you.
 */
#परिभाषय HT_3_BIT_CAP_आवरणम्	0xE0
#परिभाषय HT_CAPTYPE_SLAVE	0x00	/* Slave/Primary link configuration */
#परिभाषय HT_CAPTYPE_HOST		0x20	/* Host/Secondary link configuration */

#परिभाषय HT_5_BIT_CAP_आवरणम्	0xF8
#परिभाषय HT_CAPTYPE_IRQ		0x80	/* IRQ Configuration */
#परिभाषय HT_CAPTYPE_REMAPPING_40	0xA0	/* 40 bit address remapping */
#परिभाषय HT_CAPTYPE_REMAPPING_64 0xA2	/* 64 bit address remapping */
#परिभाषय HT_CAPTYPE_UNITID_CLUMP	0x90	/* Unit ID clumping */
#परिभाषय HT_CAPTYPE_EXTCONF	0x98	/* Extended Configuration Space Access */
#परिभाषय HT_CAPTYPE_MSI_MAPPING	0xA8	/* MSI Mapping Capability */
#परिभाषय  HT_MSI_FLAGS		0x02		/* Offset to flags */
#परिभाषय  HT_MSI_FLAGS_ENABLE	0x1		/* Mapping enable */
#परिभाषय  HT_MSI_FLAGS_FIXED	0x2		/* Fixed mapping only */
#परिभाषय  HT_MSI_FIXED_ADDR	0x00000000FEE00000ULL	/* Fixed addr */
#परिभाषय  HT_MSI_ADDR_LO		0x04		/* Offset to low addr bits */
#परिभाषय  HT_MSI_ADDR_LO_आवरणम्	0xFFF00000	/* Low address bit mask */
#परिभाषय  HT_MSI_ADDR_HI		0x08		/* Offset to high addr bits */
#परिभाषय HT_CAPTYPE_प्रत्यक्षम्_ROUTE	0xB0	/* Direct routing configuration */
#परिभाषय HT_CAPTYPE_VCSET	0xB8	/* Virtual Channel configuration */
#परिभाषय HT_CAPTYPE_ERROR_RETRY	0xC0	/* Retry on error configuration */
#परिभाषय HT_CAPTYPE_GEN3		0xD0	/* Generation 3 hypertransport configuration */
#परिभाषय HT_CAPTYPE_PM		0xE0	/* Hypertransport powermanagement configuration */

/* Alternative Routing-ID Interpretation */
#परिभाषय PCI_ARI_CAP		0x04	/* ARI Capability Register */
#परिभाषय  PCI_ARI_CAP_MFVC	0x0001	/* MFVC Function Groups Capability */
#परिभाषय  PCI_ARI_CAP_ACS	0x0002	/* ACS Function Groups Capability */
#परिभाषय  PCI_ARI_CAP_NFN(x)	(((x) >> 8) & 0xff) /* Next Function Number */
#परिभाषय PCI_ARI_CTRL		0x06	/* ARI Control Register */
#परिभाषय  PCI_ARI_CTRL_MFVC	0x0001	/* MFVC Function Groups Enable */
#परिभाषय  PCI_ARI_CTRL_ACS	0x0002	/* ACS Function Groups Enable */
#परिभाषय  PCI_ARI_CTRL_FG(x)	(((x) >> 4) & 7) /* Function Group */

#यदि_अन्तः /* LINUX_PCI_REGS_H */
