// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // scsi_drive_setup
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "fw/paravirt.h" // runningOnQEMU
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_DEVICE_ID
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "stacks.h" // run_thread
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // usleep

#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_DOORBELL  0x00
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_WRITE_SEQ 0x04
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_HOST_DIAG 0x08
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_TEST      0x0c
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_DIAG_ದತ್ತಾಂಶ 0x10
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_DIAG_ADDR 0x14
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_ISTATUS   0x30
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_IMASK     0x34
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_REQ_Q     0x40
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_REG_REP_Q     0x44

#ವ್ಯಾಖ್ಯಾನಿಸು MPT_DOORBELL_ಸಂದೇಶ_RESET 0x40
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_DOORBELL_HANDSHAKE 0x42

#ವ್ಯಾಖ್ಯಾನಿಸು MPT_IMASK_DOORBELL 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_IMASK_ಉತ್ತರ    0x08

ರಚನೆ mpt_lun_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ pci_device *pci;
    u32 iobase;
    u8 ಗುರಿ;
    u8 lun;
};

u8 ಉತ್ತರ_ಸಂದೇಶ[4] __attribute((aligned(4))) VARLOW;

#ವ್ಯಾಖ್ಯಾನಿಸು MPT_ಸಂದೇಶ_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#ವ್ಯಾಖ್ಯಾನಿಸು MPT_ಸಂದೇಶ_HDR_FUNCTION_IOC_INIT               (0x02)

ಸ್ಥಿರ ರಚನೆ MptIOCInitRequest
{
    u8     WhoInit;             /* Which system sent this init request. */
    u8     Reserved1;           /* Reserved */
    u8     Chain_ಸ್ಥಾನಾಂತರ;         /* Chain offset in the SG list. */
    u8     Function;            /* Function to execute. */
    u8     Flags;               /* Flags */
    u8     MaxDevices;          /* Max devices the driver can handle. */
    u8     MaxBuses;            /* Max buses the driver can handle. */
    u8     ಸಂದೇಶ_Flags;        /* Message flags. */
    u32    ಸಂದೇಶ_Context;      /* Message context ID. */
    u16    ಉತ್ತರ_ಚೌಕಟ್ಟು_ಗಾತ್ರ;      /* Reply frame size. */
    u16    Reserved2;           /* Reserved */
    u32    Host_Mfa_ಮೇಲಿನ_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_ಬಫರ್_ಮೇಲಿನ_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_ಸಂದೇಶ_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .ಉತ್ತರ_ಚೌಕಟ್ಟು_ಗಾತ್ರ = ಗಾತ್ರ(ಉತ್ತರ_ಸಂದೇಶ),
    .Host_Mfa_ಮೇಲಿನ_Addr = 0,
    .Sense_ಬಫರ್_ಮೇಲಿನ_Addr = 0
};

ರಚನೆ Mpt_IOCInit_ಉತ್ತರ {
    u8     WhoInit;     /* Which subsystem sent this init request. */
    u8     Reserved1;   /* Reserved */
    u8     ಸಂದೇಶ_ಉದ್ದಳತೆ; /* Message length */
    u8     Function;    /* Function. */
    u8     Flags;       /* Flags */
    u8     MaxDevices;  /* Maximum number of devices the driver can handle. */
    u8     MaxBuses;    /* Maximum number of busses the driver can handle. */
    u8     ಸಂದೇಶ_Flags; /* Message flags. */
    u32    ಸಂದೇಶ_Context; /* Message context ID */
    u16    Reserved2;   /* Reserved */
    u16    IOCStatus;   /* IO controller status. */
    u32    IOCLogInfo;  /* IO controller log information. */
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ MptSCSIIORequest {
    u8     ಗುರಿ_ID;            /* Target ID */
    u8     Bus;                 /* Bus number */
    u8     Chain_ಸ್ಥಾನಾಂತರ;         /* Chain offset */
    u8     Function;            /* Function number. */
    u8     CDBLength;           /* CDB length. */
    u8     Sense_ಬಫರ್_ಉದ್ದಳತೆ;   /* Sense buffer length. */
    u8     Reserved;            /* Reserved */
    u8     ಸಂದೇಶ_Flags;        /* Message flags. */
    u32    ಸಂದೇಶ_Context;      /* Message context ID. */
    u8     LUN[8];              /* LUN */
    u32    Control;             /* Control values. */
    u8     CDB[16];             /* The CDB. */
    u32    ದತ್ತಾಂಶ_ಉದ್ದಳತೆ;          /* Data length. */
    u32    Sense_ಬಫರ್_ಕೆಳಗಿನ_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#ವ್ಯಾಖ್ಯಾನಿಸು MPT_POLL_TIMEOUT  60000

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ MptSGEntrySimple32 {
    u32 Flags_ಉದ್ದಳತೆ;
    u32 ದತ್ತಾಂಶ_ಬಫರ್_ವಿಳಾಸ_ಕೆಳಗಿನ;
} __attribute__((packed)) MptSGEntrySimple32_t;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
mpt_scsi_cmd(u32 iobase, ರಚನೆ disk_op_s *op,
             u8 *cdb, u16 ಗುರಿ, u16 lun, u16 blocksize)
{
    u32 ಕೊನೆ = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    ರಚನೆ scsi_req {
        MptSCSIIORequest_t      scsi_io;
        MptSGEntrySimple32_t    sge;
    } __attribute__((packed, aligned(4))) req = {
        .scsi_io = {
            .ಗುರಿ_ID = ಗುರಿ,
            .Bus = 0,
            .Function = MPT_ಸಂದೇಶ_HDR_FUNCTION_SCSI_IO_REQUEST,
            .CDBLength = 16,
            .Sense_ಬಫರ್_ಉದ್ದಳತೆ = 18,
            .ಸಂದೇಶ_Context = ಕೊನೆ & 0x7fffffff,
            .ದತ್ತಾಂಶ_ಉದ್ದಳತೆ = op->ಎಣಿಕೆ_2 * blocksize,
            .Sense_ಬಫರ್_ಕೆಳಗಿನ_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .Flags_ಉದ್ದಳತೆ = (op->ಎಣಿಕೆ_2 * blocksize) | 0xD1000000,
            .ದತ್ತಾಂಶ_ಬಫರ್_ವಿಳಾಸ_ಕೆಳಗಿನ = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    ಸ್ಮರಣೆ_ನಕಲು(req.scsi_io.CDB, cdb, 16);
    ಆದರೆ (blocksize) {
        ಆದರೆ (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } ಇಲ್ಲದಿದ್ದರೆ {
            req.scsi_io.Control = 1 << 24;
            req.sge.Flags_ಉದ್ದಳತೆ |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    ಗಾಗಿ (;;) {
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            ಹಿಂತಿರುಗಿ DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        ಆದರೆ (istatus & MPT_IMASK_ಉತ್ತರ) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            ಆದರೆ (resp == req.scsi_io.ಸಂದೇಶ_Context) {
                ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
            } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (resp & 0x80000000) {
                outl((u32)&ಉತ್ತರ_ಸಂದೇಶ[0], iobase + MPT_REG_REP_Q);
                ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

ಪೂರ್ಣಾಂಕ
mpt_scsi_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_MPT_SCSI)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, cdbcmd, ಗಾತ್ರ(cdbcmd));
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);

    ರಚನೆ mpt_lun_s *llun_gf =
        container_ದ(op->drive_fl, ರಚನೆ mpt_lun_s, drive);
    u16 ಗುರಿ = GET_GLOBALFLAT(llun_gf->ಗುರಿ);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ಹಿಂತಿರುಗಿ mpt_scsi_cmd(iobase, op, cdbcmd, ಗುರಿ, lun, blocksize);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mpt_scsi_init_lun(ರಚನೆ mpt_lun_s *llun, ರಚನೆ pci_device *pci,
                  u32 iobase, u8 ಗುರಿ, u8 lun)
{
    memset(llun, 0, ಗಾತ್ರ(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ಗುರಿ = ಗುರಿ;
    llun->lun = lun;
    llun->iobase = iobase;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
mpt_scsi_ಕೂಡಿಸು_lun(u32 lun, ರಚನೆ drive_s *tmpl_drv)
{
    ರಚನೆ mpt_lun_s *tmpl_llun =
        container_ದ(tmpl_drv, ರಚನೆ mpt_lun_s, drive);
    ರಚನೆ mpt_lun_s *llun = malloc_fseg(ಗಾತ್ರ(*llun));
    ಆದರೆ (!llun) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ಗುರಿ, lun);

    boot_lchs_ಕಂಡುಹಿಡಿ_scsi_device(llun->pci, llun->ಗುರಿ, llun->lun,
                               &(llun->drive.lchs));
    ಅಕ್ಷರ *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->ಗುರಿ, llun->lun);
    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_scsi_device(llun->pci, llun->ಗುರಿ, llun->lun);
    ಪೂರ್ಣಾಂಕ ret = scsi_drive_ಸಿದ್ಧತೆ(&llun->drive, name, prio);
    ಬಿಡುಗಡೆ(name);
    ಆದರೆ (ret) {
        ಹೋಗು fail;
    }
    ಹಿಂತಿರುಗಿ 0;

fail:
    ಬಿಡುಗಡೆ(llun);
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
mpt_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(ರಚನೆ pci_device *pci, u32 iobase, u8 ಗುರಿ)
{
    ರಚನೆ mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, ಗುರಿ, 0);

    ಆದರೆ (scsi_rep_luns_ಪರಿಶೀಲನೆ(&llun0.drive, mpt_scsi_ಕೂಡಿಸು_lun) < 0)
        scsi_sequential_ಪರಿಶೀಲನೆ(&llun0.drive, 8, mpt_scsi_ಕೂಡಿಸು_lun);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
mpt_ಹೊರಹೋಗು_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_mpt_scsi(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ pci_device *pci = ದತ್ತಾಂಶ;
    u16 *ಸಂದೇಶ_ಒಳ_p;
    u32 iobase = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!iobase)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ Mpt_IOCInit_ಉತ್ತರ Mpt_IOCInit_ಉತ್ತರ;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_ಹೊರಹೋಗು_doorbell(MPT_DOORBELL_ಸಂದೇಶ_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_ಉತ್ತರ , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_ಹೊರಹೋಗು_doorbell(MPT_DOORBELL_HANDSHAKE,
                     ಗಾತ್ರ(MptIOCInitRequest)/ಗಾತ್ರ(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          ಗಾತ್ರ(MptIOCInitRequest)/ಗಾತ್ರ(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    ಸಂದೇಶ_ಒಳ_p = (u16 *)&Mpt_IOCInit_ಉತ್ತರ;
    ಆಗ(ಸಂದೇಶ_ಒಳ_p != (u16 *)(&Mpt_IOCInit_ಉತ್ತರ + 1))
        *ಸಂದೇಶ_ಒಳ_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&ಉತ್ತರ_ಸಂದೇಶ[0], iobase + MPT_REG_REP_Q);

    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < 7; i++)
        mpt_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(pci, iobase, i);
}

ಶೂನ್ಯ
mpt_scsi_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        ಹಿಂತಿರುಗಿ;
    }

    dprintf(3, "init MPT\n");

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            ಸತತ_thread(init_mpt_scsi, pci);
    }
}
