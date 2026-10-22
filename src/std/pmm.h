#నిర్వచితం_కాకపోతే __PMM_H
#నిర్వచించు __PMM_H

#అన్తర్భావ "types.h" // u32

#నిర్వచించు PMM_SIGNATURE 0x4d4d5024 // $PMM

నిర్మాణం pmmheader {
    u32 signature;
    u8 సంస్కరణ_2;
    u8 నిడివి;
    u8 checksum;
    నిర్మాణం segoff_s నమోదు_2;
    u8 reserved[5];
} PACKED;

#నిర్వచించు PMM_FUNCTION_NOT_SUPPORTED 0xffffffff

#అయితే_ముగింపు // pmm.h
