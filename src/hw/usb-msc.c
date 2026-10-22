// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBALFLAT
#ضمن "block.h" // DTYPE_USB
#ضمن "blockcmd.h" // cdb_read
#ضمن "config.h" // CONFIG_USB_MSC
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-msc.h" // usb_msc_setup
#ضمن "util.h" // bootprio_find_usb

بنية usbdrive_s {
    بنية drive_s drive;
    بنية usb_pipe *bulkin, *bulkout;
    صحيح lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

بنية cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

بنية csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

ساكن صحيح
usb_msc_send(بنية usbdrive_s *udrive_gf, صحيح dir, فراغ *buf, u32 bytes)
{
    بنية usb_pipe *pipe;
    إذا (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    وإلا
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    إرجاع usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
صحيح
usb_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_USB_MSC)
        إرجاع 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->تعداد_2, op->buf_fl);
    بنية usbdrive_s *udrive_gf = container_of(
        op->drive_fl, بنية usbdrive_s, drive);

    // Setup command block wrapper.
    بنية cbw_s cbw;
    memset(&cbw, 0, حجم(cbw));
    صحيح blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    إذا (blocksize < 0)
        إرجاع default_process_op(op);
    u32 bytes = blocksize * op->تعداد_2;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    صحيح ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), حجم(cbw));
    إذا (ret)
        اذهب fail;

    // Transfer data to/from device.
    إذا (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        إذا (ret)
            اذهب fail;
    }

    // Transfer csw info.
    بنية csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), حجم(csw));
    إذا (ret)
        اذهب fail;

    إذا (!csw.bCSWStatus)
        إرجاع DISK_RET_SUCCESS;
    إذا (csw.bCSWStatus == 2)
        اذهب fail;

    إذا (blocksize)
        op->تعداد_2 -= csw.dCSWDataResidue / blocksize;
    إرجاع DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    إرجاع DISK_RET_EBADTRACK;
}

ساكن صحيح
usb_msc_maxlun(بنية usb_pipe *pipe)
{
    بنية usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_قيمة = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned حرف maxlun;
    صحيح ret = usb_send_default_control(pipe, &req, &maxlun);
    إذا (ret)
        إرجاع 0;
    إرجاع maxlun;
}

ساكن صحيح
usb_msc_lun_setup(بنية usb_pipe *inpipe, بنية usb_pipe *outpipe,
                  بنية usbdevice_s *usbdev, صحيح lun)
{
    // Allocate drive structure.
    بنية usbdrive_s *drive = malloc_fseg(حجم(*drive));
    إذا (!drive) {
        warn_noalloc();
        إرجاع -1;
    }
    memset(drive, 0, حجم(*drive));
    إذا (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    وإلا
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    صحيح prio = bootprio_find_usb(usbdev, lun);
    صحيح ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    إذا (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        حرر(drive);
        إرجاع -1;
    }
    إرجاع 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
صحيح
usb_msc_setup(بنية usbdevice_s *usbdev)
{
    إذا (!CONFIG_USB_MSC)
        إرجاع -1;

    // Verify right kind of device
    بنية usb_interface_descriptor *iface = usbdev->iface;
    إذا ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        إرجاع -1;
    }

    // Find bulk in and bulk out endpoints.
    بنية usb_pipe *inpipe = عدم, *outpipe = عدم;
    بنية usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    بنية usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    إذا (!indesc || !outdesc)
        اذهب fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    إذا (!inpipe || !outpipe)
        اذهب fail;

    صحيح maxlun = usb_msc_maxlun(usbdev->defpipe);
    صحيح lun, pipesused = 0;
    لكل (lun = 0; lun < maxlun + 1; lun++) {
        صحيح ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        إذا (!ret)
            pipesused = 1;
    }

    إذا (!pipesused)
        اذهب fail;

    إرجاع 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    إرجاع -1;
}
