#అన్తర్భావ "config.h" // CONFIG_DEBUG_LEVEL
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "stacks.h" // run_thread
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // acpi_dsdt_*
#అన్తర్భావ "virtio-pci.h"
#అన్తర్భావ "virtio-blk.h"
#అన్తర్భావ "virtio-scsi.h"
#అన్తర్భావ "virtio-ring.h"
#అన్తర్భావ "virtio-mmio.h"

శూన్య virtio_mmio_అమరిక_acpi(శూన్య)
{
    స్థిర స్థిరరాశి అక్షరం *virtio_hid = "LNRO0005";
    నిర్మాణం acpi_device *dev;
    u64 mem, irq, వాడని;

    కొరకు (dev = acpi_dsdt_కనుగొను_string(ఖాళీ, virtio_hid);
         dev != ఖాళీ;
         dev = acpi_dsdt_కనుగొను_string(dev, virtio_hid)) {
        ఒకవేళ (acpi_dsdt_కనుగొను_mem(dev, &mem, &వాడని) < 0)
            కొనసాగు;
        ఒకవేళ (acpi_dsdt_కనుగొను_irq(dev, &irq) < 0)
            కొనసాగు;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_అమరిక_one(mem);
    }
}

శూన్య virtio_mmio_అమరిక_one(u64 addr)
{
    స్థిర స్థిరరాశి అక్షరం *పేర్లు[] = {
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
    స్థిరరాశి అక్షరం *name;
    u32 magic, సంస్కరణ_2, devid;
    శూన్య *mmio;

    ఒకవేళ (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        తిరిగి;
    }

    mmio = (శూన్య*)(u32)(addr);
    magic = readl(mmio);
    ఒకవేళ (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        తిరిగి;
    }
    సంస్కరణ_2 = readl(mmio+4);
    ఒకవేళ (సంస్కరణ_2 != 1 /* legacy */ &&
        సంస్కరణ_2 != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, సంస్కరణ_2);
        తిరిగి;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_కొలత(పేర్లు) && పేర్లు[devid] != ఖాళీ)
        ? పేర్లు[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, సంస్కరణ_2 == 1 ? ", legacy" : "");

    ఎంపిక (devid) {
    స్థితి 2: /* blk */
        నిరంతరం_thread(init_virtio_blk_mmio, mmio);
        ఆపు;
    స్థితి 8: /* scsi */
        నిరంతరం_thread(init_virtio_scsi_mmio, mmio);
        ఆపు;
    అప్రమేయ:
        ఆపు;
    }
}

శూన్య vp_init_mmio(నిర్మాణం vp_device *vp, శూన్య *mmio)
{
    memset(vp, 0, పరిమాణం(*vp));
    vp->use_mmio = 1;
    vp->common.బహుళకం = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.బహుళకం = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_సమితి_స్థితి(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
