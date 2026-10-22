// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "byteorder.h" // be32_to_cpu
#અન્તર્ભાવ "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#અન્તર્ભાવ "hw/tpm_drivers.h" // struct tpm_driver
#અન્તર્ભાવ "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#અન્તર્ભાવ "output.h" // warn_timeout
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "string.h" // memcpy
#અન્તર્ભાવ "util.h" // timer_calc_usec
#અન્તર્ભાવ "x86.h" // readl

/* low level driver implementation */
સંરચના tpm_driver {
    u32 *timeouts;
    u32 *durations;
    શૂન્ય (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(શૂન્ય);
    TPMVersion (*get_tpm_version)(શૂન્ય);
    u32 (*init)(શૂન્ય);
    u32 (*activate)(u8 locty);
    u32 (*ready)(શૂન્ય);
    u32 (*senddata)(અચલ u8 *અચલ data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(શૂન્ય);
    u32 (*waitrespready)(ગણના tpmDurationType to_t);
};

બાહ્ય સંરચના tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_અંક_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

સ્થિર અચલ u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

સ્થિર અચલ u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
સ્થિર u32 tpm_default_dur[3];
સ્થિર u32 tpm_default_to[4];

સ્થિર u32 crb_cmd_size;
સ્થિર શૂન્ય *crb_cmd;
સ્થિર u32 crb_resp_size;
સ્થિર શૂન્ય *crb_resp;

સ્થિર u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    માટે (;;) {
        u8 કિંમત = readl(reg);
        જો ((કિંમત & mask) == expect) {
            rc = 0;
            તોડો;
        }
        જો (timer_check(end)) {
            warn_timeout();
            તોડો;
        }
        yield();
    }
    પાછા rc;
}

સ્થિર u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    પાછા wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

સ્થિર u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    પાછા wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

સ્થિર u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    પાછા wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
સ્થિર u32 tis_probe(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    જો (rc)
        પાછા 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    જો ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    જો ((ifaceid & 0xf) != 0xf) {
        જો ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            પાછા 0;
        }
        જો ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            પાછા 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    પાછા rc;
}

સ્થિર TPMVersion tis_get_tpm_version(શૂન્ય)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    જો ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        જો (((reg >> 28) & 0x7) == 3)
            પાછા TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    નહીંતર જો ((reg & 0xf) == 0) {
        પાછા TPM_VERSION_2;
    }

    પાછા TPM_VERSION_1_2;
}

સ્થિર શૂન્ય init_timeout(પૂર્ણાંક driver)
{
    જો (tpm_drivers[driver].durations == ખાલી) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               કદ(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    જો (tpm_drivers[driver].timeouts == ખાલી) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               કદ(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

સ્થિર u32 tis_init(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    પાછા 1;
}


સ્થિર શૂન્ય set_timeouts(u32 timeouts[4], u32 durations[3])
{
    જો (!CONFIG_TCGBIOS)
        પાછા;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    જો (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * કદ(u32));
    જો (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * કદ(u32));
}

સ્થિર u32 tis_activate(u8 locty)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 0;
    u8 acc;
    પૂર્ણાંક l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    જો (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        માટે (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    જો ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    પાછા rc;
}

સ્થિર u32 tis_find_active_locality(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u8 locty;

    માટે (locty = 0; locty <= 4; locty++) {
        જો ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            પાછા locty;
    }

    tis_activate(0);

    પાછા 0;
}

સ્થિર u32 tis_ready(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    પાછા rc;
}

સ્થિર u32 tis_senddata(અચલ u8 *અચલ data, u32 len)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    કરો {
        જ્યારે (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            જો (burst == 0) {
                જો (timer_check(end)) {
                    warn_timeout();
                    તોડો;
                }
                yield();
            }
        }

        જો (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            તોડો;
        }

        જ્યારે (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            જો (burst == 0 || offset == len)
                તોડો;
        }

        જો (offset == len)
            end_loop = 1;
    } જ્યારે (end_loop == 0);

    પાછા rc;
}

સ્થિર u32 tis_readresp(u8 *buffer, u32 *len)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    જ્યારે (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        જો ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            તોડો;
    }

    *len = offset;

    પાછા rc;
}


સ્થિર u32 tis_waitdatavalid(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    જો (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    પાછા rc;
}

સ્થિર u32 tis_waitrespready(ગણના tpmDurationType to_t)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    જો (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    પાછા rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
સ્થિર u32 crb_probe(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    જો (rc)
        પાછા 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    જો ((ifaceid & 0xf) != 0xf) {
        જો ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } નહીંતર જો ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            પાછા 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    જો (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        પાછા 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    જો (addr > 0xffffffff)
        પાછા 0;

    પાછા 1;
}

સ્થિર TPMVersion crb_get_tpm_version(શૂન્ય)
{
    /* CRB is supposed to be TPM 2.0 only */
    પાછા TPM_VERSION_2;
}

સ્થિર u32 crb_init(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 1;

    crb_cmd = (શૂન્ય*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (શૂન્ય*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    પાછા 0;
}

સ્થિર u32 crb_activate(u8 locty)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    પાછા 0;
}

સ્થિર u32 crb_find_active_locality(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    પાછા 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

સ્થિર u32 crb_ready(શૂન્ય)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    પાછા rc;
}

સ્થિર u32 crb_senddata(અચલ u8 *અચલ data, u32 len)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    જો (len > crb_cmd_size)
        પાછા 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    પાછા 0;
}

સ્થિર u32 crb_readresp(u8 *buffer, u32 *len)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u8 locty = crb_find_active_locality();
    જો (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        પાછા 1;

    જો (*len < 6)
        પાછા 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    જો (expected < 6)
        પાછા 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    પાછા 0;
}


સ્થિર u32 crb_waitdatavalid(શૂન્ય)
{
    પાછા 0;
}

સ્થિર u32 crb_waitrespready(ગણના tpmDurationType to_t)
{
    જો (!CONFIG_TCGBIOS)
        પાછા 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    પાછા rc;
}

સંરચના tpm_driver tpm_drivers[TPM_અંક_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = ખાલી,
            .durations     = ખાલી,
            .set_timeouts  = set_timeouts,
            .probe         = tis_probe,
            .get_tpm_version = tis_get_tpm_version,
            .init          = tis_init,
            .activate      = tis_activate,
            .ready         = tis_ready,
            .senddata      = tis_senddata,
            .readresp      = tis_readresp,
            .waitdatavalid = tis_waitdatavalid,
            .waitrespready = tis_waitrespready,
        },
    [CRB_DRIVER_IDX] =
        {
            .timeouts      = ખાલી,
            .durations     = ખાલી,
            .set_timeouts  = set_timeouts,
            .probe         = crb_probe,
            .get_tpm_version = crb_get_tpm_version,
            .init          = crb_init,
            .activate      = crb_activate,
            .ready         = crb_ready,
            .senddata      = crb_senddata,
            .readresp      = crb_readresp,
            .waitdatavalid = crb_waitdatavalid,
            .waitrespready = crb_waitrespready,
        },
};

સ્થિર u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(શૂન્ય)
{
    unsigned પૂર્ણાંક i;
    માટે (i = 0; i < TPM_અંક_DRIVERS; i++) {
        સંરચના tpm_driver *td = &tpm_drivers[i];
        જો (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            પાછા td->get_tpm_version();
        }
    }
    પાછા TPM_VERSION_NONE;
}

પૂર્ણાંક
tpmhw_is_present(શૂન્ય)
{
    પાછા TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

પૂર્ણાંક
tpmhw_transmit(u8 locty, સંરચના tpm_req_header *req,
               શૂન્ય *respbuffer, u32 *respbufferlen,
               ગણના tpmDurationType to_t)
{
    જો (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        પાછા -1;

    સંરચના tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    જો (irc != 0) {
        /* tpm could not be activated */
        પાછા -1;
    }

    irc = td->senddata((શૂન્ય*)req, be32_to_cpu(req->totlen));
    જો (irc != 0)
        પાછા -1;

    irc = td->waitdatavalid();
    જો (irc != 0)
        પાછા -1;

    irc = td->waitrespready(to_t);
    જો (irc != 0)
        પાછા -1;

    irc = td->readresp(respbuffer, respbufferlen);
    જો (irc != 0 ||
        *respbufferlen < કદ(સંરચના tpm_rsp_header))
        પાછા -1;

    td->ready();

    પાછા 0;
}

શૂન્ય
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    સંરચના tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
