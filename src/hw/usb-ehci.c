// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_LOWFLAT
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "memmap.h" // PAGE_SIZE
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "usb.h" // struct usb_s
#ଅନ୍ତର୍ଭାଵ "usb-ehci.h" // struct ehci_qh
#ଅନ୍ତର୍ଭାଵ "util.h" // msleep
#ଅନ୍ତର୍ଭାଵ "x86.h" // readl

ସଂରଚନା usb_ehci_s {
    ସଂରଚନା usb_s usb;
    ସଂରଚନା ehci_caps *caps;
    ସଂରଚନା ehci_regs *regs;
    ସଂରଚନା ehci_qh *async_qh;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkports;
};

ସଂରଚନା ehci_pipe {
    ସଂରଚନା ehci_qh qh;
    ସଂରଚନା ehci_qtd *next_td, *tds;
    ଶୂନ୍ୟ *data;
    ସଂରଚନା usb_pipe pipe;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ehci_hub_detect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_ehci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    ଯଦି (!(portsc & PORT_CONNECT))
        // No device present
        ଫେରନ୍ତୁ 0;

    ଯଦି ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        ଫେରନ୍ତୁ -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    ଫେରନ୍ତୁ 1;
}

// Reset device on port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ehci_hub_reset(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_ehci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    ଯଦି (!(portsc & PORT_CONNECT))
        // No longer connected
        ଫେରନ୍ତୁ -1;
    ଯଦି (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        ଫେରନ୍ତୁ -1;
    }

    ଫେରନ୍ତୁ USB_HIGHSPEED;
}

// Disable port
ସ୍ଥିର ଶୂନ୍ୟ
ehci_hub_disconnect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_ehci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

ସ୍ଥିର ସଂରଚନା usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
check_ehci_ports(ସଂରଚନା usb_ehci_s *cntl)
{
    // Power up ports.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        ଯଦି (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    ସଂରଚନା usbhub_s hub;
    memset(&hub, 0, ଆକାର(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    ଫେରନ୍ତୁ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
ସ୍ଥିର ଶୂନ୍ୟ
ehci_waittick(ସଂରଚନା usb_ehci_s *cntl)
{
    ଯଦି (MODE16) {
        msleep(10);
        ଫେରନ୍ତୁ;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    ପାଇଁ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ଯଦି (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            ଯଦି (!(cmd & CMD_IAAD))
                ଭାଙ୍ଗ;
        }
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    ପାଇଁ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ଯଦି (sts & STS_IAA)
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

ସ୍ଥିର ଶୂନ୍ୟ
ehci_free_pipes(ସଂରଚନା usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    ସଂରଚନା ehci_qh *start = cntl->async_qh;
    ସଂରଚନା ehci_qh *pos = start;
    ପାଇଁ (;;) {
        ସଂରଚନା ehci_qh *next = (ଶୂନ୍ୟ*)(pos->next & ~EHCI_PTR_BITS);
        ଯଦି (next == start)
            ଭାଙ୍ଗ;
        ସଂରଚନା ehci_pipe *pipe = container_of(next, ସଂରଚନା ehci_pipe, qh);
        ଯଦି (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        ଅନ୍ୟଥା
            pos = next;
    }
    ehci_waittick(cntl);
    ପାଇଁ (;;) {
        ସଂରଚନା usb_pipe *usbpipe = cntl->usb.freelist;
        ଯଦି (!usbpipe)
            ଭାଙ୍ଗ;
        cntl->usb.freelist = usbpipe->freenext;
        ସଂରଚନା ehci_pipe *pipe = container_of(usbpipe, ସଂରଚନା ehci_pipe, pipe);
        ମୁକ୍ତ(pipe);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
configure_ehci(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    ସଂରଚନା ehci_framelist *fl = memalign_ଉପର(ଆକାର(*fl), ଆକାର(*fl));
    ସଂରଚନା ehci_qh *intr_qh = memalign_ଉପର(EHCI_QH_ALIGN, ଆକାର(*intr_qh));
    ସଂରଚନା ehci_qh *async_qh = memalign_ଉପର(EHCI_QH_ALIGN, ଆକାର(*async_qh));
    ଯଦି (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        ଯାଅ fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    ପାଇଁ (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        ଯଦି (!(cmd & CMD_HCRESET))
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            ଯାଅ fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, ଆକାର(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, ଆକାର(*async_qh));
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    ଯଦି (ଗଣନା_2)
        // Success
        ଫେରନ୍ତୁ;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    ମୁକ୍ତ(fl);
    ମୁକ୍ତ(intr_qh);
    ମୁକ୍ତ(async_qh);
    ମୁକ୍ତ(cntl);
}

ସ୍ଥିର ଶୂନ୍ୟ
ehci_controller_setup(ସଂରଚନା pci_device *pci)
{
    ସଂରଚନା ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ଯଦି (!caps)
        ଫେରନ୍ତୁ;
    u32 hcc_params = readl(&caps->hccparams);

    ସଂରଚନା usb_ehci_s *cntl = malloc_tmphigh(ଆକାର(*cntl));
    ଯଦି (!cntl) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(cntl, 0, ଆକାର(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (ଶୂନ୍ୟ*)caps + readb(&caps->caplength);
    ଯଦି (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

ଶୂନ୍ୟ
ehci_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_USB_EHCI)
        ଫେରନ୍ତୁ;
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
ଶୂନ୍ୟ
ehci_wait_controllers(ଶୂନ୍ୟ)
{
    ଯେତେବେଳେ (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
ସ୍ଥିର ଶୂନ୍ୟ
ehci_desc2pipe(ସଂରଚନା ehci_pipe *pipe, ସଂରଚନା usbdevice_s *usbdev
               , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    ସଂରଚନା usbdevice_s *hubdev = usbdev->hub->usbdev;
    ଯଦି (hubdev) {
        ସଂରଚନା ehci_pipe *hpipe = container_of(
            hubdev->defpipe, ସଂରଚନା ehci_pipe, pipe);
        ଯଦି (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        ଅନ୍ୟଥା
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ଯଦି (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    ଅନ୍ୟଥା ଯଦି (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

ସ୍ଥିର ସଂରଚନା usb_pipe *
ehci_alloc_intr_pipe(ସଂରଚନା usbdevice_s *usbdev
                     , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ସଂରଚନା usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_ehci_s, usb);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ଯଦି (frameexp > 10)
        frameexp = 10;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ms = 1<<frameexp;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = DIV_ROUND_UP(ticks_to_ms(2), ms);
    ସଂରଚନା ehci_pipe *pipe = memalign_ତଳ(EHCI_QH_ALIGN, ଆକାର(*pipe));
    ସଂରଚନା ehci_qtd *tds = memalign_ତଳ(EHCI_QTD_ALIGN, ଆକାର(*tds) * ଗଣନା_2);
    ଶୂନ୍ୟ *data = malloc_ତଳ(maxpacket * ଗଣନା_2);
    ଯଦି (!pipe || !tds || !data) {
        warn_noalloc();
        ଯାଅ fail;
    }
    memset(pipe, 0, ଆକାର(*pipe));
    memset(tds, 0, ଆକାର(*tds) * ଗଣନା_2);
    memset(data, 0, maxpacket * ଗଣନା_2);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ଗଣନା_2; i++) {
        ସଂରଚନା ehci_qtd *td = &tds[i];
        td->qtd_next = (i==ଗଣନା_2-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    ସଂରଚନା ehci_framelist *fl = (ଶୂନ୍ୟ*)readl(&cntl->regs->periodiclistbase);
    ଯଦି (frameexp == 0) {
        // Add to existing interrupt entry.
        ସଂରଚନା ehci_qh *intr_qh = (ଶୂନ୍ୟ*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } ଅନ୍ୟଥା {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        ପାଇଁ (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    ଫେରନ୍ତୁ &pipe->pipe;
fail:
    ମୁକ୍ତ(pipe);
    ମୁକ୍ତ(tds);
    ମୁକ୍ତ(data);
    ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା usb_pipe *
ehci_realloc_pipe(ସଂରଚନା usbdevice_s *usbdev, ସଂରଚନା usb_pipe *upipe
                  , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ଯଦି (! CONFIG_USB_EHCI)
        ଫେରନ୍ତୁ ଖାଲି;
    usb_add_freelist(upipe);
    ଯଦି (!epdesc)
        ଫେରନ୍ତୁ ଖାଲି;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ଯଦି (eptype == USB_ENDPOINT_XFER_INT)
        ଫେରନ୍ତୁ ehci_alloc_intr_pipe(usbdev, epdesc);
    ସଂରଚନା usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ସଂରଚନା usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ଯଦି (usbpipe) {
        // Use previously allocated pipe.
        ସଂରଚନା ehci_pipe *pipe = container_of(usbpipe, ସଂରଚନା ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        ଫେରନ୍ତୁ usbpipe;
    }

    // Allocate a new queue head.
    ସଂରଚନା ehci_pipe *pipe;
    ଯଦି (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, ଆକାର(*pipe));
    ଅନ୍ୟଥା
        pipe = memalign_ତଳ(EHCI_QH_ALIGN, ଆକାର(*pipe));
    ଯଦି (!pipe) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(pipe, 0, ଆକାର(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    ସଂରଚନା ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    ଫେରନ୍ତୁ &pipe->pipe;
}

ସ୍ଥିର ଶୂନ୍ୟ
ehci_reset_pipe(ସଂରଚନା ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ehci_wait_td(ସଂରଚନା ehci_pipe *pipe, ସଂରଚନା ehci_qtd *td, u32 end)
{
    u32 status;
    ପାଇଁ (;;) {
        status = td->token;
        ଯଦି (!(status & QTD_STS_ACTIVE))
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            ସଂରଚନା usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), ସଂରଚନା usb_ehci_s, usb);
            ehci_waittick(cntl);
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
    ଯଦି (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        ଫେରନ୍ତୁ -2;
    }
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ
ehci_fill_tdbuf(ସଂରଚନା ehci_qtd *td, u32 dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    ପାଇଁ (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ehci_send_pipe(ସଂରଚନା usb_pipe *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଅଚଳ ଶୂନ୍ୟ *cmd
               , ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datasize)
{
    ଯଦି (! CONFIG_USB_EHCI)
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା ehci_pipe *pipe = container_of(p, ସଂରଚନା ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[ଆକାର(ସଂରଚନା ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    ସଂରଚନା ehci_qtd *tds = (ଶୂନ୍ୟ*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, ଆକାର(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    ଯଦି (cmd) {
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
    ଯେତେବେଳେ (dest < dataend) {
        // Send data pids
        ଯଦି (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ଫେରନ୍ତୁ -1;
        }
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା transfer = dataend - dest;
        ଯଦି (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    ଯଦି (cmd) {
        // Send status pid on control transfers
        ଯଦି (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ଫେରନ୍ତୁ -1;
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0, td=tds; i<STACKQTDS; i++, td++) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = ehci_wait_td(pipe, td, end);
        ଯଦି (ret)
            ଫେରନ୍ତୁ -1;
    }

    ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ehci_poll_intr(ସଂରଚନା usb_pipe *p, ଶୂନ୍ୟ *data)
{
    ASSERT16();
    ଯଦି (! CONFIG_USB_EHCI)
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା ehci_pipe *pipe = container_of(p, ସଂରଚନା ehci_pipe, pipe);
    ସଂରଚନା ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    ଯଦି (token & QTD_STS_ACTIVE)
        // No intrs found.
        ଫେରନ୍ତୁ -1;
    // XXX - check for errors.

    // Copy data.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos = td - GET_LOWFLAT(pipe->tds);
    ଶୂନ୍ୟ *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_ତଳ, LOWFLAT2_ତଳ(tddata), maxpacket);

    // Reenable this td.
    ସଂରଚନା ehci_qtd *next = (ଶୂନ୍ୟ*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    ଫେରନ୍ତୁ 0;
}
