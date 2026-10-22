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

#ضمن "biosvar.h" // GET_GLOBALFLAT
#ضمن "block.h" // DTYPE_USB
#ضمن "blockcmd.h" // cdb_read
#ضمن "config.h" // CONFIG_USB_UAS
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-uas.h" // usb_uas_init
#ضمن "util.h" // bootprio_find_usb

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

تعريفنوع بنية {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

تعريفنوع بنية {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_length;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

تعريفنوع بنية {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_length;
    u8    sense_data[18];
} PACKED  uas_ui_sense;

تعريفنوع بنية {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

تعريفنوع بنية {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

تعريفنوع بنية {
    uas_ui_header  hdr;
    اتحاد {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

بنية uasdrive_s {
    بنية drive_s drive;
    بنية usbdevice_s *usbdev;
    بنية usb_pipe *command, *status, *data_in, *data_out;
    u32 lun;
};

صحيح
uas_process_op(بنية disk_op_s *op)
{
    ܐܢ (!CONFIG_USB_UAS)
        ܦܢܐ DISK_RET_EBADTRACK;

    بنية uasdrive_s *drive_gf = container_of(
        op->drive_fl, بنية uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, حجم(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    صحيح blocksize = scsi_fill_cmd(op, ui.command.cdb, حجم(ui.command.cdb));
    ܐܢ (blocksize < 0)
        ܦܢܐ default_process_op(op);
    صحيح ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            حجم(ui.hdr) + حجم(ui.command));
    ܐܢ (ret) {
        dprintf(1, "uas: command send fail");
        اذهب fail;
    }

    memset(&ui, 0xff, حجم(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), حجم(ui));
    ܐܢ (ret) {
        dprintf(1, "uas: status recv fail");
        اذهب fail;
    }

    بدل (ui.hdr.id) {
    حالة UAS_UI_SENSE:
        اذهب have_sense;
    حالة UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_in),
                            USB_DIR_IN, op->buf_fl, op->count * blocksize);
        ܐܢ (ret) {
            dprintf(1, "uas: data read fail");
            اذهب fail;
        }
        اكسر;
    حالة UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_out),
                            USB_DIR_OUT, op->buf_fl, op->count * blocksize);
        ܐܢ (ret) {
            dprintf(1, "uas: data write fail");
            اذهب fail;
        }
        اكسر;
    افتراضي:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        اذهب fail;
    }

    memset(&ui, 0xff, حجم(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), حجم(ui));
    ܐܢ (ret) {
        dprintf(1, "uas: status recv fail");
        اذهب fail;
    }
    ܐܢ (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        اذهب fail;
    }

have_sense:
    ܐܢ (ui.sense.status == 0) {
        ܦܢܐ DISK_RET_SUCCESS;
    }

fail:
    ܦܢܐ DISK_RET_EBADTRACK;
}

ساكن فراغ
uas_init_lun(بنية uasdrive_s *drive, بنية usbdevice_s *usbdev,
             بنية usb_pipe *command, بنية usb_pipe *status,
             بنية usb_pipe *data_in, بنية usb_pipe *data_out,
             u32 lun)
{
    memset(drive, 0, حجم(*drive));
    ܐܢ (usb_32bit_pipe(data_in))
        drive->drive.type = DTYPE_UAS_32;
    ܐܚܪܢܐ
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->data_in = data_in;
    drive->data_out = data_out;
    drive->lun = lun;
}

ساكن صحيح
uas_add_lun(u32 lun, بنية drive_s *tmpl_drv)
{
    بنية uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, بنية uasdrive_s, drive);
    بنية uasdrive_s *drive = malloc_fseg(حجم(*drive));
    ܐܢ (!drive) {
        warn_noalloc();
        ܦܢܐ -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->data_in, tmpl_lun->data_out,
                 lun);

    صحيح prio = bootprio_find_usb(drive->usbdev, drive->lun);
    صحيح ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    ܐܢ (ret) {
        حرر(drive);
        ܦܢܐ -1;
    }
    ܦܢܐ 0;
}

صحيح
usb_uas_setup(بنية usbdevice_s *usbdev)
{
    ܐܢ (!CONFIG_USB_UAS)
        ܦܢܐ -1;

    // Verify right kind of device
    بنية usb_interface_descriptor *iface = usbdev->iface;
    ܐܢ (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ܦܢܐ -1;
    }

    /* find & allocate pipes */
    بنية usb_endpoint_descriptor *ep = عدم;
    بنية usb_pipe *command = عدم;
    بنية usb_pipe *status = عدم;
    بنية usb_pipe *data_in = عدم;
    بنية usb_pipe *data_out = عدم;
    u8 *desc = (u8*)iface;
    ܟܡܐ (desc) {
        desc += desc[0];
        بدل (desc[1]) {
        حالة USB_DT_ENDPOINT:
            ep = (فراغ*)desc;
            اكسر;
        حالة USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            اذهب fail;
        حالة 0x24:
            بدل (desc[2]) {
            حالة UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                اكسر;
            حالة UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                اكسر;
            حالة UAS_PIPE_ID_DATA_IN:
                data_in = usb_alloc_pipe(usbdev, ep);
                اكسر;
            حالة UAS_PIPE_ID_DATA_OUT:
                data_out = usb_alloc_pipe(usbdev, ep);
                اكسر;
            افتراضي:
                اذهب fail;
            }
            اكسر;
        افتراضي:
            desc = عدم;
            اكسر;
        }
    }
    ܐܢ (!command || !status || !data_in || !data_out)
        اذهب fail;

    بنية uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, data_in, data_out, 0);
    صحيح ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    ܐܢ (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        اذهب fail;
    }

    ܦܢܐ 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, data_in);
    usb_free_pipe(usbdev, data_out);
    ܦܢܐ -1;
}
