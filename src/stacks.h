// Misc function and variable declarations.
#که_نه_تعریف_شوی __STACKS_H
#تعریف_کړه __STACKS_H

#ضمن "types.h" // u32

#تعریف_کړه CALL32SMM_CMDID    0xb5
#تعریف_کړه CALL32SMM_ENTERID  0x1234
#تعریف_کړه CALL32SMM_RETURNID 0x5678

// stacks.c
خارجي صحيح HaveSmmCall32;
u32 __call32(فراغ *func, u32 eax, u32 errret);
#تعریف_کړه call32(func, eax, errret) ({                            \
        خارجي فراغ _cfunc32flat_ ##func (فراغ);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
خارجي u8 Extra_ډېری[], *ډېری_Pos;
u32 __stack_hop(u32 eax, u32 edx, فراغ *func);
#تعریف_کړه ډېری_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, فراغ *func);
#تعریف_کړه ډېری_hop_شا(func, eax, edx) ({                               \
        خارجي فراغ _cfunc16_ ##func (فراغ);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
صحيح on_extra_ډېری(فراغ);
بنية bregs;
فراغ farcall16(بنية bregs *callregs);
فراغ farcall16big(بنية bregs *callregs);
فراغ __call16_int(بنية bregs *callregs, u16 دننه_واټن_2);
#تعریف_کړه call16_int(nr, callregs) افعل {                           \
        خارجي فراغ irq_trampoline_ ##nr (فراغ);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } طالما (0)
فراغ reset(فراغ);
خارجي بنية thread_info MainThread;
بنية thread_info *getCurThread(فراغ);
فراغ yield(فراغ);
فراغ yield_toirq(فراغ);
فراغ thread_تیاری(فراغ);
صحيح تارونه_during_optionroms(فراغ);
فراغ پرلپسې_thread(فراغ (*func)(فراغ*), فراغ *معلومات);
فراغ wait_تارونه(فراغ);
بنية mutex_s { u32 isLocked; };
فراغ mutex_قلف(بنية mutex_s *mutex);
فراغ mutex_unlock(بنية mutex_s *mutex);
فراغ پیل_preempt(فراغ);
فراغ پای_preempt(فراغ);
صحيح wait_preempt(فراغ);
فراغ check_preempt(فراغ);
u32 __call32_params(فراغ *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#تعریف_کړه call32_params(func, eax, edx, ecx, errret) ({                   \
        خارجي فراغ _cfunc32flat_ ##func (فراغ);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ساكن مضمن صحيح
need_hop_شا(فراغ)
{
    أرجع !MODESEGMENT || on_extra_ډېری();
}

#د_که_پای // stacks.h
