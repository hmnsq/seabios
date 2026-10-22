// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "byteorder.h" // be32_to_cpu
#समावेश "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#समावेश "hw/tpm_drivers.h" // struct tpm_driver
#समावेश "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#समावेश "output.h" // warn_timeout
#समावेश "stacks.h" // yield
#समावेश "string.h" // memcpy
#समावेश "util.h" // timer_calc_usec
#समावेश "x86.h" // readl

/* low level driver implementation */
काष्ठा tpm_driver {
    u32 *timeouts;
    u32 *durations;
    व्योम (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(व्योम);
    TPMVersion (*get_tpm_version)(व्योम);
    u32 (*init)(व्योम);
    u32 (*activate)(u8 locty);
    u32 (*ready)(व्योम);
    u32 (*senddata)(स्थिर u8 *स्थिर तथ्य, u32 len);
    u32 (*readresp)(u8 *बफर, u32 *len);
    u32 (*waitdatavalid)(व्योम);
    u32 (*waitrespready)(क्रमागत tpmDurationType to_t);
};

बाह्य काष्ठा tpm_driver tpm_drivers[];

#घोषणा TIS_DRIVER_IDX       0
#घोषणा CRB_DRIVER_IDX       1
#घोषणा TPM_अंक_DRIVERS      2

#घोषणा TPM_INVALID_DRIVER   0xf

अटल स्थिर u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

अटल स्थिर u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
अटल u32 tpm_default_dur[3];
अटल u32 tpm_default_to[4];

अटल u32 crb_cmd_आकार;
अटल व्योम *crb_cmd;
अटल u32 crb_resp_आकार;
अटल व्योम *crb_resp;

अटल u32 wait_reg8(u8* reg, u32 समय, u8 mask, u8 expect)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(समय);

    क्रम (;;) {
        u8 मूल्य = readl(reg);
        अगर ((मूल्य & mask) == expect) {
            rc = 0;
            अवरोध;
        }
        अगर (timer_check(end)) {
            warn_timeout();
            अवरोध;
        }
        yield();
    }
    वापस rc;
}

अटल u32 tis_wait_access(u8 locty, u32 समय, u8 mask, u8 expect)
{
    वापस wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), समय, mask, expect);
}

अटल u32 tis_wait_sts(u8 locty, u32 समय, u8 mask, u8 expect)
{
    वापस wait_reg8(TIS_REG(locty, TIS_REG_STS), समय, mask, expect);
}

अटल u32 crb_wait_reg(u8 locty, u16 reg, u32 समय, u8 mask, u8 expect)
{
    वापस wait_reg8(CRB_REG(locty, reg), समय, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
अटल u32 tis_probe(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    अगर (rc)
        वापस 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    अगर ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    अगर ((ifaceid & 0xf) != 0xf) {
        अगर ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            वापस 0;
        }
        अगर ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            वापस 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    वापस rc;
}

अटल TPMVersion tis_get_tpm_version(व्योम)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    अगर ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        अगर (((reg >> 28) & 0x7) == 3)
            वापस TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    अन्यथा अगर ((reg & 0xf) == 0) {
        वापस TPM_VERSION_2;
    }

    वापस TPM_VERSION_1_2;
}

अटल व्योम init_timeout(पूर्णांक driver)
{
    अगर (tpm_drivers[driver].durations == शून्य) {
        u32 *durations = tpm_default_dur;
        स_नकल(durations, tpm_default_durations,
               माप(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    अगर (tpm_drivers[driver].timeouts == शून्य) {
        u32 *timeouts = tpm_default_to;
        स_नकल(timeouts, tis_default_timeouts,
               माप(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

अटल u32 tis_init(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    वापस 1;
}


अटल व्योम set_timeouts(u32 timeouts[4], u32 durations[3])
{
    अगर (!CONFIG_TCGBIOS)
        वापस;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    अगर (tos && tos != tis_default_timeouts && timeouts)
        स_नकल(tos, timeouts, 4 * माप(u32));
    अगर (dus && dus != tpm_default_durations && durations)
        स_नकल(dus, durations, 3 * माप(u32));
}

अटल u32 tis_activate(u8 locty)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 0;
    u8 acc;
    पूर्णांक l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    अगर (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        क्रम (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    अगर ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    वापस rc;
}

अटल u32 tis_find_active_locality(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u8 locty;

    क्रम (locty = 0; locty <= 4; locty++) {
        अगर ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            वापस locty;
    }

    tis_activate(0);

    वापस 0;
}

अटल u32 tis_ready(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    वापस rc;
}

अटल u32 tis_senddata(स्थिर u8 *स्थिर तथ्य, u32 len)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    करो {
        जबतक (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            अगर (burst == 0) {
                अगर (timer_check(end)) {
                    warn_timeout();
                    अवरोध;
                }
                yield();
            }
        }

        अगर (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            अवरोध;
        }

        जबतक (1) {
            writeb(TIS_REG(locty, TIS_REG_तथ्य_FIFO), तथ्य[offset++]);
            burst--;

            अगर (burst == 0 || offset == len)
                अवरोध;
        }

        अगर (offset == len)
            end_loop = 1;
    } जबतक (end_loop == 0);

    वापस rc;
}

अटल u32 tis_readresp(u8 *बफर, u32 *len)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    जबतक (offset < *len) {
        बफर[offset] = readb(TIS_REG(locty, TIS_REG_तथ्य_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        अगर ((sts & TIS_STS_तथ्य_AVAILABLE) == 0)
            अवरोध;
    }

    *len = offset;

    वापस rc;
}


अटल u32 tis_waitdatavalid(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    अगर (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    वापस rc;
}

अटल u32 tis_waitrespready(क्रमागत tpmDurationType to_t)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    अगर (tis_wait_sts(locty, timeout,
                     TIS_STS_तथ्य_AVAILABLE, TIS_STS_तथ्य_AVAILABLE) != 0)
        rc = 1;

    वापस rc;
}

#घोषणा CRB_STATE_VALID_STS 0b10000000
#घोषणा CRB_STATE_LOC_ASSIGNED 0x00000010
#घोषणा CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
अटल u32 crb_probe(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    अगर (rc)
        वापस 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    अगर ((ifaceid & 0xf) != 0xf) {
        अगर ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } अन्यथा अगर ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            वापस 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    अगर (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        वापस 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    अगर (addr > 0xffffffff)
        वापस 0;

    वापस 1;
}

अटल TPMVersion crb_get_tpm_version(व्योम)
{
    /* CRB is supposed to be TPM 2.0 only */
    वापस TPM_VERSION_2;
}

अटल u32 crb_init(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 1;

    crb_cmd = (व्योम*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_आकार = readl(CRB_REG(0, CRB_REG_CTRL_CMD_आकार));
    crb_resp = (व्योम*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_आकार = readl(CRB_REG(0, CRB_REG_CTRL_RSP_आकार));

    init_timeout(CRB_DRIVER_IDX);

    वापस 0;
}

अटल u32 crb_activate(u8 locty)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    वापस 0;
}

अटल u32 crb_find_active_locality(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    वापस 0;
}

#घोषणा CRB_CTRL_REQ_CMD_READY 0b1
#घोषणा CRB_START_INVOKE 0b1
#घोषणा CRB_CTRL_STS_ERROR 0b1

अटल u32 crb_ready(व्योम)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    वापस rc;
}

अटल u32 crb_senddata(स्थिर u8 *स्थिर तथ्य, u32 len)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    अगर (len > crb_cmd_आकार)
        वापस 1;

    u8 locty = crb_find_active_locality();
    स_नकल(crb_cmd, तथ्य, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    वापस 0;
}

अटल u32 crb_readresp(u8 *बफर, u32 *len)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u8 locty = crb_find_active_locality();
    अगर (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        वापस 1;

    अगर (*len < 6)
        वापस 1;

    स_नकल(बफर, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &बफर[2]);
    अगर (expected < 6)
        वापस 1;

    *len = (*len < expected) ? *len : expected;

    स_नकल(बफर + 6, crb_resp + 6, *len - 6);

    वापस 0;
}


अटल u32 crb_waitdatavalid(व्योम)
{
    वापस 0;
}

अटल u32 crb_waitrespready(क्रमागत tpmDurationType to_t)
{
    अगर (!CONFIG_TCGBIOS)
        वापस 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    वापस rc;
}

काष्ठा tpm_driver tpm_drivers[TPM_अंक_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = शून्य,
            .durations     = शून्य,
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
            .timeouts      = शून्य,
            .durations     = शून्य,
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

अटल u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(व्योम)
{
    अचिन्हित पूर्णांक i;
    क्रम (i = 0; i < TPM_अंक_DRIVERS; i++) {
        काष्ठा tpm_driver *td = &tpm_drivers[i];
        अगर (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            वापस td->get_tpm_version();
        }
    }
    वापस TPM_VERSION_NONE;
}

पूर्णांक
tpmhw_is_present(व्योम)
{
    वापस TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

पूर्णांक
tpmhw_transmit(u8 locty, काष्ठा tpm_req_header *req,
               व्योम *respbuffer, u32 *respbufferlen,
               क्रमागत tpmDurationType to_t)
{
    अगर (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        वापस -1;

    काष्ठा tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    अगर (irc != 0) {
        /* tpm could not be activated */
        वापस -1;
    }

    irc = td->senddata((व्योम*)req, be32_to_cpu(req->totlen));
    अगर (irc != 0)
        वापस -1;

    irc = td->waitdatavalid();
    अगर (irc != 0)
        वापस -1;

    irc = td->waitrespready(to_t);
    अगर (irc != 0)
        वापस -1;

    irc = td->readresp(respbuffer, respbufferlen);
    अगर (irc != 0 ||
        *respbufferlen < माप(काष्ठा tpm_rsp_header))
        वापस -1;

    td->ready();

    वापस 0;
}

व्योम
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    काष्ठा tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
