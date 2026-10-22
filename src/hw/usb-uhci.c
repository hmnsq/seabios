// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOWFLAT
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h" // pci_config_writew
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_4
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-ehci.h" // ehci_wait_controllers
#ಅನ್ತರ್ಭಾವ "usb-uhci.h" // USBLEGSUP
#ಅನ್ತರ್ಭಾವ "util.h" // msleep
#ಅನ್ತರ್ಭಾವ "x86.h" // outw

ಸಂರಚನಾ usb_uhci_s {
    ಸಂರಚನಾ usb_s usb;
    u16 iobase;
    ಸಂರಚನಾ uhci_qh *control_qh;
    ಸಂರಚನಾ uhci_framelist *framelist;
};

ಸಂರಚನಾ uhci_pipe {
    ಸಂರಚನಾ uhci_qh qh;
    ಸಂರಚನಾ uhci_td *next_td;
    ಸಂರಚನಾ usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
uhci_hub_detect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_uhci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    ಯದಿ (!(status & USBPORTSC_CCS))
        // No device found.
        ಪ್ರತ್ಯಾಗಮ 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    ಪ್ರತ್ಯಾಗಮ 1;
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
uhci_hub_reset(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_uhci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    ಯದಿ (!(status & USBPORTSC_CCS))
        // No longer connected
        ಪ್ರತ್ಯಾಗಮ -1;
    outw(USBPORTSC_PE, ioport);
    ಪ್ರತ್ಯಾಗಮ !!(status & USBPORTSC_LSDA);
}

// Disable port
ಸ್ಥಿರ ಶೂನ್ಯ
uhci_hub_disconnect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_uhci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

ಸ್ಥಿರ ಸಂರಚನಾ usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
check_uhci_ports(ಸಂರಚನಾ usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    ಸಂರಚನಾ usbhub_s hub;
    memset(&hub, 0, ಪರಿಮಾಣ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    ಪ್ರತ್ಯಾಗಮ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ಸ್ಥಿರ ಶೂನ್ಯ
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    ಪ್ರತಿ (;;) {
        ಯದಿ (inw(iobase + USBFRNUM) != startframe)
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ;
        }
        yield();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
uhci_free_pipes(ಸಂರಚನಾ usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    ಸಂರಚನಾ uhci_qh *pos = (ಶೂನ್ಯ*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    ಪ್ರತಿ (;;) {
        u32 link = pos->link;
        ಯದಿ (link == UHCI_PTR_TERM)
            ಭಙ್ಗ;
        ಸಂರಚನಾ uhci_qh *next = (ಶೂನ್ಯ*)(link & ~UHCI_PTR_BITS);
        ಸಂರಚನಾ uhci_pipe *pipe = container_of(next, ಸಂರಚನಾ uhci_pipe, qh);
        ಯದಿ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        ಅನ್ಯಥಾ
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    ಪ್ರತಿ (;;) {
        ಸಂರಚನಾ usb_pipe *usbpipe = cntl->usb.freelist;
        ಯದಿ (!usbpipe)
            ಭಙ್ಗ;
        cntl->usb.freelist = usbpipe->freenext;
        ಸಂರಚನಾ uhci_pipe *pipe = container_of(usbpipe, ಸಂರಚನಾ uhci_pipe, pipe);
        ಮುಕ್ತ(pipe);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
reset_uhci(ಸಂರಚನಾ usb_uhci_s *cntl, u16 bdf)
{
    // XXX - don't reset if not needed.

    // Reset PIRQ and SMI
    pci_config_writew(bdf, USBLEGSUP, USBLEGSUP_RWC);

    // Reset the HC
    outw(USBCMD_HCRESET, cntl->iobase + USBCMD);
    udelay(5);

    // Disable interrupts and commands (just to be safe).
    outw(0, cntl->iobase + USBINTR);
    outw(0, cntl->iobase + USBCMD);
}

ಸ್ಥಿರ ಶೂನ್ಯ
configure_uhci(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    ಸಂರಚನಾ uhci_td *term_td = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*term_td));
    ಸಂರಚನಾ uhci_framelist *fl = memalign_ಮೇಲಿನ(ಪರಿಮಾಣ(*fl), ಪರಿಮಾಣ(*fl));
    ಸಂರಚನಾ uhci_pipe *intr_pipe = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*intr_pipe));
    ಸಂರಚನಾ uhci_pipe *term_pipe = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*term_pipe));
    ಯದಿ (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        ಗಚ್ಛ fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, ಪರಿಮಾಣ(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, ಪರಿಮಾಣ(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, ಪರಿಮಾಣ(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)&intr_pipe->qh | UHCI_PTR_QH;
    cntl->framelist = fl;
    cntl->control_qh = &intr_pipe->qh;
    barrier();

    // Set the frame length to the default: 1 ms exactly
    outb(USBSOF_DEFAULT, cntl->iobase + USBSOF);

    // Store the frame list base address
    outl((u32)fl->links, cntl->iobase + USBFLBASEADD);

    // Set the current frame number
    outw(0, cntl->iobase + USBFRNUM);

    // Mark as configured and running with a 64-byte max packet.
    outw(USBCMD_RS | USBCMD_CF | USBCMD_MAXP, cntl->iobase + USBCMD);

    // Find devices
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    ಯದಿ (ಎಣಿಕೆ)
        // Success
        ಪ್ರತ್ಯಾಗಮ;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    ಮುಕ್ತ(term_td);
    ಮುಕ್ತ(fl);
    ಮುಕ್ತ(intr_pipe);
    ಮುಕ್ತ(term_pipe);
    ಮುಕ್ತ(cntl);
}

ಸ್ಥಿರ ಶೂನ್ಯ
uhci_controller_setup(ಸಂರಚನಾ pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    ಯದಿ (!iobase)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ usb_uhci_s *cntl = malloc_tmphigh(ಪರಿಮಾಣ(*cntl));
    ಯದಿ (!cntl) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(cntl, 0, ಪರಿಮಾಣ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

ಶೂನ್ಯ
uhci_setup(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_USB_UHCI)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

ಸ್ಥಿರ ಸಂರಚನಾ usb_pipe *
uhci_alloc_intr_pipe(ಸಂರಚನಾ usbdevice_s *usbdev
                     , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಸಂರಚನಾ usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_uhci_s, usb);
    ಪೂರ್ಣಾಙ್ಕ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ಯದಿ (frameexp > 10)
        frameexp = 10;
    ಪೂರ್ಣಾಙ್ಕ maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    ಪೂರ್ಣಾಙ್ಕ ms = 1<<frameexp;
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = DIV_ROUND_UP(ticks_to_ms(2), ms);
    ಎಣಿಕೆ = ALIGN(ಎಣಿಕೆ, 2);
    ಸಂರಚನಾ uhci_pipe *pipe = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*pipe));
    ಸಂರಚನಾ uhci_td *tds = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*tds) * ಎಣಿಕೆ);
    ಶೂನ್ಯ *data = malloc_ಕೆಳಗಿನ(maxpacket * ಎಣಿಕೆ);
    ಯದಿ (!pipe || !tds || !data) {
        warn_noalloc();
        ಗಚ್ಛ fail;
    }
    memset(pipe, 0, ಪರಿಮಾಣ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    ಪೂರ್ಣಾಙ್ಕ lowspeed = pipe->pipe.speed;
    ಪೂರ್ಣಾಙ್ಕ devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    ಪೂರ್ಣಾಙ್ಕ toggle = 0;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ಎಣಿಕೆ; i++) {
        tds[i].link = (i==ಎಣಿಕೆ-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    ಸಂರಚನಾ uhci_framelist *fl = cntl->framelist;
    ಯದಿ (frameexp == 0) {
        // Add to existing interrupt entry.
        ಸಂರಚನಾ uhci_qh *intr_qh = (ಶೂನ್ಯ*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        ಯದಿ (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } ಅನ್ಯಥಾ {
        ಪೂರ್ಣಾಙ್ಕ startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        ಪ್ರತಿ (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    ಪ್ರತ್ಯಾಗಮ &pipe->pipe;
fail:
    ಮುಕ್ತ(pipe);
    ಮುಕ್ತ(tds);
    ಮುಕ್ತ(data);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ usb_pipe *
uhci_realloc_pipe(ಸಂರಚನಾ usbdevice_s *usbdev, ಸಂರಚನಾ usb_pipe *upipe
                  , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಯದಿ (! CONFIG_USB_UHCI)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    usb_add_freelist(upipe);
    ಯದಿ (!epdesc)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_INT)
        ಪ್ರತ್ಯಾಗಮ uhci_alloc_intr_pipe(usbdev, epdesc);
    ಸಂರಚನಾ usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ಸಂರಚನಾ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ಯದಿ (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        ಪ್ರತ್ಯಾಗಮ usbpipe;
    }

    // Allocate a new queue head.
    ಸಂರಚನಾ uhci_pipe *pipe;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(ಪರಿಮಾಣ(*pipe));
    ಅನ್ಯಥಾ
        pipe = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*pipe));
    ಯದಿ (!pipe) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(pipe, 0, ಪರಿಮಾಣ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    ಸಂರಚನಾ uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    ಪ್ರತ್ಯಾಗಮ &pipe->pipe;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
wait_pipe(ಸಂರಚನಾ uhci_pipe *pipe, u32 end)
{
    ಪ್ರತಿ (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        ಯದಿ (el_link & UHCI_PTR_TERM)
            ಪ್ರತ್ಯಾಗಮ 0;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            ಸಂರಚನಾ uhci_td *td = (ಶೂನ್ಯ*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (ಶೂನ್ಯ*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
wait_td(ಸಂರಚನಾ uhci_td *td, u32 end)
{
    u32 status;
    ಪ್ರತಿ (;;) {
        status = td->status;
        ಯದಿ (!(status & TD_CTRL_ACTIVE))
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
    ಯದಿ (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        ಪ್ರತ್ಯಾಗಮ -2;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

#define STACKTDS 16
#define TDALIGN 16

ಪೂರ್ಣಾಙ್ಕ
uhci_send_pipe(ಸಂರಚನಾ usb_pipe *p, ಪೂರ್ಣಾಙ್ಕ dir, ಧ್ರುವ ಶೂನ್ಯ *cmd
               , ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datasize)
{
    ಯದಿ (! CONFIG_USB_UHCI)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ uhci_pipe *pipe = container_of(p, ಸಂರಚನಾ uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    ಪೂರ್ಣಾಙ್ಕ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ಪೂರ್ಣಾಙ್ಕ lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    ಪೂರ್ಣಾಙ್ಕ devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    ಪೂರ್ಣಾಙ್ಕ toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[ಪರಿಮಾಣ(ಸಂರಚನಾ uhci_td) * STACKTDS + TDALIGN - 1];
    ಸಂರಚನಾ uhci_td *tds = (ಶೂನ್ಯ*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, ಪರಿಮಾಣ(*tds) * STACKTDS);
    ಪೂರ್ಣಾಙ್ಕ tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    ಯದಿ (cmd) {
        // Send setup pid on control transfers
        ಸಂರಚನಾ uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (ಶೂನ್ಯ*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    ಯಾವತ್ (datasize) {
        // Send data pids
        ಸಂರಚನಾ uhci_td *td = &tds[tdpos++ % STACKTDS];
        ಪೂರ್ಣಾಙ್ಕ ret = wait_td(td, end);
        ಯದಿ (ret)
            ಗಚ್ಛ fail;

        ಪೂರ್ಣಾಙ್ಕ transfer = datasize;
        ಯದಿ (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_TERM : (nexttd | UHCI_PTR_DEPTH));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_IN : USB_PID_OUT));
        td->buffer = data;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle ^= TD_TOKEN_TOGGLE;

        data += transfer;
        datasize -= transfer;
    }
    ಯದಿ (cmd) {
        // Send status pid on control transfers
        ಸಂರಚನಾ uhci_td *td = &tds[tdpos++ % STACKTDS];
        ಪೂರ್ಣಾಙ್ಕ ret = wait_td(td, end);
        ಯದಿ (ret)
            ಗಚ್ಛ fail;
        td->link = UHCI_PTR_TERM;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_OUT : USB_PID_IN));
        td->buffer = 0;
        barrier();
        td->status = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
    }
    SET_LOWFLAT(pipe->toggle, !!toggle);
    ಪ್ರತ್ಯಾಗಮ wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಪೂರ್ಣಾಙ್ಕ
uhci_poll_intr(ಸಂರಚನಾ usb_pipe *p, ಶೂನ್ಯ *data)
{
    ASSERT16();
    ಯದಿ (! CONFIG_USB_UHCI)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ uhci_pipe *pipe = container_of(p, ಸಂರಚನಾ uhci_pipe, pipe);
    ಸಂರಚನಾ uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    ಯದಿ (status & TD_CTRL_ACTIVE)
        // No intrs found.
        ಪ್ರತ್ಯಾಗಮ -1;
    // XXX - check for errors.

    // Copy data.
    ಶೂನ್ಯ *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_ಕೆಳಗಿನ, LOWFLAT2_ಕೆಳಗಿನ(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    ಸಂರಚನಾ uhci_td *next = (ಶೂನ್ಯ*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    ಪ್ರತ್ಯಾಗಮ 0;
}
