// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_LOWFLAT
#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "pci.h" // pci_config_writew
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#ضمن "pci_regs.h" // PCI_BASE_ADDRESS_4
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-ehci.h" // ehci_wait_controllers
#ضمن "usb-uhci.h" // USBLEGSUP
#ضمن "util.h" // msleep
#ضمن "x86.h" // outw

بنية usb_uhci_s {
    بنية usb_s usb;
    u16 iobase;
    بنية uhci_qh *control_qh;
    بنية uhci_framelist *framelist;
};

بنية uhci_pipe {
    بنية uhci_qh qh;
    بنية uhci_td *اگلا_td;
    بنية usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
ساكن صحيح
uhci_hub_detect(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_uhci_s *cntl = container_دا(hub->cntl, بنية usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + پورٹ * 2;
    u16 حالت = inw(ioport);
    إذا (!(حالت & USBPORTSC_CCS))
        // No device found.
        أرجع 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_ویلا_DRSTR);
    أرجع 1;
}

// Reset device on port
ساكن صحيح
uhci_hub_reset(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_uhci_s *cntl = container_دا(hub->cntl, بنية usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + پورٹ * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 حالت = inw(ioport);
    إذا (!(حالت & USBPORTSC_CCS))
        // No longer connected
        أرجع -1;
    outw(USBPORTSC_PE, ioport);
    أرجع !!(حالت & USBPORTSC_LSDA);
}

// Disable port
ساكن فراغ
uhci_hub_disconnect(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_uhci_s *cntl = container_دا(hub->cntl, بنية usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + پورٹ * 2;
    outw(0, ioport);
}

ساكن بنية usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساكن صحيح
check_uhci_ports(بنية usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    بنية usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    أرجع hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ساكن فراغ
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 سرا = timer_calc(1000 * 5);
    لكل (;;) {
        إذا (inw(iobase + USBFRNUM) != startframe)
            اكسر;
        إذا (timer_check(سرا)) {
            warn_timeout();
            أرجع;
        }
        yield();
    }
}

ساكن فراغ
uhci_free_pipes(بنية usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    بنية uhci_qh *pos = (فراغ*)(cntl->framelist->links[0] & ~UHCI_PTR_بٹ);
    لكل (;;) {
        u32 link = pos->link;
        إذا (link == UHCI_PTR_رکن)
            اكسر;
        بنية uhci_qh *اگلا = (فراغ*)(link & ~UHCI_PTR_بٹ);
        بنية uhci_pipe *pipe = container_دا(اگلا, بنية uhci_pipe, qh);
        إذا (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = اگلا->link;
        وإلا
            pos = اگلا;
    }
    uhci_waittick(cntl->iobase);
    لكل (;;) {
        بنية usb_pipe *usbpipe = cntl->usb.freelist;
        إذا (!usbpipe)
            اكسر;
        cntl->usb.freelist = usbpipe->freenext;
        بنية uhci_pipe *pipe = container_دا(usbpipe, بنية uhci_pipe, pipe);
        حرر(pipe);
    }
}

ساكن فراغ
reset_uhci(بنية usb_uhci_s *cntl, u16 bdf)
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

ساكن فراغ
configure_uhci(فراغ *انکڑے)
{
    بنية usb_uhci_s *cntl = انکڑے;

    // Allocate ram for schedule storage
    بنية uhci_td *رکن_td = malloc_بلند(حجم(*رکن_td));
    بنية uhci_framelist *fl = memalign_بلند(حجم(*fl), حجم(*fl));
    بنية uhci_pipe *intr_pipe = malloc_بلند(حجم(*intr_pipe));
    بنية uhci_pipe *رکن_pipe = malloc_بلند(حجم(*رکن_pipe));
    إذا (!رکن_td || !fl || !intr_pipe || !رکن_pipe) {
        warn_noalloc();
        اذهب fail;
    }

    // Work around for PIIX errata
    memset(رکن_td, 0, حجم(*رکن_td));
    رکن_td->link = UHCI_PTR_رکن;
    رکن_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_کھسکاؤ)
                      | USB_PID_وچ);
    memset(رکن_pipe, 0, حجم(*رکن_pipe));
    رکن_pipe->qh.element = (u32)رکن_td;
    رکن_pipe->qh.link = UHCI_PTR_رکن;
    رکن_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, حجم(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_رکن;
    intr_pipe->qh.link = (u32)&رکن_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    صحيح i;
    لكل (i=0; i<ARRAY_ناپ(fl->links); i++)
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
    صحيح گنتی = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    إذا (گنتی)
        // Success
        أرجع;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    حرر(رکن_td);
    حرر(fl);
    حرر(intr_pipe);
    حرر(رکن_pipe);
    حرر(cntl);
}

ساكن فراغ
uhci_controller_تیاری(بنية pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_ادھار_پتہ_4);
    إذا (!iobase)
        أرجع;

    بنية usb_uhci_s *cntl = malloc_tmphigh(حجم(*cntl));
    إذا (!cntl) {
        warn_noalloc();
        أرجع;
    }
    memset(cntl, 0, حجم(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    لگاتار_thread(configure_uhci, cntl);
}

فراغ
uhci_تیاری(فراغ)
{
    إذا (! CONFIG_USB_UHCI)
        أرجع;
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_تیاری(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

ساكن بنية usb_pipe *
uhci_alloc_intr_pipe(بنية usbdevice_s *usbdev
                     , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_uhci_s *cntl = container_دا(
        usbdev->hub->cntl, بنية usb_uhci_s, usb);
    صحيح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    إذا (frameexp > 10)
        frameexp = 10;
    صحيح maxpacket = epdesc->w_Max_Packet_ناپ;
    // Determine number of entries needed for 2 timer ticks.
    صحيح ms = 1<<frameexp;
    صحيح گنتی = DIV_گیڑا_اتے(ticks_منزل_ms(2), ms);
    گنتی = ALIGN(گنتی, 2);
    بنية uhci_pipe *pipe = malloc_کم(حجم(*pipe));
    بنية uhci_td *tds = malloc_کم(حجم(*tds) * گنتی);
    فراغ *انکڑے = malloc_کم(maxpacket * گنتی);
    إذا (!pipe || !tds || !انکڑے) {
        warn_noalloc();
        اذهب fail;
    }
    memset(pipe, 0, حجم(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    صحيح lowspeed = pipe->pipe.رفتار;
    صحيح devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->اگلا_td = &tds[0];
    pipe->iobase = cntl->iobase;

    صحيح toggle = 0;
    صحيح i;
    لكل (i=0; i<گنتی; i++) {
        tds[i].link = (i==گنتی-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].حالت = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_سرگرم);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_کھسکاؤ)
                        | USB_PID_وچ);
        tds[i].بفر = انکڑے + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    بنية uhci_framelist *fl = cntl->framelist;
    إذا (frameexp == 0) {
        // Add to existing interrupt entry.
        بنية uhci_qh *intr_qh = (فراغ*)(fl->links[0] & ~UHCI_PTR_بٹ);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        إذا (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } وإلا {
        صحيح startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        لكل (i=startpos; i<ARRAY_ناپ(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    أرجع &pipe->pipe;
fail:
    حرر(pipe);
    حرر(tds);
    حرر(انکڑے);
    أرجع عدم;
}

بنية usb_pipe *
uhci_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *upipe
                  , بنية usb_endpoint_descriptor *epdesc)
{
    إذا (! CONFIG_USB_UHCI)
        أرجع عدم;
    usb_جوڑو_freelist(upipe);
    إذا (!epdesc)
        أرجع عدم;
    u8 eptype = epdesc->bm_گن & USB_ENDPOINT_XFERTYPE_نقاب;
    إذا (eptype == USB_ENDPOINT_XFER_INT)
        أرجع uhci_alloc_intr_pipe(usbdev, epdesc);
    بنية usb_uhci_s *cntl = container_دا(
        usbdev->hub->cntl, بنية usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    بنية usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    إذا (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        أرجع usbpipe;
    }

    // Allocate a new queue head.
    بنية uhci_pipe *pipe;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(حجم(*pipe));
    وإلا
        pipe = malloc_کم(حجم(*pipe));
    إذا (!pipe) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(pipe, 0, حجم(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_رکن;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    بنية uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    أرجع &pipe->pipe;
}

ساكن صحيح
wait_pipe(بنية uhci_pipe *pipe, u32 سرا)
{
    لكل (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        إذا (el_link & UHCI_PTR_رکن)
            أرجع 0;
        إذا (timer_check(سرا)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            بنية uhci_td *td = (فراغ*)(el_link & ~UHCI_PTR_بٹ);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (فراغ*)el_link, GET_LOWFLAT(td->حالت)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            سیٹ_LOWFLAT(pipe->qh.element, UHCI_PTR_رکن);
            uhci_waittick(iobase);
            أرجع -1;
        }
        yield();
    }
}

ساكن صحيح
wait_td(بنية uhci_td *td, u32 سرا)
{
    u32 حالت;
    لكل (;;) {
        حالت = td->حالت;
        إذا (!(حالت & TD_CTRL_سرگرم))
            اكسر;
        إذا (timer_check(سرا)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
    إذا (حالت & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", حالت);
        أرجع -2;
    }
    أرجع 0;
}

#تعریف_کرو STACKTDS 16
#تعریف_کرو TDALIGN 16

صحيح
uhci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *انکڑے, صحيح datasize)
{
    إذا (! CONFIG_USB_UHCI)
        أرجع -1;
    بنية uhci_pipe *pipe = container_دا(p, بنية uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, انکڑے, datasize);
    صحيح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحيح lowspeed = GET_LOWFLAT(pipe->pipe.رفتار);
    صحيح devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    صحيح toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[حجم(بنية uhci_td) * STACKTDS + TDALIGN - 1];
    بنية uhci_td *tds = (فراغ*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, حجم(*tds) * STACKTDS);
    صحيح tdpos = 0;

    // Enable tds
    u32 سرا = timer_calc(usb_xfer_ویلا(p, datasize));
    barrier();
    سیٹ_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    إذا (cmd) {
        // Send setup pid on control transfers
        بنية uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_ڈونگھائی;
        td->token = (uhci_explen(USB_CONTROL_تیاری_ناپ)
                     | (devaddr << TD_TOKEN_DEVADDR_کھسکاؤ) | USB_PID_تیاری);
        td->بفر = (فراغ*)cmd;
        barrier();
        td->حالت = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_سرگرم);
        toggle = TD_TOKEN_TOGGLE;
    }
    طالما (datasize) {
        // Send data pids
        بنية uhci_td *td = &tds[tdpos++ % STACKTDS];
        صحيح ret = wait_td(td, سرا);
        إذا (ret)
            اذهب fail;

        صحيح transfer = datasize;
        إذا (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_رکن : (nexttd | UHCI_PTR_ڈونگھائی));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_کھسکاؤ)
                     | (dir ? USB_PID_وچ : USB_PID_باہرلا));
        td->بفر = انکڑے;
        barrier();
        td->حالت = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_سرگرم);
        toggle ^= TD_TOKEN_TOGGLE;

        انکڑے += transfer;
        datasize -= transfer;
    }
    إذا (cmd) {
        // Send status pid on control transfers
        بنية uhci_td *td = &tds[tdpos++ % STACKTDS];
        صحيح ret = wait_td(td, سرا);
        إذا (ret)
            اذهب fail;
        td->link = UHCI_PTR_رکن;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_کھسکاؤ)
                     | (dir ? USB_PID_باہرلا : USB_PID_وچ));
        td->بفر = 0;
        barrier();
        td->حالت = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_سرگرم);
    }
    سیٹ_LOWFLAT(pipe->toggle, !!toggle);
    أرجع wait_pipe(pipe, سرا);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    سیٹ_LOWFLAT(pipe->qh.element, UHCI_PTR_رکن);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    أرجع -1;
}

صحيح
uhci_poll_intr(بنية usb_pipe *p, فراغ *انکڑے)
{
    ASSERT16();
    إذا (! CONFIG_USB_UHCI)
        أرجع -1;

    بنية uhci_pipe *pipe = container_دا(p, بنية uhci_pipe, pipe);
    بنية uhci_td *td = GET_LOWFLAT(pipe->اگلا_td);
    u32 حالت = GET_LOWFLAT(td->حالت);
    u32 token = GET_LOWFLAT(td->token);
    إذا (حالت & TD_CTRL_سرگرم)
        // No intrs found.
        أرجع -1;
    // XXX - check for errors.

    // Copy data.
    فراغ *tddata = GET_LOWFLAT(td->بفر);
    memcpy_far(GET_SEG(SS), انکڑے, SEG_کم, LOWFLAT2_کم(tddata)
               , uhci_expected_لمائی(token));

    // Reenable this td.
    بنية uhci_td *اگلا = (فراغ*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_بٹ);
    سیٹ_LOWFLAT(pipe->اگلا_td, اگلا);
    barrier();
    سیٹ_LOWFLAT(td->حالت, (uhci_maxerr(0) | (حالت & TD_CTRL_LS)
                             | TD_CTRL_سرگرم));

    أرجع 0;
}
