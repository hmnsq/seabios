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
    بنية ehci_qtd *بل_td, *tds;
    فراغ *معلومات;
    بنية usb_pipe pipe;
};

ساكن صحيح PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#تعریف_کړه EHCI_وخت_POSTPOWER 20
#تعریف_کړه EHCI_وخت_POSTRESET 2

// Check if device attached to port
ساكن صحيح
ehci_hub_detect(بنية usbhub_s *hub, u32 پورټ)
{
    بنية usb_ehci_s *cntl = container_د(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورټ];
    u32 portsc = readl(portreg);

    إذا (!(portsc & پورټ_CONNECT))
        // No device present
        أرجع 0;

    إذا ((portsc & پورټ_LINESTATUS_ماسک) == پورټ_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | پورټ_OWNER);
        أرجع -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~پورټ_PE) | پورټ_RESET;
    writel(portreg, portsc);
    msleep(USB_وخت_DRSTR);
    أرجع 1;
}

// Reset device on port
ساكن صحيح
ehci_hub_reset(بنية usbhub_s *hub, u32 پورټ)
{
    بنية usb_ehci_s *cntl = container_د(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورټ];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~پورټ_RESET;
    writel(portreg, portsc);
    msleep(EHCI_وخت_POSTRESET);

    portsc = readl(portreg);
    إذا (!(portsc & پورټ_CONNECT))
        // No longer connected
        أرجع -1;
    إذا (!(portsc & پورټ_PE)) {
        // full speed device
        writel(portreg, portsc | پورټ_OWNER);
        أرجع -1;
    }

    أرجع USB_HIGHSPEED;
}

// Disable port
ساكن فراغ
ehci_hub_disconnect(بنية usbhub_s *hub, u32 پورټ)
{
    بنية usb_ehci_s *cntl = container_د(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورټ];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~پورټ_PE);
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
        إذا (!(portsc & پورټ_POWER)) {
            portsc |= پورټ_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_وخت_POSTPOWER);

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
    u32 پای_ټکی = timer_calc(100);
    لكل (;;) {
        sts = readl(&cntl->regs->usbsts);
        إذا (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            إذا (!(cmd & CMD_IAAD))
                اكسر;
        }
        إذا (timer_check(پای_ټکی)) {
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
        إذا (timer_check(پای_ټکی)) {
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

    بنية ehci_qh *پیل = cntl->async_qh;
    بنية ehci_qh *pos = پیل;
    لكل (;;) {
        بنية ehci_qh *بل = (فراغ*)(pos->بل & ~EHCI_PTR_بټونه);
        إذا (بل == پیل)
            اكسر;
        بنية ehci_pipe *pipe = container_د(بل, بنية ehci_pipe, qh);
        إذا (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->بل = بل->بل;
        وإلا
            pos = بل;
    }
    ehci_waittick(cntl);
    لكل (;;) {
        بنية usb_pipe *usbpipe = cntl->usb.freelist;
        إذا (!usbpipe)
            اكسر;
        cntl->usb.freelist = usbpipe->freenext;
        بنية ehci_pipe *pipe = container_د(usbpipe, بنية ehci_pipe, pipe);
        حرر(pipe);
    }
}

ساكن فراغ
configure_ehci(فراغ *معلومات)
{
    بنية usb_ehci_s *cntl = معلومات;

    // Allocate ram for schedule storage
    بنية ehci_framelist *fl = memalign_لوړ(حجم(*fl), حجم(*fl));
    بنية ehci_qh *intr_qh = memalign_لوړ(EHCI_QH_ALIGN, حجم(*intr_qh));
    بنية ehci_qh *async_qh = memalign_لوړ(EHCI_QH_ALIGN, حجم(*async_qh));
    إذا (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        اذهب fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 پای_ټکی = timer_calc(250);
    لكل (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        إذا (!(cmd & CMD_HCRESET))
            اكسر;
        إذا (timer_check(پای_ټکی)) {
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
    intr_qh->بل = EHCI_PTR_غړی;
    intr_qh->info2 = (0x01 << QH_SMASK_ښويدنه);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_بل = intr_qh->alt_بل = EHCI_PTR_غړی;
    صحيح i;
    لكل (i=0; i<ARRAY_اندازه(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, حجم(*async_qh));
    async_qh->بل = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_سر;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_بل = async_qh->alt_بل = EHCI_PTR_غړی;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_پرلپسې);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    صحيح شمېر = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    إذا (شمېر)
        // Success
        أرجع;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_پرلپسې);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    حرر(fl);
    حرر(intr_qh);
    حرر(async_qh);
    حرر(cntl);
}

ساكن فراغ
ehci_controller_تیاری(بنية pci_device *pci)
{
    بنية ehci_caps *caps = pci_enable_membar(pci, PCI_بنسټ_پته_0);
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
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_ماسک;
    cntl->regs = (فراغ*)caps + readb(&caps->caplength);
    إذا (hcc_params & HCC_64_بټ_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    پرلپسې_thread(configure_ehci, cntl);
}

فراغ
ehci_تیاری(فراغ)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع;
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_تیاری(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
فراغ
ehci_wait_controllers(فراغ)
{
    طالما (CONFIG_USB_EHCI && CONFIG_تارونه && PendingEHCI)
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

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_ښويدنه)
                      | (pipe->pipe.چټکتیا << QH_چټکتیا_ښويدنه)
                      | (pipe->pipe.ep << QH_EP_ښويدنه)
                      | (pipe->pipe.devaddr << QH_DEVADDR_ښويدنه));

    pipe->qh.info2 = (1 << QH_MULT_ښويدنه);
    بنية usbdevice_s *hubdev = usbdev->hub->usbdev;
    إذا (hubdev) {
        بنية ehci_pipe *hpipe = container_د(
            hubdev->defpipe, بنية ehci_pipe, pipe);
        إذا (hpipe->pipe.چټکتیا == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->پورټ+1) << QH_HUBPORT_ښويدنه)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_ښويدنه));
        وإلا
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bm_ځانګړتیاوې & USB_ENDPOINT_XFERTYPE_ماسک;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.چټکتیا != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    وإلا إذا (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_ښويدنه) | (0x1c << QH_CMASK_ښويدنه);
}

ساكن بنية usb_pipe *
ehci_alloc_intr_pipe(بنية usbdevice_s *usbdev
                     , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_ehci_s *cntl = container_د(
        usbdev->hub->cntl, بنية usb_ehci_s, usb);
    صحيح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    إذا (frameexp > 10)
        frameexp = 10;
    صحيح maxpacket = epdesc->w_Max_Packet_اندازه;
    // Determine number of entries needed for 2 timer ticks.
    صحيح ms = 1<<frameexp;
    صحيح شمېر = DIV_پړاو_پورته(ticks_منزل_ms(2), ms);
    بنية ehci_pipe *pipe = memalign_ټیټ(EHCI_QH_ALIGN, حجم(*pipe));
    بنية ehci_qtd *tds = memalign_ټیټ(EHCI_QTD_ALIGN, حجم(*tds) * شمېر);
    فراغ *معلومات = malloc_ټیټ(maxpacket * شمېر);
    إذا (!pipe || !tds || !معلومات) {
        warn_noalloc();
        اذهب fail;
    }
    memset(pipe, 0, حجم(*pipe));
    memset(tds, 0, حجم(*tds) * شمېر);
    memset(معلومات, 0, maxpacket * شمېر);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->بل_td = pipe->tds = tds;
    pipe->معلومات = معلومات;
    pipe->qh.qtd_بل = (u32)tds;

    صحيح i;
    لكل (i=0; i<شمېر; i++) {
        بنية ehci_qtd *td = &tds[i];
        td->qtd_بل = (i==شمېر-1 ? (u32)tds : (u32)&td[1]);
        td->alt_بل = EHCI_PTR_غړی;
        td->token = (ehci_explen(maxpacket) | QTD_STS_فعال
                     | QTD_PID_دننه | ehci_maxerr(3));
        td->buf[0] = (u32)معلومات + maxpacket * i;
    }

    // Add to interrupt schedule.
    بنية ehci_framelist *fl = (فراغ*)readl(&cntl->regs->periodiclistbase);
    إذا (frameexp == 0) {
        // Add to existing interrupt entry.
        بنية ehci_qh *intr_qh = (فراغ*)(fl->links[0] & ~EHCI_PTR_بټونه);
        pipe->qh.بل = intr_qh->بل;
        barrier();
        intr_qh->بل = (u32)&pipe->qh | EHCI_PTR_QH;
    } وإلا {
        صحيح startpos = 1<<(frameexp-1);
        pipe->qh.بل = fl->links[startpos];
        barrier();
        لكل (i=startpos; i<ARRAY_اندازه(fl->links); i+=ms)
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
    usb_جمع_کړه_freelist(upipe);
    إذا (!epdesc)
        أرجع عدم;
    u8 eptype = epdesc->bm_ځانګړتیاوې & USB_ENDPOINT_XFERTYPE_ماسک;
    إذا (eptype == USB_ENDPOINT_XFER_INT)
        أرجع ehci_alloc_intr_pipe(usbdev, epdesc);
    بنية usb_ehci_s *cntl = container_د(
        usbdev->hub->cntl, بنية usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    بنية usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    إذا (usbpipe) {
        // Use previously allocated pipe.
        بنية ehci_pipe *pipe = container_د(usbpipe, بنية ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        أرجع usbpipe;
    }

    // Allocate a new queue head.
    بنية ehci_pipe *pipe;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, حجم(*pipe));
    وإلا
        pipe = memalign_ټیټ(EHCI_QH_ALIGN, حجم(*pipe));
    إذا (!pipe) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(pipe, 0, حجم(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_بل = pipe->qh.alt_بل = EHCI_PTR_غړی;

    // Add queue head to controller list.
    بنية ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.بل = async_qh->بل;
    barrier();
    async_qh->بل = (u32)&pipe->qh | EHCI_PTR_QH;
    أرجع &pipe->pipe;
}

ساكن فراغ
ehci_reset_pipe(بنية ehci_pipe *pipe)
{
    سېټ_LOWFLAT(pipe->qh.qtd_بل, EHCI_PTR_غړی);
    سېټ_LOWFLAT(pipe->qh.alt_بل, EHCI_PTR_غړی);
    barrier();
    سېټ_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ساكن صحيح
ehci_wait_td(بنية ehci_pipe *pipe, بنية ehci_qtd *td, u32 پای_ټکی)
{
    u32 حالت;
    لكل (;;) {
        حالت = td->token;
        إذا (!(حالت & QTD_STS_فعال))
            اكسر;
        إذا (timer_check(پای_ټکی)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 بل = GET_LOWFLAT(pipe->qh.qtd_بل);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, بل, td, حالت);
            ehci_reset_pipe(pipe);
            بنية usb_ehci_s *cntl = container_د(
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
    u32 *pos = td->buf, پای_ټکی = dest + transfer;
    لكل (; dest < پای_ټکی; dest = ALIGN_ښکته(dest + پاڼه_اندازه, پاڼه_اندازه))
        *pos++ = dest;
}

#تعریف_کړه STACKQTDS 6

صحيح
ehci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *معلومات, صحيح datasize)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع -1;
    بنية ehci_pipe *pipe = container_د(p, بنية ehci_pipe, pipe);
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
        td->qtd_بل = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_بل = EHCI_PTR_غړی;
        td->token = (ehci_explen(USB_CONTROL_تیاری_اندازه) | QTD_STS_فعال
                     | QTD_PID_تیاری | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_تیاری_اندازه);
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
        صحيح maxtransfer = 5*پاڼه_اندازه - (dest & (پاڼه_اندازه-1));
        صحيح transfer = dataend - dest;
        إذا (transfer > maxtransfer)
            transfer = ALIGN_ښکته(maxtransfer, maxpacket);
        td->qtd_بل = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_بل = EHCI_PTR_غړی;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_فعال
                     | (dir ? QTD_PID_دننه : QTD_PID_بهر) | ehci_maxerr(3));
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
        td->qtd_بل = EHCI_PTR_غړی;
        td->alt_بل = EHCI_PTR_غړی;
        td->token = (QTD_TOGGLE | QTD_STS_فعال
                     | (dir ? QTD_PID_بهر : QTD_PID_دننه) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_بل = EHCI_PTR_غړی;
    barrier();
    سېټ_LOWFLAT(pipe->qh.qtd_بل, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 پای_ټکی = timer_calc(usb_xfer_وخت(p, datasize));
    صحيح i;
    لكل (i=0, td=tds; i<STACKQTDS; i++, td++) {
        صحيح ret = ehci_wait_td(pipe, td, پای_ټکی);
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
    بنية ehci_pipe *pipe = container_د(p, بنية ehci_pipe, pipe);
    بنية ehci_qtd *td = GET_LOWFLAT(pipe->بل_td);
    u32 token = GET_LOWFLAT(td->token);
    إذا (token & QTD_STS_فعال)
        // No intrs found.
        أرجع -1;
    // XXX - check for errors.

    // Copy data.
    صحيح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحيح pos = td - GET_LOWFLAT(pipe->tds);
    فراغ *tddata = GET_LOWFLAT(pipe->معلومات) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), معلومات, SEG_ټیټ, LOWFLAT2_ټیټ(tddata), maxpacket);

    // Reenable this td.
    بنية ehci_qtd *بل = (فراغ*)(GET_LOWFLAT(td->qtd_بل) & ~EHCI_PTR_بټونه);
    سېټ_LOWFLAT(pipe->بل_td, بل);
    سېټ_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    سېټ_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_فعال
                            | QTD_PID_دننه | ehci_maxerr(3)));

    أرجع 0;
}
