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
    ཧྲིལ lun;
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

static ཧྲིལ
usb_msc_send(sa_nracanaa usbdrive_s *udrive_gf, ཧྲིལ dir, void *buf, u32 bytes)
{
    sa_nracanaa usb_pipe *pipe;
    གལ་སྲིད (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    དེ་མིན
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    ལོག usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
ཧྲིལ
usb_process_op(sa_nracanaa disk_op_s *op)
{
    གལ་སྲིད (!CONFIG_USB_MSC)
        ལོག 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->count, op->buf_fl);
    sa_nracanaa usbdrive_s *udrive_gf = container_of(
        op->drive_fl, sa_nracanaa usbdrive_s, drive);

    // Setup command block wrapper.
    sa_nracanaa cbw_s cbw;
    memset(&cbw, 0, parimaa_na(cbw));
    ཧྲིལ blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    གལ་སྲིད (blocksize < 0)
        ལོག default_process_op(op);
    u32 bytes = blocksize * op->count;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    ཧྲིལ ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), parimaa_na(cbw));
    གལ་སྲིད (ret)
        goto fail;

    // Transfer data to/from device.
    གལ་སྲིད (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        གལ་སྲིད (ret)
            goto fail;
    }

    // Transfer csw info.
    sa_nracanaa csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), parimaa_na(csw));
    གལ་སྲིད (ret)
        goto fail;

    གལ་སྲིད (!csw.bCSWStatus)
        ལོག DISK_RET_SUCCESS;
    གལ་སྲིད (csw.bCSWStatus == 2)
        goto fail;

    གལ་སྲིད (blocksize)
        op->count -= csw.dCSWDataResidue / blocksize;
    ལོག DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    ལོག DISK_RET_EBADTRACK;
}

static ཧྲིལ
usb_msc_maxlun(sa_nracanaa usb_pipe *pipe)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.wValue = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned var_na maxlun;
    ཧྲིལ ret = usb_send_default_control(pipe, &req, &maxlun);
    གལ་སྲིད (ret)
        ལོག 0;
    ལོག maxlun;
}

static ཧྲིལ
usb_msc_lun_setup(sa_nracanaa usb_pipe *inpipe, sa_nracanaa usb_pipe *outpipe,
                  sa_nracanaa usbdevice_s *usbdev, ཧྲིལ lun)
{
    // Allocate drive structure.
    sa_nracanaa usbdrive_s *drive = malloc_fseg(parimaa_na(*drive));
    གལ་སྲིད (!drive) {
        warn_noalloc();
        ལོག -1;
    }
    memset(drive, 0, parimaa_na(*drive));
    གལ་སྲིད (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    དེ་མིན
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    ཧྲིལ prio = bootprio_find_usb(usbdev, lun);
    ཧྲིལ ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    གལ་སྲིད (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        free(drive);
        ལོག -1;
    }
    ལོག 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
ཧྲིལ
usb_msc_setup(sa_nracanaa usbdevice_s *usbdev)
{
    གལ་སྲིད (!CONFIG_USB_MSC)
        ལོག -1;

    // Verify right kind of device
    sa_nracanaa usb_interface_descriptor *iface = usbdev->iface;
    གལ་སྲིད ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ལོག -1;
    }

    // Find bulk in and bulk out endpoints.
    sa_nracanaa usb_pipe *inpipe = NULL, *outpipe = NULL;
    sa_nracanaa usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    sa_nracanaa usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    གལ་སྲིད (!indesc || !outdesc)
        goto fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    གལ་སྲིད (!inpipe || !outpipe)
        goto fail;

    ཧྲིལ maxlun = usb_msc_maxlun(usbdev->defpipe);
    ཧྲིལ lun, pipesused = 0;
    དོན་ལུ (lun = 0; lun < maxlun + 1; lun++) {
        ཧྲིལ ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        གལ་སྲིད (!ret)
            pipesused = 1;
    }

    གལ་སྲིད (!pipesused)
        goto fail;

    ལོག 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    ལོག -1;
}
