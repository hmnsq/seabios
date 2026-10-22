// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_LOWFLAT
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "memmap.h" // PAGE_SIZE
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#अन्तर्भाव "pci_regs.h" // PCI_BASE_ADDRESS_0
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "usb.h" // struct usb_s
#अन्तर्भाव "usb-ehci.h" // ehci_wait_controllers
#अन्तर्भाव "usb-ohci.h" // struct ohci_hcca
#अन्तर्भाव "util.h" // msleep
#अन्तर्भाव "x86.h" // readl

#define FIT                     (1 << 31)

संरचना usb_ohci_s {
    संरचना usb_s usb;
    संरचना ohci_regs *regs;
};

संरचना ohci_pipe {
    संरचना ohci_ed ed;
    संरचना usb_pipe pipe;
    संरचना ohci_regs *regs;
    शून्य *data;
    पूर्णाङ्क गणनम्;
    संरचना ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
स्थिर पूर्णाङ्क
ohci_hub_detect(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_ohci_s *cntl = container_of(hub->cntl, संरचना usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    प्रत्यागम (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
स्थिर शून्य
ohci_hub_disconnect(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_ohci_s *cntl = container_of(hub->cntl, संरचना usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
स्थिर पूर्णाङ्क
ohci_hub_reset(संरचना usbhub_s *hub, u32 port)
{
    संरचना usb_ohci_s *cntl = container_of(hub->cntl, संरचना usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    प्रति (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        जइ (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            भङ्ग;
        जइ (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            प्रत्यागम -1;
        }
        yield();
    }

    जइ ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        प्रत्यागम -1;

    प्रत्यागम !!(sts & RH_PS_LSDA);
}

स्थिर संरचना usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
स्थिर पूर्णाङ्क
check_ohci_ports(संरचना usb_ohci_s *cntl)
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

    संरचना usbhub_s hub;
    memset(&hub, 0, परिमाण(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    प्रत्यागम hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
स्थिर शून्य
ohci_waittick(संरचना ohci_regs *regs)
{
    barrier();
    संरचना ohci_hcca *hcca = (शून्य*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    प्रति (;;) {
        जइ (hcca->frame_no != startframe)
            भङ्ग;
        जइ (timer_check(end)) {
            warn_timeout();
            प्रत्यागम;
        }
        yield();
    }
}

स्थिर शून्य
ohci_free_pipes(संरचना usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    जइ (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    प्रति (;;) {
        संरचना ohci_ed *next = (शून्य*)*pos;
        जइ (!next)
            भङ्ग;
        संरचना ohci_pipe *pipe = container_of(next, संरचना ohci_pipe, ed);
        जइ (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            मुक्त(pipe);
        } अण्णहा {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = रिक्त;
}

स्थिर पूर्णाङ्क
start_ohci(संरचना usb_ohci_s *cntl, संरचना ohci_hcca *hcca)
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
    प्रति (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        जइ (!(status & OHCI_HCR))
            भङ्ग;
        जइ (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
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

    प्रत्यागम 0;
}

स्थिर शून्य
stop_ohci(संरचना usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

स्थिर शून्य
configure_ohci(शून्य *data)
{
    संरचना usb_ohci_s *cntl = data;

    // Allocate memory
    संरचना ohci_hcca *hcca = memalign_उच्चः(256, परिमाण(*hcca));
    संरचना ohci_ed *intr_ed = malloc_उच्चः(परिमाण(*intr_ed));
    जइ (!hcca || !intr_ed) {
        warn_noalloc();
        गच्छ मुक्त;
    }
    memset(hcca, 0, परिमाण(*hcca));
    memset(intr_ed, 0, परिमाण(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    पूर्णाङ्क i;
    प्रति (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    पूर्णाङ्क ret = start_ohci(cntl, hcca);
    जइ (ret)
        गच्छ err;

    पूर्णाङ्क गणनम् = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    जइ (! गणनम्)
        गच्छ err;
    प्रत्यागम;

err:
    stop_ohci(cntl);
मुक्त:
    मुक्त(hcca);
    मुक्त(intr_ed);
}

स्थिर शून्य
ohci_controller_setup(संरचना pci_device *pci)
{
    संरचना ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    जइ (!regs)
        प्रत्यागम;

    संरचना usb_ohci_s *cntl = malloc_tmphigh(परिमाण(*cntl));
    जइ (!cntl) {
        warn_noalloc();
        प्रत्यागम;
    }
    memset(cntl, 0, परिमाण(*cntl));
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

शून्य
ohci_setup(शून्य)
{
    जइ (! CONFIG_USB_OHCI)
        प्रत्यागम;
    संरचना pci_device *pci;
    foreachpci(pci) {
        जइ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
स्थिर शून्य
ohci_desc2pipe(संरचना ohci_pipe *pipe, संरचना usbdevice_s *usbdev
               , संरचना usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    संरचना usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, संरचना usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

स्थिर संरचना usb_pipe *
ohci_alloc_intr_pipe(संरचना usbdevice_s *usbdev
                     , संरचना usb_endpoint_descriptor *epdesc)
{
    संरचना usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, संरचना usb_ohci_s, usb);
    पूर्णाङ्क frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    जइ (frameexp > 5)
        frameexp = 5;
    पूर्णाङ्क maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    पूर्णाङ्क ms = 1<<frameexp;
    पूर्णाङ्क गणनम् = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    संरचना ohci_pipe *pipe = malloc_नीचः(परिमाण(*pipe));
    संरचना ohci_td *tds = malloc_नीचः(परिमाण(*tds) * गणनम्);
    शून्य *data = malloc_नीचः(maxpacket * गणनम्);
    जइ (!pipe || !tds || !data)
        गच्छ err;
    memset(pipe, 0, परिमाण(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->गणनम् = गणनम्;
    pipe->tds = tds;

    संरचना ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[गणनम्-1];

    पूर्णाङ्क i;
    प्रति (i=0; i<गणनम्-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    संरचना ohci_hcca *hcca = (शून्य*)cntl->regs->hcca;
    जइ (frameexp == 0) {
        // Add to existing interrupt entry.
        संरचना ohci_ed *intr_ed = (शून्य*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } अण्णहा {
        पूर्णाङ्क startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        प्रति (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    प्रत्यागम &pipe->pipe;

err:
    मुक्त(pipe);
    मुक्त(tds);
    मुक्त(data);
    प्रत्यागम रिक्त;
}

संरचना usb_pipe *
ohci_realloc_pipe(संरचना usbdevice_s *usbdev, संरचना usb_pipe *upipe
                  , संरचना usb_endpoint_descriptor *epdesc)
{
    जइ (! CONFIG_USB_OHCI)
        प्रत्यागम रिक्त;
    usb_add_freelist(upipe);
    जइ (!epdesc)
        प्रत्यागम रिक्त;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    जइ (eptype == USB_ENDPOINT_XFER_INT)
        प्रत्यागम ohci_alloc_intr_pipe(usbdev, epdesc);
    संरचना usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, संरचना usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    संरचना usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    जइ (usbpipe) {
        // Use previously allocated pipe.
        संरचना ohci_pipe *pipe = container_of(usbpipe, संरचना ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        प्रत्यागम usbpipe;
    }

    // Allocate a new queue head.
    संरचना ohci_pipe *pipe;
    जइ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(परिमाण(*pipe));
    अण्णहा
        pipe = malloc_नीचः(परिमाण(*pipe));
    जइ (!pipe) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    memset(pipe, 0, परिमाण(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    जइ (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    प्रत्यागम &pipe->pipe;
}

स्थिर पूर्णाङ्क
wait_ed(संरचना ohci_ed *ed, पूर्णाङ्क timeout)
{
    u32 end = timer_calc(timeout);
    प्रति (;;) {
        जइ ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            प्रत्यागम 0;
        जइ (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            प्रत्यागम -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

पूर्णाङ्क
ohci_send_pipe(संरचना usb_pipe *p, पूर्णाङ्क dir, ध्रुव शून्य *cmd
               , शून्य *data, पूर्णाङ्क datasize)
{
    ASSERT32FLAT();
    जइ (! CONFIG_USB_OHCI)
        प्रत्यागम -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    संरचना ohci_pipe *pipe = container_of(p, संरचना ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[परिमाण(संरचना ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    संरचना ohci_td *tds = (शून्य*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, परिमाण(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    जइ (cmd) {
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
    जाव (dest < dataend) {
        // Send data pids
        जइ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            प्रत्यागम -1;
        }
        पूर्णाङ्क maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        पूर्णाङ्क transfer = dataend - dest;
        जइ (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    जइ (cmd) {
        // Send status pid on control transfers
        जइ (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            प्रत्यागम -1;
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

    पूर्णाङ्क ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    जइ (ret)
        ohci_waittick(pipe->regs);
    प्रत्यागम ret;
}

पूर्णाङ्क
ohci_poll_intr(संरचना usb_pipe *p, शून्य *data)
{
    ASSERT16();
    जइ (! CONFIG_USB_OHCI)
        प्रत्यागम -1;

    संरचना ohci_pipe *pipe = container_of(p, संरचना ohci_pipe, pipe);
    संरचना ohci_td *tds = GET_LOWFLAT(pipe->tds);
    संरचना ohci_td *head = (शून्य*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    संरचना ohci_td *tail = (शून्य*)GET_LOWFLAT(pipe->ed.hwTailP);
    पूर्णाङ्क गणनम् = GET_LOWFLAT(pipe->गणनम्);
    पूर्णाङ्क pos = (tail - tds + 1) % गणनम्;
    संरचना ohci_td *next = &tds[pos];
    जइ (head == next)
        // No intrs found.
        प्रत्यागम -1;
    // XXX - check for errors.

    // Copy data.
    पूर्णाङ्क maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    शून्य *pipedata = GET_LOWFLAT((pipe->data));
    शून्य *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_नीचः, LOWFLAT2_नीचः(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    प्रत्यागम 0;
}
