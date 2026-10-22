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

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // scsi_drive_setup
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h" // pci_config_readl
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_DEVICE_ID_XXX
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc

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
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
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

ಸಂರಚನಾ megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_ಎಣಿಕೆ;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    ಸಙ್ಘ {
        ಸಂರಚನಾ {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        ಸಂರಚನಾ {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        ಸಂರಚನಾ {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

ಸಂರಚನಾ mfi_ld_list_s {
    u32     ಎಣಿಕೆ;
    u32     reserved_0;
    ಸಂರಚನಾ {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

ಸಂರಚನಾ megasas_lun_s {
    ಸಂರಚನಾ drive_s drive;
    ಸಂರಚನಾ megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            ಸಂರಚನಾ megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    ಪೂರ್ಣಾಙ್ಕ frame_ಎಣಿಕೆ = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    ಯದಿ (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_ಎಣಿಕೆ << 1 | 1, ioaddr + MFI_IQPL);
    } ಅನ್ಯಥಾ ಯದಿ (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_ಎಣಿಕೆ, ioaddr + MFI_IQP);
    } ಅನ್ಯಥಾ {
        outl(frame_addr | frame_ಎಣಿಕೆ << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    ಕುರು {
        ಪ್ರತಿ (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            ಯದಿ (cmd_state != 0xff)
                ಭಙ್ಗ;
            ಯದಿ (timer_check(end)) {
                warn_timeout();
                ಪ್ರತ್ಯಾಗಮ -1;
            }
            yield();
        }
    } ಯಾವತ್ (cmd_state == 0xff);

    ಯದಿ (cmd_state == 0 || cmd_state == 0x2d)
        ಪ್ರತ್ಯಾಗಮ 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಪೂರ್ಣಾಙ್ಕ
megasas_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_MEGASAS)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    u8 cdb[16];
    ಪೂರ್ಣಾಙ್ಕ blocksize = scsi_fill_cmd(op, cdb, ಪರಿಮಾಣ(cdb));
    ಯದಿ (blocksize < 0)
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    ಸಂರಚನಾ megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, ಸಂರಚನಾ megasas_lun_s, drive);
    ಸಂರಚನಾ megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    ಪೂರ್ಣಾಙ್ಕ i;

    memset_fl(frame, 0, ಪರಿಮಾಣ(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->ಎಣಿಕೆ * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    ಪ್ರತಿ (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->ಎಣಿಕೆ, blocksize);

    ಯದಿ (op->ಎಣಿಕೆ) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->ಎಣಿಕೆ * blocksize);
        SET_LOWFLAT(frame->sge_ಎಣಿಕೆ, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    ಯದಿ (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
megasas_add_lun(ಸಂರಚನಾ pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    ಸಂರಚನಾ megasas_lun_s *mlun = malloc_fseg(ಪರಿಮಾಣ(*mlun));
    ವರ್ಣ *name;
    ಪೂರ್ಣಾಙ್ಕ prio, ret = 0;

    ಯದಿ (!mlun) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    memset(mlun, 0, ಪರಿಮಾಣ(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_ಕೆಳಗಿನ(256, ಪರಿಮಾಣ(ಸಂರಚನಾ megasas_cmd_frame));
    ಯದಿ (!mlun->frame) {
        warn_noalloc();
        ಮುಕ್ತ(mlun);
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    ಮುಕ್ತ(name);
    ಯದಿ (ret) {
        ಮುಕ್ತ(mlun->frame);
        ಮುಕ್ತ(mlun);
        ret = -1;
    }

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ megasas_scan_target(ಸಂರಚನಾ pci_device *pci, u32 iobase)
{
    ಸಂರಚನಾ mfi_ld_list_s ld_list;
    ಸಂರಚನಾ megasas_cmd_frame *frame = memalign_tmp(256, ಪರಿಮಾಣ(*frame));
    ಯದಿ (!frame) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }

    memset(&ld_list, 0, ಪರಿಮಾಣ(ld_list));
    memset_fl(frame, 0, ಪರಿಮಾಣ(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_ಎಣಿಕೆ = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = ಪರಿಮಾಣ(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = ಪರಿಮಾಣ(ld_list);
    frame->context = (u32)frame;

    ಯದಿ (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.ಎಣಿಕೆ);
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i = 0; i < ld_list.ಎಣಿಕೆ; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            ಯದಿ (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ megasas_transition_to_ready(ಸಂರಚನಾ pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    ಯದಿ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    ಅನ್ಯಥಾ
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    ಯಾವತ್ (fw_state != new_state) {
        ವಿಕಲ್ಪ (new_state) {
        ಪಕ್ಷ MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            ಪ್ರತ್ಯಾಗಮ -1;
            ಭಙ್ಗ;
        ಪಕ್ಷ MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        ಪಕ್ಷ MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            ಯದಿ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } ಅನ್ಯಥಾ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ಭಙ್ಗ;
        ಪಕ್ಷ MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            ಯದಿ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                ಯದಿ (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    ಪೂರ್ಣಾಙ್ಕ j = 0;
                    u32 doorbell;

                    ಯಾವತ್ (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        ಯದಿ (!doorbell)
                            ಭಙ್ಗ;
                        msleep(20);
                        j++;
                    }
                }
            } ಅನ್ಯಥಾ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ಭಙ್ಗ;
        ಪಕ್ಷ MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            ಪ್ರತ್ಯಾಗಮ 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        ಪ್ರತಿ (;;) {
            ಯದಿ (timer_check(end)) {
                ಭಙ್ಗ;
            }
            yield();
            fw_state = new_state;
            ಯದಿ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            ಅನ್ಯಥಾ
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            ಯದಿ (new_state != fw_state) {
                ಭಙ್ಗ;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_megasas(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    ಯದಿ (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    ಯದಿ (!iobase)
        ಪ್ರತ್ಯಾಗಮ;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    ಯದಿ (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

ಶೂನ್ಯ
megasas_setup(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯದಿ (!CONFIG_MEGASAS)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init megasas\n");

    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            ಅನುವರ್ತ;
        ಯದಿ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
