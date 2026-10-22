#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STD_TCG_H
#ವ್ಯಾಖ್ಯಾನಿಸು STD_TCG_H

#ಅನ್ತರ್ಭಾವ "types.h"

#ವ್ಯಾಖ್ಯಾನಿಸು SHA1_BUFSIZE                20
#ವ್ಯಾಖ್ಯಾನಿಸು SHA256_BUFSIZE              32
#ವ್ಯಾಖ್ಯಾನಿಸು SHA384_BUFSIZE              48
#ವ್ಯಾಖ್ಯಾನಿಸು SHA512_BUFSIZE              64
#ವ್ಯಾಖ್ಯಾನಿಸು SM3_256_BUFSIZE             32
#ವ್ಯಾಖ್ಯಾನಿಸು SHA3_256_BUFSIZE            32
#ವ್ಯಾಖ್ಯಾನಿಸು SHA3_384_BUFSIZE            48
#ವ್ಯಾಖ್ಯಾನಿಸು SHA3_512_BUFSIZE            64


/****************************************************************
 * 16bit BIOS interface
 ****************************************************************/

/* Define for section 12.3 */
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_PC_OK                       0x0
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_PC_TPMERROR                 0x1
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_PC_LOGOVERFLOW              0x2
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_PC_UNSUPPORTED              0x3

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ALG_SHA                     0x4

#ವ್ಯಾಖ್ಯಾನಿಸು TCG_MAGIC                       0x41504354L
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_ಆವೃತ್ತಿ_MAJOR               1
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_ಆವೃತ್ತಿ_MINOR               2

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_OK                          0x0
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_RET_ತಳ                    0x1
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_GENERAL_ERROR               (TPM_RET_ತಳ + 0x0)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_TPM_IS_LOCKED               (TPM_RET_ತಳ + 0x1)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_NO_RESPONSE                 (TPM_RET_ತಳ + 0x2)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INVALID_RESPONSE            (TPM_RET_ತಳ + 0x3)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INVALID_ACCESS_REQUEST      (TPM_RET_ತಳ + 0x4)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_FIRMWARE_ERROR              (TPM_RET_ತಳ + 0x5)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INTEGRITY_CHECK_FAILED      (TPM_RET_ತಳ + 0x6)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INVALID_DEVICE_ID           (TPM_RET_ತಳ + 0x7)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INVALID_VENDOR_ID           (TPM_RET_ತಳ + 0x8)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_UNABLE_ಗಮ್ಯ_OPEN              (TPM_RET_ತಳ + 0x9)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_UNABLE_ಗಮ್ಯ_CLOSE             (TPM_RET_ತಳ + 0xa)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_RESPONSE_TIMEOUT            (TPM_RET_ತಳ + 0xb)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INVALID_COM_REQUEST         (TPM_RET_ತಳ + 0xc)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INVALID_ADR_REQUEST         (TPM_RET_ತಳ + 0xd)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_WRITE_BYTE_ERROR            (TPM_RET_ತಳ + 0xe)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_READ_BYTE_ERROR             (TPM_RET_ತಳ + 0xf)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_ಖಂಡ_WRITE_TIMEOUT         (TPM_RET_ತಳ + 0x10)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_CHAR_WRITE_TIMEOUT          (TPM_RET_ತಳ + 0x11)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_CHAR_READ_TIMEOUT           (TPM_RET_ತಳ + 0x12)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_ಖಂಡ_READ_TIMEOUT          (TPM_RET_ತಳ + 0x13)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_TRANSFER_ABORT              (TPM_RET_ತಳ + 0x14)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INVALID_DRV_FUNCTION        (TPM_RET_ತಳ + 0x15)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_OUTPUT_ಬಫರ್_TOO_SHORT     (TPM_RET_ತಳ + 0x16)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_FATAL_COM_ERROR             (TPM_RET_ತಳ + 0x17)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INVALID_ಒಳಹರಿವು_PARA          (TPM_RET_ತಳ + 0x18)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_TCG_COMMAND_ERROR           (TPM_RET_ತಳ + 0x19)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_INTERFACE_SHUTDOWN          (TPM_RET_ತಳ + 0x20)
//define TCG_PC_UNSUPPORTED             (TPM_RET_BASE + 0x21)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_PC_TPM_NOT_PRESENT          (TPM_RET_ತಳ + 0x22)
#ವ್ಯಾಖ್ಯಾನಿಸು TCG_PC_TPM_DEACTIVATED          (TPM_RET_ತಳ + 0x23)

/* interrupt identifiers (al register) */
ಎಣಿಕೆ irq_ids {
    TCG_ಸ್ಥಿತಿ_Check = 0,
    TCG_HashLogExtendEvent = 1,
    TCG_Pass_Through_ಗಮ್ಯ_TPM = 2,
    TCG_ShutdownPreBootInterface = 3,
    TCG_HashLogEvent = 4,
    TCG_HashAll = 5,
    TCG_TSS = 6,
    TCG_CompactHashLogExtendEvent = 7,
};

/* Input and Output blocks for the TCG BIOS commands */

ರಚನೆ hleei_short
{
    u16   ipblength;
    u16   reserved;
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *logdataptr;
    u32   logdatalen;
} PACKED;

ರಚನೆ hleei_long
{
    u16   ipblength;
    u16   reserved;
    ಶೂನ್ಯ *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    u32   reserved2;
    ಶೂನ್ಯ *logdataptr;
    u32   logdatalen;
} PACKED;

ರಚನೆ hleeo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ರಚನೆ pttti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tpmopin[0];
} PACKED;

ರಚನೆ pttto
{
    u16    opblength;
    u16    reserved;
    u8     tpmopout[0];
};

ರಚನೆ hlei
{
    u16    ipblength;
    u16    reserved;
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ  *hashdataptr;
    u32    hashdatalen;
    u32    pcrindex;
    u32    logeventtype;
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ  *logdataptr;
    u32    logdatalen;
} PACKED;

ರಚನೆ hleo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
} PACKED;

ರಚನೆ hai
{
    u16    ipblength;
    u16    reserved;
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ  *hashdataptr;
    u32    hashdatalen;
    u32    algorithmid;
} PACKED;

ರಚನೆ ti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tssoperandin[0];
} PACKED;

ರಚನೆ ಗಮ್ಯ
{
    u16    opblength;
    u16    reserved;
    u8     tssoperandout[0];
} PACKED;

ರಚನೆ pcpes
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

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_Self_Test_ತುಂಬಿದ             0x00000050
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_ForceClear               0x0000005d
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_GetCapability            0x00000065
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_PhysicalEnable           0x0000006f
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_PhysicalDisable          0x00000070
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_ಗಣ_Owner_Install          0x00000071
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_Physical_ಗಣ_Deactivated   0x00000072
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_ಗಣ_ತಾತ್ಕಾಲಿಕ_Deactivated       0x00000073
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_Startup                  0x00000099
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_PhysicalPresence         0x4000000a
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ORD_Extend                   0x00000014
#ವ್ಯಾಖ್ಯಾನಿಸು TSC_ORD_Reset_Establishment_ಬಿಟ್    0x4000000b

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ST_CLEAR                     0x0001
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ST_STATE                     0x0002
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ST_DEACTIVATED               0x0003

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PP_CMD_ENABLE                0x0020
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PP_PRESENT                   0x0008
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PP_NOT_PRESENT_ಬೀಗ          0x0014

/* TPM command error codes */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_INVALID_POSTINIT             0x26
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_BAD_LOCALITY                 0x3d

/* TPM command tags */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ಟ್ಯಾಗ್_RQU_CMD                  0x00c1
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ಟ್ಯಾಗ್_RQU_AUTH1_CMD            0x00c2
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_ಟ್ಯಾಗ್_RQU_AUTH2_CMD            0x00c3

ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ {
    u16    ಟ್ಯಾಗ್;
    u32    totlen;
    u32    ordinal;
} PACKED;

ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ {
    u16    ಟ್ಯಾಗ್;
    u32    totlen;
    u32    errcode;
} PACKED;

ರಚನೆ tpm_req_extend {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32    pcrindex;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ರಚನೆ tpm_rsp_extend {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ರಚನೆ tpm_req_getcap {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32    capArea;
    u32    sub_Cap_ಗಾತ್ರ;
    u32    subCap;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_CAP_FLAG     0x04
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_CAP_PROPERTY 0x05
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_CAP_FLAG_PERMANENT   0x108
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_CAP_FLAG_VOLATILE    0x109
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_CAP_PROP_OWNER       0x111
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_CAP_PROP_TIS_TIMEOUT 0x115
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_CAP_PROP_DURATION    0x120

ರಚನೆ tpm_permanent_flags {
    u16    ಟ್ಯಾಗ್;
    u8     flags[20];
} PACKED;

ಎಣಿಕೆ perm_Flags_ಸೂಚ್ಯಂಕ {
    PERM_FLAG_IDX_DISABLE = 0,
    PERM_FLAG_IDX_OWNERSHIP,
    PERM_FLAG_IDX_DEACTIVATED,
    PERM_FLAG_IDX_READPUBEK,
    PERM_FLAG_IDX_DISABLEOWNERCLEAR,
    PERM_FLAG_IDX_ALLOW_MAINTENANCE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_ಬೀಗ,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE,
};

ರಚನೆ tpm_res_getcap_perm_flags {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u32    ಗಾತ್ರ_2;
    ರಚನೆ tpm_permanent_flags perm_flags;
} PACKED;

ರಚನೆ tpm_stclear_flags {
    u16    ಟ್ಯಾಗ್;
    u8     flags[5];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು STCLEAR_FLAG_IDX_DEACTIVATED 0
#ವ್ಯಾಖ್ಯಾನಿಸು STCLEAR_FLAG_IDX_DISABLE_FORCE_CLEAR 1
#ವ್ಯಾಖ್ಯಾನಿಸು STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE 2
#ವ್ಯಾಖ್ಯಾನಿಸು STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE_ಬೀಗ 3
#ವ್ಯಾಖ್ಯಾನಿಸು STCLEAR_FLAG_IDX_GLOBAL_ಬೀಗ 4

ರಚನೆ tpm_res_getcap_stclear_flags {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u32    ಗಾತ್ರ_2;
    ರಚನೆ tpm_stclear_flags stclear_flags;
} PACKED;

ರಚನೆ tpm_res_getcap_ownerauth {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u32    ಗಾತ್ರ_2;
    u8     flag;
} PACKED;

ರಚನೆ tpm_res_getcap_timeouts {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u32    ಗಾತ್ರ_2;
    u32    timeouts[4];
} PACKED;

ರಚನೆ tpm_res_getcap_durations {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u32    ಗಾತ್ರ_2;
    u32    durations[3];
} PACKED;

ರಚನೆ tpm_res_sha1start {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u32    max_ಅಂಕೆ_ಬೈಟುಗಳು;
} PACKED;

ರಚನೆ tpm_res_sha1complete {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u8     hash[20];
} PACKED;


/****************************************************************
 * TPM v2.0 hardware commands
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_NO                     0
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_YES                    1

#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_SU_CLEAR               0x0000
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_SU_STATE               0x0001

#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_RH_OWNER               0x40000001
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_RS_PW                  0x40000009
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_RH_ENDORSEMENT         0x4000000b
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_RH_PLATFORM            0x4000000c

#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA1               0x0004
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA256             0x000b
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA384             0x000c
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA512             0x000d
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SM3_256            0x0012
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA3_256           0x0027
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA3_384           0x0028
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA3_512           0x0029

#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA1_FLAG          (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA256_FLAG        (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA384_FLAG        (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA512_FLAG        (1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SM3_256_FLAG       (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA3_256_FLAG      (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA3_384_FLAG      (1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ALG_SHA3_512_FLAG      (1 << 7)

/* TPM 2 command tags */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ST_NO_SESSIONS         0x8001
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_ST_SESSIONS            0x8002

/* TPM 2 commands */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_HierarchyControl    0x121
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_Clear               0x126
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_ClearControl        0x127
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_HierarchyChangeAuth 0x129
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_PCR_Allocate        0x12b
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_SelfTest            0x143
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_Startup             0x144
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_Shutdown            0x145
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_StirRandom          0x146
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_GetCapability       0x17a
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_GetRandom           0x17b
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CC_PCR_Extend          0x182

/* TPM 2 error codes */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_RC_INITIALIZE          0x100

/* TPM 2 Capabilities */
#ವ್ಯಾಖ್ಯಾನಿಸು TPM2_CAP_PCRS               0x00000005

/* TPM 2 data structures */

ರಚನೆ tpm2_req_stirrandom {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u16 ಗಾತ್ರ_2;
    u64 stir;
} PACKED;

ರಚನೆ tpm2_req_getrandom {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u16 ಬೈಟುಗಳು_Requested;
} PACKED;

ರಚನೆ tpm2b_20 {
    u16 ಗಾತ್ರ_2;
    u8 ಬಫರ್[20];
} PACKED;

ರಚನೆ tpm2_res_getrandom {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    ರಚನೆ tpm2b_20 rnd;
} PACKED;

ರಚನೆ tpm2_authblock {
    u32 handle;
    u16 noncesize;  /* always 0 */
    u8 contsession; /* always TPM2_YES */
    u16 pwdsize;    /* always 0 */
} PACKED;

ರಚನೆ tpm2_req_hierarchychangeauth {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32 authhandle;
    u32 authblocksize;
    ರಚನೆ tpm2_authblock authblock;
    ರಚನೆ tpm2b_20 newAuth;
} PACKED;

ರಚನೆ tpm2_req_extend {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32 pcrindex;
    u32 authblocksize;
    ರಚನೆ tpm2_authblock authblock;
    u8 digest[0];
} PACKED;

ರಚನೆ tpm2_req_clearcontrol {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32 authhandle;
    u32 authblocksize;
    ರಚನೆ tpm2_authblock authblock;
    u8 disable;
} PACKED;

ರಚನೆ tpm2_req_clear {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32 authhandle;
    u32 authblocksize;
    ರಚನೆ tpm2_authblock authblock;
} PACKED;

ರಚನೆ tpm2_req_hierarchycontrol {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32 authhandle;
    u32 authblocksize;
    ರಚನೆ tpm2_authblock authblock;
    u32 enable;
    u8 state;
} PACKED;

ರಚನೆ tpm2_req_getcapability {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32 capability;
    u32 property;
    u32 propertycount;
} PACKED;

ರಚನೆ tpm2_res_getcapability {
    ರಚನೆ tpm_rsp_ಶೀರ್ಷಿಕೆ hdr;
    u8 ಇನ್ನಷ್ಟು_ದತ್ತಾಂಶ;
    u32 capability;
    u8 ದತ್ತಾಂಶ[0]; /* capability dependent data */
} PACKED;

ರಚನೆ tpm2_req_pcr_allocate {
    ರಚನೆ tpm_req_ಶೀರ್ಷಿಕೆ hdr;
    u32 authhandle;
    u32 authblocksize;
    ರಚನೆ tpm2_authblock authblock;
    u32 ಎಣಿಕೆ_2;
    u8 tpms_pcr_selections[4];
} PACKED;

ರಚನೆ tpms_pcr_selection {
    u16 hashAlg;
    u8 ಗಾತ್ರ_ದ_Select;
    u8 pcrSelect[0];
} PACKED;

ರಚನೆ tpml_pcr_selection {
    u32 ಎಣಿಕೆ_2;
    ರಚನೆ tpms_pcr_selection selections[0];
} PACKED;


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

/* event types: 10.4.1 / table 11 */
#ವ್ಯಾಖ್ಯಾನಿಸು EV_POST_ಸಂಕೇತ             1
#ವ್ಯಾಖ್ಯಾನಿಸು EV_NO_ACTION             3
#ವ್ಯಾಖ್ಯಾನಿಸು EV_SEPARATOR             4
#ವ್ಯಾಖ್ಯಾನಿಸು EV_ACTION                5
#ವ್ಯಾಖ್ಯಾನಿಸು EV_EVENT_ಟ್ಯಾಗ್             6
#ವ್ಯಾಖ್ಯಾನಿಸು EV_COMPACT_HASH         12
#ವ್ಯಾಖ್ಯಾನಿಸು EV_IPL                  13
#ವ್ಯಾಖ್ಯಾನಿಸು EV_IPL_ವಿಭಜನೆ_ದತ್ತಾಂಶ   14

ರಚನೆ tpm2_digest_ಮೌಲ್ಯ {
    u16 hashAlg;
    u8 hash[0]; /* size depends on hashAlg */
} PACKED;

ರಚನೆ tpm2_digest_ಮೌಲ್ಯಗಳು {
    u32 ಎಣಿಕೆ_2;
    ರಚನೆ tpm2_digest_ಮೌಲ್ಯ digest[0];
} PACKED;

// Each entry in the TPM log contains: a tpm_log_header, a variable
// length digest, a tpm_log_trailer, and a variable length event.  The
// 'digest' matches what is sent to the TPM hardware via the Extend
// command.  On TPM1.2 the digest is a SHA1 hash; on TPM2.0 the digest
// contains a tpm2_digest_values struct followed by a variable number
// of tpm2_digest_value structs (as specified by the hardware via the
// TPM2_CAP_PCRS request).
ರಚನೆ tpm_log_ಶೀರ್ಷಿಕೆ {
    u32 pcrindex;
    u32 eventtype;
    u8 digest[0];
} PACKED;

ರಚನೆ tpm_log_trailer {
    u32 eventdatasize;
    u8 event[0];
} PACKED;

ರಚನೆ TCG_EfiSpecIdEventStruct {
    u8 signature[16];
    u32 platformClass;
    u8 spec_ಆವೃತ್ತಿ_Minor;
    u8 spec_ಆವೃತ್ತಿ_Major;
    u8 specErrata;
    u8 uintn_ಗಾತ್ರ;
    u32 ಸಂಖ್ಯೆ_ದ_Algorithms;
    ರಚನೆ TCG_Efi_Spec_Id_Event_Algorithm_ಗಾತ್ರ {
        u16 algorithmId;
        u16 digest_ಗಾತ್ರ;
    } digestSizes[];
    /*
    u8 vendorInfoSize;
    u8 vendorInfo[0];
    */
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_TCPA_ACPI_CLASS_CLIENT 0

ರಚನೆ pcctes
{
    u32 eventid;
    u32 eventdatasize;
    u8  digest[SHA1_BUFSIZE];
} PACKED;

ರಚನೆ pcctes_romex
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

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_STATE_ENABLED 1
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_STATE_ಸಕ್ರಿಯ 2
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_STATE_OWNED 4
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_STATE_OWNERINSTALL 8

#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PPI_OP_NOOP 0
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PPI_OP_ENABLE 1
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PPI_OP_DISABLE 2
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PPI_OP_ACTIVATE 3
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PPI_OP_DEACTIVATE 4
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PPI_OP_CLEAR 5
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PPI_OP_ಗಣ_OWNERINSTALL_TRUE 8
#ವ್ಯಾಖ್ಯಾನಿಸು TPM_PPI_OP_ಗಣ_OWNERINSTALL_FALSE 9

#ಆದರೆ_ಅಂತ್ಯ // tcg.h
