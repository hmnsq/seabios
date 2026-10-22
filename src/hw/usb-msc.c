// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBALFLAT
#समावेश "block.h" // DTYPE_USB
#समावेश "blockcmd.h" // cdb_read
#समावेश "config.h" // CONFIG_USB_MSC
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "usb.h" // struct usb_s
#समावेश "usb-msc.h" // usb_msc_setup
#समावेश "util.h" // bootprio_find_usb

काष्ठा usbdrive_s {
    काष्ठा drive_s drive;
    काष्ठा usb_pipe *bulkin, *bulkout;
    पूर्णांक lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#घोषणा USB_CDB_आकार 12

#घोषणा CBW_SIGNATURE 0x43425355 // USBC

काष्ठा cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 d_CBWData_Transfer_लंबाई;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#घोषणा CSW_SIGNATURE 0x53425355 // USBS

काष्ठा csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

अटल पूर्णांक
usb_msc_send(काष्ठा usbdrive_s *udrive_gf, पूर्णांक dir, व्योम *buf, u32 bytes)
{
    काष्ठा usb_pipe *pipe;
    अगर (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    अन्यथा
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    वापस usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
पूर्णांक
usb_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_USB_MSC)
        वापस 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->गिनती, op->buf_fl);
    काष्ठा usbdrive_s *udrive_gf = container_of(
        op->drive_fl, काष्ठा usbdrive_s, drive);

    // Setup command block wrapper.
    काष्ठा cbw_s cbw;
    स_रखो(&cbw, 0, माप(cbw));
    पूर्णांक blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_आकार);
    अगर (blocksize < 0)
        वापस default_process_op(op);
    u32 bytes = blocksize * op->गिनती;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.d_CBWData_Transfer_लंबाई = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_आकार;

    // Transfer cbw to device.
    पूर्णांक ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), माप(cbw));
    अगर (ret)
        जाओ fail;

    // Transfer data to/from device.
    अगर (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        अगर (ret)
            जाओ fail;
    }

    // Transfer csw info.
    काष्ठा csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), माप(csw));
    अगर (ret)
        जाओ fail;

    अगर (!csw.bCSWStatus)
        वापस DISK_RET_SUCCESS;
    अगर (csw.bCSWStatus == 2)
        जाओ fail;

    अगर (blocksize)
        op->गिनती -= csw.dCSWDataResidue / blocksize;
    वापस DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    वापस DISK_RET_EBADTRACK;
}

अटल पूर्णांक
usb_msc_maxlun(काष्ठा usb_pipe *pipe)
{
    काष्ठा usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_मूल्य = 0;
    req.w_सूचकांक = 0;
    req.w_लंबाई = 1;
    अचिन्हित अक्षर maxlun;
    पूर्णांक ret = usb_send_default_control(pipe, &req, &maxlun);
    अगर (ret)
        वापस 0;
    वापस maxlun;
}

अटल पूर्णांक
usb_msc_lun_setup(काष्ठा usb_pipe *inpipe, काष्ठा usb_pipe *outpipe,
                  काष्ठा usbdevice_s *usbdev, पूर्णांक lun)
{
    // Allocate drive structure.
    काष्ठा usbdrive_s *drive = malloc_fseg(माप(*drive));
    अगर (!drive) {
        warn_noalloc();
        वापस -1;
    }
    स_रखो(drive, 0, माप(*drive));
    अगर (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    अन्यथा
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    पूर्णांक prio = bootprio_find_usb(usbdev, lun);
    पूर्णांक ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    अगर (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        मुक्त(drive);
        वापस -1;
    }
    वापस 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
पूर्णांक
usb_msc_setup(काष्ठा usbdevice_s *usbdev)
{
    अगर (!CONFIG_USB_MSC)
        वापस -1;

    // Verify right kind of device
    काष्ठा usb_interface_descriptor *iface = usbdev->iface;
    अगर ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        वापस -1;
    }

    // Find bulk in and bulk out endpoints.
    काष्ठा usb_pipe *inpipe = शून्य, *outpipe = शून्य;
    काष्ठा usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    काष्ठा usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    अगर (!indesc || !outdesc)
        जाओ fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    अगर (!inpipe || !outpipe)
        जाओ fail;

    पूर्णांक maxlun = usb_msc_maxlun(usbdev->defpipe);
    पूर्णांक lun, pipesused = 0;
    क्रम (lun = 0; lun < maxlun + 1; lun++) {
        पूर्णांक ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        अगर (!ret)
            pipesused = 1;
    }

    अगर (!pipesused)
        जाओ fail;

    वापस 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    वापस -1;
}
