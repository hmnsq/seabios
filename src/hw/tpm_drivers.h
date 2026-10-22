#اگر_غیر_تعریف_شدہ TPM_DRIVERS_H
#تعریف_کرو TPM_DRIVERS_H

#شامل "types.h" // u32


شمار tpmDurationType {
    TPM_DURATION_TYPE_SHORT = 0,
    TPM_DURATION_TYPE_MEDIUM,
    TPM_DURATION_TYPE_LONG,
};

قسمتعریف u8 TPMVersion;

#تعریف_کرو TPM_نسخہ_NONE 0
#تعریف_کرو TPM_نسخہ_1_2  1
#تعریف_کرو TPM_نسخہ_2    2

TPMVersion tpmhw_probe(خالی);
صحیح tpmhw_is_present(خالی);
ساخت tpm_req_سرخی;
صحیح tpmhw_transmit(u8 locty, ساخت tpm_req_سرخی *req,
                   خالی *respbuffer, u32 *respbufferlen,
                   شمار tpmDurationType مقصد_t);
خالی tpmhw_سیٹ_timeouts(u32 timeouts[4], u32 durations[3]);

/* CRB driver */
/* address of locality 0 (CRB) */
#تعریف_کرو TPM_CRB_بنیاد_پتہ        0xfed40000

#تعریف_کرو CRB_REG(LOCTY, REG)                                 \
    (خالی *)(TPM_CRB_بنیاد_پتہ + (LOCTY << 12) + REG)

/* hardware registers */
#تعریف_کرو CRB_REG_LOC_STATE              0x0
#تعریف_کرو CRB_REG_LOC_CTRL               0x8
#تعریف_کرو CRB_REG_LOC_STS                0xC
#تعریف_کرو CRB_REG_INTF_ID                0x30
#تعریف_کرو CRB_REG_CTRL_EXT               0x38
#تعریف_کرو CRB_REG_CTRL_REQ               0x40
#تعریف_کرو CRB_REG_CTRL_STS               0x44
#تعریف_کرو CRB_REG_CTRL_CANCEL            0x48
#تعریف_کرو CRB_REG_CTRL_آغاز             0x4C
#تعریف_کرو CRB_REG_INT_ENABLE             0x50
#تعریف_کرو CRB_REG_INT_STS                0x54
#تعریف_کرو CRB_REG_CTRL_CMD_سائز          0x58
#تعریف_کرو CRB_REG_CTRL_CMD_LADDR         0x5C
#تعریف_کرو CRB_REG_CTRL_CMD_HADDR         0x60
#تعریف_کرو CRB_REG_CTRL_RSP_سائز          0x64
#تعریف_کرو CRB_REG_CTRL_RSP_ADDR          0x68
#تعریف_کرو CRB_REG_ڈیٹا_بفر            0x80

/* TIS driver */
/* address of locality 0 (TIS) */
#تعریف_کرو TPM_TIS_بنیاد_پتہ        0xfed40000

#تعریف_کرو TIS_REG(LOCTY, REG) \
    (خالی *)(TPM_TIS_بنیاد_پتہ + (LOCTY << 12) + REG)

/* hardware registers */
#تعریف_کرو TIS_REG_ACCESS                 0x0
#تعریف_کرو TIS_REG_INT_ENABLE             0x8
#تعریف_کرو TIS_REG_INT_سمتیہ             0xc
#تعریف_کرو TIS_REG_INT_حالت             0x10
#تعریف_کرو TIS_REG_INTF_CAPABILITY        0x14
#تعریف_کرو TIS_REG_STS                    0x18
#تعریف_کرو TIS_REG_ڈیٹا_FIFO              0x24
#تعریف_کرو TIS_REG_IFACE_ID               0x30
#تعریف_کرو TIS_REG_DID_VID                0xf00
#تعریف_کرو TIS_REG_RID                    0xf04

#تعریف_کرو TIS_STS_VALID                  (1 << 7) /* 0x80 */
#تعریف_کرو TIS_STS_COMMAND_READY          (1 << 6) /* 0x40 */
#تعریف_کرو TIS_STS_TPM_GO                 (1 << 5) /* 0x20 */
#تعریف_کرو TIS_STS_ڈیٹا_دستیاب         (1 << 4) /* 0x10 */
#تعریف_کرو TIS_STS_EXPECT                 (1 << 3) /* 0x08 */
#تعریف_کرو TIS_STS_RESPONSE_RETRY         (1 << 1) /* 0x02 */

#تعریف_کرو TIS_ACCESS_TPM_REG_VALID_STS   (1 << 7) /* 0x80 */
#تعریف_کرو TIS_ACCESS_فعال_LOCALITY     (1 << 5) /* 0x20 */
#تعریف_کرو TIS_ACCESS_BEEN_SEIZED         (1 << 4) /* 0x10 */
#تعریف_کرو TIS_ACCESS_SEIZE               (1 << 3) /* 0x08 */
#تعریف_کرو TIS_ACCESS_PENDING_REQUEST     (1 << 2) /* 0x04 */
#تعریف_کرو TIS_ACCESS_REQUEST_USE         (1 << 1) /* 0x02 */
#تعریف_کرو TIS_ACCESS_TPM_ESTABLISHMENT   (1 << 0) /* 0x01 */

/*
 * Default TIS timeouts used before getting them from the TPM itself
 */
#تعریف_کرو TIS_DEFAULT_TIMEOUT_A           750000 /* us */
#تعریف_کرو TIS_DEFAULT_TIMEOUT_B          2000000 /* us */
#تعریف_کرو TIS_DEFAULT_TIMEOUT_C           750000 /* us */
#تعریف_کرو TIS_DEFAULT_TIMEOUT_D           750000 /* us */

/*
 * Default TIS 2 timeouts given in TPM Profile (TPT) Spec
 */
#تعریف_کرو TIS2_DEFAULT_TIMEOUT_A          750000 /* us */
#تعریف_کرو TIS2_DEFAULT_TIMEOUT_B         2000000 /* us */
#تعریف_کرو TIS2_DEFAULT_TIMEOUT_C          200000 /* us */
#تعریف_کرو TIS2_DEFAULT_TIMEOUT_D           30000 /* us */

شمار tisTimeoutType {
    TIS_TIMEOUT_TYPE_A = 0,
    TIS_TIMEOUT_TYPE_B,
    TIS_TIMEOUT_TYPE_C,
    TIS_TIMEOUT_TYPE_D,
};

/*
 * Default command durations used before getting them from the
 * TPM itself
 */
#تعریف_کرو TPM_DEFAULT_DURATION_SHORT      2000000 /* us */
#تعریف_کرو TPM_DEFAULT_DURATION_MEDIUM    20000000 /* us */
#تعریف_کرو TPM_DEFAULT_DURATION_LONG      60000000 /* us */

/*
 * TPM 2 command durations; we set them to the timeout values
 * given in TPM Profile (PTP) Specification; exceeding those
 * timeout values indicates a faulty TPM.
 */
#تعریف_کرو TPM2_DEFAULT_DURATION_SHORT       750000 /* us */
#تعریف_کرو TPM2_DEFAULT_DURATION_MEDIUM     2000000 /* us */
#تعریف_کرو TPM2_DEFAULT_DURATION_LONG       2000000 /* us */

#اگر_ختم /* TPM_DRIVERS_H */
