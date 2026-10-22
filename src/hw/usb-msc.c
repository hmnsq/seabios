// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাব "block.h" // DTYPE_USB
#অন্তর্ভাব "blockcmd.h" // cdb_read
#অন্তর্ভাব "config.h" // CONFIG_USB_MSC
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "usb.h" // struct usb_s
#অন্তর্ভাব "usb-msc.h" // usb_msc_setup
#অন্তর্ভাব "util.h" // bootprio_find_usb

সংরচনা usbdrive_s {
    সংরচনা drive_s drive;
    সংরচনা usb_pipe *bulkin, *bulkout;
    পূর্ণাঙ্ক lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

সংরচনা cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

সংরচনা csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

স্থির পূর্ণাঙ্ক
usb_msc_send(সংরচনা usbdrive_s *udrive_gf, পূর্ণাঙ্ক dir, শূন্য *buf, u32 bytes)
{
    সংরচনা usb_pipe *pipe;
    যদি (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    অন্যথা
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    প্রত্যাগম usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
পূর্ণাঙ্ক
usb_process_op(সংরচনা disk_op_s *op)
{
    যদি (!CONFIG_USB_MSC)
        প্রত্যাগম 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->গুনতি, op->buf_fl);
    সংরচনা usbdrive_s *udrive_gf = container_of(
        op->drive_fl, সংরচনা usbdrive_s, drive);

    // Setup command block wrapper.
    সংরচনা cbw_s cbw;
    memset(&cbw, 0, পরিমাণ(cbw));
    পূর্ণাঙ্ক blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    যদি (blocksize < 0)
        প্রত্যাগম default_process_op(op);
    u32 bytes = blocksize * op->গুনতি;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    পূর্ণাঙ্ক ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), পরিমাণ(cbw));
    যদি (ret)
        গচ্ছ fail;

    // Transfer data to/from device.
    যদি (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        যদি (ret)
            গচ্ছ fail;
    }

    // Transfer csw info.
    সংরচনা csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), পরিমাণ(csw));
    যদি (ret)
        গচ্ছ fail;

    যদি (!csw.bCSWStatus)
        প্রত্যাগম DISK_RET_SUCCESS;
    যদি (csw.bCSWStatus == 2)
        গচ্ছ fail;

    যদি (blocksize)
        op->গুনতি -= csw.dCSWDataResidue / blocksize;
    প্রত্যাগম DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    প্রত্যাগম DISK_RET_EBADTRACK;
}

স্থির পূর্ণাঙ্ক
usb_msc_maxlun(সংরচনা usb_pipe *pipe)
{
    সংরচনা usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_মান = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned বর্ণ maxlun;
    পূর্ণাঙ্ক ret = usb_send_default_control(pipe, &req, &maxlun);
    যদি (ret)
        প্রত্যাগম 0;
    প্রত্যাগম maxlun;
}

স্থির পূর্ণাঙ্ক
usb_msc_lun_setup(সংরচনা usb_pipe *inpipe, সংরচনা usb_pipe *outpipe,
                  সংরচনা usbdevice_s *usbdev, পূর্ণাঙ্ক lun)
{
    // Allocate drive structure.
    সংরচনা usbdrive_s *drive = malloc_fseg(পরিমাণ(*drive));
    যদি (!drive) {
        warn_noalloc();
        প্রত্যাগম -1;
    }
    memset(drive, 0, পরিমাণ(*drive));
    যদি (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    অন্যথা
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    পূর্ণাঙ্ক prio = bootprio_find_usb(usbdev, lun);
    পূর্ণাঙ্ক ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    যদি (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        মুক্ত(drive);
        প্রত্যাগম -1;
    }
    প্রত্যাগম 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
পূর্ণাঙ্ক
usb_msc_setup(সংরচনা usbdevice_s *usbdev)
{
    যদি (!CONFIG_USB_MSC)
        প্রত্যাগম -1;

    // Verify right kind of device
    সংরচনা usb_interface_descriptor *iface = usbdev->iface;
    যদি ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        প্রত্যাগম -1;
    }

    // Find bulk in and bulk out endpoints.
    সংরচনা usb_pipe *inpipe = রিক্ত, *outpipe = রিক্ত;
    সংরচনা usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    সংরচনা usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    যদি (!indesc || !outdesc)
        গচ্ছ fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    যদি (!inpipe || !outpipe)
        গচ্ছ fail;

    পূর্ণাঙ্ক maxlun = usb_msc_maxlun(usbdev->defpipe);
    পূর্ণাঙ্ক lun, pipesused = 0;
    প্রতি (lun = 0; lun < maxlun + 1; lun++) {
        পূর্ণাঙ্ক ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        যদি (!ret)
            pipesused = 1;
    }

    যদি (!pipesused)
        গচ্ছ fail;

    প্রত্যাগম 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    প্রত্যাগম -1;
}
