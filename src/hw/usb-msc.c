// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBALFLAT
#അന്തര്ഭാവ "block.h" // DTYPE_USB
#അന്തര്ഭാവ "blockcmd.h" // cdb_read
#അന്തര്ഭാവ "config.h" // CONFIG_USB_MSC
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "usb.h" // struct usb_s
#അന്തര്ഭാവ "usb-msc.h" // usb_msc_setup
#അന്തര്ഭാവ "util.h" // bootprio_find_usb

ഘടന usbdrive_s {
    ഘടന drive_s drive;
    ഘടന usb_pipe *bulkin, *bulkout;
    പൂർണ്ണസംഖ്യ lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

ഘടന cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

ഘടന csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_msc_send(ഘടന usbdrive_s *udrive_gf, പൂർണ്ണസംഖ്യ dir, ശൂന്യം *buf, u32 bytes)
{
    ഘടന usb_pipe *pipe;
    എങ്കിൽ (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    അല്ലെങ്കിൽ
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    തിരികെ usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
പൂർണ്ണസംഖ്യ
usb_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_USB_MSC)
        തിരികെ 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->എണ്ണം, op->buf_fl);
    ഘടന usbdrive_s *udrive_gf = container_of(
        op->drive_fl, ഘടന usbdrive_s, drive);

    // Setup command block wrapper.
    ഘടന cbw_s cbw;
    memset(&cbw, 0, വലുപ്പം(cbw));
    പൂർണ്ണസംഖ്യ blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    എങ്കിൽ (blocksize < 0)
        തിരികെ default_process_op(op);
    u32 bytes = blocksize * op->എണ്ണം;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    പൂർണ്ണസംഖ്യ ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), വലുപ്പം(cbw));
    എങ്കിൽ (ret)
        പോകുക fail;

    // Transfer data to/from device.
    എങ്കിൽ (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        എങ്കിൽ (ret)
            പോകുക fail;
    }

    // Transfer csw info.
    ഘടന csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), വലുപ്പം(csw));
    എങ്കിൽ (ret)
        പോകുക fail;

    എങ്കിൽ (!csw.bCSWStatus)
        തിരികെ DISK_RET_SUCCESS;
    എങ്കിൽ (csw.bCSWStatus == 2)
        പോകുക fail;

    എങ്കിൽ (blocksize)
        op->എണ്ണം -= csw.dCSWDataResidue / blocksize;
    തിരികെ DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    തിരികെ DISK_RET_EBADTRACK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_msc_maxlun(ഘടന usb_pipe *pipe)
{
    ഘടന usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_മൂല്യം = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned അക്ഷരം maxlun;
    പൂർണ്ണസംഖ്യ ret = usb_send_default_control(pipe, &req, &maxlun);
    എങ്കിൽ (ret)
        തിരികെ 0;
    തിരികെ maxlun;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
usb_msc_lun_setup(ഘടന usb_pipe *inpipe, ഘടന usb_pipe *outpipe,
                  ഘടന usbdevice_s *usbdev, പൂർണ്ണസംഖ്യ lun)
{
    // Allocate drive structure.
    ഘടന usbdrive_s *drive = malloc_fseg(വലുപ്പം(*drive));
    എങ്കിൽ (!drive) {
        warn_noalloc();
        തിരികെ -1;
    }
    memset(drive, 0, വലുപ്പം(*drive));
    എങ്കിൽ (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    അല്ലെങ്കിൽ
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    പൂർണ്ണസംഖ്യ prio = bootprio_find_usb(usbdev, lun);
    പൂർണ്ണസംഖ്യ ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    എങ്കിൽ (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        സ്വതന്ത്രം(drive);
        തിരികെ -1;
    }
    തിരികെ 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
പൂർണ്ണസംഖ്യ
usb_msc_setup(ഘടന usbdevice_s *usbdev)
{
    എങ്കിൽ (!CONFIG_USB_MSC)
        തിരികെ -1;

    // Verify right kind of device
    ഘടന usb_interface_descriptor *iface = usbdev->iface;
    എങ്കിൽ ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        തിരികെ -1;
    }

    // Find bulk in and bulk out endpoints.
    ഘടന usb_pipe *inpipe = ശൂന്യ, *outpipe = ശൂന്യ;
    ഘടന usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    ഘടന usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    എങ്കിൽ (!indesc || !outdesc)
        പോകുക fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    എങ്കിൽ (!inpipe || !outpipe)
        പോകുക fail;

    പൂർണ്ണസംഖ്യ maxlun = usb_msc_maxlun(usbdev->defpipe);
    പൂർണ്ണസംഖ്യ lun, pipesused = 0;
    വേണ്ടി (lun = 0; lun < maxlun + 1; lun++) {
        പൂർണ്ണസംഖ്യ ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        എങ്കിൽ (!ret)
            pipesused = 1;
    }

    എങ്കിൽ (!pipesused)
        പോകുക fail;

    തിരികെ 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    തിരികെ -1;
}
