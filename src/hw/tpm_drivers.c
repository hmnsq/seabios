// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "byteorder.h" // be32_to_cpu
#شامل "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#شامل "hw/tpm_drivers.h" // struct tpm_driver
#شامل "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#شامل "output.h" // warn_timeout
#شامل "stacks.h" // yield
#شامل "string.h" // memcpy
#شامل "util.h" // timer_calc_usec
#شامل "x86.h" // readl

/* low level driver implementation */
ساخت tpm_driver {
    u32 *timeouts;
    u32 *durations;
    خالی (*سیٹ_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(خالی);
    TPMVersion (*get_tpm_نسخہ)(خالی);
    u32 (*init)(خالی);
    u32 (*activate)(u8 locty);
    u32 (*ready)(خالی);
    u32 (*senddata)(ثابت u8 *ثابت ڈیٹا, u32 len);
    u32 (*readresp)(u8 *بفر, u32 *len);
    u32 (*waitdatavalid)(خالی);
    u32 (*waitrespready)(شمار tpmDurationType مقصد_t);
};

خارجی ساخت tpm_driver tpm_drivers[];

#تعریف_کرو TIS_DRIVER_IDX       0
#تعریف_کرو CRB_DRIVER_IDX       1
#تعریف_کرو TPM_عدد_DRIVERS      2

#تعریف_کرو TPM_INVALID_DRIVER   0xf

ساکن ثابت u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

ساکن ثابت u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
ساکن u32 tpm_default_dur[3];
ساکن u32 tpm_default_مقصد[4];

ساکن u32 crb_cmd_سائز;
ساکن خالی *crb_cmd;
ساکن u32 crb_resp_سائز;
ساکن خالی *crb_resp;

ساکن u32 wait_reg8(u8* reg, u32 وقت, u8 نقاب, u8 expect)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 1;
    u32 انتہا = timer_calc_usec(وقت);

    برائے (;;) {
        u8 قدر = readl(reg);
        اگر ((قدر & نقاب) == expect) {
            rc = 0;
            توڑو;
        }
        اگر (timer_check(انتہا)) {
            warn_timeout();
            توڑو;
        }
        yield();
    }
    واپس rc;
}

ساکن u32 tis_wait_access(u8 locty, u32 وقت, u8 نقاب, u8 expect)
{
    واپس wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), وقت, نقاب, expect);
}

ساکن u32 tis_wait_sts(u8 locty, u32 وقت, u8 نقاب, u8 expect)
{
    واپس wait_reg8(TIS_REG(locty, TIS_REG_STS), وقت, نقاب, expect);
}

ساکن u32 crb_wait_reg(u8 locty, u16 reg, u32 وقت, u8 نقاب, u8 expect)
{
    واپس wait_reg8(CRB_REG(locty, reg), وقت, نقاب, expect);
}

/* if device is not there, return '0', '1' otherwise */
ساکن u32 tis_probe(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    اگر (rc)
        واپس 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    اگر ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    اگر ((ifaceid & 0xf) != 0xf) {
        اگر ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            واپس 0;
        }
        اگر ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            واپس 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    واپس rc;
}

ساکن TPMVersion tis_get_tpm_نسخہ(خالی)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    اگر ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        اگر (((reg >> 28) & 0x7) == 3)
            واپس TPM_نسخہ_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    ورنہ اگر ((reg & 0xf) == 0) {
        واپس TPM_نسخہ_2;
    }

    واپس TPM_نسخہ_1_2;
}

ساکن خالی init_timeout(صحیح driver)
{
    اگر (tpm_drivers[driver].durations == تہی) {
        u32 *durations = tpm_default_dur;
        حافظہ_نقل(durations, tpm_default_durations,
               حجم(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    اگر (tpm_drivers[driver].timeouts == تہی) {
        u32 *timeouts = tpm_default_مقصد;
        حافظہ_نقل(timeouts, tis_default_timeouts,
               حجم(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

ساکن u32 tis_init(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    واپس 1;
}


ساکن خالی سیٹ_timeouts(u32 timeouts[4], u32 durations[3])
{
    اگر (!CONFIG_TCGBIOS)
        واپس;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    اگر (tos && tos != tis_default_timeouts && timeouts)
        حافظہ_نقل(tos, timeouts, 4 * حجم(u32));
    اگر (dus && dus != tpm_default_durations && durations)
        حافظہ_نقل(dus, durations, 3 * حجم(u32));
}

ساکن u32 tis_activate(u8 locty)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 0;
    u8 acc;
    صحیح l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    اگر (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_فعال_LOCALITY)) {
        /* release locality in use top-downwards */
        برائے (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_فعال_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    اگر ((acc & TIS_ACCESS_فعال_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    واپس rc;
}

ساکن u32 tis_ڈھونڈو_فعال_locality(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u8 locty;

    برائے (locty = 0; locty <= 4; locty++) {
        اگر ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_فعال_LOCALITY))
            واپس locty;
    }

    tis_activate(0);

    واپس 0;
}

ساکن u32 tis_ready(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 0;
    u8 locty = tis_ڈھونڈو_فعال_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    واپس rc;
}

ساکن u32 tis_senddata(ثابت u8 *ثابت ڈیٹا, u32 len)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 0;
    u32 انحراف_2 = 0;
    u32 انتہا_loop = 0;
    u16 مدت = 0;
    u8 locty = tis_ڈھونڈو_فعال_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 انتہا = timer_calc_usec(timeout_d);

    کرو {
        جب (مدت == 0) {
               مدت = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            اگر (مدت == 0) {
                اگر (timer_check(انتہا)) {
                    warn_timeout();
                    توڑو;
                }
                yield();
            }
        }

        اگر (مدت == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            توڑو;
        }

        جب (1) {
            writeb(TIS_REG(locty, TIS_REG_ڈیٹا_FIFO), ڈیٹا[انحراف_2++]);
            مدت--;

            اگر (مدت == 0 || انحراف_2 == len)
                توڑو;
        }

        اگر (انحراف_2 == len)
            انتہا_loop = 1;
    } جب (انتہا_loop == 0);

    واپس rc;
}

ساکن u32 tis_readresp(u8 *بفر, u32 *len)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 0;
    u32 انحراف_2 = 0;
    u32 sts;
    u8 locty = tis_ڈھونڈو_فعال_locality();

    جب (انحراف_2 < *len) {
        بفر[انحراف_2] = readb(TIS_REG(locty, TIS_REG_ڈیٹا_FIFO));
        انحراف_2++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        اگر ((sts & TIS_STS_ڈیٹا_دستیاب) == 0)
            توڑو;
    }

    *len = انحراف_2;

    واپس rc;
}


ساکن u32 tis_waitdatavalid(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 0;
    u8 locty = tis_ڈھونڈو_فعال_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    اگر (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    واپس rc;
}

ساکن u32 tis_waitrespready(شمار tpmDurationType مقصد_t)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 0;
    u8 locty = tis_ڈھونڈو_فعال_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[مقصد_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    اگر (tis_wait_sts(locty, timeout,
                     TIS_STS_ڈیٹا_دستیاب, TIS_STS_ڈیٹا_دستیاب) != 0)
        rc = 1;

    واپس rc;
}

#تعریف_کرو CRB_STATE_VALID_STS 0b10000000
#تعریف_کرو CRB_STATE_LOC_ASSIGNED 0x00000010
#تعریف_کرو CRB_STATE_READY_نقاب (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
ساکن u32 crb_probe(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_نقاب, CRB_STATE_VALID_STS);
    اگر (rc)
        واپس 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    اگر ((ifaceid & 0xf) != 0xf) {
        اگر ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } ورنہ اگر ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            واپس 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    اگر (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        واپس 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    اگر (addr > 0xffffffff)
        واپس 0;

    واپس 1;
}

ساکن TPMVersion crb_get_tpm_نسخہ(خالی)
{
    /* CRB is supposed to be TPM 2.0 only */
    واپس TPM_نسخہ_2;
}

ساکن u32 crb_init(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 1;

    crb_cmd = (خالی*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_سائز = readl(CRB_REG(0, CRB_REG_CTRL_CMD_سائز));
    crb_resp = (خالی*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_سائز = readl(CRB_REG(0, CRB_REG_CTRL_RSP_سائز));

    init_timeout(CRB_DRIVER_IDX);

    واپس 0;
}

ساکن u32 crb_activate(u8 locty)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    واپس 0;
}

ساکن u32 crb_ڈھونڈو_فعال_locality(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    واپس 0;
}

#تعریف_کرو CRB_CTRL_REQ_CMD_READY 0b1
#تعریف_کرو CRB_آغاز_INVOKE 0b1
#تعریف_کرو CRB_CTRL_STS_ERROR 0b1

ساکن u32 crb_ready(خالی)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 0;
    u8 locty = crb_ڈھونڈو_فعال_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    واپس rc;
}

ساکن u32 crb_senddata(ثابت u8 *ثابت ڈیٹا, u32 len)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    اگر (len > crb_cmd_سائز)
        واپس 1;

    u8 locty = crb_ڈھونڈو_فعال_locality();
    حافظہ_نقل(crb_cmd, ڈیٹا, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_آغاز), CRB_آغاز_INVOKE);

    واپس 0;
}

ساکن u32 crb_readresp(u8 *بفر, u32 *len)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u8 locty = crb_ڈھونڈو_فعال_locality();
    اگر (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        واپس 1;

    اگر (*len < 6)
        واپس 1;

    حافظہ_نقل(بفر, crb_resp, 6);
    u32 expected = be32_مقصد_cpu(*(u32 *) &بفر[2]);
    اگر (expected < 6)
        واپس 1;

    *len = (*len < expected) ? *len : expected;

    حافظہ_نقل(بفر + 6, crb_resp + 6, *len - 6);

    واپس 0;
}


ساکن u32 crb_waitdatavalid(خالی)
{
    واپس 0;
}

ساکن u32 crb_waitrespready(شمار tpmDurationType مقصد_t)
{
    اگر (!CONFIG_TCGBIOS)
        واپس 0;

    u32 rc = 0;
    u8 locty = crb_ڈھونڈو_فعال_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[مقصد_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_آغاز, timeout,
                      CRB_آغاز_INVOKE, 0);

    واپس rc;
}

ساخت tpm_driver tpm_drivers[TPM_عدد_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = تہی,
            .durations     = تہی,
            .سیٹ_timeouts  = سیٹ_timeouts,
            .probe         = tis_probe,
            .get_tpm_نسخہ = tis_get_tpm_نسخہ,
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
            .timeouts      = تہی,
            .durations     = تہی,
            .سیٹ_timeouts  = سیٹ_timeouts,
            .probe         = crb_probe,
            .get_tpm_نسخہ = crb_get_tpm_نسخہ,
            .init          = crb_init,
            .activate      = crb_activate,
            .ready         = crb_ready,
            .senddata      = crb_senddata,
            .readresp      = crb_readresp,
            .waitdatavalid = crb_waitdatavalid,
            .waitrespready = crb_waitrespready,
        },
};

ساکن u8 TPMHW_driver_مقصد_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(خالی)
{
    غیرعلامتی صحیح i;
    برائے (i = 0; i < TPM_عدد_DRIVERS; i++) {
        ساخت tpm_driver *td = &tpm_drivers[i];
        اگر (td->probe() != 0) {
            td->init();
            TPMHW_driver_مقصد_use = i;
            واپس td->get_tpm_نسخہ();
        }
    }
    واپس TPM_نسخہ_NONE;
}

صحیح
tpmhw_is_present(خالی)
{
    واپس TPMHW_driver_مقصد_use != TPM_INVALID_DRIVER;
}

صحیح
tpmhw_transmit(u8 locty, ساخت tpm_req_سرخی *req,
               خالی *respbuffer, u32 *respbufferlen,
               شمار tpmDurationType مقصد_t)
{
    اگر (TPMHW_driver_مقصد_use == TPM_INVALID_DRIVER)
        واپس -1;

    ساخت tpm_driver *td = &tpm_drivers[TPMHW_driver_مقصد_use];

    u32 irc = td->activate(locty);
    اگر (irc != 0) {
        /* tpm could not be activated */
        واپس -1;
    }

    irc = td->senddata((خالی*)req, be32_مقصد_cpu(req->totlen));
    اگر (irc != 0)
        واپس -1;

    irc = td->waitdatavalid();
    اگر (irc != 0)
        واپس -1;

    irc = td->waitrespready(مقصد_t);
    اگر (irc != 0)
        واپس -1;

    irc = td->readresp(respbuffer, respbufferlen);
    اگر (irc != 0 ||
        *respbufferlen < حجم(ساخت tpm_rsp_سرخی))
        واپس -1;

    td->ready();

    واپس 0;
}

خالی
tpmhw_سیٹ_timeouts(u32 timeouts[4], u32 durations[3])
{
    ساخت tpm_driver *td = &tpm_drivers[TPMHW_driver_مقصد_use];
    td->سیٹ_timeouts(timeouts, durations);
}
