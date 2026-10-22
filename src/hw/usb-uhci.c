// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_LOWFLAT
#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "pci.h" // pci_config_writew
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#समावेश "pci_regs.h" // PCI_BASE_ADDRESS_4
#समावेश "string.h" // memset
#समावेश "usb.h" // struct usb_s
#समावेश "usb-ehci.h" // ehci_wait_controllers
#समावेश "usb-uhci.h" // USBLEGSUP
#समावेश "util.h" // msleep
#समावेश "x86.h" // outw

काष्ठा usb_uhci_s {
    काष्ठा usb_s usb;
    u16 iobase;
    काष्ठा uhci_qh *control_qh;
    काष्ठा uhci_framelist *framelist;
};

काष्ठा uhci_pipe {
    काष्ठा uhci_qh qh;
    काष्ठा uhci_td *अग्र_td;
    काष्ठा usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
अटल पूर्णांक
uhci_hub_detect(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_uhci_s *cntl = container_of(hub->cntl, काष्ठा usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    अगर (!(status & USBPORTSC_CCS))
        // No device found.
        वापस 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    वापस 1;
}

// Reset device on port
अटल पूर्णांक
uhci_hub_reset(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_uhci_s *cntl = container_of(hub->cntl, काष्ठा usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    अगर (!(status & USBPORTSC_CCS))
        // No longer connected
        वापस -1;
    outw(USBPORTSC_PE, ioport);
    वापस !!(status & USBPORTSC_LSDA);
}

// Disable port
अटल व्योम
uhci_hub_disconnect(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_uhci_s *cntl = container_of(hub->cntl, काष्ठा usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

अटल काष्ठा usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
अटल पूर्णांक
check_uhci_ports(काष्ठा usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    काष्ठा usbhub_s hub;
    स_रखो(&hub, 0, माप(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    वापस hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
अटल व्योम
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    क्रम (;;) {
        अगर (inw(iobase + USBFRNUM) != startframe)
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            वापस;
        }
        yield();
    }
}

अटल व्योम
uhci_free_pipes(काष्ठा usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    काष्ठा uhci_qh *pos = (व्योम*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    क्रम (;;) {
        u32 link = pos->link;
        अगर (link == UHCI_PTR_TERM)
            अवरोध;
        काष्ठा uhci_qh *अग्र = (व्योम*)(link & ~UHCI_PTR_BITS);
        काष्ठा uhci_pipe *pipe = container_of(अग्र, काष्ठा uhci_pipe, qh);
        अगर (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = अग्र->link;
        अन्यथा
            pos = अग्र;
    }
    uhci_waittick(cntl->iobase);
    क्रम (;;) {
        काष्ठा usb_pipe *usbpipe = cntl->usb.freelist;
        अगर (!usbpipe)
            अवरोध;
        cntl->usb.freelist = usbpipe->freenext;
        काष्ठा uhci_pipe *pipe = container_of(usbpipe, काष्ठा uhci_pipe, pipe);
        मुक्त(pipe);
    }
}

अटल व्योम
reset_uhci(काष्ठा usb_uhci_s *cntl, u16 bdf)
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

अटल व्योम
configure_uhci(व्योम *तथ्य)
{
    काष्ठा usb_uhci_s *cntl = तथ्य;

    // Allocate ram for schedule storage
    काष्ठा uhci_td *term_td = malloc_ऊपरी(माप(*term_td));
    काष्ठा uhci_framelist *fl = memalign_ऊपरी(माप(*fl), माप(*fl));
    काष्ठा uhci_pipe *intr_pipe = malloc_ऊपरी(माप(*intr_pipe));
    काष्ठा uhci_pipe *term_pipe = malloc_ऊपरी(माप(*term_pipe));
    अगर (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        जाओ fail;
    }

    // Work around for PIIX errata
    स_रखो(term_td, 0, माप(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    स_रखो(term_pipe, 0, माप(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    स_रखो(intr_pipe, 0, माप(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    पूर्णांक i;
    क्रम (i=0; i<ARRAY_आकार(fl->links); i++)
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
    पूर्णांक गिनती = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    अगर (गिनती)
        // Success
        वापस;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    मुक्त(term_td);
    मुक्त(fl);
    मुक्त(intr_pipe);
    मुक्त(term_pipe);
    मुक्त(cntl);
}

अटल व्योम
uhci_controller_setup(काष्ठा pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    अगर (!iobase)
        वापस;

    काष्ठा usb_uhci_s *cntl = malloc_tmphigh(माप(*cntl));
    अगर (!cntl) {
        warn_noalloc();
        वापस;
    }
    स_रखो(cntl, 0, माप(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

व्योम
uhci_setup(व्योम)
{
    अगर (! CONFIG_USB_UHCI)
        वापस;
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

अटल काष्ठा usb_pipe *
uhci_alloc_intr_pipe(काष्ठा usbdevice_s *usbdev
                     , काष्ठा usb_endpoint_descriptor *epdesc)
{
    काष्ठा usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, काष्ठा usb_uhci_s, usb);
    पूर्णांक frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    अगर (frameexp > 10)
        frameexp = 10;
    पूर्णांक maxpacket = epdesc->w_Max_Packet_आकार;
    // Determine number of entries needed for 2 timer ticks.
    पूर्णांक ms = 1<<frameexp;
    पूर्णांक गिनती = DIV_ROUND_UP(ticks_to_ms(2), ms);
    गिनती = ALIGN(गिनती, 2);
    काष्ठा uhci_pipe *pipe = malloc_निचला(माप(*pipe));
    काष्ठा uhci_td *tds = malloc_निचला(माप(*tds) * गिनती);
    व्योम *तथ्य = malloc_निचला(maxpacket * गिनती);
    अगर (!pipe || !tds || !तथ्य) {
        warn_noalloc();
        जाओ fail;
    }
    स_रखो(pipe, 0, माप(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    पूर्णांक lowspeed = pipe->pipe.speed;
    पूर्णांक devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->अग्र_td = &tds[0];
    pipe->iobase = cntl->iobase;

    पूर्णांक toggle = 0;
    पूर्णांक i;
    क्रम (i=0; i<गिनती; i++) {
        tds[i].link = (i==गिनती-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].बफर = तथ्य + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    काष्ठा uhci_framelist *fl = cntl->framelist;
    अगर (frameexp == 0) {
        // Add to existing interrupt entry.
        काष्ठा uhci_qh *intr_qh = (व्योम*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        अगर (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } अन्यथा {
        पूर्णांक startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        क्रम (i=startpos; i<ARRAY_आकार(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    वापस &pipe->pipe;
fail:
    मुक्त(pipe);
    मुक्त(tds);
    मुक्त(तथ्य);
    वापस शून्य;
}

काष्ठा usb_pipe *
uhci_realloc_pipe(काष्ठा usbdevice_s *usbdev, काष्ठा usb_pipe *upipe
                  , काष्ठा usb_endpoint_descriptor *epdesc)
{
    अगर (! CONFIG_USB_UHCI)
        वापस शून्य;
    usb_add_freelist(upipe);
    अगर (!epdesc)
        वापस शून्य;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    अगर (eptype == USB_ENDPOINT_XFER_INT)
        वापस uhci_alloc_intr_pipe(usbdev, epdesc);
    काष्ठा usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, काष्ठा usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    काष्ठा usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    अगर (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        वापस usbpipe;
    }

    // Allocate a new queue head.
    काष्ठा uhci_pipe *pipe;
    अगर (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(माप(*pipe));
    अन्यथा
        pipe = malloc_निचला(माप(*pipe));
    अगर (!pipe) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(pipe, 0, माप(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    काष्ठा uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    अगर (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    वापस &pipe->pipe;
}

अटल पूर्णांक
wait_pipe(काष्ठा uhci_pipe *pipe, u32 end)
{
    क्रम (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        अगर (el_link & UHCI_PTR_TERM)
            वापस 0;
        अगर (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            काष्ठा uhci_td *td = (व्योम*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (व्योम*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            वापस -1;
        }
        yield();
    }
}

अटल पूर्णांक
wait_td(काष्ठा uhci_td *td, u32 end)
{
    u32 status;
    क्रम (;;) {
        status = td->status;
        अगर (!(status & TD_CTRL_ACTIVE))
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
        }
        yield();
    }
    अगर (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        वापस -2;
    }
    वापस 0;
}

#घोषणा STACKTDS 16
#घोषणा TDALIGN 16

पूर्णांक
uhci_send_pipe(काष्ठा usb_pipe *p, पूर्णांक dir, स्थिर व्योम *cmd
               , व्योम *तथ्य, पूर्णांक datasize)
{
    अगर (! CONFIG_USB_UHCI)
        वापस -1;
    काष्ठा uhci_pipe *pipe = container_of(p, काष्ठा uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, तथ्य, datasize);
    पूर्णांक maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    पूर्णांक lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    पूर्णांक devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    पूर्णांक toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[माप(काष्ठा uhci_td) * STACKTDS + TDALIGN - 1];
    काष्ठा uhci_td *tds = (व्योम*)ALIGN((u32)tdsbuf, TDALIGN);
    स_रखो(tds, 0, माप(*tds) * STACKTDS);
    पूर्णांक tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    अगर (cmd) {
        // Send setup pid on control transfers
        काष्ठा uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_आकार)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->बफर = (व्योम*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    जबतक (datasize) {
        // Send data pids
        काष्ठा uhci_td *td = &tds[tdpos++ % STACKTDS];
        पूर्णांक ret = wait_td(td, end);
        अगर (ret)
            जाओ fail;

        पूर्णांक transfer = datasize;
        अगर (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_TERM : (nexttd | UHCI_PTR_DEPTH));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_IN : USB_PID_OUT));
        td->बफर = तथ्य;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle ^= TD_TOKEN_TOGGLE;

        तथ्य += transfer;
        datasize -= transfer;
    }
    अगर (cmd) {
        // Send status pid on control transfers
        काष्ठा uhci_td *td = &tds[tdpos++ % STACKTDS];
        पूर्णांक ret = wait_td(td, end);
        अगर (ret)
            जाओ fail;
        td->link = UHCI_PTR_TERM;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_OUT : USB_PID_IN));
        td->बफर = 0;
        barrier();
        td->status = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
    }
    SET_LOWFLAT(pipe->toggle, !!toggle);
    वापस wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    वापस -1;
}

पूर्णांक
uhci_poll_intr(काष्ठा usb_pipe *p, व्योम *तथ्य)
{
    ASSERT16();
    अगर (! CONFIG_USB_UHCI)
        वापस -1;

    काष्ठा uhci_pipe *pipe = container_of(p, काष्ठा uhci_pipe, pipe);
    काष्ठा uhci_td *td = GET_LOWFLAT(pipe->अग्र_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    अगर (status & TD_CTRL_ACTIVE)
        // No intrs found.
        वापस -1;
    // XXX - check for errors.

    // Copy data.
    व्योम *tddata = GET_LOWFLAT(td->बफर);
    memcpy_far(GET_SEG(SS), तथ्य, SEG_निचला, LOWFLAT2_निचला(tddata)
               , uhci_expected_लंबाई(token));

    // Reenable this td.
    काष्ठा uhci_td *अग्र = (व्योम*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->अग्र_td, अग्र);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    वापस 0;
}
