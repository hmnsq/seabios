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

#که_نه_تعریف_شوی LINUX_PCI_REGS_H
#تعریف_کړه LINUX_PCI_REGS_H

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#تعریف_کړه PCI_VENDOR_ID		0x00	/* 16 bits */
#تعریف_کړه PCI_DEVICE_ID		0x02	/* 16 bits */
#تعریف_کړه PCI_COMMAND		0x04	/* 16 bits */
#تعریف_کړه  PCI_COMMAND_IO		0x1	/* Enable response in I/O space */
#تعریف_کړه  PCI_COMMAND_حافظه	0x2	/* Enable response in Memory space */
#تعریف_کړه  PCI_COMMAND_MASTER	0x4	/* Enable bus mastering */
#تعریف_کړه  PCI_COMMAND_SPECIAL	0x8	/* Enable response to special cycles */
#تعریف_کړه  PCI_COMMAND_INVALIDATE	0x10	/* Use memory write and invalidate */
#تعریف_کړه  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#تعریف_کړه  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#تعریف_کړه  PCI_COMMAND_WAIT 	0x80	/* Enable address/data stepping */
#تعریف_کړه  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#تعریف_کړه  PCI_COMMAND_FAST_شا	0x200	/* Enable back-to-back writes */
#تعریف_کړه  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

#تعریف_کړه PCI_حالت		0x06	/* 16 bits */
#تعریف_کړه  PCI_حالت_CAP_LIST	0x10	/* Support Capability List */
#تعریف_کړه  PCI_حالت_66_MHZ	0x20	/* Support 66 Mhz PCI 2.1 bus */
#تعریف_کړه  PCI_حالت_UDF		0x40	/* Support User Definable Features [obsolete] */
#تعریف_کړه  PCI_حالت_FAST_شا	0x80	/* Accept fast-back to back */
#تعریف_کړه  PCI_حالت_PARITY	0x100	/* Detected parity error */
#تعریف_کړه  PCI_حالت_DEVSEL_ماسک	0x600	/* DEVSEL timing */
#تعریف_کړه  PCI_حالت_DEVSEL_FAST		0x000
#تعریف_کړه  PCI_حالت_DEVSEL_MEDIUM	0x200
#تعریف_کړه  PCI_حالت_DEVSEL_SLOW		0x400
#تعریف_کړه  PCI_حالت_SIG_موخه_ABORT	0x800 /* Set on target abort */
#تعریف_کړه  PCI_حالت_REC_موخه_ABORT	0x1000 /* Master ack of " */
#تعریف_کړه  PCI_حالت_REC_MASTER_ABORT	0x2000 /* Set on master abort */
#تعریف_کړه  PCI_حالت_SIG_SYSTEM_ERROR	0x4000 /* Set when we drive SERR */
#تعریف_کړه  PCI_حالت_DETECTED_PARITY	0x8000 /* Set on parity error */

#تعریف_کړه PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8 revision */
#تعریف_کړه PCI_REVISION_ID		0x08	/* Revision ID */
#تعریف_کړه PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#تعریف_کړه PCI_CLASS_DEVICE	0x0a	/* Device class */

#تعریف_کړه PCI_CACHE_کرښه_اندازه	0x0c	/* 8 bits */
#تعریف_کړه PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#تعریف_کړه PCI_سرلیک_TYPE		0x0e	/* 8 bits */
#تعریف_کړه  PCI_سرلیک_TYPE_نورمال		0
#تعریف_کړه  PCI_سرلیک_TYPE_BRIDGE		1
#تعریف_کړه  PCI_سرلیک_TYPE_CARDBUS	2

#تعریف_کړه PCI_BIST		0x0f	/* 8 bits */
#تعریف_کړه  PCI_BIST_کوډ_ماسک	0x0f	/* Return result */
#تعریف_کړه  PCI_BIST_پیل		0x40	/* 1 to start BIST, 2 secs or less */
#تعریف_کړه  PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#تعریف_کړه PCI_بنسټ_پته_0	0x10	/* 32 bits */
#تعریف_کړه PCI_بنسټ_پته_1	0x14	/* 32 bits [htype 0,1 only] */
#تعریف_کړه PCI_بنسټ_پته_2	0x18	/* 32 bits [htype 0 only] */
#تعریف_کړه PCI_بنسټ_پته_3	0x1c	/* 32 bits */
#تعریف_کړه PCI_بنسټ_پته_4	0x20	/* 32 bits */
#تعریف_کړه PCI_بنسټ_پته_5	0x24	/* 32 bits */
#تعریف_کړه  PCI_بنسټ_پته_SPACE		0x01	/* 0 = memory, 1 = I/O */
#تعریف_کړه  PCI_بنسټ_پته_SPACE_IO	0x01
#تعریف_کړه  PCI_بنسټ_پته_SPACE_حافظه	0x00
#تعریف_کړه  PCI_بنسټ_پته_MEM_TYPE_ماسک	0x06
#تعریف_کړه  PCI_بنسټ_پته_MEM_TYPE_32	0x00	/* 32 bit address */
#تعریف_کړه  PCI_بنسټ_پته_MEM_TYPE_1_M	0x02	/* Below 1M [obsolete] */
#تعریف_کړه  PCI_بنسټ_پته_MEM_TYPE_64	0x04	/* 64 bit address */
#تعریف_کړه  PCI_بنسټ_پته_MEM_PREFETCH	0x08	/* prefetchable? */
#تعریف_کړه  PCI_بنسټ_پته_MEM_ماسک	(~0x0fUL)
#تعریف_کړه  PCI_بنسټ_پته_IO_ماسک	(~0x03UL)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#تعریف_کړه PCI_CARDBUS_CIS		0x28
#تعریف_کړه PCI_SUBSYSTEM_VENDOR_ID	0x2c
#تعریف_کړه PCI_SUBSYSTEM_ID	0x2e
#تعریف_کړه PCI_ROM_پته		0x30	/* Bits 31..11 are address, 10..1 reserved */
#تعریف_کړه  PCI_ROM_پته_ENABLE	0x01
#تعریف_کړه PCI_ROM_پته_ماسک	(~0x7ffUL)

#تعریف_کړه PCI_CAPABILITY_LIST	0x34	/* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#تعریف_کړه PCI_مداخله_کرښه	0x3c	/* 8 bits */
#تعریف_کړه PCI_مداخله_PIN	0x3d	/* 8 bits */
#تعریف_کړه PCI_MIN_GNT		0x3e	/* 8 bits */
#تعریف_کړه PCI_MAX_LAT		0x3f	/* 8 bits */

/* Header type 1 (PCI-to-PCI bridges) */
#تعریف_کړه PCI_PRIMARY_BUS		0x18	/* Primary bus number */
#تعریف_کړه PCI_SECONDARY_BUS	0x19	/* Secondary bus number */
#تعریف_کړه PCI_SUBORDINATE_BUS	0x1a	/* Highest bus number behind the bridge */
#تعریف_کړه PCI_SEC_LATENCY_TIMER	0x1b	/* Latency timer for secondary interface */
#تعریف_کړه PCI_IO_بنسټ		0x1c	/* I/O range behind the bridge */
#تعریف_کړه PCI_IO_حد		0x1d
#تعریف_کړه  PCI_IO_RANGE_TYPE_ماسک	0x0fUL	/* I/O bridging type */
#تعریف_کړه  PCI_IO_RANGE_TYPE_16	0x00
#تعریف_کړه  PCI_IO_RANGE_TYPE_32	0x01
#تعریف_کړه  PCI_IO_RANGE_ماسک	(~0x0fUL)
#تعریف_کړه PCI_SEC_حالت		0x1e	/* Secondary status register, only bit 14 used */
#تعریف_کړه PCI_حافظه_بنسټ		0x20	/* Memory range behind */
#تعریف_کړه PCI_حافظه_حد	0x22
#تعریف_کړه  PCI_حافظه_RANGE_TYPE_ماسک 0x0fUL
#تعریف_کړه  PCI_حافظه_RANGE_ماسک	(~0x0fUL)
#تعریف_کړه PCI_PREF_حافظه_بنسټ	0x24	/* Prefetchable memory range behind */
#تعریف_کړه PCI_PREF_حافظه_حد	0x26
#تعریف_کړه  PCI_PREF_RANGE_TYPE_ماسک 0x0fUL
#تعریف_کړه  PCI_PREF_RANGE_TYPE_32	0x00
#تعریف_کړه  PCI_PREF_RANGE_TYPE_64	0x01
#تعریف_کړه  PCI_PREF_RANGE_ماسک	(~0x0fUL)
#تعریف_کړه PCI_PREF_بنسټ_UPPER32	0x28	/* Upper half of prefetchable memory range */
#تعریف_کړه PCI_PREF_حد_UPPER32	0x2c
#تعریف_کړه PCI_IO_بنسټ_UPPER16	0x30	/* Upper half of I/O addresses */
#تعریف_کړه PCI_IO_حد_UPPER16	0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#تعریف_کړه PCI_ROM_ADDRESS1	0x38	/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#تعریف_کړه PCI_BRIDGE_CONTROL	0x3e
#تعریف_کړه  PCI_BRIDGE_CTL_PARITY	0x01	/* Enable parity detection on secondary interface */
#تعریف_کړه  PCI_BRIDGE_CTL_SERR	0x02	/* The same for SERR forwarding */
#تعریف_کړه  PCI_BRIDGE_CTL_ISA	0x04	/* Enable ISA mode */
#تعریف_کړه  PCI_BRIDGE_CTL_VGA	0x08	/* Forward VGA addresses */
#تعریف_کړه  PCI_BRIDGE_CTL_MASTER_ABORT	0x20  /* Report master aborts */
#تعریف_کړه  PCI_BRIDGE_CTL_BUS_RESET	0x40	/* Secondary bus reset */
#تعریف_کړه  PCI_BRIDGE_CTL_FAST_شا	0x80	/* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#تعریف_کړه PCI_CB_CAPABILITY_LIST	0x14
/* 0x15 reserved */
#تعریف_کړه PCI_CB_SEC_حالت	0x16	/* Secondary status */
#تعریف_کړه PCI_CB_PRIMARY_BUS	0x18	/* PCI bus number */
#تعریف_کړه PCI_CB_CARD_BUS		0x19	/* CardBus bus number */
#تعریف_کړه PCI_CB_SUBORDINATE_BUS	0x1a	/* Subordinate bus number */
#تعریف_کړه PCI_CB_LATENCY_TIMER	0x1b	/* CardBus latency timer */
#تعریف_کړه PCI_CB_حافظه_بنسټ_0	0x1c
#تعریف_کړه PCI_CB_حافظه_حد_0	0x20
#تعریف_کړه PCI_CB_حافظه_بنسټ_1	0x24
#تعریف_کړه PCI_CB_حافظه_حد_1	0x28
#تعریف_کړه PCI_CB_IO_بنسټ_0	0x2c
#تعریف_کړه PCI_CB_IO_بنسټ_0_HI	0x2e
#تعریف_کړه PCI_CB_IO_حد_0	0x30
#تعریف_کړه PCI_CB_IO_حد_0_HI	0x32
#تعریف_کړه PCI_CB_IO_بنسټ_1	0x34
#تعریف_کړه PCI_CB_IO_بنسټ_1_HI	0x36
#تعریف_کړه PCI_CB_IO_حد_1	0x38
#تعریف_کړه PCI_CB_IO_حد_1_HI	0x3a
#تعریف_کړه  PCI_CB_IO_RANGE_ماسک	(~0x03UL)
/* 0x3c-0x3d are same as for htype 0 */
#تعریف_کړه PCI_CB_BRIDGE_CONTROL	0x3e
#تعریف_کړه  PCI_CB_BRIDGE_CTL_PARITY	0x01	/* Similar to standard bridge control register */
#تعریف_کړه  PCI_CB_BRIDGE_CTL_SERR		0x02
#تعریف_کړه  PCI_CB_BRIDGE_CTL_ISA		0x04
#تعریف_کړه  PCI_CB_BRIDGE_CTL_VGA		0x08
#تعریف_کړه  PCI_CB_BRIDGE_CTL_MASTER_ABORT	0x20
#تعریف_کړه  PCI_CB_BRIDGE_CTL_CB_RESET	0x40	/* CardBus reset */
#تعریف_کړه  PCI_CB_BRIDGE_CTL_16_بټ_INT	0x80	/* Enable interrupt for 16-bit cards */
#تعریف_کړه  PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 0x100	/* Prefetch enable for both memory regions */
#تعریف_کړه  PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 0x200
#تعریف_کړه  PCI_CB_BRIDGE_CTL_POST_WRITES	0x400
#تعریف_کړه PCI_CB_SUBSYSTEM_VENDOR_ID	0x40
#تعریف_کړه PCI_CB_SUBSYSTEM_ID		0x42
#تعریف_کړه PCI_CB_LEGACY_ډېرپېښ_بنسټ		0x44	/* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability lists */

#تعریف_کړه PCI_CAP_LIST_ID		0	/* Capability ID */
#تعریف_کړه  PCI_CAP_ID_PM		0x01	/* Power Management */
#تعریف_کړه  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#تعریف_کړه  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#تعریف_کړه  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#تعریف_کړه  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#تعریف_کړه  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#تعریف_کړه  PCI_CAP_ID_PCIX	0x07	/* PCI-X */
#تعریف_کړه  PCI_CAP_ID_HT		0x08	/* HyperTransport */
#تعریف_کړه  PCI_CAP_ID_VNDR	0x09	/* Vendor specific */
#تعریف_کړه  PCI_CAP_ID_DBG		0x0A	/* Debug port */
#تعریف_کړه  PCI_CAP_ID_CCRC	0x0B	/* CompactPCI Central Resource Control */
#تعریف_کړه  PCI_CAP_ID_SHPC 	0x0C	/* PCI Standard Hot-Plug Controller */
#تعریف_کړه  PCI_CAP_ID_SSVID	0x0D	/* Bridge subsystem vendor/device ID */
#تعریف_کړه  PCI_CAP_ID_AGP3	0x0E	/* AGP Target PCI-PCI bridge */
#تعریف_کړه  PCI_CAP_ID_EXP 	0x10	/* PCI Express */
#تعریف_کړه  PCI_CAP_ID_MSIX	0x11	/* MSI-X */
#تعریف_کړه PCI_CAP_LIST_بل	1	/* Next capability in the list */
#تعریف_کړه PCI_CAP_FLAGS		2	/* Capability defined flags (16 bits) */
#تعریف_کړه PCI_CAP_SIZEOF		4

/* Power Management Registers */

#تعریف_کړه PCI_PM_PMC		2	/* PM Capabilities Register */
#تعریف_کړه  PCI_PM_CAP_VER_ماسک	0x0007	/* Version */
#تعریف_کړه  PCI_PM_CAP_PME_CLOCK	0x0008	/* PME clock required */
#تعریف_کړه  PCI_PM_CAP_RESERVED    0x0010  /* Reserved field */
#تعریف_کړه  PCI_PM_CAP_DSI		0x0020	/* Device specific initialization */
#تعریف_کړه  PCI_PM_CAP_AUX_POWER	0x01C0	/* Auxilliary power support mask */
#تعریف_کړه  PCI_PM_CAP_D1		0x0200	/* D1 power state support */
#تعریف_کړه  PCI_PM_CAP_D2		0x0400	/* D2 power state support */
#تعریف_کړه  PCI_PM_CAP_PME		0x0800	/* PME pin supported */
#تعریف_کړه  PCI_PM_CAP_PME_ماسک	0xF800	/* PME Mask of all supported states */
#تعریف_کړه  PCI_PM_CAP_PME_D0	0x0800	/* PME# from D0 */
#تعریف_کړه  PCI_PM_CAP_PME_D1	0x1000	/* PME# from D1 */
#تعریف_کړه  PCI_PM_CAP_PME_D2	0x2000	/* PME# from D2 */
#تعریف_کړه  PCI_PM_CAP_PME_D3	0x4000	/* PME# from D3 (hot) */
#تعریف_کړه  PCI_PM_CAP_PME_D3cold	0x8000	/* PME# from D3 (cold) */
#تعریف_کړه  PCI_PM_CAP_PME_ښويدنه	11	/* Start of the PME Mask in PMC */
#تعریف_کړه PCI_PM_CTRL		4	/* PM control and status register */
#تعریف_کړه  PCI_PM_CTRL_STATE_ماسک	0x0003	/* Current power state (D0 to D3) */
#تعریف_کړه  PCI_PM_CTRL_NO_SOFT_RESET	0x0004	/* No reset for D3hot->D0 */
#تعریف_کړه  PCI_PM_CTRL_PME_ENABLE	0x0100	/* PME pin enable */
#تعریف_کړه  PCI_PM_CTRL_معلومات_SEL_ماسک	0x1e00	/* Data select (??) */
#تعریف_کړه  PCI_PM_CTRL_معلومات_SCALE_ماسک	0x6000	/* Data scale (??) */
#تعریف_کړه  PCI_PM_CTRL_PME_حالت	0x8000	/* PME pin status */
#تعریف_کړه PCI_PM_PPB_EXTENSIONS	6	/* PPB support extensions (??) */
#تعریف_کړه  PCI_PM_PPB_B2_B3	0x40	/* Stop clock when in D3hot (??) */
#تعریف_کړه  PCI_PM_BPCC_ENABLE	0x80	/* Bus power/clock control enable (??) */
#تعریف_کړه PCI_PM_معلومات_ثبتوونکی	7	/* (??) */
#تعریف_کړه PCI_PM_SIZEOF		8

/* AGP registers */

#تعریف_کړه PCI_AGP_نسخه		2	/* BCD version number */
#تعریف_کړه PCI_AGP_RFU		3	/* Rest of capability flags */
#تعریف_کړه PCI_AGP_حالت		4	/* Status register */
#تعریف_کړه  PCI_AGP_حالت_RQ_ماسک	0xff000000	/* Maximum number of requests - 1 */
#تعریف_کړه  PCI_AGP_حالت_SBA	0x0200	/* Sideband addressing supported */
#تعریف_کړه  PCI_AGP_حالت_64_بټ	0x0020	/* 64-bit addressing supported */
#تعریف_کړه  PCI_AGP_حالت_FW	0x0010	/* FW transfers supported */
#تعریف_کړه  PCI_AGP_حالت_RATE4	0x0004	/* 4x transfer rate supported */
#تعریف_کړه  PCI_AGP_حالت_RATE2	0x0002	/* 2x transfer rate supported */
#تعریف_کړه  PCI_AGP_حالت_RATE1	0x0001	/* 1x transfer rate supported */
#تعریف_کړه PCI_AGP_COMMAND		8	/* Control register */
#تعریف_کړه  PCI_AGP_COMMAND_RQ_ماسک 0xff000000  /* Master: Maximum number of requests */
#تعریف_کړه  PCI_AGP_COMMAND_SBA	0x0200	/* Sideband addressing enabled */
#تعریف_کړه  PCI_AGP_COMMAND_AGP	0x0100	/* Allow processing of AGP transactions */
#تعریف_کړه  PCI_AGP_COMMAND_64_بټ	0x0020 	/* Allow processing of 64-bit addresses */
#تعریف_کړه  PCI_AGP_COMMAND_FW	0x0010 	/* Force FW transfers */
#تعریف_کړه  PCI_AGP_COMMAND_RATE4	0x0004	/* Use 4x rate */
#تعریف_کړه  PCI_AGP_COMMAND_RATE2	0x0002	/* Use 2x rate */
#تعریف_کړه  PCI_AGP_COMMAND_RATE1	0x0001	/* Use 1x rate */
#تعریف_کړه PCI_AGP_SIZEOF		12

/* Vital Product Data */

#تعریف_کړه PCI_VPD_ADDR		2	/* Address to access (15 bits!) */
#تعریف_کړه  PCI_VPD_ADDR_ماسک	0x7fff	/* Address mask */
#تعریف_کړه  PCI_VPD_ADDR_F		0x8000	/* Write 0, 1 indicates completion */
#تعریف_کړه PCI_VPD_معلومات		4	/* 32-bits of data returned here */

/* Slot Identification */

#تعریف_کړه PCI_SID_ESR		2	/* Expansion Slot Register */
#تعریف_کړه  PCI_SID_ESR_NSLOTS	0x1f	/* Number of expansion slots available */
#تعریف_کړه  PCI_SID_ESR_FIC	0x20	/* First In Chassis Flag */
#تعریف_کړه PCI_SID_CHASSIS_NR	3	/* Chassis Number */

/* Message Signalled Interrupts registers */

#تعریف_کړه PCI_MSI_FLAGS		2	/* Various flags */
#تعریف_کړه  PCI_MSI_FLAGS_64_بټ	0x80	/* 64-bit addresses allowed */
#تعریف_کړه  PCI_MSI_FLAGS_QSIZE	0x70	/* Message queue size configured */
#تعریف_کړه  PCI_MSI_FLAGS_QMASK	0x0e	/* Maximum queue size available */
#تعریف_کړه  PCI_MSI_FLAGS_ENABLE	0x01	/* MSI feature enabled */
#تعریف_کړه  PCI_MSI_FLAGS_MASKBIT	0x100	/* 64-bit mask bits allowed */
#تعریف_کړه PCI_MSI_RFU		3	/* Rest of capability flags */
#تعریف_کړه PCI_MSI_پته_LO	4	/* Lower 32 bits */
#تعریف_کړه PCI_MSI_پته_HI	8	/* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#تعریف_کړه PCI_MSI_معلومات_32		8	/* 16 bits of data for 32-bit devices */
#تعریف_کړه PCI_MSI_معلومات_64		12	/* 16 bits of data for 64-bit devices */
#تعریف_کړه PCI_MSI_ماسک_بټ	16	/* Mask bits register */

/* MSI-X registers (these are at offset PCI_MSIX_FLAGS) */
#تعریف_کړه PCI_MSIX_FLAGS		2
#تعریف_کړه  PCI_MSIX_FLAGS_QSIZE	0x7FF
#تعریف_کړه  PCI_MSIX_FLAGS_ENABLE	(1 << 15)
#تعریف_کړه  PCI_MSIX_FLAGS_MASKALL	(1 << 14)
#تعریف_کړه PCI_MSIX_FLAGS_BIRMASK	(7 << 0)
#تعریف_کړه PCI_MSIX_FLAGS_BITMASK	(1 << 0)

/* CompactPCI Hotswap Register */

#تعریف_کړه PCI_CHSWP_CSR		2	/* Control and Status Register */
#تعریف_کړه  PCI_CHSWP_DHA		0x01	/* Device Hiding Arm */
#تعریف_کړه  PCI_CHSWP_EIM		0x02	/* ENUM# Signal Mask */
#تعریف_کړه  PCI_CHSWP_PIE		0x04	/* Pending Insert or Extract */
#تعریف_کړه  PCI_CHSWP_LOO		0x08	/* LED On / Off */
#تعریف_کړه  PCI_CHSWP_PI		0x30	/* Programming Interface */
#تعریف_کړه  PCI_CHSWP_EXT		0x40	/* ENUM# status - extraction */
#تعریف_کړه  PCI_CHSWP_INS		0x80	/* ENUM# status - insertion */

/* PCI-X registers */

#تعریف_کړه PCI_X_CMD		2	/* Modes & Features */
#تعریف_کړه  PCI_X_CMD_DPERR_E	0x0001	/* Data Parity Error Recovery Enable */
#تعریف_کړه  PCI_X_CMD_ERO		0x0002	/* Enable Relaxed Ordering */
#تعریف_کړه  PCI_X_CMD_READ_512	0x0000	/* 512 byte maximum read byte count */
#تعریف_کړه  PCI_X_CMD_READ_1K	0x0004	/* 1Kbyte maximum read byte count */
#تعریف_کړه  PCI_X_CMD_READ_2K	0x0008	/* 2Kbyte maximum read byte count */
#تعریف_کړه  PCI_X_CMD_READ_4K	0x000c	/* 4Kbyte maximum read byte count */
#تعریف_کړه  PCI_X_CMD_MAX_READ	0x000c	/* Max Memory Read Byte Count */
				/* Max # of outstanding split transactions */
#تعریف_کړه  PCI_X_CMD_وېشل_1	0x0000	/* Max 1 */
#تعریف_کړه  PCI_X_CMD_وېشل_2	0x0010	/* Max 2 */
#تعریف_کړه  PCI_X_CMD_وېشل_3	0x0020	/* Max 3 */
#تعریف_کړه  PCI_X_CMD_وېشل_4	0x0030	/* Max 4 */
#تعریف_کړه  PCI_X_CMD_وېشل_8	0x0040	/* Max 8 */
#تعریف_کړه  PCI_X_CMD_وېشل_12	0x0050	/* Max 12 */
#تعریف_کړه  PCI_X_CMD_وېشل_16	0x0060	/* Max 16 */
#تعریف_کړه  PCI_X_CMD_وېشل_32	0x0070	/* Max 32 */
#تعریف_کړه  PCI_X_CMD_MAX_وېشل	0x0070	/* Max Outstanding Split Transactions */
#تعریف_کړه  PCI_X_CMD_نسخه(x) 	(((x) >> 12) & 3) /* Version */
#تعریف_کړه PCI_X_حالت		4	/* PCI-X capabilities */
#تعریف_کړه  PCI_X_حالت_DEVFN	0x000000ff	/* A copy of devfn */
#تعریف_کړه  PCI_X_حالت_BUS	0x0000ff00	/* A copy of bus nr */
#تعریف_کړه  PCI_X_حالت_64_بټ	0x00010000	/* 64-bit device */
#تعریف_کړه  PCI_X_حالت_133_MHZ	0x00020000	/* 133 MHz capable */
#تعریف_کړه  PCI_X_حالت_SPL_DISC	0x00040000	/* Split Completion Discarded */
#تعریف_کړه  PCI_X_حالت_UNX_SPL	0x00080000	/* Unexpected Split Completion */
#تعریف_کړه  PCI_X_حالت_COMPLEX	0x00100000	/* Device Complexity */
#تعریف_کړه  PCI_X_حالت_MAX_READ	0x00600000	/* Designed Max Memory Read Count */
#تعریف_کړه  PCI_X_حالت_MAX_وېشل	0x03800000	/* Designed Max Outstanding Split Transactions */
#تعریف_کړه  PCI_X_حالت_MAX_CUM	0x1c000000	/* Designed Max Cumulative Read Size */
#تعریف_کړه  PCI_X_حالت_SPL_ERR	0x20000000	/* Rcvd Split Completion Error Msg */
#تعریف_کړه  PCI_X_حالت_266_MHZ	0x40000000	/* 266 MHz capable */
#تعریف_کړه  PCI_X_حالت_533_MHZ	0x80000000	/* 533 MHz capable */

/* PCI Express capability registers */

#تعریف_کړه PCI_EXP_FLAGS		2	/* Capabilities register */
#تعریف_کړه PCI_EXP_FLAGS_VERS	0x000f	/* Capability version */
#تعریف_کړه PCI_EXP_FLAGS_TYPE	0x00f0	/* Device/Port type */
#تعریف_کړه  PCI_EXP_TYPE_ENDPOINT	0x0	/* Express Endpoint */
#تعریف_کړه  PCI_EXP_TYPE_LEG_پای_ټکی	0x1	/* Legacy Endpoint */
#تعریف_کړه  PCI_EXP_TYPE_ریښه_پورټ 0x4	/* Root Port */
#تعریف_کړه  PCI_EXP_TYPE_UPSTREAM	0x5	/* Upstream Port */
#تعریف_کړه  PCI_EXP_TYPE_DOWNSTREAM 0x6	/* Downstream Port */
#تعریف_کړه  PCI_EXP_TYPE_PCI_BRIDGE 0x7	/* PCI/PCI-X Bridge */
#تعریف_کړه PCI_EXP_FLAGS_خونه	0x0100	/* Slot implemented */
#تعریف_کړه PCI_EXP_FLAGS_IRQ	0x3e00	/* Interrupt message number */
#تعریف_کړه PCI_EXP_DEVCAP		4	/* Device capabilities */
#تعریف_کړه  PCI_EXP_DEVCAP_PAYLOAD	0x07	/* Max_Payload_Size */
#تعریف_کړه  PCI_EXP_DEVCAP_PHANTOM	0x18	/* Phantom functions */
#تعریف_کړه  PCI_EXP_DEVCAP_EXT_نښان	0x20	/* Extended tags */
#تعریف_کړه  PCI_EXP_DEVCAP_L0S	0x1c0	/* L0s Acceptable Latency */
#تعریف_کړه  PCI_EXP_DEVCAP_L1	0xe00	/* L1 Acceptable Latency */
#تعریف_کړه  PCI_EXP_DEVCAP_ATN_BUT	0x1000	/* Attention Button Present */
#تعریف_کړه  PCI_EXP_DEVCAP_ATN_IND	0x2000	/* Attention Indicator Present */
#تعریف_کړه  PCI_EXP_DEVCAP_PWR_IND	0x4000	/* Power Indicator Present */
#تعریف_کړه  PCI_EXP_DEVCAP_RBER	0x8000	/* Role-Based Error Reporting */
#تعریف_کړه  PCI_EXP_DEVCAP_PWR_VAL	0x3fc0000 /* Slot Power Limit Value */
#تعریف_کړه  PCI_EXP_DEVCAP_PWR_SCL	0xc000000 /* Slot Power Limit Scale */
#تعریف_کړه  PCI_EXP_DEVCAP_FLR     0x10000000 /* Function Level Reset */
#تعریف_کړه PCI_EXP_DEVCTL		8	/* Device Control */
#تعریف_کړه  PCI_EXP_DEVCTL_CERE	0x0001	/* Correctable Error Reporting En. */
#تعریف_کړه  PCI_EXP_DEVCTL_NFERE	0x0002	/* Non-Fatal Error Reporting Enable */
#تعریف_کړه  PCI_EXP_DEVCTL_FERE	0x0004	/* Fatal Error Reporting Enable */
#تعریف_کړه  PCI_EXP_DEVCTL_URRE	0x0008	/* Unsupported Request Reporting En. */
#تعریف_کړه  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#تعریف_کړه  PCI_EXP_DEVCTL_PAYLOAD	0x00e0	/* Max_Payload_Size */
#تعریف_کړه  PCI_EXP_DEVCTL_EXT_نښان	0x0100	/* Extended Tag Field Enable */
#تعریف_کړه  PCI_EXP_DEVCTL_PHANTOM	0x0200	/* Phantom Functions Enable */
#تعریف_کړه  PCI_EXP_DEVCTL_AUX_PME	0x0400	/* Auxiliary Power PM Enable */
#تعریف_کړه  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#تعریف_کړه  PCI_EXP_DEVCTL_READRQ	0x7000	/* Max_Read_Request_Size */
#تعریف_کړه  PCI_EXP_DEVCTL_BCR_FLR 0x8000  /* Bridge Configuration Retry / FLR */
#تعریف_کړه PCI_EXP_DEVSTA		10	/* Device Status */
#تعریف_کړه  PCI_EXP_DEVSTA_CED	0x01	/* Correctable Error Detected */
#تعریف_کړه  PCI_EXP_DEVSTA_NFED	0x02	/* Non-Fatal Error Detected */
#تعریف_کړه  PCI_EXP_DEVSTA_FED	0x04	/* Fatal Error Detected */
#تعریف_کړه  PCI_EXP_DEVSTA_URD	0x08	/* Unsupported Request Detected */
#تعریف_کړه  PCI_EXP_DEVSTA_AUXPD	0x10	/* AUX Power Detected */
#تعریف_کړه  PCI_EXP_DEVSTA_TRPND	0x20	/* Transactions Pending */
#تعریف_کړه PCI_EXP_LNKCAP		12	/* Link Capabilities */
#تعریف_کړه  PCI_EXP_LNKCAP_ASPMS	0xc00	/* ASPM Support */
#تعریف_کړه  PCI_EXP_LNKCAP_L0SEL	0x7000	/* L0s Exit Latency */
#تعریف_کړه  PCI_EXP_LNKCAP_L1EL	0x38000	/* L1 Exit Latency */
#تعریف_کړه  PCI_EXP_LNKCAP_CLKPM	0x40000	/* L1 Clock Power Management */
#تعریف_کړه PCI_EXP_LNKCTL		16	/* Link Control */
#تعریف_کړه  PCI_EXP_LNKCTL_RL	0x20	/* Retrain Link */
#تعریف_کړه  PCI_EXP_LNKCTL_CCC	0x40	/* Common Clock COnfiguration */
#تعریف_کړه  PCI_EXP_LNKCTL_CLKREQ_EN 0x100	/* Enable clkreq */
#تعریف_کړه PCI_EXP_LNKSTA		18	/* Link Status */
#تعریف_کړه  PCI_EXP_LNKSTA_LT	0x800	/* Link Training */
#تعریف_کړه  PCI_EXP_LNKSTA_SLC	0x1000	/* Slot Clock Configuration */
#تعریف_کړه PCI_EXP_SLTCAP		20	/* Slot Capabilities */
#تعریف_کړه PCI_EXP_SLTCTL		24	/* Slot Control */
#تعریف_کړه PCI_EXP_SLTSTA		26	/* Slot Status */
#تعریف_کړه PCI_EXP_RTCTL		28	/* Root Control */
#تعریف_کړه  PCI_EXP_RTCTL_SECEE	0x01	/* System Error on Correctable Error */
#تعریف_کړه  PCI_EXP_RTCTL_SENFEE	0x02	/* System Error on Non-Fatal Error */
#تعریف_کړه  PCI_EXP_RTCTL_SEFEE	0x04	/* System Error on Fatal Error */
#تعریف_کړه  PCI_EXP_RTCTL_PMEIE	0x08	/* PME Interrupt Enable */
#تعریف_کړه  PCI_EXP_RTCTL_CRSSVE	0x10	/* CRS Software Visibility Enable */
#تعریف_کړه PCI_EXP_RTCAP		30	/* Root Capabilities */
#تعریف_کړه PCI_EXP_RTSTA		32	/* Root Status */
#تعریف_کړه PCI_EXP_DEVCAP2		36	/* Device Capabilities 2 */
#تعریف_کړه  PCI_EXP_DEVCAP2_ARI	0x20	/* Alternative Routing-ID */
#تعریف_کړه PCI_EXP_DEVCTL2		40	/* Device Control 2 */
#تعریف_کړه  PCI_EXP_DEVCTL2_ARI	0x20	/* Alternative Routing-ID */

/* Extended Capabilities (PCI-X 2.0 and Express) */
#تعریف_کړه PCI_EXT_CAP_ID(سرلیک)		(سرلیک & 0x0000ffff)
#تعریف_کړه PCI_EXT_CAP_VER(سرلیک)		((سرلیک >> 16) & 0xf)
#تعریف_کړه PCI_EXT_CAP_بل(سرلیک)	((سرلیک >> 20) & 0xffc)

#تعریف_کړه PCI_EXT_CAP_ID_ERR	1
#تعریف_کړه PCI_EXT_CAP_ID_VC	2
#تعریف_کړه PCI_EXT_CAP_ID_DSN	3
#تعریف_کړه PCI_EXT_CAP_ID_PWR	4
#تعریف_کړه PCI_EXT_CAP_ID_ARI	14

/* Advanced Error Reporting */
#تعریف_کړه PCI_ERR_UNCOR_حالت	4	/* Uncorrectable Error Status */
#تعریف_کړه  PCI_ERR_UNC_TRAIN	0x00000001	/* Training */
#تعریف_کړه  PCI_ERR_UNC_DLP	0x00000010	/* Data Link Protocol */
#تعریف_کړه  PCI_ERR_UNC_POISON_TLP	0x00001000	/* Poisoned TLP */
#تعریف_کړه  PCI_ERR_UNC_FCP	0x00002000	/* Flow Control Protocol */
#تعریف_کړه  PCI_ERR_UNC_COMP_وخت	0x00004000	/* Completion Timeout */
#تعریف_کړه  PCI_ERR_UNC_COMP_ABORT	0x00008000	/* Completer Abort */
#تعریف_کړه  PCI_ERR_UNC_UNX_COMP	0x00010000	/* Unexpected Completion */
#تعریف_کړه  PCI_ERR_UNC_RX_OVER	0x00020000	/* Receiver Overflow */
#تعریف_کړه  PCI_ERR_UNC_MALF_TLP	0x00040000	/* Malformed TLP */
#تعریف_کړه  PCI_ERR_UNC_ECRC	0x00080000	/* ECRC Error Status */
#تعریف_کړه  PCI_ERR_UNC_UNSUP	0x00100000	/* Unsupported Request */
#تعریف_کړه PCI_ERR_UNCOR_ماسک	8	/* Uncorrectable Error Mask */
	/* Same bits as above */
#تعریف_کړه PCI_ERR_UNCOR_SEVER	12	/* Uncorrectable Error Severity */
	/* Same bits as above */
#تعریف_کړه PCI_ERR_COR_حالت	16	/* Correctable Error Status */
#تعریف_کړه  PCI_ERR_COR_RCVR	0x00000001	/* Receiver Error Status */
#تعریف_کړه  PCI_ERR_COR_BAD_TLP	0x00000040	/* Bad TLP Status */
#تعریف_کړه  PCI_ERR_COR_BAD_DLLP	0x00000080	/* Bad DLLP Status */
#تعریف_کړه  PCI_ERR_COR_REP_ROLL	0x00000100	/* REPLAY_NUM Rollover */
#تعریف_کړه  PCI_ERR_COR_REP_TIMER	0x00001000	/* Replay Timer Timeout */
#تعریف_کړه PCI_ERR_COR_ماسک	20	/* Correctable Error Mask */
	/* Same bits as above */
#تعریف_کړه PCI_ERR_CAP		24	/* Advanced Error Capabilities */
#تعریف_کړه  PCI_ERR_CAP_FEP(x)	((x) & 31)	/* First Error Pointer */
#تعریف_کړه  PCI_ERR_CAP_ECRC_GENC	0x00000020	/* ECRC Generation Capable */
#تعریف_کړه  PCI_ERR_CAP_ECRC_GENE	0x00000040	/* ECRC Generation Enable */
#تعریف_کړه  PCI_ERR_CAP_ECRC_CHKC	0x00000080	/* ECRC Check Capable */
#تعریف_کړه  PCI_ERR_CAP_ECRC_CHKE	0x00000100	/* ECRC Check Enable */
#تعریف_کړه PCI_ERR_سرلیک_LOG	28	/* Header Log Register (16 bytes) */
#تعریف_کړه PCI_ERR_ریښه_COMMAND	44	/* Root Error Command */
/* Correctable Err Reporting Enable */
#تعریف_کړه PCI_ERR_ریښه_CMD_COR_EN		0x00000001
/* Non-fatal Err Reporting Enable */
#تعریف_کړه PCI_ERR_ریښه_CMD_NONFATAL_EN	0x00000002
/* Fatal Err Reporting Enable */
#تعریف_کړه PCI_ERR_ریښه_CMD_FATAL_EN	0x00000004
#تعریف_کړه PCI_ERR_ریښه_حالت	48
#تعریف_کړه PCI_ERR_ریښه_COR_RCV		0x00000001	/* ERR_COR Received */
/* Multi ERR_COR Received */
#تعریف_کړه PCI_ERR_ریښه_MULTI_COR_RCV	0x00000002
/* ERR_FATAL/NONFATAL Recevied */
#تعریف_کړه PCI_ERR_ریښه_UNCOR_RCV		0x00000004
/* Multi ERR_FATAL/NONFATAL Recevied */
#تعریف_کړه PCI_ERR_ریښه_MULTI_UNCOR_RCV	0x00000008
#تعریف_کړه PCI_ERR_ریښه_لومړی_FATAL	0x00000010	/* First Fatal */
#تعریف_کړه PCI_ERR_ریښه_NONFATAL_RCV	0x00000020	/* Non-Fatal Received */
#تعریف_کړه PCI_ERR_ریښه_FATAL_RCV		0x00000040	/* Fatal Received */
#تعریف_کړه PCI_ERR_ریښه_COR_SRC	52
#تعریف_کړه PCI_ERR_ریښه_SRC	54

/* Virtual Channel */
#تعریف_کړه PCI_VC_پورټ_REG1	4
#تعریف_کړه PCI_VC_پورټ_REG2	8
#تعریف_کړه PCI_VC_پورټ_CTRL	12
#تعریف_کړه PCI_VC_پورټ_حالت	14
#تعریف_کړه PCI_VC_RES_CAP		16
#تعریف_کړه PCI_VC_RES_CTRL		20
#تعریف_کړه PCI_VC_RES_حالت	26

/* Power Budgeting */
#تعریف_کړه PCI_PWR_DSR		4	/* Data Select Register */
#تعریف_کړه PCI_PWR_معلومات		8	/* Data Register */
#تعریف_کړه  PCI_PWR_معلومات_بنسټ(x)	((x) & 0xff)	    /* Base Power */
#تعریف_کړه  PCI_PWR_معلومات_SCALE(x)	(((x) >> 8) & 3)    /* Data Scale */
#تعریف_کړه  PCI_PWR_معلومات_PM_SUB(x)	(((x) >> 10) & 7)   /* PM Sub State */
#تعریف_کړه  PCI_PWR_معلومات_PM_STATE(x) (((x) >> 13) & 3) /* PM State */
#تعریف_کړه  PCI_PWR_معلومات_TYPE(x)	(((x) >> 15) & 7)   /* Type */
#تعریف_کړه  PCI_PWR_معلومات_RAIL(x)	(((x) >> 18) & 7)   /* Power Rail */
#تعریف_کړه PCI_PWR_CAP		12	/* Capability */
#تعریف_کړه  PCI_PWR_CAP_BUDGET(x)	((x) & 1)	/* Included in system budget */

/*
 * Hypertransport sub capability types
 *
 * Unfortunately there are both 3 bit and 5 bit capability types defined
 * in the HT spec, catering for that is a little messy. You probably don't
 * want to use these directly, just use pci_find_ht_capability() and it
 * will do the right thing for you.
 */
#تعریف_کړه HT_3_بټ_CAP_ماسک	0xE0
#تعریف_کړه HT_CAPTYPE_SLAVE	0x00	/* Slave/Primary link configuration */
#تعریف_کړه HT_CAPTYPE_HOST		0x20	/* Host/Secondary link configuration */

#تعریف_کړه HT_5_بټ_CAP_ماسک	0xF8
#تعریف_کړه HT_CAPTYPE_IRQ		0x80	/* IRQ Configuration */
#تعریف_کړه HT_CAPTYPE_REMAPPING_40	0xA0	/* 40 bit address remapping */
#تعریف_کړه HT_CAPTYPE_REMAPPING_64 0xA2	/* 64 bit address remapping */
#تعریف_کړه HT_CAPTYPE_UNITID_CLUMP	0x90	/* Unit ID clumping */
#تعریف_کړه HT_CAPTYPE_EXTCONF	0x98	/* Extended Configuration Space Access */
#تعریف_کړه HT_CAPTYPE_MSI_MAPPING	0xA8	/* MSI Mapping Capability */
#تعریف_کړه  HT_MSI_FLAGS		0x02		/* Offset to flags */
#تعریف_کړه  HT_MSI_FLAGS_ENABLE	0x1		/* Mapping enable */
#تعریف_کړه  HT_MSI_FLAGS_FIXED	0x2		/* Fixed mapping only */
#تعریف_کړه  HT_MSI_FIXED_ADDR	0x00000000FEE00000ULL	/* Fixed addr */
#تعریف_کړه  HT_MSI_ADDR_LO		0x04		/* Offset to low addr bits */
#تعریف_کړه  HT_MSI_ADDR_LO_ماسک	0xFFF00000	/* Low address bit mask */
#تعریف_کړه  HT_MSI_ADDR_HI		0x08		/* Offset to high addr bits */
#تعریف_کړه HT_CAPTYPE_مستقیم_ROUTE	0xB0	/* Direct routing configuration */
#تعریف_کړه HT_CAPTYPE_VCSET	0xB8	/* Virtual Channel configuration */
#تعریف_کړه HT_CAPTYPE_ERROR_RETRY	0xC0	/* Retry on error configuration */
#تعریف_کړه HT_CAPTYPE_GEN3		0xD0	/* Generation 3 hypertransport configuration */
#تعریف_کړه HT_CAPTYPE_PM		0xE0	/* Hypertransport powermanagement configuration */

/* Alternative Routing-ID Interpretation */
#تعریف_کړه PCI_ARI_CAP		0x04	/* ARI Capability Register */
#تعریف_کړه  PCI_ARI_CAP_MFVC	0x0001	/* MFVC Function Groups Capability */
#تعریف_کړه  PCI_ARI_CAP_ACS	0x0002	/* ACS Function Groups Capability */
#تعریف_کړه  PCI_ARI_CAP_NFN(x)	(((x) >> 8) & 0xff) /* Next Function Number */
#تعریف_کړه PCI_ARI_CTRL		0x06	/* ARI Control Register */
#تعریف_کړه  PCI_ARI_CTRL_MFVC	0x0001	/* MFVC Function Groups Enable */
#تعریف_کړه  PCI_ARI_CTRL_ACS	0x0002	/* ACS Function Groups Enable */
#تعریف_کړه  PCI_ARI_CTRL_FG(x)	(((x) >> 4) & 7) /* Function Group */

#د_که_پای /* LINUX_PCI_REGS_H */
