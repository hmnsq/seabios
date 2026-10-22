// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "block.h" // DTYPE_USB
#అన్తర్భావ "blockcmd.h" // cdb_read
#అన్తర్భావ "config.h" // CONFIG_USB_MSC
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "usb.h" // struct usb_s
#అన్తర్భావ "usb-msc.h" // usb_msc_setup
#అన్తర్భావ "util.h" // bootprio_find_usb

నిర్మాణం usbdrive_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం usb_pipe *bulkin, *bulkout;
    పూర్ణాంకం lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#నిర్వచించు USB_CDB_కొలత 12

#నిర్వచించు CBW_SIGNATURE 0x43425355 // USBC

నిర్మాణం cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 d_CBWData_Transfer_నిడివి;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#నిర్వచించు CSW_SIGNATURE 0x53425355 // USBS

నిర్మాణం csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

స్థిర పూర్ణాంకం
usb_msc_send(నిర్మాణం usbdrive_s *udrive_gf, పూర్ణాంకం dir, శూన్య *buf, u32 బైట్లు)
{
    నిర్మాణం usb_pipe *pipe;
    ఒకవేళ (dir == USB_DIR_బహిర్గతం)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    లేకపోతే
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    తిరిగి usb_send_bulk(pipe, dir, buf, బైట్లు);
}

// Low-level usb command transmit function.
పూర్ణాంకం
usb_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_USB_MSC)
        తిరిగి 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->లెక్క, op->buf_fl);
    నిర్మాణం usbdrive_s *udrive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం usbdrive_s, drive);

    // Setup command block wrapper.
    నిర్మాణం cbw_s cbw;
    memset(&cbw, 0, పరిమాణం(cbw));
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_కొలత);
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);
    u32 బైట్లు = blocksize * op->లెక్క;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.d_CBWData_Transfer_నిడివి = బైట్లు;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_లోపల : USB_DIR_బహిర్గతం;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_కొలత;

    // Transfer cbw to device.
    పూర్ణాంకం ret = usb_msc_send(udrive_gf, USB_DIR_బహిర్గతం
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), పరిమాణం(cbw));
    ఒకవేళ (ret)
        వెళ్ళు fail;

    // Transfer data to/from device.
    ఒకవేళ (బైట్లు) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, బైట్లు);
        ఒకవేళ (ret)
            వెళ్ళు fail;
    }

    // Transfer csw info.
    నిర్మాణం csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_లోపల
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), పరిమాణం(csw));
    ఒకవేళ (ret)
        వెళ్ళు fail;

    ఒకవేళ (!csw.bCSWStatus)
        తిరిగి DISK_RET_SUCCESS;
    ఒకవేళ (csw.bCSWStatus == 2)
        వెళ్ళు fail;

    ఒకవేళ (blocksize)
        op->లెక్క -= csw.dCSWDataResidue / blocksize;
    తిరిగి DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    తిరిగి DISK_RET_EBADTRACK;
}

స్థిర పూర్ణాంకం
usb_msc_maxlun(నిర్మాణం usb_pipe *pipe)
{
    నిర్మాణం usb_ctrlrequest req;
    req.bRequestType = USB_DIR_లోపల | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_విలువ = 0;
    req.w_సూచీ = 0;
    req.w_నిడివి = 1;
    గుర్తులేని అక్షరం maxlun;
    పూర్ణాంకం ret = usb_send_default_control(pipe, &req, &maxlun);
    ఒకవేళ (ret)
        తిరిగి 0;
    తిరిగి maxlun;
}

స్థిర పూర్ణాంకం
usb_msc_lun_అమరిక(నిర్మాణం usb_pipe *inpipe, నిర్మాణం usb_pipe *outpipe,
                  నిర్మాణం usbdevice_s *usbdev, పూర్ణాంకం lun)
{
    // Allocate drive structure.
    నిర్మాణం usbdrive_s *drive = malloc_fseg(పరిమాణం(*drive));
    ఒకవేళ (!drive) {
        warn_noalloc();
        తిరిగి -1;
    }
    memset(drive, 0, పరిమాణం(*drive));
    ఒకవేళ (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    లేకపోతే
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    పూర్ణాంకం prio = bootprio_కనుగొను_usb(usbdev, lun);
    పూర్ణాంకం ret = scsi_drive_అమరిక(&drive->drive, "USB MSC", prio);
    ఒకవేళ (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        విడుదల(drive);
        తిరిగి -1;
    }
    తిరిగి 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
పూర్ణాంకం
usb_msc_అమరిక(నిర్మాణం usbdevice_s *usbdev)
{
    ఒకవేళ (!CONFIG_USB_MSC)
        తిరిగి -1;

    // Verify right kind of device
    నిర్మాణం usb_interface_descriptor *iface = usbdev->iface;
    ఒకవేళ ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        తిరిగి -1;
    }

    // Find bulk in and bulk out endpoints.
    నిర్మాణం usb_pipe *inpipe = ఖాళీ, *outpipe = ఖాళీ;
    నిర్మాణం usb_endpoint_descriptor *indesc = usb_కనుగొను_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_లోపల);
    నిర్మాణం usb_endpoint_descriptor *outdesc = usb_కనుగొను_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_బహిర్గతం);
    ఒకవేళ (!indesc || !outdesc)
        వెళ్ళు fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    ఒకవేళ (!inpipe || !outpipe)
        వెళ్ళు fail;

    పూర్ణాంకం maxlun = usb_msc_maxlun(usbdev->defpipe);
    పూర్ణాంకం lun, pipesused = 0;
    కొరకు (lun = 0; lun < maxlun + 1; lun++) {
        పూర్ణాంకం ret = usb_msc_lun_అమరిక(inpipe, outpipe, usbdev, lun);
        ఒకవేళ (!ret)
            pipesused = 1;
    }

    ఒకవేళ (!pipesused)
        వెళ్ళు fail;

    తిరిగి 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    తిరిగి -1;
}
