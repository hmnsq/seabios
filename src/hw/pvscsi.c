// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "blockcmd.h" // scsi_drive_setup
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "memmap.h" // PAGE_SHIFT, virt_to_phys
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#अन्तर्भाव "pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "pvscsi.h" // pvscsi_setup
#अन्तर्भाव "stacks.h" // run_thread
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // usleep
#अन्तर्भाव "x86.h" // writel

#define MASK(n) ((1 << (n)) - 1)

#define SIMPLE_QUEUE_TAG 0x20

#define PVSCSI_INTR_CMPL_0                 (1 << 0)
#define PVSCSI_INTR_CMPL_1                 (1 << 1)
#define PVSCSI_INTR_CMPL_MASK              MASK(2)

#define PVSCSI_INTR_सन्देशः_0                  (1 << 2)
#define PVSCSI_INTR_सन्देशः_1                  (1 << 3)
#define PVSCSI_INTR_सन्देशः_MASK               (MASK(2) << 2)
#define PVSCSI_INTR_ALL_SUPPORTED          MASK(4)

#define PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#define PVSCSI_FLAG_CMD_OUT_OF_BAND_CDB    (1 << 1)
#define PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#define PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#define PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

गणना PVSCSIRegOffset {
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

गणना PVSCSICommands {
    PVSCSI_CMD_FIRST             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_SETUP_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_SETUP_सन्देशः_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_LAST              = 10
};

#define PVSCSI_SETUP_RINGS_MAX_अङ्कः_PAGES        32
संरचना PVSCSICmdDescSetupRings {
    u32    req_Ring_अङ्कः_Pages;
    u32    cmp_Ring_अङ्कः_Pages;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_SETUP_RINGS_MAX_अङ्कः_PAGES];
    u64    cmpRingPPNs[PVSCSI_SETUP_RINGS_MAX_अङ्कः_PAGES];
} PACKED;

संरचना PVSCSIRingCmpDesc {
    u64    context;
    u64    dataLen;
    u32    senseLen;
    u16    hostStatus;
    u16    scsiStatus;
    u32    pad[2];
} PACKED;

संरचना PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_अङ्कः_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_अङ्कः_Entries_Log2;

    u8     pad[104];

    u32    सन्देशः_Prod_Idx;
    u32    सन्देशः_Cons_Idx;
    u32    सन्देशः_अङ्कः_Entries_Log2;
} PACKED;

संरचना PVSCSIRingReqDesc {
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

संरचना pvscsi_ring_dsc_s {
    संरचना PVSCSIRingsState *ring_state;
    संरचना PVSCSIRingReqDesc *ring_reqs;
    संरचना PVSCSIRingCmpDesc *ring_cmps;
};

संरचना pvscsi_lun_s {
    संरचना drive_s drive;
    शून्य *iobase;
    u8 target;
    u8 lun;
    संरचना pvscsi_ring_dsc_s *ring_dsc;
};

स्थिर शून्य
pvscsi_write_cmd_desc(शून्य *iobase, u32 cmd, ध्रुव शून्य *desc, size_t len)
{
    ध्रुव u32 *ptr = desc;
    size_t i;

    len /= परिमाण(*ptr);
    writel(iobase + PVSCSI_REG_OFFSET_COMMAND, cmd);
    प्रति (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_OFFSET_COMMAND_DATA, ptr[i]);
}

स्थिर शून्य
pvscsi_kick_rw_io(शून्य *iobase)
{
    writel(iobase + PVSCSI_REG_OFFSET_KICK_RW_IO, 0);
}

स्थिर शून्य
pvscsi_wait_intr_cmpl(शून्य *iobase)
{
    जाव (!(readl(iobase + PVSCSI_REG_OFFSET_INTR_STATUS) & PVSCSI_INTR_CMPL_MASK))
        usleep(5);
    writel(iobase + PVSCSI_REG_OFFSET_INTR_STATUS, PVSCSI_INTR_CMPL_MASK);
}

स्थिर शून्य
pvscsi_init_rings(शून्य *iobase, संरचना pvscsi_ring_dsc_s **ring_dsc)
{
    संरचना PVSCSICmdDescSetupRings cmd = {0,};

    संरचना pvscsi_ring_dsc_s *dsc = malloc_उच्चः(परिमाण(*dsc));
    जइ (!dsc) {
        warn_noalloc();
        प्रत्यागम;
    }

    dsc->ring_state =
        (संरचना PVSCSIRingsState *)memalign_उच्चः(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_reqs =
        (संरचना PVSCSIRingReqDesc *)memalign_उच्चः(PAGE_SIZE, PAGE_SIZE);
    dsc->ring_cmps =
        (संरचना PVSCSIRingCmpDesc *)memalign_उच्चः(PAGE_SIZE, PAGE_SIZE);
    जइ (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        प्रत्यागम;
    }
    memset(dsc->ring_state, 0, PAGE_SIZE);
    memset(dsc->ring_reqs, 0, PAGE_SIZE);
    memset(dsc->ring_cmps, 0, PAGE_SIZE);

    cmd.req_Ring_अङ्कः_Pages = 1;
    cmd.cmp_Ring_अङ्कः_Pages = 1;
    cmd.ringsStatePPN = virt_to_phys(dsc->ring_state) >> PAGE_SHIFT;
    cmd.reqRingPPNs[0] = virt_to_phys(dsc->ring_reqs) >> PAGE_SHIFT;
    cmd.cmpRingPPNs[0] = virt_to_phys(dsc->ring_cmps) >> PAGE_SHIFT;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_SETUP_RINGS,
                          &cmd, परिमाण(cmd));
    *ring_dsc = dsc;
}

स्थिर u32
pvscsi_get_rsp(संरचना PVSCSIRingsState *s,
               संरचना PVSCSIRingCmpDesc *rsp)
{
    u32 status = rsp->hostStatus;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    प्रत्यागम status;
}

पूर्णाङ्क
pvscsi_process_op(संरचना disk_op_s *op)
{
    जइ (!CONFIG_PVSCSI)
        प्रत्यागम DISK_RET_EBADTRACK;
    संरचना pvscsi_lun_s *plun =
        container_of(op->drive_fl, संरचना pvscsi_lun_s, drive);
    संरचना pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    संरचना PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_अङ्कः_Entries_Log2;
    u32 cmp_entries = s->cmp_अङ्कः_Entries_Log2;
    संरचना PVSCSIRingReqDesc *req;
    संरचना PVSCSIRingCmpDesc *rsp;
    u32 status;

    जइ (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        प्रत्यागम DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & MASK(req_entries));
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, req->cdb, 16);
    जइ (blocksize < 0)
        प्रत्यागम default_process_op(op);
    req->bus = 0;
    req->target = plun->target;
    memset(req->lun, 0, परिमाण(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->tag = SIMPLE_QUEUE_TAG;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->dataLen = op->गणनम् * blocksize;
    req->dataAddr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & MASK(cmp_entries));
    status = pvscsi_get_rsp(s, rsp);

    प्रत्यागम status == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

स्थिर पूर्णाङ्क
pvscsi_add_lun(संरचना pci_device *pci, शून्य *iobase,
               संरचना pvscsi_ring_dsc_s *ring_dsc, u8 target, u8 lun)
{
    संरचना pvscsi_lun_s *plun = malloc_fseg(परिमाण(*plun));
    जइ (!plun) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    memset(plun, 0, परिमाण(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->target = target;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_find_scsi_device(pci, target, lun, &(plun->drive.lchs));
    वर्ण *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, target, lun);
    पूर्णाङ्क prio = bootprio_find_scsi_device(pci, target, lun);
    पूर्णाङ्क ret = scsi_drive_setup(&plun->drive, name, prio);
    मुक्त(name);
    जइ (ret)
        गच्छ fail;
    प्रत्यागम 0;

fail:
    मुक्त(plun);
    प्रत्यागम -1;
}

स्थिर शून्य
pvscsi_scan_target(संरचना pci_device *pci, शून्य *iobase,
                   संरचना pvscsi_ring_dsc_s *ring_dsc, u8 target)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_add_lun(pci, iobase, ring_dsc, target, 0);
}

स्थिर शून्य
init_pvscsi(शून्य *data)
{
    संरचना pci_device *pci = data;
    शून्य *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    जइ (!iobase)
        प्रत्यागम;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, रिक्त, 0);

    संरचना pvscsi_ring_dsc_s *ring_dsc = रिक्त;
    pvscsi_init_rings(iobase, &ring_dsc);
    पूर्णाङ्क i;
    प्रति (i = 0; i < 64; i++)
        pvscsi_scan_target(pci, iobase, ring_dsc, i);
}

शून्य
pvscsi_setup(शून्य)
{
    ASSERT32FLAT();
    जइ (! CONFIG_PVSCSI)
        प्रत्यागम;

    dprintf(3, "init pvscsi\n");

    संरचना pci_device *pci;
    foreachpci(pci) {
        जइ (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            अनुवर्त;
        run_thread(init_pvscsi, pci);
    }
}
