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

#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // scsi_drive_setup
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pci.h" // pci_config_readl
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_DEVICE_ID_XXX
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "stacks.h" // yield
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // timer_calc

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
تعریفنوع شمارش {
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

ساختار megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_شمار;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    اجتماع {
        ساختار {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        ساختار {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        ساختار {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

ساختار mfi_ld_list_s {
    u32     شمار;
    u32     reserved_0;
    ساختار {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

ساختار megasas_lun_s {
    ساختار drive_s drive;
    ساختار megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

ایستا صحیح megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            ساختار megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    صحیح frame_شمار = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    اگر (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_شمار << 1 | 1, ioaddr + MFI_IQPL);
    } وگرنه اگر (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_شمار, ioaddr + MFI_IQP);
    } وگرنه {
        outl(frame_addr | frame_شمار << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    انجامبده {
        برای (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            اگر (cmd_state != 0xff)
                بشکن;
            اگر (timer_check(end)) {
                warn_timeout();
                بازگشت -1;
            }
            yield();
        }
    } وقتی (cmd_state == 0xff);

    اگر (cmd_state == 0 || cmd_state == 0x2d)
        بازگشت 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    بازگشت -1;
}

صحیح
megasas_process_op(ساختار disk_op_s *op)
{
    اگر (!CONFIG_MEGASAS)
        بازگشت DISK_RET_EBADTRACK;
    u8 cdb[16];
    صحیح blocksize = scsi_fill_cmd(op, cdb, اندازه(cdb));
    اگر (blocksize < 0)
        بازگشت default_process_op(op);
    ساختار megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, ساختار megasas_lun_s, drive);
    ساختار megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    صحیح i;

    memset_fl(frame, 0, اندازه(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->شمار * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    برای (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->شمار, blocksize);

    اگر (op->شمار) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->شمار * blocksize);
        SET_LOWFLAT(frame->sge_شمار, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    اگر (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        بازگشت DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    بازگشت DISK_RET_EBADTRACK;
}

ایستا صحیح
megasas_add_lun(ساختار pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    ساختار megasas_lun_s *mlun = malloc_fseg(اندازه(*mlun));
    نویسه *name;
    صحیح prio, ret = 0;

    اگر (!mlun) {
        warn_noalloc();
        بازگشت -1;
    }
    memset(mlun, 0, اندازه(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_کم(256, اندازه(ساختار megasas_cmd_frame));
    اگر (!mlun->frame) {
        warn_noalloc();
        آزاد(mlun);
        بازگشت -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    آزاد(name);
    اگر (ret) {
        آزاد(mlun->frame);
        آزاد(mlun);
        ret = -1;
    }

    بازگشت ret;
}

ایستا تهی megasas_scan_target(ساختار pci_device *pci, u32 iobase)
{
    ساختار mfi_ld_list_s ld_list;
    ساختار megasas_cmd_frame *frame = memalign_tmp(256, اندازه(*frame));
    اگر (!frame) {
        warn_noalloc();
        بازگشت;
    }

    memset(&ld_list, 0, اندازه(ld_list));
    memset_fl(frame, 0, اندازه(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_شمار = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = اندازه(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = اندازه(ld_list);
    frame->context = (u32)frame;

    اگر (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.شمار);
        صحیح i;
        برای (i = 0; i < ld_list.شمار; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            اگر (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

ایستا صحیح megasas_transition_to_ready(ساختار pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    اگر (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    وگرنه
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    وقتی (fw_state != new_state) {
        انتخاب (new_state) {
        حالت MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            بازگشت -1;
            بشکن;
        حالت MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        حالت MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            اگر (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } وگرنه {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            بشکن;
        حالت MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            اگر (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                اگر (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    صحیح j = 0;
                    u32 doorbell;

                    وقتی (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        اگر (!doorbell)
                            بشکن;
                        msleep(20);
                        j++;
                    }
                }
            } وگرنه {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            بشکن;
        حالت MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            بازگشت 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        برای (;;) {
            اگر (timer_check(end)) {
                بشکن;
            }
            yield();
            fw_state = new_state;
            اگر (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            وگرنه
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            اگر (new_state != fw_state) {
                بشکن;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    بازگشت -1;
}

ایستا تهی
init_megasas(تهی *data)
{
    ساختار pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    اگر (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    اگر (!iobase)
        بازگشت;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    اگر (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

تهی
megasas_setup(تهی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_MEGASAS)
        بازگشت;

    dprintf(3, "init megasas\n");

    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            ادامه;
        اگر (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
