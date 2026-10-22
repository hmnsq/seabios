// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOWFLAT
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "usb.h" // struct usb_s
#ಅನ್ತರ್ಭಾವ "usb-ehci.h" // ehci_wait_controllers
#ಅನ್ತರ್ಭಾವ "usb-ohci.h" // struct ohci_hcca
#ಅನ್ತರ್ಭಾವ "util.h" // msleep
#ಅನ್ತರ್ಭಾವ "x86.h" // readl

#define FIT                     (1 << 31)

ಸಂರಚನಾ usb_ohci_s {
    ಸಂರಚನಾ usb_s usb;
    ಸಂರಚನಾ ohci_regs *regs;
};

ಸಂರಚನಾ ohci_pipe {
    ಸಂರಚನಾ ohci_ed ed;
    ಸಂರಚನಾ usb_pipe pipe;
    ಸಂರಚನಾ ohci_regs *regs;
    ಶೂನ್ಯ *data;
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;
    ಸಂರಚನಾ ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ohci_hub_detect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_ohci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    ಪ್ರತ್ಯಾಗಮ (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
ಸ್ಥಿರ ಶೂನ್ಯ
ohci_hub_disconnect(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_ohci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ohci_hub_reset(ಸಂರಚನಾ usbhub_s *hub, u32 port)
{
    ಸಂರಚನಾ usb_ohci_s *cntl = container_of(hub->cntl, ಸಂರಚನಾ usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    ಪ್ರತಿ (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        ಯದಿ (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }

    ಯದಿ ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        ಪ್ರತ್ಯಾಗಮ -1;

    ಪ್ರತ್ಯಾಗಮ !!(sts & RH_PS_LSDA);
}

ಸ್ಥಿರ ಸಂರಚನಾ usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
check_ohci_ports(ಸಂರಚನಾ usb_ohci_s *cntl)
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

    ಸಂರಚನಾ usbhub_s hub;
    memset(&hub, 0, ಪರಿಮಾಣ(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    ಪ್ರತ್ಯಾಗಮ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ಸ್ಥಿರ ಶೂನ್ಯ
ohci_waittick(ಸಂರಚನಾ ohci_regs *regs)
{
    barrier();
    ಸಂರಚನಾ ohci_hcca *hcca = (ಶೂನ್ಯ*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    ಪ್ರತಿ (;;) {
        ಯದಿ (hcca->frame_no != startframe)
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ;
        }
        yield();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ohci_free_pipes(ಸಂರಚನಾ usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    ಯದಿ (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    ಪ್ರತಿ (;;) {
        ಸಂರಚನಾ ohci_ed *next = (ಶೂನ್ಯ*)*pos;
        ಯದಿ (!next)
            ಭಙ್ಗ;
        ಸಂರಚನಾ ohci_pipe *pipe = container_of(next, ಸಂರಚನಾ ohci_pipe, ed);
        ಯದಿ (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            ಮುಕ್ತ(pipe);
        } ಅನ್ಯಥಾ {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = ರಿಕ್ತ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
start_ohci(ಸಂರಚನಾ usb_ohci_s *cntl, ಸಂರಚನಾ ohci_hcca *hcca)
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
    ಪ್ರತಿ (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        ಯದಿ (!(status & OHCI_HCR))
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
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

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
stop_ohci(ಸಂರಚನಾ usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

ಸ್ಥಿರ ಶೂನ್ಯ
configure_ohci(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ usb_ohci_s *cntl = data;

    // Allocate memory
    ಸಂರಚನಾ ohci_hcca *hcca = memalign_ಮೇಲಿನ(256, ಪರಿಮಾಣ(*hcca));
    ಸಂರಚನಾ ohci_ed *intr_ed = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*intr_ed));
    ಯದಿ (!hcca || !intr_ed) {
        warn_noalloc();
        ಗಚ್ಛ ಮುಕ್ತ;
    }
    memset(hcca, 0, ಪರಿಮಾಣ(*hcca));
    memset(intr_ed, 0, ಪರಿಮಾಣ(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    ಪೂರ್ಣಾಙ್ಕ ret = start_ohci(cntl, hcca);
    ಯದಿ (ret)
        ಗಚ್ಛ err;

    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    ಯದಿ (! ಎಣಿಕೆ)
        ಗಚ್ಛ err;
    ಪ್ರತ್ಯಾಗಮ;

err:
    stop_ohci(cntl);
ಮುಕ್ತ:
    ಮುಕ್ತ(hcca);
    ಮುಕ್ತ(intr_ed);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ohci_controller_setup(ಸಂರಚನಾ pci_device *pci)
{
    ಸಂರಚನಾ ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ಯದಿ (!regs)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ usb_ohci_s *cntl = malloc_tmphigh(ಪರಿಮಾಣ(*cntl));
    ಯದಿ (!cntl) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(cntl, 0, ಪರಿಮಾಣ(*cntl));
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

ಶೂನ್ಯ
ohci_setup(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_USB_OHCI)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
ಸ್ಥಿರ ಶೂನ್ಯ
ohci_desc2pipe(ಸಂರಚನಾ ohci_pipe *pipe, ಸಂರಚನಾ usbdevice_s *usbdev
               , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    ಸಂರಚನಾ usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

ಸ್ಥಿರ ಸಂರಚನಾ usb_pipe *
ohci_alloc_intr_pipe(ಸಂರಚನಾ usbdevice_s *usbdev
                     , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಸಂರಚನಾ usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_ohci_s, usb);
    ಪೂರ್ಣಾಙ್ಕ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ಯದಿ (frameexp > 5)
        frameexp = 5;
    ಪೂರ್ಣಾಙ್ಕ maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    ಪೂರ್ಣಾಙ್ಕ ms = 1<<frameexp;
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    ಸಂರಚನಾ ohci_pipe *pipe = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*pipe));
    ಸಂರಚನಾ ohci_td *tds = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*tds) * ಎಣಿಕೆ);
    ಶೂನ್ಯ *data = malloc_ಕೆಳಗಿನ(maxpacket * ಎಣಿಕೆ);
    ಯದಿ (!pipe || !tds || !data)
        ಗಚ್ಛ err;
    memset(pipe, 0, ಪರಿಮಾಣ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->ಎಣಿಕೆ = ಎಣಿಕೆ;
    pipe->tds = tds;

    ಸಂರಚನಾ ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[ಎಣಿಕೆ-1];

    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ಎಣಿಕೆ-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    ಸಂರಚನಾ ohci_hcca *hcca = (ಶೂನ್ಯ*)cntl->regs->hcca;
    ಯದಿ (frameexp == 0) {
        // Add to existing interrupt entry.
        ಸಂರಚನಾ ohci_ed *intr_ed = (ಶೂನ್ಯ*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } ಅನ್ಯಥಾ {
        ಪೂರ್ಣಾಙ್ಕ startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        ಪ್ರತಿ (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    ಪ್ರತ್ಯಾಗಮ &pipe->pipe;

err:
    ಮುಕ್ತ(pipe);
    ಮುಕ್ತ(tds);
    ಮುಕ್ತ(data);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ usb_pipe *
ohci_realloc_pipe(ಸಂರಚನಾ usbdevice_s *usbdev, ಸಂರಚನಾ usb_pipe *upipe
                  , ಸಂರಚನಾ usb_endpoint_descriptor *epdesc)
{
    ಯದಿ (! CONFIG_USB_OHCI)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    usb_add_freelist(upipe);
    ಯದಿ (!epdesc)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_INT)
        ಪ್ರತ್ಯಾಗಮ ohci_alloc_intr_pipe(usbdev, epdesc);
    ಸಂರಚನಾ usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, ಸಂರಚನಾ usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    ಸಂರಚನಾ usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ಯದಿ (usbpipe) {
        // Use previously allocated pipe.
        ಸಂರಚನಾ ohci_pipe *pipe = container_of(usbpipe, ಸಂರಚನಾ ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        ಪ್ರತ್ಯಾಗಮ usbpipe;
    }

    // Allocate a new queue head.
    ಸಂರಚನಾ ohci_pipe *pipe;
    ಯದಿ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(ಪರಿಮಾಣ(*pipe));
    ಅನ್ಯಥಾ
        pipe = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*pipe));
    ಯದಿ (!pipe) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(pipe, 0, ಪರಿಮಾಣ(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    ಯದಿ (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    ಪ್ರತ್ಯಾಗಮ &pipe->pipe;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
wait_ed(ಸಂರಚನಾ ohci_ed *ed, ಪೂರ್ಣಾಙ್ಕ timeout)
{
    u32 end = timer_calc(timeout);
    ಪ್ರತಿ (;;) {
        ಯದಿ ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            ಪ್ರತ್ಯಾಗಮ 0;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

ಪೂರ್ಣಾಙ್ಕ
ohci_send_pipe(ಸಂರಚನಾ usb_pipe *p, ಪೂರ್ಣಾಙ್ಕ dir, ಧ್ರುವ ಶೂನ್ಯ *cmd
               , ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ datasize)
{
    ASSERT32FLAT();
    ಯದಿ (! CONFIG_USB_OHCI)
        ಪ್ರತ್ಯಾಗಮ -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    ಸಂರಚನಾ ohci_pipe *pipe = container_of(p, ಸಂರಚನಾ ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[ಪರಿಮಾಣ(ಸಂರಚನಾ ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    ಸಂರಚನಾ ohci_td *tds = (ಶೂನ್ಯ*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, ಪರಿಮಾಣ(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    ಯದಿ (cmd) {
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
    ಯಾವತ್ (dest < dataend) {
        // Send data pids
        ಯದಿ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        ಪೂರ್ಣಾಙ್ಕ maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        ಪೂರ್ಣಾಙ್ಕ transfer = dataend - dest;
        ಯದಿ (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    ಯದಿ (cmd) {
        // Send status pid on control transfers
        ಯದಿ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ -1;
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

    ಪೂರ್ಣಾಙ್ಕ ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    ಯದಿ (ret)
        ohci_waittick(pipe->regs);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ
ohci_poll_intr(ಸಂರಚನಾ usb_pipe *p, ಶೂನ್ಯ *data)
{
    ASSERT16();
    ಯದಿ (! CONFIG_USB_OHCI)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ ohci_pipe *pipe = container_of(p, ಸಂರಚನಾ ohci_pipe, pipe);
    ಸಂರಚನಾ ohci_td *tds = GET_LOWFLAT(pipe->tds);
    ಸಂರಚನಾ ohci_td *head = (ಶೂನ್ಯ*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    ಸಂರಚನಾ ohci_td *tail = (ಶೂನ್ಯ*)GET_LOWFLAT(pipe->ed.hwTailP);
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = GET_LOWFLAT(pipe->ಎಣಿಕೆ);
    ಪೂರ್ಣಾಙ್ಕ pos = (tail - tds + 1) % ಎಣಿಕೆ;
    ಸಂರಚನಾ ohci_td *next = &tds[pos];
    ಯದಿ (head == next)
        // No intrs found.
        ಪ್ರತ್ಯಾಗಮ -1;
    // XXX - check for errors.

    // Copy data.
    ಪೂರ್ಣಾಙ್ಕ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ಶೂನ್ಯ *pipedata = GET_LOWFLAT((pipe->data));
    ಶೂನ್ಯ *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_ಕೆಳಗಿನ, LOWFLAT2_ಕೆಳಗಿನ(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    ಪ್ರತ್ಯಾಗಮ 0;
}
