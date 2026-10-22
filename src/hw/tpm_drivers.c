// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "byteorder.h" // be32_to_cpu
#அந்தர்பாவ "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#அந்தர்பாவ "hw/tpm_drivers.h" // struct tpm_driver
#அந்தர்பாவ "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#அந்தர்பாவ "output.h" // warn_timeout
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "util.h" // timer_calc_usec
#அந்தர்பாவ "x86.h" // readl

/* low level driver implementation */
கட்டமைப்பு tpm_driver {
    u32 *timeouts;
    u32 *durations;
    வெறுமை (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(வெறுமை);
    TPMVersion (*get_tpm_version)(வெறுமை);
    u32 (*init)(வெறுமை);
    u32 (*activate)(u8 locty);
    u32 (*ready)(வெறுமை);
    u32 (*senddata)(மாறிலி u8 *மாறிலி data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(வெறுமை);
    u32 (*waitrespready)(எண்ணிக்கை tpmDurationType to_t);
};

வெளி கட்டமைப்பு tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_இலக்கம்_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

நிலையான மாறிலி u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

நிலையான மாறிலி u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
நிலையான u32 tpm_default_dur[3];
நிலையான u32 tpm_default_to[4];

நிலையான u32 crb_cmd_size;
நிலையான வெறுமை *crb_cmd;
நிலையான u32 crb_resp_size;
நிலையான வெறுமை *crb_resp;

நிலையான u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    க்கு (;;) {
        u8 மதிப்பு = readl(reg);
        எனில் ((மதிப்பு & mask) == expect) {
            rc = 0;
            நிறுத்து;
        }
        எனில் (timer_check(end)) {
            warn_timeout();
            நிறுத்து;
        }
        yield();
    }
    திரும்பு rc;
}

நிலையான u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    திரும்பு wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

நிலையான u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    திரும்பு wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

நிலையான u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    திரும்பு wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
நிலையான u32 tis_probe(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    எனில் (rc)
        திரும்பு 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    எனில் ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    எனில் ((ifaceid & 0xf) != 0xf) {
        எனில் ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            திரும்பு 0;
        }
        எனில் ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            திரும்பு 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    திரும்பு rc;
}

நிலையான TPMVersion tis_get_tpm_version(வெறுமை)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    எனில் ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        எனில் (((reg >> 28) & 0x7) == 3)
            திரும்பு TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    இல்லையெனில் எனில் ((reg & 0xf) == 0) {
        திரும்பு TPM_VERSION_2;
    }

    திரும்பு TPM_VERSION_1_2;
}

நிலையான வெறுமை init_timeout(முழு driver)
{
    எனில் (tpm_drivers[driver].durations == வெற்று) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               அளவு(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    எனில் (tpm_drivers[driver].timeouts == வெற்று) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               அளவு(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

நிலையான u32 tis_init(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    திரும்பு 1;
}


நிலையான வெறுமை set_timeouts(u32 timeouts[4], u32 durations[3])
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    எனில் (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * அளவு(u32));
    எனில் (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * அளவு(u32));
}

நிலையான u32 tis_activate(u8 locty)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 0;
    u8 acc;
    முழு l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    எனில் (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        க்கு (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    எனில் ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    திரும்பு rc;
}

நிலையான u32 tis_find_active_locality(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u8 locty;

    க்கு (locty = 0; locty <= 4; locty++) {
        எனில் ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            திரும்பு locty;
    }

    tis_activate(0);

    திரும்பு 0;
}

நிலையான u32 tis_ready(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    திரும்பு rc;
}

நிலையான u32 tis_senddata(மாறிலி u8 *மாறிலி data, u32 len)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    செய் {
        போது (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            எனில் (burst == 0) {
                எனில் (timer_check(end)) {
                    warn_timeout();
                    நிறுத்து;
                }
                yield();
            }
        }

        எனில் (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            நிறுத்து;
        }

        போது (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            எனில் (burst == 0 || offset == len)
                நிறுத்து;
        }

        எனில் (offset == len)
            end_loop = 1;
    } போது (end_loop == 0);

    திரும்பு rc;
}

நிலையான u32 tis_readresp(u8 *buffer, u32 *len)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    போது (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        எனில் ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            நிறுத்து;
    }

    *len = offset;

    திரும்பு rc;
}


நிலையான u32 tis_waitdatavalid(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    எனில் (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    திரும்பு rc;
}

நிலையான u32 tis_waitrespready(எண்ணிக்கை tpmDurationType to_t)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    எனில் (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    திரும்பு rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
நிலையான u32 crb_probe(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    எனில் (rc)
        திரும்பு 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    எனில் ((ifaceid & 0xf) != 0xf) {
        எனில் ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } இல்லையெனில் எனில் ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            திரும்பு 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    எனில் (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        திரும்பு 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    எனில் (addr > 0xffffffff)
        திரும்பு 0;

    திரும்பு 1;
}

நிலையான TPMVersion crb_get_tpm_version(வெறுமை)
{
    /* CRB is supposed to be TPM 2.0 only */
    திரும்பு TPM_VERSION_2;
}

நிலையான u32 crb_init(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 1;

    crb_cmd = (வெறுமை*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (வெறுமை*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    திரும்பு 0;
}

நிலையான u32 crb_activate(u8 locty)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    திரும்பு 0;
}

நிலையான u32 crb_find_active_locality(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    திரும்பு 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

நிலையான u32 crb_ready(வெறுமை)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    திரும்பு rc;
}

நிலையான u32 crb_senddata(மாறிலி u8 *மாறிலி data, u32 len)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    எனில் (len > crb_cmd_size)
        திரும்பு 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    திரும்பு 0;
}

நிலையான u32 crb_readresp(u8 *buffer, u32 *len)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u8 locty = crb_find_active_locality();
    எனில் (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        திரும்பு 1;

    எனில் (*len < 6)
        திரும்பு 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    எனில் (expected < 6)
        திரும்பு 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    திரும்பு 0;
}


நிலையான u32 crb_waitdatavalid(வெறுமை)
{
    திரும்பு 0;
}

நிலையான u32 crb_waitrespready(எண்ணிக்கை tpmDurationType to_t)
{
    எனில் (!CONFIG_TCGBIOS)
        திரும்பு 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    திரும்பு rc;
}

கட்டமைப்பு tpm_driver tpm_drivers[TPM_இலக்கம்_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = வெற்று,
            .durations     = வெற்று,
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
            .timeouts      = வெற்று,
            .durations     = வெற்று,
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

நிலையான u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(வெறுமை)
{
    unsigned முழு i;
    க்கு (i = 0; i < TPM_இலக்கம்_DRIVERS; i++) {
        கட்டமைப்பு tpm_driver *td = &tpm_drivers[i];
        எனில் (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            திரும்பு td->get_tpm_version();
        }
    }
    திரும்பு TPM_VERSION_NONE;
}

முழு
tpmhw_is_present(வெறுமை)
{
    திரும்பு TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

முழு
tpmhw_transmit(u8 locty, கட்டமைப்பு tpm_req_header *req,
               வெறுமை *respbuffer, u32 *respbufferlen,
               எண்ணிக்கை tpmDurationType to_t)
{
    எனில் (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        திரும்பு -1;

    கட்டமைப்பு tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    எனில் (irc != 0) {
        /* tpm could not be activated */
        திரும்பு -1;
    }

    irc = td->senddata((வெறுமை*)req, be32_to_cpu(req->totlen));
    எனில் (irc != 0)
        திரும்பு -1;

    irc = td->waitdatavalid();
    எனில் (irc != 0)
        திரும்பு -1;

    irc = td->waitrespready(to_t);
    எனில் (irc != 0)
        திரும்பு -1;

    irc = td->readresp(respbuffer, respbufferlen);
    எனில் (irc != 0 ||
        *respbufferlen < அளவு(கட்டமைப்பு tpm_rsp_header))
        திரும்பு -1;

    td->ready();

    திரும்பு 0;
}

வெறுமை
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    கட்டமைப்பு tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
