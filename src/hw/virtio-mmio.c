#அந்தர்பாவ "config.h" // CONFIG_DEBUG_LEVEL
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "stacks.h" // run_thread
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // acpi_dsdt_*
#அந்தர்பாவ "virtio-pci.h"
#அந்தர்பாவ "virtio-blk.h"
#அந்தர்பாவ "virtio-scsi.h"
#அந்தர்பாவ "virtio-ring.h"
#அந்தர்பாவ "virtio-mmio.h"

வெறுமை virtio_mmio_setup_acpi(வெறுமை)
{
    நிலையான மாறிலி எழுத்து *virtio_hid = "LNRO0005";
    கட்டமைப்பு acpi_device *dev;
    u64 mem, irq, unused;

    க்கு (dev = acpi_dsdt_find_string(வெற்று, virtio_hid);
         dev != வெற்று;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        எனில் (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            தொடர்;
        எனில் (acpi_dsdt_find_irq(dev, &irq) < 0)
            தொடர்;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

வெறுமை virtio_mmio_setup_one(u64 addr)
{
    நிலையான மாறிலி எழுத்து *names[] = {
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
    மாறிலி எழுத்து *name;
    u32 magic, version, devid;
    வெறுமை *mmio;

    எனில் (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        திரும்பு;
    }

    mmio = (வெறுமை*)(u32)(addr);
    magic = readl(mmio);
    எனில் (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        திரும்பு;
    }
    version = readl(mmio+4);
    எனில் (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        திரும்பு;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != வெற்று)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    தேர்வு (devid) {
    நிலை 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        நிறுத்து;
    நிலை 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        நிறுத்து;
    இயல்பு:
        நிறுத்து;
    }
}

வெறுமை vp_init_mmio(கட்டமைப்பு vp_device *vp, வெறுமை *mmio)
{
    memset(vp, 0, அளவு(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
