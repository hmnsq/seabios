// Misc function and variable declarations.
#నిర్వచితం_కాకపోతే __STACKS_H
#నిర్వచించు __STACKS_H

#అన్తర్భావ "types.h" // u32

#నిర్వచించు CALL32SMM_CMDID    0xb5
#నిర్వచించు CALL32SMM_ENTERID  0x1234
#నిర్వచించు CALL32SMM_RETURNID 0x5678

// stacks.c
బాహ్య పూర్ణాంకం HaveSmmCall32;
u32 __call32(శూన్య *func, u32 eax, u32 errret);
#నిర్వచించు call32(func, eax, errret) ({                            \
        బాహ్య శూన్య _cfunc32flat_ ##func (శూన్య);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
బాహ్య u8 Extra_దొంతర[], *దొంతర_Pos;
u32 __stack_hop(u32 eax, u32 edx, శూన్య *func);
#నిర్వచించు దొంతర_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, శూన్య *func);
#నిర్వచించు దొంతర_hop_వెనుక(func, eax, edx) ({                               \
        బాహ్య శూన్య _cfunc16_ ##func (శూన్య);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
పూర్ణాంకం on_extra_దొంతర(శూన్య);
నిర్మాణం bregs;
శూన్య farcall16(నిర్మాణం bregs *callregs);
శూన్య farcall16big(నిర్మాణం bregs *callregs);
శూన్య __call16_int(నిర్మాణం bregs *callregs, u16 స్థానభ్రంశం_2);
#నిర్వచించు call16_int(nr, callregs) చేయి {                           \
        బాహ్య శూన్య irq_trampoline_ ##nr (శూన్య);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } అయితే (0)
శూన్య reset(శూన్య);
బాహ్య నిర్మాణం thread_info MainThread;
నిర్మాణం thread_info *getCurThread(శూన్య);
శూన్య yield(శూన్య);
శూన్య yield_toirq(శూన్య);
శూన్య thread_అమరిక(శూన్య);
పూర్ణాంకం దారాలు_during_optionroms(శూన్య);
శూన్య నిరంతరం_thread(శూన్య (*func)(శూన్య*), శూన్య *దత్తాంశం);
శూన్య wait_దారాలు(శూన్య);
నిర్మాణం mutex_s { u32 isLocked; };
శూన్య mutex_తాళం(నిర్మాణం mutex_s *mutex);
శూన్య mutex_unlock(నిర్మాణం mutex_s *mutex);
శూన్య ప్రారంభం_preempt(శూన్య);
శూన్య ముగింపు_preempt(శూన్య);
పూర్ణాంకం wait_preempt(శూన్య);
శూన్య check_preempt(శూన్య);
u32 __call32_params(శూన్య *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#నిర్వచించు call32_params(func, eax, edx, ecx, errret) ({                   \
        బాహ్య శూన్య _cfunc32flat_ ##func (శూన్య);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
స్థిర అంతఃవరుస పూర్ణాంకం
need_hop_వెనుక(శూన్య)
{
    తిరిగి !MODESEGMENT || on_extra_దొంతర();
}

#అయితే_ముగింపు // stacks.h
