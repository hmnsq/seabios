// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "byteorder.h" // be32_to_cpu
#ضمن "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#ضمن "hw/tpm_drivers.h" // struct tpm_driver
#ضمن "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#ضمن "output.h" // warn_timeout
#ضمن "stacks.h" // yield
#ضمن "string.h" // memcpy
#ضمن "util.h" // timer_calc_usec
#ضمن "x86.h" // readl

/* low level driver implementation */
بنية tpm_driver {
    u32 *timeouts;
    u32 *durations;
    فراغ (*سيٽ_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(فراغ);
    TPMVersion (*get_tpm_نسخو)(فراغ);
    u32 (*init)(فراغ);
    u32 (*activate)(u8 locty);
    u32 (*ready)(فراغ);
    u32 (*senddata)(ثابت u8 *ثابت معلومات, u32 len);
    u32 (*readresp)(u8 *بفر, u32 *len);
    u32 (*waitdatavalid)(فراغ);
    u32 (*waitrespready)(تعداد tpmDurationType منزل_t);
};

خارجي بنية tpm_driver tpm_drivers[];

#وصف_ڪريو TIS_DRIVER_IDX       0
#وصف_ڪريو CRB_DRIVER_IDX       1
#وصف_ڪريو TPM_انگ_DRIVERS      2

#وصف_ڪريو TPM_INVALID_DRIVER   0xf

ساكن ثابت u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

ساكن ثابت u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
ساكن u32 tpm_default_dur[3];
ساكن u32 tpm_default_منزل[4];

ساكن u32 crb_cmd_سائيز;
ساكن فراغ *crb_cmd;
ساكن u32 crb_resp_سائيز;
ساكن فراغ *crb_resp;

ساكن u32 wait_reg8(u8* reg, u32 وقت, u8 نقاب, u8 expect)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 1;
    u32 پڇاڙي = timer_calc_usec(وقت);

    لكل (;;) {
        u8 قدر = readl(reg);
        إذا ((قدر & نقاب) == expect) {
            rc = 0;
            اكسر;
        }
        إذا (timer_check(پڇاڙي)) {
            warn_timeout();
            اكسر;
        }
        yield();
    }
    أرجع rc;
}

ساكن u32 tis_wait_access(u8 locty, u32 وقت, u8 نقاب, u8 expect)
{
    أرجع wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), وقت, نقاب, expect);
}

ساكن u32 tis_wait_sts(u8 locty, u32 وقت, u8 نقاب, u8 expect)
{
    أرجع wait_reg8(TIS_REG(locty, TIS_REG_STS), وقت, نقاب, expect);
}

ساكن u32 crb_wait_reg(u8 locty, u16 reg, u32 وقت, u8 نقاب, u8 expect)
{
    أرجع wait_reg8(CRB_REG(locty, reg), وقت, نقاب, expect);
}

/* if device is not there, return '0', '1' otherwise */
ساكن u32 tis_probe(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    إذا (rc)
        أرجع 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    إذا ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    إذا ((ifaceid & 0xf) != 0xf) {
        إذا ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            أرجع 0;
        }
        إذا ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            أرجع 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    أرجع rc;
}

ساكن TPMVersion tis_get_tpm_نسخو(فراغ)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    إذا ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        إذا (((reg >> 28) & 0x7) == 3)
            أرجع TPM_نسخو_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    وإلا إذا ((reg & 0xf) == 0) {
        أرجع TPM_نسخو_2;
    }

    أرجع TPM_نسخو_1_2;
}

ساكن فراغ init_timeout(صحيح driver)
{
    إذا (tpm_drivers[driver].durations == عدم) {
        u32 *durations = tpm_default_dur;
        يادگيري_نقل(durations, tpm_default_durations,
               حجم(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    إذا (tpm_drivers[driver].timeouts == عدم) {
        u32 *timeouts = tpm_default_منزل;
        يادگيري_نقل(timeouts, tis_default_timeouts,
               حجم(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

ساكن u32 tis_init(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    أرجع 1;
}


ساكن فراغ سيٽ_timeouts(u32 timeouts[4], u32 durations[3])
{
    إذا (!CONFIG_TCGBIOS)
        أرجع;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    إذا (tos && tos != tis_default_timeouts && timeouts)
        يادگيري_نقل(tos, timeouts, 4 * حجم(u32));
    إذا (dus && dus != tpm_default_durations && durations)
        يادگيري_نقل(dus, durations, 3 * حجم(u32));
}

ساكن u32 tis_activate(u8 locty)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 0;
    u8 acc;
    صحيح l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    إذا (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_سرگرم_LOCALITY)) {
        /* release locality in use top-downwards */
        لكل (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_سرگرم_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    إذا ((acc & TIS_ACCESS_سرگرم_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    أرجع rc;
}

ساكن u32 tis_ڳوليو_سرگرم_locality(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u8 locty;

    لكل (locty = 0; locty <= 4; locty++) {
        إذا ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_سرگرم_LOCALITY))
            أرجع locty;
    }

    tis_activate(0);

    أرجع 0;
}

ساكن u32 tis_ready(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 0;
    u8 locty = tis_ڳوليو_سرگرم_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    أرجع rc;
}

ساكن u32 tis_senddata(ثابت u8 *ثابت معلومات, u32 len)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 0;
    u32 وٿي_2 = 0;
    u32 پڇاڙي_loop = 0;
    u16 مدو = 0;
    u8 locty = tis_ڳوليو_سرگرم_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 پڇاڙي = timer_calc_usec(timeout_d);

    افعل {
        طالما (مدو == 0) {
               مدو = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            إذا (مدو == 0) {
                إذا (timer_check(پڇاڙي)) {
                    warn_timeout();
                    اكسر;
                }
                yield();
            }
        }

        إذا (مدو == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            اكسر;
        }

        طالما (1) {
            writeb(TIS_REG(locty, TIS_REG_معلومات_FIFO), معلومات[وٿي_2++]);
            مدو--;

            إذا (مدو == 0 || وٿي_2 == len)
                اكسر;
        }

        إذا (وٿي_2 == len)
            پڇاڙي_loop = 1;
    } طالما (پڇاڙي_loop == 0);

    أرجع rc;
}

ساكن u32 tis_readresp(u8 *بفر, u32 *len)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 0;
    u32 وٿي_2 = 0;
    u32 sts;
    u8 locty = tis_ڳوليو_سرگرم_locality();

    طالما (وٿي_2 < *len) {
        بفر[وٿي_2] = readb(TIS_REG(locty, TIS_REG_معلومات_FIFO));
        وٿي_2++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        إذا ((sts & TIS_STS_معلومات_موجود) == 0)
            اكسر;
    }

    *len = وٿي_2;

    أرجع rc;
}


ساكن u32 tis_waitdatavalid(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 0;
    u8 locty = tis_ڳوليو_سرگرم_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    إذا (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    أرجع rc;
}

ساكن u32 tis_waitrespready(تعداد tpmDurationType منزل_t)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 0;
    u8 locty = tis_ڳوليو_سرگرم_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[منزل_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    إذا (tis_wait_sts(locty, timeout,
                     TIS_STS_معلومات_موجود, TIS_STS_معلومات_موجود) != 0)
        rc = 1;

    أرجع rc;
}

#وصف_ڪريو CRB_STATE_VALID_STS 0b10000000
#وصف_ڪريو CRB_STATE_LOC_ASSIGNED 0x00000010
#وصف_ڪريو CRB_STATE_READY_نقاب (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
ساكن u32 crb_probe(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_نقاب, CRB_STATE_VALID_STS);
    إذا (rc)
        أرجع 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    إذا ((ifaceid & 0xf) != 0xf) {
        إذا ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } وإلا إذا ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            أرجع 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    إذا (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        أرجع 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    إذا (addr > 0xffffffff)
        أرجع 0;

    أرجع 1;
}

ساكن TPMVersion crb_get_tpm_نسخو(فراغ)
{
    /* CRB is supposed to be TPM 2.0 only */
    أرجع TPM_نسخو_2;
}

ساكن u32 crb_init(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 1;

    crb_cmd = (فراغ*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_سائيز = readl(CRB_REG(0, CRB_REG_CTRL_CMD_سائيز));
    crb_resp = (فراغ*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_سائيز = readl(CRB_REG(0, CRB_REG_CTRL_RSP_سائيز));

    init_timeout(CRB_DRIVER_IDX);

    أرجع 0;
}

ساكن u32 crb_activate(u8 locty)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    أرجع 0;
}

ساكن u32 crb_ڳوليو_سرگرم_locality(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    أرجع 0;
}

#وصف_ڪريو CRB_CTRL_REQ_CMD_READY 0b1
#وصف_ڪريو CRB_شروعات_INVOKE 0b1
#وصف_ڪريو CRB_CTRL_STS_ERROR 0b1

ساكن u32 crb_ready(فراغ)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 0;
    u8 locty = crb_ڳوليو_سرگرم_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    أرجع rc;
}

ساكن u32 crb_senddata(ثابت u8 *ثابت معلومات, u32 len)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    إذا (len > crb_cmd_سائيز)
        أرجع 1;

    u8 locty = crb_ڳوليو_سرگرم_locality();
    يادگيري_نقل(crb_cmd, معلومات, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_شروعات), CRB_شروعات_INVOKE);

    أرجع 0;
}

ساكن u32 crb_readresp(u8 *بفر, u32 *len)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u8 locty = crb_ڳوليو_سرگرم_locality();
    إذا (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        أرجع 1;

    إذا (*len < 6)
        أرجع 1;

    يادگيري_نقل(بفر, crb_resp, 6);
    u32 expected = be32_منزل_cpu(*(u32 *) &بفر[2]);
    إذا (expected < 6)
        أرجع 1;

    *len = (*len < expected) ? *len : expected;

    يادگيري_نقل(بفر + 6, crb_resp + 6, *len - 6);

    أرجع 0;
}


ساكن u32 crb_waitdatavalid(فراغ)
{
    أرجع 0;
}

ساكن u32 crb_waitrespready(تعداد tpmDurationType منزل_t)
{
    إذا (!CONFIG_TCGBIOS)
        أرجع 0;

    u32 rc = 0;
    u8 locty = crb_ڳوليو_سرگرم_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[منزل_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_شروعات, timeout,
                      CRB_شروعات_INVOKE, 0);

    أرجع rc;
}

بنية tpm_driver tpm_drivers[TPM_انگ_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = عدم,
            .durations     = عدم,
            .سيٽ_timeouts  = سيٽ_timeouts,
            .probe         = tis_probe,
            .get_tpm_نسخو = tis_get_tpm_نسخو,
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
            .timeouts      = عدم,
            .durations     = عدم,
            .سيٽ_timeouts  = سيٽ_timeouts,
            .probe         = crb_probe,
            .get_tpm_نسخو = crb_get_tpm_نسخو,
            .init          = crb_init,
            .activate      = crb_activate,
            .ready         = crb_ready,
            .senddata      = crb_senddata,
            .readresp      = crb_readresp,
            .waitdatavalid = crb_waitdatavalid,
            .waitrespready = crb_waitrespready,
        },
};

ساكن u8 TPMHW_driver_منزل_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(فراغ)
{
    بنانشان صحيح i;
    لكل (i = 0; i < TPM_انگ_DRIVERS; i++) {
        بنية tpm_driver *td = &tpm_drivers[i];
        إذا (td->probe() != 0) {
            td->init();
            TPMHW_driver_منزل_use = i;
            أرجع td->get_tpm_نسخو();
        }
    }
    أرجع TPM_نسخو_NONE;
}

صحيح
tpmhw_is_present(فراغ)
{
    أرجع TPMHW_driver_منزل_use != TPM_INVALID_DRIVER;
}

صحيح
tpmhw_transmit(u8 locty, بنية tpm_req_سرو *req,
               فراغ *respbuffer, u32 *respbufferlen,
               تعداد tpmDurationType منزل_t)
{
    إذا (TPMHW_driver_منزل_use == TPM_INVALID_DRIVER)
        أرجع -1;

    بنية tpm_driver *td = &tpm_drivers[TPMHW_driver_منزل_use];

    u32 irc = td->activate(locty);
    إذا (irc != 0) {
        /* tpm could not be activated */
        أرجع -1;
    }

    irc = td->senddata((فراغ*)req, be32_منزل_cpu(req->totlen));
    إذا (irc != 0)
        أرجع -1;

    irc = td->waitdatavalid();
    إذا (irc != 0)
        أرجع -1;

    irc = td->waitrespready(منزل_t);
    إذا (irc != 0)
        أرجع -1;

    irc = td->readresp(respbuffer, respbufferlen);
    إذا (irc != 0 ||
        *respbufferlen < حجم(بنية tpm_rsp_سرو))
        أرجع -1;

    td->ready();

    أرجع 0;
}

فراغ
tpmhw_سيٽ_timeouts(u32 timeouts[4], u32 durations[3])
{
    بنية tpm_driver *td = &tpm_drivers[TPMHW_driver_منزل_use];
    td->سيٽ_timeouts(timeouts, durations);
}
