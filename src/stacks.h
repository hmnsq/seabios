// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#അന്തര്ഭാവ "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
ബാഹ്യ പൂർണ്ണസംഖ്യ HaveSmmCall32;
u32 __call32(ശൂന്യം *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        ബാഹ്യ ശൂന്യം _cfunc32flat_ ##func (ശൂന്യം);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
ബാഹ്യ u8 Extra_അട്ടി[], *അട്ടി_Pos;
u32 __stack_hop(u32 eax, u32 edx, ശൂന്യം *func);
#define അട്ടി_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, ശൂന്യം *func);
#define അട്ടി_hop_back(func, eax, edx) ({                               \
        ബാഹ്യ ശൂന്യം _cfunc16_ ##func (ശൂന്യം);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
പൂർണ്ണസംഖ്യ on_extra_അട്ടി(ശൂന്യം);
ഘടന bregs;
ശൂന്യം farcall16(ഘടന bregs *callregs);
ശൂന്യം farcall16big(ഘടന bregs *callregs);
ശൂന്യം __call16_int(ഘടന bregs *callregs, u16 offset);
#define call16_int(nr, callregs) ചെയ്യുക {                           \
        ബാഹ്യ ശൂന്യം irq_trampoline_ ##nr (ശൂന്യം);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } ആകുമ്പോൾ (0)
ശൂന്യം reset(ശൂന്യം);
ബാഹ്യ ഘടന thread_info MainThread;
ഘടന thread_info *getCurThread(ശൂന്യം);
ശൂന്യം yield(ശൂന്യം);
ശൂന്യം yield_toirq(ശൂന്യം);
ശൂന്യം thread_setup(ശൂന്യം);
പൂർണ്ണസംഖ്യ threads_during_optionroms(ശൂന്യം);
ശൂന്യം run_thread(ശൂന്യം (*func)(ശൂന്യം*), ശൂന്യം *data);
ശൂന്യം wait_threads(ശൂന്യം);
ഘടന mutex_s { u32 isLocked; };
ശൂന്യം mutex_lock(ഘടന mutex_s *mutex);
ശൂന്യം mutex_unlock(ഘടന mutex_s *mutex);
ശൂന്യം start_preempt(ശൂന്യം);
ശൂന്യം finish_preempt(ശൂന്യം);
പൂർണ്ണസംഖ്യ wait_preempt(ശൂന്യം);
ശൂന്യം check_preempt(ശൂന്യം);
u32 __call32_params(ശൂന്യം *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        ബാഹ്യ ശൂന്യം _cfunc32flat_ ##func (ശൂന്യം);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ
need_hop_back(ശൂന്യം)
{
    തിരികെ !MODESEGMENT || on_extra_അട്ടി();
}

#endif // stacks.h
