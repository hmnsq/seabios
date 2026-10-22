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

sa_nracanaa usb_ehci_s {
    sa_nracanaa usb_s usb;
    sa_nracanaa ehci_caps *caps;
    sa_nracanaa ehci_regs *regs;
    sa_nracanaa ehci_qh *async_qh;
    ຈຳນວນເຕັມ checkports;
};

sa_nracanaa ehci_pipe {
    sa_nracanaa ehci_qh qh;
    sa_nracanaa ehci_qtd *next_td, *tds;
    void *data;
    sa_nracanaa usb_pipe pipe;
};

static ຈຳນວນເຕັມ PendingEHCI;


/****************************************************************
 * Root hub
 ****************************************************************/

#define EHCI_TIME_POSTPOWER 20
#define EHCI_TIME_POSTRESET 2

// Check if device attached to port
static ຈຳນວນເຕັມ
ehci_hub_detect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_ehci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    ຖ້າ (!(portsc & PORT_CONNECT))
        // No device present
        ສົ່ງຄືນ 0;

    ຖ້າ ((portsc & PORT_LINESTATUS_MASK) == PORT_LINESTATUS_KSTATE) {
        // low speed device
        writel(portreg, portsc | PORT_OWNER);
        ສົ່ງຄືນ -1;
    }

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    portsc = (portsc & ~PORT_PE) | PORT_RESET;
    writel(portreg, portsc);
    msleep(USB_TIME_DRSTR);
    ສົ່ງຄືນ 1;
}

// Reset device on port
static ຈຳນວນເຕັມ
ehci_hub_reset(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_ehci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);

    // Finish reset on port
    portsc &= ~PORT_RESET;
    writel(portreg, portsc);
    msleep(EHCI_TIME_POSTRESET);

    portsc = readl(portreg);
    ຖ້າ (!(portsc & PORT_CONNECT))
        // No longer connected
        ສົ່ງຄືນ -1;
    ຖ້າ (!(portsc & PORT_PE)) {
        // full speed device
        writel(portreg, portsc | PORT_OWNER);
        ສົ່ງຄືນ -1;
    }

    ສົ່ງຄືນ USB_HIGHSPEED;
}

// Disable port
static void
ehci_hub_disconnect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_ehci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_ehci_s, usb);
    u32 *portreg = &cntl->regs->portsc[port];
    u32 portsc = readl(portreg);
    writel(portreg, portsc & ~PORT_PE);
}

static sa_nracanaa usbhub_op_s ehci_HubOp = {
    .detect = ehci_hub_detect,
    .reset = ehci_hub_reset,
    .disconnect = ehci_hub_disconnect,
};

// Find any devices connected to the root hub.
static ຈຳນວນເຕັມ
check_ehci_ports(sa_nracanaa usb_ehci_s *cntl)
{
    // Power up ports.
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<cntl->checkports; i++) {
        u32 *portreg = &cntl->regs->portsc[i];
        u32 portsc = readl(portreg);
        ຖ້າ (!(portsc & PORT_POWER)) {
            portsc |= PORT_POWER;
            writel(portreg, portsc);
        }
    }
    msleep(EHCI_TIME_POSTPOWER);

    sa_nracanaa usbhub_s hub;
    memset(&hub, 0, parimaa_na(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = cntl->checkports;
    hub.op = &ehci_HubOp;
    usb_enumerate(&hub);
    ສົ່ງຄືນ hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB async frame to start - for ensuring safe memory release.
static void
ehci_waittick(sa_nracanaa usb_ehci_s *cntl)
{
    ຖ້າ (MODE16) {
        msleep(10);
        ສົ່ງຄືນ;
    }
    // Wait for access to "doorbell"
    barrier();
    u32 cmd, sts;
    u32 end = timer_calc(100);
    ສຳລັບ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ຖ້າ (!(sts & STS_IAA)) {
            cmd = readl(&cntl->regs->usbcmd);
            ຖ້າ (!(cmd & CMD_IAAD))
                bha_nga;
        }
        ຖ້າ (timer_check(end)) {
            warn_timeout();
            ສົ່ງຄືນ;
        }
        yield();
    }
    // Ring "doorbell"
    writel(&cntl->regs->usbcmd, cmd | CMD_IAAD);
    // Wait for completion
    ສຳລັບ (;;) {
        sts = readl(&cntl->regs->usbsts);
        ຖ້າ (sts & STS_IAA)
            bha_nga;
        ຖ້າ (timer_check(end)) {
            warn_timeout();
            ສົ່ງຄືນ;
        }
        yield();
    }
    // Ack completion
    writel(&cntl->regs->usbsts, STS_IAA);
}

static void
ehci_free_pipes(sa_nracanaa usb_ehci_s *cntl)
{
    dprintf(7, "ehci_free_pipes %p\n", cntl);

    sa_nracanaa ehci_qh *start = cntl->async_qh;
    sa_nracanaa ehci_qh *pos = start;
    ສຳລັບ (;;) {
        sa_nracanaa ehci_qh *next = (void*)(pos->next & ~EHCI_PTR_BITS);
        ຖ້າ (next == start)
            bha_nga;
        sa_nracanaa ehci_pipe *pipe = container_of(next, sa_nracanaa ehci_pipe, qh);
        ຖ້າ (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->next = next->next;
        ບໍ່ດັ່ງນັ້ນ
            pos = next;
    }
    ehci_waittick(cntl);
    ສຳລັບ (;;) {
        sa_nracanaa usb_pipe *usbpipe = cntl->usb.freelist;
        ຖ້າ (!usbpipe)
            bha_nga;
        cntl->usb.freelist = usbpipe->freenext;
        sa_nracanaa ehci_pipe *pipe = container_of(usbpipe, sa_nracanaa ehci_pipe, pipe);
        free(pipe);
    }
}

static void
configure_ehci(void *data)
{
    sa_nracanaa usb_ehci_s *cntl = data;

    // Allocate ram for schedule storage
    sa_nracanaa ehci_framelist *fl = memalign_high(parimaa_na(*fl), parimaa_na(*fl));
    sa_nracanaa ehci_qh *intr_qh = memalign_high(EHCI_QH_ALIGN, parimaa_na(*intr_qh));
    sa_nracanaa ehci_qh *async_qh = memalign_high(EHCI_QH_ALIGN, parimaa_na(*async_qh));
    ຖ້າ (!fl || !intr_qh || !async_qh) {
        warn_noalloc();
        PendingEHCI--;
        goto fail;
    }

    // XXX - check for halted?

    // Reset the HC
    u32 cmd = readl(&cntl->regs->usbcmd);
    writel(&cntl->regs->usbcmd, (cmd & ~(CMD_ASE | CMD_PSE)) | CMD_HCRESET);
    u32 end = timer_calc(250);
    ສຳລັບ (;;) {
        cmd = readl(&cntl->regs->usbcmd);
        ຖ້າ (!(cmd & CMD_HCRESET))
            bha_nga;
        ຖ້າ (timer_check(end)) {
            warn_timeout();
            PendingEHCI--;
            goto fail;
        }
        yield();
    }

    // Disable interrupts (just to be safe).
    writel(&cntl->regs->usbintr, 0);

    // Set schedule to point to primary intr queue head
    memset(intr_qh, 0, parimaa_na(*intr_qh));
    intr_qh->next = EHCI_PTR_TERM;
    intr_qh->info2 = (0x01 << QH_SMASK_SHIFT);
    intr_qh->token = QTD_STS_HALT;
    intr_qh->qtd_next = intr_qh->alt_next = EHCI_PTR_TERM;
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)intr_qh | EHCI_PTR_QH;
    writel(&cntl->regs->periodiclistbase, (u32)fl);

    // Set async list to point to primary async queue head
    memset(async_qh, 0, parimaa_na(*async_qh));
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
    ຈຳນວນເຕັມ count = check_ehci_ports(cntl);
    ehci_free_pipes(cntl);
    ຖ້າ (count)
        // Success
        ສົ່ງຄືນ;

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
ehci_controller_setup(sa_nracanaa pci_device *pci)
{
    sa_nracanaa ehci_caps *caps = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ຖ້າ (!caps)
        ສົ່ງຄືນ;
    u32 hcc_params = readl(&caps->hccparams);

    sa_nracanaa usb_ehci_s *cntl = malloc_tmphigh(parimaa_na(*cntl));
    ຖ້າ (!cntl) {
        warn_noalloc();
        ສົ່ງຄືນ;
    }
    memset(cntl, 0, parimaa_na(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_EHCI;
    cntl->caps = caps;
    cntl->checkports = readl(&cntl->caps->hcsparams) & HCS_N_PORTS_MASK;
    cntl->regs = (void*)caps + readb(&caps->caplength);
    ຖ້າ (hcc_params & HCC_64BIT_ADDR)
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
    ຖ້າ (! CONFIG_USB_EHCI)
        ສົ່ງຄືນ;
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ຖ້າ (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_EHCI)
            ehci_controller_setup(pci);
    }
}

// Wait for all EHCI controllers to initialize.  This forces OHCI/UHCI
// setup to always be after any EHCI ports are routed to EHCI.
void
ehci_wait_controllers(void)
{
    ໃນຂະນະທີ່ (CONFIG_USB_EHCI && CONFIG_THREADS && PendingEHCI)
        yield();
}


/****************************************************************
 * End point communication
 ****************************************************************/

// Setup fields in qh
static void
ehci_desc2pipe(sa_nracanaa ehci_pipe *pipe, sa_nracanaa usbdevice_s *usbdev
               , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);

    pipe->qh.info1 = ((pipe->pipe.maxpacket << QH_MAXPACKET_SHIFT)
                      | (pipe->pipe.speed << QH_SPEED_SHIFT)
                      | (pipe->pipe.ep << QH_EP_SHIFT)
                      | (pipe->pipe.devaddr << QH_DEVADDR_SHIFT));

    pipe->qh.info2 = (1 << QH_MULT_SHIFT);
    sa_nracanaa usbdevice_s *hubdev = usbdev->hub->usbdev;
    ຖ້າ (hubdev) {
        sa_nracanaa ehci_pipe *hpipe = container_of(
            hubdev->defpipe, sa_nracanaa ehci_pipe, pipe);
        ຖ້າ (hpipe->pipe.speed == USB_HIGHSPEED)
            pipe->qh.info2 |= (((usbdev->port+1) << QH_HUBPORT_SHIFT)
                               | (hpipe->pipe.devaddr << QH_HUBADDR_SHIFT));
        ບໍ່ດັ່ງນັ້ນ
            pipe->qh.info2 = hpipe->qh.info2;
    }

    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ຖ້າ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe->qh.info1 |= ((pipe->pipe.speed != USB_HIGHSPEED ? QH_CONTROL : 0)
                           | QH_TOGGLECONTROL);
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (eptype == USB_ENDPOINT_XFER_INT)
        pipe->qh.info2 |= (0x01 << QH_SMASK_SHIFT) | (0x1c << QH_CMASK_SHIFT);
}

static sa_nracanaa usb_pipe *
ehci_alloc_intr_pipe(sa_nracanaa usbdevice_s *usbdev
                     , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    sa_nracanaa usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_ehci_s, usb);
    ຈຳນວນເຕັມ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "ehci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ຖ້າ (frameexp > 10)
        frameexp = 10;
    ຈຳນວນເຕັມ maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    ຈຳນວນເຕັມ ms = 1<<frameexp;
    ຈຳນວນເຕັມ count = DIV_ROUND_UP(ticks_to_ms(2), ms);
    sa_nracanaa ehci_pipe *pipe = memalign_low(EHCI_QH_ALIGN, parimaa_na(*pipe));
    sa_nracanaa ehci_qtd *tds = memalign_low(EHCI_QTD_ALIGN, parimaa_na(*tds) * count);
    void *data = malloc_low(maxpacket * count);
    ຖ້າ (!pipe || !tds || !data) {
        warn_noalloc();
        goto fail;
    }
    memset(pipe, 0, parimaa_na(*pipe));
    memset(tds, 0, parimaa_na(*tds) * count);
    memset(data, 0, maxpacket * count);
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->next_td = pipe->tds = tds;
    pipe->data = data;
    pipe->qh.qtd_next = (u32)tds;

    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0; i<count; i++) {
        sa_nracanaa ehci_qtd *td = &tds[i];
        td->qtd_next = (i==count-1 ? (u32)tds : (u32)&td[1]);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                     | QTD_PID_IN | ehci_maxerr(3));
        td->buf[0] = (u32)data + maxpacket * i;
    }

    // Add to interrupt schedule.
    sa_nracanaa ehci_framelist *fl = (void*)readl(&cntl->regs->periodiclistbase);
    ຖ້າ (frameexp == 0) {
        // Add to existing interrupt entry.
        sa_nracanaa ehci_qh *intr_qh = (void*)(fl->links[0] & ~EHCI_PTR_BITS);
        pipe->qh.next = intr_qh->next;
        barrier();
        intr_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    } ບໍ່ດັ່ງນັ້ນ {
        ຈຳນວນເຕັມ startpos = 1<<(frameexp-1);
        pipe->qh.next = fl->links[startpos];
        barrier();
        ສຳລັບ (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | EHCI_PTR_QH;
    }

    ສົ່ງຄືນ &pipe->pipe;
fail:
    free(pipe);
    free(tds);
    free(data);
    ສົ່ງຄືນ NULL;
}

sa_nracanaa usb_pipe *
ehci_realloc_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *upipe
                  , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    ຖ້າ (! CONFIG_USB_EHCI)
        ສົ່ງຄືນ NULL;
    usb_add_freelist(upipe);
    ຖ້າ (!epdesc)
        ສົ່ງຄືນ NULL;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ຖ້າ (eptype == USB_ENDPOINT_XFER_INT)
        ສົ່ງຄືນ ehci_alloc_intr_pipe(usbdev, epdesc);
    sa_nracanaa usb_ehci_s *cntl = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_ehci_s, usb);
    dprintf(7, "ehci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    sa_nracanaa usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ຖ້າ (usbpipe) {
        // Use previously allocated pipe.
        sa_nracanaa ehci_pipe *pipe = container_of(usbpipe, sa_nracanaa ehci_pipe, pipe);
        ehci_desc2pipe(pipe, usbdev, epdesc);
        pipe->qh.token = 0;
        ສົ່ງຄືນ usbpipe;
    }

    // Allocate a new queue head.
    sa_nracanaa ehci_pipe *pipe;
    ຖ້າ (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = memalign_tmphigh(EHCI_QH_ALIGN, parimaa_na(*pipe));
    ບໍ່ດັ່ງນັ້ນ
        pipe = memalign_low(EHCI_QH_ALIGN, parimaa_na(*pipe));
    ຖ້າ (!pipe) {
        warn_noalloc();
        ສົ່ງຄືນ NULL;
    }
    memset(pipe, 0, parimaa_na(*pipe));
    ehci_desc2pipe(pipe, usbdev, epdesc);
    pipe->qh.qtd_next = pipe->qh.alt_next = EHCI_PTR_TERM;

    // Add queue head to controller list.
    sa_nracanaa ehci_qh *async_qh = cntl->async_qh;
    pipe->qh.next = async_qh->next;
    barrier();
    async_qh->next = (u32)&pipe->qh | EHCI_PTR_QH;
    ສົ່ງຄືນ &pipe->pipe;
}

static void
ehci_reset_pipe(sa_nracanaa ehci_pipe *pipe)
{
    SET_LOWFLAT(pipe->qh.qtd_next, EHCI_PTR_TERM);
    SET_LOWFLAT(pipe->qh.alt_next, EHCI_PTR_TERM);
    barrier();
    SET_LOWFLAT(pipe->qh.token, GET_LOWFLAT(pipe->qh.token) & QTD_TOGGLE);
}

static ຈຳນວນເຕັມ
ehci_wait_td(sa_nracanaa ehci_pipe *pipe, sa_nracanaa ehci_qtd *td, u32 end)
{
    u32 status;
    ສຳລັບ (;;) {
        status = td->token;
        ຖ້າ (!(status & QTD_STS_ACTIVE))
            bha_nga;
        ຖ້າ (timer_check(end)) {
            u32 cur = GET_LOWFLAT(pipe->qh.current);
            u32 tok = GET_LOWFLAT(pipe->qh.token);
            u32 next = GET_LOWFLAT(pipe->qh.qtd_next);
            warn_timeout();
            dprintf(1, "ehci pipe=%p cur=%08x tok=%08x next=%x td=%p status=%x\n"
                    , pipe, cur, tok, next, td, status);
            ehci_reset_pipe(pipe);
            sa_nracanaa usb_ehci_s *cntl = container_of(
                GET_LOWFLAT(pipe->pipe.cntl), sa_nracanaa usb_ehci_s, usb);
            ehci_waittick(cntl);
            ສົ່ງຄືນ -1;
        }
        yield();
    }
    ຖ້າ (status & QTD_STS_HALT) {
        dprintf(1, "ehci_wait_td error - status=%x\n", status);
        ehci_reset_pipe(pipe);
        ສົ່ງຄືນ -2;
    }
    ສົ່ງຄືນ 0;
}

static void
ehci_fill_tdbuf(sa_nracanaa ehci_qtd *td, u32 dest, ຈຳນວນເຕັມ transfer)
{
    u32 *pos = td->buf, end = dest + transfer;
    ສຳລັບ (; dest < end; dest = ALIGN_DOWN(dest + PAGE_SIZE, PAGE_SIZE))
        *pos++ = dest;
}

#define STACKQTDS 6

ຈຳນວນເຕັມ
ehci_send_pipe(sa_nracanaa usb_pipe *p, ຈຳນວນເຕັມ dir, const void *cmd
               , void *data, ຈຳນວນເຕັມ datasize)
{
    ຖ້າ (! CONFIG_USB_EHCI)
        ສົ່ງຄືນ -1;
    sa_nracanaa ehci_pipe *pipe = container_of(p, sa_nracanaa ehci_pipe, pipe);
    dprintf(7, "ehci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);

    // Allocate tds on stack (with required alignment)
    u8 tdsbuf[parimaa_na(sa_nracanaa ehci_qtd) * STACKQTDS + EHCI_QTD_ALIGN - 1];
    sa_nracanaa ehci_qtd *tds = (void*)ALIGN((u32)tdsbuf, EHCI_QTD_ALIGN), *td = tds;
    memset(tds, 0, parimaa_na(*tds) * STACKQTDS);

    // Setup transfer descriptors
    u16 maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    u32 toggle = 0;
    ຖ້າ (cmd) {
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
    ໃນຂະນະທີ່ (dest < dataend) {
        // Send data pids
        ຖ້າ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ສົ່ງຄືນ -1;
        }
        ຈຳນວນເຕັມ maxtransfer = 5*PAGE_SIZE - (dest & (PAGE_SIZE-1));
        ຈຳນວນເຕັມ transfer = dataend - dest;
        ຖ້າ (transfer > maxtransfer)
            transfer = ALIGN_DOWN(maxtransfer, maxpacket);
        td->qtd_next = (u32)MAKE_FLATPTR(GET_SEG(SS), td+1);
        td->alt_next = EHCI_PTR_TERM;
        td->token = (ehci_explen(transfer) | toggle | QTD_STS_ACTIVE
                     | (dir ? QTD_PID_IN : QTD_PID_OUT) | ehci_maxerr(3));
        ehci_fill_tdbuf(td, dest, transfer);
        td++;
        dest += transfer;
    }
    ຖ້າ (cmd) {
        // Send status pid on control transfers
        ຖ້າ (td >= &tds[STACKQTDS]) {
            warn_noalloc();
            ສົ່ງຄືນ -1;
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
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i=0, td=tds; i<STACKQTDS; i++, td++) {
        ຈຳນວນເຕັມ ret = ehci_wait_td(pipe, td, end);
        ຖ້າ (ret)
            ສົ່ງຄືນ -1;
    }

    ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ
ehci_poll_intr(sa_nracanaa usb_pipe *p, void *data)
{
    ASSERT16();
    ຖ້າ (! CONFIG_USB_EHCI)
        ສົ່ງຄືນ -1;
    sa_nracanaa ehci_pipe *pipe = container_of(p, sa_nracanaa ehci_pipe, pipe);
    sa_nracanaa ehci_qtd *td = GET_LOWFLAT(pipe->next_td);
    u32 token = GET_LOWFLAT(td->token);
    ຖ້າ (token & QTD_STS_ACTIVE)
        // No intrs found.
        ສົ່ງຄືນ -1;
    // XXX - check for errors.

    // Copy data.
    ຈຳນວນເຕັມ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ຈຳນວນເຕັມ pos = td - GET_LOWFLAT(pipe->tds);
    void *tddata = GET_LOWFLAT(pipe->data) + maxpacket * pos;
    memcpy_far(GET_SEG(SS), data, SEG_LOW, LOWFLAT2LOW(tddata), maxpacket);

    // Reenable this td.
    sa_nracanaa ehci_qtd *next = (void*)(GET_LOWFLAT(td->qtd_next) & ~EHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    SET_LOWFLAT(td->buf[0], (u32)tddata);
    barrier();
    SET_LOWFLAT(td->token, (ehci_explen(maxpacket) | QTD_STS_ACTIVE
                            | QTD_PID_IN | ehci_maxerr(3)));

    ສົ່ງຄືນ 0;
}
