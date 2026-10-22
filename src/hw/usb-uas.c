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

#समावेश "biosvar.h" // GET_GLOBALFLAT
#समावेश "block.h" // DTYPE_USB
#समावेश "blockcmd.h" // cdb_read
#समावेश "config.h" // CONFIG_USB_UAS
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "usb.h" // struct usb_s
#समावेश "usb-uas.h" // usb_uas_init
#समावेश "util.h" // bootprio_find_usb

#घोषणा UAS_UI_COMMAND              0x01
#घोषणा UAS_UI_SENSE                0x03
#घोषणा UAS_UI_RESPONSE             0x04
#घोषणा UAS_UI_TASK_MGMT            0x05
#घोषणा UAS_UI_READ_READY           0x06
#घोषणा UAS_UI_WRITE_READY          0x07

#घोषणा UAS_PIPE_ID_COMMAND         0x01
#घोषणा UAS_PIPE_ID_STATUS          0x02
#घोषणा UAS_PIPE_ID_तथ्य_IN         0x03
#घोषणा UAS_PIPE_ID_तथ्य_OUT        0x04

प्रकार काष्ठा {
    u8    id;
    u8    reserved;
    u16   tag;
} PACKED  uas_ui_header;

प्रकार काष्ठा {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    add_cdb_लंबाई;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    add_cdb[];
} PACKED  uas_ui_command;

प्रकार काष्ठा {
    u16   status_qualifier;
    u8    status;
    u8    reserved[7];
    u16   sense_लंबाई;
    u8    sense_तथ्य[18];
} PACKED  uas_ui_sense;

प्रकार काष्ठा {
    u16   add_response_info;
    u8    response_code;
} PACKED  uas_ui_response;

प्रकार काष्ठा {
    u8    function;
    u8    reserved;
    u16   task_tag;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

प्रकार काष्ठा {
    uas_ui_header  hdr;
    जोड़ {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

काष्ठा uasdrive_s {
    काष्ठा drive_s drive;
    काष्ठा usbdevice_s *usbdev;
    काष्ठा usb_pipe *command, *status, *तथ्य_in, *तथ्य_out;
    u32 lun;
};

पूर्णांक
uas_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_USB_UAS)
        वापस DISK_RET_EBADTRACK;

    काष्ठा uasdrive_s *drive_gf = container_of(
        op->drive_fl, काष्ठा uasdrive_s, drive);

    uas_ui ui;
    स_रखो(&ui, 0, माप(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.tag = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    पूर्णांक blocksize = scsi_fill_cmd(op, ui.command.cdb, माप(ui.command.cdb));
    अगर (blocksize < 0)
        वापस default_process_op(op);
    पूर्णांक ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_OUT, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            माप(ui.hdr) + माप(ui.command));
    अगर (ret) {
        dprintf(1, "uas: command send fail");
        जाओ fail;
    }

    स_रखो(&ui, 0xff, माप(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), माप(ui));
    अगर (ret) {
        dprintf(1, "uas: status recv fail");
        जाओ fail;
    }

    चयन (ui.hdr.id) {
    हाल UAS_UI_SENSE:
        जाओ have_sense;
    हाल UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->तथ्य_in),
                            USB_DIR_IN, op->buf_fl, op->गिनती * blocksize);
        अगर (ret) {
            dprintf(1, "uas: data read fail");
            जाओ fail;
        }
        अवरोध;
    हाल UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->तथ्य_out),
                            USB_DIR_OUT, op->buf_fl, op->गिनती * blocksize);
        अगर (ret) {
            dprintf(1, "uas: data write fail");
            जाओ fail;
        }
        अवरोध;
    शेष:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        जाओ fail;
    }

    स_रखो(&ui, 0xff, माप(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->status),
                        USB_DIR_IN, MAKE_FLATPTR(GET_SEG(SS), &ui), माप(ui));
    अगर (ret) {
        dprintf(1, "uas: status recv fail");
        जाओ fail;
    }
    अगर (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        जाओ fail;
    }

have_sense:
    अगर (ui.sense.status == 0) {
        वापस DISK_RET_SUCCESS;
    }

fail:
    वापस DISK_RET_EBADTRACK;
}

अटल व्योम
uas_init_lun(काष्ठा uasdrive_s *drive, काष्ठा usbdevice_s *usbdev,
             काष्ठा usb_pipe *command, काष्ठा usb_pipe *status,
             काष्ठा usb_pipe *तथ्य_in, काष्ठा usb_pipe *तथ्य_out,
             u32 lun)
{
    स_रखो(drive, 0, माप(*drive));
    अगर (usb_32bit_pipe(तथ्य_in))
        drive->drive.type = DTYPE_UAS_32;
    अन्यथा
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->status = status;
    drive->तथ्य_in = तथ्य_in;
    drive->तथ्य_out = तथ्य_out;
    drive->lun = lun;
}

अटल पूर्णांक
uas_add_lun(u32 lun, काष्ठा drive_s *tmpl_drv)
{
    काष्ठा uasdrive_s *tmpl_lun =
        container_of(tmpl_drv, काष्ठा uasdrive_s, drive);
    काष्ठा uasdrive_s *drive = malloc_fseg(माप(*drive));
    अगर (!drive) {
        warn_noalloc();
        वापस -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->status,
                 tmpl_lun->तथ्य_in, tmpl_lun->तथ्य_out,
                 lun);

    पूर्णांक prio = bootprio_find_usb(drive->usbdev, drive->lun);
    पूर्णांक ret = scsi_drive_setup(&drive->drive, "USB UAS", prio);
    अगर (ret) {
        मुक्त(drive);
        वापस -1;
    }
    वापस 0;
}

पूर्णांक
usb_uas_setup(काष्ठा usbdevice_s *usbdev)
{
    अगर (!CONFIG_USB_UAS)
        वापस -1;

    // Verify right kind of device
    काष्ठा usb_interface_descriptor *iface = usbdev->iface;
    अगर (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        वापस -1;
    }

    /* find & allocate pipes */
    काष्ठा usb_endpoint_descriptor *ep = शून्य;
    काष्ठा usb_pipe *command = शून्य;
    काष्ठा usb_pipe *status = शून्य;
    काष्ठा usb_pipe *तथ्य_in = शून्य;
    काष्ठा usb_pipe *तथ्य_out = शून्य;
    u8 *desc = (u8*)iface;
    जबतक (desc) {
        desc += desc[0];
        चयन (desc[1]) {
        हाल USB_DT_ENDPOINT:
            ep = (व्योम*)desc;
            अवरोध;
        हाल USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            जाओ fail;
        हाल 0x24:
            चयन (desc[2]) {
            हाल UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                अवरोध;
            हाल UAS_PIPE_ID_STATUS:
                status = usb_alloc_pipe(usbdev, ep);
                अवरोध;
            हाल UAS_PIPE_ID_तथ्य_IN:
                तथ्य_in = usb_alloc_pipe(usbdev, ep);
                अवरोध;
            हाल UAS_PIPE_ID_तथ्य_OUT:
                तथ्य_out = usb_alloc_pipe(usbdev, ep);
                अवरोध;
            शेष:
                जाओ fail;
            }
            अवरोध;
        शेष:
            desc = शून्य;
            अवरोध;
        }
    }
    अगर (!command || !status || !तथ्य_in || !तथ्य_out)
        जाओ fail;

    काष्ठा uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, status, तथ्य_in, तथ्य_out, 0);
    पूर्णांक ret = scsi_rep_luns_scan(&lun0.drive, uas_add_lun);
    अगर (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        जाओ fail;
    }

    वापस 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, status);
    usb_free_pipe(usbdev, तथ्य_in);
    usb_free_pipe(usbdev, तथ्य_out);
    वापस -1;
}
