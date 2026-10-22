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

ಸಂರಚನಾ usbdrive_s {
    ಸಂರಚನಾ drive_s drive;
    ಸಂರಚನಾ usb_pipe *bulkin, *bulkout;
    ಪೂರ್ಣಾಙ್ಕ lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

ಸಂರಚನಾ cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

ಸಂರಚನಾ csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_msc_send(ಸಂರಚನಾ usbdrive_s *udrive_gf, ಪೂರ್ಣಾಙ್ಕ dir, ಶೂನ್ಯ *buf, u32 bytes)
{
    ಸಂರಚನಾ usb_pipe *pipe;
    ಯದಿ (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    ಅನ್ಯಥಾ
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    ಪ್ರತ್ಯಾಗಮ usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
ಪೂರ್ಣಾಙ್ಕ
usb_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_USB_MSC)
        ಪ್ರತ್ಯಾಗಮ 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->ಎಣಿಕೆ, op->buf_fl);
    ಸಂರಚನಾ usbdrive_s *udrive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ usbdrive_s, drive);

    // Setup command block wrapper.
    ಸಂರಚನಾ cbw_s cbw;
    memset(&cbw, 0, ಪರಿಮಾಣ(cbw));
    ಪೂರ್ಣಾಙ್ಕ blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    ಯದಿ (blocksize < 0)
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    u32 bytes = blocksize * op->ಎಣಿಕೆ;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    ಪೂರ್ಣಾಙ್ಕ ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), ಪರಿಮಾಣ(cbw));
    ಯದಿ (ret)
        ಗಚ್ಛ fail;

    // Transfer data to/from device.
    ಯದಿ (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        ಯದಿ (ret)
            ಗಚ್ಛ fail;
    }

    // Transfer csw info.
    ಸಂರಚನಾ csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), ಪರಿಮಾಣ(csw));
    ಯದಿ (ret)
        ಗಚ್ಛ fail;

    ಯದಿ (!csw.bCSWStatus)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
    ಯದಿ (csw.bCSWStatus == 2)
        ಗಚ್ಛ fail;

    ಯದಿ (blocksize)
        op->ಎಣಿಕೆ -= csw.dCSWDataResidue / blocksize;
    ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_msc_maxlun(ಸಂರಚನಾ usb_pipe *pipe)
{
    ಸಂರಚನಾ usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_ಮೌಲ್ಯ = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned ವರ್ಣ maxlun;
    ಪೂರ್ಣಾಙ್ಕ ret = usb_send_default_control(pipe, &req, &maxlun);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ maxlun;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
usb_msc_lun_setup(ಸಂರಚನಾ usb_pipe *inpipe, ಸಂರಚನಾ usb_pipe *outpipe,
                  ಸಂರಚನಾ usbdevice_s *usbdev, ಪೂರ್ಣಾಙ್ಕ lun)
{
    // Allocate drive structure.
    ಸಂರಚನಾ usbdrive_s *drive = malloc_fseg(ಪರಿಮಾಣ(*drive));
    ಯದಿ (!drive) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    memset(drive, 0, ಪರಿಮಾಣ(*drive));
    ಯದಿ (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    ಅನ್ಯಥಾ
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    ಪೂರ್ಣಾಙ್ಕ prio = bootprio_find_usb(usbdev, lun);
    ಪೂರ್ಣಾಙ್ಕ ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    ಯದಿ (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        ಮುಕ್ತ(drive);
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
ಪೂರ್ಣಾಙ್ಕ
usb_msc_setup(ಸಂರಚನಾ usbdevice_s *usbdev)
{
    ಯದಿ (!CONFIG_USB_MSC)
        ಪ್ರತ್ಯಾಗಮ -1;

    // Verify right kind of device
    ಸಂರಚನಾ usb_interface_descriptor *iface = usbdev->iface;
    ಯದಿ ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    // Find bulk in and bulk out endpoints.
    ಸಂರಚನಾ usb_pipe *inpipe = ರಿಕ್ತ, *outpipe = ರಿಕ್ತ;
    ಸಂರಚನಾ usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    ಸಂರಚನಾ usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    ಯದಿ (!indesc || !outdesc)
        ಗಚ್ಛ fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    ಯದಿ (!inpipe || !outpipe)
        ಗಚ್ಛ fail;

    ಪೂರ್ಣಾಙ್ಕ maxlun = usb_msc_maxlun(usbdev->defpipe);
    ಪೂರ್ಣಾಙ್ಕ lun, pipesused = 0;
    ಪ್ರತಿ (lun = 0; lun < maxlun + 1; lun++) {
        ಪೂರ್ಣಾಙ್ಕ ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        ಯದಿ (!ret)
            pipesused = 1;
    }

    ಯದಿ (!pipesused)
        ಗಚ್ಛ fail;

    ಪ್ರತ್ಯಾಗಮ 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    ಪ್ರತ್ಯಾಗಮ -1;
}
