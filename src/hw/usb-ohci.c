// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_LOWFLAT
#ضمن "config.h" // CONFIG_*
#ضمن "malloc.h" // free
#ضمن "memmap.h" // PAGE_SIZE
#ضمن "output.h" // dprintf
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#ضمن "pci_regs.h" // PCI_BASE_ADDRESS_0
#ضمن "string.h" // memset
#ضمن "usb.h" // struct usb_s
#ضمن "usb-ehci.h" // ehci_wait_controllers
#ضمن "usb-ohci.h" // struct ohci_hcca
#ضمن "util.h" // msleep
#ضمن "x86.h" // readl

#تعریف_کرو FIT                     (1 << 31)

بنية usb_ohci_s {
    بنية usb_s usb;
    بنية ohci_regs *regs;
};

بنية ohci_pipe {
    بنية ohci_ed ed;
    بنية usb_pipe pipe;
    بنية ohci_regs *regs;
    فراغ *انکڑے;
    صحيح گنتی;
    بنية ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
ساكن صحيح
ohci_hub_detect(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_ohci_s *cntl = container_دا(hub->cntl, بنية usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[پورٹ]);
    أرجع (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
ساكن فراغ
ohci_hub_disconnect(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_ohci_s *cntl = container_دا(hub->cntl, بنية usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[پورٹ], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
ساكن صحيح
ohci_hub_reset(بنية usbhub_s *hub, u32 پورٹ)
{
    بنية usb_ohci_s *cntl = container_دا(hub->cntl, بنية usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[پورٹ], RH_PS_PRS);
    u32 sts;
    u32 سرا = timer_calc(USB_ویلا_DRSTR * 2);
    لكل (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[پورٹ]);
        إذا (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            اكسر;
        إذا (timer_check(سرا)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, پورٹ);
            أرجع -1;
        }
        yield();
    }

    إذا ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        أرجع -1;

    أرجع !!(sts & RH_PS_LSDA);
}

ساكن بنية usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
ساكن صحيح
check_ohci_ports(بنية usb_ohci_s *cntl)
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

    بنية usbhub_s hub;
    memset(&hub, 0, حجم(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    أرجع hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
ساكن فراغ
ohci_waittick(بنية ohci_regs *regs)
{
    barrier();
    بنية ohci_hcca *hcca = (فراغ*)regs->hcca;
    u32 startframe = hcca->چوکھٹا_no;
    u32 سرا = timer_calc(1000 * 5);
    لكل (;;) {
        إذا (hcca->چوکھٹا_no != startframe)
            اكسر;
        إذا (timer_check(سرا)) {
            warn_timeout();
            أرجع;
        }
        yield();
    }
}

ساكن فراغ
ohci_free_pipes(بنية usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    إذا (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    لكل (;;) {
        بنية ohci_ed *اگلا = (فراغ*)*pos;
        إذا (!اگلا)
            اكسر;
        بنية ohci_pipe *pipe = container_دا(اگلا, بنية ohci_pipe, ed);
        إذا (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = اگلا->hw_اگلا_ED;
            حرر(pipe);
        } وإلا {
            pos = &اگلا->hw_اگلا_ED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = عدم;
}

ساكن صحيح
شروع_ohci(بنية usb_ohci_s *cntl, بنية ohci_hcca *hcca)
{
    u32 oldfminterval = readl(&cntl->regs->fminterval);
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;

    // XXX - check if already running?

    // Do reset
    writel(&cntl->regs->control, OHCI_USB_RESET | oldrwc);
    readl(&cntl->regs->control); // flush writes
    msleep(USB_ویلا_DRSTR);

    // Do software init (min 10us, max 2ms)
    u32 سرا = timer_calc_usec(10);
    writel(&cntl->regs->cmdstatus, OHCI_HCR);
    لكل (;;) {
        u32 حالت = readl(&cntl->regs->cmdstatus);
        إذا (!(حالت & OHCI_HCR))
            اكسر;
        إذا (timer_check(سرا)) {
            warn_timeout();
            أرجع -1;
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

    أرجع 0;
}

ساكن فراغ
stop_ohci(بنية usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

ساكن فراغ
configure_ohci(فراغ *انکڑے)
{
    بنية usb_ohci_s *cntl = انکڑے;

    // Allocate memory
    بنية ohci_hcca *hcca = memalign_بلند(256, حجم(*hcca));
    بنية ohci_ed *intr_ed = malloc_بلند(حجم(*intr_ed));
    إذا (!hcca || !intr_ed) {
        warn_noalloc();
        اذهب حرر;
    }
    memset(hcca, 0, حجم(*hcca));
    memset(intr_ed, 0, حجم(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    صحيح i;
    لكل (i=0; i<ARRAY_ناپ(hcca->int_جدول); i++)
        hcca->int_جدول[i] = (u32)intr_ed;

    صحيح ret = شروع_ohci(cntl, hcca);
    إذا (ret)
        اذهب err;

    صحيح گنتی = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    إذا (! گنتی)
        اذهب err;
    أرجع;

err:
    stop_ohci(cntl);
حرر:
    حرر(hcca);
    حرر(intr_ed);
}

ساكن فراغ
ohci_controller_تیاری(بنية pci_device *pci)
{
    بنية ohci_regs *regs = pci_enable_membar(pci, PCI_ادھار_پتہ_0);
    إذا (!regs)
        أرجع;

    بنية usb_ohci_s *cntl = malloc_tmphigh(حجم(*cntl));
    إذا (!cntl) {
        warn_noalloc();
        أرجع;
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

    لگاتار_thread(configure_ohci, cntl);
}

فراغ
ohci_تیاری(فراغ)
{
    إذا (! CONFIG_USB_OHCI)
        أرجع;
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_تیاری(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
ساكن فراغ
ohci_desc2pipe(بنية ohci_pipe *pipe, بنية usbdevice_s *usbdev
               , بنية usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->w_Max_Packet_ناپ << 16)
                       | (usbdev->رفتار ? ED_LOWSPEED : 0));
    بنية usb_ohci_s *cntl = container_دا(
        usbdev->hub->cntl, بنية usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

ساكن بنية usb_pipe *
ohci_alloc_intr_pipe(بنية usbdevice_s *usbdev
                     , بنية usb_endpoint_descriptor *epdesc)
{
    بنية usb_ohci_s *cntl = container_دا(
        usbdev->hub->cntl, بنية usb_ohci_s, usb);
    صحيح frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    إذا (frameexp > 5)
        frameexp = 5;
    صحيح maxpacket = epdesc->w_Max_Packet_ناپ;
    // Determine number of entries needed for 2 timer ticks.
    صحيح ms = 1<<frameexp;
    صحيح گنتی = DIV_گیڑا_اتے(ticks_منزل_ms(2), ms) + 1;
    بنية ohci_pipe *pipe = malloc_کم(حجم(*pipe));
    بنية ohci_td *tds = malloc_کم(حجم(*tds) * گنتی);
    فراغ *انکڑے = malloc_کم(maxpacket * گنتی);
    إذا (!pipe || !tds || !انکڑے)
        اذهب err;
    memset(pipe, 0, حجم(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->انکڑے = انکڑے;
    pipe->گنتی = گنتی;
    pipe->tds = tds;

    بنية ohci_ed *ed = &pipe->ed;
    ed->hw_سر_P = (u32)&tds[0];
    ed->hw_پوچھ_P = (u32)&tds[گنتی-1];

    صحيح i;
    لكل (i=0; i<گنتی-1; i++) {
        tds[i].hwINFO = TD_DP_وچ | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)انکڑے + maxpacket * i;
        tds[i].hw_اگلا_TD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    بنية ohci_hcca *hcca = (فراغ*)cntl->regs->hcca;
    إذا (frameexp == 0) {
        // Add to existing interrupt entry.
        بنية ohci_ed *intr_ed = (فراغ*)hcca->int_جدول[0];
        ed->hw_اگلا_ED = intr_ed->hw_اگلا_ED;
        barrier();
        intr_ed->hw_اگلا_ED = (u32)ed;
    } وإلا {
        صحيح startpos = 1<<(frameexp-1);
        ed->hw_اگلا_ED = hcca->int_جدول[startpos];
        barrier();
        لكل (i=startpos; i<ARRAY_ناپ(hcca->int_جدول); i+=ms)
            hcca->int_جدول[i] = (u32)ed;
    }

    أرجع &pipe->pipe;

err:
    حرر(pipe);
    حرر(tds);
    حرر(انکڑے);
    أرجع عدم;
}

بنية usb_pipe *
ohci_realloc_pipe(بنية usbdevice_s *usbdev, بنية usb_pipe *upipe
                  , بنية usb_endpoint_descriptor *epdesc)
{
    إذا (! CONFIG_USB_OHCI)
        أرجع عدم;
    usb_جوڑو_freelist(upipe);
    إذا (!epdesc)
        أرجع عدم;
    u8 eptype = epdesc->bm_گن & USB_ENDPOINT_XFERTYPE_نقاب;
    إذا (eptype == USB_ENDPOINT_XFER_INT)
        أرجع ohci_alloc_intr_pipe(usbdev, epdesc);
    بنية usb_ohci_s *cntl = container_دا(
        usbdev->hub->cntl, بنية usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    بنية usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    إذا (usbpipe) {
        // Use previously allocated pipe.
        بنية ohci_pipe *pipe = container_دا(usbpipe, بنية ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        أرجع usbpipe;
    }

    // Allocate a new queue head.
    بنية ohci_pipe *pipe;
    إذا (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(حجم(*pipe));
    وإلا
        pipe = malloc_کم(حجم(*pipe));
    إذا (!pipe) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(pipe, 0, حجم(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *سر = &cntl->regs->ed_controlhead;
    إذا (eptype != USB_ENDPOINT_XFER_CONTROL)
        سر = &cntl->regs->ed_bulkhead;
    pipe->ed.hw_اگلا_ED = *سر;
    barrier();
    *سر = (u32)&pipe->ed;
    أرجع &pipe->pipe;
}

ساكن صحيح
wait_ed(بنية ohci_ed *ed, صحيح timeout)
{
    u32 سرا = timer_calc(timeout);
    لكل (;;) {
        إذا ((ed->hw_سر_P & ~(ED_C|ED_H)) == ed->hw_پوچھ_P)
            أرجع 0;
        إذا (timer_check(سرا)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hw_پوچھ_P, ed->hw_سر_P, ed->hw_اگلا_ED);
            أرجع -1;
        }
        yield();
    }
}

#تعریف_کرو STACKOTDS 18
#تعریف_کرو OHCI_TD_ALIGN 16

صحيح
ohci_send_pipe(بنية usb_pipe *p, صحيح dir, ثابت فراغ *cmd
               , فراغ *انکڑے, صحيح datasize)
{
    ASSERT32FLAT();
    إذا (! CONFIG_USB_OHCI)
        أرجع -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    بنية ohci_pipe *pipe = container_دا(p, بنية ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[حجم(بنية ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    بنية ohci_td *tds = (فراغ*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, حجم(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    إذا (cmd) {
        // Send setup pid on control transfers
        td->hwINFO = TD_DP_تیاری | TD_T_DATA0 | TD_CC;
        td->hwCBP = (u32)cmd;
        td->hw_اگلا_TD = (u32)&td[1];
        td->hwBE = (u32)cmd + USB_CONTROL_تیاری_ناپ - 1;
        td++;
        toggle = TD_T_DATA1;
        statuscmd = OHCI_CLF;
    }
    u32 dest = (u32)انکڑے, dataend = dest + datasize;
    طالما (dest < dataend) {
        // Send data pids
        إذا (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            أرجع -1;
        }
        صحيح maxtransfer = 2*ورقہ_ناپ - (dest & (ورقہ_ناپ-1));
        صحيح transfer = dataend - dest;
        إذا (transfer > maxtransfer)
            transfer = ALIGN_تھلے(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_وچ : TD_DP_باہرلا) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hw_اگلا_TD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    إذا (cmd) {
        // Send status pid on control transfers
        إذا (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            أرجع -1;
        }
        td->hwINFO = (dir ? TD_DP_باہرلا : TD_DP_وچ) | TD_T_DATA1 | TD_CC;
        td->hwCBP = 0;
        td->hw_اگلا_TD = (u32)&td[1];
        td->hwBE = 0;
        td++;
    }

    // Transfer data
    pipe->ed.hw_سر_P = (u32)tds | (pipe->ed.hw_سر_P & ED_C);
    pipe->ed.hw_پوچھ_P = (u32)td;
    barrier();
    pipe->ed.hwINFO &= ~ED_SKIP;
    writel(&pipe->regs->cmdstatus, statuscmd);

    صحيح ret = wait_ed(&pipe->ed, usb_xfer_ویلا(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    إذا (ret)
        ohci_waittick(pipe->regs);
    أرجع ret;
}

صحيح
ohci_poll_intr(بنية usb_pipe *p, فراغ *انکڑے)
{
    ASSERT16();
    إذا (! CONFIG_USB_OHCI)
        أرجع -1;

    بنية ohci_pipe *pipe = container_دا(p, بنية ohci_pipe, pipe);
    بنية ohci_td *tds = GET_LOWFLAT(pipe->tds);
    بنية ohci_td *سر = (فراغ*)(GET_LOWFLAT(pipe->ed.hw_سر_P) & ~(ED_C|ED_H));
    بنية ohci_td *پوچھ = (فراغ*)GET_LOWFLAT(pipe->ed.hw_پوچھ_P);
    صحيح گنتی = GET_LOWFLAT(pipe->گنتی);
    صحيح pos = (پوچھ - tds + 1) % گنتی;
    بنية ohci_td *اگلا = &tds[pos];
    إذا (سر == اگلا)
        // No intrs found.
        أرجع -1;
    // XXX - check for errors.

    // Copy data.
    صحيح maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    فراغ *pipedata = GET_LOWFLAT((pipe->انکڑے));
    فراغ *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), انکڑے, SEG_کم, LOWFLAT2_کم(intrdata), maxpacket);

    // Reenable this td.
    سیٹ_LOWFLAT(پوچھ->hwINFO, TD_DP_وچ | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (پوچھ-tds);
    سیٹ_LOWFLAT(پوچھ->hwCBP, (u32)intrdata);
    سیٹ_LOWFLAT(پوچھ->hw_اگلا_TD, (u32)اگلا);
    سیٹ_LOWFLAT(پوچھ->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    سیٹ_LOWFLAT(pipe->ed.hw_پوچھ_P, (u32)اگلا);

    أرجع 0;
}
