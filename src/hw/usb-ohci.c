// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_LOWFLAT
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "memmap.h" // PAGE_SIZE
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#شامل "pci_regs.h" // PCI_BASE_ADDRESS_0
#شامل "string.h" // memset
#شامل "usb.h" // struct usb_s
#شامل "usb-ehci.h" // ehci_wait_controllers
#شامل "usb-ohci.h" // struct ohci_hcca
#شامل "util.h" // msleep
#شامل "x86.h" // readl

#define FIT                     (1 << 31)

ساختار usb_ohci_s {
    ساختار usb_s usb;
    ساختار ohci_regs *regs;
};

ساختار ohci_pipe {
    ساختار ohci_ed ed;
    ساختار usb_pipe pipe;
    ساختار ohci_regs *regs;
    تهی *data;
    صحیح شمار;
    ساختار ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
ایستا صحیح
ohci_hub_detect(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_ohci_s *cntl = container_of(hub->cntl, ساختار usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    بازگشت (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
ایستا تهی
ohci_hub_disconnect(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_ohci_s *cntl = container_of(hub->cntl, ساختار usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
ایستا صحیح
ohci_hub_reset(ساختار usbhub_s *hub, u32 port)
{
    ساختار usb_ohci_s *cntl = container_of(hub->cntl, ساختار usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    برای (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        اگر (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            بشکن;
        اگر (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            بازگشت -1;
        }
        yield();
    }

    اگر ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        بازگشت -1;

    بازگشت !!(sts & RH_PS_LSDA);
}

ایستا ساختار usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
ایستا صحیح
check_ohci_ports(ساختار usb_ohci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();
    // Turn on power for all devices on roothub.
    u32 rha = readl(&cntl->regs->roothub_a);
    rha &= ~(RH_A_PSM | RH_A_OCPM);
    writel(&cntl->regs->roothub_status, RH_HS_LPSC);
    writel(&cntl->regs->roothub_b, RH_B_PPCM);
    msleep((rha >> 24) * 2);
    // XXX - need to sleep for USB_TIME_SIGATT if just powered up?

    ساختار usbhub_s hub;
    memset(&hub, 0, اندازه(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    بازگشت hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ایستا تهی
ohci_waittick(ساختار ohci_regs *regs)
{
    barrier();
    ساختار ohci_hcca *hcca = (تهی*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    برای (;;) {
        اگر (hcca->frame_no != startframe)
            بشکن;
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت;
        }
        yield();
    }
}

ایستا تهی
ohci_free_pipes(ساختار usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    اگر (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    برای (;;) {
        ساختار ohci_ed *next = (تهی*)*pos;
        اگر (!next)
            بشکن;
        ساختار ohci_pipe *pipe = container_of(next, ساختار ohci_pipe, ed);
        اگر (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            آزاد(pipe);
        } وگرنه {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = پوچ;
}

ایستا صحیح
start_ohci(ساختار usb_ohci_s *cntl, ساختار ohci_hcca *hcca)
{
    u32 oldfminterval = readl(&cntl->regs->fminterval);
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;

    // XXX - check if already running?

    // Do reset
    writel(&cntl->regs->control, OHCI_USB_RESET | oldrwc);
    readl(&cntl->regs->control); // flush writes
    msleep(USB_TIME_DRSTR);

    // Do software init (min 10us, max 2ms)
    u32 end = timer_calc_usec(10);
    writel(&cntl->regs->cmdstatus, OHCI_HCR);
    برای (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        اگر (!(status & OHCI_HCR))
            بشکن;
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت -1;
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

    بازگشت 0;
}

ایستا تهی
stop_ohci(ساختار usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

ایستا تهی
configure_ohci(تهی *data)
{
    ساختار usb_ohci_s *cntl = data;

    // Allocate memory
    ساختار ohci_hcca *hcca = memalign_بالا(256, اندازه(*hcca));
    ساختار ohci_ed *intr_ed = malloc_بالا(اندازه(*intr_ed));
    اگر (!hcca || !intr_ed) {
        warn_noalloc();
        برو آزاد;
    }
    memset(hcca, 0, اندازه(*hcca));
    memset(intr_ed, 0, اندازه(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    صحیح i;
    برای (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    صحیح ret = start_ohci(cntl, hcca);
    اگر (ret)
        برو err;

    صحیح شمار = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    اگر (! شمار)
        برو err;
    بازگشت;

err:
    stop_ohci(cntl);
آزاد:
    آزاد(hcca);
    آزاد(intr_ed);
}

ایستا تهی
ohci_controller_setup(ساختار pci_device *pci)
{
    ساختار ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    اگر (!regs)
        بازگشت;

    ساختار usb_ohci_s *cntl = malloc_tmphigh(اندازه(*cntl));
    اگر (!cntl) {
        warn_noalloc();
        بازگشت;
    }
    memset(cntl, 0, اندازه(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_OHCI;
    cntl->regs = regs;

    dprintf(1, "OHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    // Disable interrupts
    writel(&cntl->regs->intrdisable, ~0);
    writel(&cntl->regs->intrstatus, ~0);

    run_thread(configure_ohci, cntl);
}

تهی
ohci_setup(تهی)
{
    اگر (! CONFIG_USB_OHCI)
        بازگشت;
    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
ایستا تهی
ohci_desc2pipe(ساختار ohci_pipe *pipe, ساختار usbdevice_s *usbdev
               , ساختار usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    ساختار usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ساختار usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

ایستا ساختار usb_pipe *
ohci_alloc_intr_pipe(ساختار usbdevice_s *usbdev
                     , ساختار usb_endpoint_descriptor *epdesc)
{
    ساختار usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ساختار usb_ohci_s, usb);
    صحیح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    اگر (frameexp > 5)
        frameexp = 5;
    صحیح maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    صحیح ms = 1<<frameexp;
    صحیح شمار = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    ساختار ohci_pipe *pipe = malloc_کم(اندازه(*pipe));
    ساختار ohci_td *tds = malloc_کم(اندازه(*tds) * شمار);
    تهی *data = malloc_کم(maxpacket * شمار);
    اگر (!pipe || !tds || !data)
        برو err;
    memset(pipe, 0, اندازه(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->شمار = شمار;
    pipe->tds = tds;

    ساختار ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[شمار-1];

    صحیح i;
    برای (i=0; i<شمار-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    ساختار ohci_hcca *hcca = (تهی*)cntl->regs->hcca;
    اگر (frameexp == 0) {
        // Add to existing interrupt entry.
        ساختار ohci_ed *intr_ed = (تهی*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } وگرنه {
        صحیح startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        برای (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    بازگشت &pipe->pipe;

err:
    آزاد(pipe);
    آزاد(tds);
    آزاد(data);
    بازگشت پوچ;
}

ساختار usb_pipe *
ohci_realloc_pipe(ساختار usbdevice_s *usbdev, ساختار usb_pipe *upipe
                  , ساختار usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_OHCI)
        بازگشت پوچ;
    usb_add_freelist(upipe);
    اگر (!epdesc)
        بازگشت پوچ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    اگر (eptype == USB_ENDPOINT_XFER_INT)
        بازگشت ohci_alloc_intr_pipe(usbdev, epdesc);
    ساختار usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ساختار usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    ساختار usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    اگر (usbpipe) {
        // Use previously allocated pipe.
        ساختار ohci_pipe *pipe = container_of(usbpipe, ساختار ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        بازگشت usbpipe;
    }

    // Allocate a new queue head.
    ساختار ohci_pipe *pipe;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(اندازه(*pipe));
    وگرنه
        pipe = malloc_کم(اندازه(*pipe));
    اگر (!pipe) {
        warn_noalloc();
        بازگشت پوچ;
    }
    memset(pipe, 0, اندازه(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    اگر (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    بازگشت &pipe->pipe;
}

ایستا صحیح
wait_ed(ساختار ohci_ed *ed, صحیح timeout)
{
    u32 end = timer_calc(timeout);
    برای (;;) {
        اگر ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            بازگشت 0;
        اگر (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            بازگشت -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

صحیح
ohci_send_pipe(ساختار usb_pipe *p, صحیح dir, ثابت تهی *cmd
               , تهی *data, صحیح datasize)
{
    ASSERT32FLAT();
    اگر (! CONFIG_USB_OHCI)
        بازگشت -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    ساختار ohci_pipe *pipe = container_of(p, ساختار ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[اندازه(ساختار ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    ساختار ohci_td *tds = (تهی*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, اندازه(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    اگر (cmd) {
        // Send setup pid on control transfers
        td->hwINFO = TD_DP_SETUP | TD_T_DATA0 | TD_CC;
        td->hwCBP = (u32)cmd;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = (u32)cmd + USB_CONTROL_SETUP_SIZE - 1;
        td++;
        toggle = TD_T_DATA1;
        statuscmd = OHCI_CLF;
    }
    u32 dest = (u32)data, dataend = dest + datasize;
    وقتی (dest < dataend) {
        // Send data pids
        اگر (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            بازگشت -1;
        }
        صحیح maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        صحیح transfer = dataend - dest;
        اگر (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    اگر (cmd) {
        // Send status pid on control transfers
        اگر (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            بازگشت -1;
        }
        td->hwINFO = (dir ? TD_DP_OUT : TD_DP_IN) | TD_T_DATA1 | TD_CC;
        td->hwCBP = 0;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = 0;
        td++;
    }

    // Transfer data
    pipe->ed.hwHeadP = (u32)tds | (pipe->ed.hwHeadP & ED_C);
    pipe->ed.hwTailP = (u32)td;
    barrier();
    pipe->ed.hwINFO &= ~ED_SKIP;
    writel(&pipe->regs->cmdstatus, statuscmd);

    صحیح ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    اگر (ret)
        ohci_waittick(pipe->regs);
    بازگشت ret;
}

صحیح
ohci_poll_intr(ساختار usb_pipe *p, تهی *data)
{
    ASSERT16();
    اگر (! CONFIG_USB_OHCI)
        بازگشت -1;

    ساختار ohci_pipe *pipe = container_of(p, ساختار ohci_pipe, pipe);
    ساختار ohci_td *tds = GET_LOWFLAT(pipe->tds);
    ساختار ohci_td *head = (تهی*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    ساختار ohci_td *tail = (تهی*)GET_LOWFLAT(pipe->ed.hwTailP);
    صحیح شمار = GET_LOWFLAT(pipe->شمار);
    صحیح pos = (tail - tds + 1) % شمار;
    ساختار ohci_td *next = &tds[pos];
    اگر (head == next)
        // No intrs found.
        بازگشت -1;
    // XXX - check for errors.

    // Copy data.
    صحیح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    تهی *pipedata = GET_LOWFLAT((pipe->data));
    تهی *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_کم, LOWFLAT2_کم(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    بازگشت 0;
}
