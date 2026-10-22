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

কাঠামো usb_uhci_s {
    কাঠামো usb_s usb;
    u16 iobase;
    কাঠামো uhci_qh *control_qh;
    কাঠামো uhci_framelist *framelist;
};

কাঠামো uhci_pipe {
    কাঠামো uhci_qh qh;
    কাঠামো uhci_td *পরবর্তী_td;
    কাঠামো usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
স্থির পূর্ণসংখ্যা
uhci_hub_detect(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_uhci_s *cntl = container_of(hub->cntl, কাঠামো usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    যদি (!(status & USBPORTSC_CCS))
        // No device found.
        ফেরত 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    ফেরত 1;
}

// Reset device on port
স্থির পূর্ণসংখ্যা
uhci_hub_reset(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_uhci_s *cntl = container_of(hub->cntl, কাঠামো usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    যদি (!(status & USBPORTSC_CCS))
        // No longer connected
        ফেরত -1;
    outw(USBPORTSC_PE, ioport);
    ফেরত !!(status & USBPORTSC_LSDA);
}

// Disable port
স্থির শূন্য
uhci_hub_disconnect(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_uhci_s *cntl = container_of(hub->cntl, কাঠামো usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

স্থির কাঠামো usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
স্থির পূর্ণসংখ্যা
check_uhci_ports(কাঠামো usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    কাঠামো usbhub_s hub;
    memset(&hub, 0, আকার(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    ফেরত hub.devcount;
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
    জন্য (;;) {
        যদি (inw(iobase + USBFRNUM) != startframe)
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত;
        }
        yield();
    }
}

স্থির শূন্য
uhci_free_pipes(কাঠামো usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    কাঠামো uhci_qh *pos = (শূন্য*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    জন্য (;;) {
        u32 link = pos->link;
        যদি (link == UHCI_PTR_TERM)
            বিরতি;
        কাঠামো uhci_qh *পরবর্তী = (শূন্য*)(link & ~UHCI_PTR_BITS);
        কাঠামো uhci_pipe *pipe = container_of(পরবর্তী, কাঠামো uhci_pipe, qh);
        যদি (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = পরবর্তী->link;
        অন্যথা
            pos = পরবর্তী;
    }
    uhci_waittick(cntl->iobase);
    জন্য (;;) {
        কাঠামো usb_pipe *usbpipe = cntl->usb.freelist;
        যদি (!usbpipe)
            বিরতি;
        cntl->usb.freelist = usbpipe->freenext;
        কাঠামো uhci_pipe *pipe = container_of(usbpipe, কাঠামো uhci_pipe, pipe);
        মুক্ত(pipe);
    }
}

স্থির শূন্য
reset_uhci(কাঠামো usb_uhci_s *cntl, u16 bdf)
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
configure_uhci(শূন্য *তথ্য)
{
    কাঠামো usb_uhci_s *cntl = তথ্য;

    // Allocate ram for schedule storage
    কাঠামো uhci_td *term_td = malloc_উপরের(আকার(*term_td));
    কাঠামো uhci_framelist *fl = memalign_উপরের(আকার(*fl), আকার(*fl));
    কাঠামো uhci_pipe *intr_pipe = malloc_উপরের(আকার(*intr_pipe));
    কাঠামো uhci_pipe *term_pipe = malloc_উপরের(আকার(*term_pipe));
    যদি (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        যাও fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, আকার(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, আকার(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, আকার(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<ARRAY_আকার(fl->links); i++)
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
    পূর্ণসংখ্যা গুনতি = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    যদি (গুনতি)
        // Success
        ফেরত;

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
uhci_controller_setup(কাঠামো pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    যদি (!iobase)
        ফেরত;

    কাঠামো usb_uhci_s *cntl = malloc_tmphigh(আকার(*cntl));
    যদি (!cntl) {
        warn_noalloc();
        ফেরত;
    }
    memset(cntl, 0, আকার(*cntl));
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
        ফেরত;
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

স্থির কাঠামো usb_pipe *
uhci_alloc_intr_pipe(কাঠামো usbdevice_s *usbdev
                     , কাঠামো usb_endpoint_descriptor *epdesc)
{
    কাঠামো usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, কাঠামো usb_uhci_s, usb);
    পূর্ণসংখ্যা frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    যদি (frameexp > 10)
        frameexp = 10;
    পূর্ণসংখ্যা maxpacket = epdesc->w_Max_Packet_আকার;
    // Determine number of entries needed for 2 timer ticks.
    পূর্ণসংখ্যা ms = 1<<frameexp;
    পূর্ণসংখ্যা গুনতি = DIV_ROUND_UP(ticks_to_ms(2), ms);
    গুনতি = ALIGN(গুনতি, 2);
    কাঠামো uhci_pipe *pipe = malloc_নিচের(আকার(*pipe));
    কাঠামো uhci_td *tds = malloc_নিচের(আকার(*tds) * গুনতি);
    শূন্য *তথ্য = malloc_নিচের(maxpacket * গুনতি);
    যদি (!pipe || !tds || !তথ্য) {
        warn_noalloc();
        যাও fail;
    }
    memset(pipe, 0, আকার(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    পূর্ণসংখ্যা lowspeed = pipe->pipe.speed;
    পূর্ণসংখ্যা devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->পরবর্তী_td = &tds[0];
    pipe->iobase = cntl->iobase;

    পূর্ণসংখ্যা toggle = 0;
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<গুনতি; i++) {
        tds[i].link = (i==গুনতি-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].বাফার = তথ্য + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    কাঠামো uhci_framelist *fl = cntl->framelist;
    যদি (frameexp == 0) {
        // Add to existing interrupt entry.
        কাঠামো uhci_qh *intr_qh = (শূন্য*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        যদি (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } অন্যথা {
        পূর্ণসংখ্যা startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        জন্য (i=startpos; i<ARRAY_আকার(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    ফেরত &pipe->pipe;
fail:
    মুক্ত(pipe);
    মুক্ত(tds);
    মুক্ত(তথ্য);
    ফেরত ফাঁকা;
}

কাঠামো usb_pipe *
uhci_realloc_pipe(কাঠামো usbdevice_s *usbdev, কাঠামো usb_pipe *upipe
                  , কাঠামো usb_endpoint_descriptor *epdesc)
{
    যদি (! CONFIG_USB_UHCI)
        ফেরত ফাঁকা;
    usb_add_freelist(upipe);
    যদি (!epdesc)
        ফেরত ফাঁকা;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    যদি (eptype == USB_ENDPOINT_XFER_INT)
        ফেরত uhci_alloc_intr_pipe(usbdev, epdesc);
    কাঠামো usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, কাঠামো usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    কাঠামো usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    যদি (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        ফেরত usbpipe;
    }

    // Allocate a new queue head.
    কাঠামো uhci_pipe *pipe;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(আকার(*pipe));
    অন্যথা
        pipe = malloc_নিচের(আকার(*pipe));
    যদি (!pipe) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(pipe, 0, আকার(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    কাঠামো uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    ফেরত &pipe->pipe;
}

স্থির পূর্ণসংখ্যা
wait_pipe(কাঠামো uhci_pipe *pipe, u32 end)
{
    জন্য (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        যদি (el_link & UHCI_PTR_TERM)
            ফেরত 0;
        যদি (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            কাঠামো uhci_td *td = (শূন্য*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (শূন্য*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            ফেরত -1;
        }
        yield();
    }
}

স্থির পূর্ণসংখ্যা
wait_td(কাঠামো uhci_td *td, u32 end)
{
    u32 status;
    জন্য (;;) {
        status = td->status;
        যদি (!(status & TD_CTRL_ACTIVE))
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত -1;
        }
        yield();
    }
    যদি (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        ফেরত -2;
    }
    ফেরত 0;
}

#define STACKTDS 16
#define TDALIGN 16

পূর্ণসংখ্যা
uhci_send_pipe(কাঠামো usb_pipe *p, পূর্ণসংখ্যা dir, ধ্রুবক শূন্য *cmd
               , শূন্য *তথ্য, পূর্ণসংখ্যা datasize)
{
    যদি (! CONFIG_USB_UHCI)
        ফেরত -1;
    কাঠামো uhci_pipe *pipe = container_of(p, কাঠামো uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, তথ্য, datasize);
    পূর্ণসংখ্যা maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    পূর্ণসংখ্যা lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    পূর্ণসংখ্যা devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    পূর্ণসংখ্যা toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[আকার(কাঠামো uhci_td) * STACKTDS + TDALIGN - 1];
    কাঠামো uhci_td *tds = (শূন্য*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, আকার(*tds) * STACKTDS);
    পূর্ণসংখ্যা tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    যদি (cmd) {
        // Send setup pid on control transfers
        কাঠামো uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_আকার)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->বাফার = (শূন্য*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    যখন (datasize) {
        // Send data pids
        কাঠামো uhci_td *td = &tds[tdpos++ % STACKTDS];
        পূর্ণসংখ্যা ret = wait_td(td, end);
        যদি (ret)
            যাও fail;

        পূর্ণসংখ্যা transfer = datasize;
        যদি (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_TERM : (nexttd | UHCI_PTR_DEPTH));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_IN : USB_PID_OUT));
        td->বাফার = তথ্য;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle ^= TD_TOKEN_TOGGLE;

        তথ্য += transfer;
        datasize -= transfer;
    }
    যদি (cmd) {
        // Send status pid on control transfers
        কাঠামো uhci_td *td = &tds[tdpos++ % STACKTDS];
        পূর্ণসংখ্যা ret = wait_td(td, end);
        যদি (ret)
            যাও fail;
        td->link = UHCI_PTR_TERM;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_OUT : USB_PID_IN));
        td->বাফার = 0;
        barrier();
        td->status = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
    }
    SET_LOWFLAT(pipe->toggle, !!toggle);
    ফেরত wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    ফেরত -1;
}

পূর্ণসংখ্যা
uhci_poll_intr(কাঠামো usb_pipe *p, শূন্য *তথ্য)
{
    ASSERT16();
    যদি (! CONFIG_USB_UHCI)
        ফেরত -1;

    কাঠামো uhci_pipe *pipe = container_of(p, কাঠামো uhci_pipe, pipe);
    কাঠামো uhci_td *td = GET_LOWFLAT(pipe->পরবর্তী_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    যদি (status & TD_CTRL_ACTIVE)
        // No intrs found.
        ফেরত -1;
    // XXX - check for errors.

    // Copy data.
    শূন্য *tddata = GET_LOWFLAT(td->বাফার);
    memcpy_far(GET_SEG(SS), তথ্য, SEG_নিচের, LOWFLAT2_নিচের(tddata)
               , uhci_expected_দৈর্ঘ্য(token));

    // Reenable this td.
    কাঠামো uhci_td *পরবর্তী = (শূন্য*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->পরবর্তী_td, পরবর্তী);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    ফেরত 0;
}
