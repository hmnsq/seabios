// String manipulation function defs.
#నిర్వచితం_కాకపోతే __STRING_H
#నిర్వచించు __STRING_H

#అన్తర్భావ "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, శూన్య *buf_far, u32 len);
u8 checksum(శూన్య *buf, u32 len);
కొలత_t అక్షరశ్రేణి_పొడవు(స్థిరరాశి అక్షరం *s);
పూర్ణాంకం memcmp_far(u16 s1seg, స్థిరరాశి శూన్య *s1, u16 s2seg, స్థిరరాశి శూన్య *s2, కొలత_t n);
పూర్ణాంకం memcmp(స్థిరరాశి శూన్య *s1, స్థిరరాశి శూన్య *s2, కొలత_t n);
పూర్ణాంకం strcmp(స్థిరరాశి అక్షరం *s1, స్థిరరాశి అక్షరం *s2);
శూన్య memset_far(u16 d_seg, శూన్య *d_far, u8 c, కొలత_t len);
శూన్య memset16_far(u16 d_seg, శూన్య *d_far, u16 c, కొలత_t len);
శూన్య *memset(శూన్య *s, పూర్ణాంకం c, కొలత_t n);
శూన్య memset_fl(శూన్య *ptr, u8 val, కొలత_t కొలత);
శూన్య memcpy_far(u16 d_seg, శూన్య *d_far
                , u16 s_seg, స్థిరరాశి శూన్య *s_far, కొలత_t len);
శూన్య memcpy_fl(శూన్య *d_fl, స్థిరరాశి శూన్య *s_fl, కొలత_t len);
శూన్య *జ్ఞాపకం_నకలు(శూన్య *d1, స్థిరరాశి శూన్య *s1, కొలత_t len);
#ఒకవేళ MODESEGMENT == 0
#నిర్వచించు జ్ఞాపకం_నకలు __builtin_memcpy
#అయితే_ముగింపు
శూన్య iomemcpy(శూన్య *d, స్థిరరాశి శూన్య *s, u32 len);
శూన్య *memmove(శూన్య *d, స్థిరరాశి శూన్య *s, కొలత_t len);
అక్షరం *strtcpy(అక్షరం *dest, స్థిరరాశి అక్షరం *src, కొలత_t len);
అక్షరం *strchr(స్థిరరాశి అక్షరం *s, పూర్ణాంకం c);
అక్షరం *nullTrailingSpace(అక్షరం *buf);

#అయితే_ముగింపు // string.h
