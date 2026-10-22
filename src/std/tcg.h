#اگر_تعریف_نشده STD_TCG_H
#تعریف_کن STD_TCG_H

#ضمن "types.h"

#تعریف_کن SHA1_BUFSIZE                20
#تعریف_کن SHA256_BUFSIZE              32
#تعریف_کن SHA384_BUFSIZE              48
#تعریف_کن SHA512_BUFSIZE              64
#تعریف_کن SM3_256_BUFSIZE             32
#تعریف_کن SHA3_256_BUFSIZE            32
#تعریف_کن SHA3_384_BUFSIZE            48
#تعریف_کن SHA3_512_BUFSIZE            64


/****************************************************************
 * 16bit BIOS interface
 ****************************************************************/

/* Define for section 12.3 */
#تعریف_کن TCG_PC_OK                       0x0
#تعریف_کن TCG_PC_TPMERROR                 0x1
#تعریف_کن TCG_PC_LOGOVERFLOW              0x2
#تعریف_کن TCG_PC_UNSUPPORTED              0x3

#تعریف_کن TPM_ALG_SHA                     0x4

#تعریف_کن TCG_MAGIC                       0x41504354L
#تعریف_کن TCG_نسخه_MAJOR               1
#تعریف_کن TCG_نسخه_MINOR               2

#تعریف_کن TPM_OK                          0x0
#تعریف_کن TPM_RET_پایه                    0x1
#تعریف_کن TCG_GENERAL_ERROR               (TPM_RET_پایه + 0x0)
#تعریف_کن TCG_TPM_IS_LOCKED               (TPM_RET_پایه + 0x1)
#تعریف_کن TCG_NO_RESPONSE                 (TPM_RET_پایه + 0x2)
#تعریف_کن TCG_INVALID_RESPONSE            (TPM_RET_پایه + 0x3)
#تعریف_کن TCG_INVALID_ACCESS_REQUEST      (TPM_RET_پایه + 0x4)
#تعریف_کن TCG_FIRMWARE_ERROR              (TPM_RET_پایه + 0x5)
#تعریف_کن TCG_INTEGRITY_CHECK_FAILED      (TPM_RET_پایه + 0x6)
#تعریف_کن TCG_INVALID_DEVICE_ID           (TPM_RET_پایه + 0x7)
#تعریف_کن TCG_INVALID_VENDOR_ID           (TPM_RET_پایه + 0x8)
#تعریف_کن TCG_UNABLE_مقصد_OPEN              (TPM_RET_پایه + 0x9)
#تعریف_کن TCG_UNABLE_مقصد_CLOSE             (TPM_RET_پایه + 0xa)
#تعریف_کن TCG_RESPONSE_TIMEOUT            (TPM_RET_پایه + 0xb)
#تعریف_کن TCG_INVALID_COM_REQUEST         (TPM_RET_پایه + 0xc)
#تعریف_کن TCG_INVALID_ADR_REQUEST         (TPM_RET_پایه + 0xd)
#تعریف_کن TCG_WRITE_BYTE_ERROR            (TPM_RET_پایه + 0xe)
#تعریف_کن TCG_READ_BYTE_ERROR             (TPM_RET_پایه + 0xf)
#تعریف_کن TCG_بلوک_WRITE_TIMEOUT         (TPM_RET_پایه + 0x10)
#تعریف_کن TCG_CHAR_WRITE_TIMEOUT          (TPM_RET_پایه + 0x11)
#تعریف_کن TCG_CHAR_READ_TIMEOUT           (TPM_RET_پایه + 0x12)
#تعریف_کن TCG_بلوک_READ_TIMEOUT          (TPM_RET_پایه + 0x13)
#تعریف_کن TCG_TRANSFER_ABORT              (TPM_RET_پایه + 0x14)
#تعریف_کن TCG_INVALID_DRV_FUNCTION        (TPM_RET_پایه + 0x15)
#تعریف_کن TCG_OUTPUT_میانگیر_TOO_SHORT     (TPM_RET_پایه + 0x16)
#تعریف_کن TCG_FATAL_COM_ERROR             (TPM_RET_پایه + 0x17)
#تعریف_کن TCG_INVALID_درون_داد_PARA          (TPM_RET_پایه + 0x18)
#تعریف_کن TCG_TCG_COMMAND_ERROR           (TPM_RET_پایه + 0x19)
#تعریف_کن TCG_INTERFACE_SHUTDOWN          (TPM_RET_پایه + 0x20)
//define TCG_PC_UNSUPPORTED             (TPM_RET_BASE + 0x21)
#تعریف_کن TCG_PC_TPM_NOT_PRESENT          (TPM_RET_پایه + 0x22)
#تعریف_کن TCG_PC_TPM_DEACTIVATED          (TPM_RET_پایه + 0x23)

/* interrupt identifiers (al register) */
تعداد irq_ids {
    TCG_وضعیت_Check = 0,
    TCG_HashLogExtendEvent = 1,
    TCG_Pass_Through_مقصد_TPM = 2,
    TCG_ShutdownPreBootInterface = 3,
    TCG_HashLogEvent = 4,
    TCG_HashAll = 5,
    TCG_TSS = 6,
    TCG_CompactHashLogExtendEvent = 7,
};

/* Input and Output blocks for the TCG BIOS commands */

بنية hleei_short
{
    u16   ipblength;
    u16   reserved;
    ثابت فراغ *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    ثابت فراغ *logdataptr;
    u32   logdatalen;
} PACKED;

بنية hleei_long
{
    u16   ipblength;
    u16   reserved;
    فراغ *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    u32   reserved2;
    فراغ *logdataptr;
    u32   logdatalen;
} PACKED;

بنية hleeo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

بنية pttti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tpmopin[0];
} PACKED;

بنية pttto
{
    u16    opblength;
    u16    reserved;
    u8     tpmopout[0];
};

بنية hlei
{
    u16    ipblength;
    u16    reserved;
    ثابت فراغ  *hashdataptr;
    u32    hashdatalen;
    u32    pcrindex;
    u32    logeventtype;
    ثابت فراغ  *logdataptr;
    u32    logdatalen;
} PACKED;

بنية hleo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
} PACKED;

بنية hai
{
    u16    ipblength;
    u16    reserved;
    ثابت فراغ  *hashdataptr;
    u32    hashdatalen;
    u32    algorithmid;
} PACKED;

بنية ti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tssoperandin[0];
} PACKED;

بنية مقصد
{
    u16    opblength;
    u16    reserved;
    u8     tssoperandout[0];
} PACKED;

بنية pcpes
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

#تعریف_کن TPM_ORD_Self_Test_پر             0x00000050
#تعریف_کن TPM_ORD_ForceClear               0x0000005d
#تعریف_کن TPM_ORD_GetCapability            0x00000065
#تعریف_کن TPM_ORD_PhysicalEnable           0x0000006f
#تعریف_کن TPM_ORD_PhysicalDisable          0x00000070
#تعریف_کن TPM_ORD_مجموعه_Owner_Install          0x00000071
#تعریف_کن TPM_ORD_Physical_مجموعه_Deactivated   0x00000072
#تعریف_کن TPM_ORD_مجموعه_موقت_Deactivated       0x00000073
#تعریف_کن TPM_ORD_Startup                  0x00000099
#تعریف_کن TPM_ORD_PhysicalPresence         0x4000000a
#تعریف_کن TPM_ORD_Extend                   0x00000014
#تعریف_کن TSC_ORD_Reset_Establishment_بیت    0x4000000b

#تعریف_کن TPM_ST_CLEAR                     0x0001
#تعریف_کن TPM_ST_STATE                     0x0002
#تعریف_کن TPM_ST_DEACTIVATED               0x0003

#تعریف_کن TPM_PP_CMD_ENABLE                0x0020
#تعریف_کن TPM_PP_PRESENT                   0x0008
#تعریف_کن TPM_PP_NOT_PRESENT_قفل          0x0014

/* TPM command error codes */
#تعریف_کن TPM_INVALID_POSTINIT             0x26
#تعریف_کن TPM_BAD_LOCALITY                 0x3d

/* TPM command tags */
#تعریف_کن TPM_برچسب_RQU_CMD                  0x00c1
#تعریف_کن TPM_برچسب_RQU_AUTH1_CMD            0x00c2
#تعریف_کن TPM_برچسب_RQU_AUTH2_CMD            0x00c3

بنية tpm_req_سرآیند {
    u16    برچسب;
    u32    totlen;
    u32    ordinal;
} PACKED;

بنية tpm_rsp_سرآیند {
    u16    برچسب;
    u32    totlen;
    u32    errcode;
} PACKED;

بنية tpm_req_extend {
    بنية tpm_req_سرآیند hdr;
    u32    pcrindex;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

بنية tpm_rsp_extend {
    بنية tpm_rsp_سرآیند hdr;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

بنية tpm_req_getcap {
    بنية tpm_req_سرآیند hdr;
    u32    capArea;
    u32    sub_Cap_اندازه;
    u32    subCap;
} PACKED;

#تعریف_کن TPM_CAP_FLAG     0x04
#تعریف_کن TPM_CAP_PROPERTY 0x05
#تعریف_کن TPM_CAP_FLAG_PERMANENT   0x108
#تعریف_کن TPM_CAP_FLAG_VOLATILE    0x109
#تعریف_کن TPM_CAP_PROP_OWNER       0x111
#تعریف_کن TPM_CAP_PROP_TIS_TIMEOUT 0x115
#تعریف_کن TPM_CAP_PROP_DURATION    0x120

بنية tpm_permanent_flags {
    u16    برچسب;
    u8     flags[20];
} PACKED;

تعداد perm_Flags_شاخص {
    PERM_FLAG_IDX_DISABLE = 0,
    PERM_FLAG_IDX_OWNERSHIP,
    PERM_FLAG_IDX_DEACTIVATED,
    PERM_FLAG_IDX_READPUBEK,
    PERM_FLAG_IDX_DISABLEOWNERCLEAR,
    PERM_FLAG_IDX_ALLOW_MAINTENANCE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_قفل,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE,
};

بنية tpm_res_getcap_perm_flags {
    بنية tpm_rsp_سرآیند hdr;
    u32    اندازه;
    بنية tpm_permanent_flags perm_flags;
} PACKED;

بنية tpm_stclear_flags {
    u16    برچسب;
    u8     flags[5];
} PACKED;

#تعریف_کن STCLEAR_FLAG_IDX_DEACTIVATED 0
#تعریف_کن STCLEAR_FLAG_IDX_DISABLE_FORCE_CLEAR 1
#تعریف_کن STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE 2
#تعریف_کن STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE_قفل 3
#تعریف_کن STCLEAR_FLAG_IDX_GLOBAL_قفل 4

بنية tpm_res_getcap_stclear_flags {
    بنية tpm_rsp_سرآیند hdr;
    u32    اندازه;
    بنية tpm_stclear_flags stclear_flags;
} PACKED;

بنية tpm_res_getcap_ownerauth {
    بنية tpm_rsp_سرآیند hdr;
    u32    اندازه;
    u8     flag;
} PACKED;

بنية tpm_res_getcap_timeouts {
    بنية tpm_rsp_سرآیند hdr;
    u32    اندازه;
    u32    timeouts[4];
} PACKED;

بنية tpm_res_getcap_durations {
    بنية tpm_rsp_سرآیند hdr;
    u32    اندازه;
    u32    durations[3];
} PACKED;

بنية tpm_res_sha1start {
    بنية tpm_rsp_سرآیند hdr;
    u32    max_رقم_بایتها;
} PACKED;

بنية tpm_res_sha1complete {
    بنية tpm_rsp_سرآیند hdr;
    u8     hash[20];
} PACKED;


/****************************************************************
 * TPM v2.0 hardware commands
 ****************************************************************/

#تعریف_کن TPM2_NO                     0
#تعریف_کن TPM2_YES                    1

#تعریف_کن TPM2_SU_CLEAR               0x0000
#تعریف_کن TPM2_SU_STATE               0x0001

#تعریف_کن TPM2_RH_OWNER               0x40000001
#تعریف_کن TPM2_RS_PW                  0x40000009
#تعریف_کن TPM2_RH_ENDORSEMENT         0x4000000b
#تعریف_کن TPM2_RH_PLATFORM            0x4000000c

#تعریف_کن TPM2_ALG_SHA1               0x0004
#تعریف_کن TPM2_ALG_SHA256             0x000b
#تعریف_کن TPM2_ALG_SHA384             0x000c
#تعریف_کن TPM2_ALG_SHA512             0x000d
#تعریف_کن TPM2_ALG_SM3_256            0x0012
#تعریف_کن TPM2_ALG_SHA3_256           0x0027
#تعریف_کن TPM2_ALG_SHA3_384           0x0028
#تعریف_کن TPM2_ALG_SHA3_512           0x0029

#تعریف_کن TPM2_ALG_SHA1_FLAG          (1 << 0)
#تعریف_کن TPM2_ALG_SHA256_FLAG        (1 << 1)
#تعریف_کن TPM2_ALG_SHA384_FLAG        (1 << 2)
#تعریف_کن TPM2_ALG_SHA512_FLAG        (1 << 3)
#تعریف_کن TPM2_ALG_SM3_256_FLAG       (1 << 4)
#تعریف_کن TPM2_ALG_SHA3_256_FLAG      (1 << 5)
#تعریف_کن TPM2_ALG_SHA3_384_FLAG      (1 << 6)
#تعریف_کن TPM2_ALG_SHA3_512_FLAG      (1 << 7)

/* TPM 2 command tags */
#تعریف_کن TPM2_ST_NO_SESSIONS         0x8001
#تعریف_کن TPM2_ST_SESSIONS            0x8002

/* TPM 2 commands */
#تعریف_کن TPM2_CC_HierarchyControl    0x121
#تعریف_کن TPM2_CC_Clear               0x126
#تعریف_کن TPM2_CC_ClearControl        0x127
#تعریف_کن TPM2_CC_HierarchyChangeAuth 0x129
#تعریف_کن TPM2_CC_PCR_Allocate        0x12b
#تعریف_کن TPM2_CC_SelfTest            0x143
#تعریف_کن TPM2_CC_Startup             0x144
#تعریف_کن TPM2_CC_Shutdown            0x145
#تعریف_کن TPM2_CC_StirRandom          0x146
#تعریف_کن TPM2_CC_GetCapability       0x17a
#تعریف_کن TPM2_CC_GetRandom           0x17b
#تعریف_کن TPM2_CC_PCR_Extend          0x182

/* TPM 2 error codes */
#تعریف_کن TPM2_RC_INITIALIZE          0x100

/* TPM 2 Capabilities */
#تعریف_کن TPM2_CAP_PCRS               0x00000005

/* TPM 2 data structures */

بنية tpm2_req_stirrandom {
    بنية tpm_req_سرآیند hdr;
    u16 اندازه;
    u64 stir;
} PACKED;

بنية tpm2_req_getrandom {
    بنية tpm_req_سرآیند hdr;
    u16 بایتها_Requested;
} PACKED;

بنية tpm2b_20 {
    u16 اندازه;
    u8 میانگیر[20];
} PACKED;

بنية tpm2_res_getrandom {
    بنية tpm_rsp_سرآیند hdr;
    بنية tpm2b_20 rnd;
} PACKED;

بنية tpm2_authblock {
    u32 handle;
    u16 noncesize;  /* always 0 */
    u8 contsession; /* always TPM2_YES */
    u16 pwdsize;    /* always 0 */
} PACKED;

بنية tpm2_req_hierarchychangeauth {
    بنية tpm_req_سرآیند hdr;
    u32 authhandle;
    u32 authblocksize;
    بنية tpm2_authblock authblock;
    بنية tpm2b_20 newAuth;
} PACKED;

بنية tpm2_req_extend {
    بنية tpm_req_سرآیند hdr;
    u32 pcrindex;
    u32 authblocksize;
    بنية tpm2_authblock authblock;
    u8 digest[0];
} PACKED;

بنية tpm2_req_clearcontrol {
    بنية tpm_req_سرآیند hdr;
    u32 authhandle;
    u32 authblocksize;
    بنية tpm2_authblock authblock;
    u8 disable;
} PACKED;

بنية tpm2_req_clear {
    بنية tpm_req_سرآیند hdr;
    u32 authhandle;
    u32 authblocksize;
    بنية tpm2_authblock authblock;
} PACKED;

بنية tpm2_req_hierarchycontrol {
    بنية tpm_req_سرآیند hdr;
    u32 authhandle;
    u32 authblocksize;
    بنية tpm2_authblock authblock;
    u32 enable;
    u8 state;
} PACKED;

بنية tpm2_req_getcapability {
    بنية tpm_req_سرآیند hdr;
    u32 capability;
    u32 property;
    u32 propertycount;
} PACKED;

بنية tpm2_res_getcapability {
    بنية tpm_rsp_سرآیند hdr;
    u8 بیشتر_داده;
    u32 capability;
    u8 داده[0]; /* capability dependent data */
} PACKED;

بنية tpm2_req_pcr_allocate {
    بنية tpm_req_سرآیند hdr;
    u32 authhandle;
    u32 authblocksize;
    بنية tpm2_authblock authblock;
    u32 شمار;
    u8 tpms_pcr_selections[4];
} PACKED;

بنية tpms_pcr_selection {
    u16 hashAlg;
    u8 اندازه_از_Select;
    u8 pcrSelect[0];
} PACKED;

بنية tpml_pcr_selection {
    u32 شمار;
    بنية tpms_pcr_selection selections[0];
} PACKED;


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

/* event types: 10.4.1 / table 11 */
#تعریف_کن EV_POST_کد             1
#تعریف_کن EV_NO_ACTION             3
#تعریف_کن EV_SEPARATOR             4
#تعریف_کن EV_ACTION                5
#تعریف_کن EV_EVENT_برچسب             6
#تعریف_کن EV_COMPACT_HASH         12
#تعریف_کن EV_IPL                  13
#تعریف_کن EV_IPL_افراز_داده   14

بنية tpm2_digest_مقدار {
    u16 hashAlg;
    u8 hash[0]; /* size depends on hashAlg */
} PACKED;

بنية tpm2_digest_مقادیر {
    u32 شمار;
    بنية tpm2_digest_مقدار digest[0];
} PACKED;

// Each entry in the TPM log contains: a tpm_log_header, a variable
// length digest, a tpm_log_trailer, and a variable length event.  The
// 'digest' matches what is sent to the TPM hardware via the Extend
// command.  On TPM1.2 the digest is a SHA1 hash; on TPM2.0 the digest
// contains a tpm2_digest_values struct followed by a variable number
// of tpm2_digest_value structs (as specified by the hardware via the
// TPM2_CAP_PCRS request).
بنية tpm_log_سرآیند {
    u32 pcrindex;
    u32 eventtype;
    u8 digest[0];
} PACKED;

بنية tpm_log_trailer {
    u32 eventdatasize;
    u8 event[0];
} PACKED;

بنية TCG_EfiSpecIdEventStruct {
    u8 signature[16];
    u32 platformClass;
    u8 spec_نسخه_Minor;
    u8 spec_نسخه_Major;
    u8 specErrata;
    u8 uintn_اندازه;
    u32 عدد_از_Algorithms;
    بنية TCG_Efi_Spec_Id_Event_Algorithm_اندازه {
        u16 algorithmId;
        u16 digest_اندازه;
    } digestSizes[];
    /*
    u8 vendorInfoSize;
    u8 vendorInfo[0];
    */
} PACKED;

#تعریف_کن TPM_TCPA_ACPI_CLASS_CLIENT 0

بنية pcctes
{
    u32 eventid;
    u32 eventdatasize;
    u8  digest[SHA1_BUFSIZE];
} PACKED;

بنية pcctes_romex
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

#تعریف_کن TPM_STATE_ENABLED 1
#تعریف_کن TPM_STATE_فعال 2
#تعریف_کن TPM_STATE_OWNED 4
#تعریف_کن TPM_STATE_OWNERINSTALL 8

#تعریف_کن TPM_PPI_OP_NOOP 0
#تعریف_کن TPM_PPI_OP_ENABLE 1
#تعریف_کن TPM_PPI_OP_DISABLE 2
#تعریف_کن TPM_PPI_OP_ACTIVATE 3
#تعریف_کن TPM_PPI_OP_DEACTIVATE 4
#تعریف_کن TPM_PPI_OP_CLEAR 5
#تعریف_کن TPM_PPI_OP_مجموعه_OWNERINSTALL_TRUE 8
#تعریف_کن TPM_PPI_OP_مجموعه_OWNERINSTALL_FALSE 9

#پایان_اگر // tcg.h
