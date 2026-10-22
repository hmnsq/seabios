// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // scsi_drive_setup
#شامل "config.h" // CONFIG_*
#شامل "fw/paravirt.h" // runningOnQEMU
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_DEVICE_ID
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "stacks.h" // run_thread
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // usleep

#تعریف_کرو MPT_REG_DOORBELL  0x00
#تعریف_کرو MPT_REG_WRITE_SEQ 0x04
#تعریف_کرو MPT_REG_HOST_DIAG 0x08
#تعریف_کرو MPT_REG_TEST      0x0c
#تعریف_کرو MPT_REG_DIAG_ڈیٹا 0x10
#تعریف_کرو MPT_REG_DIAG_ADDR 0x14
#تعریف_کرو MPT_REG_ISTATUS   0x30
#تعریف_کرو MPT_REG_IMASK     0x34
#تعریف_کرو MPT_REG_REQ_Q     0x40
#تعریف_کرو MPT_REG_REP_Q     0x44

#تعریف_کرو MPT_DOORBELL_پیغام_RESET 0x40
#تعریف_کرو MPT_DOORBELL_HANDSHAKE 0x42

#تعریف_کرو MPT_IMASK_DOORBELL 0x01
#تعریف_کرو MPT_IMASK_جواب    0x08

ساخت mpt_lun_s {
    ساخت drive_s drive;
    ساخت pci_device *pci;
    u32 iobase;
    u8 ہدف;
    u8 lun;
};

u8 جواب_پیغام[4] __attribute((aligned(4))) VARLOW;

#تعریف_کرو MPT_پیغام_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#تعریف_کرو MPT_پیغام_HDR_FUNCTION_IOC_INIT               (0x02)

ساکن ساخت MptIOCInitRequest
{
    u8     WhoInit;             /* Which system sent this init request. */
    u8     Reserved1;           /* Reserved */
    u8     Chain_انحراف;         /* Chain offset in the SG list. */
    u8     Function;            /* Function to execute. */
    u8     Flags;               /* Flags */
    u8     MaxDevices;          /* Max devices the driver can handle. */
    u8     MaxBuses;            /* Max buses the driver can handle. */
    u8     پیغام_Flags;        /* Message flags. */
    u32    پیغام_Context;      /* Message context ID. */
    u16    جواب_چوکھٹا_سائز;      /* Reply frame size. */
    u16    Reserved2;           /* Reserved */
    u32    Host_Mfa_بلند_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_بفر_بلند_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_پیغام_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .جواب_چوکھٹا_سائز = حجم(جواب_پیغام),
    .Host_Mfa_بلند_Addr = 0,
    .Sense_بفر_بلند_Addr = 0
};

ساخت Mpt_IOCInit_جواب {
    u8     WhoInit;     /* Which subsystem sent this init request. */
    u8     Reserved1;   /* Reserved */
    u8     پیغام_لمبائی; /* Message length */
    u8     Function;    /* Function. */
    u8     Flags;       /* Flags */
    u8     MaxDevices;  /* Maximum number of devices the driver can handle. */
    u8     MaxBuses;    /* Maximum number of busses the driver can handle. */
    u8     پیغام_Flags; /* Message flags. */
    u32    پیغام_Context; /* Message context ID */
    u16    Reserved2;   /* Reserved */
    u16    IOCStatus;   /* IO controller status. */
    u32    IOCLogInfo;  /* IO controller log information. */
};

قسمتعریف ساخت MptSCSIIORequest {
    u8     ہدف_ID;            /* Target ID */
    u8     Bus;                 /* Bus number */
    u8     Chain_انحراف;         /* Chain offset */
    u8     Function;            /* Function number. */
    u8     CDBLength;           /* CDB length. */
    u8     Sense_بفر_لمبائی;   /* Sense buffer length. */
    u8     Reserved;            /* Reserved */
    u8     پیغام_Flags;        /* Message flags. */
    u32    پیغام_Context;      /* Message context ID. */
    u8     LUN[8];              /* LUN */
    u32    Control;             /* Control values. */
    u8     CDB[16];             /* The CDB. */
    u32    ڈیٹا_لمبائی;          /* Data length. */
    u32    Sense_بفر_کم_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#تعریف_کرو MPT_POLL_TIMEOUT  60000

قسمتعریف ساخت MptSGEntrySimple32 {
    u32 Flags_لمبائی;
    u32 ڈیٹا_بفر_پتہ_کم;
} __attribute__((packed)) MptSGEntrySimple32_t;

ساکن صحیح
mpt_scsi_cmd(u32 iobase, ساخت disk_op_s *op,
             u8 *cdb, u16 ہدف, u16 lun, u16 blocksize)
{
    u32 انتہا = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    ساخت scsi_req {
        MptSCSIIORequest_t      scsi_io;
        MptSGEntrySimple32_t    sge;
    } __attribute__((packed, aligned(4))) req = {
        .scsi_io = {
            .ہدف_ID = ہدف,
            .Bus = 0,
            .Function = MPT_پیغام_HDR_FUNCTION_SCSI_IO_REQUEST,
            .CDBLength = 16,
            .Sense_بفر_لمبائی = 18,
            .پیغام_Context = انتہا & 0x7fffffff,
            .ڈیٹا_لمبائی = op->گنتی * blocksize,
            .Sense_بفر_کم_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .Flags_لمبائی = (op->گنتی * blocksize) | 0xD1000000,
            .ڈیٹا_بفر_پتہ_کم = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    حافظہ_نقل(req.scsi_io.CDB, cdb, 16);
    اگر (blocksize) {
        اگر (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } ورنہ {
            req.scsi_io.Control = 1 << 24;
            req.sge.Flags_لمبائی |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    برائے (;;) {
        اگر (timer_check(انتہا)) {
            واپس DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        اگر (istatus & MPT_IMASK_جواب) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            اگر (resp == req.scsi_io.پیغام_Context) {
                واپس DISK_RET_SUCCESS;
            } ورنہ اگر (resp & 0x80000000) {
                outl((u32)&جواب_پیغام[0], iobase + MPT_REG_REP_Q);
                واپس DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

صحیح
mpt_scsi_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_MPT_SCSI)
        واپس DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    صحیح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    اگر (blocksize < 0)
        واپس default_process_op(op);

    ساخت mpt_lun_s *llun_gf =
        container_کا(op->drive_fl, ساخت mpt_lun_s, drive);
    u16 ہدف = GET_GLOBALFLAT(llun_gf->ہدف);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    واپس mpt_scsi_cmd(iobase, op, cdbcmd, ہدف, lun, blocksize);
}

ساکن خالی
mpt_scsi_init_lun(ساخت mpt_lun_s *llun, ساخت pci_device *pci,
                  u32 iobase, u8 ہدف, u8 lun)
{
    memset(llun, 0, حجم(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ہدف = ہدف;
    llun->lun = lun;
    llun->iobase = iobase;
}

ساکن صحیح
mpt_scsi_جوڑو_lun(u32 lun, ساخت drive_s *tmpl_drv)
{
    ساخت mpt_lun_s *tmpl_llun =
        container_کا(tmpl_drv, ساخت mpt_lun_s, drive);
    ساخت mpt_lun_s *llun = malloc_fseg(حجم(*llun));
    اگر (!llun) {
        warn_noalloc();
        واپس -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ہدف, lun);

    boot_lchs_ڈھونڈو_scsi_device(llun->pci, llun->ہدف, llun->lun,
                               &(llun->drive.lchs));
    حرف *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->ہدف, llun->lun);
    صحیح prio = bootprio_ڈھونڈو_scsi_device(llun->pci, llun->ہدف, llun->lun);
    صحیح ret = scsi_drive_تیاری(&llun->drive, name, prio);
    آزاد(name);
    اگر (ret) {
        جاؤ fail;
    }
    واپس 0;

fail:
    آزاد(llun);
    واپس -1;
}

ساکن خالی
mpt_scsi_جائزہ_ہدف(ساخت pci_device *pci, u32 iobase, u8 ہدف)
{
    ساخت mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, ہدف, 0);

    اگر (scsi_rep_luns_جائزہ(&llun0.drive, mpt_scsi_جوڑو_lun) < 0)
        scsi_sequential_جائزہ(&llun0.drive, 8, mpt_scsi_جوڑو_lun);
}

ساکن اندرونی خالی
mpt_برآمد_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

ساکن خالی
init_mpt_scsi(خالی *ڈیٹا)
{
    ساخت pci_device *pci = ڈیٹا;
    u16 *پیغام_اندر_p;
    u32 iobase = pci_enable_iobar(pci, PCI_بنیاد_پتہ_0);
    اگر (!iobase)
        واپس;
    ساخت Mpt_IOCInit_جواب Mpt_IOCInit_جواب;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_برآمد_doorbell(MPT_DOORBELL_پیغام_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_جواب , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_برآمد_doorbell(MPT_DOORBELL_HANDSHAKE,
                     حجم(MptIOCInitRequest)/حجم(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          حجم(MptIOCInitRequest)/حجم(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    پیغام_اندر_p = (u16 *)&Mpt_IOCInit_جواب;
    جب(پیغام_اندر_p != (u16 *)(&Mpt_IOCInit_جواب + 1))
        *پیغام_اندر_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&جواب_پیغام[0], iobase + MPT_REG_REP_Q);

    صحیح i;
    برائے (i = 0; i < 7; i++)
        mpt_scsi_جائزہ_ہدف(pci, iobase, i);
}

خالی
mpt_scsi_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        واپس;
    }

    dprintf(3, "init MPT\n");

    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            تسلسل_thread(init_mpt_scsi, pci);
    }
}
