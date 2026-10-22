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
    u8 sge_गिनती;       /*07h */

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
    u32     गिनती;
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
    पूर्णाङ्क frame_गिनती = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    यदि (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_गिनती << 1 | 1, ioaddr + MFI_IQPL);
    } अन्यथा यदि (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_गिनती, ioaddr + MFI_IQP);
    } अन्यथा {
        outl(frame_addr | frame_गिनती << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    कुरु {
        प्रति (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            यदि (cmd_state != 0xff)
                भङ्ग;
            यदि (timer_check(end)) {
                warn_timeout();
                प्रत्यागम -1;
            }
            yield();
        }
    } यावत् (cmd_state == 0xff);

    यदि (cmd_state == 0 || cmd_state == 0x2d)
        प्रत्यागम 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    प्रत्यागम -1;
}

पूर्णाङ्क
megasas_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_MEGASAS)
        प्रत्यागम DISK_RET_EBADTRACK;
    u8 cdb[16];
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, cdb, परिमाण(cdb));
    यदि (blocksize < 0)
        प्रत्यागम default_process_op(op);
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
    SET_LOWFLAT(frame->data_xfer_len, op->गिनती * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    प्रति (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->गिनती, blocksize);

    यदि (op->गिनती) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->गिनती * blocksize);
        SET_LOWFLAT(frame->sge_गिनती, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    यदि (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        प्रत्यागम DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    प्रत्यागम DISK_RET_EBADTRACK;
}

स्थिर पूर्णाङ्क
megasas_add_lun(संरचना pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    संरचना megasas_lun_s *mlun = malloc_fseg(परिमाण(*mlun));
    वर्ण *name;
    पूर्णाङ्क prio, ret = 0;

    यदि (!mlun) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    memset(mlun, 0, परिमाण(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_निचला(256, परिमाण(संरचना megasas_cmd_frame));
    यदि (!mlun->frame) {
        warn_noalloc();
        मुक्त(mlun);
        प्रत्यागम -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    मुक्त(name);
    यदि (ret) {
        मुक्त(mlun->frame);
        मुक्त(mlun);
        ret = -1;
    }

    प्रत्यागम ret;
}

स्थिर शून्य megasas_scan_target(संरचना pci_device *pci, u32 iobase)
{
    संरचना mfi_ld_list_s ld_list;
    संरचना megasas_cmd_frame *frame = memalign_tmp(256, परिमाण(*frame));
    यदि (!frame) {
        warn_noalloc();
        प्रत्यागम;
    }

    memset(&ld_list, 0, परिमाण(ld_list));
    memset_fl(frame, 0, परिमाण(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_गिनती = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = परिमाण(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = परिमाण(ld_list);
    frame->context = (u32)frame;

    यदि (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.गिनती);
        पूर्णाङ्क i;
        प्रति (i = 0; i < ld_list.गिनती; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            यदि (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

स्थिर पूर्णाङ्क megasas_transition_to_ready(संरचना pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    यदि (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    अन्यथा
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    यावत् (fw_state != new_state) {
        विकल्प (new_state) {
        पक्ष MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            प्रत्यागम -1;
            भङ्ग;
        पक्ष MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        पक्ष MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            यदि (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } अन्यथा {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            भङ्ग;
        पक्ष MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            यदि (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                यदि (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    पूर्णाङ्क j = 0;
                    u32 doorbell;

                    यावत् (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        यदि (!doorbell)
                            भङ्ग;
                        msleep(20);
                        j++;
                    }
                }
            } अन्यथा {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            भङ्ग;
        पक्ष MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            प्रत्यागम 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        प्रति (;;) {
            यदि (timer_check(end)) {
                भङ्ग;
            }
            yield();
            fw_state = new_state;
            यदि (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            अन्यथा
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            यदि (new_state != fw_state) {
                भङ्ग;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    प्रत्यागम -1;
}

स्थिर शून्य
init_megasas(शून्य *data)
{
    संरचना pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    यदि (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    यदि (!iobase)
        प्रत्यागम;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    यदि (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

शून्य
megasas_setup(शून्य)
{
    ASSERT32FLAT();
    यदि (!CONFIG_MEGASAS)
        प्रत्यागम;

    dprintf(3, "init megasas\n");

    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            अनुवर्त;
        यदि (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
