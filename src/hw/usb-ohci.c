// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_LOWFLAT
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "memmap.h" // PAGE_SIZE
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#అన్తర్భావ "pci_regs.h" // PCI_BASE_ADDRESS_0
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "usb.h" // struct usb_s
#అన్తర్భావ "usb-ehci.h" // ehci_wait_controllers
#అన్తర్భావ "usb-ohci.h" // struct ohci_hcca
#అన్తర్భావ "util.h" // msleep
#అన్తర్భావ "x86.h" // readl

#నిర్వచించు FIT                     (1 << 31)

నిర్మాణం usb_ohci_s {
    నిర్మాణం usb_s usb;
    నిర్మాణం ohci_regs *regs;
};

నిర్మాణం ohci_pipe {
    నిర్మాణం ohci_ed ed;
    నిర్మాణం usb_pipe pipe;
    నిర్మాణం ohci_regs *regs;
    శూన్య *దత్తాంశం;
    పూర్ణాంకం లెక్క;
    నిర్మాణం ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
స్థిర పూర్ణాంకం
ohci_hub_detect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_ohci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[పోర్టు]);
    తిరిగి (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
స్థిర శూన్య
ohci_hub_disconnect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_ohci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[పోర్టు], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
స్థిర పూర్ణాంకం
ohci_hub_reset(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_ohci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[పోర్టు], RH_PS_PRS);
    u32 sts;
    u32 చివర = timer_calc(USB_సమయం_DRSTR * 2);
    కొరకు (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[పోర్టు]);
        ఒకవేళ (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, పోర్టు);
            తిరిగి -1;
        }
        yield();
    }

    ఒకవేళ ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        తిరిగి -1;

    తిరిగి !!(sts & RH_PS_LSDA);
}

స్థిర నిర్మాణం usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
స్థిర పూర్ణాంకం
check_ohci_ports(నిర్మాణం usb_ohci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();
    // Turn on power for all devices on roothub.
    u32 rha = readl(&cntl->regs->roothub_a);
    rha &= ~(RH_A_PSM | RH_A_OCPM);
    writel(&cntl->regs->roothub_స్థితి, RH_HS_LPSC);
    writel(&cntl->regs->roothub_b, RH_B_PPCM);
    msleep((rha >> 24) * 2);
    // XXX - need to sleep for USB_TIME_SIGATT if just powered up?

    నిర్మాణం usbhub_s hub;
    memset(&hub, 0, పరిమాణం(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    తిరిగి hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
స్థిర శూన్య
ohci_waittick(నిర్మాణం ohci_regs *regs)
{
    barrier();
    నిర్మాణం ohci_hcca *hcca = (శూన్య*)regs->hcca;
    u32 startframe = hcca->చట్రం_no;
    u32 చివర = timer_calc(1000 * 5);
    కొరకు (;;) {
        ఒకవేళ (hcca->చట్రం_no != startframe)
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి;
        }
        yield();
    }
}

స్థిర శూన్య
ohci_free_pipes(నిర్మాణం usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    ఒకవేళ (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    కొరకు (;;) {
        నిర్మాణం ohci_ed *తదుపరి = (శూన్య*)*pos;
        ఒకవేళ (!తదుపరి)
            ఆపు;
        నిర్మాణం ohci_pipe *pipe = container_యొక్క(తదుపరి, నిర్మాణం ohci_pipe, ed);
        ఒకవేళ (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = తదుపరి->hw_తదుపరి_ED;
            విడుదల(pipe);
        } లేకపోతే {
            pos = &తదుపరి->hw_తదుపరి_ED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = ఖాళీ;
}

స్థిర పూర్ణాంకం
ప్రారంభం_ohci(నిర్మాణం usb_ohci_s *cntl, నిర్మాణం ohci_hcca *hcca)
{
    u32 oldfminterval = readl(&cntl->regs->fminterval);
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;

    // XXX - check if already running?

    // Do reset
    writel(&cntl->regs->control, OHCI_USB_RESET | oldrwc);
    readl(&cntl->regs->control); // flush writes
    msleep(USB_సమయం_DRSTR);

    // Do software init (min 10us, max 2ms)
    u32 చివర = timer_calc_usec(10);
    writel(&cntl->regs->cmdstatus, OHCI_HCR);
    కొరకు (;;) {
        u32 స్థితి_2 = readl(&cntl->regs->cmdstatus);
        ఒకవేళ (!(స్థితి_2 & OHCI_HCR))
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
    }

    // Init memory
    writel(&cntl->regs->ed_controlhead, 0);
    writel(&cntl->regs->ed_bulkhead, 0);
    writel(&cntl->regs->hcca, (u32)hcca);

    // Init fminterval
    u32 fi = oldfminterval & 0x3fff;
    writel(&cntl->regs->fminterval
           , (((oldfminterval & FIT) ^ FIT)
              | fi | (((6 * (fi - 210)) / 7) << 16)));
    writel(&cntl->regs->periodicstart, ((9 * fi) / 10) & 0x3fff);
    readl(&cntl->regs->control); // flush writes

    // XXX - verify that fminterval was setup correctly.

    // Go into operational state
    writel(&cntl->regs->control
           , (OHCI_CTRL_CBSR | OHCI_CTRL_CLE | OHCI_CTRL_BLE | OHCI_CTRL_PLE
              | OHCI_USB_OPER | oldrwc));
    readl(&cntl->regs->control); // flush writes

    తిరిగి 0;
}

స్థిర శూన్య
stop_ohci(నిర్మాణం usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

స్థిర శూన్య
configure_ohci(శూన్య *దత్తాంశం)
{
    నిర్మాణం usb_ohci_s *cntl = దత్తాంశం;

    // Allocate memory
    నిర్మాణం ohci_hcca *hcca = memalign_ఎగువ(256, పరిమాణం(*hcca));
    నిర్మాణం ohci_ed *intr_ed = malloc_ఎగువ(పరిమాణం(*intr_ed));
    ఒకవేళ (!hcca || !intr_ed) {
        warn_noalloc();
        వెళ్ళు విడుదల;
    }
    memset(hcca, 0, పరిమాణం(*hcca));
    memset(intr_ed, 0, పరిమాణం(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    పూర్ణాంకం i;
    కొరకు (i=0; i<ARRAY_కొలత(hcca->int_పట్టిక); i++)
        hcca->int_పట్టిక[i] = (u32)intr_ed;

    పూర్ణాంకం ret = ప్రారంభం_ohci(cntl, hcca);
    ఒకవేళ (ret)
        వెళ్ళు err;

    పూర్ణాంకం లెక్క = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    ఒకవేళ (! లెక్క)
        వెళ్ళు err;
    తిరిగి;

err:
    stop_ohci(cntl);
విడుదల:
    విడుదల(hcca);
    విడుదల(intr_ed);
}

స్థిర శూన్య
ohci_controller_అమరిక(నిర్మాణం pci_device *pci)
{
    నిర్మాణం ohci_regs *regs = pci_enable_membar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!regs)
        తిరిగి;

    నిర్మాణం usb_ohci_s *cntl = malloc_tmphigh(పరిమాణం(*cntl));
    ఒకవేళ (!cntl) {
        warn_noalloc();
        తిరిగి;
    }
    memset(cntl, 0, పరిమాణం(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_OHCI;
    cntl->regs = regs;

    dprintf(1, "OHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    // Disable interrupts
    writel(&cntl->regs->intrdisable, ~0);
    writel(&cntl->regs->intrstatus, ~0);

    నిరంతరం_thread(configure_ohci, cntl);
}

శూన్య
ohci_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_USB_OHCI)
        తిరిగి;
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_అమరిక(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
స్థిర శూన్య
ohci_desc2pipe(నిర్మాణం ohci_pipe *pipe, నిర్మాణం usbdevice_s *usbdev
               , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->w_Max_Packet_కొలత << 16)
                       | (usbdev->వేగం ? ED_LOWSPEED : 0));
    నిర్మాణం usb_ohci_s *cntl = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

స్థిర నిర్మాణం usb_pipe *
ohci_alloc_intr_pipe(నిర్మాణం usbdevice_s *usbdev
                     , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    నిర్మాణం usb_ohci_s *cntl = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_ohci_s, usb);
    పూర్ణాంకం frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ఒకవేళ (frameexp > 5)
        frameexp = 5;
    పూర్ణాంకం maxpacket = epdesc->w_Max_Packet_కొలత;
    // Determine number of entries needed for 2 timer ticks.
    పూర్ణాంకం ms = 1<<frameexp;
    పూర్ణాంకం లెక్క = DIV_విడత_పైకి(ticks_గమ్యం_ms(2), ms) + 1;
    నిర్మాణం ohci_pipe *pipe = malloc_దిగువ(పరిమాణం(*pipe));
    నిర్మాణం ohci_td *tds = malloc_దిగువ(పరిమాణం(*tds) * లెక్క);
    శూన్య *దత్తాంశం = malloc_దిగువ(maxpacket * లెక్క);
    ఒకవేళ (!pipe || !tds || !దత్తాంశం)
        వెళ్ళు err;
    memset(pipe, 0, పరిమాణం(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->దత్తాంశం = దత్తాంశం;
    pipe->లెక్క = లెక్క;
    pipe->tds = tds;

    నిర్మాణం ohci_ed *ed = &pipe->ed;
    ed->hw_తల_P = (u32)&tds[0];
    ed->hw_తోక_P = (u32)&tds[లెక్క-1];

    పూర్ణాంకం i;
    కొరకు (i=0; i<లెక్క-1; i++) {
        tds[i].hwINFO = TD_DP_లోపల | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)దత్తాంశం + maxpacket * i;
        tds[i].hw_తదుపరి_TD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    నిర్మాణం ohci_hcca *hcca = (శూన్య*)cntl->regs->hcca;
    ఒకవేళ (frameexp == 0) {
        // Add to existing interrupt entry.
        నిర్మాణం ohci_ed *intr_ed = (శూన్య*)hcca->int_పట్టిక[0];
        ed->hw_తదుపరి_ED = intr_ed->hw_తదుపరి_ED;
        barrier();
        intr_ed->hw_తదుపరి_ED = (u32)ed;
    } లేకపోతే {
        పూర్ణాంకం startpos = 1<<(frameexp-1);
        ed->hw_తదుపరి_ED = hcca->int_పట్టిక[startpos];
        barrier();
        కొరకు (i=startpos; i<ARRAY_కొలత(hcca->int_పట్టిక); i+=ms)
            hcca->int_పట్టిక[i] = (u32)ed;
    }

    తిరిగి &pipe->pipe;

err:
    విడుదల(pipe);
    విడుదల(tds);
    విడుదల(దత్తాంశం);
    తిరిగి ఖాళీ;
}

నిర్మాణం usb_pipe *
ohci_realloc_pipe(నిర్మాణం usbdevice_s *usbdev, నిర్మాణం usb_pipe *upipe
                  , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    ఒకవేళ (! CONFIG_USB_OHCI)
        తిరిగి ఖాళీ;
    usb_కూడు_freelist(upipe);
    ఒకవేళ (!epdesc)
        తిరిగి ఖాళీ;
    u8 eptype = epdesc->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_INT)
        తిరిగి ohci_alloc_intr_pipe(usbdev, epdesc);
    నిర్మాణం usb_ohci_s *cntl = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    నిర్మాణం usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ఒకవేళ (usbpipe) {
        // Use previously allocated pipe.
        నిర్మాణం ohci_pipe *pipe = container_యొక్క(usbpipe, నిర్మాణం ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        తిరిగి usbpipe;
    }

    // Allocate a new queue head.
    నిర్మాణం ohci_pipe *pipe;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(పరిమాణం(*pipe));
    లేకపోతే
        pipe = malloc_దిగువ(పరిమాణం(*pipe));
    ఒకవేళ (!pipe) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(pipe, 0, పరిమాణం(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *తల = &cntl->regs->ed_controlhead;
    ఒకవేళ (eptype != USB_ENDPOINT_XFER_CONTROL)
        తల = &cntl->regs->ed_bulkhead;
    pipe->ed.hw_తదుపరి_ED = *తల;
    barrier();
    *తల = (u32)&pipe->ed;
    తిరిగి &pipe->pipe;
}

స్థిర పూర్ణాంకం
wait_ed(నిర్మాణం ohci_ed *ed, పూర్ణాంకం timeout)
{
    u32 చివర = timer_calc(timeout);
    కొరకు (;;) {
        ఒకవేళ ((ed->hw_తల_P & ~(ED_C|ED_H)) == ed->hw_తోక_P)
            తిరిగి 0;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hw_తోక_P, ed->hw_తల_P, ed->hw_తదుపరి_ED);
            తిరిగి -1;
        }
        yield();
    }
}

#నిర్వచించు STACKOTDS 18
#నిర్వచించు OHCI_TD_ALIGN 16

పూర్ణాంకం
ohci_send_pipe(నిర్మాణం usb_pipe *p, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
               , శూన్య *దత్తాంశం, పూర్ణాంకం datasize)
{
    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_USB_OHCI)
        తిరిగి -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    నిర్మాణం ohci_pipe *pipe = container_యొక్క(p, నిర్మాణం ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[పరిమాణం(నిర్మాణం ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    నిర్మాణం ohci_td *tds = (శూన్య*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, పరిమాణం(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    ఒకవేళ (cmd) {
        // Send setup pid on control transfers
        td->hwINFO = TD_DP_అమరిక | TD_T_DATA0 | TD_CC;
        td->hwCBP = (u32)cmd;
        td->hw_తదుపరి_TD = (u32)&td[1];
        td->hwBE = (u32)cmd + USB_CONTROL_అమరిక_కొలత - 1;
        td++;
        toggle = TD_T_DATA1;
        statuscmd = OHCI_CLF;
    }
    u32 dest = (u32)దత్తాంశం, dataend = dest + datasize;
    అయితే (dest < dataend) {
        // Send data pids
        ఒకవేళ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            తిరిగి -1;
        }
        పూర్ణాంకం maxtransfer = 2*పుట_కొలత - (dest & (పుట_కొలత-1));
        పూర్ణాంకం transfer = dataend - dest;
        ఒకవేళ (transfer > maxtransfer)
            transfer = ALIGN_కిందికి(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_లోపల : TD_DP_బహిర్గతం) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hw_తదుపరి_TD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    ఒకవేళ (cmd) {
        // Send status pid on control transfers
        ఒకవేళ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            తిరిగి -1;
        }
        td->hwINFO = (dir ? TD_DP_బహిర్గతం : TD_DP_లోపల) | TD_T_DATA1 | TD_CC;
        td->hwCBP = 0;
        td->hw_తదుపరి_TD = (u32)&td[1];
        td->hwBE = 0;
        td++;
    }

    // Transfer data
    pipe->ed.hw_తల_P = (u32)tds | (pipe->ed.hw_తల_P & ED_C);
    pipe->ed.hw_తోక_P = (u32)td;
    barrier();
    pipe->ed.hwINFO &= ~ED_SKIP;
    writel(&pipe->regs->cmdstatus, statuscmd);

    పూర్ణాంకం ret = wait_ed(&pipe->ed, usb_xfer_సమయం(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    ఒకవేళ (ret)
        ohci_waittick(pipe->regs);
    తిరిగి ret;
}

పూర్ణాంకం
ohci_poll_intr(నిర్మాణం usb_pipe *p, శూన్య *దత్తాంశం)
{
    ASSERT16();
    ఒకవేళ (! CONFIG_USB_OHCI)
        తిరిగి -1;

    నిర్మాణం ohci_pipe *pipe = container_యొక్క(p, నిర్మాణం ohci_pipe, pipe);
    నిర్మాణం ohci_td *tds = GET_LOWFLAT(pipe->tds);
    నిర్మాణం ohci_td *తల = (శూన్య*)(GET_LOWFLAT(pipe->ed.hw_తల_P) & ~(ED_C|ED_H));
    నిర్మాణం ohci_td *తోక = (శూన్య*)GET_LOWFLAT(pipe->ed.hw_తోక_P);
    పూర్ణాంకం లెక్క = GET_LOWFLAT(pipe->లెక్క);
    పూర్ణాంకం pos = (తోక - tds + 1) % లెక్క;
    నిర్మాణం ohci_td *తదుపరి = &tds[pos];
    ఒకవేళ (తల == తదుపరి)
        // No intrs found.
        తిరిగి -1;
    // XXX - check for errors.

    // Copy data.
    పూర్ణాంకం maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    శూన్య *pipedata = GET_LOWFLAT((pipe->దత్తాంశం));
    శూన్య *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), దత్తాంశం, SEG_దిగువ, LOWFLAT2_దిగువ(intrdata), maxpacket);

    // Reenable this td.
    సమితి_LOWFLAT(తోక->hwINFO, TD_DP_లోపల | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (తోక-tds);
    సమితి_LOWFLAT(తోక->hwCBP, (u32)intrdata);
    సమితి_LOWFLAT(తోక->hw_తదుపరి_TD, (u32)తదుపరి);
    సమితి_LOWFLAT(తోక->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    సమితి_LOWFLAT(pipe->ed.hw_తోక_P, (u32)తదుపరి);

    తిరిగి 0;
}
