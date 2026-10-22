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

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBALFLAT
#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // scsi_drive_setup
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pci.h" // pci_config_readl
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_DEVICE_ID_XXX
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // timer_calc

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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
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

ସଂରଚନା megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_ଗଣନା;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    ସଂଘ {
        ସଂରଚନା {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        ସଂରଚନା {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        ସଂରଚନା {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

ସଂରଚନା mfi_ld_list_s {
    u32     ଗଣନା_2;
    u32     reserved_0;
    ସଂରଚନା {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

ସଂରଚନା megasas_lun_s {
    ସଂରଚନା drive_s drive;
    ସଂରଚନା megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            ସଂରଚନା megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା frame_ଗଣନା = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    ଯଦି (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_ଗଣନା << 1 | 1, ioaddr + MFI_IQPL);
    } ଅନ୍ୟଥା ଯଦି (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_ଗଣନା, ioaddr + MFI_IQP);
    } ଅନ୍ୟଥା {
        outl(frame_addr | frame_ଗଣନା << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    କର {
        ପାଇଁ (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            ଯଦି (cmd_state != 0xff)
                ଭାଙ୍ଗ;
            ଯଦି (timer_check(end)) {
                warn_timeout();
                ଫେରନ୍ତୁ -1;
            }
            yield();
        }
    } ଯେତେବେଳେ (cmd_state == 0xff);

    ଯଦି (cmd_state == 0 || cmd_state == 0x2d)
        ଫେରନ୍ତୁ 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    ଫେରନ୍ତୁ -1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
megasas_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_MEGASAS)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    u8 cdb[16];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, cdb, ଆକାର(cdb));
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);
    ସଂରଚନା megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, ସଂରଚନା megasas_lun_s, drive);
    ସଂରଚନା megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    memset_fl(frame, 0, ଆକାର(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->ଗଣନା_2 * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    ପାଇଁ (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->ଗଣନା_2, blocksize);

    ଯଦି (op->ଗଣନା_2) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->ଗଣନା_2 * blocksize);
        SET_LOWFLAT(frame->sge_ଗଣନା, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    ଯଦି (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
megasas_add_lun(ସଂରଚନା pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    ସଂରଚନା megasas_lun_s *mlun = malloc_fseg(ଆକାର(*mlun));
    ଅକ୍ଷର *name;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio, ret = 0;

    ଯଦି (!mlun) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    memset(mlun, 0, ଆକାର(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_ତଳ(256, ଆକାର(ସଂରଚନା megasas_cmd_frame));
    ଯଦି (!mlun->frame) {
        warn_noalloc();
        ମୁକ୍ତ(mlun);
        ଫେରନ୍ତୁ -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    ମୁକ୍ତ(name);
    ଯଦି (ret) {
        ମୁକ୍ତ(mlun->frame);
        ମୁକ୍ତ(mlun);
        ret = -1;
    }

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଶୂନ୍ୟ megasas_scan_target(ସଂରଚନା pci_device *pci, u32 iobase)
{
    ସଂରଚନା mfi_ld_list_s ld_list;
    ସଂରଚନା megasas_cmd_frame *frame = memalign_tmp(256, ଆକାର(*frame));
    ଯଦି (!frame) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }

    memset(&ld_list, 0, ଆକାର(ld_list));
    memset_fl(frame, 0, ଆକାର(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_ଗଣନା = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = ଆକାର(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = ଆକାର(ld_list);
    frame->context = (u32)frame;

    ଯଦି (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.ଗଣନା_2);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i = 0; i < ld_list.ଗଣନା_2; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            ଯଦି (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା megasas_transition_to_ready(ସଂରଚନା pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    ଯଦି (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    ଅନ୍ୟଥା
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    ଯେତେବେଳେ (fw_state != new_state) {
        ବାଛ (new_state) {
        ସ୍ଥିତି MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            ଫେରନ୍ତୁ -1;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        ସ୍ଥିତି MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            ଯଦି (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } ଅନ୍ୟଥା {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ଭାଙ୍ଗ;
        ସ୍ଥିତି MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            ଯଦି (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                ଯଦି (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    ପୂର୍ଣ୍ଣସଂଖ୍ୟା j = 0;
                    u32 doorbell;

                    ଯେତେବେଳେ (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        ଯଦି (!doorbell)
                            ଭାଙ୍ଗ;
                        msleep(20);
                        j++;
                    }
                }
            } ଅନ୍ୟଥା {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ଭାଙ୍ଗ;
        ସ୍ଥିତି MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            ଫେରନ୍ତୁ 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        ପାଇଁ (;;) {
            ଯଦି (timer_check(end)) {
                ଭାଙ୍ଗ;
            }
            yield();
            fw_state = new_state;
            ଯଦି (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            ଅନ୍ୟଥା
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            ଯଦି (new_state != fw_state) {
                ଭାଙ୍ଗ;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ଶୂନ୍ୟ
init_megasas(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    ଯଦି (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    ଯଦି (!iobase)
        ଫେରନ୍ତୁ;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    ଯଦି (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

ଶୂନ୍ୟ
megasas_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (!CONFIG_MEGASAS)
        ଫେରନ୍ତୁ;

    dprintf(3, "init megasas\n");

    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            ଜାରି;
        ଯଦି (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
