// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "byteorder.h" // be32_to_cpu
#అన్తర్భావ "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#అన్తర్భావ "hw/tpm_drivers.h" // struct tpm_driver
#అన్తర్భావ "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#అన్తర్భావ "output.h" // warn_timeout
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "util.h" // timer_calc_usec
#అన్తర్భావ "x86.h" // readl

/* low level driver implementation */
నిర్మాణం tpm_driver {
    u32 *timeouts;
    u32 *durations;
    శూన్య (*సమితి_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(శూన్య);
    TPMVersion (*get_tpm_సంస్కరణ)(శూన్య);
    u32 (*init)(శూన్య);
    u32 (*activate)(u8 locty);
    u32 (*ready)(శూన్య);
    u32 (*senddata)(స్థిరరాశి u8 *స్థిరరాశి దత్తాంశం, u32 len);
    u32 (*readresp)(u8 *బఫరు, u32 *len);
    u32 (*waitdatavalid)(శూన్య);
    u32 (*waitrespready)(గణన tpmDurationType గమ్యం_t);
};

బాహ్య నిర్మాణం tpm_driver tpm_drivers[];

#నిర్వచించు TIS_DRIVER_IDX       0
#నిర్వచించు CRB_DRIVER_IDX       1
#నిర్వచించు TPM_అంకె_DRIVERS      2

#నిర్వచించు TPM_INVALID_DRIVER   0xf

స్థిర స్థిరరాశి u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

స్థిర స్థిరరాశి u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
స్థిర u32 tpm_default_dur[3];
స్థిర u32 tpm_default_గమ్యం[4];

స్థిర u32 crb_cmd_కొలత;
స్థిర శూన్య *crb_cmd;
స్థిర u32 crb_resp_కొలత;
స్థిర శూన్య *crb_resp;

స్థిర u32 wait_reg8(u8* reg, u32 సమయం, u8 ముసుగు, u8 expect)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 1;
    u32 చివర = timer_calc_usec(సమయం);

    కొరకు (;;) {
        u8 విలువ = readl(reg);
        ఒకవేళ ((విలువ & ముసుగు) == expect) {
            rc = 0;
            ఆపు;
        }
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            ఆపు;
        }
        yield();
    }
    తిరిగి rc;
}

స్థిర u32 tis_wait_access(u8 locty, u32 సమయం, u8 ముసుగు, u8 expect)
{
    తిరిగి wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), సమయం, ముసుగు, expect);
}

స్థిర u32 tis_wait_sts(u8 locty, u32 సమయం, u8 ముసుగు, u8 expect)
{
    తిరిగి wait_reg8(TIS_REG(locty, TIS_REG_STS), సమయం, ముసుగు, expect);
}

స్థిర u32 crb_wait_reg(u8 locty, u16 reg, u32 సమయం, u8 ముసుగు, u8 expect)
{
    తిరిగి wait_reg8(CRB_REG(locty, reg), సమయం, ముసుగు, expect);
}

/* if device is not there, return '0', '1' otherwise */
స్థిర u32 tis_probe(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    ఒకవేళ (rc)
        తిరిగి 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    ఒకవేళ ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    ఒకవేళ ((ifaceid & 0xf) != 0xf) {
        ఒకవేళ ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            తిరిగి 0;
        }
        ఒకవేళ ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            తిరిగి 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    తిరిగి rc;
}

స్థిర TPMVersion tis_get_tpm_సంస్కరణ(శూన్య)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    ఒకవేళ ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        ఒకవేళ (((reg >> 28) & 0x7) == 3)
            తిరిగి TPM_సంస్కరణ_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    లేకపోతే ఒకవేళ ((reg & 0xf) == 0) {
        తిరిగి TPM_సంస్కరణ_2;
    }

    తిరిగి TPM_సంస్కరణ_1_2;
}

స్థిర శూన్య init_timeout(పూర్ణాంకం driver)
{
    ఒకవేళ (tpm_drivers[driver].durations == ఖాళీ) {
        u32 *durations = tpm_default_dur;
        జ్ఞాపకం_నకలు(durations, tpm_default_durations,
               పరిమాణం(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    ఒకవేళ (tpm_drivers[driver].timeouts == ఖాళీ) {
        u32 *timeouts = tpm_default_గమ్యం;
        జ్ఞాపకం_నకలు(timeouts, tis_default_timeouts,
               పరిమాణం(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

స్థిర u32 tis_init(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    తిరిగి 1;
}


స్థిర శూన్య సమితి_timeouts(u32 timeouts[4], u32 durations[3])
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    ఒకవేళ (tos && tos != tis_default_timeouts && timeouts)
        జ్ఞాపకం_నకలు(tos, timeouts, 4 * పరిమాణం(u32));
    ఒకవేళ (dus && dus != tpm_default_durations && durations)
        జ్ఞాపకం_నకలు(dus, durations, 3 * పరిమాణం(u32));
}

స్థిర u32 tis_activate(u8 locty)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 0;
    u8 acc;
    పూర్ణాంకం l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    ఒకవేళ (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_చురుకైన_LOCALITY)) {
        /* release locality in use top-downwards */
        కొరకు (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_చురుకైన_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    ఒకవేళ ((acc & TIS_ACCESS_చురుకైన_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    తిరిగి rc;
}

స్థిర u32 tis_కనుగొను_చురుకైన_locality(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u8 locty;

    కొరకు (locty = 0; locty <= 4; locty++) {
        ఒకవేళ ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_చురుకైన_LOCALITY))
            తిరిగి locty;
    }

    tis_activate(0);

    తిరిగి 0;
}

స్థిర u32 tis_ready(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 0;
    u8 locty = tis_కనుగొను_చురుకైన_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    తిరిగి rc;
}

స్థిర u32 tis_senddata(స్థిరరాశి u8 *స్థిరరాశి దత్తాంశం, u32 len)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 0;
    u32 స్థానభ్రంశం_2 = 0;
    u32 చివర_loop = 0;
    u16 వ్యవధి = 0;
    u8 locty = tis_కనుగొను_చురుకైన_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 చివర = timer_calc_usec(timeout_d);

    చేయి {
        అయితే (వ్యవధి == 0) {
               వ్యవధి = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            ఒకవేళ (వ్యవధి == 0) {
                ఒకవేళ (timer_check(చివర)) {
                    warn_timeout();
                    ఆపు;
                }
                yield();
            }
        }

        ఒకవేళ (వ్యవధి == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            ఆపు;
        }

        అయితే (1) {
            writeb(TIS_REG(locty, TIS_REG_దత్తాంశం_FIFO), దత్తాంశం[స్థానభ్రంశం_2++]);
            వ్యవధి--;

            ఒకవేళ (వ్యవధి == 0 || స్థానభ్రంశం_2 == len)
                ఆపు;
        }

        ఒకవేళ (స్థానభ్రంశం_2 == len)
            చివర_loop = 1;
    } అయితే (చివర_loop == 0);

    తిరిగి rc;
}

స్థిర u32 tis_readresp(u8 *బఫరు, u32 *len)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 0;
    u32 స్థానభ్రంశం_2 = 0;
    u32 sts;
    u8 locty = tis_కనుగొను_చురుకైన_locality();

    అయితే (స్థానభ్రంశం_2 < *len) {
        బఫరు[స్థానభ్రంశం_2] = readb(TIS_REG(locty, TIS_REG_దత్తాంశం_FIFO));
        స్థానభ్రంశం_2++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        ఒకవేళ ((sts & TIS_STS_దత్తాంశం_అందుబాటు) == 0)
            ఆపు;
    }

    *len = స్థానభ్రంశం_2;

    తిరిగి rc;
}


స్థిర u32 tis_waitdatavalid(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 0;
    u8 locty = tis_కనుగొను_చురుకైన_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    ఒకవేళ (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    తిరిగి rc;
}

స్థిర u32 tis_waitrespready(గణన tpmDurationType గమ్యం_t)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 0;
    u8 locty = tis_కనుగొను_చురుకైన_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[గమ్యం_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    ఒకవేళ (tis_wait_sts(locty, timeout,
                     TIS_STS_దత్తాంశం_అందుబాటు, TIS_STS_దత్తాంశం_అందుబాటు) != 0)
        rc = 1;

    తిరిగి rc;
}

#నిర్వచించు CRB_STATE_VALID_STS 0b10000000
#నిర్వచించు CRB_STATE_LOC_ASSIGNED 0x00000010
#నిర్వచించు CRB_STATE_READY_ముసుగు (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
స్థిర u32 crb_probe(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_ముసుగు, CRB_STATE_VALID_STS);
    ఒకవేళ (rc)
        తిరిగి 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    ఒకవేళ ((ifaceid & 0xf) != 0xf) {
        ఒకవేళ ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } లేకపోతే ఒకవేళ ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            తిరిగి 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    ఒకవేళ (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        తిరిగి 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    ఒకవేళ (addr > 0xffffffff)
        తిరిగి 0;

    తిరిగి 1;
}

స్థిర TPMVersion crb_get_tpm_సంస్కరణ(శూన్య)
{
    /* CRB is supposed to be TPM 2.0 only */
    తిరిగి TPM_సంస్కరణ_2;
}

స్థిర u32 crb_init(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 1;

    crb_cmd = (శూన్య*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_కొలత = readl(CRB_REG(0, CRB_REG_CTRL_CMD_కొలత));
    crb_resp = (శూన్య*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_కొలత = readl(CRB_REG(0, CRB_REG_CTRL_RSP_కొలత));

    init_timeout(CRB_DRIVER_IDX);

    తిరిగి 0;
}

స్థిర u32 crb_activate(u8 locty)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    తిరిగి 0;
}

స్థిర u32 crb_కనుగొను_చురుకైన_locality(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    తిరిగి 0;
}

#నిర్వచించు CRB_CTRL_REQ_CMD_READY 0b1
#నిర్వచించు CRB_ప్రారంభం_INVOKE 0b1
#నిర్వచించు CRB_CTRL_STS_ERROR 0b1

స్థిర u32 crb_ready(శూన్య)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 0;
    u8 locty = crb_కనుగొను_చురుకైన_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    తిరిగి rc;
}

స్థిర u32 crb_senddata(స్థిరరాశి u8 *స్థిరరాశి దత్తాంశం, u32 len)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    ఒకవేళ (len > crb_cmd_కొలత)
        తిరిగి 1;

    u8 locty = crb_కనుగొను_చురుకైన_locality();
    జ్ఞాపకం_నకలు(crb_cmd, దత్తాంశం, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_ప్రారంభం), CRB_ప్రారంభం_INVOKE);

    తిరిగి 0;
}

స్థిర u32 crb_readresp(u8 *బఫరు, u32 *len)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u8 locty = crb_కనుగొను_చురుకైన_locality();
    ఒకవేళ (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        తిరిగి 1;

    ఒకవేళ (*len < 6)
        తిరిగి 1;

    జ్ఞాపకం_నకలు(బఫరు, crb_resp, 6);
    u32 expected = be32_గమ్యం_cpu(*(u32 *) &బఫరు[2]);
    ఒకవేళ (expected < 6)
        తిరిగి 1;

    *len = (*len < expected) ? *len : expected;

    జ్ఞాపకం_నకలు(బఫరు + 6, crb_resp + 6, *len - 6);

    తిరిగి 0;
}


స్థిర u32 crb_waitdatavalid(శూన్య)
{
    తిరిగి 0;
}

స్థిర u32 crb_waitrespready(గణన tpmDurationType గమ్యం_t)
{
    ఒకవేళ (!CONFIG_TCGBIOS)
        తిరిగి 0;

    u32 rc = 0;
    u8 locty = crb_కనుగొను_చురుకైన_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[గమ్యం_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_ప్రారంభం, timeout,
                      CRB_ప్రారంభం_INVOKE, 0);

    తిరిగి rc;
}

నిర్మాణం tpm_driver tpm_drivers[TPM_అంకె_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = ఖాళీ,
            .durations     = ఖాళీ,
            .సమితి_timeouts  = సమితి_timeouts,
            .probe         = tis_probe,
            .get_tpm_సంస్కరణ = tis_get_tpm_సంస్కరణ,
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
            .timeouts      = ఖాళీ,
            .durations     = ఖాళీ,
            .సమితి_timeouts  = సమితి_timeouts,
            .probe         = crb_probe,
            .get_tpm_సంస్కరణ = crb_get_tpm_సంస్కరణ,
            .init          = crb_init,
            .activate      = crb_activate,
            .ready         = crb_ready,
            .senddata      = crb_senddata,
            .readresp      = crb_readresp,
            .waitdatavalid = crb_waitdatavalid,
            .waitrespready = crb_waitrespready,
        },
};

స్థిర u8 TPMHW_driver_గమ్యం_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(శూన్య)
{
    గుర్తులేని పూర్ణాంకం i;
    కొరకు (i = 0; i < TPM_అంకె_DRIVERS; i++) {
        నిర్మాణం tpm_driver *td = &tpm_drivers[i];
        ఒకవేళ (td->probe() != 0) {
            td->init();
            TPMHW_driver_గమ్యం_use = i;
            తిరిగి td->get_tpm_సంస్కరణ();
        }
    }
    తిరిగి TPM_సంస్కరణ_NONE;
}

పూర్ణాంకం
tpmhw_is_present(శూన్య)
{
    తిరిగి TPMHW_driver_గమ్యం_use != TPM_INVALID_DRIVER;
}

పూర్ణాంకం
tpmhw_transmit(u8 locty, నిర్మాణం tpm_req_శీర్షిక *req,
               శూన్య *respbuffer, u32 *respbufferlen,
               గణన tpmDurationType గమ్యం_t)
{
    ఒకవేళ (TPMHW_driver_గమ్యం_use == TPM_INVALID_DRIVER)
        తిరిగి -1;

    నిర్మాణం tpm_driver *td = &tpm_drivers[TPMHW_driver_గమ్యం_use];

    u32 irc = td->activate(locty);
    ఒకవేళ (irc != 0) {
        /* tpm could not be activated */
        తిరిగి -1;
    }

    irc = td->senddata((శూన్య*)req, be32_గమ్యం_cpu(req->totlen));
    ఒకవేళ (irc != 0)
        తిరిగి -1;

    irc = td->waitdatavalid();
    ఒకవేళ (irc != 0)
        తిరిగి -1;

    irc = td->waitrespready(గమ్యం_t);
    ఒకవేళ (irc != 0)
        తిరిగి -1;

    irc = td->readresp(respbuffer, respbufferlen);
    ఒకవేళ (irc != 0 ||
        *respbufferlen < పరిమాణం(నిర్మాణం tpm_rsp_శీర్షిక))
        తిరిగి -1;

    td->ready();

    తిరిగి 0;
}

శూన్య
tpmhw_సమితి_timeouts(u32 timeouts[4], u32 durations[3])
{
    నిర్మాణం tpm_driver *td = &tpm_drivers[TPMHW_driver_గమ్యం_use];
    td->సమితి_timeouts(timeouts, durations);
}
