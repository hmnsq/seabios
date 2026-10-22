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

ساخت usb_ehci_s {
    ساخت usb_s usb;
    ساخت ehci_caps *caps;
    ساخت ehci_regs *regs;
    ساخت ehci_qh *async_qh;
    صحیح checkports;
};

ساخت ehci_pipe {
    ساخت ehci_qh qh;
    ساخت ehci_qtd *اگلا_td, *tds;
    خالی *ڈیٹا;
    ساخت usb_pipe pipe;
};

ساکن صحیح PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#تعریف_کرو EHCI_وقت_POSTPOWER 20
#تعریف_کرو EHCI_وقت_POSTRESET 2

// Check if device attached to port
ساکن صحیح
ehci_hub_detect(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_ehci_s *cntl = container_کا(hub->cntl, ساخت usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٹ];
    u32 portsc = readl(portreg);

    اگر (!(portsc & پورٹ_CONNECT))
        // No device present
        واپس 0;

    اگر ((portsc & پورٹ_LINESTATUS_نقاب) == پورٹ_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | پورٹ_OWNER);
        واپس -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~پورٹ_PE) | پورٹ_RESET;
    writel(portreg, portsc);
    msleep(USB_وقت_DRSTR);
    واپس 1;
}

// Reset device on port
ساکن صحیح
ehci_hub_reset(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_ehci_s *cntl = container_کا(hub->cntl, ساخت usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٹ];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~پورٹ_RESET;
    writel(portreg, portsc);
    msleep(EHCI_وقت_POSTRESET);

    portsc = readl(portreg);
    اگر (!(portsc & پورٹ_CONNECT))
        // No longer connected
        واپس -1;
    اگر (!(portsc & پورٹ_PE)) {
        // full speed device
        writel(portreg, portsc | پورٹ_OWNER);
        واپس -1;
    }

    واپس USB_HIGHSPEED;
}

// Disable port
ساکن خالی
ehci_hub_disconnect(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_ehci_s *cntl = container_کا(hub->cntl, ساخت usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[پورٹ];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~پورٹ_PE);
}

ساکن ساخت usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساکن صحیح
check_ehci_ports(ساخت usb_ehci_s *cntl)
{
    // Power up ports.
    صحیح i;
    برائے (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        اگر (!(portsc & پورٹ_POWER)) {
            portsc |= پورٹ_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_وقت_POSTPOWER);

    ساخت usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    واپس hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
ساکن خالی
ehci_waittick(ساخت usb_ehci_s *cntl)
{
    اگر (MODE16) {
        msleep(10);
        واپس;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 انتہا = timer_calc(100);
    برائے (;;) {
        sts = readl(&cntl->regs->usbsts);
        اگر (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            اگر (!(cmd & CMD_IAAD))
                توڑو;
        }
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    برائے (;;) {
        sts = readl(&cntl->regs->usbsts);
        اگر (sts & STS_IAA)
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

ساکن خالی
ehci_free_pipes(ساخت usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    ساخت ehci_qh *آغاز = cntl->async_qh;
    ساخت ehci_qh *pos = آغاز;
    برائے (;;) {
        ساخت ehci_qh *اگلا = (خالی*)(pos->اگلا & ~EHCI_PTR_بٹ);
        اگر (اگلا == آغاز)
            توڑو;
        ساخت ehci_pipe *pipe = container_کا(اگلا, ساخت ehci_pipe, qh);
        اگر (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->اگلا = اگلا->اگلا;
        ورنہ
            pos = اگلا;
    }
    ehci_waittick(cntl);
    برائے (;;) {
        ساخت usb_pipe *usbpipe = cntl->usb.freelist;
        اگر (!usbpipe)
            توڑو;
        cntl->usb.freelist = usbpipe->freenext;
        ساخت ehci_pipe *pipe = container_کا(usbpipe, ساخت ehci_pipe, pipe);
        آزاد(pipe);
    }
}

ساکن خالی
configure_ehci(خالی *ڈیٹا)
{
    ساخت usb_ehci_s *cntl = ڈیٹا;

    // Allocate ram for schedule storage
    ساخت ehci_framelist *fl = memalign_بلند(حجم(*fl), حجم(*fl));
    ساخت ehci_qh *intr_qh = memalign_بلند(EHCI_QH_ALIGN, حجم(*intr_qh));
    ساخت ehci_qh *async_qh = memalign_بلند(EHCI_QH_ALIGN, حجم(*async_qh));
    اگر (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        جاؤ fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 انتہا = timer_calc(250);
    برائے (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        اگر (!(cmd & CMD_HCRESET))
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            PendingEHCI--;
            جاؤ fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, حجم(*intr_qh));
    intr_qh->اگلا = EHCI_PTR_رکن;
    intr_qh->info2 = (0x01 << QH_SMASK_سرکاؤ);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_اگلا = intr_qh->alt_اگلا = EHCI_PTR_رکن;
    صحیح i;
    برائے (i=0; i<ARRAY_سائز(fl->links); i++)
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
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_تسلسل);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    صحیح گنتی = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    اگر (گنتی)
        // Success
        واپس;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_تسلسل);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    آزاد(fl);
    آزاد(intr_qh);
    آزاد(async_qh);
    آزاد(cntl);
}

ساکن خالی
ehci_controller_تیاری(ساخت pci_device *pci)
{
    ساخت ehci_caps *caps = pci_enable_membar(pci, PCI_بنیاد_پتہ_0);
    اگر (!caps)
        واپس;
    u32 hcc_params = readl(&caps->hccparams);

    ساخت usb_ehci_s *cntl = malloc_tmphigh(حجم(*cntl));
    اگر (!cntl) {
        warn_noalloc();
        واپس;
    }
    memset(cntl, 0, حجم(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_نقاب;
    cntl->regs = (خالی*)caps + readb(&caps->caplength);
    اگر (hcc_params & HCC_64_ثنائی_ہندسہ_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    تسلسل_thread(configure_ehci, cntl);
}

خالی
ehci_تیاری(خالی)
{
    اگر (! CONFIG_USB_EHCI)
        واپس;
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_تیاری(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
خالی
ehci_wait_controllers(خالی)
{
    جب (CONFIG_USB_EHCI && CONFIG_دھاگے && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
ساکن خالی
ehci_desc2pipe(ساخت ehci_pipe *pipe, ساخت usbdevice_s *usbdev
               , ساخت usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_سرکاؤ)
                      | (pipe->pipe.رفتار << QH_رفتار_سرکاؤ)
                      | (pipe->pipe.ep << QH_EP_سرکاؤ)
                      | (pipe->pipe.devaddr << QH_DEVADDR_سرکاؤ));

    pipe->qh.info2 = (1 << QH_MULT_سرکاؤ);
    ساخت usbdevice_s *hubdev = usbdev->hub->usbdev;
    اگر (hubdev) {
        ساخت ehci_pipe *hpipe = container_کا(
            hubdev->defpipe, ساخت ehci_pipe, pipe);
        اگر (hpipe->pipe.رفتار == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->پورٹ+1) << QH_HUBPORT_سرکاؤ)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_سرکاؤ));
        ورنہ
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.رفتار != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    ورنہ اگر (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_سرکاؤ) | (0x1c << QH_CMASK_سرکاؤ);
}

ساکن ساخت usb_pipe *
ehci_alloc_intr_pipe(ساخت usbdevice_s *usbdev
                     , ساخت usb_endpoint_descriptor *epdesc)
{
    ساخت usb_ehci_s *cntl = container_کا(
        usbdev->hub->cntl, ساخت usb_ehci_s, usb);
    صحیح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    اگر (frameexp > 10)
        frameexp = 10;
    صحیح maxpacket = epdesc->w_Max_Packet_سائز;
    // Determine number of entries needed for 2 timer ticks.
    صحیح ms = 1<<frameexp;
    صحیح گنتی = DIV_باری_اوپر(ticks_مقصد_ms(2), ms);
    ساخت ehci_pipe *pipe = memalign_کم(EHCI_QH_ALIGN, حجم(*pipe));
    ساخت ehci_qtd *tds = memalign_کم(EHCI_QTD_ALIGN, حجم(*tds) * گنتی);
    خالی *ڈیٹا = malloc_کم(maxpacket * گنتی);
    اگر (!pipe || !tds || !ڈیٹا) {
        warn_noalloc();
        جاؤ fail;
    }
    memset(pipe, 0, حجم(*pipe));
    memset(tds, 0, حجم(*tds) * گنتی);
    memset(ڈیٹا, 0, maxpacket * گنتی);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->اگلا_td = pipe->tds = tds;
    pipe->ڈیٹا = ڈیٹا;
    pipe->qh.qtd_اگلا = (u32)tds;

    صحیح i;
    برائے (i=0; i<گنتی; i++) {
        ساخت ehci_qtd *td = &tds[i];
        td->qtd_اگلا = (i==گنتی-1 ? (u32)tds : (u32)&td[1]);
        td->alt_اگلا = EHCI_PTR_رکن;
        td->token = (ehci_explen(maxpacket) | QTD_STS_فعال
                     | QTD_PID_اندر | ehci_maxerr(3));
        td->buf[0] = (u32)ڈیٹا + maxpacket * i;
    }

    // Add to interrupt schedule.
    ساخت ehci_framelist *fl = (خالی*)readl(&cntl->regs->periodiclistbase);
    اگر (frameexp == 0) {
        // Add to existing interrupt entry.
        ساخت ehci_qh *intr_qh = (خالی*)(fl->links[0] & ~EHCI_PTR_بٹ);
        pipe->qh.اگلا = intr_qh->اگلا;
        barrier();
        intr_qh->اگلا = (u32)&pipe->qh | EHCI_PTR_QH;
    } ورنہ {
        صحیح startpos = 1<<(frameexp-1);
        pipe->qh.اگلا = fl->links[startpos];
        barrier();
        برائے (i=startpos; i<ARRAY_سائز(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    واپس &pipe->pipe;
fail:
    آزاد(pipe);
    آزاد(tds);
    آزاد(ڈیٹا);
    واپس تہی;
}

ساخت usb_pipe *
ehci_realloc_pipe(ساخت usbdevice_s *usbdev, ساخت usb_pipe *upipe
                  , ساخت usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_EHCI)
        واپس تہی;
    usb_جوڑو_freelist(upipe);
    اگر (!epdesc)
        واپس تہی;
    u8 eptype = epdesc->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب;
    اگر (eptype == USB_ENDPOINT_XFER_INT)
        واپس ehci_alloc_intr_pipe(usbdev, epdesc);
    ساخت usb_ehci_s *cntl = container_کا(
        usbdev->hub->cntl, ساخت usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ساخت usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    اگر (usbpipe) {
        // Use previously allocated pipe.
        ساخت ehci_pipe *pipe = container_کا(usbpipe, ساخت ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        واپس usbpipe;
    }

    // Allocate a new queue head.
    ساخت ehci_pipe *pipe;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, حجم(*pipe));
    ورنہ
        pipe = memalign_کم(EHCI_QH_ALIGN, حجم(*pipe));
    اگر (!pipe) {
        warn_noalloc();
        واپس تہی;
    }
    memset(pipe, 0, حجم(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_اگلا = pipe->qh.alt_اگلا = EHCI_PTR_رکن;

    // Add queue head to controller list.
    ساخت ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.اگلا = async_qh->اگلا;
    barrier();
    async_qh->اگلا = (u32)&pipe->qh | EHCI_PTR_QH;
    واپس &pipe->pipe;
}

ساکن خالی
ehci_reset_pipe(ساخت ehci_pipe *pipe)
{
    سیٹ_LOWFLAT(pipe->qh.qtd_اگلا, EHCI_PTR_رکن);
    سیٹ_LOWFLAT(pipe->qh.alt_اگلا, EHCI_PTR_رکن);
    barrier();
    سیٹ_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

ساکن صحیح
ehci_wait_td(ساخت ehci_pipe *pipe, ساخت ehci_qtd *td, u32 انتہا)
{
    u32 حالت_2;
    برائے (;;) {
        حالت_2 = td->token;
        اگر (!(حالت_2 & QTD_STS_فعال))
            توڑو;
        اگر (timer_check(انتہا)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 اگلا = GET_LOWFLAT(pipe->qh.qtd_اگلا);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, اگلا, td, حالت_2);
            ehci_reset_pipe(pipe);
            ساخت usb_ehci_s *cntl = container_کا(
                GET_LOWFLAT(pipe->pipe.cntl), ساخت usb_ehci_s, usb);
            ehci_waittick(cntl);
            واپس -1;
        }
        yield();
    }
    اگر (حالت_2 & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", حالت_2);
        ehci_reset_pipe(pipe);
        واپس -2;
    }
    واپس 0;
}

ساکن خالی
ehci_fill_tdbuf(ساخت ehci_qtd *td, u32 dest, صحیح transfer)
{
    u32 *pos = td->buf, انتہا = dest + transfer;
    برائے (; dest < انتہا; dest = ALIGN_نیچے_کو(dest + صفحہ_سائز, صفحہ_سائز))
        *pos++ = dest;
}

#تعریف_کرو STACKQTDS 6

صحیح
ehci_send_pipe(ساخت usb_pipe *p, صحیح dir, ثابت خالی *cmd
               , خالی *ڈیٹا, صحیح datasize)
{
    اگر (! CONFIG_USB_EHCI)
        واپس -1;
    ساخت ehci_pipe *pipe = container_کا(p, ساخت ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, ڈیٹا, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[حجم(ساخت ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    ساخت ehci_qtd *tds = (خالی*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, حجم(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    اگر (cmd) {
        // Send setup pid on control transfers
        td->qtd_اگلا = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_اگلا = EHCI_PTR_رکن;
        td->token = (ehci_explen(USB_CONTROL_تیاری_سائز) | QTD_STS_فعال
                     | QTD_PID_تیاری | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_تیاری_سائز);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)ڈیٹا, dataend = dest + datasize;
    جب (dest < dataend) {
        // Send data pids
        اگر (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            واپس -1;
        }
        صحیح maxtransfer = 5*صفحہ_سائز - (dest & (صفحہ_سائز-1));
        صحیح transfer = dataend - dest;
        اگر (transfer > maxtransfer)
            transfer = ALIGN_نیچے_کو(maxtransfer, maxpacket);
        td->qtd_اگلا = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_اگلا = EHCI_PTR_رکن;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_فعال
                     | (dir ? QTD_PID_اندر : QTD_PID_برآمد) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    اگر (cmd) {
        // Send status pid on control transfers
        اگر (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            واپس -1;
        }
        td->qtd_اگلا = EHCI_PTR_رکن;
        td->alt_اگلا = EHCI_PTR_رکن;
        td->token = (QTD_TOGGLE | QTD_STS_فعال
                     | (dir ? QTD_PID_برآمد : QTD_PID_اندر) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_اگلا = EHCI_PTR_رکن;
    barrier();
    سیٹ_LOWFLAT(pipe->qh.qtd_اگلا, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 انتہا = timer_calc(usb_xfer_وقت(p, datasize));
    صحیح i;
    برائے (i=0, td=tds; i<STACKQTDS; i++, td++) {
        صحیح ret = ehci_wait_td(pipe, td, انتہا);
        اگر (ret)
            واپس -1;
    }

    واپس 0;
}

صحیح
ehci_poll_intr(ساخت usb_pipe *p, خالی *ڈیٹا)
{
    ASSERT16();
    اگر (! CONFIG_USB_EHCI)
        واپس -1;
    ساخت ehci_pipe *pipe = container_کا(p, ساخت ehci_pipe, pipe);
    ساخت ehci_qtd *td = GET_LOWFLAT(pipe->اگلا_td);
    u32 token = GET_LOWFLAT(td->token);
    اگر (token & QTD_STS_فعال)
        // No intrs found.
        واپس -1;
    // XXX - check for errors.

    // Copy data.
    صحیح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحیح pos = td - GET_LOWFLAT(pipe->tds);
    خالی *tddata = GET_LOWFLAT(pipe->ڈیٹا) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), ڈیٹا, SEG_کم, LOWFLAT2_کم(tddata), maxpacket);

    // Reenable this td.
    ساخت ehci_qtd *اگلا = (خالی*)(GET_LOWFLAT(td->qtd_اگلا) & ~EHCI_PTR_بٹ);
    سیٹ_LOWFLAT(pipe->اگلا_td, اگلا);
    سیٹ_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    سیٹ_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_فعال
                            | QTD_PID_اندر | ehci_maxerr(3)));

    واپس 0;
}
