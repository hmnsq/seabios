// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBALFLAT
#அந்தர்பாவ "block.h" // DTYPE_USB
#அந்தர்பாவ "blockcmd.h" // cdb_read
#அந்தர்பாவ "config.h" // CONFIG_USB_MSC
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "usb.h" // struct usb_s
#அந்தர்பாவ "usb-msc.h" // usb_msc_setup
#அந்தர்பாவ "util.h" // bootprio_find_usb

கட்டமைப்பு usbdrive_s {
    கட்டமைப்பு drive_s drive;
    கட்டமைப்பு usb_pipe *bulkin, *bulkout;
    முழு lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

கட்டமைப்பு cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

கட்டமைப்பு csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

நிலையான முழு
usb_msc_send(கட்டமைப்பு usbdrive_s *udrive_gf, முழு dir, வெறுமை *buf, u32 bytes)
{
    கட்டமைப்பு usb_pipe *pipe;
    எனில் (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    இல்லையெனில்
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    திரும்பு usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
முழு
usb_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_USB_MSC)
        திரும்பு 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->மொத்தம், op->buf_fl);
    கட்டமைப்பு usbdrive_s *udrive_gf = container_of(
        op->drive_fl, கட்டமைப்பு usbdrive_s, drive);

    // Setup command block wrapper.
    கட்டமைப்பு cbw_s cbw;
    memset(&cbw, 0, அளவு(cbw));
    முழு blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    எனில் (blocksize < 0)
        திரும்பு default_process_op(op);
    u32 bytes = blocksize * op->மொத்தம்;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    முழு ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), அளவு(cbw));
    எனில் (ret)
        செல் fail;

    // Transfer data to/from device.
    எனில் (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        எனில் (ret)
            செல் fail;
    }

    // Transfer csw info.
    கட்டமைப்பு csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), அளவு(csw));
    எனில் (ret)
        செல் fail;

    எனில் (!csw.bCSWStatus)
        திரும்பு DISK_RET_SUCCESS;
    எனில் (csw.bCSWStatus == 2)
        செல் fail;

    எனில் (blocksize)
        op->மொத்தம் -= csw.dCSWDataResidue / blocksize;
    திரும்பு DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    திரும்பு DISK_RET_EBADTRACK;
}

நிலையான முழு
usb_msc_maxlun(கட்டமைப்பு usb_pipe *pipe)
{
    கட்டமைப்பு usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_மதிப்பு = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned எழுத்து maxlun;
    முழு ret = usb_send_default_control(pipe, &req, &maxlun);
    எனில் (ret)
        திரும்பு 0;
    திரும்பு maxlun;
}

நிலையான முழு
usb_msc_lun_setup(கட்டமைப்பு usb_pipe *inpipe, கட்டமைப்பு usb_pipe *outpipe,
                  கட்டமைப்பு usbdevice_s *usbdev, முழு lun)
{
    // Allocate drive structure.
    கட்டமைப்பு usbdrive_s *drive = malloc_fseg(அளவு(*drive));
    எனில் (!drive) {
        warn_noalloc();
        திரும்பு -1;
    }
    memset(drive, 0, அளவு(*drive));
    எனில் (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    இல்லையெனில்
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    முழு prio = bootprio_find_usb(usbdev, lun);
    முழு ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    எனில் (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        விடுவி(drive);
        திரும்பு -1;
    }
    திரும்பு 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
முழு
usb_msc_setup(கட்டமைப்பு usbdevice_s *usbdev)
{
    எனில் (!CONFIG_USB_MSC)
        திரும்பு -1;

    // Verify right kind of device
    கட்டமைப்பு usb_interface_descriptor *iface = usbdev->iface;
    எனில் ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        திரும்பு -1;
    }

    // Find bulk in and bulk out endpoints.
    கட்டமைப்பு usb_pipe *inpipe = வெற்று, *outpipe = வெற்று;
    கட்டமைப்பு usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    கட்டமைப்பு usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    எனில் (!indesc || !outdesc)
        செல் fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    எனில் (!inpipe || !outpipe)
        செல் fail;

    முழு maxlun = usb_msc_maxlun(usbdev->defpipe);
    முழு lun, pipesused = 0;
    க்கு (lun = 0; lun < maxlun + 1; lun++) {
        முழு ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        எனில் (!ret)
            pipesused = 1;
    }

    எனில் (!pipesused)
        செல் fail;

    திரும்பு 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    திரும்பு -1;
}
