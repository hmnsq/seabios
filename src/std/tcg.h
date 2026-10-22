#నిర్వచితం_కాకపోతే STD_TCG_H
#నిర్వచించు STD_TCG_H

#అన్తర్భావ "types.h"

#నిర్వచించు SHA1_BUFSIZE                20
#నిర్వచించు SHA256_BUFSIZE              32
#నిర్వచించు SHA384_BUFSIZE              48
#నిర్వచించు SHA512_BUFSIZE              64
#నిర్వచించు SM3_256_BUFSIZE             32
#నిర్వచించు SHA3_256_BUFSIZE            32
#నిర్వచించు SHA3_384_BUFSIZE            48
#నిర్వచించు SHA3_512_BUFSIZE            64


/****************************************************************
 * 16bit BIOS interface
 ****************************************************************/

/* Define for section 12.3 */
#నిర్వచించు TCG_PC_OK                       0x0
#నిర్వచించు TCG_PC_TPMERROR                 0x1
#నిర్వచించు TCG_PC_LOGOVERFLOW              0x2
#నిర్వచించు TCG_PC_UNSUPPORTED              0x3

#నిర్వచించు TPM_ALG_SHA                     0x4

#నిర్వచించు TCG_MAGIC                       0x41504354L
#నిర్వచించు TCG_సంస్కరణ_MAJOR               1
#నిర్వచించు TCG_సంస్కరణ_MINOR               2

#నిర్వచించు TPM_OK                          0x0
#నిర్వచించు TPM_RET_పునాది                    0x1
#నిర్వచించు TCG_GENERAL_ERROR               (TPM_RET_పునాది + 0x0)
#నిర్వచించు TCG_TPM_IS_LOCKED               (TPM_RET_పునాది + 0x1)
#నిర్వచించు TCG_NO_RESPONSE                 (TPM_RET_పునాది + 0x2)
#నిర్వచించు TCG_INVALID_RESPONSE            (TPM_RET_పునాది + 0x3)
#నిర్వచించు TCG_INVALID_ACCESS_REQUEST      (TPM_RET_పునాది + 0x4)
#నిర్వచించు TCG_FIRMWARE_ERROR              (TPM_RET_పునాది + 0x5)
#నిర్వచించు TCG_INTEGRITY_CHECK_FAILED      (TPM_RET_పునాది + 0x6)
#నిర్వచించు TCG_INVALID_DEVICE_ID           (TPM_RET_పునాది + 0x7)
#నిర్వచించు TCG_INVALID_VENDOR_ID           (TPM_RET_పునాది + 0x8)
#నిర్వచించు TCG_UNABLE_గమ్యం_OPEN              (TPM_RET_పునాది + 0x9)
#నిర్వచించు TCG_UNABLE_గమ్యం_CLOSE             (TPM_RET_పునాది + 0xa)
#నిర్వచించు TCG_RESPONSE_TIMEOUT            (TPM_RET_పునాది + 0xb)
#నిర్వచించు TCG_INVALID_COM_REQUEST         (TPM_RET_పునాది + 0xc)
#నిర్వచించు TCG_INVALID_ADR_REQUEST         (TPM_RET_పునాది + 0xd)
#నిర్వచించు TCG_WRITE_BYTE_ERROR            (TPM_RET_పునాది + 0xe)
#నిర్వచించు TCG_READ_BYTE_ERROR             (TPM_RET_పునాది + 0xf)
#నిర్వచించు TCG_ఖండం_WRITE_TIMEOUT         (TPM_RET_పునాది + 0x10)
#నిర్వచించు TCG_CHAR_WRITE_TIMEOUT          (TPM_RET_పునాది + 0x11)
#నిర్వచించు TCG_CHAR_READ_TIMEOUT           (TPM_RET_పునాది + 0x12)
#నిర్వచించు TCG_ఖండం_READ_TIMEOUT          (TPM_RET_పునాది + 0x13)
#నిర్వచించు TCG_TRANSFER_ABORT              (TPM_RET_పునాది + 0x14)
#నిర్వచించు TCG_INVALID_DRV_FUNCTION        (TPM_RET_పునాది + 0x15)
#నిర్వచించు TCG_OUTPUT_బఫరు_TOO_SHORT     (TPM_RET_పునాది + 0x16)
#నిర్వచించు TCG_FATAL_COM_ERROR             (TPM_RET_పునాది + 0x17)
#నిర్వచించు TCG_INVALID_ఆదానం_PARA          (TPM_RET_పునాది + 0x18)
#నిర్వచించు TCG_TCG_COMMAND_ERROR           (TPM_RET_పునాది + 0x19)
#నిర్వచించు TCG_INTERFACE_SHUTDOWN          (TPM_RET_పునాది + 0x20)
//define TCG_PC_UNSUPPORTED             (TPM_RET_BASE + 0x21)
#నిర్వచించు TCG_PC_TPM_NOT_PRESENT          (TPM_RET_పునాది + 0x22)
#నిర్వచించు TCG_PC_TPM_DEACTIVATED          (TPM_RET_పునాది + 0x23)

/* interrupt identifiers (al register) */
గణన irq_ids {
    TCG_స్థితి_Check = 0,
    TCG_HashLogExtendEvent = 1,
    TCG_Pass_Through_గమ్యం_TPM = 2,
    TCG_ShutdownPreBootInterface = 3,
    TCG_HashLogEvent = 4,
    TCG_HashAll = 5,
    TCG_TSS = 6,
    TCG_CompactHashLogExtendEvent = 7,
};

/* Input and Output blocks for the TCG BIOS commands */

నిర్మాణం hleei_short
{
    u16   ipblength;
    u16   reserved;
    స్థిరరాశి శూన్య *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    స్థిరరాశి శూన్య *logdataptr;
    u32   logdatalen;
} PACKED;

నిర్మాణం hleei_long
{
    u16   ipblength;
    u16   reserved;
    శూన్య *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    u32   reserved2;
    శూన్య *logdataptr;
    u32   logdatalen;
} PACKED;

నిర్మాణం hleeo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

నిర్మాణం pttti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tpmopin[0];
} PACKED;

నిర్మాణం pttto
{
    u16    opblength;
    u16    reserved;
    u8     tpmopout[0];
};

నిర్మాణం hlei
{
    u16    ipblength;
    u16    reserved;
    స్థిరరాశి శూన్య  *hashdataptr;
    u32    hashdatalen;
    u32    pcrindex;
    u32    logeventtype;
    స్థిరరాశి శూన్య  *logdataptr;
    u32    logdatalen;
} PACKED;

నిర్మాణం hleo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
} PACKED;

నిర్మాణం hai
{
    u16    ipblength;
    u16    reserved;
    స్థిరరాశి శూన్య  *hashdataptr;
    u32    hashdatalen;
    u32    algorithmid;
} PACKED;

నిర్మాణం ti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tssoperandin[0];
} PACKED;

నిర్మాణం గమ్యం
{
    u16    opblength;
    u16    reserved;
    u8     tssoperandout[0];
} PACKED;

నిర్మాణం pcpes
{
    u32    pcrindex;
    u32    eventtype;
    u8     digest[SHA1_BUFSIZE];
    u32    eventdatasize;
    u8     event[0];
} PACKED;


/****************************************************************
 * TPM v1.2 hardware commands
 ****************************************************************/

#నిర్వచించు TPM_ORD_Self_Test_నిండిన             0x00000050
#నిర్వచించు TPM_ORD_ForceClear               0x0000005d
#నిర్వచించు TPM_ORD_GetCapability            0x00000065
#నిర్వచించు TPM_ORD_PhysicalEnable           0x0000006f
#నిర్వచించు TPM_ORD_PhysicalDisable          0x00000070
#నిర్వచించు TPM_ORD_సమితి_Owner_Install          0x00000071
#నిర్వచించు TPM_ORD_Physical_సమితి_Deactivated   0x00000072
#నిర్వచించు TPM_ORD_సమితి_తాత్కాలిక_Deactivated       0x00000073
#నిర్వచించు TPM_ORD_Startup                  0x00000099
#నిర్వచించు TPM_ORD_PhysicalPresence         0x4000000a
#నిర్వచించు TPM_ORD_Extend                   0x00000014
#నిర్వచించు TSC_ORD_Reset_Establishment_బిట్టు    0x4000000b

#నిర్వచించు TPM_ST_CLEAR                     0x0001
#నిర్వచించు TPM_ST_STATE                     0x0002
#నిర్వచించు TPM_ST_DEACTIVATED               0x0003

#నిర్వచించు TPM_PP_CMD_ENABLE                0x0020
#నిర్వచించు TPM_PP_PRESENT                   0x0008
#నిర్వచించు TPM_PP_NOT_PRESENT_తాళం          0x0014

/* TPM command error codes */
#నిర్వచించు TPM_INVALID_POSTINIT             0x26
#నిర్వచించు TPM_BAD_LOCALITY                 0x3d

/* TPM command tags */
#నిర్వచించు TPM_ట్యాగ్_RQU_CMD                  0x00c1
#నిర్వచించు TPM_ట్యాగ్_RQU_AUTH1_CMD            0x00c2
#నిర్వచించు TPM_ట్యాగ్_RQU_AUTH2_CMD            0x00c3

నిర్మాణం tpm_req_శీర్షిక {
    u16    ట్యాగ్;
    u32    totlen;
    u32    ordinal;
} PACKED;

నిర్మాణం tpm_rsp_శీర్షిక {
    u16    ట్యాగ్;
    u32    totlen;
    u32    errcode;
} PACKED;

నిర్మాణం tpm_req_extend {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32    pcrindex;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

నిర్మాణం tpm_rsp_extend {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

నిర్మాణం tpm_req_getcap {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32    capArea;
    u32    sub_Cap_కొలత;
    u32    subCap;
} PACKED;

#నిర్వచించు TPM_CAP_FLAG     0x04
#నిర్వచించు TPM_CAP_PROPERTY 0x05
#నిర్వచించు TPM_CAP_FLAG_PERMANENT   0x108
#నిర్వచించు TPM_CAP_FLAG_VOLATILE    0x109
#నిర్వచించు TPM_CAP_PROP_OWNER       0x111
#నిర్వచించు TPM_CAP_PROP_TIS_TIMEOUT 0x115
#నిర్వచించు TPM_CAP_PROP_DURATION    0x120

నిర్మాణం tpm_permanent_flags {
    u16    ట్యాగ్;
    u8     flags[20];
} PACKED;

గణన perm_Flags_సూచీ {
    PERM_FLAG_IDX_DISABLE = 0,
    PERM_FLAG_IDX_OWNERSHIP,
    PERM_FLAG_IDX_DEACTIVATED,
    PERM_FLAG_IDX_READPUBEK,
    PERM_FLAG_IDX_DISABLEOWNERCLEAR,
    PERM_FLAG_IDX_ALLOW_MAINTENANCE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_తాళం,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE,
};

నిర్మాణం tpm_res_getcap_perm_flags {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u32    కొలత;
    నిర్మాణం tpm_permanent_flags perm_flags;
} PACKED;

నిర్మాణం tpm_stclear_flags {
    u16    ట్యాగ్;
    u8     flags[5];
} PACKED;

#నిర్వచించు STCLEAR_FLAG_IDX_DEACTIVATED 0
#నిర్వచించు STCLEAR_FLAG_IDX_DISABLE_FORCE_CLEAR 1
#నిర్వచించు STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE 2
#నిర్వచించు STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE_తాళం 3
#నిర్వచించు STCLEAR_FLAG_IDX_GLOBAL_తాళం 4

నిర్మాణం tpm_res_getcap_stclear_flags {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u32    కొలత;
    నిర్మాణం tpm_stclear_flags stclear_flags;
} PACKED;

నిర్మాణం tpm_res_getcap_ownerauth {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u32    కొలత;
    u8     flag;
} PACKED;

నిర్మాణం tpm_res_getcap_timeouts {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u32    కొలత;
    u32    timeouts[4];
} PACKED;

నిర్మాణం tpm_res_getcap_durations {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u32    కొలత;
    u32    durations[3];
} PACKED;

నిర్మాణం tpm_res_sha1start {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u32    max_అంకె_బైట్లు;
} PACKED;

నిర్మాణం tpm_res_sha1complete {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u8     hash[20];
} PACKED;


/****************************************************************
 * TPM v2.0 hardware commands
 ****************************************************************/

#నిర్వచించు TPM2_NO                     0
#నిర్వచించు TPM2_YES                    1

#నిర్వచించు TPM2_SU_CLEAR               0x0000
#నిర్వచించు TPM2_SU_STATE               0x0001

#నిర్వచించు TPM2_RH_OWNER               0x40000001
#నిర్వచించు TPM2_RS_PW                  0x40000009
#నిర్వచించు TPM2_RH_ENDORSEMENT         0x4000000b
#నిర్వచించు TPM2_RH_PLATFORM            0x4000000c

#నిర్వచించు TPM2_ALG_SHA1               0x0004
#నిర్వచించు TPM2_ALG_SHA256             0x000b
#నిర్వచించు TPM2_ALG_SHA384             0x000c
#నిర్వచించు TPM2_ALG_SHA512             0x000d
#నిర్వచించు TPM2_ALG_SM3_256            0x0012
#నిర్వచించు TPM2_ALG_SHA3_256           0x0027
#నిర్వచించు TPM2_ALG_SHA3_384           0x0028
#నిర్వచించు TPM2_ALG_SHA3_512           0x0029

#నిర్వచించు TPM2_ALG_SHA1_FLAG          (1 << 0)
#నిర్వచించు TPM2_ALG_SHA256_FLAG        (1 << 1)
#నిర్వచించు TPM2_ALG_SHA384_FLAG        (1 << 2)
#నిర్వచించు TPM2_ALG_SHA512_FLAG        (1 << 3)
#నిర్వచించు TPM2_ALG_SM3_256_FLAG       (1 << 4)
#నిర్వచించు TPM2_ALG_SHA3_256_FLAG      (1 << 5)
#నిర్వచించు TPM2_ALG_SHA3_384_FLAG      (1 << 6)
#నిర్వచించు TPM2_ALG_SHA3_512_FLAG      (1 << 7)

/* TPM 2 command tags */
#నిర్వచించు TPM2_ST_NO_SESSIONS         0x8001
#నిర్వచించు TPM2_ST_SESSIONS            0x8002

/* TPM 2 commands */
#నిర్వచించు TPM2_CC_HierarchyControl    0x121
#నిర్వచించు TPM2_CC_Clear               0x126
#నిర్వచించు TPM2_CC_ClearControl        0x127
#నిర్వచించు TPM2_CC_HierarchyChangeAuth 0x129
#నిర్వచించు TPM2_CC_PCR_Allocate        0x12b
#నిర్వచించు TPM2_CC_SelfTest            0x143
#నిర్వచించు TPM2_CC_Startup             0x144
#నిర్వచించు TPM2_CC_Shutdown            0x145
#నిర్వచించు TPM2_CC_StirRandom          0x146
#నిర్వచించు TPM2_CC_GetCapability       0x17a
#నిర్వచించు TPM2_CC_GetRandom           0x17b
#నిర్వచించు TPM2_CC_PCR_Extend          0x182

/* TPM 2 error codes */
#నిర్వచించు TPM2_RC_INITIALIZE          0x100

/* TPM 2 Capabilities */
#నిర్వచించు TPM2_CAP_PCRS               0x00000005

/* TPM 2 data structures */

నిర్మాణం tpm2_req_stirrandom {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u16 కొలత;
    u64 stir;
} PACKED;

నిర్మాణం tpm2_req_getrandom {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u16 బైట్లు_Requested;
} PACKED;

నిర్మాణం tpm2b_20 {
    u16 కొలత;
    u8 బఫరు[20];
} PACKED;

నిర్మాణం tpm2_res_getrandom {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    నిర్మాణం tpm2b_20 rnd;
} PACKED;

నిర్మాణం tpm2_authblock {
    u32 handle;
    u16 noncesize;  /* always 0 */
    u8 contsession; /* always TPM2_YES */
    u16 pwdsize;    /* always 0 */
} PACKED;

నిర్మాణం tpm2_req_hierarchychangeauth {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32 authhandle;
    u32 authblocksize;
    నిర్మాణం tpm2_authblock authblock;
    నిర్మాణం tpm2b_20 newAuth;
} PACKED;

నిర్మాణం tpm2_req_extend {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32 pcrindex;
    u32 authblocksize;
    నిర్మాణం tpm2_authblock authblock;
    u8 digest[0];
} PACKED;

నిర్మాణం tpm2_req_clearcontrol {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32 authhandle;
    u32 authblocksize;
    నిర్మాణం tpm2_authblock authblock;
    u8 disable;
} PACKED;

నిర్మాణం tpm2_req_clear {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32 authhandle;
    u32 authblocksize;
    నిర్మాణం tpm2_authblock authblock;
} PACKED;

నిర్మాణం tpm2_req_hierarchycontrol {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32 authhandle;
    u32 authblocksize;
    నిర్మాణం tpm2_authblock authblock;
    u32 enable;
    u8 state;
} PACKED;

నిర్మాణం tpm2_req_getcapability {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32 capability;
    u32 property;
    u32 propertycount;
} PACKED;

నిర్మాణం tpm2_res_getcapability {
    నిర్మాణం tpm_rsp_శీర్షిక hdr;
    u8 మరిన్ని_దత్తాంశం;
    u32 capability;
    u8 దత్తాంశం[0]; /* capability dependent data */
} PACKED;

నిర్మాణం tpm2_req_pcr_allocate {
    నిర్మాణం tpm_req_శీర్షిక hdr;
    u32 authhandle;
    u32 authblocksize;
    నిర్మాణం tpm2_authblock authblock;
    u32 లెక్క;
    u8 tpms_pcr_selections[4];
} PACKED;

నిర్మాణం tpms_pcr_selection {
    u16 hashAlg;
    u8 కొలత_యొక్క_Select;
    u8 pcrSelect[0];
} PACKED;

నిర్మాణం tpml_pcr_selection {
    u32 లెక్క;
    నిర్మాణం tpms_pcr_selection selections[0];
} PACKED;


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

/* event types: 10.4.1 / table 11 */
#నిర్వచించు EV_POST_సంకేతం             1
#నిర్వచించు EV_NO_ACTION             3
#నిర్వచించు EV_SEPARATOR             4
#నిర్వచించు EV_ACTION                5
#నిర్వచించు EV_EVENT_ట్యాగ్             6
#నిర్వచించు EV_COMPACT_HASH         12
#నిర్వచించు EV_IPL                  13
#నిర్వచించు EV_IPL_విభజన_దత్తాంశం   14

నిర్మాణం tpm2_digest_విలువ {
    u16 hashAlg;
    u8 hash[0]; /* size depends on hashAlg */
} PACKED;

నిర్మాణం tpm2_digest_విలువలు {
    u32 లెక్క;
    నిర్మాణం tpm2_digest_విలువ digest[0];
} PACKED;

// Each entry in the TPM log contains: a tpm_log_header, a variable
// length digest, a tpm_log_trailer, and a variable length event.  The
// 'digest' matches what is sent to the TPM hardware via the Extend
// command.  On TPM1.2 the digest is a SHA1 hash; on TPM2.0 the digest
// contains a tpm2_digest_values struct followed by a variable number
// of tpm2_digest_value structs (as specified by the hardware via the
// TPM2_CAP_PCRS request).
నిర్మాణం tpm_log_శీర్షిక {
    u32 pcrindex;
    u32 eventtype;
    u8 digest[0];
} PACKED;

నిర్మాణం tpm_log_trailer {
    u32 eventdatasize;
    u8 event[0];
} PACKED;

నిర్మాణం TCG_EfiSpecIdEventStruct {
    u8 signature[16];
    u32 platformClass;
    u8 spec_సంస్కరణ_Minor;
    u8 spec_సంస్కరణ_Major;
    u8 specErrata;
    u8 uintn_కొలత;
    u32 సంఖ్య_యొక్క_Algorithms;
    నిర్మాణం TCG_Efi_Spec_Id_Event_Algorithm_కొలత {
        u16 algorithmId;
        u16 digest_కొలత;
    } digestSizes[];
    /*
    u8 vendorInfoSize;
    u8 vendorInfo[0];
    */
} PACKED;

#నిర్వచించు TPM_TCPA_ACPI_CLASS_CLIENT 0

నిర్మాణం pcctes
{
    u32 eventid;
    u32 eventdatasize;
    u8  digest[SHA1_BUFSIZE];
} PACKED;

నిర్మాణం pcctes_romex
{
    u32 eventid;
    u32 eventdatasize;
    u16 reserved;
    u16 pfa;
    u8  digest[SHA1_BUFSIZE];
} PACKED;


/****************************************************************
 * Physical presence interface
 ****************************************************************/

#నిర్వచించు TPM_STATE_ENABLED 1
#నిర్వచించు TPM_STATE_చురుకైన 2
#నిర్వచించు TPM_STATE_OWNED 4
#నిర్వచించు TPM_STATE_OWNERINSTALL 8

#నిర్వచించు TPM_PPI_OP_NOOP 0
#నిర్వచించు TPM_PPI_OP_ENABLE 1
#నిర్వచించు TPM_PPI_OP_DISABLE 2
#నిర్వచించు TPM_PPI_OP_ACTIVATE 3
#నిర్వచించు TPM_PPI_OP_DEACTIVATE 4
#నిర్వచించు TPM_PPI_OP_CLEAR 5
#నిర్వచించు TPM_PPI_OP_సమితి_OWNERINSTALL_TRUE 8
#నిర్వచించు TPM_PPI_OP_సమితి_OWNERINSTALL_FALSE 9

#అయితే_ముగింపు // tcg.h
