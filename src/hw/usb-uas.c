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

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBALFLAT
#ଅନ୍ତର୍ଭାଵ "block.h" // DTYPE_USB
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // cdb_read
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_USB_UAS
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "usb.h" // struct usb_s
#ଅନ୍ତର୍ଭାଵ "usb-uas.h" // usb_uas_init
#ଅନ୍ତର୍ଭାଵ "util.h" // bootprio_find_usb

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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_length;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_length;
    u8    sense_data[18];
} PACKED  uas_ui_sense;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
    uas_ui_header  hdr;
    ସଂଘ {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

ସଂରଚନା uasdrive_s {
    ସଂରଚନା drive_s drive;
    ସଂରଚନା usbdevice_s *usbdev;
    ସଂରଚନା usb_pipe *command, *status, *data_in, *data_out;
    u32 lun;
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
uas_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_USB_UAS)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;

    ସଂରଚନା uasdrive_s *drive_gf = container_of(
        op->drive_fl, ସଂରଚନା uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, ଆକାର(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, ui.command.cdb, ଆକାର(ui.command.cdb));
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            ଆକାର(ui.hdr) + ଆକାର(ui.command));
    ଯଦି (ret) {
        dprintf(1, "uas: command send fail");
        ଯାଅ fail;
    }

    memset(&ui, 0xff, ଆକାର(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), ଆକାର(ui));
    ଯଦି (ret) {
        dprintf(1, "uas: status recv fail");
        ଯାଅ fail;
    }

    ବାଛ (ui.hdr.id) {
    ସ୍ଥିତି UAS_UI_SENSE:
        ଯାଅ have_sense;
    ସ୍ଥିତି UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_in),
                            USB_DIR_IN, op->buf_fl, op->ଗଣନା_2 * blocksize);
        ଯଦି (ret) {
            dprintf(1, "uas: data read fail");
            ଯାଅ fail;
        }
        ଭାଙ୍ଗ;
    ସ୍ଥିତି UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_out),
                            USB_DIR_OUT, op->buf_fl, op->ଗଣନା_2 * blocksize);
        ଯଦି (ret) {
            dprintf(1, "uas: data write fail");
            ଯାଅ fail;
        }
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        ଯାଅ fail;
    }

    memset(&ui, 0xff, ଆକାର(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), ଆକାର(ui));
    ଯଦି (ret) {
        dprintf(1, "uas: status recv fail");
        ଯାଅ fail;
    }
    ଯଦି (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        ଯାଅ fail;
    }

have_sense:
    ଯଦି (ui.sense.status == 0) {
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;
    }

fail:
    ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
}

ସ୍ଥିର ଶୂନ୍ୟ
uas_init_lun(ସଂରଚନା uasdrive_s *drive, ସଂରଚନା usbdevice_s *usbdev,
             ସଂରଚନା usb_pipe *command, ସଂରଚନା usb_pipe *status,
             ସଂରଚନା usb_pipe *data_in, ସଂରଚନା usb_pipe *data_out,
             u32 lun)
{
    memset(drive, 0, ଆକାର(*drive));
    ଯଦି (usb_32bit_pipe(data_in))
        drive->drive.type = DTYPE_UAS_32;
    ଅନ୍ୟଥା
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->data_in = data_in;
    drive->data_out = data_out;
    drive->lun = lun;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
uas_add_lun(u32 lun, ସଂରଚନା drive_s *tmpl_drv)
{
    ସଂରଚନା uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, ସଂରଚନା uasdrive_s, drive);
    ସଂରଚନା uasdrive_s *drive = malloc_fseg(ଆକାର(*drive));
    ଯଦି (!drive) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->data_in, tmpl_lun->data_out,
                 lun);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = bootprio_find_usb(drive->usbdev, drive->lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    ଯଦି (ret) {
        ମୁକ୍ତ(drive);
        ଫେରନ୍ତୁ -1;
    }
    ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_uas_setup(ସଂରଚନା usbdevice_s *usbdev)
{
    ଯଦି (!CONFIG_USB_UAS)
        ଫେରନ୍ତୁ -1;

    // Verify right kind of device
    ସଂରଚନା usb_interface_descriptor *iface = usbdev->iface;
    ଯଦି (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ଫେରନ୍ତୁ -1;
    }

    /* find & allocate pipes */
    ସଂରଚନା usb_endpoint_descriptor *ep = ଖାଲି;
    ସଂରଚନା usb_pipe *command = ଖାଲି;
    ସଂରଚନା usb_pipe *status = ଖାଲି;
    ସଂରଚନା usb_pipe *data_in = ଖାଲି;
    ସଂରଚନା usb_pipe *data_out = ଖାଲି;
    u8 *desc = (u8*)iface;
    ଯେତେବେଳେ (desc) {
        desc += desc[0];
        ବାଛ (desc[1]) {
        ସ୍ଥିତି USB_DT_ENDPOINT:
            ep = (ଶୂନ୍ୟ*)desc;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            ଯାଅ fail;
        ସ୍ଥିତି 0x24:
            ବାଛ (desc[2]) {
            ସ୍ଥିତି UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                ଭାଙ୍ଗ;
            ସ୍ଥିତି UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                ଭାଙ୍ଗ;
            ସ୍ଥିତି UAS_PIPE_ID_DATA_IN:
                data_in = usb_alloc_pipe(usbdev, ep);
                ଭାଙ୍ଗ;
            ସ୍ଥିତି UAS_PIPE_ID_DATA_OUT:
                data_out = usb_alloc_pipe(usbdev, ep);
                ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
                ଯାଅ fail;
            }
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            desc = ଖାଲି;
            ଭାଙ୍ଗ;
        }
    }
    ଯଦି (!command || !status || !data_in || !data_out)
        ଯାଅ fail;

    ସଂରଚନା uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, data_in, data_out, 0);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    ଯଦି (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        ଯାଅ fail;
    }

    ଫେରନ୍ତୁ 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, data_in);
    usb_free_pipe(usbdev, data_out);
    ଫେରନ୍ତୁ -1;
}
