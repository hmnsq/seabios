// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBALFLAT
#अन्तर्भाव "block.h" // DTYPE_USB
#अन्तर्भाव "blockcmd.h" // cdb_read
#अन्तर्भाव "config.h" // CONFIG_USB_MSC
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "usb.h" // struct usb_s
#अन्तर्भाव "usb-msc.h" // usb_msc_setup
#अन्तर्भाव "util.h" // bootprio_find_usb

संरचना usbdrive_s {
    संरचना drive_s drive;
    संरचना usb_pipe *bulkin, *bulkout;
    पूर्णाङ्क lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

संरचना cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

संरचना csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

स्थिर पूर्णाङ्क
usb_msc_send(संरचना usbdrive_s *udrive_gf, पूर्णाङ्क dir, शून्य *buf, u32 bytes)
{
    संरचना usb_pipe *pipe;
    यदि (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    अन्यथा
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    प्रत्यागम usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
पूर्णाङ्क
usb_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_USB_MSC)
        प्रत्यागम 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->गिनती, op->buf_fl);
    संरचना usbdrive_s *udrive_gf = container_of(
        op->drive_fl, संरचना usbdrive_s, drive);

    // Setup command block wrapper.
    संरचना cbw_s cbw;
    स_रखो(&cbw, 0, परिमाण(cbw));
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    यदि (blocksize < 0)
        प्रत्यागम default_process_op(op);
    u32 bytes = blocksize * op->गिनती;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    पूर्णाङ्क ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), परिमाण(cbw));
    यदि (ret)
        गच्छ fail;

    // Transfer data to/from device.
    यदि (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        यदि (ret)
            गच्छ fail;
    }

    // Transfer csw info.
    संरचना csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), परिमाण(csw));
    यदि (ret)
        गच्छ fail;

    यदि (!csw.bCSWStatus)
        प्रत्यागम DISK_RET_SUCCESS;
    यदि (csw.bCSWStatus == 2)
        गच्छ fail;

    यदि (blocksize)
        op->गिनती -= csw.dCSWDataResidue / blocksize;
    प्रत्यागम DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    प्रत्यागम DISK_RET_EBADTRACK;
}

स्थिर पूर्णाङ्क
usb_msc_maxlun(संरचना usb_pipe *pipe)
{
    संरचना usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_मूल्य = 0;
    req.wIndex = 0;
    req.wLength = 1;
    अचिन्हित वर्ण maxlun;
    पूर्णाङ्क ret = usb_send_default_control(pipe, &req, &maxlun);
    यदि (ret)
        प्रत्यागम 0;
    प्रत्यागम maxlun;
}

स्थिर पूर्णाङ्क
usb_msc_lun_setup(संरचना usb_pipe *inpipe, संरचना usb_pipe *outpipe,
                  संरचना usbdevice_s *usbdev, पूर्णाङ्क lun)
{
    // Allocate drive structure.
    संरचना usbdrive_s *drive = malloc_fseg(परिमाण(*drive));
    यदि (!drive) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    स_रखो(drive, 0, परिमाण(*drive));
    यदि (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    अन्यथा
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    पूर्णाङ्क prio = bootprio_find_usb(usbdev, lun);
    पूर्णाङ्क ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    यदि (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        मुक्त(drive);
        प्रत्यागम -1;
    }
    प्रत्यागम 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
पूर्णाङ्क
usb_msc_setup(संरचना usbdevice_s *usbdev)
{
    यदि (!CONFIG_USB_MSC)
        प्रत्यागम -1;

    // Verify right kind of device
    संरचना usb_interface_descriptor *iface = usbdev->iface;
    यदि ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        प्रत्यागम -1;
    }

    // Find bulk in and bulk out endpoints.
    संरचना usb_pipe *inpipe = रिक्त, *outpipe = रिक्त;
    संरचना usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    संरचना usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    यदि (!indesc || !outdesc)
        गच्छ fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    यदि (!inpipe || !outpipe)
        गच्छ fail;

    पूर्णाङ्क maxlun = usb_msc_maxlun(usbdev->defpipe);
    पूर्णाङ्क lun, pipesused = 0;
    प्रति (lun = 0; lun < maxlun + 1; lun++) {
        पूर्णाङ्क ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        यदि (!ret)
            pipesused = 1;
    }

    यदि (!pipesused)
        गच्छ fail;

    प्रत्यागम 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    प्रत्यागम -1;
}
