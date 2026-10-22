// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBALFLAT
#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "blockcmd.h" // scsi_drive_setup
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "fw/paravirt.h" // runningOnQEMU
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_DEVICE_ID
#અન્તર્ભાવ "pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "stacks.h" // run_thread
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // usleep

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

#define MPT_DOORBELL_સંદેશ_RESET 0x40
#define MPT_DOORBELL_HANDSHAKE 0x42

#define MPT_IMASK_DOORBELL 0x01
#define MPT_IMASK_REPLY    0x08

સંરચના mpt_lun_s {
    સંરચના drive_s drive;
    સંરચના pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

u8 reply_સંદેશ[4] __attribute((aligned(4))) VARLOW;

#define MPT_MESSAGE_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#define MPT_MESSAGE_HDR_FUNCTION_IOC_INIT               (0x02)

સ્થિર સંરચના MptIOCInitRequest
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
    u32    Host_Mfa_ઉપલું_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_Buffer_ઉપલું_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_MESSAGE_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .ReplyFrameSize = કદ(reply_સંદેશ),
    .Host_Mfa_ઉપલું_Addr = 0,
    .Sense_Buffer_ઉપલું_Addr = 0
};

સંરચના MptIOCInitReply {
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

પ્રકારવ્યાખ્યા સંરચના MptSCSIIORequest {
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
    u32    Sense_Buffer_નીચલું_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#define MPT_POLL_TIMEOUT  60000

પ્રકારવ્યાખ્યા સંરચના MptSGEntrySimple32 {
    u32 FlagsLength;
    u32 Data_Buffer_Address_નીચલું;
} __attribute__((packed)) MptSGEntrySimple32_t;

સ્થિર પૂર્ણાંક
mpt_scsi_cmd(u32 iobase, સંરચના disk_op_s *op,
             u8 *cdb, u16 target, u16 lun, u16 blocksize)
{
    u32 end = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    સંરચના scsi_req {
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
            .DataLength = op->ગણતરી * blocksize,
            .Sense_Buffer_નીચલું_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .FlagsLength = (op->ગણતરી * blocksize) | 0xD1000000,
            .Data_Buffer_Address_નીચલું = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    memcpy(req.scsi_io.CDB, cdb, 16);
    જો (blocksize) {
        જો (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } નહીંતર {
            req.scsi_io.Control = 1 << 24;
            req.sge.FlagsLength |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    માટે (;;) {
        જો (timer_check(end)) {
            પાછા DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        જો (istatus & MPT_IMASK_REPLY) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            જો (resp == req.scsi_io.MessageContext) {
                પાછા DISK_RET_SUCCESS;
            } નહીંતર જો (resp & 0x80000000) {
                outl((u32)&reply_સંદેશ[0], iobase + MPT_REG_REP_Q);
                પાછા DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

પૂર્ણાંક
mpt_scsi_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_MPT_SCSI)
        પાછા DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    પૂર્ણાંક blocksize = scsi_fill_cmd(op, cdbcmd, કદ(cdbcmd));
    જો (blocksize < 0)
        પાછા default_process_op(op);

    સંરચના mpt_lun_s *llun_gf =
        container_of(op->drive_fl, સંરચના mpt_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    પાછા mpt_scsi_cmd(iobase, op, cdbcmd, target, lun, blocksize);
}

સ્થિર શૂન્ય
mpt_scsi_init_lun(સંરચના mpt_lun_s *llun, સંરચના pci_device *pci,
                  u32 iobase, u8 target, u8 lun)
{
    memset(llun, 0, કદ(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

સ્થિર પૂર્ણાંક
mpt_scsi_add_lun(u32 lun, સંરચના drive_s *tmpl_drv)
{
    સંરચના mpt_lun_s *tmpl_llun =
        container_of(tmpl_drv, સંરચના mpt_lun_s, drive);
    સંરચના mpt_lun_s *llun = malloc_fseg(કદ(*llun));
    જો (!llun) {
        warn_noalloc();
        પાછા -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    અક્ષર *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    પૂર્ણાંક prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    પૂર્ણાંક ret = scsi_drive_setup(&llun->drive, name, prio);
    મુક્ત(name);
    જો (ret) {
        જાઓ fail;
    }
    પાછા 0;

fail:
    મુક્ત(llun);
    પાછા -1;
}

સ્થિર શૂન્ય
mpt_scsi_scan_target(સંરચના pci_device *pci, u32 iobase, u8 target)
{
    સંરચના mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, target, 0);

    જો (scsi_rep_luns_scan(&llun0.drive, mpt_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, mpt_scsi_add_lun);
}

સ્થિર અંતઃરેખ શૂન્ય
mpt_out_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

સ્થિર શૂન્ય
init_mpt_scsi(શૂન્ય *data)
{
    સંરચના pci_device *pci = data;
    u16 *સંદેશ_in_p;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    જો (!iobase)
        પાછા;
    સંરચના MptIOCInitReply MptIOCInitReply;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_out_doorbell(MPT_DOORBELL_સંદેશ_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_REPLY , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_out_doorbell(MPT_DOORBELL_HANDSHAKE,
                     કદ(MptIOCInitRequest)/કદ(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          કદ(MptIOCInitRequest)/કદ(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    સંદેશ_in_p = (u16 *)&MptIOCInitReply;
    જ્યારે(સંદેશ_in_p != (u16 *)(&MptIOCInitReply + 1))
        *સંદેશ_in_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&reply_સંદેશ[0], iobase + MPT_REG_REP_Q);

    પૂર્ણાંક i;
    માટે (i = 0; i < 7; i++)
        mpt_scsi_scan_target(pci, iobase, i);
}

શૂન્ય
mpt_scsi_setup(શૂન્ય)
{
    ASSERT32FLAT();
    જો (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        પાછા;
    }

    dprintf(3, "init MPT\n");

    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            run_thread(init_mpt_scsi, pci);
    }
}
