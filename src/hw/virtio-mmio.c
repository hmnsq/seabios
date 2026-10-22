#شامل "config.h" // CONFIG_DEBUG_LEVEL
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "stacks.h" // run_thread
#شامل "string.h" // memset
#شامل "util.h" // acpi_dsdt_*
#شامل "virtio-pci.h"
#شامل "virtio-blk.h"
#شامل "virtio-scsi.h"
#شامل "virtio-ring.h"
#شامل "virtio-mmio.h"

تهی virtio_mmio_setup_acpi(تهی)
{
    ایستا ثابت نویسه *virtio_hid = "LNRO0005";
    ساختار acpi_device *dev;
    u64 mem, irq, unused;

    برای (dev = acpi_dsdt_find_string(پوچ, virtio_hid);
         dev != پوچ;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        اگر (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            ادامه;
        اگر (acpi_dsdt_find_irq(dev, &irq) < 0)
            ادامه;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

تهی virtio_mmio_setup_one(u64 addr)
{
    ایستا ثابت نویسه *names[] = {
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
    ثابت نویسه *name;
    u32 magic, version, devid;
    تهی *mmio;

    اگر (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        بازگشت;
    }

    mmio = (تهی*)(u32)(addr);
    magic = readl(mmio);
    اگر (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        بازگشت;
    }
    version = readl(mmio+4);
    اگر (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        بازگشت;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != پوچ)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    انتخاب (devid) {
    حالت 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        بشکن;
    حالت 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        بشکن;
    پیشفرض:
        بشکن;
    }
}

تهی vp_init_mmio(ساختار vp_device *vp, تهی *mmio)
{
    memset(vp, 0, اندازه(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
