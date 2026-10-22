// Misc function and variable declarations.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __STACKS_H
#ವ್ಯಾಖ್ಯಾನಿಸು __STACKS_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

#ವ್ಯಾಖ್ಯಾನಿಸು CALL32SMM_CMDID    0xb5
#ವ್ಯಾಖ್ಯಾನಿಸು CALL32SMM_ENTERID  0x1234
#ವ್ಯಾಖ್ಯಾನಿಸು CALL32SMM_RETURNID 0x5678

// stacks.c
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ HaveSmmCall32;
u32 __call32(ಶೂನ್ಯ *func, u32 eax, u32 errret);
#ವ್ಯಾಖ್ಯಾನಿಸು call32(func, eax, errret) ({                            \
        ಬಾಹ್ಯ ಶೂನ್ಯ _cfunc32flat_ ##func (ಶೂನ್ಯ);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
ಬಾಹ್ಯ u8 Extra_ರಾಶಿ[], *ರಾಶಿ_Pos;
u32 __stack_hop(u32 eax, u32 edx, ಶೂನ್ಯ *func);
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, ಶೂನ್ಯ *func);
#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_hop_ಹಿಂಭಾಗ(func, eax, edx) ({                               \
        ಬಾಹ್ಯ ಶೂನ್ಯ _cfunc16_ ##func (ಶೂನ್ಯ);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
ಪೂರ್ಣಾಂಕ on_extra_ರಾಶಿ(ಶೂನ್ಯ);
ರಚನೆ bregs;
ಶೂನ್ಯ farcall16(ರಚನೆ bregs *callregs);
ಶೂನ್ಯ farcall16big(ರಚನೆ bregs *callregs);
ಶೂನ್ಯ __call16_int(ರಚನೆ bregs *callregs, u16 ಸ್ಥಾನಾಂತರ_2);
#ವ್ಯಾಖ್ಯಾನಿಸು call16_int(nr, callregs) ಮಾಡು {                           \
        ಬಾಹ್ಯ ಶೂನ್ಯ irq_trampoline_ ##nr (ಶೂನ್ಯ);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } ಆಗ (0)
ಶೂನ್ಯ reset(ಶೂನ್ಯ);
ಬಾಹ್ಯ ರಚನೆ thread_info MainThread;
ರಚನೆ thread_info *getCurThread(ಶೂನ್ಯ);
ಶೂನ್ಯ yield(ಶೂನ್ಯ);
ಶೂನ್ಯ yield_toirq(ಶೂನ್ಯ);
ಶೂನ್ಯ thread_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ ಎಳೆಗಳು_during_optionroms(ಶೂನ್ಯ);
ಶೂನ್ಯ ಸತತ_thread(ಶೂನ್ಯ (*func)(ಶೂನ್ಯ*), ಶೂನ್ಯ *ದತ್ತಾಂಶ);
ಶೂನ್ಯ wait_ಎಳೆಗಳು(ಶೂನ್ಯ);
ರಚನೆ mutex_s { u32 isLocked; };
ಶೂನ್ಯ mutex_ಬೀಗ(ರಚನೆ mutex_s *mutex);
ಶೂನ್ಯ mutex_unlock(ರಚನೆ mutex_s *mutex);
ಶೂನ್ಯ ಆರಂಭ_preempt(ಶೂನ್ಯ);
ಶೂನ್ಯ ಮುಕ್ತಾಯ_preempt(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ wait_preempt(ಶೂನ್ಯ);
ಶೂನ್ಯ check_preempt(ಶೂನ್ಯ);
u32 __call32_params(ಶೂನ್ಯ *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#ವ್ಯಾಖ್ಯಾನಿಸು call32_params(func, eax, edx, ecx, errret) ({                   \
        ಬಾಹ್ಯ ಶೂನ್ಯ _cfunc32flat_ ##func (ಶೂನ್ಯ);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
need_hop_ಹಿಂಭಾಗ(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ !MODESEGMENT || on_extra_ರಾಶಿ();
}

#ಆದರೆ_ಅಂತ್ಯ // stacks.h
