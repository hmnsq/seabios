#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STD_TCG_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STD_TCG_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA1_BUFSIZE                20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA256_BUFSIZE              32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA384_BUFSIZE              48
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA512_BUFSIZE              64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SM3_256_BUFSIZE             32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA3_256_BUFSIZE            32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA3_384_BUFSIZE            48
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHA3_512_BUFSIZE            64


/****************************************************************
 * 16bit BIOS interface
 ****************************************************************/

/* Define for section 12.3 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_PC_OK                       0x0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_PC_TPMERROR                 0x1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_PC_LOGOVERFLOW              0x2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_PC_UNSUPPORTED              0x3

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ALG_SHA                     0x4

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_MAGIC                       0x41504354L
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_ਸੰਸਕਰਣ_MAJOR               1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_ਸੰਸਕਰਣ_MINOR               2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_OK                          0x0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_RET_ਅਧਾਰ                    0x1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_GENERAL_ERROR               (TPM_RET_ਅਧਾਰ + 0x0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_TPM_IS_LOCKED               (TPM_RET_ਅਧਾਰ + 0x1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_NO_RESPONSE                 (TPM_RET_ਅਧਾਰ + 0x2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INVALID_RESPONSE            (TPM_RET_ਅਧਾਰ + 0x3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INVALID_ACCESS_REQUEST      (TPM_RET_ਅਧਾਰ + 0x4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_FIRMWARE_ERROR              (TPM_RET_ਅਧਾਰ + 0x5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INTEGRITY_CHECK_FAILED      (TPM_RET_ਅਧਾਰ + 0x6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INVALID_DEVICE_ID           (TPM_RET_ਅਧਾਰ + 0x7)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INVALID_VENDOR_ID           (TPM_RET_ਅਧਾਰ + 0x8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_UNABLE_ਮੰਜ਼ਿਲ_OPEN              (TPM_RET_ਅਧਾਰ + 0x9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_UNABLE_ਮੰਜ਼ਿਲ_CLOSE             (TPM_RET_ਅਧਾਰ + 0xa)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_RESPONSE_TIMEOUT            (TPM_RET_ਅਧਾਰ + 0xb)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INVALID_COM_REQUEST         (TPM_RET_ਅਧਾਰ + 0xc)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INVALID_ADR_REQUEST         (TPM_RET_ਅਧਾਰ + 0xd)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_WRITE_BYTE_ERROR            (TPM_RET_ਅਧਾਰ + 0xe)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_READ_BYTE_ERROR             (TPM_RET_ਅਧਾਰ + 0xf)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_ਖੰਡ_WRITE_TIMEOUT         (TPM_RET_ਅਧਾਰ + 0x10)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_CHAR_WRITE_TIMEOUT          (TPM_RET_ਅਧਾਰ + 0x11)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_CHAR_READ_TIMEOUT           (TPM_RET_ਅਧਾਰ + 0x12)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_ਖੰਡ_READ_TIMEOUT          (TPM_RET_ਅਧਾਰ + 0x13)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_TRANSFER_ABORT              (TPM_RET_ਅਧਾਰ + 0x14)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INVALID_DRV_FUNCTION        (TPM_RET_ਅਧਾਰ + 0x15)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_OUTPUT_ਬਫਰ_TOO_SHORT     (TPM_RET_ਅਧਾਰ + 0x16)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_FATAL_COM_ERROR             (TPM_RET_ਅਧਾਰ + 0x17)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INVALID_ਨਿਵੇਸ਼_PARA          (TPM_RET_ਅਧਾਰ + 0x18)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_TCG_COMMAND_ERROR           (TPM_RET_ਅਧਾਰ + 0x19)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_INTERFACE_SHUTDOWN          (TPM_RET_ਅਧਾਰ + 0x20)
//define TCG_PC_UNSUPPORTED             (TPM_RET_BASE + 0x21)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_PC_TPM_NOT_PRESENT          (TPM_RET_ਅਧਾਰ + 0x22)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCG_PC_TPM_DEACTIVATED          (TPM_RET_ਅਧਾਰ + 0x23)

/* interrupt identifiers (al register) */
ਗਿਣਤੀ irq_ids {
    TCG_ਸਥਿਤੀ_Check = 0,
    TCG_HashLogExtendEvent = 1,
    TCG_Pass_Through_ਮੰਜ਼ਿਲ_TPM = 2,
    TCG_ShutdownPreBootInterface = 3,
    TCG_HashLogEvent = 4,
    TCG_HashAll = 5,
    TCG_TSS = 6,
    TCG_CompactHashLogExtendEvent = 7,
};

/* Input and Output blocks for the TCG BIOS commands */

ਬਣਤਰ hleei_short
{
    u16   ipblength;
    u16   reserved;
    ਅਚਲ ਖਾਲੀ *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    ਅਚਲ ਖਾਲੀ *logdataptr;
    u32   logdatalen;
} PACKED;

ਬਣਤਰ hleei_long
{
    u16   ipblength;
    u16   reserved;
    ਖਾਲੀ *hashdataptr;
    u32   hashdatalen;
    u32   pcrindex;
    u32   reserved2;
    ਖਾਲੀ *logdataptr;
    u32   logdatalen;
} PACKED;

ਬਣਤਰ hleeo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ਬਣਤਰ pttti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tpmopin[0];
} PACKED;

ਬਣਤਰ pttto
{
    u16    opblength;
    u16    reserved;
    u8     tpmopout[0];
};

ਬਣਤਰ hlei
{
    u16    ipblength;
    u16    reserved;
    ਅਚਲ ਖਾਲੀ  *hashdataptr;
    u32    hashdatalen;
    u32    pcrindex;
    u32    logeventtype;
    ਅਚਲ ਖਾਲੀ  *logdataptr;
    u32    logdatalen;
} PACKED;

ਬਣਤਰ hleo
{
    u16    opblength;
    u16    reserved;
    u32    eventnumber;
} PACKED;

ਬਣਤਰ hai
{
    u16    ipblength;
    u16    reserved;
    ਅਚਲ ਖਾਲੀ  *hashdataptr;
    u32    hashdatalen;
    u32    algorithmid;
} PACKED;

ਬਣਤਰ ti
{
    u16    ipblength;
    u16    reserved;
    u16    opblength;
    u16    reserved2;
    u8     tssoperandin[0];
} PACKED;

ਬਣਤਰ ਮੰਜ਼ਿਲ
{
    u16    opblength;
    u16    reserved;
    u8     tssoperandout[0];
} PACKED;

ਬਣਤਰ pcpes
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_Self_Test_ਭਰਿਆ             0x00000050
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_ForceClear               0x0000005d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_GetCapability            0x00000065
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_PhysicalEnable           0x0000006f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_PhysicalDisable          0x00000070
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_ਸੈੱਟ_Owner_Install          0x00000071
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_Physical_ਸੈੱਟ_Deactivated   0x00000072
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_ਸੈੱਟ_ਆਰਜ਼ੀ_Deactivated       0x00000073
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_Startup                  0x00000099
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_PhysicalPresence         0x4000000a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ORD_Extend                   0x00000014
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TSC_ORD_Reset_Establishment_ਦੋਆਧਾਰੀ_ਅੰਕ    0x4000000b

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ST_CLEAR                     0x0001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ST_STATE                     0x0002
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ST_DEACTIVATED               0x0003

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PP_CMD_ENABLE                0x0020
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PP_PRESENT                   0x0008
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PP_NOT_PRESENT_ਤਾਲਾ          0x0014

/* TPM command error codes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_INVALID_POSTINIT             0x26
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_BAD_LOCALITY                 0x3d

/* TPM command tags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ਟੈਗ_RQU_CMD                  0x00c1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ਟੈਗ_RQU_AUTH1_CMD            0x00c2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_ਟੈਗ_RQU_AUTH2_CMD            0x00c3

ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ {
    u16    ਟੈਗ;
    u32    totlen;
    u32    ordinal;
} PACKED;

ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ {
    u16    ਟੈਗ;
    u32    totlen;
    u32    errcode;
} PACKED;

ਬਣਤਰ tpm_req_extend {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32    pcrindex;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ਬਣਤਰ tpm_rsp_extend {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u8     digest[SHA1_BUFSIZE];
} PACKED;

ਬਣਤਰ tpm_req_getcap {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32    capArea;
    u32    sub_Cap_ਮਾਪ;
    u32    subCap;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_CAP_FLAG     0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_CAP_PROPERTY 0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_CAP_FLAG_PERMANENT   0x108
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_CAP_FLAG_VOLATILE    0x109
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_CAP_PROP_OWNER       0x111
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_CAP_PROP_TIS_TIMEOUT 0x115
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_CAP_PROP_DURATION    0x120

ਬਣਤਰ tpm_permanent_flags {
    u16    ਟੈਗ;
    u8     flags[20];
} PACKED;

ਗਿਣਤੀ perm_Flags_ਸੂਚਕਾਂਕ {
    PERM_FLAG_IDX_DISABLE = 0,
    PERM_FLAG_IDX_OWNERSHIP,
    PERM_FLAG_IDX_DEACTIVATED,
    PERM_FLAG_IDX_READPUBEK,
    PERM_FLAG_IDX_DISABLEOWNERCLEAR,
    PERM_FLAG_IDX_ALLOW_MAINTENANCE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_LIFETIME_ਤਾਲਾ,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_HW_ENABLE,
    PERM_FLAG_IDX_PHYSICAL_PRESENCE_CMD_ENABLE,
};

ਬਣਤਰ tpm_res_getcap_perm_flags {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u32    ਮਾਪ;
    ਬਣਤਰ tpm_permanent_flags perm_flags;
} PACKED;

ਬਣਤਰ tpm_stclear_flags {
    u16    ਟੈਗ;
    u8     flags[5];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STCLEAR_FLAG_IDX_DEACTIVATED 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STCLEAR_FLAG_IDX_DISABLE_FORCE_CLEAR 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STCLEAR_FLAG_IDX_PHYSICAL_PRESENCE_ਤਾਲਾ 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STCLEAR_FLAG_IDX_GLOBAL_ਤਾਲਾ 4

ਬਣਤਰ tpm_res_getcap_stclear_flags {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u32    ਮਾਪ;
    ਬਣਤਰ tpm_stclear_flags stclear_flags;
} PACKED;

ਬਣਤਰ tpm_res_getcap_ownerauth {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u32    ਮਾਪ;
    u8     flag;
} PACKED;

ਬਣਤਰ tpm_res_getcap_timeouts {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u32    ਮਾਪ;
    u32    timeouts[4];
} PACKED;

ਬਣਤਰ tpm_res_getcap_durations {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u32    ਮਾਪ;
    u32    durations[3];
} PACKED;

ਬਣਤਰ tpm_res_sha1start {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u32    max_ਅੰਕ_ਬਾਈਟਾਂ;
} PACKED;

ਬਣਤਰ tpm_res_sha1complete {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u8     hash[20];
} PACKED;


/****************************************************************
 * TPM v2.0 hardware commands
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_NO                     0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_YES                    1

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_SU_CLEAR               0x0000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_SU_STATE               0x0001

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_RH_OWNER               0x40000001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_RS_PW                  0x40000009
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_RH_ENDORSEMENT         0x4000000b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_RH_PLATFORM            0x4000000c

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA1               0x0004
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA256             0x000b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA384             0x000c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA512             0x000d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SM3_256            0x0012
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA3_256           0x0027
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA3_384           0x0028
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA3_512           0x0029

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA1_FLAG          (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA256_FLAG        (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA384_FLAG        (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA512_FLAG        (1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SM3_256_FLAG       (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA3_256_FLAG      (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA3_384_FLAG      (1 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ALG_SHA3_512_FLAG      (1 << 7)

/* TPM 2 command tags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ST_NO_SESSIONS         0x8001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_ST_SESSIONS            0x8002

/* TPM 2 commands */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_HierarchyControl    0x121
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_Clear               0x126
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_ClearControl        0x127
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_HierarchyChangeAuth 0x129
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_PCR_Allocate        0x12b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_SelfTest            0x143
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_Startup             0x144
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_Shutdown            0x145
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_StirRandom          0x146
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_GetCapability       0x17a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_GetRandom           0x17b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CC_PCR_Extend          0x182

/* TPM 2 error codes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_RC_INITIALIZE          0x100

/* TPM 2 Capabilities */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM2_CAP_PCRS               0x00000005

/* TPM 2 data structures */

ਬਣਤਰ tpm2_req_stirrandom {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u16 ਮਾਪ;
    u64 stir;
} PACKED;

ਬਣਤਰ tpm2_req_getrandom {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u16 ਬਾਈਟਾਂ_Requested;
} PACKED;

ਬਣਤਰ tpm2b_20 {
    u16 ਮਾਪ;
    u8 ਬਫਰ[20];
} PACKED;

ਬਣਤਰ tpm2_res_getrandom {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    ਬਣਤਰ tpm2b_20 rnd;
} PACKED;

ਬਣਤਰ tpm2_authblock {
    u32 handle;
    u16 noncesize;  /* always 0 */
    u8 contsession; /* always TPM2_YES */
    u16 pwdsize;    /* always 0 */
} PACKED;

ਬਣਤਰ tpm2_req_hierarchychangeauth {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32 authhandle;
    u32 authblocksize;
    ਬਣਤਰ tpm2_authblock authblock;
    ਬਣਤਰ tpm2b_20 newAuth;
} PACKED;

ਬਣਤਰ tpm2_req_extend {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32 pcrindex;
    u32 authblocksize;
    ਬਣਤਰ tpm2_authblock authblock;
    u8 digest[0];
} PACKED;

ਬਣਤਰ tpm2_req_clearcontrol {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32 authhandle;
    u32 authblocksize;
    ਬਣਤਰ tpm2_authblock authblock;
    u8 disable;
} PACKED;

ਬਣਤਰ tpm2_req_clear {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32 authhandle;
    u32 authblocksize;
    ਬਣਤਰ tpm2_authblock authblock;
} PACKED;

ਬਣਤਰ tpm2_req_hierarchycontrol {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32 authhandle;
    u32 authblocksize;
    ਬਣਤਰ tpm2_authblock authblock;
    u32 enable;
    u8 state;
} PACKED;

ਬਣਤਰ tpm2_req_getcapability {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32 capability;
    u32 property;
    u32 propertycount;
} PACKED;

ਬਣਤਰ tpm2_res_getcapability {
    ਬਣਤਰ tpm_rsp_ਸਿਰਲੇਖ hdr;
    u8 ਹੋਰ_ਅੰਕੜੇ;
    u32 capability;
    u8 ਅੰਕੜੇ[0]; /* capability dependent data */
} PACKED;

ਬਣਤਰ tpm2_req_pcr_allocate {
    ਬਣਤਰ tpm_req_ਸਿਰਲੇਖ hdr;
    u32 authhandle;
    u32 authblocksize;
    ਬਣਤਰ tpm2_authblock authblock;
    u32 ਗਿਣਤੀ_2;
    u8 tpms_pcr_selections[4];
} PACKED;

ਬਣਤਰ tpms_pcr_selection {
    u16 hashAlg;
    u8 ਮਾਪ_ਦਾ_Select;
    u8 pcrSelect[0];
} PACKED;

ਬਣਤਰ tpml_pcr_selection {
    u32 ਗਿਣਤੀ_2;
    ਬਣਤਰ tpms_pcr_selection selections[0];
} PACKED;


/****************************************************************
 * ACPI TCPA table interface
 ****************************************************************/

/* event types: 10.4.1 / table 11 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EV_POST_ਕੋਡ             1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EV_NO_ACTION             3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EV_SEPARATOR             4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EV_ACTION                5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EV_EVENT_ਟੈਗ             6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EV_COMPACT_HASH         12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EV_IPL                  13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EV_IPL_ਹਿੱਸੇ_ਕਰੋ_ਅੰਕੜੇ   14

ਬਣਤਰ tpm2_digest_ਮੁੱਲ {
    u16 hashAlg;
    u8 hash[0]; /* size depends on hashAlg */
} PACKED;

ਬਣਤਰ tpm2_digest_ਮੁੱਲਾਂ {
    u32 ਗਿਣਤੀ_2;
    ਬਣਤਰ tpm2_digest_ਮੁੱਲ digest[0];
} PACKED;

// Each entry in the TPM log contains: a tpm_log_header, a variable
// length digest, a tpm_log_trailer, and a variable length event.  The
// 'digest' matches what is sent to the TPM hardware via the Extend
// command.  On TPM1.2 the digest is a SHA1 hash; on TPM2.0 the digest
// contains a tpm2_digest_values struct followed by a variable number
// of tpm2_digest_value structs (as specified by the hardware via the
// TPM2_CAP_PCRS request).
ਬਣਤਰ tpm_log_ਸਿਰਲੇਖ {
    u32 pcrindex;
    u32 eventtype;
    u8 digest[0];
} PACKED;

ਬਣਤਰ tpm_log_trailer {
    u32 eventdatasize;
    u8 event[0];
} PACKED;

ਬਣਤਰ TCG_EfiSpecIdEventStruct {
    u8 signature[16];
    u32 platformClass;
    u8 spec_ਸੰਸਕਰਣ_Minor;
    u8 spec_ਸੰਸਕਰਣ_Major;
    u8 specErrata;
    u8 uintn_ਮਾਪ;
    u32 ਸੰਖਿਆ_ਦਾ_Algorithms;
    ਬਣਤਰ TCG_Efi_Spec_Id_Event_Algorithm_ਮਾਪ {
        u16 algorithmId;
        u16 digest_ਮਾਪ;
    } digestSizes[];
    /*
    u8 vendorInfoSize;
    u8 vendorInfo[0];
    */
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_TCPA_ACPI_CLASS_CLIENT 0

ਬਣਤਰ pcctes
{
    u32 eventid;
    u32 eventdatasize;
    u8  digest[SHA1_BUFSIZE];
} PACKED;

ਬਣਤਰ pcctes_romex
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_STATE_ENABLED 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_STATE_ਸਰਗਰਮ 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_STATE_OWNED 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_STATE_OWNERINSTALL 8

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PPI_OP_NOOP 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PPI_OP_ENABLE 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PPI_OP_DISABLE 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PPI_OP_ACTIVATE 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PPI_OP_DEACTIVATE 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PPI_OP_CLEAR 5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PPI_OP_ਸੈੱਟ_OWNERINSTALL_TRUE 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TPM_PPI_OP_ਸੈੱਟ_OWNERINSTALL_FALSE 9

#ਜੇ_ਅੰਤ // tcg.h
