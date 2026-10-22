// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "byteorder.h" // be32_to_cpu
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#ଅନ୍ତର୍ଭାଵ "hw/tpm_drivers.h" // struct tpm_driver
#ଅନ୍ତର୍ଭାଵ "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#ଅନ୍ତର୍ଭାଵ "output.h" // warn_timeout
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "util.h" // timer_calc_usec
#ଅନ୍ତର୍ଭାଵ "x86.h" // readl

/* low level driver implementation */
ସଂରଚନା tpm_driver {
    u32 *timeouts;
    u32 *durations;
    ଶୂନ୍ୟ (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(ଶୂନ୍ୟ);
    TPMVersion (*get_tpm_version)(ଶୂନ୍ୟ);
    u32 (*init)(ଶୂନ୍ୟ);
    u32 (*activate)(u8 locty);
    u32 (*ready)(ଶୂନ୍ୟ);
    u32 (*senddata)(ଅଚଳ u8 *ଅଚଳ data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(ଶୂନ୍ୟ);
    u32 (*waitrespready)(ଗଣନା tpmDurationType to_t);
};

ବାହ୍ୟ ସଂରଚନା tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_ଅଙ୍କ_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

ସ୍ଥିର ଅଚଳ u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

ସ୍ଥିର ଅଚଳ u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
ସ୍ଥିର u32 tpm_default_dur[3];
ସ୍ଥିର u32 tpm_default_to[4];

ସ୍ଥିର u32 crb_cmd_size;
ସ୍ଥିର ଶୂନ୍ୟ *crb_cmd;
ସ୍ଥିର u32 crb_resp_size;
ସ୍ଥିର ଶୂନ୍ୟ *crb_resp;

ସ୍ଥିର u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    ପାଇଁ (;;) {
        u8 ମୂଲ୍ୟ = readl(reg);
        ଯଦି ((ମୂଲ୍ୟ & mask) == expect) {
            rc = 0;
            ଭାଙ୍ଗ;
        }
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଭାଙ୍ଗ;
        }
        yield();
    }
    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    ଫେରନ୍ତୁ wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

ସ୍ଥିର u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    ଫେରନ୍ତୁ wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

ସ୍ଥିର u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    ଫେରନ୍ତୁ wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
ସ୍ଥିର u32 tis_probe(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    ଯଦି (rc)
        ଫେରନ୍ତୁ 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    ଯଦି ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    ଯଦି ((ifaceid & 0xf) != 0xf) {
        ଯଦି ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            ଫେରନ୍ତୁ 0;
        }
        ଯଦି ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            ଫେରନ୍ତୁ 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର TPMVersion tis_get_tpm_version(ଶୂନ୍ୟ)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    ଯଦି ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        ଯଦି (((reg >> 28) & 0x7) == 3)
            ଫେରନ୍ତୁ TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    ଅନ୍ୟଥା ଯଦି ((reg & 0xf) == 0) {
        ଫେରନ୍ତୁ TPM_VERSION_2;
    }

    ଫେରନ୍ତୁ TPM_VERSION_1_2;
}

ସ୍ଥିର ଶୂନ୍ୟ init_timeout(ପୂର୍ଣ୍ଣସଂଖ୍ୟା driver)
{
    ଯଦି (tpm_drivers[driver].durations == ଖାଲି) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               ଆକାର(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    ଯଦି (tpm_drivers[driver].timeouts == ଖାଲି) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               ଆକାର(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

ସ୍ଥିର u32 tis_init(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ଶୂନ୍ୟ set_timeouts(u32 timeouts[4], u32 durations[3])
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    ଯଦି (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * ଆକାର(u32));
    ଯଦି (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * ଆକାର(u32));
}

ସ୍ଥିର u32 tis_activate(u8 locty)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 0;
    u8 acc;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    ଯଦି (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        ପାଇଁ (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    ଯଦି ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32 tis_find_active_locality(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u8 locty;

    ପାଇଁ (locty = 0; locty <= 4; locty++) {
        ଯଦି ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            ଫେରନ୍ତୁ locty;
    }

    tis_activate(0);

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32 tis_ready(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32 tis_senddata(ଅଚଳ u8 *ଅଚଳ data, u32 len)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    କର {
        ଯେତେବେଳେ (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            ଯଦି (burst == 0) {
                ଯଦି (timer_check(end)) {
                    warn_timeout();
                    ଭାଙ୍ଗ;
                }
                yield();
            }
        }

        ଯଦି (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            ଭାଙ୍ଗ;
        }

        ଯେତେବେଳେ (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            ଯଦି (burst == 0 || offset == len)
                ଭାଙ୍ଗ;
        }

        ଯଦି (offset == len)
            end_loop = 1;
    } ଯେତେବେଳେ (end_loop == 0);

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32 tis_readresp(u8 *buffer, u32 *len)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    ଯେତେବେଳେ (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        ଯଦି ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            ଭାଙ୍ଗ;
    }

    *len = offset;

    ଫେରନ୍ତୁ rc;
}


ସ୍ଥିର u32 tis_waitdatavalid(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    ଯଦି (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32 tis_waitrespready(ଗଣନା tpmDurationType to_t)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    ଯଦି (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    ଫେରନ୍ତୁ rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
ସ୍ଥିର u32 crb_probe(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    ଯଦି (rc)
        ଫେରନ୍ତୁ 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    ଯଦି ((ifaceid & 0xf) != 0xf) {
        ଯଦି ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } ଅନ୍ୟଥା ଯଦି ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            ଫେରନ୍ତୁ 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    ଯଦି (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        ଫେରନ୍ତୁ 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    ଯଦି (addr > 0xffffffff)
        ଫେରନ୍ତୁ 0;

    ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର TPMVersion crb_get_tpm_version(ଶୂନ୍ୟ)
{
    /* CRB is supposed to be TPM 2.0 only */
    ଫେରନ୍ତୁ TPM_VERSION_2;
}

ସ୍ଥିର u32 crb_init(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 1;

    crb_cmd = (ଶୂନ୍ୟ*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (ଶୂନ୍ୟ*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32 crb_activate(u8 locty)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32 crb_find_active_locality(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    ଫେରନ୍ତୁ 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

ସ୍ଥିର u32 crb_ready(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର u32 crb_senddata(ଅଚଳ u8 *ଅଚଳ data, u32 len)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    ଯଦି (len > crb_cmd_size)
        ଫେରନ୍ତୁ 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32 crb_readresp(u8 *buffer, u32 *len)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u8 locty = crb_find_active_locality();
    ଯଦି (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        ଫେରନ୍ତୁ 1;

    ଯଦି (*len < 6)
        ଫେରନ୍ତୁ 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    ଯଦି (expected < 6)
        ଫେରନ୍ତୁ 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର u32 crb_waitdatavalid(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର u32 crb_waitrespready(ଗଣନା tpmDurationType to_t)
{
    ଯଦି (!CONFIG_TCGBIOS)
        ଫେରନ୍ତୁ 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    ଫେରନ୍ତୁ rc;
}

ସଂରଚନା tpm_driver tpm_drivers[TPM_ଅଙ୍କ_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = ଖାଲି,
            .durations     = ଖାଲି,
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
            .timeouts      = ଖାଲି,
            .durations     = ଖାଲି,
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

ସ୍ଥିର u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(ଶୂନ୍ୟ)
{
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < TPM_ଅଙ୍କ_DRIVERS; i++) {
        ସଂରଚନା tpm_driver *td = &tpm_drivers[i];
        ଯଦି (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            ଫେରନ୍ତୁ td->get_tpm_version();
        }
    }
    ଫେରନ୍ତୁ TPM_VERSION_NONE;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpmhw_is_present(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tpmhw_transmit(u8 locty, ସଂରଚନା tpm_req_header *req,
               ଶୂନ୍ୟ *respbuffer, u32 *respbufferlen,
               ଗଣନା tpmDurationType to_t)
{
    ଯଦି (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    ଯଦି (irc != 0) {
        /* tpm could not be activated */
        ଫେରନ୍ତୁ -1;
    }

    irc = td->senddata((ଶୂନ୍ୟ*)req, be32_to_cpu(req->totlen));
    ଯଦି (irc != 0)
        ଫେରନ୍ତୁ -1;

    irc = td->waitdatavalid();
    ଯଦି (irc != 0)
        ଫେରନ୍ତୁ -1;

    irc = td->waitrespready(to_t);
    ଯଦି (irc != 0)
        ଫେରନ୍ତୁ -1;

    irc = td->readresp(respbuffer, respbufferlen);
    ଯଦି (irc != 0 ||
        *respbufferlen < ଆକାର(ସଂରଚନା tpm_rsp_header))
        ଫେରନ୍ତୁ -1;

    td->ready();

    ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    ସଂରଚନା tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
