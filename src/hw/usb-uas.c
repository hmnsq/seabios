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

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBALFLAT
#അന്തര്ഭാവ "block.h" // DTYPE_USB
#അന്തര്ഭാവ "blockcmd.h" // cdb_read
#അന്തര്ഭാവ "config.h" // CONFIG_USB_UAS
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "usb.h" // struct usb_s
#അന്തര്ഭാവ "usb-uas.h" // usb_uas_init
#അന്തര്ഭാവ "util.h" // bootprio_find_usb

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

തരനിർവചനം ഘടന {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

തരനിർവചനം ഘടന {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_length;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

തരനിർവചനം ഘടന {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_length;
    u8    sense_data[18];
} PACKED  uas_ui_sense;

തരനിർവചനം ഘടന {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

തരനിർവചനം ഘടന {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

തരനിർവചനം ഘടന {
    uas_ui_header  hdr;
    സംഘം {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

ഘടന uasdrive_s {
    ഘടന drive_s drive;
    ഘടന usbdevice_s *usbdev;
    ഘടന usb_pipe *command, *status, *data_in, *data_out;
    u32 lun;
};

പൂർണ്ണസംഖ്യ
uas_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_USB_UAS)
        തിരികെ DISK_RET_EBADTRACK;

    ഘടന uasdrive_s *drive_gf = container_of(
        op->drive_fl, ഘടന uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, വലുപ്പം(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    പൂർണ്ണസംഖ്യ blocksize = scsi_fill_cmd(op, ui.command.cdb, വലുപ്പം(ui.command.cdb));
    എങ്കിൽ (blocksize < 0)
        തിരികെ default_process_op(op);
    പൂർണ്ണസംഖ്യ ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            വലുപ്പം(ui.hdr) + വലുപ്പം(ui.command));
    എങ്കിൽ (ret) {
        dprintf(1, "uas: command send fail");
        പോകുക fail;
    }

    memset(&ui, 0xff, വലുപ്പം(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), വലുപ്പം(ui));
    എങ്കിൽ (ret) {
        dprintf(1, "uas: status recv fail");
        പോകുക fail;
    }

    തിരഞ്ഞെടുപ്പ് (ui.hdr.id) {
    അവസ്ഥ UAS_UI_SENSE:
        പോകുക have_sense;
    അവസ്ഥ UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_in),
                            USB_DIR_IN, op->buf_fl, op->എണ്ണം * blocksize);
        എങ്കിൽ (ret) {
            dprintf(1, "uas: data read fail");
            പോകുക fail;
        }
        നിർത്തുക;
    അവസ്ഥ UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_out),
                            USB_DIR_OUT, op->buf_fl, op->എണ്ണം * blocksize);
        എങ്കിൽ (ret) {
            dprintf(1, "uas: data write fail");
            പോകുക fail;
        }
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        പോകുക fail;
    }

    memset(&ui, 0xff, വലുപ്പം(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), വലുപ്പം(ui));
    എങ്കിൽ (ret) {
        dprintf(1, "uas: status recv fail");
        പോകുക fail;
    }
    എങ്കിൽ (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        പോകുക fail;
    }

have_sense:
    എങ്കിൽ (ui.sense.status == 0) {
        തിരികെ DISK_RET_SUCCESS;
    }

fail:
    തിരികെ DISK_RET_EBADTRACK;
}

സ്ഥിതം ശൂന്യം
uas_init_lun(ഘടന uasdrive_s *drive, ഘടന usbdevice_s *usbdev,
             ഘടന usb_pipe *command, ഘടന usb_pipe *status,
             ഘടന usb_pipe *data_in, ഘടന usb_pipe *data_out,
             u32 lun)
{
    memset(drive, 0, വലുപ്പം(*drive));
    എങ്കിൽ (usb_32bit_pipe(data_in))
        drive->drive.type = DTYPE_UAS_32;
    അല്ലെങ്കിൽ
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->data_in = data_in;
    drive->data_out = data_out;
    drive->lun = lun;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
uas_add_lun(u32 lun, ഘടന drive_s *tmpl_drv)
{
    ഘടന uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, ഘടന uasdrive_s, drive);
    ഘടന uasdrive_s *drive = malloc_fseg(വലുപ്പം(*drive));
    എങ്കിൽ (!drive) {
        warn_noalloc();
        തിരികെ -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->data_in, tmpl_lun->data_out,
                 lun);

    പൂർണ്ണസംഖ്യ prio = bootprio_find_usb(drive->usbdev, drive->lun);
    പൂർണ്ണസംഖ്യ ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    എങ്കിൽ (ret) {
        സ്വതന്ത്രം(drive);
        തിരികെ -1;
    }
    തിരികെ 0;
}

പൂർണ്ണസംഖ്യ
usb_uas_setup(ഘടന usbdevice_s *usbdev)
{
    എങ്കിൽ (!CONFIG_USB_UAS)
        തിരികെ -1;

    // Verify right kind of device
    ഘടന usb_interface_descriptor *iface = usbdev->iface;
    എങ്കിൽ (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        തിരികെ -1;
    }

    /* find & allocate pipes */
    ഘടന usb_endpoint_descriptor *ep = ശൂന്യ;
    ഘടന usb_pipe *command = ശൂന്യ;
    ഘടന usb_pipe *status = ശൂന്യ;
    ഘടന usb_pipe *data_in = ശൂന്യ;
    ഘടന usb_pipe *data_out = ശൂന്യ;
    u8 *desc = (u8*)iface;
    ആകുമ്പോൾ (desc) {
        desc += desc[0];
        തിരഞ്ഞെടുപ്പ് (desc[1]) {
        അവസ്ഥ USB_DT_ENDPOINT:
            ep = (ശൂന്യം*)desc;
            നിർത്തുക;
        അവസ്ഥ USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            പോകുക fail;
        അവസ്ഥ 0x24:
            തിരഞ്ഞെടുപ്പ് (desc[2]) {
            അവസ്ഥ UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                നിർത്തുക;
            അവസ്ഥ UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                നിർത്തുക;
            അവസ്ഥ UAS_PIPE_ID_DATA_IN:
                data_in = usb_alloc_pipe(usbdev, ep);
                നിർത്തുക;
            അവസ്ഥ UAS_PIPE_ID_DATA_OUT:
                data_out = usb_alloc_pipe(usbdev, ep);
                നിർത്തുക;
            സ്ഥിരസ്ഥിതി:
                പോകുക fail;
            }
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            desc = ശൂന്യ;
            നിർത്തുക;
        }
    }
    എങ്കിൽ (!command || !status || !data_in || !data_out)
        പോകുക fail;

    ഘടന uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, data_in, data_out, 0);
    പൂർണ്ണസംഖ്യ ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    എങ്കിൽ (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        പോകുക fail;
    }

    തിരികെ 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, data_in);
    usb_free_pipe(usbdev, data_out);
    തിരികെ -1;
}
