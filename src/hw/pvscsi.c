// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "blockcmd.h" // scsi_drive_setup
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "memmap.h" // PAGE_SHIFT, virt_to_phys
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#অন্তর্ভাব "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "pvscsi.h" // pvscsi_setup
#অন্তর্ভাব "stacks.h" // run_thread
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // usleep
#অন্তর্ভাব "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_বার্তা_0                  (1 << 2)
#define PVSCSI_INTR_বার্তা_1                  (1 << 3)
#define PVSCSI_INTR_বার্তা_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

গণনা PVSCSIRegOffset {
    PVSCSI_REG_OFFSET_COMMAND        =    0x0,
    PVSCSI_REG_OFFSET_COMMAND_তথ্য   =    0x4,
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
    PVSCSI_CMD_SETUP_বার্তা_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_অঙ্ক_PAGES        32
কাঠামো PVSCSICmdDescSetupRings {
    u32    req_Ring_অঙ্ক_Pages;
    u32    cmp_Ring_অঙ্ক_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_অঙ্ক_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_অঙ্ক_PAGES];
} PACKED;

কাঠামো PVSCSIRingCmpDesc {
    u64    context;
    u64    তথ্য_Len;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

কাঠামো PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_অঙ্ক_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_অঙ্ক_Entries_Log2;

    u8     pad[104];

    u32    বার্তা_Prod_Idx;
    u32    বার্তা_Cons_Idx;
    u32    বার্তা_অঙ্ক_Entries_Log2;
} PACKED;

কাঠামো PVSCSIRingReqDesc {
    u64    context;
    u64    তথ্য_Addr;
    u64    তথ্য_Len;
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

কাঠামো pvscsi_ring_dsc_s {
    কাঠামো PVSCSIRingsState *ring_state;
    কাঠামো PVSCSIRingReqDesc *ring_reqs;
    কাঠামো PVSCSIRingCmpDesc *ring_cmps;
};

কাঠামো pvscsi_lun_s {
    কাঠামো drive_s drive;
    শূন্য *iobase;
    u8 target;
    u8 lun;
    কাঠামো pvscsi_ring_dsc_s *ring_dsc;
};

স্থির শূন্য
pvscsi_write_cmd_desc(শূন্য *iobase, u32 cmd, ধ্রুবক শূন্য *desc, আকার_t len)
{
    ধ্রুবক u32 *ptr = desc;
    আকার_t i;

    len /= আকার(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    জন্য (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_তথ্য, ptr[i]);
}

স্থির শূন্য
pvscsi_kick_rw_io(শূন্য *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

স্থির শূন্য
pvscsi_wait_intr_cmpl(শূন্য *iobase)
{
    যখন (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

স্থির শূন্য
pvscsi_init_rings(শূন্য *iobase, কাঠামো pvscsi_ring_dsc_s **ring_dsc)
{
    কাঠামো PVSCSICmdDescSetupRings cmd = {0,};

    কাঠামো pvscsi_ring_dsc_s *dsc = malloc_উপরের(আকার(*dsc));
    যদি (!dsc) {
        warn_noalloc();
        ফেরত;
    }

    dsc->ring_state =
        (কাঠামো PVSCSIRingsState *)memalign_উপরের(PAGE_আকার, PAGE_আকার);
    dsc->ring_reqs =
        (কাঠামো PVSCSIRingReqDesc *)memalign_উপরের(PAGE_আকার, PAGE_আকার);
    dsc->ring_cmps =
        (কাঠামো PVSCSIRingCmpDesc *)memalign_উপরের(PAGE_আকার, PAGE_আকার);
    যদি (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        ফেরত;
    }
    memset(dsc->ring_state, 0, PAGE_আকার);
    memset(dsc->ring_reqs, 0, PAGE_আকার);
    memset(dsc->ring_cmps, 0, PAGE_আকার);

    cmd.req_Ring_অঙ্ক_Pages = 1;
    cmd.cmp_Ring_অঙ্ক_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, আকার(cmd));
    *ring_dsc = dsc;
}

স্থির u32
pvscsi_get_rsp(কাঠামো PVSCSIRingsState *s,
               কাঠামো PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    ফেরত status;
}

পূর্ণসংখ্যা
pvscsi_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_PVSCSI)
        ফেরত DISK_RET_EBADTRACK;
    কাঠামো pvscsi_lun_s *plun =
        container_of(op->drive_fl, কাঠামো pvscsi_lun_s, drive);
    কাঠামো pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    কাঠামো PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_অঙ্ক_Entries_Log2;
    u32 cmp_entries = s->cmp_অঙ্ক_Entries_Log2;
    কাঠামো PVSCSIRingReqDesc *req;
    কাঠামো PVSCSIRingCmpDesc *rsp;
    u32 status;

    যদি (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        ফেরত DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, req->cdb, 16);
    যদি (blocksize < 0)
        ফেরত default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, আকার(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->তথ্য_Len = op->গুনতি * blocksize;
    req->তথ্য_Addr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    ফেরত status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

স্থির পূর্ণসংখ্যা
pvscsi_add_lun(কাঠামো pci_device *pci, শূন্য *iobase,
               কাঠামো pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    কাঠামো pvscsi_lun_s *plun = malloc_fseg(আকার(*plun));
    যদি (!plun) {
        warn_noalloc();
        ফেরত -1;
    }
    memset(plun, 0, আকার(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    অক্ষর *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    পূর্ণসংখ্যা prio = bootprio_find_scsi_device(pci, target, lun);
    পূর্ণসংখ্যা ret = scsi_drive_setup(&plun->drive, name, prio);
    মুক্ত(name);
    যদি (ret)
        যাও fail;
    ফেরত 0;

fail:
    মুক্ত(plun);
    ফেরত -1;
}

স্থির শূন্য
pvscsi_scan_target(কাঠামো pci_device *pci, শূন্য *iobase,
                   কাঠামো pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

স্থির শূন্য
init_pvscsi(শূন্য *তথ্য)
{
    কাঠামো pci_device *pci = তথ্য;
    শূন্য *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!iobase)
        ফেরত;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, ফাঁকা, 0);

    কাঠামো pvscsi_ring_dsc_s *ring_dsc = ফাঁকা;
    pvscsi_init_rings(iobase, &ring_dsc);
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

শূন্য
pvscsi_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (! CONFIG_PVSCSI)
        ফেরত;

    dprintf(3, "init pvscsi\n");

    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            চালু;
        run_thread(init_pvscsi, pci);
    }
}
