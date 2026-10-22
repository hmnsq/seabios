// Implementation of a TPM driver for the TPM TIS interface
//
// Copyright (C) 2006-2011 IBM Corporation
//
// Authors:
//     Stefan Berger <stefanb@linux.vnet.ibm.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "byteorder.h" // be32_to_cpu
#include "config.h" // CONFIG_TPM_TIS_SHA1THRESHOLD
#include "hw/tpm_drivers.h" // struct tpm_driver
#include "std/tcg.h" // TCG_RESPONSE_TIMEOUT
#include "output.h" // warn_timeout
#include "stacks.h" // yield
#include "string.h" // memcpy
#include "util.h" // timer_calc_usec
#include "x86.h" // readl

/* low level driver implementation */
sa_nracanaa tpm_driver {
    u32 *timeouts;
    u32 *durations;
    void (*set_timeouts)(u32 timeouts[4], u32 durations[3]);
    u32 (*probe)(void);
    TPMVersion (*get_tpm_version)(void);
    u32 (*init)(void);
    u32 (*activate)(u8 locty);
    u32 (*ready)(void);
    u32 (*senddata)(const u8 *const data, u32 len);
    u32 (*readresp)(u8 *buffer, u32 *len);
    u32 (*waitdatavalid)(void);
    u32 (*waitrespready)(ga_nanaa tpmDurationType to_t);
};

extern sa_nracanaa tpm_driver tpm_drivers[];

#define TIS_DRIVER_IDX       0
#define CRB_DRIVER_IDX       1
#define TPM_NUM_DRIVERS      2

#define TPM_INVALID_DRIVER   0xf

static const u32 tis_default_timeouts[4] = {
    TIS_DEFAULT_TIMEOUT_A,
    TIS_DEFAULT_TIMEOUT_B,
    TIS_DEFAULT_TIMEOUT_C,
    TIS_DEFAULT_TIMEOUT_D,
};

static const u32 tpm_default_durations[3] = {
    TPM_DEFAULT_DURATION_SHORT,
    TPM_DEFAULT_DURATION_MEDIUM,
    TPM_DEFAULT_DURATION_LONG,
};

/* determined values */
static u32 tpm_default_dur[3];
static u32 tpm_default_to[4];

static u32 crb_cmd_size;
static void *crb_cmd;
static u32 crb_resp_size;
static void *crb_resp;

static u32 wait_reg8(u8* reg, u32 time, u8 mask, u8 expect)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 1;
    u32 end = timer_calc_usec(time);

    དོན་ལུ (;;) {
        u8 value = readl(reg);
        གལ་སྲིད ((value & mask) == expect) {
            rc = 0;
            bha_nga;
        }
        གལ་སྲིད (timer_check(end)) {
            warn_timeout();
            bha_nga;
        }
        yield();
    }
    ལོག rc;
}

static u32 tis_wait_access(u8 locty, u32 time, u8 mask, u8 expect)
{
    ལོག wait_reg8(TIS_REG(locty, TIS_REG_ACCESS), time, mask, expect);
}

static u32 tis_wait_sts(u8 locty, u32 time, u8 mask, u8 expect)
{
    ལོག wait_reg8(TIS_REG(locty, TIS_REG_STS), time, mask, expect);
}

static u32 crb_wait_reg(u8 locty, u16 reg, u32 time, u8 mask, u8 expect)
{
    ལོག wait_reg8(CRB_REG(locty, reg), time, mask, expect);
}

/* if device is not there, return '0', '1' otherwise */
static u32 tis_probe(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    /* Wait for the interface to report it's ready */
    u32 rc = tis_wait_access(0, TIS_DEFAULT_TIMEOUT_A,
                             TIS_ACCESS_TPM_REG_VALID_STS,
                             TIS_ACCESS_TPM_REG_VALID_STS);
    གལ་སྲིད (rc)
        ལོག 0;

    u32 didvid = readl(TIS_REG(0, TIS_REG_DID_VID));

    གལ་སྲིད ((didvid != 0) && (didvid != 0xffffffff))
        rc = 1;

    /* TPM 2 has an interface register */
    u32 ifaceid = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    གལ་སྲིད ((ifaceid & 0xf) != 0xf) {
        གལ་སྲིད ((ifaceid & 0xf) == 1) {
            /* CRB is active; no TIS */
            ལོག 0;
        }
        གལ་སྲིད ((ifaceid & (1 << 13)) == 0) {
            /* TIS cannot be selected */
            ལོག 0;
        }
        /* write of 0 to bits 17-18 selects TIS */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), 0);
        /* since we only support TIS, we lock it */
        writel(TIS_REG(0, TIS_REG_IFACE_ID), (1 << 19));
    }

    ལོག rc;
}

static TPMVersion tis_get_tpm_version(void)
{
    u32 reg = readl(TIS_REG(0, TIS_REG_IFACE_ID));

    /*
     * FIFO interface as defined in TIS1.3 is active
     * Interface capabilities are defined in TIS_REG_INTF_CAPABILITY
     */
    གལ་སྲིད ((reg & 0xf) == 0xf) {
        reg = readl(TIS_REG(0, TIS_REG_INTF_CAPABILITY));
        /* Interface 1.3 for TPM 2.0 */
        གལ་སྲིད (((reg >> 28) & 0x7) == 3)
            ལོག TPM_VERSION_2;
    }
    /* FIFO interface as defined in PTP for TPM 2.0 is active */
    དེ་མིན གལ་སྲིད ((reg & 0xf) == 0) {
        ལོག TPM_VERSION_2;
    }

    ལོག TPM_VERSION_1_2;
}

static void init_timeout(ཧྲིལ driver)
{
    གལ་སྲིད (tpm_drivers[driver].durations == NULL) {
        u32 *durations = tpm_default_dur;
        memcpy(durations, tpm_default_durations,
               parimaa_na(tpm_default_durations));
        tpm_drivers[driver].durations = durations;
    }

    གལ་སྲིད (tpm_drivers[driver].timeouts == NULL) {
        u32 *timeouts = tpm_default_to;
        memcpy(timeouts, tis_default_timeouts,
               parimaa_na(tis_default_timeouts));
        tpm_drivers[driver].timeouts = timeouts;
    }
}

static u32 tis_init(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 1;

    writeb(TIS_REG(0, TIS_REG_INT_ENABLE), 0);

    init_timeout(TIS_DRIVER_IDX);

    ལོག 1;
}


static void set_timeouts(u32 timeouts[4], u32 durations[3])
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག;

    u32 *tos = tpm_drivers[TIS_DRIVER_IDX].timeouts;
    u32 *dus = tpm_drivers[TIS_DRIVER_IDX].durations;

    གལ་སྲིད (tos && tos != tis_default_timeouts && timeouts)
        memcpy(tos, timeouts, 4 * parimaa_na(u32));
    གལ་སྲིད (dus && dus != tpm_default_durations && durations)
        memcpy(dus, durations, 3 * parimaa_na(u32));
}

static u32 tis_activate(u8 locty)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 0;
    u8 acc;
    ཧྲིལ l;
    u32 timeout_a = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_A];

    གལ་སྲིད (!(readb(TIS_REG(locty, TIS_REG_ACCESS)) &
          TIS_ACCESS_ACTIVE_LOCALITY)) {
        /* release locality in use top-downwards */
        དོན་ལུ (l = 4; l >= 0; l--)
            writeb(TIS_REG(l, TIS_REG_ACCESS),
                   TIS_ACCESS_ACTIVE_LOCALITY);
    }

    /* request access to locality */
    writeb(TIS_REG(locty, TIS_REG_ACCESS), TIS_ACCESS_REQUEST_USE);

    acc = readb(TIS_REG(locty, TIS_REG_ACCESS));
    གལ་སྲིད ((acc & TIS_ACCESS_ACTIVE_LOCALITY)) {
        writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
        rc = tis_wait_sts(locty, timeout_a,
                          TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);
    }

    ལོག rc;
}

static u32 tis_find_active_locality(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u8 locty;

    དོན་ལུ (locty = 0; locty <= 4; locty++) {
        གལ་སྲིད ((readb(TIS_REG(locty, TIS_REG_ACCESS)) &
             TIS_ACCESS_ACTIVE_LOCALITY))
            ལོག locty;
    }

    tis_activate(0);

    ལོག 0;
}

static u32 tis_ready(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_b = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_B];

    writeb(TIS_REG(locty, TIS_REG_STS), TIS_STS_COMMAND_READY);
    rc = tis_wait_sts(locty, timeout_b,
                      TIS_STS_COMMAND_READY, TIS_STS_COMMAND_READY);

    ལོག rc;
}

static u32 tis_senddata(const u8 *const data, u32 len)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 end_loop = 0;
    u16 burst = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_d = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_D];
    u32 end = timer_calc_usec(timeout_d);

    do {
        སྐབས (burst == 0) {
               burst = readl(TIS_REG(locty, TIS_REG_STS)) >> 8;
            གལ་སྲིད (burst == 0) {
                གལ་སྲིད (timer_check(end)) {
                    warn_timeout();
                    bha_nga;
                }
                yield();
            }
        }

        གལ་སྲིད (burst == 0) {
            rc = TCG_RESPONSE_TIMEOUT;
            bha_nga;
        }

        སྐབས (1) {
            writeb(TIS_REG(locty, TIS_REG_DATA_FIFO), data[offset++]);
            burst--;

            གལ་སྲིད (burst == 0 || offset == len)
                bha_nga;
        }

        གལ་སྲིད (offset == len)
            end_loop = 1;
    } སྐབས (end_loop == 0);

    ལོག rc;
}

static u32 tis_readresp(u8 *buffer, u32 *len)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 0;
    u32 offset = 0;
    u32 sts;
    u8 locty = tis_find_active_locality();

    སྐབས (offset < *len) {
        buffer[offset] = readb(TIS_REG(locty, TIS_REG_DATA_FIFO));
        offset++;
        sts = readb(TIS_REG(locty, TIS_REG_STS));
        /* data left ? */
        གལ་སྲིད ((sts & TIS_STS_DATA_AVAILABLE) == 0)
            bha_nga;
    }

    *len = offset;

    ལོག rc;
}


static u32 tis_waitdatavalid(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout_c = tpm_drivers[TIS_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    གལ་སྲིད (tis_wait_sts(locty, timeout_c, TIS_STS_VALID, TIS_STS_VALID) != 0)
        rc = 1;

    ལོག rc;
}

static u32 tis_waitrespready(ga_nanaa tpmDurationType to_t)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 0;
    u8 locty = tis_find_active_locality();
    u32 timeout = tpm_drivers[TIS_DRIVER_IDX].durations[to_t];

    writeb(TIS_REG(locty ,TIS_REG_STS), TIS_STS_TPM_GO);

    གལ་སྲིད (tis_wait_sts(locty, timeout,
                     TIS_STS_DATA_AVAILABLE, TIS_STS_DATA_AVAILABLE) != 0)
        rc = 1;

    ལོག rc;
}

#define CRB_STATE_VALID_STS 0b10000000
#define CRB_STATE_LOC_ASSIGNED 0x00000010
#define CRB_STATE_READY_MASK (CRB_STATE_VALID_STS | CRB_STATE_LOC_ASSIGNED)

/* if device is not there, return '0', '1' otherwise */
static u32 crb_probe(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    /* Wait for the interface to report it's ready */
    u32 rc = crb_wait_reg(0, CRB_REG_LOC_STATE, TIS2_DEFAULT_TIMEOUT_D,
                          CRB_STATE_READY_MASK, CRB_STATE_VALID_STS);
    གལ་སྲིད (rc)
        ལོག 0;

    u32 ifaceid = readl(CRB_REG(0, CRB_REG_INTF_ID));

    གལ་སྲིད ((ifaceid & 0xf) != 0xf) {
        གལ་སྲིད ((ifaceid & 0xf) == 1) {
            /* CRB is active */
        } དེ་མིན གལ་སྲིད ((ifaceid & (1 << 14)) == 0) {
            /* CRB cannot be selected */
            ལོག 0;
        }
        /* write of 1 to bits 17-18 selects CRB */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 17));
        /* lock it */
        writel(CRB_REG(0, CRB_REG_INTF_ID), (1 << 19));
    }

    /* no support for 64 bit addressing yet */
    གལ་སྲིད (readl(CRB_REG(0, CRB_REG_CTRL_CMD_HADDR)))
        ལོག 0;

    u64 addr = readq(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    གལ་སྲིད (addr > 0xffffffff)
        ལོག 0;

    ལོག 1;
}

static TPMVersion crb_get_tpm_version(void)
{
    /* CRB is supposed to be TPM 2.0 only */
    ལོག TPM_VERSION_2;
}

static u32 crb_init(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 1;

    crb_cmd = (void*)readl(CRB_REG(0, CRB_REG_CTRL_CMD_LADDR));
    crb_cmd_size = readl(CRB_REG(0, CRB_REG_CTRL_CMD_SIZE));
    crb_resp = (void*)readl(CRB_REG(0, CRB_REG_CTRL_RSP_ADDR));
    crb_resp_size = readl(CRB_REG(0, CRB_REG_CTRL_RSP_SIZE));

    init_timeout(CRB_DRIVER_IDX);

    ལོག 0;
}

static u32 crb_activate(u8 locty)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    writeb(CRB_REG(locty, CRB_REG_LOC_CTRL), 1);

    ལོག 0;
}

static u32 crb_find_active_locality(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    ལོག 0;
}

#define CRB_CTRL_REQ_CMD_READY 0b1
#define CRB_START_INVOKE 0b1
#define CRB_CTRL_STS_ERROR 0b1

static u32 crb_ready(void)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout_c = tpm_drivers[CRB_DRIVER_IDX].timeouts[TIS_TIMEOUT_TYPE_C];

    writel(CRB_REG(locty, CRB_REG_CTRL_REQ), CRB_CTRL_REQ_CMD_READY);
    rc = crb_wait_reg(locty, CRB_REG_CTRL_REQ, timeout_c,
                      CRB_CTRL_REQ_CMD_READY, 0);

    ལོག rc;
}

static u32 crb_senddata(const u8 *const data, u32 len)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    གལ་སྲིད (len > crb_cmd_size)
        ལོག 1;

    u8 locty = crb_find_active_locality();
    memcpy(crb_cmd, data, len);
    writel(CRB_REG(locty, CRB_REG_CTRL_START), CRB_START_INVOKE);

    ལོག 0;
}

static u32 crb_readresp(u8 *buffer, u32 *len)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u8 locty = crb_find_active_locality();
    གལ་སྲིད (readl(CRB_REG(locty, CRB_REG_CTRL_STS)) & CRB_CTRL_STS_ERROR)
        ལོག 1;

    གལ་སྲིད (*len < 6)
        ལོག 1;

    memcpy(buffer, crb_resp, 6);
    u32 expected = be32_to_cpu(*(u32 *) &buffer[2]);
    གལ་སྲིད (expected < 6)
        ལོག 1;

    *len = (*len < expected) ? *len : expected;

    memcpy(buffer + 6, crb_resp + 6, *len - 6);

    ལོག 0;
}


static u32 crb_waitdatavalid(void)
{
    ལོག 0;
}

static u32 crb_waitrespready(ga_nanaa tpmDurationType to_t)
{
    གལ་སྲིད (!CONFIG_TCGBIOS)
        ལོག 0;

    u32 rc = 0;
    u8 locty = crb_find_active_locality();
    u32 timeout = tpm_drivers[CRB_DRIVER_IDX].durations[to_t];

    rc = crb_wait_reg(locty, CRB_REG_CTRL_START, timeout,
                      CRB_START_INVOKE, 0);

    ལོག rc;
}

sa_nracanaa tpm_driver tpm_drivers[TPM_NUM_DRIVERS] = {
    [TIS_DRIVER_IDX] =
        {
            .timeouts      = NULL,
            .durations     = NULL,
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
            .timeouts      = NULL,
            .durations     = NULL,
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

static u8 TPMHW_driver_to_use = TPM_INVALID_DRIVER;

TPMVersion
tpmhw_probe(void)
{
    unsigned ཧྲིལ i;
    དོན་ལུ (i = 0; i < TPM_NUM_DRIVERS; i++) {
        sa_nracanaa tpm_driver *td = &tpm_drivers[i];
        གལ་སྲིད (td->probe() != 0) {
            td->init();
            TPMHW_driver_to_use = i;
            ལོག td->get_tpm_version();
        }
    }
    ལོག TPM_VERSION_NONE;
}

ཧྲིལ
tpmhw_is_present(void)
{
    ལོག TPMHW_driver_to_use != TPM_INVALID_DRIVER;
}

ཧྲིལ
tpmhw_transmit(u8 locty, sa_nracanaa tpm_req_header *req,
               void *respbuffer, u32 *respbufferlen,
               ga_nanaa tpmDurationType to_t)
{
    གལ་སྲིད (TPMHW_driver_to_use == TPM_INVALID_DRIVER)
        ལོག -1;

    sa_nracanaa tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];

    u32 irc = td->activate(locty);
    གལ་སྲིད (irc != 0) {
        /* tpm could not be activated */
        ལོག -1;
    }

    irc = td->senddata((void*)req, be32_to_cpu(req->totlen));
    གལ་སྲིད (irc != 0)
        ལོག -1;

    irc = td->waitdatavalid();
    གལ་སྲིད (irc != 0)
        ལོག -1;

    irc = td->waitrespready(to_t);
    གལ་སྲིད (irc != 0)
        ལོག -1;

    irc = td->readresp(respbuffer, respbufferlen);
    གལ་སྲིད (irc != 0 ||
        *respbufferlen < parimaa_na(sa_nracanaa tpm_rsp_header))
        ལོག -1;

    td->ready();

    ལོག 0;
}

void
tpmhw_set_timeouts(u32 timeouts[4], u32 durations[3])
{
    sa_nracanaa tpm_driver *td = &tpm_drivers[TPMHW_driver_to_use];
    td->set_timeouts(timeouts, durations);
}
