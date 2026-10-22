// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_LOWFLAT
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "memmap.h" // PAGE_SIZE
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "usb.h" // struct usb_s
#ଅନ୍ତର୍ଭାଵ "usb-ehci.h" // ehci_wait_controllers
#ଅନ୍ତର୍ଭାଵ "usb-ohci.h" // struct ohci_hcca
#ଅନ୍ତର୍ଭାଵ "util.h" // msleep
#ଅନ୍ତର୍ଭାଵ "x86.h" // readl

#define FIT                     (1 << 31)

ସଂରଚନା usb_ohci_s {
    ସଂରଚନା usb_s usb;
    ସଂରଚନା ohci_regs *regs;
};

ସଂରଚନା ohci_pipe {
    ସଂରଚନା ohci_ed ed;
    ସଂରଚନା usb_pipe pipe;
    ସଂରଚନା ohci_regs *regs;
    ଶୂନ୍ୟ *data;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;
    ସଂରଚନା ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ohci_hub_detect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_ohci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    ଫେରନ୍ତୁ (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
ସ୍ଥିର ଶୂନ୍ୟ
ohci_hub_disconnect(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_ohci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ohci_hub_reset(ସଂରଚନା usbhub_s *hub, u32 port)
{
    ସଂରଚନା usb_ohci_s *cntl = container_of(hub->cntl, ସଂରଚନା usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    ପାଇଁ (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        ଯଦି (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }

    ଯଦି ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        ଫେରନ୍ତୁ -1;

    ଫେରନ୍ତୁ !!(sts & RH_PS_LSDA);
}

ସ୍ଥିର ସଂରଚନା usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
check_ohci_ports(ସଂରଚନା usb_ohci_s *cntl)
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

    ସଂରଚନା usbhub_s hub;
    memset(&hub, 0, ଆକାର(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    ଫେରନ୍ତୁ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ସ୍ଥିର ଶୂନ୍ୟ
ohci_waittick(ସଂରଚନା ohci_regs *regs)
{
    barrier();
    ସଂରଚନା ohci_hcca *hcca = (ଶୂନ୍ୟ*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    ପାଇଁ (;;) {
        ଯଦି (hcca->frame_no != startframe)
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ;
        }
        yield();
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
ohci_free_pipes(ସଂରଚନା usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    ଯଦି (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    ପାଇଁ (;;) {
        ସଂରଚନା ohci_ed *next = (ଶୂନ୍ୟ*)*pos;
        ଯଦି (!next)
            ଭାଙ୍ଗ;
        ସଂରଚନା ohci_pipe *pipe = container_of(next, ସଂରଚନା ohci_pipe, ed);
        ଯଦି (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            ମୁକ୍ତ(pipe);
        } ଅନ୍ୟଥା {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = ଖାଲି;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
start_ohci(ସଂରଚନା usb_ohci_s *cntl, ସଂରଚନା ohci_hcca *hcca)
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
    ପାଇଁ (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        ଯଦି (!(status & OHCI_HCR))
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
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

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ
stop_ohci(ସଂରଚନା usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

ସ୍ଥିର ଶୂନ୍ୟ
configure_ohci(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା usb_ohci_s *cntl = data;

    // Allocate memory
    ସଂରଚନା ohci_hcca *hcca = memalign_ଉପର(256, ଆକାର(*hcca));
    ସଂରଚନା ohci_ed *intr_ed = malloc_ଉପର(ଆକାର(*intr_ed));
    ଯଦି (!hcca || !intr_ed) {
        warn_noalloc();
        ଯାଅ ମୁକ୍ତ;
    }
    memset(hcca, 0, ଆକାର(*hcca));
    memset(intr_ed, 0, ଆକାର(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = start_ohci(cntl, hcca);
    ଯଦି (ret)
        ଯାଅ err;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    ଯଦି (! ଗଣନା_2)
        ଯାଅ err;
    ଫେରନ୍ତୁ;

err:
    stop_ohci(cntl);
ମୁକ୍ତ:
    ମୁକ୍ତ(hcca);
    ମୁକ୍ତ(intr_ed);
}

ସ୍ଥିର ଶୂନ୍ୟ
ohci_controller_setup(ସଂରଚନା pci_device *pci)
{
    ସଂରଚନା ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ଯଦି (!regs)
        ଫେରନ୍ତୁ;

    ସଂରଚନା usb_ohci_s *cntl = malloc_tmphigh(ଆକାର(*cntl));
    ଯଦି (!cntl) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(cntl, 0, ଆକାର(*cntl));
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

ଶୂନ୍ୟ
ohci_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_USB_OHCI)
        ଫେରନ୍ତୁ;
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
ସ୍ଥିର ଶୂନ୍ୟ
ohci_desc2pipe(ସଂରଚନା ohci_pipe *pipe, ସଂରଚନା usbdevice_s *usbdev
               , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    ସଂରଚନା usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

ସ୍ଥିର ସଂରଚନା usb_pipe *
ohci_alloc_intr_pipe(ସଂରଚନା usbdevice_s *usbdev
                     , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ସଂରଚନା usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_ohci_s, usb);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ଯଦି (frameexp > 5)
        frameexp = 5;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ms = 1<<frameexp;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    ସଂରଚନା ohci_pipe *pipe = malloc_ତଳ(ଆକାର(*pipe));
    ସଂରଚନା ohci_td *tds = malloc_ତଳ(ଆକାର(*tds) * ଗଣନା_2);
    ଶୂନ୍ୟ *data = malloc_ତଳ(maxpacket * ଗଣନା_2);
    ଯଦି (!pipe || !tds || !data)
        ଯାଅ err;
    memset(pipe, 0, ଆକାର(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->ଗଣନା_2 = ଗଣନା_2;
    pipe->tds = tds;

    ସଂରଚନା ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[ଗଣନା_2-1];

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ଗଣନା_2-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    ସଂରଚନା ohci_hcca *hcca = (ଶୂନ୍ୟ*)cntl->regs->hcca;
    ଯଦି (frameexp == 0) {
        // Add to existing interrupt entry.
        ସଂରଚନା ohci_ed *intr_ed = (ଶୂନ୍ୟ*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } ଅନ୍ୟଥା {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        ପାଇଁ (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    ଫେରନ୍ତୁ &pipe->pipe;

err:
    ମୁକ୍ତ(pipe);
    ମୁକ୍ତ(tds);
    ମୁକ୍ତ(data);
    ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା usb_pipe *
ohci_realloc_pipe(ସଂରଚନା usbdevice_s *usbdev, ସଂରଚନା usb_pipe *upipe
                  , ସଂରଚନା usb_endpoint_descriptor *epdesc)
{
    ଯଦି (! CONFIG_USB_OHCI)
        ଫେରନ୍ତୁ ଖାଲି;
    usb_add_freelist(upipe);
    ଯଦି (!epdesc)
        ଫେରନ୍ତୁ ଖାଲି;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ଯଦି (eptype == USB_ENDPOINT_XFER_INT)
        ଫେରନ୍ତୁ ohci_alloc_intr_pipe(usbdev, epdesc);
    ସଂରଚନା usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ସଂରଚନା usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    ସଂରଚନା usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ଯଦି (usbpipe) {
        // Use previously allocated pipe.
        ସଂରଚନା ohci_pipe *pipe = container_of(usbpipe, ସଂରଚନା ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        ଫେରନ୍ତୁ usbpipe;
    }

    // Allocate a new queue head.
    ସଂରଚନା ohci_pipe *pipe;
    ଯଦି (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(ଆକାର(*pipe));
    ଅନ୍ୟଥା
        pipe = malloc_ତଳ(ଆକାର(*pipe));
    ଯଦି (!pipe) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(pipe, 0, ଆକାର(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    ଯଦି (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    ଫେରନ୍ତୁ &pipe->pipe;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
wait_ed(ସଂରଚନା ohci_ed *ed, ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
    u32 end = timer_calc(timeout);
    ପାଇଁ (;;) {
        ଯଦି ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            ଫେରନ୍ତୁ 0;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ohci_send_pipe(ସଂରଚନା usb_pipe *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଅଚଳ ଶୂନ୍ୟ *cmd
               , ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା datasize)
{
    ASSERT32FLAT();
    ଯଦି (! CONFIG_USB_OHCI)
        ଫେରନ୍ତୁ -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    ସଂରଚନା ohci_pipe *pipe = container_of(p, ସଂରଚନା ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[ଆକାର(ସଂରଚନା ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    ସଂରଚନା ohci_td *tds = (ଶୂନ୍ୟ*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, ଆକାର(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    ଯଦି (cmd) {
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
    ଯେତେବେଳେ (dest < dataend) {
        // Send data pids
        ଯଦି (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ଫେରନ୍ତୁ -1;
        }
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା transfer = dataend - dest;
        ଯଦି (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    ଯଦି (cmd) {
        // Send status pid on control transfers
        ଯଦି (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ଫେରନ୍ତୁ -1;
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

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    ଯଦି (ret)
        ohci_waittick(pipe->regs);
    ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ohci_poll_intr(ସଂରଚନା usb_pipe *p, ଶୂନ୍ୟ *data)
{
    ASSERT16();
    ଯଦି (! CONFIG_USB_OHCI)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା ohci_pipe *pipe = container_of(p, ସଂରଚନା ohci_pipe, pipe);
    ସଂରଚନା ohci_td *tds = GET_LOWFLAT(pipe->tds);
    ସଂରଚନା ohci_td *head = (ଶୂନ୍ୟ*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    ସଂରଚନା ohci_td *tail = (ଶୂନ୍ୟ*)GET_LOWFLAT(pipe->ed.hwTailP);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = GET_LOWFLAT(pipe->ଗଣନା_2);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos = (tail - tds + 1) % ଗଣନା_2;
    ସଂରଚନା ohci_td *next = &tds[pos];
    ଯଦି (head == next)
        // No intrs found.
        ଫେରନ୍ତୁ -1;
    // XXX - check for errors.

    // Copy data.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ଶୂନ୍ୟ *pipedata = GET_LOWFLAT((pipe->data));
    ଶୂନ୍ୟ *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_ତଳ, LOWFLAT2_ତଳ(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    ଫେରନ୍ତୁ 0;
}
