#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _PCI_CAP_H
#ವ್ಯಾಖ್ಯಾನಿಸು _PCI_CAP_H

#ಅನ್ತರ್ಭಾವ "types.h"

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
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_CAP_REDHAT_TYPE_ಸ್ಥಾನಾಂತರ  3

/* List of valid Red Hat vendor-specific capability types */
#ವ್ಯಾಖ್ಯಾನಿಸು REDHAT_CAP_RESOURCE_RESERVE 1


/* Offsets of RESOURCE_RESERVE capability fields */
#ವ್ಯಾಖ್ಯಾನಿಸು RES_RESERVE_BUS_RES        4
#ವ್ಯಾಖ್ಯಾನಿಸು RES_RESERVE_IO             8
#ವ್ಯಾಖ್ಯಾನಿಸು RES_RESERVE_MEM            16
#ವ್ಯಾಖ್ಯಾನಿಸು RES_RESERVE_PREF_MEM_32    20
#ವ್ಯಾಖ್ಯಾನಿಸು RES_RESERVE_PREF_MEM_64    24
#ವ್ಯಾಖ್ಯಾನಿಸು RES_RESERVE_CAP_ಗಾತ್ರ       32

#ಆದರೆ_ಅಂತ್ಯ /* _PCI_CAP_H */
