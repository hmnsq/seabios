// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#অন্তর্ভাব "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
বাহ্যিক পূর্ণসংখ্যা HaveSmmCall32;
u32 __call32(শূন্য *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        বাহ্যিক শূন্য _cfunc32flat_ ##func (শূন্য);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
বাহ্যিক u8 Extra_স্তূপ[], *স্তূপ_Pos;
u32 __stack_hop(u32 eax, u32 edx, শূন্য *func);
#define স্তূপ_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, শূন্য *func);
#define স্তূপ_hop_back(func, eax, edx) ({                               \
        বাহ্যিক শূন্য _cfunc16_ ##func (শূন্য);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
পূর্ণসংখ্যা on_extra_স্তূপ(শূন্য);
কাঠামো bregs;
শূন্য farcall16(কাঠামো bregs *callregs);
শূন্য farcall16big(কাঠামো bregs *callregs);
শূন্য __call16_int(কাঠামো bregs *callregs, u16 offset);
#define call16_int(nr, callregs) করো {                           \
        বাহ্যিক শূন্য irq_trampoline_ ##nr (শূন্য);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } যখন (0)
শূন্য reset(শূন্য);
বাহ্যিক কাঠামো thread_info MainThread;
কাঠামো thread_info *getCurThread(শূন্য);
শূন্য yield(শূন্য);
শূন্য yield_toirq(শূন্য);
শূন্য thread_setup(শূন্য);
পূর্ণসংখ্যা threads_during_optionroms(শূন্য);
শূন্য run_thread(শূন্য (*func)(শূন্য*), শূন্য *তথ্য);
শূন্য wait_threads(শূন্য);
কাঠামো mutex_s { u32 isLocked; };
শূন্য mutex_lock(কাঠামো mutex_s *mutex);
শূন্য mutex_unlock(কাঠামো mutex_s *mutex);
শূন্য start_preempt(শূন্য);
শূন্য finish_preempt(শূন্য);
পূর্ণসংখ্যা wait_preempt(শূন্য);
শূন্য check_preempt(শূন্য);
u32 __call32_params(শূন্য *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        বাহ্যিক শূন্য _cfunc32flat_ ##func (শূন্য);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
স্থির অন্তঃরেখ পূর্ণসংখ্যা
need_hop_back(শূন্য)
{
    ফেরত !MODESEGMENT || on_extra_স্তূপ();
}

#endif // stacks.h
