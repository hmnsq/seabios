// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "byteorder.h" // be32_to_cpu
#अन्तर्भाव "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#अन्तर्भाव "hw/tpm_drivers.h" // struct tpm_driver
#अन्तर्भाव "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#अन्तर्भाव "output.h" // warn_timeout
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "util.h" // timer_calc_usec
#अन्तर्भाव "x86.h" // readl

/* low level driver implementation */
संरचना tpm_driver {
    u32 *timeouts;
    u32 *durations;
    शून्य (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(शून्य);
    TPMVersion (*get_tpm_version)(शून्य);
    u32 (*init)(शून्य);
    u32 (*activate)(u8 locty);
    u32 (*ready)(शून्य);
    u32 (*senddata)(ध्रुव u8 *ध्रुव data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(शून्य);
    u32 (*waitrespready)(गणना tpmDurationType to_t);
};

बाह्य संरचना tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_अङ्कः_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

स्थिर ध्रुव u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

स्थिर ध्रुव u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
स्थिर u32 tpm_default_dur[3];
स्थिर u32 tpm_default_to[4];

स्थिर u32 crb_cmd_size;
स्थिर शून्य *crb_cmd;
स्थिर u32 crb_resp_size;
स्थिर शून्य *crb_resp;

स्थिर u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    अत्थाय (;;) {
        u8 मानम् = readl(reg);
        सचे ((मानम् & mask) == expect) {
            rc = 0;
            भङ्ग;
        }
        सचे (timer_check(end)) {
            warn_timeout();
            भङ्ग;
        }
        yield();
    }
    पटिनिवत्त rc;
}

स्थिर u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    पटिनिवत्त wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

स्थिर u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    पटिनिवत्त wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

स्थिर u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    पटिनिवत्त wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
स्थिर u32 tis_probe(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    सचे (rc)
        पटिनिवत्त 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    सचे ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    सचे ((ifaceid & 0xf) != 0xf) {
        सचे ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            पटिनिवत्त 0;
        }
        सचे ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            पटिनिवत्त 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    पटिनिवत्त rc;
}

स्थिर TPMVersion tis_get_tpm_version(शून्य)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    सचे ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        सचे (((reg >> 28) & 0x7) == 3)
            पटिनिवत्त TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    अञ्ञथा सचे ((reg & 0xf) == 0) {
        पटिनिवत्त TPM_VERSION_2;
    }

    पटिनिवत्त TPM_VERSION_1_2;
}

स्थिर शून्य init_timeout(पूर्णाङ्क driver)
{
    सचे (tpm_drivers[driver].durations == रिक्त) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               परिमाण(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    सचे (tpm_drivers[driver].timeouts == रिक्त) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               परिमाण(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

स्थिर u32 tis_init(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    पटिनिवत्त 1;
}


स्थिर शून्य set_timeouts(u32 timeouts[4], u32 durations[3])
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    सचे (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * परिमाण(u32));
    सचे (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * परिमाण(u32));
}

स्थिर u32 tis_activate(u8 locty)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 0;
    u8 acc;
    पूर्णाङ्क l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    सचे (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        अत्थाय (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    सचे ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    पटिनिवत्त rc;
}

स्थिर u32 tis_find_active_locality(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u8 locty;

    अत्थाय (locty = 0; locty <= 4; locty++) {
        सचे ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            पटिनिवत्त locty;
    }

    tis_activate(0);

    पटिनिवत्त 0;
}

स्थिर u32 tis_ready(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    पटिनिवत्त rc;
}

स्थिर u32 tis_senddata(ध्रुव u8 *ध्रुव data, u32 len)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    करोहि {
        याव (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            सचे (burst == 0) {
                सचे (timer_check(end)) {
                    warn_timeout();
                    भङ्ग;
                }
                yield();
            }
        }

        सचे (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            भङ्ग;
        }

        याव (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            सचे (burst == 0 || offset == len)
                भङ्ग;
        }

        सचे (offset == len)
            end_loop = 1;
    } याव (end_loop == 0);

    पटिनिवत्त rc;
}

स्थिर u32 tis_readresp(u8 *buffer, u32 *len)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    याव (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        सचे ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            भङ्ग;
    }

    *len = offset;

    पटिनिवत्त rc;
}


स्थिर u32 tis_waitdatavalid(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    सचे (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    पटिनिवत्त rc;
}

स्थिर u32 tis_waitrespready(गणना tpmDurationType to_t)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    सचे (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    पटिनिवत्त rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
स्थिर u32 crb_probe(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    सचे (rc)
        पटिनिवत्त 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    सचे ((ifaceid & 0xf) != 0xf) {
        सचे ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } अञ्ञथा सचे ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            पटिनिवत्त 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    सचे (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        पटिनिवत्त 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    सचे (addr > 0xffffffff)
        पटिनिवत्त 0;

    पटिनिवत्त 1;
}

स्थिर TPMVersion crb_get_tpm_version(शून्य)
{
    /* CRB is supposed to be TPM 2.0 only */
    पटिनिवत्त TPM_VERSION_2;
}

स्थिर u32 crb_init(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 1;

    crb_cmd = (शून्य*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (शून्य*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    पटिनिवत्त 0;
}

स्थिर u32 crb_activate(u8 locty)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    पटिनिवत्त 0;
}

स्थिर u32 crb_find_active_locality(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    पटिनिवत्त 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

स्थिर u32 crb_ready(शून्य)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    पटिनिवत्त rc;
}

स्थिर u32 crb_senddata(ध्रुव u8 *ध्रुव data, u32 len)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    सचे (len > crb_cmd_size)
        पटिनिवत्त 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    पटिनिवत्त 0;
}

स्थिर u32 crb_readresp(u8 *buffer, u32 *len)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u8 locty = crb_find_active_locality();
    सचे (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        पटिनिवत्त 1;

    सचे (*len < 6)
        पटिनिवत्त 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    सचे (expected < 6)
        पटिनिवत्त 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    पटिनिवत्त 0;
}


स्थिर u32 crb_waitdatavalid(शून्य)
{
    पटिनिवत्त 0;
}

स्थिर u32 crb_waitrespready(गणना tpmDurationType to_t)
{
    सचे (!CONFIG_TCGBIOS)
        पटिनिवत्त 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    पटिनिवत्त rc;
}

संरचना tpm_driver tpm_drivers[TPM_अङ्कः_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = रिक्त,
            .durations     = रिक्त,
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
            .timeouts      = रिक्त,
            .durations     = रिक्त,
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

स्थिर u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(शून्य)
{
    unsigned पूर्णाङ्क i;
    अत्थाय (i = 0; i < TPM_अङ्कः_DRIVERS; i++) {
        संरचना tpm_driver *td = &tpm_drivers[i];
        सचे (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            पटिनिवत्त td->get_tpm_version();
        }
    }
    पटिनिवत्त TPM_VERSION_NONE;
}

पूर्णाङ्क
tpmhw_is_present(शून्य)
{
    पटिनिवत्त TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

पूर्णाङ्क
tpmhw_transmit(u8 locty, संरचना tpm_req_header *req,
               शून्य *respbuffer, u32 *respbufferlen,
               गणना tpmDurationType to_t)
{
    सचे (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        पटिनिवत्त -1;

    संरचना tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    सचे (irc != 0) {
        /* tpm could not be activated */
        पटिनिवत्त -1;
    }

    irc = td->senddata((शून्य*)req, be32_to_cpu(req->totlen));
    सचे (irc != 0)
        पटिनिवत्त -1;

    irc = td->waitdatavalid();
    सचे (irc != 0)
        पटिनिवत्त -1;

    irc = td->waitrespready(to_t);
    सचे (irc != 0)
        पटिनिवत्त -1;

    irc = td->readresp(respbuffer, respbufferlen);
    सचे (irc != 0 ||
        *respbufferlen < परिमाण(संरचना tpm_rsp_header))
        पटिनिवत्त -1;

    td->ready();

    पटिनिवत्त 0;
}

शून्य
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    संरचना tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
