// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_LOWFLAT
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pci.h" // pci_config_writew
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#అన్తర్భావ "pci_regs.h" // PCI_BASE_ADDRESS_4
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "usb.h" // struct usb_s
#అన్తర్భావ "usb-ehci.h" // ehci_wait_controllers
#అన్తర్భావ "usb-uhci.h" // USBLEGSUP
#అన్తర్భావ "util.h" // msleep
#అన్తర్భావ "x86.h" // outw

నిర్మాణం usb_uhci_s {
    నిర్మాణం usb_s usb;
    u16 iobase;
    నిర్మాణం uhci_qh *control_qh;
    నిర్మాణం uhci_framelist *framelist;
};

నిర్మాణం uhci_pipe {
    నిర్మాణం uhci_qh qh;
    నిర్మాణం uhci_td *తదుపరి_td;
    నిర్మాణం usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
స్థిర పూర్ణాంకం
uhci_hub_detect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_uhci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + పోర్టు * 2;
    u16 స్థితి_2 = inw(ioport);
    ఒకవేళ (!(స్థితి_2 & USBPORTSC_CCS))
        // No device found.
        తిరిగి 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_సమయం_DRSTR);
    తిరిగి 1;
}

// Reset device on port
స్థిర పూర్ణాంకం
uhci_hub_reset(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_uhci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + పోర్టు * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 స్థితి_2 = inw(ioport);
    ఒకవేళ (!(స్థితి_2 & USBPORTSC_CCS))
        // No longer connected
        తిరిగి -1;
    outw(USBPORTSC_PE, ioport);
    తిరిగి !!(స్థితి_2 & USBPORTSC_LSDA);
}

// Disable port
స్థిర శూన్య
uhci_hub_disconnect(నిర్మాణం usbhub_s *hub, u32 పోర్టు)
{
    నిర్మాణం usb_uhci_s *cntl = container_యొక్క(hub->cntl, నిర్మాణం usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + పోర్టు * 2;
    outw(0, ioport);
}

స్థిర నిర్మాణం usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
స్థిర పూర్ణాంకం
check_uhci_ports(నిర్మాణం usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    నిర్మాణం usbhub_s hub;
    memset(&hub, 0, పరిమాణం(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    తిరిగి hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
స్థిర శూన్య
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 చివర = timer_calc(1000 * 5);
    కొరకు (;;) {
        ఒకవేళ (inw(iobase + USBFRNUM) != startframe)
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి;
        }
        yield();
    }
}

స్థిర శూన్య
uhci_free_pipes(నిర్మాణం usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    నిర్మాణం uhci_qh *pos = (శూన్య*)(cntl->framelist->links[0] & ~UHCI_PTR_బిట్లు);
    కొరకు (;;) {
        u32 link = pos->link;
        ఒకవేళ (link == UHCI_PTR_పదం)
            ఆపు;
        నిర్మాణం uhci_qh *తదుపరి = (శూన్య*)(link & ~UHCI_PTR_బిట్లు);
        నిర్మాణం uhci_pipe *pipe = container_యొక్క(తదుపరి, నిర్మాణం uhci_pipe, qh);
        ఒకవేళ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = తదుపరి->link;
        లేకపోతే
            pos = తదుపరి;
    }
    uhci_waittick(cntl->iobase);
    కొరకు (;;) {
        నిర్మాణం usb_pipe *usbpipe = cntl->usb.freelist;
        ఒకవేళ (!usbpipe)
            ఆపు;
        cntl->usb.freelist = usbpipe->freenext;
        నిర్మాణం uhci_pipe *pipe = container_యొక్క(usbpipe, నిర్మాణం uhci_pipe, pipe);
        విడుదల(pipe);
    }
}

స్థిర శూన్య
reset_uhci(నిర్మాణం usb_uhci_s *cntl, u16 bdf)
{
    // XXX - don't reset if not needed.

    // Reset PIRQ and SMI
    pci_config_writew(bdf, USBLEGSUP, USBLEGSUP_RWC);

    // Reset the HC
    outw(USBCMD_HCRESET, cntl->iobase + USBCMD);
    udelay(5);

    // Disable interrupts and commands (just to be safe).
    outw(0, cntl->iobase + USBINTR);
    outw(0, cntl->iobase + USBCMD);
}

స్థిర శూన్య
configure_uhci(శూన్య *దత్తాంశం)
{
    నిర్మాణం usb_uhci_s *cntl = దత్తాంశం;

    // Allocate ram for schedule storage
    నిర్మాణం uhci_td *పదం_td = malloc_ఎగువ(పరిమాణం(*పదం_td));
    నిర్మాణం uhci_framelist *fl = memalign_ఎగువ(పరిమాణం(*fl), పరిమాణం(*fl));
    నిర్మాణం uhci_pipe *intr_pipe = malloc_ఎగువ(పరిమాణం(*intr_pipe));
    నిర్మాణం uhci_pipe *పదం_pipe = malloc_ఎగువ(పరిమాణం(*పదం_pipe));
    ఒకవేళ (!పదం_td || !fl || !intr_pipe || !పదం_pipe) {
        warn_noalloc();
        వెళ్ళు fail;
    }

    // Work around for PIIX errata
    memset(పదం_td, 0, పరిమాణం(*పదం_td));
    పదం_td->link = UHCI_PTR_పదం;
    పదం_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_జరుపు)
                      | USB_PID_లోపల);
    memset(పదం_pipe, 0, పరిమాణం(*పదం_pipe));
    పదం_pipe->qh.element = (u32)పదం_td;
    పదం_pipe->qh.link = UHCI_PTR_పదం;
    పదం_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, పరిమాణం(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_పదం;
    intr_pipe->qh.link = (u32)&పదం_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    పూర్ణాంకం i;
    కొరకు (i=0; i<ARRAY_కొలత(fl->links); i++)
        fl->links[i] = (u32)&intr_pipe->qh | UHCI_PTR_QH;
    cntl->framelist = fl;
    cntl->control_qh = &intr_pipe->qh;
    barrier();

    // Set the frame length to the default: 1 ms exactly
    outb(USBSOF_DEFAULT, cntl->iobase + USBSOF);

    // Store the frame list base address
    outl((u32)fl->links, cntl->iobase + USBFLBASEADD);

    // Set the current frame number
    outw(0, cntl->iobase + USBFRNUM);

    // Mark as configured and running with a 64-byte max packet.
    outw(USBCMD_RS | USBCMD_CF | USBCMD_MAXP, cntl->iobase + USBCMD);

    // Find devices
    పూర్ణాంకం లెక్క = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    ఒకవేళ (లెక్క)
        // Success
        తిరిగి;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    విడుదల(పదం_td);
    విడుదల(fl);
    విడుదల(intr_pipe);
    విడుదల(పదం_pipe);
    విడుదల(cntl);
}

స్థిర శూన్య
uhci_controller_అమరిక(నిర్మాణం pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_4);
    ఒకవేళ (!iobase)
        తిరిగి;

    నిర్మాణం usb_uhci_s *cntl = malloc_tmphigh(పరిమాణం(*cntl));
    ఒకవేళ (!cntl) {
        warn_noalloc();
        తిరిగి;
    }
    memset(cntl, 0, పరిమాణం(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    నిరంతరం_thread(configure_uhci, cntl);
}

శూన్య
uhci_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_USB_UHCI)
        తిరిగి;
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_అమరిక(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

స్థిర నిర్మాణం usb_pipe *
uhci_alloc_intr_pipe(నిర్మాణం usbdevice_s *usbdev
                     , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    నిర్మాణం usb_uhci_s *cntl = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_uhci_s, usb);
    పూర్ణాంకం frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ఒకవేళ (frameexp > 10)
        frameexp = 10;
    పూర్ణాంకం maxpacket = epdesc->w_Max_Packet_కొలత;
    // Determine number of entries needed for 2 timer ticks.
    పూర్ణాంకం ms = 1<<frameexp;
    పూర్ణాంకం లెక్క = DIV_విడత_పైకి(ticks_గమ్యం_ms(2), ms);
    లెక్క = ALIGN(లెక్క, 2);
    నిర్మాణం uhci_pipe *pipe = malloc_దిగువ(పరిమాణం(*pipe));
    నిర్మాణం uhci_td *tds = malloc_దిగువ(పరిమాణం(*tds) * లెక్క);
    శూన్య *దత్తాంశం = malloc_దిగువ(maxpacket * లెక్క);
    ఒకవేళ (!pipe || !tds || !దత్తాంశం) {
        warn_noalloc();
        వెళ్ళు fail;
    }
    memset(pipe, 0, పరిమాణం(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    పూర్ణాంకం lowspeed = pipe->pipe.వేగం;
    పూర్ణాంకం devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->తదుపరి_td = &tds[0];
    pipe->iobase = cntl->iobase;

    పూర్ణాంకం toggle = 0;
    పూర్ణాంకం i;
    కొరకు (i=0; i<లెక్క; i++) {
        tds[i].link = (i==లెక్క-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].స్థితి_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_చురుకైన);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_జరుపు)
                        | USB_PID_లోపల);
        tds[i].బఫరు = దత్తాంశం + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    నిర్మాణం uhci_framelist *fl = cntl->framelist;
    ఒకవేళ (frameexp == 0) {
        // Add to existing interrupt entry.
        నిర్మాణం uhci_qh *intr_qh = (శూన్య*)(fl->links[0] & ~UHCI_PTR_బిట్లు);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        ఒకవేళ (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } లేకపోతే {
        పూర్ణాంకం startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        కొరకు (i=startpos; i<ARRAY_కొలత(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    తిరిగి &pipe->pipe;
fail:
    విడుదల(pipe);
    విడుదల(tds);
    విడుదల(దత్తాంశం);
    తిరిగి ఖాళీ;
}

నిర్మాణం usb_pipe *
uhci_realloc_pipe(నిర్మాణం usbdevice_s *usbdev, నిర్మాణం usb_pipe *upipe
                  , నిర్మాణం usb_endpoint_descriptor *epdesc)
{
    ఒకవేళ (! CONFIG_USB_UHCI)
        తిరిగి ఖాళీ;
    usb_కూడు_freelist(upipe);
    ఒకవేళ (!epdesc)
        తిరిగి ఖాళీ;
    u8 eptype = epdesc->bm_లక్షణాలు & USB_ENDPOINT_XFERTYPE_ముసుగు;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_INT)
        తిరిగి uhci_alloc_intr_pipe(usbdev, epdesc);
    నిర్మాణం usb_uhci_s *cntl = container_యొక్క(
        usbdev->hub->cntl, నిర్మాణం usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    నిర్మాణం usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ఒకవేళ (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        తిరిగి usbpipe;
    }

    // Allocate a new queue head.
    నిర్మాణం uhci_pipe *pipe;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(పరిమాణం(*pipe));
    లేకపోతే
        pipe = malloc_దిగువ(పరిమాణం(*pipe));
    ఒకవేళ (!pipe) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(pipe, 0, పరిమాణం(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_పదం;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    నిర్మాణం uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    ఒకవేళ (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    తిరిగి &pipe->pipe;
}

స్థిర పూర్ణాంకం
wait_pipe(నిర్మాణం uhci_pipe *pipe, u32 చివర)
{
    కొరకు (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        ఒకవేళ (el_link & UHCI_PTR_పదం)
            తిరిగి 0;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            నిర్మాణం uhci_td *td = (శూన్య*)(el_link & ~UHCI_PTR_బిట్లు);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (శూన్య*)el_link, GET_LOWFLAT(td->స్థితి_2)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            సమితి_LOWFLAT(pipe->qh.element, UHCI_PTR_పదం);
            uhci_waittick(iobase);
            తిరిగి -1;
        }
        yield();
    }
}

స్థిర పూర్ణాంకం
wait_td(నిర్మాణం uhci_td *td, u32 చివర)
{
    u32 స్థితి_2;
    కొరకు (;;) {
        స్థితి_2 = td->స్థితి_2;
        ఒకవేళ (!(స్థితి_2 & TD_CTRL_చురుకైన))
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
        yield();
    }
    ఒకవేళ (స్థితి_2 & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", స్థితి_2);
        తిరిగి -2;
    }
    తిరిగి 0;
}

#నిర్వచించు STACKTDS 16
#నిర్వచించు TDALIGN 16

పూర్ణాంకం
uhci_send_pipe(నిర్మాణం usb_pipe *p, పూర్ణాంకం dir, స్థిరరాశి శూన్య *cmd
               , శూన్య *దత్తాంశం, పూర్ణాంకం datasize)
{
    ఒకవేళ (! CONFIG_USB_UHCI)
        తిరిగి -1;
    నిర్మాణం uhci_pipe *pipe = container_యొక్క(p, నిర్మాణం uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, దత్తాంశం, datasize);
    పూర్ణాంకం maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    పూర్ణాంకం lowspeed = GET_LOWFLAT(pipe->pipe.వేగం);
    పూర్ణాంకం devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    పూర్ణాంకం toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[పరిమాణం(నిర్మాణం uhci_td) * STACKTDS + TDALIGN - 1];
    నిర్మాణం uhci_td *tds = (శూన్య*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, పరిమాణం(*tds) * STACKTDS);
    పూర్ణాంకం tdpos = 0;

    // Enable tds
    u32 చివర = timer_calc(usb_xfer_సమయం(p, datasize));
    barrier();
    సమితి_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    ఒకవేళ (cmd) {
        // Send setup pid on control transfers
        నిర్మాణం uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_లోతు;
        td->token = (uhci_explen(USB_CONTROL_అమరిక_కొలత)
                     | (devaddr << TD_TOKEN_DEVADDR_జరుపు) | USB_PID_అమరిక);
        td->బఫరు = (శూన్య*)cmd;
        barrier();
        td->స్థితి_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_చురుకైన);
        toggle = TD_TOKEN_TOGGLE;
    }
    అయితే (datasize) {
        // Send data pids
        నిర్మాణం uhci_td *td = &tds[tdpos++ % STACKTDS];
        పూర్ణాంకం ret = wait_td(td, చివర);
        ఒకవేళ (ret)
            వెళ్ళు fail;

        పూర్ణాంకం transfer = datasize;
        ఒకవేళ (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_పదం : (nexttd | UHCI_PTR_లోతు));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_జరుపు)
                     | (dir ? USB_PID_లోపల : USB_PID_బహిర్గతం));
        td->బఫరు = దత్తాంశం;
        barrier();
        td->స్థితి_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_చురుకైన);
        toggle ^= TD_TOKEN_TOGGLE;

        దత్తాంశం += transfer;
        datasize -= transfer;
    }
    ఒకవేళ (cmd) {
        // Send status pid on control transfers
        నిర్మాణం uhci_td *td = &tds[tdpos++ % STACKTDS];
        పూర్ణాంకం ret = wait_td(td, చివర);
        ఒకవేళ (ret)
            వెళ్ళు fail;
        td->link = UHCI_PTR_పదం;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_జరుపు)
                     | (dir ? USB_PID_బహిర్గతం : USB_PID_లోపల));
        td->బఫరు = 0;
        barrier();
        td->స్థితి_2 = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_చురుకైన);
    }
    సమితి_LOWFLAT(pipe->toggle, !!toggle);
    తిరిగి wait_pipe(pipe, చివర);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    సమితి_LOWFLAT(pipe->qh.element, UHCI_PTR_పదం);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    తిరిగి -1;
}

పూర్ణాంకం
uhci_poll_intr(నిర్మాణం usb_pipe *p, శూన్య *దత్తాంశం)
{
    ASSERT16();
    ఒకవేళ (! CONFIG_USB_UHCI)
        తిరిగి -1;

    నిర్మాణం uhci_pipe *pipe = container_యొక్క(p, నిర్మాణం uhci_pipe, pipe);
    నిర్మాణం uhci_td *td = GET_LOWFLAT(pipe->తదుపరి_td);
    u32 స్థితి_2 = GET_LOWFLAT(td->స్థితి_2);
    u32 token = GET_LOWFLAT(td->token);
    ఒకవేళ (స్థితి_2 & TD_CTRL_చురుకైన)
        // No intrs found.
        తిరిగి -1;
    // XXX - check for errors.

    // Copy data.
    శూన్య *tddata = GET_LOWFLAT(td->బఫరు);
    memcpy_far(GET_SEG(SS), దత్తాంశం, SEG_దిగువ, LOWFLAT2_దిగువ(tddata)
               , uhci_expected_నిడివి(token));

    // Reenable this td.
    నిర్మాణం uhci_td *తదుపరి = (శూన్య*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_బిట్లు);
    సమితి_LOWFLAT(pipe->తదుపరి_td, తదుపరి);
    barrier();
    సమితి_LOWFLAT(td->స్థితి_2, (uhci_maxerr(0) | (స్థితి_2 & TD_CTRL_LS)
                             | TD_CTRL_చురుకైన));

    తిరిగి 0;
}
