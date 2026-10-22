// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_LOWFLAT
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "memmap.h" // PAGE_SIZE
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#అన్తర్భావ "pci_regs.h" // PCI_BASE_ADDRESS_0
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "usb.h" // struct usb_s
#అన్తర్భావ "usb-ehci.h" // struct ehci_qh
#అన్తర్భావ "util.h" // msleep
#అన్తర్భావ "x86.h" // readl

నిర్మాణం usb_ehci_s {
    నిర్మాణం usb_s usb;
    నిర్మాణం ehci_caps *caps;
    నిర్మాణం ehci_regs *regs;
    నిర్మాణం ehci_qh *async_qh;
    పూర్ణాంకం checkports;
};

నిర్మాణం ehci_pipe {
    నిర్మాణం ehci_qh qh;
    నిర్మాణం ehci_qtd *తదుపరి_td, *tds;
    శూన్య *దత్తాంశం;
    నిర్మాణం usb_pipe pipe;
};

స్థిర పూర్ణాంకం PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#నిర్వచించు EHCI_సమయం_POSTPOWER 20
#నిర్వచించు EHCI_సమయం_POSTRESET 2

// Check if device attached to port
స్థిర పూర్ణాంకం
ehci_hub_detect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_ehci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[పోర్టు];
    u32 portsc = readl(portreg);

    ఒకవేళ (!(portsc & పోర్టు_CONNECT))
        // No device present
        తిరిగి 0;

    ఒకవేళ ((portsc & పోర్టు_LINESTATUS_ముసుగు) == పోర్టు_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | పోర్టు_OWNER);
        తిరిగి -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~పోర్టు_PE) | పోర్టు_RESET;
    writel(portreg, portsc);
    msleep(USB_సమయం_DRSTR);
    తిరిగి 1;
}

// Reset device on port
స్థిర పూర్ణాంకం
ehci_hub_reset(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_ehci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[పోర్టు];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~పోర్టు_RESET;
    writel(portreg, portsc);
    msleep(EHCI_సమయం_POSTRESET);

    portsc = readl(portreg);
    ఒకవేళ (!(portsc & పోర్టు_CONNECT))
        // No longer connected
        తిరిగి -1;
    ఒకవేళ (!(portsc & పోర్టు_PE)) {
        // full speed device
        writel(portreg, portsc | పోర్టు_OWNER);
        తిరిగి -1;
    }

    తిరిగి USB_HIGHSPEED;
}

// Disable port
స్థిర శూన్య
ehci_hub_disconnect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_ehci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[పోర్టు];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~పోర్టు_PE);
}

స్థిర నిర్మాణం usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
స్థిర పూర్ణాంకం
check_ehci_ports(నిర్మాణం usb_ehci_s *cntl)
{
    // Power up ports.
    పూర్ణాంకం i;
    కొరకు (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        ఒకవేళ (!(portsc & పోర్టు_POWER)) {
            portsc |= పోర్టు_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_సమయం_POSTPOWER);

    నిర్మాణం usbhub_s hub;
    memset(&hub, 0, పరిమాణం(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    తిరిగి hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
స్థిర శూన్య
ehci_waittick(నిర్మాణం usb_ehci_s *cntl)
{
    ఒకవేళ (MODE16) {
        msleep(10);
        తిరిగి;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 చివర = timer_calc(100);
    కొరకు (;;) {
        sts = readl(&cntl->regs->usbsts);
        ఒకవేళ (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            ఒకవేళ (!(cmd & CMD_IAAD))
                ఆపు;
        }
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    కొరకు (;;) {
        sts = readl(&cntl->regs->usbsts);
        ఒకవేళ (sts & STS_IAA)
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

స్థిర శూన్య
ehci_free_pipes(నిర్మాణం usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    నిర్మాణం ehci_qh *ప్రారంభం = cntl->async_qh;
    నిర్మాణం ehci_qh *pos = ప్రారంభం;
    కొరకు (;;) {
        నిర్మాణం ehci_qh *తదుపరి = (శూన్య*)(pos->తదుపరి & ~EHCI_PTR_బిట్లు);
        ఒకవేళ (తదుపరి == ప్రారంభం)
            ఆపు;
        నిర్మాణం ehci_pipe *pipe = container_యొక్క(తదుపరి, నిర్మాణం ehci_pipe, qh);
        ఒకవేళ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->తదుపరి = తదుపరి->తదుపరి;
        లేకపోతే
            pos = తదుపరి;
    }
    ehci_waittick(cntl);
    కొరకు (;;) {
        నిర్మాణం usb_pipe *usbpipe = cntl->usb.freelist;
        ఒకవేళ (!usbpipe)
            ఆపు;
        cntl->usb.freelist = usbpipe->freenext;
        నిర్మాణం ehci_pipe *pipe = container_యొక్క(usbpipe, నిర్మాణం ehci_pipe, pipe);
        విడుదల(pipe);
    }
}

స్థిర శూన్య
configure_ehci(శూన్య *దత్తాంశం)
{
    నిర్మాణం usb_ehci_s *cntl = దత్తాంశం;

    // Allocate ram for schedule storage
    నిర్మాణం ehci_framelist *fl = memalign_ఎగువ(పరిమాణం(*fl), పరిమాణం(*fl));
    నిర్మాణం ehci_qh *intr_qh = memalign_ఎగువ(EHCI_QH_ALIGN, పరిమాణం(*intr_qh));
    నిర్మాణం ehci_qh *async_qh = memalign_ఎగువ(EHCI_QH_ALIGN, పరిమాణం(*async_qh));
    ఒకవేళ (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        వెళ్ళు fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 చివర = timer_calc(250);
    కొరకు (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        ఒకవేళ (!(cmd & CMD_HCRESET))
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            PendingEHCI--;
            వెళ్ళు fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, పరిమాణం(*intr_qh));
    intr_qh->తదుపరి = EHCI_PTR_పదం;
    intr_qh->info2 = (0x01 << QH_SMASK_జరుపు);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_తదుపరి = intr_qh->alt_తదుపరి = EHCI_PTR_పదం;
    పూర్ణాంకం i;
    కొరకు (i=0; i<ARRAY_కొలత(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, పరిమాణం(*async_qh));
    async_qh->తదుపరి = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_తల;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_తదుపరి = async_qh->alt_తదుపరి = EHCI_PTR_పదం;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_నిరంతరం);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    పూర్ణాంకం లెక్క = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    ఒకవేళ (లెక్క)
        // Success
        తిరిగి;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_నిరంతరం);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    విడుదల(fl);
    విడుదల(intr_qh);
    విడుదల(async_qh);
    విడుదల(cntl);
}

స్థిర శూన్య
ehci_controller_అమరిక(నిర్మాణం pci_device *pci)
{
    నిర్మాణం ehci_caps *caps = pci_enable_membar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!caps)
        తిరిగి;
    u32 hcc_params = readl(&caps->hccparams);

    నిర్మాణం usb_ehci_s *cntl = malloc_tmphigh(పరిమాణం(*cntl));
    ఒకవేళ (!cntl) {
        warn_noalloc();
        తిరిగి;
    }
    memset(cntl, 0, పరిమాణం(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_ముసుగు;
    cntl->regs = (శూన్య*)caps + readb(&caps->caplength);
    ఒకవేళ (hcc_params & HCC_64_బిట్టు_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    నిరంతరం_thread(configure_ehci, cntl);
}

శూన్య
ehci_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_USB_EHCI)
        తిరిగి;
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_అమరిక(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
శూన్య
ehci_wait_controllers(శూన్య)
{
    అయితే (CONFIG_USB_EHCI && CONFIG_దారాలు && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
స్థిర శూన్య
ehci_desc2pipe(నిర్మాణం ehci_pipe *pipe, నిర్మాణం usbdevice_s *usbdev
               , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_జరుపు)
                      | (pipe->pipe.వేగం << QH_వేగం_జరుపు)
                      | (pipe->pipe.ep << QH_EP_జరుపు)
                      | (pipe->pipe.devaddr << QH_DEVADDR_జరుపు));

    pipe->qh.info2 = (1 << QH_MULT_జరుపు);
    నిర్మాణం usbdevice_s *hubdev = usbdev->hub->usbdev;
    ఒకవేళ (hubdev) {
        నిర్మాణం ehci_pipe *hpipe = container_యొక్క(
            hubdev->defpipe, నిర్మాణం ehci_pipe, pipe);
        ఒకవేళ (hpipe->pipe.వేగం == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->పోర్టు+1) << QH_HUBPORT_జరుపు)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_జరుపు));
        లేకపోతే
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.వేగం != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    లేకపోతే ఒకవేళ (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_జరుపు) | (0x1c << QH_CMASK_జరుపు);
}

స్థిర నిర్మాణం usb_pipe *
ehci_alloc_intr_pipe(నిర్మాణం usbdevice_s *usbdev
                     , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    నిర్మాణం usb_ehci_s *cntl = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_ehci_s, usb);
    పూర్ణాంకం frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ఒకవేళ (frameexp > 10)
        frameexp = 10;
    పూర్ణాంకం maxpacket = epdesc->w_Max_Packet_కొలత;
    // Determine number of entries needed for 2 timer ticks.
    పూర్ణాంకం ms = 1<<frameexp;
    పూర్ణాంకం లెక్క = DIV_విడత_పైకి(ticks_గమ్యం_ms(2), ms);
    నిర్మాణం ehci_pipe *pipe = memalign_దిగువ(EHCI_QH_ALIGN, పరిమాణం(*pipe));
    నిర్మాణం ehci_qtd *tds = memalign_దిగువ(EHCI_QTD_ALIGN, పరిమాణం(*tds) * లెక్క);
    శూన్య *దత్తాంశం = malloc_దిగువ(maxpacket * లెక్క);
    ఒకవేళ (!pipe || !tds || !దత్తాంశం) {
        warn_noalloc();
        వెళ్ళు fail;
    }
    memset(pipe, 0, పరిమాణం(*pipe));
    memset(tds, 0, పరిమాణం(*tds) * లెక్క);
    memset(దత్తాంశం, 0, maxpacket * లెక్క);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->తదుపరి_td = pipe->tds = tds;
    pipe->దత్తాంశం = దత్తాంశం;
    pipe->qh.qtd_తదుపరి = (u32)tds;

    పూర్ణాంకం i;
    కొరకు (i=0; i<లెక్క; i++) {
        నిర్మాణం ehci_qtd *td = &tds[i];
        td->qtd_తదుపరి = (i==లెక్క-1 ? (u32)tds : (u32)&td[1]);
        td->alt_తదుపరి = EHCI_PTR_పదం;
        td->token = (ehci_explen(maxpacket) | QTD_STS_చురుకైన
                     | QTD_PID_లోపల | ehci_maxerr(3));
        td->buf[0] = (u32)దత్తాంశం + maxpacket * i;
    }

    // Add to interrupt schedule.
    నిర్మాణం ehci_framelist *fl = (శూన్య*)readl(&cntl->regs->periodiclistbase);
    ఒకవేళ (frameexp == 0) {
        // Add to existing interrupt entry.
        నిర్మాణం ehci_qh *intr_qh = (శూన్య*)(fl->links[0] & ~EHCI_PTR_బిట్లు);
        pipe->qh.తదుపరి = intr_qh->తదుపరి;
        barrier();
        intr_qh->తదుపరి = (u32)&pipe->qh | EHCI_PTR_QH;
    } లేకపోతే {
        పూర్ణాంకం startpos = 1<<(frameexp-1);
        pipe->qh.తదుపరి = fl->links[startpos];
        barrier();
        కొరకు (i=startpos; i<ARRAY_కొలత(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    తిరిగి &pipe->pipe;
fail:
    విడుదల(pipe);
    విడుదల(tds);
    విడుదల(దత్తాంశం);
    తిరిగి ఖాళీ;
}

నిర్మాణం usb_pipe *
ehci_realloc_pipe(నిర్మాణం usbdevice_s *usbdev, నిర్మాణం usb_pipe *upipe
                  , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    ఒకవేళ (! CONFIG_USB_EHCI)
        తిరిగి ఖాళీ;
    usb_కూడు_freelist(upipe);
    ఒకవేళ (!epdesc)
        తిరిగి ఖాళీ;
    u8 eptype = epdesc->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_INT)
        తిరిగి ehci_alloc_intr_pipe(usbdev, epdesc);
    నిర్మాణం usb_ehci_s *cntl = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    నిర్మాణం usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ఒకవేళ (usbpipe) {
        // Use previously allocated pipe.
        నిర్మాణం ehci_pipe *pipe = container_యొక్క(usbpipe, నిర్మాణం ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        తిరిగి usbpipe;
    }

    // Allocate a new queue head.
    నిర్మాణం ehci_pipe *pipe;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, పరిమాణం(*pipe));
    లేకపోతే
        pipe = memalign_దిగువ(EHCI_QH_ALIGN, పరిమాణం(*pipe));
    ఒకవేళ (!pipe) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(pipe, 0, పరిమాణం(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_తదుపరి = pipe->qh.alt_తదుపరి = EHCI_PTR_పదం;

    // Add queue head to controller list.
    నిర్మాణం ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.తదుపరి = async_qh->తదుపరి;
    barrier();
    async_qh->తదుపరి = (u32)&pipe->qh | EHCI_PTR_QH;
    తిరిగి &pipe->pipe;
}

స్థిర శూన్య
ehci_reset_pipe(నిర్మాణం ehci_pipe *pipe)
{
    సమితి_LOWFLAT(pipe->qh.qtd_తదుపరి, EHCI_PTR_పదం);
    సమితి_LOWFLAT(pipe->qh.alt_తదుపరి, EHCI_PTR_పదం);
    barrier();
    సమితి_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

స్థిర పూర్ణాంకం
ehci_wait_td(నిర్మాణం ehci_pipe *pipe, నిర్మాణం ehci_qtd *td, u32 చివర)
{
    u32 స్థితి_2;
    కొరకు (;;) {
        స్థితి_2 = td->token;
        ఒకవేళ (!(స్థితి_2 & QTD_STS_చురుకైన))
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 తదుపరి = GET_LOWFLAT(pipe->qh.qtd_తదుపరి);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, తదుపరి, td, స్థితి_2);
            ehci_reset_pipe(pipe);
            నిర్మాణం usb_ehci_s *cntl = container_యొక్క(
                GET_LOWFLAT(pipe->pipe.cntl), నిర్మాణం usb_ehci_s, usb);
            ehci_waittick(cntl);
            తిరిగి -1;
        }
        yield();
    }
    ఒకవేళ (స్థితి_2 & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", స్థితి_2);
        ehci_reset_pipe(pipe);
        తిరిగి -2;
    }
    తిరిగి 0;
}

స్థిర శూన్య
ehci_fill_tdbuf(నిర్మాణం ehci_qtd *td, u32 dest, పూర్ణాంకం transfer)
{
    u32 *pos = td->buf, చివర = dest + transfer;
    కొరకు (; dest < చివర; dest = ALIGN_కిందికి(dest + పుట_కొలత, పుట_కొలత))
        *pos++ = dest;
}

#నిర్వచించు STACKQTDS 6

పూర్ణాంకం
ehci_send_pipe(నిర్మాణం usb_pipe *p, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
               , శూన్య *దత్తాంశం, పూర్ణాంకం datasize)
{
    ఒకవేళ (! CONFIG_USB_EHCI)
        తిరిగి -1;
    నిర్మాణం ehci_pipe *pipe = container_యొక్క(p, నిర్మాణం ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, దత్తాంశం, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[పరిమాణం(నిర్మాణం ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    నిర్మాణం ehci_qtd *tds = (శూన్య*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, పరిమాణం(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    ఒకవేళ (cmd) {
        // Send setup pid on control transfers
        td->qtd_తదుపరి = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_తదుపరి = EHCI_PTR_పదం;
        td->token = (ehci_explen(USB_CONTROL_అమరిక_కొలత) | QTD_STS_చురుకైన
                     | QTD_PID_అమరిక | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_అమరిక_కొలత);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)దత్తాంశం, dataend = dest + datasize;
    అయితే (dest < dataend) {
        // Send data pids
        ఒకవేళ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            తిరిగి -1;
        }
        పూర్ణాంకం maxtransfer = 5*పుట_కొలత - (dest & (పుట_కొలత-1));
        పూర్ణాంకం transfer = dataend - dest;
        ఒకవేళ (transfer > maxtransfer)
            transfer = ALIGN_కిందికి(maxtransfer, maxpacket);
        td->qtd_తదుపరి = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_తదుపరి = EHCI_PTR_పదం;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_చురుకైన
                     | (dir ? QTD_PID_లోపల : QTD_PID_బహిర్గతం) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    ఒకవేళ (cmd) {
        // Send status pid on control transfers
        ఒకవేళ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            తిరిగి -1;
        }
        td->qtd_తదుపరి = EHCI_PTR_పదం;
        td->alt_తదుపరి = EHCI_PTR_పదం;
        td->token = (QTD_TOGGLE | QTD_STS_చురుకైన
                     | (dir ? QTD_PID_బహిర్గతం : QTD_PID_లోపల) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_తదుపరి = EHCI_PTR_పదం;
    barrier();
    సమితి_LOWFLAT(pipe->qh.qtd_తదుపరి, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 చివర = timer_calc(usb_xfer_సమయం(p, datasize));
    పూర్ణాంకం i;
    కొరకు (i=0, td=tds; i<STACKQTDS; i++, td++) {
        పూర్ణాంకం ret = ehci_wait_td(pipe, td, చివర);
        ఒకవేళ (ret)
            తిరిగి -1;
    }

    తిరిగి 0;
}

పూర్ణాంకం
ehci_poll_intr(నిర్మాణం usb_pipe *p, శూన్య *దత్తాంశం)
{
    ASSERT16();
    ఒకవేళ (! CONFIG_USB_EHCI)
        తిరిగి -1;
    నిర్మాణం ehci_pipe *pipe = container_యొక్క(p, నిర్మాణం ehci_pipe, pipe);
    నిర్మాణం ehci_qtd *td = GET_LOWFLAT(pipe->తదుపరి_td);
    u32 token = GET_LOWFLAT(td->token);
    ఒకవేళ (token & QTD_STS_చురుకైన)
        // No intrs found.
        తిరిగి -1;
    // XXX - check for errors.

    // Copy data.
    పూర్ణాంకం maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    పూర్ణాంకం pos = td - GET_LOWFLAT(pipe->tds);
    శూన్య *tddata = GET_LOWFLAT(pipe->దత్తాంశం) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), దత్తాంశం, SEG_దిగువ, LOWFLAT2_దిగువ(tddata), maxpacket);

    // Reenable this td.
    నిర్మాణం ehci_qtd *తదుపరి = (శూన్య*)(GET_LOWFLAT(td->qtd_తదుపరి) & ~EHCI_PTR_బిట్లు);
    సమితి_LOWFLAT(pipe->తదుపరి_td, తదుపరి);
    సమితి_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    సమితి_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_చురుకైన
                            | QTD_PID_లోపల | ehci_maxerr(3)));

    తిరిగి 0;
}
