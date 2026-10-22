// Misc function and variable declarations.
#اگر_تعریف_نشده __STACKS_H
#تعریف_کن __STACKS_H

#ضمن "types.h" // u32

#تعریف_کن CALL32SMM_CMDID    0xb5
#تعریف_کن CALL32SMM_ENTERID  0x1234
#تعریف_کن CALL32SMM_RETURNID 0x5678

// stacks.c
خارجي صحيح HaveSmmCall32;
u32 __call32(فراغ *func, u32 eax, u32 errret);
#تعریف_کن call32(func, eax, errret) ({                            \
        خارجي فراغ _cfunc32flat_ ##func (فراغ);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
خارجي u8 Extra_پشته[], *پشته_Pos;
u32 __stack_hop(u32 eax, u32 edx, فراغ *func);
#تعریف_کن پشته_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, فراغ *func);
#تعریف_کن پشته_hop_پشت(func, eax, edx) ({                               \
        خارجي فراغ _cfunc16_ ##func (فراغ);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
صحيح on_extra_پشته(فراغ);
بنية bregs;
فراغ farcall16(بنية bregs *callregs);
فراغ farcall16big(بنية bregs *callregs);
فراغ __call16_int(بنية bregs *callregs, u16 جابه_جایی_2);
#تعریف_کن call16_int(nr, callregs) افعل {                           \
        خارجي فراغ irq_trampoline_ ##nr (فراغ);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } طالما (0)
فراغ reset(فراغ);
خارجي بنية thread_info MainThread;
بنية thread_info *getCurThread(فراغ);
فراغ yield(فراغ);
فراغ yield_toirq(فراغ);
فراغ thread_راهاندازی(فراغ);
صحيح نخها_during_optionroms(فراغ);
فراغ پیاپی_thread(فراغ (*func)(فراغ*), فراغ *داده);
فراغ wait_نخها(فراغ);
بنية mutex_s { u32 isLocked; };
فراغ mutex_قفل(بنية mutex_s *mutex);
فراغ mutex_unlock(بنية mutex_s *mutex);
فراغ شروع_preempt(فراغ);
فراغ پایان_preempt(فراغ);
صحيح wait_preempt(فراغ);
فراغ check_preempt(فراغ);
u32 __call32_params(فراغ *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#تعریف_کن call32_params(func, eax, edx, ecx, errret) ({                   \
        خارجي فراغ _cfunc32flat_ ##func (فراغ);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ساكن مضمن صحيح
need_hop_پشت(فراغ)
{
    أرجع !MODESEGMENT || on_extra_پشته();
}

#پایان_اگر // stacks.h
