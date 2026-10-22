// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_LOWFLAT
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pci.h" // pci_config_writew
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#شامل "pci_regs.h" // PCI_BASE_ADDRESS_4
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-ehci.h" // ehci_wait_controllers
#شامل "usb-uhci.h" // USBLEGSUP
#شامل "util.h" // msleep
#شامل "x86.h" // outw

ساختار usb_uhci_s {
    ساختار usb_s usb;
    u16 iobase;
    ساختار uhci_qh *control_qh;
    ساختار uhci_framelist *framelist;
};

ساختار uhci_pipe {
    ساختار uhci_qh qh;
    ساختار uhci_td *next_td;
    ساختار usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
ایستا صحیح
uhci_hub_detect(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_uhci_s *cntl = container_of(hub->cntl, ساختار usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    اگر (!(status & USBPORTSC_CCS))
        // No device found.
        بازگشت 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    بازگشت 1;
}

// Reset device on port
ایستا صحیح
uhci_hub_reset(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_uhci_s *cntl = container_of(hub->cntl, ساختار usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    اگر (!(status & USBPORTSC_CCS))
        // No longer connected
        بازگشت -1;
    outw(USBPORTSC_PE, ioport);
    بازگشت !!(status & USBPORTSC_LSDA);
}

// Disable port
ایستا تهی
uhci_hub_disconnect(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_uhci_s *cntl = container_of(hub->cntl, ساختار usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

ایستا ساختار usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ایستا صحیح
check_uhci_ports(ساختار usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    ساختار usbhub_s hub;
    memset(&hub, 0, اندازه(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    بازگشت hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ایستا تهی
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    برای (;;) {
        اگر (inw(iobase + USBFRNUM) != startframe)
            بشکن;
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت;
        }
        yield();
    }
}

ایستا تهی
uhci_free_pipes(ساختار usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    ساختار uhci_qh *pos = (تهی*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    برای (;;) {
        u32 link = pos->link;
        اگر (link == UHCI_PTR_TERM)
            بشکن;
        ساختار uhci_qh *next = (تهی*)(link & ~UHCI_PTR_BITS);
        ساختار uhci_pipe *pipe = container_of(next, ساختار uhci_pipe, qh);
        اگر (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        وگرنه
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    برای (;;) {
        ساختار usb_pipe *usbpipe = cntl->usb.freelist;
        اگر (!usbpipe)
            بشکن;
        cntl->usb.freelist = usbpipe->freenext;
        ساختار uhci_pipe *pipe = container_of(usbpipe, ساختار uhci_pipe, pipe);
        آزاد(pipe);
    }
}

ایستا تهی
reset_uhci(ساختار usb_uhci_s *cntl, u16 bdf)
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

ایستا تهی
configure_uhci(تهی *data)
{
    ساختار usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    ساختار uhci_td *term_td = malloc_بالا(اندازه(*term_td));
    ساختار uhci_framelist *fl = memalign_بالا(اندازه(*fl), اندازه(*fl));
    ساختار uhci_pipe *intr_pipe = malloc_بالا(اندازه(*intr_pipe));
    ساختار uhci_pipe *term_pipe = malloc_بالا(اندازه(*term_pipe));
    اگر (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        برو fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, اندازه(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, اندازه(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, اندازه(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    صحیح i;
    برای (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    صحیح شمار = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    اگر (شمار)
        // Success
        بازگشت;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    آزاد(term_td);
    آزاد(fl);
    آزاد(intr_pipe);
    آزاد(term_pipe);
    آزاد(cntl);
}

ایستا تهی
uhci_controller_setup(ساختار pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    اگر (!iobase)
        بازگشت;

    ساختار usb_uhci_s *cntl = malloc_tmphigh(اندازه(*cntl));
    اگر (!cntl) {
        warn_noalloc();
        بازگشت;
    }
    memset(cntl, 0, اندازه(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

تهی
uhci_setup(تهی)
{
    اگر (! CONFIG_USB_UHCI)
        بازگشت;
    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

ایستا ساختار usb_pipe *
uhci_alloc_intr_pipe(ساختار usbdevice_s *usbdev
                     , ساختار usb_endpoint_descriptor *epdesc)
{
    ساختار usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, ساختار usb_uhci_s, usb);
    صحیح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    اگر (frameexp > 10)
        frameexp = 10;
    صحیح maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    صحیح ms = 1<<frameexp;
    صحیح شمار = DIV_ROUND_UP(ticks_to_ms(2), ms);
    شمار = ALIGN(شمار, 2);
    ساختار uhci_pipe *pipe = malloc_کم(اندازه(*pipe));
    ساختار uhci_td *tds = malloc_کم(اندازه(*tds) * شمار);
    تهی *data = malloc_کم(maxpacket * شمار);
    اگر (!pipe || !tds || !data) {
        warn_noalloc();
        برو fail;
    }
    memset(pipe, 0, اندازه(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    صحیح lowspeed = pipe->pipe.speed;
    صحیح devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    صحیح toggle = 0;
    صحیح i;
    برای (i=0; i<شمار; i++) {
        tds[i].link = (i==شمار-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    ساختار uhci_framelist *fl = cntl->framelist;
    اگر (frameexp == 0) {
        // Add to existing interrupt entry.
        ساختار uhci_qh *intr_qh = (تهی*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        اگر (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } وگرنه {
        صحیح startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        برای (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    بازگشت &pipe->pipe;
fail:
    آزاد(pipe);
    آزاد(tds);
    آزاد(data);
    بازگشت پوچ;
}

ساختار usb_pipe *
uhci_realloc_pipe(ساختار usbdevice_s *usbdev, ساختار usb_pipe *upipe
                  , ساختار usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_UHCI)
        بازگشت پوچ;
    usb_add_freelist(upipe);
    اگر (!epdesc)
        بازگشت پوچ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    اگر (eptype == USB_ENDPOINT_XFER_INT)
        بازگشت uhci_alloc_intr_pipe(usbdev, epdesc);
    ساختار usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, ساختار usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ساختار usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    اگر (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        بازگشت usbpipe;
    }

    // Allocate a new queue head.
    ساختار uhci_pipe *pipe;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(اندازه(*pipe));
    وگرنه
        pipe = malloc_کم(اندازه(*pipe));
    اگر (!pipe) {
        warn_noalloc();
        بازگشت پوچ;
    }
    memset(pipe, 0, اندازه(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    ساختار uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    بازگشت &pipe->pipe;
}

ایستا صحیح
wait_pipe(ساختار uhci_pipe *pipe, u32 end)
{
    برای (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        اگر (el_link & UHCI_PTR_TERM)
            بازگشت 0;
        اگر (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            ساختار uhci_td *td = (تهی*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (تهی*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            بازگشت -1;
        }
        yield();
    }
}

ایستا صحیح
wait_td(ساختار uhci_td *td, u32 end)
{
    u32 status;
    برای (;;) {
        status = td->status;
        اگر (!(status & TD_CTRL_ACTIVE))
            بشکن;
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت -1;
        }
        yield();
    }
    اگر (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        بازگشت -2;
    }
    بازگشت 0;
}

#define STACKTDS 16
#define TDALIGN 16

صحیح
uhci_send_pipe(ساختار usb_pipe *p, صحیح dir, ثابت تهی *cmd
               , تهی *data, صحیح datasize)
{
    اگر (! CONFIG_USB_UHCI)
        بازگشت -1;
    ساختار uhci_pipe *pipe = container_of(p, ساختار uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    صحیح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحیح lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    صحیح devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    صحیح toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[اندازه(ساختار uhci_td) * STACKTDS + TDALIGN - 1];
    ساختار uhci_td *tds = (تهی*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, اندازه(*tds) * STACKTDS);
    صحیح tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    اگر (cmd) {
        // Send setup pid on control transfers
        ساختار uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (تهی*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    وقتی (datasize) {
        // Send data pids
        ساختار uhci_td *td = &tds[tdpos++ % STACKTDS];
        صحیح ret = wait_td(td, end);
        اگر (ret)
            برو fail;

        صحیح transfer = datasize;
        اگر (transfer > maxpacket)
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
    اگر (cmd) {
        // Send status pid on control transfers
        ساختار uhci_td *td = &tds[tdpos++ % STACKTDS];
        صحیح ret = wait_td(td, end);
        اگر (ret)
            برو fail;
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
    بازگشت wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    بازگشت -1;
}

صحیح
uhci_poll_intr(ساختار usb_pipe *p, تهی *data)
{
    ASSERT16();
    اگر (! CONFIG_USB_UHCI)
        بازگشت -1;

    ساختار uhci_pipe *pipe = container_of(p, ساختار uhci_pipe, pipe);
    ساختار uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    اگر (status & TD_CTRL_ACTIVE)
        // No intrs found.
        بازگشت -1;
    // XXX - check for errors.

    // Copy data.
    تهی *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_کم, LOWFLAT2_کم(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    ساختار uhci_td *next = (تهی*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    بازگشت 0;
}
