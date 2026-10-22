// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#ضمن "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
خارجي صحيح HaveSmmCall32;
u32 __call32(فراغ *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        خارجي فراغ _cfunc32flat_ ##func (فراغ);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
خارجي u8 ExtraStack[], *StackPos;
u32 __stack_hop(u32 eax, u32 edx, فراغ *func);
#define stack_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, فراغ *func);
#define stack_hop_back(func, eax, edx) ({                               \
        خارجي فراغ _cfunc16_ ##func (فراغ);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
صحيح on_extra_stack(فراغ);
بنية bregs;
فراغ farcall16(بنية bregs *callregs);
فراغ farcall16big(بنية bregs *callregs);
فراغ __call16_int(بنية bregs *callregs, u16 offset);
#define call16_int(nr, callregs) עשה {                           \
        خارجي فراغ irq_trampoline_ ##nr (فراغ);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } כלעוד (0)
فراغ reset(فراغ);
خارجي بنية thread_info MainThread;
بنية thread_info *getCurThread(فراغ);
فراغ yield(فراغ);
فراغ yield_toirq(فراغ);
فراغ thread_setup(فراغ);
صحيح threads_during_optionroms(فراغ);
فراغ run_thread(فراغ (*func)(فراغ*), فراغ *data);
فراغ wait_threads(فراغ);
بنية mutex_s { u32 isLocked; };
فراغ mutex_lock(بنية mutex_s *mutex);
فراغ mutex_unlock(بنية mutex_s *mutex);
فراغ start_preempt(فراغ);
فراغ finish_preempt(فراغ);
صحيح wait_preempt(فراغ);
فراغ check_preempt(فراغ);
u32 __call32_params(فراغ *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        خارجي فراغ _cfunc32flat_ ##func (فراغ);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ساكن مضمن صحيح
need_hop_back(فراغ)
{
    החזר !MODESEGMENT || on_extra_stack();
}

#endif // stacks.h
