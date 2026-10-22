// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ HaveSmmCall32;
u32 __call32(ಶೂನ್ಯ *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        ಬಾಹ್ಯ ಶೂನ್ಯ _cfunc32flat_ ##func (ಶೂನ್ಯ);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
ಬಾಹ್ಯ u8 Extra_ರಾಶಿ[], *ರಾಶಿ_Pos;
u32 __stack_hop(u32 eax, u32 edx, ಶೂನ್ಯ *func);
#define ರಾಶಿ_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, ಶೂನ್ಯ *func);
#define ರಾಶಿ_hop_back(func, eax, edx) ({                               \
        ಬಾಹ್ಯ ಶೂನ್ಯ _cfunc16_ ##func (ಶೂನ್ಯ);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
ಪೂರ್ಣಾಙ್ಕ on_extra_ರಾಶಿ(ಶೂನ್ಯ);
ಸಂರಚನಾ bregs;
ಶೂನ್ಯ farcall16(ಸಂರಚನಾ bregs *callregs);
ಶೂನ್ಯ farcall16big(ಸಂರಚನಾ bregs *callregs);
ಶೂನ್ಯ __call16_int(ಸಂರಚನಾ bregs *callregs, u16 offset);
#define call16_int(nr, callregs) ಕುರು {                           \
        ಬಾಹ್ಯ ಶೂನ್ಯ irq_trampoline_ ##nr (ಶೂನ್ಯ);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } ಯಾವತ್ (0)
ಶೂನ್ಯ reset(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಸಂರಚನಾ thread_info MainThread;
ಸಂರಚನಾ thread_info *getCurThread(ಶೂನ್ಯ);
ಶೂನ್ಯ yield(ಶೂನ್ಯ);
ಶೂನ್ಯ yield_toirq(ಶೂನ್ಯ);
ಶೂನ್ಯ thread_setup(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ threads_during_optionroms(ಶೂನ್ಯ);
ಶೂನ್ಯ run_thread(ಶೂನ್ಯ (*func)(ಶೂನ್ಯ*), ಶೂನ್ಯ *data);
ಶೂನ್ಯ wait_threads(ಶೂನ್ಯ);
ಸಂರಚನಾ mutex_s { u32 isLocked; };
ಶೂನ್ಯ mutex_lock(ಸಂರಚನಾ mutex_s *mutex);
ಶೂನ್ಯ mutex_unlock(ಸಂರಚನಾ mutex_s *mutex);
ಶೂನ್ಯ start_preempt(ಶೂನ್ಯ);
ಶೂನ್ಯ finish_preempt(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ wait_preempt(ಶೂನ್ಯ);
ಶೂನ್ಯ check_preempt(ಶೂನ್ಯ);
u32 __call32_params(ಶೂನ್ಯ *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        ಬಾಹ್ಯ ಶೂನ್ಯ _cfunc32flat_ ##func (ಶೂನ್ಯ);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
need_hop_back(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ !MODESEGMENT || on_extra_ರಾಶಿ();
}

#endif // stacks.h
