#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TPM_DRIVERS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_DRIVERS_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32


ਗਿਣਤੀ tpmDurationType {
    TPM_DURATION_TYPE_SHORT = 0,
    TPM_DURATION_TYPE_MEDIUM,
    TPM_DURATION_TYPE_LONG,
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ u8 TPMVersion;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ਸੰਸਕਰਣ_NONE 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ਸੰਸਕਰਣ_1_2  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ਸੰਸਕਰਣ_2    2

TPMVersion tpmhw_probe(ਖਾਲੀ);
ਪੂਰਨ tpmhw_is_present(ਖਾਲੀ);
ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ;
ਪੂਰਨ tpmhw_transmit(u8 locty, ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ *req,
                   ਖਾਲੀ *respbuffer, u32 *respbufferlen,
                   ਗਿਣਤੀ tpmDurationType ਮੰਜ਼ਿਲ_t);
ਖਾਲੀ tpmhw_ਸੈੱਟ_timeouts(u32 timeouts[4], u32 durations[3]);

/* CRB driver */
/* address of locality 0 (CRB) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_CRB_ਅਧਾਰ_ਪਤਾ        0xfed40000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG(LOCTY, REG)                                 \
    (ਖਾਲੀ *)(TPM_CRB_ਅਧਾਰ_ਪਤਾ + (LOCTY << 12) + REG)

/* hardware registers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_LOC_STATE              0x0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_LOC_CTRL               0x8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_LOC_STS                0xC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_INTF_ID                0x30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_EXT               0x38
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_REQ               0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_STS               0x44
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_CANCEL            0x48
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_ਸ਼ੁਰੂ             0x4C
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_INT_ENABLE             0x50
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_INT_STS                0x54
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_CMD_ਮਾਪ          0x58
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_CMD_LADDR         0x5C
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_CMD_HADDR         0x60
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_RSP_ਮਾਪ          0x64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_CTRL_RSP_ADDR          0x68
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CRB_REG_ਅੰਕੜੇ_ਬਫਰ            0x80

/* TIS driver */
/* address of locality 0 (TIS) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_TIS_ਅਧਾਰ_ਪਤਾ        0xfed40000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG(LOCTY, REG) \
    (ਖਾਲੀ *)(TPM_TIS_ਅਧਾਰ_ਪਤਾ + (LOCTY << 12) + REG)

/* hardware registers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_ACCESS                 0x0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_INT_ENABLE             0x8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_INT_ਸਦਿਸ਼             0xc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_INT_ਸਥਿਤੀ             0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_INTF_CAPABILITY        0x14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_STS                    0x18
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_ਅੰਕੜੇ_FIFO              0x24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_IFACE_ID               0x30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_DID_VID                0xf00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_REG_RID                    0xf04

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_STS_VALID                  (1 << 7) /* 0x80 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_STS_COMMAND_READY          (1 << 6) /* 0x40 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_STS_TPM_GO                 (1 << 5) /* 0x20 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_STS_ਅੰਕੜੇ_ਉਪਲਬਧ         (1 << 4) /* 0x10 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_STS_EXPECT                 (1 << 3) /* 0x08 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_STS_RESPONSE_RETRY         (1 << 1) /* 0x02 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_ACCESS_TPM_REG_VALID_STS   (1 << 7) /* 0x80 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_ACCESS_ਸਰਗਰਮ_LOCALITY     (1 << 5) /* 0x20 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_ACCESS_BEEN_SEIZED         (1 << 4) /* 0x10 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_ACCESS_SEIZE               (1 << 3) /* 0x08 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_ACCESS_PENDING_REQUEST     (1 << 2) /* 0x04 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_ACCESS_REQUEST_USE         (1 << 1) /* 0x02 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_ACCESS_TPM_ESTABLISHMENT   (1 << 0) /* 0x01 */

/*
 * Default TIS timeouts used before getting them from the TPM itself
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_DEFAULT_TIMEOUT_A           750000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_DEFAULT_TIMEOUT_B          2000000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_DEFAULT_TIMEOUT_C           750000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS_DEFAULT_TIMEOUT_D           750000 /* us */

/*
 * Default TIS 2 timeouts given in TPM Profile (TPT) Spec
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS2_DEFAULT_TIMEOUT_A          750000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS2_DEFAULT_TIMEOUT_B         2000000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS2_DEFAULT_TIMEOUT_C          200000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIS2_DEFAULT_TIMEOUT_D           30000 /* us */

ਗਿਣਤੀ tisTimeoutType {
    TIS_TIMEOUT_TYPE_A = 0,
    TIS_TIMEOUT_TYPE_B,
    TIS_TIMEOUT_TYPE_C,
    TIS_TIMEOUT_TYPE_D,
};

/*
 * Default command durations used before getting them from the
 * TPM itself
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_DEFAULT_DURATION_SHORT      2000000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_DEFAULT_DURATION_MEDIUM    20000000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_DEFAULT_DURATION_LONG      60000000 /* us */

/*
 * TPM 2 command durations; we set them to the timeout values
 * given in TPM Profile (PTP) Specification; exceeding those
 * timeout values indicates a faulty TPM.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_DEFAULT_DURATION_SHORT       750000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_DEFAULT_DURATION_MEDIUM     2000000 /* us */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_DEFAULT_DURATION_LONG       2000000 /* us */

#ਜੇ_ਅੰਤ /* TPM_DRIVERS_H */
