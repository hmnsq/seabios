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

#تعریف_کن UAS_UI_COMMAND              0x01
#تعریف_کن UAS_UI_SENSE                0x03
#تعریف_کن UAS_UI_RESPONSE             0x04
#تعریف_کن UAS_UI_TASK_MGMT            0x05
#تعریف_کن UAS_UI_READ_READY           0x06
#تعریف_کن UAS_UI_WRITE_READY          0x07

#تعریف_کن UAS_PIPE_ID_COMMAND         0x01
#تعریف_کن UAS_PIPE_ID_وضعیت          0x02
#تعریف_کن UAS_PIPE_ID_داده_درون         0x03
#تعریف_کن UAS_PIPE_ID_داده_بیرون        0x04

تعريفنوع بنية {
    u8    id;
    u8    reserved;
    u16   برچسب;
} PACKED  uas_ui_سرآیند;

تعريفنوع بنية {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    جمع_کن_cdb_درازی;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    جمع_کن_cdb[];
} PACKED  uas_ui_command;

تعريفنوع بنية {
    u16   وضعیت_qualifier;
    u8    وضعیت;
    u8    reserved[7];
    u16   sense_درازی;
    u8    sense_داده[18];
} PACKED  uas_ui_sense;

تعريفنوع بنية {
    u16   جمع_کن_response_info;
    u8    response_کد;
} PACKED  uas_ui_response;

تعريفنوع بنية {
    u8    function;
    u8    reserved;
    u16   task_برچسب;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

تعريفنوع بنية {
    uas_ui_سرآیند  hdr;
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
    بنية usb_pipe *command, *وضعیت, *داده_درون, *داده_بیرون;
    u32 lun;
};

صحيح
uas_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_USB_UAS)
        أرجع DISK_RET_EBADTRACK;

    بنية uasdrive_s *drive_gf = container_از(
        op->drive_fl, بنية uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, حجم(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.برچسب = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    صحيح blocksize = scsi_fill_cmd(op, ui.command.cdb, حجم(ui.command.cdb));
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    صحيح ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_بیرون, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            حجم(ui.hdr) + حجم(ui.command));
    إذا (ret) {
        dprintf(1, "uas: command send fail");
        اذهب fail;
    }

    memset(&ui, 0xff, حجم(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->وضعیت),
                        USB_DIR_درون, MAKE_FLATPTR(GET_SEG(SS), &ui), حجم(ui));
    إذا (ret) {
        dprintf(1, "uas: status recv fail");
        اذهب fail;
    }

    بدل (ui.hdr.id) {
    حالة UAS_UI_SENSE:
        اذهب have_sense;
    حالة UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->داده_درون),
                            USB_DIR_درون, op->buf_fl, op->شمار * blocksize);
        إذا (ret) {
            dprintf(1, "uas: data read fail");
            اذهب fail;
        }
        اكسر;
    حالة UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->داده_بیرون),
                            USB_DIR_بیرون, op->buf_fl, op->شمار * blocksize);
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
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->وضعیت),
                        USB_DIR_درون, MAKE_FLATPTR(GET_SEG(SS), &ui), حجم(ui));
    إذا (ret) {
        dprintf(1, "uas: status recv fail");
        اذهب fail;
    }
    إذا (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        اذهب fail;
    }

have_sense:
    إذا (ui.sense.وضعیت == 0) {
        أرجع DISK_RET_SUCCESS;
    }

fail:
    أرجع DISK_RET_EBADTRACK;
}

ساكن فراغ
uas_init_lun(بنية uasdrive_s *drive, بنية usbdevice_s *usbdev,
             بنية usb_pipe *command, بنية usb_pipe *وضعیت,
             بنية usb_pipe *داده_درون, بنية usb_pipe *داده_بیرون,
             u32 lun)
{
    memset(drive, 0, حجم(*drive));
    إذا (usb_32bit_pipe(داده_درون))
        drive->drive.type = DTYPE_UAS_32;
    وإلا
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->وضعیت = وضعیت;
    drive->داده_درون = داده_درون;
    drive->داده_بیرون = داده_بیرون;
    drive->lun = lun;
}

ساكن صحيح
uas_جمع_کن_lun(u32 lun, بنية drive_s *tmpl_drv)
{
    بنية uasdrive_s *tmpl_lun =
        container_از(tmpl_drv, بنية uasdrive_s, drive);
    بنية uasdrive_s *drive = malloc_fseg(حجم(*drive));
    إذا (!drive) {
        warn_noalloc();
        أرجع -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->وضعیت,
                 tmpl_lun->داده_درون, tmpl_lun->داده_بیرون,
                 lun);

    صحيح prio = bootprio_پیدا_کن_usb(drive->usbdev, drive->lun);
    صحيح ret = scsi_drive_راهاندازی(&drive->drive, "USB UAS", prio);
    إذا (ret) {
        حرر(drive);
        أرجع -1;
    }
    أرجع 0;
}

صحيح
usb_uas_راهاندازی(بنية usbdevice_s *usbdev)
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
    بنية usb_pipe *وضعیت = عدم;
    بنية usb_pipe *داده_درون = عدم;
    بنية usb_pipe *داده_بیرون = عدم;
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
            حالة UAS_PIPE_ID_وضعیت:
                وضعیت = usb_alloc_pipe(usbdev, ep);
                اكسر;
            حالة UAS_PIPE_ID_داده_درون:
                داده_درون = usb_alloc_pipe(usbdev, ep);
                اكسر;
            حالة UAS_PIPE_ID_داده_بیرون:
                داده_بیرون = usb_alloc_pipe(usbdev, ep);
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
    إذا (!command || !وضعیت || !داده_درون || !داده_بیرون)
        اذهب fail;

    بنية uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, وضعیت, داده_درون, داده_بیرون, 0);
    صحيح ret = scsi_rep_luns_پیمایش(&lun0.drive, uas_جمع_کن_lun);
    إذا (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        اذهب fail;
    }

    أرجع 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, وضعیت);
    usb_free_pipe(usbdev, داده_درون);
    usb_free_pipe(usbdev, داده_بیرون);
    أرجع -1;
}
