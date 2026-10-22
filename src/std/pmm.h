#अगर_अघोषित __PMM_H
#घोषणा __PMM_H

#समावेश "types.h" // u32

#घोषणा PMM_SIGNATURE 0x4d4d5024 // $PMM

काष्ठा pmmheader {
    u32 signature;
    u8 version;
    u8 लंबाई;
    u8 checksum;
    काष्ठा segoff_s entry;
    u8 reserved[5];
} PACKED;

#घोषणा PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#पूर्ण_अगर // pmm.h
