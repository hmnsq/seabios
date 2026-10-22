#ضمن "config.h" // CONFIG_DEBUG_LEVEL
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "stacks.h" // run_thread
#ضمن "string.h" // memset
#ضمن "util.h" // acpi_dsdt_*
#ضمن "virtio-pci.h"
#ضمن "virtio-blk.h"
#ضمن "virtio-scsi.h"
#ضمن "virtio-ring.h"
#ضمن "virtio-mmio.h"

فراغ virtio_mmio_تياري_acpi(فراغ)
{
    ساكن ثابت حرف *virtio_hid = "LNRO0005";
    بنية acpi_device *dev;
    u64 mem, irq, اڻ_استعمال_ٿيل;

    لكل (dev = acpi_dsdt_ڳوليو_string(عدم, virtio_hid);
         dev != عدم;
         dev = acpi_dsdt_ڳوليو_string(dev, virtio_hid)) {
        إذا (acpi_dsdt_ڳوليو_mem(dev, &mem, &اڻ_استعمال_ٿيل) < 0)
            استمر;
        إذا (acpi_dsdt_ڳوليو_irq(dev, &irq) < 0)
            استمر;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_تياري_one(mem);
    }
}

فراغ virtio_mmio_تياري_one(u64 addr)
{
    ساكن ثابت حرف *نالا[] = {
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
    ثابت حرف *name;
    u32 magic, نسخو_2, devid;
    فراغ *mmio;

    إذا (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        أرجع;
    }

    mmio = (فراغ*)(u32)(addr);
    magic = readl(mmio);
    إذا (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        أرجع;
    }
    نسخو_2 = readl(mmio+4);
    إذا (نسخو_2 != 1 /* legacy */ &&
        نسخو_2 != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, نسخو_2);
        أرجع;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_سائيز(نالا) && نالا[devid] != عدم)
        ? نالا[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, نسخو_2 == 1 ? ", legacy" : "");

    بدل (devid) {
    حالة 2: /* blk */
        لاڳيتو_thread(init_virtio_blk_mmio, mmio);
        اكسر;
    حالة 8: /* scsi */
        لاڳيتو_thread(init_virtio_scsi_mmio, mmio);
        اكسر;
    افتراضي:
        اكسر;
    }
}

فراغ vp_init_mmio(بنية vp_device *vp, فراغ *mmio)
{
    memset(vp, 0, حجم(*vp));
    vp->use_mmio = 1;
    vp->common.گهڻائي = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.گهڻائي = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_سيٽ_حالت(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
