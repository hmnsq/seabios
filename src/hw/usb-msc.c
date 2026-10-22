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
    ကိန်းပြည့် lun;
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

static ကိန်းပြည့်
usb_msc_send(sa_nracanaa usbdrive_s *udrive_gf, ကိန်းပြည့် dir, void *buf, u32 bytes)
{
    sa_nracanaa usb_pipe *pipe;
    အကယ်၍ (dir == USB_DIR_OUT)
        pipe = GET_GLOBALFLAT(udrive_gf->bulkout);
    မဟုတ်ပါက
        pipe = GET_GLOBALFLAT(udrive_gf->bulkin);
    ပြန်ပေး usb_send_bulk(pipe, dir, buf, bytes);
}

// Low-level usb command transmit function.
ကိန်းပြည့်
usb_process_op(sa_nracanaa disk_op_s *op)
{
    အကယ်၍ (!CONFIG_USB_MSC)
        ပြန်ပေး 0;

    dprintf(16, "usb_cmd_data id=%p write=%d count=%d buf=%p\n"
            , op->drive_fl, 0, op->count, op->buf_fl);
    sa_nracanaa usbdrive_s *udrive_gf = container_of(
        op->drive_fl, sa_nracanaa usbdrive_s, drive);

    // Setup command block wrapper.
    sa_nracanaa cbw_s cbw;
    memset(&cbw, 0, parimaa_na(cbw));
    ကိန်းပြည့် blocksize = scsi_fill_cmd(op, cbw.CBWCB, USB_CDB_SIZE);
    အကယ်၍ (blocksize < 0)
        ပြန်ပေး default_process_op(op);
    u32 bytes = blocksize * op->count;
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 999; // XXX
    cbw.dCBWDataTransferLength = bytes;
    cbw.bmCBWFlags = scsi_is_read(op) ? USB_DIR_IN : USB_DIR_OUT;
    cbw.bCBWLUN = GET_GLOBALFLAT(udrive_gf->lun);
    cbw.bCBWCBLength = USB_CDB_SIZE;

    // Transfer cbw to device.
    ကိန်းပြည့် ret = usb_msc_send(udrive_gf, USB_DIR_OUT
                           , MAKE_FLATPTR(GET_SEG(SS), &cbw), parimaa_na(cbw));
    အကယ်၍ (ret)
        goto fail;

    // Transfer data to/from device.
    အကယ်၍ (bytes) {
        ret = usb_msc_send(udrive_gf, cbw.bmCBWFlags, op->buf_fl, bytes);
        အကယ်၍ (ret)
            goto fail;
    }

    // Transfer csw info.
    sa_nracanaa csw_s csw;
    ret = usb_msc_send(udrive_gf, USB_DIR_IN
                       , MAKE_FLATPTR(GET_SEG(SS), &csw), parimaa_na(csw));
    အကယ်၍ (ret)
        goto fail;

    အကယ်၍ (!csw.bCSWStatus)
        ပြန်ပေး DISK_RET_SUCCESS;
    အကယ်၍ (csw.bCSWStatus == 2)
        goto fail;

    အကယ်၍ (blocksize)
        op->count -= csw.dCSWDataResidue / blocksize;
    ပြန်ပေး DISK_RET_EBADTRACK;

fail:
    // XXX - reset connection
    dprintf(1, "USB transmission failed\n");
    ပြန်ပေး DISK_RET_EBADTRACK;
}

static ကိန်းပြည့်
usb_msc_maxlun(sa_nracanaa usb_pipe *pipe)
{
    sa_nracanaa usb_ctrlrequest req;
    req.bRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    req.bRequest = 0xfe;
    req.wValue = 0;
    req.wIndex = 0;
    req.wLength = 1;
    unsigned var_na maxlun;
    ကိန်းပြည့် ret = usb_send_default_control(pipe, &req, &maxlun);
    အကယ်၍ (ret)
        ပြန်ပေး 0;
    ပြန်ပေး maxlun;
}

static ကိန်းပြည့်
usb_msc_lun_setup(sa_nracanaa usb_pipe *inpipe, sa_nracanaa usb_pipe *outpipe,
                  sa_nracanaa usbdevice_s *usbdev, ကိန်းပြည့် lun)
{
    // Allocate drive structure.
    sa_nracanaa usbdrive_s *drive = malloc_fseg(parimaa_na(*drive));
    အကယ်၍ (!drive) {
        warn_noalloc();
        ပြန်ပေး -1;
    }
    memset(drive, 0, parimaa_na(*drive));
    အကယ်၍ (usb_32bit_pipe(inpipe))
        drive->drive.type = DTYPE_USB_32;
    မဟုတ်ပါက
        drive->drive.type = DTYPE_USB;
    drive->bulkin = inpipe;
    drive->bulkout = outpipe;
    drive->lun = lun;

    ကိန်းပြည့် prio = bootprio_find_usb(usbdev, lun);
    ကိန်းပြည့် ret = scsi_drive_setup(&drive->drive, "USB MSC", prio);
    အကယ်၍ (ret) {
        dprintf(1, "Unable to configure USB MSC drive.\n");
        free(drive);
        ပြန်ပေး -1;
    }
    ပြန်ပေး 0;
}

/****************************************************************
 * Setup
 ****************************************************************/

// Configure a usb msc device.
ကိန်းပြည့်
usb_msc_setup(sa_nracanaa usbdevice_s *usbdev)
{
    အကယ်၍ (!CONFIG_USB_MSC)
        ပြန်ပေး -1;

    // Verify right kind of device
    sa_nracanaa usb_interface_descriptor *iface = usbdev->iface;
    အကယ်၍ ((iface->bInterfaceSubClass != US_SC_SCSI &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8070 &&
         iface->bInterfaceSubClass != US_SC_ATAPI_8020)
        || iface->bInterfaceProtocol != US_PR_BULK) {
        dprintf(1, "Unsupported MSC USB device (subclass=%02x proto=%02x)\n"
                , iface->bInterfaceSubClass, iface->bInterfaceProtocol);
        ပြန်ပေး -1;
    }

    // Find bulk in and bulk out endpoints.
    sa_nracanaa usb_pipe *inpipe = NULL, *outpipe = NULL;
    sa_nracanaa usb_endpoint_descriptor *indesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_IN);
    sa_nracanaa usb_endpoint_descriptor *outdesc = usb_find_desc(
        usbdev, USB_ENDPOINT_XFER_BULK, USB_DIR_OUT);
    အကယ်၍ (!indesc || !outdesc)
        goto fail;
    inpipe = usb_alloc_pipe(usbdev, indesc);
    outpipe = usb_alloc_pipe(usbdev, outdesc);
    အကယ်၍ (!inpipe || !outpipe)
        goto fail;

    ကိန်းပြည့် maxlun = usb_msc_maxlun(usbdev->defpipe);
    ကိန်းပြည့် lun, pipesused = 0;
    အတွက် (lun = 0; lun < maxlun + 1; lun++) {
        ကိန်းပြည့် ret = usb_msc_lun_setup(inpipe, outpipe, usbdev, lun);
        အကယ်၍ (!ret)
            pipesused = 1;
    }

    အကယ်၍ (!pipesused)
        goto fail;

    ပြန်ပေး 0;
fail:
    dprintf(1, "Unable to configure USB MSC device.\n");
    usb_free_pipe(usbdev, inpipe);
    usb_free_pipe(usbdev, outpipe);
    ပြန်ပေး -1;
}
