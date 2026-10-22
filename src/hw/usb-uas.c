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

#அந்தர்பாவ "biosvar.h" // GET_GLOBALFLAT
#அந்தர்பாவ "block.h" // DTYPE_USB
#அந்தர்பாவ "blockcmd.h" // cdb_read
#அந்தர்பாவ "config.h" // CONFIG_USB_UAS
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "usb.h" // struct usb_s
#அந்தர்பாவ "usb-uas.h" // usb_uas_init
#அந்தர்பாவ "util.h" // bootprio_find_usb

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

வகைவரை கட்டமைப்பு {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

வகைவரை கட்டமைப்பு {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_length;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

வகைவரை கட்டமைப்பு {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_length;
    u8    sense_data[18];
} PACKED  uas_ui_sense;

வகைவரை கட்டமைப்பு {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

வகைவரை கட்டமைப்பு {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

வகைவரை கட்டமைப்பு {
    uas_ui_header  hdr;
    ஒன்றிப்பு {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

கட்டமைப்பு uasdrive_s {
    கட்டமைப்பு drive_s drive;
    கட்டமைப்பு usbdevice_s *usbdev;
    கட்டமைப்பு usb_pipe *command, *status, *data_in, *data_out;
    u32 lun;
};

முழு
uas_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_USB_UAS)
        திரும்பு DISK_RET_EBADTRACK;

    கட்டமைப்பு uasdrive_s *drive_gf = container_of(
        op->drive_fl, கட்டமைப்பு uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, அளவு(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    முழு blocksize = scsi_fill_cmd(op, ui.command.cdb, அளவு(ui.command.cdb));
    எனில் (blocksize < 0)
        திரும்பு default_process_op(op);
    முழு ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            அளவு(ui.hdr) + அளவு(ui.command));
    எனில் (ret) {
        dprintf(1, "uas: command send fail");
        செல் fail;
    }

    memset(&ui, 0xff, அளவு(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), அளவு(ui));
    எனில் (ret) {
        dprintf(1, "uas: status recv fail");
        செல் fail;
    }

    தேர்வு (ui.hdr.id) {
    நிலை UAS_UI_SENSE:
        செல் have_sense;
    நிலை UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_in),
                            USB_DIR_IN, op->buf_fl, op->மொத்தம் * blocksize);
        எனில் (ret) {
            dprintf(1, "uas: data read fail");
            செல் fail;
        }
        நிறுத்து;
    நிலை UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_out),
                            USB_DIR_OUT, op->buf_fl, op->மொத்தம் * blocksize);
        எனில் (ret) {
            dprintf(1, "uas: data write fail");
            செல் fail;
        }
        நிறுத்து;
    இயல்பு:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        செல் fail;
    }

    memset(&ui, 0xff, அளவு(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), அளவு(ui));
    எனில் (ret) {
        dprintf(1, "uas: status recv fail");
        செல் fail;
    }
    எனில் (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        செல் fail;
    }

have_sense:
    எனில் (ui.sense.status == 0) {
        திரும்பு DISK_RET_SUCCESS;
    }

fail:
    திரும்பு DISK_RET_EBADTRACK;
}

நிலையான வெறுமை
uas_init_lun(கட்டமைப்பு uasdrive_s *drive, கட்டமைப்பு usbdevice_s *usbdev,
             கட்டமைப்பு usb_pipe *command, கட்டமைப்பு usb_pipe *status,
             கட்டமைப்பு usb_pipe *data_in, கட்டமைப்பு usb_pipe *data_out,
             u32 lun)
{
    memset(drive, 0, அளவு(*drive));
    எனில் (usb_32bit_pipe(data_in))
        drive->drive.type = DTYPE_UAS_32;
    இல்லையெனில்
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->data_in = data_in;
    drive->data_out = data_out;
    drive->lun = lun;
}

நிலையான முழு
uas_add_lun(u32 lun, கட்டமைப்பு drive_s *tmpl_drv)
{
    கட்டமைப்பு uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, கட்டமைப்பு uasdrive_s, drive);
    கட்டமைப்பு uasdrive_s *drive = malloc_fseg(அளவு(*drive));
    எனில் (!drive) {
        warn_noalloc();
        திரும்பு -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->data_in, tmpl_lun->data_out,
                 lun);

    முழு prio = bootprio_find_usb(drive->usbdev, drive->lun);
    முழு ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    எனில் (ret) {
        விடுவி(drive);
        திரும்பு -1;
    }
    திரும்பு 0;
}

முழு
usb_uas_setup(கட்டமைப்பு usbdevice_s *usbdev)
{
    எனில் (!CONFIG_USB_UAS)
        திரும்பு -1;

    // Verify right kind of device
    கட்டமைப்பு usb_interface_descriptor *iface = usbdev->iface;
    எனில் (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        திரும்பு -1;
    }

    /* find & allocate pipes */
    கட்டமைப்பு usb_endpoint_descriptor *ep = வெற்று;
    கட்டமைப்பு usb_pipe *command = வெற்று;
    கட்டமைப்பு usb_pipe *status = வெற்று;
    கட்டமைப்பு usb_pipe *data_in = வெற்று;
    கட்டமைப்பு usb_pipe *data_out = வெற்று;
    u8 *desc = (u8*)iface;
    போது (desc) {
        desc += desc[0];
        தேர்வு (desc[1]) {
        நிலை USB_DT_ENDPOINT:
            ep = (வெறுமை*)desc;
            நிறுத்து;
        நிலை USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            செல் fail;
        நிலை 0x24:
            தேர்வு (desc[2]) {
            நிலை UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                நிறுத்து;
            நிலை UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                நிறுத்து;
            நிலை UAS_PIPE_ID_DATA_IN:
                data_in = usb_alloc_pipe(usbdev, ep);
                நிறுத்து;
            நிலை UAS_PIPE_ID_DATA_OUT:
                data_out = usb_alloc_pipe(usbdev, ep);
                நிறுத்து;
            இயல்பு:
                செல் fail;
            }
            நிறுத்து;
        இயல்பு:
            desc = வெற்று;
            நிறுத்து;
        }
    }
    எனில் (!command || !status || !data_in || !data_out)
        செல் fail;

    கட்டமைப்பு uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, data_in, data_out, 0);
    முழு ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    எனில் (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        செல் fail;
    }

    திரும்பு 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, data_in);
    usb_free_pipe(usbdev, data_out);
    திரும்பு -1;
}
