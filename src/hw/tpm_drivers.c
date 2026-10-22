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
    فراغ (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(فراغ);
    TPMVersion (*get_tpm_version)(فراغ);
    u32 (*init)(فراغ);
    u32 (*activate)(u8 locty);
    u32 (*ready)(فراغ);
    u32 (*senddata)(ثابت u8 *ثابت data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(فراغ);
    u32 (*waitrespready)(تعداد tpmDurationType to_t);
};

خارجي بنية tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_NUM_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

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
ساكن u32 tpm_default_to[4];

ساكن u32 crb_cmd_size;
ساكن فراغ *crb_cmd;
ساكن u32 crb_resp_size;
ساكن فراغ *crb_resp;

ساكن u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    עבור (;;) {
        u8 value = readl(reg);
        אם ((value & mask) == expect) {
            rc = 0;
            اكسر;
        }
        אם (timer_check(end)) {
            warn_timeout();
            اكسر;
        }
        yield();
    }
    החזר rc;
}

ساكن u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    החזר wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

ساكن u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    החזר wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

ساكن u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    החזר wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
ساكن u32 tis_probe(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    אם (rc)
        החזר 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    אם ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    אם ((ifaceid & 0xf) != 0xf) {
        אם ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            החזר 0;
        }
        אם ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            החזר 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    החזר rc;
}

ساكن TPMVersion tis_get_tpm_version(فراغ)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    אם ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        אם (((reg >> 28) & 0x7) == 3)
            החזר TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    אחרת אם ((reg & 0xf) == 0) {
        החזר TPM_VERSION_2;
    }

    החזר TPM_VERSION_1_2;
}

ساكن فراغ init_timeout(صحيح driver)
{
    אם (tpm_drivers[driver].durations == عدم) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               حجم(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    אם (tpm_drivers[driver].timeouts == عدم) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               حجم(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

ساكن u32 tis_init(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    החזר 1;
}


ساكن فراغ set_timeouts(u32 timeouts[4], u32 durations[3])
{
    אם (!CONFIG_TCGBIOS)
        החזר;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    אם (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * حجم(u32));
    אם (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * حجم(u32));
}

ساكن u32 tis_activate(u8 locty)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 0;
    u8 acc;
    صحيح l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    אם (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        עבור (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    אם ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    החזר rc;
}

ساكن u32 tis_find_active_locality(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u8 locty;

    עבור (locty = 0; locty <= 4; locty++) {
        אם ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            החזר locty;
    }

    tis_activate(0);

    החזר 0;
}

ساكن u32 tis_ready(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    החזר rc;
}

ساكن u32 tis_senddata(ثابت u8 *ثابت data, u32 len)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    עשה {
        כלעוד (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            אם (burst == 0) {
                אם (timer_check(end)) {
                    warn_timeout();
                    اكسر;
                }
                yield();
            }
        }

        אם (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            اكسر;
        }

        כלעוד (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            אם (burst == 0 || offset == len)
                اكسر;
        }

        אם (offset == len)
            end_loop = 1;
    } כלעוד (end_loop == 0);

    החזר rc;
}

ساكن u32 tis_readresp(u8 *buffer, u32 *len)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    כלעוד (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        אם ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            اكسر;
    }

    *len = offset;

    החזר rc;
}


ساكن u32 tis_waitdatavalid(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    אם (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    החזר rc;
}

ساكن u32 tis_waitrespready(تعداد tpmDurationType to_t)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    אם (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    החזר rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
ساكن u32 crb_probe(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    אם (rc)
        החזר 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    אם ((ifaceid & 0xf) != 0xf) {
        אם ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } אחרת אם ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            החזר 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    אם (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        החזר 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    אם (addr > 0xffffffff)
        החזר 0;

    החזר 1;
}

ساكن TPMVersion crb_get_tpm_version(فراغ)
{
    /* CRB is supposed to be TPM 2.0 only */
    החזר TPM_VERSION_2;
}

ساكن u32 crb_init(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 1;

    crb_cmd = (فراغ*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (فراغ*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    החזר 0;
}

ساكن u32 crb_activate(u8 locty)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    החזר 0;
}

ساكن u32 crb_find_active_locality(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    החזר 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

ساكن u32 crb_ready(فراغ)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    החזר rc;
}

ساكن u32 crb_senddata(ثابت u8 *ثابت data, u32 len)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    אם (len > crb_cmd_size)
        החזר 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    החזר 0;
}

ساكن u32 crb_readresp(u8 *buffer, u32 *len)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u8 locty = crb_find_active_locality();
    אם (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        החזר 1;

    אם (*len < 6)
        החזר 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    אם (expected < 6)
        החזר 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    החזר 0;
}


ساكن u32 crb_waitdatavalid(فراغ)
{
    החזר 0;
}

ساكن u32 crb_waitrespready(تعداد tpmDurationType to_t)
{
    אם (!CONFIG_TCGBIOS)
        החזר 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    החזר rc;
}

بنية tpm_driver tpm_drivers[TPM_NUM_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = عدم,
            .durations     = عدم,
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
            .timeouts      = عدم,
            .durations     = عدم,
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

ساكن u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(فراغ)
{
    unsigned صحيح i;
    עבור (i = 0; i < TPM_NUM_DRIVERS; i++) {
        بنية tpm_driver *td = &tpm_drivers[i];
        אם (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            החזר td->get_tpm_version();
        }
    }
    החזר TPM_VERSION_NONE;
}

صحيح
tpmhw_is_present(فراغ)
{
    החזר TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

صحيح
tpmhw_transmit(u8 locty, بنية tpm_req_header *req,
               فراغ *respbuffer, u32 *respbufferlen,
               تعداد tpmDurationType to_t)
{
    אם (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        החזר -1;

    بنية tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    אם (irc != 0) {
        /* tpm could not be activated */
        החזר -1;
    }

    irc = td->senddata((فراغ*)req, be32_to_cpu(req->totlen));
    אם (irc != 0)
        החזר -1;

    irc = td->waitdatavalid();
    אם (irc != 0)
        החזר -1;

    irc = td->waitrespready(to_t);
    אם (irc != 0)
        החזר -1;

    irc = td->readresp(respbuffer, respbufferlen);
    אם (irc != 0 ||
        *respbufferlen < حجم(بنية tpm_rsp_header))
        החזר -1;

    td->ready();

    החזר 0;
}

فراغ
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    بنية tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
