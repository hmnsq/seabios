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

#অন্তর্ভাব "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "blockcmd.h" // scsi_drive_setup
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pci.h" // pci_config_readl
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_DEVICE_ID_XXX
#অন্তর্ভাব "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // timer_calc

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
প্রকারলেখ গণনা {
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

কাঠামো megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_গুনতি;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 তথ্য_xfer_len;   /*14h */

    সংঘ {
        কাঠামো {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        কাঠামো {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        কাঠামো {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

কাঠামো mfi_ld_list_s {
    u32     গুনতি;
    u32     reserved_0;
    কাঠামো {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         আকার_2;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

কাঠামো megasas_lun_s {
    কাঠামো drive_s drive;
    কাঠামো megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

স্থির পূর্ণসংখ্যা megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            কাঠামো megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    পূর্ণসংখ্যা frame_গুনতি = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    যদি (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_গুনতি << 1 | 1, ioaddr + MFI_IQPL);
    } অন্যথা যদি (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_গুনতি, ioaddr + MFI_IQP);
    } অন্যথা {
        outl(frame_addr | frame_গুনতি << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    করো {
        জন্য (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            যদি (cmd_state != 0xff)
                বিরতি;
            যদি (timer_check(end)) {
                warn_timeout();
                ফেরত -1;
            }
            yield();
        }
    } যখন (cmd_state == 0xff);

    যদি (cmd_state == 0 || cmd_state == 0x2d)
        ফেরত 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    ফেরত -1;
}

পূর্ণসংখ্যা
megasas_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_MEGASAS)
        ফেরত DISK_RET_EBADTRACK;
    u8 cdb[16];
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, cdb, আকার(cdb));
    যদি (blocksize < 0)
        ফেরত default_process_op(op);
    কাঠামো megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, কাঠামো megasas_lun_s, drive);
    কাঠামো megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    পূর্ণসংখ্যা i;

    memset_fl(frame, 0, আকার(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->তথ্য_xfer_len, op->গুনতি * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    জন্য (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->গুনতি, blocksize);

    যদি (op->গুনতি) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->গুনতি * blocksize);
        SET_LOWFLAT(frame->sge_গুনতি, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    যদি (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        ফেরত DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    ফেরত DISK_RET_EBADTRACK;
}

স্থির পূর্ণসংখ্যা
megasas_add_lun(কাঠামো pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    কাঠামো megasas_lun_s *mlun = malloc_fseg(আকার(*mlun));
    অক্ষর *name;
    পূর্ণসংখ্যা prio, ret = 0;

    যদি (!mlun) {
        warn_noalloc();
        ফেরত -1;
    }
    memset(mlun, 0, আকার(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_নিচের(256, আকার(কাঠামো megasas_cmd_frame));
    যদি (!mlun->frame) {
        warn_noalloc();
        মুক্ত(mlun);
        ফেরত -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    মুক্ত(name);
    যদি (ret) {
        মুক্ত(mlun->frame);
        মুক্ত(mlun);
        ret = -1;
    }

    ফেরত ret;
}

স্থির শূন্য megasas_scan_target(কাঠামো pci_device *pci, u32 iobase)
{
    কাঠামো mfi_ld_list_s ld_list;
    কাঠামো megasas_cmd_frame *frame = memalign_tmp(256, আকার(*frame));
    যদি (!frame) {
        warn_noalloc();
        ফেরত;
    }

    memset(&ld_list, 0, আকার(ld_list));
    memset_fl(frame, 0, আকার(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_গুনতি = 1;
    frame->flags = 0x0011;
    frame->তথ্য_xfer_len = আকার(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = আকার(ld_list);
    frame->context = (u32)frame;

    যদি (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.গুনতি);
        পূর্ণসংখ্যা i;
        জন্য (i = 0; i < ld_list.গুনতি; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            যদি (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

স্থির পূর্ণসংখ্যা megasas_transition_to_ready(কাঠামো pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    যদি (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    অন্যথা
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    যখন (fw_state != new_state) {
        নির্বাচন (new_state) {
        ক্ষেত্র MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            ফেরত -1;
            বিরতি;
        ক্ষেত্র MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        ক্ষেত্র MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            যদি (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } অন্যথা {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            বিরতি;
        ক্ষেত্র MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            যদি (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                যদি (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    পূর্ণসংখ্যা j = 0;
                    u32 doorbell;

                    যখন (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        যদি (!doorbell)
                            বিরতি;
                        msleep(20);
                        j++;
                    }
                }
            } অন্যথা {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            বিরতি;
        ক্ষেত্র MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            ফেরত 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        জন্য (;;) {
            যদি (timer_check(end)) {
                বিরতি;
            }
            yield();
            fw_state = new_state;
            যদি (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            অন্যথা
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            যদি (new_state != fw_state) {
                বিরতি;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    ফেরত -1;
}

স্থির শূন্য
init_megasas(শূন্য *তথ্য)
{
    কাঠামো pci_device *pci = তথ্য;
    u32 bar = PCI_BASE_ADDRESS_2;
    যদি (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    যদি (!iobase)
        ফেরত;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    যদি (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

শূন্য
megasas_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (!CONFIG_MEGASAS)
        ফেরত;

    dprintf(3, "init megasas\n");

    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            চালু;
        যদি (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
