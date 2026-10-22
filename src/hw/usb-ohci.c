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

#تعریف_کرو FIT                     (1 << 31)

ساخت usb_ohci_s {
    ساخت usb_s usb;
    ساخت ohci_regs *regs;
};

ساخت ohci_pipe {
    ساخت ohci_ed ed;
    ساخت usb_pipe pipe;
    ساخت ohci_regs *regs;
    خالی *ڈیٹا;
    صحیح گنتی;
    ساخت ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
ساکن صحیح
ohci_hub_detect(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_ohci_s *cntl = container_کا(hub->cntl, ساخت usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[پورٹ]);
    واپس (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
ساکن خالی
ohci_hub_disconnect(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_ohci_s *cntl = container_کا(hub->cntl, ساخت usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[پورٹ], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
ساکن صحیح
ohci_hub_reset(ساخت usbhub_s *hub, u32 پورٹ)
{
    ساخت usb_ohci_s *cntl = container_کا(hub->cntl, ساخت usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[پورٹ], RH_PS_PRS);
    u32 sts;
    u32 انتہا = timer_calc(USB_وقت_DRSTR * 2);
    برائے (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[پورٹ]);
        اگر (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            توڑو;
        اگر (timer_check(انتہا)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, پورٹ);
            واپس -1;
        }
        yield();
    }

    اگر ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        واپس -1;

    واپس !!(sts & RH_PS_LSDA);
}

ساکن ساخت usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساکن صحیح
check_ohci_ports(ساخت usb_ohci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();
    // Turn on power for all devices on roothub.
    u32 rha = readl(&cntl->regs->roothub_a);
    rha &= ~(RH_A_PSM | RH_A_OCPM);
    writel(&cntl->regs->roothub_حالت, RH_HS_LPSC);
    writel(&cntl->regs->roothub_b, RH_B_PPCM);
    msleep((rha >> 24) * 2);
    // XXX - need to sleep for USB_TIME_SIGATT if just powered up?

    ساخت usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    واپس hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ساکن خالی
ohci_waittick(ساخت ohci_regs *regs)
{
    barrier();
    ساخت ohci_hcca *hcca = (خالی*)regs->hcca;
    u32 startframe = hcca->چوکھٹا_no;
    u32 انتہا = timer_calc(1000 * 5);
    برائے (;;) {
        اگر (hcca->چوکھٹا_no != startframe)
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس;
        }
        yield();
    }
}

ساکن خالی
ohci_free_pipes(ساخت usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    اگر (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    برائے (;;) {
        ساخت ohci_ed *اگلا = (خالی*)*pos;
        اگر (!اگلا)
            توڑو;
        ساخت ohci_pipe *pipe = container_کا(اگلا, ساخت ohci_pipe, ed);
        اگر (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = اگلا->hw_اگلا_ED;
            آزاد(pipe);
        } ورنہ {
            pos = &اگلا->hw_اگلا_ED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = تہی;
}

ساکن صحیح
آغاز_ohci(ساخت usb_ohci_s *cntl, ساخت ohci_hcca *hcca)
{
    u32 oldfminterval = readl(&cntl->regs->fminterval);
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;

    // XXX - check if already running?

    // Do reset
    writel(&cntl->regs->control, OHCI_USB_RESET | oldrwc);
    readl(&cntl->regs->control); // flush writes
    msleep(USB_وقت_DRSTR);

    // Do software init (min 10us, max 2ms)
    u32 انتہا = timer_calc_usec(10);
    writel(&cntl->regs->cmdstatus, OHCI_HCR);
    برائے (;;) {
        u32 حالت_2 = readl(&cntl->regs->cmdstatus);
        اگر (!(حالت_2 & OHCI_HCR))
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس -1;
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

    واپس 0;
}

ساکن خالی
stop_ohci(ساخت usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

ساکن خالی
configure_ohci(خالی *ڈیٹا)
{
    ساخت usb_ohci_s *cntl = ڈیٹا;

    // Allocate memory
    ساخت ohci_hcca *hcca = memalign_بلند(256, حجم(*hcca));
    ساخت ohci_ed *intr_ed = malloc_بلند(حجم(*intr_ed));
    اگر (!hcca || !intr_ed) {
        warn_noalloc();
        جاؤ آزاد;
    }
    memset(hcca, 0, حجم(*hcca));
    memset(intr_ed, 0, حجم(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    صحیح i;
    برائے (i=0; i<ARRAY_سائز(hcca->int_جدول); i++)
        hcca->int_جدول[i] = (u32)intr_ed;

    صحیح ret = آغاز_ohci(cntl, hcca);
    اگر (ret)
        جاؤ err;

    صحیح گنتی = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    اگر (! گنتی)
        جاؤ err;
    واپس;

err:
    stop_ohci(cntl);
آزاد:
    آزاد(hcca);
    آزاد(intr_ed);
}

ساکن خالی
ohci_controller_تیاری(ساخت pci_device *pci)
{
    ساخت ohci_regs *regs = pci_enable_membar(pci, PCI_بنیاد_پتہ_0);
    اگر (!regs)
        واپس;

    ساخت usb_ohci_s *cntl = malloc_tmphigh(حجم(*cntl));
    اگر (!cntl) {
        warn_noalloc();
        واپس;
    }
    memset(cntl, 0, حجم(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_OHCI;
    cntl->regs = regs;

    dprintf(1, "OHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    // Disable interrupts
    writel(&cntl->regs->intrdisable, ~0);
    writel(&cntl->regs->intrstatus, ~0);

    تسلسل_thread(configure_ohci, cntl);
}

خالی
ohci_تیاری(خالی)
{
    اگر (! CONFIG_USB_OHCI)
        واپس;
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_تیاری(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
ساکن خالی
ohci_desc2pipe(ساخت ohci_pipe *pipe, ساخت usbdevice_s *usbdev
               , ساخت usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->w_Max_Packet_سائز << 16)
                       | (usbdev->رفتار ? ED_LOWSPEED : 0));
    ساخت usb_ohci_s *cntl = container_کا(
        usbdev->hub->cntl, ساخت usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

ساکن ساخت usb_pipe *
ohci_alloc_intr_pipe(ساخت usbdevice_s *usbdev
                     , ساخت usb_endpoint_descriptor *epdesc)
{
    ساخت usb_ohci_s *cntl = container_کا(
        usbdev->hub->cntl, ساخت usb_ohci_s, usb);
    صحیح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    اگر (frameexp > 5)
        frameexp = 5;
    صحیح maxpacket = epdesc->w_Max_Packet_سائز;
    // Determine number of entries needed for 2 timer ticks.
    صحیح ms = 1<<frameexp;
    صحیح گنتی = DIV_باری_اوپر(ticks_مقصد_ms(2), ms) + 1;
    ساخت ohci_pipe *pipe = malloc_کم(حجم(*pipe));
    ساخت ohci_td *tds = malloc_کم(حجم(*tds) * گنتی);
    خالی *ڈیٹا = malloc_کم(maxpacket * گنتی);
    اگر (!pipe || !tds || !ڈیٹا)
        جاؤ err;
    memset(pipe, 0, حجم(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->ڈیٹا = ڈیٹا;
    pipe->گنتی = گنتی;
    pipe->tds = tds;

    ساخت ohci_ed *ed = &pipe->ed;
    ed->hw_سر_P = (u32)&tds[0];
    ed->hw_دم_P = (u32)&tds[گنتی-1];

    صحیح i;
    برائے (i=0; i<گنتی-1; i++) {
        tds[i].hwINFO = TD_DP_اندر | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)ڈیٹا + maxpacket * i;
        tds[i].hw_اگلا_TD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    ساخت ohci_hcca *hcca = (خالی*)cntl->regs->hcca;
    اگر (frameexp == 0) {
        // Add to existing interrupt entry.
        ساخت ohci_ed *intr_ed = (خالی*)hcca->int_جدول[0];
        ed->hw_اگلا_ED = intr_ed->hw_اگلا_ED;
        barrier();
        intr_ed->hw_اگلا_ED = (u32)ed;
    } ورنہ {
        صحیح startpos = 1<<(frameexp-1);
        ed->hw_اگلا_ED = hcca->int_جدول[startpos];
        barrier();
        برائے (i=startpos; i<ARRAY_سائز(hcca->int_جدول); i+=ms)
            hcca->int_جدول[i] = (u32)ed;
    }

    واپس &pipe->pipe;

err:
    آزاد(pipe);
    آزاد(tds);
    آزاد(ڈیٹا);
    واپس تہی;
}

ساخت usb_pipe *
ohci_realloc_pipe(ساخت usbdevice_s *usbdev, ساخت usb_pipe *upipe
                  , ساخت usb_endpoint_descriptor *epdesc)
{
    اگر (! CONFIG_USB_OHCI)
        واپس تہی;
    usb_جوڑو_freelist(upipe);
    اگر (!epdesc)
        واپس تہی;
    u8 eptype = epdesc->bm_اوصاف & USB_ENDPOINT_XFERTYPE_نقاب;
    اگر (eptype == USB_ENDPOINT_XFER_INT)
        واپس ohci_alloc_intr_pipe(usbdev, epdesc);
    ساخت usb_ohci_s *cntl = container_کا(
        usbdev->hub->cntl, ساخت usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    ساخت usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    اگر (usbpipe) {
        // Use previously allocated pipe.
        ساخت ohci_pipe *pipe = container_کا(usbpipe, ساخت ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        واپس usbpipe;
    }

    // Allocate a new queue head.
    ساخت ohci_pipe *pipe;
    اگر (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(حجم(*pipe));
    ورنہ
        pipe = malloc_کم(حجم(*pipe));
    اگر (!pipe) {
        warn_noalloc();
        واپس تہی;
    }
    memset(pipe, 0, حجم(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *سر = &cntl->regs->ed_controlhead;
    اگر (eptype != USB_ENDPOINT_XFER_CONTROL)
        سر = &cntl->regs->ed_bulkhead;
    pipe->ed.hw_اگلا_ED = *سر;
    barrier();
    *سر = (u32)&pipe->ed;
    واپس &pipe->pipe;
}

ساکن صحیح
wait_ed(ساخت ohci_ed *ed, صحیح timeout)
{
    u32 انتہا = timer_calc(timeout);
    برائے (;;) {
        اگر ((ed->hw_سر_P & ~(ED_C|ED_H)) == ed->hw_دم_P)
            واپس 0;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hw_دم_P, ed->hw_سر_P, ed->hw_اگلا_ED);
            واپس -1;
        }
        yield();
    }
}

#تعریف_کرو STACKOTDS 18
#تعریف_کرو OHCI_TD_ALIGN 16

صحیح
ohci_send_pipe(ساخت usb_pipe *p, صحیح dir, ثابت خالی *cmd
               , خالی *ڈیٹا, صحیح datasize)
{
    ASSERT32FLAT();
    اگر (! CONFIG_USB_OHCI)
        واپس -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    ساخت ohci_pipe *pipe = container_کا(p, ساخت ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[حجم(ساخت ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    ساخت ohci_td *tds = (خالی*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, حجم(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    اگر (cmd) {
        // Send setup pid on control transfers
        td->hwINFO = TD_DP_تیاری | TD_T_DATA0 | TD_CC;
        td->hwCBP = (u32)cmd;
        td->hw_اگلا_TD = (u32)&td[1];
        td->hwBE = (u32)cmd + USB_CONTROL_تیاری_سائز - 1;
        td++;
        toggle = TD_T_DATA1;
        statuscmd = OHCI_CLF;
    }
    u32 dest = (u32)ڈیٹا, dataend = dest + datasize;
    جب (dest < dataend) {
        // Send data pids
        اگر (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            واپس -1;
        }
        صحیح maxtransfer = 2*صفحہ_سائز - (dest & (صفحہ_سائز-1));
        صحیح transfer = dataend - dest;
        اگر (transfer > maxtransfer)
            transfer = ALIGN_نیچے_کو(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_اندر : TD_DP_برآمد) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hw_اگلا_TD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    اگر (cmd) {
        // Send status pid on control transfers
        اگر (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            واپس -1;
        }
        td->hwINFO = (dir ? TD_DP_برآمد : TD_DP_اندر) | TD_T_DATA1 | TD_CC;
        td->hwCBP = 0;
        td->hw_اگلا_TD = (u32)&td[1];
        td->hwBE = 0;
        td++;
    }

    // Transfer data
    pipe->ed.hw_سر_P = (u32)tds | (pipe->ed.hw_سر_P & ED_C);
    pipe->ed.hw_دم_P = (u32)td;
    barrier();
    pipe->ed.hwINFO &= ~ED_SKIP;
    writel(&pipe->regs->cmdstatus, statuscmd);

    صحیح ret = wait_ed(&pipe->ed, usb_xfer_وقت(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    اگر (ret)
        ohci_waittick(pipe->regs);
    واپس ret;
}

صحیح
ohci_poll_intr(ساخت usb_pipe *p, خالی *ڈیٹا)
{
    ASSERT16();
    اگر (! CONFIG_USB_OHCI)
        واپس -1;

    ساخت ohci_pipe *pipe = container_کا(p, ساخت ohci_pipe, pipe);
    ساخت ohci_td *tds = GET_LOWFLAT(pipe->tds);
    ساخت ohci_td *سر = (خالی*)(GET_LOWFLAT(pipe->ed.hw_سر_P) & ~(ED_C|ED_H));
    ساخت ohci_td *دم = (خالی*)GET_LOWFLAT(pipe->ed.hw_دم_P);
    صحیح گنتی = GET_LOWFLAT(pipe->گنتی);
    صحیح pos = (دم - tds + 1) % گنتی;
    ساخت ohci_td *اگلا = &tds[pos];
    اگر (سر == اگلا)
        // No intrs found.
        واپس -1;
    // XXX - check for errors.

    // Copy data.
    صحیح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    خالی *pipedata = GET_LOWFLAT((pipe->ڈیٹا));
    خالی *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), ڈیٹا, SEG_کم, LOWFLAT2_کم(intrdata), maxpacket);

    // Reenable this td.
    سیٹ_LOWFLAT(دم->hwINFO, TD_DP_اندر | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (دم-tds);
    سیٹ_LOWFLAT(دم->hwCBP, (u32)intrdata);
    سیٹ_LOWFLAT(دم->hw_اگلا_TD, (u32)اگلا);
    سیٹ_LOWFLAT(دم->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    سیٹ_LOWFLAT(pipe->ed.hw_دم_P, (u32)اگلا);

    واپس 0;
}
