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

#अन्तर्भाव "biosvar.h" // GET_GLOBALFLAT
#अन्तर्भाव "block.h" // DTYPE_USB
#अन्तर्भाव "blockcmd.h" // cdb_read
#अन्तर्भाव "config.h" // CONFIG_USB_UAS
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "usb.h" // struct usb_s
#अन्तर्भाव "usb-uas.h" // usb_uas_init
#अन्तर्भाव "util.h" // bootprio_find_usb

#परिभाषय UAS_UI_COMMAND              0x01
#परिभाषय UAS_UI_SENSE                0x03
#परिभाषय UAS_UI_RESPONSE             0x04
#परिभाषय UAS_UI_TASK_MGMT            0x05
#परिभाषय UAS_UI_READ_READY           0x06
#परिभाषय UAS_UI_WRITE_READY          0x07

#परिभाषय UAS_PIPE_ID_COMMAND         0x01
#परिभाषय UAS_PIPE_ID_स्थितिः          0x02
#परिभाषय UAS_PIPE_ID_दत्त_अन्तर्         0x03
#परिभाषय UAS_PIPE_ID_दत्त_बहिर्गमनम्        0x04

प्रकारनिर्देश संरचना {
    u8    id;
    u8    reserved;
    u16   चिह्नकम्;
} PACKED  uas_ui_शीर्षकम्;

प्रकारनिर्देश संरचना {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    सङ्कलय_cdb_दीर्घता;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    सङ्कलय_cdb[];
} PACKED  uas_ui_command;

प्रकारनिर्देश संरचना {
    u16   स्थितिः_qualifier;
    u8    स्थितिः;
    u8    reserved[7];
    u16   sense_दीर्घता;
    u8    sense_दत्त[18];
} PACKED  uas_ui_sense;

प्रकारनिर्देश संरचना {
    u16   सङ्कलय_response_info;
    u8    response_संहिता;
} PACKED  uas_ui_response;

प्रकारनिर्देश संरचना {
    u8    function;
    u8    reserved;
    u16   task_चिह्नकम्;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

प्रकारनिर्देश संरचना {
    uas_ui_शीर्षकम्  hdr;
    सङ्घ {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

संरचना uasdrive_s {
    संरचना drive_s drive;
    संरचना usbdevice_s *usbdev;
    संरचना usb_pipe *command, *स्थितिः, *दत्त_अन्तर्, *दत्त_बहिर्गमनम्;
    u32 lun;
};

पूर्णाङ्क
uas_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_USB_UAS)
        प्रत्यागम DISK_RET_EBADTRACK;

    संरचना uasdrive_s *drive_gf = container_सम्बन्धि(
        op->drive_fl, संरचना uasdrive_s, drive);

    uas_ui ui;
    स्मृतिं_पूरय(&ui, 0, परिमाण(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.चिह्नकम् = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, ui.command.cdb, परिमाण(ui.command.cdb));
    यदि (blocksize < 0)
        प्रत्यागम default_process_op(op);
    पूर्णाङ्क ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_बहिर्गमनम्, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            परिमाण(ui.hdr) + परिमाण(ui.command));
    यदि (ret) {
        dprintf(1, "uas: command send fail");
        गच्छ fail;
    }

    स्मृतिं_पूरय(&ui, 0xff, परिमाण(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->स्थितिः),
                        USB_DIR_अन्तर्, MAKE_FLATPTR(GET_SEG(SS), &ui), परिमाण(ui));
    यदि (ret) {
        dprintf(1, "uas: status recv fail");
        गच्छ fail;
    }

    विकल्प (ui.hdr.id) {
    पक्ष UAS_UI_SENSE:
        गच्छ have_sense;
    पक्ष UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->दत्त_अन्तर्),
                            USB_DIR_अन्तर्, op->buf_fl, op->गणनम् * blocksize);
        यदि (ret) {
            dprintf(1, "uas: data read fail");
            गच्छ fail;
        }
        भङ्ग;
    पक्ष UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->दत्त_बहिर्गमनम्),
                            USB_DIR_बहिर्गमनम्, op->buf_fl, op->गणनम् * blocksize);
        यदि (ret) {
            dprintf(1, "uas: data write fail");
            गच्छ fail;
        }
        भङ्ग;
    स्वभाव:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        गच्छ fail;
    }

    स्मृतिं_पूरय(&ui, 0xff, परिमाण(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->स्थितिः),
                        USB_DIR_अन्तर्, MAKE_FLATPTR(GET_SEG(SS), &ui), परिमाण(ui));
    यदि (ret) {
        dprintf(1, "uas: status recv fail");
        गच्छ fail;
    }
    यदि (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        गच्छ fail;
    }

have_sense:
    यदि (ui.sense.स्थितिः == 0) {
        प्रत्यागम DISK_RET_SUCCESS;
    }

fail:
    प्रत्यागम DISK_RET_EBADTRACK;
}

स्थिर शून्य
uas_init_lun(संरचना uasdrive_s *drive, संरचना usbdevice_s *usbdev,
             संरचना usb_pipe *command, संरचना usb_pipe *स्थितिः,
             संरचना usb_pipe *दत्त_अन्तर्, संरचना usb_pipe *दत्त_बहिर्गमनम्,
             u32 lun)
{
    स्मृतिं_पूरय(drive, 0, परिमाण(*drive));
    यदि (usb_32bit_pipe(दत्त_अन्तर्))
        drive->drive.type = DTYPE_UAS_32;
    अन्यथा
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->स्थितिः = स्थितिः;
    drive->दत्त_अन्तर् = दत्त_अन्तर्;
    drive->दत्त_बहिर्गमनम् = दत्त_बहिर्गमनम्;
    drive->lun = lun;
}

स्थिर पूर्णाङ्क
uas_सङ्कलय_lun(u32 lun, संरचना drive_s *tmpl_drv)
{
    संरचना uasdrive_s *tmpl_lun =
        container_सम्बन्धि(tmpl_drv, संरचना uasdrive_s, drive);
    संरचना uasdrive_s *drive = malloc_fseg(परिमाण(*drive));
    यदि (!drive) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->स्थितिः,
                 tmpl_lun->दत्त_अन्तर्, tmpl_lun->दत्त_बहिर्गमनम्,
                 lun);

    पूर्णाङ्क prio = bootprio_find_usb(drive->usbdev, drive->lun);
    पूर्णाङ्क ret = scsi_drive_सज्जता(&drive->drive, "USB UAS", prio);
    यदि (ret) {
        मुक्त(drive);
        प्रत्यागम -1;
    }
    प्रत्यागम 0;
}

पूर्णाङ्क
usb_uas_सज्जता(संरचना usbdevice_s *usbdev)
{
    यदि (!CONFIG_USB_UAS)
        प्रत्यागम -1;

    // Verify right kind of device
    संरचना usb_interface_descriptor *iface = usbdev->iface;
    यदि (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        प्रत्यागम -1;
    }

    /* find & allocate pipes */
    संरचना usb_endpoint_descriptor *ep = रिक्त;
    संरचना usb_pipe *command = रिक्त;
    संरचना usb_pipe *स्थितिः = रिक्त;
    संरचना usb_pipe *दत्त_अन्तर् = रिक्त;
    संरचना usb_pipe *दत्त_बहिर्गमनम् = रिक्त;
    u8 *desc = (u8*)iface;
    यावत् (desc) {
        desc += desc[0];
        विकल्प (desc[1]) {
        पक्ष USB_DT_ENDPOINT:
            ep = (शून्य*)desc;
            भङ्ग;
        पक्ष USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            गच्छ fail;
        पक्ष 0x24:
            विकल्प (desc[2]) {
            पक्ष UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                भङ्ग;
            पक्ष UAS_PIPE_ID_स्थितिः:
                स्थितिः = usb_alloc_pipe(usbdev, ep);
                भङ्ग;
            पक्ष UAS_PIPE_ID_दत्त_अन्तर्:
                दत्त_अन्तर् = usb_alloc_pipe(usbdev, ep);
                भङ्ग;
            पक्ष UAS_PIPE_ID_दत्त_बहिर्गमनम्:
                दत्त_बहिर्गमनम् = usb_alloc_pipe(usbdev, ep);
                भङ्ग;
            स्वभाव:
                गच्छ fail;
            }
            भङ्ग;
        स्वभाव:
            desc = रिक्त;
            भङ्ग;
        }
    }
    यदि (!command || !स्थितिः || !दत्त_अन्तर् || !दत्त_बहिर्गमनम्)
        गच्छ fail;

    संरचना uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, स्थितिः, दत्त_अन्तर्, दत्त_बहिर्गमनम्, 0);
    पूर्णाङ्क ret = scsi_rep_luns_अवलोकनम्(&lun0.drive, uas_सङ्कलय_lun);
    यदि (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        गच्छ fail;
    }

    प्रत्यागम 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, स्थितिः);
    usb_free_pipe(usbdev, दत्त_अन्तर्);
    usb_free_pipe(usbdev, दत्त_बहिर्गमनम्);
    प्रत्यागम -1;
}
