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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LINUX_PCI_REGS_H
#ವ್ಯಾಖ್ಯಾನಿಸು LINUX_PCI_REGS_H

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VENDOR_ID		0x00	/* 16 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_ID		0x02	/* 16 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_COMMAND		0x04	/* 16 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_IO		0x1	/* Enable response in I/O space */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_ಸ್ಮರಣೆ	0x2	/* Enable response in Memory space */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_MASTER	0x4	/* Enable bus mastering */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_SPECIAL	0x8	/* Enable response to special cycles */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_INVALIDATE	0x10	/* Use memory write and invalidate */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_WAIT 	0x80	/* Enable address/data stepping */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_FAST_ಹಿಂಭಾಗ	0x200	/* Enable back-to-back writes */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಸ್ಥಿತಿ		0x06	/* 16 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_CAP_LIST	0x10	/* Support Capability List */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_66_MHZ	0x20	/* Support 66 Mhz PCI 2.1 bus */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_UDF		0x40	/* Support User Definable Features [obsolete] */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_FAST_ಹಿಂಭಾಗ	0x80	/* Accept fast-back to back */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_PARITY	0x100	/* Detected parity error */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_DEVSEL_ಮುಖವಾಡ	0x600	/* DEVSEL timing */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_DEVSEL_FAST		0x000
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_DEVSEL_MEDIUM	0x200
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_DEVSEL_SLOW		0x400
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_SIG_ಗುರಿ_ABORT	0x800 /* Set on target abort */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_REC_ಗುರಿ_ABORT	0x1000 /* Master ack of " */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_REC_MASTER_ABORT	0x2000 /* Set on master abort */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_SIG_SYSTEM_ERROR	0x4000 /* Set when we drive SERR */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಥಿತಿ_DETECTED_PARITY	0x8000 /* Set on parity error */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8 revision */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_REVISION_ID		0x08	/* Revision ID */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CLASS_DEVICE	0x0a	/* Device class */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CACHE_ಸಾಲು_ಗಾತ್ರ	0x0c	/* 8 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಶೀರ್ಷಿಕೆ_TYPE		0x0e	/* 8 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಶೀರ್ಷಿಕೆ_TYPE_ಸಾಮಾನ್ಯ		0
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಶೀರ್ಷಿಕೆ_TYPE_BRIDGE		1
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಶೀರ್ಷಿಕೆ_TYPE_CARDBUS	2

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_BIST		0x0f	/* 8 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BIST_ಸಂಕೇತ_ಮುಖವಾಡ	0x0f	/* Return result */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BIST_ಆರಂಭ		0x40	/* 1 to start BIST, 2 secs or less */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ತಳ_ವಿಳಾಸ_0	0x10	/* 32 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ತಳ_ವಿಳಾಸ_1	0x14	/* 32 bits [htype 0,1 only] */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ತಳ_ವಿಳಾಸ_2	0x18	/* 32 bits [htype 0 only] */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ತಳ_ವಿಳಾಸ_3	0x1c	/* 32 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ತಳ_ವಿಳಾಸ_4	0x20	/* 32 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ತಳ_ವಿಳಾಸ_5	0x24	/* 32 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_SPACE		0x01	/* 0 = memory, 1 = I/O */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_SPACE_IO	0x01
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_SPACE_ಸ್ಮರಣೆ	0x00
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_ಮುಖವಾಡ	0x06
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_32	0x00	/* 32 bit address */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_1_M	0x02	/* Below 1M [obsolete] */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_64	0x04	/* 64 bit address */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_MEM_PREFETCH	0x08	/* prefetchable? */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_MEM_ಮುಖವಾಡ	(~0x0fUL)
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ತಳ_ವಿಳಾಸ_IO_ಮುಖವಾಡ	(~0x03UL)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CARDBUS_CIS		0x28
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_SUBSYSTEM_VENDOR_ID	0x2c
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_SUBSYSTEM_ID	0x2e
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ROM_ವಿಳಾಸ		0x30	/* Bits 31..11 are address, 10..1 reserved */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ROM_ವಿಳಾಸ_ENABLE	0x01
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ROM_ವಿಳಾಸ_ಮುಖವಾಡ	(~0x7ffUL)

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CAPABILITY_LIST	0x34	/* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಅಡಚಣೆ_ಸಾಲು	0x3c	/* 8 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಅಡಚಣೆ_PIN	0x3d	/* 8 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MIN_GNT		0x3e	/* 8 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MAX_LAT		0x3f	/* 8 bits */

/* Header type 1 (PCI-to-PCI bridges) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PRIMARY_BUS		0x18	/* Primary bus number */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_SECONDARY_BUS	0x19	/* Secondary bus number */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_SUBORDINATE_BUS	0x1a	/* Highest bus number behind the bridge */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_SEC_LATENCY_TIMER	0x1b	/* Latency timer for secondary interface */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_IO_ತಳ		0x1c	/* I/O range behind the bridge */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_IO_ಎಲ್ಲೆ		0x1d
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_IO_RANGE_TYPE_ಮುಖವಾಡ	0x0fUL	/* I/O bridging type */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_IO_RANGE_TYPE_16	0x00
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_IO_RANGE_TYPE_32	0x01
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_IO_RANGE_ಮುಖವಾಡ	(~0x0fUL)
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_SEC_ಸ್ಥಿತಿ		0x1e	/* Secondary status register, only bit 14 used */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಸ್ಮರಣೆ_ತಳ		0x20	/* Memory range behind */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ಸ್ಮರಣೆ_ಎಲ್ಲೆ	0x22
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಮರಣೆ_RANGE_TYPE_ಮುಖವಾಡ 0x0fUL
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ಸ್ಮರಣೆ_RANGE_ಮುಖವಾಡ	(~0x0fUL)
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PREF_ಸ್ಮರಣೆ_ತಳ	0x24	/* Prefetchable memory range behind */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PREF_ಸ್ಮರಣೆ_ಎಲ್ಲೆ	0x26
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PREF_RANGE_TYPE_ಮುಖವಾಡ 0x0fUL
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PREF_RANGE_TYPE_32	0x00
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PREF_RANGE_TYPE_64	0x01
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PREF_RANGE_ಮುಖವಾಡ	(~0x0fUL)
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PREF_ತಳ_UPPER32	0x28	/* Upper half of prefetchable memory range */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PREF_ಎಲ್ಲೆ_UPPER32	0x2c
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_IO_ತಳ_UPPER16	0x30	/* Upper half of I/O addresses */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_IO_ಎಲ್ಲೆ_UPPER16	0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ROM_ADDRESS1	0x38	/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_BRIDGE_CONTROL	0x3e
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BRIDGE_CTL_PARITY	0x01	/* Enable parity detection on secondary interface */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BRIDGE_CTL_SERR	0x02	/* The same for SERR forwarding */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BRIDGE_CTL_ISA	0x04	/* Enable ISA mode */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BRIDGE_CTL_VGA	0x08	/* Forward VGA addresses */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BRIDGE_CTL_MASTER_ABORT	0x20  /* Report master aborts */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BRIDGE_CTL_BUS_RESET	0x40	/* Secondary bus reset */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_BRIDGE_CTL_FAST_ಹಿಂಭಾಗ	0x80	/* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_CAPABILITY_LIST	0x14
/* 0x15 reserved */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_SEC_ಸ್ಥಿತಿ	0x16	/* Secondary status */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_PRIMARY_BUS	0x18	/* PCI bus number */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_CARD_BUS		0x19	/* CardBus bus number */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_SUBORDINATE_BUS	0x1a	/* Subordinate bus number */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_LATENCY_TIMER	0x1b	/* CardBus latency timer */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_ಸ್ಮರಣೆ_ತಳ_0	0x1c
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_ಸ್ಮರಣೆ_ಎಲ್ಲೆ_0	0x20
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_ಸ್ಮರಣೆ_ತಳ_1	0x24
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_ಸ್ಮರಣೆ_ಎಲ್ಲೆ_1	0x28
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_IO_ತಳ_0	0x2c
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_IO_ತಳ_0_HI	0x2e
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_IO_ಎಲ್ಲೆ_0	0x30
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_IO_ಎಲ್ಲೆ_0_HI	0x32
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_IO_ತಳ_1	0x34
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_IO_ತಳ_1_HI	0x36
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_IO_ಎಲ್ಲೆ_1	0x38
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_IO_ಎಲ್ಲೆ_1_HI	0x3a
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_IO_RANGE_ಮುಖವಾಡ	(~0x03UL)
/* 0x3c-0x3d are same as for htype 0 */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_BRIDGE_CONTROL	0x3e
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_PARITY	0x01	/* Similar to standard bridge control register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_SERR		0x02
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_ISA		0x04
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_VGA		0x08
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_MASTER_ABORT	0x20
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_CB_RESET	0x40	/* CardBus reset */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_16_ಬಿಟ್_INT	0x80	/* Enable interrupt for 16-bit cards */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 0x100	/* Prefetch enable for both memory regions */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 0x200
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CB_BRIDGE_CTL_POST_WRITES	0x400
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_SUBSYSTEM_VENDOR_ID	0x40
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_SUBSYSTEM_ID		0x42
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CB_LEGACY_ಬಹುಲಕ_ತಳ		0x44	/* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability lists */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CAP_LIST_ID		0	/* Capability ID */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_PM		0x01	/* Power Management */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_PCIX	0x07	/* PCI-X */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_HT		0x08	/* HyperTransport */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_VNDR	0x09	/* Vendor specific */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_DBG		0x0A	/* Debug port */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_CCRC	0x0B	/* CompactPCI Central Resource Control */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_SHPC 	0x0C	/* PCI Standard Hot-Plug Controller */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_SSVID	0x0D	/* Bridge subsystem vendor/device ID */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_AGP3	0x0E	/* AGP Target PCI-PCI bridge */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_EXP 	0x10	/* PCI Express */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CAP_ID_MSIX	0x11	/* MSI-X */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CAP_LIST_ಮುಂದಿನ	1	/* Next capability in the list */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CAP_FLAGS		2	/* Capability defined flags (16 bits) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CAP_SIZEOF		4

/* Power Management Registers */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PM_PMC		2	/* PM Capabilities Register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_VER_ಮುಖವಾಡ	0x0007	/* Version */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME_CLOCK	0x0008	/* PME clock required */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_RESERVED    0x0010  /* Reserved field */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_DSI		0x0020	/* Device specific initialization */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_AUX_POWER	0x01C0	/* Auxilliary power support mask */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_D1		0x0200	/* D1 power state support */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_D2		0x0400	/* D2 power state support */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME		0x0800	/* PME pin supported */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME_ಮುಖವಾಡ	0xF800	/* PME Mask of all supported states */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME_D0	0x0800	/* PME# from D0 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME_D1	0x1000	/* PME# from D1 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME_D2	0x2000	/* PME# from D2 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME_D3	0x4000	/* PME# from D3 (hot) */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME_D3cold	0x8000	/* PME# from D3 (cold) */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CAP_PME_ಸರಿಸು	11	/* Start of the PME Mask in PMC */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PM_CTRL		4	/* PM control and status register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CTRL_STATE_ಮುಖವಾಡ	0x0003	/* Current power state (D0 to D3) */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CTRL_NO_SOFT_RESET	0x0004	/* No reset for D3hot->D0 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CTRL_PME_ENABLE	0x0100	/* PME pin enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CTRL_ದತ್ತಾಂಶ_SEL_ಮುಖವಾಡ	0x1e00	/* Data select (??) */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CTRL_ದತ್ತಾಂಶ_SCALE_ಮುಖವಾಡ	0x6000	/* Data scale (??) */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_CTRL_PME_ಸ್ಥಿತಿ	0x8000	/* PME pin status */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PM_PPB_EXTENSIONS	6	/* PPB support extensions (??) */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_PPB_B2_B3	0x40	/* Stop clock when in D3hot (??) */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PM_BPCC_ENABLE	0x80	/* Bus power/clock control enable (??) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PM_ದತ್ತಾಂಶ_ರಿಜಿಸ್ಟರ್	7	/* (??) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PM_SIZEOF		8

/* AGP registers */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_AGP_ಆವೃತ್ತಿ		2	/* BCD version number */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_AGP_RFU		3	/* Rest of capability flags */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_AGP_ಸ್ಥಿತಿ		4	/* Status register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_ಸ್ಥಿತಿ_RQ_ಮುಖವಾಡ	0xff000000	/* Maximum number of requests - 1 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_ಸ್ಥಿತಿ_SBA	0x0200	/* Sideband addressing supported */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_ಸ್ಥಿತಿ_64_ಬಿಟ್	0x0020	/* 64-bit addressing supported */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_ಸ್ಥಿತಿ_FW	0x0010	/* FW transfers supported */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_ಸ್ಥಿತಿ_RATE4	0x0004	/* 4x transfer rate supported */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_ಸ್ಥಿತಿ_RATE2	0x0002	/* 2x transfer rate supported */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_ಸ್ಥಿತಿ_RATE1	0x0001	/* 1x transfer rate supported */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_AGP_COMMAND		8	/* Control register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_COMMAND_RQ_ಮುಖವಾಡ 0xff000000  /* Master: Maximum number of requests */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_COMMAND_SBA	0x0200	/* Sideband addressing enabled */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_COMMAND_AGP	0x0100	/* Allow processing of AGP transactions */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_COMMAND_64_ಬಿಟ್	0x0020 	/* Allow processing of 64-bit addresses */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_COMMAND_FW	0x0010 	/* Force FW transfers */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_COMMAND_RATE4	0x0004	/* Use 4x rate */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_COMMAND_RATE2	0x0002	/* Use 2x rate */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_AGP_COMMAND_RATE1	0x0001	/* Use 1x rate */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_AGP_SIZEOF		12

/* Vital Product Data */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VPD_ADDR		2	/* Address to access (15 bits!) */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_VPD_ADDR_ಮುಖವಾಡ	0x7fff	/* Address mask */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_VPD_ADDR_F		0x8000	/* Write 0, 1 indicates completion */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VPD_ದತ್ತಾಂಶ		4	/* 32-bits of data returned here */

/* Slot Identification */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_SID_ESR		2	/* Expansion Slot Register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_SID_ESR_NSLOTS	0x1f	/* Number of expansion slots available */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_SID_ESR_FIC	0x20	/* First In Chassis Flag */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_SID_CHASSIS_NR	3	/* Chassis Number */

/* Message Signalled Interrupts registers */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSI_FLAGS		2	/* Various flags */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_MSI_FLAGS_64_ಬಿಟ್	0x80	/* 64-bit addresses allowed */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_MSI_FLAGS_QSIZE	0x70	/* Message queue size configured */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_MSI_FLAGS_QMASK	0x0e	/* Maximum queue size available */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_MSI_FLAGS_ENABLE	0x01	/* MSI feature enabled */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_MSI_FLAGS_MASKBIT	0x100	/* 64-bit mask bits allowed */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSI_RFU		3	/* Rest of capability flags */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSI_ವಿಳಾಸ_LO	4	/* Lower 32 bits */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSI_ವಿಳಾಸ_HI	8	/* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSI_ದತ್ತಾಂಶ_32		8	/* 16 bits of data for 32-bit devices */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSI_ದತ್ತಾಂಶ_64		12	/* 16 bits of data for 64-bit devices */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSI_ಮುಖವಾಡ_ಬಿಟ್	16	/* Mask bits register */

/* MSI-X registers (these are at offset PCI_MSIX_FLAGS) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSIX_FLAGS		2
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_MSIX_FLAGS_QSIZE	0x7FF
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_MSIX_FLAGS_ENABLE	(1 << 15)
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_MSIX_FLAGS_MASKALL	(1 << 14)
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSIX_FLAGS_BIRMASK	(7 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_MSIX_FLAGS_BITMASK	(1 << 0)

/* CompactPCI Hotswap Register */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CHSWP_CSR		2	/* Control and Status Register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CHSWP_DHA		0x01	/* Device Hiding Arm */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CHSWP_EIM		0x02	/* ENUM# Signal Mask */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CHSWP_PIE		0x04	/* Pending Insert or Extract */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CHSWP_LOO		0x08	/* LED On / Off */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CHSWP_PI		0x30	/* Programming Interface */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CHSWP_EXT		0x40	/* ENUM# status - extraction */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_CHSWP_INS		0x80	/* ENUM# status - insertion */

/* PCI-X registers */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_X_CMD		2	/* Modes & Features */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_DPERR_E	0x0001	/* Data Parity Error Recovery Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ERO		0x0002	/* Enable Relaxed Ordering */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_READ_512	0x0000	/* 512 byte maximum read byte count */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_READ_1K	0x0004	/* 1Kbyte maximum read byte count */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_READ_2K	0x0008	/* 2Kbyte maximum read byte count */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_READ_4K	0x000c	/* 4Kbyte maximum read byte count */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_MAX_READ	0x000c	/* Max Memory Read Byte Count */
				/* Max # of outstanding split transactions */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ವಿಭಜಿಸು_1	0x0000	/* Max 1 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ವಿಭಜಿಸು_2	0x0010	/* Max 2 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ವಿಭಜಿಸು_3	0x0020	/* Max 3 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ವಿಭಜಿಸು_4	0x0030	/* Max 4 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ವಿಭಜಿಸು_8	0x0040	/* Max 8 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ವಿಭಜಿಸು_12	0x0050	/* Max 12 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ವಿಭಜಿಸು_16	0x0060	/* Max 16 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ವಿಭಜಿಸು_32	0x0070	/* Max 32 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_MAX_ವಿಭಜಿಸು	0x0070	/* Max Outstanding Split Transactions */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_CMD_ಆವೃತ್ತಿ(x) 	(((x) >> 12) & 3) /* Version */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_X_ಸ್ಥಿತಿ		4	/* PCI-X capabilities */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_DEVFN	0x000000ff	/* A copy of devfn */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_BUS	0x0000ff00	/* A copy of bus nr */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_64_ಬಿಟ್	0x00010000	/* 64-bit device */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_133_MHZ	0x00020000	/* 133 MHz capable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_SPL_DISC	0x00040000	/* Split Completion Discarded */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_UNX_SPL	0x00080000	/* Unexpected Split Completion */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_COMPLEX	0x00100000	/* Device Complexity */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_MAX_READ	0x00600000	/* Designed Max Memory Read Count */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_MAX_ವಿಭಜಿಸು	0x03800000	/* Designed Max Outstanding Split Transactions */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_MAX_CUM	0x1c000000	/* Designed Max Cumulative Read Size */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_SPL_ERR	0x20000000	/* Rcvd Split Completion Error Msg */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_266_MHZ	0x40000000	/* 266 MHz capable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_X_ಸ್ಥಿತಿ_533_MHZ	0x80000000	/* 533 MHz capable */

/* PCI Express capability registers */

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_FLAGS		2	/* Capabilities register */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_FLAGS_VERS	0x000f	/* Capability version */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_FLAGS_TYPE	0x00f0	/* Device/Port type */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_TYPE_ENDPOINT	0x0	/* Express Endpoint */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_TYPE_LEG_ಕೊನೆ	0x1	/* Legacy Endpoint */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_TYPE_ಬೇರು_ಪೋರ್ಟ್ 0x4	/* Root Port */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_TYPE_UPSTREAM	0x5	/* Upstream Port */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_TYPE_DOWNSTREAM 0x6	/* Downstream Port */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_TYPE_PCI_BRIDGE 0x7	/* PCI/PCI-X Bridge */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_FLAGS_ಸ್ಥಾನ	0x0100	/* Slot implemented */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_FLAGS_IRQ	0x3e00	/* Interrupt message number */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_DEVCAP		4	/* Device capabilities */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_PAYLOAD	0x07	/* Max_Payload_Size */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_PHANTOM	0x18	/* Phantom functions */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_EXT_ಟ್ಯಾಗ್	0x20	/* Extended tags */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_L0S	0x1c0	/* L0s Acceptable Latency */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_L1	0xe00	/* L1 Acceptable Latency */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_ATN_BUT	0x1000	/* Attention Button Present */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_ATN_IND	0x2000	/* Attention Indicator Present */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_PWR_IND	0x4000	/* Power Indicator Present */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_RBER	0x8000	/* Role-Based Error Reporting */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_PWR_VAL	0x3fc0000 /* Slot Power Limit Value */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_PWR_SCL	0xc000000 /* Slot Power Limit Scale */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP_FLR     0x10000000 /* Function Level Reset */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_DEVCTL		8	/* Device Control */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_CERE	0x0001	/* Correctable Error Reporting En. */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_NFERE	0x0002	/* Non-Fatal Error Reporting Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_FERE	0x0004	/* Fatal Error Reporting Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_URRE	0x0008	/* Unsupported Request Reporting En. */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_PAYLOAD	0x00e0	/* Max_Payload_Size */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_EXT_ಟ್ಯಾಗ್	0x0100	/* Extended Tag Field Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_PHANTOM	0x0200	/* Phantom Functions Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_AUX_PME	0x0400	/* Auxiliary Power PM Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_READRQ	0x7000	/* Max_Read_Request_Size */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL_BCR_FLR 0x8000  /* Bridge Configuration Retry / FLR */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_DEVSTA		10	/* Device Status */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVSTA_CED	0x01	/* Correctable Error Detected */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVSTA_NFED	0x02	/* Non-Fatal Error Detected */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVSTA_FED	0x04	/* Fatal Error Detected */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVSTA_URD	0x08	/* Unsupported Request Detected */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVSTA_AUXPD	0x10	/* AUX Power Detected */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVSTA_TRPND	0x20	/* Transactions Pending */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_LNKCAP		12	/* Link Capabilities */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKCAP_ASPMS	0xc00	/* ASPM Support */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKCAP_L0SEL	0x7000	/* L0s Exit Latency */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKCAP_L1EL	0x38000	/* L1 Exit Latency */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKCAP_CLKPM	0x40000	/* L1 Clock Power Management */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_LNKCTL		16	/* Link Control */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKCTL_RL	0x20	/* Retrain Link */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKCTL_CCC	0x40	/* Common Clock COnfiguration */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKCTL_CLKREQ_EN 0x100	/* Enable clkreq */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_LNKSTA		18	/* Link Status */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKSTA_LT	0x800	/* Link Training */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_LNKSTA_SLC	0x1000	/* Slot Clock Configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_SLTCAP		20	/* Slot Capabilities */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_SLTCTL		24	/* Slot Control */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_SLTSTA		26	/* Slot Status */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_RTCTL		28	/* Root Control */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_RTCTL_SECEE	0x01	/* System Error on Correctable Error */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_RTCTL_SENFEE	0x02	/* System Error on Non-Fatal Error */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_RTCTL_SEFEE	0x04	/* System Error on Fatal Error */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_RTCTL_PMEIE	0x08	/* PME Interrupt Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_RTCTL_CRSSVE	0x10	/* CRS Software Visibility Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_RTCAP		30	/* Root Capabilities */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_RTSTA		32	/* Root Status */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_DEVCAP2		36	/* Device Capabilities 2 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCAP2_ARI	0x20	/* Alternative Routing-ID */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXP_DEVCTL2		40	/* Device Control 2 */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_EXP_DEVCTL2_ARI	0x20	/* Alternative Routing-ID */

/* Extended Capabilities (PCI-X 2.0 and Express) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXT_CAP_ID(ಶೀರ್ಷಿಕೆ)		(ಶೀರ್ಷಿಕೆ & 0x0000ffff)
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXT_CAP_VER(ಶೀರ್ಷಿಕೆ)		((ಶೀರ್ಷಿಕೆ >> 16) & 0xf)
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXT_CAP_ಮುಂದಿನ(ಶೀರ್ಷಿಕೆ)	((ಶೀರ್ಷಿಕೆ >> 20) & 0xffc)

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXT_CAP_ID_ERR	1
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXT_CAP_ID_VC	2
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXT_CAP_ID_DSN	3
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXT_CAP_ID_PWR	4
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_EXT_CAP_ID_ARI	14

/* Advanced Error Reporting */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_UNCOR_ಸ್ಥಿತಿ	4	/* Uncorrectable Error Status */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_TRAIN	0x00000001	/* Training */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_DLP	0x00000010	/* Data Link Protocol */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_POISON_TLP	0x00001000	/* Poisoned TLP */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_FCP	0x00002000	/* Flow Control Protocol */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_COMP_ಸಮಯ	0x00004000	/* Completion Timeout */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_COMP_ABORT	0x00008000	/* Completer Abort */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_UNX_COMP	0x00010000	/* Unexpected Completion */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_RX_OVER	0x00020000	/* Receiver Overflow */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_MALF_TLP	0x00040000	/* Malformed TLP */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_ECRC	0x00080000	/* ECRC Error Status */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_UNC_UNSUP	0x00100000	/* Unsupported Request */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_UNCOR_ಮುಖವಾಡ	8	/* Uncorrectable Error Mask */
	/* Same bits as above */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_UNCOR_SEVER	12	/* Uncorrectable Error Severity */
	/* Same bits as above */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_COR_ಸ್ಥಿತಿ	16	/* Correctable Error Status */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_COR_RCVR	0x00000001	/* Receiver Error Status */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_COR_BAD_TLP	0x00000040	/* Bad TLP Status */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_COR_BAD_DLLP	0x00000080	/* Bad DLLP Status */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_COR_REP_ROLL	0x00000100	/* REPLAY_NUM Rollover */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_COR_REP_TIMER	0x00001000	/* Replay Timer Timeout */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_COR_ಮುಖವಾಡ	20	/* Correctable Error Mask */
	/* Same bits as above */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_CAP		24	/* Advanced Error Capabilities */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_CAP_FEP(x)	((x) & 31)	/* First Error Pointer */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_CAP_ECRC_GENC	0x00000020	/* ECRC Generation Capable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_CAP_ECRC_GENE	0x00000040	/* ECRC Generation Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_CAP_ECRC_CHKC	0x00000080	/* ECRC Check Capable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ERR_CAP_ECRC_CHKE	0x00000100	/* ECRC Check Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಶೀರ್ಷಿಕೆ_LOG	28	/* Header Log Register (16 bytes) */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_COMMAND	44	/* Root Error Command */
/* Correctable Err Reporting Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_CMD_COR_EN		0x00000001
/* Non-fatal Err Reporting Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_CMD_NONFATAL_EN	0x00000002
/* Fatal Err Reporting Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_CMD_FATAL_EN	0x00000004
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_ಸ್ಥಿತಿ	48
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_COR_RCV		0x00000001	/* ERR_COR Received */
/* Multi ERR_COR Received */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_MULTI_COR_RCV	0x00000002
/* ERR_FATAL/NONFATAL Recevied */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_UNCOR_RCV		0x00000004
/* Multi ERR_FATAL/NONFATAL Recevied */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_MULTI_UNCOR_RCV	0x00000008
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_ಮೊದಲ_FATAL	0x00000010	/* First Fatal */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_NONFATAL_RCV	0x00000020	/* Non-Fatal Received */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_FATAL_RCV		0x00000040	/* Fatal Received */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_COR_SRC	52
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ERR_ಬೇರು_SRC	54

/* Virtual Channel */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VC_ಪೋರ್ಟ್_REG1	4
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VC_ಪೋರ್ಟ್_REG2	8
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VC_ಪೋರ್ಟ್_CTRL	12
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VC_ಪೋರ್ಟ್_ಸ್ಥಿತಿ	14
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VC_RES_CAP		16
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VC_RES_CTRL		20
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_VC_RES_ಸ್ಥಿತಿ	26

/* Power Budgeting */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PWR_DSR		4	/* Data Select Register */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PWR_ದತ್ತಾಂಶ		8	/* Data Register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PWR_ದತ್ತಾಂಶ_ತಳ(x)	((x) & 0xff)	    /* Base Power */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PWR_ದತ್ತಾಂಶ_SCALE(x)	(((x) >> 8) & 3)    /* Data Scale */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PWR_ದತ್ತಾಂಶ_PM_SUB(x)	(((x) >> 10) & 7)   /* PM Sub State */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PWR_ದತ್ತಾಂಶ_PM_STATE(x) (((x) >> 13) & 3) /* PM State */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PWR_ದತ್ತಾಂಶ_TYPE(x)	(((x) >> 15) & 7)   /* Type */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PWR_ದತ್ತಾಂಶ_RAIL(x)	(((x) >> 18) & 7)   /* Power Rail */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_PWR_CAP		12	/* Capability */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_PWR_CAP_BUDGET(x)	((x) & 1)	/* Included in system budget */

/*
 * Hypertransport sub capability types
 *
 * Unfortunately there are both 3 bit and 5 bit capability types defined
 * in the HT spec, catering for that is a little messy. You probably don't
 * want to use these directly, just use pci_find_ht_capability() and it
 * will do the right thing for you.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_3_ಬಿಟ್_CAP_ಮುಖವಾಡ	0xE0
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_SLAVE	0x00	/* Slave/Primary link configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_HOST		0x20	/* Host/Secondary link configuration */

#ವ್ಯಾಖ್ಯಾನಿಸು HT_5_ಬಿಟ್_CAP_ಮುಖವಾಡ	0xF8
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_IRQ		0x80	/* IRQ Configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_REMAPPING_40	0xA0	/* 40 bit address remapping */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_REMAPPING_64 0xA2	/* 64 bit address remapping */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_UNITID_CLUMP	0x90	/* Unit ID clumping */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_EXTCONF	0x98	/* Extended Configuration Space Access */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_MSI_MAPPING	0xA8	/* MSI Mapping Capability */
#ವ್ಯಾಖ್ಯಾನಿಸು  HT_MSI_FLAGS		0x02		/* Offset to flags */
#ವ್ಯಾಖ್ಯಾನಿಸು  HT_MSI_FLAGS_ENABLE	0x1		/* Mapping enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  HT_MSI_FLAGS_FIXED	0x2		/* Fixed mapping only */
#ವ್ಯಾಖ್ಯಾನಿಸು  HT_MSI_FIXED_ADDR	0x00000000FEE00000ULL	/* Fixed addr */
#ವ್ಯಾಖ್ಯಾನಿಸು  HT_MSI_ADDR_LO		0x04		/* Offset to low addr bits */
#ವ್ಯಾಖ್ಯಾನಿಸು  HT_MSI_ADDR_LO_ಮುಖವಾಡ	0xFFF00000	/* Low address bit mask */
#ವ್ಯಾಖ್ಯಾನಿಸು  HT_MSI_ADDR_HI		0x08		/* Offset to high addr bits */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_ನೇರ_ROUTE	0xB0	/* Direct routing configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_VCSET	0xB8	/* Virtual Channel configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_ERROR_RETRY	0xC0	/* Retry on error configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_GEN3		0xD0	/* Generation 3 hypertransport configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು HT_CAPTYPE_PM		0xE0	/* Hypertransport powermanagement configuration */

/* Alternative Routing-ID Interpretation */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ARI_CAP		0x04	/* ARI Capability Register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ARI_CAP_MFVC	0x0001	/* MFVC Function Groups Capability */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ARI_CAP_ACS	0x0002	/* ACS Function Groups Capability */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ARI_CAP_NFN(x)	(((x) >> 8) & 0xff) /* Next Function Number */
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ARI_CTRL		0x06	/* ARI Control Register */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ARI_CTRL_MFVC	0x0001	/* MFVC Function Groups Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ARI_CTRL_ACS	0x0002	/* ACS Function Groups Enable */
#ವ್ಯಾಖ್ಯಾನಿಸು  PCI_ARI_CTRL_FG(x)	(((x) >> 4) & 7) /* Function Group */

#ಆದರೆ_ಅಂತ್ಯ /* LINUX_PCI_REGS_H */
