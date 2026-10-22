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

#ವ್ಯಾಖ್ಯಾನಿಸು ಮುಖವಾಡ_2(n) ((1 << (n)) - 1)

#ವ್ಯಾಖ್ಯಾನಿಸು SIMPLE_ಸರತಿ_ಟ್ಯಾಗ್ 0x20

#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_INTR_CMPL_0                 (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_INTR_CMPL_1                 (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_INTR_CMPL_ಮುಖವಾಡ              ಮುಖವಾಡ_2(2)

#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_INTR_ಸಂದೇಶ_0                  (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_INTR_ಸಂದೇಶ_1                  (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_INTR_ಸಂದೇಶ_ಮುಖವಾಡ               (ಮುಖವಾಡ_2(2) << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_INTR_ALL_SUPPORTED          ಮುಖವಾಡ_2(4)

#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_FLAG_CMD_WITH_SG_LIST       (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_FLAG_CMD_ಹೊರಹೋಗು_ದ_BAND_CDB    (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_FLAG_CMD_DIR_NONE           (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_FLAG_CMD_DIR_TOHOST         (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_FLAG_CMD_DIR_TODEVICE       (1 << 4)

ಎಣಿಕೆ PVSCSIReg_ಸ್ಥಾನಾಂತರ {
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_COMMAND        =    0x0,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_COMMAND_ದತ್ತಾಂಶ   =    0x4,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_COMMAND_ಸ್ಥಿತಿ =    0x8,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_ಕೊನೆಯ_STS_0     =  0x100,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_ಕೊನೆಯ_STS_1     =  0x104,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_ಕೊನೆಯ_STS_2     =  0x108,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_ಕೊನೆಯ_STS_3     =  0x10c,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_INTR_ಸ್ಥಿತಿ    = 0x100c,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_INTR_ಮುಖವಾಡ      = 0x2010,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_KICK_NON_RW_IO = 0x3014,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_DEBUG          = 0x3018,
    PVSCSI_REG_ಸ್ಥಾನಾಂತರ_KICK_RW_IO     = 0x4018,
};

ಎಣಿಕೆ PVSCSICommands {
    PVSCSI_CMD_ಮೊದಲ             = 0,
    PVSCSI_CMD_ADAPTER_RESET     = 1,
    PVSCSI_CMD_ISSUE_SCSI        = 2,
    PVSCSI_CMD_ಸಿದ್ಧತೆ_RINGS       = 3,
    PVSCSI_CMD_RESET_BUS         = 4,
    PVSCSI_CMD_RESET_DEVICE      = 5,
    PVSCSI_CMD_ABORT_CMD         = 6,
    PVSCSI_CMD_CONFIG            = 7,
    PVSCSI_CMD_ಸಿದ್ಧತೆ_ಸಂದೇಶ_RING    = 8,
    PVSCSI_CMD_DEVICE_UNPLUG     = 9,
    PVSCSI_CMD_ಕೊನೆಯ              = 10
};

#ವ್ಯಾಖ್ಯಾನಿಸು PVSCSI_ಸಿದ್ಧತೆ_RINGS_MAX_ಅಂಕೆ_ಪುಟಗಳು        32
ರಚನೆ PVSCSICmd_Desc_ಸಿದ್ಧತೆ_Rings {
    u32    req_Ring_ಅಂಕೆ_ಪುಟಗಳು;
    u32    cmp_Ring_ಅಂಕೆ_ಪುಟಗಳು;
    u64    ringsStatePPN;
    u64    reqRingPPNs[PVSCSI_ಸಿದ್ಧತೆ_RINGS_MAX_ಅಂಕೆ_ಪುಟಗಳು];
    u64    cmpRingPPNs[PVSCSI_ಸಿದ್ಧತೆ_RINGS_MAX_ಅಂಕೆ_ಪುಟಗಳು];
} PACKED;

ರಚನೆ PVSCSIRingCmpDesc {
    u64    context;
    u64    ದತ್ತಾಂಶ_Len;
    u32    senseLen;
    u16    host_ಸ್ಥಿತಿ;
    u16    scsi_ಸ್ಥಿತಿ_2;
    u32    pad[2];
} PACKED;

ರಚನೆ PVSCSIRingsState {
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

ರಚನೆ PVSCSIRingReqDesc {
    u64    context;
    u64    ದತ್ತಾಂಶ_Addr;
    u64    ದತ್ತಾಂಶ_Len;
    u64    senseAddr;
    u32    senseLen;
    u32    flags;
    u8     cdb[16];
    u8     cdbLen;
    u8     lun[8];
    u8     ಟ್ಯಾಗ್;
    u8     bus;
    u8     ಗುರಿ;
    u8     vcpuHint;
    u8     ಬಳಸದ[59];
} PACKED;

ರಚನೆ pvscsi_ring_dsc_s {
    ರಚನೆ PVSCSIRingsState *ring_state;
    ರಚನೆ PVSCSIRingReqDesc *ring_reqs;
    ರಚನೆ PVSCSIRingCmpDesc *ring_cmps;
};

ರಚನೆ pvscsi_lun_s {
    ರಚನೆ drive_s drive;
    ಶೂನ್ಯ *iobase;
    u8 ಗುರಿ;
    u8 lun;
    ರಚನೆ pvscsi_ring_dsc_s *ring_dsc;
};

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_write_cmd_desc(ಶೂನ್ಯ *iobase, u32 cmd, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *desc, ಗಾತ್ರ_t len)
{
    ಸ್ಥಿರಾಂಕ u32 *ptr = desc;
    ಗಾತ್ರ_t i;

    len /= ಗಾತ್ರ(*ptr);
    writel(iobase + PVSCSI_REG_ಸ್ಥಾನಾಂತರ_COMMAND, cmd);
    ಗಾಗಿ (i = 0; i < len; i++)
        writel(iobase + PVSCSI_REG_ಸ್ಥಾನಾಂತರ_COMMAND_ದತ್ತಾಂಶ, ptr[i]);
}

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_kick_rw_io(ಶೂನ್ಯ *iobase)
{
    writel(iobase + PVSCSI_REG_ಸ್ಥಾನಾಂತರ_KICK_RW_IO, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_wait_intr_cmpl(ಶೂನ್ಯ *iobase)
{
    ಆಗ (!(readl(iobase + PVSCSI_REG_ಸ್ಥಾನಾಂತರ_INTR_ಸ್ಥಿತಿ) & PVSCSI_INTR_CMPL_ಮುಖವಾಡ))
        usleep(5);
    writel(iobase + PVSCSI_REG_ಸ್ಥಾನಾಂತರ_INTR_ಸ್ಥಿತಿ, PVSCSI_INTR_CMPL_ಮುಖವಾಡ);
}

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_init_rings(ಶೂನ್ಯ *iobase, ರಚನೆ pvscsi_ring_dsc_s **ring_dsc)
{
    ರಚನೆ PVSCSICmd_Desc_ಸಿದ್ಧತೆ_Rings cmd = {0,};

    ರಚನೆ pvscsi_ring_dsc_s *dsc = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*dsc));
    ಆದರೆ (!dsc) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }

    dsc->ring_state =
        (ರಚನೆ PVSCSIRingsState *)memalign_ಮೇಲಿನ(ಪುಟ_ಗಾತ್ರ, ಪುಟ_ಗಾತ್ರ);
    dsc->ring_reqs =
        (ರಚನೆ PVSCSIRingReqDesc *)memalign_ಮೇಲಿನ(ಪುಟ_ಗಾತ್ರ, ಪುಟ_ಗಾತ್ರ);
    dsc->ring_cmps =
        (ರಚನೆ PVSCSIRingCmpDesc *)memalign_ಮೇಲಿನ(ಪುಟ_ಗಾತ್ರ, ಪುಟ_ಗಾತ್ರ);
    ಆದರೆ (!dsc->ring_state || !dsc->ring_reqs || !dsc->ring_cmps) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(dsc->ring_state, 0, ಪುಟ_ಗಾತ್ರ);
    memset(dsc->ring_reqs, 0, ಪುಟ_ಗಾತ್ರ);
    memset(dsc->ring_cmps, 0, ಪುಟ_ಗಾತ್ರ);

    cmd.req_Ring_ಅಂಕೆ_ಪುಟಗಳು = 1;
    cmd.cmp_Ring_ಅಂಕೆ_ಪುಟಗಳು = 1;
    cmd.ringsStatePPN = virt_ಗಮ್ಯ_phys(dsc->ring_state) >> ಪುಟ_ಸರಿಸು;
    cmd.reqRingPPNs[0] = virt_ಗಮ್ಯ_phys(dsc->ring_reqs) >> ಪುಟ_ಸರಿಸು;
    cmd.cmpRingPPNs[0] = virt_ಗಮ್ಯ_phys(dsc->ring_cmps) >> ಪುಟ_ಸರಿಸು;

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ಸಿದ್ಧತೆ_RINGS,
                          &cmd, ಗಾತ್ರ(cmd));
    *ring_dsc = dsc;
}

ಸ್ಥಿರ u32
pvscsi_get_rsp(ರಚನೆ PVSCSIRingsState *s,
               ರಚನೆ PVSCSIRingCmpDesc *rsp)
{
    u32 ಸ್ಥಿತಿ_2 = rsp->host_ಸ್ಥಿತಿ;
    s->cmpConsIdx = s->cmpConsIdx + 1;
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

ಪೂರ್ಣಾಂಕ
pvscsi_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_PVSCSI)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ರಚನೆ pvscsi_lun_s *plun =
        container_ದ(op->drive_fl, ರಚನೆ pvscsi_lun_s, drive);
    ರಚನೆ pvscsi_ring_dsc_s *ring_dsc = plun->ring_dsc;
    ರಚನೆ PVSCSIRingsState *s = ring_dsc->ring_state;
    u32 req_entries = s->req_ಅಂಕೆ_Entries_Log2;
    u32 cmp_entries = s->cmp_ಅಂಕೆ_Entries_Log2;
    ರಚನೆ PVSCSIRingReqDesc *req;
    ರಚನೆ PVSCSIRingCmpDesc *rsp;
    u32 ಸ್ಥಿತಿ_2;

    ಆದರೆ (s->reqProdIdx - s->cmpConsIdx >= 1 << req_entries) {
        dprintf(1, "pvscsi: ring full: reqProdIdx=%d cmpConsIdx=%d\n",
                s->reqProdIdx, s->cmpConsIdx);
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    }

    req = ring_dsc->ring_reqs + (s->reqProdIdx & ಮುಖವಾಡ_2(req_entries));
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, req->cdb, 16);
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);
    req->bus = 0;
    req->ಗುರಿ = plun->ಗುರಿ;
    memset(req->lun, 0, ಗಾತ್ರ(req->lun));
    req->lun[1] = plun->lun;
    req->senseLen = 0;
    req->senseAddr = 0;
    req->cdbLen = 16;
    req->vcpuHint = 0;
    req->ಟ್ಯಾಗ್ = SIMPLE_ಸರತಿ_ಟ್ಯಾಗ್;
    req->flags = scsi_is_read(op) ?
        PVSCSI_FLAG_CMD_DIR_TOHOST : PVSCSI_FLAG_CMD_DIR_TODEVICE;
    req->ದತ್ತಾಂಶ_Len = op->ಎಣಿಕೆ_2 * blocksize;
    req->ದತ್ತಾಂಶ_Addr = (u32)op->buf_fl;
    s->reqProdIdx = s->reqProdIdx + 1;

    pvscsi_kick_rw_io(plun->iobase);
    pvscsi_wait_intr_cmpl(plun->iobase);

    rsp = ring_dsc->ring_cmps + (s->cmpConsIdx & ಮುಖವಾಡ_2(cmp_entries));
    ಸ್ಥಿತಿ_2 = pvscsi_get_rsp(s, rsp);

    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 == 0 ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
pvscsi_ಕೂಡಿಸು_lun(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *iobase,
               ರಚನೆ pvscsi_ring_dsc_s *ring_dsc, u8 ಗುರಿ, u8 lun)
{
    ರಚನೆ pvscsi_lun_s *plun = malloc_fseg(ಗಾತ್ರ(*plun));
    ಆದರೆ (!plun) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    memset(plun, 0, ಗಾತ್ರ(*plun));
    plun->drive.type = DTYPE_PVSCSI;
    plun->drive.cntl_id = pci->bdf;
    plun->ಗುರಿ = ಗುರಿ;
    plun->lun = lun;
    plun->iobase = iobase;
    plun->ring_dsc = ring_dsc;

    boot_lchs_ಕಂಡುಹಿಡಿ_scsi_device(pci, ಗುರಿ, lun, &(plun->drive.lchs));
    ಅಕ್ಷರ *name = znprintf(MAXDESCSIZE, "pvscsi %pP %d:%d", pci, ಗುರಿ, lun);
    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_scsi_device(pci, ಗುರಿ, lun);
    ಪೂರ್ಣಾಂಕ ret = scsi_drive_ಸಿದ್ಧತೆ(&plun->drive, name, prio);
    ಬಿಡುಗಡೆ(name);
    ಆದರೆ (ret)
        ಹೋಗು fail;
    ಹಿಂತಿರುಗಿ 0;

fail:
    ಬಿಡುಗಡೆ(plun);
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
pvscsi_ಪರಿಶೀಲನೆ_ಗುರಿ(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *iobase,
                   ರಚನೆ pvscsi_ring_dsc_s *ring_dsc, u8 ಗುರಿ)
{
    /* pvscsi has no more than a single lun per target */
    pvscsi_ಕೂಡಿಸು_lun(pci, iobase, ring_dsc, ಗುರಿ, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_pvscsi(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ pci_device *pci = ದತ್ತಾಂಶ;
    ಶೂನ್ಯ *iobase = pci_enable_membar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!iobase)
        ಹಿಂತಿರುಗಿ;
    pci_enable_busmaster(pci);

    dprintf(1, "found pvscsi at %pP, io @ %p\n", pci, iobase);

    pvscsi_write_cmd_desc(iobase, PVSCSI_CMD_ADAPTER_RESET, ಖಾಲಿ, 0);

    ರಚನೆ pvscsi_ring_dsc_s *ring_dsc = ಖಾಲಿ;
    pvscsi_init_rings(iobase, &ring_dsc);
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < 64; i++)
        pvscsi_ಪರಿಶೀಲನೆ_ಗುರಿ(pci, iobase, ring_dsc, i);
}

ಶೂನ್ಯ
pvscsi_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_PVSCSI)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init pvscsi\n");

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->vendor != PCI_VENDOR_ID_VMWARE
            || pci->device != PCI_DEVICE_ID_VMWARE_PVSCSI)
            ಮುಂದುವರಿ;
        ಸತತ_thread(init_pvscsi, pci);
    }
}
