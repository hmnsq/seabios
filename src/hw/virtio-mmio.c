#অন্তর্ভাৱ "config.h" // CONFIG_DEBUG_LEVEL
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "stacks.h" // run_thread
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // acpi_dsdt_*
#অন্তর্ভাৱ "virtio-pci.h"
#অন্তর্ভাৱ "virtio-blk.h"
#অন্তর্ভাৱ "virtio-scsi.h"
#অন্তর্ভাৱ "virtio-ring.h"
#অন্তর্ভাৱ "virtio-mmio.h"

শূন্য virtio_mmio_setup_acpi(শূন্য)
{
    স্থির ধ্রুৱ ৱর্ণ *virtio_hid = "LNRO0005";
    সংরচনা acpi_device *dev;
    u64 mem, irq, unused;

    বাবে (dev = acpi_dsdt_find_string(রিক্ত, virtio_hid);
         dev != রিক্ত;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        যদি (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            অনুৱর্ত;
        যদি (acpi_dsdt_find_irq(dev, &irq) < 0)
            অনুৱর্ত;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

শূন্য virtio_mmio_setup_one(u64 addr)
{
    স্থির ধ্রুৱ ৱর্ণ *names[] = {
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
    ধ্রুৱ ৱর্ণ *name;
    u32 magic, version, devid;
    শূন্য *mmio;

    যদি (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        ঘূৰাই;
    }

    mmio = (শূন্য*)(u32)(addr);
    magic = readl(mmio);
    যদি (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        ঘূৰাই;
    }
    version = readl(mmio+4);
    যদি (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        ঘূৰাই;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != রিক্ত)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    ৱিকল্প (devid) {
    পক্ষ 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        ভঙ্গ;
    পক্ষ 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        ভঙ্গ;
    স্ৱভাৱ:
        ভঙ্গ;
    }
}

শূন্য vp_init_mmio(সংরচনা vp_device *vp, শূন্য *mmio)
{
    memset(vp, 0, পরিমাণ(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
