// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା HaveSmmCall32;
u32 __call32(ଶୂନ୍ୟ *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        ବାହ୍ୟ ଶୂନ୍ୟ _cfunc32flat_ ##func (ଶୂନ୍ୟ);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
ବାହ୍ୟ u8 Extra_ଗଦା[], *ଗଦା_Pos;
u32 __stack_hop(u32 eax, u32 edx, ଶୂନ୍ୟ *func);
#define ଗଦା_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, ଶୂନ୍ୟ *func);
#define ଗଦା_hop_back(func, eax, edx) ({                               \
        ବାହ୍ୟ ଶୂନ୍ୟ _cfunc16_ ##func (ଶୂନ୍ୟ);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
ପୂର୍ଣ୍ଣସଂଖ୍ୟା on_extra_ଗଦା(ଶୂନ୍ୟ);
ସଂରଚନା bregs;
ଶୂନ୍ୟ farcall16(ସଂରଚନା bregs *callregs);
ଶୂନ୍ୟ farcall16big(ସଂରଚନା bregs *callregs);
ଶୂନ୍ୟ __call16_int(ସଂରଚନା bregs *callregs, u16 offset);
#define call16_int(nr, callregs) କର {                           \
        ବାହ୍ୟ ଶୂନ୍ୟ irq_trampoline_ ##nr (ଶୂନ୍ୟ);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } ଯେତେବେଳେ (0)
ଶୂନ୍ୟ reset(ଶୂନ୍ୟ);
ବାହ୍ୟ ସଂରଚନା thread_info MainThread;
ସଂରଚନା thread_info *getCurThread(ଶୂନ୍ୟ);
ଶୂନ୍ୟ yield(ଶୂନ୍ୟ);
ଶୂନ୍ୟ yield_toirq(ଶୂନ୍ୟ);
ଶୂନ୍ୟ thread_setup(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା threads_during_optionroms(ଶୂନ୍ୟ);
ଶୂନ୍ୟ run_thread(ଶୂନ୍ୟ (*func)(ଶୂନ୍ୟ*), ଶୂନ୍ୟ *data);
ଶୂନ୍ୟ wait_threads(ଶୂନ୍ୟ);
ସଂରଚନା mutex_s { u32 isLocked; };
ଶୂନ୍ୟ mutex_lock(ସଂରଚନା mutex_s *mutex);
ଶୂନ୍ୟ mutex_unlock(ସଂରଚନା mutex_s *mutex);
ଶୂନ୍ୟ start_preempt(ଶୂନ୍ୟ);
ଶୂନ୍ୟ finish_preempt(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା wait_preempt(ଶୂନ୍ୟ);
ଶୂନ୍ୟ check_preempt(ଶୂନ୍ୟ);
u32 __call32_params(ଶୂନ୍ୟ *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        ବାହ୍ୟ ଶୂନ୍ୟ _cfunc32flat_ ##func (ଶୂନ୍ୟ);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା
need_hop_back(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ !MODESEGMENT || on_extra_ଗଦା();
}

#endif // stacks.h
