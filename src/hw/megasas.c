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

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // scsi_drive_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pci.h" // pci_config_readl
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_DEVICE_ID_XXX
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_DB 0x0 // Doorbell
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_OMSG0 0x18 // Outbound message 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_IDB 0x20 // Inbound doorbell
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_ODB 0x2c // Outbound doorbell
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_IQP 0x40 // Inbound queue port
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_OSP0 0xb0 // Outbound scratch pad0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_IQPL 0xc0 // Inbound queue port (low bytes)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_IQPH 0xc4 // Inbound queue port (high bytes)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_STATE_ਨਕਾਬ                0xf0000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_STATE_WAIT_HANDSHAKE      0x60000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_STATE_BOOT_ਸੁਨੇਹਾ_PENDING 0x90000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_STATE_READY               0xb0000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_STATE_OPERATIONAL         0xc0000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFI_STATE_FAULT               0xf0000000

/* MFI Commands */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
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

ਬਣਤਰ megasas_cmd_ਚੌਖਟਾ {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_ਸਥਿਤੀ;      /*02h */
    u8 scsi_ਸਥਿਤੀ;     /*03h */

    u8 ਨਿਸ਼ਾਨਾ_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_ਗਿਣਤੀ;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 ਅੰਕੜੇ_xfer_len;   /*14h */

    ਸੰਘ {
        ਬਣਤਰ {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        ਬਣਤਰ {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        ਬਣਤਰ {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

ਬਣਤਰ mfi_ld_list_s {
    u32     ਗਿਣਤੀ_2;
    u32     reserved_0;
    ਬਣਤਰ {
        u8          ਨਿਸ਼ਾਨਾ;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         ਮਾਪ;
    } lds[64];
} __attribute__ ((packed));

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

ਬਣਤਰ megasas_lun_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ megasas_cmd_ਚੌਖਟਾ *ਚੌਖਟਾ;
    u32 iobase;
    u16 pci_id;
    u8 ਨਿਸ਼ਾਨਾ;
    u8 lun;
};

ਸਥਿਰ ਪੂਰਨ megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            ਬਣਤਰ megasas_cmd_ਚੌਖਟਾ *ਚੌਖਟਾ)
{
    u32 ਚੌਖਟਾ_addr = (u32)ਚੌਖਟਾ;
    ਪੂਰਨ ਚੌਖਟਾ_ਗਿਣਤੀ = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", ਚੌਖਟਾ_addr);
    ਜੇ (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(ਚੌਖਟਾ_addr | ਚੌਖਟਾ_ਗਿਣਤੀ << 1 | 1, ioaddr + MFI_IQPL);
    } ਨਹੀਂਤਾਂ ਜੇ (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(ਚੌਖਟਾ_addr >> 3 | ਚੌਖਟਾ_ਗਿਣਤੀ, ioaddr + MFI_IQP);
    } ਨਹੀਂਤਾਂ {
        outl(ਚੌਖਟਾ_addr | ਚੌਖਟਾ_ਗਿਣਤੀ << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 ਸਿਰਾ = timer_calc(MEGASAS_POLL_TIMEOUT);
    ਕਰੋ {
        ਲਈ (;;) {
            cmd_state = GET_LOWFLAT(ਚੌਖਟਾ->cmd_ਸਥਿਤੀ);
            ਜੇ (cmd_state != 0xff)
                ਰੋਕੋ;
            ਜੇ (timer_check(ਸਿਰਾ)) {
                warn_timeout();
                ਵਾਪਸ -1;
            }
            yield();
        }
    } ਜਦੋਂ (cmd_state == 0xff);

    ਜੇ (cmd_state == 0 || cmd_state == 0x2d)
        ਵਾਪਸ 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", ਚੌਖਟਾ_addr, cmd_state);
    ਵਾਪਸ -1;
}

ਪੂਰਨ
megasas_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_MEGASAS)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    u8 cdb[16];
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, cdb, ਆਕਾਰ(cdb));
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);
    ਬਣਤਰ megasas_lun_s *mlun_gf =
        container_ਦਾ(op->drive_fl, ਬਣਤਰ megasas_lun_s, drive);
    ਬਣਤਰ megasas_cmd_ਚੌਖਟਾ *ਚੌਖਟਾ = GET_GLOBALFLAT(mlun_gf->ਚੌਖਟਾ);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    ਪੂਰਨ i;

    memset_fl(ਚੌਖਟਾ, 0, ਆਕਾਰ(*ਚੌਖਟਾ));
    ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->cmd, MFI_CMD_LD_SCSI_IO);
    ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->cmd_ਸਥਿਤੀ, 0xFF);
    ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->ਨਿਸ਼ਾਨਾ_id, GET_GLOBALFLAT(mlun_gf->ਨਿਸ਼ਾਨਾ));
    ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->lun, GET_GLOBALFLAT(mlun_gf->lun));
    ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->flags, 0x0001);
    ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->ਅੰਕੜੇ_xfer_len, op->ਗਿਣਤੀ_2 * blocksize);
    ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->cdb_len, 16);

    ਲਈ (i = 0; i < 16; i++) {
        ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->ਗਿਣਤੀ_2, blocksize);

    ਜੇ (op->ਗਿਣਤੀ_2) {
        ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->pthru.sgl_addr, (u32)op->buf_fl);
        ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->pthru.sgl_len, op->ਗਿਣਤੀ_2 * blocksize);
        ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->sge_ਗਿਣਤੀ, 1);
    }
    ਸੈੱਟ_LOWFLAT(ਚੌਖਟਾ->context, (u32)ਚੌਖਟਾ);

    ਜੇ (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), ਚੌਖਟਾ) == 0)
        ਵਾਪਸ DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    ਵਾਪਸ DISK_RET_EBADTRACK;
}

ਸਥਿਰ ਪੂਰਨ
megasas_ਜੋੜੋ_lun(ਬਣਤਰ pci_device *pci, u32 iobase, u8 ਨਿਸ਼ਾਨਾ, u8 lun)
{
    ਬਣਤਰ megasas_lun_s *mlun = malloc_fseg(ਆਕਾਰ(*mlun));
    ਅੱਖਰ *name;
    ਪੂਰਨ prio, ret = 0;

    ਜੇ (!mlun) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    memset(mlun, 0, ਆਕਾਰ(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->ਨਿਸ਼ਾਨਾ = ਨਿਸ਼ਾਨਾ;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->ਚੌਖਟਾ = memalign_ਨੀਵਾਂ(256, ਆਕਾਰ(ਬਣਤਰ megasas_cmd_ਚੌਖਟਾ));
    ਜੇ (!mlun->ਚੌਖਟਾ) {
        warn_noalloc();
        ਮੁਕਤ(mlun);
        ਵਾਪਸ -1;
    }
    boot_lchs_ਲੱਭੋ_scsi_device(pci, ਨਿਸ਼ਾਨਾ, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, ਨਿਸ਼ਾਨਾ, lun);
    prio = bootprio_ਲੱਭੋ_scsi_device(pci, ਨਿਸ਼ਾਨਾ, lun);
    ret = scsi_drive_ਤਿਆਰੀ(&mlun->drive, name, prio);
    ਮੁਕਤ(name);
    ਜੇ (ret) {
        ਮੁਕਤ(mlun->ਚੌਖਟਾ);
        ਮੁਕਤ(mlun);
        ret = -1;
    }

    ਵਾਪਸ ret;
}

ਸਥਿਰ ਖਾਲੀ megasas_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(ਬਣਤਰ pci_device *pci, u32 iobase)
{
    ਬਣਤਰ mfi_ld_list_s ld_list;
    ਬਣਤਰ megasas_cmd_ਚੌਖਟਾ *ਚੌਖਟਾ = memalign_tmp(256, ਆਕਾਰ(*ਚੌਖਟਾ));
    ਜੇ (!ਚੌਖਟਾ) {
        warn_noalloc();
        ਵਾਪਸ;
    }

    memset(&ld_list, 0, ਆਕਾਰ(ld_list));
    memset_fl(ਚੌਖਟਾ, 0, ਆਕਾਰ(*ਚੌਖਟਾ));

    ਚੌਖਟਾ->cmd = MFI_CMD_DCMD;
    ਚੌਖਟਾ->cmd_ਸਥਿਤੀ = 0xFF;
    ਚੌਖਟਾ->sge_ਗਿਣਤੀ = 1;
    ਚੌਖਟਾ->flags = 0x0011;
    ਚੌਖਟਾ->ਅੰਕੜੇ_xfer_len = ਆਕਾਰ(ld_list);
    ਚੌਖਟਾ->dcmd.opcode = 0x03010000;
    ਚੌਖਟਾ->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    ਚੌਖਟਾ->dcmd.sgl_len = ਆਕਾਰ(ld_list);
    ਚੌਖਟਾ->context = (u32)ਚੌਖਟਾ;

    ਜੇ (megasas_fire_cmd(pci->device, iobase, ਚੌਖਟਾ) == 0) {
        dprintf(2, "%d LD found\n", ld_list.ਗਿਣਤੀ_2);
        ਪੂਰਨ i;
        ਲਈ (i = 0; i < ld_list.ਗਿਣਤੀ_2; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].ਨਿਸ਼ਾਨਾ, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            ਜੇ (ld_list.lds[i].state != 0) {
                megasas_ਜੋੜੋ_lun(pci, iobase,
                                ld_list.lds[i].ਨਿਸ਼ਾਨਾ, ld_list.lds[i].lun);
            }
        }
    }
}

ਸਥਿਰ ਪੂਰਨ megasas_transition_ਮੰਜ਼ਿਲ_ready(ਬਣਤਰ pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    ਜੇ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_ਨਕਾਬ;
    ਨਹੀਂਤਾਂ
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_ਨਕਾਬ;

    ਜਦੋਂ (fw_state != new_state) {
        ਚੋਣ (new_state) {
        ਹਾਲਤ MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            ਵਾਪਸ -1;
            ਰੋਕੋ;
        ਹਾਲਤ MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        ਹਾਲਤ MFI_STATE_BOOT_ਸੁਨੇਹਾ_PENDING:
            mfi_flags |= 0x10;
            ਜੇ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } ਨਹੀਂਤਾਂ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ਰੋਕੋ;
        ਹਾਲਤ MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            ਜੇ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                ਜੇ (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    ਪੂਰਨ j = 0;
                    u32 doorbell;

                    ਜਦੋਂ (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        ਜੇ (!doorbell)
                            ਰੋਕੋ;
                        msleep(20);
                        j++;
                    }
                }
            } ਨਹੀਂਤਾਂ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ਰੋਕੋ;
        ਹਾਲਤ MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            ਵਾਪਸ 0;
        }
        // The current state should not last longer than poll timeout
        u32 ਸਿਰਾ = timer_calc(MEGASAS_POLL_TIMEOUT);
        ਲਈ (;;) {
            ਜੇ (timer_check(ਸਿਰਾ)) {
                ਰੋਕੋ;
            }
            yield();
            fw_state = new_state;
            ਜੇ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_ਨਕਾਬ;
            ਨਹੀਂਤਾਂ
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_ਨਕਾਬ;
            ਜੇ (new_state != fw_state) {
                ਰੋਕੋ;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_ਨਕਾਬ);
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਖਾਲੀ
init_megasas(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ pci_device *pci = ਅੰਕੜੇ;
    u32 bar = PCI_ਅਧਾਰ_ਪਤਾ_2;
    ਜੇ (!(pci_config_readl(pci->bdf, bar) & PCI_ਅਧਾਰ_ਪਤਾ_IO_ਨਕਾਬ))
        bar = PCI_ਅਧਾਰ_ਪਤਾ_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    ਜੇ (!iobase)
        ਵਾਪਸ;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    ਜੇ (megasas_transition_ਮੰਜ਼ਿਲ_ready(pci, iobase) == 0)
        megasas_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(pci, iobase);
}

ਖਾਲੀ
megasas_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_MEGASAS)
        ਵਾਪਸ;

    dprintf(3, "init megasas\n");

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            ਜਾਰੀ;
        ਜੇ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
            ਲੜੀਵਾਰ_thread(init_megasas, pci);
    }
}
