#నిర్వచితం_కాకపోతే TPM_DRIVERS_H
#నిర్వచించు TPM_DRIVERS_H

#అన్తర్భావ "types.h" // u32


గణన tpmDurationType {
    TPM_DURATION_TYPE_SHORT = 0,
    TPM_DURATION_TYPE_MEDIUM,
    TPM_DURATION_TYPE_LONG,
};

రకనిర్వచనం u8 TPMVersion;

#నిర్వచించు TPM_సంస్కరణ_NONE 0
#నిర్వచించు TPM_సంస్కరణ_1_2  1
#నిర్వచించు TPM_సంస్కరణ_2    2

TPMVersion tpmhw_probe(శూన్య);
పూర్ణాంకం tpmhw_is_present(శూన్య);
నిర్మాణం tpm_req_శీర్షిక;
పూర్ణాంకం tpmhw_transmit(u8 locty, నిర్మాణం tpm_req_శీర్షిక *req,
                   శూన్య *respbuffer, u32 *respbufferlen,
                   గణన tpmDurationType గమ్యం_t);
శూన్య tpmhw_సమితి_timeouts(u32 timeouts[4], u32 durations[3]);

/* CRB driver */
/* address of locality 0 (CRB) */
#నిర్వచించు TPM_CRB_పునాది_చిరునామా        0xfed40000

#నిర్వచించు CRB_REG(LOCTY, REG)                                 \
    (శూన్య *)(TPM_CRB_పునాది_చిరునామా + (LOCTY << 12) + REG)

/* hardware registers */
#నిర్వచించు CRB_REG_LOC_STATE              0x0
#నిర్వచించు CRB_REG_LOC_CTRL               0x8
#నిర్వచించు CRB_REG_LOC_STS                0xC
#నిర్వచించు CRB_REG_INTF_ID                0x30
#నిర్వచించు CRB_REG_CTRL_EXT               0x38
#నిర్వచించు CRB_REG_CTRL_REQ               0x40
#నిర్వచించు CRB_REG_CTRL_STS               0x44
#నిర్వచించు CRB_REG_CTRL_CANCEL            0x48
#నిర్వచించు CRB_REG_CTRL_ప్రారంభం             0x4C
#నిర్వచించు CRB_REG_INT_ENABLE             0x50
#నిర్వచించు CRB_REG_INT_STS                0x54
#నిర్వచించు CRB_REG_CTRL_CMD_కొలత          0x58
#నిర్వచించు CRB_REG_CTRL_CMD_LADDR         0x5C
#నిర్వచించు CRB_REG_CTRL_CMD_HADDR         0x60
#నిర్వచించు CRB_REG_CTRL_RSP_కొలత          0x64
#నిర్వచించు CRB_REG_CTRL_RSP_ADDR          0x68
#నిర్వచించు CRB_REG_దత్తాంశం_బఫరు            0x80

/* TIS driver */
/* address of locality 0 (TIS) */
#నిర్వచించు TPM_TIS_పునాది_చిరునామా        0xfed40000

#నిర్వచించు TIS_REG(LOCTY, REG) \
    (శూన్య *)(TPM_TIS_పునాది_చిరునామా + (LOCTY << 12) + REG)

/* hardware registers */
#నిర్వచించు TIS_REG_ACCESS                 0x0
#నిర్వచించు TIS_REG_INT_ENABLE             0x8
#నిర్వచించు TIS_REG_INT_సదిశ             0xc
#నిర్వచించు TIS_REG_INT_స్థితి             0x10
#నిర్వచించు TIS_REG_INTF_CAPABILITY        0x14
#నిర్వచించు TIS_REG_STS                    0x18
#నిర్వచించు TIS_REG_దత్తాంశం_FIFO              0x24
#నిర్వచించు TIS_REG_IFACE_ID               0x30
#నిర్వచించు TIS_REG_DID_VID                0xf00
#నిర్వచించు TIS_REG_RID                    0xf04

#నిర్వచించు TIS_STS_VALID                  (1 << 7) /* 0x80 */
#నిర్వచించు TIS_STS_COMMAND_READY          (1 << 6) /* 0x40 */
#నిర్వచించు TIS_STS_TPM_GO                 (1 << 5) /* 0x20 */
#నిర్వచించు TIS_STS_దత్తాంశం_అందుబాటు         (1 << 4) /* 0x10 */
#నిర్వచించు TIS_STS_EXPECT                 (1 << 3) /* 0x08 */
#నిర్వచించు TIS_STS_RESPONSE_RETRY         (1 << 1) /* 0x02 */

#నిర్వచించు TIS_ACCESS_TPM_REG_VALID_STS   (1 << 7) /* 0x80 */
#నిర్వచించు TIS_ACCESS_చురుకైన_LOCALITY     (1 << 5) /* 0x20 */
#నిర్వచించు TIS_ACCESS_BEEN_SEIZED         (1 << 4) /* 0x10 */
#నిర్వచించు TIS_ACCESS_SEIZE               (1 << 3) /* 0x08 */
#నిర్వచించు TIS_ACCESS_PENDING_REQUEST     (1 << 2) /* 0x04 */
#నిర్వచించు TIS_ACCESS_REQUEST_USE         (1 << 1) /* 0x02 */
#నిర్వచించు TIS_ACCESS_TPM_ESTABLISHMENT   (1 << 0) /* 0x01 */

/*
 * Default TIS timeouts used before getting them from the TPM itself
 */
#నిర్వచించు TIS_DEFAULT_TIMEOUT_A           750000 /* us */
#నిర్వచించు TIS_DEFAULT_TIMEOUT_B          2000000 /* us */
#నిర్వచించు TIS_DEFAULT_TIMEOUT_C           750000 /* us */
#నిర్వచించు TIS_DEFAULT_TIMEOUT_D           750000 /* us */

/*
 * Default TIS 2 timeouts given in TPM Profile (TPT) Spec
 */
#నిర్వచించు TIS2_DEFAULT_TIMEOUT_A          750000 /* us */
#నిర్వచించు TIS2_DEFAULT_TIMEOUT_B         2000000 /* us */
#నిర్వచించు TIS2_DEFAULT_TIMEOUT_C          200000 /* us */
#నిర్వచించు TIS2_DEFAULT_TIMEOUT_D           30000 /* us */

గణన tisTimeoutType {
    TIS_TIMEOUT_TYPE_A = 0,
    TIS_TIMEOUT_TYPE_B,
    TIS_TIMEOUT_TYPE_C,
    TIS_TIMEOUT_TYPE_D,
};

/*
 * Default command durations used before getting them from the
 * TPM itself
 */
#నిర్వచించు TPM_DEFAULT_DURATION_SHORT      2000000 /* us */
#నిర్వచించు TPM_DEFAULT_DURATION_MEDIUM    20000000 /* us */
#నిర్వచించు TPM_DEFAULT_DURATION_LONG      60000000 /* us */

/*
 * TPM 2 command durations; we set them to the timeout values
 * given in TPM Profile (PTP) Specification; exceeding those
 * timeout values indicates a faulty TPM.
 */
#నిర్వచించు TPM2_DEFAULT_DURATION_SHORT       750000 /* us */
#నిర్వచించు TPM2_DEFAULT_DURATION_MEDIUM     2000000 /* us */
#నిర్వచించు TPM2_DEFAULT_DURATION_LONG       2000000 /* us */

#అయితే_ముగింపు /* TPM_DRIVERS_H */
