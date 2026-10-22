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

struct usb_ohci_s {
    struct usb_s usb;
    struct ohci_regs *regs;
};

struct ohci_pipe {
    struct ohci_ed ed;
    struct usb_pipe pipe;
    struct ohci_regs *regs;
    void *data;
    цео count;
    struct ohci_td *tds;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to port
static цео
ohci_hub_detect(struct usbhub_s *hub, u32 port)
{
    struct usb_ohci_s *cntl = container_of(hub->cntl, struct usb_ohci_s, usb);
    u32 sts = readl(&cntl->regs->roothub_portstatus[port]);
    врати (sts & RH_PS_CCS) ? 1 : 0;
}

// Disable port
static void
ohci_hub_disconnect(struct usbhub_s *hub, u32 port)
{
    struct usb_ohci_s *cntl = container_of(hub->cntl, struct usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_CCS|RH_PS_LSDA);
}

// Reset device on port
static цео
ohci_hub_reset(struct usbhub_s *hub, u32 port)
{
    struct usb_ohci_s *cntl = container_of(hub->cntl, struct usb_ohci_s, usb);
    writel(&cntl->regs->roothub_portstatus[port], RH_PS_PRS);
    u32 sts;
    u32 end = timer_calc(USB_TIME_DRSTR * 2);
    за (;;) {
        sts = readl(&cntl->regs->roothub_portstatus[port]);
        ако (!(sts & RH_PS_PRS))
            // XXX - need to ensure USB_TIME_DRSTR time in reset?
            break;
        ако (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ohci_hub_disconnect(hub, port);
            врати -1;
        }
        yield();
    }

    ако ((sts & (RH_PS_CCS|RH_PS_PES)) != (RH_PS_CCS|RH_PS_PES))
        // Device no longer present
        врати -1;

    врати !!(sts & RH_PS_LSDA);
}

static struct usbhub_op_s ohci_HubOp = {
    .detect = ohci_hub_detect,
    .reset = ohci_hub_reset,
    .disconnect = ohci_hub_disconnect,
};

// Find any devices connected to the root hub.
static цео
check_ohci_ports(struct usb_ohci_s *cntl)
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

    struct usbhub_s hub;
    memset(&hub, 0, sizeof(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = rha & RH_A_NDP;
    hub.op = &ohci_HubOp;
    usb_enumerate(&hub);
    врати hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
static void
ohci_waittick(struct ohci_regs *regs)
{
    barrier();
    struct ohci_hcca *hcca = (void*)regs->hcca;
    u32 startframe = hcca->frame_no;
    u32 end = timer_calc(1000 * 5);
    за (;;) {
        ако (hcca->frame_no != startframe)
            break;
        ако (timer_check(end)) {
            warn_timeout();
            врати;
        }
        yield();
    }
}

static void
ohci_free_pipes(struct usb_ohci_s *cntl)
{
    dprintf(7, "ohci_free_pipes %p\n", cntl);

    u32 creg = readl(&cntl->regs->control);
    ако (creg & (OHCI_CTRL_CLE|OHCI_CTRL_BLE)) {
        writel(&cntl->regs->control, creg & ~(OHCI_CTRL_CLE|OHCI_CTRL_BLE));
        ohci_waittick(cntl->regs);
    }

    u32 *pos = &cntl->regs->ed_controlhead;
    за (;;) {
        struct ohci_ed *next = (void*)*pos;
        ако (!next)
            break;
        struct ohci_pipe *pipe = container_of(next, struct ohci_pipe, ed);
        ако (usb_is_freelist(&cntl->usb, &pipe->pipe)) {
            *pos = next->hwNextED;
            free(pipe);
        } инако {
            pos = &next->hwNextED;
        }
    }

    writel(&cntl->regs->ed_controlcurrent, 0);
    writel(&cntl->regs->ed_bulkcurrent, 0);
    writel(&cntl->regs->control, creg);
    cntl->usb.freelist = NULL;
}

static цео
start_ohci(struct usb_ohci_s *cntl, struct ohci_hcca *hcca)
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
    за (;;) {
        u32 status = readl(&cntl->regs->cmdstatus);
        ако (!(status & OHCI_HCR))
            break;
        ако (timer_check(end)) {
            warn_timeout();
            врати -1;
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

    врати 0;
}

static void
stop_ohci(struct usb_ohci_s *cntl)
{
    u32 oldrwc = readl(&cntl->regs->control) & OHCI_CTRL_RWC;
    writel(&cntl->regs->control, oldrwc);
    readl(&cntl->regs->control); // flush writes
}

static void
configure_ohci(void *data)
{
    struct usb_ohci_s *cntl = data;

    // Allocate memory
    struct ohci_hcca *hcca = memalign_high(256, sizeof(*hcca));
    struct ohci_ed *intr_ed = malloc_high(sizeof(*intr_ed));
    ако (!hcca || !intr_ed) {
        warn_noalloc();
        goto free;
    }
    memset(hcca, 0, sizeof(*hcca));
    memset(intr_ed, 0, sizeof(*intr_ed));
    intr_ed->hwINFO = ED_SKIP;
    цео i;
    за (i=0; i<ARRAY_SIZE(hcca->int_table); i++)
        hcca->int_table[i] = (u32)intr_ed;

    цео ret = start_ohci(cntl, hcca);
    ако (ret)
        goto err;

    цео count = check_ohci_ports(cntl);
    ohci_free_pipes(cntl);
    ако (! count)
        goto err;
    врати;

err:
    stop_ohci(cntl);
free:
    free(hcca);
    free(intr_ed);
}

static void
ohci_controller_setup(struct pci_device *pci)
{
    struct ohci_regs *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ако (!regs)
        врати;

    struct usb_ohci_s *cntl = malloc_tmphigh(sizeof(*cntl));
    ако (!cntl) {
        warn_noalloc();
        врати;
    }
    memset(cntl, 0, sizeof(*cntl));
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
    ако (! CONFIG_USB_OHCI)
        врати;
    struct pci_device *pci;
    foreachpci(pci) {
        ако (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_OHCI)
            ohci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in ed
static void
ohci_desc2pipe(struct ohci_pipe *pipe, struct usbdevice_s *usbdev
               , struct usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->ed.hwINFO = (ED_SKIP | usbdev->devaddr | (pipe->pipe.ep << 7)
                       | (epdesc->wMaxPacketSize << 16)
                       | (usbdev->speed ? ED_LOWSPEED : 0));
    struct usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, struct usb_ohci_s, usb);
    pipe->regs = cntl->regs;
}

static struct usb_pipe *
ohci_alloc_intr_pipe(struct usbdevice_s *usbdev
                     , struct usb_endpoint_descriptor *epdesc)
{
    struct usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, struct usb_ohci_s, usb);
    цео frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ohci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ако (frameexp > 5)
        frameexp = 5;
    цео maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    цео ms = 1<<frameexp;
    цео count = DIV_ROUND_UP(ticks_to_ms(2), ms) + 1;
    struct ohci_pipe *pipe = malloc_low(sizeof(*pipe));
    struct ohci_td *tds = malloc_low(sizeof(*tds) * count);
    void *data = malloc_low(maxpacket * count);
    ако (!pipe || !tds || !data)
        goto err;
    memset(pipe, 0, sizeof(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);
    pipe->ed.hwINFO &= ~ED_SKIP;
    pipe->data = data;
    pipe->count = count;
    pipe->tds = tds;

    struct ohci_ed *ed = &pipe->ed;
    ed->hwHeadP = (u32)&tds[0];
    ed->hwTailP = (u32)&tds[count-1];

    цео i;
    за (i=0; i<count-1; i++) {
        tds[i].hwINFO = TD_DP_IN | TD_T_TOGGLE | TD_CC;
        tds[i].hwCBP = (u32)data + maxpacket * i;
        tds[i].hwNextTD = (u32)&tds[i+1];
        tds[i].hwBE = tds[i].hwCBP + maxpacket - 1;
    }

    // Add to interrupt schedule.
    struct ohci_hcca *hcca = (void*)cntl->regs->hcca;
    ако (frameexp == 0) {
        // Add to existing interrupt entry.
        struct ohci_ed *intr_ed = (void*)hcca->int_table[0];
        ed->hwNextED = intr_ed->hwNextED;
        barrier();
        intr_ed->hwNextED = (u32)ed;
    } инако {
        цео startpos = 1<<(frameexp-1);
        ed->hwNextED = hcca->int_table[startpos];
        barrier();
        за (i=startpos; i<ARRAY_SIZE(hcca->int_table); i+=ms)
            hcca->int_table[i] = (u32)ed;
    }

    врати &pipe->pipe;

err:
    free(pipe);
    free(tds);
    free(data);
    врати NULL;
}

struct usb_pipe *
ohci_realloc_pipe(struct usbdevice_s *usbdev, struct usb_pipe *upipe
                  , struct usb_endpoint_descriptor *epdesc)
{
    ако (! CONFIG_USB_OHCI)
        врати NULL;
    usb_add_freelist(upipe);
    ако (!epdesc)
        врати NULL;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ако (eptype == USB_ENDPOINT_XFER_INT)
        врати ohci_alloc_intr_pipe(usbdev, epdesc);
    struct usb_ohci_s *cntl = container_of(
        usbdev->hub->cntl, struct usb_ohci_s, usb);
    dprintf(7, "ohci_alloc_async_pipe %p\n", &cntl->usb);

    struct usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ако (usbpipe) {
        // Use previously allocated pipe.
        struct ohci_pipe *pipe = container_of(usbpipe, struct ohci_pipe, pipe);
        ohci_desc2pipe(pipe, usbdev, epdesc);
        врати usbpipe;
    }

    // Allocate a new queue head.
    struct ohci_pipe *pipe;
    ако (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(sizeof(*pipe));
    инако
        pipe = malloc_low(sizeof(*pipe));
    ако (!pipe) {
        warn_noalloc();
        врати NULL;
    }
    memset(pipe, 0, sizeof(*pipe));
    ohci_desc2pipe(pipe, usbdev, epdesc);

    // Add queue head to controller list.
    u32 *head = &cntl->regs->ed_controlhead;
    ако (eptype != USB_ENDPOINT_XFER_CONTROL)
        head = &cntl->regs->ed_bulkhead;
    pipe->ed.hwNextED = *head;
    barrier();
    *head = (u32)&pipe->ed;
    врати &pipe->pipe;
}

static цео
wait_ed(struct ohci_ed *ed, цео timeout)
{
    u32 end = timer_calc(timeout);
    за (;;) {
        ако ((ed->hwHeadP & ~(ED_C|ED_H)) == ed->hwTailP)
            врати 0;
        ако (timer_check(end)) {
            warn_timeout();
            dprintf(1, "ohci ed info=%x tail=%x head=%x next=%x\n"
                    , ed->hwINFO, ed->hwTailP, ed->hwHeadP, ed->hwNextED);
            врати -1;
        }
        yield();
    }
}

#define STACKOTDS 18
#define OHCI_TD_ALIGN 16

цео
ohci_send_pipe(struct usb_pipe *p, цео dir, const void *cmd
               , void *data, цео datasize)
{
    ASSERT32FLAT();
    ако (! CONFIG_USB_OHCI)
        врати -1;
    dprintf(7, "ohci_send_pipe %p\n", p);
    struct ohci_pipe *pipe = container_of(p, struct ohci_pipe, pipe);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[sizeof(struct ohci_td) * STACKOTDS + OHCI_TD_ALIGN - 1];
    struct ohci_td *tds = (void*)ALIGN((u32)tdsbuf, OHCI_TD_ALIGN), *td = tds;
    memset(tds, 0, sizeof(*tds) * STACKOTDS);

    // Setup transfer descriptors
    u16 maxpacket = pipe->pipe.maxpacket;
    u32 toggle = 0, statuscmd = OHCI_BLF;
    ако (cmd) {
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
    док (dest < dataend) {
        // Send data pids
        ако (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            врати -1;
        }
        цео maxtransfer = 2*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        цео transfer = dataend - dest;
        ако (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->hwINFO = (dir ? TD_DP_IN : TD_DP_OUT) | toggle | TD_CC;
        td->hwCBP = dest;
        td->hwNextTD = (u32)&td[1];
        td->hwBE = dest + transfer - 1;
        td++;
        dest += transfer;
    }
    ако (cmd) {
        // Send status pid on control transfers
        ако (td >= &tds[STACKOTDS]) {
            warn_noalloc();
            врати -1;
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

    цео ret = wait_ed(&pipe->ed, usb_xfer_time(p, datasize));
    pipe->ed.hwINFO |= ED_SKIP;
    ако (ret)
        ohci_waittick(pipe->regs);
    врати ret;
}

цео
ohci_poll_intr(struct usb_pipe *p, void *data)
{
    ASSERT16();
    ако (! CONFIG_USB_OHCI)
        врати -1;

    struct ohci_pipe *pipe = container_of(p, struct ohci_pipe, pipe);
    struct ohci_td *tds = GET_LOWFLAT(pipe->tds);
    struct ohci_td *head = (void*)(GET_LOWFLAT(pipe->ed.hwHeadP) & ~(ED_C|ED_H));
    struct ohci_td *tail = (void*)GET_LOWFLAT(pipe->ed.hwTailP);
    цео count = GET_LOWFLAT(pipe->count);
    цео pos = (tail - tds + 1) % count;
    struct ohci_td *next = &tds[pos];
    ако (head == next)
        // No intrs found.
        врати -1;
    // XXX - check for errors.

    // Copy data.
    цео maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
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

    врати 0;
}
