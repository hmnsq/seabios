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

#وصف_ڪريو MFI_DB 0x0 // Doorbell
#وصف_ڪريو MFI_OMSG0 0x18 // Outbound message 0
#وصف_ڪريو MFI_IDB 0x20 // Inbound doorbell
#وصف_ڪريو MFI_ODB 0x2c // Outbound doorbell
#وصف_ڪريو MFI_IQP 0x40 // Inbound queue port
#وصف_ڪريو MFI_OSP0 0xb0 // Outbound scratch pad0
#وصف_ڪريو MFI_IQPL 0xc0 // Inbound queue port (low bytes)
#وصف_ڪريو MFI_IQPH 0xc4 // Inbound queue port (high bytes)

#وصف_ڪريو MFI_STATE_نقاب                0xf0000000
#وصف_ڪريو MFI_STATE_WAIT_HANDSHAKE      0x60000000
#وصف_ڪريو MFI_STATE_BOOT_پيغام_PENDING 0x90000000
#وصف_ڪريو MFI_STATE_READY               0xb0000000
#وصف_ڪريو MFI_STATE_OPERATIONAL         0xc0000000
#وصف_ڪريو MFI_STATE_FAULT               0xf0000000

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

بنية megasas_cmd_چوکٺو {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_حالت;      /*02h */
    u8 scsi_حالت;     /*03h */

    u8 هدف_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_ڳڻپ;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 معلومات_xfer_len;   /*14h */

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
    u32     ڳڻپ;
    u32     reserved_0;
    بنية {
        u8          هدف;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         سائيز;
    } lds[64];
} __attribute__ ((packed));

#وصف_ڪريو MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

بنية megasas_lun_s {
    بنية drive_s drive;
    بنية megasas_cmd_چوکٺو *چوکٺو;
    u32 iobase;
    u16 pci_id;
    u8 هدف;
    u8 lun;
};

ساكن صحيح megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            بنية megasas_cmd_چوکٺو *چوکٺو)
{
    u32 چوکٺو_addr = (u32)چوکٺو;
    صحيح چوکٺو_ڳڻپ = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", چوکٺو_addr);
    إذا (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(چوکٺو_addr | چوکٺو_ڳڻپ << 1 | 1, ioaddr + MFI_IQPL);
    } وإلا إذا (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(چوکٺو_addr >> 3 | چوکٺو_ڳڻپ, ioaddr + MFI_IQP);
    } وإلا {
        outl(چوکٺو_addr | چوکٺو_ڳڻپ << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 پڇاڙي = timer_calc(MEGASAS_POLL_TIMEOUT);
    افعل {
        لكل (;;) {
            cmd_state = GET_LOWFLAT(چوکٺو->cmd_حالت);
            إذا (cmd_state != 0xff)
                اكسر;
            إذا (timer_check(پڇاڙي)) {
                warn_timeout();
                أرجع -1;
            }
            yield();
        }
    } طالما (cmd_state == 0xff);

    إذا (cmd_state == 0 || cmd_state == 0x2d)
        أرجع 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", چوکٺو_addr, cmd_state);
    أرجع -1;
}

صحيح
megasas_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_MEGASAS)
        أرجع DISK_RET_EBADTRACK;
    u8 cdb[16];
    صحيح blocksize = scsi_fill_cmd(op, cdb, حجم(cdb));
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    بنية megasas_lun_s *mlun_gf =
        container_جو(op->drive_fl, بنية megasas_lun_s, drive);
    بنية megasas_cmd_چوکٺو *چوکٺو = GET_GLOBALFLAT(mlun_gf->چوکٺو);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    صحيح i;

    memset_fl(چوکٺو, 0, حجم(*چوکٺو));
    سيٽ_LOWFLAT(چوکٺو->cmd, MFI_CMD_LD_SCSI_IO);
    سيٽ_LOWFLAT(چوکٺو->cmd_حالت, 0xFF);
    سيٽ_LOWFLAT(چوکٺو->هدف_id, GET_GLOBALFLAT(mlun_gf->هدف));
    سيٽ_LOWFLAT(چوکٺو->lun, GET_GLOBALFLAT(mlun_gf->lun));
    سيٽ_LOWFLAT(چوکٺو->flags, 0x0001);
    سيٽ_LOWFLAT(چوکٺو->معلومات_xfer_len, op->ڳڻپ * blocksize);
    سيٽ_LOWFLAT(چوکٺو->cdb_len, 16);

    لكل (i = 0; i < 16; i++) {
        سيٽ_LOWFLAT(چوکٺو->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->ڳڻپ, blocksize);

    إذا (op->ڳڻپ) {
        سيٽ_LOWFLAT(چوکٺو->pthru.sgl_addr, (u32)op->buf_fl);
        سيٽ_LOWFLAT(چوکٺو->pthru.sgl_len, op->ڳڻپ * blocksize);
        سيٽ_LOWFLAT(چوکٺو->sge_ڳڻپ, 1);
    }
    سيٽ_LOWFLAT(چوکٺو->context, (u32)چوکٺو);

    إذا (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), چوکٺو) == 0)
        أرجع DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    أرجع DISK_RET_EBADTRACK;
}

ساكن صحيح
megasas_جوڙيو_lun(بنية pci_device *pci, u32 iobase, u8 هدف, u8 lun)
{
    بنية megasas_lun_s *mlun = malloc_fseg(حجم(*mlun));
    حرف *name;
    صحيح prio, ret = 0;

    إذا (!mlun) {
        warn_noalloc();
        أرجع -1;
    }
    memset(mlun, 0, حجم(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->هدف = هدف;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->چوکٺو = memalign_هيٺيون(256, حجم(بنية megasas_cmd_چوکٺو));
    إذا (!mlun->چوکٺو) {
        warn_noalloc();
        حرر(mlun);
        أرجع -1;
    }
    boot_lchs_ڳوليو_scsi_device(pci, هدف, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, هدف, lun);
    prio = bootprio_ڳوليو_scsi_device(pci, هدف, lun);
    ret = scsi_drive_تياري(&mlun->drive, name, prio);
    حرر(name);
    إذا (ret) {
        حرر(mlun->چوکٺو);
        حرر(mlun);
        ret = -1;
    }

    أرجع ret;
}

ساكن فراغ megasas_جاچ_هدف(بنية pci_device *pci, u32 iobase)
{
    بنية mfi_ld_list_s ld_list;
    بنية megasas_cmd_چوکٺو *چوکٺو = memalign_tmp(256, حجم(*چوکٺو));
    إذا (!چوکٺو) {
        warn_noalloc();
        أرجع;
    }

    memset(&ld_list, 0, حجم(ld_list));
    memset_fl(چوکٺو, 0, حجم(*چوکٺو));

    چوکٺو->cmd = MFI_CMD_DCMD;
    چوکٺو->cmd_حالت = 0xFF;
    چوکٺو->sge_ڳڻپ = 1;
    چوکٺو->flags = 0x0011;
    چوکٺو->معلومات_xfer_len = حجم(ld_list);
    چوکٺو->dcmd.opcode = 0x03010000;
    چوکٺو->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    چوکٺو->dcmd.sgl_len = حجم(ld_list);
    چوکٺو->context = (u32)چوکٺو;

    إذا (megasas_fire_cmd(pci->device, iobase, چوکٺو) == 0) {
        dprintf(2, "%d LD found\n", ld_list.ڳڻپ);
        صحيح i;
        لكل (i = 0; i < ld_list.ڳڻپ; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].هدف, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            إذا (ld_list.lds[i].state != 0) {
                megasas_جوڙيو_lun(pci, iobase,
                                ld_list.lds[i].هدف, ld_list.lds[i].lun);
            }
        }
    }
}

ساكن صحيح megasas_transition_منزل_ready(بنية pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    إذا (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_نقاب;
    وإلا
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_نقاب;

    طالما (fw_state != new_state) {
        بدل (new_state) {
        حالة MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            أرجع -1;
            اكسر;
        حالة MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        حالة MFI_STATE_BOOT_پيغام_PENDING:
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

                    طالما (j < MEGASAS_POLL_TIMEOUT) {
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
            أرجع 0;
        }
        // The current state should not last longer than poll timeout
        u32 پڇاڙي = timer_calc(MEGASAS_POLL_TIMEOUT);
        لكل (;;) {
            إذا (timer_check(پڇاڙي)) {
                اكسر;
            }
            yield();
            fw_state = new_state;
            إذا (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_نقاب;
            وإلا
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_نقاب;
            إذا (new_state != fw_state) {
                اكسر;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_نقاب);
    أرجع -1;
}

ساكن فراغ
init_megasas(فراغ *معلومات)
{
    بنية pci_device *pci = معلومات;
    u32 bar = PCI_بنياد_پتو_2;
    إذا (!(pci_config_readl(pci->bdf, bar) & PCI_بنياد_پتو_IO_نقاب))
        bar = PCI_بنياد_پتو_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    إذا (!iobase)
        أرجع;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    إذا (megasas_transition_منزل_ready(pci, iobase) == 0)
        megasas_جاچ_هدف(pci, iobase);
}

فراغ
megasas_تياري(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_MEGASAS)
        أرجع;

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
            لاڳيتو_thread(init_megasas, pci);
    }
}
