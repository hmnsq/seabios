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

#અન્તર્ભાવ "biosvar.h" // GET_GLOBALFLAT
#અન્તર્ભાવ "block.h" // DTYPE_USB
#અન્તર્ભાવ "blockcmd.h" // cdb_read
#અન્તર્ભાવ "config.h" // CONFIG_USB_UAS
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "usb.h" // struct usb_s
#અન્તર્ભાવ "usb-uas.h" // usb_uas_init
#અન્તર્ભાવ "util.h" // bootprio_find_usb

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

પ્રકારવ્યાખ્યા સંરચના {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

પ્રકારવ્યાખ્યા સંરચના {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_length;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

પ્રકારવ્યાખ્યા સંરચના {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_length;
    u8    sense_data[18];
} PACKED  uas_ui_sense;

પ્રકારવ્યાખ્યા સંરચના {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

પ્રકારવ્યાખ્યા સંરચના {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

પ્રકારવ્યાખ્યા સંરચના {
    uas_ui_header  hdr;
    સંઘ {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

સંરચના uasdrive_s {
    સંરચના drive_s drive;
    સંરચના usbdevice_s *usbdev;
    સંરચના usb_pipe *command, *status, *data_in, *data_out;
    u32 lun;
};

પૂર્ણાંક
uas_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_USB_UAS)
        પાછા DISK_RET_EBADTRACK;

    સંરચના uasdrive_s *drive_gf = container_of(
        op->drive_fl, સંરચના uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, કદ(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    પૂર્ણાંક blocksize = scsi_fill_cmd(op, ui.command.cdb, કદ(ui.command.cdb));
    જો (blocksize < 0)
        પાછા default_process_op(op);
    પૂર્ણાંક ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            કદ(ui.hdr) + કદ(ui.command));
    જો (ret) {
        dprintf(1, "uas: command send fail");
        જાઓ fail;
    }

    memset(&ui, 0xff, કદ(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), કદ(ui));
    જો (ret) {
        dprintf(1, "uas: status recv fail");
        જાઓ fail;
    }

    પસંદગી (ui.hdr.id) {
    સ્થિતિ UAS_UI_SENSE:
        જાઓ have_sense;
    સ્થિતિ UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_in),
                            USB_DIR_IN, op->buf_fl, op->ગણતરી * blocksize);
        જો (ret) {
            dprintf(1, "uas: data read fail");
            જાઓ fail;
        }
        તોડો;
    સ્થિતિ UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->data_out),
                            USB_DIR_OUT, op->buf_fl, op->ગણતરી * blocksize);
        જો (ret) {
            dprintf(1, "uas: data write fail");
            જાઓ fail;
        }
        તોડો;
    મૂળભૂત:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        જાઓ fail;
    }

    memset(&ui, 0xff, કદ(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), કદ(ui));
    જો (ret) {
        dprintf(1, "uas: status recv fail");
        જાઓ fail;
    }
    જો (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        જાઓ fail;
    }

have_sense:
    જો (ui.sense.status == 0) {
        પાછા DISK_RET_SUCCESS;
    }

fail:
    પાછા DISK_RET_EBADTRACK;
}

સ્થિર શૂન્ય
uas_init_lun(સંરચના uasdrive_s *drive, સંરચના usbdevice_s *usbdev,
             સંરચના usb_pipe *command, સંરચના usb_pipe *status,
             સંરચના usb_pipe *data_in, સંરચના usb_pipe *data_out,
             u32 lun)
{
    memset(drive, 0, કદ(*drive));
    જો (usb_32bit_pipe(data_in))
        drive->drive.type = DTYPE_UAS_32;
    નહીંતર
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->data_in = data_in;
    drive->data_out = data_out;
    drive->lun = lun;
}

સ્થિર પૂર્ણાંક
uas_add_lun(u32 lun, સંરચના drive_s *tmpl_drv)
{
    સંરચના uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, સંરચના uasdrive_s, drive);
    સંરચના uasdrive_s *drive = malloc_fseg(કદ(*drive));
    જો (!drive) {
        warn_noalloc();
        પાછા -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->data_in, tmpl_lun->data_out,
                 lun);

    પૂર્ણાંક prio = bootprio_find_usb(drive->usbdev, drive->lun);
    પૂર્ણાંક ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    જો (ret) {
        મુક્ત(drive);
        પાછા -1;
    }
    પાછા 0;
}

પૂર્ણાંક
usb_uas_setup(સંરચના usbdevice_s *usbdev)
{
    જો (!CONFIG_USB_UAS)
        પાછા -1;

    // Verify right kind of device
    સંરચના usb_interface_descriptor *iface = usbdev->iface;
    જો (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        પાછા -1;
    }

    /* find & allocate pipes */
    સંરચના usb_endpoint_descriptor *ep = ખાલી;
    સંરચના usb_pipe *command = ખાલી;
    સંરચના usb_pipe *status = ખાલી;
    સંરચના usb_pipe *data_in = ખાલી;
    સંરચના usb_pipe *data_out = ખાલી;
    u8 *desc = (u8*)iface;
    જ્યારે (desc) {
        desc += desc[0];
        પસંદગી (desc[1]) {
        સ્થિતિ USB_DT_ENDPOINT:
            ep = (શૂન્ય*)desc;
            તોડો;
        સ્થિતિ USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            જાઓ fail;
        સ્થિતિ 0x24:
            પસંદગી (desc[2]) {
            સ્થિતિ UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                તોડો;
            સ્થિતિ UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                તોડો;
            સ્થિતિ UAS_PIPE_ID_DATA_IN:
                data_in = usb_alloc_pipe(usbdev, ep);
                તોડો;
            સ્થિતિ UAS_PIPE_ID_DATA_OUT:
                data_out = usb_alloc_pipe(usbdev, ep);
                તોડો;
            મૂળભૂત:
                જાઓ fail;
            }
            તોડો;
        મૂળભૂત:
            desc = ખાલી;
            તોડો;
        }
    }
    જો (!command || !status || !data_in || !data_out)
        જાઓ fail;

    સંરચના uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, data_in, data_out, 0);
    પૂર્ણાંક ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    જો (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        જાઓ fail;
    }

    પાછા 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, data_in);
    usb_free_pipe(usbdev, data_out);
    પાછા -1;
}
