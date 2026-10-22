// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBALFLAT
#ضمن "block.h" // struct drive_s
#ضمن "blockcmd.h" // scsi_drive_setup
#ضمن "config.h" // CONFIG_*
#ضمن "fw/paravirt.h" // runningOnQEMU
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_DEVICE_ID
#ضمن "pci_regs.h" // PCI_VENDOR_ID
#ضمن "stacks.h" // run_thread
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // usleep

#تعریف_کن MPT_REG_DOORBELL  0x00
#تعریف_کن MPT_REG_WRITE_SEQ 0x04
#تعریف_کن MPT_REG_HOST_DIAG 0x08
#تعریف_کن MPT_REG_TEST      0x0c
#تعریف_کن MPT_REG_DIAG_داده 0x10
#تعریف_کن MPT_REG_DIAG_ADDR 0x14
#تعریف_کن MPT_REG_ISTATUS   0x30
#تعریف_کن MPT_REG_IMASK     0x34
#تعریف_کن MPT_REG_REQ_Q     0x40
#تعریف_کن MPT_REG_REP_Q     0x44

#تعریف_کن MPT_DOORBELL_پیام_RESET 0x40
#تعریف_کن MPT_DOORBELL_HANDSHAKE 0x42

#تعریف_کن MPT_IMASK_DOORBELL 0x01
#تعریف_کن MPT_IMASK_پاسخ    0x08

بنية mpt_lun_s {
    بنية drive_s drive;
    بنية pci_device *pci;
    u32 iobase;
    u8 هدف;
    u8 lun;
};

u8 پاسخ_پیام[4] __attribute((aligned(4))) VARLOW;

#تعریف_کن MPT_پیام_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#تعریف_کن MPT_پیام_HDR_FUNCTION_IOC_INIT               (0x02)

ساكن بنية MptIOCInitRequest
{
    u8     WhoInit;             /* Which system sent this init request. */
    u8     Reserved1;           /* Reserved */
    u8     Chain_جابه_جایی;         /* Chain offset in the SG list. */
    u8     Function;            /* Function to execute. */
    u8     Flags;               /* Flags */
    u8     MaxDevices;          /* Max devices the driver can handle. */
    u8     MaxBuses;            /* Max buses the driver can handle. */
    u8     پیام_Flags;        /* Message flags. */
    u32    پیام_Context;      /* Message context ID. */
    u16    پاسخ_قاب_اندازه;      /* Reply frame size. */
    u16    Reserved2;           /* Reserved */
    u32    Host_Mfa_بالا_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_میانگیر_بالا_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_پیام_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .پاسخ_قاب_اندازه = حجم(پاسخ_پیام),
    .Host_Mfa_بالا_Addr = 0,
    .Sense_میانگیر_بالا_Addr = 0
};

بنية Mpt_IOCInit_پاسخ {
    u8     WhoInit;     /* Which subsystem sent this init request. */
    u8     Reserved1;   /* Reserved */
    u8     پیام_درازی; /* Message length */
    u8     Function;    /* Function. */
    u8     Flags;       /* Flags */
    u8     MaxDevices;  /* Maximum number of devices the driver can handle. */
    u8     MaxBuses;    /* Maximum number of busses the driver can handle. */
    u8     پیام_Flags; /* Message flags. */
    u32    پیام_Context; /* Message context ID */
    u16    Reserved2;   /* Reserved */
    u16    IOCStatus;   /* IO controller status. */
    u32    IOCLogInfo;  /* IO controller log information. */
};

تعريفنوع بنية MptSCSIIORequest {
    u8     هدف_ID;            /* Target ID */
    u8     Bus;                 /* Bus number */
    u8     Chain_جابه_جایی;         /* Chain offset */
    u8     Function;            /* Function number. */
    u8     CDBLength;           /* CDB length. */
    u8     Sense_میانگیر_درازی;   /* Sense buffer length. */
    u8     Reserved;            /* Reserved */
    u8     پیام_Flags;        /* Message flags. */
    u32    پیام_Context;      /* Message context ID. */
    u8     LUN[8];              /* LUN */
    u32    Control;             /* Control values. */
    u8     CDB[16];             /* The CDB. */
    u32    داده_درازی;          /* Data length. */
    u32    Sense_میانگیر_کم_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#تعریف_کن MPT_POLL_TIMEOUT  60000

تعريفنوع بنية MptSGEntrySimple32 {
    u32 Flags_درازی;
    u32 داده_میانگیر_آدرس_کم;
} __attribute__((packed)) MptSGEntrySimple32_t;

ساكن صحيح
mpt_scsi_cmd(u32 iobase, بنية disk_op_s *op,
             u8 *cdb, u16 هدف, u16 lun, u16 blocksize)
{
    u32 انتها = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    بنية scsi_req {
        MptSCSIIORequest_t      scsi_io;
        MptSGEntrySimple32_t    sge;
    } __attribute__((packed, aligned(4))) req = {
        .scsi_io = {
            .هدف_ID = هدف,
            .Bus = 0,
            .Function = MPT_پیام_HDR_FUNCTION_SCSI_IO_REQUEST,
            .CDBLength = 16,
            .Sense_میانگیر_درازی = 18,
            .پیام_Context = انتها & 0x7fffffff,
            .داده_درازی = op->شمار * blocksize,
            .Sense_میانگیر_کم_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .Flags_درازی = (op->شمار * blocksize) | 0xD1000000,
            .داده_میانگیر_آدرس_کم = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    کپی_حافظه(req.scsi_io.CDB, cdb, 16);
    إذا (blocksize) {
        إذا (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } وإلا {
            req.scsi_io.Control = 1 << 24;
            req.sge.Flags_درازی |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    لكل (;;) {
        إذا (timer_check(انتها)) {
            أرجع DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        إذا (istatus & MPT_IMASK_پاسخ) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            إذا (resp == req.scsi_io.پیام_Context) {
                أرجع DISK_RET_SUCCESS;
            } وإلا إذا (resp & 0x80000000) {
                outl((u32)&پاسخ_پیام[0], iobase + MPT_REG_REP_Q);
                أرجع DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

صحيح
mpt_scsi_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_MPT_SCSI)
        أرجع DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    صحيح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    إذا (blocksize < 0)
        أرجع default_process_op(op);

    بنية mpt_lun_s *llun_gf =
        container_از(op->drive_fl, بنية mpt_lun_s, drive);
    u16 هدف = GET_GLOBALFLAT(llun_gf->هدف);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    أرجع mpt_scsi_cmd(iobase, op, cdbcmd, هدف, lun, blocksize);
}

ساكن فراغ
mpt_scsi_init_lun(بنية mpt_lun_s *llun, بنية pci_device *pci,
                  u32 iobase, u8 هدف, u8 lun)
{
    memset(llun, 0, حجم(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->هدف = هدف;
    llun->lun = lun;
    llun->iobase = iobase;
}

ساكن صحيح
mpt_scsi_جمع_کن_lun(u32 lun, بنية drive_s *tmpl_drv)
{
    بنية mpt_lun_s *tmpl_llun =
        container_از(tmpl_drv, بنية mpt_lun_s, drive);
    بنية mpt_lun_s *llun = malloc_fseg(حجم(*llun));
    إذا (!llun) {
        warn_noalloc();
        أرجع -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->هدف, lun);

    boot_lchs_پیدا_کن_scsi_device(llun->pci, llun->هدف, llun->lun,
                               &(llun->drive.lchs));
    حرف *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->هدف, llun->lun);
    صحيح prio = bootprio_پیدا_کن_scsi_device(llun->pci, llun->هدف, llun->lun);
    صحيح ret = scsi_drive_راهاندازی(&llun->drive, name, prio);
    حرر(name);
    إذا (ret) {
        اذهب fail;
    }
    أرجع 0;

fail:
    حرر(llun);
    أرجع -1;
}

ساكن فراغ
mpt_scsi_پیمایش_هدف(بنية pci_device *pci, u32 iobase, u8 هدف)
{
    بنية mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, هدف, 0);

    إذا (scsi_rep_luns_پیمایش(&llun0.drive, mpt_scsi_جمع_کن_lun) < 0)
        scsi_sequential_پیمایش(&llun0.drive, 8, mpt_scsi_جمع_کن_lun);
}

ساكن مضمن فراغ
mpt_بیرون_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

ساكن فراغ
init_mpt_scsi(فراغ *داده)
{
    بنية pci_device *pci = داده;
    u16 *پیام_درون_p;
    u32 iobase = pci_enable_iobar(pci, PCI_پایه_آدرس_0);
    إذا (!iobase)
        أرجع;
    بنية Mpt_IOCInit_پاسخ Mpt_IOCInit_پاسخ;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_بیرون_doorbell(MPT_DOORBELL_پیام_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_پاسخ , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_بیرون_doorbell(MPT_DOORBELL_HANDSHAKE,
                     حجم(MptIOCInitRequest)/حجم(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          حجم(MptIOCInitRequest)/حجم(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    پیام_درون_p = (u16 *)&Mpt_IOCInit_پاسخ;
    طالما(پیام_درون_p != (u16 *)(&Mpt_IOCInit_پاسخ + 1))
        *پیام_درون_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&پاسخ_پیام[0], iobase + MPT_REG_REP_Q);

    صحيح i;
    لكل (i = 0; i < 7; i++)
        mpt_scsi_پیمایش_هدف(pci, iobase, i);
}

فراغ
mpt_scsi_راهاندازی(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        أرجع;
    }

    dprintf(3, "init MPT\n");

    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            پیاپی_thread(init_mpt_scsi, pci);
    }
}
