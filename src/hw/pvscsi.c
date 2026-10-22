// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "blockcmd.h" // scsi_drive_setup
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "memmap.h" // PAGE_SHIFT, virt_to_phys
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "pvscsi.h" // pvscsi_setup
#అన్తర్భావ "stacks.h" // run_thread
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // usleep
#అన్తర్భావ "x86.h" // writel

#నిర్వచించు ముసుగు_2(n) ((1 << (n)) - 1)

#నిర్వచించు SIMPLE_వరుస_ట్యాగ్ 0x20

#నిర్వచించు PVSCSI_INTR_CMPL_0                 (1 << 0)
#నిర్వచించు PVSCSI_INTR_CMPL_1                 (1 << 1)
#నిర్వచించు PVSCSI_INTR_CMPL_ముసుగు              ముసుగు_2(2)

#నిర్వచించు PVSCSI_INTR_సందేశం_0                  (1 << 2)
#నిర్వచించు PVSCSI_INTR_సందేశం_1                  (1 << 3)
#నిర్వచించు PVSCSI_INTR_సందేశం_ముసుగు               (ముసుగు_2(2) << 2)
#నిర్వచించు PVSCSI_INTR_ALL_SUPPORTED          ముసుగు_2(4)

#నిర్వచించు PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#నిర్వచించు PVSCSI_FLAG_CMD_బహిర్గతం_యొక్క_BAND_CDB    (1 << 1)
#నిర్వచించు PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#నిర్వచించు PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#నిర్వచించు PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

గణన PVSCSIReg_స్థానభ్రంశం {
    PVSCSI_REG_స్థానభ్రంశం_COMMAND        =    0x0,
    PVSCSI_REG_స్థానభ్రంశం_COMMAND_దత్తాంశం   =    0x4,
    PVSCSI_REG_స్థానభ్రంశం_COMMAND_స్థితి =    0x8,
    PVSCSI_REG_స్థానభ్రంశం_చివరి_STS_0     =  0x100,
    PVSCSI_REG_స్థానభ్రంశం_చివరి_STS_1     =  0x104,
    PVSCSI_REG_స్థానభ్రంశం_చివరి_STS_2     =  0x108,
    PVSCSI_REG_స్థానభ్రంశం_చివరి_STS_3     =  0x10c,
    PVSCSI_REG_స్థానభ్రంశం_INTR_స్థితి    = 0x100c,
    PVSCSI_REG_స్థానభ్రంశం_INTR_ముసుగు      = 0x2010,
    PVSCSI_REG_స్థానభ్రంశం_KICK_NON_RW_IO = 0x3014,
    PVSCSI_REG_స్థానభ్రంశం_DEBUG          = 0x3018,
    PVSCSI_REG_స్థానభ్రంశం_KICK_RW_IO     = 0x4018,
};

గణన PVSCSICommands {
    PVSCSI_CMD_మొదటి             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_అమరిక_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_అమరిక_సందేశం_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_చివరి              = 10
};

#నిర్వచించు PVSCSI_అమరిక_RINGS_MAX_అంకె_పుటలు        32
నిర్మాణం PVSCSICmd_Desc_అమరిక_Rings {
    u32    req_Ring_అంకె_పుటలు;
    u32    cmp_Ring_అంకె_పుటలు;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_అమరిక_RINGS_MAX_అంకె_పుటలు];
    u64    cmpRingPPNs[PVSCSI_అమరిక_RINGS_MAX_అంకె_పుటలు];
} PACKED;

నిర్మాణం PVSCSIRingCmpDesc {
    u64    context;
    u64    దత్తాంశం_Len;
    u32    senseLen;
    u16    host_స్థితి;
    u16    scsi_స్థితి_2;
    u32    pad[2];
} PACKED;

నిర్మాణం PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_అంకె_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_అంకె_Entries_Log2;

    u8     pad[104];

    u32    సందేశం_Prod_Idx;
    u32    సందేశం_Cons_Idx;
    u32    సందేశం_అంకె_Entries_Log2;
} PACKED;

నిర్మాణం PVSCSIRingReqDesc {
    u64    context;
    u64    దత్తాంశం_Addr;
    u64    దత్తాంశం_Len;
    u64    senseAddr;
    u32    senseLen;
    u32    flags;
    u8     cdb[16];
    u8     cdbLen;
    u8     lun[8];
    u8     ట్యాగ్;
    u8     bus;
    u8     లక్ష్యం;
    u8     vcpuHint;
    u8     వాడని[59];
} PACKED;

నిర్మాణం pvscsi_ring_dsc_s {
    నిర్మాణం PVSCSIRingsState *ring_state;
    నిర్మాణం PVSCSIRingReqDesc *ring_reqs;
    నిర్మాణం PVSCSIRingCmpDesc *ring_cmps;
};

నిర్మాణం pvscsi_lun_s {
    నిర్మాణం drive_s drive;
    శూన్య *iobase;
    u8 లక్ష్యం;
    u8 lun;
    నిర్మాణం pvscsi_ring_dsc_s *ring_dsc;
};

స్థిర శూన్య
pvscsi_write_cmd_desc(శూన్య *iobase, u32 cmd, స్థిరరాశి శూన్య *desc, కొలత_t len)
{
    స్థిరరాశి u32 *ptr = desc;
    కొలత_t i;

    len /= పరిమాణం(*ptr);
    writel(iobase + PVSCSI_REG_స్థానభ్రంశం_COMMAND, cmd);
    కొరకు (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_స్థానభ్రంశం_COMMAND_దత్తాంశం, ptr[i]);
}

స్థిర శూన్య
pvscsi_kick_rw_io(శూన్య *iobase)
{
    writel(iobase + PVSCSI_REG_స్థానభ్రంశం_KICK_RW_IO, 0);
}

స్థిర శూన్య
pvscsi_wait_intr_cmpl(శూన్య *iobase)
{
    అయితే (!(readl(iobase + PVSCSI_REG_స్థానభ్రంశం_INTR_స్థితి) & PVSCSI_INTR_CMPL_ముసుగు))
        usleep(5);
    writel(iobase + PVSCSI_REG_స్థానభ్రంశం_INTR_స్థితి, PVSCSI_INTR_CMPL_ముసుగు);
}

స్థిర శూన్య
pvscsi_init_rings(శూన్య *iobase, నిర్మాణం pvscsi_ring_dsc_s **ring_dsc)
{
    నిర్మాణం PVSCSICmd_Desc_అమరిక_Rings cmd = {0,};

    నిర్మాణం pvscsi_ring_dsc_s *dsc = malloc_ఎగువ(పరిమాణం(*dsc));
    ఒకవేళ (!dsc) {
        warn_noalloc();
        తిరిగి;
    }

    dsc->ring_state =
        (నిర్మాణం PVSCSIRingsState *)memalign_ఎగువ(పుట_కొలత, పుట_కొలత);
    dsc->ring_reqs =
        (నిర్మాణం PVSCSIRingReqDesc *)memalign_ఎగువ(పుట_కొలత, పుట_కొలత);
    dsc->ring_cmps =
        (నిర్మాణం PVSCSIRingCmpDesc *)memalign_ఎగువ(పుట_కొలత, పుట_కొలత);
    ఒకవేళ (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        తిరిగి;
    }
    memset(dsc->ring_state, 0, పుట_కొలత);
    memset(dsc->ring_reqs, 0, పుట_కొలత);
    memset(dsc->ring_cmps, 0, పుట_కొలత);

    cmd.req_Ring_అంకె_పుటలు = 1;
    cmd.cmp_Ring_అంకె_పుటలు = 1;
    cmd.ringsStatePPN = virt_గమ్యం_phys(dsc->ring_state) >> పుట_జరుపు;
    cmd.reqRingPPNs[0] = virt_గమ్యం_phys(dsc->ring_reqs) >> పుట_జరుపు;
    cmd.cmpRingPPNs[0] = virt_గమ్యం_phys(dsc->ring_cmps) >> పుట_జరుపు;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_అమరిక_RINGS,
                          &cmd, పరిమాణం(cmd));
    *ring_dsc = dsc;
}

స్థిర u32
pvscsi_get_rsp(నిర్మాణం PVSCSIRingsState *s,
               నిర్మాణం PVSCSIRingCmpDesc *rsp)
{
    u32 స్థితి_2 = rsp->host_స్థితి;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    తిరిగి స్థితి_2;
}

పూర్ణాంకం
pvscsi_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_PVSCSI)
        తిరిగి DISK_RET_EBADTRACK;
    నిర్మాణం pvscsi_lun_s *plun =
        container_యొక్క(op->drive_fl, నిర్మాణం pvscsi_lun_s, drive);
    నిర్మాణం pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    నిర్మాణం PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_అంకె_Entries_Log2;
    u32 cmp_entries = s->cmp_అంకె_Entries_Log2;
    నిర్మాణం PVSCSIRingReqDesc *req;
    నిర్మాణం PVSCSIRingCmpDesc *rsp;
    u32 స్థితి_2;

    ఒకవేళ (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        తిరిగి DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & ముసుగు_2(req_entries));
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, req->cdb, 16);
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);
    req->bus = 0;
    req->లక్ష్యం = plun->లక్ష్యం;
    memset(req->lun, 0, పరిమాణం(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->ట్యాగ్ = SIMPLE_వరుస_ట్యాగ్;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->దత్తాంశం_Len = op->లెక్క * blocksize;
    req->దత్తాంశం_Addr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & ముసుగు_2(cmp_entries));
    స్థితి_2 = pvscsi_get_rsp(s, rsp);

    తిరిగి స్థితి_2 == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

స్థిర పూర్ణాంకం
pvscsi_కూడు_lun(నిర్మాణం pci_device *pci, శూన్య *iobase,
               నిర్మాణం pvscsi_ring_dsc_s *ring_dsc, u8 లక్ష్యం, u8 lun)
{
    నిర్మాణం pvscsi_lun_s *plun = malloc_fseg(పరిమాణం(*plun));
    ఒకవేళ (!plun) {
        warn_noalloc();
        తిరిగి -1;
    }
    memset(plun, 0, పరిమాణం(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->లక్ష్యం = లక్ష్యం;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_కనుగొను_scsi_device(pci, లక్ష్యం, lun, &(plun->drive.lchs));
    అక్షరం *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, లక్ష్యం, lun);
    పూర్ణాంకం prio = bootprio_కనుగొను_scsi_device(pci, లక్ష్యం, lun);
    పూర్ణాంకం ret = scsi_drive_అమరిక(&plun->drive, name, prio);
    విడుదల(name);
    ఒకవేళ (ret)
        వెళ్ళు fail;
    తిరిగి 0;

fail:
    విడుదల(plun);
    తిరిగి -1;
}

స్థిర శూన్య
pvscsi_పరిశీలన_లక్ష్యం(నిర్మాణం pci_device *pci, శూన్య *iobase,
                   నిర్మాణం pvscsi_ring_dsc_s *ring_dsc, u8 లక్ష్యం)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_కూడు_lun(pci, iobase, ring_dsc, లక్ష్యం, 0);
}

స్థిర శూన్య
init_pvscsi(శూన్య *దత్తాంశం)
{
    నిర్మాణం pci_device *pci = దత్తాంశం;
    శూన్య *iobase = pci_enable_membar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!iobase)
        తిరిగి;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, ఖాళీ, 0);

    నిర్మాణం pvscsi_ring_dsc_s *ring_dsc = ఖాళీ;
    pvscsi_init_rings(iobase, &ring_dsc);
    పూర్ణాంకం i;
    కొరకు (i = 0; i < 64; i++)
        pvscsi_పరిశీలన_లక్ష్యం(pci, iobase, ring_dsc, i);
}

శూన్య
pvscsi_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_PVSCSI)
        తిరిగి;

    dprintf(3, "init pvscsi\n");

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            కొనసాగు;
        నిరంతరం_thread(init_pvscsi, pci);
    }
}
