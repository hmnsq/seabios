#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_DEBUG_LEVEL
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // run_thread
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // acpi_dsdt_*
#ಅನ್ತರ್ಭಾವ "virtio-pci.h"
#ಅನ್ತರ್ಭಾವ "virtio-blk.h"
#ಅನ್ತರ್ಭಾವ "virtio-scsi.h"
#ಅನ್ತರ್ಭಾವ "virtio-ring.h"
#ಅನ್ತರ್ಭಾವ "virtio-mmio.h"

ಶೂನ್ಯ virtio_mmio_setup_acpi(ಶೂನ್ಯ)
{
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *virtio_hid = "LNRO0005";
    ಸಂರಚನಾ acpi_device *dev;
    u64 mem, irq, unused;

    ಪ್ರತಿ (dev = acpi_dsdt_find_string(ರಿಕ್ತ, virtio_hid);
         dev != ರಿಕ್ತ;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        ಯದಿ (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            ಅನುವರ್ತ;
        ಯದಿ (acpi_dsdt_find_irq(dev, &irq) < 0)
            ಅನುವರ್ತ;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

ಶೂನ್ಯ virtio_mmio_setup_one(u64 addr)
{
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *names[] = {
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
    ಧ್ರುವ ವರ್ಣ *name;
    u32 magic, version, devid;
    ಶೂನ್ಯ *mmio;

    ಯದಿ (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        ಪ್ರತ್ಯಾಗಮ;
    }

    mmio = (ಶೂನ್ಯ*)(u32)(addr);
    magic = readl(mmio);
    ಯದಿ (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        ಪ್ರತ್ಯಾಗಮ;
    }
    version = readl(mmio+4);
    ಯದಿ (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        ಪ್ರತ್ಯಾಗಮ;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != ರಿಕ್ತ)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    ವಿಕಲ್ಪ (devid) {
    ಪಕ್ಷ 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        ಭಙ್ಗ;
    ಪಕ್ಷ 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        ಭಙ್ಗ;
    }
}

ಶೂನ್ಯ vp_init_mmio(ಸಂರಚನಾ vp_device *vp, ಶೂನ್ಯ *mmio)
{
    memset(vp, 0, ಪರಿಮಾಣ(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
