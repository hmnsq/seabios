// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "byteorder.h" // be32_to_cpu
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#ಅನ್ತರ್ಭಾವ "hw/tpm_drivers.h" // struct tpm_driver
#ಅನ್ತರ್ಭಾವ "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#ಅನ್ತರ್ಭಾವ "output.h" // warn_timeout
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc_usec
#ಅನ್ತರ್ಭಾವ "x86.h" // readl

/* low level driver implementation */
ಸಂರಚನಾ tpm_driver {
    u32 *timeouts;
    u32 *durations;
    ಶೂನ್ಯ (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(ಶೂನ್ಯ);
    TPMVersion (*get_tpm_version)(ಶೂನ್ಯ);
    u32 (*init)(ಶೂನ್ಯ);
    u32 (*activate)(u8 locty);
    u32 (*ready)(ಶೂನ್ಯ);
    u32 (*senddata)(ಧ್ರುವ u8 *ಧ್ರುವ data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(ಶೂನ್ಯ);
    u32 (*waitrespready)(ಗಣನಾ tpmDurationType to_t);
};

ಬಾಹ್ಯ ಸಂರಚನಾ tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_ಅಂಕೆ_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

ಸ್ಥಿರ ಧ್ರುವ u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

ಸ್ಥಿರ ಧ್ರುವ u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
ಸ್ಥಿರ u32 tpm_default_dur[3];
ಸ್ಥಿರ u32 tpm_default_to[4];

ಸ್ಥಿರ u32 crb_cmd_size;
ಸ್ಥಿರ ಶೂನ್ಯ *crb_cmd;
ಸ್ಥಿರ u32 crb_resp_size;
ಸ್ಥಿರ ಶೂನ್ಯ *crb_resp;

ಸ್ಥಿರ u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    ಪ್ರತಿ (;;) {
        u8 ಮೌಲ್ಯ = readl(reg);
        ಯದಿ ((ಮೌಲ್ಯ & mask) == expect) {
            rc = 0;
            ಭಙ್ಗ;
        }
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಭಙ್ಗ;
        }
        yield();
    }
    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    ಪ್ರತ್ಯಾಗಮ wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

ಸ್ಥಿರ u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    ಪ್ರತ್ಯಾಗಮ wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

ಸ್ಥಿರ u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    ಪ್ರತ್ಯಾಗಮ wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
ಸ್ಥಿರ u32 tis_probe(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    ಯದಿ (rc)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    ಯದಿ ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    ಯದಿ ((ifaceid & 0xf) != 0xf) {
        ಯದಿ ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            ಪ್ರತ್ಯಾಗಮ 0;
        }
        ಯದಿ ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            ಪ್ರತ್ಯಾಗಮ 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ TPMVersion tis_get_tpm_version(ಶೂನ್ಯ)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    ಯದಿ ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        ಯದಿ (((reg >> 28) & 0x7) == 3)
            ಪ್ರತ್ಯಾಗಮ TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    ಅನ್ಯಥಾ ಯದಿ ((reg & 0xf) == 0) {
        ಪ್ರತ್ಯಾಗಮ TPM_VERSION_2;
    }

    ಪ್ರತ್ಯಾಗಮ TPM_VERSION_1_2;
}

ಸ್ಥಿರ ಶೂನ್ಯ init_timeout(ಪೂರ್ಣಾಙ್ಕ driver)
{
    ಯದಿ (tpm_drivers[driver].durations == ರಿಕ್ತ) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               ಪರಿಮಾಣ(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    ಯದಿ (tpm_drivers[driver].timeouts == ರಿಕ್ತ) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               ಪರಿಮಾಣ(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

ಸ್ಥಿರ u32 tis_init(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಶೂನ್ಯ set_timeouts(u32 timeouts[4], u32 durations[3])
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    ಯದಿ (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * ಪರಿಮಾಣ(u32));
    ಯದಿ (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * ಪರಿಮಾಣ(u32));
}

ಸ್ಥಿರ u32 tis_activate(u8 locty)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 0;
    u8 acc;
    ಪೂರ್ಣಾಙ್ಕ l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    ಯದಿ (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        ಪ್ರತಿ (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    ಯದಿ ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32 tis_find_active_locality(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u8 locty;

    ಪ್ರತಿ (locty = 0; locty <= 4; locty++) {
        ಯದಿ ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            ಪ್ರತ್ಯಾಗಮ locty;
    }

    tis_activate(0);

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32 tis_ready(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32 tis_senddata(ಧ್ರುವ u8 *ಧ್ರುವ data, u32 len)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    ಕುರು {
        ಯಾವತ್ (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            ಯದಿ (burst == 0) {
                ಯದಿ (timer_check(end)) {
                    warn_timeout();
                    ಭಙ್ಗ;
                }
                yield();
            }
        }

        ಯದಿ (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            ಭಙ್ಗ;
        }

        ಯಾವತ್ (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            ಯದಿ (burst == 0 || offset == len)
                ಭಙ್ಗ;
        }

        ಯದಿ (offset == len)
            end_loop = 1;
    } ಯಾವತ್ (end_loop == 0);

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32 tis_readresp(u8 *buffer, u32 *len)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    ಯಾವತ್ (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        ಯದಿ ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            ಭಙ್ಗ;
    }

    *len = offset;

    ಪ್ರತ್ಯಾಗಮ rc;
}


ಸ್ಥಿರ u32 tis_waitdatavalid(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    ಯದಿ (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32 tis_waitrespready(ಗಣನಾ tpmDurationType to_t)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    ಯದಿ (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    ಪ್ರತ್ಯಾಗಮ rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
ಸ್ಥಿರ u32 crb_probe(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    ಯದಿ (rc)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    ಯದಿ ((ifaceid & 0xf) != 0xf) {
        ಯದಿ ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } ಅನ್ಯಥಾ ಯದಿ ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            ಪ್ರತ್ಯಾಗಮ 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    ಯದಿ (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        ಪ್ರತ್ಯಾಗಮ 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    ಯದಿ (addr > 0xffffffff)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ TPMVersion crb_get_tpm_version(ಶೂನ್ಯ)
{
    /* CRB is supposed to be TPM 2.0 only */
    ಪ್ರತ್ಯಾಗಮ TPM_VERSION_2;
}

ಸ್ಥಿರ u32 crb_init(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 1;

    crb_cmd = (ಶೂನ್ಯ*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (ಶೂನ್ಯ*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32 crb_activate(u8 locty)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32 crb_find_active_locality(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಪ್ರತ್ಯಾಗಮ 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

ಸ್ಥಿರ u32 crb_ready(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ u32 crb_senddata(ಧ್ರುವ u8 *ಧ್ರುವ data, u32 len)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಯದಿ (len > crb_cmd_size)
        ಪ್ರತ್ಯಾಗಮ 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32 crb_readresp(u8 *buffer, u32 *len)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u8 locty = crb_find_active_locality();
    ಯದಿ (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        ಪ್ರತ್ಯಾಗಮ 1;

    ಯದಿ (*len < 6)
        ಪ್ರತ್ಯಾಗಮ 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    ಯದಿ (expected < 6)
        ಪ್ರತ್ಯಾಗಮ 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ u32 crb_waitdatavalid(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ u32 crb_waitrespready(ಗಣನಾ tpmDurationType to_t)
{
    ಯದಿ (!CONFIG_TCGBIOS)
        ಪ್ರತ್ಯಾಗಮ 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    ಪ್ರತ್ಯಾಗಮ rc;
}

ಸಂರಚನಾ tpm_driver tpm_drivers[TPM_ಅಂಕೆ_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = ರಿಕ್ತ,
            .durations     = ರಿಕ್ತ,
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
            .timeouts      = ರಿಕ್ತ,
            .durations     = ರಿಕ್ತ,
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

ಸ್ಥಿರ u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(ಶೂನ್ಯ)
{
    unsigned ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < TPM_ಅಂಕೆ_DRIVERS; i++) {
        ಸಂರಚನಾ tpm_driver *td = &tpm_drivers[i];
        ಯದಿ (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            ಪ್ರತ್ಯಾಗಮ td->get_tpm_version();
        }
    }
    ಪ್ರತ್ಯಾಗಮ TPM_VERSION_NONE;
}

ಪೂರ್ಣಾಙ್ಕ
tpmhw_is_present(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

ಪೂರ್ಣಾಙ್ಕ
tpmhw_transmit(u8 locty, ಸಂರಚನಾ tpm_req_header *req,
               ಶೂನ್ಯ *respbuffer, u32 *respbufferlen,
               ಗಣನಾ tpmDurationType to_t)
{
    ಯದಿ (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    ಯದಿ (irc != 0) {
        /* tpm could not be activated */
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    irc = td->senddata((ಶೂನ್ಯ*)req, be32_to_cpu(req->totlen));
    ಯದಿ (irc != 0)
        ಪ್ರತ್ಯಾಗಮ -1;

    irc = td->waitdatavalid();
    ಯದಿ (irc != 0)
        ಪ್ರತ್ಯಾಗಮ -1;

    irc = td->waitrespready(to_t);
    ಯದಿ (irc != 0)
        ಪ್ರತ್ಯಾಗಮ -1;

    irc = td->readresp(respbuffer, respbufferlen);
    ಯದಿ (irc != 0 ||
        *respbufferlen < ಪರಿಮಾಣ(ಸಂರಚನಾ tpm_rsp_header))
        ಪ್ರತ್ಯಾಗಮ -1;

    td->ready();

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    ಸಂರಚನಾ tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
