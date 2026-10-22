// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_LOWFLAT
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pci.h" // pci_config_writew
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#অন্তর্ভাব "pci_regs.h" // PCI_BASE_ADDRESS_4
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "usb.h" // struct usb_s
#অন্তর্ভাব "usb-ehci.h" // ehci_wait_controllers
#অন্তর্ভাব "usb-uhci.h" // USBLEGSUP
#অন্তর্ভাব "util.h" // msleep
#অন্তর্ভাব "x86.h" // outw

সংরচনা usb_uhci_s {
    সংরচনা usb_s usb;
    u16 iobase;
    সংরচনা uhci_qh *control_qh;
    সংরচনা uhci_framelist *framelist;
};

সংরচনা uhci_pipe {
    সংরচনা uhci_qh qh;
    সংরচনা uhci_td *next_td;
    সংরচনা usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
স্থির পূর্ণাঙ্ক
uhci_hub_detect(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_uhci_s *cntl = container_of(hub->cntl, সংরচনা usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    যদি (!(status & USBPORTSC_CCS))
        // No device found.
        প্রত্যাগম 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    প্রত্যাগম 1;
}

// Reset device on port
স্থির পূর্ণাঙ্ক
uhci_hub_reset(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_uhci_s *cntl = container_of(hub->cntl, সংরচনা usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    যদি (!(status & USBPORTSC_CCS))
        // No longer connected
        প্রত্যাগম -1;
    outw(USBPORTSC_PE, ioport);
    প্রত্যাগম !!(status & USBPORTSC_LSDA);
}

// Disable port
স্থির শূন্য
uhci_hub_disconnect(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_uhci_s *cntl = container_of(hub->cntl, সংরচনা usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

স্থির সংরচনা usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
স্থির পূর্ণাঙ্ক
check_uhci_ports(সংরচনা usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    সংরচনা usbhub_s hub;
    memset(&hub, 0, পরিমাণ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    প্রত্যাগম hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
স্থির শূন্য
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    প্রতি (;;) {
        যদি (inw(iobase + USBFRNUM) != startframe)
            ভঙ্গ;
        যদি (timer_check(end)) {
            warn_timeout();
            প্রত্যাগম;
        }
        yield();
    }
}

স্থির শূন্য
uhci_free_pipes(সংরচনা usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    সংরচনা uhci_qh *pos = (শূন্য*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    প্রতি (;;) {
        u32 link = pos->link;
        যদি (link == UHCI_PTR_TERM)
            ভঙ্গ;
        সংরচনা uhci_qh *next = (শূন্য*)(link & ~UHCI_PTR_BITS);
        সংরচনা uhci_pipe *pipe = container_of(next, সংরচনা uhci_pipe, qh);
        যদি (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        অন্যথা
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    প্রতি (;;) {
        সংরচনা usb_pipe *usbpipe = cntl->usb.freelist;
        যদি (!usbpipe)
            ভঙ্গ;
        cntl->usb.freelist = usbpipe->freenext;
        সংরচনা uhci_pipe *pipe = container_of(usbpipe, সংরচনা uhci_pipe, pipe);
        মুক্ত(pipe);
    }
}

স্থির শূন্য
reset_uhci(সংরচনা usb_uhci_s *cntl, u16 bdf)
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

স্থির শূন্য
configure_uhci(শূন্য *data)
{
    সংরচনা usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    সংরচনা uhci_td *term_td = malloc_উপরের(পরিমাণ(*term_td));
    সংরচনা uhci_framelist *fl = memalign_উপরের(পরিমাণ(*fl), পরিমাণ(*fl));
    সংরচনা uhci_pipe *intr_pipe = malloc_উপরের(পরিমাণ(*intr_pipe));
    সংরচনা uhci_pipe *term_pipe = malloc_উপরের(পরিমাণ(*term_pipe));
    যদি (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        গচ্ছ fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, পরিমাণ(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, পরিমাণ(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, পরিমাণ(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    পূর্ণাঙ্ক i;
    প্রতি (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    পূর্ণাঙ্ক গুনতি = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    যদি (গুনতি)
        // Success
        প্রত্যাগম;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    মুক্ত(term_td);
    মুক্ত(fl);
    মুক্ত(intr_pipe);
    মুক্ত(term_pipe);
    মুক্ত(cntl);
}

স্থির শূন্য
uhci_controller_setup(সংরচনা pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    যদি (!iobase)
        প্রত্যাগম;

    সংরচনা usb_uhci_s *cntl = malloc_tmphigh(পরিমাণ(*cntl));
    যদি (!cntl) {
        warn_noalloc();
        প্রত্যাগম;
    }
    memset(cntl, 0, পরিমাণ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

শূন্য
uhci_setup(শূন্য)
{
    যদি (! CONFIG_USB_UHCI)
        প্রত্যাগম;
    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

স্থির সংরচনা usb_pipe *
uhci_alloc_intr_pipe(সংরচনা usbdevice_s *usbdev
                     , সংরচনা usb_endpoint_descriptor *epdesc)
{
    সংরচনা usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, সংরচনা usb_uhci_s, usb);
    পূর্ণাঙ্ক frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    যদি (frameexp > 10)
        frameexp = 10;
    পূর্ণাঙ্ক maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    পূর্ণাঙ্ক ms = 1<<frameexp;
    পূর্ণাঙ্ক গুনতি = DIV_ROUND_UP(ticks_to_ms(2), ms);
    গুনতি = ALIGN(গুনতি, 2);
    সংরচনা uhci_pipe *pipe = malloc_নিচের(পরিমাণ(*pipe));
    সংরচনা uhci_td *tds = malloc_নিচের(পরিমাণ(*tds) * গুনতি);
    শূন্য *data = malloc_নিচের(maxpacket * গুনতি);
    যদি (!pipe || !tds || !data) {
        warn_noalloc();
        গচ্ছ fail;
    }
    memset(pipe, 0, পরিমাণ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    পূর্ণাঙ্ক lowspeed = pipe->pipe.speed;
    পূর্ণাঙ্ক devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    পূর্ণাঙ্ক toggle = 0;
    পূর্ণাঙ্ক i;
    প্রতি (i=0; i<গুনতি; i++) {
        tds[i].link = (i==গুনতি-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    সংরচনা uhci_framelist *fl = cntl->framelist;
    যদি (frameexp == 0) {
        // Add to existing interrupt entry.
        সংরচনা uhci_qh *intr_qh = (শূন্য*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        যদি (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } অন্যথা {
        পূর্ণাঙ্ক startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        প্রতি (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    প্রত্যাগম &pipe->pipe;
fail:
    মুক্ত(pipe);
    মুক্ত(tds);
    মুক্ত(data);
    প্রত্যাগম রিক্ত;
}

সংরচনা usb_pipe *
uhci_realloc_pipe(সংরচনা usbdevice_s *usbdev, সংরচনা usb_pipe *upipe
                  , সংরচনা usb_endpoint_descriptor *epdesc)
{
    যদি (! CONFIG_USB_UHCI)
        প্রত্যাগম রিক্ত;
    usb_add_freelist(upipe);
    যদি (!epdesc)
        প্রত্যাগম রিক্ত;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    যদি (eptype == USB_ENDPOINT_XFER_INT)
        প্রত্যাগম uhci_alloc_intr_pipe(usbdev, epdesc);
    সংরচনা usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, সংরচনা usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    সংরচনা usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    যদি (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        প্রত্যাগম usbpipe;
    }

    // Allocate a new queue head.
    সংরচনা uhci_pipe *pipe;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(পরিমাণ(*pipe));
    অন্যথা
        pipe = malloc_নিচের(পরিমাণ(*pipe));
    যদি (!pipe) {
        warn_noalloc();
        প্রত্যাগম রিক্ত;
    }
    memset(pipe, 0, পরিমাণ(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    সংরচনা uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    প্রত্যাগম &pipe->pipe;
}

স্থির পূর্ণাঙ্ক
wait_pipe(সংরচনা uhci_pipe *pipe, u32 end)
{
    প্রতি (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        যদি (el_link & UHCI_PTR_TERM)
            প্রত্যাগম 0;
        যদি (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            সংরচনা uhci_td *td = (শূন্য*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (শূন্য*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            প্রত্যাগম -1;
        }
        yield();
    }
}

স্থির পূর্ণাঙ্ক
wait_td(সংরচনা uhci_td *td, u32 end)
{
    u32 status;
    প্রতি (;;) {
        status = td->status;
        যদি (!(status & TD_CTRL_ACTIVE))
            ভঙ্গ;
        যদি (timer_check(end)) {
            warn_timeout();
            প্রত্যাগম -1;
        }
        yield();
    }
    যদি (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        প্রত্যাগম -2;
    }
    প্রত্যাগম 0;
}

#define STACKTDS 16
#define TDALIGN 16

পূর্ণাঙ্ক
uhci_send_pipe(সংরচনা usb_pipe *p, পূর্ণাঙ্ক dir, ধ্রুব শূন্য *cmd
               , শূন্য *data, পূর্ণাঙ্ক datasize)
{
    যদি (! CONFIG_USB_UHCI)
        প্রত্যাগম -1;
    সংরচনা uhci_pipe *pipe = container_of(p, সংরচনা uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    পূর্ণাঙ্ক maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    পূর্ণাঙ্ক lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    পূর্ণাঙ্ক devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    পূর্ণাঙ্ক toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[পরিমাণ(সংরচনা uhci_td) * STACKTDS + TDALIGN - 1];
    সংরচনা uhci_td *tds = (শূন্য*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, পরিমাণ(*tds) * STACKTDS);
    পূর্ণাঙ্ক tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    যদি (cmd) {
        // Send setup pid on control transfers
        সংরচনা uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (শূন্য*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    যাবত্ (datasize) {
        // Send data pids
        সংরচনা uhci_td *td = &tds[tdpos++ % STACKTDS];
        পূর্ণাঙ্ক ret = wait_td(td, end);
        যদি (ret)
            গচ্ছ fail;

        পূর্ণাঙ্ক transfer = datasize;
        যদি (transfer > maxpacket)
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
    যদি (cmd) {
        // Send status pid on control transfers
        সংরচনা uhci_td *td = &tds[tdpos++ % STACKTDS];
        পূর্ণাঙ্ক ret = wait_td(td, end);
        যদি (ret)
            গচ্ছ fail;
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
    প্রত্যাগম wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    প্রত্যাগম -1;
}

পূর্ণাঙ্ক
uhci_poll_intr(সংরচনা usb_pipe *p, শূন্য *data)
{
    ASSERT16();
    যদি (! CONFIG_USB_UHCI)
        প্রত্যাগম -1;

    সংরচনা uhci_pipe *pipe = container_of(p, সংরচনা uhci_pipe, pipe);
    সংরচনা uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    যদি (status & TD_CTRL_ACTIVE)
        // No intrs found.
        প্রত্যাগম -1;
    // XXX - check for errors.

    // Copy data.
    শূন্য *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_নিচের, LOWFLAT2_নিচের(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    সংরচনা uhci_td *next = (শূন্য*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    প্রত্যাগম 0;
}
