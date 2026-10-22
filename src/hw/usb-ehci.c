// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_LOWFLAT
#ضمن "config.h" // CONFIG_*
#ضمن "output.h" // dprintf
#ضمن "malloc.h" // free
#ضمن "memmap.h" // PAGE_SIZE
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ضمن "pci_regs.h" // PCI_BASE_ADDRESS_0
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-ehci.h" // struct ehci_qh
#ضمن "util.h" // msleep
#ضمن "x86.h" // readl

بنية usb_ehci_s {
    بنية usb_s usb;
    بنية ehci_caps *caps;
    بنية ehci_regs *regs;
    بنية ehci_qh *async_qh;
    صحيح checkports;
};

بنية ehci_pipe {
    بنية ehci_qh qh;
    بنية ehci_qtd *next_td, *tds;
    فراغ *data;
    بنية usb_pipe pipe;
};

ساكن صحيح PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
ساكن صحيح
ehci_hub_detect(بنية usbhub_s *hub, u32 port)
{
    بنية usb_ehci_s *cntl = container_of(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    إذا (!(portsc & PORT_CONNECT))
        // No device present
        أرجع 0;

    إذا ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        أرجع -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    أرجع 1;
}

// Reset device on port
ساكن صحيح
ehci_hub_reset(بنية usbhub_s *hub, u32 port)
{
    بنية usb_ehci_s *cntl = container_of(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    إذا (!(portsc & PORT_CONNECT))
        // No longer connected
        أرجع -1;
    إذا (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        أرجع -1;
    }

    أرجع USB_HIGHSPEED;
}

// Disable port
ساكن فراغ
ehci_hub_disconnect(بنية usbhub_s *hub, u32 port)
{
    بنية usb_ehci_s *cntl = container_of(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

ساكن بنية usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساكن صحيح
check_ehci_ports(بنية usb_ehci_s *cntl)
{
    // Power up ports.
    صحيح i;
    لكل (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        إذا (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    بنية usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    أرجع hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
ساكن فراغ
ehci_waittick(بنية usb_ehci_s *cntl)
{
    إذا (MODE16) {
        msleep(10);
        أرجع;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    لكل (;;) {
        sts = readl(&cntl->regs->usbsts);
        إذا (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            إذا (!(cmd & CMD_IAAD))
                اكسر;
        }
        إذا (timer_check(end)) {
            warn_timeout();
            أرجع;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    لكل (;;) {
        sts = readl(&cntl->regs->usbsts);
        إذا (sts & STS_IAA)
            اكسر;
        إذا (timer_check(end)) {
            warn_timeout();
            أرجع;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

ساكن فراغ
ehci_free_pipes(بنية usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    بنية ehci_qh *start = cntl->async_qh;
    بنية ehci_qh *pos = start;
    لكل (;;) {
        بنية ehci_qh *next = (فراغ*)(pos->next & ~EHCI_PTR_BITS);
        إذا (next == start)
            اكسر;
        بنية ehci_pipe *pipe = container_of(next, بنية ehci_pipe, qh);
        إذا (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        وإلا
            pos = next;
    }
    ehci_waittick(cntl);
    لكل (;;) {
        بنية usb_pipe *usbpipe = cntl->usb.freelist;
        إذا (!usbpipe)
            اكسر;
        cntl->usb.freelist = usbpipe->freenext;
        بنية ehci_pipe *pipe = container_of(usbpipe, بنية ehci_pipe, pipe);
        حرر(pipe);
    }
}

ساكن فراغ
configure_ehci(فراغ *data)
{
    بنية usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    بنية ehci_framelist *fl = memalign_يۇقىرى(حجم(*fl), حجم(*fl));
    بنية ehci_qh *intr_qh = memalign_يۇقىرى(EHCI_QH_ALIGN, حجم(*intr_qh));
    بنية ehci_qh *async_qh = memalign_يۇقىرى(EHCI_QH_ALIGN, حجم(*async_qh));
    إذا (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        اذهب fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    لكل (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        إذا (!(cmd & CMD_HCRESET))
            اكسر;
        إذا (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            اذهب fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, حجم(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    صحيح i;
    لكل (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, حجم(*async_qh));
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
    صحيح سانى = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    إذا (سانى)
        // Success
        أرجع;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    حرر(fl);
    حرر(intr_qh);
    حرر(async_qh);
    حرر(cntl);
}

ساكن فراغ
ehci_controller_setup(بنية pci_device *pci)
{
    بنية ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    إذا (!caps)
        أرجع;
    u32 hcc_params = readl(&caps->hccparams);

    بنية usb_ehci_s *cntl = malloc_tmphigh(حجم(*cntl));
    إذا (!cntl) {
        warn_noalloc();
        أرجع;
    }
    memset(cntl, 0, حجم(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (فراغ*)caps + readb(&caps->caplength);
    إذا (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

فراغ
ehci_setup(فراغ)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع;
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
فراغ
ehci_wait_controllers(فراغ)
{
    طالما (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
ساكن فراغ
ehci_desc2pipe(بنية ehci_pipe *pipe, بنية usbdevice_s *usbdev
               , بنية usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    بنية usbdevice_s *hubdev = usbdev->hub->usbdev;
    إذا (hubdev) {
        بنية ehci_pipe *hpipe = container_of(
            hubdev->defpipe, بنية ehci_pipe, pipe);
        إذا (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        وإلا
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    وإلا إذا (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

ساكن بنية usb_pipe *
ehci_alloc_intr_pipe(بنية usbdevice_s *usbdev
                     , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, بنية usb_ehci_s, usb);
    صحيح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    إذا (frameexp > 10)
        frameexp = 10;
    صحيح maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    صحيح ms = 1<<frameexp;
    صحيح سانى = DIV_ROUND_UP(ticks_to_ms(2), ms);
    بنية ehci_pipe *pipe = memalign_تۆۋەن(EHCI_QH_ALIGN, حجم(*pipe));
    بنية ehci_qtd *tds = memalign_تۆۋەن(EHCI_QTD_ALIGN, حجم(*tds) * سانى);
    فراغ *data = malloc_تۆۋەن(maxpacket * سانى);
    إذا (!pipe || !tds || !data) {
        warn_noalloc();
        اذهب fail;
    }
    memset(pipe, 0, حجم(*pipe));
    memset(tds, 0, حجم(*tds) * سانى);
    memset(data, 0, maxpacket * سانى);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    صحيح i;
    لكل (i=0; i<سانى; i++) {
        بنية ehci_qtd *td = &tds[i];
        td->qtd_next = (i==سانى-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    بنية ehci_framelist *fl = (فراغ*)readl(&cntl->regs->periodiclistbase);
    إذا (frameexp == 0) {
        // Add to existing interrupt entry.
        بنية ehci_qh *intr_qh = (فراغ*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } وإلا {
        صحيح startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        لكل (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    أرجع &pipe->pipe;
fail:
    حرر(pipe);
    حرر(tds);
    حرر(data);
    أرجع عدم;
}

بنية usb_pipe *
ehci_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *upipe
                  , بنية usb_endpoint_descriptor *epdesc)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع عدم;
    usb_add_freelist(upipe);
    إذا (!epdesc)
        أرجع عدم;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    إذا (eptype == USB_ENDPOINT_XFER_INT)
        أرجع ehci_alloc_intr_pipe(usbdev, epdesc);
    بنية usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, بنية usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    بنية usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    إذا (usbpipe) {
        // Use previously allocated pipe.
        بنية ehci_pipe *pipe = container_of(usbpipe, بنية ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        أرجع usbpipe;
    }

    // Allocate a new queue head.
    بنية ehci_pipe *pipe;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, حجم(*pipe));
    وإلا
        pipe = memalign_تۆۋەن(EHCI_QH_ALIGN, حجم(*pipe));
    إذا (!pipe) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(pipe, 0, حجم(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    بنية ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    أرجع &pipe->pipe;
}

ساكن فراغ
ehci_reset_pipe(بنية ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ساكن صحيح
ehci_wait_td(بنية ehci_pipe *pipe, بنية ehci_qtd *td, u32 end)
{
    u32 status;
    لكل (;;) {
        status = td->token;
        إذا (!(status & QTD_STS_ACTIVE))
            اكسر;
        إذا (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            بنية usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), بنية usb_ehci_s, usb);
            ehci_waittick(cntl);
            أرجع -1;
        }
        yield();
    }
    إذا (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        أرجع -2;
    }
    أرجع 0;
}

ساكن فراغ
ehci_fill_tdbuf(بنية ehci_qtd *td, u32 dest, صحيح transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    لكل (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

صحيح
ehci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *data, صحيح datasize)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع -1;
    بنية ehci_pipe *pipe = container_of(p, بنية ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[حجم(بنية ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    بنية ehci_qtd *tds = (فراغ*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, حجم(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    إذا (cmd) {
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
    طالما (dest < dataend) {
        // Send data pids
        إذا (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            أرجع -1;
        }
        صحيح maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        صحيح transfer = dataend - dest;
        إذا (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    إذا (cmd) {
        // Send status pid on control transfers
        إذا (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            أرجع -1;
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
    صحيح i;
    لكل (i=0, td=tds; i<STACKQTDS; i++, td++) {
        صحيح ret = ehci_wait_td(pipe, td, end);
        إذا (ret)
            أرجع -1;
    }

    أرجع 0;
}

صحيح
ehci_poll_intr(بنية usb_pipe *p, فراغ *data)
{
    ASSERT16();
    إذا (! CONFIG_USB_EHCI)
        أرجع -1;
    بنية ehci_pipe *pipe = container_of(p, بنية ehci_pipe, pipe);
    بنية ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    إذا (token & QTD_STS_ACTIVE)
        // No intrs found.
        أرجع -1;
    // XXX - check for errors.

    // Copy data.
    صحيح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحيح pos = td - GET_LOWFLAT(pipe->tds);
    فراغ *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_تۆۋەن, LOWFLAT2_تۆۋەن(tddata), maxpacket);

    // Reenable this td.
    بنية ehci_qtd *next = (فراغ*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    أرجع 0;
}
