// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // scsi_drive_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/paravirt.h" // runningOnQEMU
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_DEVICE_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // run_thread
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // usleep

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_DOORBELL  0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_WRITE_SEQ 0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_HOST_DIAG 0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_TEST      0x0c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_DIAG_ਅੰਕੜੇ 0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_DIAG_ADDR 0x14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_ISTATUS   0x30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_IMASK     0x34
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_REQ_Q     0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_REG_REP_Q     0x44

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_DOORBELL_ਸੁਨੇਹਾ_RESET 0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_DOORBELL_HANDSHAKE 0x42

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_IMASK_DOORBELL 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_IMASK_ਜਵਾਬ    0x08

ਬਣਤਰ mpt_lun_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ pci_device *pci;
    u32 iobase;
    u8 ਨਿਸ਼ਾਨਾ;
    u8 lun;
};

u8 ਜਵਾਬ_ਸੁਨੇਹਾ[4] __attribute((aligned(4))) VARLOW;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_ਸੁਨੇਹਾ_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_ਸੁਨੇਹਾ_HDR_FUNCTION_IOC_INIT               (0x02)

ਸਥਿਰ ਬਣਤਰ MptIOCInitRequest
{
    u8     WhoInit;             /* Which system sent this init request. */
    u8     Reserved1;           /* Reserved */
    u8     Chain_ਵਿੱਥ;         /* Chain offset in the SG list. */
    u8     Function;            /* Function to execute. */
    u8     Flags;               /* Flags */
    u8     MaxDevices;          /* Max devices the driver can handle. */
    u8     MaxBuses;            /* Max buses the driver can handle. */
    u8     ਸੁਨੇਹਾ_Flags;        /* Message flags. */
    u32    ਸੁਨੇਹਾ_Context;      /* Message context ID. */
    u16    ਜਵਾਬ_ਚੌਖਟਾ_ਮਾਪ;      /* Reply frame size. */
    u16    Reserved2;           /* Reserved */
    u32    Host_Mfa_ਉੱਚਾ_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_ਬਫਰ_ਉੱਚਾ_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_ਸੁਨੇਹਾ_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .ਜਵਾਬ_ਚੌਖਟਾ_ਮਾਪ = ਆਕਾਰ(ਜਵਾਬ_ਸੁਨੇਹਾ),
    .Host_Mfa_ਉੱਚਾ_Addr = 0,
    .Sense_ਬਫਰ_ਉੱਚਾ_Addr = 0
};

ਬਣਤਰ Mpt_IOCInit_ਜਵਾਬ {
    u8     WhoInit;     /* Which subsystem sent this init request. */
    u8     Reserved1;   /* Reserved */
    u8     ਸੁਨੇਹਾ_ਲੰਮਾਈ; /* Message length */
    u8     Function;    /* Function. */
    u8     Flags;       /* Flags */
    u8     MaxDevices;  /* Maximum number of devices the driver can handle. */
    u8     MaxBuses;    /* Maximum number of busses the driver can handle. */
    u8     ਸੁਨੇਹਾ_Flags; /* Message flags. */
    u32    ਸੁਨੇਹਾ_Context; /* Message context ID */
    u16    Reserved2;   /* Reserved */
    u16    IOCStatus;   /* IO controller status. */
    u32    IOCLogInfo;  /* IO controller log information. */
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ MptSCSIIORequest {
    u8     ਨਿਸ਼ਾਨਾ_ID;            /* Target ID */
    u8     Bus;                 /* Bus number */
    u8     Chain_ਵਿੱਥ;         /* Chain offset */
    u8     Function;            /* Function number. */
    u8     CDBLength;           /* CDB length. */
    u8     Sense_ਬਫਰ_ਲੰਮਾਈ;   /* Sense buffer length. */
    u8     Reserved;            /* Reserved */
    u8     ਸੁਨੇਹਾ_Flags;        /* Message flags. */
    u32    ਸੁਨੇਹਾ_Context;      /* Message context ID. */
    u8     LUN[8];              /* LUN */
    u32    Control;             /* Control values. */
    u8     CDB[16];             /* The CDB. */
    u32    ਅੰਕੜੇ_ਲੰਮਾਈ;          /* Data length. */
    u32    Sense_ਬਫਰ_ਨੀਵਾਂ_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MPT_POLL_TIMEOUT  60000

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ MptSGEntrySimple32 {
    u32 Flags_ਲੰਮਾਈ;
    u32 ਅੰਕੜੇ_ਬਫਰ_ਪਤਾ_ਨੀਵਾਂ;
} __attribute__((packed)) MptSGEntrySimple32_t;

ਸਥਿਰ ਪੂਰਨ
mpt_scsi_cmd(u32 iobase, ਬਣਤਰ disk_op_s *op,
             u8 *cdb, u16 ਨਿਸ਼ਾਨਾ, u16 lun, u16 blocksize)
{
    u32 ਸਿਰਾ = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    ਬਣਤਰ scsi_req {
        MptSCSIIORequest_t      scsi_io;
        MptSGEntrySimple32_t    sge;
    } __attribute__((packed, aligned(4))) req = {
        .scsi_io = {
            .ਨਿਸ਼ਾਨਾ_ID = ਨਿਸ਼ਾਨਾ,
            .Bus = 0,
            .Function = MPT_ਸੁਨੇਹਾ_HDR_FUNCTION_SCSI_IO_REQUEST,
            .CDBLength = 16,
            .Sense_ਬਫਰ_ਲੰਮਾਈ = 18,
            .ਸੁਨੇਹਾ_Context = ਸਿਰਾ & 0x7fffffff,
            .ਅੰਕੜੇ_ਲੰਮਾਈ = op->ਗਿਣਤੀ_2 * blocksize,
            .Sense_ਬਫਰ_ਨੀਵਾਂ_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .Flags_ਲੰਮਾਈ = (op->ਗਿਣਤੀ_2 * blocksize) | 0xD1000000,
            .ਅੰਕੜੇ_ਬਫਰ_ਪਤਾ_ਨੀਵਾਂ = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(req.scsi_io.CDB, cdb, 16);
    ਜੇ (blocksize) {
        ਜੇ (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } ਨਹੀਂਤਾਂ {
            req.scsi_io.Control = 1 << 24;
            req.sge.Flags_ਲੰਮਾਈ |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    ਲਈ (;;) {
        ਜੇ (timer_check(ਸਿਰਾ)) {
            ਵਾਪਸ DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        ਜੇ (istatus & MPT_IMASK_ਜਵਾਬ) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            ਜੇ (resp == req.scsi_io.ਸੁਨੇਹਾ_Context) {
                ਵਾਪਸ DISK_RET_SUCCESS;
            } ਨਹੀਂਤਾਂ ਜੇ (resp & 0x80000000) {
                outl((u32)&ਜਵਾਬ_ਸੁਨੇਹਾ[0], iobase + MPT_REG_REP_Q);
                ਵਾਪਸ DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

ਪੂਰਨ
mpt_scsi_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_MPT_SCSI)
        ਵਾਪਸ DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, cdbcmd, ਆਕਾਰ(cdbcmd));
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);

    ਬਣਤਰ mpt_lun_s *llun_gf =
        container_ਦਾ(op->drive_fl, ਬਣਤਰ mpt_lun_s, drive);
    u16 ਨਿਸ਼ਾਨਾ = GET_GLOBALFLAT(llun_gf->ਨਿਸ਼ਾਨਾ);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ਵਾਪਸ mpt_scsi_cmd(iobase, op, cdbcmd, ਨਿਸ਼ਾਨਾ, lun, blocksize);
}

ਸਥਿਰ ਖਾਲੀ
mpt_scsi_init_lun(ਬਣਤਰ mpt_lun_s *llun, ਬਣਤਰ pci_device *pci,
                  u32 iobase, u8 ਨਿਸ਼ਾਨਾ, u8 lun)
{
    memset(llun, 0, ਆਕਾਰ(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ਨਿਸ਼ਾਨਾ = ਨਿਸ਼ਾਨਾ;
    llun->lun = lun;
    llun->iobase = iobase;
}

ਸਥਿਰ ਪੂਰਨ
mpt_scsi_ਜੋੜੋ_lun(u32 lun, ਬਣਤਰ drive_s *tmpl_drv)
{
    ਬਣਤਰ mpt_lun_s *tmpl_llun =
        container_ਦਾ(tmpl_drv, ਬਣਤਰ mpt_lun_s, drive);
    ਬਣਤਰ mpt_lun_s *llun = malloc_fseg(ਆਕਾਰ(*llun));
    ਜੇ (!llun) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ਨਿਸ਼ਾਨਾ, lun);

    boot_lchs_ਲੱਭੋ_scsi_device(llun->pci, llun->ਨਿਸ਼ਾਨਾ, llun->lun,
                               &(llun->drive.lchs));
    ਅੱਖਰ *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->ਨਿਸ਼ਾਨਾ, llun->lun);
    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_scsi_device(llun->pci, llun->ਨਿਸ਼ਾਨਾ, llun->lun);
    ਪੂਰਨ ret = scsi_drive_ਤਿਆਰੀ(&llun->drive, name, prio);
    ਮੁਕਤ(name);
    ਜੇ (ret) {
        ਜਾਓ fail;
    }
    ਵਾਪਸ 0;

fail:
    ਮੁਕਤ(llun);
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਖਾਲੀ
mpt_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(ਬਣਤਰ pci_device *pci, u32 iobase, u8 ਨਿਸ਼ਾਨਾ)
{
    ਬਣਤਰ mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, ਨਿਸ਼ਾਨਾ, 0);

    ਜੇ (scsi_rep_luns_ਜਾਂਚ(&llun0.drive, mpt_scsi_ਜੋੜੋ_lun) < 0)
        scsi_sequential_ਜਾਂਚ(&llun0.drive, 8, mpt_scsi_ਜੋੜੋ_lun);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
mpt_ਬਾਹਰਲਾ_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

ਸਥਿਰ ਖਾਲੀ
init_mpt_scsi(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ pci_device *pci = ਅੰਕੜੇ;
    u16 *ਸੁਨੇਹਾ_ਵਿੱਚ_p;
    u32 iobase = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!iobase)
        ਵਾਪਸ;
    ਬਣਤਰ Mpt_IOCInit_ਜਵਾਬ Mpt_IOCInit_ਜਵਾਬ;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_ਬਾਹਰਲਾ_doorbell(MPT_DOORBELL_ਸੁਨੇਹਾ_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_ਜਵਾਬ , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_ਬਾਹਰਲਾ_doorbell(MPT_DOORBELL_HANDSHAKE,
                     ਆਕਾਰ(MptIOCInitRequest)/ਆਕਾਰ(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          ਆਕਾਰ(MptIOCInitRequest)/ਆਕਾਰ(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    ਸੁਨੇਹਾ_ਵਿੱਚ_p = (u16 *)&Mpt_IOCInit_ਜਵਾਬ;
    ਜਦੋਂ(ਸੁਨੇਹਾ_ਵਿੱਚ_p != (u16 *)(&Mpt_IOCInit_ਜਵਾਬ + 1))
        *ਸੁਨੇਹਾ_ਵਿੱਚ_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&ਜਵਾਬ_ਸੁਨੇਹਾ[0], iobase + MPT_REG_REP_Q);

    ਪੂਰਨ i;
    ਲਈ (i = 0; i < 7; i++)
        mpt_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(pci, iobase, i);
}

ਖਾਲੀ
mpt_scsi_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        ਵਾਪਸ;
    }

    dprintf(3, "init MPT\n");

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            ਲੜੀਵਾਰ_thread(init_mpt_scsi, pci);
    }
}
