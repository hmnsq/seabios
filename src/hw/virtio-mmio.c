#समावेश "config.h" // CONFIG_DEBUG_LEVEL
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "stacks.h" // run_thread
#समावेश "string.h" // memset
#समावेश "util.h" // acpi_dsdt_*
#समावेश "virtio-pci.h"
#समावेश "virtio-blk.h"
#समावेश "virtio-scsi.h"
#समावेश "virtio-ring.h"
#समावेश "virtio-mmio.h"

व्योम virtio_mmio_setup_acpi(व्योम)
{
    अटल स्थिर अक्षर *virtio_hid = "LNRO0005";
    काष्ठा acpi_device *dev;
    u64 mem, irq, unused;

    क्रम (dev = acpi_dsdt_find_string(शून्य, virtio_hid);
         dev != शून्य;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        अगर (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            जारी;
        अगर (acpi_dsdt_find_irq(dev, &irq) < 0)
            जारी;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

व्योम virtio_mmio_setup_one(u64 addr)
{
    अटल स्थिर अक्षर *names[] = {
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
    स्थिर अक्षर *name;
    u32 magic, version, devid;
    व्योम *mmio;

    अगर (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        वापस;
    }

    mmio = (व्योम*)(u32)(addr);
    magic = readl(mmio);
    अगर (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        वापस;
    }
    version = readl(mmio+4);
    अगर (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        वापस;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_आकार(names) && names[devid] != शून्य)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    चयन (devid) {
    हाल 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        अवरोध;
    हाल 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        अवरोध;
    शेष:
        अवरोध;
    }
}

व्योम vp_init_mmio(काष्ठा vp_device *vp, व्योम *mmio)
{
    स_रखो(vp, 0, माप(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
