#अन्तर्भाव "config.h" // CONFIG_DEBUG_LEVEL
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "stacks.h" // run_thread
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // acpi_dsdt_*
#अन्तर्भाव "virtio-pci.h"
#अन्तर्भाव "virtio-blk.h"
#अन्तर्भाव "virtio-scsi.h"
#अन्तर्भाव "virtio-ring.h"
#अन्तर्भाव "virtio-mmio.h"

शून्य virtio_mmio_setup_acpi(शून्य)
{
    स्थिर स्थिरांक अक्षर *virtio_hid = "LNRO0005";
    संरचना acpi_device *dev;
    u64 mem, irq, unused;

    साठी (dev = acpi_dsdt_find_string(रिक्त, virtio_hid);
         dev != रिक्त;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        जर (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            सुरूठेवा;
        जर (acpi_dsdt_find_irq(dev, &irq) < 0)
            सुरूठेवा;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

शून्य virtio_mmio_setup_one(u64 addr)
{
    स्थिर स्थिरांक अक्षर *names[] = {
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
    स्थिरांक अक्षर *name;
    u32 magic, version, devid;
    शून्य *mmio;

    जर (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        परत;
    }

    mmio = (शून्य*)(u32)(addr);
    magic = readl(mmio);
    जर (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        परत;
    }
    version = readl(mmio+4);
    जर (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        परत;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != रिक्त)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    निवड (devid) {
    स्थिती 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        थांबा;
    स्थिती 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        थांबा;
    पूर्वनिर्धारित:
        थांबा;
    }
}

शून्य vp_init_mmio(संरचना vp_device *vp, शून्य *mmio)
{
    स_रखो(vp, 0, आकार(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
