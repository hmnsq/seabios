// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBALFLAT
#અન્તર્ભાવ "block.h" // DTYPE_USB
#અન્તર્ભાવ "blockcmd.h" // cdb_read
#અન્તર્ભાવ "config.h" // CONFIG_USB_MSC
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "usb.h" // struct usb_s
#અન્તર્ભાવ "usb-msc.h" // usb_msc_setup
#અન્તર્ભાવ "util.h" // bootprio_find_usb

સંરચના usbdrive_s {
    સંરચના drive_s drive;
    સંરચના usb_pipe *bulkin, *bulkout;
    પૂર્ણાંક lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

સંરચના cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

સંરચના csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

સ્થિર પૂર્ણાંક
usb_msc_send(સંરચના usbdrive_s *udrive_gf, પૂર્ણાંક dir, શૂન્ય *buf, u32 bytes)
{
    સંરચના usb_pipe *pipe;
    જો (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    નહીંતર
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    પાછા usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
પૂર્ણાંક
usb_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_USB_MSC)
        પાછા 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->ગણતરી, op->buf_fl);
    સંરચના usbdrive_s *udrive_gf = container_of(
        op->drive_fl, સંરચના usbdrive_s, drive);

    // Setup command block wrapper.
    સંરચના cbw_s cbw;
    memset(&cbw, 0, કદ(cbw));
    પૂર્ણાંક blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    જો (blocksize < 0)
        પાછા default_process_op(op);
    u32 bytes = blocksize * op->ગણતરી;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    પૂર્ણાંક ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), કદ(cbw));
    જો (ret)
        જાઓ fail;

    // Transfer data to/from device.
    જો (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        જો (ret)
            જાઓ fail;
    }

    // Transfer csw info.
    સંરચના csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), કદ(csw));
    જો (ret)
        જાઓ fail;

    જો (!csw.bCSWStatus)
        પાછા DISK_RET_SUCCESS;
    જો (csw.bCSWStatus == 2)
        જાઓ fail;

    જો (blocksize)
        op->ગણતરી -= csw.dCSWDataResidue / blocksize;
    પાછા DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    પાછા DISK_RET_EBADTRACK;
}

સ્થિર પૂર્ણાંક
usb_msc_maxlun(સંરચના usb_pipe *pipe)
{
    સંરચના usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_કિંમત = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned અક્ષર maxlun;
    પૂર્ણાંક ret = usb_send_default_control(pipe, &req, &maxlun);
    જો (ret)
        પાછા 0;
    પાછા maxlun;
}

સ્થિર પૂર્ણાંક
usb_msc_lun_setup(સંરચના usb_pipe *inpipe, સંરચના usb_pipe *outpipe,
                  સંરચના usbdevice_s *usbdev, પૂર્ણાંક lun)
{
    // Allocate drive structure.
    સંરચના usbdrive_s *drive = malloc_fseg(કદ(*drive));
    જો (!drive) {
        warn_noalloc();
        પાછા -1;
    }
    memset(drive, 0, કદ(*drive));
    જો (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    નહીંતર
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    પૂર્ણાંક prio = bootprio_find_usb(usbdev, lun);
    પૂર્ણાંક ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    જો (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        મુક્ત(drive);
        પાછા -1;
    }
    પાછા 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
પૂર્ણાંક
usb_msc_setup(સંરચના usbdevice_s *usbdev)
{
    જો (!CONFIG_USB_MSC)
        પાછા -1;

    // Verify right kind of device
    સંરચના usb_interface_descriptor *iface = usbdev->iface;
    જો ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        પાછા -1;
    }

    // Find bulk in and bulk out endpoints.
    સંરચના usb_pipe *inpipe = ખાલી, *outpipe = ખાલી;
    સંરચના usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    સંરચના usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    જો (!indesc || !outdesc)
        જાઓ fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    જો (!inpipe || !outpipe)
        જાઓ fail;

    પૂર્ણાંક maxlun = usb_msc_maxlun(usbdev->defpipe);
    પૂર્ણાંક lun, pipesused = 0;
    માટે (lun = 0; lun < maxlun + 1; lun++) {
        પૂર્ણાંક ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        જો (!ret)
            pipesused = 1;
    }

    જો (!pipesused)
        જાઓ fail;

    પાછા 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    પાછા -1;
}
