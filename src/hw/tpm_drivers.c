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
    शून्य (*समुच्चयः_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(शून्य);
    TPMVersion (*get_tpm_संस्करणम्)(शून्य);
    u32 (*init)(शून्य);
    u32 (*activate)(u8 locty);
    u32 (*ready)(शून्य);
    u32 (*senddata)(ध्रुव u8 *ध्रुव दत्त, u32 len);
    u32 (*readresp)(u8 *बफर, u32 *len);
    u32 (*waitdatavalid)(शून्य);
    u32 (*waitrespready)(गणना tpmDurationType गन्तव्यम्_t);
};

बाह्य संरचना tpm_driver tpm_drivers[];

#परिभाषय TIS_DRIVER_IDX       0
#परिभाषय CRB_DRIVER_IDX       1
#परिभाषय TPM_अङ्कः_DRIVERS      2

#परिभाषय TPM_INVALID_DRIVER   0xf

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
स्थिर u32 tpm_default_गन्तव्यम्[4];

स्थिर u32 crb_cmd_आकार;
स्थिर शून्य *crb_cmd;
स्थिर u32 crb_resp_आकार;
स्थिर शून्य *crb_resp;

स्थिर u32 wait_reg8(u8* reg, u32 time, u8 आवरणम्, u8 expect)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 1;
    u32 अन्तः = timer_calc_usec(time);

    प्रति (;;) {
        u8 मानम् = readl(reg);
        यदि ((मानम् & आवरणम्) == expect) {
            rc = 0;
            भङ्ग;
        }
        यदि (timer_check(अन्तः)) {
            warn_timeout();
            भङ्ग;
        }
        yield();
    }
    प्रत्यागम rc;
}

स्थिर u32 tis_wait_access(u8 locty, u32 time, u8 आवरणम्, u8 expect)
{
    प्रत्यागम wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, आवरणम्, expect);
}

स्थिर u32 tis_wait_sts(u8 locty, u32 time, u8 आवरणम्, u8 expect)
{
    प्रत्यागम wait_reg8(TIS_REG(locty, TIS_REG_STS), time, आवरणम्, expect);
}

स्थिर u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 आवरणम्, u8 expect)
{
    प्रत्यागम wait_reg8(CRB_REG(locty, reg), time, आवरणम्, expect);
}

/* if device is not there, return '0', '1' otherwise */
स्थिर u32 tis_probe(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    यदि (rc)
        प्रत्यागम 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    यदि ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    यदि ((ifaceid & 0xf) != 0xf) {
        यदि ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            प्रत्यागम 0;
        }
        यदि ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            प्रत्यागम 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    प्रत्यागम rc;
}

स्थिर TPMVersion tis_get_tpm_संस्करणम्(शून्य)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    यदि ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        यदि (((reg >> 28) & 0x7) == 3)
            प्रत्यागम TPM_संस्करणम्_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    अन्यथा यदि ((reg & 0xf) == 0) {
        प्रत्यागम TPM_संस्करणम्_2;
    }

    प्रत्यागम TPM_संस्करणम्_1_2;
}

स्थिर शून्य init_timeout(पूर्णाङ्क driver)
{
    यदि (tpm_drivers[driver].durations == रिक्त) {
        u32 *durations = tpm_default_dur;
        स्मृतिं_प्रतिलिख(durations, tpm_default_durations,
               परिमाण(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    यदि (tpm_drivers[driver].timeouts == रिक्त) {
        u32 *timeouts = tpm_default_गन्तव्यम्;
        स्मृतिं_प्रतिलिख(timeouts, tis_default_timeouts,
               परिमाण(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

स्थिर u32 tis_init(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    प्रत्यागम 1;
}


स्थिर शून्य समुच्चयः_timeouts(u32 timeouts[4], u32 durations[3])
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    यदि (tos && tos != tis_default_timeouts && timeouts)
        स्मृतिं_प्रतिलिख(tos, timeouts, 4 * परिमाण(u32));
    यदि (dus && dus != tpm_default_durations && durations)
        स्मृतिं_प्रतिलिख(dus, durations, 3 * परिमाण(u32));
}

स्थिर u32 tis_activate(u8 locty)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 0;
    u8 acc;
    पूर्णाङ्क l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    यदि (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_सक्रिय_LOCALITY)) {
        /* release locality in use top-downwards */
        प्रति (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_सक्रिय_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    यदि ((acc & TIS_ACCESS_सक्रिय_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    प्रत्यागम rc;
}

स्थिर u32 tis_find_सक्रिय_locality(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u8 locty;

    प्रति (locty = 0; locty <= 4; locty++) {
        यदि ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_सक्रिय_LOCALITY))
            प्रत्यागम locty;
    }

    tis_activate(0);

    प्रत्यागम 0;
}

स्थिर u32 tis_ready(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 0;
    u8 locty = tis_find_सक्रिय_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    प्रत्यागम rc;
}

स्थिर u32 tis_senddata(ध्रुव u8 *ध्रुव दत्त, u32 len)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 0;
    u32 विस्थापनम्_2 = 0;
    u32 अन्तः_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_सक्रिय_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 अन्तः = timer_calc_usec(timeout_d);

    कुरु {
        यावत् (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            यदि (burst == 0) {
                यदि (timer_check(अन्तः)) {
                    warn_timeout();
                    भङ्ग;
                }
                yield();
            }
        }

        यदि (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            भङ्ग;
        }

        यावत् (1) {
            writeb(TIS_REG(locty, TIS_REG_दत्त_FIFO), दत्त[विस्थापनम्_2++]);
            burst--;

            यदि (burst == 0 || विस्थापनम्_2 == len)
                भङ्ग;
        }

        यदि (विस्थापनम्_2 == len)
            अन्तः_loop = 1;
    } यावत् (अन्तः_loop == 0);

    प्रत्यागम rc;
}

स्थिर u32 tis_readresp(u8 *बफर, u32 *len)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 0;
    u32 विस्थापनम्_2 = 0;
    u32 sts;
    u8 locty = tis_find_सक्रिय_locality();

    यावत् (विस्थापनम्_2 < *len) {
        बफर[विस्थापनम्_2] = readb(TIS_REG(locty, TIS_REG_दत्त_FIFO));
        विस्थापनम्_2++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        यदि ((sts & TIS_STS_दत्त_उपलब्धम्) == 0)
            भङ्ग;
    }

    *len = विस्थापनम्_2;

    प्रत्यागम rc;
}


स्थिर u32 tis_waitdatavalid(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 0;
    u8 locty = tis_find_सक्रिय_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    यदि (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    प्रत्यागम rc;
}

स्थिर u32 tis_waitrespready(गणना tpmDurationType गन्तव्यम्_t)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 0;
    u8 locty = tis_find_सक्रिय_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[गन्तव्यम्_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    यदि (tis_wait_sts(locty, timeout,
                     TIS_STS_दत्त_उपलब्धम्, TIS_STS_दत्त_उपलब्धम्) != 0)
        rc = 1;

    प्रत्यागम rc;
}

#परिभाषय CRB_STATE_VALID_STS 0b10000000
#परिभाषय CRB_STATE_LOC_ASSIGNED 0x00000010
#परिभाषय CRB_STATE_READY_आवरणम् (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
स्थिर u32 crb_probe(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_आवरणम्, CRB_STATE_VALID_STS);
    यदि (rc)
        प्रत्यागम 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    यदि ((ifaceid & 0xf) != 0xf) {
        यदि ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } अन्यथा यदि ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            प्रत्यागम 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    यदि (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        प्रत्यागम 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    यदि (addr > 0xffffffff)
        प्रत्यागम 0;

    प्रत्यागम 1;
}

स्थिर TPMVersion crb_get_tpm_संस्करणम्(शून्य)
{
    /* CRB is supposed to be TPM 2.0 only */
    प्रत्यागम TPM_संस्करणम्_2;
}

स्थिर u32 crb_init(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 1;

    crb_cmd = (शून्य*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_आकार = readl(CRB_REG(0, CRB_REG_CTRL_CMD_आकार));
    crb_resp = (शून्य*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_आकार = readl(CRB_REG(0, CRB_REG_CTRL_RSP_आकार));

    init_timeout(CRB_DRIVER_IDX);

    प्रत्यागम 0;
}

स्थिर u32 crb_activate(u8 locty)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    प्रत्यागम 0;
}

स्थिर u32 crb_find_सक्रिय_locality(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    प्रत्यागम 0;
}

#परिभाषय CRB_CTRL_REQ_CMD_READY 0b1
#परिभाषय CRB_START_INVOKE 0b1
#परिभाषय CRB_CTRL_STS_ERROR 0b1

स्थिर u32 crb_ready(शून्य)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 0;
    u8 locty = crb_find_सक्रिय_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    प्रत्यागम rc;
}

स्थिर u32 crb_senddata(ध्रुव u8 *ध्रुव दत्त, u32 len)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    यदि (len > crb_cmd_आकार)
        प्रत्यागम 1;

    u8 locty = crb_find_सक्रिय_locality();
    स्मृतिं_प्रतिलिख(crb_cmd, दत्त, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    प्रत्यागम 0;
}

स्थिर u32 crb_readresp(u8 *बफर, u32 *len)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u8 locty = crb_find_सक्रिय_locality();
    यदि (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        प्रत्यागम 1;

    यदि (*len < 6)
        प्रत्यागम 1;

    स्मृतिं_प्रतिलिख(बफर, crb_resp, 6);
    u32 expected = be32_गन्तव्यम्_cpu(*(u32 *) &बफर[2]);
    यदि (expected < 6)
        प्रत्यागम 1;

    *len = (*len < expected) ? *len : expected;

    स्मृतिं_प्रतिलिख(बफर + 6, crb_resp + 6, *len - 6);

    प्रत्यागम 0;
}


स्थिर u32 crb_waitdatavalid(शून्य)
{
    प्रत्यागम 0;
}

स्थिर u32 crb_waitrespready(गणना tpmDurationType गन्तव्यम्_t)
{
    यदि (!CONFIG_TCGBIOS)
        प्रत्यागम 0;

    u32 rc = 0;
    u8 locty = crb_find_सक्रिय_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[गन्तव्यम्_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    प्रत्यागम rc;
}

संरचना tpm_driver tpm_drivers[TPM_अङ्कः_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = रिक्त,
            .durations     = रिक्त,
            .समुच्चयः_timeouts  = समुच्चयः_timeouts,
            .probe         = tis_probe,
            .get_tpm_संस्करणम् = tis_get_tpm_संस्करणम्,
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
            .समुच्चयः_timeouts  = समुच्चयः_timeouts,
            .probe         = crb_probe,
            .get_tpm_संस्करणम् = crb_get_tpm_संस्करणम्,
            .init          = crb_init,
            .activate      = crb_activate,
            .ready         = crb_ready,
            .senddata      = crb_senddata,
            .readresp      = crb_readresp,
            .waitdatavalid = crb_waitdatavalid,
            .waitrespready = crb_waitrespready,
        },
};

स्थिर u8 TPMHW_driver_गन्तव्यम्_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(शून्य)
{
    अचिह्नित पूर्णाङ्क i;
    प्रति (i = 0; i < TPM_अङ्कः_DRIVERS; i++) {
        संरचना tpm_driver *td = &tpm_drivers[i];
        यदि (td->probe() != 0) {
            td->init();
            TPMHW_driver_गन्तव्यम्_use = i;
            प्रत्यागम td->get_tpm_संस्करणम्();
        }
    }
    प्रत्यागम TPM_संस्करणम्_NONE;
}

पूर्णाङ्क
tpmhw_is_present(शून्य)
{
    प्रत्यागम TPMHW_driver_गन्तव्यम्_use != TPM_INVALID_DRIVER;
}

पूर्णाङ्क
tpmhw_transmit(u8 locty, संरचना tpm_req_शीर्षकम् *req,
               शून्य *respbuffer, u32 *respbufferlen,
               गणना tpmDurationType गन्तव्यम्_t)
{
    यदि (TPMHW_driver_गन्तव्यम्_use == TPM_INVALID_DRIVER)
        प्रत्यागम -1;

    संरचना tpm_driver *td = &tpm_drivers[TPMHW_driver_गन्तव्यम्_use];

    u32 irc = td->activate(locty);
    यदि (irc != 0) {
        /* tpm could not be activated */
        प्रत्यागम -1;
    }

    irc = td->senddata((शून्य*)req, be32_गन्तव्यम्_cpu(req->totlen));
    यदि (irc != 0)
        प्रत्यागम -1;

    irc = td->waitdatavalid();
    यदि (irc != 0)
        प्रत्यागम -1;

    irc = td->waitrespready(गन्तव्यम्_t);
    यदि (irc != 0)
        प्रत्यागम -1;

    irc = td->readresp(respbuffer, respbufferlen);
    यदि (irc != 0 ||
        *respbufferlen < परिमाण(संरचना tpm_rsp_शीर्षकम्))
        प्रत्यागम -1;

    td->ready();

    प्रत्यागम 0;
}

शून्य
tpmhw_समुच्चयः_timeouts(u32 timeouts[4], u32 durations[3])
{
    संरचना tpm_driver *td = &tpm_drivers[TPMHW_driver_गन्तव्यम्_use];
    td->समुच्चयः_timeouts(timeouts, durations);
}
