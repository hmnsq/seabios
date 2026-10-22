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

#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "block.h" // DTYPE_USB
#شامل "blockcmd.h" // cdb_read
#شامل "config.h" // CONFIG_USB_UAS
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-uas.h" // usb_uas_init
#شامل "util.h" // bootprio_find_usb

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

تعریفنوع ساختار {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

تعریفنوع ساختار {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_length;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

تعریفنوع ساختار {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_length;
    u8    sense_data[18];
} PACKED  uas_ui_sense;

تعریفنوع ساختار {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

تعریفنوع ساختار {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

تعریفنوع ساختار {
    uas_ui_header  hdr;
    اجتماع {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

ساختار uasdrive_s {
    ساختار drive_s drive;
    ساختار usbdevice_s *usbdev;
    ساختار usb_pipe *command, *status, *data_in, *data_out;
    u32 lun;
};

صحیح
uas_process_op(ساختار disk_op_s *op)
{
    اگر (!CONFIG_USB_UAS)
        بازگشت DISK_RET_EBADTRACK;

    ساختار uasdrive_s *drive_gf = container_of(
        op->drive_fl, ساختار uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, اندازه(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    صحیح blocksize = scsi_fill_cmd(op, ui.command.cdb, اندازه(ui.command.cdb));
    اگر (blocksize < 0)
        بازگشت default_process_op(op);
    صحیح ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            اندازه(ui.hdr) + اندازه(ui.command));
    اگر (ret) {
        dprintf(1, "uas: command send fail");
        برو fail;
    }

    memset(&ui, 0xff, اندازه(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), اندازه(ui));
    اگر (ret) {
        dprintf(1, "uas: status recv fail");
        برو fail;
    }

    انتخاب (ui.hdr.id) {
    حالت UAS_UI_SENSE:
        برو have_sense;
    حالت UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_in),
                            USB_DIR_IN, op->buf_fl, op->شمار * blocksize);
        اگر (ret) {
            dprintf(1, "uas: data read fail");
            برو fail;
        }
        بشکن;
    حالت UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_out),
                            USB_DIR_OUT, op->buf_fl, op->شمار * blocksize);
        اگر (ret) {
            dprintf(1, "uas: data write fail");
            برو fail;
        }
        بشکن;
    پیشفرض:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        برو fail;
    }

    memset(&ui, 0xff, اندازه(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), اندازه(ui));
    اگر (ret) {
        dprintf(1, "uas: status recv fail");
        برو fail;
    }
    اگر (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        برو fail;
    }

have_sense:
    اگر (ui.sense.status == 0) {
        بازگشت DISK_RET_SUCCESS;
    }

fail:
    بازگشت DISK_RET_EBADTRACK;
}

ایستا تهی
uas_init_lun(ساختار uasdrive_s *drive, ساختار usbdevice_s *usbdev,
             ساختار usb_pipe *command, ساختار usb_pipe *status,
             ساختار usb_pipe *data_in, ساختار usb_pipe *data_out,
             u32 lun)
{
    memset(drive, 0, اندازه(*drive));
    اگر (usb_32bit_pipe(data_in))
        drive->drive.type = DTYPE_UAS_32;
    وگرنه
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->data_in = data_in;
    drive->data_out = data_out;
    drive->lun = lun;
}

ایستا صحیح
uas_add_lun(u32 lun, ساختار drive_s *tmpl_drv)
{
    ساختار uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, ساختار uasdrive_s, drive);
    ساختار uasdrive_s *drive = malloc_fseg(اندازه(*drive));
    اگر (!drive) {
        warn_noalloc();
        بازگشت -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->data_in, tmpl_lun->data_out,
                 lun);

    صحیح prio = bootprio_find_usb(drive->usbdev, drive->lun);
    صحیح ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    اگر (ret) {
        آزاد(drive);
        بازگشت -1;
    }
    بازگشت 0;
}

صحیح
usb_uas_setup(ساختار usbdevice_s *usbdev)
{
    اگر (!CONFIG_USB_UAS)
        بازگشت -1;

    // Verify right kind of device
    ساختار usb_interface_descriptor *iface = usbdev->iface;
    اگر (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        بازگشت -1;
    }

    /* find & allocate pipes */
    ساختار usb_endpoint_descriptor *ep = پوچ;
    ساختار usb_pipe *command = پوچ;
    ساختار usb_pipe *status = پوچ;
    ساختار usb_pipe *data_in = پوچ;
    ساختار usb_pipe *data_out = پوچ;
    u8 *desc = (u8*)iface;
    وقتی (desc) {
        desc += desc[0];
        انتخاب (desc[1]) {
        حالت USB_DT_ENDPOINT:
            ep = (تهی*)desc;
            بشکن;
        حالت USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            برو fail;
        حالت 0x24:
            انتخاب (desc[2]) {
            حالت UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                بشکن;
            حالت UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                بشکن;
            حالت UAS_PIPE_ID_DATA_IN:
                data_in = usb_alloc_pipe(usbdev, ep);
                بشکن;
            حالت UAS_PIPE_ID_DATA_OUT:
                data_out = usb_alloc_pipe(usbdev, ep);
                بشکن;
            پیشفرض:
                برو fail;
            }
            بشکن;
        پیشفرض:
            desc = پوچ;
            بشکن;
        }
    }
    اگر (!command || !status || !data_in || !data_out)
        برو fail;

    ساختار uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, data_in, data_out, 0);
    صحیح ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    اگر (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        برو fail;
    }

    بازگشت 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, data_in);
    usb_free_pipe(usbdev, data_out);
    بازگشت -1;
}
