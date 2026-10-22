// Misc function and variable declarations.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __STACKS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __STACKS_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL32SMM_CMDID    0xb5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL32SMM_ENTERID  0x1234
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL32SMM_RETURNID 0x5678

// stacks.c
ਬਾਹਰੀ ਪੂਰਨ HaveSmmCall32;
u32 __call32(ਖਾਲੀ *func, u32 eax, u32 errret);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ call32(func, eax, errret) ({                            \
        ਬਾਹਰੀ ਖਾਲੀ _cfunc32flat_ ##func (ਖਾਲੀ);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
ਬਾਹਰੀ u8 Extra_ਢੇਰ[], *ਢੇਰ_Pos;
u32 __stack_hop(u32 eax, u32 edx, ਖਾਲੀ *func);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, ਖਾਲੀ *func);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_hop_ਪਿੱਠ(func, eax, edx) ({                               \
        ਬਾਹਰੀ ਖਾਲੀ _cfunc16_ ##func (ਖਾਲੀ);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
ਪੂਰਨ on_extra_ਢੇਰ(ਖਾਲੀ);
ਬਣਤਰ bregs;
ਖਾਲੀ farcall16(ਬਣਤਰ bregs *callregs);
ਖਾਲੀ farcall16big(ਬਣਤਰ bregs *callregs);
ਖਾਲੀ __call16_int(ਬਣਤਰ bregs *callregs, u16 ਵਿੱਥ_2);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ call16_int(nr, callregs) ਕਰੋ {                           \
        ਬਾਹਰੀ ਖਾਲੀ irq_trampoline_ ##nr (ਖਾਲੀ);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } ਜਦੋਂ (0)
ਖਾਲੀ reset(ਖਾਲੀ);
ਬਾਹਰੀ ਬਣਤਰ thread_info MainThread;
ਬਣਤਰ thread_info *getCurThread(ਖਾਲੀ);
ਖਾਲੀ yield(ਖਾਲੀ);
ਖਾਲੀ yield_toirq(ਖਾਲੀ);
ਖਾਲੀ thread_ਤਿਆਰੀ(ਖਾਲੀ);
ਪੂਰਨ ਧਾਗੇ_during_optionroms(ਖਾਲੀ);
ਖਾਲੀ ਲੜੀਵਾਰ_thread(ਖਾਲੀ (*func)(ਖਾਲੀ*), ਖਾਲੀ *ਅੰਕੜੇ);
ਖਾਲੀ wait_ਧਾਗੇ(ਖਾਲੀ);
ਬਣਤਰ mutex_s { u32 isLocked; };
ਖਾਲੀ mutex_ਤਾਲਾ(ਬਣਤਰ mutex_s *mutex);
ਖਾਲੀ mutex_unlock(ਬਣਤਰ mutex_s *mutex);
ਖਾਲੀ ਸ਼ੁਰੂ_preempt(ਖਾਲੀ);
ਖਾਲੀ ਅੰਤ_preempt(ਖਾਲੀ);
ਪੂਰਨ wait_preempt(ਖਾਲੀ);
ਖਾਲੀ check_preempt(ਖਾਲੀ);
u32 __call32_params(ਖਾਲੀ *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ call32_params(func, eax, edx, ecx, errret) ({                   \
        ਬਾਹਰੀ ਖਾਲੀ _cfunc32flat_ ##func (ਖਾਲੀ);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ
need_hop_ਪਿੱਠ(ਖਾਲੀ)
{
    ਵਾਪਸ !MODESEGMENT || on_extra_ਢੇਰ();
}

#ਜੇ_ਅੰਤ // stacks.h
