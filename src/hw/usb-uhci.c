// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_LOWFLAT
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pci.h" // pci_config_writew
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#അന്തര്ഭാവ "pci_regs.h" // PCI_BASE_ADDRESS_4
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "usb.h" // struct usb_s
#അന്തര്ഭാവ "usb-ehci.h" // ehci_wait_controllers
#അന്തര്ഭാവ "usb-uhci.h" // USBLEGSUP
#അന്തര്ഭാവ "util.h" // msleep
#അന്തര്ഭാവ "x86.h" // outw

ഘടന usb_uhci_s {
    ഘടന usb_s usb;
    u16 iobase;
    ഘടന uhci_qh *control_qh;
    ഘടന uhci_framelist *framelist;
};

ഘടന uhci_pipe {
    ഘടന uhci_qh qh;
    ഘടന uhci_td *next_td;
    ഘടന usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
സ്ഥിതം പൂർണ്ണസംഖ്യ
uhci_hub_detect(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_uhci_s *cntl = container_of(hub->cntl, ഘടന usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    എങ്കിൽ (!(status & USBPORTSC_CCS))
        // No device found.
        തിരികെ 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    തിരികെ 1;
}

// Reset device on port
സ്ഥിതം പൂർണ്ണസംഖ്യ
uhci_hub_reset(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_uhci_s *cntl = container_of(hub->cntl, ഘടന usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    എങ്കിൽ (!(status & USBPORTSC_CCS))
        // No longer connected
        തിരികെ -1;
    outw(USBPORTSC_PE, ioport);
    തിരികെ !!(status & USBPORTSC_LSDA);
}

// Disable port
സ്ഥിതം ശൂന്യം
uhci_hub_disconnect(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_uhci_s *cntl = container_of(hub->cntl, ഘടന usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

സ്ഥിതം ഘടന usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
സ്ഥിതം പൂർണ്ണസംഖ്യ
check_uhci_ports(ഘടന usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    ഘടന usbhub_s hub;
    memset(&hub, 0, വലുപ്പം(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    തിരികെ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
സ്ഥിതം ശൂന്യം
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    വേണ്ടി (;;) {
        എങ്കിൽ (inw(iobase + USBFRNUM) != startframe)
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ;
        }
        yield();
    }
}

സ്ഥിതം ശൂന്യം
uhci_free_pipes(ഘടന usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    ഘടന uhci_qh *pos = (ശൂന്യം*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    വേണ്ടി (;;) {
        u32 link = pos->link;
        എങ്കിൽ (link == UHCI_PTR_TERM)
            നിർത്തുക;
        ഘടന uhci_qh *next = (ശൂന്യം*)(link & ~UHCI_PTR_BITS);
        ഘടന uhci_pipe *pipe = container_of(next, ഘടന uhci_pipe, qh);
        എങ്കിൽ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        അല്ലെങ്കിൽ
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    വേണ്ടി (;;) {
        ഘടന usb_pipe *usbpipe = cntl->usb.freelist;
        എങ്കിൽ (!usbpipe)
            നിർത്തുക;
        cntl->usb.freelist = usbpipe->freenext;
        ഘടന uhci_pipe *pipe = container_of(usbpipe, ഘടന uhci_pipe, pipe);
        സ്വതന്ത്രം(pipe);
    }
}

സ്ഥിതം ശൂന്യം
reset_uhci(ഘടന usb_uhci_s *cntl, u16 bdf)
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

സ്ഥിതം ശൂന്യം
configure_uhci(ശൂന്യം *data)
{
    ഘടന usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    ഘടന uhci_td *term_td = malloc_മുകളിലെ(വലുപ്പം(*term_td));
    ഘടന uhci_framelist *fl = memalign_മുകളിലെ(വലുപ്പം(*fl), വലുപ്പം(*fl));
    ഘടന uhci_pipe *intr_pipe = malloc_മുകളിലെ(വലുപ്പം(*intr_pipe));
    ഘടന uhci_pipe *term_pipe = malloc_മുകളിലെ(വലുപ്പം(*term_pipe));
    എങ്കിൽ (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        പോകുക fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, വലുപ്പം(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, വലുപ്പം(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, വലുപ്പം(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    പൂർണ്ണസംഖ്യ എണ്ണം = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    എങ്കിൽ (എണ്ണം)
        // Success
        തിരികെ;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    സ്വതന്ത്രം(term_td);
    സ്വതന്ത്രം(fl);
    സ്വതന്ത്രം(intr_pipe);
    സ്വതന്ത്രം(term_pipe);
    സ്വതന്ത്രം(cntl);
}

സ്ഥിതം ശൂന്യം
uhci_controller_setup(ഘടന pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    എങ്കിൽ (!iobase)
        തിരികെ;

    ഘടന usb_uhci_s *cntl = malloc_tmphigh(വലുപ്പം(*cntl));
    എങ്കിൽ (!cntl) {
        warn_noalloc();
        തിരികെ;
    }
    memset(cntl, 0, വലുപ്പം(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

ശൂന്യം
uhci_setup(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_USB_UHCI)
        തിരികെ;
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

സ്ഥിതം ഘടന usb_pipe *
uhci_alloc_intr_pipe(ഘടന usbdevice_s *usbdev
                     , ഘടന usb_endpoint_descriptor *epdesc)
{
    ഘടന usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, ഘടന usb_uhci_s, usb);
    പൂർണ്ണസംഖ്യ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    എങ്കിൽ (frameexp > 10)
        frameexp = 10;
    പൂർണ്ണസംഖ്യ maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    പൂർണ്ണസംഖ്യ ms = 1<<frameexp;
    പൂർണ്ണസംഖ്യ എണ്ണം = DIV_ROUND_UP(ticks_to_ms(2), ms);
    എണ്ണം = ALIGN(എണ്ണം, 2);
    ഘടന uhci_pipe *pipe = malloc_താഴെയുള്ള(വലുപ്പം(*pipe));
    ഘടന uhci_td *tds = malloc_താഴെയുള്ള(വലുപ്പം(*tds) * എണ്ണം);
    ശൂന്യം *data = malloc_താഴെയുള്ള(maxpacket * എണ്ണം);
    എങ്കിൽ (!pipe || !tds || !data) {
        warn_noalloc();
        പോകുക fail;
    }
    memset(pipe, 0, വലുപ്പം(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    പൂർണ്ണസംഖ്യ lowspeed = pipe->pipe.speed;
    പൂർണ്ണസംഖ്യ devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    പൂർണ്ണസംഖ്യ toggle = 0;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<എണ്ണം; i++) {
        tds[i].link = (i==എണ്ണം-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    ഘടന uhci_framelist *fl = cntl->framelist;
    എങ്കിൽ (frameexp == 0) {
        // Add to existing interrupt entry.
        ഘടന uhci_qh *intr_qh = (ശൂന്യം*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        എങ്കിൽ (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } അല്ലെങ്കിൽ {
        പൂർണ്ണസംഖ്യ startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        വേണ്ടി (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    തിരികെ &pipe->pipe;
fail:
    സ്വതന്ത്രം(pipe);
    സ്വതന്ത്രം(tds);
    സ്വതന്ത്രം(data);
    തിരികെ ശൂന്യ;
}

ഘടന usb_pipe *
uhci_realloc_pipe(ഘടന usbdevice_s *usbdev, ഘടന usb_pipe *upipe
                  , ഘടന usb_endpoint_descriptor *epdesc)
{
    എങ്കിൽ (! CONFIG_USB_UHCI)
        തിരികെ ശൂന്യ;
    usb_add_freelist(upipe);
    എങ്കിൽ (!epdesc)
        തിരികെ ശൂന്യ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_INT)
        തിരികെ uhci_alloc_intr_pipe(usbdev, epdesc);
    ഘടന usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, ഘടന usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ഘടന usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    എങ്കിൽ (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        തിരികെ usbpipe;
    }

    // Allocate a new queue head.
    ഘടന uhci_pipe *pipe;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(വലുപ്പം(*pipe));
    അല്ലെങ്കിൽ
        pipe = malloc_താഴെയുള്ള(വലുപ്പം(*pipe));
    എങ്കിൽ (!pipe) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    memset(pipe, 0, വലുപ്പം(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    ഘടന uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    തിരികെ &pipe->pipe;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
wait_pipe(ഘടന uhci_pipe *pipe, u32 end)
{
    വേണ്ടി (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        എങ്കിൽ (el_link & UHCI_PTR_TERM)
            തിരികെ 0;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            ഘടന uhci_td *td = (ശൂന്യം*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (ശൂന്യം*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            തിരികെ -1;
        }
        yield();
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
wait_td(ഘടന uhci_td *td, u32 end)
{
    u32 status;
    വേണ്ടി (;;) {
        status = td->status;
        എങ്കിൽ (!(status & TD_CTRL_ACTIVE))
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ -1;
        }
        yield();
    }
    എങ്കിൽ (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        തിരികെ -2;
    }
    തിരികെ 0;
}

#define STACKTDS 16
#define TDALIGN 16

പൂർണ്ണസംഖ്യ
uhci_send_pipe(ഘടന usb_pipe *p, പൂർണ്ണസംഖ്യ dir, സ്ഥിരം ശൂന്യം *cmd
               , ശൂന്യം *data, പൂർണ്ണസംഖ്യ datasize)
{
    എങ്കിൽ (! CONFIG_USB_UHCI)
        തിരികെ -1;
    ഘടന uhci_pipe *pipe = container_of(p, ഘടന uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    പൂർണ്ണസംഖ്യ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    പൂർണ്ണസംഖ്യ lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    പൂർണ്ണസംഖ്യ devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    പൂർണ്ണസംഖ്യ toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[വലുപ്പം(ഘടന uhci_td) * STACKTDS + TDALIGN - 1];
    ഘടന uhci_td *tds = (ശൂന്യം*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, വലുപ്പം(*tds) * STACKTDS);
    പൂർണ്ണസംഖ്യ tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    എങ്കിൽ (cmd) {
        // Send setup pid on control transfers
        ഘടന uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (ശൂന്യം*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    ആകുമ്പോൾ (datasize) {
        // Send data pids
        ഘടന uhci_td *td = &tds[tdpos++ % STACKTDS];
        പൂർണ്ണസംഖ്യ ret = wait_td(td, end);
        എങ്കിൽ (ret)
            പോകുക fail;

        പൂർണ്ണസംഖ്യ transfer = datasize;
        എങ്കിൽ (transfer > maxpacket)
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
    എങ്കിൽ (cmd) {
        // Send status pid on control transfers
        ഘടന uhci_td *td = &tds[tdpos++ % STACKTDS];
        പൂർണ്ണസംഖ്യ ret = wait_td(td, end);
        എങ്കിൽ (ret)
            പോകുക fail;
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
    തിരികെ wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    തിരികെ -1;
}

പൂർണ്ണസംഖ്യ
uhci_poll_intr(ഘടന usb_pipe *p, ശൂന്യം *data)
{
    ASSERT16();
    എങ്കിൽ (! CONFIG_USB_UHCI)
        തിരികെ -1;

    ഘടന uhci_pipe *pipe = container_of(p, ഘടന uhci_pipe, pipe);
    ഘടന uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    എങ്കിൽ (status & TD_CTRL_ACTIVE)
        // No intrs found.
        തിരികെ -1;
    // XXX - check for errors.

    // Copy data.
    ശൂന്യം *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_താഴെയുള്ള, LOWFLAT2_താഴെയുള്ള(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    ഘടന uhci_td *next = (ശൂന്യം*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    തിരികെ 0;
}
