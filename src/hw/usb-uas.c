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

#وصف_ڪريو UAS_UI_COMMAND              0x01
#وصف_ڪريو UAS_UI_SENSE                0x03
#وصف_ڪريو UAS_UI_RESPONSE             0x04
#وصف_ڪريو UAS_UI_TASK_MGMT            0x05
#وصف_ڪريو UAS_UI_READ_READY           0x06
#وصف_ڪريو UAS_UI_WRITE_READY          0x07

#وصف_ڪريو UAS_PIPE_ID_COMMAND         0x01
#وصف_ڪريو UAS_PIPE_ID_حالت          0x02
#وصف_ڪريو UAS_PIPE_ID_معلومات_اندريون         0x03
#وصف_ڪريو UAS_PIPE_ID_معلومات_ٻاهريون        0x04

تعريفنوع بنية {
    u8    id;
    u8    reserved;
    u16   ٽيگ;
} PACKED  uas_ui_سرو;

تعريفنوع بنية {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    جوڙيو_cdb_ڊيگهائي;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    جوڙيو_cdb[];
} PACKED  uas_ui_command;

تعريفنوع بنية {
    u16   حالت_qualifier;
    u8    حالت;
    u8    reserved[7];
    u16   sense_ڊيگهائي;
    u8    sense_معلومات[18];
} PACKED  uas_ui_sense;

تعريفنوع بنية {
    u16   جوڙيو_response_info;
    u8    response_ڪوڊ;
} PACKED  uas_ui_response;

تعريفنوع بنية {
    u8    function;
    u8    reserved;
    u16   task_ٽيگ;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

تعريفنوع بنية {
    uas_ui_سرو  hdr;
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
    بنية usb_pipe *command, *حالت, *معلومات_اندريون, *معلومات_ٻاهريون;
    u32 lun;
};

صحيح
uas_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_USB_UAS)
        أرجع DISK_RET_EBADTRACK;

    بنية uasdrive_s *drive_gf = container_جو(
        op->drive_fl, بنية uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, حجم(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.ٽيگ = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    صحيح blocksize = scsi_fill_cmd(op, ui.command.cdb, حجم(ui.command.cdb));
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    صحيح ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_ٻاهريون, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            حجم(ui.hdr) + حجم(ui.command));
    إذا (ret) {
        dprintf(1, "uas: command send fail");
        اذهب fail;
    }

    memset(&ui, 0xff, حجم(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->حالت),
                        USB_DIR_اندريون, MAKE_FLATPTR(GET_SEG(SS), &ui), حجم(ui));
    إذا (ret) {
        dprintf(1, "uas: status recv fail");
        اذهب fail;
    }

    بدل (ui.hdr.id) {
    حالة UAS_UI_SENSE:
        اذهب have_sense;
    حالة UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->معلومات_اندريون),
                            USB_DIR_اندريون, op->buf_fl, op->ڳڻپ * blocksize);
        إذا (ret) {
            dprintf(1, "uas: data read fail");
            اذهب fail;
        }
        اكسر;
    حالة UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->معلومات_ٻاهريون),
                            USB_DIR_ٻاهريون, op->buf_fl, op->ڳڻپ * blocksize);
        إذا (ret) {
            dprintf(1, "uas: data write fail");
            اذهب fail;
        }
        اكسر;
    افتراضي:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        اذهب fail;
    }

    memset(&ui, 0xff, حجم(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->حالت),
                        USB_DIR_اندريون, MAKE_FLATPTR(GET_SEG(SS), &ui), حجم(ui));
    إذا (ret) {
        dprintf(1, "uas: status recv fail");
        اذهب fail;
    }
    إذا (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        اذهب fail;
    }

have_sense:
    إذا (ui.sense.حالت == 0) {
        أرجع DISK_RET_SUCCESS;
    }

fail:
    أرجع DISK_RET_EBADTRACK;
}

ساكن فراغ
uas_init_lun(بنية uasdrive_s *drive, بنية usbdevice_s *usbdev,
             بنية usb_pipe *command, بنية usb_pipe *حالت,
             بنية usb_pipe *معلومات_اندريون, بنية usb_pipe *معلومات_ٻاهريون,
             u32 lun)
{
    memset(drive, 0, حجم(*drive));
    إذا (usb_32bit_pipe(معلومات_اندريون))
        drive->drive.type = DTYPE_UAS_32;
    وإلا
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->حالت = حالت;
    drive->معلومات_اندريون = معلومات_اندريون;
    drive->معلومات_ٻاهريون = معلومات_ٻاهريون;
    drive->lun = lun;
}

ساكن صحيح
uas_جوڙيو_lun(u32 lun, بنية drive_s *tmpl_drv)
{
    بنية uasdrive_s *tmpl_lun =
        container_جو(tmpl_drv, بنية uasdrive_s, drive);
    بنية uasdrive_s *drive = malloc_fseg(حجم(*drive));
    إذا (!drive) {
        warn_noalloc();
        أرجع -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->حالت,
                 tmpl_lun->معلومات_اندريون, tmpl_lun->معلومات_ٻاهريون,
                 lun);

    صحيح prio = bootprio_ڳوليو_usb(drive->usbdev, drive->lun);
    صحيح ret = scsi_drive_تياري(&drive->drive, "USB UAS", prio);
    إذا (ret) {
        حرر(drive);
        أرجع -1;
    }
    أرجع 0;
}

صحيح
usb_uas_تياري(بنية usbdevice_s *usbdev)
{
    إذا (!CONFIG_USB_UAS)
        أرجع -1;

    // Verify right kind of device
    بنية usb_interface_descriptor *iface = usbdev->iface;
    إذا (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        أرجع -1;
    }

    /* find & allocate pipes */
    بنية usb_endpoint_descriptor *ep = عدم;
    بنية usb_pipe *command = عدم;
    بنية usb_pipe *حالت = عدم;
    بنية usb_pipe *معلومات_اندريون = عدم;
    بنية usb_pipe *معلومات_ٻاهريون = عدم;
    u8 *desc = (u8*)iface;
    طالما (desc) {
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
            حالة UAS_PIPE_ID_حالت:
                حالت = usb_alloc_pipe(usbdev, ep);
                اكسر;
            حالة UAS_PIPE_ID_معلومات_اندريون:
                معلومات_اندريون = usb_alloc_pipe(usbdev, ep);
                اكسر;
            حالة UAS_PIPE_ID_معلومات_ٻاهريون:
                معلومات_ٻاهريون = usb_alloc_pipe(usbdev, ep);
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
    إذا (!command || !حالت || !معلومات_اندريون || !معلومات_ٻاهريون)
        اذهب fail;

    بنية uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, حالت, معلومات_اندريون, معلومات_ٻاهريون, 0);
    صحيح ret = scsi_rep_luns_جاچ(&lun0.drive, uas_جوڙيو_lun);
    إذا (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        اذهب fail;
    }

    أرجع 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, حالت);
    usb_free_pipe(usbdev, معلومات_اندريون);
    usb_free_pipe(usbdev, معلومات_ٻاهريون);
    أرجع -1;
}
