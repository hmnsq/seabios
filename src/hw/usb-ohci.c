// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_LOWFLAT
#समावेश "config.h" // CONFIG_*
#समावेश "malloc.h" // free
#समावेश "memmap.h" // PAGE_SIZE
#समावेश "output.h" // dprintf
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#समावेश "pci_regs.h" // PCI_BASE_ADDRESS_0
#समावेश "string.h" // memset
#समावेश "usb.h" // struct usb_s
#समावेश "usb-ehci.h" // ehci_wait_controllers
#समावेश "usb-ohci.h" // struct ohci_hcca
#समावेश "util.h" // msleep
#समावेश "x86.h" // readl

#घोषणा FIT                     (1 << 31)

काष्ठा usb_ohci_s {
    काष्ठा usb_s usb;
    काष्ठा ohci_regs *regs;
};

काष्ठा ohci_pipe {
    काष्ठा ohci_ed ed;
    काष्ठा usb_pipe pipe;
    काष्ठा ohci_regs *regs;
    व्योम *तथ्य;
    पूर्णांक गिनती;
    काष्ठा ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
अटल पूर्णांक
ohci_hub_detect(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_ohci_s *cntl = container_of(hub->cntl, काष्ठा usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    वापस (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
अटल व्योम
ohci_hub_disconnect(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_ohci_s *cntl = container_of(hub->cntl, काष्ठा usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
अटल पूर्णांक
ohci_hub_reset(काष्ठा usbhub_s *hub, u32 port)
{
    काष्ठा usb_ohci_s *cntl = container_of(hub->cntl, काष्ठा usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    क्रम (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        अगर (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            अवरोध;
        अगर (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            वापस -1;
        }
        yield();
    }

    अगर ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        वापस -1;

    वापस !!(sts & RH_PS_LSDA);
}

अटल काष्ठा usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
अटल पूर्णांक
check_ohci_ports(काष्ठा usb_ohci_s *cntl)
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

    काष्ठा usbhub_s hub;
    स_रखो(&hub, 0, माप(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    वापस hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
अटल व्योम
ohci_waittick(काष्ठा ohci_regs *regs)
{
    barrier();
    काष्ठा ohci_hcca *hcca = (व्योम*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    क्रम (;;) {
        अगर (hcca->frame_no != startframe)
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            वापस;
        }
        yield();
    }
}

अटल व्योम
ohci_free_pipes(काष्ठा usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    अगर (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    क्रम (;;) {
        काष्ठा ohci_ed *अग्र = (व्योम*)*pos;
        अगर (!अग्र)
            अवरोध;
        काष्ठा ohci_pipe *pipe = container_of(अग्र, काष्ठा ohci_pipe, ed);
        अगर (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = अग्र->hw_अग्र_ED;
            मुक्त(pipe);
        } अन्यथा {
            pos = &अग्र->hw_अग्र_ED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = शून्य;
}

अटल पूर्णांक
start_ohci(काष्ठा usb_ohci_s *cntl, काष्ठा ohci_hcca *hcca)
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
    क्रम (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        अगर (!(status & OHCI_HCR))
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
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

    वापस 0;
}

अटल व्योम
stop_ohci(काष्ठा usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

अटल व्योम
configure_ohci(व्योम *तथ्य)
{
    काष्ठा usb_ohci_s *cntl = तथ्य;

    // Allocate memory
    काष्ठा ohci_hcca *hcca = memalign_ऊपरी(256, माप(*hcca));
    काष्ठा ohci_ed *intr_ed = malloc_ऊपरी(माप(*intr_ed));
    अगर (!hcca || !intr_ed) {
        warn_noalloc();
        जाओ मुक्त;
    }
    स_रखो(hcca, 0, माप(*hcca));
    स_रखो(intr_ed, 0, माप(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    पूर्णांक i;
    क्रम (i=0; i<ARRAY_आकार(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    पूर्णांक ret = start_ohci(cntl, hcca);
    अगर (ret)
        जाओ err;

    पूर्णांक गिनती = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    अगर (! गिनती)
        जाओ err;
    वापस;

err:
    stop_ohci(cntl);
मुक्त:
    मुक्त(hcca);
    मुक्त(intr_ed);
}

अटल व्योम
ohci_controller_setup(काष्ठा pci_device *pci)
{
    काष्ठा ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    अगर (!regs)
        वापस;

    काष्ठा usb_ohci_s *cntl = malloc_tmphigh(माप(*cntl));
    अगर (!cntl) {
        warn_noalloc();
        वापस;
    }
    स_रखो(cntl, 0, माप(*cntl));
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

व्योम
ohci_setup(व्योम)
{
    अगर (! CONFIG_USB_OHCI)
        वापस;
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
अटल व्योम
ohci_desc2pipe(काष्ठा ohci_pipe *pipe, काष्ठा usbdevice_s *usbdev
               , काष्ठा usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->w_Max_Packet_आकार << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    काष्ठा usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, काष्ठा usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

अटल काष्ठा usb_pipe *
ohci_alloc_intr_pipe(काष्ठा usbdevice_s *usbdev
                     , काष्ठा usb_endpoint_descriptor *epdesc)
{
    काष्ठा usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, काष्ठा usb_ohci_s, usb);
    पूर्णांक frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    अगर (frameexp > 5)
        frameexp = 5;
    पूर्णांक maxpacket = epdesc->w_Max_Packet_आकार;
    // Determine number of entries needed for 2 timer ticks.
    पूर्णांक ms = 1<<frameexp;
    पूर्णांक गिनती = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    काष्ठा ohci_pipe *pipe = malloc_निचला(माप(*pipe));
    काष्ठा ohci_td *tds = malloc_निचला(माप(*tds) * गिनती);
    व्योम *तथ्य = malloc_निचला(maxpacket * गिनती);
    अगर (!pipe || !tds || !तथ्य)
        जाओ err;
    स_रखो(pipe, 0, माप(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->तथ्य = तथ्य;
    pipe->गिनती = गिनती;
    pipe->tds = tds;

    काष्ठा ohci_ed *ed = &pipe->ed;
    ed->hw_शीर्ष_P = (u32)&tds[0];
    ed->hw_पुच्छ_P = (u32)&tds[गिनती-1];

    पूर्णांक i;
    क्रम (i=0; i<गिनती-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)तथ्य + maxpacket * i;
        tds[i].hw_अग्र_TD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    काष्ठा ohci_hcca *hcca = (व्योम*)cntl->regs->hcca;
    अगर (frameexp == 0) {
        // Add to existing interrupt entry.
        काष्ठा ohci_ed *intr_ed = (व्योम*)hcca->int_table[0];
        ed->hw_अग्र_ED = intr_ed->hw_अग्र_ED;
        barrier();
        intr_ed->hw_अग्र_ED = (u32)ed;
    } अन्यथा {
        पूर्णांक startpos = 1<<(frameexp-1);
        ed->hw_अग्र_ED = hcca->int_table[startpos];
        barrier();
        क्रम (i=startpos; i<ARRAY_आकार(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    वापस &pipe->pipe;

err:
    मुक्त(pipe);
    मुक्त(tds);
    मुक्त(तथ्य);
    वापस शून्य;
}

काष्ठा usb_pipe *
ohci_realloc_pipe(काष्ठा usbdevice_s *usbdev, काष्ठा usb_pipe *upipe
                  , काष्ठा usb_endpoint_descriptor *epdesc)
{
    अगर (! CONFIG_USB_OHCI)
        वापस शून्य;
    usb_add_freelist(upipe);
    अगर (!epdesc)
        वापस शून्य;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    अगर (eptype == USB_ENDPOINT_XFER_INT)
        वापस ohci_alloc_intr_pipe(usbdev, epdesc);
    काष्ठा usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, काष्ठा usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    काष्ठा usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    अगर (usbpipe) {
        // Use previously allocated pipe.
        काष्ठा ohci_pipe *pipe = container_of(usbpipe, काष्ठा ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        वापस usbpipe;
    }

    // Allocate a new queue head.
    काष्ठा ohci_pipe *pipe;
    अगर (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(माप(*pipe));
    अन्यथा
        pipe = malloc_निचला(माप(*pipe));
    अगर (!pipe) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(pipe, 0, माप(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *शीर्ष = &cntl->regs->ed_controlhead;
    अगर (eptype != USB_ENDPOINT_XFER_CONTROL)
        शीर्ष = &cntl->regs->ed_bulkhead;
    pipe->ed.hw_अग्र_ED = *शीर्ष;
    barrier();
    *शीर्ष = (u32)&pipe->ed;
    वापस &pipe->pipe;
}

अटल पूर्णांक
wait_ed(काष्ठा ohci_ed *ed, पूर्णांक timeout)
{
    u32 end = timer_calc(timeout);
    क्रम (;;) {
        अगर ((ed->hw_शीर्ष_P & ~(ED_C|ED_H)) == ed->hw_पुच्छ_P)
            वापस 0;
        अगर (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hw_पुच्छ_P, ed->hw_शीर्ष_P, ed->hw_अग्र_ED);
            वापस -1;
        }
        yield();
    }
}

#घोषणा STACKOTDS 18
#घोषणा OHCI_TD_ALIGN 16

पूर्णांक
ohci_send_pipe(काष्ठा usb_pipe *p, पूर्णांक dir, स्थिर व्योम *cmd
               , व्योम *तथ्य, पूर्णांक datasize)
{
    ASSERT32FLAT();
    अगर (! CONFIG_USB_OHCI)
        वापस -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    काष्ठा ohci_pipe *pipe = container_of(p, काष्ठा ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[माप(काष्ठा ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    काष्ठा ohci_td *tds = (व्योम*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    स_रखो(tds, 0, माप(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    अगर (cmd) {
        // Send setup pid on control transfers
        td->hwINFO = TD_DP_SETUP | TD_T_DATA0 | TD_CC;
        td->hwCBP = (u32)cmd;
        td->hw_अग्र_TD = (u32)&td[1];
        td->hwBE = (u32)cmd + USB_CONTROL_SETUP_आकार - 1;
        td++;
        toggle = TD_T_DATA1;
        statuscmd = OHCI_CLF;
    }
    u32 dest = (u32)तथ्य, dataend = dest + datasize;
    जबतक (dest < dataend) {
        // Send data pids
        अगर (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            वापस -1;
        }
        पूर्णांक maxtransfer = 2*PAGE_आकार - (dest & (PAGE_आकार-1));
        पूर्णांक transfer = dataend - dest;
        अगर (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hw_अग्र_TD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    अगर (cmd) {
        // Send status pid on control transfers
        अगर (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            वापस -1;
        }
        td->hwINFO = (dir ? TD_DP_OUT : TD_DP_IN) | TD_T_DATA1 | TD_CC;
        td->hwCBP = 0;
        td->hw_अग्र_TD = (u32)&td[1];
        td->hwBE = 0;
        td++;
    }

    // Transfer data
    pipe->ed.hw_शीर्ष_P = (u32)tds | (pipe->ed.hw_शीर्ष_P & ED_C);
    pipe->ed.hw_पुच्छ_P = (u32)td;
    barrier();
    pipe->ed.hwINFO &= ~ED_SKIP;
    writel(&pipe->regs->cmdstatus, statuscmd);

    पूर्णांक ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    अगर (ret)
        ohci_waittick(pipe->regs);
    वापस ret;
}

पूर्णांक
ohci_poll_intr(काष्ठा usb_pipe *p, व्योम *तथ्य)
{
    ASSERT16();
    अगर (! CONFIG_USB_OHCI)
        वापस -1;

    काष्ठा ohci_pipe *pipe = container_of(p, काष्ठा ohci_pipe, pipe);
    काष्ठा ohci_td *tds = GET_LOWFLAT(pipe->tds);
    काष्ठा ohci_td *शीर्ष = (व्योम*)(GET_LOWFLAT(pipe->ed.hw_शीर्ष_P) & ~(ED_C|ED_H));
    काष्ठा ohci_td *पुच्छ = (व्योम*)GET_LOWFLAT(pipe->ed.hw_पुच्छ_P);
    पूर्णांक गिनती = GET_LOWFLAT(pipe->गिनती);
    पूर्णांक pos = (पुच्छ - tds + 1) % गिनती;
    काष्ठा ohci_td *अग्र = &tds[pos];
    अगर (शीर्ष == अग्र)
        // No intrs found.
        वापस -1;
    // XXX - check for errors.

    // Copy data.
    पूर्णांक maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    व्योम *pipedata = GET_LOWFLAT((pipe->तथ्य));
    व्योम *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), तथ्य, SEG_निचला, LOWFLAT2_निचला(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(पुच्छ->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (पुच्छ-tds);
    SET_LOWFLAT(पुच्छ->hwCBP, (u32)intrdata);
    SET_LOWFLAT(पुच्छ->hw_अग्र_TD, (u32)अग्र);
    SET_LOWFLAT(पुच्छ->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hw_पुच्छ_P, (u32)अग्र);

    वापस 0;
}
