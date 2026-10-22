// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাৱ "block.h" // struct drive_s
#অন্তর্ভাৱ "blockcmd.h" // scsi_drive_setup
#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "fw/paravirt.h" // runningOnQEMU
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "pcidevice.h" // foreachpci
#অন্তর্ভাৱ "pci_ids.h" // PCI_DEVICE_ID
#অন্তর্ভাৱ "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাৱ "stacks.h" // run_thread
#অন্তর্ভাৱ "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // usleep

#define MPT_REG_DOORBELL  0x00
#define MPT_REG_WRITE_SEQ 0x04
#define MPT_REG_HOST_DIAG 0x08
#define MPT_REG_TEST      0x0c
#define MPT_REG_DIAG_DATA 0x10
#define MPT_REG_DIAG_ADDR 0x14
#define MPT_REG_ISTATUS   0x30
#define MPT_REG_IMASK     0x34
#define MPT_REG_REQ_Q     0x40
#define MPT_REG_REP_Q     0x44

#define MPT_DOORBELL_বাৰ্তা_RESET 0x40
#define MPT_DOORBELL_HANDSHAKE 0x42

#define MPT_IMASK_DOORBELL 0x01
#define MPT_IMASK_REPLY    0x08

সংরচনা mpt_lun_s {
    সংরচনা drive_s drive;
    সংরচনা pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

u8 reply_বাৰ্তা[4] __attribute((aligned(4))) VARLOW;

#define MPT_MESSAGE_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#define MPT_MESSAGE_HDR_FUNCTION_IOC_INIT               (0x02)

স্থির সংরচনা MptIOCInitRequest
{
    u8     WhoInit;             /* Which system sent this init request. */
    u8     Reserved1;           /* Reserved */
    u8     ChainOffset;         /* Chain offset in the SG list. */
    u8     Function;            /* Function to execute. */
    u8     Flags;               /* Flags */
    u8     MaxDevices;          /* Max devices the driver can handle. */
    u8     MaxBuses;            /* Max buses the driver can handle. */
    u8     MessageFlags;        /* Message flags. */
    u32    MessageContext;      /* Message context ID. */
    u16    ReplyFrameSize;      /* Reply frame size. */
    u16    Reserved2;           /* Reserved */
    u32    Host_Mfa_উপৰেৰ_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_Buffer_উপৰেৰ_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_MESSAGE_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .ReplyFrameSize = পরিমাণ(reply_বাৰ্তা),
    .Host_Mfa_উপৰেৰ_Addr = 0,
    .Sense_Buffer_উপৰেৰ_Addr = 0
};

সংরচনা MptIOCInitReply {
    u8     WhoInit;     /* Which subsystem sent this init request. */
    u8     Reserved1;   /* Reserved */
    u8     MessageLength; /* Message length */
    u8     Function;    /* Function. */
    u8     Flags;       /* Flags */
    u8     MaxDevices;  /* Maximum number of devices the driver can handle. */
    u8     MaxBuses;    /* Maximum number of busses the driver can handle. */
    u8     MessageFlags; /* Message flags. */
    u32    MessageContext; /* Message context ID */
    u16    Reserved2;   /* Reserved */
    u16    IOCStatus;   /* IO controller status. */
    u32    IOCLogInfo;  /* IO controller log information. */
};

প্রকারনির্দেশ সংরচনা MptSCSIIORequest {
    u8     TargetID;            /* Target ID */
    u8     Bus;                 /* Bus number */
    u8     ChainOffset;         /* Chain offset */
    u8     Function;            /* Function number. */
    u8     CDBLength;           /* CDB length. */
    u8     SenseBufferLength;   /* Sense buffer length. */
    u8     Reserved;            /* Reserved */
    u8     MessageFlags;        /* Message flags. */
    u32    MessageContext;      /* Message context ID. */
    u8     LUN[8];              /* LUN */
    u32    Control;             /* Control values. */
    u8     CDB[16];             /* The CDB. */
    u32    DataLength;          /* Data length. */
    u32    Sense_Buffer_নিচেৰ_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#define MPT_POLL_TIMEOUT  60000

প্রকারনির্দেশ সংরচনা MptSGEntrySimple32 {
    u32 FlagsLength;
    u32 Data_Buffer_Address_নিচেৰ;
} __attribute__((packed)) MptSGEntrySimple32_t;

স্থির পূর্ণাঙ্ক
mpt_scsi_cmd(u32 iobase, সংরচনা disk_op_s *op,
             u8 *cdb, u16 target, u16 lun, u16 blocksize)
{
    u32 end = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    সংরচনা scsi_req {
        MptSCSIIORequest_t      scsi_io;
        MptSGEntrySimple32_t    sge;
    } __attribute__((packed, aligned(4))) req = {
        .scsi_io = {
            .TargetID = target,
            .Bus = 0,
            .Function = MPT_MESSAGE_HDR_FUNCTION_SCSI_IO_REQUEST,
            .CDBLength = 16,
            .SenseBufferLength = 18,
            .MessageContext = end & 0x7fffffff,
            .DataLength = op->গুনতি * blocksize,
            .Sense_Buffer_নিচেৰ_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .FlagsLength = (op->গুনতি * blocksize) | 0xD1000000,
            .Data_Buffer_Address_নিচেৰ = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    memcpy(req.scsi_io.CDB, cdb, 16);
    যদি (blocksize) {
        যদি (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } অন্যথা {
            req.scsi_io.Control = 1 << 24;
            req.sge.FlagsLength |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    বাবে (;;) {
        যদি (timer_check(end)) {
            ঘূৰাই DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        যদি (istatus & MPT_IMASK_REPLY) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            যদি (resp == req.scsi_io.MessageContext) {
                ঘূৰাই DISK_RET_SUCCESS;
            } অন্যথা যদি (resp & 0x80000000) {
                outl((u32)&reply_বাৰ্তা[0], iobase + MPT_REG_REP_Q);
                ঘূৰাই DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

পূর্ণাঙ্ক
mpt_scsi_process_op(সংরচনা disk_op_s *op)
{
    যদি (!CONFIG_MPT_SCSI)
        ঘূৰাই DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    পূর্ণাঙ্ক blocksize = scsi_fill_cmd(op, cdbcmd, পরিমাণ(cdbcmd));
    যদি (blocksize < 0)
        ঘূৰাই default_process_op(op);

    সংরচনা mpt_lun_s *llun_gf =
        container_of(op->drive_fl, সংরচনা mpt_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ঘূৰাই mpt_scsi_cmd(iobase, op, cdbcmd, target, lun, blocksize);
}

স্থির শূন্য
mpt_scsi_init_lun(সংরচনা mpt_lun_s *llun, সংরচনা pci_device *pci,
                  u32 iobase, u8 target, u8 lun)
{
    memset(llun, 0, পরিমাণ(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

স্থির পূর্ণাঙ্ক
mpt_scsi_add_lun(u32 lun, সংরচনা drive_s *tmpl_drv)
{
    সংরচনা mpt_lun_s *tmpl_llun =
        container_of(tmpl_drv, সংরচনা mpt_lun_s, drive);
    সংরচনা mpt_lun_s *llun = malloc_fseg(পরিমাণ(*llun));
    যদি (!llun) {
        warn_noalloc();
        ঘূৰাই -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    ৱর্ণ *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    পূর্ণাঙ্ক prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    পূর্ণাঙ্ক ret = scsi_drive_setup(&llun->drive, name, prio);
    মুক্ত(name);
    যদি (ret) {
        গচ্ছ fail;
    }
    ঘূৰাই 0;

fail:
    মুক্ত(llun);
    ঘূৰাই -1;
}

স্থির শূন্য
mpt_scsi_scan_target(সংরচনা pci_device *pci, u32 iobase, u8 target)
{
    সংরচনা mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, target, 0);

    যদি (scsi_rep_luns_scan(&llun0.drive, mpt_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, mpt_scsi_add_lun);
}

স্থির অন্তঃস্থ শূন্য
mpt_out_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

স্থির শূন্য
init_mpt_scsi(শূন্য *data)
{
    সংরচনা pci_device *pci = data;
    u16 *বাৰ্তা_in_p;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    যদি (!iobase)
        ঘূৰাই;
    সংরচনা MptIOCInitReply MptIOCInitReply;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_out_doorbell(MPT_DOORBELL_বাৰ্তা_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_REPLY , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_out_doorbell(MPT_DOORBELL_HANDSHAKE,
                     পরিমাণ(MptIOCInitRequest)/পরিমাণ(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          পরিমাণ(MptIOCInitRequest)/পরিমাণ(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    বাৰ্তা_in_p = (u16 *)&MptIOCInitReply;
    যেতিয়ালৈ(বাৰ্তা_in_p != (u16 *)(&MptIOCInitReply + 1))
        *বাৰ্তা_in_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&reply_বাৰ্তা[0], iobase + MPT_REG_REP_Q);

    পূর্ণাঙ্ক i;
    বাবে (i = 0; i < 7; i++)
        mpt_scsi_scan_target(pci, iobase, i);
}

শূন্য
mpt_scsi_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        ঘূৰাই;
    }

    dprintf(3, "init MPT\n");

    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            run_thread(init_mpt_scsi, pci);
    }
}
