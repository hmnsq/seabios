// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // scsi_drive_setup
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "memmap.h" // PAGE_SHIFT, virt_to_phys
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "pvscsi.h" // pvscsi_setup
#ଅନ୍ତର୍ଭାଵ "stacks.h" // run_thread
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // usleep
#ଅନ୍ତର୍ଭାଵ "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_ସନ୍ଦେଶ_0                  (1 << 2)
#define PVSCSI_INTR_ସନ୍ଦେଶ_1                  (1 << 3)
#define PVSCSI_INTR_ସନ୍ଦେଶ_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

ଗଣନା PVSCSIRegOffset {
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

ଗଣନା PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_ସନ୍ଦେଶ_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_ଅଙ୍କ_PAGES        32
ସଂରଚନା PVSCSICmdDescSetupRings {
    u32    req_Ring_ଅଙ୍କ_Pages;
    u32    cmp_Ring_ଅଙ୍କ_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_ଅଙ୍କ_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_ଅଙ୍କ_PAGES];
} PACKED;

ସଂରଚନା PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

ସଂରଚନା PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_ଅଙ୍କ_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_ଅଙ୍କ_Entries_Log2;

    u8     pad[104];

    u32    ସନ୍ଦେଶ_Prod_Idx;
    u32    ସନ୍ଦେଶ_Cons_Idx;
    u32    ସନ୍ଦେଶ_ଅଙ୍କ_Entries_Log2;
} PACKED;

ସଂରଚନା PVSCSIRingReqDesc {
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

ସଂରଚନା pvscsi_ring_dsc_s {
    ସଂରଚନା PVSCSIRingsState *ring_state;
    ସଂରଚନା PVSCSIRingReqDesc *ring_reqs;
    ସଂରଚନା PVSCSIRingCmpDesc *ring_cmps;
};

ସଂରଚନା pvscsi_lun_s {
    ସଂରଚନା drive_s drive;
    ଶୂନ୍ୟ *iobase;
    u8 target;
    u8 lun;
    ସଂରଚନା pvscsi_ring_dsc_s *ring_dsc;
};

ସ୍ଥିର ଶୂନ୍ୟ
pvscsi_write_cmd_desc(ଶୂନ୍ୟ *iobase, u32 cmd, ଅଚଳ ଶୂନ୍ୟ *desc, size_t len)
{
    ଅଚଳ u32 *ptr = desc;
    size_t i;

    len /= ଆକାର(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    ପାଇଁ (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

ସ୍ଥିର ଶୂନ୍ୟ
pvscsi_kick_rw_io(ଶୂନ୍ୟ *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

ସ୍ଥିର ଶୂନ୍ୟ
pvscsi_wait_intr_cmpl(ଶୂନ୍ୟ *iobase)
{
    ଯେତେବେଳେ (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

ସ୍ଥିର ଶୂନ୍ୟ
pvscsi_init_rings(ଶୂନ୍ୟ *iobase, ସଂରଚନା pvscsi_ring_dsc_s **ring_dsc)
{
    ସଂରଚନା PVSCSICmdDescSetupRings cmd = {0,};

    ସଂରଚନା pvscsi_ring_dsc_s *dsc = malloc_ଉପର(ଆକାର(*dsc));
    ଯଦି (!dsc) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }

    dsc->ring_state =
        (ସଂରଚନା PVSCSIRingsState *)memalign_ଉପର(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (ସଂରଚନା PVSCSIRingReqDesc *)memalign_ଉପର(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (ସଂରଚନା PVSCSIRingCmpDesc *)memalign_ଉପର(PAGE_SIZE, PAGE_SIZE);
    ଯଦି (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_ଅଙ୍କ_Pages = 1;
    cmd.cmp_Ring_ଅଙ୍କ_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, ଆକାର(cmd));
    *ring_dsc = dsc;
}

ସ୍ଥିର u32
pvscsi_get_rsp(ସଂରଚନା PVSCSIRingsState *s,
               ସଂରଚନା PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    ଫେରନ୍ତୁ status;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
pvscsi_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_PVSCSI)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    ସଂରଚନା pvscsi_lun_s *plun =
        container_of(op->drive_fl, ସଂରଚନା pvscsi_lun_s, drive);
    ସଂରଚନା pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    ସଂରଚନା PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_ଅଙ୍କ_Entries_Log2;
    u32 cmp_entries = s->cmp_ଅଙ୍କ_Entries_Log2;
    ସଂରଚନା PVSCSIRingReqDesc *req;
    ସଂରଚନା PVSCSIRingCmpDesc *rsp;
    u32 status;

    ଯଦି (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, req->cdb, 16);
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, ଆକାର(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->ଗଣନା_2 * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    ଫେରନ୍ତୁ status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
pvscsi_add_lun(ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *iobase,
               ସଂରଚନା pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    ସଂରଚନା pvscsi_lun_s *plun = malloc_fseg(ଆକାର(*plun));
    ଯଦି (!plun) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    memset(plun, 0, ଆକାର(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    ଅକ୍ଷର *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = bootprio_find_scsi_device(pci, target, lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_drive_setup(&plun->drive, name, prio);
    ମୁକ୍ତ(name);
    ଯଦି (ret)
        ଯାଅ fail;
    ଫେରନ୍ତୁ 0;

fail:
    ମୁକ୍ତ(plun);
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ଶୂନ୍ୟ
pvscsi_scan_target(ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *iobase,
                   ସଂରଚନା pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

ସ୍ଥିର ଶୂନ୍ୟ
init_pvscsi(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା pci_device *pci = data;
    ଶୂନ୍ୟ *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ଯଦି (!iobase)
        ଫେରନ୍ତୁ;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, ଖାଲି, 0);

    ସଂରଚନା pvscsi_ring_dsc_s *ring_dsc = ଖାଲି;
    pvscsi_init_rings(iobase, &ring_dsc);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

ଶୂନ୍ୟ
pvscsi_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (! CONFIG_PVSCSI)
        ଫେରନ୍ତୁ;

    dprintf(3, "init pvscsi\n");

    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            ଜାରି;
        run_thread(init_pvscsi, pci);
    }
}
