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
ساختار tpm_driver {
    u32 *timeouts;
    u32 *durations;
    تهی (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(تهی);
    TPMVersion (*get_tpm_version)(تهی);
    u32 (*init)(تهی);
    u32 (*activate)(u8 locty);
    u32 (*ready)(تهی);
    u32 (*senddata)(ثابت u8 *ثابت data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(تهی);
    u32 (*waitrespready)(شمارش tpmDurationType to_t);
};

بیرونی ساختار tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_رقم_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

ایستا ثابت u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

ایستا ثابت u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
ایستا u32 tpm_default_dur[3];
ایستا u32 tpm_default_to[4];

ایستا u32 crb_cmd_size;
ایستا تهی *crb_cmd;
ایستا u32 crb_resp_size;
ایستا تهی *crb_resp;

ایستا u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    برای (;;) {
        u8 مقدار = readl(reg);
        اگر ((مقدار & mask) == expect) {
            rc = 0;
            بشکن;
        }
        اگر (timer_check(end)) {
            warn_timeout();
            بشکن;
        }
        yield();
    }
    بازگشت rc;
}

ایستا u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    بازگشت wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

ایستا u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    بازگشت wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

ایستا u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    بازگشت wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
ایستا u32 tis_probe(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    اگر (rc)
        بازگشت 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    اگر ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    اگر ((ifaceid & 0xf) != 0xf) {
        اگر ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            بازگشت 0;
        }
        اگر ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            بازگشت 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    بازگشت rc;
}

ایستا TPMVersion tis_get_tpm_version(تهی)
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
            بازگشت TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    وگرنه اگر ((reg & 0xf) == 0) {
        بازگشت TPM_VERSION_2;
    }

    بازگشت TPM_VERSION_1_2;
}

ایستا تهی init_timeout(صحیح driver)
{
    اگر (tpm_drivers[driver].durations == پوچ) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               اندازه(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    اگر (tpm_drivers[driver].timeouts == پوچ) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               اندازه(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

ایستا u32 tis_init(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    بازگشت 1;
}


ایستا تهی set_timeouts(u32 timeouts[4], u32 durations[3])
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    اگر (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * اندازه(u32));
    اگر (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * اندازه(u32));
}

ایستا u32 tis_activate(u8 locty)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 0;
    u8 acc;
    صحیح l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    اگر (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        برای (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    اگر ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    بازگشت rc;
}

ایستا u32 tis_find_active_locality(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u8 locty;

    برای (locty = 0; locty <= 4; locty++) {
        اگر ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            بازگشت locty;
    }

    tis_activate(0);

    بازگشت 0;
}

ایستا u32 tis_ready(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    بازگشت rc;
}

ایستا u32 tis_senddata(ثابت u8 *ثابت data, u32 len)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    انجامبده {
        وقتی (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            اگر (burst == 0) {
                اگر (timer_check(end)) {
                    warn_timeout();
                    بشکن;
                }
                yield();
            }
        }

        اگر (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            بشکن;
        }

        وقتی (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            اگر (burst == 0 || offset == len)
                بشکن;
        }

        اگر (offset == len)
            end_loop = 1;
    } وقتی (end_loop == 0);

    بازگشت rc;
}

ایستا u32 tis_readresp(u8 *buffer, u32 *len)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    وقتی (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        اگر ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            بشکن;
    }

    *len = offset;

    بازگشت rc;
}


ایستا u32 tis_waitdatavalid(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    اگر (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    بازگشت rc;
}

ایستا u32 tis_waitrespready(شمارش tpmDurationType to_t)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    اگر (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    بازگشت rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
ایستا u32 crb_probe(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    اگر (rc)
        بازگشت 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    اگر ((ifaceid & 0xf) != 0xf) {
        اگر ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } وگرنه اگر ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            بازگشت 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    اگر (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        بازگشت 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    اگر (addr > 0xffffffff)
        بازگشت 0;

    بازگشت 1;
}

ایستا TPMVersion crb_get_tpm_version(تهی)
{
    /* CRB is supposed to be TPM 2.0 only */
    بازگشت TPM_VERSION_2;
}

ایستا u32 crb_init(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 1;

    crb_cmd = (تهی*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (تهی*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    بازگشت 0;
}

ایستا u32 crb_activate(u8 locty)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    بازگشت 0;
}

ایستا u32 crb_find_active_locality(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    بازگشت 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

ایستا u32 crb_ready(تهی)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    بازگشت rc;
}

ایستا u32 crb_senddata(ثابت u8 *ثابت data, u32 len)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    اگر (len > crb_cmd_size)
        بازگشت 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    بازگشت 0;
}

ایستا u32 crb_readresp(u8 *buffer, u32 *len)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u8 locty = crb_find_active_locality();
    اگر (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        بازگشت 1;

    اگر (*len < 6)
        بازگشت 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    اگر (expected < 6)
        بازگشت 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    بازگشت 0;
}


ایستا u32 crb_waitdatavalid(تهی)
{
    بازگشت 0;
}

ایستا u32 crb_waitrespready(شمارش tpmDurationType to_t)
{
    اگر (!CONFIG_TCGBIOS)
        بازگشت 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    بازگشت rc;
}

ساختار tpm_driver tpm_drivers[TPM_رقم_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = پوچ,
            .durations     = پوچ,
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
            .timeouts      = پوچ,
            .durations     = پوچ,
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

ایستا u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(تهی)
{
    unsigned صحیح i;
    برای (i = 0; i < TPM_رقم_DRIVERS; i++) {
        ساختار tpm_driver *td = &tpm_drivers[i];
        اگر (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            بازگشت td->get_tpm_version();
        }
    }
    بازگشت TPM_VERSION_NONE;
}

صحیح
tpmhw_is_present(تهی)
{
    بازگشت TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

صحیح
tpmhw_transmit(u8 locty, ساختار tpm_req_header *req,
               تهی *respbuffer, u32 *respbufferlen,
               شمارش tpmDurationType to_t)
{
    اگر (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        بازگشت -1;

    ساختار tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    اگر (irc != 0) {
        /* tpm could not be activated */
        بازگشت -1;
    }

    irc = td->senddata((تهی*)req, be32_to_cpu(req->totlen));
    اگر (irc != 0)
        بازگشت -1;

    irc = td->waitdatavalid();
    اگر (irc != 0)
        بازگشت -1;

    irc = td->waitrespready(to_t);
    اگر (irc != 0)
        بازگشت -1;

    irc = td->readresp(respbuffer, respbufferlen);
    اگر (irc != 0 ||
        *respbufferlen < اندازه(ساختار tpm_rsp_header))
        بازگشت -1;

    td->ready();

    بازگشت 0;
}

تهی
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    ساختار tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
