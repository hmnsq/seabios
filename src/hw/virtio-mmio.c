#അന്തര്ഭാവ "config.h" // CONFIG_DEBUG_LEVEL
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "stacks.h" // run_thread
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // acpi_dsdt_*
#അന്തര്ഭാവ "virtio-pci.h"
#അന്തര്ഭാവ "virtio-blk.h"
#അന്തര്ഭാവ "virtio-scsi.h"
#അന്തര്ഭാവ "virtio-ring.h"
#അന്തര്ഭാവ "virtio-mmio.h"

ശൂന്യം virtio_mmio_setup_acpi(ശൂന്യം)
{
    സ്ഥിതം സ്ഥിരം അക്ഷരം *virtio_hid = "LNRO0005";
    ഘടന acpi_device *dev;
    u64 mem, irq, unused;

    വേണ്ടി (dev = acpi_dsdt_find_string(ശൂന്യ, virtio_hid);
         dev != ശൂന്യ;
         dev = acpi_dsdt_find_string(dev, virtio_hid)) {
        എങ്കിൽ (acpi_dsdt_find_mem(dev, &mem, &unused) < 0)
            തുടരുക;
        എങ്കിൽ (acpi_dsdt_find_irq(dev, &irq) < 0)
            തുടരുക;
        dprintf(1, "ACPI: virtio-mmio device %s at 0x%llx, irq %lld\n",
                acpi_dsdt_name(dev), mem, irq);
        virtio_mmio_setup_one(mem);
    }
}

ശൂന്യം virtio_mmio_setup_one(u64 addr)
{
    സ്ഥിതം സ്ഥിരം അക്ഷരം *names[] = {
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
    സ്ഥിരം അക്ഷരം *name;
    u32 magic, version, devid;
    ശൂന്യം *mmio;

    എങ്കിൽ (addr >= 0x100000000) {
        dprintf(1, "virtio-mmio: %llx: above 4G\n", addr);
        തിരികെ;
    }

    mmio = (ശൂന്യം*)(u32)(addr);
    magic = readl(mmio);
    എങ്കിൽ (magic != 0x74726976) {
        dprintf(1, "virtio-mmio: %llx: magic mismatch\n", addr);
        തിരികെ;
    }
    version = readl(mmio+4);
    എങ്കിൽ (version != 1 /* legacy */ &&
        version != 2 /* 1.0 */) {
        dprintf(1, "virtio-mmio: %llx: unknown version %d\n", addr, version);
        തിരികെ;
    }
    devid = readl(mmio+8);

    name = (devid < ARRAY_SIZE(names) && names[devid] != ശൂന്യ)
        ? names[devid] : "unknown";
    dprintf(1, "virtio-mmio: %llx: device id %x (%s%s)\n",
            addr, devid, name, version == 1 ? ", legacy" : "");

    തിരഞ്ഞെടുപ്പ് (devid) {
    അവസ്ഥ 2: /* blk */
        run_thread(init_virtio_blk_mmio, mmio);
        നിർത്തുക;
    അവസ്ഥ 8: /* scsi */
        run_thread(init_virtio_scsi_mmio, mmio);
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        നിർത്തുക;
    }
}

ശൂന്യം vp_init_mmio(ഘടന vp_device *vp, ശൂന്യം *mmio)
{
    memset(vp, 0, വലുപ്പം(*vp));
    vp->use_mmio = 1;
    vp->common.mode = VP_ACCESS_MMIO;
    vp->common.memaddr = mmio;
    vp->device.mode = VP_ACCESS_MMIO;
    vp->device.memaddr = mmio + 0x100;
    vp_reset(vp);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER);
}
