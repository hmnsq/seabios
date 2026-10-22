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

#تعریف_کرو UAS_UI_COMMAND              0x01
#تعریف_کرو UAS_UI_SENSE                0x03
#تعریف_کرو UAS_UI_RESPONSE             0x04
#تعریف_کرو UAS_UI_TASK_MGMT            0x05
#تعریف_کرو UAS_UI_READ_READY           0x06
#تعریف_کرو UAS_UI_WRITE_READY          0x07

#تعریف_کرو UAS_PIPE_ID_COMMAND         0x01
#تعریف_کرو UAS_PIPE_ID_حالت          0x02
#تعریف_کرو UAS_PIPE_ID_ڈیٹا_اندر         0x03
#تعریف_کرو UAS_PIPE_ID_ڈیٹا_برآمد        0x04

قسمتعریف ساخت {
    u8    id;
    u8    reserved;
    u16   لیبل;
} PACKED  uas_ui_سرخی;

قسمتعریف ساخت {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    جوڑو_cdb_لمبائی;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    جوڑو_cdb[];
} PACKED  uas_ui_command;

قسمتعریف ساخت {
    u16   حالت_qualifier;
    u8    حالت_2;
    u8    reserved[7];
    u16   sense_لمبائی;
    u8    sense_ڈیٹا[18];
} PACKED  uas_ui_sense;

قسمتعریف ساخت {
    u16   جوڑو_response_info;
    u8    response_رمز;
} PACKED  uas_ui_response;

قسمتعریف ساخت {
    u8    function;
    u8    reserved;
    u16   task_لیبل;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

قسمتعریف ساخت {
    uas_ui_سرخی  hdr;
    اتحاد {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

ساخت uasdrive_s {
    ساخت drive_s drive;
    ساخت usbdevice_s *usbdev;
    ساخت usb_pipe *command, *حالت_2, *ڈیٹا_اندر, *ڈیٹا_برآمد;
    u32 lun;
};

صحیح
uas_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_USB_UAS)
        واپس DISK_RET_EBADTRACK;

    ساخت uasdrive_s *drive_gf = container_کا(
        op->drive_fl, ساخت uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, حجم(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.لیبل = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    صحیح blocksize = scsi_fill_cmd(op, ui.command.cdb, حجم(ui.command.cdb));
    اگر (blocksize < 0)
        واپس default_process_op(op);
    صحیح ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_برآمد, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            حجم(ui.hdr) + حجم(ui.command));
    اگر (ret) {
        dprintf(1, "uas: command send fail");
        جاؤ fail;
    }

    memset(&ui, 0xff, حجم(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->حالت_2),
                        USB_DIR_اندر, MAKE_FLATPTR(GET_SEG(SS), &ui), حجم(ui));
    اگر (ret) {
        dprintf(1, "uas: status recv fail");
        جاؤ fail;
    }

    انتخاب (ui.hdr.id) {
    حالت UAS_UI_SENSE:
        جاؤ have_sense;
    حالت UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ڈیٹا_اندر),
                            USB_DIR_اندر, op->buf_fl, op->گنتی * blocksize);
        اگر (ret) {
            dprintf(1, "uas: data read fail");
            جاؤ fail;
        }
        توڑو;
    حالت UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ڈیٹا_برآمد),
                            USB_DIR_برآمد, op->buf_fl, op->گنتی * blocksize);
        اگر (ret) {
            dprintf(1, "uas: data write fail");
            جاؤ fail;
        }
        توڑو;
    طےشدہ:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        جاؤ fail;
    }

    memset(&ui, 0xff, حجم(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->حالت_2),
                        USB_DIR_اندر, MAKE_FLATPTR(GET_SEG(SS), &ui), حجم(ui));
    اگر (ret) {
        dprintf(1, "uas: status recv fail");
        جاؤ fail;
    }
    اگر (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        جاؤ fail;
    }

have_sense:
    اگر (ui.sense.حالت_2 == 0) {
        واپس DISK_RET_SUCCESS;
    }

fail:
    واپس DISK_RET_EBADTRACK;
}

ساکن خالی
uas_init_lun(ساخت uasdrive_s *drive, ساخت usbdevice_s *usbdev,
             ساخت usb_pipe *command, ساخت usb_pipe *حالت_2,
             ساخت usb_pipe *ڈیٹا_اندر, ساخت usb_pipe *ڈیٹا_برآمد,
             u32 lun)
{
    memset(drive, 0, حجم(*drive));
    اگر (usb_32bit_pipe(ڈیٹا_اندر))
        drive->drive.type = DTYPE_UAS_32;
    ورنہ
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->حالت_2 = حالت_2;
    drive->ڈیٹا_اندر = ڈیٹا_اندر;
    drive->ڈیٹا_برآمد = ڈیٹا_برآمد;
    drive->lun = lun;
}

ساکن صحیح
uas_جوڑو_lun(u32 lun, ساخت drive_s *tmpl_drv)
{
    ساخت uasdrive_s *tmpl_lun =
        container_کا(tmpl_drv, ساخت uasdrive_s, drive);
    ساخت uasdrive_s *drive = malloc_fseg(حجم(*drive));
    اگر (!drive) {
        warn_noalloc();
        واپس -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->حالت_2,
                 tmpl_lun->ڈیٹا_اندر, tmpl_lun->ڈیٹا_برآمد,
                 lun);

    صحیح prio = bootprio_ڈھونڈو_usb(drive->usbdev, drive->lun);
    صحیح ret = scsi_drive_تیاری(&drive->drive, "USB UAS", prio);
    اگر (ret) {
        آزاد(drive);
        واپس -1;
    }
    واپس 0;
}

صحیح
usb_uas_تیاری(ساخت usbdevice_s *usbdev)
{
    اگر (!CONFIG_USB_UAS)
        واپس -1;

    // Verify right kind of device
    ساخت usb_interface_descriptor *iface = usbdev->iface;
    اگر (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        واپس -1;
    }

    /* find & allocate pipes */
    ساخت usb_endpoint_descriptor *ep = تہی;
    ساخت usb_pipe *command = تہی;
    ساخت usb_pipe *حالت_2 = تہی;
    ساخت usb_pipe *ڈیٹا_اندر = تہی;
    ساخت usb_pipe *ڈیٹا_برآمد = تہی;
    u8 *desc = (u8*)iface;
    جب (desc) {
        desc += desc[0];
        انتخاب (desc[1]) {
        حالت USB_DT_ENDPOINT:
            ep = (خالی*)desc;
            توڑو;
        حالت USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            جاؤ fail;
        حالت 0x24:
            انتخاب (desc[2]) {
            حالت UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                توڑو;
            حالت UAS_PIPE_ID_حالت:
                حالت_2 = usb_alloc_pipe(usbdev, ep);
                توڑو;
            حالت UAS_PIPE_ID_ڈیٹا_اندر:
                ڈیٹا_اندر = usb_alloc_pipe(usbdev, ep);
                توڑو;
            حالت UAS_PIPE_ID_ڈیٹا_برآمد:
                ڈیٹا_برآمد = usb_alloc_pipe(usbdev, ep);
                توڑو;
            طےشدہ:
                جاؤ fail;
            }
            توڑو;
        طےشدہ:
            desc = تہی;
            توڑو;
        }
    }
    اگر (!command || !حالت_2 || !ڈیٹا_اندر || !ڈیٹا_برآمد)
        جاؤ fail;

    ساخت uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, حالت_2, ڈیٹا_اندر, ڈیٹا_برآمد, 0);
    صحیح ret = scsi_rep_luns_جائزہ(&lun0.drive, uas_جوڑو_lun);
    اگر (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        جاؤ fail;
    }

    واپس 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, حالت_2);
    usb_free_pipe(usbdev, ڈیٹا_اندر);
    usb_free_pipe(usbdev, ڈیٹا_برآمد);
    واپس -1;
}
