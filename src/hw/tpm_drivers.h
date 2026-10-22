#यदि_अपरिभाषितम् TPM_DRIVERS_H
#परिभाषय TPM_DRIVERS_H

#अन्तर्भाव "types.h" // u32


गणना tpmDurationType {
    TPM_DURATION_TYPE_SHORT = 0,
    TPM_DURATION_TYPE_MEDIUM,
    TPM_DURATION_TYPE_LONG,
};

प्रकारनिर्देश u8 TPMVersion;

#परिभाषय TPM_संस्करणम्_NONE 0
#परिभाषय TPM_संस्करणम्_1_2  1
#परिभाषय TPM_संस्करणम्_2    2

TPMVersion tpmhw_probe(शून्य);
पूर्णाङ्क tpmhw_is_present(शून्य);
संरचना tpm_req_शीर्षकम्;
पूर्णाङ्क tpmhw_transmit(u8 locty, संरचना tpm_req_शीर्षकम् *req,
                   शून्य *respbuffer, u32 *respbufferlen,
                   गणना tpmDurationType गन्तव्यम्_t);
शून्य tpmhw_समुच्चयः_timeouts(u32 timeouts[4], u32 durations[3]);

/* CRB driver */
/* address of locality 0 (CRB) */
#परिभाषय TPM_CRB_आधारः_स्थानाङ्कः        0xfed40000

#परिभाषय CRB_REG(LOCTY, REG)                                 \
    (शून्य *)(TPM_CRB_आधारः_स्थानाङ्कः + (LOCTY << 12) + REG)

/* hardware registers */
#परिभाषय CRB_REG_LOC_STATE              0x0
#परिभाषय CRB_REG_LOC_CTRL               0x8
#परिभाषय CRB_REG_LOC_STS                0xC
#परिभाषय CRB_REG_INTF_ID                0x30
#परिभाषय CRB_REG_CTRL_EXT               0x38
#परिभाषय CRB_REG_CTRL_REQ               0x40
#परिभाषय CRB_REG_CTRL_STS               0x44
#परिभाषय CRB_REG_CTRL_CANCEL            0x48
#परिभाषय CRB_REG_CTRL_START             0x4C
#परिभाषय CRB_REG_INT_ENABLE             0x50
#परिभाषय CRB_REG_INT_STS                0x54
#परिभाषय CRB_REG_CTRL_CMD_आकार          0x58
#परिभाषय CRB_REG_CTRL_CMD_LADDR         0x5C
#परिभाषय CRB_REG_CTRL_CMD_HADDR         0x60
#परिभाषय CRB_REG_CTRL_RSP_आकार          0x64
#परिभाषय CRB_REG_CTRL_RSP_ADDR          0x68
#परिभाषय CRB_REG_दत्त_बफर            0x80

/* TIS driver */
/* address of locality 0 (TIS) */
#परिभाषय TPM_TIS_आधारः_स्थानाङ्कः        0xfed40000

#परिभाषय TIS_REG(LOCTY, REG) \
    (शून्य *)(TPM_TIS_आधारः_स्थानाङ्कः + (LOCTY << 12) + REG)

/* hardware registers */
#परिभाषय TIS_REG_ACCESS                 0x0
#परिभाषय TIS_REG_INT_ENABLE             0x8
#परिभाषय TIS_REG_INT_सदिशम्             0xc
#परिभाषय TIS_REG_INT_स्थितिः             0x10
#परिभाषय TIS_REG_INTF_CAPABILITY        0x14
#परिभाषय TIS_REG_STS                    0x18
#परिभाषय TIS_REG_दत्त_FIFO              0x24
#परिभाषय TIS_REG_IFACE_ID               0x30
#परिभाषय TIS_REG_DID_VID                0xf00
#परिभाषय TIS_REG_RID                    0xf04

#परिभाषय TIS_STS_VALID                  (1 << 7) /* 0x80 */
#परिभाषय TIS_STS_COMMAND_READY          (1 << 6) /* 0x40 */
#परिभाषय TIS_STS_TPM_GO                 (1 << 5) /* 0x20 */
#परिभाषय TIS_STS_दत्त_उपलब्धम्         (1 << 4) /* 0x10 */
#परिभाषय TIS_STS_EXPECT                 (1 << 3) /* 0x08 */
#परिभाषय TIS_STS_RESPONSE_RETRY         (1 << 1) /* 0x02 */

#परिभाषय TIS_ACCESS_TPM_REG_VALID_STS   (1 << 7) /* 0x80 */
#परिभाषय TIS_ACCESS_सक्रिय_LOCALITY     (1 << 5) /* 0x20 */
#परिभाषय TIS_ACCESS_BEEN_SEIZED         (1 << 4) /* 0x10 */
#परिभाषय TIS_ACCESS_SEIZE               (1 << 3) /* 0x08 */
#परिभाषय TIS_ACCESS_PENDING_REQUEST     (1 << 2) /* 0x04 */
#परिभाषय TIS_ACCESS_REQUEST_USE         (1 << 1) /* 0x02 */
#परिभाषय TIS_ACCESS_TPM_ESTABLISHMENT   (1 << 0) /* 0x01 */

/*
 * Default TIS timeouts used before getting them from the TPM itself
 */
#परिभाषय TIS_DEFAULT_TIMEOUT_A           750000 /* us */
#परिभाषय TIS_DEFAULT_TIMEOUT_B          2000000 /* us */
#परिभाषय TIS_DEFAULT_TIMEOUT_C           750000 /* us */
#परिभाषय TIS_DEFAULT_TIMEOUT_D           750000 /* us */

/*
 * Default TIS 2 timeouts given in TPM Profile (TPT) Spec
 */
#परिभाषय TIS2_DEFAULT_TIMEOUT_A          750000 /* us */
#परिभाषय TIS2_DEFAULT_TIMEOUT_B         2000000 /* us */
#परिभाषय TIS2_DEFAULT_TIMEOUT_C          200000 /* us */
#परिभाषय TIS2_DEFAULT_TIMEOUT_D           30000 /* us */

गणना tisTimeoutType {
    TIS_TIMEOUT_TYPE_A = 0,
    TIS_TIMEOUT_TYPE_B,
    TIS_TIMEOUT_TYPE_C,
    TIS_TIMEOUT_TYPE_D,
};

/*
 * Default command durations used before getting them from the
 * TPM itself
 */
#परिभाषय TPM_DEFAULT_DURATION_SHORT      2000000 /* us */
#परिभाषय TPM_DEFAULT_DURATION_MEDIUM    20000000 /* us */
#परिभाषय TPM_DEFAULT_DURATION_LONG      60000000 /* us */

/*
 * TPM 2 command durations; we set them to the timeout values
 * given in TPM Profile (PTP) Specification; exceeding those
 * timeout values indicates a faulty TPM.
 */
#परिभाषय TPM2_DEFAULT_DURATION_SHORT       750000 /* us */
#परिभाषय TPM2_DEFAULT_DURATION_MEDIUM     2000000 /* us */
#परिभाषय TPM2_DEFAULT_DURATION_LONG       2000000 /* us */

#यदि_अन्तः /* TPM_DRIVERS_H */
