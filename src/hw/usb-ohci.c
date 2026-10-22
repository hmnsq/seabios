// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_LOWFLAT
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "memmap.h" // PAGE_SIZE
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#અન્તર્ભાવ "pci_regs.h" // PCI_BASE_ADDRESS_0
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "usb.h" // struct usb_s
#અન્તર્ભાવ "usb-ehci.h" // ehci_wait_controllers
#અન્તર્ભાવ "usb-ohci.h" // struct ohci_hcca
#અન્તર્ભાવ "util.h" // msleep
#અન્તર્ભાવ "x86.h" // readl

#define FIT                     (1 << 31)

સંરચના usb_ohci_s {
    સંરચના usb_s usb;
    સંરચના ohci_regs *regs;
};

સંરચના ohci_pipe {
    સંરચના ohci_ed ed;
    સંરચના usb_pipe pipe;
    સંરચના ohci_regs *regs;
    શૂન્ય *data;
    પૂર્ણાંક ગણતરી;
    સંરચના ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
સ્થિર પૂર્ણાંક
ohci_hub_detect(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_ohci_s *cntl = container_of(hub->cntl, સંરચના usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    પાછા (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
સ્થિર શૂન્ય
ohci_hub_disconnect(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_ohci_s *cntl = container_of(hub->cntl, સંરચના usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
સ્થિર પૂર્ણાંક
ohci_hub_reset(સંરચના usbhub_s *hub, u32 port)
{
    સંરચના usb_ohci_s *cntl = container_of(hub->cntl, સંરચના usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    માટે (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        જો (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            તોડો;
        જો (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            પાછા -1;
        }
        yield();
    }

    જો ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        પાછા -1;

    પાછા !!(sts & RH_PS_LSDA);
}

સ્થિર સંરચના usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
સ્થિર પૂર્ણાંક
check_ohci_ports(સંરચના usb_ohci_s *cntl)
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

    સંરચના usbhub_s hub;
    memset(&hub, 0, કદ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    પાછા hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
સ્થિર શૂન્ય
ohci_waittick(સંરચના ohci_regs *regs)
{
    barrier();
    સંરચના ohci_hcca *hcca = (શૂન્ય*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    માટે (;;) {
        જો (hcca->frame_no != startframe)
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            પાછા;
        }
        yield();
    }
}

સ્થિર શૂન્ય
ohci_free_pipes(સંરચના usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    જો (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    માટે (;;) {
        સંરચના ohci_ed *next = (શૂન્ય*)*pos;
        જો (!next)
            તોડો;
        સંરચના ohci_pipe *pipe = container_of(next, સંરચના ohci_pipe, ed);
        જો (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            મુક્ત(pipe);
        } નહીંતર {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = ખાલી;
}

સ્થિર પૂર્ણાંક
start_ohci(સંરચના usb_ohci_s *cntl, સંરચના ohci_hcca *hcca)
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
    માટે (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        જો (!(status & OHCI_HCR))
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            પાછા -1;
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

    પાછા 0;
}

સ્થિર શૂન્ય
stop_ohci(સંરચના usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

સ્થિર શૂન્ય
configure_ohci(શૂન્ય *data)
{
    સંરચના usb_ohci_s *cntl = data;

    // Allocate memory
    સંરચના ohci_hcca *hcca = memalign_ઉપલું(256, કદ(*hcca));
    સંરચના ohci_ed *intr_ed = malloc_ઉપલું(કદ(*intr_ed));
    જો (!hcca || !intr_ed) {
        warn_noalloc();
        જાઓ મુક્ત;
    }
    memset(hcca, 0, કદ(*hcca));
    memset(intr_ed, 0, કદ(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    પૂર્ણાંક i;
    માટે (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    પૂર્ણાંક ret = start_ohci(cntl, hcca);
    જો (ret)
        જાઓ err;

    પૂર્ણાંક ગણતરી = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    જો (! ગણતરી)
        જાઓ err;
    પાછા;

err:
    stop_ohci(cntl);
મુક્ત:
    મુક્ત(hcca);
    મુક્ત(intr_ed);
}

સ્થિર શૂન્ય
ohci_controller_setup(સંરચના pci_device *pci)
{
    સંરચના ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    જો (!regs)
        પાછા;

    સંરચના usb_ohci_s *cntl = malloc_tmphigh(કદ(*cntl));
    જો (!cntl) {
        warn_noalloc();
        પાછા;
    }
    memset(cntl, 0, કદ(*cntl));
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

શૂન્ય
ohci_setup(શૂન્ય)
{
    જો (! CONFIG_USB_OHCI)
        પાછા;
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
સ્થિર શૂન્ય
ohci_desc2pipe(સંરચના ohci_pipe *pipe, સંરચના usbdevice_s *usbdev
               , સંરચના usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    સંરચના usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, સંરચના usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

સ્થિર સંરચના usb_pipe *
ohci_alloc_intr_pipe(સંરચના usbdevice_s *usbdev
                     , સંરચના usb_endpoint_descriptor *epdesc)
{
    સંરચના usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, સંરચના usb_ohci_s, usb);
    પૂર્ણાંક frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    જો (frameexp > 5)
        frameexp = 5;
    પૂર્ણાંક maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    પૂર્ણાંક ms = 1<<frameexp;
    પૂર્ણાંક ગણતરી = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    સંરચના ohci_pipe *pipe = malloc_નીચલું(કદ(*pipe));
    સંરચના ohci_td *tds = malloc_નીચલું(કદ(*tds) * ગણતરી);
    શૂન્ય *data = malloc_નીચલું(maxpacket * ગણતરી);
    જો (!pipe || !tds || !data)
        જાઓ err;
    memset(pipe, 0, કદ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->ગણતરી = ગણતરી;
    pipe->tds = tds;

    સંરચના ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[ગણતરી-1];

    પૂર્ણાંક i;
    માટે (i=0; i<ગણતરી-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    સંરચના ohci_hcca *hcca = (શૂન્ય*)cntl->regs->hcca;
    જો (frameexp == 0) {
        // Add to existing interrupt entry.
        સંરચના ohci_ed *intr_ed = (શૂન્ય*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } નહીંતર {
        પૂર્ણાંક startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        માટે (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    પાછા &pipe->pipe;

err:
    મુક્ત(pipe);
    મુક્ત(tds);
    મુક્ત(data);
    પાછા ખાલી;
}

સંરચના usb_pipe *
ohci_realloc_pipe(સંરચના usbdevice_s *usbdev, સંરચના usb_pipe *upipe
                  , સંરચના usb_endpoint_descriptor *epdesc)
{
    જો (! CONFIG_USB_OHCI)
        પાછા ખાલી;
    usb_add_freelist(upipe);
    જો (!epdesc)
        પાછા ખાલી;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    જો (eptype == USB_ENDPOINT_XFER_INT)
        પાછા ohci_alloc_intr_pipe(usbdev, epdesc);
    સંરચના usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, સંરચના usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    સંરચના usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    જો (usbpipe) {
        // Use previously allocated pipe.
        સંરચના ohci_pipe *pipe = container_of(usbpipe, સંરચના ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        પાછા usbpipe;
    }

    // Allocate a new queue head.
    સંરચના ohci_pipe *pipe;
    જો (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(કદ(*pipe));
    નહીંતર
        pipe = malloc_નીચલું(કદ(*pipe));
    જો (!pipe) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(pipe, 0, કદ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    જો (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    પાછા &pipe->pipe;
}

સ્થિર પૂર્ણાંક
wait_ed(સંરચના ohci_ed *ed, પૂર્ણાંક timeout)
{
    u32 end = timer_calc(timeout);
    માટે (;;) {
        જો ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            પાછા 0;
        જો (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            પાછા -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

પૂર્ણાંક
ohci_send_pipe(સંરચના usb_pipe *p, પૂર્ણાંક dir, અચલ શૂન્ય *cmd
               , શૂન્ય *data, પૂર્ણાંક datasize)
{
    ASSERT32FLAT();
    જો (! CONFIG_USB_OHCI)
        પાછા -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    સંરચના ohci_pipe *pipe = container_of(p, સંરચના ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[કદ(સંરચના ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    સંરચના ohci_td *tds = (શૂન્ય*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, કદ(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    જો (cmd) {
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
    જ્યારે (dest < dataend) {
        // Send data pids
        જો (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            પાછા -1;
        }
        પૂર્ણાંક maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        પૂર્ણાંક transfer = dataend - dest;
        જો (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    જો (cmd) {
        // Send status pid on control transfers
        જો (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            પાછા -1;
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

    પૂર્ણાંક ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    જો (ret)
        ohci_waittick(pipe->regs);
    પાછા ret;
}

પૂર્ણાંક
ohci_poll_intr(સંરચના usb_pipe *p, શૂન્ય *data)
{
    ASSERT16();
    જો (! CONFIG_USB_OHCI)
        પાછા -1;

    સંરચના ohci_pipe *pipe = container_of(p, સંરચના ohci_pipe, pipe);
    સંરચના ohci_td *tds = GET_LOWFLAT(pipe->tds);
    સંરચના ohci_td *head = (શૂન્ય*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    સંરચના ohci_td *tail = (શૂન્ય*)GET_LOWFLAT(pipe->ed.hwTailP);
    પૂર્ણાંક ગણતરી = GET_LOWFLAT(pipe->ગણતરી);
    પૂર્ણાંક pos = (tail - tds + 1) % ગણતરી;
    સંરચના ohci_td *next = &tds[pos];
    જો (head == next)
        // No intrs found.
        પાછા -1;
    // XXX - check for errors.

    // Copy data.
    પૂર્ણાંક maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    શૂન્ય *pipedata = GET_LOWFLAT((pipe->data));
    શૂન્ય *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_નીચલું, LOWFLAT2_નીચલું(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    પાછા 0;
}
