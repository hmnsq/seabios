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
ರಚನೆ tpm_driver {
    u32 *timeouts;
    u32 *durations;
    ಶೂನ್ಯ (*ಗಣ_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(ಶೂನ್ಯ);
    TPMVersion (*get_tpm_ಆವೃತ್ತಿ)(ಶೂನ್ಯ);
    u32 (*init)(ಶೂನ್ಯ);
    u32 (*activate)(u8 locty);
    u32 (*ready)(ಶೂನ್ಯ);
    u32 (*senddata)(ಸ್ಥಿರಾಂಕ u8 *ಸ್ಥಿರಾಂಕ ದತ್ತಾಂಶ, u32 len);
    u32 (*readresp)(u8 *ಬಫರ್, u32 *len);
    u32 (*waitdatavalid)(ಶೂನ್ಯ);
    u32 (*waitrespready)(ಎಣಿಕೆ tpmDurationType ಗಮ್ಯ_t);
};

ಬಾಹ್ಯ ರಚನೆ tpm_driver tpm_drivers[];

#ವ್ಯಾಖ್ಯಾನಿಸು TIS_DRIVER_IDX       0
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_DRIVER_IDX       1
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ಅಂಕೆ_DRIVERS      2

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_INVALID_DRIVER   0xf

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
ಸ್ಥಿರ u32 tpm_default_dur[3];
ಸ್ಥಿರ u32 tpm_default_ಗಮ್ಯ[4];

ಸ್ಥಿರ u32 crb_cmd_ಗಾತ್ರ;
ಸ್ಥಿರ ಶೂನ್ಯ *crb_cmd;
ಸ್ಥಿರ u32 crb_resp_ಗಾತ್ರ;
ಸ್ಥಿರ ಶೂನ್ಯ *crb_resp;

ಸ್ಥಿರ u32 wait_reg8(u8* reg, u32 ಸಮಯ, u8 ಮುಖವಾಡ, u8 expect)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 1;
    u32 ಕೊನೆ = timer_calc_usec(ಸಮಯ);

    ಗಾಗಿ (;;) {
        u8 ಮೌಲ್ಯ = readl(reg);
        ಆದರೆ ((ಮೌಲ್ಯ & ಮುಖವಾಡ) == expect) {
            rc = 0;
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ನಿಲ್ಲಿಸು;
        }
        yield();
    }
    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32 tis_wait_access(u8 locty, u32 ಸಮಯ, u8 ಮುಖವಾಡ, u8 expect)
{
    ಹಿಂತಿರುಗಿ wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), ಸಮಯ, ಮುಖವಾಡ, expect);
}

ಸ್ಥಿರ u32 tis_wait_sts(u8 locty, u32 ಸಮಯ, u8 ಮುಖವಾಡ, u8 expect)
{
    ಹಿಂತಿರುಗಿ wait_reg8(TIS_REG(locty, TIS_REG_STS), ಸಮಯ, ಮುಖವಾಡ, expect);
}

ಸ್ಥಿರ u32 crb_wait_reg(u8 locty, u16 reg, u32 ಸಮಯ, u8 ಮುಖವಾಡ, u8 expect)
{
    ಹಿಂತಿರುಗಿ wait_reg8(CRB_REG(locty, reg), ಸಮಯ, ಮುಖವಾಡ, expect);
}

/* if device is not there, return '0', '1' otherwise */
ಸ್ಥಿರ u32 tis_probe(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    ಆದರೆ (rc)
        ಹಿಂತಿರುಗಿ 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    ಆದರೆ ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    ಆದರೆ ((ifaceid & 0xf) != 0xf) {
        ಆದರೆ ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            ಹಿಂತಿರುಗಿ 0;
        }
        ಆದರೆ ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            ಹಿಂತಿರುಗಿ 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ TPMVersion tis_get_tpm_ಆವೃತ್ತಿ(ಶೂನ್ಯ)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    ಆದರೆ ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        ಆದರೆ (((reg >> 28) & 0x7) == 3)
            ಹಿಂತಿರುಗಿ TPM_ಆವೃತ್ತಿ_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((reg & 0xf) == 0) {
        ಹಿಂತಿರುಗಿ TPM_ಆವೃತ್ತಿ_2;
    }

    ಹಿಂತಿರುಗಿ TPM_ಆವೃತ್ತಿ_1_2;
}

ಸ್ಥಿರ ಶೂನ್ಯ init_timeout(ಪೂರ್ಣಾಂಕ driver)
{
    ಆದರೆ (tpm_drivers[driver].durations == ಖಾಲಿ) {
        u32 *durations = tpm_default_dur;
        ಸ್ಮರಣೆ_ನಕಲು(durations, tpm_default_durations,
               ಗಾತ್ರ(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    ಆದರೆ (tpm_drivers[driver].timeouts == ಖಾಲಿ) {
        u32 *timeouts = tpm_default_ಗಮ್ಯ;
        ಸ್ಮರಣೆ_ನಕಲು(timeouts, tis_default_timeouts,
               ಗಾತ್ರ(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

ಸ್ಥಿರ u32 tis_init(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_timeouts(u32 timeouts[4], u32 durations[3])
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    ಆದರೆ (tos && tos != tis_default_timeouts && timeouts)
        ಸ್ಮರಣೆ_ನಕಲು(tos, timeouts, 4 * ಗಾತ್ರ(u32));
    ಆದರೆ (dus && dus != tpm_default_durations && durations)
        ಸ್ಮರಣೆ_ನಕಲು(dus, durations, 3 * ಗಾತ್ರ(u32));
}

ಸ್ಥಿರ u32 tis_activate(u8 locty)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 0;
    u8 acc;
    ಪೂರ್ಣಾಂಕ l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    ಆದರೆ (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ಸಕ್ರಿಯ_LOCALITY)) {
        /* release locality in use top-downwards */
        ಗಾಗಿ (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ಸಕ್ರಿಯ_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    ಆದರೆ ((acc & TIS_ACCESS_ಸಕ್ರಿಯ_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32 tis_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u8 locty;

    ಗಾಗಿ (locty = 0; locty <= 4; locty++) {
        ಆದರೆ ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ಸಕ್ರಿಯ_LOCALITY))
            ಹಿಂತಿರುಗಿ locty;
    }

    tis_activate(0);

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32 tis_ready(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 0;
    u8 locty = tis_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32 tis_senddata(ಸ್ಥಿರಾಂಕ u8 *ಸ್ಥಿರಾಂಕ ದತ್ತಾಂಶ, u32 len)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 0;
    u32 ಸ್ಥಾನಾಂತರ_2 = 0;
    u32 ಕೊನೆ_loop = 0;
    u16 ಅವಧಿ = 0;
    u8 locty = tis_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 ಕೊನೆ = timer_calc_usec(timeout_d);

    ಮಾಡು {
        ಆಗ (ಅವಧಿ == 0) {
               ಅವಧಿ = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            ಆದರೆ (ಅವಧಿ == 0) {
                ಆದರೆ (timer_check(ಕೊನೆ)) {
                    warn_timeout();
                    ನಿಲ್ಲಿಸು;
                }
                yield();
            }
        }

        ಆದರೆ (ಅವಧಿ == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            ನಿಲ್ಲಿಸು;
        }

        ಆಗ (1) {
            writeb(TIS_REG(locty, TIS_REG_ದತ್ತಾಂಶ_FIFO), ದತ್ತಾಂಶ[ಸ್ಥಾನಾಂತರ_2++]);
            ಅವಧಿ--;

            ಆದರೆ (ಅವಧಿ == 0 || ಸ್ಥಾನಾಂತರ_2 == len)
                ನಿಲ್ಲಿಸು;
        }

        ಆದರೆ (ಸ್ಥಾನಾಂತರ_2 == len)
            ಕೊನೆ_loop = 1;
    } ಆಗ (ಕೊನೆ_loop == 0);

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32 tis_readresp(u8 *ಬಫರ್, u32 *len)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 0;
    u32 ಸ್ಥಾನಾಂತರ_2 = 0;
    u32 sts;
    u8 locty = tis_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();

    ಆಗ (ಸ್ಥಾನಾಂತರ_2 < *len) {
        ಬಫರ್[ಸ್ಥಾನಾಂತರ_2] = readb(TIS_REG(locty, TIS_REG_ದತ್ತಾಂಶ_FIFO));
        ಸ್ಥಾನಾಂತರ_2++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        ಆದರೆ ((sts & TIS_STS_ದತ್ತಾಂಶ_ಲಭ್ಯ) == 0)
            ನಿಲ್ಲಿಸು;
    }

    *len = ಸ್ಥಾನಾಂತರ_2;

    ಹಿಂತಿರುಗಿ rc;
}


ಸ್ಥಿರ u32 tis_waitdatavalid(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 0;
    u8 locty = tis_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    ಆದರೆ (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32 tis_waitrespready(ಎಣಿಕೆ tpmDurationType ಗಮ್ಯ_t)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 0;
    u8 locty = tis_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[ಗಮ್ಯ_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    ಆದರೆ (tis_wait_sts(locty, timeout,
                     TIS_STS_ದತ್ತಾಂಶ_ಲಭ್ಯ, TIS_STS_ದತ್ತಾಂಶ_ಲಭ್ಯ) != 0)
        rc = 1;

    ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿಸು CRB_STATE_VALID_STS 0b10000000
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_STATE_LOC_ASSIGNED 0x00000010
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_STATE_READY_ಮುಖವಾಡ (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
ಸ್ಥಿರ u32 crb_probe(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_ಮುಖವಾಡ, CRB_STATE_VALID_STS);
    ಆದರೆ (rc)
        ಹಿಂತಿರುಗಿ 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    ಆದರೆ ((ifaceid & 0xf) != 0xf) {
        ಆದರೆ ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            ಹಿಂತಿರುಗಿ 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    ಆದರೆ (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        ಹಿಂತಿರುಗಿ 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    ಆದರೆ (addr > 0xffffffff)
        ಹಿಂತಿರುಗಿ 0;

    ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ TPMVersion crb_get_tpm_ಆವೃತ್ತಿ(ಶೂನ್ಯ)
{
    /* CRB is supposed to be TPM 2.0 only */
    ಹಿಂತಿರುಗಿ TPM_ಆವೃತ್ತಿ_2;
}

ಸ್ಥಿರ u32 crb_init(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 1;

    crb_cmd = (ಶೂನ್ಯ*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_ಗಾತ್ರ = readl(CRB_REG(0, CRB_REG_CTRL_CMD_ಗಾತ್ರ));
    crb_resp = (ಶೂನ್ಯ*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_ಗಾತ್ರ = readl(CRB_REG(0, CRB_REG_CTRL_RSP_ಗಾತ್ರ));

    init_timeout(CRB_DRIVER_IDX);

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32 crb_activate(u8 locty)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32 crb_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು CRB_CTRL_REQ_CMD_READY 0b1
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_ಆರಂಭ_INVOKE 0b1
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_CTRL_STS_ERROR 0b1

ಸ್ಥಿರ u32 crb_ready(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 0;
    u8 locty = crb_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ u32 crb_senddata(ಸ್ಥಿರಾಂಕ u8 *ಸ್ಥಿರಾಂಕ ದತ್ತಾಂಶ, u32 len)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ (len > crb_cmd_ಗಾತ್ರ)
        ಹಿಂತಿರುಗಿ 1;

    u8 locty = crb_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();
    ಸ್ಮರಣೆ_ನಕಲು(crb_cmd, ದತ್ತಾಂಶ, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_ಆರಂಭ), CRB_ಆರಂಭ_INVOKE);

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32 crb_readresp(u8 *ಬಫರ್, u32 *len)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u8 locty = crb_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();
    ಆದರೆ (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        ಹಿಂತಿರುಗಿ 1;

    ಆದರೆ (*len < 6)
        ಹಿಂತಿರುಗಿ 1;

    ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್, crb_resp, 6);
    u32 expected = be32_ಗಮ್ಯ_cpu(*(u32 *) &ಬಫರ್[2]);
    ಆದರೆ (expected < 6)
        ಹಿಂತಿರುಗಿ 1;

    *len = (*len < expected) ? *len : expected;

    ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್ + 6, crb_resp + 6, *len - 6);

    ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ u32 crb_waitdatavalid(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ u32 crb_waitrespready(ಎಣಿಕೆ tpmDurationType ಗಮ್ಯ_t)
{
    ಆದರೆ (!CONFIG_TCGBIOS)
        ಹಿಂತಿರುಗಿ 0;

    u32 rc = 0;
    u8 locty = crb_ಕಂಡುಹಿಡಿ_ಸಕ್ರಿಯ_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[ಗಮ್ಯ_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_ಆರಂಭ, timeout,
                      CRB_ಆರಂಭ_INVOKE, 0);

    ಹಿಂತಿರುಗಿ rc;
}

ರಚನೆ tpm_driver tpm_drivers[TPM_ಅಂಕೆ_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = ಖಾಲಿ,
            .durations     = ಖಾಲಿ,
            .ಗಣ_timeouts  = ಗಣ_timeouts,
            .probe         = tis_probe,
            .get_tpm_ಆವೃತ್ತಿ = tis_get_tpm_ಆವೃತ್ತಿ,
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
            .timeouts      = ಖಾಲಿ,
            .durations     = ಖಾಲಿ,
            .ಗಣ_timeouts  = ಗಣ_timeouts,
            .probe         = crb_probe,
            .get_tpm_ಆವೃತ್ತಿ = crb_get_tpm_ಆವೃತ್ತಿ,
            .init          = crb_init,
            .activate      = crb_activate,
            .ready         = crb_ready,
            .senddata      = crb_senddata,
            .readresp      = crb_readresp,
            .waitdatavalid = crb_waitdatavalid,
            .waitrespready = crb_waitrespready,
        },
};

ಸ್ಥಿರ u8 TPMHW_driver_ಗಮ್ಯ_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(ಶೂನ್ಯ)
{
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < TPM_ಅಂಕೆ_DRIVERS; i++) {
        ರಚನೆ tpm_driver *td = &tpm_drivers[i];
        ಆದರೆ (td->probe() != 0) {
            td->init();
            TPMHW_driver_ಗಮ್ಯ_use = i;
            ಹಿಂತಿರುಗಿ td->get_tpm_ಆವೃತ್ತಿ();
        }
    }
    ಹಿಂತಿರುಗಿ TPM_ಆವೃತ್ತಿ_NONE;
}

ಪೂರ್ಣಾಂಕ
tpmhw_is_present(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ TPMHW_driver_ಗಮ್ಯ_use != TPM_INVALID_DRIVER;
}

ಪೂರ್ಣಾಂಕ
tpmhw_transmit(u8 locty, ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ *req,
               ಶೂನ್ಯ *respbuffer, u32 *respbufferlen,
               ಎಣಿಕೆ tpmDurationType ಗಮ್ಯ_t)
{
    ಆದರೆ (TPMHW_driver_ಗಮ್ಯ_use == TPM_INVALID_DRIVER)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ tpm_driver *td = &tpm_drivers[TPMHW_driver_ಗಮ್ಯ_use];

    u32 irc = td->activate(locty);
    ಆದರೆ (irc != 0) {
        /* tpm could not be activated */
        ಹಿಂತಿರುಗಿ -1;
    }

    irc = td->senddata((ಶೂನ್ಯ*)req, be32_ಗಮ್ಯ_cpu(req->totlen));
    ಆದರೆ (irc != 0)
        ಹಿಂತಿರುಗಿ -1;

    irc = td->waitdatavalid();
    ಆದರೆ (irc != 0)
        ಹಿಂತಿರುಗಿ -1;

    irc = td->waitrespready(ಗಮ್ಯ_t);
    ಆದರೆ (irc != 0)
        ಹಿಂತಿರುಗಿ -1;

    irc = td->readresp(respbuffer, respbufferlen);
    ಆದರೆ (irc != 0 ||
        *respbufferlen < ಗಾತ್ರ(ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ))
        ಹಿಂತಿರುಗಿ -1;

    td->ready();

    ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ
tpmhw_ಗಣ_timeouts(u32 timeouts[4], u32 durations[3])
{
    ರಚನೆ tpm_driver *td = &tpm_drivers[TPMHW_driver_ಗಮ್ಯ_use];
    td->ಗಣ_timeouts(timeouts, durations);
}
