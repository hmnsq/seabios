// QEMU VMWARE Paravirtualized SCSI boot support.
//
// Copyright (c) 2013 Ravello Systems LTD (http://ravellosystems.com)
//
// Authors:
//  Evgeny Budilovsky <evgeny.budilovsky@ravellosystems.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // scsi_drive_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" // PAGE_SHIFT, virt_to_phys
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_DEVICE_ID_VMWARE_PVSCSI
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "pvscsi.h" // pvscsi_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // run_thread
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // usleep
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // writel

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਨਕਾਬ_2(n) ((1 << (n)) - 1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIMPLE_ਕਤਾਰ_ਟੈਗ 0x20

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_INTR_CMPL_0                 (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_INTR_CMPL_1                 (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_INTR_CMPL_ਨਕਾਬ              ਨਕਾਬ_2(2)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_INTR_ਸੁਨੇਹਾ_0                  (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_INTR_ਸੁਨੇਹਾ_1                  (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_INTR_ਸੁਨੇਹਾ_ਨਕਾਬ               (ਨਕਾਬ_2(2) << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_INTR_ALL_SUPPORTED          ਨਕਾਬ_2(4)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_FLAG_CMD_ਬਾਹਰਲਾ_ਦਾ_BAND_CDB    (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

ਗਿਣਤੀ PVSCSIReg_ਵਿੱਥ {
    PVSCSI_REG_ਵਿੱਥ_COMMAND        =    0x0,
    PVSCSI_REG_ਵਿੱਥ_COMMAND_ਅੰਕੜੇ   =    0x4,
    PVSCSI_REG_ਵਿੱਥ_COMMAND_ਸਥਿਤੀ =    0x8,
    PVSCSI_REG_ਵਿੱਥ_ਆਖਰੀ_STS_0     =  0x100,
    PVSCSI_REG_ਵਿੱਥ_ਆਖਰੀ_STS_1     =  0x104,
    PVSCSI_REG_ਵਿੱਥ_ਆਖਰੀ_STS_2     =  0x108,
    PVSCSI_REG_ਵਿੱਥ_ਆਖਰੀ_STS_3     =  0x10c,
    PVSCSI_REG_ਵਿੱਥ_INTR_ਸਥਿਤੀ    = 0x100c,
    PVSCSI_REG_ਵਿੱਥ_INTR_ਨਕਾਬ      = 0x2010,
    PVSCSI_REG_ਵਿੱਥ_KICK_NON_RW_IO = 0x3014,
    PVSCSI_REG_ਵਿੱਥ_DEBUG          = 0x3018,
    PVSCSI_REG_ਵਿੱਥ_KICK_RW_IO     = 0x4018,
};

ਗਿਣਤੀ PVSCSICommands {
    PVSCSI_CMD_ਪਹਿਲਾ             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_ਤਿਆਰੀ_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_ਤਿਆਰੀ_ਸੁਨੇਹਾ_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_ਆਖਰੀ              = 10
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PVSCSI_ਤਿਆਰੀ_RINGS_MAX_ਅੰਕ_ਪੰਨੇ        32
ਬਣਤਰ PVSCSICmd_Desc_ਤਿਆਰੀ_Rings {
    u32    req_Ring_ਅੰਕ_ਪੰਨੇ;
    u32    cmp_Ring_ਅੰਕ_ਪੰਨੇ;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_ਤਿਆਰੀ_RINGS_MAX_ਅੰਕ_ਪੰਨੇ];
    u64    cmpRingPPNs[PVSCSI_ਤਿਆਰੀ_RINGS_MAX_ਅੰਕ_ਪੰਨੇ];
} PACKED;

ਬਣਤਰ PVSCSIRingCmpDesc {
    u64    context;
    u64    ਅੰਕੜੇ_Len;
    u32    senseLen;
    u16    host_ਸਥਿਤੀ;
    u16    scsi_ਸਥਿਤੀ_2;
    u32    pad[2];
} PACKED;

ਬਣਤਰ PVSCSIRingsState {
    u32    reqProdIdx;
    u32    reqConsIdx;
    u32    req_ਅੰਕ_Entries_Log2;

    u32    cmpProdIdx;
    u32    cmpConsIdx;
    u32    cmp_ਅੰਕ_Entries_Log2;

    u8     pad[104];

    u32    ਸੁਨੇਹਾ_Prod_Idx;
    u32    ਸੁਨੇਹਾ_Cons_Idx;
    u32    ਸੁਨੇਹਾ_ਅੰਕ_Entries_Log2;
} PACKED;

ਬਣਤਰ PVSCSIRingReqDesc {
    u64    context;
    u64    ਅੰਕੜੇ_Addr;
    u64    ਅੰਕੜੇ_Len;
    u64    senseAddr;
    u32    senseLen;
    u32    flags;
    u8     cdb[16];
    u8     cdbLen;
    u8     lun[8];
    u8     ਟੈਗ;
    u8     bus;
    u8     ਨਿਸ਼ਾਨਾ;
    u8     vcpuHint;
    u8     ਅਣਵਰਤਿਆ[59];
} PACKED;

ਬਣਤਰ pvscsi_ring_dsc_s {
    ਬਣਤਰ PVSCSIRingsState *ring_state;
    ਬਣਤਰ PVSCSIRingReqDesc *ring_reqs;
    ਬਣਤਰ PVSCSIRingCmpDesc *ring_cmps;
};

ਬਣਤਰ pvscsi_lun_s {
    ਬਣਤਰ drive_s drive;
    ਖਾਲੀ *iobase;
    u8 ਨਿਸ਼ਾਨਾ;
    u8 lun;
    ਬਣਤਰ pvscsi_ring_dsc_s *ring_dsc;
};

ਸਥਿਰ ਖਾਲੀ
pvscsi_write_cmd_desc(ਖਾਲੀ *iobase, u32 cmd, ਅਚਲ ਖਾਲੀ *desc, ਮਾਪ_t len)
{
    ਅਚਲ u32 *ptr = desc;
    ਮਾਪ_t i;

    len /= ਆਕਾਰ(*ptr);
    writel(iobase + PVSCSI_REG_ਵਿੱਥ_COMMAND, cmd);
    ਲਈ (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_ਵਿੱਥ_COMMAND_ਅੰਕੜੇ, ptr[i]);
}

ਸਥਿਰ ਖਾਲੀ
pvscsi_kick_rw_io(ਖਾਲੀ *iobase)
{
    writel(iobase + PVSCSI_REG_ਵਿੱਥ_KICK_RW_IO, 0);
}

ਸਥਿਰ ਖਾਲੀ
pvscsi_wait_intr_cmpl(ਖਾਲੀ *iobase)
{
    ਜਦੋਂ (!(readl(iobase + PVSCSI_REG_ਵਿੱਥ_INTR_ਸਥਿਤੀ) & PVSCSI_INTR_CMPL_ਨਕਾਬ))
        usleep(5);
    writel(iobase + PVSCSI_REG_ਵਿੱਥ_INTR_ਸਥਿਤੀ, PVSCSI_INTR_CMPL_ਨਕਾਬ);
}

ਸਥਿਰ ਖਾਲੀ
pvscsi_init_rings(ਖਾਲੀ *iobase, ਬਣਤਰ pvscsi_ring_dsc_s **ring_dsc)
{
    ਬਣਤਰ PVSCSICmd_Desc_ਤਿਆਰੀ_Rings cmd = {0,};

    ਬਣਤਰ pvscsi_ring_dsc_s *dsc = malloc_ਉੱਚਾ(ਆਕਾਰ(*dsc));
    ਜੇ (!dsc) {
        warn_noalloc();
        ਵਾਪਸ;
    }

    dsc->ring_state =
        (ਬਣਤਰ PVSCSIRingsState *)memalign_ਉੱਚਾ(ਪੰਨਾ_ਮਾਪ, ਪੰਨਾ_ਮਾਪ);
    dsc->ring_reqs =
        (ਬਣਤਰ PVSCSIRingReqDesc *)memalign_ਉੱਚਾ(ਪੰਨਾ_ਮਾਪ, ਪੰਨਾ_ਮਾਪ);
    dsc->ring_cmps =
        (ਬਣਤਰ PVSCSIRingCmpDesc *)memalign_ਉੱਚਾ(ਪੰਨਾ_ਮਾਪ, ਪੰਨਾ_ਮਾਪ);
    ਜੇ (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(dsc->ring_state, 0, ਪੰਨਾ_ਮਾਪ);
    memset(dsc->ring_reqs, 0, ਪੰਨਾ_ਮਾਪ);
    memset(dsc->ring_cmps, 0, ਪੰਨਾ_ਮਾਪ);

    cmd.req_Ring_ਅੰਕ_ਪੰਨੇ = 1;
    cmd.cmp_Ring_ਅੰਕ_ਪੰਨੇ = 1;
    cmd.ringsStatePPN = virt_ਮੰਜ਼ਿਲ_phys(dsc->ring_state) >> ਪੰਨਾ_ਖਿਸਕਾਓ;
    cmd.reqRingPPNs[0] = virt_ਮੰਜ਼ਿਲ_phys(dsc->ring_reqs) >> ਪੰਨਾ_ਖਿਸਕਾਓ;
    cmd.cmpRingPPNs[0] = virt_ਮੰਜ਼ਿਲ_phys(dsc->ring_cmps) >> ਪੰਨਾ_ਖਿਸਕਾਓ;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ਤਿਆਰੀ_RINGS,
                          &cmd, ਆਕਾਰ(cmd));
    *ring_dsc = dsc;
}

ਸਥਿਰ u32
pvscsi_get_rsp(ਬਣਤਰ PVSCSIRingsState *s,
               ਬਣਤਰ PVSCSIRingCmpDesc *rsp)
{
    u32 ਸਥਿਤੀ = rsp->host_ਸਥਿਤੀ;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    ਵਾਪਸ ਸਥਿਤੀ;
}

ਪੂਰਨ
pvscsi_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_PVSCSI)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਬਣਤਰ pvscsi_lun_s *plun =
        container_ਦਾ(op->drive_fl, ਬਣਤਰ pvscsi_lun_s, drive);
    ਬਣਤਰ pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    ਬਣਤਰ PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_ਅੰਕ_Entries_Log2;
    u32 cmp_entries = s->cmp_ਅੰਕ_Entries_Log2;
    ਬਣਤਰ PVSCSIRingReqDesc *req;
    ਬਣਤਰ PVSCSIRingCmpDesc *rsp;
    u32 ਸਥਿਤੀ;

    ਜੇ (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        ਵਾਪਸ DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & ਨਕਾਬ_2(req_entries));
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, req->cdb, 16);
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);
    req->bus = 0;
    req->ਨਿਸ਼ਾਨਾ = plun->ਨਿਸ਼ਾਨਾ;
    memset(req->lun, 0, ਆਕਾਰ(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->ਟੈਗ = SIMPLE_ਕਤਾਰ_ਟੈਗ;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->ਅੰਕੜੇ_Len = op->ਗਿਣਤੀ_2 * blocksize;
    req->ਅੰਕੜੇ_Addr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & ਨਕਾਬ_2(cmp_entries));
    ਸਥਿਤੀ = pvscsi_get_rsp(s, rsp);

    ਵਾਪਸ ਸਥਿਤੀ == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ਸਥਿਰ ਪੂਰਨ
pvscsi_ਜੋੜੋ_lun(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *iobase,
               ਬਣਤਰ pvscsi_ring_dsc_s *ring_dsc, u8 ਨਿਸ਼ਾਨਾ, u8 lun)
{
    ਬਣਤਰ pvscsi_lun_s *plun = malloc_fseg(ਆਕਾਰ(*plun));
    ਜੇ (!plun) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    memset(plun, 0, ਆਕਾਰ(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->ਨਿਸ਼ਾਨਾ = ਨਿਸ਼ਾਨਾ;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_ਲੱਭੋ_scsi_device(pci, ਨਿਸ਼ਾਨਾ, lun, &(plun->drive.lchs));
    ਅੱਖਰ *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, ਨਿਸ਼ਾਨਾ, lun);
    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_scsi_device(pci, ਨਿਸ਼ਾਨਾ, lun);
    ਪੂਰਨ ret = scsi_drive_ਤਿਆਰੀ(&plun->drive, name, prio);
    ਮੁਕਤ(name);
    ਜੇ (ret)
        ਜਾਓ fail;
    ਵਾਪਸ 0;

fail:
    ਮੁਕਤ(plun);
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਖਾਲੀ
pvscsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *iobase,
                   ਬਣਤਰ pvscsi_ring_dsc_s *ring_dsc, u8 ਨਿਸ਼ਾਨਾ)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_ਜੋੜੋ_lun(pci, iobase, ring_dsc, ਨਿਸ਼ਾਨਾ, 0);
}

ਸਥਿਰ ਖਾਲੀ
init_pvscsi(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ pci_device *pci = ਅੰਕੜੇ;
    ਖਾਲੀ *iobase = pci_enable_membar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!iobase)
        ਵਾਪਸ;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, ਨਿਰਾ, 0);

    ਬਣਤਰ pvscsi_ring_dsc_s *ring_dsc = ਨਿਰਾ;
    pvscsi_init_rings(iobase, &ring_dsc);
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < 64; i++)
        pvscsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(pci, iobase, ring_dsc, i);
}

ਖਾਲੀ
pvscsi_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (! CONFIG_PVSCSI)
        ਵਾਪਸ;

    dprintf(3, "init pvscsi\n");

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            ਜਾਰੀ;
        ਲੜੀਵਾਰ_thread(init_pvscsi, pci);
    }
}
