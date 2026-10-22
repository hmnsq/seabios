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

#include "biosvar.h" // GET_GLOBALFLAT
#include "block.h" // struct drive_s
#include "blockcmd.h" // scsi_drive_setup
#include "config.h" // CONFIG_*
#include "malloc.h" // free
#include "output.h" // dprintf
#include "pci.h" // pci_config_readl
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_DEVICE_ID_XXX
#include "pci_regs.h" // PCI_VENDOR_ID
#include "stacks.h" // yield
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // timer_calc

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
typedef ga_nanaa {
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

sa_nracanaa megasas_cmd_frame {
    u8 cmd;             /*00h */
    u8 sense_len;       /*01h */
    u8 cmd_status;      /*02h */
    u8 scsi_status;     /*03h */

    u8 target_id;       /*04h */
    u8 lun;             /*05h */
    u8 cdb_len;         /*06h */
    u8 sge_count;       /*07h */

    u32 context;        /*08h */
    u32 context_64;     /*0Ch */

    u16 flags;          /*10h */
    u16 timeout;        /*12h */
    u32 data_xfer_len;   /*14h */

    sa_ngha {
        sa_nracanaa {
            u32 opcode;       /*18h */
            u8 mbox[12];      /*1Ch */
            u32 sgl_addr;     /*28h */
            u32 sgl_len;      /*32h */
            u32 pad;          /*34h */
        } dcmd;
        sa_nracanaa {
            u32 sense_buf_lo; /*18h */
            u32 sense_buf_hi; /*1Ch */
            u8 cdb[16];       /*20h */
            u32 sgl_addr;     /*30h */
            u32 sgl_len;      /*34h */
        } pthru;
        sa_nracanaa {
            u8 pad[22];       /*18h */
        } gen;
    };
} __attribute__ ((packed));

sa_nracanaa mfi_ld_list_s {
    u32     count;
    u32     reserved_0;
    sa_nracanaa {
        u8          target;
        u8          lun;
        u16         seq;
        u8          state;
        u8          reserved_1[3];
        u64         size;
    } lds[64];
} __attribute__ ((packed));

#define MEGASAS_POLL_TIMEOUT 60000 // 60 seconds polling timeout

sa_nracanaa megasas_lun_s {
    sa_nracanaa drive_s drive;
    sa_nracanaa megasas_cmd_frame *frame;
    u32 iobase;
    u16 pci_id;
    u8 target;
    u8 lun;
};

static ཧྲིལ megasas_fire_cmd(u16 pci_id, u32 ioaddr,
                            sa_nracanaa megasas_cmd_frame *frame)
{
    u32 frame_addr = (u32)frame;
    ཧྲིལ frame_count = 1;
    u8 cmd_state;

    dprintf(2, "Frame 0x%x\n", frame_addr);
    གལ་སྲིད (pci_id == PCI_DEVICE_ID_LSI_SAS2004 ||
        pci_id == PCI_DEVICE_ID_LSI_SAS2008) {
        outl(0, ioaddr + MFI_IQPH);
        outl(frame_addr | frame_count << 1 | 1, ioaddr + MFI_IQPL);
    } དེ་མིན གལ་སྲིད (pci_id == PCI_DEVICE_ID_DELL_PERC5 ||
               pci_id == PCI_DEVICE_ID_LSI_SAS1064R ||
               pci_id == PCI_DEVICE_ID_LSI_VERDE_ZCR) {
        outl(frame_addr >> 3 | frame_count, ioaddr + MFI_IQP);
    } དེ་མིན {
        outl(frame_addr | frame_count << 1 | 1, ioaddr + MFI_IQP);
    }

    u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
    do {
        དོན་ལུ (;;) {
            cmd_state = GET_LOWFLAT(frame->cmd_status);
            གལ་སྲིད (cmd_state != 0xff)
                bha_nga;
            གལ་སྲིད (timer_check(end)) {
                warn_timeout();
                ལོག -1;
            }
            yield();
        }
    } སྐབས (cmd_state == 0xff);

    གལ་སྲིད (cmd_state == 0 || cmd_state == 0x2d)
        ལོག 0;
    dprintf(1, "ERROR: Frame 0x%x, status 0x%x\n", frame_addr, cmd_state);
    ལོག -1;
}

ཧྲིལ
megasas_process_op(sa_nracanaa disk_op_s *op)
{
    གལ་སྲིད (!CONFIG_MEGASAS)
        ལོག DISK_RET_EBADTRACK;
    u8 cdb[16];
    ཧྲིལ blocksize = scsi_fill_cmd(op, cdb, parimaa_na(cdb));
    གལ་སྲིད (blocksize < 0)
        ལོག default_process_op(op);
    sa_nracanaa megasas_lun_s *mlun_gf =
        container_of(op->drive_fl, sa_nracanaa megasas_lun_s, drive);
    sa_nracanaa megasas_cmd_frame *frame = GET_GLOBALFLAT(mlun_gf->frame);
    u16 pci_id = GET_GLOBALFLAT(mlun_gf->pci_id);
    ཧྲིལ i;

    memset_fl(frame, 0, parimaa_na(*frame));
    SET_LOWFLAT(frame->cmd, MFI_CMD_LD_SCSI_IO);
    SET_LOWFLAT(frame->cmd_status, 0xFF);
    SET_LOWFLAT(frame->target_id, GET_GLOBALFLAT(mlun_gf->target));
    SET_LOWFLAT(frame->lun, GET_GLOBALFLAT(mlun_gf->lun));
    SET_LOWFLAT(frame->flags, 0x0001);
    SET_LOWFLAT(frame->data_xfer_len, op->count * blocksize);
    SET_LOWFLAT(frame->cdb_len, 16);

    དོན་ལུ (i = 0; i < 16; i++) {
        SET_LOWFLAT(frame->pthru.cdb[i], cdb[i]);
    }
    dprintf(2, "pthru cmd 0x%x count %d bs %d\n",
            cdb[0], op->count, blocksize);

    གལ་སྲིད (op->count) {
        SET_LOWFLAT(frame->pthru.sgl_addr, (u32)op->buf_fl);
        SET_LOWFLAT(frame->pthru.sgl_len, op->count * blocksize);
        SET_LOWFLAT(frame->sge_count, 1);
    }
    SET_LOWFLAT(frame->context, (u32)frame);

    གལ་སྲིད (megasas_fire_cmd(pci_id, GET_GLOBALFLAT(mlun_gf->iobase), frame) == 0)
        ལོག DISK_RET_SUCCESS;

    dprintf(2, "pthru cmd 0x%x failed\n", cdb[0]);
    ལོག DISK_RET_EBADTRACK;
}

static ཧྲིལ
megasas_add_lun(sa_nracanaa pci_device *pci, u32 iobase, u8 target, u8 lun)
{
    sa_nracanaa megasas_lun_s *mlun = malloc_fseg(parimaa_na(*mlun));
    var_na *name;
    ཧྲིལ prio, ret = 0;

    གལ་སྲིད (!mlun) {
        warn_noalloc();
        ལོག -1;
    }
    memset(mlun, 0, parimaa_na(*mlun));
    mlun->drive.type = DTYPE_MEGASAS;
    mlun->drive.cntl_id = pci->bdf;
    mlun->pci_id = pci->device;
    mlun->target = target;
    mlun->lun = lun;
    mlun->iobase = iobase;
    mlun->frame = memalign_low(256, parimaa_na(sa_nracanaa megasas_cmd_frame));
    གལ་སྲིད (!mlun->frame) {
        warn_noalloc();
        free(mlun);
        ལོག -1;
    }
    boot_lchs_find_scsi_device(pci, target, lun, &(mlun->drive.lchs));
    name = znprintf(MAXDESCSIZE, "MegaRAID SAS (PCI %pP) LD %d:%d"
                    , pci, target, lun);
    prio = bootprio_find_scsi_device(pci, target, lun);
    ret = scsi_drive_setup(&mlun->drive, name, prio);
    free(name);
    གལ་སྲིད (ret) {
        free(mlun->frame);
        free(mlun);
        ret = -1;
    }

    ལོག ret;
}

static void megasas_scan_target(sa_nracanaa pci_device *pci, u32 iobase)
{
    sa_nracanaa mfi_ld_list_s ld_list;
    sa_nracanaa megasas_cmd_frame *frame = memalign_tmp(256, parimaa_na(*frame));
    གལ་སྲིད (!frame) {
        warn_noalloc();
        ལོག;
    }

    memset(&ld_list, 0, parimaa_na(ld_list));
    memset_fl(frame, 0, parimaa_na(*frame));

    frame->cmd = MFI_CMD_DCMD;
    frame->cmd_status = 0xFF;
    frame->sge_count = 1;
    frame->flags = 0x0011;
    frame->data_xfer_len = parimaa_na(ld_list);
    frame->dcmd.opcode = 0x03010000;
    frame->dcmd.sgl_addr = (u32)MAKE_FLATPTR(GET_SEG(SS), &ld_list);
    frame->dcmd.sgl_len = parimaa_na(ld_list);
    frame->context = (u32)frame;

    གལ་སྲིད (megasas_fire_cmd(pci->device, iobase, frame) == 0) {
        dprintf(2, "%d LD found\n", ld_list.count);
        ཧྲིལ i;
        དོན་ལུ (i = 0; i < ld_list.count; i++) {
            dprintf(2, "LD %d:%d state 0x%x\n",
                    ld_list.lds[i].target, ld_list.lds[i].lun,
                    ld_list.lds[i].state);
            གལ་སྲིད (ld_list.lds[i].state != 0) {
                megasas_add_lun(pci, iobase,
                                ld_list.lds[i].target, ld_list.lds[i].lun);
            }
        }
    }
}

static ཧྲིལ megasas_transition_to_ready(sa_nracanaa pci_device *pci, u32 ioaddr)
{
    u32 fw_state = 0, new_state, mfi_flags = 0;

    གལ་སྲིད (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
        pci->device == PCI_DEVICE_ID_DELL_PERC5)
        new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
    དེ་མིན
        new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;

    སྐབས (fw_state != new_state) {
        switch (new_state) {
        pak_sa MFI_STATE_FAULT:
            dprintf(1, "ERROR: fw in fault state\n");
            ལོག -1;
            bha_nga;
        pak_sa MFI_STATE_WAIT_HANDSHAKE:
            mfi_flags = 0x08;
            /* fallthrough */
        pak_sa MFI_STATE_BOOT_MESSAGE_PENDING:
            mfi_flags |= 0x10;
            གལ་སྲིད (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
            } དེ་མིན {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            bha_nga;
        pak_sa MFI_STATE_OPERATIONAL:
            mfi_flags = 0x07;
            གལ་སྲིད (pci->device == PCI_DEVICE_ID_LSI_SAS2004 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2008 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                outl(mfi_flags, ioaddr + MFI_DB);
                གལ་སྲིད (pci->device == PCI_DEVICE_ID_LSI_SAS2208 ||
                    pci->device == PCI_DEVICE_ID_LSI_SAS3108) {
                    ཧྲིལ j = 0;
                    u32 doorbell;

                    སྐབས (j < MEGASAS_POLL_TIMEOUT) {
                        doorbell = inl(ioaddr + MFI_DB) & 1;
                        གལ་སྲིད (!doorbell)
                            bha_nga;
                        msleep(20);
                        j++;
                    }
                }
            } དེ་མིན {
                outl(mfi_flags, ioaddr + MFI_IDB);
            }
            bha_nga;
        pak_sa MFI_STATE_READY:
            dprintf(2, "MegaRAID SAS fw ready\n");
            ལོག 0;
        }
        // The current state should not last longer than poll timeout
        u32 end = timer_calc(MEGASAS_POLL_TIMEOUT);
        དོན་ལུ (;;) {
            གལ་སྲིད (timer_check(end)) {
                bha_nga;
            }
            yield();
            fw_state = new_state;
            གལ་སྲིད (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
                pci->device == PCI_DEVICE_ID_DELL_PERC5)
                new_state = inl(ioaddr + MFI_OMSG0) & MFI_STATE_MASK;
            དེ་མིན
                new_state = inl(ioaddr + MFI_OSP0) & MFI_STATE_MASK;
            གལ་སྲིད (new_state != fw_state) {
                bha_nga;
            }
        }
    }
    dprintf(1, "ERROR: fw in state %x\n", new_state & MFI_STATE_MASK);
    ལོག -1;
}

static void
init_megasas(void *data)
{
    sa_nracanaa pci_device *pci = data;
    u32 bar = PCI_BASE_ADDRESS_2;
    གལ་སྲིད (!(pci_config_readl(pci->bdf, bar) & PCI_BASE_ADDRESS_IO_MASK))
        bar = PCI_BASE_ADDRESS_0;
    u32 iobase = pci_enable_iobar(pci, bar);
    གལ་སྲིད (!iobase)
        ལོག;
    pci_enable_busmaster(pci);

    dprintf(1, "found MegaRAID SAS at %pP, io @ %x\n", pci, iobase);

    // reset
    གལ་སྲིད (megasas_transition_to_ready(pci, iobase) == 0)
        megasas_scan_target(pci, iobase);
}

void
megasas_setup(void)
{
    ASSERT32FLAT();
    གལ་སྲིད (!CONFIG_MEGASAS)
        ལོག;

    dprintf(3, "init megasas\n");

    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        གལ་སྲིད (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC &&
            pci->vendor != PCI_VENDOR_ID_DELL)
            continue;
        གལ་སྲིད (pci->device == PCI_DEVICE_ID_LSI_SAS1064R ||
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
