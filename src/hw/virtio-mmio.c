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

فراغ virtio_mmio_تیاری_acpi(فراغ)
{
    ساكن ثابت حرف *virtio_hid = "LNRO0005";
    بنية acpi_device *dev;
    u64 mem, irq, ناکارول_شوی;

    لكل (dev = acpi_dsdt_پیدا_کړه_string(عدم, virtio_hid);
         dev != عدم;
         dev = acpi_dsdt_پیدا_کړه_string(dev, virtio_hid)) {
        إذا (acpi_dsdt_پیدا_کړه_mem(dev, &mem, &ناکارول_شوی) < 0)
            استمر;
        إذا (acpi_dsdt_پیدا_کړه_irq(dev, &irq) < 0)
            استمر;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_تیاری_one(mem);
    }
}

فراغ virtio_mmio_تیاری_one(u64 addr)
{
    ساكن ثابت حرف *نومونه[] = {
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
    u32 magic, نسخه_2, devid;
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
    نسخه_2 = readl(mmio+4);
    إذا (نسخه_2 != 1 /* legacy */ &&
        نسخه_2 != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, نسخه_2);
        أرجع;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_اندازه(نومونه) && نومونه[devid] != عدم)
        ? نومونه[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, نسخه_2 == 1 ? ", legacy" : "");

    بدل (devid) {
    حالة 2: /* blk */
        پرلپسې_thread(init_virtio_blk_mmio, mmio);
        اكسر;
    حالة 8: /* scsi */
        پرلپسې_thread(init_virtio_scsi_mmio, mmio);
        اكسر;
    افتراضي:
        اكسر;
    }
}

فراغ vp_init_mmio(بنية vp_device *vp, فراغ *mmio)
{
    memset(vp, 0, حجم(*vp));
    vp->use_mmio = 1;
    vp->common.ډېرپېښ = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.ډېرپېښ = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_سېټ_حالت(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
