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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LINUX_PCI_REGS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LINUX_PCI_REGS_H

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VENDOR_ID		0x00	/* 16 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_ID		0x02	/* 16 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_COMMAND		0x04	/* 16 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_IO		0x1	/* Enable response in I/O space */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_ਸਮ੍ਰਿਤੀ	0x2	/* Enable response in Memory space */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_MASTER	0x4	/* Enable bus mastering */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_SPECIAL	0x8	/* Enable response to special cycles */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_INVALIDATE	0x10	/* Use memory write and invalidate */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_WAIT 	0x80	/* Enable address/data stepping */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_FAST_ਪਿੱਠ	0x200	/* Enable back-to-back writes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਸਥਿਤੀ		0x06	/* 16 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_CAP_LIST	0x10	/* Support Capability List */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_66_MHZ	0x20	/* Support 66 Mhz PCI 2.1 bus */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_UDF		0x40	/* Support User Definable Features [obsolete] */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_FAST_ਪਿੱਠ	0x80	/* Accept fast-back to back */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_PARITY	0x100	/* Detected parity error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_DEVSEL_ਨਕਾਬ	0x600	/* DEVSEL timing */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_DEVSEL_FAST		0x000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_DEVSEL_MEDIUM	0x200
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_DEVSEL_SLOW		0x400
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_SIG_ਨਿਸ਼ਾਨਾ_ABORT	0x800 /* Set on target abort */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_REC_ਨਿਸ਼ਾਨਾ_ABORT	0x1000 /* Master ack of " */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_REC_MASTER_ABORT	0x2000 /* Set on master abort */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_SIG_SYSTEM_ERROR	0x4000 /* Set when we drive SERR */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਥਿਤੀ_DETECTED_PARITY	0x8000 /* Set on parity error */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8 revision */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_REVISION_ID		0x08	/* Revision ID */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CLASS_DEVICE	0x0a	/* Device class */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CACHE_ਸਤਰ_ਮਾਪ	0x0c	/* 8 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਸਿਰਲੇਖ_TYPE		0x0e	/* 8 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਿਰਲੇਖ_TYPE_ਸਾਧਾਰਨ		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਿਰਲੇਖ_TYPE_BRIDGE		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਿਰਲੇਖ_TYPE_CARDBUS	2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_BIST		0x0f	/* 8 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BIST_ਕੋਡ_ਨਕਾਬ	0x0f	/* Return result */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BIST_ਸ਼ੁਰੂ		0x40	/* 1 to start BIST, 2 secs or less */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਅਧਾਰ_ਪਤਾ_0	0x10	/* 32 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਅਧਾਰ_ਪਤਾ_1	0x14	/* 32 bits [htype 0,1 only] */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਅਧਾਰ_ਪਤਾ_2	0x18	/* 32 bits [htype 0 only] */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਅਧਾਰ_ਪਤਾ_3	0x1c	/* 32 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਅਧਾਰ_ਪਤਾ_4	0x20	/* 32 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਅਧਾਰ_ਪਤਾ_5	0x24	/* 32 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_SPACE		0x01	/* 0 = memory, 1 = I/O */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_SPACE_IO	0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_SPACE_ਸਮ੍ਰਿਤੀ	0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_ਨਕਾਬ	0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_32	0x00	/* 32 bit address */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_1_M	0x02	/* Below 1M [obsolete] */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_64	0x04	/* 64 bit address */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_MEM_PREFETCH	0x08	/* prefetchable? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_MEM_ਨਕਾਬ	(~0x0fUL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਅਧਾਰ_ਪਤਾ_IO_ਨਕਾਬ	(~0x03UL)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CARDBUS_CIS		0x28
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_SUBSYSTEM_VENDOR_ID	0x2c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_SUBSYSTEM_ID	0x2e
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ROM_ਪਤਾ		0x30	/* Bits 31..11 are address, 10..1 reserved */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ROM_ਪਤਾ_ENABLE	0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ROM_ਪਤਾ_ਨਕਾਬ	(~0x7ffUL)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CAPABILITY_LIST	0x34	/* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਰੁਕਾਵਟ_ਸਤਰ	0x3c	/* 8 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਰੁਕਾਵਟ_PIN	0x3d	/* 8 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MIN_GNT		0x3e	/* 8 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MAX_LAT		0x3f	/* 8 bits */

/* Header type 1 (PCI-to-PCI bridges) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PRIMARY_BUS		0x18	/* Primary bus number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_SECONDARY_BUS	0x19	/* Secondary bus number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_SUBORDINATE_BUS	0x1a	/* Highest bus number behind the bridge */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_SEC_LATENCY_TIMER	0x1b	/* Latency timer for secondary interface */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_IO_ਅਧਾਰ		0x1c	/* I/O range behind the bridge */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_IO_ਹੱਦਬੰਦੀ		0x1d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_IO_RANGE_TYPE_ਨਕਾਬ	0x0fUL	/* I/O bridging type */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_IO_RANGE_TYPE_16	0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_IO_RANGE_TYPE_32	0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_IO_RANGE_ਨਕਾਬ	(~0x0fUL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_SEC_ਸਥਿਤੀ		0x1e	/* Secondary status register, only bit 14 used */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ		0x20	/* Memory range behind */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ	0x22
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਮ੍ਰਿਤੀ_RANGE_TYPE_ਨਕਾਬ 0x0fUL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ਸਮ੍ਰਿਤੀ_RANGE_ਨਕਾਬ	(~0x0fUL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PREF_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ	0x24	/* Prefetchable memory range behind */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PREF_ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ	0x26
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PREF_RANGE_TYPE_ਨਕਾਬ 0x0fUL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PREF_RANGE_TYPE_32	0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PREF_RANGE_TYPE_64	0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PREF_RANGE_ਨਕਾਬ	(~0x0fUL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PREF_ਅਧਾਰ_UPPER32	0x28	/* Upper half of prefetchable memory range */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PREF_ਹੱਦਬੰਦੀ_UPPER32	0x2c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_IO_ਅਧਾਰ_UPPER16	0x30	/* Upper half of I/O addresses */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_IO_ਹੱਦਬੰਦੀ_UPPER16	0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ROM_ADDRESS1	0x38	/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_BRIDGE_CONTROL	0x3e
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BRIDGE_CTL_PARITY	0x01	/* Enable parity detection on secondary interface */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BRIDGE_CTL_SERR	0x02	/* The same for SERR forwarding */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BRIDGE_CTL_ISA	0x04	/* Enable ISA mode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BRIDGE_CTL_VGA	0x08	/* Forward VGA addresses */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BRIDGE_CTL_MASTER_ABORT	0x20  /* Report master aborts */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BRIDGE_CTL_BUS_RESET	0x40	/* Secondary bus reset */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_BRIDGE_CTL_FAST_ਪਿੱਠ	0x80	/* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_CAPABILITY_LIST	0x14
/* 0x15 reserved */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_SEC_ਸਥਿਤੀ	0x16	/* Secondary status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_PRIMARY_BUS	0x18	/* PCI bus number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_CARD_BUS		0x19	/* CardBus bus number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_SUBORDINATE_BUS	0x1a	/* Subordinate bus number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_LATENCY_TIMER	0x1b	/* CardBus latency timer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ_0	0x1c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ_0	0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_ਸਮ੍ਰਿਤੀ_ਅਧਾਰ_1	0x24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_ਸਮ੍ਰਿਤੀ_ਹੱਦਬੰਦੀ_1	0x28
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_IO_ਅਧਾਰ_0	0x2c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_IO_ਅਧਾਰ_0_HI	0x2e
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_IO_ਹੱਦਬੰਦੀ_0	0x30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_IO_ਹੱਦਬੰਦੀ_0_HI	0x32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_IO_ਅਧਾਰ_1	0x34
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_IO_ਅਧਾਰ_1_HI	0x36
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_IO_ਹੱਦਬੰਦੀ_1	0x38
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_IO_ਹੱਦਬੰਦੀ_1_HI	0x3a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_IO_RANGE_ਨਕਾਬ	(~0x03UL)
/* 0x3c-0x3d are same as for htype 0 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_BRIDGE_CONTROL	0x3e
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_PARITY	0x01	/* Similar to standard bridge control register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_SERR		0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_ISA		0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_VGA		0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_MASTER_ABORT	0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_CB_RESET	0x40	/* CardBus reset */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_16_ਦੋਆਧਾਰੀ_ਅੰਕ_INT	0x80	/* Enable interrupt for 16-bit cards */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 0x100	/* Prefetch enable for both memory regions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 0x200
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CB_BRIDGE_CTL_POST_WRITES	0x400
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_SUBSYSTEM_VENDOR_ID	0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_SUBSYSTEM_ID		0x42
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CB_LEGACY_ਬਹੁਲਕ_ਅਧਾਰ		0x44	/* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability lists */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CAP_LIST_ID		0	/* Capability ID */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_PM		0x01	/* Power Management */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_PCIX	0x07	/* PCI-X */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_HT		0x08	/* HyperTransport */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_VNDR	0x09	/* Vendor specific */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_DBG		0x0A	/* Debug port */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_CCRC	0x0B	/* CompactPCI Central Resource Control */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_SHPC 	0x0C	/* PCI Standard Hot-Plug Controller */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_SSVID	0x0D	/* Bridge subsystem vendor/device ID */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_AGP3	0x0E	/* AGP Target PCI-PCI bridge */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_EXP 	0x10	/* PCI Express */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CAP_ID_MSIX	0x11	/* MSI-X */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CAP_LIST_ਅਗਲਾ	1	/* Next capability in the list */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CAP_FLAGS		2	/* Capability defined flags (16 bits) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CAP_SIZEOF		4

/* Power Management Registers */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PM_PMC		2	/* PM Capabilities Register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_VER_ਨਕਾਬ	0x0007	/* Version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME_CLOCK	0x0008	/* PME clock required */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_RESERVED    0x0010  /* Reserved field */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_DSI		0x0020	/* Device specific initialization */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_AUX_POWER	0x01C0	/* Auxilliary power support mask */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_D1		0x0200	/* D1 power state support */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_D2		0x0400	/* D2 power state support */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME		0x0800	/* PME pin supported */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME_ਨਕਾਬ	0xF800	/* PME Mask of all supported states */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME_D0	0x0800	/* PME# from D0 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME_D1	0x1000	/* PME# from D1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME_D2	0x2000	/* PME# from D2 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME_D3	0x4000	/* PME# from D3 (hot) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME_D3cold	0x8000	/* PME# from D3 (cold) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CAP_PME_ਖਿਸਕਾਓ	11	/* Start of the PME Mask in PMC */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PM_CTRL		4	/* PM control and status register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CTRL_STATE_ਨਕਾਬ	0x0003	/* Current power state (D0 to D3) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CTRL_NO_SOFT_RESET	0x0004	/* No reset for D3hot->D0 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CTRL_PME_ENABLE	0x0100	/* PME pin enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CTRL_ਅੰਕੜੇ_SEL_ਨਕਾਬ	0x1e00	/* Data select (??) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CTRL_ਅੰਕੜੇ_SCALE_ਨਕਾਬ	0x6000	/* Data scale (??) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_CTRL_PME_ਸਥਿਤੀ	0x8000	/* PME pin status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PM_PPB_EXTENSIONS	6	/* PPB support extensions (??) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_PPB_B2_B3	0x40	/* Stop clock when in D3hot (??) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PM_BPCC_ENABLE	0x80	/* Bus power/clock control enable (??) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PM_ਅੰਕੜੇ_ਰਜਿਸਟਰ	7	/* (??) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PM_SIZEOF		8

/* AGP registers */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_AGP_ਸੰਸਕਰਣ		2	/* BCD version number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_AGP_RFU		3	/* Rest of capability flags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_AGP_ਸਥਿਤੀ		4	/* Status register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_ਸਥਿਤੀ_RQ_ਨਕਾਬ	0xff000000	/* Maximum number of requests - 1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_ਸਥਿਤੀ_SBA	0x0200	/* Sideband addressing supported */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_ਸਥਿਤੀ_64_ਦੋਆਧਾਰੀ_ਅੰਕ	0x0020	/* 64-bit addressing supported */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_ਸਥਿਤੀ_FW	0x0010	/* FW transfers supported */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_ਸਥਿਤੀ_RATE4	0x0004	/* 4x transfer rate supported */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_ਸਥਿਤੀ_RATE2	0x0002	/* 2x transfer rate supported */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_ਸਥਿਤੀ_RATE1	0x0001	/* 1x transfer rate supported */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_AGP_COMMAND		8	/* Control register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_COMMAND_RQ_ਨਕਾਬ 0xff000000  /* Master: Maximum number of requests */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_COMMAND_SBA	0x0200	/* Sideband addressing enabled */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_COMMAND_AGP	0x0100	/* Allow processing of AGP transactions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_COMMAND_64_ਦੋਆਧਾਰੀ_ਅੰਕ	0x0020 	/* Allow processing of 64-bit addresses */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_COMMAND_FW	0x0010 	/* Force FW transfers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_COMMAND_RATE4	0x0004	/* Use 4x rate */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_COMMAND_RATE2	0x0002	/* Use 2x rate */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_AGP_COMMAND_RATE1	0x0001	/* Use 1x rate */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_AGP_SIZEOF		12

/* Vital Product Data */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VPD_ADDR		2	/* Address to access (15 bits!) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_VPD_ADDR_ਨਕਾਬ	0x7fff	/* Address mask */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_VPD_ADDR_F		0x8000	/* Write 0, 1 indicates completion */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VPD_ਅੰਕੜੇ		4	/* 32-bits of data returned here */

/* Slot Identification */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_SID_ESR		2	/* Expansion Slot Register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_SID_ESR_NSLOTS	0x1f	/* Number of expansion slots available */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_SID_ESR_FIC	0x20	/* First In Chassis Flag */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_SID_CHASSIS_NR	3	/* Chassis Number */

/* Message Signalled Interrupts registers */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSI_FLAGS		2	/* Various flags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_MSI_FLAGS_64_ਦੋਆਧਾਰੀ_ਅੰਕ	0x80	/* 64-bit addresses allowed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_MSI_FLAGS_QSIZE	0x70	/* Message queue size configured */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_MSI_FLAGS_QMASK	0x0e	/* Maximum queue size available */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_MSI_FLAGS_ENABLE	0x01	/* MSI feature enabled */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_MSI_FLAGS_MASKBIT	0x100	/* 64-bit mask bits allowed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSI_RFU		3	/* Rest of capability flags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSI_ਪਤਾ_LO	4	/* Lower 32 bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSI_ਪਤਾ_HI	8	/* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSI_ਅੰਕੜੇ_32		8	/* 16 bits of data for 32-bit devices */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSI_ਅੰਕੜੇ_64		12	/* 16 bits of data for 64-bit devices */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSI_ਨਕਾਬ_ਦੋਆਧਾਰੀ_ਅੰਕ	16	/* Mask bits register */

/* MSI-X registers (these are at offset PCI_MSIX_FLAGS) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSIX_FLAGS		2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_MSIX_FLAGS_QSIZE	0x7FF
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_MSIX_FLAGS_ENABLE	(1 << 15)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_MSIX_FLAGS_MASKALL	(1 << 14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSIX_FLAGS_BIRMASK	(7 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_MSIX_FLAGS_BITMASK	(1 << 0)

/* CompactPCI Hotswap Register */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_CHSWP_CSR		2	/* Control and Status Register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CHSWP_DHA		0x01	/* Device Hiding Arm */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CHSWP_EIM		0x02	/* ENUM# Signal Mask */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CHSWP_PIE		0x04	/* Pending Insert or Extract */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CHSWP_LOO		0x08	/* LED On / Off */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CHSWP_PI		0x30	/* Programming Interface */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CHSWP_EXT		0x40	/* ENUM# status - extraction */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_CHSWP_INS		0x80	/* ENUM# status - insertion */

/* PCI-X registers */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_X_CMD		2	/* Modes & Features */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_DPERR_E	0x0001	/* Data Parity Error Recovery Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ERO		0x0002	/* Enable Relaxed Ordering */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_READ_512	0x0000	/* 512 byte maximum read byte count */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_READ_1K	0x0004	/* 1Kbyte maximum read byte count */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_READ_2K	0x0008	/* 2Kbyte maximum read byte count */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_READ_4K	0x000c	/* 4Kbyte maximum read byte count */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_MAX_READ	0x000c	/* Max Memory Read Byte Count */
				/* Max # of outstanding split transactions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਵੰਡੋ_1	0x0000	/* Max 1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਵੰਡੋ_2	0x0010	/* Max 2 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਵੰਡੋ_3	0x0020	/* Max 3 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਵੰਡੋ_4	0x0030	/* Max 4 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਵੰਡੋ_8	0x0040	/* Max 8 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਵੰਡੋ_12	0x0050	/* Max 12 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਵੰਡੋ_16	0x0060	/* Max 16 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਵੰਡੋ_32	0x0070	/* Max 32 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_MAX_ਵੰਡੋ	0x0070	/* Max Outstanding Split Transactions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_CMD_ਸੰਸਕਰਣ(x) 	(((x) >> 12) & 3) /* Version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_X_ਸਥਿਤੀ		4	/* PCI-X capabilities */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_DEVFN	0x000000ff	/* A copy of devfn */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_BUS	0x0000ff00	/* A copy of bus nr */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_64_ਦੋਆਧਾਰੀ_ਅੰਕ	0x00010000	/* 64-bit device */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_133_MHZ	0x00020000	/* 133 MHz capable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_SPL_DISC	0x00040000	/* Split Completion Discarded */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_UNX_SPL	0x00080000	/* Unexpected Split Completion */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_COMPLEX	0x00100000	/* Device Complexity */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_MAX_READ	0x00600000	/* Designed Max Memory Read Count */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_MAX_ਵੰਡੋ	0x03800000	/* Designed Max Outstanding Split Transactions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_MAX_CUM	0x1c000000	/* Designed Max Cumulative Read Size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_SPL_ERR	0x20000000	/* Rcvd Split Completion Error Msg */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_266_MHZ	0x40000000	/* 266 MHz capable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_X_ਸਥਿਤੀ_533_MHZ	0x80000000	/* 533 MHz capable */

/* PCI Express capability registers */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_FLAGS		2	/* Capabilities register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_FLAGS_VERS	0x000f	/* Capability version */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_FLAGS_TYPE	0x00f0	/* Device/Port type */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_TYPE_ENDPOINT	0x0	/* Express Endpoint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_TYPE_LEG_ਸਿਰਾ	0x1	/* Legacy Endpoint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_TYPE_ਜੜ੍ਹ_ਪੋਰਟ 0x4	/* Root Port */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_TYPE_UPSTREAM	0x5	/* Upstream Port */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_TYPE_DOWNSTREAM 0x6	/* Downstream Port */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_TYPE_PCI_BRIDGE 0x7	/* PCI/PCI-X Bridge */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_FLAGS_ਥਾਂ	0x0100	/* Slot implemented */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_FLAGS_IRQ	0x3e00	/* Interrupt message number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_DEVCAP		4	/* Device capabilities */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_PAYLOAD	0x07	/* Max_Payload_Size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_PHANTOM	0x18	/* Phantom functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_EXT_ਟੈਗ	0x20	/* Extended tags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_L0S	0x1c0	/* L0s Acceptable Latency */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_L1	0xe00	/* L1 Acceptable Latency */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_ATN_BUT	0x1000	/* Attention Button Present */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_ATN_IND	0x2000	/* Attention Indicator Present */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_PWR_IND	0x4000	/* Power Indicator Present */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_RBER	0x8000	/* Role-Based Error Reporting */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_PWR_VAL	0x3fc0000 /* Slot Power Limit Value */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_PWR_SCL	0xc000000 /* Slot Power Limit Scale */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP_FLR     0x10000000 /* Function Level Reset */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_DEVCTL		8	/* Device Control */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_CERE	0x0001	/* Correctable Error Reporting En. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_NFERE	0x0002	/* Non-Fatal Error Reporting Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_FERE	0x0004	/* Fatal Error Reporting Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_URRE	0x0008	/* Unsupported Request Reporting En. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_PAYLOAD	0x00e0	/* Max_Payload_Size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_EXT_ਟੈਗ	0x0100	/* Extended Tag Field Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_PHANTOM	0x0200	/* Phantom Functions Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_AUX_PME	0x0400	/* Auxiliary Power PM Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_READRQ	0x7000	/* Max_Read_Request_Size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL_BCR_FLR 0x8000  /* Bridge Configuration Retry / FLR */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_DEVSTA		10	/* Device Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVSTA_CED	0x01	/* Correctable Error Detected */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVSTA_NFED	0x02	/* Non-Fatal Error Detected */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVSTA_FED	0x04	/* Fatal Error Detected */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVSTA_URD	0x08	/* Unsupported Request Detected */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVSTA_AUXPD	0x10	/* AUX Power Detected */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVSTA_TRPND	0x20	/* Transactions Pending */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_LNKCAP		12	/* Link Capabilities */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKCAP_ASPMS	0xc00	/* ASPM Support */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKCAP_L0SEL	0x7000	/* L0s Exit Latency */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKCAP_L1EL	0x38000	/* L1 Exit Latency */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKCAP_CLKPM	0x40000	/* L1 Clock Power Management */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_LNKCTL		16	/* Link Control */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKCTL_RL	0x20	/* Retrain Link */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKCTL_CCC	0x40	/* Common Clock COnfiguration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKCTL_CLKREQ_EN 0x100	/* Enable clkreq */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_LNKSTA		18	/* Link Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKSTA_LT	0x800	/* Link Training */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_LNKSTA_SLC	0x1000	/* Slot Clock Configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_SLTCAP		20	/* Slot Capabilities */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_SLTCTL		24	/* Slot Control */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_SLTSTA		26	/* Slot Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_RTCTL		28	/* Root Control */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_RTCTL_SECEE	0x01	/* System Error on Correctable Error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_RTCTL_SENFEE	0x02	/* System Error on Non-Fatal Error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_RTCTL_SEFEE	0x04	/* System Error on Fatal Error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_RTCTL_PMEIE	0x08	/* PME Interrupt Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_RTCTL_CRSSVE	0x10	/* CRS Software Visibility Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_RTCAP		30	/* Root Capabilities */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_RTSTA		32	/* Root Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_DEVCAP2		36	/* Device Capabilities 2 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCAP2_ARI	0x20	/* Alternative Routing-ID */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXP_DEVCTL2		40	/* Device Control 2 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_EXP_DEVCTL2_ARI	0x20	/* Alternative Routing-ID */

/* Extended Capabilities (PCI-X 2.0 and Express) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXT_CAP_ID(ਸਿਰਲੇਖ)		(ਸਿਰਲੇਖ & 0x0000ffff)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXT_CAP_VER(ਸਿਰਲੇਖ)		((ਸਿਰਲੇਖ >> 16) & 0xf)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXT_CAP_ਅਗਲਾ(ਸਿਰਲੇਖ)	((ਸਿਰਲੇਖ >> 20) & 0xffc)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXT_CAP_ID_ERR	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXT_CAP_ID_VC	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXT_CAP_ID_DSN	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXT_CAP_ID_PWR	4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_EXT_CAP_ID_ARI	14

/* Advanced Error Reporting */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_UNCOR_ਸਥਿਤੀ	4	/* Uncorrectable Error Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_TRAIN	0x00000001	/* Training */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_DLP	0x00000010	/* Data Link Protocol */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_POISON_TLP	0x00001000	/* Poisoned TLP */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_FCP	0x00002000	/* Flow Control Protocol */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_COMP_ਸਮਾਂ	0x00004000	/* Completion Timeout */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_COMP_ABORT	0x00008000	/* Completer Abort */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_UNX_COMP	0x00010000	/* Unexpected Completion */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_RX_OVER	0x00020000	/* Receiver Overflow */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_MALF_TLP	0x00040000	/* Malformed TLP */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_ECRC	0x00080000	/* ECRC Error Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_UNC_UNSUP	0x00100000	/* Unsupported Request */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_UNCOR_ਨਕਾਬ	8	/* Uncorrectable Error Mask */
	/* Same bits as above */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_UNCOR_SEVER	12	/* Uncorrectable Error Severity */
	/* Same bits as above */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_COR_ਸਥਿਤੀ	16	/* Correctable Error Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_COR_RCVR	0x00000001	/* Receiver Error Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_COR_BAD_TLP	0x00000040	/* Bad TLP Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_COR_BAD_DLLP	0x00000080	/* Bad DLLP Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_COR_REP_ROLL	0x00000100	/* REPLAY_NUM Rollover */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_COR_REP_TIMER	0x00001000	/* Replay Timer Timeout */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_COR_ਨਕਾਬ	20	/* Correctable Error Mask */
	/* Same bits as above */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_CAP		24	/* Advanced Error Capabilities */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_CAP_FEP(x)	((x) & 31)	/* First Error Pointer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_CAP_ECRC_GENC	0x00000020	/* ECRC Generation Capable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_CAP_ECRC_GENE	0x00000040	/* ECRC Generation Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_CAP_ECRC_CHKC	0x00000080	/* ECRC Check Capable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ERR_CAP_ECRC_CHKE	0x00000100	/* ECRC Check Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਸਿਰਲੇਖ_LOG	28	/* Header Log Register (16 bytes) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_COMMAND	44	/* Root Error Command */
/* Correctable Err Reporting Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_CMD_COR_EN		0x00000001
/* Non-fatal Err Reporting Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_CMD_NONFATAL_EN	0x00000002
/* Fatal Err Reporting Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_CMD_FATAL_EN	0x00000004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_ਸਥਿਤੀ	48
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_COR_RCV		0x00000001	/* ERR_COR Received */
/* Multi ERR_COR Received */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_MULTI_COR_RCV	0x00000002
/* ERR_FATAL/NONFATAL Recevied */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_UNCOR_RCV		0x00000004
/* Multi ERR_FATAL/NONFATAL Recevied */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_MULTI_UNCOR_RCV	0x00000008
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_ਪਹਿਲਾ_FATAL	0x00000010	/* First Fatal */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_NONFATAL_RCV	0x00000020	/* Non-Fatal Received */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_FATAL_RCV		0x00000040	/* Fatal Received */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_COR_SRC	52
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ERR_ਜੜ੍ਹ_SRC	54

/* Virtual Channel */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VC_ਪੋਰਟ_REG1	4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VC_ਪੋਰਟ_REG2	8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VC_ਪੋਰਟ_CTRL	12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VC_ਪੋਰਟ_ਸਥਿਤੀ	14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VC_RES_CAP		16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VC_RES_CTRL		20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_VC_RES_ਸਥਿਤੀ	26

/* Power Budgeting */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PWR_DSR		4	/* Data Select Register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PWR_ਅੰਕੜੇ		8	/* Data Register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PWR_ਅੰਕੜੇ_ਅਧਾਰ(x)	((x) & 0xff)	    /* Base Power */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PWR_ਅੰਕੜੇ_SCALE(x)	(((x) >> 8) & 3)    /* Data Scale */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PWR_ਅੰਕੜੇ_PM_SUB(x)	(((x) >> 10) & 7)   /* PM Sub State */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PWR_ਅੰਕੜੇ_PM_STATE(x) (((x) >> 13) & 3) /* PM State */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PWR_ਅੰਕੜੇ_TYPE(x)	(((x) >> 15) & 7)   /* Type */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PWR_ਅੰਕੜੇ_RAIL(x)	(((x) >> 18) & 7)   /* Power Rail */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_PWR_CAP		12	/* Capability */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_PWR_CAP_BUDGET(x)	((x) & 1)	/* Included in system budget */

/*
 * Hypertransport sub capability types
 *
 * Unfortunately there are both 3 bit and 5 bit capability types defined
 * in the HT spec, catering for that is a little messy. You probably don't
 * want to use these directly, just use pci_find_ht_capability() and it
 * will do the right thing for you.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_3_ਦੋਆਧਾਰੀ_ਅੰਕ_CAP_ਨਕਾਬ	0xE0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_SLAVE	0x00	/* Slave/Primary link configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_HOST		0x20	/* Host/Secondary link configuration */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_5_ਦੋਆਧਾਰੀ_ਅੰਕ_CAP_ਨਕਾਬ	0xF8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_IRQ		0x80	/* IRQ Configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_REMAPPING_40	0xA0	/* 40 bit address remapping */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_REMAPPING_64 0xA2	/* 64 bit address remapping */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_UNITID_CLUMP	0x90	/* Unit ID clumping */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_EXTCONF	0x98	/* Extended Configuration Space Access */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_MSI_MAPPING	0xA8	/* MSI Mapping Capability */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  HT_MSI_FLAGS		0x02		/* Offset to flags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  HT_MSI_FLAGS_ENABLE	0x1		/* Mapping enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  HT_MSI_FLAGS_FIXED	0x2		/* Fixed mapping only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  HT_MSI_FIXED_ADDR	0x00000000FEE00000ULL	/* Fixed addr */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  HT_MSI_ADDR_LO		0x04		/* Offset to low addr bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  HT_MSI_ADDR_LO_ਨਕਾਬ	0xFFF00000	/* Low address bit mask */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  HT_MSI_ADDR_HI		0x08		/* Offset to high addr bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_ਸਿੱਧਾ_ROUTE	0xB0	/* Direct routing configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_VCSET	0xB8	/* Virtual Channel configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_ERROR_RETRY	0xC0	/* Retry on error configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_GEN3		0xD0	/* Generation 3 hypertransport configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HT_CAPTYPE_PM		0xE0	/* Hypertransport powermanagement configuration */

/* Alternative Routing-ID Interpretation */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ARI_CAP		0x04	/* ARI Capability Register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ARI_CAP_MFVC	0x0001	/* MFVC Function Groups Capability */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ARI_CAP_ACS	0x0002	/* ACS Function Groups Capability */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ARI_CAP_NFN(x)	(((x) >> 8) & 0xff) /* Next Function Number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ARI_CTRL		0x06	/* ARI Control Register */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ARI_CTRL_MFVC	0x0001	/* MFVC Function Groups Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ARI_CTRL_ACS	0x0002	/* ACS Function Groups Enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  PCI_ARI_CTRL_FG(x)	(((x) >> 4) & 7) /* Function Group */

#ਜੇ_ਅੰਤ /* LINUX_PCI_REGS_H */
