// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // be32_to_cpu
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/tpm_drivers.h" // struct tpm_driver
#ਅਨ੍ਤਰ੍ਭਾਵ "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // warn_timeout
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc_usec
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // readl

/* low level driver implementation */
ਬਣਤਰ tpm_driver {
    u32 *timeouts;
    u32 *durations;
    ਖਾਲੀ (*ਸੈੱਟ_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(ਖਾਲੀ);
    TPMVersion (*get_tpm_ਸੰਸਕਰਣ)(ਖਾਲੀ);
    u32 (*init)(ਖਾਲੀ);
    u32 (*activate)(u8 locty);
    u32 (*ready)(ਖਾਲੀ);
    u32 (*senddata)(ਅਚਲ u8 *ਅਚਲ ਅੰਕੜੇ, u32 len);
    u32 (*readresp)(u8 *ਬਫਰ, u32 *len);
    u32 (*waitdatavalid)(ਖਾਲੀ);
    u32 (*waitrespready)(ਗਿਣਤੀ tpmDurationType ਮੰਜ਼ਿਲ_t);
};

ਬਾਹਰੀ ਬਣਤਰ tpm_driver tpm_drivers[];

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_DRIVER_IDX       0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_DRIVER_IDX       1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ਅੰਕ_DRIVERS      2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_INVALID_DRIVER   0xf

ਸਥਿਰ ਅਚਲ u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

ਸਥਿਰ ਅਚਲ u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
ਸਥਿਰ u32 tpm_default_dur[3];
ਸਥਿਰ u32 tpm_default_ਮੰਜ਼ਿਲ[4];

ਸਥਿਰ u32 crb_cmd_ਮਾਪ;
ਸਥਿਰ ਖਾਲੀ *crb_cmd;
ਸਥਿਰ u32 crb_resp_ਮਾਪ;
ਸਥਿਰ ਖਾਲੀ *crb_resp;

ਸਥਿਰ u32 wait_reg8(u8* reg, u32 ਸਮਾਂ, u8 ਨਕਾਬ, u8 expect)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 1;
    u32 ਸਿਰਾ = timer_calc_usec(ਸਮਾਂ);

    ਲਈ (;;) {
        u8 ਮੁੱਲ = readl(reg);
        ਜੇ ((ਮੁੱਲ & ਨਕਾਬ) == expect) {
            rc = 0;
            ਰੋਕੋ;
        }
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਰੋਕੋ;
        }
        yield();
    }
    ਵਾਪਸ rc;
}

ਸਥਿਰ u32 tis_wait_access(u8 locty, u32 ਸਮਾਂ, u8 ਨਕਾਬ, u8 expect)
{
    ਵਾਪਸ wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), ਸਮਾਂ, ਨਕਾਬ, expect);
}

ਸਥਿਰ u32 tis_wait_sts(u8 locty, u32 ਸਮਾਂ, u8 ਨਕਾਬ, u8 expect)
{
    ਵਾਪਸ wait_reg8(TIS_REG(locty, TIS_REG_STS), ਸਮਾਂ, ਨਕਾਬ, expect);
}

ਸਥਿਰ u32 crb_wait_reg(u8 locty, u16 reg, u32 ਸਮਾਂ, u8 ਨਕਾਬ, u8 expect)
{
    ਵਾਪਸ wait_reg8(CRB_REG(locty, reg), ਸਮਾਂ, ਨਕਾਬ, expect);
}

/* if device is not there, return '0', '1' otherwise */
ਸਥਿਰ u32 tis_probe(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    ਜੇ (rc)
        ਵਾਪਸ 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    ਜੇ ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    ਜੇ ((ifaceid & 0xf) != 0xf) {
        ਜੇ ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            ਵਾਪਸ 0;
        }
        ਜੇ ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            ਵਾਪਸ 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    ਵਾਪਸ rc;
}

ਸਥਿਰ TPMVersion tis_get_tpm_ਸੰਸਕਰਣ(ਖਾਲੀ)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    ਜੇ ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        ਜੇ (((reg >> 28) & 0x7) == 3)
            ਵਾਪਸ TPM_ਸੰਸਕਰਣ_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    ਨਹੀਂਤਾਂ ਜੇ ((reg & 0xf) == 0) {
        ਵਾਪਸ TPM_ਸੰਸਕਰਣ_2;
    }

    ਵਾਪਸ TPM_ਸੰਸਕਰਣ_1_2;
}

ਸਥਿਰ ਖਾਲੀ init_timeout(ਪੂਰਨ driver)
{
    ਜੇ (tpm_drivers[driver].durations == ਨਿਰਾ) {
        u32 *durations = tpm_default_dur;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(durations, tpm_default_durations,
               ਆਕਾਰ(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    ਜੇ (tpm_drivers[driver].timeouts == ਨਿਰਾ) {
        u32 *timeouts = tpm_default_ਮੰਜ਼ਿਲ;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(timeouts, tis_default_timeouts,
               ਆਕਾਰ(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

ਸਥਿਰ u32 tis_init(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    ਵਾਪਸ 1;
}


ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_timeouts(u32 timeouts[4], u32 durations[3])
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    ਜੇ (tos && tos != tis_default_timeouts && timeouts)
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(tos, timeouts, 4 * ਆਕਾਰ(u32));
    ਜੇ (dus && dus != tpm_default_durations && durations)
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(dus, durations, 3 * ਆਕਾਰ(u32));
}

ਸਥਿਰ u32 tis_activate(u8 locty)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 0;
    u8 acc;
    ਪੂਰਨ l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    ਜੇ (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ਸਰਗਰਮ_LOCALITY)) {
        /* release locality in use top-downwards */
        ਲਈ (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ਸਰਗਰਮ_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    ਜੇ ((acc & TIS_ACCESS_ਸਰਗਰਮ_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    ਵਾਪਸ rc;
}

ਸਥਿਰ u32 tis_ਲੱਭੋ_ਸਰਗਰਮ_locality(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u8 locty;

    ਲਈ (locty = 0; locty <= 4; locty++) {
        ਜੇ ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ਸਰਗਰਮ_LOCALITY))
            ਵਾਪਸ locty;
    }

    tis_activate(0);

    ਵਾਪਸ 0;
}

ਸਥਿਰ u32 tis_ready(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 0;
    u8 locty = tis_ਲੱਭੋ_ਸਰਗਰਮ_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    ਵਾਪਸ rc;
}

ਸਥਿਰ u32 tis_senddata(ਅਚਲ u8 *ਅਚਲ ਅੰਕੜੇ, u32 len)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 0;
    u32 ਵਿੱਥ_2 = 0;
    u32 ਸਿਰਾ_loop = 0;
    u16 ਮਿਆਦ = 0;
    u8 locty = tis_ਲੱਭੋ_ਸਰਗਰਮ_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 ਸਿਰਾ = timer_calc_usec(timeout_d);

    ਕਰੋ {
        ਜਦੋਂ (ਮਿਆਦ == 0) {
               ਮਿਆਦ = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            ਜੇ (ਮਿਆਦ == 0) {
                ਜੇ (timer_check(ਸਿਰਾ)) {
                    warn_timeout();
                    ਰੋਕੋ;
                }
                yield();
            }
        }

        ਜੇ (ਮਿਆਦ == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            ਰੋਕੋ;
        }

        ਜਦੋਂ (1) {
            writeb(TIS_REG(locty, TIS_REG_ਅੰਕੜੇ_FIFO), ਅੰਕੜੇ[ਵਿੱਥ_2++]);
            ਮਿਆਦ--;

            ਜੇ (ਮਿਆਦ == 0 || ਵਿੱਥ_2 == len)
                ਰੋਕੋ;
        }

        ਜੇ (ਵਿੱਥ_2 == len)
            ਸਿਰਾ_loop = 1;
    } ਜਦੋਂ (ਸਿਰਾ_loop == 0);

    ਵਾਪਸ rc;
}

ਸਥਿਰ u32 tis_readresp(u8 *ਬਫਰ, u32 *len)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 0;
    u32 ਵਿੱਥ_2 = 0;
    u32 sts;
    u8 locty = tis_ਲੱਭੋ_ਸਰਗਰਮ_locality();

    ਜਦੋਂ (ਵਿੱਥ_2 < *len) {
        ਬਫਰ[ਵਿੱਥ_2] = readb(TIS_REG(locty, TIS_REG_ਅੰਕੜੇ_FIFO));
        ਵਿੱਥ_2++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        ਜੇ ((sts & TIS_STS_ਅੰਕੜੇ_ਉਪਲਬਧ) == 0)
            ਰੋਕੋ;
    }

    *len = ਵਿੱਥ_2;

    ਵਾਪਸ rc;
}


ਸਥਿਰ u32 tis_waitdatavalid(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 0;
    u8 locty = tis_ਲੱਭੋ_ਸਰਗਰਮ_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    ਜੇ (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    ਵਾਪਸ rc;
}

ਸਥਿਰ u32 tis_waitrespready(ਗਿਣਤੀ tpmDurationType ਮੰਜ਼ਿਲ_t)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 0;
    u8 locty = tis_ਲੱਭੋ_ਸਰਗਰਮ_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[ਮੰਜ਼ਿਲ_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    ਜੇ (tis_wait_sts(locty, timeout,
                     TIS_STS_ਅੰਕੜੇ_ਉਪਲਬਧ, TIS_STS_ਅੰਕੜੇ_ਉਪਲਬਧ) != 0)
        rc = 1;

    ਵਾਪਸ rc;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_STATE_VALID_STS 0b10000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_STATE_LOC_ASSIGNED 0x00000010
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_STATE_READY_ਨਕਾਬ (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
ਸਥਿਰ u32 crb_probe(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_ਨਕਾਬ, CRB_STATE_VALID_STS);
    ਜੇ (rc)
        ਵਾਪਸ 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    ਜੇ ((ifaceid & 0xf) != 0xf) {
        ਜੇ ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } ਨਹੀਂਤਾਂ ਜੇ ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            ਵਾਪਸ 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    ਜੇ (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        ਵਾਪਸ 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    ਜੇ (addr > 0xffffffff)
        ਵਾਪਸ 0;

    ਵਾਪਸ 1;
}

ਸਥਿਰ TPMVersion crb_get_tpm_ਸੰਸਕਰਣ(ਖਾਲੀ)
{
    /* CRB is supposed to be TPM 2.0 only */
    ਵਾਪਸ TPM_ਸੰਸਕਰਣ_2;
}

ਸਥਿਰ u32 crb_init(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 1;

    crb_cmd = (ਖਾਲੀ*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_ਮਾਪ = readl(CRB_REG(0, CRB_REG_CTRL_CMD_ਮਾਪ));
    crb_resp = (ਖਾਲੀ*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_ਮਾਪ = readl(CRB_REG(0, CRB_REG_CTRL_RSP_ਮਾਪ));

    init_timeout(CRB_DRIVER_IDX);

    ਵਾਪਸ 0;
}

ਸਥਿਰ u32 crb_activate(u8 locty)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    ਵਾਪਸ 0;
}

ਸਥਿਰ u32 crb_ਲੱਭੋ_ਸਰਗਰਮ_locality(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_CTRL_REQ_CMD_READY 0b1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_ਸ਼ੁਰੂ_INVOKE 0b1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_CTRL_STS_ERROR 0b1

ਸਥਿਰ u32 crb_ready(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 0;
    u8 locty = crb_ਲੱਭੋ_ਸਰਗਰਮ_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    ਵਾਪਸ rc;
}

ਸਥਿਰ u32 crb_senddata(ਅਚਲ u8 *ਅਚਲ ਅੰਕੜੇ, u32 len)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    ਜੇ (len > crb_cmd_ਮਾਪ)
        ਵਾਪਸ 1;

    u8 locty = crb_ਲੱਭੋ_ਸਰਗਰਮ_locality();
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(crb_cmd, ਅੰਕੜੇ, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_ਸ਼ੁਰੂ), CRB_ਸ਼ੁਰੂ_INVOKE);

    ਵਾਪਸ 0;
}

ਸਥਿਰ u32 crb_readresp(u8 *ਬਫਰ, u32 *len)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u8 locty = crb_ਲੱਭੋ_ਸਰਗਰਮ_locality();
    ਜੇ (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        ਵਾਪਸ 1;

    ਜੇ (*len < 6)
        ਵਾਪਸ 1;

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ, crb_resp, 6);
    u32 expected = be32_ਮੰਜ਼ਿਲ_cpu(*(u32 *) &ਬਫਰ[2]);
    ਜੇ (expected < 6)
        ਵਾਪਸ 1;

    *len = (*len < expected) ? *len : expected;

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ + 6, crb_resp + 6, *len - 6);

    ਵਾਪਸ 0;
}


ਸਥਿਰ u32 crb_waitdatavalid(ਖਾਲੀ)
{
    ਵਾਪਸ 0;
}

ਸਥਿਰ u32 crb_waitrespready(ਗਿਣਤੀ tpmDurationType ਮੰਜ਼ਿਲ_t)
{
    ਜੇ (!CONFIG_TCGBIOS)
        ਵਾਪਸ 0;

    u32 rc = 0;
    u8 locty = crb_ਲੱਭੋ_ਸਰਗਰਮ_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[ਮੰਜ਼ਿਲ_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_ਸ਼ੁਰੂ, timeout,
                      CRB_ਸ਼ੁਰੂ_INVOKE, 0);

    ਵਾਪਸ rc;
}

ਬਣਤਰ tpm_driver tpm_drivers[TPM_ਅੰਕ_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = ਨਿਰਾ,
            .durations     = ਨਿਰਾ,
            .ਸੈੱਟ_timeouts  = ਸੈੱਟ_timeouts,
            .probe         = tis_probe,
            .get_tpm_ਸੰਸਕਰਣ = tis_get_tpm_ਸੰਸਕਰਣ,
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
            .timeouts      = ਨਿਰਾ,
            .durations     = ਨਿਰਾ,
            .ਸੈੱਟ_timeouts  = ਸੈੱਟ_timeouts,
            .probe         = crb_probe,
            .get_tpm_ਸੰਸਕਰਣ = crb_get_tpm_ਸੰਸਕਰਣ,
            .init          = crb_init,
            .activate      = crb_activate,
            .ready         = crb_ready,
            .senddata      = crb_senddata,
            .readresp      = crb_readresp,
            .waitdatavalid = crb_waitdatavalid,
            .waitrespready = crb_waitrespready,
        },
};

ਸਥਿਰ u8 TPMHW_driver_ਮੰਜ਼ਿਲ_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(ਖਾਲੀ)
{
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
    ਲਈ (i = 0; i < TPM_ਅੰਕ_DRIVERS; i++) {
        ਬਣਤਰ tpm_driver *td = &tpm_drivers[i];
        ਜੇ (td->probe() != 0) {
            td->init();
            TPMHW_driver_ਮੰਜ਼ਿਲ_use = i;
            ਵਾਪਸ td->get_tpm_ਸੰਸਕਰਣ();
        }
    }
    ਵਾਪਸ TPM_ਸੰਸਕਰਣ_NONE;
}

ਪੂਰਨ
tpmhw_is_present(ਖਾਲੀ)
{
    ਵਾਪਸ TPMHW_driver_ਮੰਜ਼ਿਲ_use != TPM_INVALID_DRIVER;
}

ਪੂਰਨ
tpmhw_transmit(u8 locty, ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ *req,
               ਖਾਲੀ *respbuffer, u32 *respbufferlen,
               ਗਿਣਤੀ tpmDurationType ਮੰਜ਼ਿਲ_t)
{
    ਜੇ (TPMHW_driver_ਮੰਜ਼ਿਲ_use == TPM_INVALID_DRIVER)
        ਵਾਪਸ -1;

    ਬਣਤਰ tpm_driver *td = &tpm_drivers[TPMHW_driver_ਮੰਜ਼ਿਲ_use];

    u32 irc = td->activate(locty);
    ਜੇ (irc != 0) {
        /* tpm could not be activated */
        ਵਾਪਸ -1;
    }

    irc = td->senddata((ਖਾਲੀ*)req, be32_ਮੰਜ਼ਿਲ_cpu(req->totlen));
    ਜੇ (irc != 0)
        ਵਾਪਸ -1;

    irc = td->waitdatavalid();
    ਜੇ (irc != 0)
        ਵਾਪਸ -1;

    irc = td->waitrespready(ਮੰਜ਼ਿਲ_t);
    ਜੇ (irc != 0)
        ਵਾਪਸ -1;

    irc = td->readresp(respbuffer, respbufferlen);
    ਜੇ (irc != 0 ||
        *respbufferlen < ਆਕਾਰ(ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ))
        ਵਾਪਸ -1;

    td->ready();

    ਵਾਪਸ 0;
}

ਖਾਲੀ
tpmhw_ਸੈੱਟ_timeouts(u32 timeouts[4], u32 durations[3])
{
    ਬਣਤਰ tpm_driver *td = &tpm_drivers[TPMHW_driver_ਮੰਜ਼ਿਲ_use];
    td->ਸੈੱਟ_timeouts(timeouts, durations);
}
