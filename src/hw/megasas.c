// MegaRAID SAS boot support.
//
// Copyright (C) 2012 Hannes Reinecke, SUSE Linux Products GmbH
//
// Authors:
//  Hannes Reinecke <hare@suse.de>
//
// based on virtio-scsi.c which is written by:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "blockcmd.h" // scsi_drive_setup
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pci.h" // pci_config_readl
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_DEVICE_ID_XXX
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // timer_calc

#నిర్వచించు MFI_DB 0x0 // Doorbell
#నిర్వచించు MFI_OMSG0 0x18 // Outbound message 0
#నిర్వచించు MFI_IDB 0x20 // Inbound doorbell
#నిర్వచించు MFI_ODB 0x2c // Outbound doorbell
#నిర్వచించు MFI_IQP 0x40 // Inbound queue port
#నిర్వచించు MFI_OSP0 0xb0 // Outbound scratch pad0
#నిర్వచించు MFI_IQPL 0xc0 // Inbound queue port (low bytes)
#నిర్వచించు MFI_IQPH 0xc4 // Inbound queue port (high bytes)

#నిర్వచించు MFI_STATE_ముసుగు                0xf0000000
#నిర్వచించు MFI_STATE_WAIT_HANDSHAKE      0x60000000
#నిర్వచించు MFI_STATE_BOOT_సందేశం_PENDING 0x90000000
#నిర్వచించు MFI_STATE_READY               0xb0000000
#నిర్వచించు MFI_STATE_OPERATIONAL         0xc0000000
#నిర్వచించు MFI_STATE_FAULT               0xf0000000

/* MFI Commands */
రకనిర్వచనం గణన {
    MFI_CMD_INIT = 0x00,
    MFI_CMD_LD_READ,
    MFI_CMD_LD_WRITE,
    MFI_CMD_LD_SCSI_IO,
    MFI_CMD_PD_SCSI_IO,
    MFI_CMD_DCMD,
    MFI_CMD_ABORT,
    MFI_CMD_SMP,
    MFI_CMD_STP
} mfi_cmd_t;

నిర్మాణం megasas_cmd_చట్రం {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_స్థితి;      /*02h */
    u8 scsi_స్థితి;     /*03h */

    u8 లక్ష్యం_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_లెక్క;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 దత్తాంశం_xfer_len;   /*14h */

    సంఘం {
        నిర్మాణం {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        నిర్మాణం {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        నిర్మాణం {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

నిర్మాణం mfi_ld_list_s {
    u32     లెక్క;
    u32     reserved_0;
    నిర్మాణం {
        u8          లక్ష్యం;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         కొలత;
    } lds[64];
} __attribute__ ((packed));

#నిర్వచించు MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

నిర్మాణం megasas_lun_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం megasas_cmd_చట్రం *చట్రం;
    u32 iobase;
    u16 pci_id;
    u8 లక్ష్యం;
    u8 lun;
};

స్థిర పూర్ణాంకం megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            నిర్మాణం megasas_cmd_చట్రం *చట్రం)
{
    u32 చట్రం_addr = (u32)చట్రం;
    పూర్ణాంకం చట్రం_లెక్క = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", చట్రం_addr);
    ఒకవేళ (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(చట్రం_addr | చట్రం_లెక్క << 1 | 1, ioaddr + MFI_IQPL);
    } లేకపోతే ఒకవేళ (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(చట్రం_addr >> 3 | చట్రం_లెక్క, ioaddr + MFI_IQP);
    } లేకపోతే {
        outl(చట్రం_addr | చట్రం_లెక్క << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 చివర = timer_calc(MEGASAS_POLL_TIMEOUT);
    చేయి {
        కొరకు (;;) {
            cmd_state = GET_LOWFLAT(చట్రం->cmd_స్థితి);
            ఒకవేళ (cmd_state != 0xff)
                ఆపు;
            ఒకవేళ (timer_check(చివర)) {
                warn_timeout();
                తిరిగి -1;
            }
            yield();
        }
    } అయితే (cmd_state == 0xff);

    ఒకవేళ (cmd_state == 0 || cmd_state == 0x2d)
        తిరిగి 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", చట్రం_addr, cmd_state);
    తిరిగి -1;
}

పూర్ణాంకం
megasas_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_MEGASAS)
        తిరిగి DISK_RET_EBADTRACK;
    u8 cdb[16];
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, cdb, పరిమాణం(cdb));
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);
    నిర్మాణం megasas_lun_s *mlun_gf =
        container_యొక్క(op->drive_fl, నిర్మాణం megasas_lun_s, drive);
    నిర్మాణం megasas_cmd_చట్రం *చట్రం = GET_GLOBALFLAT(mlun_gf->చట్రం);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    పూర్ణాంకం i;

    memset_fl(చట్రం, 0, పరిమాణం(*చట్రం));
    సమితి_LOWFLAT(చట్రం->cmd, MFI_CMD_LD_SCSI_IO);
    సమితి_LOWFLAT(చట్రం->cmd_స్థితి, 0xFF);
    సమితి_LOWFLAT(చట్రం->లక్ష్యం_id, GET_GLOBALFLAT(mlun_gf->లక్ష్యం));
    సమితి_LOWFLAT(చట్రం->lun, GET_GLOBALFLAT(mlun_gf->lun));
    సమితి_LOWFLAT(చట్రం->flags, 0x0001);
    సమితి_LOWFLAT(చట్రం->దత్తాంశం_xfer_len, op->లెక్క * blocksize);
    సమితి_LOWFLAT(చట్రం->cdb_len, 16);

    కొరకు (i = 0; i < 16; i++) {
        సమితి_LOWFLAT(చట్రం->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->లెక్క, blocksize);

    ఒకవేళ (op->లెక్క) {
        సమితి_LOWFLAT(చట్రం->pthru.sgl_addr, (u32)op->buf_fl);
        సమితి_LOWFLAT(చట్రం->pthru.sgl_len, op->లెక్క * blocksize);
        సమితి_LOWFLAT(చట్రం->sge_లెక్క, 1);
    }
    సమితి_LOWFLAT(చట్రం->context, (u32)చట్రం);

    ఒకవేళ (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), చట్రం) == 0)
        తిరిగి DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    తిరిగి DISK_RET_EBADTRACK;
}

స్థిర పూర్ణాంకం
megasas_కూడు_lun(నిర్మాణం pci_device *pci, u32 iobase, u8 లక్ష్యం, u8 lun)
{
    నిర్మాణం megasas_lun_s *mlun = malloc_fseg(పరిమాణం(*mlun));
    అక్షరం *name;
    పూర్ణాంకం prio, ret = 0;

    ఒకవేళ (!mlun) {
        warn_noalloc();
        తిరిగి -1;
    }
    memset(mlun, 0, పరిమాణం(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->లక్ష్యం = లక్ష్యం;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->చట్రం = memalign_దిగువ(256, పరిమాణం(నిర్మాణం megasas_cmd_చట్రం));
    ఒకవేళ (!mlun->చట్రం) {
        warn_noalloc();
        విడుదల(mlun);
        తిరిగి -1;
    }
    boot_lchs_కనుగొను_scsi_device(pci, లక్ష్యం, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, లక్ష్యం, lun);
    prio = bootprio_కనుగొను_scsi_device(pci, లక్ష్యం, lun);
    ret = scsi_drive_అమరిక(&mlun->drive, name, prio);
    విడుదల(name);
    ఒకవేళ (ret) {
        విడుదల(mlun->చట్రం);
        విడుదల(mlun);
        ret = -1;
    }

    తిరిగి ret;
}

స్థిర శూన్య megasas_పరిశీలన_లక్ష్యం(నిర్మాణం pci_device *pci, u32 iobase)
{
    నిర్మాణం mfi_ld_list_s ld_list;
    నిర్మాణం megasas_cmd_చట్రం *చట్రం = memalign_tmp(256, పరిమాణం(*చట్రం));
    ఒకవేళ (!చట్రం) {
        warn_noalloc();
        తిరిగి;
    }

    memset(&ld_list, 0, పరిమాణం(ld_list));
    memset_fl(చట్రం, 0, పరిమాణం(*చట్రం));

    చట్రం->cmd = MFI_CMD_DCMD;
    చట్రం->cmd_స్థితి = 0xFF;
    చట్రం->sge_లెక్క = 1;
    చట్రం->flags = 0x0011;
    చట్రం->దత్తాంశం_xfer_len = పరిమాణం(ld_list);
    చట్రం->dcmd.opcode = 0x03010000;
    చట్రం->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    చట్రం->dcmd.sgl_len = పరిమాణం(ld_list);
    చట్రం->context = (u32)చట్రం;

    ఒకవేళ (megasas_fire_cmd(pci->device, iobase, చట్రం) == 0) {
        dprintf(2, "%d LD found\n", ld_list.లెక్క);
        పూర్ణాంకం i;
        కొరకు (i = 0; i < ld_list.లెక్క; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].లక్ష్యం, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            ఒకవేళ (ld_list.lds[i].state != 0) {
                megasas_కూడు_lun(pci, iobase,
                                ld_list.lds[i].లక్ష్యం, ld_list.lds[i].lun);
            }
        }
    }
}

స్థిర పూర్ణాంకం megasas_transition_గమ్యం_ready(నిర్మాణం pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    ఒకవేళ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_ముసుగు;
    లేకపోతే
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_ముసుగు;

    అయితే (fw_state != new_state) {
        ఎంపిక (new_state) {
        స్థితి MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            తిరిగి -1;
            ఆపు;
        స్థితి MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        స్థితి MFI_STATE_BOOT_సందేశం_PENDING:
            mfi_flags |= 0x10;
            ఒకవేళ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } లేకపోతే {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ఆపు;
        స్థితి MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            ఒకవేళ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                ఒకవేళ (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    పూర్ణాంకం j = 0;
                    u32 doorbell;

                    అయితే (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        ఒకవేళ (!doorbell)
                            ఆపు;
                        msleep(20);
                        j++;
                    }
                }
            } లేకపోతే {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ఆపు;
        స్థితి MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            తిరిగి 0;
        }
        // The current state should not last longer than poll timeout
        u32 చివర = timer_calc(MEGASAS_POLL_TIMEOUT);
        కొరకు (;;) {
            ఒకవేళ (timer_check(చివర)) {
                ఆపు;
            }
            yield();
            fw_state = new_state;
            ఒకవేళ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_ముసుగు;
            లేకపోతే
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_ముసుగు;
            ఒకవేళ (new_state != fw_state) {
                ఆపు;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_ముసుగు);
    తిరిగి -1;
}

స్థిర శూన్య
init_megasas(శూన్య *దత్తాంశం)
{
    నిర్మాణం pci_device *pci = దత్తాంశం;
    u32 bar = PCI_పునాది_చిరునామా_2;
    ఒకవేళ (!(pci_config_readl(pci->bdf, bar) & PCI_పునాది_చిరునామా_IO_ముసుగు))
        bar = PCI_పునాది_చిరునామా_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    ఒకవేళ (!iobase)
        తిరిగి;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    ఒకవేళ (megasas_transition_గమ్యం_ready(pci, iobase) == 0)
        megasas_పరిశీలన_లక్ష్యం(pci, iobase);
}

శూన్య
megasas_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_MEGASAS)
        తిరిగి;

    dprintf(3, "init megasas\n");

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            కొనసాగు;
        ఒకవేళ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
            pci->device == PCI_DEVICE_ID_LSI_SAS1078 ||
            pci->device == PCI_DEVICE_ID_LSI_SAS1078DE ||
            pci->device == PCI_DEVICE_ID_LSI_SAS2108 ||
            pci->device == PCI_DEVICE_ID_LSI_SAS2108E ||
            pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
            pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
            pci->device == PCI_DEVICE_ID_LSI_VERDE_ZCR ||
            pci->device == PCI_DEVICE_ID_DELL_PERC5 ||
            pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
            pci->device == PCI_DEVICE_ID_LSI_SAS3108)
            నిరంతరం_thread(init_megasas, pci);
    }
}
