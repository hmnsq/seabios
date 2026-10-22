// Code for handling EHCI USB controllers.
//
// Copyright (C) 2010-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_LOWFLAT
#include "config.h" // CONFIG_*
#include "output.h" // dprintf
#include "malloc.h" // free
#include "memmap.h" // PAGE_SIZE
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#include "pci_regs.h" // PCI_BASE_ADDRESS_0
#include "string.h" // memset
#include "usb.h" // struct usb_s
#include "usb-ehci.h" // struct ehci_qh
#include "util.h" // msleep
#include "x86.h" // readl

struct usb_ehci_s {
    struct usb_s usb;
    struct ehci_caps *caps;
    struct ehci_regs *regs;
    struct ehci_qh *async_qh;
    цео checkports;
};

struct ehci_pipe {
    struct ehci_qh qh;
    struct ehci_qtd *next_td, *tds;
    void *data;
    struct usb_pipe pipe;
};

static цео PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
static цео
ehci_hub_detect(struct usbhub_s *hub, u32 port)
{
    struct usb_ehci_s *cntl = container_of(hub->cntl, struct usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    ако (!(portsc & PORT_CONNECT))
        // No device present
        врати 0;

    ако ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        врати -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    врати 1;
}

// Reset device on port
static цео
ehci_hub_reset(struct usbhub_s *hub, u32 port)
{
    struct usb_ehci_s *cntl = container_of(hub->cntl, struct usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    ако (!(portsc & PORT_CONNECT))
        // No longer connected
        врати -1;
    ако (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        врати -1;
    }

    врати USB_HIGHSPEED;
}

// Disable port
static void
ehci_hub_disconnect(struct usbhub_s *hub, u32 port)
{
    struct usb_ehci_s *cntl = container_of(hub->cntl, struct usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

static struct usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
static цео
check_ehci_ports(struct usb_ehci_s *cntl)
{
    // Power up ports.
    цео i;
    за (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        ако (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    struct usbhub_s hub;
    memset(&hub, 0, sizeof(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    врати hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
static void
ehci_waittick(struct usb_ehci_s *cntl)
{
    ако (MODE16) {
        msleep(10);
        врати;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    за (;;) {
        sts = readl(&cntl->regs->usbsts);
        ако (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            ако (!(cmd & CMD_IAAD))
                break;
        }
        ако (timer_check(end)) {
            warn_timeout();
            врати;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    за (;;) {
        sts = readl(&cntl->regs->usbsts);
        ако (sts & STS_IAA)
            break;
        ако (timer_check(end)) {
            warn_timeout();
            врати;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

static void
ehci_free_pipes(struct usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    struct ehci_qh *start = cntl->async_qh;
    struct ehci_qh *pos = start;
    за (;;) {
        struct ehci_qh *next = (void*)(pos->next & ~EHCI_PTR_BITS);
        ако (next == start)
            break;
        struct ehci_pipe *pipe = container_of(next, struct ehci_pipe, qh);
        ако (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        инако
            pos = next;
    }
    ehci_waittick(cntl);
    за (;;) {
        struct usb_pipe *usbpipe = cntl->usb.freelist;
        ако (!usbpipe)
            break;
        cntl->usb.freelist = usbpipe->freenext;
        struct ehci_pipe *pipe = container_of(usbpipe, struct ehci_pipe, pipe);
        free(pipe);
    }
}

static void
configure_ehci(void *data)
{
    struct usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    struct ehci_framelist *fl = memalign_high(sizeof(*fl), sizeof(*fl));
    struct ehci_qh *intr_qh = memalign_high(EHCI_QH_ALIGN, sizeof(*intr_qh));
    struct ehci_qh *async_qh = memalign_high(EHCI_QH_ALIGN, sizeof(*async_qh));
    ако (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        goto fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    за (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        ако (!(cmd & CMD_HCRESET))
            break;
        ако (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            goto fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, sizeof(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    цео i;
    за (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, sizeof(*async_qh));
    async_qh->next = (u32)async_qh | EHCI_PTR_QH;
    async_qh->info1 = QH_HEAD;
    async_qh->token = QTD_STS_HALT;
    async_qh->qtd_next = async_qh->alt_next = EHCI_PTR_TERM;
    cntl->async_qh = async_qh;
    writel(&cntl->regs->asynclistbase, (u32)async_qh);

    // Enable queues
    writel(&cntl->regs->usbcmd, cmd | CMD_ASE | CMD_PSE | CMD_RUN);

    // Set default of high speed for root hub.
    writel(&cntl->regs->configflag, 1);
    PendingEHCI--;

    // Find devices
    цео count = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    ако (count)
        // Success
        врати;

    // No devices found - shutdown and free controller.
    writel(&cntl->regs->usbcmd, cmd & ~CMD_RUN);
    msleep(4);  // 2ms to stop reading memory - XXX
fail:
    free(fl);
    free(intr_qh);
    free(async_qh);
    free(cntl);
}

static void
ehci_controller_setup(struct pci_device *pci)
{
    struct ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ако (!caps)
        врати;
    u32 hcc_params = readl(&caps->hccparams);

    struct usb_ehci_s *cntl = malloc_tmphigh(sizeof(*cntl));
    ако (!cntl) {
        warn_noalloc();
        врати;
    }
    memset(cntl, 0, sizeof(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (void*)caps + readb(&caps->caplength);
    ако (hcc_params & HCC_64BIT_ADDR)
        cntl->regs->ctrldssegment = 0;
    PendingEHCI++;

    dprintf(1, "EHCI init on dev %pP (regs=%p)\n", pci, cntl->regs);

    pci_enable_busmaster(pci);

    // XXX - check for and disable SMM control?

    run_thread(configure_ehci, cntl);
}

void
ehci_setup(void)
{
    ако (! CONFIG_USB_EHCI)
        врати;
    struct pci_device *pci;
    foreachpci(pci) {
        ако (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
void
ehci_wait_controllers(void)
{
    док (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
static void
ehci_desc2pipe(struct ehci_pipe *pipe, struct usbdevice_s *usbdev
               , struct usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    struct usbdevice_s *hubdev = usbdev->hub->usbdev;
    ако (hubdev) {
        struct ehci_pipe *hpipe = container_of(
            hubdev->defpipe, struct ehci_pipe, pipe);
        ако (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        инако
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ако (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    инако ако (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

static struct usb_pipe *
ehci_alloc_intr_pipe(struct usbdevice_s *usbdev
                     , struct usb_endpoint_descriptor *epdesc)
{
    struct usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, struct usb_ehci_s, usb);
    цео frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ако (frameexp > 10)
        frameexp = 10;
    цео maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    цео ms = 1<<frameexp;
    цео count = DIV_ROUND_UP(ticks_to_ms(2), ms);
    struct ehci_pipe *pipe = memalign_low(EHCI_QH_ALIGN, sizeof(*pipe));
    struct ehci_qtd *tds = memalign_low(EHCI_QTD_ALIGN, sizeof(*tds) * count);
    void *data = malloc_low(maxpacket * count);
    ако (!pipe || !tds || !data) {
        warn_noalloc();
        goto fail;
    }
    memset(pipe, 0, sizeof(*pipe));
    memset(tds, 0, sizeof(*tds) * count);
    memset(data, 0, maxpacket * count);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    цео i;
    за (i=0; i<count; i++) {
        struct ehci_qtd *td = &tds[i];
        td->qtd_next = (i==count-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    struct ehci_framelist *fl = (void*)readl(&cntl->regs->periodiclistbase);
    ако (frameexp == 0) {
        // Add to existing interrupt entry.
        struct ehci_qh *intr_qh = (void*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } инако {
        цео startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        за (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    врати &pipe->pipe;
fail:
    free(pipe);
    free(tds);
    free(data);
    врати NULL;
}

struct usb_pipe *
ehci_realloc_pipe(struct usbdevice_s *usbdev, struct usb_pipe *upipe
                  , struct usb_endpoint_descriptor *epdesc)
{
    ако (! CONFIG_USB_EHCI)
        врати NULL;
    usb_add_freelist(upipe);
    ако (!epdesc)
        врати NULL;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ако (eptype == USB_ENDPOINT_XFER_INT)
        врати ehci_alloc_intr_pipe(usbdev, epdesc);
    struct usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, struct usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    struct usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ако (usbpipe) {
        // Use previously allocated pipe.
        struct ehci_pipe *pipe = container_of(usbpipe, struct ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        врати usbpipe;
    }

    // Allocate a new queue head.
    struct ehci_pipe *pipe;
    ако (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, sizeof(*pipe));
    инако
        pipe = memalign_low(EHCI_QH_ALIGN, sizeof(*pipe));
    ако (!pipe) {
        warn_noalloc();
        врати NULL;
    }
    memset(pipe, 0, sizeof(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    struct ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    врати &pipe->pipe;
}

static void
ehci_reset_pipe(struct ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

static цео
ehci_wait_td(struct ehci_pipe *pipe, struct ehci_qtd *td, u32 end)
{
    u32 status;
    за (;;) {
        status = td->token;
        ако (!(status & QTD_STS_ACTIVE))
            break;
        ако (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            struct usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), struct usb_ehci_s, usb);
            ehci_waittick(cntl);
            врати -1;
        }
        yield();
    }
    ако (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        врати -2;
    }
    врати 0;
}

static void
ehci_fill_tdbuf(struct ehci_qtd *td, u32 dest, цео transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    за (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

цео
ehci_send_pipe(struct usb_pipe *p, цео dir, const void *cmd
               , void *data, цео datasize)
{
    ако (! CONFIG_USB_EHCI)
        врати -1;
    struct ehci_pipe *pipe = container_of(p, struct ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[sizeof(struct ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    struct ehci_qtd *tds = (void*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, sizeof(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    ако (cmd) {
        // Send setup pid on control transfers
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(USB_CONTROL_SETUP_SIZE) | QTD_STS_ACTIVE
                     | QTD_PID_SETUP | ehci_maxerr(3));
        ehci_fill_tdbuf(td, (u32)cmd, USB_CONTROL_SETUP_SIZE);
        td++;
        toggle = QTD_TOGGLE;
    }
    u32 dest = (u32)data, dataend = dest + datasize;
    док (dest < dataend) {
        // Send data pids
        ако (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            врати -1;
        }
        цео maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        цео transfer = dataend - dest;
        ако (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    ако (cmd) {
        // Send status pid on control transfers
        ако (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            врати -1;
        }
        td->qtd_next = EHCI_PTR_TERM;
        td->alt_next = EHCI_PTR_TERM;
        td->token = (QTD_TOGGLE | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_OUT : QTD_PID_IN) | ehci_maxerr(3));
        td++;
    }

    // Transfer data
    (td-1)->qtd_next = EHCI_PTR_TERM;
    barrier();
    SET_LOWFLAT(pipe->qh.qtd_next, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    цео i;
    за (i=0, td=tds; i<STACKQTDS; i++, td++) {
        цео ret = ehci_wait_td(pipe, td, end);
        ако (ret)
            врати -1;
    }

    врати 0;
}

цео
ehci_poll_intr(struct usb_pipe *p, void *data)
{
    ASSERT16();
    ако (! CONFIG_USB_EHCI)
        врати -1;
    struct ehci_pipe *pipe = container_of(p, struct ehci_pipe, pipe);
    struct ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    ако (token & QTD_STS_ACTIVE)
        // No intrs found.
        врати -1;
    // XXX - check for errors.

    // Copy data.
    цео maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    цео pos = td - GET_LOWFLAT(pipe->tds);
    void *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_LOW, LOWFLAT2LOW(tddata), maxpacket);

    // Reenable this td.
    struct ehci_qtd *next = (void*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    врати 0;
}
