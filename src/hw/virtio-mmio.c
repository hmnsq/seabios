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

خالی virtio_mmio_تیاری_acpi(خالی)
{
    ساکن ثابت حرف *virtio_hid = "LNRO0005";
    ساخت acpi_device *dev;
    u64 mem, irq, غیر_مستعمل;

    برائے (dev = acpi_dsdt_ڈھونڈو_string(تہی, virtio_hid);
         dev != تہی;
         dev = acpi_dsdt_ڈھونڈو_string(dev, virtio_hid)) {
        اگر (acpi_dsdt_ڈھونڈو_mem(dev, &mem, &غیر_مستعمل) < 0)
            جاری;
        اگر (acpi_dsdt_ڈھونڈو_irq(dev, &irq) < 0)
            جاری;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_تیاری_one(mem);
    }
}

خالی virtio_mmio_تیاری_one(u64 addr)
{
    ساکن ثابت حرف *نام[] = {
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
    u32 magic, نسخہ_2, devid;
    خالی *mmio;

    اگر (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        واپس;
    }

    mmio = (خالی*)(u32)(addr);
    magic = readl(mmio);
    اگر (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        واپس;
    }
    نسخہ_2 = readl(mmio+4);
    اگر (نسخہ_2 != 1 /* legacy */ &&
        نسخہ_2 != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, نسخہ_2);
        واپس;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_سائز(نام) && نام[devid] != تہی)
        ? نام[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, نسخہ_2 == 1 ? ", legacy" : "");

    انتخاب (devid) {
    حالت 2: /* blk */
        تسلسل_thread(init_virtio_blk_mmio, mmio);
        توڑو;
    حالت 8: /* scsi */
        تسلسل_thread(init_virtio_scsi_mmio, mmio);
        توڑو;
    طےشدہ:
        توڑو;
    }
}

خالی vp_init_mmio(ساخت vp_device *vp, خالی *mmio)
{
    memset(vp, 0, حجم(*vp));
    vp->use_mmio = 1;
    vp->common.منوال = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.منوال = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_سیٹ_حالت(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
