#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_DEBUG_LEVEL
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "stacks.h" // run_thread
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // acpi_dsdt_*
#ଅନ୍ତର୍ଭାଵ "virtio-pci.h"
#ଅନ୍ତର୍ଭାଵ "virtio-blk.h"
#ଅନ୍ତର୍ଭାଵ "virtio-scsi.h"
#ଅନ୍ତର୍ଭାଵ "virtio-ring.h"
#ଅନ୍ତର୍ଭାଵ "virtio-mmio.h"

ଶୂନ୍ୟ virtio_mmio_setup_acpi(ଶୂନ୍ୟ)
{
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *virtio_hid = "LNRO0005";
    ସଂରଚନା acpi_device *dev;
    u64 mem, irq, unused;

    ପାଇଁ (dev = acpi_dsdt_find_string(ଖାଲି, virtio_hid);
         dev != ଖାଲି;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        ଯଦି (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            ଜାରି;
        ଯଦି (acpi_dsdt_find_irq(dev, &irq) < 0)
            ଜାରି;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

ଶୂନ୍ୟ virtio_mmio_setup_one(u64 addr)
{
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *names[] = {
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
    ଅଚଳ ଅକ୍ଷର *name;
    u32 magic, version, devid;
    ଶୂନ୍ୟ *mmio;

    ଯଦି (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        ଫେରନ୍ତୁ;
    }

    mmio = (ଶୂନ୍ୟ*)(u32)(addr);
    magic = readl(mmio);
    ଯଦି (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        ଫେରନ୍ତୁ;
    }
    version = readl(mmio+4);
    ଯଦି (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        ଫେରନ୍ତୁ;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != ଖାଲି)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    ବାଛ (devid) {
    ସ୍ଥିତି 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        ଭାଙ୍ଗ;
    ସ୍ଥିତି 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଭାଙ୍ଗ;
    }
}

ଶୂନ୍ୟ vp_init_mmio(ସଂରଚନା vp_device *vp, ଶୂନ୍ୟ *mmio)
{
    memset(vp, 0, ଆକାର(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
