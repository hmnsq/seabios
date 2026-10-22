// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "blockcmd.h" // scsi_drive_setup
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "memmap.h" // PAGE_SHIFT, virt_to_phys
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#അന്തര്ഭാവ "pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "pvscsi.h" // pvscsi_setup
#അന്തര്ഭാവ "stacks.h" // run_thread
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // usleep
#അന്തര്ഭാവ "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_സന്ദേശം_0                  (1 << 2)
#define PVSCSI_INTR_സന്ദേശം_1                  (1 << 3)
#define PVSCSI_INTR_സന്ദേശം_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

എണ്ണൽ PVSCSIRegOffset {
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

എണ്ണൽ PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_സന്ദേശം_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_അക്കം_PAGES        32
ഘടന PVSCSICmdDescSetupRings {
    u32    req_Ring_അക്കം_Pages;
    u32    cmp_Ring_അക്കം_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_അക്കം_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_അക്കം_PAGES];
} PACKED;

ഘടന PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

ഘടന PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_അക്കം_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_അക്കം_Entries_Log2;

    u8     pad[104];

    u32    സന്ദേശം_Prod_Idx;
    u32    സന്ദേശം_Cons_Idx;
    u32    സന്ദേശം_അക്കം_Entries_Log2;
} PACKED;

ഘടന PVSCSIRingReqDesc {
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

ഘടന pvscsi_ring_dsc_s {
    ഘടന PVSCSIRingsState *ring_state;
    ഘടന PVSCSIRingReqDesc *ring_reqs;
    ഘടന PVSCSIRingCmpDesc *ring_cmps;
};

ഘടന pvscsi_lun_s {
    ഘടന drive_s drive;
    ശൂന്യം *iobase;
    u8 target;
    u8 lun;
    ഘടന pvscsi_ring_dsc_s *ring_dsc;
};

സ്ഥിതം ശൂന്യം
pvscsi_write_cmd_desc(ശൂന്യം *iobase, u32 cmd, സ്ഥിരം ശൂന്യം *desc, size_t len)
{
    സ്ഥിരം u32 *ptr = desc;
    size_t i;

    len /= വലുപ്പം(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    വേണ്ടി (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

സ്ഥിതം ശൂന്യം
pvscsi_kick_rw_io(ശൂന്യം *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

സ്ഥിതം ശൂന്യം
pvscsi_wait_intr_cmpl(ശൂന്യം *iobase)
{
    ആകുമ്പോൾ (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

സ്ഥിതം ശൂന്യം
pvscsi_init_rings(ശൂന്യം *iobase, ഘടന pvscsi_ring_dsc_s **ring_dsc)
{
    ഘടന PVSCSICmdDescSetupRings cmd = {0,};

    ഘടന pvscsi_ring_dsc_s *dsc = malloc_മുകളിലെ(വലുപ്പം(*dsc));
    എങ്കിൽ (!dsc) {
        warn_noalloc();
        തിരികെ;
    }

    dsc->ring_state =
        (ഘടന PVSCSIRingsState *)memalign_മുകളിലെ(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (ഘടന PVSCSIRingReqDesc *)memalign_മുകളിലെ(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (ഘടന PVSCSIRingCmpDesc *)memalign_മുകളിലെ(PAGE_SIZE, PAGE_SIZE);
    എങ്കിൽ (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        തിരികെ;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_അക്കം_Pages = 1;
    cmd.cmp_Ring_അക്കം_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, വലുപ്പം(cmd));
    *ring_dsc = dsc;
}

സ്ഥിതം u32
pvscsi_get_rsp(ഘടന PVSCSIRingsState *s,
               ഘടന PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    തിരികെ status;
}

പൂർണ്ണസംഖ്യ
pvscsi_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_PVSCSI)
        തിരികെ DISK_RET_EBADTRACK;
    ഘടന pvscsi_lun_s *plun =
        container_of(op->drive_fl, ഘടന pvscsi_lun_s, drive);
    ഘടന pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    ഘടന PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_അക്കം_Entries_Log2;
    u32 cmp_entries = s->cmp_അക്കം_Entries_Log2;
    ഘടന PVSCSIRingReqDesc *req;
    ഘടന PVSCSIRingCmpDesc *rsp;
    u32 status;

    എങ്കിൽ (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        തിരികെ DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    പൂർണ്ണസംഖ്യ blocksize = scsi_fill_cmd(op, req->cdb, 16);
    എങ്കിൽ (blocksize < 0)
        തിരികെ default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, വലുപ്പം(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->എണ്ണം * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    തിരികെ status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
pvscsi_add_lun(ഘടന pci_device *pci, ശൂന്യം *iobase,
               ഘടന pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    ഘടന pvscsi_lun_s *plun = malloc_fseg(വലുപ്പം(*plun));
    എങ്കിൽ (!plun) {
        warn_noalloc();
        തിരികെ -1;
    }
    memset(plun, 0, വലുപ്പം(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    അക്ഷരം *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    പൂർണ്ണസംഖ്യ prio = bootprio_find_scsi_device(pci, target, lun);
    പൂർണ്ണസംഖ്യ ret = scsi_drive_setup(&plun->drive, name, prio);
    സ്വതന്ത്രം(name);
    എങ്കിൽ (ret)
        പോകുക fail;
    തിരികെ 0;

fail:
    സ്വതന്ത്രം(plun);
    തിരികെ -1;
}

സ്ഥിതം ശൂന്യം
pvscsi_scan_target(ഘടന pci_device *pci, ശൂന്യം *iobase,
                   ഘടന pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

സ്ഥിതം ശൂന്യം
init_pvscsi(ശൂന്യം *data)
{
    ഘടന pci_device *pci = data;
    ശൂന്യം *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    എങ്കിൽ (!iobase)
        തിരികെ;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, ശൂന്യ, 0);

    ഘടന pvscsi_ring_dsc_s *ring_dsc = ശൂന്യ;
    pvscsi_init_rings(iobase, &ring_dsc);
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

ശൂന്യം
pvscsi_setup(ശൂന്യം)
{
    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_PVSCSI)
        തിരികെ;

    dprintf(3, "init pvscsi\n");

    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            തുടരുക;
        run_thread(init_pvscsi, pci);
    }
}
