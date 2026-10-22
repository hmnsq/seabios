// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_LOWFLAT
#شامل "config.h" // CONFIG_*
#شامل "output.h" // dprintf
#شامل "malloc.h" // free
#شامل "memmap.h" // PAGE_SIZE
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#شامل "pci_regs.h" // PCI_BASE_ADDRESS_0
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-ehci.h" // struct ehci_qh
#شامل "util.h" // msleep
#شامل "x86.h" // readl

ساختار usb_ehci_s {
    ساختار usb_s usb;
    ساختار ehci_caps *caps;
    ساختار ehci_regs *regs;
    ساختار ehci_qh *async_qh;
    صحیح checkports;
};

ساختار ehci_pipe {
    ساختار ehci_qh qh;
    ساختار ehci_qtd *next_td, *tds;
    تهی *data;
    ساختار usb_pipe pipe;
};

ایستا صحیح PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
ایستا صحیح
ehci_hub_detect(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_ehci_s *cntl = container_of(hub->cntl, ساختار usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    اگر (!(portsc & PORT_CONNECT))
        // No device present
        بازگشت 0;

    اگر ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        بازگشت -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    بازگشت 1;
}

// Reset device on port
ایستا صحیح
ehci_hub_reset(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_ehci_s *cntl = container_of(hub->cntl, ساختار usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    اگر (!(portsc & PORT_CONNECT))
        // No longer connected
        بازگشت -1;
    اگر (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        بازگشت -1;
    }

    بازگشت USB_HIGHSPEED;
}

// Disable port
ایستا تهی
ehci_hub_disconnect(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_ehci_s *cntl = container_of(hub->cntl, ساختار usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

ایستا ساختار usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
ایستا صحیح
check_ehci_ports(ساختار usb_ehci_s *cntl)
{
    // Power up ports.
    صحیح i;
    برای (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        اگر (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    ساختار usbhub_s hub;
    memset(&hub, 0, اندازه(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    بازگشت hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
ایستا تهی
ehci_waittick(ساختار usb_ehci_s *cntl)
{
    اگر (MODE16) {
        msleep(10);
        بازگشت;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    برای (;;) {
        sts = readl(&cntl->regs->usbsts);
        اگر (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            اگر (!(cmd & CMD_IAAD))
                بشکن;
        }
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    برای (;;) {
        sts = readl(&cntl->regs->usbsts);
        اگر (sts & STS_IAA)
            بشکن;
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

ایستا تهی
ehci_free_pipes(ساختار usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    ساختار ehci_qh *start = cntl->async_qh;
    ساختار ehci_qh *pos = start;
    برای (;;) {
        ساختار ehci_qh *next = (تهی*)(pos->next & ~EHCI_PTR_BITS);
        اگر (next == start)
            بشکن;
        ساختار ehci_pipe *pipe = container_of(next, ساختار ehci_pipe, qh);
        اگر (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        وگرنه
            pos = next;
    }
    ehci_waittick(cntl);
    برای (;;) {
        ساختار usb_pipe *usbpipe = cntl->usb.freelist;
        اگر (!usbpipe)
            بشکن;
        cntl->usb.freelist = usbpipe->freenext;
        ساختار ehci_pipe *pipe = container_of(usbpipe, ساختار ehci_pipe, pipe);
        آزاد(pipe);
    }
}

ایستا تهی
configure_ehci(تهی *data)
{
    ساختار usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    ساختار ehci_framelist *fl = memalign_بالا(اندازه(*fl), اندازه(*fl));
    ساختار ehci_qh *intr_qh = memalign_بالا(EHCI_QH_ALIGN, اندازه(*intr_qh));
    ساختار ehci_qh *async_qh = memalign_بالا(EHCI_QH_ALIGN, اندازه(*async_qh));
    اگر (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        برو fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    برای (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        اگر (!(cmd & CMD_HCRESET))
            بشکن;
        اگر (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            برو fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, اندازه(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    صحیح i;
    برای (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, اندازه(*async_qh));
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
    صحیح شمار = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    اگر (شمار)
        // Success
        بازگشت;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    آزاد(fl);
    آزاد(intr_qh);
    آزاد(async_qh);
    آزاد(cntl);
}

ایستا تهی
ehci_controller_setup(ساختار pci_device *pci)
{
    ساختار ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    اگر (!caps)
        بازگشت;
    u32 hcc_params = readl(&caps->hccparams);

    ساختار usb_ehci_s *cntl = malloc_tmphigh(اندازه(*cntl));
    اگر (!cntl) {
        warn_noalloc();
        بازگشت;
    }
    memset(cntl, 0, اندازه(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (تهی*)caps + readb(&caps->caplength);
    اگر (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

تهی
ehci_setup(تهی)
{
    اگر (! CONFIG_USB_EHCI)
        بازگشت;
    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
تهی
ehci_wait_controllers(تهی)
{
    وقتی (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
ایستا تهی
ehci_desc2pipe(ساختار ehci_pipe *pipe, ساختار usbdevice_s *usbdev
               , ساختار usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    ساختار usbdevice_s *hubdev = usbdev->hub->usbdev;
    اگر (hubdev) {
        ساختار ehci_pipe *hpipe = container_of(
            hubdev->defpipe, ساختار ehci_pipe, pipe);
        اگر (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        وگرنه
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    وگرنه اگر (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

ایستا ساختار usb_pipe *
ehci_alloc_intr_pipe(ساختار usbdevice_s *usbdev
                     , ساختار usb_endpoint_descriptor *epdesc)
{
    ساختار usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, ساختار usb_ehci_s, usb);
    صحیح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    اگر (frameexp > 10)
        frameexp = 10;
    صحیح maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    صحیح ms = 1<<frameexp;
    صحیح شمار = DIV_ROUND_UP(ticks_to_ms(2), ms);
    ساختار ehci_pipe *pipe = memalign_کم(EHCI_QH_ALIGN, اندازه(*pipe));
    ساختار ehci_qtd *tds = memalign_کم(EHCI_QTD_ALIGN, اندازه(*tds) * شمار);
    تهی *data = malloc_کم(maxpacket * شمار);
    اگر (!pipe || !tds || !data) {
        warn_noalloc();
        برو fail;
    }
    memset(pipe, 0, اندازه(*pipe));
    memset(tds, 0, اندازه(*tds) * شمار);
    memset(data, 0, maxpacket * شمار);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    صحیح i;
    برای (i=0; i<شمار; i++) {
        ساختار ehci_qtd *td = &tds[i];
        td->qtd_next = (i==شمار-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    ساختار ehci_framelist *fl = (تهی*)readl(&cntl->regs->periodiclistbase);
    اگر (frameexp == 0) {
        // Add to existing interrupt entry.
        ساختار ehci_qh *intr_qh = (تهی*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } وگرنه {
        صحیح startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        برای (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    بازگشت &pipe->pipe;
fail:
    آزاد(pipe);
    آزاد(tds);
    آزاد(data);
    بازگشت پوچ;
}

ساختار usb_pipe *
ehci_realloc_pipe(ساختار usbdevice_s *usbdev, ساختار usb_pipe *upipe
                  , ساختار usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_EHCI)
        بازگشت پوچ;
    usb_add_freelist(upipe);
    اگر (!epdesc)
        بازگشت پوچ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    اگر (eptype == USB_ENDPOINT_XFER_INT)
        بازگشت ehci_alloc_intr_pipe(usbdev, epdesc);
    ساختار usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, ساختار usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ساختار usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    اگر (usbpipe) {
        // Use previously allocated pipe.
        ساختار ehci_pipe *pipe = container_of(usbpipe, ساختار ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        بازگشت usbpipe;
    }

    // Allocate a new queue head.
    ساختار ehci_pipe *pipe;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, اندازه(*pipe));
    وگرنه
        pipe = memalign_کم(EHCI_QH_ALIGN, اندازه(*pipe));
    اگر (!pipe) {
        warn_noalloc();
        بازگشت پوچ;
    }
    memset(pipe, 0, اندازه(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    ساختار ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    بازگشت &pipe->pipe;
}

ایستا تهی
ehci_reset_pipe(ساختار ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ایستا صحیح
ehci_wait_td(ساختار ehci_pipe *pipe, ساختار ehci_qtd *td, u32 end)
{
    u32 status;
    برای (;;) {
        status = td->token;
        اگر (!(status & QTD_STS_ACTIVE))
            بشکن;
        اگر (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            ساختار usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), ساختار usb_ehci_s, usb);
            ehci_waittick(cntl);
            بازگشت -1;
        }
        yield();
    }
    اگر (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        بازگشت -2;
    }
    بازگشت 0;
}

ایستا تهی
ehci_fill_tdbuf(ساختار ehci_qtd *td, u32 dest, صحیح transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    برای (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

صحیح
ehci_send_pipe(ساختار usb_pipe *p, صحیح dir, ثابت تهی *cmd
               , تهی *data, صحیح datasize)
{
    اگر (! CONFIG_USB_EHCI)
        بازگشت -1;
    ساختار ehci_pipe *pipe = container_of(p, ساختار ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[اندازه(ساختار ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    ساختار ehci_qtd *tds = (تهی*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, اندازه(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    اگر (cmd) {
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
    وقتی (dest < dataend) {
        // Send data pids
        اگر (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            بازگشت -1;
        }
        صحیح maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        صحیح transfer = dataend - dest;
        اگر (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    اگر (cmd) {
        // Send status pid on control transfers
        اگر (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            بازگشت -1;
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
    صحیح i;
    برای (i=0, td=tds; i<STACKQTDS; i++, td++) {
        صحیح ret = ehci_wait_td(pipe, td, end);
        اگر (ret)
            بازگشت -1;
    }

    بازگشت 0;
}

صحیح
ehci_poll_intr(ساختار usb_pipe *p, تهی *data)
{
    ASSERT16();
    اگر (! CONFIG_USB_EHCI)
        بازگشت -1;
    ساختار ehci_pipe *pipe = container_of(p, ساختار ehci_pipe, pipe);
    ساختار ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    اگر (token & QTD_STS_ACTIVE)
        // No intrs found.
        بازگشت -1;
    // XXX - check for errors.

    // Copy data.
    صحیح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحیح pos = td - GET_LOWFLAT(pipe->tds);
    تهی *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_کم, LOWFLAT2_کم(tddata), maxpacket);

    // Reenable this td.
    ساختار ehci_qtd *next = (تهی*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    بازگشت 0;
}
