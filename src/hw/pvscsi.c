// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // scsi_drive_setup
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "memmap.h" // PAGE_SHIFT, virt_to_phys
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "pvscsi.h" // pvscsi_setup
#شامل "stacks.h" // run_thread
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // usleep
#شامل "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_پیام_0                  (1 << 2)
#define PVSCSI_INTR_پیام_1                  (1 << 3)
#define PVSCSI_INTR_پیام_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

شمارش PVSCSIRegOffset {
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

شمارش PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_پیام_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_رقم_PAGES        32
ساختار PVSCSICmdDescSetupRings {
    u32    req_Ring_رقم_Pages;
    u32    cmp_Ring_رقم_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_رقم_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_رقم_PAGES];
} PACKED;

ساختار PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

ساختار PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_رقم_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_رقم_Entries_Log2;

    u8     pad[104];

    u32    پیام_Prod_Idx;
    u32    پیام_Cons_Idx;
    u32    پیام_رقم_Entries_Log2;
} PACKED;

ساختار PVSCSIRingReqDesc {
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

ساختار pvscsi_ring_dsc_s {
    ساختار PVSCSIRingsState *ring_state;
    ساختار PVSCSIRingReqDesc *ring_reqs;
    ساختار PVSCSIRingCmpDesc *ring_cmps;
};

ساختار pvscsi_lun_s {
    ساختار drive_s drive;
    تهی *iobase;
    u8 target;
    u8 lun;
    ساختار pvscsi_ring_dsc_s *ring_dsc;
};

ایستا تهی
pvscsi_write_cmd_desc(تهی *iobase, u32 cmd, ثابت تهی *desc, size_t len)
{
    ثابت u32 *ptr = desc;
    size_t i;

    len /= اندازه(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    برای (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

ایستا تهی
pvscsi_kick_rw_io(تهی *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

ایستا تهی
pvscsi_wait_intr_cmpl(تهی *iobase)
{
    وقتی (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

ایستا تهی
pvscsi_init_rings(تهی *iobase, ساختار pvscsi_ring_dsc_s **ring_dsc)
{
    ساختار PVSCSICmdDescSetupRings cmd = {0,};

    ساختار pvscsi_ring_dsc_s *dsc = malloc_بالا(اندازه(*dsc));
    اگر (!dsc) {
        warn_noalloc();
        بازگشت;
    }

    dsc->ring_state =
        (ساختار PVSCSIRingsState *)memalign_بالا(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (ساختار PVSCSIRingReqDesc *)memalign_بالا(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (ساختار PVSCSIRingCmpDesc *)memalign_بالا(PAGE_SIZE, PAGE_SIZE);
    اگر (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        بازگشت;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_رقم_Pages = 1;
    cmd.cmp_Ring_رقم_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, اندازه(cmd));
    *ring_dsc = dsc;
}

ایستا u32
pvscsi_get_rsp(ساختار PVSCSIRingsState *s,
               ساختار PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    بازگشت status;
}

صحیح
pvscsi_process_op(ساختار disk_op_s *op)
{
    اگر (!CONFIG_PVSCSI)
        بازگشت DISK_RET_EBADTRACK;
    ساختار pvscsi_lun_s *plun =
        container_of(op->drive_fl, ساختار pvscsi_lun_s, drive);
    ساختار pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    ساختار PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_رقم_Entries_Log2;
    u32 cmp_entries = s->cmp_رقم_Entries_Log2;
    ساختار PVSCSIRingReqDesc *req;
    ساختار PVSCSIRingCmpDesc *rsp;
    u32 status;

    اگر (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        بازگشت DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    صحیح blocksize = scsi_fill_cmd(op, req->cdb, 16);
    اگر (blocksize < 0)
        بازگشت default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, اندازه(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->شمار * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    بازگشت status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ایستا صحیح
pvscsi_add_lun(ساختار pci_device *pci, تهی *iobase,
               ساختار pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    ساختار pvscsi_lun_s *plun = malloc_fseg(اندازه(*plun));
    اگر (!plun) {
        warn_noalloc();
        بازگشت -1;
    }
    memset(plun, 0, اندازه(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    نویسه *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    صحیح prio = bootprio_find_scsi_device(pci, target, lun);
    صحیح ret = scsi_drive_setup(&plun->drive, name, prio);
    آزاد(name);
    اگر (ret)
        برو fail;
    بازگشت 0;

fail:
    آزاد(plun);
    بازگشت -1;
}

ایستا تهی
pvscsi_scan_target(ساختار pci_device *pci, تهی *iobase,
                   ساختار pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

ایستا تهی
init_pvscsi(تهی *data)
{
    ساختار pci_device *pci = data;
    تهی *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    اگر (!iobase)
        بازگشت;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, پوچ, 0);

    ساختار pvscsi_ring_dsc_s *ring_dsc = پوچ;
    pvscsi_init_rings(iobase, &ring_dsc);
    صحیح i;
    برای (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

تهی
pvscsi_setup(تهی)
{
    ASSERT32FLAT();
    اگر (! CONFIG_PVSCSI)
        بازگشت;

    dprintf(3, "init pvscsi\n");

    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            ادامه;
        run_thread(init_pvscsi, pci);
    }
}
