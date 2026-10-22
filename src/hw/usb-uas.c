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

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // DTYPE_USB
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // cdb_read
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_USB_UAS
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-uas.h" // usb_uas_init
#ಅನ್ತರ್ಭಾವ "util.h" // bootprio_find_usb

#ವ್ಯಾಖ್ಯಾನಿಸು UAS_UI_COMMAND              0x01
#ವ್ಯಾಖ್ಯಾನಿಸು UAS_UI_SENSE                0x03
#ವ್ಯಾಖ್ಯಾನಿಸು UAS_UI_RESPONSE             0x04
#ವ್ಯಾಖ್ಯಾನಿಸು UAS_UI_TASK_MGMT            0x05
#ವ್ಯಾಖ್ಯಾನಿಸು UAS_UI_READ_READY           0x06
#ವ್ಯಾಖ್ಯಾನಿಸು UAS_UI_WRITE_READY          0x07

#ವ್ಯಾಖ್ಯಾನಿಸು UAS_PIPE_ID_COMMAND         0x01
#ವ್ಯಾಖ್ಯಾನಿಸು UAS_PIPE_ID_ಸ್ಥಿತಿ          0x02
#ವ್ಯಾಖ್ಯಾನಿಸು UAS_PIPE_ID_ದತ್ತಾಂಶ_ಒಳ         0x03
#ವ್ಯಾಖ್ಯಾನಿಸು UAS_PIPE_ID_ದತ್ತಾಂಶ_ಹೊರಹೋಗು        0x04

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    u8    id;
    u8    reserved;
    u16   ಟ್ಯಾಗ್;
} PACKED  uas_ui_ಶೀರ್ಷಿಕೆ;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    ಕೂಡಿಸು_cdb_ಉದ್ದಳತೆ;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    ಕೂಡಿಸು_cdb[];
} PACKED  uas_ui_command;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    u16   ಸ್ಥಿತಿ_qualifier;
    u8    ಸ್ಥಿತಿ_2;
    u8    reserved[7];
    u16   sense_ಉದ್ದಳತೆ;
    u8    sense_ದತ್ತಾಂಶ[18];
} PACKED  uas_ui_sense;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    u16   ಕೂಡಿಸು_response_info;
    u8    response_ಸಂಕೇತ;
} PACKED  uas_ui_response;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    u8    function;
    u8    reserved;
    u16   task_ಟ್ಯಾಗ್;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
    uas_ui_ಶೀರ್ಷಿಕೆ  hdr;
    ಒಕ್ಕೂಟ {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

ರಚನೆ uasdrive_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ usbdevice_s *usbdev;
    ರಚನೆ usb_pipe *command, *ಸ್ಥಿತಿ_2, *ದತ್ತಾಂಶ_ಒಳ, *ದತ್ತಾಂಶ_ಹೊರಹೋಗು;
    u32 lun;
};

ಪೂರ್ಣಾಂಕ
uas_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_USB_UAS)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;

    ರಚನೆ uasdrive_s *drive_gf = container_ದ(
        op->drive_fl, ರಚನೆ uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, ಗಾತ್ರ(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.ಟ್ಯಾಗ್ = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, ui.command.cdb, ಗಾತ್ರ(ui.command.cdb));
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);
    ಪೂರ್ಣಾಂಕ ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_ಹೊರಹೋಗು, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            ಗಾತ್ರ(ui.hdr) + ಗಾತ್ರ(ui.command));
    ಆದರೆ (ret) {
        dprintf(1, "uas: command send fail");
        ಹೋಗು fail;
    }

    memset(&ui, 0xff, ಗಾತ್ರ(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ಸ್ಥಿತಿ_2),
                        USB_DIR_ಒಳ, MAKE_FLATPTR(GET_SEG(SS), &ui), ಗಾತ್ರ(ui));
    ಆದರೆ (ret) {
        dprintf(1, "uas: status recv fail");
        ಹೋಗು fail;
    }

    ಆಯ್ಕೆ (ui.hdr.id) {
    ಸ್ಥಿತಿ UAS_UI_SENSE:
        ಹೋಗು have_sense;
    ಸ್ಥಿತಿ UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ದತ್ತಾಂಶ_ಒಳ),
                            USB_DIR_ಒಳ, op->buf_fl, op->ಎಣಿಕೆ_2 * blocksize);
        ಆದರೆ (ret) {
            dprintf(1, "uas: data read fail");
            ಹೋಗು fail;
        }
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ದತ್ತಾಂಶ_ಹೊರಹೋಗು),
                            USB_DIR_ಹೊರಹೋಗು, op->buf_fl, op->ಎಣಿಕೆ_2 * blocksize);
        ಆದರೆ (ret) {
            dprintf(1, "uas: data write fail");
            ಹೋಗು fail;
        }
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        ಹೋಗು fail;
    }

    memset(&ui, 0xff, ಗಾತ್ರ(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->ಸ್ಥಿತಿ_2),
                        USB_DIR_ಒಳ, MAKE_FLATPTR(GET_SEG(SS), &ui), ಗಾತ್ರ(ui));
    ಆದರೆ (ret) {
        dprintf(1, "uas: status recv fail");
        ಹೋಗು fail;
    }
    ಆದರೆ (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        ಹೋಗು fail;
    }

have_sense:
    ಆದರೆ (ui.sense.ಸ್ಥಿತಿ_2 == 0) {
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
    }

fail:
    ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಶೂನ್ಯ
uas_init_lun(ರಚನೆ uasdrive_s *drive, ರಚನೆ usbdevice_s *usbdev,
             ರಚನೆ usb_pipe *command, ರಚನೆ usb_pipe *ಸ್ಥಿತಿ_2,
             ರಚನೆ usb_pipe *ದತ್ತಾಂಶ_ಒಳ, ರಚನೆ usb_pipe *ದತ್ತಾಂಶ_ಹೊರಹೋಗು,
             u32 lun)
{
    memset(drive, 0, ಗಾತ್ರ(*drive));
    ಆದರೆ (usb_32bit_pipe(ದತ್ತಾಂಶ_ಒಳ))
        drive->drive.type = DTYPE_UAS_32;
    ಇಲ್ಲದಿದ್ದರೆ
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->ಸ್ಥಿತಿ_2 = ಸ್ಥಿತಿ_2;
    drive->ದತ್ತಾಂಶ_ಒಳ = ದತ್ತಾಂಶ_ಒಳ;
    drive->ದತ್ತಾಂಶ_ಹೊರಹೋಗು = ದತ್ತಾಂಶ_ಹೊರಹೋಗು;
    drive->lun = lun;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
uas_ಕೂಡಿಸು_lun(u32 lun, ರಚನೆ drive_s *tmpl_drv)
{
    ರಚನೆ uasdrive_s *tmpl_lun =
        container_ದ(tmpl_drv, ರಚನೆ uasdrive_s, drive);
    ರಚನೆ uasdrive_s *drive = malloc_fseg(ಗಾತ್ರ(*drive));
    ಆದರೆ (!drive) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->ಸ್ಥಿತಿ_2,
                 tmpl_lun->ದತ್ತಾಂಶ_ಒಳ, tmpl_lun->ದತ್ತಾಂಶ_ಹೊರಹೋಗು,
                 lun);

    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_usb(drive->usbdev, drive->lun);
    ಪೂರ್ಣಾಂಕ ret = scsi_drive_ಸಿದ್ಧತೆ(&drive->drive, "USB UAS", prio);
    ಆದರೆ (ret) {
        ಬಿಡುಗಡೆ(drive);
        ಹಿಂತಿರುಗಿ -1;
    }
    ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ
usb_uas_ಸಿದ್ಧತೆ(ರಚನೆ usbdevice_s *usbdev)
{
    ಆದರೆ (!CONFIG_USB_UAS)
        ಹಿಂತಿರುಗಿ -1;

    // Verify right kind of device
    ರಚನೆ usb_interface_descriptor *iface = usbdev->iface;
    ಆದರೆ (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ಹಿಂತಿರುಗಿ -1;
    }

    /* find & allocate pipes */
    ರಚನೆ usb_endpoint_descriptor *ep = ಖಾಲಿ;
    ರಚನೆ usb_pipe *command = ಖಾಲಿ;
    ರಚನೆ usb_pipe *ಸ್ಥಿತಿ_2 = ಖಾಲಿ;
    ರಚನೆ usb_pipe *ದತ್ತಾಂಶ_ಒಳ = ಖಾಲಿ;
    ರಚನೆ usb_pipe *ದತ್ತಾಂಶ_ಹೊರಹೋಗು = ಖಾಲಿ;
    u8 *desc = (u8*)iface;
    ಆಗ (desc) {
        desc += desc[0];
        ಆಯ್ಕೆ (desc[1]) {
        ಸ್ಥಿತಿ USB_DT_ENDPOINT:
            ep = (ಶೂನ್ಯ*)desc;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            ಹೋಗು fail;
        ಸ್ಥಿತಿ 0x24:
            ಆಯ್ಕೆ (desc[2]) {
            ಸ್ಥಿತಿ UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ UAS_PIPE_ID_ಸ್ಥಿತಿ:
                ಸ್ಥಿತಿ_2 = usb_alloc_pipe(usbdev, ep);
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ UAS_PIPE_ID_ದತ್ತಾಂಶ_ಒಳ:
                ದತ್ತಾಂಶ_ಒಳ = usb_alloc_pipe(usbdev, ep);
                ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ UAS_PIPE_ID_ದತ್ತಾಂಶ_ಹೊರಹೋಗು:
                ದತ್ತಾಂಶ_ಹೊರಹೋಗು = usb_alloc_pipe(usbdev, ep);
                ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:
                ಹೋಗು fail;
            }
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            desc = ಖಾಲಿ;
            ನಿಲ್ಲಿಸು;
        }
    }
    ಆದರೆ (!command || !ಸ್ಥಿತಿ_2 || !ದತ್ತಾಂಶ_ಒಳ || !ದತ್ತಾಂಶ_ಹೊರಹೋಗು)
        ಹೋಗು fail;

    ರಚನೆ uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, ಸ್ಥಿತಿ_2, ದತ್ತಾಂಶ_ಒಳ, ದತ್ತಾಂಶ_ಹೊರಹೋಗು, 0);
    ಪೂರ್ಣಾಂಕ ret = scsi_rep_luns_ಪರಿಶೀಲನೆ(&lun0.drive, uas_ಕೂಡಿಸು_lun);
    ಆದರೆ (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        ಹೋಗು fail;
    }

    ಹಿಂತಿರುಗಿ 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, ಸ್ಥಿತಿ_2);
    usb_free_pipe(usbdev, ದತ್ತಾಂಶ_ಒಳ);
    usb_free_pipe(usbdev, ದತ್ತಾಂಶ_ಹೊರಹೋಗು);
    ಹಿಂತಿರುಗಿ -1;
}
