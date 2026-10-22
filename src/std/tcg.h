#اگر_غیر_تعریف_شدہ STD_TCG_H
#تعریف_کرو STD_TCG_H

#شامل "types.h"

#تعریف_کرو SHA1_BUFSIZE                20
#تعریف_کرو SHA256_BUFSIZE              32
#تعریف_کرو SHA384_BUFSIZE              48
#تعریف_کرو SHA512_BUFSIZE              64
#تعریف_کرو SM3_256_BUFSIZE             32
#تعریف_کرو SHA3_256_BUFSIZE            32
#تعریف_کرو SHA3_384_BUFSIZE            48
#تعریف_کرو SHA3_512_BUFSIZE            64


/****************************************************************
 * 16bit BIOS interface
 ****************************************************************/

/* Define for section 12.3 */
#تعریف_کرو TCG_PC_OK                       0x0
#تعریف_کرو TCG_PC_TPMERROR                 0x1
#تعریف_کرو TCG_PC_LOGOVERFLOW              0x2
#تعریف_کرو TCG_PC_UNSUPPORTED              0x3

#تعریف_کرو TPM_ALG_SHA                     0x4

#تعریف_کرو TCG_MAGIC                       0x41504354L
#تعریف_کرو TCG_نسخہ_MAJOR               1
#تعریف_کرو TCG_نسخہ_MINOR               2

#تعریف_کرو TPM_OK                          0x0
#تعریف_کرو TPM_RET_بنیاد                    0x1
#تعریف_کرو TCG_GENERAL_ERROR               (TPM_RET_بنیاد + 0x0)
#تعریف_کرو TCG_TPM_IS_LOCKED               (TPM_RET_بنیاد + 0x1)
#تعریف_کرو TCG_NO_RESPONSE                 (TPM_RET_بنیاد + 0x2)
#تعریف_کرو TCG_INVALID_RESPONSE            (TPM_RET_بنیاد + 0x3)
#تعریف_کرو TCG_INVALID_ACCESS_REQUEST      (TPM_RET_بنیاد + 0x4)
#تعریف_کرو TCG_FIRMWARE_ERROR              (TPM_RET_بنیاد + 0x5)
#تعریف_کرو TCG_INTEGRITY_CHECK_FAILED      (TPM_RET_بنیاد + 0x6)
#تعریف_کرو TCG_INVALID_DEVICE_ID           (TPM_RET_بنیاد + 0x7)
#تعریف_کرو TCG_INVALID_VENDOR_ID           (TPM_RET_بنیاد + 0x8)
#تعریف_کرو TCG_UNABLE_مقصد_OPEN              (TPM_RET_بنیاد + 0x9)
#تعریف_کرو TCG_UNABLE_مقصد_CLOSE             (TPM_RET_بنیاد + 0xa)
#تعریف_کرو TCG_RESPONSE_TIMEOUT            (TPM_RET_بنیاد + 0xb)
#تعریف_کرو TCG_INVALID_COM_REQUEST         (TPM_RET_بنیاد + 0xc)
#تعریف_کرو TCG_INVALID_ADR_REQUEST         (TPM_RET_بنیاد + 0xd)
#تعریف_کرو TCG_WRITE_BYTE_ERROR            (TPM_RET_بنیاد + 0xe)
#تعریف_کرو TCG_READ_BYTE_ERROR             (TPM_RET_بنیاد + 0xf)
#تعریف_کرو TCG_بلاک_WRITE_TIMEOUT         (TPM_RET_بنیاد + 0x10)
#تعریف_کرو TCG_CHAR_WRITE_TIMEOUT          (TPM_RET_بنیاد + 0x11)
#تعریف_کرو TCG_CHAR_READ_TIMEOUT           (TPM_RET_بنیاد + 0x12)
#تعریف_کرو TCG_بلاک_READ_TIMEOUT          (TPM_RET_بنیاد + 0x13)
#تعریف_کرو TCG_TRANSFER_ABORT              (TPM_RET_بنیاد + 0x14)
#تعریف_کرو TCG_INVALID_DRV_FUNCTION        (TPM_RET_بنیاد + 0x15)
#تعریف_کرو TCG_OUTPUT_بفر_TOO_SHORT     (TPM_RET_بنیاد + 0x16)
#تعریف_کرو TCG_FATAL_COM_ERROR             (TPM_RET_بنیاد + 0x17)
#تعریف_کرو TCG_INVALID_داخلہ_PARA          (TPM_RET_بنیاد + 0x18)
#تعریف_کرو TCG_TCG_COMMAND_ERROR           (TPM_RET_بنیاد + 0x19)
#تعریف_کرو TCG_INTERFACE_SHUTDOWN          (TPM_RET_بنیاد + 0x20)
//define TCG_PC_UNSUPPORTED             (TPM_RET_BASE + 0x21)
#تعریف_کرو TCG_PC_TPM_NOT_PRESENT          (TPM_RET_بنیاد + 0x22)
#تعریف_کرو TCG_PC_TPM_DEACTIVATED          (TPM_RET_بنیاد + 0x23)

/* interrupt identifiers (al register) */
شمار irq_ids {
    TCG_حالت_Check = 0,
    TCG_HashLogExtendEvent = 1,
    TCG_Pass_Through_مقصد_TPM = 2,
    TCG_ShutdownPreBootInterface = 3,
    TCG_HashLogEvent = 4,
    TCG_HashAll = 5,
    TCG_TSS = 6,
    TCG_CompactHashLogExtendEvent = 7,
};

/* Input and Output blocks for the TCG BIOS commands */

ساخت hleei_short
{
    u16   ipblength;
    u16   reserved;
    ثابت خالی *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    ثابت خالی *logdataptr;
    u32   logdatalen;
} PACKED;

ساخت hleei_long
{
    u16   ipblength;
    u16   reserved;
    خالی *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    u32   reserved2;
    خالی *logdataptr;
    u32   logdatalen;
} PACKED;

ساخت hleeo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ساخت pttti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tpmopin[0];
} PACKED;

ساخت pttto
{
    u16    opblength;
    u16    reserved;
    u8     tpmopout[0];
};

ساخت hlei
{
    u16    ipblength;
    u16    reserved;
    ثابت خالی  *hashdataptr;
    u32    hashdatalen;
    u32    pcrindex;
    u32    logeventtype;
    ثابت خالی  *logdataptr;
    u32    logdatalen;
} PACKED;

ساخت hleo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
} PACKED;

ساخت hai
{
    u16    ipblength;
    u16    reserved;
    ثابت خالی  *hashdataptr;
    u32    hashdatalen;
    u32    algorithmid;
} PACKED;

ساخت ti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tssoperandin[0];
} PACKED;

ساخت مقصد
{
    u16    opblength;
    u16    reserved;
    u8     tssoperandout[0];
} PACKED;

ساخت pcpes
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

#تعریف_کرو TPM_ORD_Self_Test_بھرا             0x00000050
#تعریف_کرو TPM_ORD_ForceClear               0x0000005d
#تعریف_کرو TPM_ORD_GetCapability            0x00000065
#تعریف_کرو TPM_ORD_PhysicalEnable           0x0000006f
#تعریف_کرو TPM_ORD_PhysicalDisable          0x00000070
#تعریف_کرو TPM_ORD_سیٹ_Owner_Install          0x00000071
#تعریف_کرو TPM_ORD_Physical_سیٹ_Deactivated   0x00000072
#تعریف_کرو TPM_ORD_سیٹ_عارضی_Deactivated       0x00000073
#تعریف_کرو TPM_ORD_Startup                  0x00000099
#تعریف_کرو TPM_ORD_PhysicalPresence         0x4000000a
#تعریف_کرو TPM_ORD_Extend                   0x00000014
#تعریف_کرو TSC_ORD_Reset_Establishment_ثنائی_ہندسہ    0x4000000b

#تعریف_کرو TPM_ST_CLEAR                     0x0001
#تعریف_کرو TPM_ST_STATE                     0x0002
#تعریف_کرو TPM_ST_DEACTIVATED               0x0003

#تعریف_کرو TPM_PP_CMD_ENABLE                0x0020
#تعریف_کرو TPM_PP_PRESENT                   0x0008
#تعریف_کرو TPM_PP_NOT_PRESENT_تالا          0x0014

/* TPM command error codes */
#تعریف_کرو TPM_INVALID_POSTINIT             0x26
#تعریف_کرو TPM_BAD_LOCALITY                 0x3d

/* TPM command tags */
#تعریف_کرو TPM_لیبل_RQU_CMD                  0x00c1
#تعریف_کرو TPM_لیبل_RQU_AUTH1_CMD            0x00c2
#تعریف_کرو TPM_لیبل_RQU_AUTH2_CMD            0x00c3

ساخت tpm_req_سرخی {
    u16    لیبل;
    u32    totlen;
    u32    ordinal;
} PACKED;

ساخت tpm_rsp_سرخی {
    u16    لیبل;
    u32    totlen;
    u32    errcode;
} PACKED;

ساخت tpm_req_extend {
    ساخت tpm_req_سرخی hdr;
    u32    pcrindex;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ساخت tpm_rsp_extend {
    ساخت tpm_rsp_سرخی hdr;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ساخت tpm_req_getcap {
    ساخت tpm_req_سرخی hdr;
    u32    capArea;
    u32    sub_Cap_سائز;
    u32    subCap;
} PACKED;

#تعریف_کرو TPM_CAP_FLAG     0x04
#تعریف_کرو TPM_CAP_PROPERTY 0x05
#تعریف_کرو TPM_CAP_FLAG_PERMANENT   0x108
#تعریف_کرو TPM_CAP_FLAG_VOLATILE    0x109
#تعریف_کرو TPM_CAP_PROP_OWNER       0x111
#تعریف_کرو TPM_CAP_PROP_TIS_TIMEOUT 0x115
#تعریف_کرو TPM_CAP_PROP_DURATION    0x120

ساخت tpm_permanent_flags {
    u16    لیبل;
    u8     flags[20];
} PACKED;

شمار perm_Flags_اشاریہ {
    PERM_FLAG_IDX_DISABLE = 0,
    PERM_FLAG_IDX_OWNERSHIP,
    PERM_FLAG_IDX_DEACTIVATED,
    PERM_FLAG_IDX_READPUBEK,
    PERM_FLAG_IDX_DISABLEOWNERCLEAR,
    PERM_FLAG_IDX_ALLOW_MAINTENANCE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_تالا,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE,
};

ساخت tpm_res_getcap_perm_flags {
    ساخت tpm_rsp_سرخی hdr;
    u32    سائز;
    ساخت tpm_permanent_flags perm_flags;
} PACKED;

ساخت tpm_stclear_flags {
    u16    لیبل;
    u8     flags[5];
} PACKED;

#تعریف_کرو STCLEAR_FLAG_IDX_DEACTIVATED 0
#تعریف_کرو STCLEAR_FLAG_IDX_DISABLE_FORCE_CLEAR 1
#تعریف_کرو STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE 2
#تعریف_کرو STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE_تالا 3
#تعریف_کرو STCLEAR_FLAG_IDX_GLOBAL_تالا 4

ساخت tpm_res_getcap_stclear_flags {
    ساخت tpm_rsp_سرخی hdr;
    u32    سائز;
    ساخت tpm_stclear_flags stclear_flags;
} PACKED;

ساخت tpm_res_getcap_ownerauth {
    ساخت tpm_rsp_سرخی hdr;
    u32    سائز;
    u8     flag;
} PACKED;

ساخت tpm_res_getcap_timeouts {
    ساخت tpm_rsp_سرخی hdr;
    u32    سائز;
    u32    timeouts[4];
} PACKED;

ساخت tpm_res_getcap_durations {
    ساخت tpm_rsp_سرخی hdr;
    u32    سائز;
    u32    durations[3];
} PACKED;

ساخت tpm_res_sha1start {
    ساخت tpm_rsp_سرخی hdr;
    u32    max_عدد_بائٹیں;
} PACKED;

ساخت tpm_res_sha1complete {
    ساخت tpm_rsp_سرخی hdr;
    u8     hash[20];
} PACKED;


/****************************************************************
 * TPM v2.0 hardware commands
 ****************************************************************/

#تعریف_کرو TPM2_NO                     0
#تعریف_کرو TPM2_YES                    1

#تعریف_کرو TPM2_SU_CLEAR               0x0000
#تعریف_کرو TPM2_SU_STATE               0x0001

#تعریف_کرو TPM2_RH_OWNER               0x40000001
#تعریف_کرو TPM2_RS_PW                  0x40000009
#تعریف_کرو TPM2_RH_ENDORSEMENT         0x4000000b
#تعریف_کرو TPM2_RH_PLATFORM            0x4000000c

#تعریف_کرو TPM2_ALG_SHA1               0x0004
#تعریف_کرو TPM2_ALG_SHA256             0x000b
#تعریف_کرو TPM2_ALG_SHA384             0x000c
#تعریف_کرو TPM2_ALG_SHA512             0x000d
#تعریف_کرو TPM2_ALG_SM3_256            0x0012
#تعریف_کرو TPM2_ALG_SHA3_256           0x0027
#تعریف_کرو TPM2_ALG_SHA3_384           0x0028
#تعریف_کرو TPM2_ALG_SHA3_512           0x0029

#تعریف_کرو TPM2_ALG_SHA1_FLAG          (1 << 0)
#تعریف_کرو TPM2_ALG_SHA256_FLAG        (1 << 1)
#تعریف_کرو TPM2_ALG_SHA384_FLAG        (1 << 2)
#تعریف_کرو TPM2_ALG_SHA512_FLAG        (1 << 3)
#تعریف_کرو TPM2_ALG_SM3_256_FLAG       (1 << 4)
#تعریف_کرو TPM2_ALG_SHA3_256_FLAG      (1 << 5)
#تعریف_کرو TPM2_ALG_SHA3_384_FLAG      (1 << 6)
#تعریف_کرو TPM2_ALG_SHA3_512_FLAG      (1 << 7)

/* TPM 2 command tags */
#تعریف_کرو TPM2_ST_NO_SESSIONS         0x8001
#تعریف_کرو TPM2_ST_SESSIONS            0x8002

/* TPM 2 commands */
#تعریف_کرو TPM2_CC_HierarchyControl    0x121
#تعریف_کرو TPM2_CC_Clear               0x126
#تعریف_کرو TPM2_CC_ClearControl        0x127
#تعریف_کرو TPM2_CC_HierarchyChangeAuth 0x129
#تعریف_کرو TPM2_CC_PCR_Allocate        0x12b
#تعریف_کرو TPM2_CC_SelfTest            0x143
#تعریف_کرو TPM2_CC_Startup             0x144
#تعریف_کرو TPM2_CC_Shutdown            0x145
#تعریف_کرو TPM2_CC_StirRandom          0x146
#تعریف_کرو TPM2_CC_GetCapability       0x17a
#تعریف_کرو TPM2_CC_GetRandom           0x17b
#تعریف_کرو TPM2_CC_PCR_Extend          0x182

/* TPM 2 error codes */
#تعریف_کرو TPM2_RC_INITIALIZE          0x100

/* TPM 2 Capabilities */
#تعریف_کرو TPM2_CAP_PCRS               0x00000005

/* TPM 2 data structures */

ساخت tpm2_req_stirrandom {
    ساخت tpm_req_سرخی hdr;
    u16 سائز;
    u64 stir;
} PACKED;

ساخت tpm2_req_getrandom {
    ساخت tpm_req_سرخی hdr;
    u16 بائٹیں_Requested;
} PACKED;

ساخت tpm2b_20 {
    u16 سائز;
    u8 بفر[20];
} PACKED;

ساخت tpm2_res_getrandom {
    ساخت tpm_rsp_سرخی hdr;
    ساخت tpm2b_20 rnd;
} PACKED;

ساخت tpm2_authblock {
    u32 handle;
    u16 noncesize;  /* always 0 */
    u8 contsession; /* always TPM2_YES */
    u16 pwdsize;    /* always 0 */
} PACKED;

ساخت tpm2_req_hierarchychangeauth {
    ساخت tpm_req_سرخی hdr;
    u32 authhandle;
    u32 authblocksize;
    ساخت tpm2_authblock authblock;
    ساخت tpm2b_20 newAuth;
} PACKED;

ساخت tpm2_req_extend {
    ساخت tpm_req_سرخی hdr;
    u32 pcrindex;
    u32 authblocksize;
    ساخت tpm2_authblock authblock;
    u8 digest[0];
} PACKED;

ساخت tpm2_req_clearcontrol {
    ساخت tpm_req_سرخی hdr;
    u32 authhandle;
    u32 authblocksize;
    ساخت tpm2_authblock authblock;
    u8 disable;
} PACKED;

ساخت tpm2_req_clear {
    ساخت tpm_req_سرخی hdr;
    u32 authhandle;
    u32 authblocksize;
    ساخت tpm2_authblock authblock;
} PACKED;

ساخت tpm2_req_hierarchycontrol {
    ساخت tpm_req_سرخی hdr;
    u32 authhandle;
    u32 authblocksize;
    ساخت tpm2_authblock authblock;
    u32 enable;
    u8 state;
} PACKED;

ساخت tpm2_req_getcapability {
    ساخت tpm_req_سرخی hdr;
    u32 capability;
    u32 property;
    u32 propertycount;
} PACKED;

ساخت tpm2_res_getcapability {
    ساخت tpm_rsp_سرخی hdr;
    u8 مزید_ڈیٹا;
    u32 capability;
    u8 ڈیٹا[0]; /* capability dependent data */
} PACKED;

ساخت tpm2_req_pcr_allocate {
    ساخت tpm_req_سرخی hdr;
    u32 authhandle;
    u32 authblocksize;
    ساخت tpm2_authblock authblock;
    u32 گنتی;
    u8 tpms_pcr_selections[4];
} PACKED;

ساخت tpms_pcr_selection {
    u16 hashAlg;
    u8 سائز_کا_Select;
    u8 pcrSelect[0];
} PACKED;

ساخت tpml_pcr_selection {
    u32 گنتی;
    ساخت tpms_pcr_selection selections[0];
} PACKED;


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

/* event types: 10.4.1 / table 11 */
#تعریف_کرو EV_POST_رمز             1
#تعریف_کرو EV_NO_ACTION             3
#تعریف_کرو EV_SEPARATOR             4
#تعریف_کرو EV_ACTION                5
#تعریف_کرو EV_EVENT_لیبل             6
#تعریف_کرو EV_COMPACT_HASH         12
#تعریف_کرو EV_IPL                  13
#تعریف_کرو EV_IPL_تقسیم_ڈیٹا   14

ساخت tpm2_digest_قدر {
    u16 hashAlg;
    u8 hash[0]; /* size depends on hashAlg */
} PACKED;

ساخت tpm2_digest_اقدار {
    u32 گنتی;
    ساخت tpm2_digest_قدر digest[0];
} PACKED;

// Each entry in the TPM log contains: a tpm_log_header, a variable
// length digest, a tpm_log_trailer, and a variable length event.  The
// 'digest' matches what is sent to the TPM hardware via the Extend
// command.  On TPM1.2 the digest is a SHA1 hash; on TPM2.0 the digest
// contains a tpm2_digest_values struct followed by a variable number
// of tpm2_digest_value structs (as specified by the hardware via the
// TPM2_CAP_PCRS request).
ساخت tpm_log_سرخی {
    u32 pcrindex;
    u32 eventtype;
    u8 digest[0];
} PACKED;

ساخت tpm_log_trailer {
    u32 eventdatasize;
    u8 event[0];
} PACKED;

ساخت TCG_EfiSpecIdEventStruct {
    u8 signature[16];
    u32 platformClass;
    u8 spec_نسخہ_Minor;
    u8 spec_نسخہ_Major;
    u8 specErrata;
    u8 uintn_سائز;
    u32 نمبر_کا_Algorithms;
    ساخت TCG_Efi_Spec_Id_Event_Algorithm_سائز {
        u16 algorithmId;
        u16 digest_سائز;
    } digestSizes[];
    /*
    u8 vendorInfoSize;
    u8 vendorInfo[0];
    */
} PACKED;

#تعریف_کرو TPM_TCPA_ACPI_CLASS_CLIENT 0

ساخت pcctes
{
    u32 eventid;
    u32 eventdatasize;
    u8  digest[SHA1_BUFSIZE];
} PACKED;

ساخت pcctes_romex
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

#تعریف_کرو TPM_STATE_ENABLED 1
#تعریف_کرو TPM_STATE_فعال 2
#تعریف_کرو TPM_STATE_OWNED 4
#تعریف_کرو TPM_STATE_OWNERINSTALL 8

#تعریف_کرو TPM_PPI_OP_NOOP 0
#تعریف_کرو TPM_PPI_OP_ENABLE 1
#تعریف_کرو TPM_PPI_OP_DISABLE 2
#تعریف_کرو TPM_PPI_OP_ACTIVATE 3
#تعریف_کرو TPM_PPI_OP_DEACTIVATE 4
#تعریف_کرو TPM_PPI_OP_CLEAR 5
#تعریف_کرو TPM_PPI_OP_سیٹ_OWNERINSTALL_TRUE 8
#تعریف_کرو TPM_PPI_OP_سیٹ_OWNERINSTALL_FALSE 9

#اگر_ختم // tcg.h
