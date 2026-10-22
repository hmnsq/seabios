// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_LOWFLAT
#समावेश "config.h" // CONFIG_*
#समावेश "output.h" // dprintf
#समावेश "malloc.h" // free
#समावेश "memmap.h" // PAGE_SIZE
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#समावेश "pci_regs.h" // PCI_BASE_ADDRESS_0
#समावेश "string.h" // memset
#समावेश "usb.h" // struct usb_s
#समावेश "usb-ehci.h" // struct ehci_qh
#समावेश "util.h" // msleep
#समावेश "x86.h" // readl

काष्ठा usb_ehci_s {
    काष्ठा usb_s usb;
    काष्ठा ehci_caps *caps;
    काष्ठा ehci_regs *regs;
    काष्ठा ehci_qh *async_qh;
    पूर्णांक checkports;
};

काष्ठा ehci_pipe {
    काष्ठा ehci_qh qh;
    काष्ठा ehci_qtd *अग्र_td, *tds;
    व्योम *तथ्य;
    काष्ठा usb_pipe pipe;
};

अटल पूर्णांक PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#घोषणा EHCI_TIME_POSTPOWER 20
#घोषणा EHCI_TIME_POSTRESET 2

// Check if device attached to port
अटल पूर्णांक
ehci_hub_detect(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_ehci_s *cntl = container_of(hub->cntl, काष्ठा usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    अगर (!(portsc & PORT_CONNECT))
        // No device present
        वापस 0;

    अगर ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        वापस -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    वापस 1;
}

// Reset device on port
अटल पूर्णांक
ehci_hub_reset(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_ehci_s *cntl = container_of(hub->cntl, काष्ठा usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    अगर (!(portsc & PORT_CONNECT))
        // No longer connected
        वापस -1;
    अगर (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        वापस -1;
    }

    वापस USB_HIGHSPEED;
}

// Disable port
अटल व्योम
ehci_hub_disconnect(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_ehci_s *cntl = container_of(hub->cntl, काष्ठा usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

अटल काष्ठा usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
अटल पूर्णांक
check_ehci_ports(काष्ठा usb_ehci_s *cntl)
{
    // Power up ports.
    पूर्णांक i;
    क्रम (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        अगर (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    काष्ठा usbhub_s hub;
    स_रखो(&hub, 0, माप(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    वापस hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
अटल व्योम
ehci_waittick(काष्ठा usb_ehci_s *cntl)
{
    अगर (MODE16) {
        msleep(10);
        वापस;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    क्रम (;;) {
        sts = readl(&cntl->regs->usbsts);
        अगर (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            अगर (!(cmd & CMD_IAAD))
                अवरोध;
        }
        अगर (timer_check(end)) {
            warn_timeout();
            वापस;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    क्रम (;;) {
        sts = readl(&cntl->regs->usbsts);
        अगर (sts & STS_IAA)
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            वापस;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

अटल व्योम
ehci_free_pipes(काष्ठा usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    काष्ठा ehci_qh *start = cntl->async_qh;
    काष्ठा ehci_qh *pos = start;
    क्रम (;;) {
        काष्ठा ehci_qh *अग्र = (व्योम*)(pos->अग्र & ~EHCI_PTR_BITS);
        अगर (अग्र == start)
            अवरोध;
        काष्ठा ehci_pipe *pipe = container_of(अग्र, काष्ठा ehci_pipe, qh);
        अगर (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->अग्र = अग्र->अग्र;
        अन्यथा
            pos = अग्र;
    }
    ehci_waittick(cntl);
    क्रम (;;) {
        काष्ठा usb_pipe *usbpipe = cntl->usb.freelist;
        अगर (!usbpipe)
            अवरोध;
        cntl->usb.freelist = usbpipe->freenext;
        काष्ठा ehci_pipe *pipe = container_of(usbpipe, काष्ठा ehci_pipe, pipe);
        मुक्त(pipe);
    }
}

अटल व्योम
configure_ehci(व्योम *तथ्य)
{
    काष्ठा usb_ehci_s *cntl = तथ्य;

    // Allocate ram for schedule storage
    काष्ठा ehci_framelist *fl = memalign_ऊपरी(माप(*fl), माप(*fl));
    काष्ठा ehci_qh *intr_qh = memalign_ऊपरी(EHCI_QH_ALIGN, माप(*intr_qh));
    काष्ठा ehci_qh *async_qh = memalign_ऊपरी(EHCI_QH_ALIGN, माप(*async_qh));
    अगर (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        जाओ fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    क्रम (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        अगर (!(cmd & CMD_HCRESET))
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            जाओ fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    स_रखो(intr_qh, 0, माप(*intr_qh));
    intr_qh->अग्र = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_अग्र = intr_qh->alt_अग्र = EHCI_PTR_TERM;
    पूर्णांक i;
    क्रम (i=0; i<ARRAY_आकार(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    स_रखो(async_qh, 0, माप(*async_qh));
    async_qh->अग्र = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_शीर्ष;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_अग्र = async_qh->alt_अग्र = EHCI_PTR_TERM;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_RUN);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    पूर्णांक गिनती = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    अगर (गिनती)
        // Success
        वापस;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    मुक्त(fl);
    मुक्त(intr_qh);
    मुक्त(async_qh);
    मुक्त(cntl);
}

अटल व्योम
ehci_controller_setup(काष्ठा pci_device *pci)
{
    काष्ठा ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    अगर (!caps)
        वापस;
    u32 hcc_params = readl(&caps->hccparams);

    काष्ठा usb_ehci_s *cntl = malloc_tmphigh(माप(*cntl));
    अगर (!cntl) {
        warn_noalloc();
        वापस;
    }
    स_रखो(cntl, 0, माप(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (व्योम*)caps + readb(&caps->caplength);
    अगर (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

व्योम
ehci_setup(व्योम)
{
    अगर (! CONFIG_USB_EHCI)
        वापस;
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
व्योम
ehci_wait_controllers(व्योम)
{
    जबतक (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
अटल व्योम
ehci_desc2pipe(काष्ठा ehci_pipe *pipe, काष्ठा usbdevice_s *usbdev
               , काष्ठा usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    काष्ठा usbdevice_s *hubdev = usbdev->hub->usbdev;
    अगर (hubdev) {
        काष्ठा ehci_pipe *hpipe = container_of(
            hubdev->defpipe, काष्ठा ehci_pipe, pipe);
        अगर (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        अन्यथा
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    अगर (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    अन्यथा अगर (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

अटल काष्ठा usb_pipe *
ehci_alloc_intr_pipe(काष्ठा usbdevice_s *usbdev
                     , काष्ठा usb_endpoint_descriptor *epdesc)
{
    काष्ठा usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, काष्ठा usb_ehci_s, usb);
    पूर्णांक frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    अगर (frameexp > 10)
        frameexp = 10;
    पूर्णांक maxpacket = epdesc->w_Max_Packet_आकार;
    // Determine number of entries needed for 2 timer ticks.
    पूर्णांक ms = 1<<frameexp;
    पूर्णांक गिनती = DIV_ROUND_UP(ticks_to_ms(2), ms);
    काष्ठा ehci_pipe *pipe = memalign_निचला(EHCI_QH_ALIGN, माप(*pipe));
    काष्ठा ehci_qtd *tds = memalign_निचला(EHCI_QTD_ALIGN, माप(*tds) * गिनती);
    व्योम *तथ्य = malloc_निचला(maxpacket * गिनती);
    अगर (!pipe || !tds || !तथ्य) {
        warn_noalloc();
        जाओ fail;
    }
    स_रखो(pipe, 0, माप(*pipe));
    स_रखो(tds, 0, माप(*tds) * गिनती);
    स_रखो(तथ्य, 0, maxpacket * गिनती);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->अग्र_td = pipe->tds = tds;
    pipe->तथ्य = तथ्य;
    pipe->qh.qtd_अग्र = (u32)tds;

    पूर्णांक i;
    क्रम (i=0; i<गिनती; i++) {
        काष्ठा ehci_qtd *td = &tds[i];
        td->qtd_अग्र = (i==गिनती-1 ? (u32)tds : (u32)&td[1]);
        td->alt_अग्र = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)तथ्य + maxpacket * i;
    }

    // Add to interrupt schedule.
    काष्ठा ehci_framelist *fl = (व्योम*)readl(&cntl->regs->periodiclistbase);
    अगर (frameexp == 0) {
        // Add to existing interrupt entry.
        काष्ठा ehci_qh *intr_qh = (व्योम*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.अग्र = intr_qh->अग्र;
        barrier();
        intr_qh->अग्र = (u32)&pipe->qh | EHCI_PTR_QH;
    } अन्यथा {
        पूर्णांक startpos = 1<<(frameexp-1);
        pipe->qh.अग्र = fl->links[startpos];
        barrier();
        क्रम (i=startpos; i<ARRAY_आकार(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    वापस &pipe->pipe;
fail:
    मुक्त(pipe);
    मुक्त(tds);
    मुक्त(तथ्य);
    वापस शून्य;
}

काष्ठा usb_pipe *
ehci_realloc_pipe(काष्ठा usbdevice_s *usbdev, काष्ठा usb_pipe *upipe
                  , काष्ठा usb_endpoint_descriptor *epdesc)
{
    अगर (! CONFIG_USB_EHCI)
        वापस शून्य;
    usb_add_freelist(upipe);
    अगर (!epdesc)
        वापस शून्य;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    अगर (eptype == USB_ENDPOINT_XFER_INT)
        वापस ehci_alloc_intr_pipe(usbdev, epdesc);
    काष्ठा usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, काष्ठा usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    काष्ठा usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    अगर (usbpipe) {
        // Use previously allocated pipe.
        काष्ठा ehci_pipe *pipe = container_of(usbpipe, काष्ठा ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        वापस usbpipe;
    }

    // Allocate a new queue head.
    काष्ठा ehci_pipe *pipe;
    अगर (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, माप(*pipe));
    अन्यथा
        pipe = memalign_निचला(EHCI_QH_ALIGN, माप(*pipe));
    अगर (!pipe) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(pipe, 0, माप(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_अग्र = pipe->qh.alt_अग्र = EHCI_PTR_TERM;

    // Add queue head to controller list.
    काष्ठा ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.अग्र = async_qh->अग्र;
    barrier();
    async_qh->अग्र = (u32)&pipe->qh | EHCI_PTR_QH;
    वापस &pipe->pipe;
}

अटल व्योम
ehci_reset_pipe(काष्ठा ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_अग्र, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_अग्र, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

अटल पूर्णांक
ehci_wait_td(काष्ठा ehci_pipe *pipe, काष्ठा ehci_qtd *td, u32 end)
{
    u32 status;
    क्रम (;;) {
        status = td->token;
        अगर (!(status & QTD_STS_ACTIVE))
            अवरोध;
        अगर (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 अग्र = GET_LOWFLAT(pipe->qh.qtd_अग्र);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, अग्र, td, status);
            ehci_reset_pipe(pipe);
            काष्ठा usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), काष्ठा usb_ehci_s, usb);
            ehci_waittick(cntl);
            वापस -1;
        }
        yield();
    }
    अगर (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        वापस -2;
    }
    वापस 0;
}

अटल व्योम
ehci_fill_tdbuf(काष्ठा ehci_qtd *td, u32 dest, पूर्णांक transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    क्रम (; dest < end; dest = ALIGN_DOWN(dest + PAGE_आकार, PAGE_आकार))
        *pos++ = dest;
}

#घोषणा STACKQTDS 6

पूर्णांक
ehci_send_pipe(काष्ठा usb_pipe *p, पूर्णांक dir, स्थिर व्योम *cmd
               , व्योम *तथ्य, पूर्णांक datasize)
{
    अगर (! CONFIG_USB_EHCI)
        वापस -1;
    काष्ठा ehci_pipe *pipe = container_of(p, काष्ठा ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, तथ्य, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[माप(काष्ठा ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    काष्ठा ehci_qtd *tds = (व्योम*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    स_रखो(tds, 0, माप(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    अगर (cmd) {
        // Send setup pid on control transfers
        td->qtd_अग्र = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_अग्र = EHCI_PTR_TERM;
        td->token = (ehci_explen(USB_CONTROL_SETUP_आकार) | QTD_STS_ACTIVE
                     | QTD_PID_SETUP | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_SETUP_आकार);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)तथ्य, dataend = dest + datasize;
    जबतक (dest < dataend) {
        // Send data pids
        अगर (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            वापस -1;
        }
        पूर्णांक maxtransfer = 5*PAGE_आकार - (dest & (PAGE_आकार-1));
        पूर्णांक transfer = dataend - dest;
        अगर (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_अग्र = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_अग्र = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    अगर (cmd) {
        // Send status pid on control transfers
        अगर (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            वापस -1;
        }
        td->qtd_अग्र = EHCI_PTR_TERM;
        td->alt_अग्र = EHCI_PTR_TERM;
        td->token = (QTD_TOGGLE | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_OUT : QTD_PID_IN) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_अग्र = EHCI_PTR_TERM;
    barrier();
    SET_LOWFLAT(pipe->qh.qtd_अग्र, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    पूर्णांक i;
    क्रम (i=0, td=tds; i<STACKQTDS; i++, td++) {
        पूर्णांक ret = ehci_wait_td(pipe, td, end);
        अगर (ret)
            वापस -1;
    }

    वापस 0;
}

पूर्णांक
ehci_poll_intr(काष्ठा usb_pipe *p, व्योम *तथ्य)
{
    ASSERT16();
    अगर (! CONFIG_USB_EHCI)
        वापस -1;
    काष्ठा ehci_pipe *pipe = container_of(p, काष्ठा ehci_pipe, pipe);
    काष्ठा ehci_qtd *td = GET_LOWFLAT(pipe->अग्र_td);
    u32 token = GET_LOWFLAT(td->token);
    अगर (token & QTD_STS_ACTIVE)
        // No intrs found.
        वापस -1;
    // XXX - check for errors.

    // Copy data.
    पूर्णांक maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    पूर्णांक pos = td - GET_LOWFLAT(pipe->tds);
    व्योम *tddata = GET_LOWFLAT(pipe->तथ्य) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), तथ्य, SEG_निचला, LOWFLAT2_निचला(tddata), maxpacket);

    // Reenable this td.
    काष्ठा ehci_qtd *अग्र = (व्योम*)(GET_LOWFLAT(td->qtd_अग्र) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->अग्र_td, अग्र);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    वापस 0;
}
