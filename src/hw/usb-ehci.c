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
    بنية ehci_qtd *اگلا_td, *tds;
    فراغ *انکڑے;
    بنية usb_pipe pipe;
};

ساكن صحيح PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#تعریف_کرو EHCI_ویلا_POSTPOWER 20
#تعریف_کرو EHCI_ویلا_POSTRESET 2

// Check if device attached to port
ساكن صحيح
ehci_hub_detect(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_ehci_s *cntl = container_دا(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٹ];
    u32 portsc = readl(portreg);

    إذا (!(portsc & پورٹ_CONNECT))
        // No device present
        أرجع 0;

    إذا ((portsc & پورٹ_LINESTATUS_نقاب) == پورٹ_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | پورٹ_OWNER);
        أرجع -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~پورٹ_PE) | پورٹ_RESET;
    writel(portreg, portsc);
    msleep(USB_ویلا_DRSTR);
    أرجع 1;
}

// Reset device on port
ساكن صحيح
ehci_hub_reset(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_ehci_s *cntl = container_دا(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٹ];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~پورٹ_RESET;
    writel(portreg, portsc);
    msleep(EHCI_ویلا_POSTRESET);

    portsc = readl(portreg);
    إذا (!(portsc & پورٹ_CONNECT))
        // No longer connected
        أرجع -1;
    إذا (!(portsc & پورٹ_PE)) {
        // full speed device
        writel(portreg, portsc | پورٹ_OWNER);
        أرجع -1;
    }

    أرجع USB_HIGHSPEED;
}

// Disable port
ساكن فراغ
ehci_hub_disconnect(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_ehci_s *cntl = container_دا(hub->cntl, بنية usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٹ];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~پورٹ_PE);
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
        إذا (!(portsc & پورٹ_POWER)) {
            portsc |= پورٹ_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_ویلا_POSTPOWER);

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
    u32 سرا = timer_calc(100);
    لكل (;;) {
        sts = readl(&cntl->regs->usbsts);
        إذا (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            إذا (!(cmd & CMD_IAAD))
                اكسر;
        }
        إذا (timer_check(سرا)) {
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
        إذا (timer_check(سرا)) {
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

    بنية ehci_qh *شروع = cntl->async_qh;
    بنية ehci_qh *pos = شروع;
    لكل (;;) {
        بنية ehci_qh *اگلا = (فراغ*)(pos->اگلا & ~EHCI_PTR_بٹ);
        إذا (اگلا == شروع)
            اكسر;
        بنية ehci_pipe *pipe = container_دا(اگلا, بنية ehci_pipe, qh);
        إذا (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->اگلا = اگلا->اگلا;
        وإلا
            pos = اگلا;
    }
    ehci_waittick(cntl);
    لكل (;;) {
        بنية usb_pipe *usbpipe = cntl->usb.freelist;
        إذا (!usbpipe)
            اكسر;
        cntl->usb.freelist = usbpipe->freenext;
        بنية ehci_pipe *pipe = container_دا(usbpipe, بنية ehci_pipe, pipe);
        حرر(pipe);
    }
}

ساكن فراغ
configure_ehci(فراغ *انکڑے)
{
    بنية usb_ehci_s *cntl = انکڑے;

    // Allocate ram for schedule storage
    بنية ehci_framelist *fl = memalign_بلند(حجم(*fl), حجم(*fl));
    بنية ehci_qh *intr_qh = memalign_بلند(EHCI_QH_ALIGN, حجم(*intr_qh));
    بنية ehci_qh *async_qh = memalign_بلند(EHCI_QH_ALIGN, حجم(*async_qh));
    إذا (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        اذهب fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 سرا = timer_calc(250);
    لكل (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        إذا (!(cmd & CMD_HCRESET))
            اكسر;
        إذا (timer_check(سرا)) {
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
    intr_qh->اگلا = EHCI_PTR_رکن;
    intr_qh->info2 = (0x01 << QH_SMASK_کھسکاؤ);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_اگلا = intr_qh->alt_اگلا = EHCI_PTR_رکن;
    صحيح i;
    لكل (i=0; i<ARRAY_ناپ(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, حجم(*async_qh));
    async_qh->اگلا = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_سر;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_اگلا = async_qh->alt_اگلا = EHCI_PTR_رکن;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_لگاتار);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    صحيح گنتی = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    إذا (گنتی)
        // Success
        أرجع;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_لگاتار);
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
    بنية ehci_caps *caps = pci_enable_membar(pci, PCI_ادھار_پتہ_0);
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
    إذا (hcc_params & HCC_64_ثنائی_ہندسہ_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    لگاتار_thread(configure_ehci, cntl);
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
    طالما (CONFIG_USB_EHCI && CONFIG_دھاگے && PendingEHCI)
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

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_کھسکاؤ)
                      | (pipe->pipe.رفتار << QH_رفتار_کھسکاؤ)
                      | (pipe->pipe.ep << QH_EP_کھسکاؤ)
                      | (pipe->pipe.devaddr << QH_DEVADDR_کھسکاؤ));

    pipe->qh.info2 = (1 << QH_MULT_کھسکاؤ);
    بنية usbdevice_s *hubdev = usbdev->hub->usbdev;
    إذا (hubdev) {
        بنية ehci_pipe *hpipe = container_دا(
            hubdev->defpipe, بنية ehci_pipe, pipe);
        إذا (hpipe->pipe.رفتار == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->پورٹ+1) << QH_HUBPORT_کھسکاؤ)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_کھسکاؤ));
        وإلا
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bm_گن & USB_ENDPOINT_XFERTYPE_نقاب;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.رفتار != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    وإلا إذا (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_کھسکاؤ) | (0x1c << QH_CMASK_کھسکاؤ);
}

ساكن بنية usb_pipe *
ehci_alloc_intr_pipe(بنية usbdevice_s *usbdev
                     , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_ehci_s *cntl = container_دا(
        usbdev->hub->cntl, بنية usb_ehci_s, usb);
    صحيح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    إذا (frameexp > 10)
        frameexp = 10;
    صحيح maxpacket = epdesc->w_Max_Packet_ناپ;
    // Determine number of entries needed for 2 timer ticks.
    صحيح ms = 1<<frameexp;
    صحيح گنتی = DIV_گیڑا_اتے(ticks_منزل_ms(2), ms);
    بنية ehci_pipe *pipe = memalign_کم(EHCI_QH_ALIGN, حجم(*pipe));
    بنية ehci_qtd *tds = memalign_کم(EHCI_QTD_ALIGN, حجم(*tds) * گنتی);
    فراغ *انکڑے = malloc_کم(maxpacket * گنتی);
    إذا (!pipe || !tds || !انکڑے) {
        warn_noalloc();
        اذهب fail;
    }
    memset(pipe, 0, حجم(*pipe));
    memset(tds, 0, حجم(*tds) * گنتی);
    memset(انکڑے, 0, maxpacket * گنتی);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->اگلا_td = pipe->tds = tds;
    pipe->انکڑے = انکڑے;
    pipe->qh.qtd_اگلا = (u32)tds;

    صحيح i;
    لكل (i=0; i<گنتی; i++) {
        بنية ehci_qtd *td = &tds[i];
        td->qtd_اگلا = (i==گنتی-1 ? (u32)tds : (u32)&td[1]);
        td->alt_اگلا = EHCI_PTR_رکن;
        td->token = (ehci_explen(maxpacket) | QTD_STS_سرگرم
                     | QTD_PID_وچ | ehci_maxerr(3));
        td->buf[0] = (u32)انکڑے + maxpacket * i;
    }

    // Add to interrupt schedule.
    بنية ehci_framelist *fl = (فراغ*)readl(&cntl->regs->periodiclistbase);
    إذا (frameexp == 0) {
        // Add to existing interrupt entry.
        بنية ehci_qh *intr_qh = (فراغ*)(fl->links[0] & ~EHCI_PTR_بٹ);
        pipe->qh.اگلا = intr_qh->اگلا;
        barrier();
        intr_qh->اگلا = (u32)&pipe->qh | EHCI_PTR_QH;
    } وإلا {
        صحيح startpos = 1<<(frameexp-1);
        pipe->qh.اگلا = fl->links[startpos];
        barrier();
        لكل (i=startpos; i<ARRAY_ناپ(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    أرجع &pipe->pipe;
fail:
    حرر(pipe);
    حرر(tds);
    حرر(انکڑے);
    أرجع عدم;
}

بنية usb_pipe *
ehci_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *upipe
                  , بنية usb_endpoint_descriptor *epdesc)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع عدم;
    usb_جوڑو_freelist(upipe);
    إذا (!epdesc)
        أرجع عدم;
    u8 eptype = epdesc->bm_گن & USB_ENDPOINT_XFERTYPE_نقاب;
    إذا (eptype == USB_ENDPOINT_XFER_INT)
        أرجع ehci_alloc_intr_pipe(usbdev, epdesc);
    بنية usb_ehci_s *cntl = container_دا(
        usbdev->hub->cntl, بنية usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    بنية usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    إذا (usbpipe) {
        // Use previously allocated pipe.
        بنية ehci_pipe *pipe = container_دا(usbpipe, بنية ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        أرجع usbpipe;
    }

    // Allocate a new queue head.
    بنية ehci_pipe *pipe;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, حجم(*pipe));
    وإلا
        pipe = memalign_کم(EHCI_QH_ALIGN, حجم(*pipe));
    إذا (!pipe) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(pipe, 0, حجم(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_اگلا = pipe->qh.alt_اگلا = EHCI_PTR_رکن;

    // Add queue head to controller list.
    بنية ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.اگلا = async_qh->اگلا;
    barrier();
    async_qh->اگلا = (u32)&pipe->qh | EHCI_PTR_QH;
    أرجع &pipe->pipe;
}

ساكن فراغ
ehci_reset_pipe(بنية ehci_pipe *pipe)
{
    سیٹ_LOWFLAT(pipe->qh.qtd_اگلا, EHCI_PTR_رکن);
    سیٹ_LOWFLAT(pipe->qh.alt_اگلا, EHCI_PTR_رکن);
    barrier();
    سیٹ_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ساكن صحيح
ehci_wait_td(بنية ehci_pipe *pipe, بنية ehci_qtd *td, u32 سرا)
{
    u32 حالت;
    لكل (;;) {
        حالت = td->token;
        إذا (!(حالت & QTD_STS_سرگرم))
            اكسر;
        إذا (timer_check(سرا)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 اگلا = GET_LOWFLAT(pipe->qh.qtd_اگلا);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, اگلا, td, حالت);
            ehci_reset_pipe(pipe);
            بنية usb_ehci_s *cntl = container_دا(
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
    u32 *pos = td->buf, سرا = dest + transfer;
    لكل (; dest < سرا; dest = ALIGN_تھلے(dest + ورقہ_ناپ, ورقہ_ناپ))
        *pos++ = dest;
}

#تعریف_کرو STACKQTDS 6

صحيح
ehci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *انکڑے, صحيح datasize)
{
    إذا (! CONFIG_USB_EHCI)
        أرجع -1;
    بنية ehci_pipe *pipe = container_دا(p, بنية ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, انکڑے, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[حجم(بنية ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    بنية ehci_qtd *tds = (فراغ*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, حجم(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    إذا (cmd) {
        // Send setup pid on control transfers
        td->qtd_اگلا = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_اگلا = EHCI_PTR_رکن;
        td->token = (ehci_explen(USB_CONTROL_تیاری_ناپ) | QTD_STS_سرگرم
                     | QTD_PID_تیاری | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_تیاری_ناپ);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)انکڑے, dataend = dest + datasize;
    طالما (dest < dataend) {
        // Send data pids
        إذا (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            أرجع -1;
        }
        صحيح maxtransfer = 5*ورقہ_ناپ - (dest & (ورقہ_ناپ-1));
        صحيح transfer = dataend - dest;
        إذا (transfer > maxtransfer)
            transfer = ALIGN_تھلے(maxtransfer, maxpacket);
        td->qtd_اگلا = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_اگلا = EHCI_PTR_رکن;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_سرگرم
                     | (dir ? QTD_PID_وچ : QTD_PID_باہرلا) | ehci_maxerr(3));
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
        td->qtd_اگلا = EHCI_PTR_رکن;
        td->alt_اگلا = EHCI_PTR_رکن;
        td->token = (QTD_TOGGLE | QTD_STS_سرگرم
                     | (dir ? QTD_PID_باہرلا : QTD_PID_وچ) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_اگلا = EHCI_PTR_رکن;
    barrier();
    سیٹ_LOWFLAT(pipe->qh.qtd_اگلا, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 سرا = timer_calc(usb_xfer_ویلا(p, datasize));
    صحيح i;
    لكل (i=0, td=tds; i<STACKQTDS; i++, td++) {
        صحيح ret = ehci_wait_td(pipe, td, سرا);
        إذا (ret)
            أرجع -1;
    }

    أرجع 0;
}

صحيح
ehci_poll_intr(بنية usb_pipe *p, فراغ *انکڑے)
{
    ASSERT16();
    إذا (! CONFIG_USB_EHCI)
        أرجع -1;
    بنية ehci_pipe *pipe = container_دا(p, بنية ehci_pipe, pipe);
    بنية ehci_qtd *td = GET_LOWFLAT(pipe->اگلا_td);
    u32 token = GET_LOWFLAT(td->token);
    إذا (token & QTD_STS_سرگرم)
        // No intrs found.
        أرجع -1;
    // XXX - check for errors.

    // Copy data.
    صحيح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحيح pos = td - GET_LOWFLAT(pipe->tds);
    فراغ *tddata = GET_LOWFLAT(pipe->انکڑے) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), انکڑے, SEG_کم, LOWFLAT2_کم(tddata), maxpacket);

    // Reenable this td.
    بنية ehci_qtd *اگلا = (فراغ*)(GET_LOWFLAT(td->qtd_اگلا) & ~EHCI_PTR_بٹ);
    سیٹ_LOWFLAT(pipe->اگلا_td, اگلا);
    سیٹ_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    سیٹ_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_سرگرم
                            | QTD_PID_وچ | ehci_maxerr(3)));

    أرجع 0;
}
