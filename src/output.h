#నిర్వచితం_కాకపోతే __OUTPUT_H
#నిర్వచించు __OUTPUT_H

#అన్తర్భావ "config.h" // CONFIG_DEBUG_LEVEL
#అన్తర్భావ "types.h" // u32

// output.c
శూన్య debug_banner(శూన్య);
శూన్య panic(స్థిరరాశి అక్షరం *fmt, ...)
    __attribute__ ((format (రాయి, 1, 2))) __noreturn;
శూన్య రాయి(స్థిరరాశి అక్షరం *fmt, ...)
    __attribute__ ((format (రాయి, 1, 2)));
పూర్ణాంకం snprintf(అక్షరం *str, కొలత_t కొలత, స్థిరరాశి అక్షరం *fmt, ...)
    __attribute__ ((format (రాయి, 3, 4)));
అక్షరం * znprintf(కొలత_t కొలత, స్థిరరాశి అక్షరం *fmt, ...)
    __attribute__ ((format (రాయి, 2, 3)));
శూన్య __dprintf(స్థిరరాశి అక్షరం *fmt, ...)
    __attribute__ ((format (రాయి, 1, 2)));
నిర్మాణం bregs;
శూన్య __debug_enter(నిర్మాణం bregs *regs, స్థిరరాశి అక్షరం *fname);
శూన్య __debug_isr(స్థిరరాశి అక్షరం *fname);
శూన్య __debug_stub(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname);
శూన్య __warn_invalid(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname);
శూన్య __warn_unimplemented(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname);
శూన్య __warn_internalerror(పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname);
శూన్య __warn_noalloc(పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname);
శూన్య __warn_timeout(పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname);
శూన్య __set_invalid(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname);
శూన్య __set_unimplemented(నిర్మాణం bregs *regs, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *fname);
శూన్య __set_code_invalid(నిర్మాణం bregs *regs, u32 linecode, స్థిరరాశి అక్షరం *fname);
శూన్య __set_code_unimplemented(నిర్మాణం bregs *regs, u32 linecode
                              , స్థిరరాశి అక్షరం *fname);
శూన్య hexdump(స్థిరరాశి శూన్య *d, పూర్ణాంకం len);

#నిర్వచించు dprintf(lvl, fmt, వాదనలు...) చేయి {                         \
        ఒకవేళ (CONFIG_DEBUG_LEVEL && (lvl) <= CONFIG_DEBUG_LEVEL)  \
            __dprintf((fmt) , ##args );                         \
    } అయితే (0)
#నిర్వచించు debug_enter(regs, lvl) చేయి {                     \
        ఒకవేళ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_enter((regs), __func__);            \
    } అయితే (0)
#నిర్వచించు debug_isr(lvl) చేయి {                             \
        ఒకవేళ ((lvl) && (lvl) <= CONFIG_DEBUG_LEVEL)       \
            __debug_isr(__func__);                      \
    } అయితే (0)
#నిర్వచించు debug_stub(regs)                        \
    __debug_stub((regs), __LINE__, __func__)
#నిర్వచించు warn_invalid(regs)                      \
    __warn_invalid((regs), __LINE__, __func__)
#నిర్వచించు warn_unimplemented(regs)                        \
    __warn_unimplemented((regs), __LINE__, __func__)
#నిర్వచించు warn_internalerror()                    \
    __warn_internalerror(__LINE__, __func__)
#నిర్వచించు warn_noalloc()                          \
    __warn_noalloc(__LINE__, __func__)
#నిర్వచించు warn_timeout()                          \
    __warn_timeout(__LINE__, __func__)
#నిర్వచించు సమితి_invalid(regs)                       \
    __set_invalid((regs), __LINE__, __func__)
#నిర్వచించు సమితి_సంకేతం_invalid(regs, సంకేతం_2)                                    \
    __set_code_invalid((regs), (సంకేతం_2) | (__LINE__ << 8), __func__)
#నిర్వచించు సమితి_unimplemented(regs)                         \
    __set_unimplemented((regs), __LINE__, __func__)
#నిర్వచించు సమితి_సంకేతం_unimplemented(regs, సంకేతం_2)                              \
    __set_code_unimplemented((regs), (సంకేతం_2) | (__LINE__ << 8), __func__)

#అయితే_ముగింపు // output.h
