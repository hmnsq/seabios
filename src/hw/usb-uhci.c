// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_LOWFLAT
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pci.h" // pci_config_writew
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#अन्तर्भाव "pci_regs.h" // PCI_BASE_ADDRESS_4
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "usb.h" // struct usb_s
#अन्तर्भाव "usb-ehci.h" // ehci_wait_controllers
#अन्तर्भाव "usb-uhci.h" // USBLEGSUP
#अन्तर्भाव "util.h" // msleep
#अन्तर्भाव "x86.h" // outw

संरचना usb_uhci_s {
    संरचना usb_s usb;
    u16 iobase;
    संरचना uhci_qh *control_qh;
    संरचना uhci_framelist *framelist;
};

संरचना uhci_pipe {
    संरचना uhci_qh qh;
    संरचना uhci_td *next_td;
    संरचना usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
स्थिर पूर्णाङ्क
uhci_hub_detect(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_uhci_s *cntl = container_of(hub->cntl, संरचना usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    यदि (!(status & USBPORTSC_CCS))
        // No device found.
        प्रत्यागम 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    प्रत्यागम 1;
}

// Reset device on port
स्थिर पूर्णाङ्क
uhci_hub_reset(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_uhci_s *cntl = container_of(hub->cntl, संरचना usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    यदि (!(status & USBPORTSC_CCS))
        // No longer connected
        प्रत्यागम -1;
    outw(USBPORTSC_PE, ioport);
    प्रत्यागम !!(status & USBPORTSC_LSDA);
}

// Disable port
स्थिर शून्य
uhci_hub_disconnect(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_uhci_s *cntl = container_of(hub->cntl, संरचना usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

स्थिर संरचना usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
स्थिर पूर्णाङ्क
check_uhci_ports(संरचना usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    संरचना usbhub_s hub;
    स_रखो(&hub, 0, परिमाण(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    प्रत्यागम hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
स्थिर शून्य
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    प्रति (;;) {
        यदि (inw(iobase + USBFRNUM) != startframe)
            भङ्ग;
        यदि (timer_check(end)) {
            warn_timeout();
            प्रत्यागम;
        }
        yield();
    }
}

स्थिर शून्य
uhci_free_pipes(संरचना usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    संरचना uhci_qh *pos = (शून्य*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    प्रति (;;) {
        u32 link = pos->link;
        यदि (link == UHCI_PTR_TERM)
            भङ्ग;
        संरचना uhci_qh *next = (शून्य*)(link & ~UHCI_PTR_BITS);
        संरचना uhci_pipe *pipe = container_of(next, संरचना uhci_pipe, qh);
        यदि (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        अन्यथा
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    प्रति (;;) {
        संरचना usb_pipe *usbpipe = cntl->usb.freelist;
        यदि (!usbpipe)
            भङ्ग;
        cntl->usb.freelist = usbpipe->freenext;
        संरचना uhci_pipe *pipe = container_of(usbpipe, संरचना uhci_pipe, pipe);
        मुक्त(pipe);
    }
}

स्थिर शून्य
reset_uhci(संरचना usb_uhci_s *cntl, u16 bdf)
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

स्थिर शून्य
configure_uhci(शून्य *data)
{
    संरचना usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    संरचना uhci_td *term_td = malloc_ऊपरी(परिमाण(*term_td));
    संरचना uhci_framelist *fl = memalign_ऊपरी(परिमाण(*fl), परिमाण(*fl));
    संरचना uhci_pipe *intr_pipe = malloc_ऊपरी(परिमाण(*intr_pipe));
    संरचना uhci_pipe *term_pipe = malloc_ऊपरी(परिमाण(*term_pipe));
    यदि (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        गच्छ fail;
    }

    // Work around for PIIX errata
    स_रखो(term_td, 0, परिमाण(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    स_रखो(term_pipe, 0, परिमाण(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    स_रखो(intr_pipe, 0, परिमाण(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    पूर्णाङ्क i;
    प्रति (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    पूर्णाङ्क गिनती = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    यदि (गिनती)
        // Success
        प्रत्यागम;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    मुक्त(term_td);
    मुक्त(fl);
    मुक्त(intr_pipe);
    मुक्त(term_pipe);
    मुक्त(cntl);
}

स्थिर शून्य
uhci_controller_setup(संरचना pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    यदि (!iobase)
        प्रत्यागम;

    संरचना usb_uhci_s *cntl = malloc_tmphigh(परिमाण(*cntl));
    यदि (!cntl) {
        warn_noalloc();
        प्रत्यागम;
    }
    स_रखो(cntl, 0, परिमाण(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

शून्य
uhci_setup(शून्य)
{
    यदि (! CONFIG_USB_UHCI)
        प्रत्यागम;
    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

स्थिर संरचना usb_pipe *
uhci_alloc_intr_pipe(संरचना usbdevice_s *usbdev
                     , संरचना usb_endpoint_descriptor *epdesc)
{
    संरचना usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, संरचना usb_uhci_s, usb);
    पूर्णाङ्क frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    यदि (frameexp > 10)
        frameexp = 10;
    पूर्णाङ्क maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    पूर्णाङ्क ms = 1<<frameexp;
    पूर्णाङ्क गिनती = DIV_ROUND_UP(ticks_to_ms(2), ms);
    गिनती = ALIGN(गिनती, 2);
    संरचना uhci_pipe *pipe = malloc_निचला(परिमाण(*pipe));
    संरचना uhci_td *tds = malloc_निचला(परिमाण(*tds) * गिनती);
    शून्य *data = malloc_निचला(maxpacket * गिनती);
    यदि (!pipe || !tds || !data) {
        warn_noalloc();
        गच्छ fail;
    }
    स_रखो(pipe, 0, परिमाण(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    पूर्णाङ्क lowspeed = pipe->pipe.speed;
    पूर्णाङ्क devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    पूर्णाङ्क toggle = 0;
    पूर्णाङ्क i;
    प्रति (i=0; i<गिनती; i++) {
        tds[i].link = (i==गिनती-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    संरचना uhci_framelist *fl = cntl->framelist;
    यदि (frameexp == 0) {
        // Add to existing interrupt entry.
        संरचना uhci_qh *intr_qh = (शून्य*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        यदि (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } अन्यथा {
        पूर्णाङ्क startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        प्रति (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    प्रत्यागम &pipe->pipe;
fail:
    मुक्त(pipe);
    मुक्त(tds);
    मुक्त(data);
    प्रत्यागम रिक्त;
}

संरचना usb_pipe *
uhci_realloc_pipe(संरचना usbdevice_s *usbdev, संरचना usb_pipe *upipe
                  , संरचना usb_endpoint_descriptor *epdesc)
{
    यदि (! CONFIG_USB_UHCI)
        प्रत्यागम रिक्त;
    usb_add_freelist(upipe);
    यदि (!epdesc)
        प्रत्यागम रिक्त;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    यदि (eptype == USB_ENDPOINT_XFER_INT)
        प्रत्यागम uhci_alloc_intr_pipe(usbdev, epdesc);
    संरचना usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, संरचना usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    संरचना usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    यदि (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        प्रत्यागम usbpipe;
    }

    // Allocate a new queue head.
    संरचना uhci_pipe *pipe;
    यदि (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(परिमाण(*pipe));
    अन्यथा
        pipe = malloc_निचला(परिमाण(*pipe));
    यदि (!pipe) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स_रखो(pipe, 0, परिमाण(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    संरचना uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    यदि (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    प्रत्यागम &pipe->pipe;
}

स्थिर पूर्णाङ्क
wait_pipe(संरचना uhci_pipe *pipe, u32 end)
{
    प्रति (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        यदि (el_link & UHCI_PTR_TERM)
            प्रत्यागम 0;
        यदि (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            संरचना uhci_td *td = (शून्य*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (शून्य*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            प्रत्यागम -1;
        }
        yield();
    }
}

स्थिर पूर्णाङ्क
wait_td(संरचना uhci_td *td, u32 end)
{
    u32 status;
    प्रति (;;) {
        status = td->status;
        यदि (!(status & TD_CTRL_ACTIVE))
            भङ्ग;
        यदि (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
        }
        yield();
    }
    यदि (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        प्रत्यागम -2;
    }
    प्रत्यागम 0;
}

#define STACKTDS 16
#define TDALIGN 16

पूर्णाङ्क
uhci_send_pipe(संरचना usb_pipe *p, पूर्णाङ्क dir, ध्रुव शून्य *cmd
               , शून्य *data, पूर्णाङ्क datasize)
{
    यदि (! CONFIG_USB_UHCI)
        प्रत्यागम -1;
    संरचना uhci_pipe *pipe = container_of(p, संरचना uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    पूर्णाङ्क maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    पूर्णाङ्क lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    पूर्णाङ्क devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    पूर्णाङ्क toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[परिमाण(संरचना uhci_td) * STACKTDS + TDALIGN - 1];
    संरचना uhci_td *tds = (शून्य*)ALIGN((u32)tdsbuf, TDALIGN);
    स_रखो(tds, 0, परिमाण(*tds) * STACKTDS);
    पूर्णाङ्क tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    यदि (cmd) {
        // Send setup pid on control transfers
        संरचना uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (शून्य*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    यावत् (datasize) {
        // Send data pids
        संरचना uhci_td *td = &tds[tdpos++ % STACKTDS];
        पूर्णाङ्क ret = wait_td(td, end);
        यदि (ret)
            गच्छ fail;

        पूर्णाङ्क transfer = datasize;
        यदि (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_TERM : (nexttd | UHCI_PTR_DEPTH));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_IN : USB_PID_OUT));
        td->buffer = data;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle ^= TD_TOKEN_TOGGLE;

        data += transfer;
        datasize -= transfer;
    }
    यदि (cmd) {
        // Send status pid on control transfers
        संरचना uhci_td *td = &tds[tdpos++ % STACKTDS];
        पूर्णाङ्क ret = wait_td(td, end);
        यदि (ret)
            गच्छ fail;
        td->link = UHCI_PTR_TERM;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_OUT : USB_PID_IN));
        td->buffer = 0;
        barrier();
        td->status = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
    }
    SET_LOWFLAT(pipe->toggle, !!toggle);
    प्रत्यागम wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    प्रत्यागम -1;
}

पूर्णाङ्क
uhci_poll_intr(संरचना usb_pipe *p, शून्य *data)
{
    ASSERT16();
    यदि (! CONFIG_USB_UHCI)
        प्रत्यागम -1;

    संरचना uhci_pipe *pipe = container_of(p, संरचना uhci_pipe, pipe);
    संरचना uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    यदि (status & TD_CTRL_ACTIVE)
        // No intrs found.
        प्रत्यागम -1;
    // XXX - check for errors.

    // Copy data.
    शून्य *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_निचला, LOWFLAT2_निचला(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    संरचना uhci_td *next = (शून्य*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    प्रत्यागम 0;
}
