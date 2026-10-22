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

#تعریف_کرو MFI_DB 0x0 // Doorbell
#تعریف_کرو MFI_OMSG0 0x18 // Outbound message 0
#تعریف_کرو MFI_IDB 0x20 // Inbound doorbell
#تعریف_کرو MFI_ODB 0x2c // Outbound doorbell
#تعریف_کرو MFI_IQP 0x40 // Inbound queue port
#تعریف_کرو MFI_OSP0 0xb0 // Outbound scratch pad0
#تعریف_کرو MFI_IQPL 0xc0 // Inbound queue port (low bytes)
#تعریف_کرو MFI_IQPH 0xc4 // Inbound queue port (high bytes)

#تعریف_کرو MFI_STATE_نقاب                0xf0000000
#تعریف_کرو MFI_STATE_WAIT_HANDSHAKE      0x60000000
#تعریف_کرو MFI_STATE_BOOT_پیغام_PENDING 0x90000000
#تعریف_کرو MFI_STATE_READY               0xb0000000
#تعریف_کرو MFI_STATE_OPERATIONAL         0xc0000000
#تعریف_کرو MFI_STATE_FAULT               0xf0000000

/* MFI Commands */
قسمتعریف شمار {
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

ساخت megasas_cmd_چوکھٹا {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_حالت;      /*02h */
    u8 scsi_حالت;     /*03h */

    u8 ہدف_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_گنتی;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 ڈیٹا_xfer_len;   /*14h */

    اتحاد {
        ساخت {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        ساخت {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        ساخت {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

ساخت mfi_ld_list_s {
    u32     گنتی;
    u32     reserved_0;
    ساخت {
        u8          ہدف;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         سائز;
    } lds[64];
} __attribute__ ((packed));

#تعریف_کرو MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

ساخت megasas_lun_s {
    ساخت drive_s drive;
    ساخت megasas_cmd_چوکھٹا *چوکھٹا;
    u32 iobase;
    u16 pci_id;
    u8 ہدف;
    u8 lun;
};

ساکن صحیح megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            ساخت megasas_cmd_چوکھٹا *چوکھٹا)
{
    u32 چوکھٹا_addr = (u32)چوکھٹا;
    صحیح چوکھٹا_گنتی = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", چوکھٹا_addr);
    اگر (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(چوکھٹا_addr | چوکھٹا_گنتی << 1 | 1, ioaddr + MFI_IQPL);
    } ورنہ اگر (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(چوکھٹا_addr >> 3 | چوکھٹا_گنتی, ioaddr + MFI_IQP);
    } ورنہ {
        outl(چوکھٹا_addr | چوکھٹا_گنتی << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 انتہا = timer_calc(MEGASAS_POLL_TIMEOUT);
    کرو {
        برائے (;;) {
            cmd_state = GET_LOWFLAT(چوکھٹا->cmd_حالت);
            اگر (cmd_state != 0xff)
                توڑو;
            اگر (timer_check(انتہا)) {
                warn_timeout();
                واپس -1;
            }
            yield();
        }
    } جب (cmd_state == 0xff);

    اگر (cmd_state == 0 || cmd_state == 0x2d)
        واپس 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", چوکھٹا_addr, cmd_state);
    واپس -1;
}

صحیح
megasas_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_MEGASAS)
        واپس DISK_RET_EBADTRACK;
    u8 cdb[16];
    صحیح blocksize = scsi_fill_cmd(op, cdb, حجم(cdb));
    اگر (blocksize < 0)
        واپس default_process_op(op);
    ساخت megasas_lun_s *mlun_gf =
        container_کا(op->drive_fl, ساخت megasas_lun_s, drive);
    ساخت megasas_cmd_چوکھٹا *چوکھٹا = GET_GLOBALFLAT(mlun_gf->چوکھٹا);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    صحیح i;

    memset_fl(چوکھٹا, 0, حجم(*چوکھٹا));
    سیٹ_LOWFLAT(چوکھٹا->cmd, MFI_CMD_LD_SCSI_IO);
    سیٹ_LOWFLAT(چوکھٹا->cmd_حالت, 0xFF);
    سیٹ_LOWFLAT(چوکھٹا->ہدف_id, GET_GLOBALFLAT(mlun_gf->ہدف));
    سیٹ_LOWFLAT(چوکھٹا->lun, GET_GLOBALFLAT(mlun_gf->lun));
    سیٹ_LOWFLAT(چوکھٹا->flags, 0x0001);
    سیٹ_LOWFLAT(چوکھٹا->ڈیٹا_xfer_len, op->گنتی * blocksize);
    سیٹ_LOWFLAT(چوکھٹا->cdb_len, 16);

    برائے (i = 0; i < 16; i++) {
        سیٹ_LOWFLAT(چوکھٹا->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->گنتی, blocksize);

    اگر (op->گنتی) {
        سیٹ_LOWFLAT(چوکھٹا->pthru.sgl_addr, (u32)op->buf_fl);
        سیٹ_LOWFLAT(چوکھٹا->pthru.sgl_len, op->گنتی * blocksize);
        سیٹ_LOWFLAT(چوکھٹا->sge_گنتی, 1);
    }
    سیٹ_LOWFLAT(چوکھٹا->context, (u32)چوکھٹا);

    اگر (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), چوکھٹا) == 0)
        واپس DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    واپس DISK_RET_EBADTRACK;
}

ساکن صحیح
megasas_جوڑو_lun(ساخت pci_device *pci, u32 iobase, u8 ہدف, u8 lun)
{
    ساخت megasas_lun_s *mlun = malloc_fseg(حجم(*mlun));
    حرف *name;
    صحیح prio, ret = 0;

    اگر (!mlun) {
        warn_noalloc();
        واپس -1;
    }
    memset(mlun, 0, حجم(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->ہدف = ہدف;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->چوکھٹا = memalign_کم(256, حجم(ساخت megasas_cmd_چوکھٹا));
    اگر (!mlun->چوکھٹا) {
        warn_noalloc();
        آزاد(mlun);
        واپس -1;
    }
    boot_lchs_ڈھونڈو_scsi_device(pci, ہدف, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, ہدف, lun);
    prio = bootprio_ڈھونڈو_scsi_device(pci, ہدف, lun);
    ret = scsi_drive_تیاری(&mlun->drive, name, prio);
    آزاد(name);
    اگر (ret) {
        آزاد(mlun->چوکھٹا);
        آزاد(mlun);
        ret = -1;
    }

    واپس ret;
}

ساکن خالی megasas_جائزہ_ہدف(ساخت pci_device *pci, u32 iobase)
{
    ساخت mfi_ld_list_s ld_list;
    ساخت megasas_cmd_چوکھٹا *چوکھٹا = memalign_tmp(256, حجم(*چوکھٹا));
    اگر (!چوکھٹا) {
        warn_noalloc();
        واپس;
    }

    memset(&ld_list, 0, حجم(ld_list));
    memset_fl(چوکھٹا, 0, حجم(*چوکھٹا));

    چوکھٹا->cmd = MFI_CMD_DCMD;
    چوکھٹا->cmd_حالت = 0xFF;
    چوکھٹا->sge_گنتی = 1;
    چوکھٹا->flags = 0x0011;
    چوکھٹا->ڈیٹا_xfer_len = حجم(ld_list);
    چوکھٹا->dcmd.opcode = 0x03010000;
    چوکھٹا->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    چوکھٹا->dcmd.sgl_len = حجم(ld_list);
    چوکھٹا->context = (u32)چوکھٹا;

    اگر (megasas_fire_cmd(pci->device, iobase, چوکھٹا) == 0) {
        dprintf(2, "%d LD found\n", ld_list.گنتی);
        صحیح i;
        برائے (i = 0; i < ld_list.گنتی; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].ہدف, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            اگر (ld_list.lds[i].state != 0) {
                megasas_جوڑو_lun(pci, iobase,
                                ld_list.lds[i].ہدف, ld_list.lds[i].lun);
            }
        }
    }
}

ساکن صحیح megasas_transition_مقصد_ready(ساخت pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    اگر (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_نقاب;
    ورنہ
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_نقاب;

    جب (fw_state != new_state) {
        انتخاب (new_state) {
        حالت MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            واپس -1;
            توڑو;
        حالت MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        حالت MFI_STATE_BOOT_پیغام_PENDING:
            mfi_flags |= 0x10;
            اگر (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } ورنہ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            توڑو;
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

                    جب (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        اگر (!doorbell)
                            توڑو;
                        msleep(20);
                        j++;
                    }
                }
            } ورنہ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            توڑو;
        حالت MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            واپس 0;
        }
        // The current state should not last longer than poll timeout
        u32 انتہا = timer_calc(MEGASAS_POLL_TIMEOUT);
        برائے (;;) {
            اگر (timer_check(انتہا)) {
                توڑو;
            }
            yield();
            fw_state = new_state;
            اگر (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_نقاب;
            ورنہ
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_نقاب;
            اگر (new_state != fw_state) {
                توڑو;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_نقاب);
    واپس -1;
}

ساکن خالی
init_megasas(خالی *ڈیٹا)
{
    ساخت pci_device *pci = ڈیٹا;
    u32 bar = PCI_بنیاد_پتہ_2;
    اگر (!(pci_config_readl(pci->bdf, bar) & PCI_بنیاد_پتہ_IO_نقاب))
        bar = PCI_بنیاد_پتہ_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    اگر (!iobase)
        واپس;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    اگر (megasas_transition_مقصد_ready(pci, iobase) == 0)
        megasas_جائزہ_ہدف(pci, iobase);
}

خالی
megasas_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_MEGASAS)
        واپس;

    dprintf(3, "init megasas\n");

    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            جاری;
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
            تسلسل_thread(init_megasas, pci);
    }
}
