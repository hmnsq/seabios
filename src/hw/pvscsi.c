// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // scsi_drive_setup
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SHIFT, virt_to_phys
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "pvscsi.h" // pvscsi_setup
#ಅನ್ತರ್ಭಾವ "stacks.h" // run_thread
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // usleep
#ಅನ್ತರ್ಭಾವ "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_ಸಂದೇಶ_0                  (1 << 2)
#define PVSCSI_INTR_ಸಂದೇಶ_1                  (1 << 3)
#define PVSCSI_INTR_ಸಂದೇಶ_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

ಗಣನಾ PVSCSIRegOffset {
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

ಗಣನಾ PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_ಸಂದೇಶ_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_ಅಂಕೆ_PAGES        32
ಸಂರಚನಾ PVSCSICmdDescSetupRings {
    u32    req_Ring_ಅಂಕೆ_Pages;
    u32    cmp_Ring_ಅಂಕೆ_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_ಅಂಕೆ_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_ಅಂಕೆ_PAGES];
} PACKED;

ಸಂರಚನಾ PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

ಸಂರಚನಾ PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_ಅಂಕೆ_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_ಅಂಕೆ_Entries_Log2;

    u8     pad[104];

    u32    ಸಂದೇಶ_Prod_Idx;
    u32    ಸಂದೇಶ_Cons_Idx;
    u32    ಸಂದೇಶ_ಅಂಕೆ_Entries_Log2;
} PACKED;

ಸಂರಚನಾ PVSCSIRingReqDesc {
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

ಸಂರಚನಾ pvscsi_ring_dsc_s {
    ಸಂರಚನಾ PVSCSIRingsState *ring_state;
    ಸಂರಚನಾ PVSCSIRingReqDesc *ring_reqs;
    ಸಂರಚನಾ PVSCSIRingCmpDesc *ring_cmps;
};

ಸಂರಚನಾ pvscsi_lun_s {
    ಸಂರಚನಾ drive_s drive;
    ಶೂನ್ಯ *iobase;
    u8 target;
    u8 lun;
    ಸಂರಚನಾ pvscsi_ring_dsc_s *ring_dsc;
};

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_write_cmd_desc(ಶೂನ್ಯ *iobase, u32 cmd, ಧ್ರುವ ಶೂನ್ಯ *desc, size_t len)
{
    ಧ್ರುವ u32 *ptr = desc;
    size_t i;

    len /= ಪರಿಮಾಣ(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    ಪ್ರತಿ (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_kick_rw_io(ಶೂನ್ಯ *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_wait_intr_cmpl(ಶೂನ್ಯ *iobase)
{
    ಯಾವತ್ (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_init_rings(ಶೂನ್ಯ *iobase, ಸಂರಚನಾ pvscsi_ring_dsc_s **ring_dsc)
{
    ಸಂರಚನಾ PVSCSICmdDescSetupRings cmd = {0,};

    ಸಂರಚನಾ pvscsi_ring_dsc_s *dsc = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*dsc));
    ಯದಿ (!dsc) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }

    dsc->ring_state =
        (ಸಂರಚನಾ PVSCSIRingsState *)memalign_ಮೇಲಿನ(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (ಸಂರಚನಾ PVSCSIRingReqDesc *)memalign_ಮೇಲಿನ(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (ಸಂರಚನಾ PVSCSIRingCmpDesc *)memalign_ಮೇಲಿನ(PAGE_SIZE, PAGE_SIZE);
    ಯದಿ (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_ಅಂಕೆ_Pages = 1;
    cmd.cmp_Ring_ಅಂಕೆ_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, ಪರಿಮಾಣ(cmd));
    *ring_dsc = dsc;
}

ಸ್ಥಿರ u32
pvscsi_get_rsp(ಸಂರಚನಾ PVSCSIRingsState *s,
               ಸಂರಚನಾ PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    ಪ್ರತ್ಯಾಗಮ status;
}

ಪೂರ್ಣಾಙ್ಕ
pvscsi_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_PVSCSI)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    ಸಂರಚನಾ pvscsi_lun_s *plun =
        container_of(op->drive_fl, ಸಂರಚನಾ pvscsi_lun_s, drive);
    ಸಂರಚನಾ pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    ಸಂರಚನಾ PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_ಅಂಕೆ_Entries_Log2;
    u32 cmp_entries = s->cmp_ಅಂಕೆ_Entries_Log2;
    ಸಂರಚನಾ PVSCSIRingReqDesc *req;
    ಸಂರಚನಾ PVSCSIRingCmpDesc *rsp;
    u32 status;

    ಯದಿ (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    ಪೂರ್ಣಾಙ್ಕ blocksize = scsi_fill_cmd(op, req->cdb, 16);
    ಯದಿ (blocksize < 0)
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, ಪರಿಮಾಣ(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->ಎಣಿಕೆ * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    ಪ್ರತ್ಯಾಗಮ status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
pvscsi_add_lun(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *iobase,
               ಸಂರಚನಾ pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    ಸಂರಚನಾ pvscsi_lun_s *plun = malloc_fseg(ಪರಿಮಾಣ(*plun));
    ಯದಿ (!plun) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    memset(plun, 0, ಪರಿಮಾಣ(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    ವರ್ಣ *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    ಪೂರ್ಣಾಙ್ಕ prio = bootprio_find_scsi_device(pci, target, lun);
    ಪೂರ್ಣಾಙ್ಕ ret = scsi_drive_setup(&plun->drive, name, prio);
    ಮುಕ್ತ(name);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;
    ಪ್ರತ್ಯಾಗಮ 0;

fail:
    ಮುಕ್ತ(plun);
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_scan_target(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *iobase,
                   ಸಂರಚನಾ pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_pvscsi(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ pci_device *pci = data;
    ಶೂನ್ಯ *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ಯದಿ (!iobase)
        ಪ್ರತ್ಯಾಗಮ;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, ರಿಕ್ತ, 0);

    ಸಂರಚನಾ pvscsi_ring_dsc_s *ring_dsc = ರಿಕ್ತ;
    pvscsi_init_rings(iobase, &ring_dsc);
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

ಶೂನ್ಯ
pvscsi_setup(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯದಿ (! CONFIG_PVSCSI)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init pvscsi\n");

    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            ಅನುವರ್ತ;
        run_thread(init_pvscsi, pci);
    }
}
