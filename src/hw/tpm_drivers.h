#جيڪڏهن_وصف_نه_ٿيل TPM_DRIVERS_H
#وصف_ڪريو TPM_DRIVERS_H

#ضمن "types.h" // u32


تعداد tpmDurationType {
    TPM_DURATION_TYPE_SHORT = 0,
    TPM_DURATION_TYPE_MEDIUM,
    TPM_DURATION_TYPE_LONG,
};

تعريفنوع u8 TPMVersion;

#وصف_ڪريو TPM_نسخو_NONE 0
#وصف_ڪريو TPM_نسخو_1_2  1
#وصف_ڪريو TPM_نسخو_2    2

TPMVersion tpmhw_probe(فراغ);
صحيح tpmhw_is_present(فراغ);
بنية tpm_req_سرو;
صحيح tpmhw_transmit(u8 locty, بنية tpm_req_سرو *req,
                   فراغ *respbuffer, u32 *respbufferlen,
                   تعداد tpmDurationType منزل_t);
فراغ tpmhw_سيٽ_timeouts(u32 timeouts[4], u32 durations[3]);

/* CRB driver */
/* address of locality 0 (CRB) */
#وصف_ڪريو TPM_CRB_بنياد_پتو        0xfed40000

#وصف_ڪريو CRB_REG(LOCTY, REG)                                 \
    (فراغ *)(TPM_CRB_بنياد_پتو + (LOCTY << 12) + REG)

/* hardware registers */
#وصف_ڪريو CRB_REG_LOC_STATE              0x0
#وصف_ڪريو CRB_REG_LOC_CTRL               0x8
#وصف_ڪريو CRB_REG_LOC_STS                0xC
#وصف_ڪريو CRB_REG_INTF_ID                0x30
#وصف_ڪريو CRB_REG_CTRL_EXT               0x38
#وصف_ڪريو CRB_REG_CTRL_REQ               0x40
#وصف_ڪريو CRB_REG_CTRL_STS               0x44
#وصف_ڪريو CRB_REG_CTRL_CANCEL            0x48
#وصف_ڪريو CRB_REG_CTRL_شروعات             0x4C
#وصف_ڪريو CRB_REG_INT_ENABLE             0x50
#وصف_ڪريو CRB_REG_INT_STS                0x54
#وصف_ڪريو CRB_REG_CTRL_CMD_سائيز          0x58
#وصف_ڪريو CRB_REG_CTRL_CMD_LADDR         0x5C
#وصف_ڪريو CRB_REG_CTRL_CMD_HADDR         0x60
#وصف_ڪريو CRB_REG_CTRL_RSP_سائيز          0x64
#وصف_ڪريو CRB_REG_CTRL_RSP_ADDR          0x68
#وصف_ڪريو CRB_REG_معلومات_بفر            0x80

/* TIS driver */
/* address of locality 0 (TIS) */
#وصف_ڪريو TPM_TIS_بنياد_پتو        0xfed40000

#وصف_ڪريو TIS_REG(LOCTY, REG) \
    (فراغ *)(TPM_TIS_بنياد_پتو + (LOCTY << 12) + REG)

/* hardware registers */
#وصف_ڪريو TIS_REG_ACCESS                 0x0
#وصف_ڪريو TIS_REG_INT_ENABLE             0x8
#وصف_ڪريو TIS_REG_INT_ويڪٽر             0xc
#وصف_ڪريو TIS_REG_INT_حالت             0x10
#وصف_ڪريو TIS_REG_INTF_CAPABILITY        0x14
#وصف_ڪريو TIS_REG_STS                    0x18
#وصف_ڪريو TIS_REG_معلومات_FIFO              0x24
#وصف_ڪريو TIS_REG_IFACE_ID               0x30
#وصف_ڪريو TIS_REG_DID_VID                0xf00
#وصف_ڪريو TIS_REG_RID                    0xf04

#وصف_ڪريو TIS_STS_VALID                  (1 << 7) /* 0x80 */
#وصف_ڪريو TIS_STS_COMMAND_READY          (1 << 6) /* 0x40 */
#وصف_ڪريو TIS_STS_TPM_GO                 (1 << 5) /* 0x20 */
#وصف_ڪريو TIS_STS_معلومات_موجود         (1 << 4) /* 0x10 */
#وصف_ڪريو TIS_STS_EXPECT                 (1 << 3) /* 0x08 */
#وصف_ڪريو TIS_STS_RESPONSE_RETRY         (1 << 1) /* 0x02 */

#وصف_ڪريو TIS_ACCESS_TPM_REG_VALID_STS   (1 << 7) /* 0x80 */
#وصف_ڪريو TIS_ACCESS_سرگرم_LOCALITY     (1 << 5) /* 0x20 */
#وصف_ڪريو TIS_ACCESS_BEEN_SEIZED         (1 << 4) /* 0x10 */
#وصف_ڪريو TIS_ACCESS_SEIZE               (1 << 3) /* 0x08 */
#وصف_ڪريو TIS_ACCESS_PENDING_REQUEST     (1 << 2) /* 0x04 */
#وصف_ڪريو TIS_ACCESS_REQUEST_USE         (1 << 1) /* 0x02 */
#وصف_ڪريو TIS_ACCESS_TPM_ESTABLISHMENT   (1 << 0) /* 0x01 */

/*
 * Default TIS timeouts used before getting them from the TPM itself
 */
#وصف_ڪريو TIS_DEFAULT_TIMEOUT_A           750000 /* us */
#وصف_ڪريو TIS_DEFAULT_TIMEOUT_B          2000000 /* us */
#وصف_ڪريو TIS_DEFAULT_TIMEOUT_C           750000 /* us */
#وصف_ڪريو TIS_DEFAULT_TIMEOUT_D           750000 /* us */

/*
 * Default TIS 2 timeouts given in TPM Profile (TPT) Spec
 */
#وصف_ڪريو TIS2_DEFAULT_TIMEOUT_A          750000 /* us */
#وصف_ڪريو TIS2_DEFAULT_TIMEOUT_B         2000000 /* us */
#وصف_ڪريو TIS2_DEFAULT_TIMEOUT_C          200000 /* us */
#وصف_ڪريو TIS2_DEFAULT_TIMEOUT_D           30000 /* us */

تعداد tisTimeoutType {
    TIS_TIMEOUT_TYPE_A = 0,
    TIS_TIMEOUT_TYPE_B,
    TIS_TIMEOUT_TYPE_C,
    TIS_TIMEOUT_TYPE_D,
};

/*
 * Default command durations used before getting them from the
 * TPM itself
 */
#وصف_ڪريو TPM_DEFAULT_DURATION_SHORT      2000000 /* us */
#وصف_ڪريو TPM_DEFAULT_DURATION_MEDIUM    20000000 /* us */
#وصف_ڪريو TPM_DEFAULT_DURATION_LONG      60000000 /* us */

/*
 * TPM 2 command durations; we set them to the timeout values
 * given in TPM Profile (PTP) Specification; exceeding those
 * timeout values indicates a faulty TPM.
 */
#وصف_ڪريو TPM2_DEFAULT_DURATION_SHORT       750000 /* us */
#وصف_ڪريو TPM2_DEFAULT_DURATION_MEDIUM     2000000 /* us */
#وصف_ڪريو TPM2_DEFAULT_DURATION_LONG       2000000 /* us */

#جيڪڏهن_پورو /* TPM_DRIVERS_H */
