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

#ವ್ಯಾಖ್ಯಾನಿಸು MFI_DB 0x0 // Doorbell
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_OMSG0 0x18 // Outbound message 0
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_IDB 0x20 // Inbound doorbell
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_ODB 0x2c // Outbound doorbell
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_IQP 0x40 // Inbound queue port
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_OSP0 0xb0 // Outbound scratch pad0
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_IQPL 0xc0 // Inbound queue port (low bytes)
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_IQPH 0xc4 // Inbound queue port (high bytes)

#ವ್ಯಾಖ್ಯಾನಿಸು MFI_STATE_ಮುಖವಾಡ                0xf0000000
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_STATE_WAIT_HANDSHAKE      0x60000000
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_STATE_BOOT_ಸಂದೇಶ_PENDING 0x90000000
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_STATE_READY               0xb0000000
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_STATE_OPERATIONAL         0xc0000000
#ವ್ಯಾಖ್ಯಾನಿಸು MFI_STATE_FAULT               0xf0000000

/* MFI Commands */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
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

ರಚನೆ megasas_cmd_ಚೌಕಟ್ಟು {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_ಸ್ಥಿತಿ;      /*02h */
    u8 scsi_ಸ್ಥಿತಿ;     /*03h */

    u8 ಗುರಿ_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_ಎಣಿಕೆ;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 ದತ್ತಾಂಶ_xfer_len;   /*14h */

    ಒಕ್ಕೂಟ {
        ರಚನೆ {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        ರಚನೆ {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        ರಚನೆ {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

ರಚನೆ mfi_ld_list_s {
    u32     ಎಣಿಕೆ_2;
    u32     reserved_0;
    ರಚನೆ {
        u8          ಗುರಿ;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         ಗಾತ್ರ_2;
    } lds[64];
} __attribute__ ((packed));

#ವ್ಯಾಖ್ಯಾನಿಸು MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

ರಚನೆ megasas_lun_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ megasas_cmd_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು;
    u32 iobase;
    u16 pci_id;
    u8 ಗುರಿ;
    u8 lun;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            ರಚನೆ megasas_cmd_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು)
{
    u32 ಚೌಕಟ್ಟು_addr = (u32)ಚೌಕಟ್ಟು;
    ಪೂರ್ಣಾಂಕ ಚೌಕಟ್ಟು_ಎಣಿಕೆ = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", ಚೌಕಟ್ಟು_addr);
    ಆದರೆ (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(ಚೌಕಟ್ಟು_addr | ಚೌಕಟ್ಟು_ಎಣಿಕೆ << 1 | 1, ioaddr + MFI_IQPL);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(ಚೌಕಟ್ಟು_addr >> 3 | ಚೌಕಟ್ಟು_ಎಣಿಕೆ, ioaddr + MFI_IQP);
    } ಇಲ್ಲದಿದ್ದರೆ {
        outl(ಚೌಕಟ್ಟು_addr | ಚೌಕಟ್ಟು_ಎಣಿಕೆ << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 ಕೊನೆ = timer_calc(MEGASAS_POLL_TIMEOUT);
    ಮಾಡು {
        ಗಾಗಿ (;;) {
            cmd_state = GET_LOWFLAT(ಚೌಕಟ್ಟು->cmd_ಸ್ಥಿತಿ);
            ಆದರೆ (cmd_state != 0xff)
                ನಿಲ್ಲಿಸು;
            ಆದರೆ (timer_check(ಕೊನೆ)) {
                warn_timeout();
                ಹಿಂತಿರುಗಿ -1;
            }
            yield();
        }
    } ಆಗ (cmd_state == 0xff);

    ಆದರೆ (cmd_state == 0 || cmd_state == 0x2d)
        ಹಿಂತಿರುಗಿ 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", ಚೌಕಟ್ಟು_addr, cmd_state);
    ಹಿಂತಿರುಗಿ -1;
}

ಪೂರ್ಣಾಂಕ
megasas_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_MEGASAS)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    u8 cdb[16];
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, cdb, ಗಾತ್ರ(cdb));
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);
    ರಚನೆ megasas_lun_s *mlun_gf =
        container_ದ(op->drive_fl, ರಚನೆ megasas_lun_s, drive);
    ರಚನೆ megasas_cmd_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು = GET_GLOBALFLAT(mlun_gf->ಚೌಕಟ್ಟು);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    ಪೂರ್ಣಾಂಕ i;

    memset_fl(ಚೌಕಟ್ಟು, 0, ಗಾತ್ರ(*ಚೌಕಟ್ಟು));
    ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->cmd, MFI_CMD_LD_SCSI_IO);
    ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->cmd_ಸ್ಥಿತಿ, 0xFF);
    ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->ಗುರಿ_id, GET_GLOBALFLAT(mlun_gf->ಗುರಿ));
    ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->lun, GET_GLOBALFLAT(mlun_gf->lun));
    ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->flags, 0x0001);
    ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->ದತ್ತಾಂಶ_xfer_len, op->ಎಣಿಕೆ_2 * blocksize);
    ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->cdb_len, 16);

    ಗಾಗಿ (i = 0; i < 16; i++) {
        ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->ಎಣಿಕೆ_2, blocksize);

    ಆದರೆ (op->ಎಣಿಕೆ_2) {
        ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->pthru.sgl_addr, (u32)op->buf_fl);
        ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->pthru.sgl_len, op->ಎಣಿಕೆ_2 * blocksize);
        ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->sge_ಎಣಿಕೆ, 1);
    }
    ಗಣ_LOWFLAT(ಚೌಕಟ್ಟು->context, (u32)ಚೌಕಟ್ಟು);

    ಆದರೆ (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), ಚೌಕಟ್ಟು) == 0)
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
megasas_ಕೂಡಿಸು_lun(ರಚನೆ pci_device *pci, u32 iobase, u8 ಗುರಿ, u8 lun)
{
    ರಚನೆ megasas_lun_s *mlun = malloc_fseg(ಗಾತ್ರ(*mlun));
    ಅಕ್ಷರ *name;
    ಪೂರ್ಣಾಂಕ prio, ret = 0;

    ಆದರೆ (!mlun) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    memset(mlun, 0, ಗಾತ್ರ(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->ಗುರಿ = ಗುರಿ;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->ಚೌಕಟ್ಟು = memalign_ಕೆಳಗಿನ(256, ಗಾತ್ರ(ರಚನೆ megasas_cmd_ಚೌಕಟ್ಟು));
    ಆದರೆ (!mlun->ಚೌಕಟ್ಟು) {
        warn_noalloc();
        ಬಿಡುಗಡೆ(mlun);
        ಹಿಂತಿರುಗಿ -1;
    }
    boot_lchs_ಕಂಡುಹಿಡಿ_scsi_device(pci, ಗುರಿ, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, ಗುರಿ, lun);
    prio = bootprio_ಕಂಡುಹಿಡಿ_scsi_device(pci, ಗುರಿ, lun);
    ret = scsi_drive_ಸಿದ್ಧತೆ(&mlun->drive, name, prio);
    ಬಿಡುಗಡೆ(name);
    ಆದರೆ (ret) {
        ಬಿಡುಗಡೆ(mlun->ಚೌಕಟ್ಟು);
        ಬಿಡುಗಡೆ(mlun);
        ret = -1;
    }

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ megasas_ಪರಿಶೀಲನೆ_ಗುರಿ(ರಚನೆ pci_device *pci, u32 iobase)
{
    ರಚನೆ mfi_ld_list_s ld_list;
    ರಚನೆ megasas_cmd_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು = memalign_tmp(256, ಗಾತ್ರ(*ಚೌಕಟ್ಟು));
    ಆದರೆ (!ಚೌಕಟ್ಟು) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }

    memset(&ld_list, 0, ಗಾತ್ರ(ld_list));
    memset_fl(ಚೌಕಟ್ಟು, 0, ಗಾತ್ರ(*ಚೌಕಟ್ಟು));

    ಚೌಕಟ್ಟು->cmd = MFI_CMD_DCMD;
    ಚೌಕಟ್ಟು->cmd_ಸ್ಥಿತಿ = 0xFF;
    ಚೌಕಟ್ಟು->sge_ಎಣಿಕೆ = 1;
    ಚೌಕಟ್ಟು->flags = 0x0011;
    ಚೌಕಟ್ಟು->ದತ್ತಾಂಶ_xfer_len = ಗಾತ್ರ(ld_list);
    ಚೌಕಟ್ಟು->dcmd.opcode = 0x03010000;
    ಚೌಕಟ್ಟು->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    ಚೌಕಟ್ಟು->dcmd.sgl_len = ಗಾತ್ರ(ld_list);
    ಚೌಕಟ್ಟು->context = (u32)ಚೌಕಟ್ಟು;

    ಆದರೆ (megasas_fire_cmd(pci->device, iobase, ಚೌಕಟ್ಟು) == 0) {
        dprintf(2, "%d LD found\n", ld_list.ಎಣಿಕೆ_2);
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i = 0; i < ld_list.ಎಣಿಕೆ_2; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].ಗುರಿ, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            ಆದರೆ (ld_list.lds[i].state != 0) {
                megasas_ಕೂಡಿಸು_lun(pci, iobase,
                                ld_list.lds[i].ಗುರಿ, ld_list.lds[i].lun);
            }
        }
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ megasas_transition_ಗಮ್ಯ_ready(ರಚನೆ pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    ಆದರೆ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_ಮುಖವಾಡ;
    ಇಲ್ಲದಿದ್ದರೆ
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_ಮುಖವಾಡ;

    ಆಗ (fw_state != new_state) {
        ಆಯ್ಕೆ (new_state) {
        ಸ್ಥಿತಿ MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            ಹಿಂತಿರುಗಿ -1;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        ಸ್ಥಿತಿ MFI_STATE_BOOT_ಸಂದೇಶ_PENDING:
            mfi_flags |= 0x10;
            ಆದರೆ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } ಇಲ್ಲದಿದ್ದರೆ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            ಆದರೆ (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                ಆದರೆ (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    ಪೂರ್ಣಾಂಕ j = 0;
                    u32 doorbell;

                    ಆಗ (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        ಆದರೆ (!doorbell)
                            ನಿಲ್ಲಿಸು;
                        msleep(20);
                        j++;
                    }
                }
            } ಇಲ್ಲದಿದ್ದರೆ {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            ಹಿಂತಿರುಗಿ 0;
        }
        // The current state should not last longer than poll timeout
        u32 ಕೊನೆ = timer_calc(MEGASAS_POLL_TIMEOUT);
        ಗಾಗಿ (;;) {
            ಆದರೆ (timer_check(ಕೊನೆ)) {
                ನಿಲ್ಲಿಸು;
            }
            yield();
            fw_state = new_state;
            ಆದರೆ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_ಮುಖವಾಡ;
            ಇಲ್ಲದಿದ್ದರೆ
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_ಮುಖವಾಡ;
            ಆದರೆ (new_state != fw_state) {
                ನಿಲ್ಲಿಸು;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_ಮುಖವಾಡ);
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_megasas(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ pci_device *pci = ದತ್ತಾಂಶ;
    u32 bar = PCI_ತಳ_ವಿಳಾಸ_2;
    ಆದರೆ (!(pci_config_readl(pci->bdf, bar) & PCI_ತಳ_ವಿಳಾಸ_IO_ಮುಖವಾಡ))
        bar = PCI_ತಳ_ವಿಳಾಸ_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    ಆದರೆ (!iobase)
        ಹಿಂತಿರುಗಿ;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    ಆದರೆ (megasas_transition_ಗಮ್ಯ_ready(pci, iobase) == 0)
        megasas_ಪರಿಶೀಲನೆ_ಗುರಿ(pci, iobase);
}

ಶೂನ್ಯ
megasas_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_MEGASAS)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init megasas\n");

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            ಮುಂದುವರಿ;
        ಆದರೆ (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
            ಸತತ_thread(init_megasas, pci);
    }
}
