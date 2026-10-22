// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "block.h" // struct drive_s
#समावेश "blockcmd.h" // scsi_drive_setup
#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // free
#समावेश "memmap.h" // PAGE_SHIFT, virt_to_phys
#समावेश "output.h" // dprintf
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#समावेश "pci_regs.h" // PCI_VENDOR_ID
#समावेश "pvscsi.h" // pvscsi_setup
#समावेश "stacks.h" // run_thread
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // usleep
#समावेश "x86.h" // writel

#घोषणा MASK(n) ((1 << (n)) - 1)

#घोषणा SIMPLE_QUEUE_TAG 0x20

#घोषणा PVSCSI_INTR_CMPL_0                 (1 << 0)
#घोषणा PVSCSI_INTR_CMPL_1                 (1 << 1)
#घोषणा PVSCSI_INTR_CMPL_MASK              MASK(2)

#घोषणा PVSCSI_INTR_संदेश_0                  (1 << 2)
#घोषणा PVSCSI_INTR_संदेश_1                  (1 << 3)
#घोषणा PVSCSI_INTR_संदेश_MASK               (MASK(2) << 2)
#घोषणा PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#घोषणा PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#घोषणा PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#घोषणा PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#घोषणा PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#घोषणा PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

क्रमागत PVSCSIRegOffset {
    PVSCSI_REG_OFFSET_COMMAND        =    0x0,
    PVSCSI_REG_OFFSET_COMMAND_तथ्य   =    0x4,
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

क्रमागत PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_संदेश_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#घोषणा PVSCSI_SETUP_RINGS_MAX_अंक_PAGES        32
काष्ठा PVSCSICmdDescSetupRings {
    u32    req_Ring_अंक_Pages;
    u32    cmp_Ring_अंक_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_अंक_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_अंक_PAGES];
} PACKED;

काष्ठा PVSCSIRingCmpDesc {
    u64    context;
    u64    तथ्य_Len;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

काष्ठा PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_अंक_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_अंक_Entries_Log2;

    u8     pad[104];

    u32    संदेश_Prod_Idx;
    u32    संदेश_Cons_Idx;
    u32    संदेश_अंक_Entries_Log2;
} PACKED;

काष्ठा PVSCSIRingReqDesc {
    u64    context;
    u64    तथ्य_Addr;
    u64    तथ्य_Len;
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

काष्ठा pvscsi_ring_dsc_s {
    काष्ठा PVSCSIRingsState *ring_state;
    काष्ठा PVSCSIRingReqDesc *ring_reqs;
    काष्ठा PVSCSIRingCmpDesc *ring_cmps;
};

काष्ठा pvscsi_lun_s {
    काष्ठा drive_s drive;
    व्योम *iobase;
    u8 target;
    u8 lun;
    काष्ठा pvscsi_ring_dsc_s *ring_dsc;
};

अटल व्योम
pvscsi_write_cmd_desc(व्योम *iobase, u32 cmd, स्थिर व्योम *desc, माप_प्रकार len)
{
    स्थिर u32 *ptr = desc;
    माप_प्रकार i;

    len /= माप(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    क्रम (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_तथ्य, ptr[i]);
}

अटल व्योम
pvscsi_kick_rw_io(व्योम *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

अटल व्योम
pvscsi_wait_intr_cmpl(व्योम *iobase)
{
    जबतक (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

अटल व्योम
pvscsi_init_rings(व्योम *iobase, काष्ठा pvscsi_ring_dsc_s **ring_dsc)
{
    काष्ठा PVSCSICmdDescSetupRings cmd = {0,};

    काष्ठा pvscsi_ring_dsc_s *dsc = malloc_ऊपरी(माप(*dsc));
    अगर (!dsc) {
        warn_noalloc();
        वापस;
    }

    dsc->ring_state =
        (काष्ठा PVSCSIRingsState *)memalign_ऊपरी(PAGE_आकार, PAGE_आकार);
    dsc->ring_reqs =
        (काष्ठा PVSCSIRingReqDesc *)memalign_ऊपरी(PAGE_आकार, PAGE_आकार);
    dsc->ring_cmps =
        (काष्ठा PVSCSIRingCmpDesc *)memalign_ऊपरी(PAGE_आकार, PAGE_आकार);
    अगर (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        वापस;
    }
    स_रखो(dsc->ring_state, 0, PAGE_आकार);
    स_रखो(dsc->ring_reqs, 0, PAGE_आकार);
    स_रखो(dsc->ring_cmps, 0, PAGE_आकार);

    cmd.req_Ring_अंक_Pages = 1;
    cmd.cmp_Ring_अंक_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, माप(cmd));
    *ring_dsc = dsc;
}

अटल u32
pvscsi_get_rsp(काष्ठा PVSCSIRingsState *s,
               काष्ठा PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    वापस status;
}

पूर्णांक
pvscsi_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_PVSCSI)
        वापस DISK_RET_EBADTRACK;
    काष्ठा pvscsi_lun_s *plun =
        container_of(op->drive_fl, काष्ठा pvscsi_lun_s, drive);
    काष्ठा pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    काष्ठा PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_अंक_Entries_Log2;
    u32 cmp_entries = s->cmp_अंक_Entries_Log2;
    काष्ठा PVSCSIRingReqDesc *req;
    काष्ठा PVSCSIRingCmpDesc *rsp;
    u32 status;

    अगर (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        वापस DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    पूर्णांक blocksize = scsi_fill_cmd(op, req->cdb, 16);
    अगर (blocksize < 0)
        वापस default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    स_रखो(req->lun, 0, माप(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->तथ्य_Len = op->गिनती * blocksize;
    req->तथ्य_Addr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    वापस status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

अटल पूर्णांक
pvscsi_add_lun(काष्ठा pci_device *pci, व्योम *iobase,
               काष्ठा pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    काष्ठा pvscsi_lun_s *plun = malloc_fseg(माप(*plun));
    अगर (!plun) {
        warn_noalloc();
        वापस -1;
    }
    स_रखो(plun, 0, माप(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    अक्षर *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    पूर्णांक prio = bootprio_find_scsi_device(pci, target, lun);
    पूर्णांक ret = scsi_drive_setup(&plun->drive, name, prio);
    मुक्त(name);
    अगर (ret)
        जाओ fail;
    वापस 0;

fail:
    मुक्त(plun);
    वापस -1;
}

अटल व्योम
pvscsi_scan_target(काष्ठा pci_device *pci, व्योम *iobase,
                   काष्ठा pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

अटल व्योम
init_pvscsi(व्योम *तथ्य)
{
    काष्ठा pci_device *pci = तथ्य;
    व्योम *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    अगर (!iobase)
        वापस;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, शून्य, 0);

    काष्ठा pvscsi_ring_dsc_s *ring_dsc = शून्य;
    pvscsi_init_rings(iobase, &ring_dsc);
    पूर्णांक i;
    क्रम (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

व्योम
pvscsi_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (! CONFIG_PVSCSI)
        वापस;

    dprintf(3, "init pvscsi\n");

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            जारी;
        run_thread(init_pvscsi, pci);
    }
}
