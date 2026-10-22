// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // DTYPE_USB
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // cdb_read
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_USB_MSC
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // struct usb_s
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-msc.h" // usb_msc_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // bootprio_find_usb

ਬਣਤਰ usbdrive_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ usb_pipe *bulkin, *bulkout;
    ਪੂਰਨ lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USB_CDB_ਮਾਪ 12

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBW_SIGNATURE 0x43425355 // USBC

ਬਣਤਰ cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 d_CBWData_Transfer_ਲੰਮਾਈ;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CSW_SIGNATURE 0x53425355 // USBS

ਬਣਤਰ csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

ਸਥਿਰ ਪੂਰਨ
usb_msc_send(ਬਣਤਰ usbdrive_s *udrive_gf, ਪੂਰਨ dir, ਖਾਲੀ *buf, u32 ਬਾਈਟਾਂ)
{
    ਬਣਤਰ usb_pipe *pipe;
    ਜੇ (dir == USB_DIR_ਬਾਹਰਲਾ)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    ਨਹੀਂਤਾਂ
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    ਵਾਪਸ usb_send_bulk(pipe, dir, buf, ਬਾਈਟਾਂ);
}

// Low-level usb command transmit function.
ਪੂਰਨ
usb_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_USB_MSC)
        ਵਾਪਸ 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->ਗਿਣਤੀ_2, op->buf_fl);
    ਬਣਤਰ usbdrive_s *udrive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ usbdrive_s, drive);

    // Setup command block wrapper.
    ਬਣਤਰ cbw_s cbw;
    memset(&cbw, 0, ਆਕਾਰ(cbw));
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_ਮਾਪ);
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);
    u32 ਬਾਈਟਾਂ = blocksize * op->ਗਿਣਤੀ_2;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.d_CBWData_Transfer_ਲੰਮਾਈ = ਬਾਈਟਾਂ;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_ਵਿੱਚ : USB_DIR_ਬਾਹਰਲਾ;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_ਮਾਪ;

    // Transfer cbw to device.
    ਪੂਰਨ ret = usb_msc_send(udrive_gf, USB_DIR_ਬਾਹਰਲਾ
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), ਆਕਾਰ(cbw));
    ਜੇ (ret)
        ਜਾਓ fail;

    // Transfer data to/from device.
    ਜੇ (ਬਾਈਟਾਂ) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, ਬਾਈਟਾਂ);
        ਜੇ (ret)
            ਜਾਓ fail;
    }

    // Transfer csw info.
    ਬਣਤਰ csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_ਵਿੱਚ
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), ਆਕਾਰ(csw));
    ਜੇ (ret)
        ਜਾਓ fail;

    ਜੇ (!csw.bCSWStatus)
        ਵਾਪਸ DISK_RET_SUCCESS;
    ਜੇ (csw.bCSWStatus == 2)
        ਜਾਓ fail;

    ਜੇ (blocksize)
        op->ਗਿਣਤੀ_2 -= csw.dCSWDataResidue / blocksize;
    ਵਾਪਸ DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    ਵਾਪਸ DISK_RET_EBADTRACK;
}

ਸਥਿਰ ਪੂਰਨ
usb_msc_maxlun(ਬਣਤਰ usb_pipe *pipe)
{
    ਬਣਤਰ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_ਵਿੱਚ | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_ਮੁੱਲ = 0;
    req.w_ਸੂਚਕਾਂਕ = 0;
    req.w_ਲੰਮਾਈ = 1;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ maxlun;
    ਪੂਰਨ ret = usb_send_default_control(pipe, &req, &maxlun);
    ਜੇ (ret)
        ਵਾਪਸ 0;
    ਵਾਪਸ maxlun;
}

ਸਥਿਰ ਪੂਰਨ
usb_msc_lun_ਤਿਆਰੀ(ਬਣਤਰ usb_pipe *inpipe, ਬਣਤਰ usb_pipe *outpipe,
                  ਬਣਤਰ usbdevice_s *usbdev, ਪੂਰਨ lun)
{
    // Allocate drive structure.
    ਬਣਤਰ usbdrive_s *drive = malloc_fseg(ਆਕਾਰ(*drive));
    ਜੇ (!drive) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    memset(drive, 0, ਆਕਾਰ(*drive));
    ਜੇ (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    ਨਹੀਂਤਾਂ
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_usb(usbdev, lun);
    ਪੂਰਨ ret = scsi_drive_ਤਿਆਰੀ(&drive->drive, "USB MSC", prio);
    ਜੇ (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        ਮੁਕਤ(drive);
        ਵਾਪਸ -1;
    }
    ਵਾਪਸ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
ਪੂਰਨ
usb_msc_ਤਿਆਰੀ(ਬਣਤਰ usbdevice_s *usbdev)
{
    ਜੇ (!CONFIG_USB_MSC)
        ਵਾਪਸ -1;

    // Verify right kind of device
    ਬਣਤਰ usb_interface_descriptor *iface = usbdev->iface;
    ਜੇ ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ਵਾਪਸ -1;
    }

    // Find bulk in and bulk out endpoints.
    ਬਣਤਰ usb_pipe *inpipe = ਨਿਰਾ, *outpipe = ਨਿਰਾ;
    ਬਣਤਰ usb_endpoint_descriptor *indesc = usb_ਲੱਭੋ_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_ਵਿੱਚ);
    ਬਣਤਰ usb_endpoint_descriptor *outdesc = usb_ਲੱਭੋ_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_ਬਾਹਰਲਾ);
    ਜੇ (!indesc || !outdesc)
        ਜਾਓ fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    ਜੇ (!inpipe || !outpipe)
        ਜਾਓ fail;

    ਪੂਰਨ maxlun = usb_msc_maxlun(usbdev->defpipe);
    ਪੂਰਨ lun, pipesused = 0;
    ਲਈ (lun = 0; lun < maxlun + 1; lun++) {
        ਪੂਰਨ ret = usb_msc_lun_ਤਿਆਰੀ(inpipe, outpipe, usbdev, lun);
        ਜੇ (!ret)
            pipesused = 1;
    }

    ਜੇ (!pipesused)
        ਜਾਓ fail;

    ਵਾਪਸ 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    ਵਾਪਸ -1;
}
