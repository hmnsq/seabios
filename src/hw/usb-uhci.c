// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_LOWFLAT
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pci.h" // pci_config_writew
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_BASE_ADDRESS_4
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "usb.h" // struct usb_s
#ଅନ୍ତର୍ଭାଵ "usb-ehci.h" // ehci_wait_controllers
#ଅନ୍ତର୍ଭାଵ "usb-uhci.h" // USBLEGSUP
#ଅନ୍ତର୍ଭାଵ "util.h" // msleep
#ଅନ୍ତର୍ଭାଵ "x86.h" // outw

ସଂରଚନା usb_uhci_s {
    ସଂରଚନା usb_s usb;
    u16 iobase;
    ସଂରଚନା uhci_qh *control_qh;
    ସଂରଚନା uhci_framelist *framelist;
};

ସଂରଚନା uhci_pipe {
    ସଂରଚନା uhci_qh qh;
    ସଂରଚନା uhci_td *next_td;
    ସଂରଚନା usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
uhci_hub_detect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_uhci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    ଯଦି (!(status & USBPORTSC_CCS))
        // No device found.
        ଫେରନ୍ତୁ 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    ଫେରନ୍ତୁ 1;
}

// Reset device on port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
uhci_hub_reset(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_uhci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    ଯଦି (!(status & USBPORTSC_CCS))
        // No longer connected
        ଫେରନ୍ତୁ -1;
    outw(USBPORTSC_PE, ioport);
    ଫେରନ୍ତୁ !!(status & USBPORTSC_LSDA);
}

// Disable port
ସ୍ଥିର ଶୂନ୍ୟ
uhci_hub_disconnect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_uhci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

ସ୍ଥିର ସଂରଚନା usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
check_uhci_ports(ସଂରଚନା usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    ସଂରଚନା usbhub_s hub;
    memset(&hub, 0, ଆକାର(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    ଫେରନ୍ତୁ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ସ୍ଥିର ଶୂନ୍ୟ
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    ପାଇଁ (;;) {
        ଯଦି (inw(iobase + USBFRNUM) != startframe)
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ;
        }
        yield();
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
uhci_free_pipes(ସଂରଚନା usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    ସଂରଚନା uhci_qh *pos = (ଶୂନ୍ୟ*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    ପାଇଁ (;;) {
        u32 link = pos->link;
        ଯଦି (link == UHCI_PTR_TERM)
            ଭାଙ୍ଗ;
        ସଂରଚନା uhci_qh *next = (ଶୂନ୍ୟ*)(link & ~UHCI_PTR_BITS);
        ସଂରଚନା uhci_pipe *pipe = container_of(next, ସଂରଚନା uhci_pipe, qh);
        ଯଦି (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        ଅନ୍ୟଥା
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    ପାଇଁ (;;) {
        ସଂରଚନା usb_pipe *usbpipe = cntl->usb.freelist;
        ଯଦି (!usbpipe)
            ଭାଙ୍ଗ;
        cntl->usb.freelist = usbpipe->freenext;
        ସଂରଚନା uhci_pipe *pipe = container_of(usbpipe, ସଂରଚନା uhci_pipe, pipe);
        ମୁକ୍ତ(pipe);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
reset_uhci(ସଂରଚନା usb_uhci_s *cntl, u16 bdf)
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

ସ୍ଥିର ଶୂନ୍ୟ
configure_uhci(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    ସଂରଚନା uhci_td *term_td = malloc_ଉପର(ଆକାର(*term_td));
    ସଂରଚନା uhci_framelist *fl = memalign_ଉପର(ଆକାର(*fl), ଆକାର(*fl));
    ସଂରଚନା uhci_pipe *intr_pipe = malloc_ଉପର(ଆକାର(*intr_pipe));
    ସଂରଚନା uhci_pipe *term_pipe = malloc_ଉପର(ଆକାର(*term_pipe));
    ଯଦି (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        ଯାଅ fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, ଆକାର(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, ଆକାର(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, ଆକାର(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    ଯଦି (ଗଣନା_2)
        // Success
        ଫେରନ୍ତୁ;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    ମୁକ୍ତ(term_td);
    ମୁକ୍ତ(fl);
    ମୁକ୍ତ(intr_pipe);
    ମୁକ୍ତ(term_pipe);
    ମୁକ୍ତ(cntl);
}

ସ୍ଥିର ଶୂନ୍ୟ
uhci_controller_setup(ସଂରଚନା pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    ଯଦି (!iobase)
        ଫେରନ୍ତୁ;

    ସଂରଚନା usb_uhci_s *cntl = malloc_tmphigh(ଆକାର(*cntl));
    ଯଦି (!cntl) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(cntl, 0, ଆକାର(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

ଶୂନ୍ୟ
uhci_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_USB_UHCI)
        ଫେରନ୍ତୁ;
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

ସ୍ଥିର ସଂରଚନା usb_pipe *
uhci_alloc_intr_pipe(ସଂରଚନା usbdevice_s *usbdev
                     , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ସଂରଚନା usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_uhci_s, usb);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ଯଦି (frameexp > 10)
        frameexp = 10;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ms = 1<<frameexp;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = DIV_ROUND_UP(ticks_to_ms(2), ms);
    ଗଣନା_2 = ALIGN(ଗଣନା_2, 2);
    ସଂରଚନା uhci_pipe *pipe = malloc_ତଳ(ଆକାର(*pipe));
    ସଂରଚନା uhci_td *tds = malloc_ତଳ(ଆକାର(*tds) * ଗଣନା_2);
    ଶୂନ୍ୟ *data = malloc_ତଳ(maxpacket * ଗଣନା_2);
    ଯଦି (!pipe || !tds || !data) {
        warn_noalloc();
        ଯାଅ fail;
    }
    memset(pipe, 0, ଆକାର(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା lowspeed = pipe->pipe.speed;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା toggle = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ଗଣନା_2; i++) {
        tds[i].link = (i==ଗଣନା_2-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    ସଂରଚନା uhci_framelist *fl = cntl->framelist;
    ଯଦି (frameexp == 0) {
        // Add to existing interrupt entry.
        ସଂରଚନା uhci_qh *intr_qh = (ଶୂନ୍ୟ*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        ଯଦି (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } ଅନ୍ୟଥା {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        ପାଇଁ (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    ଫେରନ୍ତୁ &pipe->pipe;
fail:
    ମୁକ୍ତ(pipe);
    ମୁକ୍ତ(tds);
    ମୁକ୍ତ(data);
    ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା usb_pipe *
uhci_realloc_pipe(ସଂରଚନା usbdevice_s *usbdev, ସଂରଚନା usb_pipe *upipe
                  , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ଯଦି (! CONFIG_USB_UHCI)
        ଫେରନ୍ତୁ ଖାଲି;
    usb_add_freelist(upipe);
    ଯଦି (!epdesc)
        ଫେରନ୍ତୁ ଖାଲି;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ଯଦି (eptype == USB_ENDPOINT_XFER_INT)
        ଫେରନ୍ତୁ uhci_alloc_intr_pipe(usbdev, epdesc);
    ସଂରଚନା usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ସଂରଚନା usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ଯଦି (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        ଫେରନ୍ତୁ usbpipe;
    }

    // Allocate a new queue head.
    ସଂରଚନା uhci_pipe *pipe;
    ଯଦି (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(ଆକାର(*pipe));
    ଅନ୍ୟଥା
        pipe = malloc_ତଳ(ଆକାର(*pipe));
    ଯଦି (!pipe) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(pipe, 0, ଆକାର(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    ସଂରଚନା uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    ଯଦି (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    ଫେରନ୍ତୁ &pipe->pipe;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
wait_pipe(ସଂରଚନା uhci_pipe *pipe, u32 end)
{
    ପାଇଁ (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        ଯଦି (el_link & UHCI_PTR_TERM)
            ଫେରନ୍ତୁ 0;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            ସଂରଚନା uhci_td *td = (ଶୂନ୍ୟ*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (ଶୂନ୍ୟ*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
wait_td(ସଂରଚନା uhci_td *td, u32 end)
{
    u32 status;
    ପାଇଁ (;;) {
        status = td->status;
        ଯଦି (!(status & TD_CTRL_ACTIVE))
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
    ଯଦି (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        ଫେରନ୍ତୁ -2;
    }
    ଫେରନ୍ତୁ 0;
}

#define STACKTDS 16
#define TDALIGN 16

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
uhci_send_pipe(ସଂରଚନା usb_pipe *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଅଚଳ ଶୂନ୍ୟ *cmd
               , ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datasize)
{
    ଯଦି (! CONFIG_USB_UHCI)
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା uhci_pipe *pipe = container_of(p, ସଂରଚନା uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[ଆକାର(ସଂରଚନା uhci_td) * STACKTDS + TDALIGN - 1];
    ସଂରଚନା uhci_td *tds = (ଶୂନ୍ୟ*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, ଆକାର(*tds) * STACKTDS);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    ଯଦି (cmd) {
        // Send setup pid on control transfers
        ସଂରଚନା uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (ଶୂନ୍ୟ*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    ଯେତେବେଳେ (datasize) {
        // Send data pids
        ସଂରଚନା uhci_td *td = &tds[tdpos++ % STACKTDS];
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = wait_td(td, end);
        ଯଦି (ret)
            ଯାଅ fail;

        ପୂର୍ଣ୍ଣସଂଖ୍ୟା transfer = datasize;
        ଯଦି (transfer > maxpacket)
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
    ଯଦି (cmd) {
        // Send status pid on control transfers
        ସଂରଚନା uhci_td *td = &tds[tdpos++ % STACKTDS];
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = wait_td(td, end);
        ଯଦି (ret)
            ଯାଅ fail;
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
    ଫେରନ୍ତୁ wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    ଫେରନ୍ତୁ -1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
uhci_poll_intr(ସଂରଚନା usb_pipe *p, ଶୂନ୍ୟ *data)
{
    ASSERT16();
    ଯଦି (! CONFIG_USB_UHCI)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା uhci_pipe *pipe = container_of(p, ସଂରଚନା uhci_pipe, pipe);
    ସଂରଚନା uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    ଯଦି (status & TD_CTRL_ACTIVE)
        // No intrs found.
        ଫେରନ୍ତୁ -1;
    // XXX - check for errors.

    // Copy data.
    ଶୂନ୍ୟ *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_ତଳ, LOWFLAT2_ତଳ(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    ସଂରଚନା uhci_td *next = (ଶୂନ୍ୟ*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    ଫେରନ୍ତୁ 0;
}
