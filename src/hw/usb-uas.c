// Code for handling usb attached scsi devices.
//
// only usb 2.0 for now.
//
// once we have xhci driver with usb 3.0 support this must
// be updated to use usb3 streams so booting from usb3
// devices actually works.
//
// Authors:
//  Gerd Hoffmann <kraxel@redhat.com>
//
// based on usb-msc.c which is written by:
//  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBALFLAT
#include "block.h" // DTYPE_USB
#include "blockcmd.h" // cdb_read
#include "config.h" // CONFIG_USB_UAS
#include "malloc.h" // free
#include "output.h" // dprintf
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "usb.h" // struct usb_s
#include "usb-uas.h" // usb_uas_init
#include "util.h" // bootprio_find_usb

#define UAS_UI_COMMAND              0x01
#define UAS_UI_SENSE                0x03
#define UAS_UI_RESPONSE             0x04
#define UAS_UI_TASK_MGMT            0x05
#define UAS_UI_READ_READY           0x06
#define UAS_UI_WRITE_READY          0x07

#define UAS_PIPE_ID_COMMAND         0x01
#define UAS_PIPE_ID_STATUS          0x02
#define UAS_PIPE_ID_DATA_IN         0x03
#define UAS_PIPE_ID_DATA_OUT        0x04

typedef sa_nracanaa {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

typedef sa_nracanaa {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_length;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

typedef sa_nracanaa {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_length;
    u8    sense_data[18];
} PACKED  uas_ui_sense;

typedef sa_nracanaa {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

typedef sa_nracanaa {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

typedef sa_nracanaa {
    uas_ui_header  hdr;
    sa_ngha {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

sa_nracanaa uasdrive_s {
    sa_nracanaa drive_s drive;
    sa_nracanaa usbdevice_s *usbdev;
    sa_nracanaa usb_pipe *command, *status, *data_in, *data_out;
    u32 lun;
};

ຈຳນວນເຕັມ
uas_process_op(sa_nracanaa disk_op_s *op)
{
    ຖ້າ (!CONFIG_USB_UAS)
        ສົ່ງຄືນ DISK_RET_EBADTRACK;

    sa_nracanaa uasdrive_s *drive_gf = container_of(
        op->drive_fl, sa_nracanaa uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, parimaa_na(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    ຈຳນວນເຕັມ blocksize = scsi_fill_cmd(op, ui.command.cdb, parimaa_na(ui.command.cdb));
    ຖ້າ (blocksize < 0)
        ສົ່ງຄືນ default_process_op(op);
    ຈຳນວນເຕັມ ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            parimaa_na(ui.hdr) + parimaa_na(ui.command));
    ຖ້າ (ret) {
        dprintf(1, "uas: command send fail");
        goto fail;
    }

    memset(&ui, 0xff, parimaa_na(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), parimaa_na(ui));
    ຖ້າ (ret) {
        dprintf(1, "uas: status recv fail");
        goto fail;
    }

    switch (ui.hdr.id) {
    pak_sa UAS_UI_SENSE:
        goto have_sense;
    pak_sa UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_in),
                            USB_DIR_IN, op->buf_fl, op->count * blocksize);
        ຖ້າ (ret) {
            dprintf(1, "uas: data read fail");
            goto fail;
        }
        bha_nga;
    pak_sa UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_out),
                            USB_DIR_OUT, op->buf_fl, op->count * blocksize);
        ຖ້າ (ret) {
            dprintf(1, "uas: data write fail");
            goto fail;
        }
        bha_nga;
    default:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        goto fail;
    }

    memset(&ui, 0xff, parimaa_na(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), parimaa_na(ui));
    ຖ້າ (ret) {
        dprintf(1, "uas: status recv fail");
        goto fail;
    }
    ຖ້າ (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        goto fail;
    }

have_sense:
    ຖ້າ (ui.sense.status == 0) {
        ສົ່ງຄືນ DISK_RET_SUCCESS;
    }

fail:
    ສົ່ງຄືນ DISK_RET_EBADTRACK;
}

static void
uas_init_lun(sa_nracanaa uasdrive_s *drive, sa_nracanaa usbdevice_s *usbdev,
             sa_nracanaa usb_pipe *command, sa_nracanaa usb_pipe *status,
             sa_nracanaa usb_pipe *data_in, sa_nracanaa usb_pipe *data_out,
             u32 lun)
{
    memset(drive, 0, parimaa_na(*drive));
    ຖ້າ (usb_32bit_pipe(data_in))
        drive->drive.type = DTYPE_UAS_32;
    ບໍ່ດັ່ງນັ້ນ
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->data_in = data_in;
    drive->data_out = data_out;
    drive->lun = lun;
}

static ຈຳນວນເຕັມ
uas_add_lun(u32 lun, sa_nracanaa drive_s *tmpl_drv)
{
    sa_nracanaa uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, sa_nracanaa uasdrive_s, drive);
    sa_nracanaa uasdrive_s *drive = malloc_fseg(parimaa_na(*drive));
    ຖ້າ (!drive) {
        warn_noalloc();
        ສົ່ງຄືນ -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->data_in, tmpl_lun->data_out,
                 lun);

    ຈຳນວນເຕັມ prio = bootprio_find_usb(drive->usbdev, drive->lun);
    ຈຳນວນເຕັມ ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    ຖ້າ (ret) {
        free(drive);
        ສົ່ງຄືນ -1;
    }
    ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ
usb_uas_setup(sa_nracanaa usbdevice_s *usbdev)
{
    ຖ້າ (!CONFIG_USB_UAS)
        ສົ່ງຄືນ -1;

    // Verify right kind of device
    sa_nracanaa usb_interface_descriptor *iface = usbdev->iface;
    ຖ້າ (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ສົ່ງຄືນ -1;
    }

    /* find & allocate pipes */
    sa_nracanaa usb_endpoint_descriptor *ep = NULL;
    sa_nracanaa usb_pipe *command = NULL;
    sa_nracanaa usb_pipe *status = NULL;
    sa_nracanaa usb_pipe *data_in = NULL;
    sa_nracanaa usb_pipe *data_out = NULL;
    u8 *desc = (u8*)iface;
    ໃນຂະນະທີ່ (desc) {
        desc += desc[0];
        switch (desc[1]) {
        pak_sa USB_DT_ENDPOINT:
            ep = (void*)desc;
            bha_nga;
        pak_sa USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            goto fail;
        pak_sa 0x24:
            switch (desc[2]) {
            pak_sa UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                bha_nga;
            pak_sa UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                bha_nga;
            pak_sa UAS_PIPE_ID_DATA_IN:
                data_in = usb_alloc_pipe(usbdev, ep);
                bha_nga;
            pak_sa UAS_PIPE_ID_DATA_OUT:
                data_out = usb_alloc_pipe(usbdev, ep);
                bha_nga;
            default:
                goto fail;
            }
            bha_nga;
        default:
            desc = NULL;
            bha_nga;
        }
    }
    ຖ້າ (!command || !status || !data_in || !data_out)
        goto fail;

    sa_nracanaa uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, data_in, data_out, 0);
    ຈຳນວນເຕັມ ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    ຖ້າ (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        goto fail;
    }

    ສົ່ງຄືນ 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, data_in);
    usb_free_pipe(usbdev, data_out);
    ສົ່ງຄືນ -1;
}
