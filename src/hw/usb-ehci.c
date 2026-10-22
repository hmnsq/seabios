// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "biosvar.h" // GET_LOWFLAT
#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "memmap.h" // PAGE_SIZE
#অন্তর্ভাৱ "pcidevice.h" // foreachpci
#অন্তর্ভাৱ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#অন্তর্ভাৱ "pci_regs.h" // PCI_BASE_ADDRESS_0
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "usb.h" // struct usb_s
#অন্তর্ভাৱ "usb-ehci.h" // struct ehci_qh
#অন্তর্ভাৱ "util.h" // msleep
#অন্তর্ভাৱ "x86.h" // readl

সংরচনা usb_ehci_s {
    সংরচনা usb_s usb;
    সংরচনা ehci_caps *caps;
    সংরচনা ehci_regs *regs;
    সংরচনা ehci_qh *async_qh;
    পূর্ণাঙ্ক checkports;
};

সংরচনা ehci_pipe {
    সংরচনা ehci_qh qh;
    সংরচনা ehci_qtd *next_td, *tds;
    শূন্য *data;
    সংরচনা usb_pipe pipe;
};

স্থির পূর্ণাঙ্ক PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
স্থির পূর্ণাঙ্ক
ehci_hub_detect(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_ehci_s *cntl = container_of(hub->cntl, সংরচনা usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    যদি (!(portsc & PORT_CONNECT))
        // No device present
        ঘূৰাই 0;

    যদি ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        ঘূৰাই -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    ঘূৰাই 1;
}

// Reset device on port
স্থির পূর্ণাঙ্ক
ehci_hub_reset(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_ehci_s *cntl = container_of(hub->cntl, সংরচনা usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    যদি (!(portsc & PORT_CONNECT))
        // No longer connected
        ঘূৰাই -1;
    যদি (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        ঘূৰাই -1;
    }

    ঘূৰাই USB_HIGHSPEED;
}

// Disable port
স্থির শূন্য
ehci_hub_disconnect(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_ehci_s *cntl = container_of(hub->cntl, সংরচনা usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

স্থির সংরচনা usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
স্থির পূর্ণাঙ্ক
check_ehci_ports(সংরচনা usb_ehci_s *cntl)
{
    // Power up ports.
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        যদি (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    সংরচনা usbhub_s hub;
    memset(&hub, 0, পরিমাণ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    ঘূৰাই hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
স্থির শূন্য
ehci_waittick(সংরচনা usb_ehci_s *cntl)
{
    যদি (MODE16) {
        msleep(10);
        ঘূৰাই;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    বাবে (;;) {
        sts = readl(&cntl->regs->usbsts);
        যদি (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            যদি (!(cmd & CMD_IAAD))
                ভঙ্গ;
        }
        যদি (timer_check(end)) {
            warn_timeout();
            ঘূৰাই;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    বাবে (;;) {
        sts = readl(&cntl->regs->usbsts);
        যদি (sts & STS_IAA)
            ভঙ্গ;
        যদি (timer_check(end)) {
            warn_timeout();
            ঘূৰাই;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

স্থির শূন্য
ehci_free_pipes(সংরচনা usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    সংরচনা ehci_qh *start = cntl->async_qh;
    সংরচনা ehci_qh *pos = start;
    বাবে (;;) {
        সংরচনা ehci_qh *next = (শূন্য*)(pos->next & ~EHCI_PTR_BITS);
        যদি (next == start)
            ভঙ্গ;
        সংরচনা ehci_pipe *pipe = container_of(next, সংরচনা ehci_pipe, qh);
        যদি (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        অন্যথা
            pos = next;
    }
    ehci_waittick(cntl);
    বাবে (;;) {
        সংরচনা usb_pipe *usbpipe = cntl->usb.freelist;
        যদি (!usbpipe)
            ভঙ্গ;
        cntl->usb.freelist = usbpipe->freenext;
        সংরচনা ehci_pipe *pipe = container_of(usbpipe, সংরচনা ehci_pipe, pipe);
        মুক্ত(pipe);
    }
}

স্থির শূন্য
configure_ehci(শূন্য *data)
{
    সংরচনা usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    সংরচনা ehci_framelist *fl = memalign_উপৰেৰ(পরিমাণ(*fl), পরিমাণ(*fl));
    সংরচনা ehci_qh *intr_qh = memalign_উপৰেৰ(EHCI_QH_ALIGN, পরিমাণ(*intr_qh));
    সংরচনা ehci_qh *async_qh = memalign_উপৰেৰ(EHCI_QH_ALIGN, পরিমাণ(*async_qh));
    যদি (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        গচ্ছ fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    বাবে (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        যদি (!(cmd & CMD_HCRESET))
            ভঙ্গ;
        যদি (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            গচ্ছ fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, পরিমাণ(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, পরিমাণ(*async_qh));
    async_qh->next = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_HEAD;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_next = async_qh->alt_next = EHCI_PTR_TERM;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_RUN);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    পূর্ণাঙ্ক গুনতি = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    যদি (গুনতি)
        // Success
        ঘূৰাই;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    মুক্ত(fl);
    মুক্ত(intr_qh);
    মুক্ত(async_qh);
    মুক্ত(cntl);
}

স্থির শূন্য
ehci_controller_setup(সংরচনা pci_device *pci)
{
    সংরচনা ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!caps)
        ঘূৰাই;
    u32 hcc_params = readl(&caps->hccparams);

    সংরচনা usb_ehci_s *cntl = malloc_tmphigh(পরিমাণ(*cntl));
    যদি (!cntl) {
        warn_noalloc();
        ঘূৰাই;
    }
    memset(cntl, 0, পরিমাণ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (শূন্য*)caps + readb(&caps->caplength);
    যদি (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

শূন্য
ehci_setup(শূন্য)
{
    যদি (! CONFIG_USB_EHCI)
        ঘূৰাই;
    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
শূন্য
ehci_wait_controllers(শূন্য)
{
    যেতিয়ালৈ (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
স্থির শূন্য
ehci_desc2pipe(সংরচনা ehci_pipe *pipe, সংরচনা usbdevice_s *usbdev
               , সংরচনা usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    সংরচনা usbdevice_s *hubdev = usbdev->hub->usbdev;
    যদি (hubdev) {
        সংরচনা ehci_pipe *hpipe = container_of(
            hubdev->defpipe, সংরচনা ehci_pipe, pipe);
        যদি (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        অন্যথা
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    অন্যথা যদি (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

স্থির সংরচনা usb_pipe *
ehci_alloc_intr_pipe(সংরচনা usbdevice_s *usbdev
                     , সংরচনা usb_endpoint_descriptor *epdesc)
{
    সংরচনা usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, সংরচনা usb_ehci_s, usb);
    পূর্ণাঙ্ক frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    যদি (frameexp > 10)
        frameexp = 10;
    পূর্ণাঙ্ক maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    পূর্ণাঙ্ক ms = 1<<frameexp;
    পূর্ণাঙ্ক গুনতি = DIV_ROUND_UP(ticks_to_ms(2), ms);
    সংরচনা ehci_pipe *pipe = memalign_নিচেৰ(EHCI_QH_ALIGN, পরিমাণ(*pipe));
    সংরচনা ehci_qtd *tds = memalign_নিচেৰ(EHCI_QTD_ALIGN, পরিমাণ(*tds) * গুনতি);
    শূন্য *data = malloc_নিচেৰ(maxpacket * গুনতি);
    যদি (!pipe || !tds || !data) {
        warn_noalloc();
        গচ্ছ fail;
    }
    memset(pipe, 0, পরিমাণ(*pipe));
    memset(tds, 0, পরিমাণ(*tds) * গুনতি);
    memset(data, 0, maxpacket * গুনতি);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    পূর্ণাঙ্ক i;
    বাবে (i=0; i<গুনতি; i++) {
        সংরচনা ehci_qtd *td = &tds[i];
        td->qtd_next = (i==গুনতি-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    সংরচনা ehci_framelist *fl = (শূন্য*)readl(&cntl->regs->periodiclistbase);
    যদি (frameexp == 0) {
        // Add to existing interrupt entry.
        সংরচনা ehci_qh *intr_qh = (শূন্য*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } অন্যথা {
        পূর্ণাঙ্ক startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        বাবে (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    ঘূৰাই &pipe->pipe;
fail:
    মুক্ত(pipe);
    মুক্ত(tds);
    মুক্ত(data);
    ঘূৰাই রিক্ত;
}

সংরচনা usb_pipe *
ehci_realloc_pipe(সংরচনা usbdevice_s *usbdev, সংরচনা usb_pipe *upipe
                  , সংরচনা usb_endpoint_descriptor *epdesc)
{
    যদি (! CONFIG_USB_EHCI)
        ঘূৰাই রিক্ত;
    usb_add_freelist(upipe);
    যদি (!epdesc)
        ঘূৰাই রিক্ত;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    যদি (eptype == USB_ENDPOINT_XFER_INT)
        ঘূৰাই ehci_alloc_intr_pipe(usbdev, epdesc);
    সংরচনা usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, সংরচনা usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    সংরচনা usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    যদি (usbpipe) {
        // Use previously allocated pipe.
        সংরচনা ehci_pipe *pipe = container_of(usbpipe, সংরচনা ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        ঘূৰাই usbpipe;
    }

    // Allocate a new queue head.
    সংরচনা ehci_pipe *pipe;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, পরিমাণ(*pipe));
    অন্যথা
        pipe = memalign_নিচেৰ(EHCI_QH_ALIGN, পরিমাণ(*pipe));
    যদি (!pipe) {
        warn_noalloc();
        ঘূৰাই রিক্ত;
    }
    memset(pipe, 0, পরিমাণ(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    সংরচনা ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    ঘূৰাই &pipe->pipe;
}

স্থির শূন্য
ehci_reset_pipe(সংরচনা ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

স্থির পূর্ণাঙ্ক
ehci_wait_td(সংরচনা ehci_pipe *pipe, সংরচনা ehci_qtd *td, u32 end)
{
    u32 status;
    বাবে (;;) {
        status = td->token;
        যদি (!(status & QTD_STS_ACTIVE))
            ভঙ্গ;
        যদি (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            সংরচনা usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), সংরচনা usb_ehci_s, usb);
            ehci_waittick(cntl);
            ঘূৰাই -1;
        }
        yield();
    }
    যদি (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        ঘূৰাই -2;
    }
    ঘূৰাই 0;
}

স্থির শূন্য
ehci_fill_tdbuf(সংরচনা ehci_qtd *td, u32 dest, পূর্ণাঙ্ক transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    বাবে (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

পূর্ণাঙ্ক
ehci_send_pipe(সংরচনা usb_pipe *p, পূর্ণাঙ্ক dir, ধ্রুৱ শূন্য *cmd
               , শূন্য *data, পূর্ণাঙ্ক datasize)
{
    যদি (! CONFIG_USB_EHCI)
        ঘূৰাই -1;
    সংরচনা ehci_pipe *pipe = container_of(p, সংরচনা ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[পরিমাণ(সংরচনা ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    সংরচনা ehci_qtd *tds = (শূন্য*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, পরিমাণ(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    যদি (cmd) {
        // Send setup pid on control transfers
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(USB_CONTROL_SETUP_SIZE) | QTD_STS_ACTIVE
                     | QTD_PID_SETUP | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_SETUP_SIZE);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)data, dataend = dest + datasize;
    যেতিয়ালৈ (dest < dataend) {
        // Send data pids
        যদি (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ঘূৰাই -1;
        }
        পূর্ণাঙ্ক maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        পূর্ণাঙ্ক transfer = dataend - dest;
        যদি (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    যদি (cmd) {
        // Send status pid on control transfers
        যদি (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ঘূৰাই -1;
        }
        td->qtd_next = EHCI_PTR_TERM;
        td->alt_next = EHCI_PTR_TERM;
        td->token = (QTD_TOGGLE | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_OUT : QTD_PID_IN) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_next = EHCI_PTR_TERM;
    barrier();
    SET_LOWFLAT(pipe->qh.qtd_next, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    পূর্ণাঙ্ক i;
    বাবে (i=0, td=tds; i<STACKQTDS; i++, td++) {
        পূর্ণাঙ্ক ret = ehci_wait_td(pipe, td, end);
        যদি (ret)
            ঘূৰাই -1;
    }

    ঘূৰাই 0;
}

পূর্ণাঙ্ক
ehci_poll_intr(সংরচনা usb_pipe *p, শূন্য *data)
{
    ASSERT16();
    যদি (! CONFIG_USB_EHCI)
        ঘূৰাই -1;
    সংরচনা ehci_pipe *pipe = container_of(p, সংরচনা ehci_pipe, pipe);
    সংরচনা ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    যদি (token & QTD_STS_ACTIVE)
        // No intrs found.
        ঘূৰাই -1;
    // XXX - check for errors.

    // Copy data.
    পূর্ণাঙ্ক maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    পূর্ণাঙ্ক pos = td - GET_LOWFLAT(pipe->tds);
    শূন্য *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_নিচেৰ, LOWFLAT2_নিচেৰ(tddata), maxpacket);

    // Reenable this td.
    সংরচনা ehci_qtd *next = (শূন্য*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    ঘূৰাই 0;
}
