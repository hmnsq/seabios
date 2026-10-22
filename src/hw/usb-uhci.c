// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_LOWFLAT
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pci.h" // pci_config_writew
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#அந்தர்பாவ "pci_regs.h" // PCI_BASE_ADDRESS_4
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "usb.h" // struct usb_s
#அந்தர்பாவ "usb-ehci.h" // ehci_wait_controllers
#அந்தர்பாவ "usb-uhci.h" // USBLEGSUP
#அந்தர்பாவ "util.h" // msleep
#அந்தர்பாவ "x86.h" // outw

கட்டமைப்பு usb_uhci_s {
    கட்டமைப்பு usb_s usb;
    u16 iobase;
    கட்டமைப்பு uhci_qh *control_qh;
    கட்டமைப்பு uhci_framelist *framelist;
};

கட்டமைப்பு uhci_pipe {
    கட்டமைப்பு uhci_qh qh;
    கட்டமைப்பு uhci_td *next_td;
    கட்டமைப்பு usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
நிலையான முழு
uhci_hub_detect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_uhci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    எனில் (!(status & USBPORTSC_CCS))
        // No device found.
        திரும்பு 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    திரும்பு 1;
}

// Reset device on port
நிலையான முழு
uhci_hub_reset(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_uhci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    எனில் (!(status & USBPORTSC_CCS))
        // No longer connected
        திரும்பு -1;
    outw(USBPORTSC_PE, ioport);
    திரும்பு !!(status & USBPORTSC_LSDA);
}

// Disable port
நிலையான வெறுமை
uhci_hub_disconnect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_uhci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

நிலையான கட்டமைப்பு usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
நிலையான முழு
check_uhci_ports(கட்டமைப்பு usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    கட்டமைப்பு usbhub_s hub;
    memset(&hub, 0, அளவு(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    திரும்பு hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
நிலையான வெறுமை
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    க்கு (;;) {
        எனில் (inw(iobase + USBFRNUM) != startframe)
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு;
        }
        yield();
    }
}

நிலையான வெறுமை
uhci_free_pipes(கட்டமைப்பு usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    கட்டமைப்பு uhci_qh *pos = (வெறுமை*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    க்கு (;;) {
        u32 link = pos->link;
        எனில் (link == UHCI_PTR_TERM)
            நிறுத்து;
        கட்டமைப்பு uhci_qh *next = (வெறுமை*)(link & ~UHCI_PTR_BITS);
        கட்டமைப்பு uhci_pipe *pipe = container_of(next, கட்டமைப்பு uhci_pipe, qh);
        எனில் (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        இல்லையெனில்
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    க்கு (;;) {
        கட்டமைப்பு usb_pipe *usbpipe = cntl->usb.freelist;
        எனில் (!usbpipe)
            நிறுத்து;
        cntl->usb.freelist = usbpipe->freenext;
        கட்டமைப்பு uhci_pipe *pipe = container_of(usbpipe, கட்டமைப்பு uhci_pipe, pipe);
        விடுவி(pipe);
    }
}

நிலையான வெறுமை
reset_uhci(கட்டமைப்பு usb_uhci_s *cntl, u16 bdf)
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

நிலையான வெறுமை
configure_uhci(வெறுமை *data)
{
    கட்டமைப்பு usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    கட்டமைப்பு uhci_td *term_td = malloc_உயர்வு(அளவு(*term_td));
    கட்டமைப்பு uhci_framelist *fl = memalign_உயர்வு(அளவு(*fl), அளவு(*fl));
    கட்டமைப்பு uhci_pipe *intr_pipe = malloc_உயர்வு(அளவு(*intr_pipe));
    கட்டமைப்பு uhci_pipe *term_pipe = malloc_உயர்வு(அளவு(*term_pipe));
    எனில் (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        செல் fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, அளவு(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, அளவு(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, அளவு(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    முழு i;
    க்கு (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    முழு மொத்தம் = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    எனில் (மொத்தம்)
        // Success
        திரும்பு;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    விடுவி(term_td);
    விடுவி(fl);
    விடுவி(intr_pipe);
    விடுவி(term_pipe);
    விடுவி(cntl);
}

நிலையான வெறுமை
uhci_controller_setup(கட்டமைப்பு pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    எனில் (!iobase)
        திரும்பு;

    கட்டமைப்பு usb_uhci_s *cntl = malloc_tmphigh(அளவு(*cntl));
    எனில் (!cntl) {
        warn_noalloc();
        திரும்பு;
    }
    memset(cntl, 0, அளவு(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

வெறுமை
uhci_setup(வெறுமை)
{
    எனில் (! CONFIG_USB_UHCI)
        திரும்பு;
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

நிலையான கட்டமைப்பு usb_pipe *
uhci_alloc_intr_pipe(கட்டமைப்பு usbdevice_s *usbdev
                     , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    கட்டமைப்பு usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_uhci_s, usb);
    முழு frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    எனில் (frameexp > 10)
        frameexp = 10;
    முழு maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    முழு ms = 1<<frameexp;
    முழு மொத்தம் = DIV_ROUND_UP(ticks_to_ms(2), ms);
    மொத்தம் = ALIGN(மொத்தம், 2);
    கட்டமைப்பு uhci_pipe *pipe = malloc_கீழ்(அளவு(*pipe));
    கட்டமைப்பு uhci_td *tds = malloc_கீழ்(அளவு(*tds) * மொத்தம்);
    வெறுமை *data = malloc_கீழ்(maxpacket * மொத்தம்);
    எனில் (!pipe || !tds || !data) {
        warn_noalloc();
        செல் fail;
    }
    memset(pipe, 0, அளவு(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    முழு lowspeed = pipe->pipe.speed;
    முழு devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    முழு toggle = 0;
    முழு i;
    க்கு (i=0; i<மொத்தம்; i++) {
        tds[i].link = (i==மொத்தம்-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    கட்டமைப்பு uhci_framelist *fl = cntl->framelist;
    எனில் (frameexp == 0) {
        // Add to existing interrupt entry.
        கட்டமைப்பு uhci_qh *intr_qh = (வெறுமை*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        எனில் (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } இல்லையெனில் {
        முழு startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        க்கு (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    திரும்பு &pipe->pipe;
fail:
    விடுவி(pipe);
    விடுவி(tds);
    விடுவி(data);
    திரும்பு வெற்று;
}

கட்டமைப்பு usb_pipe *
uhci_realloc_pipe(கட்டமைப்பு usbdevice_s *usbdev, கட்டமைப்பு usb_pipe *upipe
                  , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    எனில் (! CONFIG_USB_UHCI)
        திரும்பு வெற்று;
    usb_add_freelist(upipe);
    எனில் (!epdesc)
        திரும்பு வெற்று;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    எனில் (eptype == USB_ENDPOINT_XFER_INT)
        திரும்பு uhci_alloc_intr_pipe(usbdev, epdesc);
    கட்டமைப்பு usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    கட்டமைப்பு usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    எனில் (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        திரும்பு usbpipe;
    }

    // Allocate a new queue head.
    கட்டமைப்பு uhci_pipe *pipe;
    எனில் (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(அளவு(*pipe));
    இல்லையெனில்
        pipe = malloc_கீழ்(அளவு(*pipe));
    எனில் (!pipe) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(pipe, 0, அளவு(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    கட்டமைப்பு uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    எனில் (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    திரும்பு &pipe->pipe;
}

நிலையான முழு
wait_pipe(கட்டமைப்பு uhci_pipe *pipe, u32 end)
{
    க்கு (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        எனில் (el_link & UHCI_PTR_TERM)
            திரும்பு 0;
        எனில் (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            கட்டமைப்பு uhci_td *td = (வெறுமை*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (வெறுமை*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            திரும்பு -1;
        }
        yield();
    }
}

நிலையான முழு
wait_td(கட்டமைப்பு uhci_td *td, u32 end)
{
    u32 status;
    க்கு (;;) {
        status = td->status;
        எனில் (!(status & TD_CTRL_ACTIVE))
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
        }
        yield();
    }
    எனில் (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        திரும்பு -2;
    }
    திரும்பு 0;
}

#define STACKTDS 16
#define TDALIGN 16

முழு
uhci_send_pipe(கட்டமைப்பு usb_pipe *p, முழு dir, மாறிலி வெறுமை *cmd
               , வெறுமை *data, முழு datasize)
{
    எனில் (! CONFIG_USB_UHCI)
        திரும்பு -1;
    கட்டமைப்பு uhci_pipe *pipe = container_of(p, கட்டமைப்பு uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    முழு maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    முழு lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    முழு devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    முழு toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[அளவு(கட்டமைப்பு uhci_td) * STACKTDS + TDALIGN - 1];
    கட்டமைப்பு uhci_td *tds = (வெறுமை*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, அளவு(*tds) * STACKTDS);
    முழு tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    எனில் (cmd) {
        // Send setup pid on control transfers
        கட்டமைப்பு uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (வெறுமை*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    போது (datasize) {
        // Send data pids
        கட்டமைப்பு uhci_td *td = &tds[tdpos++ % STACKTDS];
        முழு ret = wait_td(td, end);
        எனில் (ret)
            செல் fail;

        முழு transfer = datasize;
        எனில் (transfer > maxpacket)
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
    எனில் (cmd) {
        // Send status pid on control transfers
        கட்டமைப்பு uhci_td *td = &tds[tdpos++ % STACKTDS];
        முழு ret = wait_td(td, end);
        எனில் (ret)
            செல் fail;
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
    திரும்பு wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    திரும்பு -1;
}

முழு
uhci_poll_intr(கட்டமைப்பு usb_pipe *p, வெறுமை *data)
{
    ASSERT16();
    எனில் (! CONFIG_USB_UHCI)
        திரும்பு -1;

    கட்டமைப்பு uhci_pipe *pipe = container_of(p, கட்டமைப்பு uhci_pipe, pipe);
    கட்டமைப்பு uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    எனில் (status & TD_CTRL_ACTIVE)
        // No intrs found.
        திரும்பு -1;
    // XXX - check for errors.

    // Copy data.
    வெறுமை *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_கீழ், LOWFLAT2_கீழ்(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    கட்டமைப்பு uhci_td *next = (வெறுமை*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    திரும்பு 0;
}
