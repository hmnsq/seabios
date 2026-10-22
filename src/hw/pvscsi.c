// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "block.h" // struct drive_s
#include "blockcmd.h" // scsi_drive_setup
#include "config.h" // CONFIG_*
#include "malloc.h" // free
#include "memmap.h" // PAGE_SHIFT, virt_to_phys
#include "output.h" // dprintf
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#include "pci_regs.h" // PCI_VENDOR_ID
#include "pvscsi.h" // pvscsi_setup
#include "stacks.h" // run_thread
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // usleep
#include "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_MSG_0                  (1 << 2)
#define PVSCSI_INTR_MSG_1                  (1 << 3)
#define PVSCSI_INTR_MSG_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

ga_nanaa PVSCSIRegOffset {
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

ga_nanaa PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_MSG_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_NUM_PAGES        32
sa_nracanaa PVSCSICmdDescSetupRings {
    u32    reqRingNumPages;
    u32    cmpRingNumPages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_NUM_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_NUM_PAGES];
} PACKED;

sa_nracanaa PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

sa_nracanaa PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    reqNumEntriesLog2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmpNumEntriesLog2;

    u8     pad[104];

    u32    msgProdIdx;
    u32    msgConsIdx;
    u32    msgNumEntriesLog2;
} PACKED;

sa_nracanaa PVSCSIRingReqDesc {
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

sa_nracanaa pvscsi_ring_dsc_s {
    sa_nracanaa PVSCSIRingsState *ring_state;
    sa_nracanaa PVSCSIRingReqDesc *ring_reqs;
    sa_nracanaa PVSCSIRingCmpDesc *ring_cmps;
};

sa_nracanaa pvscsi_lun_s {
    sa_nracanaa drive_s drive;
    void *iobase;
    u8 target;
    u8 lun;
    sa_nracanaa pvscsi_ring_dsc_s *ring_dsc;
};

static void
pvscsi_write_cmd_desc(void *iobase, u32 cmd, const void *desc, size_t len)
{
    const u32 *ptr = desc;
    size_t i;

    len /= parimaa_na(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    for (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

static void
pvscsi_kick_rw_io(void *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

static void
pvscsi_wait_intr_cmpl(void *iobase)
{
    ᯀᯬᯝ (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

static void
pvscsi_init_rings(void *iobase, sa_nracanaa pvscsi_ring_dsc_s **ring_dsc)
{
    sa_nracanaa PVSCSICmdDescSetupRings cmd = {0,};

    sa_nracanaa pvscsi_ring_dsc_s *dsc = malloc_high(parimaa_na(*dsc));
    ᯀᯮᯝ (!dsc) {
        warn_noalloc();
        ᯀᯮᯞ;
    }

    dsc->ring_state =
        (sa_nracanaa PVSCSIRingsState *)memalign_high(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (sa_nracanaa PVSCSIRingReqDesc *)memalign_high(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (sa_nracanaa PVSCSIRingCmpDesc *)memalign_high(PAGE_SIZE, PAGE_SIZE);
    ᯀᯮᯝ (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        ᯀᯮᯞ;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.reqRingNumPages = 1;
    cmd.cmpRingNumPages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, parimaa_na(cmd));
    *ring_dsc = dsc;
}

static u32
pvscsi_get_rsp(sa_nracanaa PVSCSIRingsState *s,
               sa_nracanaa PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    ᯀᯮᯞ status;
}

puur_naa_nka
pvscsi_process_op(sa_nracanaa disk_op_s *op)
{
    ᯀᯮᯝ (!CONFIG_PVSCSI)
        ᯀᯮᯞ DISK_RET_EBADTRACK;
    sa_nracanaa pvscsi_lun_s *plun =
        container_of(op->drive_fl, sa_nracanaa pvscsi_lun_s, drive);
    sa_nracanaa pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    sa_nracanaa PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->reqNumEntriesLog2;
    u32 cmp_entries = s->cmpNumEntriesLog2;
    sa_nracanaa PVSCSIRingReqDesc *req;
    sa_nracanaa PVSCSIRingCmpDesc *rsp;
    u32 status;

    ᯀᯮᯝ (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        ᯀᯮᯞ DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    puur_naa_nka blocksize = scsi_fill_cmd(op, req->cdb, 16);
    ᯀᯮᯝ (blocksize < 0)
        ᯀᯮᯞ default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, parimaa_na(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->count * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    ᯀᯮᯞ status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

static puur_naa_nka
pvscsi_add_lun(sa_nracanaa pci_device *pci, void *iobase,
               sa_nracanaa pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    sa_nracanaa pvscsi_lun_s *plun = malloc_fseg(parimaa_na(*plun));
    ᯀᯮᯝ (!plun) {
        warn_noalloc();
        ᯀᯮᯞ -1;
    }
    memset(plun, 0, parimaa_na(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    var_na *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    puur_naa_nka prio = bootprio_find_scsi_device(pci, target, lun);
    puur_naa_nka ret = scsi_drive_setup(&plun->drive, name, prio);
    free(name);
    ᯀᯮᯝ (ret)
        goto fail;
    ᯀᯮᯞ 0;

fail:
    free(plun);
    ᯀᯮᯞ -1;
}

static void
pvscsi_scan_target(sa_nracanaa pci_device *pci, void *iobase,
                   sa_nracanaa pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

static void
init_pvscsi(void *data)
{
    sa_nracanaa pci_device *pci = data;
    void *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ᯀᯮᯝ (!iobase)
        ᯀᯮᯞ;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, NULL, 0);

    sa_nracanaa pvscsi_ring_dsc_s *ring_dsc = NULL;
    pvscsi_init_rings(iobase, &ring_dsc);
    puur_naa_nka i;
    for (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

void
pvscsi_setup(void)
{
    ASSERT32FLAT();
    ᯀᯮᯝ (! CONFIG_PVSCSI)
        ᯀᯮᯞ;

    dprintf(3, "init pvscsi\n");

    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ᯀᯮᯝ (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            continue;
        run_thread(init_pvscsi, pci);
    }
}
