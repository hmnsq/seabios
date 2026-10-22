// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_LOWFLAT
#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "pci.h" // pci_config_writew
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ضمن "pci_regs.h" // PCI_BASE_ADDRESS_4
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-ehci.h" // ehci_wait_controllers
#ضمن "usb-uhci.h" // USBLEGSUP
#ضمن "util.h" // msleep
#ضمن "x86.h" // outw

بنية usb_uhci_s {
    بنية usb_s usb;
    u16 iobase;
    بنية uhci_qh *control_qh;
    بنية uhci_framelist *framelist;
};

بنية uhci_pipe {
    بنية uhci_qh qh;
    بنية uhci_td *next_td;
    بنية usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
ساكن صحيح
uhci_hub_detect(بنية usbhub_s *hub, u32 port)
{
    بنية usb_uhci_s *cntl = container_of(hub->cntl, بنية usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    אם (!(status & USBPORTSC_CCS))
        // No device found.
        החזר 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    החזר 1;
}

// Reset device on port
ساكن صحيح
uhci_hub_reset(بنية usbhub_s *hub, u32 port)
{
    بنية usb_uhci_s *cntl = container_of(hub->cntl, بنية usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    אם (!(status & USBPORTSC_CCS))
        // No longer connected
        החזר -1;
    outw(USBPORTSC_PE, ioport);
    החזר !!(status & USBPORTSC_LSDA);
}

// Disable port
ساكن فراغ
uhci_hub_disconnect(بنية usbhub_s *hub, u32 port)
{
    بنية usb_uhci_s *cntl = container_of(hub->cntl, بنية usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

ساكن بنية usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساكن صحيح
check_uhci_ports(بنية usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    بنية usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    החזר hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ساكن فراغ
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    עבור (;;) {
        אם (inw(iobase + USBFRNUM) != startframe)
            اكسر;
        אם (timer_check(end)) {
            warn_timeout();
            החזר;
        }
        yield();
    }
}

ساكن فراغ
uhci_free_pipes(بنية usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    بنية uhci_qh *pos = (فراغ*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    עבור (;;) {
        u32 link = pos->link;
        אם (link == UHCI_PTR_TERM)
            اكسر;
        بنية uhci_qh *next = (فراغ*)(link & ~UHCI_PTR_BITS);
        بنية uhci_pipe *pipe = container_of(next, بنية uhci_pipe, qh);
        אם (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        אחרת
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    עבור (;;) {
        بنية usb_pipe *usbpipe = cntl->usb.freelist;
        אם (!usbpipe)
            اكسر;
        cntl->usb.freelist = usbpipe->freenext;
        بنية uhci_pipe *pipe = container_of(usbpipe, بنية uhci_pipe, pipe);
        حرر(pipe);
    }
}

ساكن فراغ
reset_uhci(بنية usb_uhci_s *cntl, u16 bdf)
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

ساكن فراغ
configure_uhci(فراغ *data)
{
    بنية usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    بنية uhci_td *term_td = malloc_high(حجم(*term_td));
    بنية uhci_framelist *fl = memalign_high(حجم(*fl), حجم(*fl));
    بنية uhci_pipe *intr_pipe = malloc_high(حجم(*intr_pipe));
    بنية uhci_pipe *term_pipe = malloc_high(حجم(*term_pipe));
    אם (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        اذهب fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, حجم(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, حجم(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, حجم(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    صحيح i;
    עבור (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    صحيح count = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    אם (count)
        // Success
        החזר;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    حرر(term_td);
    حرر(fl);
    حرر(intr_pipe);
    حرر(term_pipe);
    حرر(cntl);
}

ساكن فراغ
uhci_controller_setup(بنية pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    אם (!iobase)
        החזר;

    بنية usb_uhci_s *cntl = malloc_tmphigh(حجم(*cntl));
    אם (!cntl) {
        warn_noalloc();
        החזר;
    }
    memset(cntl, 0, حجم(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

فراغ
uhci_setup(فراغ)
{
    אם (! CONFIG_USB_UHCI)
        החזר;
    بنية pci_device *pci;
    foreachpci(pci) {
        אם (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

ساكن بنية usb_pipe *
uhci_alloc_intr_pipe(بنية usbdevice_s *usbdev
                     , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, بنية usb_uhci_s, usb);
    صحيح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    אם (frameexp > 10)
        frameexp = 10;
    صحيح maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    صحيح ms = 1<<frameexp;
    صحيح count = DIV_ROUND_UP(ticks_to_ms(2), ms);
    count = ALIGN(count, 2);
    بنية uhci_pipe *pipe = malloc_low(حجم(*pipe));
    بنية uhci_td *tds = malloc_low(حجم(*tds) * count);
    فراغ *data = malloc_low(maxpacket * count);
    אם (!pipe || !tds || !data) {
        warn_noalloc();
        اذهب fail;
    }
    memset(pipe, 0, حجم(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    صحيح lowspeed = pipe->pipe.speed;
    صحيح devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    صحيح toggle = 0;
    صحيح i;
    עבור (i=0; i<count; i++) {
        tds[i].link = (i==count-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    بنية uhci_framelist *fl = cntl->framelist;
    אם (frameexp == 0) {
        // Add to existing interrupt entry.
        بنية uhci_qh *intr_qh = (فراغ*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        אם (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } אחרת {
        صحيح startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        עבור (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    החזר &pipe->pipe;
fail:
    حرر(pipe);
    حرر(tds);
    حرر(data);
    החזר عدم;
}

بنية usb_pipe *
uhci_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *upipe
                  , بنية usb_endpoint_descriptor *epdesc)
{
    אם (! CONFIG_USB_UHCI)
        החזר عدم;
    usb_add_freelist(upipe);
    אם (!epdesc)
        החזר عدم;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    אם (eptype == USB_ENDPOINT_XFER_INT)
        החזר uhci_alloc_intr_pipe(usbdev, epdesc);
    بنية usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, بنية usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    بنية usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    אם (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        החזר usbpipe;
    }

    // Allocate a new queue head.
    بنية uhci_pipe *pipe;
    אם (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(حجم(*pipe));
    אחרת
        pipe = malloc_low(حجم(*pipe));
    אם (!pipe) {
        warn_noalloc();
        החזר عدم;
    }
    memset(pipe, 0, حجم(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    بنية uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    אם (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    החזר &pipe->pipe;
}

ساكن صحيح
wait_pipe(بنية uhci_pipe *pipe, u32 end)
{
    עבור (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        אם (el_link & UHCI_PTR_TERM)
            החזר 0;
        אם (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            بنية uhci_td *td = (فراغ*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (فراغ*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            החזר -1;
        }
        yield();
    }
}

ساكن صحيح
wait_td(بنية uhci_td *td, u32 end)
{
    u32 status;
    עבור (;;) {
        status = td->status;
        אם (!(status & TD_CTRL_ACTIVE))
            اكسر;
        אם (timer_check(end)) {
            warn_timeout();
            החזר -1;
        }
        yield();
    }
    אם (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        החזר -2;
    }
    החזר 0;
}

#define STACKTDS 16
#define TDALIGN 16

صحيح
uhci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *data, صحيح datasize)
{
    אם (! CONFIG_USB_UHCI)
        החזר -1;
    بنية uhci_pipe *pipe = container_of(p, بنية uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    صحيح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحيح lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    صحيح devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    صحيح toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[حجم(بنية uhci_td) * STACKTDS + TDALIGN - 1];
    بنية uhci_td *tds = (فراغ*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, حجم(*tds) * STACKTDS);
    صحيح tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    אם (cmd) {
        // Send setup pid on control transfers
        بنية uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (فراغ*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    כלעוד (datasize) {
        // Send data pids
        بنية uhci_td *td = &tds[tdpos++ % STACKTDS];
        صحيح ret = wait_td(td, end);
        אם (ret)
            اذهب fail;

        صحيح transfer = datasize;
        אם (transfer > maxpacket)
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
    אם (cmd) {
        // Send status pid on control transfers
        بنية uhci_td *td = &tds[tdpos++ % STACKTDS];
        صحيح ret = wait_td(td, end);
        אם (ret)
            اذهب fail;
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
    החזר wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    החזר -1;
}

صحيح
uhci_poll_intr(بنية usb_pipe *p, فراغ *data)
{
    ASSERT16();
    אם (! CONFIG_USB_UHCI)
        החזר -1;

    بنية uhci_pipe *pipe = container_of(p, بنية uhci_pipe, pipe);
    بنية uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    אם (status & TD_CTRL_ACTIVE)
        // No intrs found.
        החזר -1;
    // XXX - check for errors.

    // Copy data.
    فراغ *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_LOW, LOWFLAT2LOW(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    بنية uhci_td *next = (فراغ*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    החזר 0;
}
