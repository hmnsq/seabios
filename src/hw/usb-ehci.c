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
    بنية ehci_qtd *ايندڙ_td, *tds;
    فراغ *معلومات;
    بنية usb_pipe pipe;
};

ساكن صحيح PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#وصف_ڪريو EHCI_وقت_POSTPOWER 20
#وصف_ڪريو EHCI_وقت_POSTRESET 2

// Check if device attached to port
ساكن صحيح
ehci_hub_detect(بنية usbhub_s *hub, u32 پورٽ)
{
    بنية usb_ehci_s *cntl = container_جو(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٽ];
    u32 portsc = readl(portreg);

    إذا (!(portsc & پورٽ_CONNECT))
        // No device present
        أرجع 0;

    إذا ((portsc & پورٽ_LINESTATUS_نقاب) == پورٽ_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | پورٽ_OWNER);
        أرجع -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~پورٽ_PE) | پورٽ_RESET;
    writel(portreg, portsc);
    msleep(USB_وقت_DRSTR);
    أرجع 1;
}

// Reset device on port
ساكن صحيح
ehci_hub_reset(بنية usbhub_s *hub, u32 پورٽ)
{
    بنية usb_ehci_s *cntl = container_جو(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٽ];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~پورٽ_RESET;
    writel(portreg, portsc);
    msleep(EHCI_وقت_POSTRESET);

    portsc = readl(portreg);
    إذا (!(portsc & پورٽ_CONNECT))
        // No longer connected
        أرجع -1;
    إذا (!(portsc & پورٽ_PE)) {
        // full speed device
        writel(portreg, portsc | پورٽ_OWNER);
        أرجع -1;
    }

    أرجع USB_HIGHSPEED;
}

// Disable port
ساكن فراغ
ehci_hub_disconnect(بنية usbhub_s *hub, u32 پورٽ)
{
    بنية usb_ehci_s *cntl = container_جو(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٽ];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~پورٽ_PE);
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
        إذا (!(portsc & پورٽ_POWER)) {
            portsc |= پورٽ_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_وقت_POSTPOWER);

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
    u32 پڇاڙي = timer_calc(100);
    لكل (;;) {
        sts = readl(&cntl->regs->usbsts);
        إذا (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            إذا (!(cmd & CMD_IAAD))
                اكسر;
        }
        إذا (timer_check(پڇاڙي)) {
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
        إذا (timer_check(پڇاڙي)) {
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

    بنية ehci_qh *شروعات = cntl->async_qh;
    بنية ehci_qh *pos = شروعات;
    لكل (;;) {
        بنية ehci_qh *ايندڙ = (فراغ*)(pos->ايندڙ & ~EHCI_PTR_بٽ);
        إذا (ايندڙ == شروعات)
            اكسر;
        بنية ehci_pipe *pipe = container_جو(ايندڙ, بنية ehci_pipe, qh);
        إذا (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->ايندڙ = ايندڙ->ايندڙ;
        وإلا
            pos = ايندڙ;
    }
    ehci_waittick(cntl);
    لكل (;;) {
        بنية usb_pipe *usbpipe = cntl->usb.freelist;
        إذا (!usbpipe)
            اكسر;
        cntl->usb.freelist = usbpipe->freenext;
        بنية ehci_pipe *pipe = container_جو(usbpipe, بنية ehci_pipe, pipe);
        حرر(pipe);
    }
}

ساكن فراغ
configure_ehci(فراغ *معلومات)
{
    بنية usb_ehci_s *cntl = معلومات;

    // Allocate ram for schedule storage
    بنية ehci_framelist *fl = memalign_مٿاهون(حجم(*fl), حجم(*fl));
    بنية ehci_qh *intr_qh = memalign_مٿاهون(EHCI_QH_ALIGN, حجم(*intr_qh));
    بنية ehci_qh *async_qh = memalign_مٿاهون(EHCI_QH_ALIGN, حجم(*async_qh));
    إذا (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        اذهب fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 پڇاڙي = timer_calc(250);
    لكل (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        إذا (!(cmd & CMD_HCRESET))
            اكسر;
        إذا (timer_check(پڇاڙي)) {
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
    intr_qh->ايندڙ = EHCI_PTR_رڪن;
    intr_qh->info2 = (0x01 << QH_SMASK_سرڪايو);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_ايندڙ = intr_qh->alt_ايندڙ = EHCI_PTR_رڪن;
    صحيح i;
    لكل (i=0; i<ARRAY_سائيز(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, حجم(*async_qh));
    async_qh->ايندڙ = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_سر;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_ايندڙ = async_qh->alt_ايندڙ = EHCI_PTR_رڪن;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_لاڳيتو);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    صحيح ڳڻپ = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    إذا (ڳڻپ)
        // Success
        أرجع;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_لاڳيتو);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    حرر(fl);
    حرر(intr_qh);
    حرر(async_qh);
    حرر(cntl);
}

ساكن فراغ
ehci_controller_تياري(بنية pci_device *pci)
{
    بنية ehci_caps *caps = pci_enable_membar(pci, PCI_بنياد_پتو_0);
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
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_نقاب;
    cntl->regs = (فراغ*)caps + readb(&caps->caplength);
    إذا (hcc_params & HCC_64_ٻٽو_انگ_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    لاڳيتو_thread(configure_ehci, cntl);
}

فراغ
ehci_تياري(فراغ)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع;
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_تياري(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
فراغ
ehci_wait_controllers(فراغ)
{
    طالما (CONFIG_USB_EHCI && CONFIG_ڌاڳا && PendingEHCI)
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

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_سرڪايو)
                      | (pipe->pipe.رفتار << QH_رفتار_سرڪايو)
                      | (pipe->pipe.ep << QH_EP_سرڪايو)
                      | (pipe->pipe.devaddr << QH_DEVADDR_سرڪايو));

    pipe->qh.info2 = (1 << QH_MULT_سرڪايو);
    بنية usbdevice_s *hubdev = usbdev->hub->usbdev;
    إذا (hubdev) {
        بنية ehci_pipe *hpipe = container_جو(
            hubdev->defpipe, بنية ehci_pipe, pipe);
        إذا (hpipe->pipe.رفتار == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->پورٽ+1) << QH_HUBPORT_سرڪايو)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_سرڪايو));
        وإلا
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bm_خاصيتون & USB_ENDPOINT_XFERTYPE_نقاب;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.رفتار != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    وإلا إذا (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_سرڪايو) | (0x1c << QH_CMASK_سرڪايو);
}

ساكن بنية usb_pipe *
ehci_alloc_intr_pipe(بنية usbdevice_s *usbdev
                     , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_ehci_s *cntl = container_جو(
        usbdev->hub->cntl, بنية usb_ehci_s, usb);
    صحيح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    إذا (frameexp > 10)
        frameexp = 10;
    صحيح maxpacket = epdesc->w_Max_Packet_سائيز;
    // Determine number of entries needed for 2 timer ticks.
    صحيح ms = 1<<frameexp;
    صحيح ڳڻپ = DIV_ڦيرو_مٿي(ticks_منزل_ms(2), ms);
    بنية ehci_pipe *pipe = memalign_هيٺيون(EHCI_QH_ALIGN, حجم(*pipe));
    بنية ehci_qtd *tds = memalign_هيٺيون(EHCI_QTD_ALIGN, حجم(*tds) * ڳڻپ);
    فراغ *معلومات = malloc_هيٺيون(maxpacket * ڳڻپ);
    إذا (!pipe || !tds || !معلومات) {
        warn_noalloc();
        اذهب fail;
    }
    memset(pipe, 0, حجم(*pipe));
    memset(tds, 0, حجم(*tds) * ڳڻپ);
    memset(معلومات, 0, maxpacket * ڳڻپ);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ايندڙ_td = pipe->tds = tds;
    pipe->معلومات = معلومات;
    pipe->qh.qtd_ايندڙ = (u32)tds;

    صحيح i;
    لكل (i=0; i<ڳڻپ; i++) {
        بنية ehci_qtd *td = &tds[i];
        td->qtd_ايندڙ = (i==ڳڻپ-1 ? (u32)tds : (u32)&td[1]);
        td->alt_ايندڙ = EHCI_PTR_رڪن;
        td->token = (ehci_explen(maxpacket) | QTD_STS_سرگرم
                     | QTD_PID_اندريون | ehci_maxerr(3));
        td->buf[0] = (u32)معلومات + maxpacket * i;
    }

    // Add to interrupt schedule.
    بنية ehci_framelist *fl = (فراغ*)readl(&cntl->regs->periodiclistbase);
    إذا (frameexp == 0) {
        // Add to existing interrupt entry.
        بنية ehci_qh *intr_qh = (فراغ*)(fl->links[0] & ~EHCI_PTR_بٽ);
        pipe->qh.ايندڙ = intr_qh->ايندڙ;
        barrier();
        intr_qh->ايندڙ = (u32)&pipe->qh | EHCI_PTR_QH;
    } وإلا {
        صحيح startpos = 1<<(frameexp-1);
        pipe->qh.ايندڙ = fl->links[startpos];
        barrier();
        لكل (i=startpos; i<ARRAY_سائيز(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    أرجع &pipe->pipe;
fail:
    حرر(pipe);
    حرر(tds);
    حرر(معلومات);
    أرجع عدم;
}

بنية usb_pipe *
ehci_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *upipe
                  , بنية usb_endpoint_descriptor *epdesc)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع عدم;
    usb_جوڙيو_freelist(upipe);
    إذا (!epdesc)
        أرجع عدم;
    u8 eptype = epdesc->bm_خاصيتون & USB_ENDPOINT_XFERTYPE_نقاب;
    إذا (eptype == USB_ENDPOINT_XFER_INT)
        أرجع ehci_alloc_intr_pipe(usbdev, epdesc);
    بنية usb_ehci_s *cntl = container_جو(
        usbdev->hub->cntl, بنية usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    بنية usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    إذا (usbpipe) {
        // Use previously allocated pipe.
        بنية ehci_pipe *pipe = container_جو(usbpipe, بنية ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        أرجع usbpipe;
    }

    // Allocate a new queue head.
    بنية ehci_pipe *pipe;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, حجم(*pipe));
    وإلا
        pipe = memalign_هيٺيون(EHCI_QH_ALIGN, حجم(*pipe));
    إذا (!pipe) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(pipe, 0, حجم(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_ايندڙ = pipe->qh.alt_ايندڙ = EHCI_PTR_رڪن;

    // Add queue head to controller list.
    بنية ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.ايندڙ = async_qh->ايندڙ;
    barrier();
    async_qh->ايندڙ = (u32)&pipe->qh | EHCI_PTR_QH;
    أرجع &pipe->pipe;
}

ساكن فراغ
ehci_reset_pipe(بنية ehci_pipe *pipe)
{
    سيٽ_LOWFLAT(pipe->qh.qtd_ايندڙ, EHCI_PTR_رڪن);
    سيٽ_LOWFLAT(pipe->qh.alt_ايندڙ, EHCI_PTR_رڪن);
    barrier();
    سيٽ_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ساكن صحيح
ehci_wait_td(بنية ehci_pipe *pipe, بنية ehci_qtd *td, u32 پڇاڙي)
{
    u32 حالت;
    لكل (;;) {
        حالت = td->token;
        إذا (!(حالت & QTD_STS_سرگرم))
            اكسر;
        إذا (timer_check(پڇاڙي)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 ايندڙ = GET_LOWFLAT(pipe->qh.qtd_ايندڙ);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, ايندڙ, td, حالت);
            ehci_reset_pipe(pipe);
            بنية usb_ehci_s *cntl = container_جو(
                GET_LOWFLAT(pipe->pipe.cntl), بنية usb_ehci_s, usb);
            ehci_waittick(cntl);
            أرجع -1;
        }
        yield();
    }
    إذا (حالت & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", حالت);
        ehci_reset_pipe(pipe);
        أرجع -2;
    }
    أرجع 0;
}

ساكن فراغ
ehci_fill_tdbuf(بنية ehci_qtd *td, u32 dest, صحيح transfer)
{
    u32 *pos = td->buf, پڇاڙي = dest + transfer;
    لكل (; dest < پڇاڙي; dest = ALIGN_هيٺ_ڏانهن(dest + صفحو_سائيز, صفحو_سائيز))
        *pos++ = dest;
}

#وصف_ڪريو STACKQTDS 6

صحيح
ehci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *معلومات, صحيح datasize)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع -1;
    بنية ehci_pipe *pipe = container_جو(p, بنية ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, معلومات, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[حجم(بنية ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    بنية ehci_qtd *tds = (فراغ*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, حجم(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    إذا (cmd) {
        // Send setup pid on control transfers
        td->qtd_ايندڙ = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_ايندڙ = EHCI_PTR_رڪن;
        td->token = (ehci_explen(USB_CONTROL_تياري_سائيز) | QTD_STS_سرگرم
                     | QTD_PID_تياري | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_تياري_سائيز);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)معلومات, dataend = dest + datasize;
    طالما (dest < dataend) {
        // Send data pids
        إذا (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            أرجع -1;
        }
        صحيح maxtransfer = 5*صفحو_سائيز - (dest & (صفحو_سائيز-1));
        صحيح transfer = dataend - dest;
        إذا (transfer > maxtransfer)
            transfer = ALIGN_هيٺ_ڏانهن(maxtransfer, maxpacket);
        td->qtd_ايندڙ = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_ايندڙ = EHCI_PTR_رڪن;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_سرگرم
                     | (dir ? QTD_PID_اندريون : QTD_PID_ٻاهريون) | ehci_maxerr(3));
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
        td->qtd_ايندڙ = EHCI_PTR_رڪن;
        td->alt_ايندڙ = EHCI_PTR_رڪن;
        td->token = (QTD_TOGGLE | QTD_STS_سرگرم
                     | (dir ? QTD_PID_ٻاهريون : QTD_PID_اندريون) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_ايندڙ = EHCI_PTR_رڪن;
    barrier();
    سيٽ_LOWFLAT(pipe->qh.qtd_ايندڙ, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 پڇاڙي = timer_calc(usb_xfer_وقت(p, datasize));
    صحيح i;
    لكل (i=0, td=tds; i<STACKQTDS; i++, td++) {
        صحيح ret = ehci_wait_td(pipe, td, پڇاڙي);
        إذا (ret)
            أرجع -1;
    }

    أرجع 0;
}

صحيح
ehci_poll_intr(بنية usb_pipe *p, فراغ *معلومات)
{
    ASSERT16();
    إذا (! CONFIG_USB_EHCI)
        أرجع -1;
    بنية ehci_pipe *pipe = container_جو(p, بنية ehci_pipe, pipe);
    بنية ehci_qtd *td = GET_LOWFLAT(pipe->ايندڙ_td);
    u32 token = GET_LOWFLAT(td->token);
    إذا (token & QTD_STS_سرگرم)
        // No intrs found.
        أرجع -1;
    // XXX - check for errors.

    // Copy data.
    صحيح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحيح pos = td - GET_LOWFLAT(pipe->tds);
    فراغ *tddata = GET_LOWFLAT(pipe->معلومات) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), معلومات, SEG_هيٺيون, LOWFLAT2_هيٺيون(tddata), maxpacket);

    // Reenable this td.
    بنية ehci_qtd *ايندڙ = (فراغ*)(GET_LOWFLAT(td->qtd_ايندڙ) & ~EHCI_PTR_بٽ);
    سيٽ_LOWFLAT(pipe->ايندڙ_td, ايندڙ);
    سيٽ_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    سيٽ_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_سرگرم
                            | QTD_PID_اندريون | ehci_maxerr(3)));

    أرجع 0;
}
