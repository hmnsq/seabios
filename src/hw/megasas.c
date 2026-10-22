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

#ضمن "biosvar.h" // GET_GLOBALFLAT
#ضمن "block.h" // struct drive_s
#ضمن "blockcmd.h" // scsi_drive_setup
#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "pci.h" // pci_config_readl
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_DEVICE_ID_XXX
#ضمن "pci_regs.h" // PCI_VENDOR_ID
#ضمن "stacks.h" // yield
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // timer_calc

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
تعريفنوع تعداد {
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

بنية megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_تعداد;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    اتحاد {
        بنية {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        بنية {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        بنية {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

بنية mfi_ld_list_s {
    u32     تعداد_2;
    u32     reserved_0;
    بنية {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

بنية megasas_lun_s {
    بنية drive_s drive;
    بنية megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

ساكن صحيح megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            بنية megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    صحيح frame_تعداد = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    إذا (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_تعداد << 1 | 1, ioaddr + MFI_IQPL);
    } وإلا إذا (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_تعداد, ioaddr + MFI_IQP);
    } وإلا {
        outl(frame_addr | frame_تعداد << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    افعل {
        لكل (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            إذا (cmd_state != 0xff)
                اكسر;
            إذا (timer_check(end)) {
                warn_timeout();
                إرجاع -1;
            }
            yield();
        }
    } بينما (cmd_state == 0xff);

    إذا (cmd_state == 0 || cmd_state == 0x2d)
        إرجاع 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    إرجاع -1;
}

صحيح
megasas_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_MEGASAS)
        إرجاع DISK_RET_EBADTRACK;
    u8 cdb[16];
    صحيح blocksize = scsi_fill_cmd(op, cdb, حجم(cdb));
    إذا (blocksize < 0)
        إرجاع default_process_op(op);
    بنية megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, بنية megasas_lun_s, drive);
    بنية megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    صحيح i;

    memset_fl(frame, 0, حجم(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->تعداد_2 * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    لكل (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->تعداد_2, blocksize);

    إذا (op->تعداد_2) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->تعداد_2 * blocksize);
        SET_LOWFLAT(frame->sge_تعداد, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    إذا (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        إرجاع DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    إرجاع DISK_RET_EBADTRACK;
}

ساكن صحيح
megasas_add_lun(بنية pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    بنية megasas_lun_s *mlun = malloc_fseg(حجم(*mlun));
    حرف *name;
    صحيح prio, ret = 0;

    إذا (!mlun) {
        warn_noalloc();
        إرجاع -1;
    }
    memset(mlun, 0, حجم(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_أدنى(256, حجم(بنية megasas_cmd_frame));
    إذا (!mlun->frame) {
        warn_noalloc();
        حرر(mlun);
        إرجاع -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    حرر(name);
    إذا (ret) {
        حرر(mlun->frame);
        حرر(mlun);
        ret = -1;
    }

    إرجاع ret;
}

ساكن فراغ megasas_scan_target(بنية pci_device *pci, u32 iobase)
{
    بنية mfi_ld_list_s ld_list;
    بنية megasas_cmd_frame *frame = memalign_tmp(256, حجم(*frame));
    إذا (!frame) {
        warn_noalloc();
        إرجاع;
    }

    memset(&ld_list, 0, حجم(ld_list));
    memset_fl(frame, 0, حجم(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_تعداد = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = حجم(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = حجم(ld_list);
    frame->context = (u32)frame;

    إذا (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.تعداد_2);
        صحيح i;
        لكل (i = 0; i < ld_list.تعداد_2; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            إذا (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

ساكن صحيح megasas_transition_to_ready(بنية pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    إذا (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    وإلا
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    بينما (fw_state != new_state) {
        بدل (new_state) {
        حالة MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            إرجاع -1;
            اكسر;
        حالة MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        حالة MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            إذا (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } وإلا {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            اكسر;
        حالة MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            إذا (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                إذا (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    صحيح j = 0;
                    u32 doorbell;

                    بينما (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        إذا (!doorbell)
                            اكسر;
                        msleep(20);
                        j++;
                    }
                }
            } وإلا {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            اكسر;
        حالة MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            إرجاع 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        لكل (;;) {
            إذا (timer_check(end)) {
                اكسر;
            }
            yield();
            fw_state = new_state;
            إذا (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            وإلا
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            إذا (new_state != fw_state) {
                اكسر;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    إرجاع -1;
}

ساكن فراغ
init_megasas(فراغ *data)
{
    بنية pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    إذا (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    إذا (!iobase)
        إرجاع;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    إذا (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

فراغ
megasas_setup(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_MEGASAS)
        إرجاع;

    dprintf(3, "init megasas\n");

    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            استمر;
        إذا (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
