#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ TPM_DRIVERS_H
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_DRIVERS_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32


ಎಣಿಕೆ tpmDurationType {
    TPM_DURATION_TYPE_SHORT = 0,
    TPM_DURATION_TYPE_MEDIUM,
    TPM_DURATION_TYPE_LONG,
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u8 TPMVersion;

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ಆವೃತ್ತಿ_NONE 0
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ಆವೃತ್ತಿ_1_2  1
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ಆವೃತ್ತಿ_2    2

TPMVersion tpmhw_probe(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ tpmhw_is_present(ಶೂನ್ಯ);
ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ;
ಪೂರ್ಣಾಂಕ tpmhw_transmit(u8 locty, ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ *req,
                   ಶೂನ್ಯ *respbuffer, u32 *respbufferlen,
                   ಎಣಿಕೆ tpmDurationType ಗಮ್ಯ_t);
ಶೂನ್ಯ tpmhw_ಗಣ_timeouts(u32 timeouts[4], u32 durations[3]);

/* CRB driver */
/* address of locality 0 (CRB) */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_CRB_ತಳ_ವಿಳಾಸ        0xfed40000

#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG(LOCTY, REG)                                 \
    (ಶೂನ್ಯ *)(TPM_CRB_ತಳ_ವಿಳಾಸ + (LOCTY << 12) + REG)

/* hardware registers */
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_LOC_STATE              0x0
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_LOC_CTRL               0x8
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_LOC_STS                0xC
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_INTF_ID                0x30
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_EXT               0x38
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_REQ               0x40
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_STS               0x44
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_CANCEL            0x48
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_ಆರಂಭ             0x4C
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_INT_ENABLE             0x50
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_INT_STS                0x54
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_CMD_ಗಾತ್ರ          0x58
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_CMD_LADDR         0x5C
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_CMD_HADDR         0x60
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_RSP_ಗಾತ್ರ          0x64
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_CTRL_RSP_ADDR          0x68
#ವ್ಯಾಖ್ಯಾನಿಸು CRB_REG_ದತ್ತಾಂಶ_ಬಫರ್            0x80

/* TIS driver */
/* address of locality 0 (TIS) */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_TIS_ತಳ_ವಿಳಾಸ        0xfed40000

#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG(LOCTY, REG) \
    (ಶೂನ್ಯ *)(TPM_TIS_ತಳ_ವಿಳಾಸ + (LOCTY << 12) + REG)

/* hardware registers */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_ACCESS                 0x0
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_INT_ENABLE             0x8
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_INT_ಸದಿಶ             0xc
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_INT_ಸ್ಥಿತಿ             0x10
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_INTF_CAPABILITY        0x14
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_STS                    0x18
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_ದತ್ತಾಂಶ_FIFO              0x24
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_IFACE_ID               0x30
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_DID_VID                0xf00
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_REG_RID                    0xf04

#ವ್ಯಾಖ್ಯಾನಿಸು TIS_STS_VALID                  (1 << 7) /* 0x80 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_STS_COMMAND_READY          (1 << 6) /* 0x40 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_STS_TPM_GO                 (1 << 5) /* 0x20 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_STS_ದತ್ತಾಂಶ_ಲಭ್ಯ         (1 << 4) /* 0x10 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_STS_EXPECT                 (1 << 3) /* 0x08 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_STS_RESPONSE_RETRY         (1 << 1) /* 0x02 */

#ವ್ಯಾಖ್ಯಾನಿಸು TIS_ACCESS_TPM_REG_VALID_STS   (1 << 7) /* 0x80 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_ACCESS_ಸಕ್ರಿಯ_LOCALITY     (1 << 5) /* 0x20 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_ACCESS_BEEN_SEIZED         (1 << 4) /* 0x10 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_ACCESS_SEIZE               (1 << 3) /* 0x08 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_ACCESS_PENDING_REQUEST     (1 << 2) /* 0x04 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_ACCESS_REQUEST_USE         (1 << 1) /* 0x02 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_ACCESS_TPM_ESTABLISHMENT   (1 << 0) /* 0x01 */

/*
 * Default TIS timeouts used before getting them from the TPM itself
 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_DEFAULT_TIMEOUT_A           750000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_DEFAULT_TIMEOUT_B          2000000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_DEFAULT_TIMEOUT_C           750000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS_DEFAULT_TIMEOUT_D           750000 /* us */

/*
 * Default TIS 2 timeouts given in TPM Profile (TPT) Spec
 */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS2_DEFAULT_TIMEOUT_A          750000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS2_DEFAULT_TIMEOUT_B         2000000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS2_DEFAULT_TIMEOUT_C          200000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TIS2_DEFAULT_TIMEOUT_D           30000 /* us */

ಎಣಿಕೆ tisTimeoutType {
    TIS_TIMEOUT_TYPE_A = 0,
    TIS_TIMEOUT_TYPE_B,
    TIS_TIMEOUT_TYPE_C,
    TIS_TIMEOUT_TYPE_D,
};

/*
 * Default command durations used before getting them from the
 * TPM itself
 */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_DEFAULT_DURATION_SHORT      2000000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_DEFAULT_DURATION_MEDIUM    20000000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_DEFAULT_DURATION_LONG      60000000 /* us */

/*
 * TPM 2 command durations; we set them to the timeout values
 * given in TPM Profile (PTP) Specification; exceeding those
 * timeout values indicates a faulty TPM.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_DEFAULT_DURATION_SHORT       750000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_DEFAULT_DURATION_MEDIUM     2000000 /* us */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_DEFAULT_DURATION_LONG       2000000 /* us */

#ಆದರೆ_ಅಂತ್ಯ /* TPM_DRIVERS_H */
