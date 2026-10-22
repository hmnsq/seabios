// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "block.h" // DTYPE_USB
#شامل "blockcmd.h" // cdb_read
#شامل "config.h" // CONFIG_USB_MSC
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-msc.h" // usb_msc_setup
#شامل "util.h" // bootprio_find_usb

ساختار usbdrive_s {
    ساختار drive_s drive;
    ساختار usb_pipe *bulkin, *bulkout;
    صحیح lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

ساختار cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

ساختار csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

ایستا صحیح
usb_msc_send(ساختار usbdrive_s *udrive_gf, صحیح dir, تهی *buf, u32 bytes)
{
    ساختار usb_pipe *pipe;
    اگر (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    وگرنه
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    بازگشت usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
صحیح
usb_process_op(ساختار disk_op_s *op)
{
    اگر (!CONFIG_USB_MSC)
        بازگشت 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->شمار, op->buf_fl);
    ساختار usbdrive_s *udrive_gf = container_of(
        op->drive_fl, ساختار usbdrive_s, drive);

    // Setup command block wrapper.
    ساختار cbw_s cbw;
    memset(&cbw, 0, اندازه(cbw));
    صحیح blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    اگر (blocksize < 0)
        بازگشت default_process_op(op);
    u32 bytes = blocksize * op->شمار;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    صحیح ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), اندازه(cbw));
    اگر (ret)
        برو fail;

    // Transfer data to/from device.
    اگر (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        اگر (ret)
            برو fail;
    }

    // Transfer csw info.
    ساختار csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), اندازه(csw));
    اگر (ret)
        برو fail;

    اگر (!csw.bCSWStatus)
        بازگشت DISK_RET_SUCCESS;
    اگر (csw.bCSWStatus == 2)
        برو fail;

    اگر (blocksize)
        op->شمار -= csw.dCSWDataResidue / blocksize;
    بازگشت DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    بازگشت DISK_RET_EBADTRACK;
}

ایستا صحیح
usb_msc_maxlun(ساختار usb_pipe *pipe)
{
    ساختار usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_مقدار = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned نویسه maxlun;
    صحیح ret = usb_send_default_control(pipe, &req, &maxlun);
    اگر (ret)
        بازگشت 0;
    بازگشت maxlun;
}

ایستا صحیح
usb_msc_lun_setup(ساختار usb_pipe *inpipe, ساختار usb_pipe *outpipe,
                  ساختار usbdevice_s *usbdev, صحیح lun)
{
    // Allocate drive structure.
    ساختار usbdrive_s *drive = malloc_fseg(اندازه(*drive));
    اگر (!drive) {
        warn_noalloc();
        بازگشت -1;
    }
    memset(drive, 0, اندازه(*drive));
    اگر (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    وگرنه
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    صحیح prio = bootprio_find_usb(usbdev, lun);
    صحیح ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    اگر (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        آزاد(drive);
        بازگشت -1;
    }
    بازگشت 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
صحیح
usb_msc_setup(ساختار usbdevice_s *usbdev)
{
    اگر (!CONFIG_USB_MSC)
        بازگشت -1;

    // Verify right kind of device
    ساختار usb_interface_descriptor *iface = usbdev->iface;
    اگر ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        بازگشت -1;
    }

    // Find bulk in and bulk out endpoints.
    ساختار usb_pipe *inpipe = پوچ, *outpipe = پوچ;
    ساختار usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    ساختار usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    اگر (!indesc || !outdesc)
        برو fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    اگر (!inpipe || !outpipe)
        برو fail;

    صحیح maxlun = usb_msc_maxlun(usbdev->defpipe);
    صحیح lun, pipesused = 0;
    برای (lun = 0; lun < maxlun + 1; lun++) {
        صحیح ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        اگر (!ret)
            pipesused = 1;
    }

    اگر (!pipesused)
        برو fail;

    بازگشت 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    بازگشت -1;
}
