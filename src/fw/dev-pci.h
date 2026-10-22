#که_نه_تعریف_شوی _PCI_CAP_H
#تعریف_کړه _PCI_CAP_H

#ضمن "types.h"

/*
 *
 * QEMU-specific vendor(Red Hat)-specific capability.
 * It's intended to provide some hints for firmware to init PCI devices.
 *
 * Its structure is shown below:
 *
 * Header:
 *
 * u8 id;       Standard PCI Capability Header field
 * u8 next;     Standard PCI Capability Header field
 * u8 len;      Standard PCI Capability Header field
 * u8 type;     Red Hat vendor-specific capability type
 * Data:
 *
 * u32 bus_res;     minimum bus number to reserve;
 *                  this is necessary for PCI Express Root Ports
 *                  to support PCI bridges hotplug
 * u64 io;          IO space to reserve
 * u32 mem;         non-prefetchable memory to reserve
 *
 * At most of the following two fields may be set to a value
 * different from 0xFF...F:
 * u32 prefetchable_mem_32;     prefetchable memory to reserve (32-bit MMIO)
 * u64 prefetchable_mem_64;     prefetchable memory to reserve (64-bit MMIO)
 *
 * If any field value in Data section is 0xFF...F,
 * it means that such kind of reservation is not needed and must be ignored.
 *
*/

/* Offset of vendor-specific capability type field */
#تعریف_کړه PCI_CAP_REDHAT_TYPE_دننه_واټن  3

/* List of valid Red Hat vendor-specific capability types */
#تعریف_کړه REDHAT_CAP_RESOURCE_RESERVE 1


/* Offsets of RESOURCE_RESERVE capability fields */
#تعریف_کړه RES_RESERVE_BUS_RES        4
#تعریف_کړه RES_RESERVE_IO             8
#تعریف_کړه RES_RESERVE_MEM            16
#تعریف_کړه RES_RESERVE_PREF_MEM_32    20
#تعریف_کړه RES_RESERVE_PREF_MEM_64    24
#تعریف_کړه RES_RESERVE_CAP_اندازه       32

#د_که_پای /* _PCI_CAP_H */
