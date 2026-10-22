// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_LOWFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" // PAGE_SIZE
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "usb.h" // struct usb_s
#ਅਨ੍ਤਰ੍ਭਾਵ "usb-ehci.h" // struct ehci_qh
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // msleep
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // readl

ਬਣਤਰ usb_ehci_s {
    ਬਣਤਰ usb_s usb;
    ਬਣਤਰ ehci_caps *caps;
    ਬਣਤਰ ehci_regs *regs;
    ਬਣਤਰ ehci_qh *async_qh;
    ਪੂਰਨ checkports;
};

ਬਣਤਰ ehci_pipe {
    ਬਣਤਰ ehci_qh qh;
    ਬਣਤਰ ehci_qtd *ਅਗਲਾ_td, *tds;
    ਖਾਲੀ *ਅੰਕੜੇ;
    ਬਣਤਰ usb_pipe pipe;
};

ਸਥਿਰ ਪੂਰਨ PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EHCI_ਸਮਾਂ_POSTPOWER 20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EHCI_ਸਮਾਂ_POSTRESET 2

// Check if device attached to port
ਸਥਿਰ ਪੂਰਨ
ehci_hub_detect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_ehci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[ਪੋਰਟ];
    u32 portsc = readl(portreg);

    ਜੇ (!(portsc & ਪੋਰਟ_CONNECT))
        // No device present
        ਵਾਪਸ 0;

    ਜੇ ((portsc & ਪੋਰਟ_LINESTATUS_ਨਕਾਬ) == ਪੋਰਟ_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | ਪੋਰਟ_OWNER);
        ਵਾਪਸ -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~ਪੋਰਟ_PE) | ਪੋਰਟ_RESET;
    writel(portreg, portsc);
    msleep(USB_ਸਮਾਂ_DRSTR);
    ਵਾਪਸ 1;
}

// Reset device on port
ਸਥਿਰ ਪੂਰਨ
ehci_hub_reset(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_ehci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[ਪੋਰਟ];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~ਪੋਰਟ_RESET;
    writel(portreg, portsc);
    msleep(EHCI_ਸਮਾਂ_POSTRESET);

    portsc = readl(portreg);
    ਜੇ (!(portsc & ਪੋਰਟ_CONNECT))
        // No longer connected
        ਵਾਪਸ -1;
    ਜੇ (!(portsc & ਪੋਰਟ_PE)) {
        // full speed device
        writel(portreg, portsc | ਪੋਰਟ_OWNER);
        ਵਾਪਸ -1;
    }

    ਵਾਪਸ USB_HIGHSPEED;
}

// Disable port
ਸਥਿਰ ਖਾਲੀ
ehci_hub_disconnect(ਬਣਤਰ usbhub_s *hub, u32 ਪੋਰਟ)
{
    ਬਣਤਰ usb_ehci_s *cntl = container_ਦਾ(hub->cntl, ਬਣਤਰ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[ਪੋਰਟ];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~ਪੋਰਟ_PE);
}

ਸਥਿਰ ਬਣਤਰ usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
ਸਥਿਰ ਪੂਰਨ
check_ehci_ports(ਬਣਤਰ usb_ehci_s *cntl)
{
    // Power up ports.
    ਪੂਰਨ i;
    ਲਈ (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        ਜੇ (!(portsc & ਪੋਰਟ_POWER)) {
            portsc |= ਪੋਰਟ_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_ਸਮਾਂ_POSTPOWER);

    ਬਣਤਰ usbhub_s hub;
    memset(&hub, 0, ਆਕਾਰ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    ਵਾਪਸ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
ਸਥਿਰ ਖਾਲੀ
ehci_waittick(ਬਣਤਰ usb_ehci_s *cntl)
{
    ਜੇ (MODE16) {
        msleep(10);
        ਵਾਪਸ;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 ਸਿਰਾ = timer_calc(100);
    ਲਈ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ਜੇ (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            ਜੇ (!(cmd & CMD_IAAD))
                ਰੋਕੋ;
        }
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    ਲਈ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ਜੇ (sts & STS_IAA)
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

ਸਥਿਰ ਖਾਲੀ
ehci_free_pipes(ਬਣਤਰ usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    ਬਣਤਰ ehci_qh *ਸ਼ੁਰੂ = cntl->async_qh;
    ਬਣਤਰ ehci_qh *pos = ਸ਼ੁਰੂ;
    ਲਈ (;;) {
        ਬਣਤਰ ehci_qh *ਅਗਲਾ = (ਖਾਲੀ*)(pos->ਅਗਲਾ & ~EHCI_PTR_ਬਿੱਟ);
        ਜੇ (ਅਗਲਾ == ਸ਼ੁਰੂ)
            ਰੋਕੋ;
        ਬਣਤਰ ehci_pipe *pipe = container_ਦਾ(ਅਗਲਾ, ਬਣਤਰ ehci_pipe, qh);
        ਜੇ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->ਅਗਲਾ = ਅਗਲਾ->ਅਗਲਾ;
        ਨਹੀਂਤਾਂ
            pos = ਅਗਲਾ;
    }
    ehci_waittick(cntl);
    ਲਈ (;;) {
        ਬਣਤਰ usb_pipe *usbpipe = cntl->usb.freelist;
        ਜੇ (!usbpipe)
            ਰੋਕੋ;
        cntl->usb.freelist = usbpipe->freenext;
        ਬਣਤਰ ehci_pipe *pipe = container_ਦਾ(usbpipe, ਬਣਤਰ ehci_pipe, pipe);
        ਮੁਕਤ(pipe);
    }
}

ਸਥਿਰ ਖਾਲੀ
configure_ehci(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ usb_ehci_s *cntl = ਅੰਕੜੇ;

    // Allocate ram for schedule storage
    ਬਣਤਰ ehci_framelist *fl = memalign_ਉੱਚਾ(ਆਕਾਰ(*fl), ਆਕਾਰ(*fl));
    ਬਣਤਰ ehci_qh *intr_qh = memalign_ਉੱਚਾ(EHCI_QH_ALIGN, ਆਕਾਰ(*intr_qh));
    ਬਣਤਰ ehci_qh *async_qh = memalign_ਉੱਚਾ(EHCI_QH_ALIGN, ਆਕਾਰ(*async_qh));
    ਜੇ (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        ਜਾਓ fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 ਸਿਰਾ = timer_calc(250);
    ਲਈ (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        ਜੇ (!(cmd & CMD_HCRESET))
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            PendingEHCI--;
            ਜਾਓ fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, ਆਕਾਰ(*intr_qh));
    intr_qh->ਅਗਲਾ = EHCI_PTR_ਪਦ;
    intr_qh->info2 = (0x01 << QH_SMASK_ਖਿਸਕਾਓ);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_ਅਗਲਾ = intr_qh->alt_ਅਗਲਾ = EHCI_PTR_ਪਦ;
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ARRAY_ਮਾਪ(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, ਆਕਾਰ(*async_qh));
    async_qh->ਅਗਲਾ = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_ਸਿਰ;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_ਅਗਲਾ = async_qh->alt_ਅਗਲਾ = EHCI_PTR_ਪਦ;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_ਲੜੀਵਾਰ);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    ਪੂਰਨ ਗਿਣਤੀ_2 = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    ਜੇ (ਗਿਣਤੀ_2)
        // Success
        ਵਾਪਸ;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_ਲੜੀਵਾਰ);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    ਮੁਕਤ(fl);
    ਮੁਕਤ(intr_qh);
    ਮੁਕਤ(async_qh);
    ਮੁਕਤ(cntl);
}

ਸਥਿਰ ਖਾਲੀ
ehci_controller_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    ਬਣਤਰ ehci_caps *caps = pci_enable_membar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!caps)
        ਵਾਪਸ;
    u32 hcc_params = readl(&caps->hccparams);

    ਬਣਤਰ usb_ehci_s *cntl = malloc_tmphigh(ਆਕਾਰ(*cntl));
    ਜੇ (!cntl) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(cntl, 0, ਆਕਾਰ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_ਨਕਾਬ;
    cntl->regs = (ਖਾਲੀ*)caps + readb(&caps->caplength);
    ਜੇ (hcc_params & HCC_64_ਦੋਆਧਾਰੀ_ਅੰਕ_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    ਲੜੀਵਾਰ_thread(configure_ehci, cntl);
}

ਖਾਲੀ
ehci_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_USB_EHCI)
        ਵਾਪਸ;
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_ਤਿਆਰੀ(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
ਖਾਲੀ
ehci_wait_controllers(ਖਾਲੀ)
{
    ਜਦੋਂ (CONFIG_USB_EHCI && CONFIG_ਧਾਗੇ && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
ਸਥਿਰ ਖਾਲੀ
ehci_desc2pipe(ਬਣਤਰ ehci_pipe *pipe, ਬਣਤਰ usbdevice_s *usbdev
               , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_ਖਿਸਕਾਓ)
                      | (pipe->pipe.ਰਫਤਾਰ << QH_ਰਫਤਾਰ_ਖਿਸਕਾਓ)
                      | (pipe->pipe.ep << QH_EP_ਖਿਸਕਾਓ)
                      | (pipe->pipe.devaddr << QH_DEVADDR_ਖਿਸਕਾਓ));

    pipe->qh.info2 = (1 << QH_MULT_ਖਿਸਕਾਓ);
    ਬਣਤਰ usbdevice_s *hubdev = usbdev->hub->usbdev;
    ਜੇ (hubdev) {
        ਬਣਤਰ ehci_pipe *hpipe = container_ਦਾ(
            hubdev->defpipe, ਬਣਤਰ ehci_pipe, pipe);
        ਜੇ (hpipe->pipe.ਰਫਤਾਰ == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->ਪੋਰਟ+1) << QH_HUBPORT_ਖਿਸਕਾਓ)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_ਖਿਸਕਾਓ));
        ਨਹੀਂਤਾਂ
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ;
    ਜੇ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.ਰਫਤਾਰ != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    ਨਹੀਂਤਾਂ ਜੇ (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_ਖਿਸਕਾਓ) | (0x1c << QH_CMASK_ਖਿਸਕਾਓ);
}

ਸਥਿਰ ਬਣਤਰ usb_pipe *
ehci_alloc_intr_pipe(ਬਣਤਰ usbdevice_s *usbdev
                     , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਬਣਤਰ usb_ehci_s *cntl = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_ehci_s, usb);
    ਪੂਰਨ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ਜੇ (frameexp > 10)
        frameexp = 10;
    ਪੂਰਨ maxpacket = epdesc->w_Max_Packet_ਮਾਪ;
    // Determine number of entries needed for 2 timer ticks.
    ਪੂਰਨ ms = 1<<frameexp;
    ਪੂਰਨ ਗਿਣਤੀ_2 = DIV_ਗੇੜਾ_ਉੱਪਰ(ticks_ਮੰਜ਼ਿਲ_ms(2), ms);
    ਬਣਤਰ ehci_pipe *pipe = memalign_ਨੀਵਾਂ(EHCI_QH_ALIGN, ਆਕਾਰ(*pipe));
    ਬਣਤਰ ehci_qtd *tds = memalign_ਨੀਵਾਂ(EHCI_QTD_ALIGN, ਆਕਾਰ(*tds) * ਗਿਣਤੀ_2);
    ਖਾਲੀ *ਅੰਕੜੇ = malloc_ਨੀਵਾਂ(maxpacket * ਗਿਣਤੀ_2);
    ਜੇ (!pipe || !tds || !ਅੰਕੜੇ) {
        warn_noalloc();
        ਜਾਓ fail;
    }
    memset(pipe, 0, ਆਕਾਰ(*pipe));
    memset(tds, 0, ਆਕਾਰ(*tds) * ਗਿਣਤੀ_2);
    memset(ਅੰਕੜੇ, 0, maxpacket * ਗਿਣਤੀ_2);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ਅਗਲਾ_td = pipe->tds = tds;
    pipe->ਅੰਕੜੇ = ਅੰਕੜੇ;
    pipe->qh.qtd_ਅਗਲਾ = (u32)tds;

    ਪੂਰਨ i;
    ਲਈ (i=0; i<ਗਿਣਤੀ_2; i++) {
        ਬਣਤਰ ehci_qtd *td = &tds[i];
        td->qtd_ਅਗਲਾ = (i==ਗਿਣਤੀ_2-1 ? (u32)tds : (u32)&td[1]);
        td->alt_ਅਗਲਾ = EHCI_PTR_ਪਦ;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ਸਰਗਰਮ
                     | QTD_PID_ਵਿੱਚ | ehci_maxerr(3));
        td->buf[0] = (u32)ਅੰਕੜੇ + maxpacket * i;
    }

    // Add to interrupt schedule.
    ਬਣਤਰ ehci_framelist *fl = (ਖਾਲੀ*)readl(&cntl->regs->periodiclistbase);
    ਜੇ (frameexp == 0) {
        // Add to existing interrupt entry.
        ਬਣਤਰ ehci_qh *intr_qh = (ਖਾਲੀ*)(fl->links[0] & ~EHCI_PTR_ਬਿੱਟ);
        pipe->qh.ਅਗਲਾ = intr_qh->ਅਗਲਾ;
        barrier();
        intr_qh->ਅਗਲਾ = (u32)&pipe->qh | EHCI_PTR_QH;
    } ਨਹੀਂਤਾਂ {
        ਪੂਰਨ startpos = 1<<(frameexp-1);
        pipe->qh.ਅਗਲਾ = fl->links[startpos];
        barrier();
        ਲਈ (i=startpos; i<ARRAY_ਮਾਪ(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    ਵਾਪਸ &pipe->pipe;
fail:
    ਮੁਕਤ(pipe);
    ਮੁਕਤ(tds);
    ਮੁਕਤ(ਅੰਕੜੇ);
    ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ usb_pipe *
ehci_realloc_pipe(ਬਣਤਰ usbdevice_s *usbdev, ਬਣਤਰ usb_pipe *upipe
                  , ਬਣਤਰ usb_endpoint_descriptor *epdesc)
{
    ਜੇ (! CONFIG_USB_EHCI)
        ਵਾਪਸ ਨਿਰਾ;
    usb_ਜੋੜੋ_freelist(upipe);
    ਜੇ (!epdesc)
        ਵਾਪਸ ਨਿਰਾ;
    u8 eptype = epdesc->bm_ਗੁਣ & USB_ENDPOINT_XFERTYPE_ਨਕਾਬ;
    ਜੇ (eptype == USB_ENDPOINT_XFER_INT)
        ਵਾਪਸ ehci_alloc_intr_pipe(usbdev, epdesc);
    ਬਣਤਰ usb_ehci_s *cntl = container_ਦਾ(
        usbdev->hub->cntl, ਬਣਤਰ usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ਬਣਤਰ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ਜੇ (usbpipe) {
        // Use previously allocated pipe.
        ਬਣਤਰ ehci_pipe *pipe = container_ਦਾ(usbpipe, ਬਣਤਰ ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        ਵਾਪਸ usbpipe;
    }

    // Allocate a new queue head.
    ਬਣਤਰ ehci_pipe *pipe;
    ਜੇ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, ਆਕਾਰ(*pipe));
    ਨਹੀਂਤਾਂ
        pipe = memalign_ਨੀਵਾਂ(EHCI_QH_ALIGN, ਆਕਾਰ(*pipe));
    ਜੇ (!pipe) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(pipe, 0, ਆਕਾਰ(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_ਅਗਲਾ = pipe->qh.alt_ਅਗਲਾ = EHCI_PTR_ਪਦ;

    // Add queue head to controller list.
    ਬਣਤਰ ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.ਅਗਲਾ = async_qh->ਅਗਲਾ;
    barrier();
    async_qh->ਅਗਲਾ = (u32)&pipe->qh | EHCI_PTR_QH;
    ਵਾਪਸ &pipe->pipe;
}

ਸਥਿਰ ਖਾਲੀ
ehci_reset_pipe(ਬਣਤਰ ehci_pipe *pipe)
{
    ਸੈੱਟ_LOWFLAT(pipe->qh.qtd_ਅਗਲਾ, EHCI_PTR_ਪਦ);
    ਸੈੱਟ_LOWFLAT(pipe->qh.alt_ਅਗਲਾ, EHCI_PTR_ਪਦ);
    barrier();
    ਸੈੱਟ_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ਸਥਿਰ ਪੂਰਨ
ehci_wait_td(ਬਣਤਰ ehci_pipe *pipe, ਬਣਤਰ ehci_qtd *td, u32 ਸਿਰਾ)
{
    u32 ਸਥਿਤੀ;
    ਲਈ (;;) {
        ਸਥਿਤੀ = td->token;
        ਜੇ (!(ਸਥਿਤੀ & QTD_STS_ਸਰਗਰਮ))
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 ਅਗਲਾ = GET_LOWFLAT(pipe->qh.qtd_ਅਗਲਾ);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, ਅਗਲਾ, td, ਸਥਿਤੀ);
            ehci_reset_pipe(pipe);
            ਬਣਤਰ usb_ehci_s *cntl = container_ਦਾ(
                GET_LOWFLAT(pipe->pipe.cntl), ਬਣਤਰ usb_ehci_s, usb);
            ehci_waittick(cntl);
            ਵਾਪਸ -1;
        }
        yield();
    }
    ਜੇ (ਸਥਿਤੀ & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", ਸਥਿਤੀ);
        ehci_reset_pipe(pipe);
        ਵਾਪਸ -2;
    }
    ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ
ehci_fill_tdbuf(ਬਣਤਰ ehci_qtd *td, u32 dest, ਪੂਰਨ transfer)
{
    u32 *pos = td->buf, ਸਿਰਾ = dest + transfer;
    ਲਈ (; dest < ਸਿਰਾ; dest = ALIGN_ਥੱਲੇ(dest + ਪੰਨਾ_ਮਾਪ, ਪੰਨਾ_ਮਾਪ))
        *pos++ = dest;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STACKQTDS 6

ਪੂਰਨ
ehci_send_pipe(ਬਣਤਰ usb_pipe *p, ਪੂਰਨ dir, ਅਚਲ ਖਾਲੀ *cmd
               , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ datasize)
{
    ਜੇ (! CONFIG_USB_EHCI)
        ਵਾਪਸ -1;
    ਬਣਤਰ ehci_pipe *pipe = container_ਦਾ(p, ਬਣਤਰ ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, ਅੰਕੜੇ, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[ਆਕਾਰ(ਬਣਤਰ ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    ਬਣਤਰ ehci_qtd *tds = (ਖਾਲੀ*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, ਆਕਾਰ(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    ਜੇ (cmd) {
        // Send setup pid on control transfers
        td->qtd_ਅਗਲਾ = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_ਅਗਲਾ = EHCI_PTR_ਪਦ;
        td->token = (ehci_explen(USB_CONTROL_ਤਿਆਰੀ_ਮਾਪ) | QTD_STS_ਸਰਗਰਮ
                     | QTD_PID_ਤਿਆਰੀ | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_ਤਿਆਰੀ_ਮਾਪ);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)ਅੰਕੜੇ, dataend = dest + datasize;
    ਜਦੋਂ (dest < dataend) {
        // Send data pids
        ਜੇ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ਵਾਪਸ -1;
        }
        ਪੂਰਨ maxtransfer = 5*ਪੰਨਾ_ਮਾਪ - (dest & (ਪੰਨਾ_ਮਾਪ-1));
        ਪੂਰਨ transfer = dataend - dest;
        ਜੇ (transfer > maxtransfer)
            transfer = ALIGN_ਥੱਲੇ(maxtransfer, maxpacket);
        td->qtd_ਅਗਲਾ = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_ਅਗਲਾ = EHCI_PTR_ਪਦ;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ਸਰਗਰਮ
                     | (dir ? QTD_PID_ਵਿੱਚ : QTD_PID_ਬਾਹਰਲਾ) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    ਜੇ (cmd) {
        // Send status pid on control transfers
        ਜੇ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ਵਾਪਸ -1;
        }
        td->qtd_ਅਗਲਾ = EHCI_PTR_ਪਦ;
        td->alt_ਅਗਲਾ = EHCI_PTR_ਪਦ;
        td->token = (QTD_TOGGLE | QTD_STS_ਸਰਗਰਮ
                     | (dir ? QTD_PID_ਬਾਹਰਲਾ : QTD_PID_ਵਿੱਚ) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_ਅਗਲਾ = EHCI_PTR_ਪਦ;
    barrier();
    ਸੈੱਟ_LOWFLAT(pipe->qh.qtd_ਅਗਲਾ, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 ਸਿਰਾ = timer_calc(usb_xfer_ਸਮਾਂ(p, datasize));
    ਪੂਰਨ i;
    ਲਈ (i=0, td=tds; i<STACKQTDS; i++, td++) {
        ਪੂਰਨ ret = ehci_wait_td(pipe, td, ਸਿਰਾ);
        ਜੇ (ret)
            ਵਾਪਸ -1;
    }

    ਵਾਪਸ 0;
}

ਪੂਰਨ
ehci_poll_intr(ਬਣਤਰ usb_pipe *p, ਖਾਲੀ *ਅੰਕੜੇ)
{
    ASSERT16();
    ਜੇ (! CONFIG_USB_EHCI)
        ਵਾਪਸ -1;
    ਬਣਤਰ ehci_pipe *pipe = container_ਦਾ(p, ਬਣਤਰ ehci_pipe, pipe);
    ਬਣਤਰ ehci_qtd *td = GET_LOWFLAT(pipe->ਅਗਲਾ_td);
    u32 token = GET_LOWFLAT(td->token);
    ਜੇ (token & QTD_STS_ਸਰਗਰਮ)
        // No intrs found.
        ਵਾਪਸ -1;
    // XXX - check for errors.

    // Copy data.
    ਪੂਰਨ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ਪੂਰਨ pos = td - GET_LOWFLAT(pipe->tds);
    ਖਾਲੀ *tddata = GET_LOWFLAT(pipe->ਅੰਕੜੇ) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), ਅੰਕੜੇ, SEG_ਨੀਵਾਂ, LOWFLAT2_ਨੀਵਾਂ(tddata), maxpacket);

    // Reenable this td.
    ਬਣਤਰ ehci_qtd *ਅਗਲਾ = (ਖਾਲੀ*)(GET_LOWFLAT(td->qtd_ਅਗਲਾ) & ~EHCI_PTR_ਬਿੱਟ);
    ਸੈੱਟ_LOWFLAT(pipe->ਅਗਲਾ_td, ਅਗਲਾ);
    ਸੈੱਟ_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    ਸੈੱਟ_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ਸਰਗਰਮ
                            | QTD_PID_ਵਿੱਚ | ehci_maxerr(3)));

    ਵਾਪਸ 0;
}
