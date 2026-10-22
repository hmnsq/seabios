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

#समावेश "biosvar.h" // GET_GLOBALFLAT
#समावेश "block.h" // struct drive_s
#समावेश "blockcmd.h" // scsi_drive_setup
#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "pci.h" // pci_config_readl
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_DEVICE_ID_XXX
#समावेश "pci_regs.h" // PCI_VENDOR_ID
#समावेश "stacks.h" // yield
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // timer_calc

#घोषणा MFI_DB 0x0 // Doorbell
#घोषणा MFI_OMSG0 0x18 // Outbound message 0
#घोषणा MFI_IDB 0x20 // Inbound doorbell
#घोषणा MFI_ODB 0x2c // Outbound doorbell
#घोषणा MFI_IQP 0x40 // Inbound queue port
#घोषणा MFI_OSP0 0xb0 // Outbound scratch pad0
#घोषणा MFI_IQPL 0xc0 // Inbound queue port (low bytes)
#घोषणा MFI_IQPH 0xc4 // Inbound queue port (high bytes)

#घोषणा MFI_STATE_MASK                0xf0000000
#घोषणा MFI_STATE_WAIT_HANDSHAKE      0x60000000
#घोषणा MFI_STATE_BOOT_MESSAGE_PENDING 0x90000000
#घोषणा MFI_STATE_READY               0xb0000000
#घोषणा MFI_STATE_OPERATIONAL         0xc0000000
#घोषणा MFI_STATE_FAULT               0xf0000000

/* MFI Commands */
प्रकार क्रमागत {
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

काष्ठा megasas_cmd_frame {
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
    u32 तथ्य_xfer_len;   /*14h */

    जोड़ {
        काष्ठा {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        काष्ठा {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        काष्ठा {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

काष्ठा mfi_ld_list_s {
    u32     गिनती;
    u32     reserved_0;
    काष्ठा {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         आकार;
    } lds[64];
} __attribute__ ((packed));

#घोषणा MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

काष्ठा megasas_lun_s {
    काष्ठा drive_s drive;
    काष्ठा megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

अटल पूर्णांक megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            काष्ठा megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    पूर्णांक frame_गिनती = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    अगर (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_गिनती << 1 | 1, ioaddr + MFI_IQPL);
    } अन्यथा अगर (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_गिनती, ioaddr + MFI_IQP);
    } अन्यथा {
        outl(frame_addr | frame_गिनती << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    करो {
        क्रम (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            अगर (cmd_state != 0xff)
                अवरोध;
            अगर (timer_check(end)) {
                warn_timeout();
                वापस -1;
            }
            yield();
        }
    } जबतक (cmd_state == 0xff);

    अगर (cmd_state == 0 || cmd_state == 0x2d)
        वापस 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    वापस -1;
}

पूर्णांक
megasas_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_MEGASAS)
        वापस DISK_RET_EBADTRACK;
    u8 cdb[16];
    पूर्णांक blocksize = scsi_fill_cmd(op, cdb, माप(cdb));
    अगर (blocksize < 0)
        वापस default_process_op(op);
    काष्ठा megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, काष्ठा megasas_lun_s, drive);
    काष्ठा megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    पूर्णांक i;

    memset_fl(frame, 0, माप(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->तथ्य_xfer_len, op->गिनती * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    क्रम (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->गिनती, blocksize);

    अगर (op->गिनती) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->गिनती * blocksize);
        SET_LOWFLAT(frame->sge_गिनती, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    अगर (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        वापस DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    वापस DISK_RET_EBADTRACK;
}

अटल पूर्णांक
megasas_add_lun(काष्ठा pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    काष्ठा megasas_lun_s *mlun = malloc_fseg(माप(*mlun));
    अक्षर *name;
    पूर्णांक prio, ret = 0;

    अगर (!mlun) {
        warn_noalloc();
        वापस -1;
    }
    स_रखो(mlun, 0, माप(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_निचला(256, माप(काष्ठा megasas_cmd_frame));
    अगर (!mlun->frame) {
        warn_noalloc();
        मुक्त(mlun);
        वापस -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    मुक्त(name);
    अगर (ret) {
        मुक्त(mlun->frame);
        मुक्त(mlun);
        ret = -1;
    }

    वापस ret;
}

अटल व्योम megasas_scan_target(काष्ठा pci_device *pci, u32 iobase)
{
    काष्ठा mfi_ld_list_s ld_list;
    काष्ठा megasas_cmd_frame *frame = memalign_tmp(256, माप(*frame));
    अगर (!frame) {
        warn_noalloc();
        वापस;
    }

    स_रखो(&ld_list, 0, माप(ld_list));
    memset_fl(frame, 0, माप(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_गिनती = 1;
    frame->flags = 0x0011;
    frame->तथ्य_xfer_len = माप(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = माप(ld_list);
    frame->context = (u32)frame;

    अगर (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.गिनती);
        पूर्णांक i;
        क्रम (i = 0; i < ld_list.गिनती; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            अगर (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

अटल पूर्णांक megasas_transition_to_ready(काष्ठा pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    अगर (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    अन्यथा
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    जबतक (fw_state != new_state) {
        चयन (new_state) {
        हाल MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            वापस -1;
            अवरोध;
        हाल MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        हाल MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            अगर (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } अन्यथा {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            अवरोध;
        हाल MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            अगर (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                अगर (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    पूर्णांक j = 0;
                    u32 doorbell;

                    जबतक (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        अगर (!doorbell)
                            अवरोध;
                        msleep(20);
                        j++;
                    }
                }
            } अन्यथा {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            अवरोध;
        हाल MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            वापस 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        क्रम (;;) {
            अगर (timer_check(end)) {
                अवरोध;
            }
            yield();
            fw_state = new_state;
            अगर (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            अन्यथा
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            अगर (new_state != fw_state) {
                अवरोध;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    वापस -1;
}

अटल व्योम
init_megasas(व्योम *तथ्य)
{
    काष्ठा pci_device *pci = तथ्य;
    u32 bar = PCI_BASE_ADDRESS_2;
    अगर (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    अगर (!iobase)
        वापस;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    अगर (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

व्योम
megasas_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (!CONFIG_MEGASAS)
        वापस;

    dprintf(3, "init megasas\n");

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            जारी;
        अगर (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
