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

#અન્તર્ભાવ "biosvar.h" // GET_GLOBALFLAT
#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "blockcmd.h" // scsi_drive_setup
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pci.h" // pci_config_readl
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_DEVICE_ID_XXX
#અન્તર્ભાવ "pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // timer_calc

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
પ્રકારવ્યાખ્યા ગણના {
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

સંરચના megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_ગણતરી;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    સંઘ {
        સંરચના {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        સંરચના {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        સંરચના {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

સંરચના mfi_ld_list_s {
    u32     ગણતરી;
    u32     reserved_0;
    સંરચના {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

સંરચના megasas_lun_s {
    સંરચના drive_s drive;
    સંરચના megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

સ્થિર પૂર્ણાંક megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            સંરચના megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    પૂર્ણાંક frame_ગણતરી = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    જો (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_ગણતરી << 1 | 1, ioaddr + MFI_IQPL);
    } નહીંતર જો (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_ગણતરી, ioaddr + MFI_IQP);
    } નહીંતર {
        outl(frame_addr | frame_ગણતરી << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    કરો {
        માટે (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            જો (cmd_state != 0xff)
                તોડો;
            જો (timer_check(end)) {
                warn_timeout();
                પાછા -1;
            }
            yield();
        }
    } જ્યારે (cmd_state == 0xff);

    જો (cmd_state == 0 || cmd_state == 0x2d)
        પાછા 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    પાછા -1;
}

પૂર્ણાંક
megasas_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_MEGASAS)
        પાછા DISK_RET_EBADTRACK;
    u8 cdb[16];
    પૂર્ણાંક blocksize = scsi_fill_cmd(op, cdb, કદ(cdb));
    જો (blocksize < 0)
        પાછા default_process_op(op);
    સંરચના megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, સંરચના megasas_lun_s, drive);
    સંરચના megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    પૂર્ણાંક i;

    memset_fl(frame, 0, કદ(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->ગણતરી * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    માટે (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->ગણતરી, blocksize);

    જો (op->ગણતરી) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->ગણતરી * blocksize);
        SET_LOWFLAT(frame->sge_ગણતરી, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    જો (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        પાછા DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    પાછા DISK_RET_EBADTRACK;
}

સ્થિર પૂર્ણાંક
megasas_add_lun(સંરચના pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    સંરચના megasas_lun_s *mlun = malloc_fseg(કદ(*mlun));
    અક્ષર *name;
    પૂર્ણાંક prio, ret = 0;

    જો (!mlun) {
        warn_noalloc();
        પાછા -1;
    }
    memset(mlun, 0, કદ(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_નીચલું(256, કદ(સંરચના megasas_cmd_frame));
    જો (!mlun->frame) {
        warn_noalloc();
        મુક્ત(mlun);
        પાછા -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    મુક્ત(name);
    જો (ret) {
        મુક્ત(mlun->frame);
        મુક્ત(mlun);
        ret = -1;
    }

    પાછા ret;
}

સ્થિર શૂન્ય megasas_scan_target(સંરચના pci_device *pci, u32 iobase)
{
    સંરચના mfi_ld_list_s ld_list;
    સંરચના megasas_cmd_frame *frame = memalign_tmp(256, કદ(*frame));
    જો (!frame) {
        warn_noalloc();
        પાછા;
    }

    memset(&ld_list, 0, કદ(ld_list));
    memset_fl(frame, 0, કદ(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_ગણતરી = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = કદ(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = કદ(ld_list);
    frame->context = (u32)frame;

    જો (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.ગણતરી);
        પૂર્ણાંક i;
        માટે (i = 0; i < ld_list.ગણતરી; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            જો (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

સ્થિર પૂર્ણાંક megasas_transition_to_ready(સંરચના pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    જો (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    નહીંતર
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    જ્યારે (fw_state != new_state) {
        પસંદગી (new_state) {
        સ્થિતિ MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            પાછા -1;
            તોડો;
        સ્થિતિ MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        સ્થિતિ MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            જો (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } નહીંતર {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            તોડો;
        સ્થિતિ MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            જો (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                જો (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    પૂર્ણાંક j = 0;
                    u32 doorbell;

                    જ્યારે (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        જો (!doorbell)
                            તોડો;
                        msleep(20);
                        j++;
                    }
                }
            } નહીંતર {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            તોડો;
        સ્થિતિ MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            પાછા 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        માટે (;;) {
            જો (timer_check(end)) {
                તોડો;
            }
            yield();
            fw_state = new_state;
            જો (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            નહીંતર
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            જો (new_state != fw_state) {
                તોડો;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    પાછા -1;
}

સ્થિર શૂન્ય
init_megasas(શૂન્ય *data)
{
    સંરચના pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    જો (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    જો (!iobase)
        પાછા;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    જો (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

શૂન્ય
megasas_setup(શૂન્ય)
{
    ASSERT32FLAT();
    જો (!CONFIG_MEGASAS)
        પાછા;

    dprintf(3, "init megasas\n");

    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            ચાલુ;
        જો (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
