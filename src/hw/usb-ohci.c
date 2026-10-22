// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_LOWFLAT
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "memmap.h" // PAGE_SIZE
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#অন্তর্ভাব "pci_regs.h" // PCI_BASE_ADDRESS_0
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "usb.h" // struct usb_s
#অন্তর্ভাব "usb-ehci.h" // ehci_wait_controllers
#অন্তর্ভাব "usb-ohci.h" // struct ohci_hcca
#অন্তর্ভাব "util.h" // msleep
#অন্তর্ভাব "x86.h" // readl

#define FIT                     (1 << 31)

সংরচনা usb_ohci_s {
    সংরচনা usb_s usb;
    সংরচনা ohci_regs *regs;
};

সংরচনা ohci_pipe {
    সংরচনা ohci_ed ed;
    সংরচনা usb_pipe pipe;
    সংরচনা ohci_regs *regs;
    শূন্য *data;
    পূর্ণাঙ্ক গুনতি;
    সংরচনা ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
স্থির পূর্ণাঙ্ক
ohci_hub_detect(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_ohci_s *cntl = container_of(hub->cntl, সংরচনা usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    প্রত্যাগম (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
স্থির শূন্য
ohci_hub_disconnect(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_ohci_s *cntl = container_of(hub->cntl, সংরচনা usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
স্থির পূর্ণাঙ্ক
ohci_hub_reset(সংরচনা usbhub_s *hub, u32 port)
{
    সংরচনা usb_ohci_s *cntl = container_of(hub->cntl, সংরচনা usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    প্রতি (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        যদি (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            ভঙ্গ;
        যদি (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            প্রত্যাগম -1;
        }
        yield();
    }

    যদি ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        প্রত্যাগম -1;

    প্রত্যাগম !!(sts & RH_PS_LSDA);
}

স্থির সংরচনা usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
স্থির পূর্ণাঙ্ক
check_ohci_ports(সংরচনা usb_ohci_s *cntl)
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

    সংরচনা usbhub_s hub;
    memset(&hub, 0, পরিমাণ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    প্রত্যাগম hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
স্থির শূন্য
ohci_waittick(সংরচনা ohci_regs *regs)
{
    barrier();
    সংরচনা ohci_hcca *hcca = (শূন্য*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    প্রতি (;;) {
        যদি (hcca->frame_no != startframe)
            ভঙ্গ;
        যদি (timer_check(end)) {
            warn_timeout();
            প্রত্যাগম;
        }
        yield();
    }
}

স্থির শূন্য
ohci_free_pipes(সংরচনা usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    যদি (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    প্রতি (;;) {
        সংরচনা ohci_ed *next = (শূন্য*)*pos;
        যদি (!next)
            ভঙ্গ;
        সংরচনা ohci_pipe *pipe = container_of(next, সংরচনা ohci_pipe, ed);
        যদি (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            মুক্ত(pipe);
        } অন্যথা {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = রিক্ত;
}

স্থির পূর্ণাঙ্ক
start_ohci(সংরচনা usb_ohci_s *cntl, সংরচনা ohci_hcca *hcca)
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
    প্রতি (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        যদি (!(status & OHCI_HCR))
            ভঙ্গ;
        যদি (timer_check(end)) {
            warn_timeout();
            প্রত্যাগম -1;
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

    প্রত্যাগম 0;
}

স্থির শূন্য
stop_ohci(সংরচনা usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

স্থির শূন্য
configure_ohci(শূন্য *data)
{
    সংরচনা usb_ohci_s *cntl = data;

    // Allocate memory
    সংরচনা ohci_hcca *hcca = memalign_উপরের(256, পরিমাণ(*hcca));
    সংরচনা ohci_ed *intr_ed = malloc_উপরের(পরিমাণ(*intr_ed));
    যদি (!hcca || !intr_ed) {
        warn_noalloc();
        গচ্ছ মুক্ত;
    }
    memset(hcca, 0, পরিমাণ(*hcca));
    memset(intr_ed, 0, পরিমাণ(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    পূর্ণাঙ্ক i;
    প্রতি (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    পূর্ণাঙ্ক ret = start_ohci(cntl, hcca);
    যদি (ret)
        গচ্ছ err;

    পূর্ণাঙ্ক গুনতি = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    যদি (! গুনতি)
        গচ্ছ err;
    প্রত্যাগম;

err:
    stop_ohci(cntl);
মুক্ত:
    মুক্ত(hcca);
    মুক্ত(intr_ed);
}

স্থির শূন্য
ohci_controller_setup(সংরচনা pci_device *pci)
{
    সংরচনা ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!regs)
        প্রত্যাগম;

    সংরচনা usb_ohci_s *cntl = malloc_tmphigh(পরিমাণ(*cntl));
    যদি (!cntl) {
        warn_noalloc();
        প্রত্যাগম;
    }
    memset(cntl, 0, পরিমাণ(*cntl));
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

শূন্য
ohci_setup(শূন্য)
{
    যদি (! CONFIG_USB_OHCI)
        প্রত্যাগম;
    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
স্থির শূন্য
ohci_desc2pipe(সংরচনা ohci_pipe *pipe, সংরচনা usbdevice_s *usbdev
               , সংরচনা usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    সংরচনা usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, সংরচনা usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

স্থির সংরচনা usb_pipe *
ohci_alloc_intr_pipe(সংরচনা usbdevice_s *usbdev
                     , সংরচনা usb_endpoint_descriptor *epdesc)
{
    সংরচনা usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, সংরচনা usb_ohci_s, usb);
    পূর্ণাঙ্ক frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    যদি (frameexp > 5)
        frameexp = 5;
    পূর্ণাঙ্ক maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    পূর্ণাঙ্ক ms = 1<<frameexp;
    পূর্ণাঙ্ক গুনতি = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    সংরচনা ohci_pipe *pipe = malloc_নিচের(পরিমাণ(*pipe));
    সংরচনা ohci_td *tds = malloc_নিচের(পরিমাণ(*tds) * গুনতি);
    শূন্য *data = malloc_নিচের(maxpacket * গুনতি);
    যদি (!pipe || !tds || !data)
        গচ্ছ err;
    memset(pipe, 0, পরিমাণ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->গুনতি = গুনতি;
    pipe->tds = tds;

    সংরচনা ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[গুনতি-1];

    পূর্ণাঙ্ক i;
    প্রতি (i=0; i<গুনতি-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    সংরচনা ohci_hcca *hcca = (শূন্য*)cntl->regs->hcca;
    যদি (frameexp == 0) {
        // Add to existing interrupt entry.
        সংরচনা ohci_ed *intr_ed = (শূন্য*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } অন্যথা {
        পূর্ণাঙ্ক startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        প্রতি (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    প্রত্যাগম &pipe->pipe;

err:
    মুক্ত(pipe);
    মুক্ত(tds);
    মুক্ত(data);
    প্রত্যাগম রিক্ত;
}

সংরচনা usb_pipe *
ohci_realloc_pipe(সংরচনা usbdevice_s *usbdev, সংরচনা usb_pipe *upipe
                  , সংরচনা usb_endpoint_descriptor *epdesc)
{
    যদি (! CONFIG_USB_OHCI)
        প্রত্যাগম রিক্ত;
    usb_add_freelist(upipe);
    যদি (!epdesc)
        প্রত্যাগম রিক্ত;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    যদি (eptype == USB_ENDPOINT_XFER_INT)
        প্রত্যাগম ohci_alloc_intr_pipe(usbdev, epdesc);
    সংরচনা usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, সংরচনা usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    সংরচনা usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    যদি (usbpipe) {
        // Use previously allocated pipe.
        সংরচনা ohci_pipe *pipe = container_of(usbpipe, সংরচনা ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        প্রত্যাগম usbpipe;
    }

    // Allocate a new queue head.
    সংরচনা ohci_pipe *pipe;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(পরিমাণ(*pipe));
    অন্যথা
        pipe = malloc_নিচের(পরিমাণ(*pipe));
    যদি (!pipe) {
        warn_noalloc();
        প্রত্যাগম রিক্ত;
    }
    memset(pipe, 0, পরিমাণ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    যদি (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    প্রত্যাগম &pipe->pipe;
}

স্থির পূর্ণাঙ্ক
wait_ed(সংরচনা ohci_ed *ed, পূর্ণাঙ্ক timeout)
{
    u32 end = timer_calc(timeout);
    প্রতি (;;) {
        যদি ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            প্রত্যাগম 0;
        যদি (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            প্রত্যাগম -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

পূর্ণাঙ্ক
ohci_send_pipe(সংরচনা usb_pipe *p, পূর্ণাঙ্ক dir, ধ্রুব শূন্য *cmd
               , শূন্য *data, পূর্ণাঙ্ক datasize)
{
    ASSERT32FLAT();
    যদি (! CONFIG_USB_OHCI)
        প্রত্যাগম -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    সংরচনা ohci_pipe *pipe = container_of(p, সংরচনা ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[পরিমাণ(সংরচনা ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    সংরচনা ohci_td *tds = (শূন্য*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, পরিমাণ(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    যদি (cmd) {
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
    যাবত্ (dest < dataend) {
        // Send data pids
        যদি (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            প্রত্যাগম -1;
        }
        পূর্ণাঙ্ক maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        পূর্ণাঙ্ক transfer = dataend - dest;
        যদি (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    যদি (cmd) {
        // Send status pid on control transfers
        যদি (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            প্রত্যাগম -1;
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

    পূর্ণাঙ্ক ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    যদি (ret)
        ohci_waittick(pipe->regs);
    প্রত্যাগম ret;
}

পূর্ণাঙ্ক
ohci_poll_intr(সংরচনা usb_pipe *p, শূন্য *data)
{
    ASSERT16();
    যদি (! CONFIG_USB_OHCI)
        প্রত্যাগম -1;

    সংরচনা ohci_pipe *pipe = container_of(p, সংরচনা ohci_pipe, pipe);
    সংরচনা ohci_td *tds = GET_LOWFLAT(pipe->tds);
    সংরচনা ohci_td *head = (শূন্য*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    সংরচনা ohci_td *tail = (শূন্য*)GET_LOWFLAT(pipe->ed.hwTailP);
    পূর্ণাঙ্ক গুনতি = GET_LOWFLAT(pipe->গুনতি);
    পূর্ণাঙ্ক pos = (tail - tds + 1) % গুনতি;
    সংরচনা ohci_td *next = &tds[pos];
    যদি (head == next)
        // No intrs found.
        প্রত্যাগম -1;
    // XXX - check for errors.

    // Copy data.
    পূর্ণাঙ্ক maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    শূন্য *pipedata = GET_LOWFLAT((pipe->data));
    শূন্য *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_নিচের, LOWFLAT2_নিচের(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    প্রত্যাগম 0;
}
