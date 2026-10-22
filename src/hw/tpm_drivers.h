#اگر_تعریف_نشده TPM_DRIVERS_H
#تعریف_کن TPM_DRIVERS_H

#ضمن "types.h" // u32


تعداد tpmDurationType {
    TPM_DURATION_TYPE_SHORT = 0,
    TPM_DURATION_TYPE_MEDIUM,
    TPM_DURATION_TYPE_LONG,
};

تعريفنوع u8 TPMVersion;

#تعریف_کن TPM_نسخه_NONE 0
#تعریف_کن TPM_نسخه_1_2  1
#تعریف_کن TPM_نسخه_2    2

TPMVersion tpmhw_probe(فراغ);
صحيح tpmhw_is_present(فراغ);
بنية tpm_req_سرآیند;
صحيح tpmhw_transmit(u8 locty, بنية tpm_req_سرآیند *req,
                   فراغ *respbuffer, u32 *respbufferlen,
                   تعداد tpmDurationType مقصد_t);
فراغ tpmhw_مجموعه_timeouts(u32 timeouts[4], u32 durations[3]);

/* CRB driver */
/* address of locality 0 (CRB) */
#تعریف_کن TPM_CRB_پایه_آدرس        0xfed40000

#تعریف_کن CRB_REG(LOCTY, REG)                                 \
    (فراغ *)(TPM_CRB_پایه_آدرس + (LOCTY << 12) + REG)

/* hardware registers */
#تعریف_کن CRB_REG_LOC_STATE              0x0
#تعریف_کن CRB_REG_LOC_CTRL               0x8
#تعریف_کن CRB_REG_LOC_STS                0xC
#تعریف_کن CRB_REG_INTF_ID                0x30
#تعریف_کن CRB_REG_CTRL_EXT               0x38
#تعریف_کن CRB_REG_CTRL_REQ               0x40
#تعریف_کن CRB_REG_CTRL_STS               0x44
#تعریف_کن CRB_REG_CTRL_CANCEL            0x48
#تعریف_کن CRB_REG_CTRL_شروع             0x4C
#تعریف_کن CRB_REG_INT_ENABLE             0x50
#تعریف_کن CRB_REG_INT_STS                0x54
#تعریف_کن CRB_REG_CTRL_CMD_اندازه          0x58
#تعریف_کن CRB_REG_CTRL_CMD_LADDR         0x5C
#تعریف_کن CRB_REG_CTRL_CMD_HADDR         0x60
#تعریف_کن CRB_REG_CTRL_RSP_اندازه          0x64
#تعریف_کن CRB_REG_CTRL_RSP_ADDR          0x68
#تعریف_کن CRB_REG_داده_میانگیر            0x80

/* TIS driver */
/* address of locality 0 (TIS) */
#تعریف_کن TPM_TIS_پایه_آدرس        0xfed40000

#تعریف_کن TIS_REG(LOCTY, REG) \
    (فراغ *)(TPM_TIS_پایه_آدرس + (LOCTY << 12) + REG)

/* hardware registers */
#تعریف_کن TIS_REG_ACCESS                 0x0
#تعریف_کن TIS_REG_INT_ENABLE             0x8
#تعریف_کن TIS_REG_INT_وکتور             0xc
#تعریف_کن TIS_REG_INT_وضعیت             0x10
#تعریف_کن TIS_REG_INTF_CAPABILITY        0x14
#تعریف_کن TIS_REG_STS                    0x18
#تعریف_کن TIS_REG_داده_FIFO              0x24
#تعریف_کن TIS_REG_IFACE_ID               0x30
#تعریف_کن TIS_REG_DID_VID                0xf00
#تعریف_کن TIS_REG_RID                    0xf04

#تعریف_کن TIS_STS_VALID                  (1 << 7) /* 0x80 */
#تعریف_کن TIS_STS_COMMAND_READY          (1 << 6) /* 0x40 */
#تعریف_کن TIS_STS_TPM_GO                 (1 << 5) /* 0x20 */
#تعریف_کن TIS_STS_داده_موجود         (1 << 4) /* 0x10 */
#تعریف_کن TIS_STS_EXPECT                 (1 << 3) /* 0x08 */
#تعریف_کن TIS_STS_RESPONSE_RETRY         (1 << 1) /* 0x02 */

#تعریف_کن TIS_ACCESS_TPM_REG_VALID_STS   (1 << 7) /* 0x80 */
#تعریف_کن TIS_ACCESS_فعال_LOCALITY     (1 << 5) /* 0x20 */
#تعریف_کن TIS_ACCESS_BEEN_SEIZED         (1 << 4) /* 0x10 */
#تعریف_کن TIS_ACCESS_SEIZE               (1 << 3) /* 0x08 */
#تعریف_کن TIS_ACCESS_PENDING_REQUEST     (1 << 2) /* 0x04 */
#تعریف_کن TIS_ACCESS_REQUEST_USE         (1 << 1) /* 0x02 */
#تعریف_کن TIS_ACCESS_TPM_ESTABLISHMENT   (1 << 0) /* 0x01 */

/*
 * Default TIS timeouts used before getting them from the TPM itself
 */
#تعریف_کن TIS_DEFAULT_TIMEOUT_A           750000 /* us */
#تعریف_کن TIS_DEFAULT_TIMEOUT_B          2000000 /* us */
#تعریف_کن TIS_DEFAULT_TIMEOUT_C           750000 /* us */
#تعریف_کن TIS_DEFAULT_TIMEOUT_D           750000 /* us */

/*
 * Default TIS 2 timeouts given in TPM Profile (TPT) Spec
 */
#تعریف_کن TIS2_DEFAULT_TIMEOUT_A          750000 /* us */
#تعریف_کن TIS2_DEFAULT_TIMEOUT_B         2000000 /* us */
#تعریف_کن TIS2_DEFAULT_TIMEOUT_C          200000 /* us */
#تعریف_کن TIS2_DEFAULT_TIMEOUT_D           30000 /* us */

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
#تعریف_کن TPM_DEFAULT_DURATION_SHORT      2000000 /* us */
#تعریف_کن TPM_DEFAULT_DURATION_MEDIUM    20000000 /* us */
#تعریف_کن TPM_DEFAULT_DURATION_LONG      60000000 /* us */

/*
 * TPM 2 command durations; we set them to the timeout values
 * given in TPM Profile (PTP) Specification; exceeding those
 * timeout values indicates a faulty TPM.
 */
#تعریف_کن TPM2_DEFAULT_DURATION_SHORT       750000 /* us */
#تعریف_کن TPM2_DEFAULT_DURATION_MEDIUM     2000000 /* us */
#تعریف_کن TPM2_DEFAULT_DURATION_LONG       2000000 /* us */

#پایان_اگر /* TPM_DRIVERS_H */
