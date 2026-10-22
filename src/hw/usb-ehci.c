// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_LOWFLAT
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "memmap.h" // PAGE_SIZE
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#അന്തര്ഭാവ "pci_regs.h" // PCI_BASE_ADDRESS_0
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "usb.h" // struct usb_s
#അന്തര്ഭാവ "usb-ehci.h" // struct ehci_qh
#അന്തര്ഭാവ "util.h" // msleep
#അന്തര്ഭാവ "x86.h" // readl

ഘടന usb_ehci_s {
    ഘടന usb_s usb;
    ഘടന ehci_caps *caps;
    ഘടന ehci_regs *regs;
    ഘടന ehci_qh *async_qh;
    പൂർണ്ണസംഖ്യ checkports;
};

ഘടന ehci_pipe {
    ഘടന ehci_qh qh;
    ഘടന ehci_qtd *next_td, *tds;
    ശൂന്യം *data;
    ഘടന usb_pipe pipe;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
സ്ഥിതം പൂർണ്ണസംഖ്യ
ehci_hub_detect(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_ehci_s *cntl = container_of(hub->cntl, ഘടന usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    എങ്കിൽ (!(portsc & PORT_CONNECT))
        // No device present
        തിരികെ 0;

    എങ്കിൽ ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        തിരികെ -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    തിരികെ 1;
}

// Reset device on port
സ്ഥിതം പൂർണ്ണസംഖ്യ
ehci_hub_reset(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_ehci_s *cntl = container_of(hub->cntl, ഘടന usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    എങ്കിൽ (!(portsc & PORT_CONNECT))
        // No longer connected
        തിരികെ -1;
    എങ്കിൽ (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        തിരികെ -1;
    }

    തിരികെ USB_HIGHSPEED;
}

// Disable port
സ്ഥിതം ശൂന്യം
ehci_hub_disconnect(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_ehci_s *cntl = container_of(hub->cntl, ഘടന usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

സ്ഥിതം ഘടന usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
സ്ഥിതം പൂർണ്ണസംഖ്യ
check_ehci_ports(ഘടന usb_ehci_s *cntl)
{
    // Power up ports.
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        എങ്കിൽ (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    ഘടന usbhub_s hub;
    memset(&hub, 0, വലുപ്പം(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    തിരികെ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
സ്ഥിതം ശൂന്യം
ehci_waittick(ഘടന usb_ehci_s *cntl)
{
    എങ്കിൽ (MODE16) {
        msleep(10);
        തിരികെ;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    വേണ്ടി (;;) {
        sts = readl(&cntl->regs->usbsts);
        എങ്കിൽ (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            എങ്കിൽ (!(cmd & CMD_IAAD))
                നിർത്തുക;
        }
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    വേണ്ടി (;;) {
        sts = readl(&cntl->regs->usbsts);
        എങ്കിൽ (sts & STS_IAA)
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

സ്ഥിതം ശൂന്യം
ehci_free_pipes(ഘടന usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    ഘടന ehci_qh *start = cntl->async_qh;
    ഘടന ehci_qh *pos = start;
    വേണ്ടി (;;) {
        ഘടന ehci_qh *next = (ശൂന്യം*)(pos->next & ~EHCI_PTR_BITS);
        എങ്കിൽ (next == start)
            നിർത്തുക;
        ഘടന ehci_pipe *pipe = container_of(next, ഘടന ehci_pipe, qh);
        എങ്കിൽ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        അല്ലെങ്കിൽ
            pos = next;
    }
    ehci_waittick(cntl);
    വേണ്ടി (;;) {
        ഘടന usb_pipe *usbpipe = cntl->usb.freelist;
        എങ്കിൽ (!usbpipe)
            നിർത്തുക;
        cntl->usb.freelist = usbpipe->freenext;
        ഘടന ehci_pipe *pipe = container_of(usbpipe, ഘടന ehci_pipe, pipe);
        സ്വതന്ത്രം(pipe);
    }
}

സ്ഥിതം ശൂന്യം
configure_ehci(ശൂന്യം *data)
{
    ഘടന usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    ഘടന ehci_framelist *fl = memalign_മുകളിലെ(വലുപ്പം(*fl), വലുപ്പം(*fl));
    ഘടന ehci_qh *intr_qh = memalign_മുകളിലെ(EHCI_QH_ALIGN, വലുപ്പം(*intr_qh));
    ഘടന ehci_qh *async_qh = memalign_മുകളിലെ(EHCI_QH_ALIGN, വലുപ്പം(*async_qh));
    എങ്കിൽ (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        പോകുക fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    വേണ്ടി (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        എങ്കിൽ (!(cmd & CMD_HCRESET))
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            പോകുക fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, വലുപ്പം(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, വലുപ്പം(*async_qh));
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
    പൂർണ്ണസംഖ്യ എണ്ണം = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    എങ്കിൽ (എണ്ണം)
        // Success
        തിരികെ;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    സ്വതന്ത്രം(fl);
    സ്വതന്ത്രം(intr_qh);
    സ്വതന്ത്രം(async_qh);
    സ്വതന്ത്രം(cntl);
}

സ്ഥിതം ശൂന്യം
ehci_controller_setup(ഘടന pci_device *pci)
{
    ഘടന ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    എങ്കിൽ (!caps)
        തിരികെ;
    u32 hcc_params = readl(&caps->hccparams);

    ഘടന usb_ehci_s *cntl = malloc_tmphigh(വലുപ്പം(*cntl));
    എങ്കിൽ (!cntl) {
        warn_noalloc();
        തിരികെ;
    }
    memset(cntl, 0, വലുപ്പം(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (ശൂന്യം*)caps + readb(&caps->caplength);
    എങ്കിൽ (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

ശൂന്യം
ehci_setup(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_USB_EHCI)
        തിരികെ;
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
ശൂന്യം
ehci_wait_controllers(ശൂന്യം)
{
    ആകുമ്പോൾ (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
സ്ഥിതം ശൂന്യം
ehci_desc2pipe(ഘടന ehci_pipe *pipe, ഘടന usbdevice_s *usbdev
               , ഘടന usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    ഘടന usbdevice_s *hubdev = usbdev->hub->usbdev;
    എങ്കിൽ (hubdev) {
        ഘടന ehci_pipe *hpipe = container_of(
            hubdev->defpipe, ഘടന ehci_pipe, pipe);
        എങ്കിൽ (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        അല്ലെങ്കിൽ
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    അല്ലെങ്കിൽ എങ്കിൽ (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

സ്ഥിതം ഘടന usb_pipe *
ehci_alloc_intr_pipe(ഘടന usbdevice_s *usbdev
                     , ഘടന usb_endpoint_descriptor *epdesc)
{
    ഘടന usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, ഘടന usb_ehci_s, usb);
    പൂർണ്ണസംഖ്യ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    എങ്കിൽ (frameexp > 10)
        frameexp = 10;
    പൂർണ്ണസംഖ്യ maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    പൂർണ്ണസംഖ്യ ms = 1<<frameexp;
    പൂർണ്ണസംഖ്യ എണ്ണം = DIV_ROUND_UP(ticks_to_ms(2), ms);
    ഘടന ehci_pipe *pipe = memalign_താഴെയുള്ള(EHCI_QH_ALIGN, വലുപ്പം(*pipe));
    ഘടന ehci_qtd *tds = memalign_താഴെയുള്ള(EHCI_QTD_ALIGN, വലുപ്പം(*tds) * എണ്ണം);
    ശൂന്യം *data = malloc_താഴെയുള്ള(maxpacket * എണ്ണം);
    എങ്കിൽ (!pipe || !tds || !data) {
        warn_noalloc();
        പോകുക fail;
    }
    memset(pipe, 0, വലുപ്പം(*pipe));
    memset(tds, 0, വലുപ്പം(*tds) * എണ്ണം);
    memset(data, 0, maxpacket * എണ്ണം);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<എണ്ണം; i++) {
        ഘടന ehci_qtd *td = &tds[i];
        td->qtd_next = (i==എണ്ണം-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    ഘടന ehci_framelist *fl = (ശൂന്യം*)readl(&cntl->regs->periodiclistbase);
    എങ്കിൽ (frameexp == 0) {
        // Add to existing interrupt entry.
        ഘടന ehci_qh *intr_qh = (ശൂന്യം*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } അല്ലെങ്കിൽ {
        പൂർണ്ണസംഖ്യ startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        വേണ്ടി (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    തിരികെ &pipe->pipe;
fail:
    സ്വതന്ത്രം(pipe);
    സ്വതന്ത്രം(tds);
    സ്വതന്ത്രം(data);
    തിരികെ ശൂന്യ;
}

ഘടന usb_pipe *
ehci_realloc_pipe(ഘടന usbdevice_s *usbdev, ഘടന usb_pipe *upipe
                  , ഘടന usb_endpoint_descriptor *epdesc)
{
    എങ്കിൽ (! CONFIG_USB_EHCI)
        തിരികെ ശൂന്യ;
    usb_add_freelist(upipe);
    എങ്കിൽ (!epdesc)
        തിരികെ ശൂന്യ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_INT)
        തിരികെ ehci_alloc_intr_pipe(usbdev, epdesc);
    ഘടന usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, ഘടന usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ഘടന usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    എങ്കിൽ (usbpipe) {
        // Use previously allocated pipe.
        ഘടന ehci_pipe *pipe = container_of(usbpipe, ഘടന ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        തിരികെ usbpipe;
    }

    // Allocate a new queue head.
    ഘടന ehci_pipe *pipe;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, വലുപ്പം(*pipe));
    അല്ലെങ്കിൽ
        pipe = memalign_താഴെയുള്ള(EHCI_QH_ALIGN, വലുപ്പം(*pipe));
    എങ്കിൽ (!pipe) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    memset(pipe, 0, വലുപ്പം(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    ഘടന ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    തിരികെ &pipe->pipe;
}

സ്ഥിതം ശൂന്യം
ehci_reset_pipe(ഘടന ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
ehci_wait_td(ഘടന ehci_pipe *pipe, ഘടന ehci_qtd *td, u32 end)
{
    u32 status;
    വേണ്ടി (;;) {
        status = td->token;
        എങ്കിൽ (!(status & QTD_STS_ACTIVE))
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            ഘടന usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), ഘടന usb_ehci_s, usb);
            ehci_waittick(cntl);
            തിരികെ -1;
        }
        yield();
    }
    എങ്കിൽ (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        തിരികെ -2;
    }
    തിരികെ 0;
}

സ്ഥിതം ശൂന്യം
ehci_fill_tdbuf(ഘടന ehci_qtd *td, u32 dest, പൂർണ്ണസംഖ്യ transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    വേണ്ടി (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

പൂർണ്ണസംഖ്യ
ehci_send_pipe(ഘടന usb_pipe *p, പൂർണ്ണസംഖ്യ dir, സ്ഥിരം ശൂന്യം *cmd
               , ശൂന്യം *data, പൂർണ്ണസംഖ്യ datasize)
{
    എങ്കിൽ (! CONFIG_USB_EHCI)
        തിരികെ -1;
    ഘടന ehci_pipe *pipe = container_of(p, ഘടന ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[വലുപ്പം(ഘടന ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    ഘടന ehci_qtd *tds = (ശൂന്യം*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, വലുപ്പം(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    എങ്കിൽ (cmd) {
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
    ആകുമ്പോൾ (dest < dataend) {
        // Send data pids
        എങ്കിൽ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            തിരികെ -1;
        }
        പൂർണ്ണസംഖ്യ maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        പൂർണ്ണസംഖ്യ transfer = dataend - dest;
        എങ്കിൽ (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    എങ്കിൽ (cmd) {
        // Send status pid on control transfers
        എങ്കിൽ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            തിരികെ -1;
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
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0, td=tds; i<STACKQTDS; i++, td++) {
        പൂർണ്ണസംഖ്യ ret = ehci_wait_td(pipe, td, end);
        എങ്കിൽ (ret)
            തിരികെ -1;
    }

    തിരികെ 0;
}

പൂർണ്ണസംഖ്യ
ehci_poll_intr(ഘടന usb_pipe *p, ശൂന്യം *data)
{
    ASSERT16();
    എങ്കിൽ (! CONFIG_USB_EHCI)
        തിരികെ -1;
    ഘടന ehci_pipe *pipe = container_of(p, ഘടന ehci_pipe, pipe);
    ഘടന ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    എങ്കിൽ (token & QTD_STS_ACTIVE)
        // No intrs found.
        തിരികെ -1;
    // XXX - check for errors.

    // Copy data.
    പൂർണ്ണസംഖ്യ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    പൂർണ്ണസംഖ്യ pos = td - GET_LOWFLAT(pipe->tds);
    ശൂന്യം *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_താഴെയുള്ള, LOWFLAT2_താഴെയുള്ള(tddata), maxpacket);

    // Reenable this td.
    ഘടന ehci_qtd *next = (ശൂന്യം*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    തിരികെ 0;
}
