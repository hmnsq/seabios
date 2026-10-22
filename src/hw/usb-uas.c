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

#অন্তর্ভাব "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাব "block.h" // DTYPE_USB
#অন্তর্ভাব "blockcmd.h" // cdb_read
#অন্তর্ভাব "config.h" // CONFIG_USB_UAS
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "usb.h" // struct usb_s
#অন্তর্ভাব "usb-uas.h" // usb_uas_init
#অন্তর্ভাব "util.h" // bootprio_find_usb

#define UAS_UI_COMMAND              0x01
#define UAS_UI_SENSE                0x03
#define UAS_UI_RESPONSE             0x04
#define UAS_UI_TASK_MGMT            0x05
#define UAS_UI_READ_READY           0x06
#define UAS_UI_WRITE_READY          0x07

#define UAS_PIPE_ID_COMMAND         0x01
#define UAS_PIPE_ID_STATUS          0x02
#define UAS_PIPE_ID_তথ্য_IN         0x03
#define UAS_PIPE_ID_তথ্য_OUT        0x04

প্রকারলেখ কাঠামো {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

প্রকারলেখ কাঠামো {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_দৈর্ঘ্য;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

প্রকারলেখ কাঠামো {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_দৈর্ঘ্য;
    u8    sense_তথ্য[18];
} PACKED  uas_ui_sense;

প্রকারলেখ কাঠামো {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

প্রকারলেখ কাঠামো {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

প্রকারলেখ কাঠামো {
    uas_ui_header  hdr;
    সংঘ {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

কাঠামো uasdrive_s {
    কাঠামো drive_s drive;
    কাঠামো usbdevice_s *usbdev;
    কাঠামো usb_pipe *command, *status, *তথ্য_in, *তথ্য_out;
    u32 lun;
};

পূর্ণসংখ্যা
uas_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_USB_UAS)
        ফেরত DISK_RET_EBADTRACK;

    কাঠামো uasdrive_s *drive_gf = container_of(
        op->drive_fl, কাঠামো uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, আকার(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, ui.command.cdb, আকার(ui.command.cdb));
    যদি (blocksize < 0)
        ফেরত default_process_op(op);
    পূর্ণসংখ্যা ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            আকার(ui.hdr) + আকার(ui.command));
    যদি (ret) {
        dprintf(1, "uas: command send fail");
        যাও fail;
    }

    memset(&ui, 0xff, আকার(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), আকার(ui));
    যদি (ret) {
        dprintf(1, "uas: status recv fail");
        যাও fail;
    }

    নির্বাচন (ui.hdr.id) {
    ক্ষেত্র UAS_UI_SENSE:
        যাও have_sense;
    ক্ষেত্র UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->তথ্য_in),
                            USB_DIR_IN, op->buf_fl, op->গুনতি * blocksize);
        যদি (ret) {
            dprintf(1, "uas: data read fail");
            যাও fail;
        }
        বিরতি;
    ক্ষেত্র UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->তথ্য_out),
                            USB_DIR_OUT, op->buf_fl, op->গুনতি * blocksize);
        যদি (ret) {
            dprintf(1, "uas: data write fail");
            যাও fail;
        }
        বিরতি;
    পূর্বনির্ধারিত:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        যাও fail;
    }

    memset(&ui, 0xff, আকার(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), আকার(ui));
    যদি (ret) {
        dprintf(1, "uas: status recv fail");
        যাও fail;
    }
    যদি (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        যাও fail;
    }

have_sense:
    যদি (ui.sense.status == 0) {
        ফেরত DISK_RET_SUCCESS;
    }

fail:
    ফেরত DISK_RET_EBADTRACK;
}

স্থির শূন্য
uas_init_lun(কাঠামো uasdrive_s *drive, কাঠামো usbdevice_s *usbdev,
             কাঠামো usb_pipe *command, কাঠামো usb_pipe *status,
             কাঠামো usb_pipe *তথ্য_in, কাঠামো usb_pipe *তথ্য_out,
             u32 lun)
{
    memset(drive, 0, আকার(*drive));
    যদি (usb_32bit_pipe(তথ্য_in))
        drive->drive.type = DTYPE_UAS_32;
    অন্যথা
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->তথ্য_in = তথ্য_in;
    drive->তথ্য_out = তথ্য_out;
    drive->lun = lun;
}

স্থির পূর্ণসংখ্যা
uas_add_lun(u32 lun, কাঠামো drive_s *tmpl_drv)
{
    কাঠামো uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, কাঠামো uasdrive_s, drive);
    কাঠামো uasdrive_s *drive = malloc_fseg(আকার(*drive));
    যদি (!drive) {
        warn_noalloc();
        ফেরত -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->তথ্য_in, tmpl_lun->তথ্য_out,
                 lun);

    পূর্ণসংখ্যা prio = bootprio_find_usb(drive->usbdev, drive->lun);
    পূর্ণসংখ্যা ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    যদি (ret) {
        মুক্ত(drive);
        ফেরত -1;
    }
    ফেরত 0;
}

পূর্ণসংখ্যা
usb_uas_setup(কাঠামো usbdevice_s *usbdev)
{
    যদি (!CONFIG_USB_UAS)
        ফেরত -1;

    // Verify right kind of device
    কাঠামো usb_interface_descriptor *iface = usbdev->iface;
    যদি (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ফেরত -1;
    }

    /* find & allocate pipes */
    কাঠামো usb_endpoint_descriptor *ep = ফাঁকা;
    কাঠামো usb_pipe *command = ফাঁকা;
    কাঠামো usb_pipe *status = ফাঁকা;
    কাঠামো usb_pipe *তথ্য_in = ফাঁকা;
    কাঠামো usb_pipe *তথ্য_out = ফাঁকা;
    u8 *desc = (u8*)iface;
    যখন (desc) {
        desc += desc[0];
        নির্বাচন (desc[1]) {
        ক্ষেত্র USB_DT_ENDPOINT:
            ep = (শূন্য*)desc;
            বিরতি;
        ক্ষেত্র USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            যাও fail;
        ক্ষেত্র 0x24:
            নির্বাচন (desc[2]) {
            ক্ষেত্র UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                বিরতি;
            ক্ষেত্র UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                বিরতি;
            ক্ষেত্র UAS_PIPE_ID_তথ্য_IN:
                তথ্য_in = usb_alloc_pipe(usbdev, ep);
                বিরতি;
            ক্ষেত্র UAS_PIPE_ID_তথ্য_OUT:
                তথ্য_out = usb_alloc_pipe(usbdev, ep);
                বিরতি;
            পূর্বনির্ধারিত:
                যাও fail;
            }
            বিরতি;
        পূর্বনির্ধারিত:
            desc = ফাঁকা;
            বিরতি;
        }
    }
    যদি (!command || !status || !তথ্য_in || !তথ্য_out)
        যাও fail;

    কাঠামো uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, তথ্য_in, তথ্য_out, 0);
    পূর্ণসংখ্যা ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    যদি (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        যাও fail;
    }

    ফেরত 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, তথ্য_in);
    usb_free_pipe(usbdev, তথ্য_out);
    ফেরত -1;
}
