// Misc function and variable declarations.
#यदि_अपरिभाषितम् __STACKS_H
#परिभाषय __STACKS_H

#अन्तर्भाव "types.h" // u32

#परिभाषय CALL32SMM_CMDID    0xb5
#परिभाषय CALL32SMM_ENTERID  0x1234
#परिभाषय CALL32SMM_RETURNID 0x5678

// stacks.c
बाह्य पूर्णाङ्क HaveSmmCall32;
u32 __call32(शून्य *func, u32 eax, u32 errret);
#परिभाषय call32(func, eax, errret) ({                            \
        बाह्य शून्य _cfunc32flat_ ##func (शून्य);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
बाह्य u8 Extra_राशिः[], *राशिः_Pos;
u32 __stack_hop(u32 eax, u32 edx, शून्य *func);
#परिभाषय राशिः_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, शून्य *func);
#परिभाषय राशिः_hop_पृष्ठभागः(func, eax, edx) ({                               \
        बाह्य शून्य _cfunc16_ ##func (शून्य);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
पूर्णाङ्क on_extra_राशिः(शून्य);
संरचना bregs;
शून्य farcall16(संरचना bregs *callregs);
शून्य farcall16big(संरचना bregs *callregs);
शून्य __call16_int(संरचना bregs *callregs, u16 विस्थापनम्_2);
#परिभाषय call16_int(nr, callregs) कुरु {                           \
        बाह्य शून्य irq_trampoline_ ##nr (शून्य);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } यावत् (0)
शून्य reset(शून्य);
बाह्य संरचना thread_info MainThread;
संरचना thread_info *getCurThread(शून्य);
शून्य yield(शून्य);
शून्य yield_toirq(शून्य);
शून्य thread_सज्जता(शून्य);
पूर्णाङ्क तन्तवः_during_optionroms(शून्य);
शून्य सततम्_thread(शून्य (*func)(शून्य*), शून्य *दत्त);
शून्य wait_तन्तवः(शून्य);
संरचना mutex_s { u32 isLocked; };
शून्य mutex_तालकम्(संरचना mutex_s *mutex);
शून्य mutex_unlock(संरचना mutex_s *mutex);
शून्य start_preempt(शून्य);
शून्य finish_preempt(शून्य);
पूर्णाङ्क wait_preempt(शून्य);
शून्य check_preempt(शून्य);
u32 __call32_params(शून्य *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#परिभाषय call32_params(func, eax, edx, ecx, errret) ({                   \
        बाह्य शून्य _cfunc32flat_ ##func (शून्य);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
स्थिर अन्तःस्थ पूर्णाङ्क
need_hop_पृष्ठभागः(शून्य)
{
    प्रत्यागम !MODESEGMENT || on_extra_राशिः();
}

#यदि_अन्तः // stacks.h
