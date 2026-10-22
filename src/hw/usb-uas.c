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

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // DTYPE_USB
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // cdb_read
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_USB_UAS
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-uas.h" // usb_uas_init
#ಅನ್ತರ್ಭಾವ "util.h" // bootprio_find_usb

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

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_length;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_length;
    u8    sense_data[18];
} PACKED  uas_ui_sense;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
    uas_ui_header  hdr;
    ಸಙ್ಘ {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

ಸಂರಚನಾ uasdrive_s {
    ಸಂರಚನಾ drive_s drive;
    ಸಂರಚನಾ usbdevice_s *usbdev;
    ಸಂರಚನಾ usb_pipe *command, *status, *data_in, *data_out;
    u32 lun;
};

ಪೂರ್ಣಾಙ್ಕ
uas_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_USB_UAS)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;

    ಸಂರಚನಾ uasdrive_s *drive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, ಪರಿಮಾಣ(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    ಪೂರ್ಣಾಙ್ಕ blocksize = scsi_fill_cmd(op, ui.command.cdb, ಪರಿಮಾಣ(ui.command.cdb));
    ಯದಿ (blocksize < 0)
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    ಪೂರ್ಣಾಙ್ಕ ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            ಪರಿಮಾಣ(ui.hdr) + ಪರಿಮಾಣ(ui.command));
    ಯದಿ (ret) {
        dprintf(1, "uas: command send fail");
        ಗಚ್ಛ fail;
    }

    memset(&ui, 0xff, ಪರಿಮಾಣ(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), ಪರಿಮಾಣ(ui));
    ಯದಿ (ret) {
        dprintf(1, "uas: status recv fail");
        ಗಚ್ಛ fail;
    }

    ವಿಕಲ್ಪ (ui.hdr.id) {
    ಪಕ್ಷ UAS_UI_SENSE:
        ಗಚ್ಛ have_sense;
    ಪಕ್ಷ UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_in),
                            USB_DIR_IN, op->buf_fl, op->ಎಣಿಕೆ * blocksize);
        ಯದಿ (ret) {
            dprintf(1, "uas: data read fail");
            ಗಚ್ಛ fail;
        }
        ಭಙ್ಗ;
    ಪಕ್ಷ UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_out),
                            USB_DIR_OUT, op->buf_fl, op->ಎಣಿಕೆ * blocksize);
        ಯದಿ (ret) {
            dprintf(1, "uas: data write fail");
            ಗಚ್ಛ fail;
        }
        ಭಙ್ಗ;
    ಸ್ವಭಾವ:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        ಗಚ್ಛ fail;
    }

    memset(&ui, 0xff, ಪರಿಮಾಣ(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), ಪರಿಮಾಣ(ui));
    ಯದಿ (ret) {
        dprintf(1, "uas: status recv fail");
        ಗಚ್ಛ fail;
    }
    ಯದಿ (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        ಗಚ್ಛ fail;
    }

have_sense:
    ಯದಿ (ui.sense.status == 0) {
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
    }

fail:
    ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಶೂನ್ಯ
uas_init_lun(ಸಂರಚನಾ uasdrive_s *drive, ಸಂರಚನಾ usbdevice_s *usbdev,
             ಸಂರಚನಾ usb_pipe *command, ಸಂರಚನಾ usb_pipe *status,
             ಸಂರಚನಾ usb_pipe *data_in, ಸಂರಚನಾ usb_pipe *data_out,
             u32 lun)
{
    memset(drive, 0, ಪರಿಮಾಣ(*drive));
    ಯದಿ (usb_32bit_pipe(data_in))
        drive->drive.type = DTYPE_UAS_32;
    ಅನ್ಯಥಾ
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->data_in = data_in;
    drive->data_out = data_out;
    drive->lun = lun;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
uas_add_lun(u32 lun, ಸಂರಚನಾ drive_s *tmpl_drv)
{
    ಸಂರಚನಾ uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, ಸಂರಚನಾ uasdrive_s, drive);
    ಸಂರಚನಾ uasdrive_s *drive = malloc_fseg(ಪರಿಮಾಣ(*drive));
    ಯದಿ (!drive) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->data_in, tmpl_lun->data_out,
                 lun);

    ಪೂರ್ಣಾಙ್ಕ prio = bootprio_find_usb(drive->usbdev, drive->lun);
    ಪೂರ್ಣಾಙ್ಕ ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    ಯದಿ (ret) {
        ಮುಕ್ತ(drive);
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ
usb_uas_setup(ಸಂರಚನಾ usbdevice_s *usbdev)
{
    ಯದಿ (!CONFIG_USB_UAS)
        ಪ್ರತ್ಯಾಗಮ -1;

    // Verify right kind of device
    ಸಂರಚನಾ usb_interface_descriptor *iface = usbdev->iface;
    ಯದಿ (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    /* find & allocate pipes */
    ಸಂರಚನಾ usb_endpoint_descriptor *ep = ರಿಕ್ತ;
    ಸಂರಚನಾ usb_pipe *command = ರಿಕ್ತ;
    ಸಂರಚನಾ usb_pipe *status = ರಿಕ್ತ;
    ಸಂರಚನಾ usb_pipe *data_in = ರಿಕ್ತ;
    ಸಂರಚನಾ usb_pipe *data_out = ರಿಕ್ತ;
    u8 *desc = (u8*)iface;
    ಯಾವತ್ (desc) {
        desc += desc[0];
        ವಿಕಲ್ಪ (desc[1]) {
        ಪಕ್ಷ USB_DT_ENDPOINT:
            ep = (ಶೂನ್ಯ*)desc;
            ಭಙ್ಗ;
        ಪಕ್ಷ USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            ಗಚ್ಛ fail;
        ಪಕ್ಷ 0x24:
            ವಿಕಲ್ಪ (desc[2]) {
            ಪಕ್ಷ UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                ಭಙ್ಗ;
            ಪಕ್ಷ UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                ಭಙ್ಗ;
            ಪಕ್ಷ UAS_PIPE_ID_DATA_IN:
                data_in = usb_alloc_pipe(usbdev, ep);
                ಭಙ್ಗ;
            ಪಕ್ಷ UAS_PIPE_ID_DATA_OUT:
                data_out = usb_alloc_pipe(usbdev, ep);
                ಭಙ್ಗ;
            ಸ್ವಭಾವ:
                ಗಚ್ಛ fail;
            }
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            desc = ರಿಕ್ತ;
            ಭಙ್ಗ;
        }
    }
    ಯದಿ (!command || !status || !data_in || !data_out)
        ಗಚ್ಛ fail;

    ಸಂರಚನಾ uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, data_in, data_out, 0);
    ಪೂರ್ಣಾಙ್ಕ ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    ಯದಿ (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        ಗಚ್ಛ fail;
    }

    ಪ್ರತ್ಯಾಗಮ 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, data_in);
    usb_free_pipe(usbdev, data_out);
    ಪ್ರತ್ಯಾಗಮ -1;
}
