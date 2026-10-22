// Misc function and variable declarations.
#ifndef __STACKS_H
#define __STACKS_H

#अन्तर्भाव "types.h" // u32

#define CALL32SMM_CMDID    0xb5
#define CALL32SMM_ENTERID  0x1234
#define CALL32SMM_RETURNID 0x5678

// stacks.c
बाह्य पूर्णांक HaveSmmCall32;
u32 __call32(शून्य *func, u32 eax, u32 errret);
#define call32(func, eax, errret) ({                            \
        बाह्य शून्य _cfunc32flat_ ##func (शून्य);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
बाह्य u8 Extra_चळत[], *चळत_Pos;
u32 __stack_hop(u32 eax, u32 edx, शून्य *func);
#define चळत_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, शून्य *func);
#define चळत_hop_back(func, eax, edx) ({                               \
        बाह्य शून्य _cfunc16_ ##func (शून्य);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
पूर्णांक on_extra_चळत(शून्य);
संरचना bregs;
शून्य farcall16(संरचना bregs *callregs);
शून्य farcall16big(संरचना bregs *callregs);
शून्य __call16_int(संरचना bregs *callregs, u16 offset);
#define call16_int(nr, callregs) करा {                           \
        बाह्य शून्य irq_trampoline_ ##nr (शून्य);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } पर्यंत (0)
शून्य reset(शून्य);
बाह्य संरचना thread_info MainThread;
संरचना thread_info *getCurThread(शून्य);
शून्य yield(शून्य);
शून्य yield_toirq(शून्य);
शून्य thread_setup(शून्य);
पूर्णांक threads_during_optionroms(शून्य);
शून्य run_thread(शून्य (*func)(शून्य*), शून्य *data);
शून्य wait_threads(शून्य);
संरचना mutex_s { u32 isLocked; };
शून्य mutex_lock(संरचना mutex_s *mutex);
शून्य mutex_unlock(संरचना mutex_s *mutex);
शून्य start_preempt(शून्य);
शून्य finish_preempt(शून्य);
पूर्णांक wait_preempt(शून्य);
शून्य check_preempt(शून्य);
u32 __call32_params(शून्य *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#define call32_params(func, eax, edx, ecx, errret) ({                   \
        बाह्य शून्य _cfunc32flat_ ##func (शून्य);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
स्थिर ओळीत पूर्णांक
need_hop_back(शून्य)
{
    परत !MODESEGMENT || on_extra_चळत();
}

#endif // stacks.h
