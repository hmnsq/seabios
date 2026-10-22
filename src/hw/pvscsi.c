// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // scsi_drive_setup
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "memmap.h" // PAGE_SHIFT, virt_to_phys
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "pvscsi.h" // pvscsi_setup
#شامل "stacks.h" // run_thread
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // usleep
#شامل "x86.h" // writel

#تعریف_کرو نقاب_2(n) ((1 << (n)) - 1)

#تعریف_کرو SIMPLE_قطار_لیبل 0x20

#تعریف_کرو PVSCSI_INTR_CMPL_0                 (1 << 0)
#تعریف_کرو PVSCSI_INTR_CMPL_1                 (1 << 1)
#تعریف_کرو PVSCSI_INTR_CMPL_نقاب              نقاب_2(2)

#تعریف_کرو PVSCSI_INTR_پیغام_0                  (1 << 2)
#تعریف_کرو PVSCSI_INTR_پیغام_1                  (1 << 3)
#تعریف_کرو PVSCSI_INTR_پیغام_نقاب               (نقاب_2(2) << 2)
#تعریف_کرو PVSCSI_INTR_ALL_SUPPORTED          نقاب_2(4)

#تعریف_کرو PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#تعریف_کرو PVSCSI_FLAG_CMD_برآمد_کا_BAND_CDB    (1 << 1)
#تعریف_کرو PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#تعریف_کرو PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#تعریف_کرو PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

شمار PVSCSIReg_انحراف {
    PVSCSI_REG_انحراف_COMMAND        =    0x0,
    PVSCSI_REG_انحراف_COMMAND_ڈیٹا   =    0x4,
    PVSCSI_REG_انحراف_COMMAND_حالت =    0x8,
    PVSCSI_REG_انحراف_آخری_STS_0     =  0x100,
    PVSCSI_REG_انحراف_آخری_STS_1     =  0x104,
    PVSCSI_REG_انحراف_آخری_STS_2     =  0x108,
    PVSCSI_REG_انحراف_آخری_STS_3     =  0x10c,
    PVSCSI_REG_انحراف_INTR_حالت    = 0x100c,
    PVSCSI_REG_انحراف_INTR_نقاب      = 0x2010,
    PVSCSI_REG_انحراف_KICK_NON_RW_IO = 0x3014,
    PVSCSI_REG_انحراف_DEBUG          = 0x3018,
    PVSCSI_REG_انحراف_KICK_RW_IO     = 0x4018,
};

شمار PVSCSICommands {
    PVSCSI_CMD_پہلا             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_تیاری_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_تیاری_پیغام_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_آخری              = 10
};

#تعریف_کرو PVSCSI_تیاری_RINGS_MAX_عدد_صفحات        32
ساخت PVSCSICmd_Desc_تیاری_Rings {
    u32    req_Ring_عدد_صفحات;
    u32    cmp_Ring_عدد_صفحات;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_تیاری_RINGS_MAX_عدد_صفحات];
    u64    cmpRingPPNs[PVSCSI_تیاری_RINGS_MAX_عدد_صفحات];
} PACKED;

ساخت PVSCSIRingCmpDesc {
    u64    context;
    u64    ڈیٹا_Len;
    u32    senseLen;
    u16    host_حالت;
    u16    scsi_حالت_2;
    u32    pad[2];
} PACKED;

ساخت PVSCSIRingsState {
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

ساخت PVSCSIRingReqDesc {
    u64    context;
    u64    ڈیٹا_Addr;
    u64    ڈیٹا_Len;
    u64    senseAddr;
    u32    senseLen;
    u32    flags;
    u8     cdb[16];
    u8     cdbLen;
    u8     lun[8];
    u8     لیبل;
    u8     bus;
    u8     ہدف;
    u8     vcpuHint;
    u8     غیر_مستعمل[59];
} PACKED;

ساخت pvscsi_ring_dsc_s {
    ساخت PVSCSIRingsState *ring_state;
    ساخت PVSCSIRingReqDesc *ring_reqs;
    ساخت PVSCSIRingCmpDesc *ring_cmps;
};

ساخت pvscsi_lun_s {
    ساخت drive_s drive;
    خالی *iobase;
    u8 ہدف;
    u8 lun;
    ساخت pvscsi_ring_dsc_s *ring_dsc;
};

ساکن خالی
pvscsi_write_cmd_desc(خالی *iobase, u32 cmd, ثابت خالی *desc, سائز_t len)
{
    ثابت u32 *ptr = desc;
    سائز_t i;

    len /= حجم(*ptr);
    writel(iobase + PVSCSI_REG_انحراف_COMMAND, cmd);
    برائے (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_انحراف_COMMAND_ڈیٹا, ptr[i]);
}

ساکن خالی
pvscsi_kick_rw_io(خالی *iobase)
{
    writel(iobase + PVSCSI_REG_انحراف_KICK_RW_IO, 0);
}

ساکن خالی
pvscsi_wait_intr_cmpl(خالی *iobase)
{
    جب (!(readl(iobase + PVSCSI_REG_انحراف_INTR_حالت) & PVSCSI_INTR_CMPL_نقاب))
        usleep(5);
    writel(iobase + PVSCSI_REG_انحراف_INTR_حالت, PVSCSI_INTR_CMPL_نقاب);
}

ساکن خالی
pvscsi_init_rings(خالی *iobase, ساخت pvscsi_ring_dsc_s **ring_dsc)
{
    ساخت PVSCSICmd_Desc_تیاری_Rings cmd = {0,};

    ساخت pvscsi_ring_dsc_s *dsc = malloc_بلند(حجم(*dsc));
    اگر (!dsc) {
        warn_noalloc();
        واپس;
    }

    dsc->ring_state =
        (ساخت PVSCSIRingsState *)memalign_بلند(صفحہ_سائز, صفحہ_سائز);
    dsc->ring_reqs =
        (ساخت PVSCSIRingReqDesc *)memalign_بلند(صفحہ_سائز, صفحہ_سائز);
    dsc->ring_cmps =
        (ساخت PVSCSIRingCmpDesc *)memalign_بلند(صفحہ_سائز, صفحہ_سائز);
    اگر (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        واپس;
    }
    memset(dsc->ring_state, 0, صفحہ_سائز);
    memset(dsc->ring_reqs, 0, صفحہ_سائز);
    memset(dsc->ring_cmps, 0, صفحہ_سائز);

    cmd.req_Ring_عدد_صفحات = 1;
    cmd.cmp_Ring_عدد_صفحات = 1;
    cmd.ringsStatePPN = virt_مقصد_phys(dsc->ring_state) >> صفحہ_سرکاؤ;
    cmd.reqRingPPNs[0] = virt_مقصد_phys(dsc->ring_reqs) >> صفحہ_سرکاؤ;
    cmd.cmpRingPPNs[0] = virt_مقصد_phys(dsc->ring_cmps) >> صفحہ_سرکاؤ;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_تیاری_RINGS,
                          &cmd, حجم(cmd));
    *ring_dsc = dsc;
}

ساکن u32
pvscsi_get_rsp(ساخت PVSCSIRingsState *s,
               ساخت PVSCSIRingCmpDesc *rsp)
{
    u32 حالت_2 = rsp->host_حالت;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    واپس حالت_2;
}

صحیح
pvscsi_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_PVSCSI)
        واپس DISK_RET_EBADTRACK;
    ساخت pvscsi_lun_s *plun =
        container_کا(op->drive_fl, ساخت pvscsi_lun_s, drive);
    ساخت pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    ساخت PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_عدد_Entries_Log2;
    u32 cmp_entries = s->cmp_عدد_Entries_Log2;
    ساخت PVSCSIRingReqDesc *req;
    ساخت PVSCSIRingCmpDesc *rsp;
    u32 حالت_2;

    اگر (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        واپس DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & نقاب_2(req_entries));
    صحیح blocksize = scsi_fill_cmd(op, req->cdb, 16);
    اگر (blocksize < 0)
        واپس default_process_op(op);
    req->bus = 0;
    req->ہدف = plun->ہدف;
    memset(req->lun, 0, حجم(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->لیبل = SIMPLE_قطار_لیبل;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->ڈیٹا_Len = op->گنتی * blocksize;
    req->ڈیٹا_Addr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & نقاب_2(cmp_entries));
    حالت_2 = pvscsi_get_rsp(s, rsp);

    واپس حالت_2 == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ساکن صحیح
pvscsi_جوڑو_lun(ساخت pci_device *pci, خالی *iobase,
               ساخت pvscsi_ring_dsc_s *ring_dsc, u8 ہدف, u8 lun)
{
    ساخت pvscsi_lun_s *plun = malloc_fseg(حجم(*plun));
    اگر (!plun) {
        warn_noalloc();
        واپس -1;
    }
    memset(plun, 0, حجم(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->ہدف = ہدف;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_ڈھونڈو_scsi_device(pci, ہدف, lun, &(plun->drive.lchs));
    حرف *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, ہدف, lun);
    صحیح prio = bootprio_ڈھونڈو_scsi_device(pci, ہدف, lun);
    صحیح ret = scsi_drive_تیاری(&plun->drive, name, prio);
    آزاد(name);
    اگر (ret)
        جاؤ fail;
    واپس 0;

fail:
    آزاد(plun);
    واپس -1;
}

ساکن خالی
pvscsi_جائزہ_ہدف(ساخت pci_device *pci, خالی *iobase,
                   ساخت pvscsi_ring_dsc_s *ring_dsc, u8 ہدف)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_جوڑو_lun(pci, iobase, ring_dsc, ہدف, 0);
}

ساکن خالی
init_pvscsi(خالی *ڈیٹا)
{
    ساخت pci_device *pci = ڈیٹا;
    خالی *iobase = pci_enable_membar(pci, PCI_بنیاد_پتہ_0);
    اگر (!iobase)
        واپس;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, تہی, 0);

    ساخت pvscsi_ring_dsc_s *ring_dsc = تہی;
    pvscsi_init_rings(iobase, &ring_dsc);
    صحیح i;
    برائے (i = 0; i < 64; i++)
        pvscsi_جائزہ_ہدف(pci, iobase, ring_dsc, i);
}

خالی
pvscsi_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (! CONFIG_PVSCSI)
        واپس;

    dprintf(3, "init pvscsi\n");

    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            جاری;
        تسلسل_thread(init_pvscsi, pci);
    }
}
