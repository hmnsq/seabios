// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_LOWFLAT
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "memmap.h" // PAGE_SIZE
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#અન્તર્ભાવ "pci_regs.h" // PCI_BASE_ADDRESS_0
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "usb.h" // struct usb_s
#અન્તર્ભાવ "usb-ehci.h" // struct ehci_qh
#અન્તર્ભાવ "util.h" // msleep
#અન્તર્ભાવ "x86.h" // readl

સંરચના usb_ehci_s {
    સંરચના usb_s usb;
    સંરચના ehci_caps *caps;
    સંરચના ehci_regs *regs;
    સંરચના ehci_qh *async_qh;
    પૂર્ણાંક checkports;
};

સંરચના ehci_pipe {
    સંરચના ehci_qh qh;
    સંરચના ehci_qtd *next_td, *tds;
    શૂન્ય *data;
    સંરચના usb_pipe pipe;
};

સ્થિર પૂર્ણાંક PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
સ્થિર પૂર્ણાંક
ehci_hub_detect(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_ehci_s *cntl = container_of(hub->cntl, સંરચના usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    જો (!(portsc & PORT_CONNECT))
        // No device present
        પાછા 0;

    જો ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        પાછા -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    પાછા 1;
}

// Reset device on port
સ્થિર પૂર્ણાંક
ehci_hub_reset(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_ehci_s *cntl = container_of(hub->cntl, સંરચના usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    જો (!(portsc & PORT_CONNECT))
        // No longer connected
        પાછા -1;
    જો (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        પાછા -1;
    }

    પાછા USB_HIGHSPEED;
}

// Disable port
સ્થિર શૂન્ય
ehci_hub_disconnect(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_ehci_s *cntl = container_of(hub->cntl, સંરચના usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

સ્થિર સંરચના usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
સ્થિર પૂર્ણાંક
check_ehci_ports(સંરચના usb_ehci_s *cntl)
{
    // Power up ports.
    પૂર્ણાંક i;
    માટે (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        જો (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    સંરચના usbhub_s hub;
    memset(&hub, 0, કદ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    પાછા hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
સ્થિર શૂન્ય
ehci_waittick(સંરચના usb_ehci_s *cntl)
{
    જો (MODE16) {
        msleep(10);
        પાછા;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    માટે (;;) {
        sts = readl(&cntl->regs->usbsts);
        જો (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            જો (!(cmd & CMD_IAAD))
                તોડો;
        }
        જો (timer_check(end)) {
            warn_timeout();
            પાછા;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    માટે (;;) {
        sts = readl(&cntl->regs->usbsts);
        જો (sts & STS_IAA)
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            પાછા;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

સ્થિર શૂન્ય
ehci_free_pipes(સંરચના usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    સંરચના ehci_qh *start = cntl->async_qh;
    સંરચના ehci_qh *pos = start;
    માટે (;;) {
        સંરચના ehci_qh *next = (શૂન્ય*)(pos->next & ~EHCI_PTR_BITS);
        જો (next == start)
            તોડો;
        સંરચના ehci_pipe *pipe = container_of(next, સંરચના ehci_pipe, qh);
        જો (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        નહીંતર
            pos = next;
    }
    ehci_waittick(cntl);
    માટે (;;) {
        સંરચના usb_pipe *usbpipe = cntl->usb.freelist;
        જો (!usbpipe)
            તોડો;
        cntl->usb.freelist = usbpipe->freenext;
        સંરચના ehci_pipe *pipe = container_of(usbpipe, સંરચના ehci_pipe, pipe);
        મુક્ત(pipe);
    }
}

સ્થિર શૂન્ય
configure_ehci(શૂન્ય *data)
{
    સંરચના usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    સંરચના ehci_framelist *fl = memalign_ઉપલું(કદ(*fl), કદ(*fl));
    સંરચના ehci_qh *intr_qh = memalign_ઉપલું(EHCI_QH_ALIGN, કદ(*intr_qh));
    સંરચના ehci_qh *async_qh = memalign_ઉપલું(EHCI_QH_ALIGN, કદ(*async_qh));
    જો (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        જાઓ fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    માટે (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        જો (!(cmd & CMD_HCRESET))
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            જાઓ fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, કદ(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    પૂર્ણાંક i;
    માટે (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, કદ(*async_qh));
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
    પૂર્ણાંક ગણતરી = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    જો (ગણતરી)
        // Success
        પાછા;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    મુક્ત(fl);
    મુક્ત(intr_qh);
    મુક્ત(async_qh);
    મુક્ત(cntl);
}

સ્થિર શૂન્ય
ehci_controller_setup(સંરચના pci_device *pci)
{
    સંરચના ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    જો (!caps)
        પાછા;
    u32 hcc_params = readl(&caps->hccparams);

    સંરચના usb_ehci_s *cntl = malloc_tmphigh(કદ(*cntl));
    જો (!cntl) {
        warn_noalloc();
        પાછા;
    }
    memset(cntl, 0, કદ(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (શૂન્ય*)caps + readb(&caps->caplength);
    જો (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

શૂન્ય
ehci_setup(શૂન્ય)
{
    જો (! CONFIG_USB_EHCI)
        પાછા;
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
શૂન્ય
ehci_wait_controllers(શૂન્ય)
{
    જ્યારે (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
સ્થિર શૂન્ય
ehci_desc2pipe(સંરચના ehci_pipe *pipe, સંરચના usbdevice_s *usbdev
               , સંરચના usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    સંરચના usbdevice_s *hubdev = usbdev->hub->usbdev;
    જો (hubdev) {
        સંરચના ehci_pipe *hpipe = container_of(
            hubdev->defpipe, સંરચના ehci_pipe, pipe);
        જો (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        નહીંતર
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    જો (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    નહીંતર જો (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

સ્થિર સંરચના usb_pipe *
ehci_alloc_intr_pipe(સંરચના usbdevice_s *usbdev
                     , સંરચના usb_endpoint_descriptor *epdesc)
{
    સંરચના usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, સંરચના usb_ehci_s, usb);
    પૂર્ણાંક frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    જો (frameexp > 10)
        frameexp = 10;
    પૂર્ણાંક maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    પૂર્ણાંક ms = 1<<frameexp;
    પૂર્ણાંક ગણતરી = DIV_ROUND_UP(ticks_to_ms(2), ms);
    સંરચના ehci_pipe *pipe = memalign_નીચલું(EHCI_QH_ALIGN, કદ(*pipe));
    સંરચના ehci_qtd *tds = memalign_નીચલું(EHCI_QTD_ALIGN, કદ(*tds) * ગણતરી);
    શૂન્ય *data = malloc_નીચલું(maxpacket * ગણતરી);
    જો (!pipe || !tds || !data) {
        warn_noalloc();
        જાઓ fail;
    }
    memset(pipe, 0, કદ(*pipe));
    memset(tds, 0, કદ(*tds) * ગણતરી);
    memset(data, 0, maxpacket * ગણતરી);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    પૂર્ણાંક i;
    માટે (i=0; i<ગણતરી; i++) {
        સંરચના ehci_qtd *td = &tds[i];
        td->qtd_next = (i==ગણતરી-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    સંરચના ehci_framelist *fl = (શૂન્ય*)readl(&cntl->regs->periodiclistbase);
    જો (frameexp == 0) {
        // Add to existing interrupt entry.
        સંરચના ehci_qh *intr_qh = (શૂન્ય*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } નહીંતર {
        પૂર્ણાંક startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        માટે (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    પાછા &pipe->pipe;
fail:
    મુક્ત(pipe);
    મુક્ત(tds);
    મુક્ત(data);
    પાછા ખાલી;
}

સંરચના usb_pipe *
ehci_realloc_pipe(સંરચના usbdevice_s *usbdev, સંરચના usb_pipe *upipe
                  , સંરચના usb_endpoint_descriptor *epdesc)
{
    જો (! CONFIG_USB_EHCI)
        પાછા ખાલી;
    usb_add_freelist(upipe);
    જો (!epdesc)
        પાછા ખાલી;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    જો (eptype == USB_ENDPOINT_XFER_INT)
        પાછા ehci_alloc_intr_pipe(usbdev, epdesc);
    સંરચના usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, સંરચના usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    સંરચના usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    જો (usbpipe) {
        // Use previously allocated pipe.
        સંરચના ehci_pipe *pipe = container_of(usbpipe, સંરચના ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        પાછા usbpipe;
    }

    // Allocate a new queue head.
    સંરચના ehci_pipe *pipe;
    જો (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, કદ(*pipe));
    નહીંતર
        pipe = memalign_નીચલું(EHCI_QH_ALIGN, કદ(*pipe));
    જો (!pipe) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(pipe, 0, કદ(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    સંરચના ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    પાછા &pipe->pipe;
}

સ્થિર શૂન્ય
ehci_reset_pipe(સંરચના ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

સ્થિર પૂર્ણાંક
ehci_wait_td(સંરચના ehci_pipe *pipe, સંરચના ehci_qtd *td, u32 end)
{
    u32 status;
    માટે (;;) {
        status = td->token;
        જો (!(status & QTD_STS_ACTIVE))
            તોડો;
        જો (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            સંરચના usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), સંરચના usb_ehci_s, usb);
            ehci_waittick(cntl);
            પાછા -1;
        }
        yield();
    }
    જો (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        પાછા -2;
    }
    પાછા 0;
}

સ્થિર શૂન્ય
ehci_fill_tdbuf(સંરચના ehci_qtd *td, u32 dest, પૂર્ણાંક transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    માટે (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

પૂર્ણાંક
ehci_send_pipe(સંરચના usb_pipe *p, પૂર્ણાંક dir, અચલ શૂન્ય *cmd
               , શૂન્ય *data, પૂર્ણાંક datasize)
{
    જો (! CONFIG_USB_EHCI)
        પાછા -1;
    સંરચના ehci_pipe *pipe = container_of(p, સંરચના ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[કદ(સંરચના ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    સંરચના ehci_qtd *tds = (શૂન્ય*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, કદ(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    જો (cmd) {
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
    જ્યારે (dest < dataend) {
        // Send data pids
        જો (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            પાછા -1;
        }
        પૂર્ણાંક maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        પૂર્ણાંક transfer = dataend - dest;
        જો (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    જો (cmd) {
        // Send status pid on control transfers
        જો (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            પાછા -1;
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
    પૂર્ણાંક i;
    માટે (i=0, td=tds; i<STACKQTDS; i++, td++) {
        પૂર્ણાંક ret = ehci_wait_td(pipe, td, end);
        જો (ret)
            પાછા -1;
    }

    પાછા 0;
}

પૂર્ણાંક
ehci_poll_intr(સંરચના usb_pipe *p, શૂન્ય *data)
{
    ASSERT16();
    જો (! CONFIG_USB_EHCI)
        પાછા -1;
    સંરચના ehci_pipe *pipe = container_of(p, સંરચના ehci_pipe, pipe);
    સંરચના ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    જો (token & QTD_STS_ACTIVE)
        // No intrs found.
        પાછા -1;
    // XXX - check for errors.

    // Copy data.
    પૂર્ણાંક maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    પૂર્ણાંક pos = td - GET_LOWFLAT(pipe->tds);
    શૂન્ય *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_નીચલું, LOWFLAT2_નીચલું(tddata), maxpacket);

    // Reenable this td.
    સંરચના ehci_qtd *next = (શૂન્ય*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    પાછા 0;
}
