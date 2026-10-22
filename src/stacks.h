// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#அந்தர்பாவ "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
வெளி முழு HaveSmmCall32;
u32 __call32(வெறுமை *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        வெளி வெறுமை _cfunc32flat_ ##func (வெறுமை);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
வெளி u8 Extra_அடுக்கு[], *அடுக்கு_Pos;
u32 __stack_hop(u32 eax, u32 edx, வெறுமை *func);
#define அடுக்கு_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, வெறுமை *func);
#define அடுக்கு_hop_back(func, eax, edx) ({                               \
        வெளி வெறுமை _cfunc16_ ##func (வெறுமை);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
முழு on_extra_அடுக்கு(வெறுமை);
கட்டமைப்பு bregs;
வெறுமை farcall16(கட்டமைப்பு bregs *callregs);
வெறுமை farcall16big(கட்டமைப்பு bregs *callregs);
வெறுமை __call16_int(கட்டமைப்பு bregs *callregs, u16 offset);
#define call16_int(nr, callregs) செய் {                           \
        வெளி வெறுமை irq_trampoline_ ##nr (வெறுமை);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } போது (0)
வெறுமை reset(வெறுமை);
வெளி கட்டமைப்பு thread_info MainThread;
கட்டமைப்பு thread_info *getCurThread(வெறுமை);
வெறுமை yield(வெறுமை);
வெறுமை yield_toirq(வெறுமை);
வெறுமை thread_setup(வெறுமை);
முழு threads_during_optionroms(வெறுமை);
வெறுமை run_thread(வெறுமை (*func)(வெறுமை*), வெறுமை *data);
வெறுமை wait_threads(வெறுமை);
கட்டமைப்பு mutex_s { u32 isLocked; };
வெறுமை mutex_lock(கட்டமைப்பு mutex_s *mutex);
வெறுமை mutex_unlock(கட்டமைப்பு mutex_s *mutex);
வெறுமை start_preempt(வெறுமை);
வெறுமை finish_preempt(வெறுமை);
முழு wait_preempt(வெறுமை);
வெறுமை check_preempt(வெறுமை);
u32 __call32_params(வெறுமை *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        வெளி வெறுமை _cfunc32flat_ ##func (வெறுமை);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
நிலையான உள்வரி முழு
need_hop_back(வெறுமை)
{
    திரும்பு !MODESEGMENT || on_extra_அடுக்கு();
}

#endif // stacks.h
