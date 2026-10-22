// Code for handling OHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_LOWFLAT
#include "config.h" // CONFIG_*
#include "malloc.h" // free
#include "memmap.h" // PAGE_SIZE
#include "output.h" // dprintf
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_CLASS_SERIAL_USB_OHCI
#include "pci_regs.h" // PCI_BASE_ADDRESS_0
#include "string.h" // memset
#include "usb.h" // struct usb_s
#include "usb-ehci.h" // ehci_wait_controllers
#include "usb-ohci.h" // struct ohci_hcca
#include "util.h" // msleep
#include "x86.h" // readl

#define FIT                     (1 << 31)

sa_nracanaa usb_ohci_s {
    sa_nracanaa usb_s usb;
    sa_nracanaa ohci_regs *regs;
};

sa_nracanaa ohci_pipe {
    sa_nracanaa ohci_ed ed;
    sa_nracanaa usb_pipe pipe;
    sa_nracanaa ohci_regs *regs;
    void *data;
    පූර්ණ_සංඛ්‍යා count;
    sa_nracanaa ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
static පූර්ණ_සංඛ්‍යා
ohci_hub_detect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_ohci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    ආපසු (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
static void
ohci_hub_disconnect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_ohci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
static පූර්ණ_සංඛ්‍යා
ohci_hub_reset(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_ohci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    සඳහා (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        නම් (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            bha_nga;
        නම් (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            ආපසු -1;
        }
        yield();
    }

    නම් ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        ආපසු -1;

    ආපසු !!(sts & RH_PS_LSDA);
}

static sa_nracanaa usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
static පූර්ණ_සංඛ්‍යා
check_ohci_ports(sa_nracanaa usb_ohci_s *cntl)
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

    sa_nracanaa usbhub_s hub;
    memset(&hub, 0, parimaa_na(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    ආපසු hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
static void
ohci_waittick(sa_nracanaa ohci_regs *regs)
{
    barrier();
    sa_nracanaa ohci_hcca *hcca = (void*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    සඳහා (;;) {
        නම් (hcca->frame_no != startframe)
            bha_nga;
        නම් (timer_check(end)) {
            warn_timeout();
            ආපසු;
        }
        yield();
    }
}

static void
ohci_free_pipes(sa_nracanaa usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    නම් (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    සඳහා (;;) {
        sa_nracanaa ohci_ed *next = (void*)*pos;
        නම් (!next)
            bha_nga;
        sa_nracanaa ohci_pipe *pipe = container_of(next, sa_nracanaa ohci_pipe, ed);
        නම් (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            free(pipe);
        } නැතහොත් {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = NULL;
}

static පූර්ණ_සංඛ්‍යා
start_ohci(sa_nracanaa usb_ohci_s *cntl, sa_nracanaa ohci_hcca *hcca)
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
    සඳහා (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        නම් (!(status & OHCI_HCR))
            bha_nga;
        නම් (timer_check(end)) {
            warn_timeout();
            ආපසු -1;
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

    ආපසු 0;
}

static void
stop_ohci(sa_nracanaa usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

static void
configure_ohci(void *data)
{
    sa_nracanaa usb_ohci_s *cntl = data;

    // Allocate memory
    sa_nracanaa ohci_hcca *hcca = memalign_high(256, parimaa_na(*hcca));
    sa_nracanaa ohci_ed *intr_ed = malloc_high(parimaa_na(*intr_ed));
    නම් (!hcca || !intr_ed) {
        warn_noalloc();
        goto free;
    }
    memset(hcca, 0, parimaa_na(*hcca));
    memset(intr_ed, 0, parimaa_na(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    පූර්ණ_සංඛ්‍යා ret = start_ohci(cntl, hcca);
    නම් (ret)
        goto err;

    පූර්ණ_සංඛ්‍යා count = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    නම් (! count)
        goto err;
    ආපසු;

err:
    stop_ohci(cntl);
free:
    free(hcca);
    free(intr_ed);
}

static void
ohci_controller_setup(sa_nracanaa pci_device *pci)
{
    sa_nracanaa ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    නම් (!regs)
        ආපසු;

    sa_nracanaa usb_ohci_s *cntl = malloc_tmphigh(parimaa_na(*cntl));
    නම් (!cntl) {
        warn_noalloc();
        ආපසු;
    }
    memset(cntl, 0, parimaa_na(*cntl));
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

void
ohci_setup(void)
{
    නම් (! CONFIG_USB_OHCI)
        ආපසු;
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        නම් (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
static void
ohci_desc2pipe(sa_nracanaa ohci_pipe *pipe, sa_nracanaa usbdevice_s *usbdev
               , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    sa_nracanaa usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

static sa_nracanaa usb_pipe *
ohci_alloc_intr_pipe(sa_nracanaa usbdevice_s *usbdev
                     , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    sa_nracanaa usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_ohci_s, usb);
    පූර්ණ_සංඛ්‍යා frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    නම් (frameexp > 5)
        frameexp = 5;
    පූර්ණ_සංඛ්‍යා maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    පූර්ණ_සංඛ්‍යා ms = 1<<frameexp;
    පූර්ණ_සංඛ්‍යා count = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    sa_nracanaa ohci_pipe *pipe = malloc_low(parimaa_na(*pipe));
    sa_nracanaa ohci_td *tds = malloc_low(parimaa_na(*tds) * count);
    void *data = malloc_low(maxpacket * count);
    නම් (!pipe || !tds || !data)
        goto err;
    memset(pipe, 0, parimaa_na(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->count = count;
    pipe->tds = tds;

    sa_nracanaa ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[count-1];

    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i=0; i<count-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    sa_nracanaa ohci_hcca *hcca = (void*)cntl->regs->hcca;
    නම් (frameexp == 0) {
        // Add to existing interrupt entry.
        sa_nracanaa ohci_ed *intr_ed = (void*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } නැතහොත් {
        පූර්ණ_සංඛ්‍යා startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        සඳහා (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    ආපසු &pipe->pipe;

err:
    free(pipe);
    free(tds);
    free(data);
    ආපසු NULL;
}

sa_nracanaa usb_pipe *
ohci_realloc_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *upipe
                  , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    නම් (! CONFIG_USB_OHCI)
        ආපසු NULL;
    usb_add_freelist(upipe);
    නම් (!epdesc)
        ආපසු NULL;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    නම් (eptype == USB_ENDPOINT_XFER_INT)
        ආපසු ohci_alloc_intr_pipe(usbdev, epdesc);
    sa_nracanaa usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    sa_nracanaa usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    නම් (usbpipe) {
        // Use previously allocated pipe.
        sa_nracanaa ohci_pipe *pipe = container_of(usbpipe, sa_nracanaa ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        ආපසු usbpipe;
    }

    // Allocate a new queue head.
    sa_nracanaa ohci_pipe *pipe;
    නම් (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(parimaa_na(*pipe));
    නැතහොත්
        pipe = malloc_low(parimaa_na(*pipe));
    නම් (!pipe) {
        warn_noalloc();
        ආපසු NULL;
    }
    memset(pipe, 0, parimaa_na(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    නම් (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    ආපසු &pipe->pipe;
}

static පූර්ණ_සංඛ්‍යා
wait_ed(sa_nracanaa ohci_ed *ed, පූර්ණ_සංඛ්‍යා timeout)
{
    u32 end = timer_calc(timeout);
    සඳහා (;;) {
        නම් ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            ආපසු 0;
        නම් (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            ආපසු -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

පූර්ණ_සංඛ්‍යා
ohci_send_pipe(sa_nracanaa usb_pipe *p, පූර්ණ_සංඛ්‍යා dir, const void *cmd
               , void *data, පූර්ණ_සංඛ්‍යා datasize)
{
    ASSERT32FLAT();
    නම් (! CONFIG_USB_OHCI)
        ආපසු -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    sa_nracanaa ohci_pipe *pipe = container_of(p, sa_nracanaa ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[parimaa_na(sa_nracanaa ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    sa_nracanaa ohci_td *tds = (void*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, parimaa_na(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    නම් (cmd) {
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
    අතරතුර (dest < dataend) {
        // Send data pids
        නම් (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ආපසු -1;
        }
        පූර්ණ_සංඛ්‍යා maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        පූර්ණ_සංඛ්‍යා transfer = dataend - dest;
        නම් (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    නම් (cmd) {
        // Send status pid on control transfers
        නම් (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            ආපසු -1;
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

    පූර්ණ_සංඛ්‍යා ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    නම් (ret)
        ohci_waittick(pipe->regs);
    ආපසු ret;
}

පූර්ණ_සංඛ්‍යා
ohci_poll_intr(sa_nracanaa usb_pipe *p, void *data)
{
    ASSERT16();
    නම් (! CONFIG_USB_OHCI)
        ආපසු -1;

    sa_nracanaa ohci_pipe *pipe = container_of(p, sa_nracanaa ohci_pipe, pipe);
    sa_nracanaa ohci_td *tds = GET_LOWFLAT(pipe->tds);
    sa_nracanaa ohci_td *head = (void*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    sa_nracanaa ohci_td *tail = (void*)GET_LOWFLAT(pipe->ed.hwTailP);
    පූර්ණ_සංඛ්‍යා count = GET_LOWFLAT(pipe->count);
    පූර්ණ_සංඛ්‍යා pos = (tail - tds + 1) % count;
    sa_nracanaa ohci_td *next = &tds[pos];
    නම් (head == next)
        // No intrs found.
        ආපසු -1;
    // XXX - check for errors.

    // Copy data.
    පූර්ණ_සංඛ්‍යා maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    void *pipedata = GET_LOWFLAT((pipe->data));
    void *intrdata = pipedata + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_LOW, LOWFLAT2LOW(intrdata), maxpacket);

    // Reenable this td.
    SET_LOWFLAT(tail->hwINFO, TD_DP_IN | TD_T_TOGGLE | TD_CC);
    intrdata = pipedata + maxpacket * (tail-tds);
    SET_LOWFLAT(tail->hwCBP, (u32)intrdata);
    SET_LOWFLAT(tail->hwNextTD, (u32)next);
    SET_LOWFLAT(tail->hwBE, (u32)intrdata + maxpacket - 1);
    barrier();
    SET_LOWFLAT(pipe->ed.hwTailP, (u32)next);

    ආපසු 0;
}
