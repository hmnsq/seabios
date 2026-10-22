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
    u32 (*senddata)(स्थिरांक u8 *स्थिरांक data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(शून्य);
    u32 (*waitrespready)(गणना tpmDurationType to_t);
};

बाह्य संरचना tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_अंक_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

स्थिर स्थिरांक u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

स्थिर स्थिरांक u32 tpm_default_durations[3] = {
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

स्थिर u32 wait_reg8(u8* reg, u32 समय, u8 mask, u8 expect)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(समय);

    साठी (;;) {
        u8 मूल्य = readl(reg);
        जर ((मूल्य & mask) == expect) {
            rc = 0;
            थांबा;
        }
        जर (timer_check(end)) {
            warn_timeout();
            थांबा;
        }
        yield();
    }
    परत rc;
}

स्थिर u32 tis_wait_access(u8 locty, u32 समय, u8 mask, u8 expect)
{
    परत wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), समय, mask, expect);
}

स्थिर u32 tis_wait_sts(u8 locty, u32 समय, u8 mask, u8 expect)
{
    परत wait_reg8(TIS_REG(locty, TIS_REG_STS), समय, mask, expect);
}

स्थिर u32 crb_wait_reg(u8 locty, u16 reg, u32 समय, u8 mask, u8 expect)
{
    परत wait_reg8(CRB_REG(locty, reg), समय, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
स्थिर u32 tis_probe(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    जर (rc)
        परत 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    जर ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    जर ((ifaceid & 0xf) != 0xf) {
        जर ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            परत 0;
        }
        जर ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            परत 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    परत rc;
}

स्थिर TPMVersion tis_get_tpm_version(शून्य)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    जर ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        जर (((reg >> 28) & 0x7) == 3)
            परत TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    अन्यथा जर ((reg & 0xf) == 0) {
        परत TPM_VERSION_2;
    }

    परत TPM_VERSION_1_2;
}

स्थिर शून्य init_timeout(पूर्णांक driver)
{
    जर (tpm_drivers[driver].durations == रिक्त) {
        u32 *durations = tpm_default_dur;
        स_नकल(durations, tpm_default_durations,
               आकार(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    जर (tpm_drivers[driver].timeouts == रिक्त) {
        u32 *timeouts = tpm_default_to;
        स_नकल(timeouts, tis_default_timeouts,
               आकार(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

स्थिर u32 tis_init(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    परत 1;
}


स्थिर शून्य set_timeouts(u32 timeouts[4], u32 durations[3])
{
    जर (!CONFIG_TCGBIOS)
        परत;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    जर (tos && tos != tis_default_timeouts && timeouts)
        स_नकल(tos, timeouts, 4 * आकार(u32));
    जर (dus && dus != tpm_default_durations && durations)
        स_नकल(dus, durations, 3 * आकार(u32));
}

स्थिर u32 tis_activate(u8 locty)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 0;
    u8 acc;
    पूर्णांक l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    जर (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        साठी (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    जर ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    परत rc;
}

स्थिर u32 tis_find_active_locality(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u8 locty;

    साठी (locty = 0; locty <= 4; locty++) {
        जर ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            परत locty;
    }

    tis_activate(0);

    परत 0;
}

स्थिर u32 tis_ready(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    परत rc;
}

स्थिर u32 tis_senddata(स्थिरांक u8 *स्थिरांक data, u32 len)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    करा {
        पर्यंत (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            जर (burst == 0) {
                जर (timer_check(end)) {
                    warn_timeout();
                    थांबा;
                }
                yield();
            }
        }

        जर (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            थांबा;
        }

        पर्यंत (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            जर (burst == 0 || offset == len)
                थांबा;
        }

        जर (offset == len)
            end_loop = 1;
    } पर्यंत (end_loop == 0);

    परत rc;
}

स्थिर u32 tis_readresp(u8 *buffer, u32 *len)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    पर्यंत (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        जर ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            थांबा;
    }

    *len = offset;

    परत rc;
}


स्थिर u32 tis_waitdatavalid(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    जर (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    परत rc;
}

स्थिर u32 tis_waitrespready(गणना tpmDurationType to_t)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    जर (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    परत rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
स्थिर u32 crb_probe(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    जर (rc)
        परत 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    जर ((ifaceid & 0xf) != 0xf) {
        जर ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } अन्यथा जर ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            परत 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    जर (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        परत 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    जर (addr > 0xffffffff)
        परत 0;

    परत 1;
}

स्थिर TPMVersion crb_get_tpm_version(शून्य)
{
    /* CRB is supposed to be TPM 2.0 only */
    परत TPM_VERSION_2;
}

स्थिर u32 crb_init(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 1;

    crb_cmd = (शून्य*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (शून्य*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    परत 0;
}

स्थिर u32 crb_activate(u8 locty)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    परत 0;
}

स्थिर u32 crb_find_active_locality(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    परत 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

स्थिर u32 crb_ready(शून्य)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    परत rc;
}

स्थिर u32 crb_senddata(स्थिरांक u8 *स्थिरांक data, u32 len)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    जर (len > crb_cmd_size)
        परत 1;

    u8 locty = crb_find_active_locality();
    स_नकल(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    परत 0;
}

स्थिर u32 crb_readresp(u8 *buffer, u32 *len)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u8 locty = crb_find_active_locality();
    जर (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        परत 1;

    जर (*len < 6)
        परत 1;

    स_नकल(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    जर (expected < 6)
        परत 1;

    *len = (*len < expected) ? *len : expected;

    स_नकल(buffer + 6, crb_resp + 6, *len - 6);

    परत 0;
}


स्थिर u32 crb_waitdatavalid(शून्य)
{
    परत 0;
}

स्थिर u32 crb_waitrespready(गणना tpmDurationType to_t)
{
    जर (!CONFIG_TCGBIOS)
        परत 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    परत rc;
}

संरचना tpm_driver tpm_drivers[TPM_अंक_DRIVERS] = {
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
    अचिन्हित पूर्णांक i;
    साठी (i = 0; i < TPM_अंक_DRIVERS; i++) {
        संरचना tpm_driver *td = &tpm_drivers[i];
        जर (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            परत td->get_tpm_version();
        }
    }
    परत TPM_VERSION_NONE;
}

पूर्णांक
tpmhw_is_present(शून्य)
{
    परत TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

पूर्णांक
tpmhw_transmit(u8 locty, संरचना tpm_req_header *req,
               शून्य *respbuffer, u32 *respbufferlen,
               गणना tpmDurationType to_t)
{
    जर (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        परत -1;

    संरचना tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    जर (irc != 0) {
        /* tpm could not be activated */
        परत -1;
    }

    irc = td->senddata((शून्य*)req, be32_to_cpu(req->totlen));
    जर (irc != 0)
        परत -1;

    irc = td->waitdatavalid();
    जर (irc != 0)
        परत -1;

    irc = td->waitrespready(to_t);
    जर (irc != 0)
        परत -1;

    irc = td->readresp(respbuffer, respbufferlen);
    जर (irc != 0 ||
        *respbufferlen < आकार(संरचना tpm_rsp_header))
        परत -1;

    td->ready();

    परत 0;
}

शून्य
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    संरचना tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
