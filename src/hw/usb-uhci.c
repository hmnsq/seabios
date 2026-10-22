// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_LOWFLAT
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pci.h" // pci_config_writew
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#અન્તર્ભાવ "pci_regs.h" // PCI_BASE_ADDRESS_4
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "usb.h" // struct usb_s
#અન્તર્ભાવ "usb-ehci.h" // ehci_wait_controllers
#અન્તર્ભાવ "usb-uhci.h" // USBLEGSUP
#અન્તર્ભાવ "util.h" // msleep
#અન્તર્ભાવ "x86.h" // outw

સંરચના usb_uhci_s {
    સંરચના usb_s usb;
    u16 iobase;
    સંરચના uhci_qh *control_qh;
    સંરચના uhci_framelist *framelist;
};

સંરચના uhci_pipe {
    સંરચના uhci_qh qh;
    સંરચના uhci_td *next_td;
    સંરચના usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
સ્થિર પૂર્ણાંક
uhci_hub_detect(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_uhci_s *cntl = container_of(hub->cntl, સંરચના usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    જો (!(status & USBPORTSC_CCS))
        // No device found.
        પાછા 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    પાછા 1;
}

// Reset device on port
સ્થિર પૂર્ણાંક
uhci_hub_reset(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_uhci_s *cntl = container_of(hub->cntl, સંરચના usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    જો (!(status & USBPORTSC_CCS))
        // No longer connected
        પાછા -1;
    outw(USBPORTSC_PE, ioport);
    પાછા !!(status & USBPORTSC_LSDA);
}

// Disable port
સ્થિર શૂન્ય
uhci_hub_disconnect(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_uhci_s *cntl = container_of(hub->cntl, સંરચના usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

સ્થિર સંરચના usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
સ્થિર પૂર્ણાંક
check_uhci_ports(સંરચના usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    સંરચના usbhub_s hub;
    memset(&hub, 0, કદ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    પાછા hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
સ્થિર શૂન્ય
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    માટે (;;) {
        જો (inw(iobase + USBFRNUM) != startframe)
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            પાછા;
        }
        yield();
    }
}

સ્થિર શૂન્ય
uhci_free_pipes(સંરચના usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    સંરચના uhci_qh *pos = (શૂન્ય*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    માટે (;;) {
        u32 link = pos->link;
        જો (link == UHCI_PTR_TERM)
            તોડો;
        સંરચના uhci_qh *next = (શૂન્ય*)(link & ~UHCI_PTR_BITS);
        સંરચના uhci_pipe *pipe = container_of(next, સંરચના uhci_pipe, qh);
        જો (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        નહીંતર
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    માટે (;;) {
        સંરચના usb_pipe *usbpipe = cntl->usb.freelist;
        જો (!usbpipe)
            તોડો;
        cntl->usb.freelist = usbpipe->freenext;
        સંરચના uhci_pipe *pipe = container_of(usbpipe, સંરચના uhci_pipe, pipe);
        મુક્ત(pipe);
    }
}

સ્થિર શૂન્ય
reset_uhci(સંરચના usb_uhci_s *cntl, u16 bdf)
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

સ્થિર શૂન્ય
configure_uhci(શૂન્ય *data)
{
    સંરચના usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    સંરચના uhci_td *term_td = malloc_ઉપલું(કદ(*term_td));
    સંરચના uhci_framelist *fl = memalign_ઉપલું(કદ(*fl), કદ(*fl));
    સંરચના uhci_pipe *intr_pipe = malloc_ઉપલું(કદ(*intr_pipe));
    સંરચના uhci_pipe *term_pipe = malloc_ઉપલું(કદ(*term_pipe));
    જો (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        જાઓ fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, કદ(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, કદ(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, કદ(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    પૂર્ણાંક i;
    માટે (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    પૂર્ણાંક ગણતરી = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    જો (ગણતરી)
        // Success
        પાછા;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    મુક્ત(term_td);
    મુક્ત(fl);
    મુક્ત(intr_pipe);
    મુક્ત(term_pipe);
    મુક્ત(cntl);
}

સ્થિર શૂન્ય
uhci_controller_setup(સંરચના pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    જો (!iobase)
        પાછા;

    સંરચના usb_uhci_s *cntl = malloc_tmphigh(કદ(*cntl));
    જો (!cntl) {
        warn_noalloc();
        પાછા;
    }
    memset(cntl, 0, કદ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

શૂન્ય
uhci_setup(શૂન્ય)
{
    જો (! CONFIG_USB_UHCI)
        પાછા;
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

સ્થિર સંરચના usb_pipe *
uhci_alloc_intr_pipe(સંરચના usbdevice_s *usbdev
                     , સંરચના usb_endpoint_descriptor *epdesc)
{
    સંરચના usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, સંરચના usb_uhci_s, usb);
    પૂર્ણાંક frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    જો (frameexp > 10)
        frameexp = 10;
    પૂર્ણાંક maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    પૂર્ણાંક ms = 1<<frameexp;
    પૂર્ણાંક ગણતરી = DIV_ROUND_UP(ticks_to_ms(2), ms);
    ગણતરી = ALIGN(ગણતરી, 2);
    સંરચના uhci_pipe *pipe = malloc_નીચલું(કદ(*pipe));
    સંરચના uhci_td *tds = malloc_નીચલું(કદ(*tds) * ગણતરી);
    શૂન્ય *data = malloc_નીચલું(maxpacket * ગણતરી);
    જો (!pipe || !tds || !data) {
        warn_noalloc();
        જાઓ fail;
    }
    memset(pipe, 0, કદ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    પૂર્ણાંક lowspeed = pipe->pipe.speed;
    પૂર્ણાંક devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    પૂર્ણાંક toggle = 0;
    પૂર્ણાંક i;
    માટે (i=0; i<ગણતરી; i++) {
        tds[i].link = (i==ગણતરી-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    સંરચના uhci_framelist *fl = cntl->framelist;
    જો (frameexp == 0) {
        // Add to existing interrupt entry.
        સંરચના uhci_qh *intr_qh = (શૂન્ય*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        જો (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } નહીંતર {
        પૂર્ણાંક startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        માટે (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    પાછા &pipe->pipe;
fail:
    મુક્ત(pipe);
    મુક્ત(tds);
    મુક્ત(data);
    પાછા ખાલી;
}

સંરચના usb_pipe *
uhci_realloc_pipe(સંરચના usbdevice_s *usbdev, સંરચના usb_pipe *upipe
                  , સંરચના usb_endpoint_descriptor *epdesc)
{
    જો (! CONFIG_USB_UHCI)
        પાછા ખાલી;
    usb_add_freelist(upipe);
    જો (!epdesc)
        પાછા ખાલી;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    જો (eptype == USB_ENDPOINT_XFER_INT)
        પાછા uhci_alloc_intr_pipe(usbdev, epdesc);
    સંરચના usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, સંરચના usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    સંરચના usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    જો (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        પાછા usbpipe;
    }

    // Allocate a new queue head.
    સંરચના uhci_pipe *pipe;
    જો (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(કદ(*pipe));
    નહીંતર
        pipe = malloc_નીચલું(કદ(*pipe));
    જો (!pipe) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(pipe, 0, કદ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    સંરચના uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    જો (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    પાછા &pipe->pipe;
}

સ્થિર પૂર્ણાંક
wait_pipe(સંરચના uhci_pipe *pipe, u32 end)
{
    માટે (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        જો (el_link & UHCI_PTR_TERM)
            પાછા 0;
        જો (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            સંરચના uhci_td *td = (શૂન્ય*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (શૂન્ય*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            પાછા -1;
        }
        yield();
    }
}

સ્થિર પૂર્ણાંક
wait_td(સંરચના uhci_td *td, u32 end)
{
    u32 status;
    માટે (;;) {
        status = td->status;
        જો (!(status & TD_CTRL_ACTIVE))
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            પાછા -1;
        }
        yield();
    }
    જો (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        પાછા -2;
    }
    પાછા 0;
}

#define STACKTDS 16
#define TDALIGN 16

પૂર્ણાંક
uhci_send_pipe(સંરચના usb_pipe *p, પૂર્ણાંક dir, અચલ શૂન્ય *cmd
               , શૂન્ય *data, પૂર્ણાંક datasize)
{
    જો (! CONFIG_USB_UHCI)
        પાછા -1;
    સંરચના uhci_pipe *pipe = container_of(p, સંરચના uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    પૂર્ણાંક maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    પૂર્ણાંક lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    પૂર્ણાંક devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    પૂર્ણાંક toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[કદ(સંરચના uhci_td) * STACKTDS + TDALIGN - 1];
    સંરચના uhci_td *tds = (શૂન્ય*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, કદ(*tds) * STACKTDS);
    પૂર્ણાંક tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    જો (cmd) {
        // Send setup pid on control transfers
        સંરચના uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (શૂન્ય*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    જ્યારે (datasize) {
        // Send data pids
        સંરચના uhci_td *td = &tds[tdpos++ % STACKTDS];
        પૂર્ણાંક ret = wait_td(td, end);
        જો (ret)
            જાઓ fail;

        પૂર્ણાંક transfer = datasize;
        જો (transfer > maxpacket)
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
    જો (cmd) {
        // Send status pid on control transfers
        સંરચના uhci_td *td = &tds[tdpos++ % STACKTDS];
        પૂર્ણાંક ret = wait_td(td, end);
        જો (ret)
            જાઓ fail;
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
    પાછા wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    પાછા -1;
}

પૂર્ણાંક
uhci_poll_intr(સંરચના usb_pipe *p, શૂન્ય *data)
{
    ASSERT16();
    જો (! CONFIG_USB_UHCI)
        પાછા -1;

    સંરચના uhci_pipe *pipe = container_of(p, સંરચના uhci_pipe, pipe);
    સંરચના uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    જો (status & TD_CTRL_ACTIVE)
        // No intrs found.
        પાછા -1;
    // XXX - check for errors.

    // Copy data.
    શૂન્ય *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_નીચલું, LOWFLAT2_નીચલું(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    સંરચના uhci_td *next = (શૂન્ય*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    પાછા 0;
}
