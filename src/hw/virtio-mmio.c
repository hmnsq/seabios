#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_DEBUG_LEVEL
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // run_thread
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // acpi_dsdt_*
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-pci.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-blk.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-scsi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-mmio.h"

ਖਾਲੀ virtio_mmio_ਤਿਆਰੀ_acpi(ਖਾਲੀ)
{
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *virtio_hid = "LNRO0005";
    ਬਣਤਰ acpi_device *dev;
    u64 mem, irq, ਅਣਵਰਤਿਆ;

    ਲਈ (dev = acpi_dsdt_ਲੱਭੋ_string(ਨਿਰਾ, virtio_hid);
         dev != ਨਿਰਾ;
         dev = acpi_dsdt_ਲੱਭੋ_string(dev, virtio_hid)) {
        ਜੇ (acpi_dsdt_ਲੱਭੋ_mem(dev, &mem, &ਅਣਵਰਤਿਆ) < 0)
            ਜਾਰੀ;
        ਜੇ (acpi_dsdt_ਲੱਭੋ_irq(dev, &irq) < 0)
            ਜਾਰੀ;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_ਤਿਆਰੀ_one(mem);
    }
}

ਖਾਲੀ virtio_mmio_ਤਿਆਰੀ_one(u64 addr)
{
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਨਾਂ[] = {
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
    ਅਚਲ ਅੱਖਰ *name;
    u32 magic, ਸੰਸਕਰਣ_2, devid;
    ਖਾਲੀ *mmio;

    ਜੇ (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        ਵਾਪਸ;
    }

    mmio = (ਖਾਲੀ*)(u32)(addr);
    magic = readl(mmio);
    ਜੇ (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        ਵਾਪਸ;
    }
    ਸੰਸਕਰਣ_2 = readl(mmio+4);
    ਜੇ (ਸੰਸਕਰਣ_2 != 1 /* legacy */ &&
        ਸੰਸਕਰਣ_2 != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, ਸੰਸਕਰਣ_2);
        ਵਾਪਸ;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_ਮਾਪ(ਨਾਂ) && ਨਾਂ[devid] != ਨਿਰਾ)
        ? ਨਾਂ[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, ਸੰਸਕਰਣ_2 == 1 ? ", legacy" : "");

    ਚੋਣ (devid) {
    ਹਾਲਤ 2: /* blk */
        ਲੜੀਵਾਰ_thread(init_virtio_blk_mmio, mmio);
        ਰੋਕੋ;
    ਹਾਲਤ 8: /* scsi */
        ਲੜੀਵਾਰ_thread(init_virtio_scsi_mmio, mmio);
        ਰੋਕੋ;
    ਮੂਲ:
        ਰੋਕੋ;
    }
}

ਖਾਲੀ vp_init_mmio(ਬਣਤਰ vp_device *vp, ਖਾਲੀ *mmio)
{
    memset(vp, 0, ਆਕਾਰ(*vp));
    vp->use_mmio = 1;
    vp->common.ਬਹੁਲਕ = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.ਬਹੁਲਕ = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_ਸੈੱਟ_ਸਥਿਤੀ(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
