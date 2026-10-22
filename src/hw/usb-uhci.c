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

ರಚನೆ usb_uhci_s {
    ರಚನೆ usb_s usb;
    u16 iobase;
    ರಚನೆ uhci_qh *control_qh;
    ರಚನೆ uhci_framelist *framelist;
};

ರಚನೆ uhci_pipe {
    ರಚನೆ uhci_qh qh;
    ರಚನೆ uhci_td *ಮುಂದಿನ_td;
    ರಚನೆ usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
uhci_hub_detect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_uhci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + ಪೋರ್ಟ್ * 2;
    u16 ಸ್ಥಿತಿ_2 = inw(ioport);
    ಆದರೆ (!(ಸ್ಥಿತಿ_2 & USBPORTSC_CCS))
        // No device found.
        ಹಿಂತಿರುಗಿ 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_ಸಮಯ_DRSTR);
    ಹಿಂತಿರುಗಿ 1;
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
uhci_hub_reset(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_uhci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + ಪೋರ್ಟ್ * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 ಸ್ಥಿತಿ_2 = inw(ioport);
    ಆದರೆ (!(ಸ್ಥಿತಿ_2 & USBPORTSC_CCS))
        // No longer connected
        ಹಿಂತಿರುಗಿ -1;
    outw(USBPORTSC_PE, ioport);
    ಹಿಂತಿರುಗಿ !!(ಸ್ಥಿತಿ_2 & USBPORTSC_LSDA);
}

// Disable port
ಸ್ಥಿರ ಶೂನ್ಯ
uhci_hub_disconnect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_uhci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + ಪೋರ್ಟ್ * 2;
    outw(0, ioport);
}

ಸ್ಥಿರ ರಚನೆ usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
check_uhci_ports(ರಚನೆ usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    ರಚನೆ usbhub_s hub;
    memset(&hub, 0, ಗಾತ್ರ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    ಹಿಂತಿರುಗಿ hub.devcount;
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
    u32 ಕೊನೆ = timer_calc(1000 * 5);
    ಗಾಗಿ (;;) {
        ಆದರೆ (inw(iobase + USBFRNUM) != startframe)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ;
        }
        yield();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
uhci_free_pipes(ರಚನೆ usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    ರಚನೆ uhci_qh *pos = (ಶೂನ್ಯ*)(cntl->framelist->links[0] & ~UHCI_PTR_ಬಿಟ್ಟುಗಳು);
    ಗಾಗಿ (;;) {
        u32 link = pos->link;
        ಆದರೆ (link == UHCI_PTR_ಪದ)
            ನಿಲ್ಲಿಸು;
        ರಚನೆ uhci_qh *ಮುಂದಿನ = (ಶೂನ್ಯ*)(link & ~UHCI_PTR_ಬಿಟ್ಟುಗಳು);
        ರಚನೆ uhci_pipe *pipe = container_ದ(ಮುಂದಿನ, ರಚನೆ uhci_pipe, qh);
        ಆದರೆ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = ಮುಂದಿನ->link;
        ಇಲ್ಲದಿದ್ದರೆ
            pos = ಮುಂದಿನ;
    }
    uhci_waittick(cntl->iobase);
    ಗಾಗಿ (;;) {
        ರಚನೆ usb_pipe *usbpipe = cntl->usb.freelist;
        ಆದರೆ (!usbpipe)
            ನಿಲ್ಲಿಸು;
        cntl->usb.freelist = usbpipe->freenext;
        ರಚನೆ uhci_pipe *pipe = container_ದ(usbpipe, ರಚನೆ uhci_pipe, pipe);
        ಬಿಡುಗಡೆ(pipe);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
reset_uhci(ರಚನೆ usb_uhci_s *cntl, u16 bdf)
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
configure_uhci(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ usb_uhci_s *cntl = ದತ್ತಾಂಶ;

    // Allocate ram for schedule storage
    ರಚನೆ uhci_td *ಪದ_td = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*ಪದ_td));
    ರಚನೆ uhci_framelist *fl = memalign_ಮೇಲಿನ(ಗಾತ್ರ(*fl), ಗಾತ್ರ(*fl));
    ರಚನೆ uhci_pipe *intr_pipe = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*intr_pipe));
    ರಚನೆ uhci_pipe *ಪದ_pipe = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*ಪದ_pipe));
    ಆದರೆ (!ಪದ_td || !fl || !intr_pipe || !ಪದ_pipe) {
        warn_noalloc();
        ಹೋಗು fail;
    }

    // Work around for PIIX errata
    memset(ಪದ_td, 0, ಗಾತ್ರ(*ಪದ_td));
    ಪದ_td->link = UHCI_PTR_ಪದ;
    ಪದ_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_ಸರಿಸು)
                      | USB_PID_ಒಳ);
    memset(ಪದ_pipe, 0, ಗಾತ್ರ(*ಪದ_pipe));
    ಪದ_pipe->qh.element = (u32)ಪದ_td;
    ಪದ_pipe->qh.link = UHCI_PTR_ಪದ;
    ಪದ_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, ಗಾತ್ರ(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_ಪದ;
    intr_pipe->qh.link = (u32)&ಪದ_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ARRAY_ಗಾತ್ರ(fl->links); i++)
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
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    ಆದರೆ (ಎಣಿಕೆ_2)
        // Success
        ಹಿಂತಿರುಗಿ;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    ಬಿಡುಗಡೆ(ಪದ_td);
    ಬಿಡುಗಡೆ(fl);
    ಬಿಡುಗಡೆ(intr_pipe);
    ಬಿಡುಗಡೆ(ಪದ_pipe);
    ಬಿಡುಗಡೆ(cntl);
}

ಸ್ಥಿರ ಶೂನ್ಯ
uhci_controller_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_4);
    ಆದರೆ (!iobase)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ usb_uhci_s *cntl = malloc_tmphigh(ಗಾತ್ರ(*cntl));
    ಆದರೆ (!cntl) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(cntl, 0, ಗಾತ್ರ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    ಸತತ_thread(configure_uhci, cntl);
}

ಶೂನ್ಯ
uhci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_USB_UHCI)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_ಸಿದ್ಧತೆ(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

ಸ್ಥಿರ ರಚನೆ usb_pipe *
uhci_alloc_intr_pipe(ರಚನೆ usbdevice_s *usbdev
                     , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ರಚನೆ usb_uhci_s *cntl = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_uhci_s, usb);
    ಪೂರ್ಣಾಂಕ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ಆದರೆ (frameexp > 10)
        frameexp = 10;
    ಪೂರ್ಣಾಂಕ maxpacket = epdesc->w_Max_Packet_ಗಾತ್ರ;
    // Determine number of entries needed for 2 timer ticks.
    ಪೂರ್ಣಾಂಕ ms = 1<<frameexp;
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = DIV_ಸುತ್ತು_ಮೇಲೆ(ticks_ಗಮ್ಯ_ms(2), ms);
    ಎಣಿಕೆ_2 = ALIGN(ಎಣಿಕೆ_2, 2);
    ರಚನೆ uhci_pipe *pipe = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*pipe));
    ರಚನೆ uhci_td *tds = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*tds) * ಎಣಿಕೆ_2);
    ಶೂನ್ಯ *ದತ್ತಾಂಶ = malloc_ಕೆಳಗಿನ(maxpacket * ಎಣಿಕೆ_2);
    ಆದರೆ (!pipe || !tds || !ದತ್ತಾಂಶ) {
        warn_noalloc();
        ಹೋಗು fail;
    }
    memset(pipe, 0, ಗಾತ್ರ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    ಪೂರ್ಣಾಂಕ lowspeed = pipe->pipe.ವೇಗ;
    ಪೂರ್ಣಾಂಕ devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->ಮುಂದಿನ_td = &tds[0];
    pipe->iobase = cntl->iobase;

    ಪೂರ್ಣಾಂಕ toggle = 0;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ಎಣಿಕೆ_2; i++) {
        tds[i].link = (i==ಎಣಿಕೆ_2-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].ಸ್ಥಿತಿ_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ಸಕ್ರಿಯ);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_ಸರಿಸು)
                        | USB_PID_ಒಳ);
        tds[i].ಬಫರ್ = ದತ್ತಾಂಶ + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    ರಚನೆ uhci_framelist *fl = cntl->framelist;
    ಆದರೆ (frameexp == 0) {
        // Add to existing interrupt entry.
        ರಚನೆ uhci_qh *intr_qh = (ಶೂನ್ಯ*)(fl->links[0] & ~UHCI_PTR_ಬಿಟ್ಟುಗಳು);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        ಆದರೆ (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಪೂರ್ಣಾಂಕ startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        ಗಾಗಿ (i=startpos; i<ARRAY_ಗಾತ್ರ(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    ಹಿಂತಿರುಗಿ &pipe->pipe;
fail:
    ಬಿಡುಗಡೆ(pipe);
    ಬಿಡುಗಡೆ(tds);
    ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ usb_pipe *
uhci_realloc_pipe(ರಚನೆ usbdevice_s *usbdev, ರಚನೆ usb_pipe *upipe
                  , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಆದರೆ (! CONFIG_USB_UHCI)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    usb_ಕೂಡಿಸು_freelist(upipe);
    ಆದರೆ (!epdesc)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    u8 eptype = epdesc->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_INT)
        ಹಿಂತಿರುಗಿ uhci_alloc_intr_pipe(usbdev, epdesc);
    ರಚನೆ usb_uhci_s *cntl = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ರಚನೆ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ಆದರೆ (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        ಹಿಂತಿರುಗಿ usbpipe;
    }

    // Allocate a new queue head.
    ರಚನೆ uhci_pipe *pipe;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(ಗಾತ್ರ(*pipe));
    ಇಲ್ಲದಿದ್ದರೆ
        pipe = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*pipe));
    ಆದರೆ (!pipe) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(pipe, 0, ಗಾತ್ರ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_ಪದ;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    ರಚನೆ uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    ಹಿಂತಿರುಗಿ &pipe->pipe;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
wait_pipe(ರಚನೆ uhci_pipe *pipe, u32 ಕೊನೆ)
{
    ಗಾಗಿ (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        ಆದರೆ (el_link & UHCI_PTR_ಪದ)
            ಹಿಂತಿರುಗಿ 0;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            ರಚನೆ uhci_td *td = (ಶೂನ್ಯ*)(el_link & ~UHCI_PTR_ಬಿಟ್ಟುಗಳು);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (ಶೂನ್ಯ*)el_link, GET_LOWFLAT(td->ಸ್ಥಿತಿ_2)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            ಗಣ_LOWFLAT(pipe->qh.element, UHCI_PTR_ಪದ);
            uhci_waittick(iobase);
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
wait_td(ರಚನೆ uhci_td *td, u32 ಕೊನೆ)
{
    u32 ಸ್ಥಿತಿ_2;
    ಗಾಗಿ (;;) {
        ಸ್ಥಿತಿ_2 = td->ಸ್ಥಿತಿ_2;
        ಆದರೆ (!(ಸ್ಥಿತಿ_2 & TD_CTRL_ಸಕ್ರಿಯ))
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
    ಆದರೆ (ಸ್ಥಿತಿ_2 & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", ಸ್ಥಿತಿ_2);
        ಹಿಂತಿರುಗಿ -2;
    }
    ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು STACKTDS 16
#ವ್ಯಾಖ್ಯಾನಿಸು TDALIGN 16

ಪೂರ್ಣಾಂಕ
uhci_send_pipe(ರಚನೆ usb_pipe *p, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
               , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize)
{
    ಆದರೆ (! CONFIG_USB_UHCI)
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ uhci_pipe *pipe = container_ದ(p, ರಚನೆ uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, ದತ್ತಾಂಶ, datasize);
    ಪೂರ್ಣಾಂಕ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ಪೂರ್ಣಾಂಕ lowspeed = GET_LOWFLAT(pipe->pipe.ವೇಗ);
    ಪೂರ್ಣಾಂಕ devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    ಪೂರ್ಣಾಂಕ toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[ಗಾತ್ರ(ರಚನೆ uhci_td) * STACKTDS + TDALIGN - 1];
    ರಚನೆ uhci_td *tds = (ಶೂನ್ಯ*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, ಗಾತ್ರ(*tds) * STACKTDS);
    ಪೂರ್ಣಾಂಕ tdpos = 0;

    // Enable tds
    u32 ಕೊನೆ = timer_calc(usb_xfer_ಸಮಯ(p, datasize));
    barrier();
    ಗಣ_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    ಆದರೆ (cmd) {
        // Send setup pid on control transfers
        ರಚನೆ uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_ಆಳ;
        td->token = (uhci_explen(USB_CONTROL_ಸಿದ್ಧತೆ_ಗಾತ್ರ)
                     | (devaddr << TD_TOKEN_DEVADDR_ಸರಿಸು) | USB_PID_ಸಿದ್ಧತೆ);
        td->ಬಫರ್ = (ಶೂನ್ಯ*)cmd;
        barrier();
        td->ಸ್ಥಿತಿ_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ಸಕ್ರಿಯ);
        toggle = TD_TOKEN_TOGGLE;
    }
    ಆಗ (datasize) {
        // Send data pids
        ರಚನೆ uhci_td *td = &tds[tdpos++ % STACKTDS];
        ಪೂರ್ಣಾಂಕ ret = wait_td(td, ಕೊನೆ);
        ಆದರೆ (ret)
            ಹೋಗು fail;

        ಪೂರ್ಣಾಂಕ transfer = datasize;
        ಆದರೆ (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_ಪದ : (nexttd | UHCI_PTR_ಆಳ));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_ಸರಿಸು)
                     | (dir ? USB_PID_ಒಳ : USB_PID_ಹೊರಹೋಗು));
        td->ಬಫರ್ = ದತ್ತಾಂಶ;
        barrier();
        td->ಸ್ಥಿತಿ_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ಸಕ್ರಿಯ);
        toggle ^= TD_TOKEN_TOGGLE;

        ದತ್ತಾಂಶ += transfer;
        datasize -= transfer;
    }
    ಆದರೆ (cmd) {
        // Send status pid on control transfers
        ರಚನೆ uhci_td *td = &tds[tdpos++ % STACKTDS];
        ಪೂರ್ಣಾಂಕ ret = wait_td(td, ಕೊನೆ);
        ಆದರೆ (ret)
            ಹೋಗು fail;
        td->link = UHCI_PTR_ಪದ;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_ಸರಿಸು)
                     | (dir ? USB_PID_ಹೊರಹೋಗು : USB_PID_ಒಳ));
        td->ಬಫರ್ = 0;
        barrier();
        td->ಸ್ಥಿತಿ_2 = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ಸಕ್ರಿಯ);
    }
    ಗಣ_LOWFLAT(pipe->toggle, !!toggle);
    ಹಿಂತಿರುಗಿ wait_pipe(pipe, ಕೊನೆ);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    ಗಣ_LOWFLAT(pipe->qh.element, UHCI_PTR_ಪದ);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    ಹಿಂತಿರುಗಿ -1;
}

ಪೂರ್ಣಾಂಕ
uhci_poll_intr(ರಚನೆ usb_pipe *p, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ASSERT16();
    ಆದರೆ (! CONFIG_USB_UHCI)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ uhci_pipe *pipe = container_ದ(p, ರಚನೆ uhci_pipe, pipe);
    ರಚನೆ uhci_td *td = GET_LOWFLAT(pipe->ಮುಂದಿನ_td);
    u32 ಸ್ಥಿತಿ_2 = GET_LOWFLAT(td->ಸ್ಥಿತಿ_2);
    u32 token = GET_LOWFLAT(td->token);
    ಆದರೆ (ಸ್ಥಿತಿ_2 & TD_CTRL_ಸಕ್ರಿಯ)
        // No intrs found.
        ಹಿಂತಿರುಗಿ -1;
    // XXX - check for errors.

    // Copy data.
    ಶೂನ್ಯ *tddata = GET_LOWFLAT(td->ಬಫರ್);
    memcpy_far(GET_SEG(SS), ದತ್ತಾಂಶ, SEG_ಕೆಳಗಿನ, LOWFLAT2_ಕೆಳಗಿನ(tddata)
               , uhci_expected_ಉದ್ದಳತೆ(token));

    // Reenable this td.
    ರಚನೆ uhci_td *ಮುಂದಿನ = (ಶೂನ್ಯ*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_ಬಿಟ್ಟುಗಳು);
    ಗಣ_LOWFLAT(pipe->ಮುಂದಿನ_td, ಮುಂದಿನ);
    barrier();
    ಗಣ_LOWFLAT(td->ಸ್ಥಿತಿ_2, (uhci_maxerr(0) | (ಸ್ಥಿತಿ_2 & TD_CTRL_LS)
                             | TD_CTRL_ಸಕ್ರಿಯ));

    ಹಿಂತಿರುಗಿ 0;
}
