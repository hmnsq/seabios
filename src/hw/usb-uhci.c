// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_LOWFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pci.h" // pci_config_writew
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_BASE_ADDRESS_4
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // struct usb_s
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-ehci.h" // ehci_wait_controllers
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-uhci.h" // USBLEGSUP
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // msleep
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // outw

ਬਣਤਰ usb_uhci_s {
    ਬਣਤਰ usb_s usb;
    u16 iobase;
    ਬਣਤਰ uhci_qh *control_qh;
    ਬਣਤਰ uhci_framelist *framelist;
};

ਬਣਤਰ uhci_pipe {
    ਬਣਤਰ uhci_qh qh;
    ਬਣਤਰ uhci_td *ਅਗਲਾ_td;
    ਬਣਤਰ usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
ਸਥਿਰ ਪੂਰਨ
uhci_hub_detect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_uhci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + ਪੋਰਟ * 2;
    u16 ਸਥਿਤੀ = inw(ioport);
    ਜੇ (!(ਸਥਿਤੀ & USBPORTSC_CCS))
        // No device found.
        ਵਾਪਸ 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_ਸਮਾਂ_DRSTR);
    ਵਾਪਸ 1;
}

// Reset device on port
ਸਥਿਰ ਪੂਰਨ
uhci_hub_reset(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_uhci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + ਪੋਰਟ * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 ਸਥਿਤੀ = inw(ioport);
    ਜੇ (!(ਸਥਿਤੀ & USBPORTSC_CCS))
        // No longer connected
        ਵਾਪਸ -1;
    outw(USBPORTSC_PE, ioport);
    ਵਾਪਸ !!(ਸਥਿਤੀ & USBPORTSC_LSDA);
}

// Disable port
ਸਥਿਰ ਖਾਲੀ
uhci_hub_disconnect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_uhci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + ਪੋਰਟ * 2;
    outw(0, ioport);
}

ਸਥਿਰ ਬਣਤਰ usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ਸਥਿਰ ਪੂਰਨ
check_uhci_ports(ਬਣਤਰ usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    ਬਣਤਰ usbhub_s hub;
    memset(&hub, 0, ਆਕਾਰ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    ਵਾਪਸ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ਸਥਿਰ ਖਾਲੀ
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 ਸਿਰਾ = timer_calc(1000 * 5);
    ਲਈ (;;) {
        ਜੇ (inw(iobase + USBFRNUM) != startframe)
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ;
        }
        yield();
    }
}

ਸਥਿਰ ਖਾਲੀ
uhci_free_pipes(ਬਣਤਰ usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    ਬਣਤਰ uhci_qh *pos = (ਖਾਲੀ*)(cntl->framelist->links[0] & ~UHCI_PTR_ਬਿੱਟ);
    ਲਈ (;;) {
        u32 link = pos->link;
        ਜੇ (link == UHCI_PTR_ਪਦ)
            ਰੋਕੋ;
        ਬਣਤਰ uhci_qh *ਅਗਲਾ = (ਖਾਲੀ*)(link & ~UHCI_PTR_ਬਿੱਟ);
        ਬਣਤਰ uhci_pipe *pipe = container_ਦਾ(ਅਗਲਾ, ਬਣਤਰ uhci_pipe, qh);
        ਜੇ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = ਅਗਲਾ->link;
        ਨਹੀਂਤਾਂ
            pos = ਅਗਲਾ;
    }
    uhci_waittick(cntl->iobase);
    ਲਈ (;;) {
        ਬਣਤਰ usb_pipe *usbpipe = cntl->usb.freelist;
        ਜੇ (!usbpipe)
            ਰੋਕੋ;
        cntl->usb.freelist = usbpipe->freenext;
        ਬਣਤਰ uhci_pipe *pipe = container_ਦਾ(usbpipe, ਬਣਤਰ uhci_pipe, pipe);
        ਮੁਕਤ(pipe);
    }
}

ਸਥਿਰ ਖਾਲੀ
reset_uhci(ਬਣਤਰ usb_uhci_s *cntl, u16 bdf)
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

ਸਥਿਰ ਖਾਲੀ
configure_uhci(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ usb_uhci_s *cntl = ਅੰਕੜੇ;

    // Allocate ram for schedule storage
    ਬਣਤਰ uhci_td *ਪਦ_td = malloc_ਉੱਚਾ(ਆਕਾਰ(*ਪਦ_td));
    ਬਣਤਰ uhci_framelist *fl = memalign_ਉੱਚਾ(ਆਕਾਰ(*fl), ਆਕਾਰ(*fl));
    ਬਣਤਰ uhci_pipe *intr_pipe = malloc_ਉੱਚਾ(ਆਕਾਰ(*intr_pipe));
    ਬਣਤਰ uhci_pipe *ਪਦ_pipe = malloc_ਉੱਚਾ(ਆਕਾਰ(*ਪਦ_pipe));
    ਜੇ (!ਪਦ_td || !fl || !intr_pipe || !ਪਦ_pipe) {
        warn_noalloc();
        ਜਾਓ fail;
    }

    // Work around for PIIX errata
    memset(ਪਦ_td, 0, ਆਕਾਰ(*ਪਦ_td));
    ਪਦ_td->link = UHCI_PTR_ਪਦ;
    ਪਦ_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_ਖਿਸਕਾਓ)
                      | USB_PID_ਵਿੱਚ);
    memset(ਪਦ_pipe, 0, ਆਕਾਰ(*ਪਦ_pipe));
    ਪਦ_pipe->qh.element = (u32)ਪਦ_td;
    ਪਦ_pipe->qh.link = UHCI_PTR_ਪਦ;
    ਪਦ_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, ਆਕਾਰ(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_ਪਦ;
    intr_pipe->qh.link = (u32)&ਪਦ_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ARRAY_ਮਾਪ(fl->links); i++)
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
    ਪੂਰਨ ਗਿਣਤੀ_2 = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    ਜੇ (ਗਿਣਤੀ_2)
        // Success
        ਵਾਪਸ;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    ਮੁਕਤ(ਪਦ_td);
    ਮੁਕਤ(fl);
    ਮੁਕਤ(intr_pipe);
    ਮੁਕਤ(ਪਦ_pipe);
    ਮੁਕਤ(cntl);
}

ਸਥਿਰ ਖਾਲੀ
uhci_controller_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_4);
    ਜੇ (!iobase)
        ਵਾਪਸ;

    ਬਣਤਰ usb_uhci_s *cntl = malloc_tmphigh(ਆਕਾਰ(*cntl));
    ਜੇ (!cntl) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(cntl, 0, ਆਕਾਰ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    ਲੜੀਵਾਰ_thread(configure_uhci, cntl);
}

ਖਾਲੀ
uhci_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_USB_UHCI)
        ਵਾਪਸ;
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_ਤਿਆਰੀ(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

ਸਥਿਰ ਬਣਤਰ usb_pipe *
uhci_alloc_intr_pipe(ਬਣਤਰ usbdevice_s *usbdev
                     , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਬਣਤਰ usb_uhci_s *cntl = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_uhci_s, usb);
    ਪੂਰਨ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ਜੇ (frameexp > 10)
        frameexp = 10;
    ਪੂਰਨ maxpacket = epdesc->w_Max_Packet_ਮਾਪ;
    // Determine number of entries needed for 2 timer ticks.
    ਪੂਰਨ ms = 1<<frameexp;
    ਪੂਰਨ ਗਿਣਤੀ_2 = DIV_ਗੇੜਾ_ਉੱਪਰ(ticks_ਮੰਜ਼ਿਲ_ms(2), ms);
    ਗਿਣਤੀ_2 = ALIGN(ਗਿਣਤੀ_2, 2);
    ਬਣਤਰ uhci_pipe *pipe = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*pipe));
    ਬਣਤਰ uhci_td *tds = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*tds) * ਗਿਣਤੀ_2);
    ਖਾਲੀ *ਅੰਕੜੇ = malloc_ਨੀਵਾਂ(maxpacket * ਗਿਣਤੀ_2);
    ਜੇ (!pipe || !tds || !ਅੰਕੜੇ) {
        warn_noalloc();
        ਜਾਓ fail;
    }
    memset(pipe, 0, ਆਕਾਰ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    ਪੂਰਨ lowspeed = pipe->pipe.ਰਫਤਾਰ;
    ਪੂਰਨ devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->ਅਗਲਾ_td = &tds[0];
    pipe->iobase = cntl->iobase;

    ਪੂਰਨ toggle = 0;
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ਗਿਣਤੀ_2; i++) {
        tds[i].link = (i==ਗਿਣਤੀ_2-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].ਸਥਿਤੀ = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ਸਰਗਰਮ);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_ਖਿਸਕਾਓ)
                        | USB_PID_ਵਿੱਚ);
        tds[i].ਬਫਰ = ਅੰਕੜੇ + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    ਬਣਤਰ uhci_framelist *fl = cntl->framelist;
    ਜੇ (frameexp == 0) {
        // Add to existing interrupt entry.
        ਬਣਤਰ uhci_qh *intr_qh = (ਖਾਲੀ*)(fl->links[0] & ~UHCI_PTR_ਬਿੱਟ);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        ਜੇ (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } ਨਹੀਂਤਾਂ {
        ਪੂਰਨ startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        ਲਈ (i=startpos; i<ARRAY_ਮਾਪ(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    ਵਾਪਸ &pipe->pipe;
fail:
    ਮੁਕਤ(pipe);
    ਮੁਕਤ(tds);
    ਮੁਕਤ(ਅੰਕੜੇ);
    ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ usb_pipe *
uhci_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev, ਬਣਤਰ usb_pipe *upipe
                  , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਜੇ (! CONFIG_USB_UHCI)
        ਵਾਪਸ ਨਿਰਾ;
    usb_ਜੋੜੋ_freelist(upipe);
    ਜੇ (!epdesc)
        ਵਾਪਸ ਨਿਰਾ;
    u8 eptype = epdesc->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ;
    ਜੇ (eptype == USB_ENDPOINT_XFER_INT)
        ਵਾਪਸ uhci_alloc_intr_pipe(usbdev, epdesc);
    ਬਣਤਰ usb_uhci_s *cntl = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ਬਣਤਰ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ਜੇ (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        ਵਾਪਸ usbpipe;
    }

    // Allocate a new queue head.
    ਬਣਤਰ uhci_pipe *pipe;
    ਜੇ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(ਆਕਾਰ(*pipe));
    ਨਹੀਂਤਾਂ
        pipe = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*pipe));
    ਜੇ (!pipe) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(pipe, 0, ਆਕਾਰ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_ਪਦ;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    ਬਣਤਰ uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    ਜੇ (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    ਵਾਪਸ &pipe->pipe;
}

ਸਥਿਰ ਪੂਰਨ
wait_pipe(ਬਣਤਰ uhci_pipe *pipe, u32 ਸਿਰਾ)
{
    ਲਈ (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        ਜੇ (el_link & UHCI_PTR_ਪਦ)
            ਵਾਪਸ 0;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            ਬਣਤਰ uhci_td *td = (ਖਾਲੀ*)(el_link & ~UHCI_PTR_ਬਿੱਟ);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (ਖਾਲੀ*)el_link, GET_LOWFLAT(td->ਸਥਿਤੀ)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            ਸੈੱਟ_LOWFLAT(pipe->qh.element, UHCI_PTR_ਪਦ);
            uhci_waittick(iobase);
            ਵਾਪਸ -1;
        }
        yield();
    }
}

ਸਥਿਰ ਪੂਰਨ
wait_td(ਬਣਤਰ uhci_td *td, u32 ਸਿਰਾ)
{
    u32 ਸਥਿਤੀ;
    ਲਈ (;;) {
        ਸਥਿਤੀ = td->ਸਥਿਤੀ;
        ਜੇ (!(ਸਥਿਤੀ & TD_CTRL_ਸਰਗਰਮ))
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
        yield();
    }
    ਜੇ (ਸਥਿਤੀ & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", ਸਥਿਤੀ);
        ਵਾਪਸ -2;
    }
    ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STACKTDS 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TDALIGN 16

ਪੂਰਨ
uhci_send_pipe(ਬਣਤਰ usb_pipe *p, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
               , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize)
{
    ਜੇ (! CONFIG_USB_UHCI)
        ਵਾਪਸ -1;
    ਬਣਤਰ uhci_pipe *pipe = container_ਦਾ(p, ਬਣਤਰ uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, ਅੰਕੜੇ, datasize);
    ਪੂਰਨ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ਪੂਰਨ lowspeed = GET_LOWFLAT(pipe->pipe.ਰਫਤਾਰ);
    ਪੂਰਨ devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    ਪੂਰਨ toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[ਆਕਾਰ(ਬਣਤਰ uhci_td) * STACKTDS + TDALIGN - 1];
    ਬਣਤਰ uhci_td *tds = (ਖਾਲੀ*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, ਆਕਾਰ(*tds) * STACKTDS);
    ਪੂਰਨ tdpos = 0;

    // Enable tds
    u32 ਸਿਰਾ = timer_calc(usb_xfer_ਸਮਾਂ(p, datasize));
    barrier();
    ਸੈੱਟ_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    ਜੇ (cmd) {
        // Send setup pid on control transfers
        ਬਣਤਰ uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_ਡੂੰਘਾਈ;
        td->token = (uhci_explen(USB_CONTROL_ਤਿਆਰੀ_ਮਾਪ)
                     | (devaddr << TD_TOKEN_DEVADDR_ਖਿਸਕਾਓ) | USB_PID_ਤਿਆਰੀ);
        td->ਬਫਰ = (ਖਾਲੀ*)cmd;
        barrier();
        td->ਸਥਿਤੀ = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ਸਰਗਰਮ);
        toggle = TD_TOKEN_TOGGLE;
    }
    ਜਦੋਂ (datasize) {
        // Send data pids
        ਬਣਤਰ uhci_td *td = &tds[tdpos++ % STACKTDS];
        ਪੂਰਨ ret = wait_td(td, ਸਿਰਾ);
        ਜੇ (ret)
            ਜਾਓ fail;

        ਪੂਰਨ transfer = datasize;
        ਜੇ (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_ਪਦ : (nexttd | UHCI_PTR_ਡੂੰਘਾਈ));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_ਖਿਸਕਾਓ)
                     | (dir ? USB_PID_ਵਿੱਚ : USB_PID_ਬਾਹਰਲਾ));
        td->ਬਫਰ = ਅੰਕੜੇ;
        barrier();
        td->ਸਥਿਤੀ = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ਸਰਗਰਮ);
        toggle ^= TD_TOKEN_TOGGLE;

        ਅੰਕੜੇ += transfer;
        datasize -= transfer;
    }
    ਜੇ (cmd) {
        // Send status pid on control transfers
        ਬਣਤਰ uhci_td *td = &tds[tdpos++ % STACKTDS];
        ਪੂਰਨ ret = wait_td(td, ਸਿਰਾ);
        ਜੇ (ret)
            ਜਾਓ fail;
        td->link = UHCI_PTR_ਪਦ;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_ਖਿਸਕਾਓ)
                     | (dir ? USB_PID_ਬਾਹਰਲਾ : USB_PID_ਵਿੱਚ));
        td->ਬਫਰ = 0;
        barrier();
        td->ਸਥਿਤੀ = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ਸਰਗਰਮ);
    }
    ਸੈੱਟ_LOWFLAT(pipe->toggle, !!toggle);
    ਵਾਪਸ wait_pipe(pipe, ਸਿਰਾ);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    ਸੈੱਟ_LOWFLAT(pipe->qh.element, UHCI_PTR_ਪਦ);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    ਵਾਪਸ -1;
}

ਪੂਰਨ
uhci_poll_intr(ਬਣਤਰ usb_pipe *p, ਖਾਲੀ *ਅੰਕੜੇ)
{
    ASSERT16();
    ਜੇ (! CONFIG_USB_UHCI)
        ਵਾਪਸ -1;

    ਬਣਤਰ uhci_pipe *pipe = container_ਦਾ(p, ਬਣਤਰ uhci_pipe, pipe);
    ਬਣਤਰ uhci_td *td = GET_LOWFLAT(pipe->ਅਗਲਾ_td);
    u32 ਸਥਿਤੀ = GET_LOWFLAT(td->ਸਥਿਤੀ);
    u32 token = GET_LOWFLAT(td->token);
    ਜੇ (ਸਥਿਤੀ & TD_CTRL_ਸਰਗਰਮ)
        // No intrs found.
        ਵਾਪਸ -1;
    // XXX - check for errors.

    // Copy data.
    ਖਾਲੀ *tddata = GET_LOWFLAT(td->ਬਫਰ);
    memcpy_far(GET_SEG(SS), ਅੰਕੜੇ, SEG_ਨੀਵਾਂ, LOWFLAT2_ਨੀਵਾਂ(tddata)
               , uhci_expected_ਲੰਮਾਈ(token));

    // Reenable this td.
    ਬਣਤਰ uhci_td *ਅਗਲਾ = (ਖਾਲੀ*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_ਬਿੱਟ);
    ਸੈੱਟ_LOWFLAT(pipe->ਅਗਲਾ_td, ਅਗਲਾ);
    barrier();
    ਸੈੱਟ_LOWFLAT(td->ਸਥਿਤੀ, (uhci_maxerr(0) | (ਸਥਿਤੀ & TD_CTRL_LS)
                             | TD_CTRL_ਸਰਗਰਮ));

    ਵਾਪਸ 0;
}
