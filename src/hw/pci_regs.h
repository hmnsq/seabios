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

#నిర్వచితం_కాకపోతే LINUX_PCI_REGS_H
#నిర్వచించు LINUX_PCI_REGS_H

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#నిర్వచించు PCI_VENDOR_ID		0x00	/* 16 bits */
#నిర్వచించు PCI_DEVICE_ID		0x02	/* 16 bits */
#నిర్వచించు PCI_COMMAND		0x04	/* 16 bits */
#నిర్వచించు  PCI_COMMAND_IO		0x1	/* Enable response in I/O space */
#నిర్వచించు  PCI_COMMAND_జ్ఞాపకం	0x2	/* Enable response in Memory space */
#నిర్వచించు  PCI_COMMAND_MASTER	0x4	/* Enable bus mastering */
#నిర్వచించు  PCI_COMMAND_SPECIAL	0x8	/* Enable response to special cycles */
#నిర్వచించు  PCI_COMMAND_INVALIDATE	0x10	/* Use memory write and invalidate */
#నిర్వచించు  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#నిర్వచించు  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#నిర్వచించు  PCI_COMMAND_WAIT 	0x80	/* Enable address/data stepping */
#నిర్వచించు  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#నిర్వచించు  PCI_COMMAND_FAST_వెనుక	0x200	/* Enable back-to-back writes */
#నిర్వచించు  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

#నిర్వచించు PCI_స్థితి		0x06	/* 16 bits */
#నిర్వచించు  PCI_స్థితి_CAP_LIST	0x10	/* Support Capability List */
#నిర్వచించు  PCI_స్థితి_66_MHZ	0x20	/* Support 66 Mhz PCI 2.1 bus */
#నిర్వచించు  PCI_స్థితి_UDF		0x40	/* Support User Definable Features [obsolete] */
#నిర్వచించు  PCI_స్థితి_FAST_వెనుక	0x80	/* Accept fast-back to back */
#నిర్వచించు  PCI_స్థితి_PARITY	0x100	/* Detected parity error */
#నిర్వచించు  PCI_స్థితి_DEVSEL_ముసుగు	0x600	/* DEVSEL timing */
#నిర్వచించు  PCI_స్థితి_DEVSEL_FAST		0x000
#నిర్వచించు  PCI_స్థితి_DEVSEL_MEDIUM	0x200
#నిర్వచించు  PCI_స్థితి_DEVSEL_SLOW		0x400
#నిర్వచించు  PCI_స్థితి_SIG_లక్ష్యం_ABORT	0x800 /* Set on target abort */
#నిర్వచించు  PCI_స్థితి_REC_లక్ష్యం_ABORT	0x1000 /* Master ack of " */
#నిర్వచించు  PCI_స్థితి_REC_MASTER_ABORT	0x2000 /* Set on master abort */
#నిర్వచించు  PCI_స్థితి_SIG_SYSTEM_ERROR	0x4000 /* Set when we drive SERR */
#నిర్వచించు  PCI_స్థితి_DETECTED_PARITY	0x8000 /* Set on parity error */

#నిర్వచించు PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8 revision */
#నిర్వచించు PCI_REVISION_ID		0x08	/* Revision ID */
#నిర్వచించు PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#నిర్వచించు PCI_CLASS_DEVICE	0x0a	/* Device class */

#నిర్వచించు PCI_CACHE_పంక్తి_కొలత	0x0c	/* 8 bits */
#నిర్వచించు PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#నిర్వచించు PCI_శీర్షిక_TYPE		0x0e	/* 8 bits */
#నిర్వచించు  PCI_శీర్షిక_TYPE_సాధారణ		0
#నిర్వచించు  PCI_శీర్షిక_TYPE_BRIDGE		1
#నిర్వచించు  PCI_శీర్షిక_TYPE_CARDBUS	2

#నిర్వచించు PCI_BIST		0x0f	/* 8 bits */
#నిర్వచించు  PCI_BIST_సంకేతం_ముసుగు	0x0f	/* Return result */
#నిర్వచించు  PCI_BIST_ప్రారంభం		0x40	/* 1 to start BIST, 2 secs or less */
#నిర్వచించు  PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#నిర్వచించు PCI_పునాది_చిరునామా_0	0x10	/* 32 bits */
#నిర్వచించు PCI_పునాది_చిరునామా_1	0x14	/* 32 bits [htype 0,1 only] */
#నిర్వచించు PCI_పునాది_చిరునామా_2	0x18	/* 32 bits [htype 0 only] */
#నిర్వచించు PCI_పునాది_చిరునామా_3	0x1c	/* 32 bits */
#నిర్వచించు PCI_పునాది_చిరునామా_4	0x20	/* 32 bits */
#నిర్వచించు PCI_పునాది_చిరునామా_5	0x24	/* 32 bits */
#నిర్వచించు  PCI_పునాది_చిరునామా_SPACE		0x01	/* 0 = memory, 1 = I/O */
#నిర్వచించు  PCI_పునాది_చిరునామా_SPACE_IO	0x01
#నిర్వచించు  PCI_పునాది_చిరునామా_SPACE_జ్ఞాపకం	0x00
#నిర్వచించు  PCI_పునాది_చిరునామా_MEM_TYPE_ముసుగు	0x06
#నిర్వచించు  PCI_పునాది_చిరునామా_MEM_TYPE_32	0x00	/* 32 bit address */
#నిర్వచించు  PCI_పునాది_చిరునామా_MEM_TYPE_1_M	0x02	/* Below 1M [obsolete] */
#నిర్వచించు  PCI_పునాది_చిరునామా_MEM_TYPE_64	0x04	/* 64 bit address */
#నిర్వచించు  PCI_పునాది_చిరునామా_MEM_PREFETCH	0x08	/* prefetchable? */
#నిర్వచించు  PCI_పునాది_చిరునామా_MEM_ముసుగు	(~0x0fUL)
#నిర్వచించు  PCI_పునాది_చిరునామా_IO_ముసుగు	(~0x03UL)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#నిర్వచించు PCI_CARDBUS_CIS		0x28
#నిర్వచించు PCI_SUBSYSTEM_VENDOR_ID	0x2c
#నిర్వచించు PCI_SUBSYSTEM_ID	0x2e
#నిర్వచించు PCI_ROM_చిరునామా		0x30	/* Bits 31..11 are address, 10..1 reserved */
#నిర్వచించు  PCI_ROM_చిరునామా_ENABLE	0x01
#నిర్వచించు PCI_ROM_చిరునామా_ముసుగు	(~0x7ffUL)

#నిర్వచించు PCI_CAPABILITY_LIST	0x34	/* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#నిర్వచించు PCI_అంతరాయం_పంక్తి	0x3c	/* 8 bits */
#నిర్వచించు PCI_అంతరాయం_PIN	0x3d	/* 8 bits */
#నిర్వచించు PCI_MIN_GNT		0x3e	/* 8 bits */
#నిర్వచించు PCI_MAX_LAT		0x3f	/* 8 bits */

/* Header type 1 (PCI-to-PCI bridges) */
#నిర్వచించు PCI_PRIMARY_BUS		0x18	/* Primary bus number */
#నిర్వచించు PCI_SECONDARY_BUS	0x19	/* Secondary bus number */
#నిర్వచించు PCI_SUBORDINATE_BUS	0x1a	/* Highest bus number behind the bridge */
#నిర్వచించు PCI_SEC_LATENCY_TIMER	0x1b	/* Latency timer for secondary interface */
#నిర్వచించు PCI_IO_పునాది		0x1c	/* I/O range behind the bridge */
#నిర్వచించు PCI_IO_హద్దు		0x1d
#నిర్వచించు  PCI_IO_RANGE_TYPE_ముసుగు	0x0fUL	/* I/O bridging type */
#నిర్వచించు  PCI_IO_RANGE_TYPE_16	0x00
#నిర్వచించు  PCI_IO_RANGE_TYPE_32	0x01
#నిర్వచించు  PCI_IO_RANGE_ముసుగు	(~0x0fUL)
#నిర్వచించు PCI_SEC_స్థితి		0x1e	/* Secondary status register, only bit 14 used */
#నిర్వచించు PCI_జ్ఞాపకం_పునాది		0x20	/* Memory range behind */
#నిర్వచించు PCI_జ్ఞాపకం_హద్దు	0x22
#నిర్వచించు  PCI_జ్ఞాపకం_RANGE_TYPE_ముసుగు 0x0fUL
#నిర్వచించు  PCI_జ్ఞాపకం_RANGE_ముసుగు	(~0x0fUL)
#నిర్వచించు PCI_PREF_జ్ఞాపకం_పునాది	0x24	/* Prefetchable memory range behind */
#నిర్వచించు PCI_PREF_జ్ఞాపకం_హద్దు	0x26
#నిర్వచించు  PCI_PREF_RANGE_TYPE_ముసుగు 0x0fUL
#నిర్వచించు  PCI_PREF_RANGE_TYPE_32	0x00
#నిర్వచించు  PCI_PREF_RANGE_TYPE_64	0x01
#నిర్వచించు  PCI_PREF_RANGE_ముసుగు	(~0x0fUL)
#నిర్వచించు PCI_PREF_పునాది_UPPER32	0x28	/* Upper half of prefetchable memory range */
#నిర్వచించు PCI_PREF_హద్దు_UPPER32	0x2c
#నిర్వచించు PCI_IO_పునాది_UPPER16	0x30	/* Upper half of I/O addresses */
#నిర్వచించు PCI_IO_హద్దు_UPPER16	0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#నిర్వచించు PCI_ROM_ADDRESS1	0x38	/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#నిర్వచించు PCI_BRIDGE_CONTROL	0x3e
#నిర్వచించు  PCI_BRIDGE_CTL_PARITY	0x01	/* Enable parity detection on secondary interface */
#నిర్వచించు  PCI_BRIDGE_CTL_SERR	0x02	/* The same for SERR forwarding */
#నిర్వచించు  PCI_BRIDGE_CTL_ISA	0x04	/* Enable ISA mode */
#నిర్వచించు  PCI_BRIDGE_CTL_VGA	0x08	/* Forward VGA addresses */
#నిర్వచించు  PCI_BRIDGE_CTL_MASTER_ABORT	0x20  /* Report master aborts */
#నిర్వచించు  PCI_BRIDGE_CTL_BUS_RESET	0x40	/* Secondary bus reset */
#నిర్వచించు  PCI_BRIDGE_CTL_FAST_వెనుక	0x80	/* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#నిర్వచించు PCI_CB_CAPABILITY_LIST	0x14
/* 0x15 reserved */
#నిర్వచించు PCI_CB_SEC_స్థితి	0x16	/* Secondary status */
#నిర్వచించు PCI_CB_PRIMARY_BUS	0x18	/* PCI bus number */
#నిర్వచించు PCI_CB_CARD_BUS		0x19	/* CardBus bus number */
#నిర్వచించు PCI_CB_SUBORDINATE_BUS	0x1a	/* Subordinate bus number */
#నిర్వచించు PCI_CB_LATENCY_TIMER	0x1b	/* CardBus latency timer */
#నిర్వచించు PCI_CB_జ్ఞాపకం_పునాది_0	0x1c
#నిర్వచించు PCI_CB_జ్ఞాపకం_హద్దు_0	0x20
#నిర్వచించు PCI_CB_జ్ఞాపకం_పునాది_1	0x24
#నిర్వచించు PCI_CB_జ్ఞాపకం_హద్దు_1	0x28
#నిర్వచించు PCI_CB_IO_పునాది_0	0x2c
#నిర్వచించు PCI_CB_IO_పునాది_0_HI	0x2e
#నిర్వచించు PCI_CB_IO_హద్దు_0	0x30
#నిర్వచించు PCI_CB_IO_హద్దు_0_HI	0x32
#నిర్వచించు PCI_CB_IO_పునాది_1	0x34
#నిర్వచించు PCI_CB_IO_పునాది_1_HI	0x36
#నిర్వచించు PCI_CB_IO_హద్దు_1	0x38
#నిర్వచించు PCI_CB_IO_హద్దు_1_HI	0x3a
#నిర్వచించు  PCI_CB_IO_RANGE_ముసుగు	(~0x03UL)
/* 0x3c-0x3d are same as for htype 0 */
#నిర్వచించు PCI_CB_BRIDGE_CONTROL	0x3e
#నిర్వచించు  PCI_CB_BRIDGE_CTL_PARITY	0x01	/* Similar to standard bridge control register */
#నిర్వచించు  PCI_CB_BRIDGE_CTL_SERR		0x02
#నిర్వచించు  PCI_CB_BRIDGE_CTL_ISA		0x04
#నిర్వచించు  PCI_CB_BRIDGE_CTL_VGA		0x08
#నిర్వచించు  PCI_CB_BRIDGE_CTL_MASTER_ABORT	0x20
#నిర్వచించు  PCI_CB_BRIDGE_CTL_CB_RESET	0x40	/* CardBus reset */
#నిర్వచించు  PCI_CB_BRIDGE_CTL_16_బిట్టు_INT	0x80	/* Enable interrupt for 16-bit cards */
#నిర్వచించు  PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 0x100	/* Prefetch enable for both memory regions */
#నిర్వచించు  PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 0x200
#నిర్వచించు  PCI_CB_BRIDGE_CTL_POST_WRITES	0x400
#నిర్వచించు PCI_CB_SUBSYSTEM_VENDOR_ID	0x40
#నిర్వచించు PCI_CB_SUBSYSTEM_ID		0x42
#నిర్వచించు PCI_CB_LEGACY_బహుళకం_పునాది		0x44	/* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability lists */

#నిర్వచించు PCI_CAP_LIST_ID		0	/* Capability ID */
#నిర్వచించు  PCI_CAP_ID_PM		0x01	/* Power Management */
#నిర్వచించు  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#నిర్వచించు  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#నిర్వచించు  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#నిర్వచించు  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#నిర్వచించు  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#నిర్వచించు  PCI_CAP_ID_PCIX	0x07	/* PCI-X */
#నిర్వచించు  PCI_CAP_ID_HT		0x08	/* HyperTransport */
#నిర్వచించు  PCI_CAP_ID_VNDR	0x09	/* Vendor specific */
#నిర్వచించు  PCI_CAP_ID_DBG		0x0A	/* Debug port */
#నిర్వచించు  PCI_CAP_ID_CCRC	0x0B	/* CompactPCI Central Resource Control */
#నిర్వచించు  PCI_CAP_ID_SHPC 	0x0C	/* PCI Standard Hot-Plug Controller */
#నిర్వచించు  PCI_CAP_ID_SSVID	0x0D	/* Bridge subsystem vendor/device ID */
#నిర్వచించు  PCI_CAP_ID_AGP3	0x0E	/* AGP Target PCI-PCI bridge */
#నిర్వచించు  PCI_CAP_ID_EXP 	0x10	/* PCI Express */
#నిర్వచించు  PCI_CAP_ID_MSIX	0x11	/* MSI-X */
#నిర్వచించు PCI_CAP_LIST_తదుపరి	1	/* Next capability in the list */
#నిర్వచించు PCI_CAP_FLAGS		2	/* Capability defined flags (16 bits) */
#నిర్వచించు PCI_CAP_SIZEOF		4

/* Power Management Registers */

#నిర్వచించు PCI_PM_PMC		2	/* PM Capabilities Register */
#నిర్వచించు  PCI_PM_CAP_VER_ముసుగు	0x0007	/* Version */
#నిర్వచించు  PCI_PM_CAP_PME_CLOCK	0x0008	/* PME clock required */
#నిర్వచించు  PCI_PM_CAP_RESERVED    0x0010  /* Reserved field */
#నిర్వచించు  PCI_PM_CAP_DSI		0x0020	/* Device specific initialization */
#నిర్వచించు  PCI_PM_CAP_AUX_POWER	0x01C0	/* Auxilliary power support mask */
#నిర్వచించు  PCI_PM_CAP_D1		0x0200	/* D1 power state support */
#నిర్వచించు  PCI_PM_CAP_D2		0x0400	/* D2 power state support */
#నిర్వచించు  PCI_PM_CAP_PME		0x0800	/* PME pin supported */
#నిర్వచించు  PCI_PM_CAP_PME_ముసుగు	0xF800	/* PME Mask of all supported states */
#నిర్వచించు  PCI_PM_CAP_PME_D0	0x0800	/* PME# from D0 */
#నిర్వచించు  PCI_PM_CAP_PME_D1	0x1000	/* PME# from D1 */
#నిర్వచించు  PCI_PM_CAP_PME_D2	0x2000	/* PME# from D2 */
#నిర్వచించు  PCI_PM_CAP_PME_D3	0x4000	/* PME# from D3 (hot) */
#నిర్వచించు  PCI_PM_CAP_PME_D3cold	0x8000	/* PME# from D3 (cold) */
#నిర్వచించు  PCI_PM_CAP_PME_జరుపు	11	/* Start of the PME Mask in PMC */
#నిర్వచించు PCI_PM_CTRL		4	/* PM control and status register */
#నిర్వచించు  PCI_PM_CTRL_STATE_ముసుగు	0x0003	/* Current power state (D0 to D3) */
#నిర్వచించు  PCI_PM_CTRL_NO_SOFT_RESET	0x0004	/* No reset for D3hot->D0 */
#నిర్వచించు  PCI_PM_CTRL_PME_ENABLE	0x0100	/* PME pin enable */
#నిర్వచించు  PCI_PM_CTRL_దత్తాంశం_SEL_ముసుగు	0x1e00	/* Data select (??) */
#నిర్వచించు  PCI_PM_CTRL_దత్తాంశం_SCALE_ముసుగు	0x6000	/* Data scale (??) */
#నిర్వచించు  PCI_PM_CTRL_PME_స్థితి	0x8000	/* PME pin status */
#నిర్వచించు PCI_PM_PPB_EXTENSIONS	6	/* PPB support extensions (??) */
#నిర్వచించు  PCI_PM_PPB_B2_B3	0x40	/* Stop clock when in D3hot (??) */
#నిర్వచించు  PCI_PM_BPCC_ENABLE	0x80	/* Bus power/clock control enable (??) */
#నిర్వచించు PCI_PM_దత్తాంశం_రిజిస్టరు	7	/* (??) */
#నిర్వచించు PCI_PM_SIZEOF		8

/* AGP registers */

#నిర్వచించు PCI_AGP_సంస్కరణ		2	/* BCD version number */
#నిర్వచించు PCI_AGP_RFU		3	/* Rest of capability flags */
#నిర్వచించు PCI_AGP_స్థితి		4	/* Status register */
#నిర్వచించు  PCI_AGP_స్థితి_RQ_ముసుగు	0xff000000	/* Maximum number of requests - 1 */
#నిర్వచించు  PCI_AGP_స్థితి_SBA	0x0200	/* Sideband addressing supported */
#నిర్వచించు  PCI_AGP_స్థితి_64_బిట్టు	0x0020	/* 64-bit addressing supported */
#నిర్వచించు  PCI_AGP_స్థితి_FW	0x0010	/* FW transfers supported */
#నిర్వచించు  PCI_AGP_స్థితి_RATE4	0x0004	/* 4x transfer rate supported */
#నిర్వచించు  PCI_AGP_స్థితి_RATE2	0x0002	/* 2x transfer rate supported */
#నిర్వచించు  PCI_AGP_స్థితి_RATE1	0x0001	/* 1x transfer rate supported */
#నిర్వచించు PCI_AGP_COMMAND		8	/* Control register */
#నిర్వచించు  PCI_AGP_COMMAND_RQ_ముసుగు 0xff000000  /* Master: Maximum number of requests */
#నిర్వచించు  PCI_AGP_COMMAND_SBA	0x0200	/* Sideband addressing enabled */
#నిర్వచించు  PCI_AGP_COMMAND_AGP	0x0100	/* Allow processing of AGP transactions */
#నిర్వచించు  PCI_AGP_COMMAND_64_బిట్టు	0x0020 	/* Allow processing of 64-bit addresses */
#నిర్వచించు  PCI_AGP_COMMAND_FW	0x0010 	/* Force FW transfers */
#నిర్వచించు  PCI_AGP_COMMAND_RATE4	0x0004	/* Use 4x rate */
#నిర్వచించు  PCI_AGP_COMMAND_RATE2	0x0002	/* Use 2x rate */
#నిర్వచించు  PCI_AGP_COMMAND_RATE1	0x0001	/* Use 1x rate */
#నిర్వచించు PCI_AGP_SIZEOF		12

/* Vital Product Data */

#నిర్వచించు PCI_VPD_ADDR		2	/* Address to access (15 bits!) */
#నిర్వచించు  PCI_VPD_ADDR_ముసుగు	0x7fff	/* Address mask */
#నిర్వచించు  PCI_VPD_ADDR_F		0x8000	/* Write 0, 1 indicates completion */
#నిర్వచించు PCI_VPD_దత్తాంశం		4	/* 32-bits of data returned here */

/* Slot Identification */

#నిర్వచించు PCI_SID_ESR		2	/* Expansion Slot Register */
#నిర్వచించు  PCI_SID_ESR_NSLOTS	0x1f	/* Number of expansion slots available */
#నిర్వచించు  PCI_SID_ESR_FIC	0x20	/* First In Chassis Flag */
#నిర్వచించు PCI_SID_CHASSIS_NR	3	/* Chassis Number */

/* Message Signalled Interrupts registers */

#నిర్వచించు PCI_MSI_FLAGS		2	/* Various flags */
#నిర్వచించు  PCI_MSI_FLAGS_64_బిట్టు	0x80	/* 64-bit addresses allowed */
#నిర్వచించు  PCI_MSI_FLAGS_QSIZE	0x70	/* Message queue size configured */
#నిర్వచించు  PCI_MSI_FLAGS_QMASK	0x0e	/* Maximum queue size available */
#నిర్వచించు  PCI_MSI_FLAGS_ENABLE	0x01	/* MSI feature enabled */
#నిర్వచించు  PCI_MSI_FLAGS_MASKBIT	0x100	/* 64-bit mask bits allowed */
#నిర్వచించు PCI_MSI_RFU		3	/* Rest of capability flags */
#నిర్వచించు PCI_MSI_చిరునామా_LO	4	/* Lower 32 bits */
#నిర్వచించు PCI_MSI_చిరునామా_HI	8	/* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#నిర్వచించు PCI_MSI_దత్తాంశం_32		8	/* 16 bits of data for 32-bit devices */
#నిర్వచించు PCI_MSI_దత్తాంశం_64		12	/* 16 bits of data for 64-bit devices */
#నిర్వచించు PCI_MSI_ముసుగు_బిట్టు	16	/* Mask bits register */

/* MSI-X registers (these are at offset PCI_MSIX_FLAGS) */
#నిర్వచించు PCI_MSIX_FLAGS		2
#నిర్వచించు  PCI_MSIX_FLAGS_QSIZE	0x7FF
#నిర్వచించు  PCI_MSIX_FLAGS_ENABLE	(1 << 15)
#నిర్వచించు  PCI_MSIX_FLAGS_MASKALL	(1 << 14)
#నిర్వచించు PCI_MSIX_FLAGS_BIRMASK	(7 << 0)
#నిర్వచించు PCI_MSIX_FLAGS_BITMASK	(1 << 0)

/* CompactPCI Hotswap Register */

#నిర్వచించు PCI_CHSWP_CSR		2	/* Control and Status Register */
#నిర్వచించు  PCI_CHSWP_DHA		0x01	/* Device Hiding Arm */
#నిర్వచించు  PCI_CHSWP_EIM		0x02	/* ENUM# Signal Mask */
#నిర్వచించు  PCI_CHSWP_PIE		0x04	/* Pending Insert or Extract */
#నిర్వచించు  PCI_CHSWP_LOO		0x08	/* LED On / Off */
#నిర్వచించు  PCI_CHSWP_PI		0x30	/* Programming Interface */
#నిర్వచించు  PCI_CHSWP_EXT		0x40	/* ENUM# status - extraction */
#నిర్వచించు  PCI_CHSWP_INS		0x80	/* ENUM# status - insertion */

/* PCI-X registers */

#నిర్వచించు PCI_X_CMD		2	/* Modes & Features */
#నిర్వచించు  PCI_X_CMD_DPERR_E	0x0001	/* Data Parity Error Recovery Enable */
#నిర్వచించు  PCI_X_CMD_ERO		0x0002	/* Enable Relaxed Ordering */
#నిర్వచించు  PCI_X_CMD_READ_512	0x0000	/* 512 byte maximum read byte count */
#నిర్వచించు  PCI_X_CMD_READ_1K	0x0004	/* 1Kbyte maximum read byte count */
#నిర్వచించు  PCI_X_CMD_READ_2K	0x0008	/* 2Kbyte maximum read byte count */
#నిర్వచించు  PCI_X_CMD_READ_4K	0x000c	/* 4Kbyte maximum read byte count */
#నిర్వచించు  PCI_X_CMD_MAX_READ	0x000c	/* Max Memory Read Byte Count */
				/* Max # of outstanding split transactions */
#నిర్వచించు  PCI_X_CMD_విభజించు_1	0x0000	/* Max 1 */
#నిర్వచించు  PCI_X_CMD_విభజించు_2	0x0010	/* Max 2 */
#నిర్వచించు  PCI_X_CMD_విభజించు_3	0x0020	/* Max 3 */
#నిర్వచించు  PCI_X_CMD_విభజించు_4	0x0030	/* Max 4 */
#నిర్వచించు  PCI_X_CMD_విభజించు_8	0x0040	/* Max 8 */
#నిర్వచించు  PCI_X_CMD_విభజించు_12	0x0050	/* Max 12 */
#నిర్వచించు  PCI_X_CMD_విభజించు_16	0x0060	/* Max 16 */
#నిర్వచించు  PCI_X_CMD_విభజించు_32	0x0070	/* Max 32 */
#నిర్వచించు  PCI_X_CMD_MAX_విభజించు	0x0070	/* Max Outstanding Split Transactions */
#నిర్వచించు  PCI_X_CMD_సంస్కరణ(x) 	(((x) >> 12) & 3) /* Version */
#నిర్వచించు PCI_X_స్థితి		4	/* PCI-X capabilities */
#నిర్వచించు  PCI_X_స్థితి_DEVFN	0x000000ff	/* A copy of devfn */
#నిర్వచించు  PCI_X_స్థితి_BUS	0x0000ff00	/* A copy of bus nr */
#నిర్వచించు  PCI_X_స్థితి_64_బిట్టు	0x00010000	/* 64-bit device */
#నిర్వచించు  PCI_X_స్థితి_133_MHZ	0x00020000	/* 133 MHz capable */
#నిర్వచించు  PCI_X_స్థితి_SPL_DISC	0x00040000	/* Split Completion Discarded */
#నిర్వచించు  PCI_X_స్థితి_UNX_SPL	0x00080000	/* Unexpected Split Completion */
#నిర్వచించు  PCI_X_స్థితి_COMPLEX	0x00100000	/* Device Complexity */
#నిర్వచించు  PCI_X_స్థితి_MAX_READ	0x00600000	/* Designed Max Memory Read Count */
#నిర్వచించు  PCI_X_స్థితి_MAX_విభజించు	0x03800000	/* Designed Max Outstanding Split Transactions */
#నిర్వచించు  PCI_X_స్థితి_MAX_CUM	0x1c000000	/* Designed Max Cumulative Read Size */
#నిర్వచించు  PCI_X_స్థితి_SPL_ERR	0x20000000	/* Rcvd Split Completion Error Msg */
#నిర్వచించు  PCI_X_స్థితి_266_MHZ	0x40000000	/* 266 MHz capable */
#నిర్వచించు  PCI_X_స్థితి_533_MHZ	0x80000000	/* 533 MHz capable */

/* PCI Express capability registers */

#నిర్వచించు PCI_EXP_FLAGS		2	/* Capabilities register */
#నిర్వచించు PCI_EXP_FLAGS_VERS	0x000f	/* Capability version */
#నిర్వచించు PCI_EXP_FLAGS_TYPE	0x00f0	/* Device/Port type */
#నిర్వచించు  PCI_EXP_TYPE_ENDPOINT	0x0	/* Express Endpoint */
#నిర్వచించు  PCI_EXP_TYPE_LEG_చివర	0x1	/* Legacy Endpoint */
#నిర్వచించు  PCI_EXP_TYPE_మూలం_పోర్టు 0x4	/* Root Port */
#నిర్వచించు  PCI_EXP_TYPE_UPSTREAM	0x5	/* Upstream Port */
#నిర్వచించు  PCI_EXP_TYPE_DOWNSTREAM 0x6	/* Downstream Port */
#నిర్వచించు  PCI_EXP_TYPE_PCI_BRIDGE 0x7	/* PCI/PCI-X Bridge */
#నిర్వచించు PCI_EXP_FLAGS_స్థానం	0x0100	/* Slot implemented */
#నిర్వచించు PCI_EXP_FLAGS_IRQ	0x3e00	/* Interrupt message number */
#నిర్వచించు PCI_EXP_DEVCAP		4	/* Device capabilities */
#నిర్వచించు  PCI_EXP_DEVCAP_PAYLOAD	0x07	/* Max_Payload_Size */
#నిర్వచించు  PCI_EXP_DEVCAP_PHANTOM	0x18	/* Phantom functions */
#నిర్వచించు  PCI_EXP_DEVCAP_EXT_ట్యాగ్	0x20	/* Extended tags */
#నిర్వచించు  PCI_EXP_DEVCAP_L0S	0x1c0	/* L0s Acceptable Latency */
#నిర్వచించు  PCI_EXP_DEVCAP_L1	0xe00	/* L1 Acceptable Latency */
#నిర్వచించు  PCI_EXP_DEVCAP_ATN_BUT	0x1000	/* Attention Button Present */
#నిర్వచించు  PCI_EXP_DEVCAP_ATN_IND	0x2000	/* Attention Indicator Present */
#నిర్వచించు  PCI_EXP_DEVCAP_PWR_IND	0x4000	/* Power Indicator Present */
#నిర్వచించు  PCI_EXP_DEVCAP_RBER	0x8000	/* Role-Based Error Reporting */
#నిర్వచించు  PCI_EXP_DEVCAP_PWR_VAL	0x3fc0000 /* Slot Power Limit Value */
#నిర్వచించు  PCI_EXP_DEVCAP_PWR_SCL	0xc000000 /* Slot Power Limit Scale */
#నిర్వచించు  PCI_EXP_DEVCAP_FLR     0x10000000 /* Function Level Reset */
#నిర్వచించు PCI_EXP_DEVCTL		8	/* Device Control */
#నిర్వచించు  PCI_EXP_DEVCTL_CERE	0x0001	/* Correctable Error Reporting En. */
#నిర్వచించు  PCI_EXP_DEVCTL_NFERE	0x0002	/* Non-Fatal Error Reporting Enable */
#నిర్వచించు  PCI_EXP_DEVCTL_FERE	0x0004	/* Fatal Error Reporting Enable */
#నిర్వచించు  PCI_EXP_DEVCTL_URRE	0x0008	/* Unsupported Request Reporting En. */
#నిర్వచించు  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#నిర్వచించు  PCI_EXP_DEVCTL_PAYLOAD	0x00e0	/* Max_Payload_Size */
#నిర్వచించు  PCI_EXP_DEVCTL_EXT_ట్యాగ్	0x0100	/* Extended Tag Field Enable */
#నిర్వచించు  PCI_EXP_DEVCTL_PHANTOM	0x0200	/* Phantom Functions Enable */
#నిర్వచించు  PCI_EXP_DEVCTL_AUX_PME	0x0400	/* Auxiliary Power PM Enable */
#నిర్వచించు  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#నిర్వచించు  PCI_EXP_DEVCTL_READRQ	0x7000	/* Max_Read_Request_Size */
#నిర్వచించు  PCI_EXP_DEVCTL_BCR_FLR 0x8000  /* Bridge Configuration Retry / FLR */
#నిర్వచించు PCI_EXP_DEVSTA		10	/* Device Status */
#నిర్వచించు  PCI_EXP_DEVSTA_CED	0x01	/* Correctable Error Detected */
#నిర్వచించు  PCI_EXP_DEVSTA_NFED	0x02	/* Non-Fatal Error Detected */
#నిర్వచించు  PCI_EXP_DEVSTA_FED	0x04	/* Fatal Error Detected */
#నిర్వచించు  PCI_EXP_DEVSTA_URD	0x08	/* Unsupported Request Detected */
#నిర్వచించు  PCI_EXP_DEVSTA_AUXPD	0x10	/* AUX Power Detected */
#నిర్వచించు  PCI_EXP_DEVSTA_TRPND	0x20	/* Transactions Pending */
#నిర్వచించు PCI_EXP_LNKCAP		12	/* Link Capabilities */
#నిర్వచించు  PCI_EXP_LNKCAP_ASPMS	0xc00	/* ASPM Support */
#నిర్వచించు  PCI_EXP_LNKCAP_L0SEL	0x7000	/* L0s Exit Latency */
#నిర్వచించు  PCI_EXP_LNKCAP_L1EL	0x38000	/* L1 Exit Latency */
#నిర్వచించు  PCI_EXP_LNKCAP_CLKPM	0x40000	/* L1 Clock Power Management */
#నిర్వచించు PCI_EXP_LNKCTL		16	/* Link Control */
#నిర్వచించు  PCI_EXP_LNKCTL_RL	0x20	/* Retrain Link */
#నిర్వచించు  PCI_EXP_LNKCTL_CCC	0x40	/* Common Clock COnfiguration */
#నిర్వచించు  PCI_EXP_LNKCTL_CLKREQ_EN 0x100	/* Enable clkreq */
#నిర్వచించు PCI_EXP_LNKSTA		18	/* Link Status */
#నిర్వచించు  PCI_EXP_LNKSTA_LT	0x800	/* Link Training */
#నిర్వచించు  PCI_EXP_LNKSTA_SLC	0x1000	/* Slot Clock Configuration */
#నిర్వచించు PCI_EXP_SLTCAP		20	/* Slot Capabilities */
#నిర్వచించు PCI_EXP_SLTCTL		24	/* Slot Control */
#నిర్వచించు PCI_EXP_SLTSTA		26	/* Slot Status */
#నిర్వచించు PCI_EXP_RTCTL		28	/* Root Control */
#నిర్వచించు  PCI_EXP_RTCTL_SECEE	0x01	/* System Error on Correctable Error */
#నిర్వచించు  PCI_EXP_RTCTL_SENFEE	0x02	/* System Error on Non-Fatal Error */
#నిర్వచించు  PCI_EXP_RTCTL_SEFEE	0x04	/* System Error on Fatal Error */
#నిర్వచించు  PCI_EXP_RTCTL_PMEIE	0x08	/* PME Interrupt Enable */
#నిర్వచించు  PCI_EXP_RTCTL_CRSSVE	0x10	/* CRS Software Visibility Enable */
#నిర్వచించు PCI_EXP_RTCAP		30	/* Root Capabilities */
#నిర్వచించు PCI_EXP_RTSTA		32	/* Root Status */
#నిర్వచించు PCI_EXP_DEVCAP2		36	/* Device Capabilities 2 */
#నిర్వచించు  PCI_EXP_DEVCAP2_ARI	0x20	/* Alternative Routing-ID */
#నిర్వచించు PCI_EXP_DEVCTL2		40	/* Device Control 2 */
#నిర్వచించు  PCI_EXP_DEVCTL2_ARI	0x20	/* Alternative Routing-ID */

/* Extended Capabilities (PCI-X 2.0 and Express) */
#నిర్వచించు PCI_EXT_CAP_ID(శీర్షిక)		(శీర్షిక & 0x0000ffff)
#నిర్వచించు PCI_EXT_CAP_VER(శీర్షిక)		((శీర్షిక >> 16) & 0xf)
#నిర్వచించు PCI_EXT_CAP_తదుపరి(శీర్షిక)	((శీర్షిక >> 20) & 0xffc)

#నిర్వచించు PCI_EXT_CAP_ID_ERR	1
#నిర్వచించు PCI_EXT_CAP_ID_VC	2
#నిర్వచించు PCI_EXT_CAP_ID_DSN	3
#నిర్వచించు PCI_EXT_CAP_ID_PWR	4
#నిర్వచించు PCI_EXT_CAP_ID_ARI	14

/* Advanced Error Reporting */
#నిర్వచించు PCI_ERR_UNCOR_స్థితి	4	/* Uncorrectable Error Status */
#నిర్వచించు  PCI_ERR_UNC_TRAIN	0x00000001	/* Training */
#నిర్వచించు  PCI_ERR_UNC_DLP	0x00000010	/* Data Link Protocol */
#నిర్వచించు  PCI_ERR_UNC_POISON_TLP	0x00001000	/* Poisoned TLP */
#నిర్వచించు  PCI_ERR_UNC_FCP	0x00002000	/* Flow Control Protocol */
#నిర్వచించు  PCI_ERR_UNC_COMP_సమయం	0x00004000	/* Completion Timeout */
#నిర్వచించు  PCI_ERR_UNC_COMP_ABORT	0x00008000	/* Completer Abort */
#నిర్వచించు  PCI_ERR_UNC_UNX_COMP	0x00010000	/* Unexpected Completion */
#నిర్వచించు  PCI_ERR_UNC_RX_OVER	0x00020000	/* Receiver Overflow */
#నిర్వచించు  PCI_ERR_UNC_MALF_TLP	0x00040000	/* Malformed TLP */
#నిర్వచించు  PCI_ERR_UNC_ECRC	0x00080000	/* ECRC Error Status */
#నిర్వచించు  PCI_ERR_UNC_UNSUP	0x00100000	/* Unsupported Request */
#నిర్వచించు PCI_ERR_UNCOR_ముసుగు	8	/* Uncorrectable Error Mask */
	/* Same bits as above */
#నిర్వచించు PCI_ERR_UNCOR_SEVER	12	/* Uncorrectable Error Severity */
	/* Same bits as above */
#నిర్వచించు PCI_ERR_COR_స్థితి	16	/* Correctable Error Status */
#నిర్వచించు  PCI_ERR_COR_RCVR	0x00000001	/* Receiver Error Status */
#నిర్వచించు  PCI_ERR_COR_BAD_TLP	0x00000040	/* Bad TLP Status */
#నిర్వచించు  PCI_ERR_COR_BAD_DLLP	0x00000080	/* Bad DLLP Status */
#నిర్వచించు  PCI_ERR_COR_REP_ROLL	0x00000100	/* REPLAY_NUM Rollover */
#నిర్వచించు  PCI_ERR_COR_REP_TIMER	0x00001000	/* Replay Timer Timeout */
#నిర్వచించు PCI_ERR_COR_ముసుగు	20	/* Correctable Error Mask */
	/* Same bits as above */
#నిర్వచించు PCI_ERR_CAP		24	/* Advanced Error Capabilities */
#నిర్వచించు  PCI_ERR_CAP_FEP(x)	((x) & 31)	/* First Error Pointer */
#నిర్వచించు  PCI_ERR_CAP_ECRC_GENC	0x00000020	/* ECRC Generation Capable */
#నిర్వచించు  PCI_ERR_CAP_ECRC_GENE	0x00000040	/* ECRC Generation Enable */
#నిర్వచించు  PCI_ERR_CAP_ECRC_CHKC	0x00000080	/* ECRC Check Capable */
#నిర్వచించు  PCI_ERR_CAP_ECRC_CHKE	0x00000100	/* ECRC Check Enable */
#నిర్వచించు PCI_ERR_శీర్షిక_LOG	28	/* Header Log Register (16 bytes) */
#నిర్వచించు PCI_ERR_మూలం_COMMAND	44	/* Root Error Command */
/* Correctable Err Reporting Enable */
#నిర్వచించు PCI_ERR_మూలం_CMD_COR_EN		0x00000001
/* Non-fatal Err Reporting Enable */
#నిర్వచించు PCI_ERR_మూలం_CMD_NONFATAL_EN	0x00000002
/* Fatal Err Reporting Enable */
#నిర్వచించు PCI_ERR_మూలం_CMD_FATAL_EN	0x00000004
#నిర్వచించు PCI_ERR_మూలం_స్థితి	48
#నిర్వచించు PCI_ERR_మూలం_COR_RCV		0x00000001	/* ERR_COR Received */
/* Multi ERR_COR Received */
#నిర్వచించు PCI_ERR_మూలం_MULTI_COR_RCV	0x00000002
/* ERR_FATAL/NONFATAL Recevied */
#నిర్వచించు PCI_ERR_మూలం_UNCOR_RCV		0x00000004
/* Multi ERR_FATAL/NONFATAL Recevied */
#నిర్వచించు PCI_ERR_మూలం_MULTI_UNCOR_RCV	0x00000008
#నిర్వచించు PCI_ERR_మూలం_మొదటి_FATAL	0x00000010	/* First Fatal */
#నిర్వచించు PCI_ERR_మూలం_NONFATAL_RCV	0x00000020	/* Non-Fatal Received */
#నిర్వచించు PCI_ERR_మూలం_FATAL_RCV		0x00000040	/* Fatal Received */
#నిర్వచించు PCI_ERR_మూలం_COR_SRC	52
#నిర్వచించు PCI_ERR_మూలం_SRC	54

/* Virtual Channel */
#నిర్వచించు PCI_VC_పోర్టు_REG1	4
#నిర్వచించు PCI_VC_పోర్టు_REG2	8
#నిర్వచించు PCI_VC_పోర్టు_CTRL	12
#నిర్వచించు PCI_VC_పోర్టు_స్థితి	14
#నిర్వచించు PCI_VC_RES_CAP		16
#నిర్వచించు PCI_VC_RES_CTRL		20
#నిర్వచించు PCI_VC_RES_స్థితి	26

/* Power Budgeting */
#నిర్వచించు PCI_PWR_DSR		4	/* Data Select Register */
#నిర్వచించు PCI_PWR_దత్తాంశం		8	/* Data Register */
#నిర్వచించు  PCI_PWR_దత్తాంశం_పునాది(x)	((x) & 0xff)	    /* Base Power */
#నిర్వచించు  PCI_PWR_దత్తాంశం_SCALE(x)	(((x) >> 8) & 3)    /* Data Scale */
#నిర్వచించు  PCI_PWR_దత్తాంశం_PM_SUB(x)	(((x) >> 10) & 7)   /* PM Sub State */
#నిర్వచించు  PCI_PWR_దత్తాంశం_PM_STATE(x) (((x) >> 13) & 3) /* PM State */
#నిర్వచించు  PCI_PWR_దత్తాంశం_TYPE(x)	(((x) >> 15) & 7)   /* Type */
#నిర్వచించు  PCI_PWR_దత్తాంశం_RAIL(x)	(((x) >> 18) & 7)   /* Power Rail */
#నిర్వచించు PCI_PWR_CAP		12	/* Capability */
#నిర్వచించు  PCI_PWR_CAP_BUDGET(x)	((x) & 1)	/* Included in system budget */

/*
 * Hypertransport sub capability types
 *
 * Unfortunately there are both 3 bit and 5 bit capability types defined
 * in the HT spec, catering for that is a little messy. You probably don't
 * want to use these directly, just use pci_find_ht_capability() and it
 * will do the right thing for you.
 */
#నిర్వచించు HT_3_బిట్టు_CAP_ముసుగు	0xE0
#నిర్వచించు HT_CAPTYPE_SLAVE	0x00	/* Slave/Primary link configuration */
#నిర్వచించు HT_CAPTYPE_HOST		0x20	/* Host/Secondary link configuration */

#నిర్వచించు HT_5_బిట్టు_CAP_ముసుగు	0xF8
#నిర్వచించు HT_CAPTYPE_IRQ		0x80	/* IRQ Configuration */
#నిర్వచించు HT_CAPTYPE_REMAPPING_40	0xA0	/* 40 bit address remapping */
#నిర్వచించు HT_CAPTYPE_REMAPPING_64 0xA2	/* 64 bit address remapping */
#నిర్వచించు HT_CAPTYPE_UNITID_CLUMP	0x90	/* Unit ID clumping */
#నిర్వచించు HT_CAPTYPE_EXTCONF	0x98	/* Extended Configuration Space Access */
#నిర్వచించు HT_CAPTYPE_MSI_MAPPING	0xA8	/* MSI Mapping Capability */
#నిర్వచించు  HT_MSI_FLAGS		0x02		/* Offset to flags */
#నిర్వచించు  HT_MSI_FLAGS_ENABLE	0x1		/* Mapping enable */
#నిర్వచించు  HT_MSI_FLAGS_FIXED	0x2		/* Fixed mapping only */
#నిర్వచించు  HT_MSI_FIXED_ADDR	0x00000000FEE00000ULL	/* Fixed addr */
#నిర్వచించు  HT_MSI_ADDR_LO		0x04		/* Offset to low addr bits */
#నిర్వచించు  HT_MSI_ADDR_LO_ముసుగు	0xFFF00000	/* Low address bit mask */
#నిర్వచించు  HT_MSI_ADDR_HI		0x08		/* Offset to high addr bits */
#నిర్వచించు HT_CAPTYPE_ప్రత్యక్ష_ROUTE	0xB0	/* Direct routing configuration */
#నిర్వచించు HT_CAPTYPE_VCSET	0xB8	/* Virtual Channel configuration */
#నిర్వచించు HT_CAPTYPE_ERROR_RETRY	0xC0	/* Retry on error configuration */
#నిర్వచించు HT_CAPTYPE_GEN3		0xD0	/* Generation 3 hypertransport configuration */
#నిర్వచించు HT_CAPTYPE_PM		0xE0	/* Hypertransport powermanagement configuration */

/* Alternative Routing-ID Interpretation */
#నిర్వచించు PCI_ARI_CAP		0x04	/* ARI Capability Register */
#నిర్వచించు  PCI_ARI_CAP_MFVC	0x0001	/* MFVC Function Groups Capability */
#నిర్వచించు  PCI_ARI_CAP_ACS	0x0002	/* ACS Function Groups Capability */
#నిర్వచించు  PCI_ARI_CAP_NFN(x)	(((x) >> 8) & 0xff) /* Next Function Number */
#నిర్వచించు PCI_ARI_CTRL		0x06	/* ARI Control Register */
#నిర్వచించు  PCI_ARI_CTRL_MFVC	0x0001	/* MFVC Function Groups Enable */
#నిర్వచించు  PCI_ARI_CTRL_ACS	0x0002	/* ACS Function Groups Enable */
#నిర్వచించు  PCI_ARI_CTRL_FG(x)	(((x) >> 4) & 7) /* Function Group */

#అయితే_ముగింపు /* LINUX_PCI_REGS_H */
