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

#अन्तर्भाव "biosvar.h" // GET_GLOBALFLAT
#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "blockcmd.h" // scsi_drive_setup
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pci.h" // pci_config_readl
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_DEVICE_ID_XXX
#अन्तर्भाव "pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // timer_calc

#define MFI_DB 0x0 // Doorbell
#define MFI_OMSG0 0x18 // Outbound message 0
#define MFI_IDB 0x20 // Inbound doorbell
#define MFI_ODB 0x2c // Outbound doorbell
#define MFI_IQP 0x40 // Inbound queue port
#define MFI_OSP0 0xb0 // Outbound scratch pad0
#define MFI_IQPL 0xc0 // Inbound queue port (low bytes)
#define MFI_IQPH 0xc4 // Inbound queue port (high bytes)

#define MFI_STATE_MASK                0xf0000000
#define MFI_STATE_WAIT_HANDSHAKE      0x60000000
#define MFI_STATE_BOOT_MESSAGE_PENDING 0x90000000
#define MFI_STATE_READY               0xb0000000
#define MFI_STATE_OPERATIONAL         0xc0000000
#define MFI_STATE_FAULT               0xf0000000

/* MFI Commands */
प्रकारनिर्देश गणना {
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

संरचना megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_गणनम्;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    सङ्घ {
        संरचना {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        संरचना {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        संरचना {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

संरचना mfi_ld_list_s {
    u32     गणनम्;
    u32     reserved_0;
    संरचना {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

संरचना megasas_lun_s {
    संरचना drive_s drive;
    संरचना megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

स्थिर पूर्णाङ्क megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            संरचना megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    पूर्णाङ्क frame_गणनम् = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    सचे (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_गणनम् << 1 | 1, ioaddr + MFI_IQPL);
    } अञ्ञथा सचे (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_गणनम्, ioaddr + MFI_IQP);
    } अञ्ञथा {
        outl(frame_addr | frame_गणनम् << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    करोहि {
        अत्थाय (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            सचे (cmd_state != 0xff)
                भङ्ग;
            सचे (timer_check(end)) {
                warn_timeout();
                पटिनिवत्त -1;
            }
            yield();
        }
    } याव (cmd_state == 0xff);

    सचे (cmd_state == 0 || cmd_state == 0x2d)
        पटिनिवत्त 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    पटिनिवत्त -1;
}

पूर्णाङ्क
megasas_process_op(संरचना disk_op_s *op)
{
    सचे (!CONFIG_MEGASAS)
        पटिनिवत्त DISK_RET_EBADTRACK;
    u8 cdb[16];
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, cdb, परिमाण(cdb));
    सचे (blocksize < 0)
        पटिनिवत्त default_process_op(op);
    संरचना megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, संरचना megasas_lun_s, drive);
    संरचना megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    पूर्णाङ्क i;

    memset_fl(frame, 0, परिमाण(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->गणनम् * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    अत्थाय (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->गणनम्, blocksize);

    सचे (op->गणनम्) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->गणनम् * blocksize);
        SET_LOWFLAT(frame->sge_गणनम्, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    सचे (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        पटिनिवत्त DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    पटिनिवत्त DISK_RET_EBADTRACK;
}

स्थिर पूर्णाङ्क
megasas_add_lun(संरचना pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    संरचना megasas_lun_s *mlun = malloc_fseg(परिमाण(*mlun));
    वर्ण *name;
    पूर्णाङ्क prio, ret = 0;

    सचे (!mlun) {
        warn_noalloc();
        पटिनिवत्त -1;
    }
    memset(mlun, 0, परिमाण(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_नीचः(256, परिमाण(संरचना megasas_cmd_frame));
    सचे (!mlun->frame) {
        warn_noalloc();
        मुक्त(mlun);
        पटिनिवत्त -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    मुक्त(name);
    सचे (ret) {
        मुक्त(mlun->frame);
        मुक्त(mlun);
        ret = -1;
    }

    पटिनिवत्त ret;
}

स्थिर शून्य megasas_scan_target(संरचना pci_device *pci, u32 iobase)
{
    संरचना mfi_ld_list_s ld_list;
    संरचना megasas_cmd_frame *frame = memalign_tmp(256, परिमाण(*frame));
    सचे (!frame) {
        warn_noalloc();
        पटिनिवत्त;
    }

    memset(&ld_list, 0, परिमाण(ld_list));
    memset_fl(frame, 0, परिमाण(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_गणनम् = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = परिमाण(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = परिमाण(ld_list);
    frame->context = (u32)frame;

    सचे (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.गणनम्);
        पूर्णाङ्क i;
        अत्थाय (i = 0; i < ld_list.गणनम्; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            सचे (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

स्थिर पूर्णाङ्क megasas_transition_to_ready(संरचना pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    सचे (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    अञ्ञथा
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    याव (fw_state != new_state) {
        विकल्प (new_state) {
        पक्ष MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            पटिनिवत्त -1;
            भङ्ग;
        पक्ष MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        पक्ष MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            सचे (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } अञ्ञथा {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            भङ्ग;
        पक्ष MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            सचे (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                सचे (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    पूर्णाङ्क j = 0;
                    u32 doorbell;

                    याव (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        सचे (!doorbell)
                            भङ्ग;
                        msleep(20);
                        j++;
                    }
                }
            } अञ्ञथा {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            भङ्ग;
        पक्ष MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            पटिनिवत्त 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        अत्थाय (;;) {
            सचे (timer_check(end)) {
                भङ्ग;
            }
            yield();
            fw_state = new_state;
            सचे (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            अञ्ञथा
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            सचे (new_state != fw_state) {
                भङ्ग;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    पटिनिवत्त -1;
}

स्थिर शून्य
init_megasas(शून्य *data)
{
    संरचना pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    सचे (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    सचे (!iobase)
        पटिनिवत्त;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    सचे (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

शून्य
megasas_setup(शून्य)
{
    ASSERT32FLAT();
    सचे (!CONFIG_MEGASAS)
        पटिनिवत्त;

    dprintf(3, "init megasas\n");

    संरचना pci_device *pci;
    foreachpci(pci) {
        सचे (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            अनुवर्त;
        सचे (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
            run_thread(init_megasas, pci);
    }
}
