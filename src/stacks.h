// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#અન્તર્ભાવ "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
બાહ્ય પૂર્ણાંક HaveSmmCall32;
u32 __call32(શૂન્ય *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        બાહ્ય શૂન્ય _cfunc32flat_ ##func (શૂન્ય);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
બાહ્ય u8 Extra_ઢગલો[], *ઢગલો_Pos;
u32 __stack_hop(u32 eax, u32 edx, શૂન્ય *func);
#define ઢગલો_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, શૂન્ય *func);
#define ઢગલો_hop_back(func, eax, edx) ({                               \
        બાહ્ય શૂન્ય _cfunc16_ ##func (શૂન્ય);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
પૂર્ણાંક on_extra_ઢગલો(શૂન્ય);
સંરચના bregs;
શૂન્ય farcall16(સંરચના bregs *callregs);
શૂન્ય farcall16big(સંરચના bregs *callregs);
શૂન્ય __call16_int(સંરચના bregs *callregs, u16 offset);
#define call16_int(nr, callregs) કરો {                           \
        બાહ્ય શૂન્ય irq_trampoline_ ##nr (શૂન્ય);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } જ્યારે (0)
શૂન્ય reset(શૂન્ય);
બાહ્ય સંરચના thread_info MainThread;
સંરચના thread_info *getCurThread(શૂન્ય);
શૂન્ય yield(શૂન્ય);
શૂન્ય yield_toirq(શૂન્ય);
શૂન્ય thread_setup(શૂન્ય);
પૂર્ણાંક threads_during_optionroms(શૂન્ય);
શૂન્ય run_thread(શૂન્ય (*func)(શૂન્ય*), શૂન્ય *data);
શૂન્ય wait_threads(શૂન્ય);
સંરચના mutex_s { u32 isLocked; };
શૂન્ય mutex_lock(સંરચના mutex_s *mutex);
શૂન્ય mutex_unlock(સંરચના mutex_s *mutex);
શૂન્ય start_preempt(શૂન્ય);
શૂન્ય finish_preempt(શૂન્ય);
પૂર્ણાંક wait_preempt(શૂન્ય);
શૂન્ય check_preempt(શૂન્ય);
u32 __call32_params(શૂન્ય *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        બાહ્ય શૂન્ય _cfunc32flat_ ##func (શૂન્ય);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
સ્થિર અંતઃરેખ પૂર્ણાંક
need_hop_back(શૂન્ય)
{
    પાછા !MODESEGMENT || on_extra_ઢગલો();
}

#endif // stacks.h
