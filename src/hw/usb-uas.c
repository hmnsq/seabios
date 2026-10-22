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

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // DTYPE_USB
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // cdb_read
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_USB_UAS
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // struct usb_s
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-uas.h" // usb_uas_init
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // bootprio_find_usb

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_UI_COMMAND              0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_UI_SENSE                0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_UI_RESPONSE             0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_UI_TASK_MGMT            0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_UI_READ_READY           0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_UI_WRITE_READY          0x07

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_PIPE_ID_COMMAND         0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_PIPE_ID_ਸਥਿਤੀ          0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_PIPE_ID_ਅੰਕੜੇ_ਵਿੱਚ         0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UAS_PIPE_ID_ਅੰਕੜੇ_ਬਾਹਰਲਾ        0x04

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    u8    id;
    u8    reserved;
    u16   ਟੈਗ;
} PACKED  uas_ui_ਸਿਰਲੇਖ;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    ਜੋੜੋ_cdb_ਲੰਮਾਈ;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    ਜੋੜੋ_cdb[];
} PACKED  uas_ui_command;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    u16   ਸਥਿਤੀ_qualifier;
    u8    ਸਥਿਤੀ;
    u8    reserved[7];
    u16   sense_ਲੰਮਾਈ;
    u8    sense_ਅੰਕੜੇ[18];
} PACKED  uas_ui_sense;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    u16   ਜੋੜੋ_response_info;
    u8    response_ਕੋਡ;
} PACKED  uas_ui_response;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    u8    function;
    u8    reserved;
    u16   task_ਟੈਗ;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
    uas_ui_ਸਿਰਲੇਖ  hdr;
    ਸੰਘ {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

ਬਣਤਰ uasdrive_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ usbdevice_s *usbdev;
    ਬਣਤਰ usb_pipe *command, *ਸਥਿਤੀ, *ਅੰਕੜੇ_ਵਿੱਚ, *ਅੰਕੜੇ_ਬਾਹਰਲਾ;
    u32 lun;
};

ਪੂਰਨ
uas_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_USB_UAS)
        ਵਾਪਸ DISK_RET_EBADTRACK;

    ਬਣਤਰ uasdrive_s *drive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, ਆਕਾਰ(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.ਟੈਗ = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, ui.command.cdb, ਆਕਾਰ(ui.command.cdb));
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);
    ਪੂਰਨ ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_ਬਾਹਰਲਾ, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            ਆਕਾਰ(ui.hdr) + ਆਕਾਰ(ui.command));
    ਜੇ (ret) {
        dprintf(1, "uas: command send fail");
        ਜਾਓ fail;
    }

    memset(&ui, 0xff, ਆਕਾਰ(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ਸਥਿਤੀ),
                        USB_DIR_ਵਿੱਚ, MAKE_FLATPTR(GET_SEG(SS), &ui), ਆਕਾਰ(ui));
    ਜੇ (ret) {
        dprintf(1, "uas: status recv fail");
        ਜਾਓ fail;
    }

    ਚੋਣ (ui.hdr.id) {
    ਹਾਲਤ UAS_UI_SENSE:
        ਜਾਓ have_sense;
    ਹਾਲਤ UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ਅੰਕੜੇ_ਵਿੱਚ),
                            USB_DIR_ਵਿੱਚ, op->buf_fl, op->ਗਿਣਤੀ_2 * blocksize);
        ਜੇ (ret) {
            dprintf(1, "uas: data read fail");
            ਜਾਓ fail;
        }
        ਰੋਕੋ;
    ਹਾਲਤ UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ਅੰਕੜੇ_ਬਾਹਰਲਾ),
                            USB_DIR_ਬਾਹਰਲਾ, op->buf_fl, op->ਗਿਣਤੀ_2 * blocksize);
        ਜੇ (ret) {
            dprintf(1, "uas: data write fail");
            ਜਾਓ fail;
        }
        ਰੋਕੋ;
    ਮੂਲ:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        ਜਾਓ fail;
    }

    memset(&ui, 0xff, ਆਕਾਰ(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ਸਥਿਤੀ),
                        USB_DIR_ਵਿੱਚ, MAKE_FLATPTR(GET_SEG(SS), &ui), ਆਕਾਰ(ui));
    ਜੇ (ret) {
        dprintf(1, "uas: status recv fail");
        ਜਾਓ fail;
    }
    ਜੇ (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        ਜਾਓ fail;
    }

have_sense:
    ਜੇ (ui.sense.ਸਥਿਤੀ == 0) {
        ਵਾਪਸ DISK_RET_SUCCESS;
    }

fail:
    ਵਾਪਸ DISK_RET_EBADTRACK;
}

ਸਥਿਰ ਖਾਲੀ
uas_init_lun(ਬਣਤਰ uasdrive_s *drive, ਬਣਤਰ usbdevice_s *usbdev,
             ਬਣਤਰ usb_pipe *command, ਬਣਤਰ usb_pipe *ਸਥਿਤੀ,
             ਬਣਤਰ usb_pipe *ਅੰਕੜੇ_ਵਿੱਚ, ਬਣਤਰ usb_pipe *ਅੰਕੜੇ_ਬਾਹਰਲਾ,
             u32 lun)
{
    memset(drive, 0, ਆਕਾਰ(*drive));
    ਜੇ (usb_32bit_pipe(ਅੰਕੜੇ_ਵਿੱਚ))
        drive->drive.type = DTYPE_UAS_32;
    ਨਹੀਂਤਾਂ
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->ਸਥਿਤੀ = ਸਥਿਤੀ;
    drive->ਅੰਕੜੇ_ਵਿੱਚ = ਅੰਕੜੇ_ਵਿੱਚ;
    drive->ਅੰਕੜੇ_ਬਾਹਰਲਾ = ਅੰਕੜੇ_ਬਾਹਰਲਾ;
    drive->lun = lun;
}

ਸਥਿਰ ਪੂਰਨ
uas_ਜੋੜੋ_lun(u32 lun, ਬਣਤਰ drive_s *tmpl_drv)
{
    ਬਣਤਰ uasdrive_s *tmpl_lun =
        container_ਦਾ(tmpl_drv, ਬਣਤਰ uasdrive_s, drive);
    ਬਣਤਰ uasdrive_s *drive = malloc_fseg(ਆਕਾਰ(*drive));
    ਜੇ (!drive) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->ਸਥਿਤੀ,
                 tmpl_lun->ਅੰਕੜੇ_ਵਿੱਚ, tmpl_lun->ਅੰਕੜੇ_ਬਾਹਰਲਾ,
                 lun);

    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_usb(drive->usbdev, drive->lun);
    ਪੂਰਨ ret = scsi_drive_ਤਿਆਰੀ(&drive->drive, "USB UAS", prio);
    ਜੇ (ret) {
        ਮੁਕਤ(drive);
        ਵਾਪਸ -1;
    }
    ਵਾਪਸ 0;
}

ਪੂਰਨ
usb_uas_ਤਿਆਰੀ(ਬਣਤਰ usbdevice_s *usbdev)
{
    ਜੇ (!CONFIG_USB_UAS)
        ਵਾਪਸ -1;

    // Verify right kind of device
    ਬਣਤਰ usb_interface_descriptor *iface = usbdev->iface;
    ਜੇ (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ਵਾਪਸ -1;
    }

    /* find & allocate pipes */
    ਬਣਤਰ usb_endpoint_descriptor *ep = ਨਿਰਾ;
    ਬਣਤਰ usb_pipe *command = ਨਿਰਾ;
    ਬਣਤਰ usb_pipe *ਸਥਿਤੀ = ਨਿਰਾ;
    ਬਣਤਰ usb_pipe *ਅੰਕੜੇ_ਵਿੱਚ = ਨਿਰਾ;
    ਬਣਤਰ usb_pipe *ਅੰਕੜੇ_ਬਾਹਰਲਾ = ਨਿਰਾ;
    u8 *desc = (u8*)iface;
    ਜਦੋਂ (desc) {
        desc += desc[0];
        ਚੋਣ (desc[1]) {
        ਹਾਲਤ USB_DT_ENDPOINT:
            ep = (ਖਾਲੀ*)desc;
            ਰੋਕੋ;
        ਹਾਲਤ USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            ਜਾਓ fail;
        ਹਾਲਤ 0x24:
            ਚੋਣ (desc[2]) {
            ਹਾਲਤ UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                ਰੋਕੋ;
            ਹਾਲਤ UAS_PIPE_ID_ਸਥਿਤੀ:
                ਸਥਿਤੀ = usb_alloc_pipe(usbdev, ep);
                ਰੋਕੋ;
            ਹਾਲਤ UAS_PIPE_ID_ਅੰਕੜੇ_ਵਿੱਚ:
                ਅੰਕੜੇ_ਵਿੱਚ = usb_alloc_pipe(usbdev, ep);
                ਰੋਕੋ;
            ਹਾਲਤ UAS_PIPE_ID_ਅੰਕੜੇ_ਬਾਹਰਲਾ:
                ਅੰਕੜੇ_ਬਾਹਰਲਾ = usb_alloc_pipe(usbdev, ep);
                ਰੋਕੋ;
            ਮੂਲ:
                ਜਾਓ fail;
            }
            ਰੋਕੋ;
        ਮੂਲ:
            desc = ਨਿਰਾ;
            ਰੋਕੋ;
        }
    }
    ਜੇ (!command || !ਸਥਿਤੀ || !ਅੰਕੜੇ_ਵਿੱਚ || !ਅੰਕੜੇ_ਬਾਹਰਲਾ)
        ਜਾਓ fail;

    ਬਣਤਰ uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, ਸਥਿਤੀ, ਅੰਕੜੇ_ਵਿੱਚ, ਅੰਕੜੇ_ਬਾਹਰਲਾ, 0);
    ਪੂਰਨ ret = scsi_rep_luns_ਜਾਂਚ(&lun0.drive, uas_ਜੋੜੋ_lun);
    ਜੇ (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        ਜਾਓ fail;
    }

    ਵਾਪਸ 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, ਸਥਿਤੀ);
    usb_free_pipe(usbdev, ਅੰਕੜੇ_ਵਿੱਚ);
    usb_free_pipe(usbdev, ਅੰਕੜੇ_ਬਾਹਰਲਾ);
    ਵਾਪਸ -1;
}
