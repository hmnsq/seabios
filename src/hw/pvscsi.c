// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "blockcmd.h" // scsi_drive_setup
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "memmap.h" // PAGE_SHIFT, virt_to_phys
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#અન્તર્ભાવ "pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "pvscsi.h" // pvscsi_setup
#અન્તર્ભાવ "stacks.h" // run_thread
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // usleep
#અન્તર્ભાવ "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_સંદેશ_0                  (1 << 2)
#define PVSCSI_INTR_સંદેશ_1                  (1 << 3)
#define PVSCSI_INTR_સંદેશ_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

ગણના PVSCSIRegOffset {
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

ગણના PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_સંદેશ_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_અંક_PAGES        32
સંરચના PVSCSICmdDescSetupRings {
    u32    req_Ring_અંક_Pages;
    u32    cmp_Ring_અંક_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_અંક_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_અંક_PAGES];
} PACKED;

સંરચના PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

સંરચના PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_અંક_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_અંક_Entries_Log2;

    u8     pad[104];

    u32    સંદેશ_Prod_Idx;
    u32    સંદેશ_Cons_Idx;
    u32    સંદેશ_અંક_Entries_Log2;
} PACKED;

સંરચના PVSCSIRingReqDesc {
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

સંરચના pvscsi_ring_dsc_s {
    સંરચના PVSCSIRingsState *ring_state;
    સંરચના PVSCSIRingReqDesc *ring_reqs;
    સંરચના PVSCSIRingCmpDesc *ring_cmps;
};

સંરચના pvscsi_lun_s {
    સંરચના drive_s drive;
    શૂન્ય *iobase;
    u8 target;
    u8 lun;
    સંરચના pvscsi_ring_dsc_s *ring_dsc;
};

સ્થિર શૂન્ય
pvscsi_write_cmd_desc(શૂન્ય *iobase, u32 cmd, અચલ શૂન્ય *desc, size_t len)
{
    અચલ u32 *ptr = desc;
    size_t i;

    len /= કદ(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    માટે (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

સ્થિર શૂન્ય
pvscsi_kick_rw_io(શૂન્ય *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

સ્થિર શૂન્ય
pvscsi_wait_intr_cmpl(શૂન્ય *iobase)
{
    જ્યારે (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

સ્થિર શૂન્ય
pvscsi_init_rings(શૂન્ય *iobase, સંરચના pvscsi_ring_dsc_s **ring_dsc)
{
    સંરચના PVSCSICmdDescSetupRings cmd = {0,};

    સંરચના pvscsi_ring_dsc_s *dsc = malloc_ઉપલું(કદ(*dsc));
    જો (!dsc) {
        warn_noalloc();
        પાછા;
    }

    dsc->ring_state =
        (સંરચના PVSCSIRingsState *)memalign_ઉપલું(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (સંરચના PVSCSIRingReqDesc *)memalign_ઉપલું(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (સંરચના PVSCSIRingCmpDesc *)memalign_ઉપલું(PAGE_SIZE, PAGE_SIZE);
    જો (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        પાછા;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_અંક_Pages = 1;
    cmd.cmp_Ring_અંક_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, કદ(cmd));
    *ring_dsc = dsc;
}

સ્થિર u32
pvscsi_get_rsp(સંરચના PVSCSIRingsState *s,
               સંરચના PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    પાછા status;
}

પૂર્ણાંક
pvscsi_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_PVSCSI)
        પાછા DISK_RET_EBADTRACK;
    સંરચના pvscsi_lun_s *plun =
        container_of(op->drive_fl, સંરચના pvscsi_lun_s, drive);
    સંરચના pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    સંરચના PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_અંક_Entries_Log2;
    u32 cmp_entries = s->cmp_અંક_Entries_Log2;
    સંરચના PVSCSIRingReqDesc *req;
    સંરચના PVSCSIRingCmpDesc *rsp;
    u32 status;

    જો (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        પાછા DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    પૂર્ણાંક blocksize = scsi_fill_cmd(op, req->cdb, 16);
    જો (blocksize < 0)
        પાછા default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, કદ(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->ગણતરી * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    પાછા status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

સ્થિર પૂર્ણાંક
pvscsi_add_lun(સંરચના pci_device *pci, શૂન્ય *iobase,
               સંરચના pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    સંરચના pvscsi_lun_s *plun = malloc_fseg(કદ(*plun));
    જો (!plun) {
        warn_noalloc();
        પાછા -1;
    }
    memset(plun, 0, કદ(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    અક્ષર *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    પૂર્ણાંક prio = bootprio_find_scsi_device(pci, target, lun);
    પૂર્ણાંક ret = scsi_drive_setup(&plun->drive, name, prio);
    મુક્ત(name);
    જો (ret)
        જાઓ fail;
    પાછા 0;

fail:
    મુક્ત(plun);
    પાછા -1;
}

સ્થિર શૂન્ય
pvscsi_scan_target(સંરચના pci_device *pci, શૂન્ય *iobase,
                   સંરચના pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

સ્થિર શૂન્ય
init_pvscsi(શૂન્ય *data)
{
    સંરચના pci_device *pci = data;
    શૂન્ય *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    જો (!iobase)
        પાછા;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, ખાલી, 0);

    સંરચના pvscsi_ring_dsc_s *ring_dsc = ખાલી;
    pvscsi_init_rings(iobase, &ring_dsc);
    પૂર્ણાંક i;
    માટે (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

શૂન્ય
pvscsi_setup(શૂન્ય)
{
    ASSERT32FLAT();
    જો (! CONFIG_PVSCSI)
        પાછા;

    dprintf(3, "init pvscsi\n");

    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            ચાલુ;
        run_thread(init_pvscsi, pci);
    }
}
