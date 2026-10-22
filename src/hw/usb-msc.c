// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBALFLAT
#ଅନ୍ତର୍ଭାଵ "block.h" // DTYPE_USB
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // cdb_read
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_USB_MSC
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "usb.h" // struct usb_s
#ଅନ୍ତର୍ଭାଵ "usb-msc.h" // usb_msc_setup
#ଅନ୍ତର୍ଭାଵ "util.h" // bootprio_find_usb

ସଂରଚନା usbdrive_s {
    ସଂରଚନା drive_s drive;
    ସଂରଚନା usb_pipe *bulkin, *bulkout;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

ସଂରଚନା cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

ସଂରଚନା csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_msc_send(ସଂରଚନା usbdrive_s *udrive_gf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଶୂନ୍ୟ *buf, u32 bytes)
{
    ସଂରଚନା usb_pipe *pipe;
    ଯଦି (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    ଅନ୍ୟଥା
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    ଫେରନ୍ତୁ usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_USB_MSC)
        ଫେରନ୍ତୁ 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->ଗଣନା_2, op->buf_fl);
    ସଂରଚନା usbdrive_s *udrive_gf = container_of(
        op->drive_fl, ସଂରଚନା usbdrive_s, drive);

    // Setup command block wrapper.
    ସଂରଚନା cbw_s cbw;
    memset(&cbw, 0, ଆକାର(cbw));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);
    u32 bytes = blocksize * op->ଗଣନା_2;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), ଆକାର(cbw));
    ଯଦି (ret)
        ଯାଅ fail;

    // Transfer data to/from device.
    ଯଦି (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        ଯଦି (ret)
            ଯାଅ fail;
    }

    // Transfer csw info.
    ସଂରଚନା csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), ଆକାର(csw));
    ଯଦି (ret)
        ଯାଅ fail;

    ଯଦି (!csw.bCSWStatus)
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;
    ଯଦି (csw.bCSWStatus == 2)
        ଯାଅ fail;

    ଯଦି (blocksize)
        op->ଗଣନା_2 -= csw.dCSWDataResidue / blocksize;
    ଫେରନ୍ତୁ DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_msc_maxlun(ସଂରଚନା usb_pipe *pipe)
{
    ସଂରଚନା usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_ମୂଲ୍ୟ = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned ଅକ୍ଷର maxlun;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_send_default_control(pipe, &req, &maxlun);
    ଯଦି (ret)
        ଫେରନ୍ତୁ 0;
    ଫେରନ୍ତୁ maxlun;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_msc_lun_setup(ସଂରଚନା usb_pipe *inpipe, ସଂରଚନା usb_pipe *outpipe,
                  ସଂରଚନା usbdevice_s *usbdev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun)
{
    // Allocate drive structure.
    ସଂରଚନା usbdrive_s *drive = malloc_fseg(ଆକାର(*drive));
    ଯଦି (!drive) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    memset(drive, 0, ଆକାର(*drive));
    ଯଦି (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    ଅନ୍ୟଥା
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = bootprio_find_usb(usbdev, lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    ଯଦି (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        ମୁକ୍ତ(drive);
        ଫେରନ୍ତୁ -1;
    }
    ଫେରନ୍ତୁ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
usb_msc_setup(ସଂରଚନା usbdevice_s *usbdev)
{
    ଯଦି (!CONFIG_USB_MSC)
        ଫେରନ୍ତୁ -1;

    // Verify right kind of device
    ସଂରଚନା usb_interface_descriptor *iface = usbdev->iface;
    ଯଦି ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ଫେରନ୍ତୁ -1;
    }

    // Find bulk in and bulk out endpoints.
    ସଂରଚନା usb_pipe *inpipe = ଖାଲି, *outpipe = ଖାଲି;
    ସଂରଚନା usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    ସଂରଚନା usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    ଯଦି (!indesc || !outdesc)
        ଯାଅ fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    ଯଦି (!inpipe || !outpipe)
        ଯାଅ fail;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxlun = usb_msc_maxlun(usbdev->defpipe);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା lun, pipesused = 0;
    ପାଇଁ (lun = 0; lun < maxlun + 1; lun++) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        ଯଦି (!ret)
            pipesused = 1;
    }

    ଯଦି (!pipesused)
        ଯାଅ fail;

    ଫେରନ୍ତୁ 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    ଫେରନ୍ତୁ -1;
}
