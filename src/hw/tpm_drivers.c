// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "byteorder.h" // be32_to_cpu
#അന്തര്ഭാവ "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#അന്തര്ഭാവ "hw/tpm_drivers.h" // struct tpm_driver
#അന്തര്ഭാവ "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#അന്തര്ഭാവ "output.h" // warn_timeout
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "util.h" // timer_calc_usec
#അന്തര്ഭാവ "x86.h" // readl

/* low level driver implementation */
ഘടന tpm_driver {
    u32 *timeouts;
    u32 *durations;
    ശൂന്യം (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(ശൂന്യം);
    TPMVersion (*get_tpm_version)(ശൂന്യം);
    u32 (*init)(ശൂന്യം);
    u32 (*activate)(u8 locty);
    u32 (*ready)(ശൂന്യം);
    u32 (*senddata)(സ്ഥിരം u8 *സ്ഥിരം data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(ശൂന്യം);
    u32 (*waitrespready)(എണ്ണൽ tpmDurationType to_t);
};

ബാഹ്യ ഘടന tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_അക്കം_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

സ്ഥിതം സ്ഥിരം u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

സ്ഥിതം സ്ഥിരം u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
സ്ഥിതം u32 tpm_default_dur[3];
സ്ഥിതം u32 tpm_default_to[4];

സ്ഥിതം u32 crb_cmd_size;
സ്ഥിതം ശൂന്യം *crb_cmd;
സ്ഥിതം u32 crb_resp_size;
സ്ഥിതം ശൂന്യം *crb_resp;

സ്ഥിതം u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    വേണ്ടി (;;) {
        u8 മൂല്യം = readl(reg);
        എങ്കിൽ ((മൂല്യം & mask) == expect) {
            rc = 0;
            നിർത്തുക;
        }
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            നിർത്തുക;
        }
        yield();
    }
    തിരികെ rc;
}

സ്ഥിതം u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    തിരികെ wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

സ്ഥിതം u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    തിരികെ wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

സ്ഥിതം u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    തിരികെ wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
സ്ഥിതം u32 tis_probe(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    എങ്കിൽ (rc)
        തിരികെ 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    എങ്കിൽ ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    എങ്കിൽ ((ifaceid & 0xf) != 0xf) {
        എങ്കിൽ ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            തിരികെ 0;
        }
        എങ്കിൽ ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            തിരികെ 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    തിരികെ rc;
}

സ്ഥിതം TPMVersion tis_get_tpm_version(ശൂന്യം)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    എങ്കിൽ ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        എങ്കിൽ (((reg >> 28) & 0x7) == 3)
            തിരികെ TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    അല്ലെങ്കിൽ എങ്കിൽ ((reg & 0xf) == 0) {
        തിരികെ TPM_VERSION_2;
    }

    തിരികെ TPM_VERSION_1_2;
}

സ്ഥിതം ശൂന്യം init_timeout(പൂർണ്ണസംഖ്യ driver)
{
    എങ്കിൽ (tpm_drivers[driver].durations == ശൂന്യ) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               വലുപ്പം(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    എങ്കിൽ (tpm_drivers[driver].timeouts == ശൂന്യ) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               വലുപ്പം(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

സ്ഥിതം u32 tis_init(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    തിരികെ 1;
}


സ്ഥിതം ശൂന്യം set_timeouts(u32 timeouts[4], u32 durations[3])
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    എങ്കിൽ (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * വലുപ്പം(u32));
    എങ്കിൽ (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * വലുപ്പം(u32));
}

സ്ഥിതം u32 tis_activate(u8 locty)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 0;
    u8 acc;
    പൂർണ്ണസംഖ്യ l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    എങ്കിൽ (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        വേണ്ടി (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    എങ്കിൽ ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    തിരികെ rc;
}

സ്ഥിതം u32 tis_find_active_locality(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u8 locty;

    വേണ്ടി (locty = 0; locty <= 4; locty++) {
        എങ്കിൽ ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            തിരികെ locty;
    }

    tis_activate(0);

    തിരികെ 0;
}

സ്ഥിതം u32 tis_ready(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    തിരികെ rc;
}

സ്ഥിതം u32 tis_senddata(സ്ഥിരം u8 *സ്ഥിരം data, u32 len)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    ചെയ്യുക {
        ആകുമ്പോൾ (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            എങ്കിൽ (burst == 0) {
                എങ്കിൽ (timer_check(end)) {
                    warn_timeout();
                    നിർത്തുക;
                }
                yield();
            }
        }

        എങ്കിൽ (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            നിർത്തുക;
        }

        ആകുമ്പോൾ (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            എങ്കിൽ (burst == 0 || offset == len)
                നിർത്തുക;
        }

        എങ്കിൽ (offset == len)
            end_loop = 1;
    } ആകുമ്പോൾ (end_loop == 0);

    തിരികെ rc;
}

സ്ഥിതം u32 tis_readresp(u8 *buffer, u32 *len)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    ആകുമ്പോൾ (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        എങ്കിൽ ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            നിർത്തുക;
    }

    *len = offset;

    തിരികെ rc;
}


സ്ഥിതം u32 tis_waitdatavalid(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    എങ്കിൽ (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    തിരികെ rc;
}

സ്ഥിതം u32 tis_waitrespready(എണ്ണൽ tpmDurationType to_t)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    എങ്കിൽ (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    തിരികെ rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
സ്ഥിതം u32 crb_probe(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    എങ്കിൽ (rc)
        തിരികെ 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    എങ്കിൽ ((ifaceid & 0xf) != 0xf) {
        എങ്കിൽ ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } അല്ലെങ്കിൽ എങ്കിൽ ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            തിരികെ 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    എങ്കിൽ (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        തിരികെ 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    എങ്കിൽ (addr > 0xffffffff)
        തിരികെ 0;

    തിരികെ 1;
}

സ്ഥിതം TPMVersion crb_get_tpm_version(ശൂന്യം)
{
    /* CRB is supposed to be TPM 2.0 only */
    തിരികെ TPM_VERSION_2;
}

സ്ഥിതം u32 crb_init(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 1;

    crb_cmd = (ശൂന്യം*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (ശൂന്യം*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    തിരികെ 0;
}

സ്ഥിതം u32 crb_activate(u8 locty)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    തിരികെ 0;
}

സ്ഥിതം u32 crb_find_active_locality(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    തിരികെ 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

സ്ഥിതം u32 crb_ready(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    തിരികെ rc;
}

സ്ഥിതം u32 crb_senddata(സ്ഥിരം u8 *സ്ഥിരം data, u32 len)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    എങ്കിൽ (len > crb_cmd_size)
        തിരികെ 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    തിരികെ 0;
}

സ്ഥിതം u32 crb_readresp(u8 *buffer, u32 *len)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u8 locty = crb_find_active_locality();
    എങ്കിൽ (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        തിരികെ 1;

    എങ്കിൽ (*len < 6)
        തിരികെ 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    എങ്കിൽ (expected < 6)
        തിരികെ 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    തിരികെ 0;
}


സ്ഥിതം u32 crb_waitdatavalid(ശൂന്യം)
{
    തിരികെ 0;
}

സ്ഥിതം u32 crb_waitrespready(എണ്ണൽ tpmDurationType to_t)
{
    എങ്കിൽ (!CONFIG_TCGBIOS)
        തിരികെ 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    തിരികെ rc;
}

ഘടന tpm_driver tpm_drivers[TPM_അക്കം_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = ശൂന്യ,
            .durations     = ശൂന്യ,
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
            .timeouts      = ശൂന്യ,
            .durations     = ശൂന്യ,
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

സ്ഥിതം u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(ശൂന്യം)
{
    unsigned പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < TPM_അക്കം_DRIVERS; i++) {
        ഘടന tpm_driver *td = &tpm_drivers[i];
        എങ്കിൽ (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            തിരികെ td->get_tpm_version();
        }
    }
    തിരികെ TPM_VERSION_NONE;
}

പൂർണ്ണസംഖ്യ
tpmhw_is_present(ശൂന്യം)
{
    തിരികെ TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

പൂർണ്ണസംഖ്യ
tpmhw_transmit(u8 locty, ഘടന tpm_req_header *req,
               ശൂന്യം *respbuffer, u32 *respbufferlen,
               എണ്ണൽ tpmDurationType to_t)
{
    എങ്കിൽ (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        തിരികെ -1;

    ഘടന tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    എങ്കിൽ (irc != 0) {
        /* tpm could not be activated */
        തിരികെ -1;
    }

    irc = td->senddata((ശൂന്യം*)req, be32_to_cpu(req->totlen));
    എങ്കിൽ (irc != 0)
        തിരികെ -1;

    irc = td->waitdatavalid();
    എങ്കിൽ (irc != 0)
        തിരികെ -1;

    irc = td->waitrespready(to_t);
    എങ്കിൽ (irc != 0)
        തിരികെ -1;

    irc = td->readresp(respbuffer, respbufferlen);
    എങ്കിൽ (irc != 0 ||
        *respbufferlen < വലുപ്പം(ഘടന tpm_rsp_header))
        തിരികെ -1;

    td->ready();

    തിരികെ 0;
}

ശൂന്യം
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    ഘടന tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
