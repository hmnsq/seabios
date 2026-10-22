// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBALFLAT
#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // scsi_drive_setup
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "fw/paravirt.h" // runningOnQEMU
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_DEVICE_ID
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "stacks.h" // run_thread
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // usleep

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

#define MPT_DOORBELL_ସନ୍ଦେଶ_RESET 0x40
#define MPT_DOORBELL_HANDSHAKE 0x42

#define MPT_IMASK_DOORBELL 0x01
#define MPT_IMASK_REPLY    0x08

ସଂରଚନା mpt_lun_s {
    ସଂରଚନା drive_s drive;
    ସଂରଚନା pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

u8 reply_ସନ୍ଦେଶ[4] __attribute((aligned(4))) VARLOW;

#define MPT_MESSAGE_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#define MPT_MESSAGE_HDR_FUNCTION_IOC_INIT               (0x02)

ସ୍ଥିର ସଂରଚନା MptIOCInitRequest
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
    u32    Host_Mfa_ଉପର_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_Buffer_ଉପର_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_MESSAGE_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .ReplyFrameSize = ଆକାର(reply_ସନ୍ଦେଶ),
    .Host_Mfa_ଉପର_Addr = 0,
    .Sense_Buffer_ଉପର_Addr = 0
};

ସଂରଚନା MptIOCInitReply {
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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା MptSCSIIORequest {
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
    u32    Sense_Buffer_ତଳ_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#define MPT_POLL_TIMEOUT  60000

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା MptSGEntrySimple32 {
    u32 FlagsLength;
    u32 Data_Buffer_Address_ତଳ;
} __attribute__((packed)) MptSGEntrySimple32_t;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
mpt_scsi_cmd(u32 iobase, ସଂରଚନା disk_op_s *op,
             u8 *cdb, u16 target, u16 lun, u16 blocksize)
{
    u32 end = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    ସଂରଚନା scsi_req {
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
            .DataLength = op->ଗଣନା_2 * blocksize,
            .Sense_Buffer_ତଳ_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .FlagsLength = (op->ଗଣନା_2 * blocksize) | 0xD1000000,
            .Data_Buffer_Address_ତଳ = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    memcpy(req.scsi_io.CDB, cdb, 16);
    ଯଦି (blocksize) {
        ଯଦି (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } ଅନ୍ୟଥା {
            req.scsi_io.Control = 1 << 24;
            req.sge.FlagsLength |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    ପାଇଁ (;;) {
        ଯଦି (timer_check(end)) {
            ଫେରନ୍ତୁ DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        ଯଦି (istatus & MPT_IMASK_REPLY) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            ଯଦି (resp == req.scsi_io.MessageContext) {
                ଫେରନ୍ତୁ DISK_RET_SUCCESS;
            } ଅନ୍ୟଥା ଯଦି (resp & 0x80000000) {
                outl((u32)&reply_ସନ୍ଦେଶ[0], iobase + MPT_REG_REP_Q);
                ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
mpt_scsi_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_MPT_SCSI)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, cdbcmd, ଆକାର(cdbcmd));
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);

    ସଂରଚନା mpt_lun_s *llun_gf =
        container_of(op->drive_fl, ସଂରଚନା mpt_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ଫେରନ୍ତୁ mpt_scsi_cmd(iobase, op, cdbcmd, target, lun, blocksize);
}

ସ୍ଥିର ଶୂନ୍ୟ
mpt_scsi_init_lun(ସଂରଚନା mpt_lun_s *llun, ସଂରଚନା pci_device *pci,
                  u32 iobase, u8 target, u8 lun)
{
    memset(llun, 0, ଆକାର(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
mpt_scsi_add_lun(u32 lun, ସଂରଚନା drive_s *tmpl_drv)
{
    ସଂରଚନା mpt_lun_s *tmpl_llun =
        container_of(tmpl_drv, ସଂରଚନା mpt_lun_s, drive);
    ସଂରଚନା mpt_lun_s *llun = malloc_fseg(ଆକାର(*llun));
    ଯଦି (!llun) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    ଅକ୍ଷର *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_drive_setup(&llun->drive, name, prio);
    ମୁକ୍ତ(name);
    ଯଦି (ret) {
        ଯାଅ fail;
    }
    ଫେରନ୍ତୁ 0;

fail:
    ମୁକ୍ତ(llun);
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ଶୂନ୍ୟ
mpt_scsi_scan_target(ସଂରଚନା pci_device *pci, u32 iobase, u8 target)
{
    ସଂରଚନା mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, target, 0);

    ଯଦି (scsi_rep_luns_scan(&llun0.drive, mpt_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, mpt_scsi_add_lun);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
mpt_out_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

ସ୍ଥିର ଶୂନ୍ୟ
init_mpt_scsi(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା pci_device *pci = data;
    u16 *ସନ୍ଦେଶ_in_p;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    ଯଦି (!iobase)
        ଫେରନ୍ତୁ;
    ସଂରଚନା MptIOCInitReply MptIOCInitReply;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_out_doorbell(MPT_DOORBELL_ସନ୍ଦେଶ_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_REPLY , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_out_doorbell(MPT_DOORBELL_HANDSHAKE,
                     ଆକାର(MptIOCInitRequest)/ଆକାର(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          ଆକାର(MptIOCInitRequest)/ଆକାର(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    ସନ୍ଦେଶ_in_p = (u16 *)&MptIOCInitReply;
    ଯେତେବେଳେ(ସନ୍ଦେଶ_in_p != (u16 *)(&MptIOCInitReply + 1))
        *ସନ୍ଦେଶ_in_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&reply_ସନ୍ଦେଶ[0], iobase + MPT_REG_REP_Q);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < 7; i++)
        mpt_scsi_scan_target(pci, iobase, i);
}

ଶୂନ୍ୟ
mpt_scsi_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        ଫେରନ୍ତୁ;
    }

    dprintf(3, "init MPT\n");

    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            run_thread(init_mpt_scsi, pci);
    }
}
