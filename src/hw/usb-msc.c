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

ساخت usbdrive_s {
    ساخت drive_s drive;
    ساخت usb_pipe *bulkin, *bulkout;
    صحیح lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#تعریف_کرو USB_CDB_سائز 12

#تعریف_کرو CBW_SIGNATURE 0x43425355 // USBC

ساخت cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 d_CBWData_Transfer_لمبائی;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#تعریف_کرو CSW_SIGNATURE 0x53425355 // USBS

ساخت csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

ساکن صحیح
usb_msc_send(ساخت usbdrive_s *udrive_gf, صحیح dir, خالی *buf, u32 بائٹیں)
{
    ساخت usb_pipe *pipe;
    اگر (dir == USB_DIR_برآمد)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    ورنہ
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    واپس usb_send_bulk(pipe, dir, buf, بائٹیں);
}

// Low-level usb command transmit function.
صحیح
usb_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_USB_MSC)
        واپس 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->گنتی, op->buf_fl);
    ساخت usbdrive_s *udrive_gf = container_کا(
        op->drive_fl, ساخت usbdrive_s, drive);

    // Setup command block wrapper.
    ساخت cbw_s cbw;
    memset(&cbw, 0, حجم(cbw));
    صحیح blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_سائز);
    اگر (blocksize < 0)
        واپس default_process_op(op);
    u32 بائٹیں = blocksize * op->گنتی;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.d_CBWData_Transfer_لمبائی = بائٹیں;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_اندر : USB_DIR_برآمد;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_سائز;

    // Transfer cbw to device.
    صحیح ret = usb_msc_send(udrive_gf, USB_DIR_برآمد
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), حجم(cbw));
    اگر (ret)
        جاؤ fail;

    // Transfer data to/from device.
    اگر (بائٹیں) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, بائٹیں);
        اگر (ret)
            جاؤ fail;
    }

    // Transfer csw info.
    ساخت csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_اندر
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), حجم(csw));
    اگر (ret)
        جاؤ fail;

    اگر (!csw.bCSWStatus)
        واپس DISK_RET_SUCCESS;
    اگر (csw.bCSWStatus == 2)
        جاؤ fail;

    اگر (blocksize)
        op->گنتی -= csw.dCSWDataResidue / blocksize;
    واپس DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    واپس DISK_RET_EBADTRACK;
}

ساکن صحیح
usb_msc_maxlun(ساخت usb_pipe *pipe)
{
    ساخت usb_ctrlrequest req;
    req.bRequestType = USB_DIR_اندر | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.w_قدر = 0;
    req.w_اشاریہ = 0;
    req.w_لمبائی = 1;
    غیرعلامتی حرف maxlun;
    صحیح ret = usb_send_default_control(pipe, &req, &maxlun);
    اگر (ret)
        واپس 0;
    واپس maxlun;
}

ساکن صحیح
usb_msc_lun_تیاری(ساخت usb_pipe *inpipe, ساخت usb_pipe *outpipe,
                  ساخت usbdevice_s *usbdev, صحیح lun)
{
    // Allocate drive structure.
    ساخت usbdrive_s *drive = malloc_fseg(حجم(*drive));
    اگر (!drive) {
        warn_noalloc();
        واپس -1;
    }
    memset(drive, 0, حجم(*drive));
    اگر (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    ورنہ
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    صحیح prio = bootprio_ڈھونڈو_usb(usbdev, lun);
    صحیح ret = scsi_drive_تیاری(&drive->drive, "USB MSC", prio);
    اگر (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        آزاد(drive);
        واپس -1;
    }
    واپس 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
صحیح
usb_msc_تیاری(ساخت usbdevice_s *usbdev)
{
    اگر (!CONFIG_USB_MSC)
        واپس -1;

    // Verify right kind of device
    ساخت usb_interface_descriptor *iface = usbdev->iface;
    اگر ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        واپس -1;
    }

    // Find bulk in and bulk out endpoints.
    ساخت usb_pipe *inpipe = تہی, *outpipe = تہی;
    ساخت usb_endpoint_descriptor *indesc = usb_ڈھونڈو_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_اندر);
    ساخت usb_endpoint_descriptor *outdesc = usb_ڈھونڈو_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_برآمد);
    اگر (!indesc || !outdesc)
        جاؤ fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    اگر (!inpipe || !outpipe)
        جاؤ fail;

    صحیح maxlun = usb_msc_maxlun(usbdev->defpipe);
    صحیح lun, pipesused = 0;
    برائے (lun = 0; lun < maxlun + 1; lun++) {
        صحیح ret = usb_msc_lun_تیاری(inpipe, outpipe, usbdev, lun);
        اگر (!ret)
            pipesused = 1;
    }

    اگر (!pipesused)
        جاؤ fail;

    واپس 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    واپس -1;
}
