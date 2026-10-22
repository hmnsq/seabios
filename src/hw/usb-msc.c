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
    số_nguyên lun;
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

static số_nguyên
usb_msc_send(struct usbdrive_s *udrive_gf, số_nguyên dir, void *buf, u32 bytes)
{
    struct usb_pipe *pipe;
    nếu (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    không_thì
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    trả_về usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
số_nguyên
usb_process_op(struct disk_op_s *op)
{
    nếu (!CONFIG_USB_MSC)
        trả_về 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->count, op->buf_fl);
    struct usbdrive_s *udrive_gf = container_of(
        op->drive_fl, struct usbdrive_s, drive);

    // Setup command block wrapper.
    struct cbw_s cbw;
    memset(&cbw, 0, sizeof(cbw));
    số_nguyên blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    nếu (blocksize < 0)
        trả_về default_process_op(op);
    u32 bytes = blocksize * op->count;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    số_nguyên ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), sizeof(cbw));
    nếu (ret)
        goto fail;

    // Transfer data to/from device.
    nếu (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        nếu (ret)
            goto fail;
    }

    // Transfer csw info.
    struct csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), sizeof(csw));
    nếu (ret)
        goto fail;

    nếu (!csw.bCSWStatus)
        trả_về DISK_RET_SUCCESS;
    nếu (csw.bCSWStatus == 2)
        goto fail;

    nếu (blocksize)
        op->count -= csw.dCSWDataResidue / blocksize;
    trả_về DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    trả_về DISK_RET_EBADTRACK;
}

static số_nguyên
usb_msc_maxlun(struct usb_pipe *pipe)
{
    struct usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.wValue = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned char maxlun;
    số_nguyên ret = usb_send_default_control(pipe, &req, &maxlun);
    nếu (ret)
        trả_về 0;
    trả_về maxlun;
}

static số_nguyên
usb_msc_lun_setup(struct usb_pipe *inpipe, struct usb_pipe *outpipe,
                  struct usbdevice_s *usbdev, số_nguyên lun)
{
    // Allocate drive structure.
    struct usbdrive_s *drive = malloc_fseg(sizeof(*drive));
    nếu (!drive) {
        warn_noalloc();
        trả_về -1;
    }
    memset(drive, 0, sizeof(*drive));
    nếu (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    không_thì
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    số_nguyên prio = bootprio_find_usb(usbdev, lun);
    số_nguyên ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    nếu (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        free(drive);
        trả_về -1;
    }
    trả_về 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
số_nguyên
usb_msc_setup(struct usbdevice_s *usbdev)
{
    nếu (!CONFIG_USB_MSC)
        trả_về -1;

    // Verify right kind of device
    struct usb_interface_descriptor *iface = usbdev->iface;
    nếu ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        trả_về -1;
    }

    // Find bulk in and bulk out endpoints.
    struct usb_pipe *inpipe = NULL, *outpipe = NULL;
    struct usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    struct usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    nếu (!indesc || !outdesc)
        goto fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    nếu (!inpipe || !outpipe)
        goto fail;

    số_nguyên maxlun = usb_msc_maxlun(usbdev->defpipe);
    số_nguyên lun, pipesused = 0;
    cho (lun = 0; lun < maxlun + 1; lun++) {
        số_nguyên ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        nếu (!ret)
            pipesused = 1;
    }

    nếu (!pipesused)
        goto fail;

    trả_về 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    trả_về -1;
}
