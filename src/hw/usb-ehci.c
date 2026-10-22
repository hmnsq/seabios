// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_LOWFLAT
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "memmap.h" // PAGE_SIZE
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#অন্তর্ভাব "pci_regs.h" // PCI_BASE_ADDRESS_0
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "usb.h" // struct usb_s
#অন্তর্ভাব "usb-ehci.h" // struct ehci_qh
#অন্তর্ভাব "util.h" // msleep
#অন্তর্ভাব "x86.h" // readl

কাঠামো usb_ehci_s {
    কাঠামো usb_s usb;
    কাঠামো ehci_caps *caps;
    কাঠামো ehci_regs *regs;
    কাঠামো ehci_qh *async_qh;
    পূর্ণসংখ্যা checkports;
};

কাঠামো ehci_pipe {
    কাঠামো ehci_qh qh;
    কাঠামো ehci_qtd *পরবর্তী_td, *tds;
    শূন্য *তথ্য;
    কাঠামো usb_pipe pipe;
};

স্থির পূর্ণসংখ্যা PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
স্থির পূর্ণসংখ্যা
ehci_hub_detect(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_ehci_s *cntl = container_of(hub->cntl, কাঠামো usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    যদি (!(portsc & PORT_CONNECT))
        // No device present
        ফেরত 0;

    যদি ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        ফেরত -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    ফেরত 1;
}

// Reset device on port
স্থির পূর্ণসংখ্যা
ehci_hub_reset(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_ehci_s *cntl = container_of(hub->cntl, কাঠামো usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    যদি (!(portsc & PORT_CONNECT))
        // No longer connected
        ফেরত -1;
    যদি (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        ফেরত -1;
    }

    ফেরত USB_HIGHSPEED;
}

// Disable port
স্থির শূন্য
ehci_hub_disconnect(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_ehci_s *cntl = container_of(hub->cntl, কাঠামো usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

স্থির কাঠামো usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
স্থির পূর্ণসংখ্যা
check_ehci_ports(কাঠামো usb_ehci_s *cntl)
{
    // Power up ports.
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        যদি (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    কাঠামো usbhub_s hub;
    memset(&hub, 0, আকার(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    ফেরত hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
স্থির শূন্য
ehci_waittick(কাঠামো usb_ehci_s *cntl)
{
    যদি (MODE16) {
        msleep(10);
        ফেরত;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    জন্য (;;) {
        sts = readl(&cntl->regs->usbsts);
        যদি (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            যদি (!(cmd & CMD_IAAD))
                বিরতি;
        }
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    জন্য (;;) {
        sts = readl(&cntl->regs->usbsts);
        যদি (sts & STS_IAA)
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

স্থির শূন্য
ehci_free_pipes(কাঠামো usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    কাঠামো ehci_qh *start = cntl->async_qh;
    কাঠামো ehci_qh *pos = start;
    জন্য (;;) {
        কাঠামো ehci_qh *পরবর্তী = (শূন্য*)(pos->পরবর্তী & ~EHCI_PTR_BITS);
        যদি (পরবর্তী == start)
            বিরতি;
        কাঠামো ehci_pipe *pipe = container_of(পরবর্তী, কাঠামো ehci_pipe, qh);
        যদি (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->পরবর্তী = পরবর্তী->পরবর্তী;
        অন্যথা
            pos = পরবর্তী;
    }
    ehci_waittick(cntl);
    জন্য (;;) {
        কাঠামো usb_pipe *usbpipe = cntl->usb.freelist;
        যদি (!usbpipe)
            বিরতি;
        cntl->usb.freelist = usbpipe->freenext;
        কাঠামো ehci_pipe *pipe = container_of(usbpipe, কাঠামো ehci_pipe, pipe);
        মুক্ত(pipe);
    }
}

স্থির শূন্য
configure_ehci(শূন্য *তথ্য)
{
    কাঠামো usb_ehci_s *cntl = তথ্য;

    // Allocate ram for schedule storage
    কাঠামো ehci_framelist *fl = memalign_উপরের(আকার(*fl), আকার(*fl));
    কাঠামো ehci_qh *intr_qh = memalign_উপরের(EHCI_QH_ALIGN, আকার(*intr_qh));
    কাঠামো ehci_qh *async_qh = memalign_উপরের(EHCI_QH_ALIGN, আকার(*async_qh));
    যদি (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        যাও fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    জন্য (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        যদি (!(cmd & CMD_HCRESET))
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            যাও fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, আকার(*intr_qh));
    intr_qh->পরবর্তী = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_পরবর্তী = intr_qh->alt_পরবর্তী = EHCI_PTR_TERM;
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<ARRAY_আকার(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, আকার(*async_qh));
    async_qh->পরবর্তী = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_শীর্ষ;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_পরবর্তী = async_qh->alt_পরবর্তী = EHCI_PTR_TERM;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_RUN);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    পূর্ণসংখ্যা গুনতি = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    যদি (গুনতি)
        // Success
        ফেরত;

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
ehci_controller_setup(কাঠামো pci_device *pci)
{
    কাঠামো ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!caps)
        ফেরত;
    u32 hcc_params = readl(&caps->hccparams);

    কাঠামো usb_ehci_s *cntl = malloc_tmphigh(আকার(*cntl));
    যদি (!cntl) {
        warn_noalloc();
        ফেরত;
    }
    memset(cntl, 0, আকার(*cntl));
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
        ফেরত;
    কাঠামো pci_device *pci;
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
    যখন (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
স্থির শূন্য
ehci_desc2pipe(কাঠামো ehci_pipe *pipe, কাঠামো usbdevice_s *usbdev
               , কাঠামো usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    কাঠামো usbdevice_s *hubdev = usbdev->hub->usbdev;
    যদি (hubdev) {
        কাঠামো ehci_pipe *hpipe = container_of(
            hubdev->defpipe, কাঠামো ehci_pipe, pipe);
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

স্থির কাঠামো usb_pipe *
ehci_alloc_intr_pipe(কাঠামো usbdevice_s *usbdev
                     , কাঠামো usb_endpoint_descriptor *epdesc)
{
    কাঠামো usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, কাঠামো usb_ehci_s, usb);
    পূর্ণসংখ্যা frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    যদি (frameexp > 10)
        frameexp = 10;
    পূর্ণসংখ্যা maxpacket = epdesc->w_Max_Packet_আকার;
    // Determine number of entries needed for 2 timer ticks.
    পূর্ণসংখ্যা ms = 1<<frameexp;
    পূর্ণসংখ্যা গুনতি = DIV_ROUND_UP(ticks_to_ms(2), ms);
    কাঠামো ehci_pipe *pipe = memalign_নিচের(EHCI_QH_ALIGN, আকার(*pipe));
    কাঠামো ehci_qtd *tds = memalign_নিচের(EHCI_QTD_ALIGN, আকার(*tds) * গুনতি);
    শূন্য *তথ্য = malloc_নিচের(maxpacket * গুনতি);
    যদি (!pipe || !tds || !তথ্য) {
        warn_noalloc();
        যাও fail;
    }
    memset(pipe, 0, আকার(*pipe));
    memset(tds, 0, আকার(*tds) * গুনতি);
    memset(তথ্য, 0, maxpacket * গুনতি);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->পরবর্তী_td = pipe->tds = tds;
    pipe->তথ্য = তথ্য;
    pipe->qh.qtd_পরবর্তী = (u32)tds;

    পূর্ণসংখ্যা i;
    জন্য (i=0; i<গুনতি; i++) {
        কাঠামো ehci_qtd *td = &tds[i];
        td->qtd_পরবর্তী = (i==গুনতি-1 ? (u32)tds : (u32)&td[1]);
        td->alt_পরবর্তী = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)তথ্য + maxpacket * i;
    }

    // Add to interrupt schedule.
    কাঠামো ehci_framelist *fl = (শূন্য*)readl(&cntl->regs->periodiclistbase);
    যদি (frameexp == 0) {
        // Add to existing interrupt entry.
        কাঠামো ehci_qh *intr_qh = (শূন্য*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.পরবর্তী = intr_qh->পরবর্তী;
        barrier();
        intr_qh->পরবর্তী = (u32)&pipe->qh | EHCI_PTR_QH;
    } অন্যথা {
        পূর্ণসংখ্যা startpos = 1<<(frameexp-1);
        pipe->qh.পরবর্তী = fl->links[startpos];
        barrier();
        জন্য (i=startpos; i<ARRAY_আকার(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    ফেরত &pipe->pipe;
fail:
    মুক্ত(pipe);
    মুক্ত(tds);
    মুক্ত(তথ্য);
    ফেরত ফাঁকা;
}

কাঠামো usb_pipe *
ehci_realloc_pipe(কাঠামো usbdevice_s *usbdev, কাঠামো usb_pipe *upipe
                  , কাঠামো usb_endpoint_descriptor *epdesc)
{
    যদি (! CONFIG_USB_EHCI)
        ফেরত ফাঁকা;
    usb_add_freelist(upipe);
    যদি (!epdesc)
        ফেরত ফাঁকা;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    যদি (eptype == USB_ENDPOINT_XFER_INT)
        ফেরত ehci_alloc_intr_pipe(usbdev, epdesc);
    কাঠামো usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, কাঠামো usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    কাঠামো usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    যদি (usbpipe) {
        // Use previously allocated pipe.
        কাঠামো ehci_pipe *pipe = container_of(usbpipe, কাঠামো ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        ফেরত usbpipe;
    }

    // Allocate a new queue head.
    কাঠামো ehci_pipe *pipe;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, আকার(*pipe));
    অন্যথা
        pipe = memalign_নিচের(EHCI_QH_ALIGN, আকার(*pipe));
    যদি (!pipe) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(pipe, 0, আকার(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_পরবর্তী = pipe->qh.alt_পরবর্তী = EHCI_PTR_TERM;

    // Add queue head to controller list.
    কাঠামো ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.পরবর্তী = async_qh->পরবর্তী;
    barrier();
    async_qh->পরবর্তী = (u32)&pipe->qh | EHCI_PTR_QH;
    ফেরত &pipe->pipe;
}

স্থির শূন্য
ehci_reset_pipe(কাঠামো ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_পরবর্তী, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_পরবর্তী, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

স্থির পূর্ণসংখ্যা
ehci_wait_td(কাঠামো ehci_pipe *pipe, কাঠামো ehci_qtd *td, u32 end)
{
    u32 status;
    জন্য (;;) {
        status = td->token;
        যদি (!(status & QTD_STS_ACTIVE))
            বিরতি;
        যদি (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 পরবর্তী = GET_LOWFLAT(pipe->qh.qtd_পরবর্তী);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, পরবর্তী, td, status);
            ehci_reset_pipe(pipe);
            কাঠামো usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), কাঠামো usb_ehci_s, usb);
            ehci_waittick(cntl);
            ফেরত -1;
        }
        yield();
    }
    যদি (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        ফেরত -2;
    }
    ফেরত 0;
}

স্থির শূন্য
ehci_fill_tdbuf(কাঠামো ehci_qtd *td, u32 dest, পূর্ণসংখ্যা transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    জন্য (; dest < end; dest = ALIGN_DOWN(dest + PAGE_আকার, PAGE_আকার))
        *pos++ = dest;
}

#define STACKQTDS 6

পূর্ণসংখ্যা
ehci_send_pipe(কাঠামো usb_pipe *p, পূর্ণসংখ্যা dir, ধ্রুবক শূন্য *cmd
               , শূন্য *তথ্য, পূর্ণসংখ্যা datasize)
{
    যদি (! CONFIG_USB_EHCI)
        ফেরত -1;
    কাঠামো ehci_pipe *pipe = container_of(p, কাঠামো ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, তথ্য, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[আকার(কাঠামো ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    কাঠামো ehci_qtd *tds = (শূন্য*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, আকার(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    যদি (cmd) {
        // Send setup pid on control transfers
        td->qtd_পরবর্তী = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_পরবর্তী = EHCI_PTR_TERM;
        td->token = (ehci_explen(USB_CONTROL_SETUP_আকার) | QTD_STS_ACTIVE
                     | QTD_PID_SETUP | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_SETUP_আকার);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)তথ্য, dataend = dest + datasize;
    যখন (dest < dataend) {
        // Send data pids
        যদি (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ফেরত -1;
        }
        পূর্ণসংখ্যা maxtransfer = 5*PAGE_আকার - (dest & (PAGE_আকার-1));
        পূর্ণসংখ্যা transfer = dataend - dest;
        যদি (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_পরবর্তী = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_পরবর্তী = EHCI_PTR_TERM;
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
            ফেরত -1;
        }
        td->qtd_পরবর্তী = EHCI_PTR_TERM;
        td->alt_পরবর্তী = EHCI_PTR_TERM;
        td->token = (QTD_TOGGLE | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_OUT : QTD_PID_IN) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_পরবর্তী = EHCI_PTR_TERM;
    barrier();
    SET_LOWFLAT(pipe->qh.qtd_পরবর্তী, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    পূর্ণসংখ্যা i;
    জন্য (i=0, td=tds; i<STACKQTDS; i++, td++) {
        পূর্ণসংখ্যা ret = ehci_wait_td(pipe, td, end);
        যদি (ret)
            ফেরত -1;
    }

    ফেরত 0;
}

পূর্ণসংখ্যা
ehci_poll_intr(কাঠামো usb_pipe *p, শূন্য *তথ্য)
{
    ASSERT16();
    যদি (! CONFIG_USB_EHCI)
        ফেরত -1;
    কাঠামো ehci_pipe *pipe = container_of(p, কাঠামো ehci_pipe, pipe);
    কাঠামো ehci_qtd *td = GET_LOWFLAT(pipe->পরবর্তী_td);
    u32 token = GET_LOWFLAT(td->token);
    যদি (token & QTD_STS_ACTIVE)
        // No intrs found.
        ফেরত -1;
    // XXX - check for errors.

    // Copy data.
    পূর্ণসংখ্যা maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    পূর্ণসংখ্যা pos = td - GET_LOWFLAT(pipe->tds);
    শূন্য *tddata = GET_LOWFLAT(pipe->তথ্য) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), তথ্য, SEG_নিচের, LOWFLAT2_নিচের(tddata), maxpacket);

    // Reenable this td.
    কাঠামো ehci_qtd *পরবর্তী = (শূন্য*)(GET_LOWFLAT(td->qtd_পরবর্তী) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->পরবর্তী_td, পরবর্তী);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    ফেরত 0;
}
