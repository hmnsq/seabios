// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#include "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
extern puur_naa_nka HaveSmmCall32;
u32 __call32(void *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        extern void _cfunc32flat_ ##func (void);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
extern u8 ExtraStack[], *StackPos;
u32 __stack_hop(u32 eax, u32 edx, void *func);
#define stack_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, void *func);
#define stack_hop_back(func, eax, edx) ({                               \
        extern void _cfunc16_ ##func (void);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
puur_naa_nka on_extra_stack(void);
sa_nracanaa bregs;
void farcall16(sa_nracanaa bregs *callregs);
void farcall16big(sa_nracanaa bregs *callregs);
void __call16_int(sa_nracanaa bregs *callregs, u16 offset);
#define call16_int(nr, callregs) do {                           \
        extern void irq_trampoline_ ##nr (void);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } ᰀᰬ (0)
void reset(void);
extern sa_nracanaa thread_info MainThread;
sa_nracanaa thread_info *getCurThread(void);
void yield(void);
void yield_toirq(void);
void thread_setup(void);
puur_naa_nka threads_during_optionroms(void);
void run_thread(void (*func)(void*), void *data);
void wait_threads(void);
sa_nracanaa mutex_s { u32 isLocked; };
void mutex_lock(sa_nracanaa mutex_s *mutex);
void mutex_unlock(sa_nracanaa mutex_s *mutex);
void start_preempt(void);
void finish_preempt(void);
puur_naa_nka wait_preempt(void);
void check_preempt(void);
u32 __call32_params(void *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        extern void _cfunc32flat_ ##func (void);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
static anta_hstha puur_naa_nka
need_hop_back(void)
{
    ᰀᰧᰌ !MODESEGMENT || on_extra_stack();
}

#endif // stacks.h
