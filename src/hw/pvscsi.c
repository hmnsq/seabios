// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "blockcmd.h" // scsi_drive_setup
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "memmap.h" // PAGE_SHIFT, virt_to_phys
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#அந்தர்பாவ "pci_regs.h" // PCI_VENDOR_ID
#அந்தர்பாவ "pvscsi.h" // pvscsi_setup
#அந்தர்பாவ "stacks.h" // run_thread
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // usleep
#அந்தர்பாவ "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_செய்தி_0                  (1 << 2)
#define PVSCSI_INTR_செய்தி_1                  (1 << 3)
#define PVSCSI_INTR_செய்தி_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

எண்ணிக்கை PVSCSIRegOffset {
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

எண்ணிக்கை PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_செய்தி_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_இலக்கம்_PAGES        32
கட்டமைப்பு PVSCSICmdDescSetupRings {
    u32    req_Ring_இலக்கம்_Pages;
    u32    cmp_Ring_இலக்கம்_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_இலக்கம்_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_இலக்கம்_PAGES];
} PACKED;

கட்டமைப்பு PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

கட்டமைப்பு PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_இலக்கம்_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_இலக்கம்_Entries_Log2;

    u8     pad[104];

    u32    செய்தி_Prod_Idx;
    u32    செய்தி_Cons_Idx;
    u32    செய்தி_இலக்கம்_Entries_Log2;
} PACKED;

கட்டமைப்பு PVSCSIRingReqDesc {
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

கட்டமைப்பு pvscsi_ring_dsc_s {
    கட்டமைப்பு PVSCSIRingsState *ring_state;
    கட்டமைப்பு PVSCSIRingReqDesc *ring_reqs;
    கட்டமைப்பு PVSCSIRingCmpDesc *ring_cmps;
};

கட்டமைப்பு pvscsi_lun_s {
    கட்டமைப்பு drive_s drive;
    வெறுமை *iobase;
    u8 target;
    u8 lun;
    கட்டமைப்பு pvscsi_ring_dsc_s *ring_dsc;
};

நிலையான வெறுமை
pvscsi_write_cmd_desc(வெறுமை *iobase, u32 cmd, மாறிலி வெறுமை *desc, size_t len)
{
    மாறிலி u32 *ptr = desc;
    size_t i;

    len /= அளவு(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    க்கு (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

நிலையான வெறுமை
pvscsi_kick_rw_io(வெறுமை *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

நிலையான வெறுமை
pvscsi_wait_intr_cmpl(வெறுமை *iobase)
{
    போது (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

நிலையான வெறுமை
pvscsi_init_rings(வெறுமை *iobase, கட்டமைப்பு pvscsi_ring_dsc_s **ring_dsc)
{
    கட்டமைப்பு PVSCSICmdDescSetupRings cmd = {0,};

    கட்டமைப்பு pvscsi_ring_dsc_s *dsc = malloc_உயர்வு(அளவு(*dsc));
    எனில் (!dsc) {
        warn_noalloc();
        திரும்பு;
    }

    dsc->ring_state =
        (கட்டமைப்பு PVSCSIRingsState *)memalign_உயர்வு(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (கட்டமைப்பு PVSCSIRingReqDesc *)memalign_உயர்வு(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (கட்டமைப்பு PVSCSIRingCmpDesc *)memalign_உயர்வு(PAGE_SIZE, PAGE_SIZE);
    எனில் (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        திரும்பு;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_இலக்கம்_Pages = 1;
    cmd.cmp_Ring_இலக்கம்_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, அளவு(cmd));
    *ring_dsc = dsc;
}

நிலையான u32
pvscsi_get_rsp(கட்டமைப்பு PVSCSIRingsState *s,
               கட்டமைப்பு PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    திரும்பு status;
}

முழு
pvscsi_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_PVSCSI)
        திரும்பு DISK_RET_EBADTRACK;
    கட்டமைப்பு pvscsi_lun_s *plun =
        container_of(op->drive_fl, கட்டமைப்பு pvscsi_lun_s, drive);
    கட்டமைப்பு pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    கட்டமைப்பு PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_இலக்கம்_Entries_Log2;
    u32 cmp_entries = s->cmp_இலக்கம்_Entries_Log2;
    கட்டமைப்பு PVSCSIRingReqDesc *req;
    கட்டமைப்பு PVSCSIRingCmpDesc *rsp;
    u32 status;

    எனில் (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        திரும்பு DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    முழு blocksize = scsi_fill_cmd(op, req->cdb, 16);
    எனில் (blocksize < 0)
        திரும்பு default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, அளவு(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->மொத்தம் * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    திரும்பு status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

நிலையான முழு
pvscsi_add_lun(கட்டமைப்பு pci_device *pci, வெறுமை *iobase,
               கட்டமைப்பு pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    கட்டமைப்பு pvscsi_lun_s *plun = malloc_fseg(அளவு(*plun));
    எனில் (!plun) {
        warn_noalloc();
        திரும்பு -1;
    }
    memset(plun, 0, அளவு(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    எழுத்து *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    முழு prio = bootprio_find_scsi_device(pci, target, lun);
    முழு ret = scsi_drive_setup(&plun->drive, name, prio);
    விடுவி(name);
    எனில் (ret)
        செல் fail;
    திரும்பு 0;

fail:
    விடுவி(plun);
    திரும்பு -1;
}

நிலையான வெறுமை
pvscsi_scan_target(கட்டமைப்பு pci_device *pci, வெறுமை *iobase,
                   கட்டமைப்பு pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

நிலையான வெறுமை
init_pvscsi(வெறுமை *data)
{
    கட்டமைப்பு pci_device *pci = data;
    வெறுமை *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    எனில் (!iobase)
        திரும்பு;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, வெற்று, 0);

    கட்டமைப்பு pvscsi_ring_dsc_s *ring_dsc = வெற்று;
    pvscsi_init_rings(iobase, &ring_dsc);
    முழு i;
    க்கு (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

வெறுமை
pvscsi_setup(வெறுமை)
{
    ASSERT32FLAT();
    எனில் (! CONFIG_PVSCSI)
        திரும்பு;

    dprintf(3, "init pvscsi\n");

    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            தொடர்;
        run_thread(init_pvscsi, pci);
    }
}
