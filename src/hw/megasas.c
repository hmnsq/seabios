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

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBALFLAT
#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "blockcmd.h" // scsi_drive_setup
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pci.h" // pci_config_readl
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_DEVICE_ID_XXX
#അന്തര്ഭാവ "pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // timer_calc

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
തരനിർവചനം എണ്ണൽ {
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

ഘടന megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_എണ്ണം;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    സംഘം {
        ഘടന {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        ഘടന {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        ഘടന {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

ഘടന mfi_ld_list_s {
    u32     എണ്ണം;
    u32     reserved_0;
    ഘടന {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

ഘടന megasas_lun_s {
    ഘടന drive_s drive;
    ഘടന megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            ഘടന megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    പൂർണ്ണസംഖ്യ frame_എണ്ണം = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    എങ്കിൽ (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_എണ്ണം << 1 | 1, ioaddr + MFI_IQPL);
    } അല്ലെങ്കിൽ എങ്കിൽ (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_എണ്ണം, ioaddr + MFI_IQP);
    } അല്ലെങ്കിൽ {
        outl(frame_addr | frame_എണ്ണം << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    ചെയ്യുക {
        വേണ്ടി (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            എങ്കിൽ (cmd_state != 0xff)
                നിർത്തുക;
            എങ്കിൽ (timer_check(end)) {
                warn_timeout();
                തിരികെ -1;
            }
            yield();
        }
    } ആകുമ്പോൾ (cmd_state == 0xff);

    എങ്കിൽ (cmd_state == 0 || cmd_state == 0x2d)
        തിരികെ 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    തിരികെ -1;
}

പൂർണ്ണസംഖ്യ
megasas_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_MEGASAS)
        തിരികെ DISK_RET_EBADTRACK;
    u8 cdb[16];
    പൂർണ്ണസംഖ്യ blocksize = scsi_fill_cmd(op, cdb, വലുപ്പം(cdb));
    എങ്കിൽ (blocksize < 0)
        തിരികെ default_process_op(op);
    ഘടന megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, ഘടന megasas_lun_s, drive);
    ഘടന megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    പൂർണ്ണസംഖ്യ i;

    memset_fl(frame, 0, വലുപ്പം(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->എണ്ണം * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    വേണ്ടി (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->എണ്ണം, blocksize);

    എങ്കിൽ (op->എണ്ണം) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->എണ്ണം * blocksize);
        SET_LOWFLAT(frame->sge_എണ്ണം, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    എങ്കിൽ (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        തിരികെ DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    തിരികെ DISK_RET_EBADTRACK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
megasas_add_lun(ഘടന pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    ഘടന megasas_lun_s *mlun = malloc_fseg(വലുപ്പം(*mlun));
    അക്ഷരം *name;
    പൂർണ്ണസംഖ്യ prio, ret = 0;

    എങ്കിൽ (!mlun) {
        warn_noalloc();
        തിരികെ -1;
    }
    memset(mlun, 0, വലുപ്പം(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_താഴെയുള്ള(256, വലുപ്പം(ഘടന megasas_cmd_frame));
    എങ്കിൽ (!mlun->frame) {
        warn_noalloc();
        സ്വതന്ത്രം(mlun);
        തിരികെ -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    സ്വതന്ത്രം(name);
    എങ്കിൽ (ret) {
        സ്വതന്ത്രം(mlun->frame);
        സ്വതന്ത്രം(mlun);
        ret = -1;
    }

    തിരികെ ret;
}

സ്ഥിതം ശൂന്യം megasas_scan_target(ഘടന pci_device *pci, u32 iobase)
{
    ഘടന mfi_ld_list_s ld_list;
    ഘടന megasas_cmd_frame *frame = memalign_tmp(256, വലുപ്പം(*frame));
    എങ്കിൽ (!frame) {
        warn_noalloc();
        തിരികെ;
    }

    memset(&ld_list, 0, വലുപ്പം(ld_list));
    memset_fl(frame, 0, വലുപ്പം(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_എണ്ണം = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = വലുപ്പം(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = വലുപ്പം(ld_list);
    frame->context = (u32)frame;

    എങ്കിൽ (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.എണ്ണം);
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i = 0; i < ld_list.എണ്ണം; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            എങ്കിൽ (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ megasas_transition_to_ready(ഘടന pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    എങ്കിൽ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    അല്ലെങ്കിൽ
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    ആകുമ്പോൾ (fw_state != new_state) {
        തിരഞ്ഞെടുപ്പ് (new_state) {
        അവസ്ഥ MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            തിരികെ -1;
            നിർത്തുക;
        അവസ്ഥ MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        അവസ്ഥ MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            എങ്കിൽ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } അല്ലെങ്കിൽ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            നിർത്തുക;
        അവസ്ഥ MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            എങ്കിൽ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                എങ്കിൽ (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    പൂർണ്ണസംഖ്യ j = 0;
                    u32 doorbell;

                    ആകുമ്പോൾ (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        എങ്കിൽ (!doorbell)
                            നിർത്തുക;
                        msleep(20);
                        j++;
                    }
                }
            } അല്ലെങ്കിൽ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            നിർത്തുക;
        അവസ്ഥ MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            തിരികെ 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        വേണ്ടി (;;) {
            എങ്കിൽ (timer_check(end)) {
                നിർത്തുക;
            }
            yield();
            fw_state = new_state;
            എങ്കിൽ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            അല്ലെങ്കിൽ
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            എങ്കിൽ (new_state != fw_state) {
                നിർത്തുക;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    തിരികെ -1;
}

സ്ഥിതം ശൂന്യം
init_megasas(ശൂന്യം *data)
{
    ഘടന pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    എങ്കിൽ (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    എങ്കിൽ (!iobase)
        തിരികെ;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    എങ്കിൽ (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

ശൂന്യം
megasas_setup(ശൂന്യം)
{
    ASSERT32FLAT();
    എങ്കിൽ (!CONFIG_MEGASAS)
        തിരികെ;

    dprintf(3, "init megasas\n");

    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            തുടരുക;
        എങ്കിൽ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
