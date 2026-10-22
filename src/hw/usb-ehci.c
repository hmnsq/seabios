// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_LOWFLAT
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "memmap.h" // PAGE_SIZE
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#அந்தர்பாவ "pci_regs.h" // PCI_BASE_ADDRESS_0
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "usb.h" // struct usb_s
#அந்தர்பாவ "usb-ehci.h" // struct ehci_qh
#அந்தர்பாவ "util.h" // msleep
#அந்தர்பாவ "x86.h" // readl

கட்டமைப்பு usb_ehci_s {
    கட்டமைப்பு usb_s usb;
    கட்டமைப்பு ehci_caps *caps;
    கட்டமைப்பு ehci_regs *regs;
    கட்டமைப்பு ehci_qh *async_qh;
    முழு checkports;
};

கட்டமைப்பு ehci_pipe {
    கட்டமைப்பு ehci_qh qh;
    கட்டமைப்பு ehci_qtd *next_td, *tds;
    வெறுமை *data;
    கட்டமைப்பு usb_pipe pipe;
};

நிலையான முழு PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
நிலையான முழு
ehci_hub_detect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_ehci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    எனில் (!(portsc & PORT_CONNECT))
        // No device present
        திரும்பு 0;

    எனில் ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        திரும்பு -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    திரும்பு 1;
}

// Reset device on port
நிலையான முழு
ehci_hub_reset(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_ehci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    எனில் (!(portsc & PORT_CONNECT))
        // No longer connected
        திரும்பு -1;
    எனில் (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        திரும்பு -1;
    }

    திரும்பு USB_HIGHSPEED;
}

// Disable port
நிலையான வெறுமை
ehci_hub_disconnect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_ehci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

நிலையான கட்டமைப்பு usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
நிலையான முழு
check_ehci_ports(கட்டமைப்பு usb_ehci_s *cntl)
{
    // Power up ports.
    முழு i;
    க்கு (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        எனில் (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    கட்டமைப்பு usbhub_s hub;
    memset(&hub, 0, அளவு(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    திரும்பு hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
நிலையான வெறுமை
ehci_waittick(கட்டமைப்பு usb_ehci_s *cntl)
{
    எனில் (MODE16) {
        msleep(10);
        திரும்பு;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    க்கு (;;) {
        sts = readl(&cntl->regs->usbsts);
        எனில் (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            எனில் (!(cmd & CMD_IAAD))
                நிறுத்து;
        }
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    க்கு (;;) {
        sts = readl(&cntl->regs->usbsts);
        எனில் (sts & STS_IAA)
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

நிலையான வெறுமை
ehci_free_pipes(கட்டமைப்பு usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    கட்டமைப்பு ehci_qh *start = cntl->async_qh;
    கட்டமைப்பு ehci_qh *pos = start;
    க்கு (;;) {
        கட்டமைப்பு ehci_qh *next = (வெறுமை*)(pos->next & ~EHCI_PTR_BITS);
        எனில் (next == start)
            நிறுத்து;
        கட்டமைப்பு ehci_pipe *pipe = container_of(next, கட்டமைப்பு ehci_pipe, qh);
        எனில் (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        இல்லையெனில்
            pos = next;
    }
    ehci_waittick(cntl);
    க்கு (;;) {
        கட்டமைப்பு usb_pipe *usbpipe = cntl->usb.freelist;
        எனில் (!usbpipe)
            நிறுத்து;
        cntl->usb.freelist = usbpipe->freenext;
        கட்டமைப்பு ehci_pipe *pipe = container_of(usbpipe, கட்டமைப்பு ehci_pipe, pipe);
        விடுவி(pipe);
    }
}

நிலையான வெறுமை
configure_ehci(வெறுமை *data)
{
    கட்டமைப்பு usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    கட்டமைப்பு ehci_framelist *fl = memalign_உயர்வு(அளவு(*fl), அளவு(*fl));
    கட்டமைப்பு ehci_qh *intr_qh = memalign_உயர்வு(EHCI_QH_ALIGN, அளவு(*intr_qh));
    கட்டமைப்பு ehci_qh *async_qh = memalign_உயர்வு(EHCI_QH_ALIGN, அளவு(*async_qh));
    எனில் (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        செல் fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    க்கு (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        எனில் (!(cmd & CMD_HCRESET))
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            செல் fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, அளவு(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    முழு i;
    க்கு (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, அளவு(*async_qh));
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
    முழு மொத்தம் = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    எனில் (மொத்தம்)
        // Success
        திரும்பு;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    விடுவி(fl);
    விடுவி(intr_qh);
    விடுவி(async_qh);
    விடுவி(cntl);
}

நிலையான வெறுமை
ehci_controller_setup(கட்டமைப்பு pci_device *pci)
{
    கட்டமைப்பு ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    எனில் (!caps)
        திரும்பு;
    u32 hcc_params = readl(&caps->hccparams);

    கட்டமைப்பு usb_ehci_s *cntl = malloc_tmphigh(அளவு(*cntl));
    எனில் (!cntl) {
        warn_noalloc();
        திரும்பு;
    }
    memset(cntl, 0, அளவு(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (வெறுமை*)caps + readb(&caps->caplength);
    எனில் (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

வெறுமை
ehci_setup(வெறுமை)
{
    எனில் (! CONFIG_USB_EHCI)
        திரும்பு;
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
வெறுமை
ehci_wait_controllers(வெறுமை)
{
    போது (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
நிலையான வெறுமை
ehci_desc2pipe(கட்டமைப்பு ehci_pipe *pipe, கட்டமைப்பு usbdevice_s *usbdev
               , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    கட்டமைப்பு usbdevice_s *hubdev = usbdev->hub->usbdev;
    எனில் (hubdev) {
        கட்டமைப்பு ehci_pipe *hpipe = container_of(
            hubdev->defpipe, கட்டமைப்பு ehci_pipe, pipe);
        எனில் (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        இல்லையெனில்
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    எனில் (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    இல்லையெனில் எனில் (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

நிலையான கட்டமைப்பு usb_pipe *
ehci_alloc_intr_pipe(கட்டமைப்பு usbdevice_s *usbdev
                     , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    கட்டமைப்பு usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_ehci_s, usb);
    முழு frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    எனில் (frameexp > 10)
        frameexp = 10;
    முழு maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    முழு ms = 1<<frameexp;
    முழு மொத்தம் = DIV_ROUND_UP(ticks_to_ms(2), ms);
    கட்டமைப்பு ehci_pipe *pipe = memalign_கீழ்(EHCI_QH_ALIGN, அளவு(*pipe));
    கட்டமைப்பு ehci_qtd *tds = memalign_கீழ்(EHCI_QTD_ALIGN, அளவு(*tds) * மொத்தம்);
    வெறுமை *data = malloc_கீழ்(maxpacket * மொத்தம்);
    எனில் (!pipe || !tds || !data) {
        warn_noalloc();
        செல் fail;
    }
    memset(pipe, 0, அளவு(*pipe));
    memset(tds, 0, அளவு(*tds) * மொத்தம்);
    memset(data, 0, maxpacket * மொத்தம்);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    முழு i;
    க்கு (i=0; i<மொத்தம்; i++) {
        கட்டமைப்பு ehci_qtd *td = &tds[i];
        td->qtd_next = (i==மொத்தம்-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    கட்டமைப்பு ehci_framelist *fl = (வெறுமை*)readl(&cntl->regs->periodiclistbase);
    எனில் (frameexp == 0) {
        // Add to existing interrupt entry.
        கட்டமைப்பு ehci_qh *intr_qh = (வெறுமை*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } இல்லையெனில் {
        முழு startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        க்கு (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    திரும்பு &pipe->pipe;
fail:
    விடுவி(pipe);
    விடுவி(tds);
    விடுவி(data);
    திரும்பு வெற்று;
}

கட்டமைப்பு usb_pipe *
ehci_realloc_pipe(கட்டமைப்பு usbdevice_s *usbdev, கட்டமைப்பு usb_pipe *upipe
                  , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    எனில் (! CONFIG_USB_EHCI)
        திரும்பு வெற்று;
    usb_add_freelist(upipe);
    எனில் (!epdesc)
        திரும்பு வெற்று;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    எனில் (eptype == USB_ENDPOINT_XFER_INT)
        திரும்பு ehci_alloc_intr_pipe(usbdev, epdesc);
    கட்டமைப்பு usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    கட்டமைப்பு usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    எனில் (usbpipe) {
        // Use previously allocated pipe.
        கட்டமைப்பு ehci_pipe *pipe = container_of(usbpipe, கட்டமைப்பு ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        திரும்பு usbpipe;
    }

    // Allocate a new queue head.
    கட்டமைப்பு ehci_pipe *pipe;
    எனில் (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, அளவு(*pipe));
    இல்லையெனில்
        pipe = memalign_கீழ்(EHCI_QH_ALIGN, அளவு(*pipe));
    எனில் (!pipe) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(pipe, 0, அளவு(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    கட்டமைப்பு ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    திரும்பு &pipe->pipe;
}

நிலையான வெறுமை
ehci_reset_pipe(கட்டமைப்பு ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

நிலையான முழு
ehci_wait_td(கட்டமைப்பு ehci_pipe *pipe, கட்டமைப்பு ehci_qtd *td, u32 end)
{
    u32 status;
    க்கு (;;) {
        status = td->token;
        எனில் (!(status & QTD_STS_ACTIVE))
            நிறுத்து;
        எனில் (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            கட்டமைப்பு usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), கட்டமைப்பு usb_ehci_s, usb);
            ehci_waittick(cntl);
            திரும்பு -1;
        }
        yield();
    }
    எனில் (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        திரும்பு -2;
    }
    திரும்பு 0;
}

நிலையான வெறுமை
ehci_fill_tdbuf(கட்டமைப்பு ehci_qtd *td, u32 dest, முழு transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    க்கு (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

முழு
ehci_send_pipe(கட்டமைப்பு usb_pipe *p, முழு dir, மாறிலி வெறுமை *cmd
               , வெறுமை *data, முழு datasize)
{
    எனில் (! CONFIG_USB_EHCI)
        திரும்பு -1;
    கட்டமைப்பு ehci_pipe *pipe = container_of(p, கட்டமைப்பு ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[அளவு(கட்டமைப்பு ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    கட்டமைப்பு ehci_qtd *tds = (வெறுமை*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, அளவு(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    எனில் (cmd) {
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
    போது (dest < dataend) {
        // Send data pids
        எனில் (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            திரும்பு -1;
        }
        முழு maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        முழு transfer = dataend - dest;
        எனில் (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    எனில் (cmd) {
        // Send status pid on control transfers
        எனில் (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            திரும்பு -1;
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
    முழு i;
    க்கு (i=0, td=tds; i<STACKQTDS; i++, td++) {
        முழு ret = ehci_wait_td(pipe, td, end);
        எனில் (ret)
            திரும்பு -1;
    }

    திரும்பு 0;
}

முழு
ehci_poll_intr(கட்டமைப்பு usb_pipe *p, வெறுமை *data)
{
    ASSERT16();
    எனில் (! CONFIG_USB_EHCI)
        திரும்பு -1;
    கட்டமைப்பு ehci_pipe *pipe = container_of(p, கட்டமைப்பு ehci_pipe, pipe);
    கட்டமைப்பு ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    எனில் (token & QTD_STS_ACTIVE)
        // No intrs found.
        திரும்பு -1;
    // XXX - check for errors.

    // Copy data.
    முழு maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    முழு pos = td - GET_LOWFLAT(pipe->tds);
    வெறுமை *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_கீழ், LOWFLAT2_கீழ்(tddata), maxpacket);

    // Reenable this td.
    கட்டமைப்பு ehci_qtd *next = (வெறுமை*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    திரும்பு 0;
}
