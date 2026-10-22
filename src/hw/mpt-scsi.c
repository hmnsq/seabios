// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBALFLAT
#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "blockcmd.h" // scsi_drive_setup
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "fw/paravirt.h" // runningOnQEMU
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_DEVICE_ID
#अन्तर्भाव "pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "stacks.h" // run_thread
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // usleep

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

#define MPT_DOORBELL_सन्देशः_RESET 0x40
#define MPT_DOORBELL_HANDSHAKE 0x42

#define MPT_IMASK_DOORBELL 0x01
#define MPT_IMASK_REPLY    0x08

संरचना mpt_lun_s {
    संरचना drive_s drive;
    संरचना pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

u8 reply_सन्देशः[4] __attribute((aligned(4))) VARLOW;

#define MPT_MESSAGE_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#define MPT_MESSAGE_HDR_FUNCTION_IOC_INIT               (0x02)

स्थिर संरचना MptIOCInitRequest
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
    u32    Host_Mfa_उच्चः_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_Buffer_उच्चः_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_MESSAGE_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .ReplyFrameSize = परिमाण(reply_सन्देशः),
    .Host_Mfa_उच्चः_Addr = 0,
    .Sense_Buffer_उच्चः_Addr = 0
};

संरचना MptIOCInitReply {
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

प्रकारनिर्देश संरचना MptSCSIIORequest {
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
    u32    Sense_Buffer_नीचः_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#define MPT_POLL_TIMEOUT  60000

प्रकारनिर्देश संरचना MptSGEntrySimple32 {
    u32 FlagsLength;
    u32 Data_Buffer_Address_नीचः;
} __attribute__((packed)) MptSGEntrySimple32_t;

स्थिर पूर्णाङ्क
mpt_scsi_cmd(u32 iobase, संरचना disk_op_s *op,
             u8 *cdb, u16 target, u16 lun, u16 blocksize)
{
    u32 end = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    संरचना scsi_req {
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
            .DataLength = op->गणनम् * blocksize,
            .Sense_Buffer_नीचः_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .FlagsLength = (op->गणनम् * blocksize) | 0xD1000000,
            .Data_Buffer_Address_नीचः = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    memcpy(req.scsi_io.CDB, cdb, 16);
    सचे (blocksize) {
        सचे (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } अञ्ञथा {
            req.scsi_io.Control = 1 << 24;
            req.sge.FlagsLength |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    अत्थाय (;;) {
        सचे (timer_check(end)) {
            पटिनिवत्त DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        सचे (istatus & MPT_IMASK_REPLY) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            सचे (resp == req.scsi_io.MessageContext) {
                पटिनिवत्त DISK_RET_SUCCESS;
            } अञ्ञथा सचे (resp & 0x80000000) {
                outl((u32)&reply_सन्देशः[0], iobase + MPT_REG_REP_Q);
                पटिनिवत्त DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

पूर्णाङ्क
mpt_scsi_process_op(संरचना disk_op_s *op)
{
    सचे (!CONFIG_MPT_SCSI)
        पटिनिवत्त DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, cdbcmd, परिमाण(cdbcmd));
    सचे (blocksize < 0)
        पटिनिवत्त default_process_op(op);

    संरचना mpt_lun_s *llun_gf =
        container_of(op->drive_fl, संरचना mpt_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    पटिनिवत्त mpt_scsi_cmd(iobase, op, cdbcmd, target, lun, blocksize);
}

स्थिर शून्य
mpt_scsi_init_lun(संरचना mpt_lun_s *llun, संरचना pci_device *pci,
                  u32 iobase, u8 target, u8 lun)
{
    memset(llun, 0, परिमाण(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

स्थिर पूर्णाङ्क
mpt_scsi_add_lun(u32 lun, संरचना drive_s *tmpl_drv)
{
    संरचना mpt_lun_s *tmpl_llun =
        container_of(tmpl_drv, संरचना mpt_lun_s, drive);
    संरचना mpt_lun_s *llun = malloc_fseg(परिमाण(*llun));
    सचे (!llun) {
        warn_noalloc();
        पटिनिवत्त -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    वर्ण *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    पूर्णाङ्क prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    पूर्णाङ्क ret = scsi_drive_setup(&llun->drive, name, prio);
    मुक्त(name);
    सचे (ret) {
        गच्छ fail;
    }
    पटिनिवत्त 0;

fail:
    मुक्त(llun);
    पटिनिवत्त -1;
}

स्थिर शून्य
mpt_scsi_scan_target(संरचना pci_device *pci, u32 iobase, u8 target)
{
    संरचना mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, target, 0);

    सचे (scsi_rep_luns_scan(&llun0.drive, mpt_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, mpt_scsi_add_lun);
}

स्थिर अन्तःस्थ शून्य
mpt_out_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

स्थिर शून्य
init_mpt_scsi(शून्य *data)
{
    संरचना pci_device *pci = data;
    u16 *सन्देशः_in_p;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    सचे (!iobase)
        पटिनिवत्त;
    संरचना MptIOCInitReply MptIOCInitReply;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_out_doorbell(MPT_DOORBELL_सन्देशः_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_REPLY , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_out_doorbell(MPT_DOORBELL_HANDSHAKE,
                     परिमाण(MptIOCInitRequest)/परिमाण(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          परिमाण(MptIOCInitRequest)/परिमाण(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    सन्देशः_in_p = (u16 *)&MptIOCInitReply;
    याव(सन्देशः_in_p != (u16 *)(&MptIOCInitReply + 1))
        *सन्देशः_in_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&reply_सन्देशः[0], iobase + MPT_REG_REP_Q);

    पूर्णाङ्क i;
    अत्थाय (i = 0; i < 7; i++)
        mpt_scsi_scan_target(pci, iobase, i);
}

शून्य
mpt_scsi_setup(शून्य)
{
    ASSERT32FLAT();
    सचे (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        पटिनिवत्त;
    }

    dprintf(3, "init MPT\n");

    संरचना pci_device *pci;
    foreachpci(pci) {
        सचे (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            run_thread(init_mpt_scsi, pci);
    }
}
