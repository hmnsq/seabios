// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "block.h" // struct drive_s
#ضمن "blockcmd.h" // scsi_drive_setup
#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "memmap.h" // PAGE_SHIFT, virt_to_phys
#ضمن "output.h" // dprintf
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#ضمن "pci_regs.h" // PCI_VENDOR_ID
#ضمن "pvscsi.h" // pvscsi_setup
#ضمن "stacks.h" // run_thread
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // usleep
#ضمن "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_پیغام_0                  (1 << 2)
#define PVSCSI_INTR_پیغام_1                  (1 << 3)
#define PVSCSI_INTR_پیغام_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

تعداد PVSCSIRegOffset {
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

تعداد PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_پیغام_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_عدد_PAGES        32
بنية PVSCSICmdDescSetupRings {
    u32    req_Ring_عدد_Pages;
    u32    cmp_Ring_عدد_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_عدد_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_عدد_PAGES];
} PACKED;

بنية PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

بنية PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_عدد_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_عدد_Entries_Log2;

    u8     pad[104];

    u32    پیغام_Prod_Idx;
    u32    پیغام_Cons_Idx;
    u32    پیغام_عدد_Entries_Log2;
} PACKED;

بنية PVSCSIRingReqDesc {
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

بنية pvscsi_ring_dsc_s {
    بنية PVSCSIRingsState *ring_state;
    بنية PVSCSIRingReqDesc *ring_reqs;
    بنية PVSCSIRingCmpDesc *ring_cmps;
};

بنية pvscsi_lun_s {
    بنية drive_s drive;
    فراغ *iobase;
    u8 target;
    u8 lun;
    بنية pvscsi_ring_dsc_s *ring_dsc;
};

ساكن فراغ
pvscsi_write_cmd_desc(فراغ *iobase, u32 cmd, ثابت فراغ *desc, size_t len)
{
    ثابت u32 *ptr = desc;
    size_t i;

    len /= حجم(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    لكل (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

ساكن فراغ
pvscsi_kick_rw_io(فراغ *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

ساكن فراغ
pvscsi_wait_intr_cmpl(فراغ *iobase)
{
    طالما (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

ساكن فراغ
pvscsi_init_rings(فراغ *iobase, بنية pvscsi_ring_dsc_s **ring_dsc)
{
    بنية PVSCSICmdDescSetupRings cmd = {0,};

    بنية pvscsi_ring_dsc_s *dsc = malloc_بلند(حجم(*dsc));
    إذا (!dsc) {
        warn_noalloc();
        أرجع;
    }

    dsc->ring_state =
        (بنية PVSCSIRingsState *)memalign_بلند(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (بنية PVSCSIRingReqDesc *)memalign_بلند(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (بنية PVSCSIRingCmpDesc *)memalign_بلند(PAGE_SIZE, PAGE_SIZE);
    إذا (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        أرجع;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_عدد_Pages = 1;
    cmd.cmp_Ring_عدد_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, حجم(cmd));
    *ring_dsc = dsc;
}

ساكن u32
pvscsi_get_rsp(بنية PVSCSIRingsState *s,
               بنية PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    أرجع status;
}

صحيح
pvscsi_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_PVSCSI)
        أرجع DISK_RET_EBADTRACK;
    بنية pvscsi_lun_s *plun =
        container_of(op->drive_fl, بنية pvscsi_lun_s, drive);
    بنية pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    بنية PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_عدد_Entries_Log2;
    u32 cmp_entries = s->cmp_عدد_Entries_Log2;
    بنية PVSCSIRingReqDesc *req;
    بنية PVSCSIRingCmpDesc *rsp;
    u32 status;

    إذا (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        أرجع DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    صحيح blocksize = scsi_fill_cmd(op, req->cdb, 16);
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, حجم(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->گنتی * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    أرجع status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ساكن صحيح
pvscsi_add_lun(بنية pci_device *pci, فراغ *iobase,
               بنية pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    بنية pvscsi_lun_s *plun = malloc_fseg(حجم(*plun));
    إذا (!plun) {
        warn_noalloc();
        أرجع -1;
    }
    memset(plun, 0, حجم(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    حرف *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    صحيح prio = bootprio_find_scsi_device(pci, target, lun);
    صحيح ret = scsi_drive_setup(&plun->drive, name, prio);
    حرر(name);
    إذا (ret)
        اذهب fail;
    أرجع 0;

fail:
    حرر(plun);
    أرجع -1;
}

ساكن فراغ
pvscsi_scan_target(بنية pci_device *pci, فراغ *iobase,
                   بنية pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

ساكن فراغ
init_pvscsi(فراغ *data)
{
    بنية pci_device *pci = data;
    فراغ *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    إذا (!iobase)
        أرجع;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, عدم, 0);

    بنية pvscsi_ring_dsc_s *ring_dsc = عدم;
    pvscsi_init_rings(iobase, &ring_dsc);
    صحيح i;
    لكل (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

فراغ
pvscsi_setup(فراغ)
{
    ASSERT32FLAT();
    إذا (! CONFIG_PVSCSI)
        أرجع;

    dprintf(3, "init pvscsi\n");

    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            استمر;
        run_thread(init_pvscsi, pci);
    }
}
