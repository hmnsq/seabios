// Misc function and variable declarations.
#اگر_غیر_تعریف_شدہ __STACKS_H
#تعریف_کرو __STACKS_H

#شامل "types.h" // u32

#تعریف_کرو CALL32SMM_CMDID    0xb5
#تعریف_کرو CALL32SMM_ENTERID  0x1234
#تعریف_کرو CALL32SMM_RETURNID 0x5678

// stacks.c
خارجی صحیح HaveSmmCall32;
u32 __call32(خالی *func, u32 eax, u32 errret);
#تعریف_کرو call32(func, eax, errret) ({                            \
        خارجی خالی _cfunc32flat_ ##func (خالی);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
خارجی u8 Extra_ڈھیر[], *ڈھیر_Pos;
u32 __stack_hop(u32 eax, u32 edx, خالی *func);
#تعریف_کرو ڈھیر_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, خالی *func);
#تعریف_کرو ڈھیر_hop_عقب(func, eax, edx) ({                               \
        خارجی خالی _cfunc16_ ##func (خالی);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
صحیح on_extra_ڈھیر(خالی);
ساخت bregs;
خالی farcall16(ساخت bregs *callregs);
خالی farcall16big(ساخت bregs *callregs);
خالی __call16_int(ساخت bregs *callregs, u16 انحراف_2);
#تعریف_کرو call16_int(nr, callregs) کرو {                           \
        خارجی خالی irq_trampoline_ ##nr (خالی);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } جب (0)
خالی reset(خالی);
خارجی ساخت thread_info MainThread;
ساخت thread_info *getCurThread(خالی);
خالی yield(خالی);
خالی yield_toirq(خالی);
خالی thread_تیاری(خالی);
صحیح دھاگے_during_optionroms(خالی);
خالی تسلسل_thread(خالی (*func)(خالی*), خالی *ڈیٹا);
خالی wait_دھاگے(خالی);
ساخت mutex_s { u32 isLocked; };
خالی mutex_تالا(ساخت mutex_s *mutex);
خالی mutex_unlock(ساخت mutex_s *mutex);
خالی آغاز_preempt(خالی);
خالی اختتام_preempt(خالی);
صحیح wait_preempt(خالی);
خالی check_preempt(خالی);
u32 __call32_params(خالی *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#تعریف_کرو call32_params(func, eax, edx, ecx, errret) ({                   \
        خارجی خالی _cfunc32flat_ ##func (خالی);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ساکن اندرونی صحیح
need_hop_عقب(خالی)
{
    واپس !MODESEGMENT || on_extra_ڈھیر();
}

#اگر_ختم // stacks.h
