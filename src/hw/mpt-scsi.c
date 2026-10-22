// MPT Fusion boot support.
//
// Copyright (c) 2012 Verizon, Inc.
// Copyright (C) 2016 Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "blockcmd.h" // scsi_drive_setup
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "fw/paravirt.h" // runningOnQEMU
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_DEVICE_ID
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "stacks.h" // run_thread
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // usleep

#నిర్వచించు MPT_REG_DOORBELL  0x00
#నిర్వచించు MPT_REG_WRITE_SEQ 0x04
#నిర్వచించు MPT_REG_HOST_DIAG 0x08
#నిర్వచించు MPT_REG_TEST      0x0c
#నిర్వచించు MPT_REG_DIAG_దత్తాంశం 0x10
#నిర్వచించు MPT_REG_DIAG_ADDR 0x14
#నిర్వచించు MPT_REG_ISTATUS   0x30
#నిర్వచించు MPT_REG_IMASK     0x34
#నిర్వచించు MPT_REG_REQ_Q     0x40
#నిర్వచించు MPT_REG_REP_Q     0x44

#నిర్వచించు MPT_DOORBELL_సందేశం_RESET 0x40
#నిర్వచించు MPT_DOORBELL_HANDSHAKE 0x42

#నిర్వచించు MPT_IMASK_DOORBELL 0x01
#నిర్వచించు MPT_IMASK_జవాబు    0x08

నిర్మాణం mpt_lun_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం pci_device *pci;
    u32 iobase;
    u8 లక్ష్యం;
    u8 lun;
};

u8 జవాబు_సందేశం[4] __attribute((aligned(4))) VARLOW;

#నిర్వచించు MPT_సందేశం_HDR_FUNCTION_SCSI_IO_REQUEST        (0x00)
#నిర్వచించు MPT_సందేశం_HDR_FUNCTION_IOC_INIT               (0x02)

స్థిర నిర్మాణం MptIOCInitRequest
{
    u8     WhoInit;             /* Which system sent this init request. */
    u8     Reserved1;           /* Reserved */
    u8     Chain_స్థానభ్రంశం;         /* Chain offset in the SG list. */
    u8     Function;            /* Function to execute. */
    u8     Flags;               /* Flags */
    u8     MaxDevices;          /* Max devices the driver can handle. */
    u8     MaxBuses;            /* Max buses the driver can handle. */
    u8     సందేశం_Flags;        /* Message flags. */
    u32    సందేశం_Context;      /* Message context ID. */
    u16    జవాబు_చట్రం_కొలత;      /* Reply frame size. */
    u16    Reserved2;           /* Reserved */
    u32    Host_Mfa_ఎగువ_Addr;     /* Upper 32bit of the message frames. */
    u32    Sense_బఫరు_ఎగువ_Addr; /* Upper 32bit of the sense buffer. */
} MptIOCInitRequest = {
    .WhoInit = 2,
    .Function = MPT_సందేశం_HDR_FUNCTION_IOC_INIT,
    .MaxDevices = 8,
    .MaxBuses = 1,
    .జవాబు_చట్రం_కొలత = పరిమాణం(జవాబు_సందేశం),
    .Host_Mfa_ఎగువ_Addr = 0,
    .Sense_బఫరు_ఎగువ_Addr = 0
};

నిర్మాణం Mpt_IOCInit_జవాబు {
    u8     WhoInit;     /* Which subsystem sent this init request. */
    u8     Reserved1;   /* Reserved */
    u8     సందేశం_నిడివి; /* Message length */
    u8     Function;    /* Function. */
    u8     Flags;       /* Flags */
    u8     MaxDevices;  /* Maximum number of devices the driver can handle. */
    u8     MaxBuses;    /* Maximum number of busses the driver can handle. */
    u8     సందేశం_Flags; /* Message flags. */
    u32    సందేశం_Context; /* Message context ID */
    u16    Reserved2;   /* Reserved */
    u16    IOCStatus;   /* IO controller status. */
    u32    IOCLogInfo;  /* IO controller log information. */
};

రకనిర్వచనం నిర్మాణం MptSCSIIORequest {
    u8     లక్ష్యం_ID;            /* Target ID */
    u8     Bus;                 /* Bus number */
    u8     Chain_స్థానభ్రంశం;         /* Chain offset */
    u8     Function;            /* Function number. */
    u8     CDBLength;           /* CDB length. */
    u8     Sense_బఫరు_నిడివి;   /* Sense buffer length. */
    u8     Reserved;            /* Reserved */
    u8     సందేశం_Flags;        /* Message flags. */
    u32    సందేశం_Context;      /* Message context ID. */
    u8     LUN[8];              /* LUN */
    u32    Control;             /* Control values. */
    u8     CDB[16];             /* The CDB. */
    u32    దత్తాంశం_నిడివి;          /* Data length. */
    u32    Sense_బఫరు_దిగువ_Addr;  /* Sense buffer low 32bit address. */
} __attribute__((packed)) MptSCSIIORequest_t;

#నిర్వచించు MPT_POLL_TIMEOUT  60000

రకనిర్వచనం నిర్మాణం MptSGEntrySimple32 {
    u32 Flags_నిడివి;
    u32 దత్తాంశం_బఫరు_చిరునామా_దిగువ;
} __attribute__((packed)) MptSGEntrySimple32_t;

స్థిర పూర్ణాంకం
mpt_scsi_cmd(u32 iobase, నిర్మాణం disk_op_s *op,
             u8 *cdb, u16 లక్ష్యం, u16 lun, u16 blocksize)
{
    u32 చివర = timer_calc(MPT_POLL_TIMEOUT);

    u8 sense_buf[18];
    నిర్మాణం scsi_req {
        MptSCSIIORequest_t      scsi_io;
        MptSGEntrySimple32_t    sge;
    } __attribute__((packed, aligned(4))) req = {
        .scsi_io = {
            .లక్ష్యం_ID = లక్ష్యం,
            .Bus = 0,
            .Function = MPT_సందేశం_HDR_FUNCTION_SCSI_IO_REQUEST,
            .CDBLength = 16,
            .Sense_బఫరు_నిడివి = 18,
            .సందేశం_Context = చివర & 0x7fffffff,
            .దత్తాంశం_నిడివి = op->లెక్క * blocksize,
            .Sense_బఫరు_దిగువ_Addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &sense_buf[0]),
        },
        .sge = {
            /* end of list, simple entry, end of buffer, last element */
            .Flags_నిడివి = (op->లెక్క * blocksize) | 0xD1000000,
            .దత్తాంశం_బఫరు_చిరునామా_దిగువ = (u32)op->buf_fl,
        }
    };

    req.scsi_io.LUN[1] = lun;
    జ్ఞాపకం_నకలు(req.scsi_io.CDB, cdb, 16);
    ఒకవేళ (blocksize) {
        ఒకవేళ (scsi_is_read(op)) {
            req.scsi_io.Control = 2 << 24;
        } లేకపోతే {
            req.scsi_io.Control = 1 << 24;
            req.sge.Flags_నిడివి |= 0x04000000;
        }
    }

    outl((u32)MAKE_FLATPTR(GET_SEG(SS), &req), iobase + MPT_REG_REQ_Q);

    కొరకు (;;) {
        ఒకవేళ (timer_check(చివర)) {
            తిరిగి DISK_RET_ETIMEOUT;
        }

        u32 istatus = inl(iobase + MPT_REG_ISTATUS);
        ఒకవేళ (istatus & MPT_IMASK_జవాబు) {
            u32 resp = inl(iobase + MPT_REG_REP_Q);
            /* another read to turn interrupt off */
            inl(iobase + MPT_REG_REP_Q);
            ఒకవేళ (resp == req.scsi_io.సందేశం_Context) {
                తిరిగి DISK_RET_SUCCESS;
            } లేకపోతే ఒకవేళ (resp & 0x80000000) {
                outl((u32)&జవాబు_సందేశం[0], iobase + MPT_REG_REP_Q);
                తిరిగి DISK_RET_EBADTRACK;
            }
        }
        usleep(50);
    }
}

పూర్ణాంకం
mpt_scsi_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_MPT_SCSI)
        తిరిగి DISK_RET_EBADTRACK;

    u8 cdbcmd[16];
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, cdbcmd, పరిమాణం(cdbcmd));
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);

    నిర్మాణం mpt_lun_s *llun_gf =
        container_యొక్క(op->drive_fl, నిర్మాణం mpt_lun_s, drive);
    u16 లక్ష్యం = GET_GLOBALFLAT(llun_gf->లక్ష్యం);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    తిరిగి mpt_scsi_cmd(iobase, op, cdbcmd, లక్ష్యం, lun, blocksize);
}

స్థిర శూన్య
mpt_scsi_init_lun(నిర్మాణం mpt_lun_s *llun, నిర్మాణం pci_device *pci,
                  u32 iobase, u8 లక్ష్యం, u8 lun)
{
    memset(llun, 0, పరిమాణం(*llun));
    llun->drive.type = DTYPE_MPT_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->లక్ష్యం = లక్ష్యం;
    llun->lun = lun;
    llun->iobase = iobase;
}

స్థిర పూర్ణాంకం
mpt_scsi_కూడు_lun(u32 lun, నిర్మాణం drive_s *tmpl_drv)
{
    నిర్మాణం mpt_lun_s *tmpl_llun =
        container_యొక్క(tmpl_drv, నిర్మాణం mpt_lun_s, drive);
    నిర్మాణం mpt_lun_s *llun = malloc_fseg(పరిమాణం(*llun));
    ఒకవేళ (!llun) {
        warn_noalloc();
        తిరిగి -1;
    }
    mpt_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->లక్ష్యం, lun);

    boot_lchs_కనుగొను_scsi_device(llun->pci, llun->లక్ష్యం, llun->lun,
                               &(llun->drive.lchs));
    అక్షరం *name = znprintf(MAXDESCSIZE, "mpt %pP %d:%d",
                          llun->pci, llun->లక్ష్యం, llun->lun);
    పూర్ణాంకం prio = bootprio_కనుగొను_scsi_device(llun->pci, llun->లక్ష్యం, llun->lun);
    పూర్ణాంకం ret = scsi_drive_అమరిక(&llun->drive, name, prio);
    విడుదల(name);
    ఒకవేళ (ret) {
        వెళ్ళు fail;
    }
    తిరిగి 0;

fail:
    విడుదల(llun);
    తిరిగి -1;
}

స్థిర శూన్య
mpt_scsi_పరిశీలన_లక్ష్యం(నిర్మాణం pci_device *pci, u32 iobase, u8 లక్ష్యం)
{
    నిర్మాణం mpt_lun_s llun0;

    mpt_scsi_init_lun(&llun0, pci, iobase, లక్ష్యం, 0);

    ఒకవేళ (scsi_rep_luns_పరిశీలన(&llun0.drive, mpt_scsi_కూడు_lun) < 0)
        scsi_sequential_పరిశీలన(&llun0.drive, 8, mpt_scsi_కూడు_lun);
}

స్థిర అంతఃవరుస శూన్య
mpt_బహిర్గతం_doorbell(u8 func, u8 arg, u16 iobase)
{
    outl((func << 24) | (arg << 16), iobase + MPT_REG_DOORBELL);
}

స్థిర శూన్య
init_mpt_scsi(శూన్య *దత్తాంశం)
{
    నిర్మాణం pci_device *pci = దత్తాంశం;
    u16 *సందేశం_లోపల_p;
    u32 iobase = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!iobase)
        తిరిగి;
    నిర్మాణం Mpt_IOCInit_జవాబు Mpt_IOCInit_జవాబు;
    pci_enable_busmaster(pci);

    dprintf(1, "found mpt-scsi(%04x) at %pP, io @ %x\n"
            , pci->device, pci, iobase);

    // reset
    mpt_బహిర్గతం_doorbell(MPT_DOORBELL_సందేశం_RESET, 0, iobase);
    outl(MPT_IMASK_DOORBELL|MPT_IMASK_జవాబు , iobase + MPT_REG_IMASK);
    outl(0, iobase + MPT_REG_ISTATUS);

    // send IOC Init message through the doorbell
    mpt_బహిర్గతం_doorbell(MPT_DOORBELL_HANDSHAKE,
                     పరిమాణం(MptIOCInitRequest)/పరిమాణం(u32),
                     iobase);

    outsl(iobase + MPT_REG_DOORBELL,
          (u32 *)&MptIOCInitRequest,
          పరిమాణం(MptIOCInitRequest)/పరిమాణం(u32));

    // Read the reply 16 bits at a time.  Cannot use insl
    // because the port is 32 bits wide.
    సందేశం_లోపల_p = (u16 *)&Mpt_IOCInit_జవాబు;
    అయితే(సందేశం_లోపల_p != (u16 *)(&Mpt_IOCInit_జవాబు + 1))
        *సందేశం_లోపల_p++ = (u16)inl(iobase + MPT_REG_DOORBELL);

    // Eat doorbell interrupt
    outl(0, iobase + MPT_REG_ISTATUS);

    // Post reply message used for SCSI errors
    outl((u32)&జవాబు_సందేశం[0], iobase + MPT_REG_REP_Q);

    పూర్ణాంకం i;
    కొరకు (i = 0; i < 7; i++)
        mpt_scsi_పరిశీలన_లక్ష్యం(pci, iobase, i);
}

శూన్య
mpt_scsi_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_MPT_SCSI || !runningOnQEMU()) {
        తిరిగి;
    }

    dprintf(3, "init MPT\n");

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->vendor == PCI_VENDOR_ID_LSI_LOGIC
            && (pci->device == PCI_DEVICE_ID_LSI_53C1030
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068
                || pci->device == PCI_DEVICE_ID_LSI_SAS1068E))
            నిరంతరం_thread(init_mpt_scsi, pci);
    }
}
