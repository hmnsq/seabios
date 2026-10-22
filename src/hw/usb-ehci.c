// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_LOWFLAT
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "memmap.h" // PAGE_SIZE
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#अन्तर्भाव "pci_regs.h" // PCI_BASE_ADDRESS_0
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "usb.h" // struct usb_s
#अन्तर्भाव "usb-ehci.h" // struct ehci_qh
#अन्तर्भाव "util.h" // msleep
#अन्तर्भाव "x86.h" // readl

संरचना usb_ehci_s {
    संरचना usb_s usb;
    संरचना ehci_caps *caps;
    संरचना ehci_regs *regs;
    संरचना ehci_qh *async_qh;
    पूर्णाङ्क checkports;
};

संरचना ehci_pipe {
    संरचना ehci_qh qh;
    संरचना ehci_qtd *next_td, *tds;
    शून्य *data;
    संरचना usb_pipe pipe;
};

स्थिर पूर्णाङ्क PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
स्थिर पूर्णाङ्क
ehci_hub_detect(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_ehci_s *cntl = container_of(hub->cntl, संरचना usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    यदि (!(portsc & PORT_CONNECT))
        // No device present
        प्रत्यागम 0;

    यदि ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        प्रत्यागम -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    प्रत्यागम 1;
}

// Reset device on port
स्थिर पूर्णाङ्क
ehci_hub_reset(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_ehci_s *cntl = container_of(hub->cntl, संरचना usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    यदि (!(portsc & PORT_CONNECT))
        // No longer connected
        प्रत्यागम -1;
    यदि (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        प्रत्यागम -1;
    }

    प्रत्यागम USB_HIGHSPEED;
}

// Disable port
स्थिर शून्य
ehci_hub_disconnect(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_ehci_s *cntl = container_of(hub->cntl, संरचना usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

स्थिर संरचना usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
स्थिर पूर्णाङ्क
check_ehci_ports(संरचना usb_ehci_s *cntl)
{
    // Power up ports.
    पूर्णाङ्क i;
    प्रति (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        यदि (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    संरचना usbhub_s hub;
    स_रखो(&hub, 0, परिमाण(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    प्रत्यागम hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
स्थिर शून्य
ehci_waittick(संरचना usb_ehci_s *cntl)
{
    यदि (MODE16) {
        msleep(10);
        प्रत्यागम;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    प्रति (;;) {
        sts = readl(&cntl->regs->usbsts);
        यदि (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            यदि (!(cmd & CMD_IAAD))
                भङ्ग;
        }
        यदि (timer_check(end)) {
            warn_timeout();
            प्रत्यागम;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    प्रति (;;) {
        sts = readl(&cntl->regs->usbsts);
        यदि (sts & STS_IAA)
            भङ्ग;
        यदि (timer_check(end)) {
            warn_timeout();
            प्रत्यागम;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

स्थिर शून्य
ehci_free_pipes(संरचना usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    संरचना ehci_qh *start = cntl->async_qh;
    संरचना ehci_qh *pos = start;
    प्रति (;;) {
        संरचना ehci_qh *next = (शून्य*)(pos->next & ~EHCI_PTR_BITS);
        यदि (next == start)
            भङ्ग;
        संरचना ehci_pipe *pipe = container_of(next, संरचना ehci_pipe, qh);
        यदि (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        अन्यथा
            pos = next;
    }
    ehci_waittick(cntl);
    प्रति (;;) {
        संरचना usb_pipe *usbpipe = cntl->usb.freelist;
        यदि (!usbpipe)
            भङ्ग;
        cntl->usb.freelist = usbpipe->freenext;
        संरचना ehci_pipe *pipe = container_of(usbpipe, संरचना ehci_pipe, pipe);
        मुक्त(pipe);
    }
}

स्थिर शून्य
configure_ehci(शून्य *data)
{
    संरचना usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    संरचना ehci_framelist *fl = memalign_ऊपरी(परिमाण(*fl), परिमाण(*fl));
    संरचना ehci_qh *intr_qh = memalign_ऊपरी(EHCI_QH_ALIGN, परिमाण(*intr_qh));
    संरचना ehci_qh *async_qh = memalign_ऊपरी(EHCI_QH_ALIGN, परिमाण(*async_qh));
    यदि (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        गच्छ fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    प्रति (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        यदि (!(cmd & CMD_HCRESET))
            भङ्ग;
        यदि (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            गच्छ fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    स_रखो(intr_qh, 0, परिमाण(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    पूर्णाङ्क i;
    प्रति (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    स_रखो(async_qh, 0, परिमाण(*async_qh));
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
    पूर्णाङ्क गिनती = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    यदि (गिनती)
        // Success
        प्रत्यागम;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    मुक्त(fl);
    मुक्त(intr_qh);
    मुक्त(async_qh);
    मुक्त(cntl);
}

स्थिर शून्य
ehci_controller_setup(संरचना pci_device *pci)
{
    संरचना ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    यदि (!caps)
        प्रत्यागम;
    u32 hcc_params = readl(&caps->hccparams);

    संरचना usb_ehci_s *cntl = malloc_tmphigh(परिमाण(*cntl));
    यदि (!cntl) {
        warn_noalloc();
        प्रत्यागम;
    }
    स_रखो(cntl, 0, परिमाण(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (शून्य*)caps + readb(&caps->caplength);
    यदि (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

शून्य
ehci_setup(शून्य)
{
    यदि (! CONFIG_USB_EHCI)
        प्रत्यागम;
    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
शून्य
ehci_wait_controllers(शून्य)
{
    यावत् (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
स्थिर शून्य
ehci_desc2pipe(संरचना ehci_pipe *pipe, संरचना usbdevice_s *usbdev
               , संरचना usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    संरचना usbdevice_s *hubdev = usbdev->hub->usbdev;
    यदि (hubdev) {
        संरचना ehci_pipe *hpipe = container_of(
            hubdev->defpipe, संरचना ehci_pipe, pipe);
        यदि (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        अन्यथा
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    यदि (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    अन्यथा यदि (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

स्थिर संरचना usb_pipe *
ehci_alloc_intr_pipe(संरचना usbdevice_s *usbdev
                     , संरचना usb_endpoint_descriptor *epdesc)
{
    संरचना usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, संरचना usb_ehci_s, usb);
    पूर्णाङ्क frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    यदि (frameexp > 10)
        frameexp = 10;
    पूर्णाङ्क maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    पूर्णाङ्क ms = 1<<frameexp;
    पूर्णाङ्क गिनती = DIV_ROUND_UP(ticks_to_ms(2), ms);
    संरचना ehci_pipe *pipe = memalign_निचला(EHCI_QH_ALIGN, परिमाण(*pipe));
    संरचना ehci_qtd *tds = memalign_निचला(EHCI_QTD_ALIGN, परिमाण(*tds) * गिनती);
    शून्य *data = malloc_निचला(maxpacket * गिनती);
    यदि (!pipe || !tds || !data) {
        warn_noalloc();
        गच्छ fail;
    }
    स_रखो(pipe, 0, परिमाण(*pipe));
    स_रखो(tds, 0, परिमाण(*tds) * गिनती);
    स_रखो(data, 0, maxpacket * गिनती);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    पूर्णाङ्क i;
    प्रति (i=0; i<गिनती; i++) {
        संरचना ehci_qtd *td = &tds[i];
        td->qtd_next = (i==गिनती-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    संरचना ehci_framelist *fl = (शून्य*)readl(&cntl->regs->periodiclistbase);
    यदि (frameexp == 0) {
        // Add to existing interrupt entry.
        संरचना ehci_qh *intr_qh = (शून्य*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } अन्यथा {
        पूर्णाङ्क startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        प्रति (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    प्रत्यागम &pipe->pipe;
fail:
    मुक्त(pipe);
    मुक्त(tds);
    मुक्त(data);
    प्रत्यागम रिक्त;
}

संरचना usb_pipe *
ehci_realloc_pipe(संरचना usbdevice_s *usbdev, संरचना usb_pipe *upipe
                  , संरचना usb_endpoint_descriptor *epdesc)
{
    यदि (! CONFIG_USB_EHCI)
        प्रत्यागम रिक्त;
    usb_add_freelist(upipe);
    यदि (!epdesc)
        प्रत्यागम रिक्त;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    यदि (eptype == USB_ENDPOINT_XFER_INT)
        प्रत्यागम ehci_alloc_intr_pipe(usbdev, epdesc);
    संरचना usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, संरचना usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    संरचना usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    यदि (usbpipe) {
        // Use previously allocated pipe.
        संरचना ehci_pipe *pipe = container_of(usbpipe, संरचना ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        प्रत्यागम usbpipe;
    }

    // Allocate a new queue head.
    संरचना ehci_pipe *pipe;
    यदि (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, परिमाण(*pipe));
    अन्यथा
        pipe = memalign_निचला(EHCI_QH_ALIGN, परिमाण(*pipe));
    यदि (!pipe) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स_रखो(pipe, 0, परिमाण(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    संरचना ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    प्रत्यागम &pipe->pipe;
}

स्थिर शून्य
ehci_reset_pipe(संरचना ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

स्थिर पूर्णाङ्क
ehci_wait_td(संरचना ehci_pipe *pipe, संरचना ehci_qtd *td, u32 end)
{
    u32 status;
    प्रति (;;) {
        status = td->token;
        यदि (!(status & QTD_STS_ACTIVE))
            भङ्ग;
        यदि (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            संरचना usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), संरचना usb_ehci_s, usb);
            ehci_waittick(cntl);
            प्रत्यागम -1;
        }
        yield();
    }
    यदि (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        प्रत्यागम -2;
    }
    प्रत्यागम 0;
}

स्थिर शून्य
ehci_fill_tdbuf(संरचना ehci_qtd *td, u32 dest, पूर्णाङ्क transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    प्रति (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

पूर्णाङ्क
ehci_send_pipe(संरचना usb_pipe *p, पूर्णाङ्क dir, ध्रुव शून्य *cmd
               , शून्य *data, पूर्णाङ्क datasize)
{
    यदि (! CONFIG_USB_EHCI)
        प्रत्यागम -1;
    संरचना ehci_pipe *pipe = container_of(p, संरचना ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[परिमाण(संरचना ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    संरचना ehci_qtd *tds = (शून्य*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    स_रखो(tds, 0, परिमाण(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    यदि (cmd) {
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
    यावत् (dest < dataend) {
        // Send data pids
        यदि (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            प्रत्यागम -1;
        }
        पूर्णाङ्क maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        पूर्णाङ्क transfer = dataend - dest;
        यदि (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    यदि (cmd) {
        // Send status pid on control transfers
        यदि (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            प्रत्यागम -1;
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
    पूर्णाङ्क i;
    प्रति (i=0, td=tds; i<STACKQTDS; i++, td++) {
        पूर्णाङ्क ret = ehci_wait_td(pipe, td, end);
        यदि (ret)
            प्रत्यागम -1;
    }

    प्रत्यागम 0;
}

पूर्णाङ्क
ehci_poll_intr(संरचना usb_pipe *p, शून्य *data)
{
    ASSERT16();
    यदि (! CONFIG_USB_EHCI)
        प्रत्यागम -1;
    संरचना ehci_pipe *pipe = container_of(p, संरचना ehci_pipe, pipe);
    संरचना ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    यदि (token & QTD_STS_ACTIVE)
        // No intrs found.
        प्रत्यागम -1;
    // XXX - check for errors.

    // Copy data.
    पूर्णाङ्क maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    पूर्णाङ्क pos = td - GET_LOWFLAT(pipe->tds);
    शून्य *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_निचला, LOWFLAT2_निचला(tddata), maxpacket);

    // Reenable this td.
    संरचना ehci_qtd *next = (शून्य*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    प्रत्यागम 0;
}
