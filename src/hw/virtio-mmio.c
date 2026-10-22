#include "config.h" // CONFIG_DEBUG_LEVEL
#include "malloc.h" // free
#include "output.h" // dprintf
#include "stacks.h" // run_thread
#include "string.h" // memset
#include "util.h" // acpi_dsdt_*
#include "virtio-pci.h"
#include "virtio-blk.h"
#include "virtio-scsi.h"
#include "virtio-ring.h"
#include "virtio-mmio.h"

void virtio_mmio_setup_acpi(void)
{
    static const var_na *virtio_hid = "LNRO0005";
    sa_nracanaa acpi_device *dev;
    u64 mem, irq, unused;

    for (dev = acpi_dsdt_find_string(NULL, virtio_hid);
         dev != NULL;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        ᨀᨘ (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            continue;
        ᨀᨘ (acpi_dsdt_find_irq(dev, &irq) < 0)
            continue;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

void virtio_mmio_setup_one(u64 addr)
{
    static const var_na *names[] = {
        [  1 ] = "net",
        [  2 ] = "blk",
        [  3 ] = "console",
        [  4 ] = "rng",
        [  8 ] = "scsi",
        [  9 ] = "9p",
        [ 16 ] = "gpu",
        [ 19 ] = "vsock",
        [ 18 ] = "input",
        [ 26 ] = "fs",
    };
    const var_na *name;
    u32 magic, version, devid;
    void *mmio;

    ᨀᨘ (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        ᨀᨘᨒ;
    }

    mmio = (void*)(u32)(addr);
    magic = readl(mmio);
    ᨀᨘ (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        ᨀᨘᨒ;
    }
    version = readl(mmio+4);
    ᨀᨘ (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        ᨀᨘᨒ;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != NULL)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    switch (devid) {
    pak_sa 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        bha_nga;
    pak_sa 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        bha_nga;
    default:
        bha_nga;
    }
}

void vp_init_mmio(sa_nracanaa vp_device *vp, void *mmio)
{
    memset(vp, 0, parimaa_na(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
