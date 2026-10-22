// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "byteorder.h" // be32_to_cpu
#অন্তর্ভাব "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#অন্তর্ভাব "hw/tpm_drivers.h" // struct tpm_driver
#অন্তর্ভাব "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#অন্তর্ভাব "output.h" // warn_timeout
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "string.h" // memcpy
#অন্তর্ভাব "util.h" // timer_calc_usec
#অন্তর্ভাব "x86.h" // readl

/* low level driver implementation */
কাঠামো tpm_driver {
    u32 *timeouts;
    u32 *durations;
    শূন্য (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(শূন্য);
    TPMVersion (*get_tpm_version)(শূন্য);
    u32 (*init)(শূন্য);
    u32 (*activate)(u8 locty);
    u32 (*ready)(শূন্য);
    u32 (*senddata)(ধ্রুবক u8 *ধ্রুবক তথ্য, u32 len);
    u32 (*readresp)(u8 *বাফার, u32 *len);
    u32 (*waitdatavalid)(শূন্য);
    u32 (*waitrespready)(গণনা tpmDurationType to_t);
};

বাহ্যিক কাঠামো tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_অঙ্ক_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

স্থির ধ্রুবক u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

স্থির ধ্রুবক u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
স্থির u32 tpm_default_dur[3];
স্থির u32 tpm_default_to[4];

স্থির u32 crb_cmd_আকার;
স্থির শূন্য *crb_cmd;
স্থির u32 crb_resp_আকার;
স্থির শূন্য *crb_resp;

স্থির u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    জন্য (;;) {
        u8 মান = readl(reg);
        যদি ((মান & mask) == expect) {
            rc = 0;
            বিরতি;
        }
        যদি (timer_check(end)) {
            warn_timeout();
            বিরতি;
        }
        yield();
    }
    ফেরত rc;
}

স্থির u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    ফেরত wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

স্থির u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    ফেরত wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

স্থির u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    ফেরত wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
স্থির u32 tis_probe(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    যদি (rc)
        ফেরত 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    যদি ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    যদি ((ifaceid & 0xf) != 0xf) {
        যদি ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            ফেরত 0;
        }
        যদি ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            ফেরত 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    ফেরত rc;
}

স্থির TPMVersion tis_get_tpm_version(শূন্য)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    যদি ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        যদি (((reg >> 28) & 0x7) == 3)
            ফেরত TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    অন্যথা যদি ((reg & 0xf) == 0) {
        ফেরত TPM_VERSION_2;
    }

    ফেরত TPM_VERSION_1_2;
}

স্থির শূন্য init_timeout(পূর্ণসংখ্যা driver)
{
    যদি (tpm_drivers[driver].durations == ফাঁকা) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               আকার(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    যদি (tpm_drivers[driver].timeouts == ফাঁকা) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               আকার(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

স্থির u32 tis_init(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    ফেরত 1;
}


স্থির শূন্য set_timeouts(u32 timeouts[4], u32 durations[3])
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    যদি (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * আকার(u32));
    যদি (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * আকার(u32));
}

স্থির u32 tis_activate(u8 locty)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 0;
    u8 acc;
    পূর্ণসংখ্যা l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    যদি (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        জন্য (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    যদি ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    ফেরত rc;
}

স্থির u32 tis_find_active_locality(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u8 locty;

    জন্য (locty = 0; locty <= 4; locty++) {
        যদি ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            ফেরত locty;
    }

    tis_activate(0);

    ফেরত 0;
}

স্থির u32 tis_ready(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    ফেরত rc;
}

স্থির u32 tis_senddata(ধ্রুবক u8 *ধ্রুবক তথ্য, u32 len)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    করো {
        যখন (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            যদি (burst == 0) {
                যদি (timer_check(end)) {
                    warn_timeout();
                    বিরতি;
                }
                yield();
            }
        }

        যদি (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            বিরতি;
        }

        যখন (1) {
            writeb(TIS_REG(locty, TIS_REG_তথ্য_FIFO), তথ্য[offset++]);
            burst--;

            যদি (burst == 0 || offset == len)
                বিরতি;
        }

        যদি (offset == len)
            end_loop = 1;
    } যখন (end_loop == 0);

    ফেরত rc;
}

স্থির u32 tis_readresp(u8 *বাফার, u32 *len)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    যখন (offset < *len) {
        বাফার[offset] = readb(TIS_REG(locty, TIS_REG_তথ্য_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        যদি ((sts & TIS_STS_তথ্য_AVAILABLE) == 0)
            বিরতি;
    }

    *len = offset;

    ফেরত rc;
}


স্থির u32 tis_waitdatavalid(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    যদি (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    ফেরত rc;
}

স্থির u32 tis_waitrespready(গণনা tpmDurationType to_t)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    যদি (tis_wait_sts(locty, timeout,
                     TIS_STS_তথ্য_AVAILABLE, TIS_STS_তথ্য_AVAILABLE) != 0)
        rc = 1;

    ফেরত rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
স্থির u32 crb_probe(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    যদি (rc)
        ফেরত 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    যদি ((ifaceid & 0xf) != 0xf) {
        যদি ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } অন্যথা যদি ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            ফেরত 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    যদি (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        ফেরত 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    যদি (addr > 0xffffffff)
        ফেরত 0;

    ফেরত 1;
}

স্থির TPMVersion crb_get_tpm_version(শূন্য)
{
    /* CRB is supposed to be TPM 2.0 only */
    ফেরত TPM_VERSION_2;
}

স্থির u32 crb_init(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 1;

    crb_cmd = (শূন্য*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_আকার = readl(CRB_REG(0, CRB_REG_CTRL_CMD_আকার));
    crb_resp = (শূন্য*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_আকার = readl(CRB_REG(0, CRB_REG_CTRL_RSP_আকার));

    init_timeout(CRB_DRIVER_IDX);

    ফেরত 0;
}

স্থির u32 crb_activate(u8 locty)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    ফেরত 0;
}

স্থির u32 crb_find_active_locality(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    ফেরত 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

স্থির u32 crb_ready(শূন্য)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    ফেরত rc;
}

স্থির u32 crb_senddata(ধ্রুবক u8 *ধ্রুবক তথ্য, u32 len)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    যদি (len > crb_cmd_আকার)
        ফেরত 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, তথ্য, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    ফেরত 0;
}

স্থির u32 crb_readresp(u8 *বাফার, u32 *len)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u8 locty = crb_find_active_locality();
    যদি (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        ফেরত 1;

    যদি (*len < 6)
        ফেরত 1;

    memcpy(বাফার, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &বাফার[2]);
    যদি (expected < 6)
        ফেরত 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(বাফার + 6, crb_resp + 6, *len - 6);

    ফেরত 0;
}


স্থির u32 crb_waitdatavalid(শূন্য)
{
    ফেরত 0;
}

স্থির u32 crb_waitrespready(গণনা tpmDurationType to_t)
{
    যদি (!CONFIG_TCGBIOS)
        ফেরত 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    ফেরত rc;
}

কাঠামো tpm_driver tpm_drivers[TPM_অঙ্ক_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = ফাঁকা,
            .durations     = ফাঁকা,
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
            .timeouts      = ফাঁকা,
            .durations     = ফাঁকা,
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

স্থির u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(শূন্য)
{
    unsigned পূর্ণসংখ্যা i;
    জন্য (i = 0; i < TPM_অঙ্ক_DRIVERS; i++) {
        কাঠামো tpm_driver *td = &tpm_drivers[i];
        যদি (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            ফেরত td->get_tpm_version();
        }
    }
    ফেরত TPM_VERSION_NONE;
}

পূর্ণসংখ্যা
tpmhw_is_present(শূন্য)
{
    ফেরত TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

পূর্ণসংখ্যা
tpmhw_transmit(u8 locty, কাঠামো tpm_req_header *req,
               শূন্য *respbuffer, u32 *respbufferlen,
               গণনা tpmDurationType to_t)
{
    যদি (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        ফেরত -1;

    কাঠামো tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    যদি (irc != 0) {
        /* tpm could not be activated */
        ফেরত -1;
    }

    irc = td->senddata((শূন্য*)req, be32_to_cpu(req->totlen));
    যদি (irc != 0)
        ফেরত -1;

    irc = td->waitdatavalid();
    যদি (irc != 0)
        ফেরত -1;

    irc = td->waitrespready(to_t);
    যদি (irc != 0)
        ফেরত -1;

    irc = td->readresp(respbuffer, respbufferlen);
    যদি (irc != 0 ||
        *respbufferlen < আকার(কাঠামো tpm_rsp_header))
        ফেরত -1;

    td->ready();

    ফেরত 0;
}

শূন্য
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    কাঠামো tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
