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

#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "block.h" // DTYPE_USB
#అన్తర్భావ "blockcmd.h" // cdb_read
#అన్తర్భావ "config.h" // CONFIG_USB_UAS
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "usb.h" // struct usb_s
#అన్తర్భావ "usb-uas.h" // usb_uas_init
#అన్తర్భావ "util.h" // bootprio_find_usb

#నిర్వచించు UAS_UI_COMMAND              0x01
#నిర్వచించు UAS_UI_SENSE                0x03
#నిర్వచించు UAS_UI_RESPONSE             0x04
#నిర్వచించు UAS_UI_TASK_MGMT            0x05
#నిర్వచించు UAS_UI_READ_READY           0x06
#నిర్వచించు UAS_UI_WRITE_READY          0x07

#నిర్వచించు UAS_PIPE_ID_COMMAND         0x01
#నిర్వచించు UAS_PIPE_ID_స్థితి          0x02
#నిర్వచించు UAS_PIPE_ID_దత్తాంశం_లోపల         0x03
#నిర్వచించు UAS_PIPE_ID_దత్తాంశం_బహిర్గతం        0x04

రకనిర్వచనం నిర్మాణం {
    u8    id;
    u8    reserved;
    u16   ట్యాగ్;
} PACKED  uas_ui_శీర్షిక;

రకనిర్వచనం నిర్మాణం {
    u8    prio_taskattr;   /* 6:3 priority, 2:0 task attribute   */
    u8    reserved_1;
    u8    కూడు_cdb_నిడివి;  /* 7:2 additional adb length (dwords) */
    u8    reserved_2;
    u8    lun[8];
    u8    cdb[16];
    u8    కూడు_cdb[];
} PACKED  uas_ui_command;

రకనిర్వచనం నిర్మాణం {
    u16   స్థితి_qualifier;
    u8    స్థితి_2;
    u8    reserved[7];
    u16   sense_నిడివి;
    u8    sense_దత్తాంశం[18];
} PACKED  uas_ui_sense;

రకనిర్వచనం నిర్మాణం {
    u16   కూడు_response_info;
    u8    response_సంకేతం;
} PACKED  uas_ui_response;

రకనిర్వచనం నిర్మాణం {
    u8    function;
    u8    reserved;
    u16   task_ట్యాగ్;
    u8    lun[8];
} PACKED  uas_ui_task_mgmt;

రకనిర్వచనం నిర్మాణం {
    uas_ui_శీర్షిక  hdr;
    సంఘం {
        uas_ui_command   command;
        uas_ui_sense     sense;
        uas_ui_task_mgmt task;
        uas_ui_response  response;
    };
} PACKED  uas_ui;

నిర్మాణం uasdrive_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం usbdevice_s *usbdev;
    నిర్మాణం usb_pipe *command, *స్థితి_2, *దత్తాంశం_లోపల, *దత్తాంశం_బహిర్గతం;
    u32 lun;
};

పూర్ణాంకం
uas_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_USB_UAS)
        తిరిగి DISK_RET_EBADTRACK;

    నిర్మాణం uasdrive_s *drive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం uasdrive_s, drive);

    uas_ui ui;
    memset(&ui, 0, పరిమాణం(ui));
    ui.hdr.id = UAS_UI_COMMAND;
    ui.hdr.ట్యాగ్ = 0xdead;
    ui.command.lun[1] = GET_GLOBALFLAT(drive_gf->lun);
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, ui.command.cdb, పరిమాణం(ui.command.cdb));
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);
    పూర్ణాంకం ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->command),
                            USB_DIR_బహిర్గతం, MAKE_FLATPTR(GET_SEG(SS), &ui),
                            పరిమాణం(ui.hdr) + పరిమాణం(ui.command));
    ఒకవేళ (ret) {
        dprintf(1, "uas: command send fail");
        వెళ్ళు fail;
    }

    memset(&ui, 0xff, పరిమాణం(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->స్థితి_2),
                        USB_DIR_లోపల, MAKE_FLATPTR(GET_SEG(SS), &ui), పరిమాణం(ui));
    ఒకవేళ (ret) {
        dprintf(1, "uas: status recv fail");
        వెళ్ళు fail;
    }

    ఎంపిక (ui.hdr.id) {
    స్థితి UAS_UI_SENSE:
        వెళ్ళు have_sense;
    స్థితి UAS_UI_READ_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->దత్తాంశం_లోపల),
                            USB_DIR_లోపల, op->buf_fl, op->లెక్క * blocksize);
        ఒకవేళ (ret) {
            dprintf(1, "uas: data read fail");
            వెళ్ళు fail;
        }
        ఆపు;
    స్థితి UAS_UI_WRITE_READY:
        ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->దత్తాంశం_బహిర్గతం),
                            USB_DIR_బహిర్గతం, op->buf_fl, op->లెక్క * blocksize);
        ఒకవేళ (ret) {
            dprintf(1, "uas: data write fail");
            వెళ్ళు fail;
        }
        ఆపు;
    అప్రమేయ:
        dprintf(1, "uas: unknown status ui id %d", ui.hdr.id);
        వెళ్ళు fail;
    }

    memset(&ui, 0xff, పరిమాణం(ui));
    ret = usb_send_bulk(GET_GLOBALFLAT(drive_gf->స్థితి_2),
                        USB_DIR_లోపల, MAKE_FLATPTR(GET_SEG(SS), &ui), పరిమాణం(ui));
    ఒకవేళ (ret) {
        dprintf(1, "uas: status recv fail");
        వెళ్ళు fail;
    }
    ఒకవేళ (ui.hdr.id != UAS_UI_SENSE) {
        dprintf(1, "uas: expected sense ui, got ui id %d", ui.hdr.id);
        వెళ్ళు fail;
    }

have_sense:
    ఒకవేళ (ui.sense.స్థితి_2 == 0) {
        తిరిగి DISK_RET_SUCCESS;
    }

fail:
    తిరిగి DISK_RET_EBADTRACK;
}

స్థిర శూన్య
uas_init_lun(నిర్మాణం uasdrive_s *drive, నిర్మాణం usbdevice_s *usbdev,
             నిర్మాణం usb_pipe *command, నిర్మాణం usb_pipe *స్థితి_2,
             నిర్మాణం usb_pipe *దత్తాంశం_లోపల, నిర్మాణం usb_pipe *దత్తాంశం_బహిర్గతం,
             u32 lun)
{
    memset(drive, 0, పరిమాణం(*drive));
    ఒకవేళ (usb_32bit_pipe(దత్తాంశం_లోపల))
        drive->drive.type = DTYPE_UAS_32;
    లేకపోతే
        drive->drive.type = DTYPE_UAS;
    drive->usbdev = usbdev;
    drive->command = command;
    drive->స్థితి_2 = స్థితి_2;
    drive->దత్తాంశం_లోపల = దత్తాంశం_లోపల;
    drive->దత్తాంశం_బహిర్గతం = దత్తాంశం_బహిర్గతం;
    drive->lun = lun;
}

స్థిర పూర్ణాంకం
uas_కూడు_lun(u32 lun, నిర్మాణం drive_s *tmpl_drv)
{
    నిర్మాణం uasdrive_s *tmpl_lun =
        container_యొక్క(tmpl_drv, నిర్మాణం uasdrive_s, drive);
    నిర్మాణం uasdrive_s *drive = malloc_fseg(పరిమాణం(*drive));
    ఒకవేళ (!drive) {
        warn_noalloc();
        తిరిగి -1;
    }
    uas_init_lun(drive, tmpl_lun->usbdev,
                 tmpl_lun->command, tmpl_lun->స్థితి_2,
                 tmpl_lun->దత్తాంశం_లోపల, tmpl_lun->దత్తాంశం_బహిర్గతం,
                 lun);

    పూర్ణాంకం prio = bootprio_కనుగొను_usb(drive->usbdev, drive->lun);
    పూర్ణాంకం ret = scsi_drive_అమరిక(&drive->drive, "USB UAS", prio);
    ఒకవేళ (ret) {
        విడుదల(drive);
        తిరిగి -1;
    }
    తిరిగి 0;
}

పూర్ణాంకం
usb_uas_అమరిక(నిర్మాణం usbdevice_s *usbdev)
{
    ఒకవేళ (!CONFIG_USB_UAS)
        తిరిగి -1;

    // Verify right kind of device
    నిర్మాణం usb_interface_descriptor *iface = usbdev->iface;
    ఒకవేళ (iface->bInterfaceSubClass != US_SC_SCSI ||
        iface->bInterfaceProtocol != US_PR_UAS) {
        dprintf(1, "Unsupported UAS device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        తిరిగి -1;
    }

    /* find & allocate pipes */
    నిర్మాణం usb_endpoint_descriptor *ep = ఖాళీ;
    నిర్మాణం usb_pipe *command = ఖాళీ;
    నిర్మాణం usb_pipe *స్థితి_2 = ఖాళీ;
    నిర్మాణం usb_pipe *దత్తాంశం_లోపల = ఖాళీ;
    నిర్మాణం usb_pipe *దత్తాంశం_బహిర్గతం = ఖాళీ;
    u8 *desc = (u8*)iface;
    అయితే (desc) {
        desc += desc[0];
        ఎంపిక (desc[1]) {
        స్థితి USB_DT_ENDPOINT:
            ep = (శూన్య*)desc;
            ఆపు;
        స్థితి USB_DT_ENDPOINT_COMPANION:
            /* No support (yet) for usb3 streams */
            dprintf(1, "Superspeed UAS devices not supported (yet)\n");
            వెళ్ళు fail;
        స్థితి 0x24:
            ఎంపిక (desc[2]) {
            స్థితి UAS_PIPE_ID_COMMAND:
                command = usb_alloc_pipe(usbdev, ep);
                ఆపు;
            స్థితి UAS_PIPE_ID_స్థితి:
                స్థితి_2 = usb_alloc_pipe(usbdev, ep);
                ఆపు;
            స్థితి UAS_PIPE_ID_దత్తాంశం_లోపల:
                దత్తాంశం_లోపల = usb_alloc_pipe(usbdev, ep);
                ఆపు;
            స్థితి UAS_PIPE_ID_దత్తాంశం_బహిర్గతం:
                దత్తాంశం_బహిర్గతం = usb_alloc_pipe(usbdev, ep);
                ఆపు;
            అప్రమేయ:
                వెళ్ళు fail;
            }
            ఆపు;
        అప్రమేయ:
            desc = ఖాళీ;
            ఆపు;
        }
    }
    ఒకవేళ (!command || !స్థితి_2 || !దత్తాంశం_లోపల || !దత్తాంశం_బహిర్గతం)
        వెళ్ళు fail;

    నిర్మాణం uasdrive_s lun0;
    uas_init_lun(&lun0, usbdev, command, స్థితి_2, దత్తాంశం_లోపల, దత్తాంశం_బహిర్గతం, 0);
    పూర్ణాంకం ret = scsi_rep_luns_పరిశీలన(&lun0.drive, uas_కూడు_lun);
    ఒకవేళ (ret <= 0) {
        dprintf(1, "Unable to configure UAS drive.\n");
        వెళ్ళు fail;
    }

    తిరిగి 0;

fail:
    usb_free_pipe(usbdev, command);
    usb_free_pipe(usbdev, స్థితి_2);
    usb_free_pipe(usbdev, దత్తాంశం_లోపల);
    usb_free_pipe(usbdev, దత్తాంశం_బహిర్గతం);
    తిరిగి -1;
}
