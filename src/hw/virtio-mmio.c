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

ಶೂನ್ಯ virtio_mmio_ಸಿದ್ಧತೆ_acpi(ಶೂನ್ಯ)
{
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *virtio_hid = "LNRO0005";
    ರಚನೆ acpi_device *dev;
    u64 mem, irq, ಬಳಸದ;

    ಗಾಗಿ (dev = acpi_dsdt_ಕಂಡುಹಿಡಿ_string(ಖಾಲಿ, virtio_hid);
         dev != ಖಾಲಿ;
         dev = acpi_dsdt_ಕಂಡುಹಿಡಿ_string(dev, virtio_hid)) {
        ಆದರೆ (acpi_dsdt_ಕಂಡುಹಿಡಿ_mem(dev, &mem, &ಬಳಸದ) < 0)
            ಮುಂದುವರಿ;
        ಆದರೆ (acpi_dsdt_ಕಂಡುಹಿಡಿ_irq(dev, &irq) < 0)
            ಮುಂದುವರಿ;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_ಸಿದ್ಧತೆ_one(mem);
    }
}

ಶೂನ್ಯ virtio_mmio_ಸಿದ್ಧತೆ_one(u64 addr)
{
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಹೆಸರುಗಳು[] = {
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
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
    u32 magic, ಆವೃತ್ತಿ_2, devid;
    ಶೂನ್ಯ *mmio;

    ಆದರೆ (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        ಹಿಂತಿರುಗಿ;
    }

    mmio = (ಶೂನ್ಯ*)(u32)(addr);
    magic = readl(mmio);
    ಆದರೆ (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        ಹಿಂತಿರುಗಿ;
    }
    ಆವೃತ್ತಿ_2 = readl(mmio+4);
    ಆದರೆ (ಆವೃತ್ತಿ_2 != 1 /* legacy */ &&
        ಆವೃತ್ತಿ_2 != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, ಆವೃತ್ತಿ_2);
        ಹಿಂತಿರುಗಿ;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_ಗಾತ್ರ(ಹೆಸರುಗಳು) && ಹೆಸರುಗಳು[devid] != ಖಾಲಿ)
        ? ಹೆಸರುಗಳು[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, ಆವೃತ್ತಿ_2 == 1 ? ", legacy" : "");

    ಆಯ್ಕೆ (devid) {
    ಸ್ಥಿತಿ 2: /* blk */
        ಸತತ_thread(init_virtio_blk_mmio, mmio);
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 8: /* scsi */
        ಸತತ_thread(init_virtio_scsi_mmio, mmio);
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        ನಿಲ್ಲಿಸು;
    }
}

ಶೂನ್ಯ vp_init_mmio(ರಚನೆ vp_device *vp, ಶೂನ್ಯ *mmio)
{
    memset(vp, 0, ಗಾತ್ರ(*vp));
    vp->use_mmio = 1;
    vp->common.ಬಹುಲಕ = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.ಬಹುಲಕ = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_ಗಣ_ಸ್ಥಿತಿ(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
