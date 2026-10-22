// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // DTYPE_USB
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // cdb_read
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_USB_MSC
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-msc.h" // usb_msc_setup
#ಅನ್ತರ್ಭಾವ "util.h" // bootprio_find_usb

ರಚನೆ usbdrive_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ usb_pipe *bulkin, *bulkout;
    ಪೂರ್ಣಾಂಕ lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು USB_CDB_ಗಾತ್ರ 12

#ವ್ಯಾಖ್ಯಾನಿಸು CBW_SIGNATURE 0x43425355 // USBC

ರಚನೆ cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 d_CBWData_Transfer_ಉದ್ದಳತೆ;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು CSW_SIGNATURE 0x53425355 // USBS

ರಚನೆ csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_msc_send(ರಚನೆ usbdrive_s *udrive_gf, ಪೂರ್ಣಾಂಕ dir, ಶೂನ್ಯ *buf, u32 ಬೈಟುಗಳು)
{
    ರಚನೆ usb_pipe *pipe;
    ಆದರೆ (dir == USB_DIR_ಹೊರಹೋಗು)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    ಇಲ್ಲದಿದ್ದರೆ
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    ಹಿಂತಿರುಗಿ usb_send_bulk(pipe, dir, buf, ಬೈಟುಗಳು);
}

// Low-level usb command transmit function.
ಪೂರ್ಣಾಂಕ
usb_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_USB_MSC)
        ಹಿಂತಿರುಗಿ 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->ಎಣಿಕೆ_2, op->buf_fl);
    ರಚನೆ usbdrive_s *udrive_gf = container_ದ(
        op->drive_fl, ರಚನೆ usbdrive_s, drive);

    // Setup command block wrapper.
    ರಚನೆ cbw_s cbw;
    memset(&cbw, 0, ಗಾತ್ರ(cbw));
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_ಗಾತ್ರ);
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);
    u32 ಬೈಟುಗಳು = blocksize * op->ಎಣಿಕೆ_2;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.d_CBWData_Transfer_ಉದ್ದಳತೆ = ಬೈಟುಗಳು;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_ಒಳ : USB_DIR_ಹೊರಹೋಗು;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_ಗಾತ್ರ;

    // Transfer cbw to device.
    ಪೂರ್ಣಾಂಕ ret = usb_msc_send(udrive_gf, USB_DIR_ಹೊರಹೋಗು
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), ಗಾತ್ರ(cbw));
    ಆದರೆ (ret)
        ಹೋಗು fail;

    // Transfer data to/from device.
    ಆದರೆ (ಬೈಟುಗಳು) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, ಬೈಟುಗಳು);
        ಆದರೆ (ret)
            ಹೋಗು fail;
    }

    // Transfer csw info.
    ರಚನೆ csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_ಒಳ
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), ಗಾತ್ರ(csw));
    ಆದರೆ (ret)
        ಹೋಗು fail;

    ಆದರೆ (!csw.bCSWStatus)
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
    ಆದರೆ (csw.bCSWStatus == 2)
        ಹೋಗು fail;

    ಆದರೆ (blocksize)
        op->ಎಣಿಕೆ_2 -= csw.dCSWDataResidue / blocksize;
    ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_msc_maxlun(ರಚನೆ usb_pipe *pipe)
{
    ರಚನೆ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ಒಳ | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_ಮೌಲ್ಯ = 0;
    req.w_ಸೂಚ್ಯಂಕ = 0;
    req.w_ಉದ್ದಳತೆ = 1;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ maxlun;
    ಪೂರ್ಣಾಂಕ ret = usb_send_default_control(pipe, &req, &maxlun);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ maxlun;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
usb_msc_lun_ಸಿದ್ಧತೆ(ರಚನೆ usb_pipe *inpipe, ರಚನೆ usb_pipe *outpipe,
                  ರಚನೆ usbdevice_s *usbdev, ಪೂರ್ಣಾಂಕ lun)
{
    // Allocate drive structure.
    ರಚನೆ usbdrive_s *drive = malloc_fseg(ಗಾತ್ರ(*drive));
    ಆದರೆ (!drive) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    memset(drive, 0, ಗಾತ್ರ(*drive));
    ಆದರೆ (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    ಇಲ್ಲದಿದ್ದರೆ
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_usb(usbdev, lun);
    ಪೂರ್ಣಾಂಕ ret = scsi_drive_ಸಿದ್ಧತೆ(&drive->drive, "USB MSC", prio);
    ಆದರೆ (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        ಬಿಡುಗಡೆ(drive);
        ಹಿಂತಿರುಗಿ -1;
    }
    ಹಿಂತಿರುಗಿ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
ಪೂರ್ಣಾಂಕ
usb_msc_ಸಿದ್ಧತೆ(ರಚನೆ usbdevice_s *usbdev)
{
    ಆದರೆ (!CONFIG_USB_MSC)
        ಹಿಂತಿರುಗಿ -1;

    // Verify right kind of device
    ರಚನೆ usb_interface_descriptor *iface = usbdev->iface;
    ಆದರೆ ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ಹಿಂತಿರುಗಿ -1;
    }

    // Find bulk in and bulk out endpoints.
    ರಚನೆ usb_pipe *inpipe = ಖಾಲಿ, *outpipe = ಖಾಲಿ;
    ರಚನೆ usb_endpoint_descriptor *indesc = usb_ಕಂಡುಹಿಡಿ_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_ಒಳ);
    ರಚನೆ usb_endpoint_descriptor *outdesc = usb_ಕಂಡುಹಿಡಿ_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_ಹೊರಹೋಗು);
    ಆದರೆ (!indesc || !outdesc)
        ಹೋಗು fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    ಆದರೆ (!inpipe || !outpipe)
        ಹೋಗು fail;

    ಪೂರ್ಣಾಂಕ maxlun = usb_msc_maxlun(usbdev->defpipe);
    ಪೂರ್ಣಾಂಕ lun, pipesused = 0;
    ಗಾಗಿ (lun = 0; lun < maxlun + 1; lun++) {
        ಪೂರ್ಣಾಂಕ ret = usb_msc_lun_ಸಿದ್ಧತೆ(inpipe, outpipe, usbdev, lun);
        ಆದರೆ (!ret)
            pipesused = 1;
    }

    ಆದರೆ (!pipesused)
        ಹೋಗು fail;

    ಹಿಂತಿರುಗಿ 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    ಹಿಂತಿರುಗಿ -1;
}
