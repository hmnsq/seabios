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
static ཧྲིལ
uhci_hub_detect(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_uhci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;
    u16 status = inw(ioport);
    གལ་སྲིད (!(status & USBPORTSC_CCS))
        // No device found.
        ལོག 0;

    // XXX - if just powered up, need to wait for USB_TIME_ATTDB?

    // Begin reset on port
    outw(USBPORTSC_PR, ioport);
    msleep(USB_TIME_DRSTR);
    ལོག 1;
}

// Reset device on port
static ཧྲིལ
uhci_hub_reset(sa_nracanaa usbhub_s *hub, u32 port)
{
    sa_nracanaa usb_uhci_s *cntl = container_of(hub->cntl, sa_nracanaa usb_uhci_s, usb);
    u16 ioport = cntl->iobase + USBPORTSC1 + port * 2;

    // Finish reset on port
    outw(0, ioport);
    udelay(6); // 64 high-speed bit times
    u16 status = inw(ioport);
    གལ་སྲིད (!(status & USBPORTSC_CCS))
        // No longer connected
        ལོག -1;
    outw(USBPORTSC_PE, ioport);
    ལོག !!(status & USBPORTSC_LSDA);
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
static ཧྲིལ
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
    ལོག hub.devcount;
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
    དོན་ལུ (;;) {
        གལ་སྲིད (inw(iobase + USBFRNUM) != startframe)
            bha_nga;
        གལ་སྲིད (timer_check(end)) {
            warn_timeout();
            ལོག;
        }
        yield();
    }
}

static void
uhci_free_pipes(sa_nracanaa usb_uhci_s *cntl)
{
    dprintf(7, "uhci_free_pipes %p\n", cntl);

    sa_nracanaa uhci_qh *pos = (void*)(cntl->framelist->links[0] & ~UHCI_PTR_BITS);
    དོན་ལུ (;;) {
        u32 link = pos->link;
        གལ་སྲིད (link == UHCI_PTR_TERM)
            bha_nga;
        sa_nracanaa uhci_qh *next = (void*)(link & ~UHCI_PTR_BITS);
        sa_nracanaa uhci_pipe *pipe = container_of(next, sa_nracanaa uhci_pipe, qh);
        གལ་སྲིད (usb_is_freelist(&cntl->usb, &pipe->pipe))
            pos->link = next->link;
        དེ་མིན
            pos = next;
    }
    uhci_waittick(cntl->iobase);
    དོན་ལུ (;;) {
        sa_nracanaa usb_pipe *usbpipe = cntl->usb.freelist;
        གལ་སྲིད (!usbpipe)
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
    གལ་སྲིད (!term_td || !fl || !intr_pipe || !term_pipe) {
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
    ཧྲིལ i;
    དོན་ལུ (i=0; i<ARRAY_SIZE(fl->links); i++)
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
    ཧྲིལ count = check_uhci_ports(cntl);
    uhci_free_pipes(cntl);
    གལ་སྲིད (count)
        // Success
        ལོག;

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
    གལ་སྲིད (!iobase)
        ལོག;

    sa_nracanaa usb_uhci_s *cntl = malloc_tmphigh(parimaa_na(*cntl));
    གལ་སྲིད (!cntl) {
        warn_noalloc();
        ལོག;
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
    གལ་སྲིད (! CONFIG_USB_UHCI)
        ལོག;
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        གལ་སྲིད (pci_classprog(pci) == PCI_CLASS_SERIAL_USB_UHCI)
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
    ཧྲིལ frameexp = usb_get_period(usbdev, epdesc);
    dprintf(7, "uhci_alloc_intr_pipe %p %d\n", &cntl->usb, frameexp);

    གལ་སྲིད (frameexp > 10)
        frameexp = 10;
    ཧྲིལ maxpacket = epdesc->wMaxPacketSize;
    // Determine number of entries needed for 2 timer ticks.
    ཧྲིལ ms = 1<<frameexp;
    ཧྲིལ count = DIV_ROUND_UP(ticks_to_ms(2), ms);
    count = ALIGN(count, 2);
    sa_nracanaa uhci_pipe *pipe = malloc_low(parimaa_na(*pipe));
    sa_nracanaa uhci_td *tds = malloc_low(parimaa_na(*tds) * count);
    void *data = malloc_low(maxpacket * count);
    གལ་སྲིད (!pipe || !tds || !data) {
        warn_noalloc();
        goto fail;
    }
    memset(pipe, 0, parimaa_na(*pipe));
    usb_desc2pipe(&pipe->pipe, usbdev, epdesc);
    ཧྲིལ lowspeed = pipe->pipe.speed;
    ཧྲིལ devaddr = pipe->pipe.devaddr | (pipe->pipe.ep << 7);
    pipe->qh.element = (u32)tds;
    pipe->next_td = &tds[0];
    pipe->iobase = cntl->iobase;

    ཧྲིལ toggle = 0;
    ཧྲིལ i;
    དོན་ལུ (i=0; i<count; i++) {
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
    གལ་སྲིད (frameexp == 0) {
        // Add to existing interrupt entry.
        sa_nracanaa uhci_qh *intr_qh = (void*)(fl->links[0] & ~UHCI_PTR_BITS);
        pipe->qh.link = intr_qh->link;
        barrier();
        intr_qh->link = (u32)&pipe->qh | UHCI_PTR_QH;
        གལ་སྲིད (cntl->control_qh == intr_qh)
            cntl->control_qh = &pipe->qh;
    } དེ་མིན {
        ཧྲིལ startpos = 1<<(frameexp-1);
        pipe->qh.link = fl->links[startpos];
        barrier();
        དོན་ལུ (i=startpos; i<ARRAY_SIZE(fl->links); i+=ms)
            fl->links[i] = (u32)&pipe->qh | UHCI_PTR_QH;
    }

    ལོག &pipe->pipe;
fail:
    free(pipe);
    free(tds);
    free(data);
    ལོག NULL;
}

sa_nracanaa usb_pipe *
uhci_realloc_pipe(sa_nracanaa usbdevice_s *usbdev, sa_nracanaa usb_pipe *upipe
                  , sa_nracanaa usb_endpoint_descriptor *epdesc)
{
    གལ་སྲིད (! CONFIG_USB_UHCI)
        ལོག NULL;
    usb_add_freelist(upipe);
    གལ་སྲིད (!epdesc)
        ལོག NULL;
    u8 eptype = epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
    གལ་སྲིད (eptype == USB_ENDPOINT_XFER_INT)
        ལོག uhci_alloc_intr_pipe(usbdev, epdesc);
    sa_nracanaa usb_uhci_s *cntl = container_of(
        usbdev->hub->cntl, sa_nracanaa usb_uhci_s, usb);
    dprintf(7, "uhci_alloc_async_pipe %p %d\n", &cntl->usb, eptype);

    sa_nracanaa usb_pipe *usbpipe = usb_get_freelist(&cntl->usb, eptype);
    གལ་སྲིད (usbpipe) {
        // Use previously allocated pipe.
        usb_desc2pipe(usbpipe, usbdev, epdesc);
        ལོག usbpipe;
    }

    // Allocate a new queue head.
    sa_nracanaa uhci_pipe *pipe;
    གལ་སྲིད (eptype == USB_ENDPOINT_XFER_CONTROL)
        pipe = malloc_tmphigh(parimaa_na(*pipe));
    དེ་མིན
        pipe = malloc_low(parimaa_na(*pipe));
    གལ་སྲིད (!pipe) {
        warn_noalloc();
        ལོག NULL;
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
    གལ་སྲིད (eptype == USB_ENDPOINT_XFER_CONTROL)
        cntl->control_qh = &pipe->qh;
    ལོག &pipe->pipe;
}

static ཧྲིལ
wait_pipe(sa_nracanaa uhci_pipe *pipe, u32 end)
{
    དོན་ལུ (;;) {
        u32 el_link = GET_LOWFLAT(pipe->qh.element);
        གལ་སྲིད (el_link & UHCI_PTR_TERM)
            ལོག 0;
        གལ་སྲིད (timer_check(end)) {
            warn_timeout();
            u16 iobase = GET_LOWFLAT(pipe->iobase);
            sa_nracanaa uhci_td *td = (void*)(el_link & ~UHCI_PTR_BITS);
            dprintf(1, "Timeout on wait_pipe %p (td=%p s=%x c=%x/%x)\n"
                    , pipe, (void*)el_link, GET_LOWFLAT(td->status)
                    , inw(iobase + USBCMD)
                    , inw(iobase + USBSTS));
            SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
            uhci_waittick(iobase);
            ལོག -1;
        }
        yield();
    }
}

static ཧྲིལ
wait_td(sa_nracanaa uhci_td *td, u32 end)
{
    u32 status;
    དོན་ལུ (;;) {
        status = td->status;
        གལ་སྲིད (!(status & TD_CTRL_ACTIVE))
            bha_nga;
        གལ་སྲིད (timer_check(end)) {
            warn_timeout();
            ལོག -1;
        }
        yield();
    }
    གལ་སྲིད (status & TD_CTRL_ANY_ERROR) {
        dprintf(1, "wait_td error - status=%x\n", status);
        ལོག -2;
    }
    ལོག 0;
}

#define STACKTDS 16
#define TDALIGN 16

ཧྲིལ
uhci_send_pipe(sa_nracanaa usb_pipe *p, ཧྲིལ dir, const void *cmd
               , void *data, ཧྲིལ datasize)
{
    གལ་སྲིད (! CONFIG_USB_UHCI)
        ལོག -1;
    sa_nracanaa uhci_pipe *pipe = container_of(p, sa_nracanaa uhci_pipe, pipe);
    dprintf(7, "uhci_send_pipe qh=%p dir=%d data=%p size=%d\n"
            , &pipe->qh, dir, data, datasize);
    ཧྲིལ maxpacket = GET_LOWFLAT(pipe->pipe.maxpacket);
    ཧྲིལ lowspeed = GET_LOWFLAT(pipe->pipe.speed);
    ཧྲིལ devaddr = (GET_LOWFLAT(pipe->pipe.devaddr)
                   | (GET_LOWFLAT(pipe->pipe.ep) << 7));
    ཧྲིལ toggle = GET_LOWFLAT(pipe->toggle) ? TD_TOKEN_TOGGLE : 0;

    // Allocate 16 tds on stack (16byte aligned)
    u8 tdsbuf[parimaa_na(sa_nracanaa uhci_td) * STACKTDS + TDALIGN - 1];
    sa_nracanaa uhci_td *tds = (void*)ALIGN((u32)tdsbuf, TDALIGN);
    memset(tds, 0, parimaa_na(*tds) * STACKTDS);
    ཧྲིལ tdpos = 0;

    // Enable tds
    u32 end = timer_calc(usb_xfer_time(p, datasize));
    barrier();
    SET_LOWFLAT(pipe->qh.element, (u32)MAKE_FLATPTR(GET_SEG(SS), tds));

    // Setup transfer descriptors
    གལ་སྲིད (cmd) {
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
    སྐབས (datasize) {
        // Send data pids
        sa_nracanaa uhci_td *td = &tds[tdpos++ % STACKTDS];
        ཧྲིལ ret = wait_td(td, end);
        གལ་སྲིད (ret)
            goto fail;

        ཧྲིལ transfer = datasize;
        གལ་སྲིད (transfer > maxpacket)
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
    གལ་སྲིད (cmd) {
        // Send status pid on control transfers
        sa_nracanaa uhci_td *td = &tds[tdpos++ % STACKTDS];
        ཧྲིལ ret = wait_td(td, end);
        གལ་སྲིད (ret)
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
    ལོག wait_pipe(pipe, end);
fail:
    dprintf(1, "uhci_send_bulk failed\n");
    SET_LOWFLAT(pipe->qh.element, UHCI_PTR_TERM);
    uhci_waittick(GET_LOWFLAT(pipe->iobase));
    ལོག -1;
}

ཧྲིལ
uhci_poll_intr(sa_nracanaa usb_pipe *p, void *data)
{
    ASSERT16();
    གལ་སྲིད (! CONFIG_USB_UHCI)
        ལོག -1;

    sa_nracanaa uhci_pipe *pipe = container_of(p, sa_nracanaa uhci_pipe, pipe);
    sa_nracanaa uhci_td *td = GET_LOWFLAT(pipe->next_td);
    u32 status = GET_LOWFLAT(td->status);
    u32 token = GET_LOWFLAT(td->token);
    གལ་སྲིད (status & TD_CTRL_ACTIVE)
        // No intrs found.
        ལོག -1;
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

    ལོག 0;
}
