// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOWFLAT
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-ehci.h" // struct ehci_qh
#ಅನ್ತರ್ಭಾವ "util.h" // msleep
#ಅನ್ತರ್ಭಾವ "x86.h" // readl

ಸಂರಚನಾ usb_ehci_s {
    ಸಂರಚನಾ usb_s usb;
    ಸಂರಚನಾ ehci_caps *caps;
    ಸಂರಚನಾ ehci_regs *regs;
    ಸಂರಚನಾ ehci_qh *async_qh;
    ಪೂರ್ಣಾಙ್ಕ checkports;
};

ಸಂರಚನಾ ehci_pipe {
    ಸಂರಚನಾ ehci_qh qh;
    ಸಂರಚನಾ ehci_qtd *next_td, *tds;
    ಶೂನ್ಯ *data;
    ಸಂರಚನಾ usb_pipe pipe;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ehci_hub_detect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_ehci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    ಯದಿ (!(portsc & PORT_CONNECT))
        // No device present
        ಪ್ರತ್ಯಾಗಮ 0;

    ಯದಿ ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    ಪ್ರತ್ಯಾಗಮ 1;
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ehci_hub_reset(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_ehci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    ಯದಿ (!(portsc & PORT_CONNECT))
        // No longer connected
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಪ್ರತ್ಯಾಗಮ USB_HIGHSPEED;
}

// Disable port
ಸ್ಥಿರ ಶೂನ್ಯ
ehci_hub_disconnect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_ehci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

ಸ್ಥಿರ ಸಂರಚನಾ usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
check_ehci_ports(ಸಂರಚನಾ usb_ehci_s *cntl)
{
    // Power up ports.
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        ಯದಿ (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    ಸಂರಚನಾ usbhub_s hub;
    memset(&hub, 0, ಪರಿಮಾಣ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    ಪ್ರತ್ಯಾಗಮ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
ಸ್ಥಿರ ಶೂನ್ಯ
ehci_waittick(ಸಂರಚನಾ usb_ehci_s *cntl)
{
    ಯದಿ (MODE16) {
        msleep(10);
        ಪ್ರತ್ಯಾಗಮ;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    ಪ್ರತಿ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ಯದಿ (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            ಯದಿ (!(cmd & CMD_IAAD))
                ಭಙ್ಗ;
        }
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    ಪ್ರತಿ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ಯದಿ (sts & STS_IAA)
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ehci_free_pipes(ಸಂರಚನಾ usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    ಸಂರಚನಾ ehci_qh *start = cntl->async_qh;
    ಸಂರಚನಾ ehci_qh *pos = start;
    ಪ್ರತಿ (;;) {
        ಸಂರಚನಾ ehci_qh *next = (ಶೂನ್ಯ*)(pos->next & ~EHCI_PTR_BITS);
        ಯದಿ (next == start)
            ಭಙ್ಗ;
        ಸಂರಚನಾ ehci_pipe *pipe = container_of(next, ಸಂರಚನಾ ehci_pipe, qh);
        ಯದಿ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        ಅನ್ಯಥಾ
            pos = next;
    }
    ehci_waittick(cntl);
    ಪ್ರತಿ (;;) {
        ಸಂರಚನಾ usb_pipe *usbpipe = cntl->usb.freelist;
        ಯದಿ (!usbpipe)
            ಭಙ್ಗ;
        cntl->usb.freelist = usbpipe->freenext;
        ಸಂರಚನಾ ehci_pipe *pipe = container_of(usbpipe, ಸಂರಚನಾ ehci_pipe, pipe);
        ಮುಕ್ತ(pipe);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
configure_ehci(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    ಸಂರಚನಾ ehci_framelist *fl = memalign_ಮೇಲಿನ(ಪರಿಮಾಣ(*fl), ಪರಿಮಾಣ(*fl));
    ಸಂರಚನಾ ehci_qh *intr_qh = memalign_ಮೇಲಿನ(EHCI_QH_ALIGN, ಪರಿಮಾಣ(*intr_qh));
    ಸಂರಚನಾ ehci_qh *async_qh = memalign_ಮೇಲಿನ(EHCI_QH_ALIGN, ಪರಿಮಾಣ(*async_qh));
    ಯದಿ (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        ಗಚ್ಛ fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    ಪ್ರತಿ (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        ಯದಿ (!(cmd & CMD_HCRESET))
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            ಗಚ್ಛ fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, ಪರಿಮಾಣ(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, ಪರಿಮಾಣ(*async_qh));
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
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    ಯದಿ (ಎಣಿಕೆ)
        // Success
        ಪ್ರತ್ಯಾಗಮ;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    ಮುಕ್ತ(fl);
    ಮುಕ್ತ(intr_qh);
    ಮುಕ್ತ(async_qh);
    ಮುಕ್ತ(cntl);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ehci_controller_setup(ಸಂರಚನಾ pci_device *pci)
{
    ಸಂರಚನಾ ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ಯದಿ (!caps)
        ಪ್ರತ್ಯಾಗಮ;
    u32 hcc_params = readl(&caps->hccparams);

    ಸಂರಚನಾ usb_ehci_s *cntl = malloc_tmphigh(ಪರಿಮಾಣ(*cntl));
    ಯದಿ (!cntl) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(cntl, 0, ಪರಿಮಾಣ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (ಶೂನ್ಯ*)caps + readb(&caps->caplength);
    ಯದಿ (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

ಶೂನ್ಯ
ehci_setup(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_USB_EHCI)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
ಶೂನ್ಯ
ehci_wait_controllers(ಶೂನ್ಯ)
{
    ಯಾವತ್ (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
ಸ್ಥಿರ ಶೂನ್ಯ
ehci_desc2pipe(ಸಂರಚನಾ ehci_pipe *pipe, ಸಂರಚನಾ usbdevice_s *usbdev
               , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    ಸಂರಚನಾ usbdevice_s *hubdev = usbdev->hub->usbdev;
    ಯದಿ (hubdev) {
        ಸಂರಚನಾ ehci_pipe *hpipe = container_of(
            hubdev->defpipe, ಸಂರಚನಾ ehci_pipe, pipe);
        ಯದಿ (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        ಅನ್ಯಥಾ
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    ಅನ್ಯಥಾ ಯದಿ (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

ಸ್ಥಿರ ಸಂರಚನಾ usb_pipe *
ehci_alloc_intr_pipe(ಸಂರಚನಾ usbdevice_s *usbdev
                     , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಸಂರಚನಾ usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_ehci_s, usb);
    ಪೂರ್ಣಾಙ್ಕ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ಯದಿ (frameexp > 10)
        frameexp = 10;
    ಪೂರ್ಣಾಙ್ಕ maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    ಪೂರ್ಣಾಙ್ಕ ms = 1<<frameexp;
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = DIV_ROUND_UP(ticks_to_ms(2), ms);
    ಸಂರಚನಾ ehci_pipe *pipe = memalign_ಕೆಳಗಿನ(EHCI_QH_ALIGN, ಪರಿಮಾಣ(*pipe));
    ಸಂರಚನಾ ehci_qtd *tds = memalign_ಕೆಳಗಿನ(EHCI_QTD_ALIGN, ಪರಿಮಾಣ(*tds) * ಎಣಿಕೆ);
    ಶೂನ್ಯ *data = malloc_ಕೆಳಗಿನ(maxpacket * ಎಣಿಕೆ);
    ಯದಿ (!pipe || !tds || !data) {
        warn_noalloc();
        ಗಚ್ಛ fail;
    }
    memset(pipe, 0, ಪರಿಮಾಣ(*pipe));
    memset(tds, 0, ಪರಿಮಾಣ(*tds) * ಎಣಿಕೆ);
    memset(data, 0, maxpacket * ಎಣಿಕೆ);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ಎಣಿಕೆ; i++) {
        ಸಂರಚನಾ ehci_qtd *td = &tds[i];
        td->qtd_next = (i==ಎಣಿಕೆ-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    ಸಂರಚನಾ ehci_framelist *fl = (ಶೂನ್ಯ*)readl(&cntl->regs->periodiclistbase);
    ಯದಿ (frameexp == 0) {
        // Add to existing interrupt entry.
        ಸಂರಚನಾ ehci_qh *intr_qh = (ಶೂನ್ಯ*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } ಅನ್ಯಥಾ {
        ಪೂರ್ಣಾಙ್ಕ startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        ಪ್ರತಿ (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    ಪ್ರತ್ಯಾಗಮ &pipe->pipe;
fail:
    ಮುಕ್ತ(pipe);
    ಮುಕ್ತ(tds);
    ಮುಕ್ತ(data);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ usb_pipe *
ehci_realloc_pipe(ಸಂರಚನಾ usbdevice_s *usbdev, ಸಂರಚನಾ usb_pipe *upipe
                  , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಯದಿ (! CONFIG_USB_EHCI)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    usb_add_freelist(upipe);
    ಯದಿ (!epdesc)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_INT)
        ಪ್ರತ್ಯಾಗಮ ehci_alloc_intr_pipe(usbdev, epdesc);
    ಸಂರಚನಾ usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ಸಂರಚನಾ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ಯದಿ (usbpipe) {
        // Use previously allocated pipe.
        ಸಂರಚನಾ ehci_pipe *pipe = container_of(usbpipe, ಸಂರಚನಾ ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        ಪ್ರತ್ಯಾಗಮ usbpipe;
    }

    // Allocate a new queue head.
    ಸಂರಚನಾ ehci_pipe *pipe;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, ಪರಿಮಾಣ(*pipe));
    ಅನ್ಯಥಾ
        pipe = memalign_ಕೆಳಗಿನ(EHCI_QH_ALIGN, ಪರಿಮಾಣ(*pipe));
    ಯದಿ (!pipe) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(pipe, 0, ಪರಿಮಾಣ(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    ಸಂರಚನಾ ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    ಪ್ರತ್ಯಾಗಮ &pipe->pipe;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ehci_reset_pipe(ಸಂರಚನಾ ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ehci_wait_td(ಸಂರಚನಾ ehci_pipe *pipe, ಸಂರಚನಾ ehci_qtd *td, u32 end)
{
    u32 status;
    ಪ್ರತಿ (;;) {
        status = td->token;
        ಯದಿ (!(status & QTD_STS_ACTIVE))
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            ಸಂರಚನಾ usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), ಸಂರಚನಾ usb_ehci_s, usb);
            ehci_waittick(cntl);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
    ಯದಿ (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        ಪ್ರತ್ಯಾಗಮ -2;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ehci_fill_tdbuf(ಸಂರಚನಾ ehci_qtd *td, u32 dest, ಪೂರ್ಣಾಙ್ಕ transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    ಪ್ರತಿ (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

ಪೂರ್ಣಾಙ್ಕ
ehci_send_pipe(ಸಂರಚನಾ usb_pipe *p, ಪೂರ್ಣಾಙ್ಕ dir, ಧ್ರುವ ಶೂನ್ಯ *cmd
               , ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datasize)
{
    ಯದಿ (! CONFIG_USB_EHCI)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ ehci_pipe *pipe = container_of(p, ಸಂರಚನಾ ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[ಪರಿಮಾಣ(ಸಂರಚನಾ ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    ಸಂರಚನಾ ehci_qtd *tds = (ಶೂನ್ಯ*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, ಪರಿಮಾಣ(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    ಯದಿ (cmd) {
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
    ಯಾವತ್ (dest < dataend) {
        // Send data pids
        ಯದಿ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        ಪೂರ್ಣಾಙ್ಕ maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        ಪೂರ್ಣಾಙ್ಕ transfer = dataend - dest;
        ಯದಿ (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    ಯದಿ (cmd) {
        // Send status pid on control transfers
        ಯದಿ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ -1;
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
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0, td=tds; i<STACKQTDS; i++, td++) {
        ಪೂರ್ಣಾಙ್ಕ ret = ehci_wait_td(pipe, td, end);
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ
ehci_poll_intr(ಸಂರಚನಾ usb_pipe *p, ಶೂನ್ಯ *data)
{
    ASSERT16();
    ಯದಿ (! CONFIG_USB_EHCI)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ ehci_pipe *pipe = container_of(p, ಸಂರಚನಾ ehci_pipe, pipe);
    ಸಂರಚನಾ ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    ಯದಿ (token & QTD_STS_ACTIVE)
        // No intrs found.
        ಪ್ರತ್ಯಾಗಮ -1;
    // XXX - check for errors.

    // Copy data.
    ಪೂರ್ಣಾಙ್ಕ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ಪೂರ್ಣಾಙ್ಕ pos = td - GET_LOWFLAT(pipe->tds);
    ಶೂನ್ಯ *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_ಕೆಳಗಿನ, LOWFLAT2_ಕೆಳಗಿನ(tddata), maxpacket);

    // Reenable this td.
    ಸಂರಚನಾ ehci_qtd *next = (ಶೂನ್ಯ*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    ಪ್ರತ್ಯಾಗಮ 0;
}
