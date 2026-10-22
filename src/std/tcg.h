#यदि_अपरिभाषितम् STD_TCG_H
#परिभाषय STD_TCG_H

#अन्तर्भाव "types.h"

#परिभाषय SHA1_BUFSIZE                20
#परिभाषय SHA256_BUFSIZE              32
#परिभाषय SHA384_BUFSIZE              48
#परिभाषय SHA512_BUFSIZE              64
#परिभाषय SM3_256_BUFSIZE             32
#परिभाषय SHA3_256_BUFSIZE            32
#परिभाषय SHA3_384_BUFSIZE            48
#परिभाषय SHA3_512_BUFSIZE            64


/****************************************************************
 * 16bit BIOS interface
 ****************************************************************/

/* Define for section 12.3 */
#परिभाषय TCG_PC_OK                       0x0
#परिभाषय TCG_PC_TPMERROR                 0x1
#परिभाषय TCG_PC_LOGOVERFLOW              0x2
#परिभाषय TCG_PC_UNSUPPORTED              0x3

#परिभाषय TPM_ALG_SHA                     0x4

#परिभाषय TCG_MAGIC                       0x41504354L
#परिभाषय TCG_संस्करणम्_MAJOR               1
#परिभाषय TCG_संस्करणम्_MINOR               2

#परिभाषय TPM_OK                          0x0
#परिभाषय TPM_RET_आधारः                    0x1
#परिभाषय TCG_GENERAL_ERROR               (TPM_RET_आधारः + 0x0)
#परिभाषय TCG_TPM_IS_LOCKED               (TPM_RET_आधारः + 0x1)
#परिभाषय TCG_NO_RESPONSE                 (TPM_RET_आधारः + 0x2)
#परिभाषय TCG_INVALID_RESPONSE            (TPM_RET_आधारः + 0x3)
#परिभाषय TCG_INVALID_ACCESS_REQUEST      (TPM_RET_आधारः + 0x4)
#परिभाषय TCG_FIRMWARE_ERROR              (TPM_RET_आधारः + 0x5)
#परिभाषय TCG_INTEGRITY_CHECK_FAILED      (TPM_RET_आधारः + 0x6)
#परिभाषय TCG_INVALID_DEVICE_ID           (TPM_RET_आधारः + 0x7)
#परिभाषय TCG_INVALID_VENDOR_ID           (TPM_RET_आधारः + 0x8)
#परिभाषय TCG_UNABLE_गन्तव्यम्_OPEN              (TPM_RET_आधारः + 0x9)
#परिभाषय TCG_UNABLE_गन्तव्यम्_CLOSE             (TPM_RET_आधारः + 0xa)
#परिभाषय TCG_RESPONSE_TIMEOUT            (TPM_RET_आधारः + 0xb)
#परिभाषय TCG_INVALID_COM_REQUEST         (TPM_RET_आधारः + 0xc)
#परिभाषय TCG_INVALID_ADR_REQUEST         (TPM_RET_आधारः + 0xd)
#परिभाषय TCG_WRITE_BYTE_ERROR            (TPM_RET_आधारः + 0xe)
#परिभाषय TCG_READ_BYTE_ERROR             (TPM_RET_आधारः + 0xf)
#परिभाषय TCG_खण्डम्_WRITE_TIMEOUT         (TPM_RET_आधारः + 0x10)
#परिभाषय TCG_CHAR_WRITE_TIMEOUT          (TPM_RET_आधारः + 0x11)
#परिभाषय TCG_CHAR_READ_TIMEOUT           (TPM_RET_आधारः + 0x12)
#परिभाषय TCG_खण्डम्_READ_TIMEOUT          (TPM_RET_आधारः + 0x13)
#परिभाषय TCG_TRANSFER_ABORT              (TPM_RET_आधारः + 0x14)
#परिभाषय TCG_INVALID_DRV_FUNCTION        (TPM_RET_आधारः + 0x15)
#परिभाषय TCG_OUTPUT_बफर_TOO_SHORT     (TPM_RET_आधारः + 0x16)
#परिभाषय TCG_FATAL_COM_ERROR             (TPM_RET_आधारः + 0x17)
#परिभाषय TCG_INVALID_निवेशः_PARA          (TPM_RET_आधारः + 0x18)
#परिभाषय TCG_TCG_COMMAND_ERROR           (TPM_RET_आधारः + 0x19)
#परिभाषय TCG_INTERFACE_SHUTDOWN          (TPM_RET_आधारः + 0x20)
//define TCG_PC_UNSUPPORTED             (TPM_RET_BASE + 0x21)
#परिभाषय TCG_PC_TPM_NOT_PRESENT          (TPM_RET_आधारः + 0x22)
#परिभाषय TCG_PC_TPM_DEACTIVATED          (TPM_RET_आधारः + 0x23)

/* interrupt identifiers (al register) */
गणना irq_ids {
    TCG_स्थितिः_Check = 0,
    TCG_HashLogExtendEvent = 1,
    TCG_Pass_Through_गन्तव्यम्_TPM = 2,
    TCG_ShutdownPreBootInterface = 3,
    TCG_HashLogEvent = 4,
    TCG_HashAll = 5,
    TCG_TSS = 6,
    TCG_CompactHashLogExtendEvent = 7,
};

/* Input and Output blocks for the TCG BIOS commands */

संरचना hleei_short
{
    u16   ipblength;
    u16   reserved;
    ध्रुव शून्य *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    ध्रुव शून्य *logdataptr;
    u32   logdatalen;
} PACKED;

संरचना hleei_long
{
    u16   ipblength;
    u16   reserved;
    शून्य *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    u32   reserved2;
    शून्य *logdataptr;
    u32   logdatalen;
} PACKED;

संरचना hleeo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

संरचना pttti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tpmopin[0];
} PACKED;

संरचना pttto
{
    u16    opblength;
    u16    reserved;
    u8     tpmopout[0];
};

संरचना hlei
{
    u16    ipblength;
    u16    reserved;
    ध्रुव शून्य  *hashdataptr;
    u32    hashdatalen;
    u32    pcrindex;
    u32    logeventtype;
    ध्रुव शून्य  *logdataptr;
    u32    logdatalen;
} PACKED;

संरचना hleo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
} PACKED;

संरचना hai
{
    u16    ipblength;
    u16    reserved;
    ध्रुव शून्य  *hashdataptr;
    u32    hashdatalen;
    u32    algorithmid;
} PACKED;

संरचना ti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tssoperandin[0];
} PACKED;

संरचना गन्तव्यम्
{
    u16    opblength;
    u16    reserved;
    u8     tssoperandout[0];
} PACKED;

संरचना pcpes
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

#परिभाषय TPM_ORD_Self_Test_पूर्णम्             0x00000050
#परिभाषय TPM_ORD_ForceClear               0x0000005d
#परिभाषय TPM_ORD_GetCapability            0x00000065
#परिभाषय TPM_ORD_PhysicalEnable           0x0000006f
#परिभाषय TPM_ORD_PhysicalDisable          0x00000070
#परिभाषय TPM_ORD_समुच्चयः_Owner_Install          0x00000071
#परिभाषय TPM_ORD_Physical_समुच्चयः_Deactivated   0x00000072
#परिभाषय TPM_ORD_समुच्चयः_अस्थायी_Deactivated       0x00000073
#परिभाषय TPM_ORD_Startup                  0x00000099
#परिभाषय TPM_ORD_PhysicalPresence         0x4000000a
#परिभाषय TPM_ORD_Extend                   0x00000014
#परिभाषय TSC_ORD_ResetEstablishmentBit    0x4000000b

#परिभाषय TPM_ST_CLEAR                     0x0001
#परिभाषय TPM_ST_STATE                     0x0002
#परिभाषय TPM_ST_DEACTIVATED               0x0003

#परिभाषय TPM_PP_CMD_ENABLE                0x0020
#परिभाषय TPM_PP_PRESENT                   0x0008
#परिभाषय TPM_PP_NOT_PRESENT_तालकम्          0x0014

/* TPM command error codes */
#परिभाषय TPM_INVALID_POSTINIT             0x26
#परिभाषय TPM_BAD_LOCALITY                 0x3d

/* TPM command tags */
#परिभाषय TPM_चिह्नकम्_RQU_CMD                  0x00c1
#परिभाषय TPM_चिह्नकम्_RQU_AUTH1_CMD            0x00c2
#परिभाषय TPM_चिह्नकम्_RQU_AUTH2_CMD            0x00c3

संरचना tpm_req_शीर्षकम् {
    u16    चिह्नकम्;
    u32    totlen;
    u32    ordinal;
} PACKED;

संरचना tpm_rsp_शीर्षकम् {
    u16    चिह्नकम्;
    u32    totlen;
    u32    errcode;
} PACKED;

संरचना tpm_req_extend {
    संरचना tpm_req_शीर्षकम् hdr;
    u32    pcrindex;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

संरचना tpm_rsp_extend {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

संरचना tpm_req_getcap {
    संरचना tpm_req_शीर्षकम् hdr;
    u32    capArea;
    u32    sub_Cap_आकार;
    u32    subCap;
} PACKED;

#परिभाषय TPM_CAP_FLAG     0x04
#परिभाषय TPM_CAP_PROPERTY 0x05
#परिभाषय TPM_CAP_FLAG_PERMANENT   0x108
#परिभाषय TPM_CAP_FLAG_VOLATILE    0x109
#परिभाषय TPM_CAP_PROP_OWNER       0x111
#परिभाषय TPM_CAP_PROP_TIS_TIMEOUT 0x115
#परिभाषय TPM_CAP_PROP_DURATION    0x120

संरचना tpm_permanent_flags {
    u16    चिह्नकम्;
    u8     flags[20];
} PACKED;

गणना perm_Flags_सूचकांक {
    PERM_FLAG_IDX_DISABLE = 0,
    PERM_FLAG_IDX_OWNERSHIP,
    PERM_FLAG_IDX_DEACTIVATED,
    PERM_FLAG_IDX_READPUBEK,
    PERM_FLAG_IDX_DISABLEOWNERCLEAR,
    PERM_FLAG_IDX_ALLOW_MAINTENANCE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_तालकम्,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE,
};

संरचना tpm_res_getcap_perm_flags {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u32    आकार;
    संरचना tpm_permanent_flags perm_flags;
} PACKED;

संरचना tpm_stclear_flags {
    u16    चिह्नकम्;
    u8     flags[5];
} PACKED;

#परिभाषय STCLEAR_FLAG_IDX_DEACTIVATED 0
#परिभाषय STCLEAR_FLAG_IDX_DISABLE_FORCE_CLEAR 1
#परिभाषय STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE 2
#परिभाषय STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE_तालकम् 3
#परिभाषय STCLEAR_FLAG_IDX_GLOBAL_तालकम् 4

संरचना tpm_res_getcap_stclear_flags {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u32    आकार;
    संरचना tpm_stclear_flags stclear_flags;
} PACKED;

संरचना tpm_res_getcap_ownerauth {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u32    आकार;
    u8     flag;
} PACKED;

संरचना tpm_res_getcap_timeouts {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u32    आकार;
    u32    timeouts[4];
} PACKED;

संरचना tpm_res_getcap_durations {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u32    आकार;
    u32    durations[3];
} PACKED;

संरचना tpm_res_sha1start {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u32    max_अङ्कः_अष्टकानि;
} PACKED;

संरचना tpm_res_sha1complete {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u8     hash[20];
} PACKED;


/****************************************************************
 * TPM v2.0 hardware commands
 ****************************************************************/

#परिभाषय TPM2_NO                     0
#परिभाषय TPM2_YES                    1

#परिभाषय TPM2_SU_CLEAR               0x0000
#परिभाषय TPM2_SU_STATE               0x0001

#परिभाषय TPM2_RH_OWNER               0x40000001
#परिभाषय TPM2_RS_PW                  0x40000009
#परिभाषय TPM2_RH_ENDORSEMENT         0x4000000b
#परिभाषय TPM2_RH_PLATFORM            0x4000000c

#परिभाषय TPM2_ALG_SHA1               0x0004
#परिभाषय TPM2_ALG_SHA256             0x000b
#परिभाषय TPM2_ALG_SHA384             0x000c
#परिभाषय TPM2_ALG_SHA512             0x000d
#परिभाषय TPM2_ALG_SM3_256            0x0012
#परिभाषय TPM2_ALG_SHA3_256           0x0027
#परिभाषय TPM2_ALG_SHA3_384           0x0028
#परिभाषय TPM2_ALG_SHA3_512           0x0029

#परिभाषय TPM2_ALG_SHA1_FLAG          (1 << 0)
#परिभाषय TPM2_ALG_SHA256_FLAG        (1 << 1)
#परिभाषय TPM2_ALG_SHA384_FLAG        (1 << 2)
#परिभाषय TPM2_ALG_SHA512_FLAG        (1 << 3)
#परिभाषय TPM2_ALG_SM3_256_FLAG       (1 << 4)
#परिभाषय TPM2_ALG_SHA3_256_FLAG      (1 << 5)
#परिभाषय TPM2_ALG_SHA3_384_FLAG      (1 << 6)
#परिभाषय TPM2_ALG_SHA3_512_FLAG      (1 << 7)

/* TPM 2 command tags */
#परिभाषय TPM2_ST_NO_SESSIONS         0x8001
#परिभाषय TPM2_ST_SESSIONS            0x8002

/* TPM 2 commands */
#परिभाषय TPM2_CC_HierarchyControl    0x121
#परिभाषय TPM2_CC_Clear               0x126
#परिभाषय TPM2_CC_ClearControl        0x127
#परिभाषय TPM2_CC_HierarchyChangeAuth 0x129
#परिभाषय TPM2_CC_PCR_Allocate        0x12b
#परिभाषय TPM2_CC_SelfTest            0x143
#परिभाषय TPM2_CC_Startup             0x144
#परिभाषय TPM2_CC_Shutdown            0x145
#परिभाषय TPM2_CC_StirRandom          0x146
#परिभाषय TPM2_CC_GetCapability       0x17a
#परिभाषय TPM2_CC_GetRandom           0x17b
#परिभाषय TPM2_CC_PCR_Extend          0x182

/* TPM 2 error codes */
#परिभाषय TPM2_RC_INITIALIZE          0x100

/* TPM 2 Capabilities */
#परिभाषय TPM2_CAP_PCRS               0x00000005

/* TPM 2 data structures */

संरचना tpm2_req_stirrandom {
    संरचना tpm_req_शीर्षकम् hdr;
    u16 आकार;
    u64 stir;
} PACKED;

संरचना tpm2_req_getrandom {
    संरचना tpm_req_शीर्षकम् hdr;
    u16 अष्टकानि_Requested;
} PACKED;

संरचना tpm2b_20 {
    u16 आकार;
    u8 बफर[20];
} PACKED;

संरचना tpm2_res_getrandom {
    संरचना tpm_rsp_शीर्षकम् hdr;
    संरचना tpm2b_20 rnd;
} PACKED;

संरचना tpm2_authblock {
    u32 handle;
    u16 noncesize;  /* always 0 */
    u8 contsession; /* always TPM2_YES */
    u16 pwdsize;    /* always 0 */
} PACKED;

संरचना tpm2_req_hierarchychangeauth {
    संरचना tpm_req_शीर्षकम् hdr;
    u32 authhandle;
    u32 authblocksize;
    संरचना tpm2_authblock authblock;
    संरचना tpm2b_20 newAuth;
} PACKED;

संरचना tpm2_req_extend {
    संरचना tpm_req_शीर्षकम् hdr;
    u32 pcrindex;
    u32 authblocksize;
    संरचना tpm2_authblock authblock;
    u8 digest[0];
} PACKED;

संरचना tpm2_req_clearcontrol {
    संरचना tpm_req_शीर्षकम् hdr;
    u32 authhandle;
    u32 authblocksize;
    संरचना tpm2_authblock authblock;
    u8 disable;
} PACKED;

संरचना tpm2_req_clear {
    संरचना tpm_req_शीर्षकम् hdr;
    u32 authhandle;
    u32 authblocksize;
    संरचना tpm2_authblock authblock;
} PACKED;

संरचना tpm2_req_hierarchycontrol {
    संरचना tpm_req_शीर्षकम् hdr;
    u32 authhandle;
    u32 authblocksize;
    संरचना tpm2_authblock authblock;
    u32 enable;
    u8 state;
} PACKED;

संरचना tpm2_req_getcapability {
    संरचना tpm_req_शीर्षकम् hdr;
    u32 capability;
    u32 property;
    u32 propertycount;
} PACKED;

संरचना tpm2_res_getcapability {
    संरचना tpm_rsp_शीर्षकम् hdr;
    u8 अधिकम्_दत्त;
    u32 capability;
    u8 दत्त[0]; /* capability dependent data */
} PACKED;

संरचना tpm2_req_pcr_allocate {
    संरचना tpm_req_शीर्षकम् hdr;
    u32 authhandle;
    u32 authblocksize;
    संरचना tpm2_authblock authblock;
    u32 गणनम्;
    u8 tpms_pcr_selections[4];
} PACKED;

संरचना tpms_pcr_selection {
    u16 hashAlg;
    u8 आकार_सम्बन्धि_Select;
    u8 pcrSelect[0];
} PACKED;

संरचना tpml_pcr_selection {
    u32 गणनम्;
    संरचना tpms_pcr_selection selections[0];
} PACKED;


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

/* event types: 10.4.1 / table 11 */
#परिभाषय EV_POST_संहिता             1
#परिभाषय EV_NO_ACTION             3
#परिभाषय EV_SEPARATOR             4
#परिभाषय EV_ACTION                5
#परिभाषय EV_EVENT_चिह्नकम्             6
#परिभाषय EV_COMPACT_HASH         12
#परिभाषय EV_IPL                  13
#परिभाषय EV_IPL_PARTITION_दत्त   14

संरचना tpm2_digest_मानम् {
    u16 hashAlg;
    u8 hash[0]; /* size depends on hashAlg */
} PACKED;

संरचना tpm2_digest_मानानि {
    u32 गणनम्;
    संरचना tpm2_digest_मानम् digest[0];
} PACKED;

// Each entry in the TPM log contains: a tpm_log_header, a variable
// length digest, a tpm_log_trailer, and a variable length event.  The
// 'digest' matches what is sent to the TPM hardware via the Extend
// command.  On TPM1.2 the digest is a SHA1 hash; on TPM2.0 the digest
// contains a tpm2_digest_values struct followed by a variable number
// of tpm2_digest_value structs (as specified by the hardware via the
// TPM2_CAP_PCRS request).
संरचना tpm_log_शीर्षकम् {
    u32 pcrindex;
    u32 eventtype;
    u8 digest[0];
} PACKED;

संरचना tpm_log_trailer {
    u32 eventdatasize;
    u8 event[0];
} PACKED;

संरचना TCG_EfiSpecIdEventStruct {
    u8 signature[16];
    u32 platformClass;
    u8 spec_संस्करणम्_Minor;
    u8 spec_संस्करणम्_Major;
    u8 specErrata;
    u8 uintn_आकार;
    u32 सङ्ख्या_सम्बन्धि_Algorithms;
    संरचना TCG_Efi_Spec_Id_Event_Algorithm_आकार {
        u16 algorithmId;
        u16 digest_आकार;
    } digestSizes[];
    /*
    u8 vendorInfoSize;
    u8 vendorInfo[0];
    */
} PACKED;

#परिभाषय TPM_TCPA_ACPI_CLASS_CLIENT 0

संरचना pcctes
{
    u32 eventid;
    u32 eventdatasize;
    u8  digest[SHA1_BUFSIZE];
} PACKED;

संरचना pcctes_romex
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

#परिभाषय TPM_STATE_ENABLED 1
#परिभाषय TPM_STATE_सक्रिय 2
#परिभाषय TPM_STATE_OWNED 4
#परिभाषय TPM_STATE_OWNERINSTALL 8

#परिभाषय TPM_PPI_OP_NOOP 0
#परिभाषय TPM_PPI_OP_ENABLE 1
#परिभाषय TPM_PPI_OP_DISABLE 2
#परिभाषय TPM_PPI_OP_ACTIVATE 3
#परिभाषय TPM_PPI_OP_DEACTIVATE 4
#परिभाषय TPM_PPI_OP_CLEAR 5
#परिभाषय TPM_PPI_OP_समुच्चयः_OWNERINSTALL_TRUE 8
#परिभाषय TPM_PPI_OP_समुच्चयः_OWNERINSTALL_FALSE 9

#यदि_अन्तः // tcg.h
