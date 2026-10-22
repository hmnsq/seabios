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

#define MPT_DOORBELL_MSG_RESET 0x40
#define MPT_DOORBELL_HANDSHAKE 0x42

#define MPT_IMASK_DOORBELL 0x01
#define MPT_IMASK_REPLY    0x08

بنية mpt_lun_s {
    بنية drive_s drive;
    بنية pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

u8 reply_msg[4] __attribute((aligned(4))) VARLOW;

#define MPT_MESSAGE_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#define MPT_MESSAGE_HDR_FUNCTION_IOC_INIT               (0x02)

ساكن بنية MptIOCInitRequest
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
    u32    HostMfaHighAddr;     /* Upper 32bit of the message frames. */
    u32    SenseBufferHighAddr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_MESSAGE_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .ReplyFrameSize = حجم(reply_msg),
    .HostMfaHighAddr = 0,
    .SenseBufferHighAddr = 0
};

بنية MptIOCInitReply {
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

تعريفنوع بنية MptSCSIIORequest {
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
    u32    SenseBufferLowAddr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#define MPT_POLL_TIMEOUT  60000

تعريفنوع بنية MptSGEntrySimple32 {
    u32 FlagsLength;
    u32 DataBufferAddressLow;
} __attribute__((packed)) MptSGEntrySimple32_t;

ساكن صحيح
mpt_scsi_cmd(u32 iobase, بنية disk_op_s *op,
             u8 *cdb, u16 target, u16 lun, u16 blocksize)
{
    u32 end = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    بنية scsi_req {
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
            .DataLength = op->count * blocksize,
            .SenseBufferLowAddr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .FlagsLength = (op->count * blocksize) | 0xD1000000,
            .DataBufferAddressLow = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    memcpy(req.scsi_io.CDB, cdb, 16);
    ܐܢ (blocksize) {
        ܐܢ (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } ܐܚܪܢܐ {
            req.scsi_io.Control = 1 << 24;
            req.sge.FlagsLength |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    لكل (;;) {
        ܐܢ (timer_check(end)) {
            ܦܢܐ DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        ܐܢ (istatus & MPT_IMASK_REPLY) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            ܐܢ (resp == req.scsi_io.MessageContext) {
                ܦܢܐ DISK_RET_SUCCESS;
            } ܐܚܪܢܐ ܐܢ (resp & 0x80000000) {
                outl((u32)&reply_msg[0], iobase + MPT_REG_REP_Q);
                ܦܢܐ DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

صحيح
mpt_scsi_process_op(بنية disk_op_s *op)
{
    ܐܢ (!CONFIG_MPT_SCSI)
        ܦܢܐ DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    صحيح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    ܐܢ (blocksize < 0)
        ܦܢܐ default_process_op(op);

    بنية mpt_lun_s *llun_gf =
        container_of(op->drive_fl, بنية mpt_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ܦܢܐ mpt_scsi_cmd(iobase, op, cdbcmd, target, lun, blocksize);
}

ساكن فراغ
mpt_scsi_init_lun(بنية mpt_lun_s *llun, بنية pci_device *pci,
                  u32 iobase, u8 target, u8 lun)
{
    memset(llun, 0, حجم(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

ساكن صحيح
mpt_scsi_add_lun(u32 lun, بنية drive_s *tmpl_drv)
{
    بنية mpt_lun_s *tmpl_llun =
        container_of(tmpl_drv, بنية mpt_lun_s, drive);
    بنية mpt_lun_s *llun = malloc_fseg(حجم(*llun));
    ܐܢ (!llun) {
        warn_noalloc();
        ܦܢܐ -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    حرف *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    صحيح prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    صحيح ret = scsi_drive_setup(&llun->drive, name, prio);
    حرر(name);
    ܐܢ (ret) {
        اذهب fail;
    }
    ܦܢܐ 0;

fail:
    حرر(llun);
    ܦܢܐ -1;
}

ساكن فراغ
mpt_scsi_scan_target(بنية pci_device *pci, u32 iobase, u8 target)
{
    بنية mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, target, 0);

    ܐܢ (scsi_rep_luns_scan(&llun0.drive, mpt_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, mpt_scsi_add_lun);
}

ساكن مضمن فراغ
mpt_out_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

ساكن فراغ
init_mpt_scsi(فراغ *data)
{
    بنية pci_device *pci = data;
    u16 *msg_in_p;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    ܐܢ (!iobase)
        ܦܢܐ;
    بنية MptIOCInitReply MptIOCInitReply;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_out_doorbell(MPT_DOORBELL_MSG_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_REPLY , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_out_doorbell(MPT_DOORBELL_HANDSHAKE,
                     حجم(MptIOCInitRequest)/حجم(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          حجم(MptIOCInitRequest)/حجم(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    msg_in_p = (u16 *)&MptIOCInitReply;
    ܟܡܐ(msg_in_p != (u16 *)(&MptIOCInitReply + 1))
        *msg_in_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&reply_msg[0], iobase + MPT_REG_REP_Q);

    صحيح i;
    لكل (i = 0; i < 7; i++)
        mpt_scsi_scan_target(pci, iobase, i);
}

فراغ
mpt_scsi_setup(فراغ)
{
    ASSERT32FLAT();
    ܐܢ (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        ܦܢܐ;
    }

    dprintf(3, "init MPT\n");

    بنية pci_device *pci;
    foreachpci(pci) {
        ܐܢ (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            run_thread(init_mpt_scsi, pci);
    }
}
