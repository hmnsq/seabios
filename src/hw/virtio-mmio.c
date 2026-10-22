#અન્તર્ભાવ "config.h" // CONFIG_DEBUG_LEVEL
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "stacks.h" // run_thread
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // acpi_dsdt_*
#અન્તર્ભાવ "virtio-pci.h"
#અન્તર્ભાવ "virtio-blk.h"
#અન્તર્ભાવ "virtio-scsi.h"
#અન્તર્ભાવ "virtio-ring.h"
#અન્તર્ભાવ "virtio-mmio.h"

શૂન્ય virtio_mmio_setup_acpi(શૂન્ય)
{
    સ્થિર અચલ અક્ષર *virtio_hid = "LNRO0005";
    સંરચના acpi_device *dev;
    u64 mem, irq, unused;

    માટે (dev = acpi_dsdt_find_string(ખાલી, virtio_hid);
         dev != ખાલી;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        જો (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            ચાલુ;
        જો (acpi_dsdt_find_irq(dev, &irq) < 0)
            ચાલુ;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

શૂન્ય virtio_mmio_setup_one(u64 addr)
{
    સ્થિર અચલ અક્ષર *names[] = {
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
    અચલ અક્ષર *name;
    u32 magic, version, devid;
    શૂન્ય *mmio;

    જો (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        પાછા;
    }

    mmio = (શૂન્ય*)(u32)(addr);
    magic = readl(mmio);
    જો (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        પાછા;
    }
    version = readl(mmio+4);
    જો (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        પાછા;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != ખાલી)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    પસંદગી (devid) {
    સ્થિતિ 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        તોડો;
    સ્થિતિ 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        તોડો;
    મૂળભૂત:
        તોડો;
    }
}

શૂન્ય vp_init_mmio(સંરચના vp_device *vp, શૂન્ય *mmio)
{
    memset(vp, 0, કદ(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
