// Code for handling UHCI USB controllers.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_LOWFLAT
#include "config.h" // CONFIG_*
#include "malloc.h" // free
#include "output.h" // dprintf
#include "pci.h" // pci_config_writew
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_CLASS_SERIAL_USB_UHCI
#include "pci_regs.h" // PCI_BASE_ADDRESS_4
#include "string.h" // memset
#include "usb.h" // struct usb_s
#include "usb-ehci.h" // ehci_wait_controllers
#include "usb-uhci.h" // USBLEGSUP
#include "util.h" // msleep
#include "x86.h" // outw

sa_nracanaa usb_uhci_s {
    sa_nracanaa usb_s usb;
    u16 iobase;
    sa_nracanaa uhci_qh *control_qh;
    sa_nracanaa uhci_framelist *framelist;
};

sa_nracanaa uhci_pipe {
    sa_nracanaa uhci_qh qh;
    sa_nracanaa uhci_td *next_td;
    sa_nracanaa usb_pipe pipe;
    u16 iobase;
    u8 toggle;
};


/****************************************************************
 * Root hub
 ****************************************************************/

// Check if device attached to a given port
static จำนวนเต็ม
uhci_hub_detect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_uhci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    ถ้า (!(status & USBPORTSC_CCS))
        // No device found.
        คืน 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    คืน 1;
}

// Reset device on port
static จำนวนเต็ม
uhci_hub_reset(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_uhci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    ถ้า (!(status & USBPORTSC_CCS))
        // No longer connected
        คืน -1;
    outw(USBPORTSC_PE, ioport);
    คืน !!(status & USBPORTSC_LSDA);
}

// Disable port
static void
uhci_hub_disconnect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_uhci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    outw(0, ioport);
}

static sa_nracanaa usbhub_op_s uhci_HubOp = {
    .detect = uhci_hub_detect,
    .reset = uhci_hub_reset,
    .disconnect = uhci_hub_disconnect,
};

// Find any devices connected to the root hub.
static จำนวนเต็ม
check_uhci_ports(sa_nracanaa usb_uhci_s *cntl)
{
    ASSERT32FLAT();
    // Wait for ehci init - in case this is a "companion controller"
    ehci_wait_controllers();

    sa_nracanaa usbhub_s hub;
    memset(&hub, 0, parimaa_na(hub));
    hub.cntl = &cntl->usb;
    hub.portcount = 2;
    hub.op = &uhci_HubOp;
    usb_enumerate(&hub);
    คืน hub.devcount;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Wait for next USB frame to start - for ensuring safe memory release.
static void
uhci_waittick(u16 iobase)
{
    barrier();
    u16 startframe = inw(iobase + USBFRNUM);
    u32 end = timer_calc(1000 * 5);
    สำหรับ (;;) {
        ถ้า (inw(iobase + USBFRNUM) != startframe)
            bha_nga;
        ถ้า (timer_check(end)) {
            warn_timeout();
            คืน;
        }
        yield();
    }
}

static void
uhci_free_pipes(sa_nracanaa usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    sa_nracanaa uhci_qh *pos = (void*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    สำหรับ (;;) {
        u32 link = pos->link;
        ถ้า (link == UHCI_PTR_TERM)
            bha_nga;
        sa_nracanaa uhci_qh *next = (void*)(link & ~UHCI_PTR_BITS);
        sa_nracanaa uhci_pipe *pipe = container_of(next, sa_nracanaa uhci_pipe, qh);
        ถ้า (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        มิฉะนั้น
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    สำหรับ (;;) {
        sa_nracanaa usb_pipe *usbpipe = cntl->usb.freelist;
        ถ้า (!usbpipe)
            bha_nga;
        cntl->usb.freelist = usbpipe->freenext;
        sa_nracanaa uhci_pipe *pipe = container_of(usbpipe, sa_nracanaa uhci_pipe, pipe);
        free(pipe);
    }
}

static void
reset_uhci(sa_nracanaa usb_uhci_s *cntl, u16 bdf)
{
    // XXX - don't reset if not needed.

    // Reset PIRQ and SMI
    pci_config_writew(bdf, USBLEGSUP, USBLEGSUP_RWC);

    // Reset the HC
    outw(USBCMD_HCRESET, cntl->iobase + USBCMD);
    udelay(5);

    // Disable interrupts and commands (just to be safe).
    outw(0, cntl->iobase + USBINTR);
    outw(0, cntl->iobase + USBCMD);
}

static void
configure_uhci(void *data)
{
    sa_nracanaa usb_uhci_s *cntl = data;

    // Allocate ram for schedule storage
    sa_nracanaa uhci_td *term_td = malloc_high(parimaa_na(*term_td));
    sa_nracanaa uhci_framelist *fl = memalign_high(parimaa_na(*fl), parimaa_na(*fl));
    sa_nracanaa uhci_pipe *intr_pipe = malloc_high(parimaa_na(*intr_pipe));
    sa_nracanaa uhci_pipe *term_pipe = malloc_high(parimaa_na(*term_pipe));
    ถ้า (!term_td || !fl || !intr_pipe || !term_pipe) {
        warn_noalloc();
        goto fail;
    }

    // Work around for PIIX errata
    memset(term_td, 0, parimaa_na(*term_td));
    term_td->link = UHCI_PTR_TERM;
    term_td->token = (uhci_explen(0) | (0x7f << TD_TOKEN_DEVADDR_SHIFT)
                      | USB_PID_IN);
    memset(term_pipe, 0, parimaa_na(*term_pipe));
    term_pipe->qh.element = (u32)term_td;
    term_pipe->qh.link = UHCI_PTR_TERM;
    term_pipe->pipe.cntl = &cntl->usb;

    // Set schedule to point to primary intr queue head
    memset(intr_pipe, 0, parimaa_na(*intr_pipe));
    intr_pipe->qh.element = UHCI_PTR_TERM;
    intr_pipe->qh.link = (u32)&term_pipe->qh | UHCI_PTR_QH;
    intr_pipe->pipe.cntl = &cntl->usb;
    จำนวนเต็ม i;
    สำหรับ (i=0; i<ARRAY_SIZE(fl->links); i++)
        fl->links[i] = (u32)&intr_pipe->qh | UHCI_PTR_QH;
    cntl->framelist = fl;
    cntl->control_qh = &intr_pipe->qh;
    barrier();

    // Set the frame length to the default: 1 ms exactly
    outb(USBSOF_DEFAULT, cntl->iobase + USBSOF);

    // Store the frame list base address
    outl((u32)fl->links, cntl->iobase + USBFLBASEADD);

    // Set the current frame number
    outw(0, cntl->iobase + USBFRNUM);

    // Mark as configured and running with a 64-byte max packet.
    outw(USBCMD_RS | USBCMD_CF | USBCMD_MAXP, cntl->iobase + USBCMD);

    // Find devices
    จำนวนเต็ม count = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    ถ้า (count)
        // Success
        คืน;

    // No devices found - shutdown and free controller.
    outw(0, cntl->iobase + USBCMD);
fail:
    free(term_td);
    free(fl);
    free(intr_pipe);
    free(term_pipe);
    free(cntl);
}

static void
uhci_controller_setup(sa_nracanaa pci_device *pci)
{
    u16 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
    ถ้า (!iobase)
        คืน;

    sa_nracanaa usb_uhci_s *cntl = malloc_tmphigh(parimaa_na(*cntl));
    ถ้า (!cntl) {
        warn_noalloc();
        คืน;
    }
    memset(cntl, 0, parimaa_na(*cntl));
    cntl->usb.pci = pci;
    cntl->usb.type = USB_TYPE_UHCI;
    cntl->iobase = iobase;

    dprintf(1, "UHCI init on dev %pP (io=%x)\n", pci, cntl->iobase);

    pci_enable_busmaster(pci);

    reset_uhci(cntl, pci->bdf);

    run_thread(configure_uhci, cntl);
}

void
uhci_setup(void)
{
    ถ้า (! CONFIG_USB_UHCI)
        คืน;
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ถ้า (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
            uhci_controller_setup(pci);
    }
}


/****************************************************************
 * End point communication
 ****************************************************************/

static sa_nracanaa usb_pipe *
uhci_alloc_intr_pipe(sa_nracanaa usbdevice_s *usbdev
                     , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    sa_nracanaa usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_uhci_s, usb);
    จำนวนเต็ม frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    ถ้า (frameexp > 10)
        frameexp = 10;
    จำนวนเต็ม maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    จำนวนเต็ม ms = 1<<frameexp;
    จำนวนเต็ม count = DIV_ROUND_UP(ticks_to_ms(2), ms);
    count = ALIGN(count, 2);
    sa_nracanaa uhci_pipe *pipe = malloc_low(parimaa_na(*pipe));
    sa_nracanaa uhci_td *tds = malloc_low(parimaa_na(*tds) * count);
    void *data = malloc_low(maxpacket * count);
    ถ้า (!pipe || !tds || !data) {
        warn_noalloc();
        goto fail;
    }
    memset(pipe, 0, parimaa_na(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    จำนวนเต็ม lowspeed = pipe->pipe.speed;
    จำนวนเต็ม devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    จำนวนเต็ม toggle = 0;
    จำนวนเต็ม i;
    สำหรับ (i=0; i<count; i++) {
        tds[i].link = (i==count-1 ? (u32)&tds[0] : (u32)&tds[i+1]);
        tds[i].status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                         | TD_CTRL_ACTIVE);
        tds[i].token = (uhci_explen(maxpacket) | toggle
                        | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                        | USB_PID_IN);
        tds[i].buffer = data + maxpacket * i;
        toggle ^= TD_TOKEN_TOGGLE;
    }

    // Add to interrupt schedule.
    sa_nracanaa uhci_framelist *fl = cntl->framelist;
    ถ้า (frameexp == 0) {
        // Add to existing interrupt entry.
        sa_nracanaa uhci_qh *intr_qh = (void*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        ถ้า (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } มิฉะนั้น {
        จำนวนเต็ม startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        สำหรับ (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    คืน &pipe->pipe;
fail:
    free(pipe);
    free(tds);
    free(data);
    คืน NULL;
}

sa_nracanaa usb_pipe *
uhci_realloc_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *upipe
                  , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    ถ้า (! CONFIG_USB_UHCI)
        คืน NULL;
    usb_add_freelist(upipe);
    ถ้า (!epdesc)
        คืน NULL;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    ถ้า (eptype == USB_ENDPOINT_XFER_INT)
        คืน uhci_alloc_intr_pipe(usbdev, epdesc);
    sa_nracanaa usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    sa_nracanaa usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    ถ้า (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        คืน usbpipe;
    }

    // Allocate a new queue head.
    sa_nracanaa uhci_pipe *pipe;
    ถ้า (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(parimaa_na(*pipe));
    มิฉะนั้น
        pipe = malloc_low(parimaa_na(*pipe));
    ถ้า (!pipe) {
        warn_noalloc();
        คืน NULL;
    }
    memset(pipe, 0, parimaa_na(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    pipe->qh.element = UHCI_PTR_TERM;
    pipe->iobase = cntl->iobase;

    // Add queue head to controller list.
    sa_nracanaa uhci_qh *control_qh = cntl->control_qh;
    pipe->qh.link = control_qh->link;
    barrier();
    control_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
    ถ้า (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    คืน &pipe->pipe;
}

static จำนวนเต็ม
wait_pipe(sa_nracanaa uhci_pipe *pipe, u32 end)
{
    สำหรับ (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        ถ้า (el_link & UHCI_PTR_TERM)
            คืน 0;
        ถ้า (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            sa_nracanaa uhci_td *td = (void*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (void*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            คืน -1;
        }
        yield();
    }
}

static จำนวนเต็ม
wait_td(sa_nracanaa uhci_td *td, u32 end)
{
    u32 status;
    สำหรับ (;;) {
        status = td->status;
        ถ้า (!(status & TD_CTRL_ACTIVE))
            bha_nga;
        ถ้า (timer_check(end)) {
            warn_timeout();
            คืน -1;
        }
        yield();
    }
    ถ้า (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        คืน -2;
    }
    คืน 0;
}

#define STACKTDS 16
#define TDALIGN 16

จำนวนเต็ม
uhci_send_pipe(sa_nracanaa usb_pipe *p, จำนวนเต็ม dir, const void *cmd
               , void *data, จำนวนเต็ม datasize)
{
    ถ้า (! CONFIG_USB_UHCI)
        คืน -1;
    sa_nracanaa uhci_pipe *pipe = container_of(p, sa_nracanaa uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    จำนวนเต็ม maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    จำนวนเต็ม lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    จำนวนเต็ม devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    จำนวนเต็ม toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[parimaa_na(sa_nracanaa uhci_td) * STACKTDS + TDALIGN - 1];
    sa_nracanaa uhci_td *tds = (void*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, parimaa_na(*tds) * STACKTDS);
    จำนวนเต็ม tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    ถ้า (cmd) {
        // Send setup pid on control transfers
        sa_nracanaa uhci_td *td = &tds[tdpos++ % STACKTDS];
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = nexttd | UHCI_PTR_DEPTH;
        td->token = (uhci_explen(USB_CONTROL_SETUP_SIZE)
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_SETUP);
        td->buffer = (void*)cmd;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle = TD_TOKEN_TOGGLE;
    }
    ขณะ (datasize) {
        // Send data pids
        sa_nracanaa uhci_td *td = &tds[tdpos++ % STACKTDS];
        จำนวนเต็ม ret = wait_td(td, end);
        ถ้า (ret)
            goto fail;

        จำนวนเต็ม transfer = datasize;
        ถ้า (transfer > maxpacket)
            transfer = maxpacket;
        u32 nexttd = (u32)MAKE_FLATPTR(GET_SEG(SS), &tds[tdpos % STACKTDS]);
        td->link = ((transfer==datasize && !cmd)
                    ? UHCI_PTR_TERM : (nexttd | UHCI_PTR_DEPTH));
        td->token = (uhci_explen(transfer) | toggle
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_IN : USB_PID_OUT));
        td->buffer = data;
        barrier();
        td->status = (uhci_maxerr(3) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
        toggle ^= TD_TOKEN_TOGGLE;

        data += transfer;
        datasize -= transfer;
    }
    ถ้า (cmd) {
        // Send status pid on control transfers
        sa_nracanaa uhci_td *td = &tds[tdpos++ % STACKTDS];
        จำนวนเต็ม ret = wait_td(td, end);
        ถ้า (ret)
            goto fail;
        td->link = UHCI_PTR_TERM;
        td->token = (uhci_explen(0) | TD_TOKEN_TOGGLE
                     | (devaddr << TD_TOKEN_DEVADDR_SHIFT)
                     | (dir ? USB_PID_OUT : USB_PID_IN));
        td->buffer = 0;
        barrier();
        td->status = (uhci_maxerr(0) | (lowspeed ? TD_CTRL_LS : 0)
                      | TD_CTRL_ACTIVE);
    }
    SET_LOWFLAT(pipe->toggle, !!toggle);
    คืน wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    คืน -1;
}

จำนวนเต็ม
uhci_poll_intr(sa_nracanaa usb_pipe *p, void *data)
{
    ASSERT16();
    ถ้า (! CONFIG_USB_UHCI)
        คืน -1;

    sa_nracanaa uhci_pipe *pipe = container_of(p, sa_nracanaa uhci_pipe, pipe);
    sa_nracanaa uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    ถ้า (status & TD_CTRL_ACTIVE)
        // No intrs found.
        คืน -1;
    // XXX - check for errors.

    // Copy data.
    void *tddata = GET_LOWFLAT(td->buffer);
    memcpy_far(GET_SEG(SS), data, SEG_LOW, LOWFLAT2LOW(tddata)
               , uhci_expected_length(token));

    // Reenable this td.
    sa_nracanaa uhci_td *next = (void*)(GET_LOWFLAT(td->link) & ~UHCI_PTR_BITS);
    SET_LOWFLAT(pipe->next_td, next);
    barrier();
    SET_LOWFLAT(td->status, (uhci_maxerr(0) | (status & TD_CTRL_LS)
                             | TD_CTRL_ACTIVE));

    คืน 0;
}
