// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#شامل "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
بیرونی صحیح HaveSmmCall32;
u32 __call32(تهی *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        بیرونی تهی _cfunc32flat_ ##func (تهی);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
بیرونی u8 Extra_پشته[], *پشته_Pos;
u32 __stack_hop(u32 eax, u32 edx, تهی *func);
#define پشته_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, تهی *func);
#define پشته_hop_back(func, eax, edx) ({                               \
        بیرونی تهی _cfunc16_ ##func (تهی);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
صحیح on_extra_پشته(تهی);
ساختار bregs;
تهی farcall16(ساختار bregs *callregs);
تهی farcall16big(ساختار bregs *callregs);
تهی __call16_int(ساختار bregs *callregs, u16 offset);
#define call16_int(nr, callregs) انجامبده {                           \
        بیرونی تهی irq_trampoline_ ##nr (تهی);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } وقتی (0)
تهی reset(تهی);
بیرونی ساختار thread_info MainThread;
ساختار thread_info *getCurThread(تهی);
تهی yield(تهی);
تهی yield_toirq(تهی);
تهی thread_setup(تهی);
صحیح threads_during_optionroms(تهی);
تهی run_thread(تهی (*func)(تهی*), تهی *data);
تهی wait_threads(تهی);
ساختار mutex_s { u32 isLocked; };
تهی mutex_lock(ساختار mutex_s *mutex);
تهی mutex_unlock(ساختار mutex_s *mutex);
تهی start_preempt(تهی);
تهی finish_preempt(تهی);
صحیح wait_preempt(تهی);
تهی check_preempt(تهی);
u32 __call32_params(تهی *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        بیرونی تهی _cfunc32flat_ ##func (تهی);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
ایستا درونخطی صحیح
need_hop_back(تهی)
{
    بازگشت !MODESEGMENT || on_extra_پشته();
}

#endif // stacks.h
