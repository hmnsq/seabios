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

#அந்தர்பாவ "biosvar.h" // GET_GLOBALFLAT
#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "blockcmd.h" // scsi_drive_setup
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pci.h" // pci_config_readl
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_DEVICE_ID_XXX
#அந்தர்பாவ "pci_regs.h" // PCI_VENDOR_ID
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // timer_calc

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
வகைவரை எண்ணிக்கை {
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

கட்டமைப்பு megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_மொத்தம்;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    ஒன்றிப்பு {
        கட்டமைப்பு {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        கட்டமைப்பு {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        கட்டமைப்பு {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

கட்டமைப்பு mfi_ld_list_s {
    u32     மொத்தம்;
    u32     reserved_0;
    கட்டமைப்பு {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

கட்டமைப்பு megasas_lun_s {
    கட்டமைப்பு drive_s drive;
    கட்டமைப்பு megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

நிலையான முழு megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            கட்டமைப்பு megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    முழு frame_மொத்தம் = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    எனில் (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_மொத்தம் << 1 | 1, ioaddr + MFI_IQPL);
    } இல்லையெனில் எனில் (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_மொத்தம், ioaddr + MFI_IQP);
    } இல்லையெனில் {
        outl(frame_addr | frame_மொத்தம் << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    செய் {
        க்கு (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            எனில் (cmd_state != 0xff)
                நிறுத்து;
            எனில் (timer_check(end)) {
                warn_timeout();
                திரும்பு -1;
            }
            yield();
        }
    } போது (cmd_state == 0xff);

    எனில் (cmd_state == 0 || cmd_state == 0x2d)
        திரும்பு 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    திரும்பு -1;
}

முழு
megasas_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_MEGASAS)
        திரும்பு DISK_RET_EBADTRACK;
    u8 cdb[16];
    முழு blocksize = scsi_fill_cmd(op, cdb, அளவு(cdb));
    எனில் (blocksize < 0)
        திரும்பு default_process_op(op);
    கட்டமைப்பு megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, கட்டமைப்பு megasas_lun_s, drive);
    கட்டமைப்பு megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    முழு i;

    memset_fl(frame, 0, அளவு(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->மொத்தம் * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    க்கு (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->மொத்தம், blocksize);

    எனில் (op->மொத்தம்) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->மொத்தம் * blocksize);
        SET_LOWFLAT(frame->sge_மொத்தம், 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    எனில் (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        திரும்பு DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    திரும்பு DISK_RET_EBADTRACK;
}

நிலையான முழு
megasas_add_lun(கட்டமைப்பு pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    கட்டமைப்பு megasas_lun_s *mlun = malloc_fseg(அளவு(*mlun));
    எழுத்து *name;
    முழு prio, ret = 0;

    எனில் (!mlun) {
        warn_noalloc();
        திரும்பு -1;
    }
    memset(mlun, 0, அளவு(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_கீழ்(256, அளவு(கட்டமைப்பு megasas_cmd_frame));
    எனில் (!mlun->frame) {
        warn_noalloc();
        விடுவி(mlun);
        திரும்பு -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    விடுவி(name);
    எனில் (ret) {
        விடுவி(mlun->frame);
        விடுவி(mlun);
        ret = -1;
    }

    திரும்பு ret;
}

நிலையான வெறுமை megasas_scan_target(கட்டமைப்பு pci_device *pci, u32 iobase)
{
    கட்டமைப்பு mfi_ld_list_s ld_list;
    கட்டமைப்பு megasas_cmd_frame *frame = memalign_tmp(256, அளவு(*frame));
    எனில் (!frame) {
        warn_noalloc();
        திரும்பு;
    }

    memset(&ld_list, 0, அளவு(ld_list));
    memset_fl(frame, 0, அளவு(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_மொத்தம் = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = அளவு(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = அளவு(ld_list);
    frame->context = (u32)frame;

    எனில் (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.மொத்தம்);
        முழு i;
        க்கு (i = 0; i < ld_list.மொத்தம்; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            எனில் (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

நிலையான முழு megasas_transition_to_ready(கட்டமைப்பு pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    எனில் (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    இல்லையெனில்
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    போது (fw_state != new_state) {
        தேர்வு (new_state) {
        நிலை MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            திரும்பு -1;
            நிறுத்து;
        நிலை MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        நிலை MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            எனில் (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } இல்லையெனில் {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            நிறுத்து;
        நிலை MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            எனில் (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                எனில் (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    முழு j = 0;
                    u32 doorbell;

                    போது (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        எனில் (!doorbell)
                            நிறுத்து;
                        msleep(20);
                        j++;
                    }
                }
            } இல்லையெனில் {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            நிறுத்து;
        நிலை MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            திரும்பு 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        க்கு (;;) {
            எனில் (timer_check(end)) {
                நிறுத்து;
            }
            yield();
            fw_state = new_state;
            எனில் (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            இல்லையெனில்
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            எனில் (new_state != fw_state) {
                நிறுத்து;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    திரும்பு -1;
}

நிலையான வெறுமை
init_megasas(வெறுமை *data)
{
    கட்டமைப்பு pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    எனில் (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    எனில் (!iobase)
        திரும்பு;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    எனில் (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

வெறுமை
megasas_setup(வெறுமை)
{
    ASSERT32FLAT();
    எனில் (!CONFIG_MEGASAS)
        திரும்பு;

    dprintf(3, "init megasas\n");

    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            தொடர்;
        எனில் (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
