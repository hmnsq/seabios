// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_LOWFLAT
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "memmap.h" // PAGE_SIZE
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#അന്തര്ഭാവ "pci_regs.h" // PCI_BASE_ADDRESS_0
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "usb.h" // struct usb_s
#അന്തര്ഭാവ "usb-ehci.h" // ehci_wait_controllers
#അന്തര്ഭാവ "usb-ohci.h" // struct ohci_hcca
#അന്തര്ഭാവ "util.h" // msleep
#അന്തര്ഭാവ "x86.h" // readl

#define FIT                     (1 << 31)

ഘടന usb_ohci_s {
    ഘടന usb_s usb;
    ഘടന ohci_regs *regs;
};

ഘടന ohci_pipe {
    ഘടന ohci_ed ed;
    ഘടന usb_pipe pipe;
    ഘടന ohci_regs *regs;
    ശൂന്യം *data;
    പൂർണ്ണസംഖ്യ എണ്ണം;
    ഘടന ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
സ്ഥിതം പൂർണ്ണസംഖ്യ
ohci_hub_detect(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_ohci_s *cntl = container_of(hub->cntl, ഘടന usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    തിരികെ (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
സ്ഥിതം ശൂന്യം
ohci_hub_disconnect(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_ohci_s *cntl = container_of(hub->cntl, ഘടന usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
സ്ഥിതം പൂർണ്ണസംഖ്യ
ohci_hub_reset(ഘടന usbhub_s *hub, u32 port)
{
    ഘടന usb_ohci_s *cntl = container_of(hub->cntl, ഘടന usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    വേണ്ടി (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        എങ്കിൽ (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            തിരികെ -1;
        }
        yield();
    }

    എങ്കിൽ ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        തിരികെ -1;

    തിരികെ !!(sts & RH_PS_LSDA);
}

സ്ഥിതം ഘടന usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
സ്ഥിതം പൂർണ്ണസംഖ്യ
check_ohci_ports(ഘടന usb_ohci_s *cntl)
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

    ഘടന usbhub_s hub;
    memset(&hub, 0, വലുപ്പം(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    തിരികെ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
സ്ഥിതം ശൂന്യം
ohci_waittick(ഘടന ohci_regs *regs)
{
    barrier();
    ഘടന ohci_hcca *hcca = (ശൂന്യം*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    വേണ്ടി (;;) {
        എങ്കിൽ (hcca->frame_no != startframe)
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ;
        }
        yield();
    }
}

സ്ഥിതം ശൂന്യം
ohci_free_pipes(ഘടന usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    എങ്കിൽ (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    വേണ്ടി (;;) {
        ഘടന ohci_ed *next = (ശൂന്യം*)*pos;
        എങ്കിൽ (!next)
            നിർത്തുക;
        ഘടന ohci_pipe *pipe = container_of(next, ഘടന ohci_pipe, ed);
        എങ്കിൽ (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            സ്വതന്ത്രം(pipe);
        } അല്ലെങ്കിൽ {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = ശൂന്യ;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
start_ohci(ഘടന usb_ohci_s *cntl, ഘടന ohci_hcca *hcca)
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
    വേണ്ടി (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        എങ്കിൽ (!(status & OHCI_HCR))
            നിർത്തുക;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ -1;
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

    തിരികെ 0;
}

സ്ഥിതം ശൂന്യം
stop_ohci(ഘടന usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

സ്ഥിതം ശൂന്യം
configure_ohci(ശൂന്യം *data)
{
    ഘടന usb_ohci_s *cntl = data;

    // Allocate memory
    ഘടന ohci_hcca *hcca = memalign_മുകളിലെ(256, വലുപ്പം(*hcca));
    ഘടന ohci_ed *intr_ed = malloc_മുകളിലെ(വലുപ്പം(*intr_ed));
    എങ്കിൽ (!hcca || !intr_ed) {
        warn_noalloc();
        പോകുക സ്വതന്ത്രം;
    }
    memset(hcca, 0, വലുപ്പം(*hcca));
    memset(intr_ed, 0, വലുപ്പം(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    പൂർണ്ണസംഖ്യ ret = start_ohci(cntl, hcca);
    എങ്കിൽ (ret)
        പോകുക err;

    പൂർണ്ണസംഖ്യ എണ്ണം = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    എങ്കിൽ (! എണ്ണം)
        പോകുക err;
    തിരികെ;

err:
    stop_ohci(cntl);
സ്വതന്ത്രം:
    സ്വതന്ത്രം(hcca);
    സ്വതന്ത്രം(intr_ed);
}

സ്ഥിതം ശൂന്യം
ohci_controller_setup(ഘടന pci_device *pci)
{
    ഘടന ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    എങ്കിൽ (!regs)
        തിരികെ;

    ഘടന usb_ohci_s *cntl = malloc_tmphigh(വലുപ്പം(*cntl));
    എങ്കിൽ (!cntl) {
        warn_noalloc();
        തിരികെ;
    }
    memset(cntl, 0, വലുപ്പം(*cntl));
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

ശൂന്യം
ohci_setup(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_USB_OHCI)
        തിരികെ;
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
സ്ഥിതം ശൂന്യം
ohci_desc2pipe(ഘടന ohci_pipe *pipe, ഘടന usbdevice_s *usbdev
               , ഘടന usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    ഘടന usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ഘടന usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

സ്ഥിതം ഘടന usb_pipe *
ohci_alloc_intr_pipe(ഘടന usbdevice_s *usbdev
                     , ഘടന usb_endpoint_descriptor *epdesc)
{
    ഘടന usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ഘടന usb_ohci_s, usb);
    പൂർണ്ണസംഖ്യ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    എങ്കിൽ (frameexp > 5)
        frameexp = 5;
    പൂർണ്ണസംഖ്യ maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    പൂർണ്ണസംഖ്യ ms = 1<<frameexp;
    പൂർണ്ണസംഖ്യ എണ്ണം = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    ഘടന ohci_pipe *pipe = malloc_താഴെയുള്ള(വലുപ്പം(*pipe));
    ഘടന ohci_td *tds = malloc_താഴെയുള്ള(വലുപ്പം(*tds) * എണ്ണം);
    ശൂന്യം *data = malloc_താഴെയുള്ള(maxpacket * എണ്ണം);
    എങ്കിൽ (!pipe || !tds || !data)
        പോകുക err;
    memset(pipe, 0, വലുപ്പം(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->എണ്ണം = എണ്ണം;
    pipe->tds = tds;

    ഘടന ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[എണ്ണം-1];

    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<എണ്ണം-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    ഘടന ohci_hcca *hcca = (ശൂന്യം*)cntl->regs->hcca;
    എങ്കിൽ (frameexp == 0) {
        // Add to existing interrupt entry.
        ഘടന ohci_ed *intr_ed = (ശൂന്യം*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } അല്ലെങ്കിൽ {
        പൂർണ്ണസംഖ്യ startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        വേണ്ടി (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    തിരികെ &pipe->pipe;

err:
    സ്വതന്ത്രം(pipe);
    സ്വതന്ത്രം(tds);
    സ്വതന്ത്രം(data);
    തിരികെ ശൂന്യ;
}

ഘടന usb_pipe *
ohci_realloc_pipe(ഘടന usbdevice_s *usbdev, ഘടന usb_pipe *upipe
                  , ഘടന usb_endpoint_descriptor *epdesc)
{
    എങ്കിൽ (! CONFIG_USB_OHCI)
        തിരികെ ശൂന്യ;
    usb_add_freelist(upipe);
    എങ്കിൽ (!epdesc)
        തിരികെ ശൂന്യ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_INT)
        തിരികെ ohci_alloc_intr_pipe(usbdev, epdesc);
    ഘടന usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ഘടന usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    ഘടന usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    എങ്കിൽ (usbpipe) {
        // Use previously allocated pipe.
        ഘടന ohci_pipe *pipe = container_of(usbpipe, ഘടന ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        തിരികെ usbpipe;
    }

    // Allocate a new queue head.
    ഘടന ohci_pipe *pipe;
    എങ്കിൽ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(വലുപ്പം(*pipe));
    അല്ലെങ്കിൽ
        pipe = malloc_താഴെയുള്ള(വലുപ്പം(*pipe));
    എങ്കിൽ (!pipe) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    memset(pipe, 0, വലുപ്പം(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    എങ്കിൽ (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    തിരികെ &pipe->pipe;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
wait_ed(ഘടന ohci_ed *ed, പൂർണ്ണസംഖ്യ timeout)
{
    u32 end = timer_calc(timeout);
    വേണ്ടി (;;) {
        എങ്കിൽ ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            തിരികെ 0;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            തിരികെ -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

പൂർണ്ണസംഖ്യ
ohci_send_pipe(ഘടന usb_pipe *p, പൂർണ്ണസംഖ്യ dir, സ്ഥിരം ശൂന്യം *cmd
               , ശൂന്യം *data, പൂർണ്ണസംഖ്യ datasize)
{
    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_USB_OHCI)
        തിരികെ -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    ഘടന ohci_pipe *pipe = container_of(p, ഘടന ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[വലുപ്പം(ഘടന ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    ഘടന ohci_td *tds = (ശൂന്യം*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, വലുപ്പം(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    എങ്കിൽ (cmd) {
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
    ആകുമ്പോൾ (dest < dataend) {
        // Send data pids
        എങ്കിൽ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            തിരികെ -1;
        }
        പൂർണ്ണസംഖ്യ maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        പൂർണ്ണസംഖ്യ transfer = dataend - dest;
        എങ്കിൽ (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    എങ്കിൽ (cmd) {
        // Send status pid on control transfers
        എങ്കിൽ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            തിരികെ -1;
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

    പൂർണ്ണസംഖ്യ ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    എങ്കിൽ (ret)
        ohci_waittick(pipe->regs);
    തിരികെ ret;
}

പൂർണ്ണസംഖ്യ
ohci_poll_intr(ഘടന usb_pipe *p, ശൂന്യം *data)
{
    ASSERT16();
    എങ്കിൽ (! CONFIG_USB_OHCI)
        തിരികെ -1;

    ഘടന ohci_pipe *pipe = container_of(p, ഘടന ohci_pipe, pipe);
    ഘടന ohci_td *tds = GET_LOWFLAT(pipe->tds);
    ഘടന ohci_td *head = (ശൂന്യം*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    ഘടന ohci_td *tail = (ശൂന്യം*)GET_LOWFLAT(pipe->ed.hwTailP);
    പൂർണ്ണസംഖ്യ എണ്ണം = GET_LOWFLAT(pipe->എണ്ണം);
    പൂർണ്ണസംഖ്യ pos = (tail - tds + 1) % എണ്ണം;
    ഘടന ohci_td *next = &tds[pos];
    എങ്കിൽ (head == next)
        // No intrs found.
        തിരികെ -1;
    // XXX - check for errors.

    // Copy data.
    പൂർണ്ണസംഖ്യ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ശൂന്യം *pipedata = GET_LOWFLAT((pipe->data));
    ശൂന്യം *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_താഴെയുള്ള, LOWFLAT2_താഴെയുള്ള(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    തിരികെ 0;
}
