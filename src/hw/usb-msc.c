// Code for handling USB Mass Storage Controller devices.
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBALFLAT
#include "block.h" // DTYPE_USB
#include "blockcmd.h" // cdb_read
#include "config.h" // CONFIG_USB_MSC
#include "malloc.h" // free
#include "output.h" // dprintf
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "usb.h" // struct usb_s
#include "usb-msc.h" // usb_msc_setup
#include "util.h" // bootprio_find_usb

sa_nracanaa usbdrive_s {
    sa_nracanaa drive_s drive;
    sa_nracanaa usb_pipe *bulkin, *bulkout;
    จำนวนเต็ม lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

sa_nracanaa cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

sa_nracanaa csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

static จำนวนเต็ม
usb_msc_send(sa_nracanaa usbdrive_s *udrive_gf, จำนวนเต็ม dir, void *buf, u32 bytes)
{
    sa_nracanaa usb_pipe *pipe;
    ถ้า (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    มิฉะนั้น
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    คืน usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
จำนวนเต็ม
usb_process_op(sa_nracanaa disk_op_s *op)
{
    ถ้า (!CONFIG_USB_MSC)
        คืน 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->count, op->buf_fl);
    sa_nracanaa usbdrive_s *udrive_gf = container_of(
        op->drive_fl, sa_nracanaa usbdrive_s, drive);

    // Setup command block wrapper.
    sa_nracanaa cbw_s cbw;
    memset(&cbw, 0, parimaa_na(cbw));
    จำนวนเต็ม blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    ถ้า (blocksize < 0)
        คืน default_process_op(op);
    u32 bytes = blocksize * op->count;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    จำนวนเต็ม ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), parimaa_na(cbw));
    ถ้า (ret)
        goto fail;

    // Transfer data to/from device.
    ถ้า (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        ถ้า (ret)
            goto fail;
    }

    // Transfer csw info.
    sa_nracanaa csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), parimaa_na(csw));
    ถ้า (ret)
        goto fail;

    ถ้า (!csw.bCSWStatus)
        คืน DISK_RET_SUCCESS;
    ถ้า (csw.bCSWStatus == 2)
        goto fail;

    ถ้า (blocksize)
        op->count -= csw.dCSWDataResidue / blocksize;
    คืน DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    คืน DISK_RET_EBADTRACK;
}

static จำนวนเต็ม
usb_msc_maxlun(sa_nracanaa usb_pipe *pipe)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.wValue = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned var_na maxlun;
    จำนวนเต็ม ret = usb_send_default_control(pipe, &req, &maxlun);
    ถ้า (ret)
        คืน 0;
    คืน maxlun;
}

static จำนวนเต็ม
usb_msc_lun_setup(sa_nracanaa usb_pipe *inpipe, sa_nracanaa usb_pipe *outpipe,
                  sa_nracanaa usbdevice_s *usbdev, จำนวนเต็ม lun)
{
    // Allocate drive structure.
    sa_nracanaa usbdrive_s *drive = malloc_fseg(parimaa_na(*drive));
    ถ้า (!drive) {
        warn_noalloc();
        คืน -1;
    }
    memset(drive, 0, parimaa_na(*drive));
    ถ้า (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    มิฉะนั้น
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    จำนวนเต็ม prio = bootprio_find_usb(usbdev, lun);
    จำนวนเต็ม ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    ถ้า (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        free(drive);
        คืน -1;
    }
    คืน 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
จำนวนเต็ม
usb_msc_setup(sa_nracanaa usbdevice_s *usbdev)
{
    ถ้า (!CONFIG_USB_MSC)
        คืน -1;

    // Verify right kind of device
    sa_nracanaa usb_interface_descriptor *iface = usbdev->iface;
    ถ้า ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        คืน -1;
    }

    // Find bulk in and bulk out endpoints.
    sa_nracanaa usb_pipe *inpipe = NULL, *outpipe = NULL;
    sa_nracanaa usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    sa_nracanaa usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    ถ้า (!indesc || !outdesc)
        goto fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    ถ้า (!inpipe || !outpipe)
        goto fail;

    จำนวนเต็ม maxlun = usb_msc_maxlun(usbdev->defpipe);
    จำนวนเต็ม lun, pipesused = 0;
    สำหรับ (lun = 0; lun < maxlun + 1; lun++) {
        จำนวนเต็ม ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        ถ้า (!ret)
            pipesused = 1;
    }

    ถ้า (!pipesused)
        goto fail;

    คืน 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    คืน -1;
}
