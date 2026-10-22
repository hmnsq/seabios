// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_LOWFLAT
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "memmap.h" // PAGE_SIZE
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#அந்தர்பாவ "pci_regs.h" // PCI_BASE_ADDRESS_0
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "usb.h" // struct usb_s
#அந்தர்பாவ "usb-ehci.h" // ehci_wait_controllers
#அந்தர்பாவ "usb-ohci.h" // struct ohci_hcca
#அந்தர்பாவ "util.h" // msleep
#அந்தர்பாவ "x86.h" // readl

#define FIT                     (1 << 31)

கட்டமைப்பு usb_ohci_s {
    கட்டமைப்பு usb_s usb;
    கட்டமைப்பு ohci_regs *regs;
};

கட்டமைப்பு ohci_pipe {
    கட்டமைப்பு ohci_ed ed;
    கட்டமைப்பு usb_pipe pipe;
    கட்டமைப்பு ohci_regs *regs;
    வெறுமை *data;
    முழு மொத்தம்;
    கட்டமைப்பு ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
நிலையான முழு
ohci_hub_detect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_ohci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    திரும்பு (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
நிலையான வெறுமை
ohci_hub_disconnect(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_ohci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
நிலையான முழு
ohci_hub_reset(கட்டமைப்பு usbhub_s *hub, u32 port)
{
    கட்டமைப்பு usb_ohci_s *cntl = container_of(hub->cntl, கட்டமைப்பு usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    க்கு (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        எனில் (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            நிறுத்து;
        எனில் (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            திரும்பு -1;
        }
        yield();
    }

    எனில் ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        திரும்பு -1;

    திரும்பு !!(sts & RH_PS_LSDA);
}

நிலையான கட்டமைப்பு usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
நிலையான முழு
check_ohci_ports(கட்டமைப்பு usb_ohci_s *cntl)
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

    கட்டமைப்பு usbhub_s hub;
    memset(&hub, 0, அளவு(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    திரும்பு hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
நிலையான வெறுமை
ohci_waittick(கட்டமைப்பு ohci_regs *regs)
{
    barrier();
    கட்டமைப்பு ohci_hcca *hcca = (வெறுமை*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    க்கு (;;) {
        எனில் (hcca->frame_no != startframe)
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு;
        }
        yield();
    }
}

நிலையான வெறுமை
ohci_free_pipes(கட்டமைப்பு usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    எனில் (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    க்கு (;;) {
        கட்டமைப்பு ohci_ed *next = (வெறுமை*)*pos;
        எனில் (!next)
            நிறுத்து;
        கட்டமைப்பு ohci_pipe *pipe = container_of(next, கட்டமைப்பு ohci_pipe, ed);
        எனில் (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            விடுவி(pipe);
        } இல்லையெனில் {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = வெற்று;
}

நிலையான முழு
start_ohci(கட்டமைப்பு usb_ohci_s *cntl, கட்டமைப்பு ohci_hcca *hcca)
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
    க்கு (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        எனில் (!(status & OHCI_HCR))
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
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

    திரும்பு 0;
}

நிலையான வெறுமை
stop_ohci(கட்டமைப்பு usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

நிலையான வெறுமை
configure_ohci(வெறுமை *data)
{
    கட்டமைப்பு usb_ohci_s *cntl = data;

    // Allocate memory
    கட்டமைப்பு ohci_hcca *hcca = memalign_உயர்வு(256, அளவு(*hcca));
    கட்டமைப்பு ohci_ed *intr_ed = malloc_உயர்வு(அளவு(*intr_ed));
    எனில் (!hcca || !intr_ed) {
        warn_noalloc();
        செல் விடுவி;
    }
    memset(hcca, 0, அளவு(*hcca));
    memset(intr_ed, 0, அளவு(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    முழு i;
    க்கு (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    முழு ret = start_ohci(cntl, hcca);
    எனில் (ret)
        செல் err;

    முழு மொத்தம் = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    எனில் (! மொத்தம்)
        செல் err;
    திரும்பு;

err:
    stop_ohci(cntl);
விடுவி:
    விடுவி(hcca);
    விடுவி(intr_ed);
}

நிலையான வெறுமை
ohci_controller_setup(கட்டமைப்பு pci_device *pci)
{
    கட்டமைப்பு ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    எனில் (!regs)
        திரும்பு;

    கட்டமைப்பு usb_ohci_s *cntl = malloc_tmphigh(அளவு(*cntl));
    எனில் (!cntl) {
        warn_noalloc();
        திரும்பு;
    }
    memset(cntl, 0, அளவு(*cntl));
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

வெறுமை
ohci_setup(வெறுமை)
{
    எனில் (! CONFIG_USB_OHCI)
        திரும்பு;
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
நிலையான வெறுமை
ohci_desc2pipe(கட்டமைப்பு ohci_pipe *pipe, கட்டமைப்பு usbdevice_s *usbdev
               , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    கட்டமைப்பு usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

நிலையான கட்டமைப்பு usb_pipe *
ohci_alloc_intr_pipe(கட்டமைப்பு usbdevice_s *usbdev
                     , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    கட்டமைப்பு usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_ohci_s, usb);
    முழு frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    எனில் (frameexp > 5)
        frameexp = 5;
    முழு maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    முழு ms = 1<<frameexp;
    முழு மொத்தம் = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    கட்டமைப்பு ohci_pipe *pipe = malloc_கீழ்(அளவு(*pipe));
    கட்டமைப்பு ohci_td *tds = malloc_கீழ்(அளவு(*tds) * மொத்தம்);
    வெறுமை *data = malloc_கீழ்(maxpacket * மொத்தம்);
    எனில் (!pipe || !tds || !data)
        செல் err;
    memset(pipe, 0, அளவு(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->மொத்தம் = மொத்தம்;
    pipe->tds = tds;

    கட்டமைப்பு ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[மொத்தம்-1];

    முழு i;
    க்கு (i=0; i<மொத்தம்-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    கட்டமைப்பு ohci_hcca *hcca = (வெறுமை*)cntl->regs->hcca;
    எனில் (frameexp == 0) {
        // Add to existing interrupt entry.
        கட்டமைப்பு ohci_ed *intr_ed = (வெறுமை*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } இல்லையெனில் {
        முழு startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        க்கு (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    திரும்பு &pipe->pipe;

err:
    விடுவி(pipe);
    விடுவி(tds);
    விடுவி(data);
    திரும்பு வெற்று;
}

கட்டமைப்பு usb_pipe *
ohci_realloc_pipe(கட்டமைப்பு usbdevice_s *usbdev, கட்டமைப்பு usb_pipe *upipe
                  , கட்டமைப்பு usb_endpoint_descriptor *epdesc)
{
    எனில் (! CONFIG_USB_OHCI)
        திரும்பு வெற்று;
    usb_add_freelist(upipe);
    எனில் (!epdesc)
        திரும்பு வெற்று;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    எனில் (eptype == USB_ENDPOINT_XFER_INT)
        திரும்பு ohci_alloc_intr_pipe(usbdev, epdesc);
    கட்டமைப்பு usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, கட்டமைப்பு usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    கட்டமைப்பு usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    எனில் (usbpipe) {
        // Use previously allocated pipe.
        கட்டமைப்பு ohci_pipe *pipe = container_of(usbpipe, கட்டமைப்பு ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        திரும்பு usbpipe;
    }

    // Allocate a new queue head.
    கட்டமைப்பு ohci_pipe *pipe;
    எனில் (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(அளவு(*pipe));
    இல்லையெனில்
        pipe = malloc_கீழ்(அளவு(*pipe));
    எனில் (!pipe) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(pipe, 0, அளவு(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    எனில் (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    திரும்பு &pipe->pipe;
}

நிலையான முழு
wait_ed(கட்டமைப்பு ohci_ed *ed, முழு timeout)
{
    u32 end = timer_calc(timeout);
    க்கு (;;) {
        எனில் ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            திரும்பு 0;
        எனில் (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            திரும்பு -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

முழு
ohci_send_pipe(கட்டமைப்பு usb_pipe *p, முழு dir, மாறிலி வெறுமை *cmd
               , வெறுமை *data, முழு datasize)
{
    ASSERT32FLAT();
    எனில் (! CONFIG_USB_OHCI)
        திரும்பு -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    கட்டமைப்பு ohci_pipe *pipe = container_of(p, கட்டமைப்பு ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[அளவு(கட்டமைப்பு ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    கட்டமைப்பு ohci_td *tds = (வெறுமை*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, அளவு(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    எனில் (cmd) {
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
    போது (dest < dataend) {
        // Send data pids
        எனில் (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            திரும்பு -1;
        }
        முழு maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        முழு transfer = dataend - dest;
        எனில் (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    எனில் (cmd) {
        // Send status pid on control transfers
        எனில் (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            திரும்பு -1;
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

    முழு ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    எனில் (ret)
        ohci_waittick(pipe->regs);
    திரும்பு ret;
}

முழு
ohci_poll_intr(கட்டமைப்பு usb_pipe *p, வெறுமை *data)
{
    ASSERT16();
    எனில் (! CONFIG_USB_OHCI)
        திரும்பு -1;

    கட்டமைப்பு ohci_pipe *pipe = container_of(p, கட்டமைப்பு ohci_pipe, pipe);
    கட்டமைப்பு ohci_td *tds = GET_LOWFLAT(pipe->tds);
    கட்டமைப்பு ohci_td *head = (வெறுமை*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    கட்டமைப்பு ohci_td *tail = (வெறுமை*)GET_LOWFLAT(pipe->ed.hwTailP);
    முழு மொத்தம் = GET_LOWFLAT(pipe->மொத்தம்);
    முழு pos = (tail - tds + 1) % மொத்தம்;
    கட்டமைப்பு ohci_td *next = &tds[pos];
    எனில் (head == next)
        // No intrs found.
        திரும்பு -1;
    // XXX - check for errors.

    // Copy data.
    முழு maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    வெறுமை *pipedata = GET_LOWFLAT((pipe->data));
    வெறுமை *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_கீழ், LOWFLAT2_கீழ்(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    திரும்பு 0;
}
