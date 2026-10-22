// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_LOWFLAT
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pci.h" // pci_config_writew
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#شامل "pci_regs.h" // PCI_BASE_ADDRESS_4
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-ehci.h" // ehci_wait_controllers
#شامل "usb-uhci.h" // USBLEGSUP
#شامل "util.h" // msleep
#شامل "x86.h" // outw

ساخت usb_uhci_s {
    ساخت usb_s usb;
    u16 iobase;
    ساخت uhci_qh *control_qh;
    ساخت uhci_framelist *framelist;
};

ساخت uhci_pipe {
    ساخت uhci_qh qh;
    ساخت uhci_td *اگلا_td;
    ساخت usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
ساکن صحیح
uhci_hub_detect(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_uhci_s *cntl = container_کا(hub->cntl, ساخت usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + پورٹ * 2;
    u16 حالت_2 = inw(ioport);
    اگر (!(حالت_2 & USBPORTSC_CCS))
        // No device found.
        واپس 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_وقت_DRSTR);
    واپس 1;
}

// Reset device on port
ساکن صحیح
uhci_hub_reset(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_uhci_s *cntl = container_کا(hub->cntl, ساخت usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + پورٹ * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 حالت_2 = inw(ioport);
    اگر (!(حالت_2 & USBPORTSC_CCS))
        // No longer connected
        واپس -1;
    outw(USBPORTSC_PE, ioport);
    واپس !!(حالت_2 & USBPORTSC_LSDA);
}

// Disable port
ساکن خالی
uhci_hub_disconnect(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_uhci_s *cntl = container_کا(hub->cntl, ساخت usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + پورٹ * 2;
    outw(0, ioport);
}

ساکن ساخت usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساکن صحیح
check_uhci_ports(ساخت usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    ساخت usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    واپس hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ساکن خالی
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 انتہا = timer_calc(1000 * 5);
    برائے (;;) {
        اگر (inw(iobase + USBFRNUM) != startframe)
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس;
        }
        yield();
    }
}

ساکن خالی
uhci_free_pipes(ساخت usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    ساخت uhci_qh *pos = (خالی*)(cntl->framelist->links[0] & ~UHCI_PTR_بٹ);
    برائے (;;) {
        u32 link = pos->link;
        اگر (link == UHCI_PTR_رکن)
            توڑو;
        ساخت uhci_qh *اگلا = (خالی*)(link & ~UHCI_PTR_بٹ);
        ساخت uhci_pipe *pipe = container_کا(اگلا, ساخت uhci_pipe, qh);
        اگر (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = اگلا->link;
        ورنہ
            pos = اگلا;
    }
    uhci_waittick(cntl->iobase);
    برائے (;;) {
        ساخت usb_pipe *usbpipe = cntl->usb.freelist;
        اگر (!usbpipe)
            توڑو;
        cntl->usb.freelist = usbpipe->freenext;
        ساخت uhci_pipe *pipe = container_کا(usbpipe, ساخت uhci_pipe, pipe);
        آزاد(pipe);
    }
}

ساکن خالی
reset_uhci(ساخت usb_uhci_s *cntl, u16 bdf)
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

ساکن خالی
configure_uhci(خالی *ڈیٹا)
{
    ساخت usb_uhci_s *cntl = ڈیٹا;

    // Allocate ram for schedule storage
    ساخت uhci_td *رکن_td = malloc_بلند(حجم(*رکن_td));
    ساخت uhci_framelist *fl = memalign_بلند(حجم(*fl), حجم(*fl));
    ساخت uhci_pipe *intr_pipe = malloc_بلند(حجم(*intr_pipe));
    ساخت uhci_pipe *رکن_pipe = malloc_بلند(حجم(*رکن_pipe));
    اگر (!رکن_td || !fl || !intr_pipe || !رکن_pipe) {
        warn_noalloc();
        جاؤ fail;
    }

    // Work around for PIIX errata
    memset(رکن_td, 0, حجم(*رکن_td));
    رکن_td->link = UHCI_PTR_رکن;
    رکن_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_سرکاؤ)
                      | USB_PID_اندر);
    memset(رکن_pipe, 0, حجم(*رکن_pipe));
    رکن_pipe->qh.element = (u32)رکن_td;
    رکن_pipe->qh.link = UHCI_PTR_رکن;
    رکن_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, حجم(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_رکن;
    intr_pipe->qh.link = (u32)&رکن_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    صحیح i;
    برائے (i=0; i<ARRAY_سائز(fl->links); i++)
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
    صحیح گنتی = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    اگر (گنتی)
        // Success
        واپس;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    آزاد(رکن_td);
    آزاد(fl);
    آزاد(intr_pipe);
    آزاد(رکن_pipe);
    آزاد(cntl);
}

ساکن خالی
uhci_controller_تیاری(ساخت pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_بنیاد_پتہ_4);
    اگر (!iobase)
        واپس;

    ساخت usb_uhci_s *cntl = malloc_tmphigh(حجم(*cntl));
    اگر (!cntl) {
        warn_noalloc();
        واپس;
    }
    memset(cntl, 0, حجم(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    تسلسل_thread(configure_uhci, cntl);
}

خالی
uhci_تیاری(خالی)
{
    اگر (! CONFIG_USB_UHCI)
        واپس;
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_تیاری(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

ساکن ساخت usb_pipe *
uhci_alloc_intr_pipe(ساخت usbdevice_s *usbdev
                     , ساخت usb_endpoint_descriptor *epdesc)
{
    ساخت usb_uhci_s *cntl = container_کا(
        usbdev->hub->cntl, ساخت usb_uhci_s, usb);
    صحیح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    اگر (frameexp > 10)
        frameexp = 10;
    صحیح maxpacket = epdesc->w_Max_Packet_سائز;
    // Determine number of entries needed for 2 timer ticks.
    صحیح ms = 1<<frameexp;
    صحیح گنتی = DIV_باری_اوپر(ticks_مقصد_ms(2), ms);
    گنتی = ALIGN(گنتی, 2);
    ساخت uhci_pipe *pipe = malloc_کم(حجم(*pipe));
    ساخت uhci_td *tds = malloc_کم(حجم(*tds) * گنتی);
    خالی *ڈیٹا = malloc_کم(maxpacket * گنتی);
    اگر (!pipe || !tds || !ڈیٹا) {
        warn_noalloc();
        جاؤ fail;
    }
    memset(pipe, 0, حجم(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    صحیح lowspeed = pipe->pipe.رفتار;
    صحیح devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->اگلا_td = &tds[0];
    pipe->iobase = cntl->iobase;

    صحیح toggle = 0;
    صحیح i;
    برائے (i=0; i<گنتی; i++) {
        tds[i].link = (i==گنتی-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].حالت_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_فعال);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_سرکاؤ)
                        | USB_PID_اندر);
        tds[i].بفر = ڈیٹا + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    ساخت uhci_framelist *fl = cntl->framelist;
    اگر (frameexp == 0) {
        // Add to existing interrupt entry.
        ساخت uhci_qh *intr_qh = (خالی*)(fl->links[0] & ~UHCI_PTR_بٹ);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        اگر (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } ورنہ {
        صحیح startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        برائے (i=startpos; i<ARRAY_سائز(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    واپس &pipe->pipe;
fail:
    آزاد(pipe);
    آزاد(tds);
    آزاد(ڈیٹا);
    واپس تہی;
}

ساخت usb_pipe *
uhci_realloc_pipe(ساخت usbdevice_s *usbdev, ساخت usb_pipe *upipe
                  , ساخت usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_UHCI)
        واپس تہی;
    usb_جوڑو_freelist(upipe);
    اگر (!epdesc)
        واپس تہی;
    u8 eptype = epdesc->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب;
    اگر (eptype == USB_ENDPOINT_XFER_INT)
        واپس uhci_alloc_intr_pipe(usbdev, epdesc);
    ساخت usb_uhci_s *cntl = container_کا(
        usbdev->hub->cntl, ساخت usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    ساخت usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    اگر (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        واپس usbpipe;
    }

    // Allocate a new queue head.
    ساخت uhci_pipe *pipe;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(حجم(*pipe));
    ورنہ
        pipe = malloc_کم(حجم(*pipe));
    اگر (!pipe) {
        warn_noalloc();
        واپس تہی;
    }
    memset(pipe, 0, حجم(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_رکن;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    ساخت uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    واپس &pipe->pipe;
}

ساکن صحیح
wait_pipe(ساخت uhci_pipe *pipe, u32 انتہا)
{
    برائے (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        اگر (el_link & UHCI_PTR_رکن)
            واپس 0;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            ساخت uhci_td *td = (خالی*)(el_link & ~UHCI_PTR_بٹ);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (خالی*)el_link, GET_LOWFLAT(td->حالت_2)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            سیٹ_LOWFLAT(pipe->qh.element, UHCI_PTR_رکن);
            uhci_waittick(iobase);
            واپس -1;
        }
        yield();
    }
}

ساکن صحیح
wait_td(ساخت uhci_td *td, u32 انتہا)
{
    u32 حالت_2;
    برائے (;;) {
        حالت_2 = td->حالت_2;
        اگر (!(حالت_2 & TD_CTRL_فعال))
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس -1;
        }
        yield();
    }
    اگر (حالت_2 & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", حالت_2);
        واپس -2;
    }
    واپس 0;
}

#تعریف_کرو STACKTDS 16
#تعریف_کرو TDALIGN 16

صحیح
uhci_send_pipe(ساخت usb_pipe *p, صحیح dir, ثابت خالی *cmd
               , خالی *ڈیٹا, صحیح datasize)
{
    اگر (! CONFIG_USB_UHCI)
        واپس -1;
    ساخت uhci_pipe *pipe = container_کا(p, ساخت uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, ڈیٹا, datasize);
    صحیح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    صحیح lowspeed = GET_LOWFLAT(pipe->pipe.رفتار);
    صحیح devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    صحیح toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[حجم(ساخت uhci_td) * STACKTDS + TDALIGN - 1];
    ساخت uhci_td *tds = (خالی*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, حجم(*tds) * STACKTDS);
    صحیح tdpos = 0;

    // Enable tds
    u32 انتہا = timer_calc(usb_xfer_وقت(p, datasize));
    barrier();
    سیٹ_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    اگر (cmd) {
        // Send setup pid on control transfers
        ساخت uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_گہرائی;
        td->token = (uhci_explen(USB_CONTROL_تیاری_سائز)
                     | (devaddr << TD_TOKEN_DEVADDR_سرکاؤ) | USB_PID_تیاری);
        td->بفر = (خالی*)cmd;
        barrier();
        td->حالت_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_فعال);
        toggle = TD_TOKEN_TOGGLE;
    }
    جب (datasize) {
        // Send data pids
        ساخت uhci_td *td = &tds[tdpos++ % STACKTDS];
        صحیح ret = wait_td(td, انتہا);
        اگر (ret)
            جاؤ fail;

        صحیح transfer = datasize;
        اگر (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_رکن : (nexttd | UHCI_PTR_گہرائی));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_سرکاؤ)
                     | (dir ? USB_PID_اندر : USB_PID_برآمد));
        td->بفر = ڈیٹا;
        barrier();
        td->حالت_2 = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_فعال);
        toggle ^= TD_TOKEN_TOGGLE;

        ڈیٹا += transfer;
        datasize -= transfer;
    }
    اگر (cmd) {
        // Send status pid on control transfers
        ساخت uhci_td *td = &tds[tdpos++ % STACKTDS];
        صحیح ret = wait_td(td, انتہا);
        اگر (ret)
            جاؤ fail;
        td->link = UHCI_PTR_رکن;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_سرکاؤ)
                     | (dir ? USB_PID_برآمد : USB_PID_اندر));
        td->بفر = 0;
        barrier();
        td->حالت_2 = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_فعال);
    }
    سیٹ_LOWFLAT(pipe->toggle, !!toggle);
    واپس wait_pipe(pipe, انتہا);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    سیٹ_LOWFLAT(pipe->qh.element, UHCI_PTR_رکن);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    واپس -1;
}

صحیح
uhci_poll_intr(ساخت usb_pipe *p, خالی *ڈیٹا)
{
    ASSERT16();
    اگر (! CONFIG_USB_UHCI)
        واپس -1;

    ساخت uhci_pipe *pipe = container_کا(p, ساخت uhci_pipe, pipe);
    ساخت uhci_td *td = GET_LOWFLAT(pipe->اگلا_td);
    u32 حالت_2 = GET_LOWFLAT(td->حالت_2);
    u32 token = GET_LOWFLAT(td->token);
    اگر (حالت_2 & TD_CTRL_فعال)
        // No intrs found.
        واپس -1;
    // XXX - check for errors.

    // Copy data.
    خالی *tddata = GET_LOWFLAT(td->بفر);
    memcpy_far(GET_SEG(SS), ڈیٹا, SEG_کم, LOWFLAT2_کم(tddata)
               , uhci_expected_لمبائی(token));

    // Reenable this td.
    ساخت uhci_td *اگلا = (خالی*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_بٹ);
    سیٹ_LOWFLAT(pipe->اگلا_td, اگلا);
    barrier();
    سیٹ_LOWFLAT(td->حالت_2, (uhci_maxerr(0) | (حالت_2 & TD_CTRL_LS)
                             | TD_CTRL_فعال));

    واپس 0;
}
