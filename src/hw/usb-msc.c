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

struct usbdrive_s {
    struct drive_s drive;
    struct usb_pipe *bulkin, *bulkout;
    цео lun;
};


/****************************************************************
 * Bulk-only drive command processing
 ****************************************************************/

#define USB_CDB_SIZE 12

#define CBW_SIGNATURE 0x43425355 // USBC

struct cbw_s {
    u32 dCBWSignature;
    u32 dCBWTag;
    u32 dCBWDataTransferLength;
    u8 bmCBWFlags;
    u8 bCBWLUN;
    u8 bCBWCBLength;
    u8 CBWCB[16];
} PACKED;

#define CSW_SIGNATURE 0x53425355 // USBS

struct csw_s {
    u32 dCSWSignature;
    u32 dCSWTag;
    u32 dCSWDataResidue;
    u8 bCSWStatus;
} PACKED;

static цео
usb_msc_send(struct usbdrive_s *udrive_gf, цео dir, void *buf, u32 bytes)
{
    struct usb_pipe *pipe;
    ако (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    инако
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    врати usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
цео
usb_process_op(struct disk_op_s *op)
{
    ако (!CONFIG_USB_MSC)
        врати 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->count, op->buf_fl);
    struct usbdrive_s *udrive_gf = container_of(
        op->drive_fl, struct usbdrive_s, drive);

    // Setup command block wrapper.
    struct cbw_s cbw;
    memset(&cbw, 0, sizeof(cbw));
    цео blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    ако (blocksize < 0)
        врати default_process_op(op);
    u32 bytes = blocksize * op->count;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    цео ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), sizeof(cbw));
    ако (ret)
        goto fail;

    // Transfer data to/from device.
    ако (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        ако (ret)
            goto fail;
    }

    // Transfer csw info.
    struct csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), sizeof(csw));
    ако (ret)
        goto fail;

    ако (!csw.bCSWStatus)
        врати DISK_RET_SUCCESS;
    ако (csw.bCSWStatus == 2)
        goto fail;

    ако (blocksize)
        op->count -= csw.dCSWDataResidue / blocksize;
    врати DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    врати DISK_RET_EBADTRACK;
}

static цео
usb_msc_maxlun(struct usb_pipe *pipe)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.wValue = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned char maxlun;
    цео ret = usb_send_default_control(pipe, &req, &maxlun);
    ако (ret)
        врати 0;
    врати maxlun;
}

static цео
usb_msc_lun_setup(struct usb_pipe *inpipe, struct usb_pipe *outpipe,
                  struct usbdevice_s *usbdev, цео lun)
{
    // Allocate drive structure.
    struct usbdrive_s *drive = malloc_fseg(sizeof(*drive));
    ако (!drive) {
        warn_noalloc();
        врати -1;
    }
    memset(drive, 0, sizeof(*drive));
    ако (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    инако
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    цео prio = bootprio_find_usb(usbdev, lun);
    цео ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    ако (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        free(drive);
        врати -1;
    }
    врати 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
цео
usb_msc_setup(struct usbdevice_s *usbdev)
{
    ако (!CONFIG_USB_MSC)
        врати -1;

    // Verify right kind of device
    struct usb_interface_descriptor *iface = usbdev->iface;
    ако ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        врати -1;
    }

    // Find bulk in and bulk out endpoints.
    struct usb_pipe *inpipe = NULL, *outpipe = NULL;
    struct usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    struct usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    ако (!indesc || !outdesc)
        goto fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    ако (!inpipe || !outpipe)
        goto fail;

    цео maxlun = usb_msc_maxlun(usbdev->defpipe);
    цео lun, pipesused = 0;
    за (lun = 0; lun < maxlun + 1; lun++) {
        цео ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        ако (!ret)
            pipesused = 1;
    }

    ако (!pipesused)
        goto fail;

    врати 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    врати -1;
}
