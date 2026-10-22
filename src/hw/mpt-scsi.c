// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBALFLAT
#समावेश "block.h" // struct drive_s
#समावेश "blockcmd.h" // scsi_drive_setup
#समावेश "config.h" // CONFIG_*
#समावेश "fw/paravirt.h" // runningOnQEMU
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_DEVICE_ID
#समावेश "pci_regs.h" // PCI_VENDOR_ID
#समावेश "stacks.h" // run_thread
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // usleep

#घोषणा MPT_REG_DOORBELL  0x00
#घोषणा MPT_REG_WRITE_SEQ 0x04
#घोषणा MPT_REG_HOST_DIAG 0x08
#घोषणा MPT_REG_TEST      0x0c
#घोषणा MPT_REG_DIAG_तथ्य 0x10
#घोषणा MPT_REG_DIAG_ADDR 0x14
#घोषणा MPT_REG_ISTATUS   0x30
#घोषणा MPT_REG_IMASK     0x34
#घोषणा MPT_REG_REQ_Q     0x40
#घोषणा MPT_REG_REP_Q     0x44

#घोषणा MPT_DOORBELL_संदेश_RESET 0x40
#घोषणा MPT_DOORBELL_HANDSHAKE 0x42

#घोषणा MPT_IMASK_DOORBELL 0x01
#घोषणा MPT_IMASK_REPLY    0x08

काष्ठा mpt_lun_s {
    काष्ठा drive_s drive;
    काष्ठा pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

u8 reply_संदेश[4] __attribute((aligned(4))) VARLOW;

#घोषणा MPT_MESSAGE_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#घोषणा MPT_MESSAGE_HDR_FUNCTION_IOC_INIT               (0x02)

अटल काष्ठा MptIOCInitRequest
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
    u16    Reply_Frame_आकार;      /* Reply frame size. */
    u16    Reserved2;           /* Reserved */
    u32    Host_Mfa_ऊपरी_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_बफर_ऊपरी_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_MESSAGE_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .Reply_Frame_आकार = माप(reply_संदेश),
    .Host_Mfa_ऊपरी_Addr = 0,
    .Sense_बफर_ऊपरी_Addr = 0
};

काष्ठा MptIOCInitReply {
    u8     WhoInit;     /* Which subsystem sent this init request. */
    u8     Reserved1;   /* Reserved */
    u8     Message_लंबाई; /* Message length */
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

प्रकार काष्ठा MptSCSIIORequest {
    u8     TargetID;            /* Target ID */
    u8     Bus;                 /* Bus number */
    u8     ChainOffset;         /* Chain offset */
    u8     Function;            /* Function number. */
    u8     CDBLength;           /* CDB length. */
    u8     Sense_बफर_लंबाई;   /* Sense buffer length. */
    u8     Reserved;            /* Reserved */
    u8     MessageFlags;        /* Message flags. */
    u32    MessageContext;      /* Message context ID. */
    u8     LUN[8];              /* LUN */
    u32    Control;             /* Control values. */
    u8     CDB[16];             /* The CDB. */
    u32    तथ्य_लंबाई;          /* Data length. */
    u32    Sense_बफर_निचला_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#घोषणा MPT_POLL_TIMEOUT  60000

प्रकार काष्ठा MptSGEntrySimple32 {
    u32 Flags_लंबाई;
    u32 तथ्य_बफर_Address_निचला;
} __attribute__((packed)) MptSGEntrySimple32_t;

अटल पूर्णांक
mpt_scsi_cmd(u32 iobase, काष्ठा disk_op_s *op,
             u8 *cdb, u16 target, u16 lun, u16 blocksize)
{
    u32 end = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    काष्ठा scsi_req {
        MptSCSIIORequest_t      scsi_io;
        MptSGEntrySimple32_t    sge;
    } __attribute__((packed, aligned(4))) req = {
        .scsi_io = {
            .TargetID = target,
            .Bus = 0,
            .Function = MPT_MESSAGE_HDR_FUNCTION_SCSI_IO_REQUEST,
            .CDBLength = 16,
            .Sense_बफर_लंबाई = 18,
            .MessageContext = end & 0x7fffffff,
            .तथ्य_लंबाई = op->गिनती * blocksize,
            .Sense_बफर_निचला_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .Flags_लंबाई = (op->गिनती * blocksize) | 0xD1000000,
            .तथ्य_बफर_Address_निचला = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    स_नकल(req.scsi_io.CDB, cdb, 16);
    अगर (blocksize) {
        अगर (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } अन्यथा {
            req.scsi_io.Control = 1 << 24;
            req.sge.Flags_लंबाई |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    क्रम (;;) {
        अगर (timer_check(end)) {
            वापस DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        अगर (istatus & MPT_IMASK_REPLY) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            अगर (resp == req.scsi_io.MessageContext) {
                वापस DISK_RET_SUCCESS;
            } अन्यथा अगर (resp & 0x80000000) {
                outl((u32)&reply_संदेश[0], iobase + MPT_REG_REP_Q);
                वापस DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

पूर्णांक
mpt_scsi_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_MPT_SCSI)
        वापस DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    पूर्णांक blocksize = scsi_fill_cmd(op, cdbcmd, माप(cdbcmd));
    अगर (blocksize < 0)
        वापस default_process_op(op);

    काष्ठा mpt_lun_s *llun_gf =
        container_of(op->drive_fl, काष्ठा mpt_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    वापस mpt_scsi_cmd(iobase, op, cdbcmd, target, lun, blocksize);
}

अटल व्योम
mpt_scsi_init_lun(काष्ठा mpt_lun_s *llun, काष्ठा pci_device *pci,
                  u32 iobase, u8 target, u8 lun)
{
    स_रखो(llun, 0, माप(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

अटल पूर्णांक
mpt_scsi_add_lun(u32 lun, काष्ठा drive_s *tmpl_drv)
{
    काष्ठा mpt_lun_s *tmpl_llun =
        container_of(tmpl_drv, काष्ठा mpt_lun_s, drive);
    काष्ठा mpt_lun_s *llun = malloc_fseg(माप(*llun));
    अगर (!llun) {
        warn_noalloc();
        वापस -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    अक्षर *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    पूर्णांक prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    पूर्णांक ret = scsi_drive_setup(&llun->drive, name, prio);
    मुक्त(name);
    अगर (ret) {
        जाओ fail;
    }
    वापस 0;

fail:
    मुक्त(llun);
    वापस -1;
}

अटल व्योम
mpt_scsi_scan_target(काष्ठा pci_device *pci, u32 iobase, u8 target)
{
    काष्ठा mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, target, 0);

    अगर (scsi_rep_luns_scan(&llun0.drive, mpt_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, mpt_scsi_add_lun);
}

अटल अंतरभूत व्योम
mpt_out_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

अटल व्योम
init_mpt_scsi(व्योम *तथ्य)
{
    काष्ठा pci_device *pci = तथ्य;
    u16 *संदेश_in_p;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    अगर (!iobase)
        वापस;
    काष्ठा MptIOCInitReply MptIOCInitReply;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_out_doorbell(MPT_DOORBELL_संदेश_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_REPLY , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_out_doorbell(MPT_DOORBELL_HANDSHAKE,
                     माप(MptIOCInitRequest)/माप(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          माप(MptIOCInitRequest)/माप(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    संदेश_in_p = (u16 *)&MptIOCInitReply;
    जबतक(संदेश_in_p != (u16 *)(&MptIOCInitReply + 1))
        *संदेश_in_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&reply_संदेश[0], iobase + MPT_REG_REP_Q);

    पूर्णांक i;
    क्रम (i = 0; i < 7; i++)
        mpt_scsi_scan_target(pci, iobase, i);
}

व्योम
mpt_scsi_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        वापस;
    }

    dprintf(3, "init MPT\n");

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            run_thread(init_mpt_scsi, pci);
    }
}
