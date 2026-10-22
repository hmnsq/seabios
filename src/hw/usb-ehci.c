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

ರಚನೆ usb_ehci_s {
    ರಚನೆ usb_s usb;
    ರಚನೆ ehci_caps *caps;
    ರಚನೆ ehci_regs *regs;
    ರಚನೆ ehci_qh *async_qh;
    ಪೂರ್ಣಾಂಕ checkports;
};

ರಚನೆ ehci_pipe {
    ರಚನೆ ehci_qh qh;
    ರಚನೆ ehci_qtd *ಮುಂದಿನ_td, *tds;
    ಶೂನ್ಯ *ದತ್ತಾಂಶ;
    ರಚನೆ usb_pipe pipe;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು EHCI_ಸಮಯ_POSTPOWER 20
#ವ್ಯಾಖ್ಯಾನಿಸು EHCI_ಸಮಯ_POSTRESET 2

// Check if device attached to port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ehci_hub_detect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_ehci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[ಪೋರ್ಟ್];
    u32 portsc = readl(portreg);

    ಆದರೆ (!(portsc & ಪೋರ್ಟ್_CONNECT))
        // No device present
        ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ ((portsc & ಪೋರ್ಟ್_LINESTATUS_ಮುಖವಾಡ) == ಪೋರ್ಟ್_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | ಪೋರ್ಟ್_OWNER);
        ಹಿಂತಿರುಗಿ -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~ಪೋರ್ಟ್_PE) | ಪೋರ್ಟ್_RESET;
    writel(portreg, portsc);
    msleep(USB_ಸಮಯ_DRSTR);
    ಹಿಂತಿರುಗಿ 1;
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ehci_hub_reset(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_ehci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[ಪೋರ್ಟ್];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~ಪೋರ್ಟ್_RESET;
    writel(portreg, portsc);
    msleep(EHCI_ಸಮಯ_POSTRESET);

    portsc = readl(portreg);
    ಆದರೆ (!(portsc & ಪೋರ್ಟ್_CONNECT))
        // No longer connected
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (!(portsc & ಪೋರ್ಟ್_PE)) {
        // full speed device
        writel(portreg, portsc | ಪೋರ್ಟ್_OWNER);
        ಹಿಂತಿರುಗಿ -1;
    }

    ಹಿಂತಿರುಗಿ USB_HIGHSPEED;
}

// Disable port
ಸ್ಥಿರ ಶೂನ್ಯ
ehci_hub_disconnect(ರಚನೆ usbhub_s *hub, u32 ಪೋರ್ಟ್)
{
    ರಚನೆ usb_ehci_s *cntl = container_ದ(hub->cntl, ರಚನೆ usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[ಪೋರ್ಟ್];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~ಪೋರ್ಟ್_PE);
}

ಸ್ಥಿರ ರಚನೆ usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
check_ehci_ports(ರಚನೆ usb_ehci_s *cntl)
{
    // Power up ports.
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        ಆದರೆ (!(portsc & ಪೋರ್ಟ್_POWER)) {
            portsc |= ಪೋರ್ಟ್_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_ಸಮಯ_POSTPOWER);

    ರಚನೆ usbhub_s hub;
    memset(&hub, 0, ಗಾತ್ರ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    ಹಿಂತಿರುಗಿ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
ಸ್ಥಿರ ಶೂನ್ಯ
ehci_waittick(ರಚನೆ usb_ehci_s *cntl)
{
    ಆದರೆ (MODE16) {
        msleep(10);
        ಹಿಂತಿರುಗಿ;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 ಕೊನೆ = timer_calc(100);
    ಗಾಗಿ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ಆದರೆ (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            ಆದರೆ (!(cmd & CMD_IAAD))
                ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    ಗಾಗಿ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ಆದರೆ (sts & STS_IAA)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ehci_free_pipes(ರಚನೆ usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    ರಚನೆ ehci_qh *ಆರಂಭ = cntl->async_qh;
    ರಚನೆ ehci_qh *pos = ಆರಂಭ;
    ಗಾಗಿ (;;) {
        ರಚನೆ ehci_qh *ಮುಂದಿನ = (ಶೂನ್ಯ*)(pos->ಮುಂದಿನ & ~EHCI_PTR_ಬಿಟ್ಟುಗಳು);
        ಆದರೆ (ಮುಂದಿನ == ಆರಂಭ)
            ನಿಲ್ಲಿಸು;
        ರಚನೆ ehci_pipe *pipe = container_ದ(ಮುಂದಿನ, ರಚನೆ ehci_pipe, qh);
        ಆದರೆ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->ಮುಂದಿನ = ಮುಂದಿನ->ಮುಂದಿನ;
        ಇಲ್ಲದಿದ್ದರೆ
            pos = ಮುಂದಿನ;
    }
    ehci_waittick(cntl);
    ಗಾಗಿ (;;) {
        ರಚನೆ usb_pipe *usbpipe = cntl->usb.freelist;
        ಆದರೆ (!usbpipe)
            ನಿಲ್ಲಿಸು;
        cntl->usb.freelist = usbpipe->freenext;
        ರಚನೆ ehci_pipe *pipe = container_ದ(usbpipe, ರಚನೆ ehci_pipe, pipe);
        ಬಿಡುಗಡೆ(pipe);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
configure_ehci(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ usb_ehci_s *cntl = ದತ್ತಾಂಶ;

    // Allocate ram for schedule storage
    ರಚನೆ ehci_framelist *fl = memalign_ಮೇಲಿನ(ಗಾತ್ರ(*fl), ಗಾತ್ರ(*fl));
    ರಚನೆ ehci_qh *intr_qh = memalign_ಮೇಲಿನ(EHCI_QH_ALIGN, ಗಾತ್ರ(*intr_qh));
    ರಚನೆ ehci_qh *async_qh = memalign_ಮೇಲಿನ(EHCI_QH_ALIGN, ಗಾತ್ರ(*async_qh));
    ಆದರೆ (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        ಹೋಗು fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 ಕೊನೆ = timer_calc(250);
    ಗಾಗಿ (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        ಆದರೆ (!(cmd & CMD_HCRESET))
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            PendingEHCI--;
            ಹೋಗು fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, ಗಾತ್ರ(*intr_qh));
    intr_qh->ಮುಂದಿನ = EHCI_PTR_ಪದ;
    intr_qh->info2 = (0x01 << QH_SMASK_ಸರಿಸು);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_ಮುಂದಿನ = intr_qh->alt_ಮುಂದಿನ = EHCI_PTR_ಪದ;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ARRAY_ಗಾತ್ರ(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, ಗಾತ್ರ(*async_qh));
    async_qh->ಮುಂದಿನ = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_ತಲೆ;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_ಮುಂದಿನ = async_qh->alt_ಮುಂದಿನ = EHCI_PTR_ಪದ;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_ಸತತ);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    ಆದರೆ (ಎಣಿಕೆ_2)
        // Success
        ಹಿಂತಿರುಗಿ;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_ಸತತ);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    ಬಿಡುಗಡೆ(fl);
    ಬಿಡುಗಡೆ(intr_qh);
    ಬಿಡುಗಡೆ(async_qh);
    ಬಿಡುಗಡೆ(cntl);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ehci_controller_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    ರಚನೆ ehci_caps *caps = pci_enable_membar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!caps)
        ಹಿಂತಿರುಗಿ;
    u32 hcc_params = readl(&caps->hccparams);

    ರಚನೆ usb_ehci_s *cntl = malloc_tmphigh(ಗಾತ್ರ(*cntl));
    ಆದರೆ (!cntl) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(cntl, 0, ಗಾತ್ರ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_ಮುಖವಾಡ;
    cntl->regs = (ಶೂನ್ಯ*)caps + readb(&caps->caplength);
    ಆದರೆ (hcc_params & HCC_64_ಬಿಟ್_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    ಸತತ_thread(configure_ehci, cntl);
}

ಶೂನ್ಯ
ehci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_USB_EHCI)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_ಸಿದ್ಧತೆ(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
ಶೂನ್ಯ
ehci_wait_controllers(ಶೂನ್ಯ)
{
    ಆಗ (CONFIG_USB_EHCI && CONFIG_ಎಳೆಗಳು && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
ಸ್ಥಿರ ಶೂನ್ಯ
ehci_desc2pipe(ರಚನೆ ehci_pipe *pipe, ರಚನೆ usbdevice_s *usbdev
               , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_ಸರಿಸು)
                      | (pipe->pipe.ವೇಗ << QH_ವೇಗ_ಸರಿಸು)
                      | (pipe->pipe.ep << QH_EP_ಸರಿಸು)
                      | (pipe->pipe.devaddr << QH_DEVADDR_ಸರಿಸು));

    pipe->qh.info2 = (1 << QH_MULT_ಸರಿಸು);
    ರಚನೆ usbdevice_s *hubdev = usbdev->hub->usbdev;
    ಆದರೆ (hubdev) {
        ರಚನೆ ehci_pipe *hpipe = container_ದ(
            hubdev->defpipe, ರಚನೆ ehci_pipe, pipe);
        ಆದರೆ (hpipe->pipe.ವೇಗ == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->ಪೋರ್ಟ್+1) << QH_HUBPORT_ಸರಿಸು)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_ಸರಿಸು));
        ಇಲ್ಲದಿದ್ದರೆ
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.ವೇಗ != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_ಸರಿಸು) | (0x1c << QH_CMASK_ಸರಿಸು);
}

ಸ್ಥಿರ ರಚನೆ usb_pipe *
ehci_alloc_intr_pipe(ರಚನೆ usbdevice_s *usbdev
                     , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ರಚನೆ usb_ehci_s *cntl = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_ehci_s, usb);
    ಪೂರ್ಣಾಂಕ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ಆದರೆ (frameexp > 10)
        frameexp = 10;
    ಪೂರ್ಣಾಂಕ maxpacket = epdesc->w_Max_Packet_ಗಾತ್ರ;
    // Determine number of entries needed for 2 timer ticks.
    ಪೂರ್ಣಾಂಕ ms = 1<<frameexp;
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = DIV_ಸುತ್ತು_ಮೇಲೆ(ticks_ಗಮ್ಯ_ms(2), ms);
    ರಚನೆ ehci_pipe *pipe = memalign_ಕೆಳಗಿನ(EHCI_QH_ALIGN, ಗಾತ್ರ(*pipe));
    ರಚನೆ ehci_qtd *tds = memalign_ಕೆಳಗಿನ(EHCI_QTD_ALIGN, ಗಾತ್ರ(*tds) * ಎಣಿಕೆ_2);
    ಶೂನ್ಯ *ದತ್ತಾಂಶ = malloc_ಕೆಳಗಿನ(maxpacket * ಎಣಿಕೆ_2);
    ಆದರೆ (!pipe || !tds || !ದತ್ತಾಂಶ) {
        warn_noalloc();
        ಹೋಗು fail;
    }
    memset(pipe, 0, ಗಾತ್ರ(*pipe));
    memset(tds, 0, ಗಾತ್ರ(*tds) * ಎಣಿಕೆ_2);
    memset(ದತ್ತಾಂಶ, 0, maxpacket * ಎಣಿಕೆ_2);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ಮುಂದಿನ_td = pipe->tds = tds;
    pipe->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
    pipe->qh.qtd_ಮುಂದಿನ = (u32)tds;

    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ಎಣಿಕೆ_2; i++) {
        ರಚನೆ ehci_qtd *td = &tds[i];
        td->qtd_ಮುಂದಿನ = (i==ಎಣಿಕೆ_2-1 ? (u32)tds : (u32)&td[1]);
        td->alt_ಮುಂದಿನ = EHCI_PTR_ಪದ;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ಸಕ್ರಿಯ
                     | QTD_PID_ಒಳ | ehci_maxerr(3));
        td->buf[0] = (u32)ದತ್ತಾಂಶ + maxpacket * i;
    }

    // Add to interrupt schedule.
    ರಚನೆ ehci_framelist *fl = (ಶೂನ್ಯ*)readl(&cntl->regs->periodiclistbase);
    ಆದರೆ (frameexp == 0) {
        // Add to existing interrupt entry.
        ರಚನೆ ehci_qh *intr_qh = (ಶೂನ್ಯ*)(fl->links[0] & ~EHCI_PTR_ಬಿಟ್ಟುಗಳು);
        pipe->qh.ಮುಂದಿನ = intr_qh->ಮುಂದಿನ;
        barrier();
        intr_qh->ಮುಂದಿನ = (u32)&pipe->qh | EHCI_PTR_QH;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಪೂರ್ಣಾಂಕ startpos = 1<<(frameexp-1);
        pipe->qh.ಮುಂದಿನ = fl->links[startpos];
        barrier();
        ಗಾಗಿ (i=startpos; i<ARRAY_ಗಾತ್ರ(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    ಹಿಂತಿರುಗಿ &pipe->pipe;
fail:
    ಬಿಡುಗಡೆ(pipe);
    ಬಿಡುಗಡೆ(tds);
    ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ usb_pipe *
ehci_realloc_pipe(ರಚನೆ usbdevice_s *usbdev, ರಚನೆ usb_pipe *upipe
                  , ರಚನೆ usb_endpoint_descriptor *epdesc)
{
    ಆದರೆ (! CONFIG_USB_EHCI)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    usb_ಕೂಡಿಸು_freelist(upipe);
    ಆದರೆ (!epdesc)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    u8 eptype = epdesc->bm_ಗುಣಲಕ್ಷಣಗಳು & USB_ENDPOINT_XFERTYPE_ಮುಖವಾಡ;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_INT)
        ಹಿಂತಿರುಗಿ ehci_alloc_intr_pipe(usbdev, epdesc);
    ರಚನೆ usb_ehci_s *cntl = container_ದ(
        usbdev->hub->cntl, ರಚನೆ usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ರಚನೆ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ಆದರೆ (usbpipe) {
        // Use previously allocated pipe.
        ರಚನೆ ehci_pipe *pipe = container_ದ(usbpipe, ರಚನೆ ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        ಹಿಂತಿರುಗಿ usbpipe;
    }

    // Allocate a new queue head.
    ರಚನೆ ehci_pipe *pipe;
    ಆದರೆ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, ಗಾತ್ರ(*pipe));
    ಇಲ್ಲದಿದ್ದರೆ
        pipe = memalign_ಕೆಳಗಿನ(EHCI_QH_ALIGN, ಗಾತ್ರ(*pipe));
    ಆದರೆ (!pipe) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(pipe, 0, ಗಾತ್ರ(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_ಮುಂದಿನ = pipe->qh.alt_ಮುಂದಿನ = EHCI_PTR_ಪದ;

    // Add queue head to controller list.
    ರಚನೆ ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.ಮುಂದಿನ = async_qh->ಮುಂದಿನ;
    barrier();
    async_qh->ಮುಂದಿನ = (u32)&pipe->qh | EHCI_PTR_QH;
    ಹಿಂತಿರುಗಿ &pipe->pipe;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ehci_reset_pipe(ರಚನೆ ehci_pipe *pipe)
{
    ಗಣ_LOWFLAT(pipe->qh.qtd_ಮುಂದಿನ, EHCI_PTR_ಪದ);
    ಗಣ_LOWFLAT(pipe->qh.alt_ಮುಂದಿನ, EHCI_PTR_ಪದ);
    barrier();
    ಗಣ_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ehci_wait_td(ರಚನೆ ehci_pipe *pipe, ರಚನೆ ehci_qtd *td, u32 ಕೊನೆ)
{
    u32 ಸ್ಥಿತಿ_2;
    ಗಾಗಿ (;;) {
        ಸ್ಥಿತಿ_2 = td->token;
        ಆದರೆ (!(ಸ್ಥಿತಿ_2 & QTD_STS_ಸಕ್ರಿಯ))
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 ಮುಂದಿನ = GET_LOWFLAT(pipe->qh.qtd_ಮುಂದಿನ);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, ಮುಂದಿನ, td, ಸ್ಥಿತಿ_2);
            ehci_reset_pipe(pipe);
            ರಚನೆ usb_ehci_s *cntl = container_ದ(
                GET_LOWFLAT(pipe->pipe.cntl), ರಚನೆ usb_ehci_s, usb);
            ehci_waittick(cntl);
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
    ಆದರೆ (ಸ್ಥಿತಿ_2 & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", ಸ್ಥಿತಿ_2);
        ehci_reset_pipe(pipe);
        ಹಿಂತಿರುಗಿ -2;
    }
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ehci_fill_tdbuf(ರಚನೆ ehci_qtd *td, u32 dest, ಪೂರ್ಣಾಂಕ transfer)
{
    u32 *pos = td->buf, ಕೊನೆ = dest + transfer;
    ಗಾಗಿ (; dest < ಕೊನೆ; dest = ALIGN_ಕೆಳಕ್ಕೆ(dest + ಪುಟ_ಗಾತ್ರ, ಪುಟ_ಗಾತ್ರ))
        *pos++ = dest;
}

#ವ್ಯಾಖ್ಯಾನಿಸು STACKQTDS 6

ಪೂರ್ಣಾಂಕ
ehci_send_pipe(ರಚನೆ usb_pipe *p, ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *cmd
               , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ datasize)
{
    ಆದರೆ (! CONFIG_USB_EHCI)
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ ehci_pipe *pipe = container_ದ(p, ರಚನೆ ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, ದತ್ತಾಂಶ, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[ಗಾತ್ರ(ರಚನೆ ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    ರಚನೆ ehci_qtd *tds = (ಶೂನ್ಯ*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, ಗಾತ್ರ(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    ಆದರೆ (cmd) {
        // Send setup pid on control transfers
        td->qtd_ಮುಂದಿನ = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_ಮುಂದಿನ = EHCI_PTR_ಪದ;
        td->token = (ehci_explen(USB_CONTROL_ಸಿದ್ಧತೆ_ಗಾತ್ರ) | QTD_STS_ಸಕ್ರಿಯ
                     | QTD_PID_ಸಿದ್ಧತೆ | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_ಸಿದ್ಧತೆ_ಗಾತ್ರ);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)ದತ್ತಾಂಶ, dataend = dest + datasize;
    ಆಗ (dest < dataend) {
        // Send data pids
        ಆದರೆ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ -1;
        }
        ಪೂರ್ಣಾಂಕ maxtransfer = 5*ಪುಟ_ಗಾತ್ರ - (dest & (ಪುಟ_ಗಾತ್ರ-1));
        ಪೂರ್ಣಾಂಕ transfer = dataend - dest;
        ಆದರೆ (transfer > maxtransfer)
            transfer = ALIGN_ಕೆಳಕ್ಕೆ(maxtransfer, maxpacket);
        td->qtd_ಮುಂದಿನ = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_ಮುಂದಿನ = EHCI_PTR_ಪದ;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ಸಕ್ರಿಯ
                     | (dir ? QTD_PID_ಒಳ : QTD_PID_ಹೊರಹೋಗು) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    ಆದರೆ (cmd) {
        // Send status pid on control transfers
        ಆದರೆ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ -1;
        }
        td->qtd_ಮುಂದಿನ = EHCI_PTR_ಪದ;
        td->alt_ಮುಂದಿನ = EHCI_PTR_ಪದ;
        td->token = (QTD_TOGGLE | QTD_STS_ಸಕ್ರಿಯ
                     | (dir ? QTD_PID_ಹೊರಹೋಗು : QTD_PID_ಒಳ) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_ಮುಂದಿನ = EHCI_PTR_ಪದ;
    barrier();
    ಗಣ_LOWFLAT(pipe->qh.qtd_ಮುಂದಿನ, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 ಕೊನೆ = timer_calc(usb_xfer_ಸಮಯ(p, datasize));
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0, td=tds; i<STACKQTDS; i++, td++) {
        ಪೂರ್ಣಾಂಕ ret = ehci_wait_td(pipe, td, ಕೊನೆ);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ -1;
    }

    ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ
ehci_poll_intr(ರಚನೆ usb_pipe *p, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ASSERT16();
    ಆದರೆ (! CONFIG_USB_EHCI)
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ ehci_pipe *pipe = container_ದ(p, ರಚನೆ ehci_pipe, pipe);
    ರಚನೆ ehci_qtd *td = GET_LOWFLAT(pipe->ಮುಂದಿನ_td);
    u32 token = GET_LOWFLAT(td->token);
    ಆದರೆ (token & QTD_STS_ಸಕ್ರಿಯ)
        // No intrs found.
        ಹಿಂತಿರುಗಿ -1;
    // XXX - check for errors.

    // Copy data.
    ಪೂರ್ಣಾಂಕ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ಪೂರ್ಣಾಂಕ pos = td - GET_LOWFLAT(pipe->tds);
    ಶೂನ್ಯ *tddata = GET_LOWFLAT(pipe->ದತ್ತಾಂಶ) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), ದತ್ತಾಂಶ, SEG_ಕೆಳಗಿನ, LOWFLAT2_ಕೆಳಗಿನ(tddata), maxpacket);

    // Reenable this td.
    ರಚನೆ ehci_qtd *ಮುಂದಿನ = (ಶೂನ್ಯ*)(GET_LOWFLAT(td->qtd_ಮುಂದಿನ) & ~EHCI_PTR_ಬಿಟ್ಟುಗಳು);
    ಗಣ_LOWFLAT(pipe->ಮುಂದಿನ_td, ಮುಂದಿನ);
    ಗಣ_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    ಗಣ_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ಸಕ್ರಿಯ
                            | QTD_PID_ಒಳ | ehci_maxerr(3)));

    ಹಿಂತಿರುಗಿ 0;
}
