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

কাঠামো usbdrive_s {
    কাঠামো drive_s drive;
    কাঠামো usb_pipe *bulkin, *bulkout;
    পূর্ণসংখ্যা lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_আকার 12

#define CBW_SIGNATURE 0x43425355 // USBC

কাঠামো cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 d_CBWData_Transfer_দৈর্ঘ্য;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

কাঠামো csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

স্থির পূর্ণসংখ্যা
usb_msc_send(কাঠামো usbdrive_s *udrive_gf, পূর্ণসংখ্যা dir, শূন্য *buf, u32 bytes)
{
    কাঠামো usb_pipe *pipe;
    যদি (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    অন্যথা
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    ফেরত usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
পূর্ণসংখ্যা
usb_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_USB_MSC)
        ফেরত 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->গুনতি, op->buf_fl);
    কাঠামো usbdrive_s *udrive_gf = container_of(
        op->drive_fl, কাঠামো usbdrive_s, drive);

    // Setup command block wrapper.
    কাঠামো cbw_s cbw;
    memset(&cbw, 0, আকার(cbw));
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_আকার);
    যদি (blocksize < 0)
        ফেরত default_process_op(op);
    u32 bytes = blocksize * op->গুনতি;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.d_CBWData_Transfer_দৈর্ঘ্য = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_আকার;

    // Transfer cbw to device.
    পূর্ণসংখ্যা ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), আকার(cbw));
    যদি (ret)
        যাও fail;

    // Transfer data to/from device.
    যদি (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        যদি (ret)
            যাও fail;
    }

    // Transfer csw info.
    কাঠামো csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), আকার(csw));
    যদি (ret)
        যাও fail;

    যদি (!csw.bCSWStatus)
        ফেরত DISK_RET_SUCCESS;
    যদি (csw.bCSWStatus == 2)
        যাও fail;

    যদি (blocksize)
        op->গুনতি -= csw.dCSWDataResidue / blocksize;
    ফেরত DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    ফেরত DISK_RET_EBADTRACK;
}

স্থির পূর্ণসংখ্যা
usb_msc_maxlun(কাঠামো usb_pipe *pipe)
{
    কাঠামো usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_মান = 0;
    req.w_সূচক = 0;
    req.w_দৈর্ঘ্য = 1;
    unsigned অক্ষর maxlun;
    পূর্ণসংখ্যা ret = usb_send_default_control(pipe, &req, &maxlun);
    যদি (ret)
        ফেরত 0;
    ফেরত maxlun;
}

স্থির পূর্ণসংখ্যা
usb_msc_lun_setup(কাঠামো usb_pipe *inpipe, কাঠামো usb_pipe *outpipe,
                  কাঠামো usbdevice_s *usbdev, পূর্ণসংখ্যা lun)
{
    // Allocate drive structure.
    কাঠামো usbdrive_s *drive = malloc_fseg(আকার(*drive));
    যদি (!drive) {
        warn_noalloc();
        ফেরত -1;
    }
    memset(drive, 0, আকার(*drive));
    যদি (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    অন্যথা
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    পূর্ণসংখ্যা prio = bootprio_find_usb(usbdev, lun);
    পূর্ণসংখ্যা ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    যদি (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        মুক্ত(drive);
        ফেরত -1;
    }
    ফেরত 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
পূর্ণসংখ্যা
usb_msc_setup(কাঠামো usbdevice_s *usbdev)
{
    যদি (!CONFIG_USB_MSC)
        ফেরত -1;

    // Verify right kind of device
    কাঠামো usb_interface_descriptor *iface = usbdev->iface;
    যদি ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ফেরত -1;
    }

    // Find bulk in and bulk out endpoints.
    কাঠামো usb_pipe *inpipe = ফাঁকা, *outpipe = ফাঁকা;
    কাঠামো usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    কাঠামো usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    যদি (!indesc || !outdesc)
        যাও fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    যদি (!inpipe || !outpipe)
        যাও fail;

    পূর্ণসংখ্যা maxlun = usb_msc_maxlun(usbdev->defpipe);
    পূর্ণসংখ্যা lun, pipesused = 0;
    জন্য (lun = 0; lun < maxlun + 1; lun++) {
        পূর্ণসংখ্যা ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        যদি (!ret)
            pipesused = 1;
    }

    যদি (!pipesused)
        যাও fail;

    ফেরত 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    ফেরত -1;
}
