// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "block.h" // struct drive_s
#অন্তর্ভাৱ "blockcmd.h" // scsi_drive_setup
#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "memmap.h" // PAGE_SHIFT, virt_to_phys
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "pcidevice.h" // foreachpci
#অন্তর্ভাৱ "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#অন্তর্ভাৱ "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাৱ "pvscsi.h" // pvscsi_setup
#অন্তর্ভাৱ "stacks.h" // run_thread
#অন্তর্ভাৱ "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // usleep
#অন্তর্ভাৱ "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_বাৰ্তা_0                  (1 << 2)
#define PVSCSI_INTR_বাৰ্তা_1                  (1 << 3)
#define PVSCSI_INTR_বাৰ্তা_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

গণনা PVSCSIRegOffset {
    PVSCSI_REG_OFFSET_COMMAND        =    0x0,
    PVSCSI_REG_OFFSET_COMMAND_DATA   =    0x4,
    PVSCSI_REG_OFFSET_COMMAND_STATUS =    0x8,
    PVSCSI_REG_OFFSET_LAST_STS_0     =  0x100,
    PVSCSI_REG_OFFSET_LAST_STS_1     =  0x104,
    PVSCSI_REG_OFFSET_LAST_STS_2     =  0x108,
    PVSCSI_REG_OFFSET_LAST_STS_3     =  0x10c,
    PVSCSI_REG_OFFSET_INTR_STATUS    = 0x100c,
    PVSCSI_REG_OFFSET_INTR_MASK      = 0x2010,
    PVSCSI_REG_OFFSET_KICK_NON_RW_IO = 0x3014,
    PVSCSI_REG_OFFSET_DEBUG          = 0x3018,
    PVSCSI_REG_OFFSET_KICK_RW_IO     = 0x4018,
};

গণনা PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_বাৰ্তা_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_অঙ্ক_PAGES        32
সংরচনা PVSCSICmdDescSetupRings {
    u32    req_Ring_অঙ্ক_Pages;
    u32    cmp_Ring_অঙ্ক_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_অঙ্ক_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_অঙ্ক_PAGES];
} PACKED;

সংরচনা PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

সংরচনা PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_অঙ্ক_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_অঙ্ক_Entries_Log2;

    u8     pad[104];

    u32    বাৰ্তা_Prod_Idx;
    u32    বাৰ্তা_Cons_Idx;
    u32    বাৰ্তা_অঙ্ক_Entries_Log2;
} PACKED;

সংরচনা PVSCSIRingReqDesc {
    u64    context;
    u64    dataAddr;
    u64    dataLen;
    u64    senseAddr;
    u32    senseLen;
    u32    flags;
    u8     cdb[16];
    u8     cdbLen;
    u8     lun[8];
    u8     tag;
    u8     bus;
    u8     target;
    u8     vcpuHint;
    u8     unused[59];
} PACKED;

সংরচনা pvscsi_ring_dsc_s {
    সংরচনা PVSCSIRingsState *ring_state;
    সংরচনা PVSCSIRingReqDesc *ring_reqs;
    সংরচনা PVSCSIRingCmpDesc *ring_cmps;
};

সংরচনা pvscsi_lun_s {
    সংরচনা drive_s drive;
    শূন্য *iobase;
    u8 target;
    u8 lun;
    সংরচনা pvscsi_ring_dsc_s *ring_dsc;
};

স্থির শূন্য
pvscsi_write_cmd_desc(শূন্য *iobase, u32 cmd, ধ্রুৱ শূন্য *desc, size_t len)
{
    ধ্রুৱ u32 *ptr = desc;
    size_t i;

    len /= পরিমাণ(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    বাবে (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

স্থির শূন্য
pvscsi_kick_rw_io(শূন্য *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

স্থির শূন্য
pvscsi_wait_intr_cmpl(শূন্য *iobase)
{
    যেতিয়ালৈ (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

স্থির শূন্য
pvscsi_init_rings(শূন্য *iobase, সংরচনা pvscsi_ring_dsc_s **ring_dsc)
{
    সংরচনা PVSCSICmdDescSetupRings cmd = {0,};

    সংরচনা pvscsi_ring_dsc_s *dsc = malloc_উপৰেৰ(পরিমাণ(*dsc));
    যদি (!dsc) {
        warn_noalloc();
        ঘূৰাই;
    }

    dsc->ring_state =
        (সংরচনা PVSCSIRingsState *)memalign_উপৰেৰ(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (সংরচনা PVSCSIRingReqDesc *)memalign_উপৰেৰ(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (সংরচনা PVSCSIRingCmpDesc *)memalign_উপৰেৰ(PAGE_SIZE, PAGE_SIZE);
    যদি (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        ঘূৰাই;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_অঙ্ক_Pages = 1;
    cmd.cmp_Ring_অঙ্ক_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, পরিমাণ(cmd));
    *ring_dsc = dsc;
}

স্থির u32
pvscsi_get_rsp(সংরচনা PVSCSIRingsState *s,
               সংরচনা PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    ঘূৰাই status;
}

পূর্ণাঙ্ক
pvscsi_process_op(সংরচনা disk_op_s *op)
{
    যদি (!CONFIG_PVSCSI)
        ঘূৰাই DISK_RET_EBADTRACK;
    সংরচনা pvscsi_lun_s *plun =
        container_of(op->drive_fl, সংরচনা pvscsi_lun_s, drive);
    সংরচনা pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    সংরচনা PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_অঙ্ক_Entries_Log2;
    u32 cmp_entries = s->cmp_অঙ্ক_Entries_Log2;
    সংরচনা PVSCSIRingReqDesc *req;
    সংরচনা PVSCSIRingCmpDesc *rsp;
    u32 status;

    যদি (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        ঘূৰাই DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    পূর্ণাঙ্ক blocksize = scsi_fill_cmd(op, req->cdb, 16);
    যদি (blocksize < 0)
        ঘূৰাই default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, পরিমাণ(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->গুনতি * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    ঘূৰাই status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

স্থির পূর্ণাঙ্ক
pvscsi_add_lun(সংরচনা pci_device *pci, শূন্য *iobase,
               সংরচনা pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    সংরচনা pvscsi_lun_s *plun = malloc_fseg(পরিমাণ(*plun));
    যদি (!plun) {
        warn_noalloc();
        ঘূৰাই -1;
    }
    memset(plun, 0, পরিমাণ(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    ৱর্ণ *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    পূর্ণাঙ্ক prio = bootprio_find_scsi_device(pci, target, lun);
    পূর্ণাঙ্ক ret = scsi_drive_setup(&plun->drive, name, prio);
    মুক্ত(name);
    যদি (ret)
        গচ্ছ fail;
    ঘূৰাই 0;

fail:
    মুক্ত(plun);
    ঘূৰাই -1;
}

স্থির শূন্য
pvscsi_scan_target(সংরচনা pci_device *pci, শূন্য *iobase,
                   সংরচনা pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

স্থির শূন্য
init_pvscsi(শূন্য *data)
{
    সংরচনা pci_device *pci = data;
    শূন্য *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!iobase)
        ঘূৰাই;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, রিক্ত, 0);

    সংরচনা pvscsi_ring_dsc_s *ring_dsc = রিক্ত;
    pvscsi_init_rings(iobase, &ring_dsc);
    পূর্ণাঙ্ক i;
    বাবে (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

শূন্য
pvscsi_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (! CONFIG_PVSCSI)
        ঘূৰাই;

    dprintf(3, "init pvscsi\n");

    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            অনুৱর্ত;
        run_thread(init_pvscsi, pci);
    }
}
