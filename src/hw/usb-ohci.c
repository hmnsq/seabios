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

কাঠামো usb_ohci_s {
    কাঠামো usb_s usb;
    কাঠামো ohci_regs *regs;
};

কাঠামো ohci_pipe {
    কাঠামো ohci_ed ed;
    কাঠামো usb_pipe pipe;
    কাঠামো ohci_regs *regs;
    শূন্য *তথ্য;
    পূর্ণসংখ্যা গুনতি;
    কাঠামো ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
স্থির পূর্ণসংখ্যা
ohci_hub_detect(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_ohci_s *cntl = container_of(hub->cntl, কাঠামো usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    ফেরত (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
স্থির শূন্য
ohci_hub_disconnect(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_ohci_s *cntl = container_of(hub->cntl, কাঠামো usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
স্থির পূর্ণসংখ্যা
ohci_hub_reset(কাঠামো usbhub_s *hub, u32 port)
{
    কাঠামো usb_ohci_s *cntl = container_of(hub->cntl, কাঠামো usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    জন্য (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        যদি (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            বিরতি;
        যদি (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            ফেরত -1;
        }
        yield();
    }

    যদি ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        ফেরত -1;

    ফেরত !!(sts & RH_PS_LSDA);
}

স্থির কাঠামো usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
স্থির পূর্ণসংখ্যা
check_ohci_ports(কাঠামো usb_ohci_s *cntl)
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

    কাঠামো usbhub_s hub;
    memset(&hub, 0, আকার(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    ফেরত hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
স্থির শূন্য
ohci_waittick(কাঠামো ohci_regs *regs)
{
    barrier();
    কাঠামো ohci_hcca *hcca = (শূন্য*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    জন্য (;;) {
        যদি (hcca->frame_no != startframe)
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত;
        }
        yield();
    }
}

স্থির শূন্য
ohci_free_pipes(কাঠামো usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    যদি (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    জন্য (;;) {
        কাঠামো ohci_ed *পরবর্তী = (শূন্য*)*pos;
        যদি (!পরবর্তী)
            বিরতি;
        কাঠামো ohci_pipe *pipe = container_of(পরবর্তী, কাঠামো ohci_pipe, ed);
        যদি (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = পরবর্তী->hw_পরবর্তী_ED;
            মুক্ত(pipe);
        } অন্যথা {
            pos = &পরবর্তী->hw_পরবর্তী_ED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = ফাঁকা;
}

স্থির পূর্ণসংখ্যা
start_ohci(কাঠামো usb_ohci_s *cntl, কাঠামো ohci_hcca *hcca)
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
    জন্য (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        যদি (!(status & OHCI_HCR))
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত -1;
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

    ফেরত 0;
}

স্থির শূন্য
stop_ohci(কাঠামো usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

স্থির শূন্য
configure_ohci(শূন্য *তথ্য)
{
    কাঠামো usb_ohci_s *cntl = তথ্য;

    // Allocate memory
    কাঠামো ohci_hcca *hcca = memalign_উপরের(256, আকার(*hcca));
    কাঠামো ohci_ed *intr_ed = malloc_উপরের(আকার(*intr_ed));
    যদি (!hcca || !intr_ed) {
        warn_noalloc();
        যাও মুক্ত;
    }
    memset(hcca, 0, আকার(*hcca));
    memset(intr_ed, 0, আকার(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<ARRAY_আকার(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    পূর্ণসংখ্যা ret = start_ohci(cntl, hcca);
    যদি (ret)
        যাও err;

    পূর্ণসংখ্যা গুনতি = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    যদি (! গুনতি)
        যাও err;
    ফেরত;

err:
    stop_ohci(cntl);
মুক্ত:
    মুক্ত(hcca);
    মুক্ত(intr_ed);
}

স্থির শূন্য
ohci_controller_setup(কাঠামো pci_device *pci)
{
    কাঠামো ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!regs)
        ফেরত;

    কাঠামো usb_ohci_s *cntl = malloc_tmphigh(আকার(*cntl));
    যদি (!cntl) {
        warn_noalloc();
        ফেরত;
    }
    memset(cntl, 0, আকার(*cntl));
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
        ফেরত;
    কাঠামো pci_device *pci;
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
ohci_desc2pipe(কাঠামো ohci_pipe *pipe, কাঠামো usbdevice_s *usbdev
               , কাঠামো usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->w_Max_Packet_আকার << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    কাঠামো usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, কাঠামো usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

স্থির কাঠামো usb_pipe *
ohci_alloc_intr_pipe(কাঠামো usbdevice_s *usbdev
                     , কাঠামো usb_endpoint_descriptor *epdesc)
{
    কাঠামো usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, কাঠামো usb_ohci_s, usb);
    পূর্ণসংখ্যা frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    যদি (frameexp > 5)
        frameexp = 5;
    পূর্ণসংখ্যা maxpacket = epdesc->w_Max_Packet_আকার;
    // Determine number of entries needed for 2 timer ticks.
    পূর্ণসংখ্যা ms = 1<<frameexp;
    পূর্ণসংখ্যা গুনতি = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    কাঠামো ohci_pipe *pipe = malloc_নিচের(আকার(*pipe));
    কাঠামো ohci_td *tds = malloc_নিচের(আকার(*tds) * গুনতি);
    শূন্য *তথ্য = malloc_নিচের(maxpacket * গুনতি);
    যদি (!pipe || !tds || !তথ্য)
        যাও err;
    memset(pipe, 0, আকার(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->তথ্য = তথ্য;
    pipe->গুনতি = গুনতি;
    pipe->tds = tds;

    কাঠামো ohci_ed *ed = &pipe->ed;
    ed->hw_শীর্ষ_P = (u32)&tds[0];
    ed->hw_লেজ_P = (u32)&tds[গুনতি-1];

    পূর্ণসংখ্যা i;
    জন্য (i=0; i<গুনতি-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)তথ্য + maxpacket * i;
        tds[i].hw_পরবর্তী_TD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    কাঠামো ohci_hcca *hcca = (শূন্য*)cntl->regs->hcca;
    যদি (frameexp == 0) {
        // Add to existing interrupt entry.
        কাঠামো ohci_ed *intr_ed = (শূন্য*)hcca->int_table[0];
        ed->hw_পরবর্তী_ED = intr_ed->hw_পরবর্তী_ED;
        barrier();
        intr_ed->hw_পরবর্তী_ED = (u32)ed;
    } অন্যথা {
        পূর্ণসংখ্যা startpos = 1<<(frameexp-1);
        ed->hw_পরবর্তী_ED = hcca->int_table[startpos];
        barrier();
        জন্য (i=startpos; i<ARRAY_আকার(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    ফেরত &pipe->pipe;

err:
    মুক্ত(pipe);
    মুক্ত(tds);
    মুক্ত(তথ্য);
    ফেরত ফাঁকা;
}

কাঠামো usb_pipe *
ohci_realloc_pipe(কাঠামো usbdevice_s *usbdev, কাঠামো usb_pipe *upipe
                  , কাঠামো usb_endpoint_descriptor *epdesc)
{
    যদি (! CONFIG_USB_OHCI)
        ফেরত ফাঁকা;
    usb_add_freelist(upipe);
    যদি (!epdesc)
        ফেরত ফাঁকা;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    যদি (eptype == USB_ENDPOINT_XFER_INT)
        ফেরত ohci_alloc_intr_pipe(usbdev, epdesc);
    কাঠামো usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, কাঠামো usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    কাঠামো usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    যদি (usbpipe) {
        // Use previously allocated pipe.
        কাঠামো ohci_pipe *pipe = container_of(usbpipe, কাঠামো ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        ফেরত usbpipe;
    }

    // Allocate a new queue head.
    কাঠামো ohci_pipe *pipe;
    যদি (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(আকার(*pipe));
    অন্যথা
        pipe = malloc_নিচের(আকার(*pipe));
    যদি (!pipe) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(pipe, 0, আকার(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *শীর্ষ = &cntl->regs->ed_controlhead;
    যদি (eptype != USB_ENDPOINT_XFER_CONTROL)
        শীর্ষ = &cntl->regs->ed_bulkhead;
    pipe->ed.hw_পরবর্তী_ED = *শীর্ষ;
    barrier();
    *শীর্ষ = (u32)&pipe->ed;
    ফেরত &pipe->pipe;
}

স্থির পূর্ণসংখ্যা
wait_ed(কাঠামো ohci_ed *ed, পূর্ণসংখ্যা timeout)
{
    u32 end = timer_calc(timeout);
    জন্য (;;) {
        যদি ((ed->hw_শীর্ষ_P & ~(ED_C|ED_H)) == ed->hw_লেজ_P)
            ফেরত 0;
        যদি (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hw_লেজ_P, ed->hw_শীর্ষ_P, ed->hw_পরবর্তী_ED);
            ফেরত -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

পূর্ণসংখ্যা
ohci_send_pipe(কাঠামো usb_pipe *p, পূর্ণসংখ্যা dir, ধ্রুবক শূন্য *cmd
               , শূন্য *তথ্য, পূর্ণসংখ্যা datasize)
{
    ASSERT32FLAT();
    যদি (! CONFIG_USB_OHCI)
        ফেরত -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    কাঠামো ohci_pipe *pipe = container_of(p, কাঠামো ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[আকার(কাঠামো ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    কাঠামো ohci_td *tds = (শূন্য*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, আকার(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    যদি (cmd) {
        // Send setup pid on control transfers
        td->hwINFO = TD_DP_SETUP | TD_T_DATA0 | TD_CC;
        td->hwCBP = (u32)cmd;
        td->hw_পরবর্তী_TD = (u32)&td[1];
        td->hwBE = (u32)cmd + USB_CONTROL_SETUP_আকার - 1;
        td++;
        toggle = TD_T_DATA1;
        statuscmd = OHCI_CLF;
    }
    u32 dest = (u32)তথ্য, dataend = dest + datasize;
    যখন (dest < dataend) {
        // Send data pids
        যদি (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ফেরত -1;
        }
        পূর্ণসংখ্যা maxtransfer = 2*PAGE_আকার - (dest & (PAGE_আকার-1));
        পূর্ণসংখ্যা transfer = dataend - dest;
        যদি (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hw_পরবর্তী_TD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    যদি (cmd) {
        // Send status pid on control transfers
        যদি (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ফেরত -1;
        }
        td->hwINFO = (dir ? TD_DP_OUT : TD_DP_IN) | TD_T_DATA1 | TD_CC;
        td->hwCBP = 0;
        td->hw_পরবর্তী_TD = (u32)&td[1];
        td->hwBE = 0;
        td++;
    }

    // Transfer data
    pipe->ed.hw_শীর্ষ_P = (u32)tds | (pipe->ed.hw_শীর্ষ_P & ED_C);
    pipe->ed.hw_লেজ_P = (u32)td;
    barrier();
    pipe->ed.hwINFO &= ~ED_SKIP;
    writel(&pipe->regs->cmdstatus, statuscmd);

    পূর্ণসংখ্যা ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    যদি (ret)
        ohci_waittick(pipe->regs);
    ফেরত ret;
}

পূর্ণসংখ্যা
ohci_poll_intr(কাঠামো usb_pipe *p, শূন্য *তথ্য)
{
    ASSERT16();
    যদি (! CONFIG_USB_OHCI)
        ফেরত -1;

    কাঠামো ohci_pipe *pipe = container_of(p, কাঠামো ohci_pipe, pipe);
    কাঠামো ohci_td *tds = GET_LOWFLAT(pipe->tds);
    কাঠামো ohci_td *শীর্ষ = (শূন্য*)(GET_LOWFLAT(pipe->ed.hw_শীর্ষ_P) & ~(ED_C|ED_H));
    কাঠামো ohci_td *লেজ = (শূন্য*)GET_LOWFLAT(pipe->ed.hw_লেজ_P);
    পূর্ণসংখ্যা গুনতি = GET_LOWFLAT(pipe->গুনতি);
    পূর্ণসংখ্যা pos = (লেজ - tds + 1) % গুনতি;
    কাঠামো ohci_td *পরবর্তী = &tds[pos];
    যদি (শীর্ষ == পরবর্তী)
        // No intrs found.
        ফেরত -1;
    // XXX - check for errors.

    // Copy data.
    পূর্ণসংখ্যা maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    শূন্য *pipedata = GET_LOWFLAT((pipe->তথ্য));
    শূন্য *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), তথ্য, SEG_নিচের, LOWFLAT2_নিচের(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(লেজ->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (লেজ-tds);
    SET_LOWFLAT(লেজ->hwCBP, (u32)intrdata);
    SET_LOWFLAT(লেজ->hw_পরবর্তী_TD, (u32)পরবর্তী);
    SET_LOWFLAT(লেজ->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hw_লেজ_P, (u32)পরবর্তী);

    ফেরত 0;
}
