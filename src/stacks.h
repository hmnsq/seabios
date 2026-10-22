// Misc function and variable declarations.
#अगर_अघोषित __STACKS_H
#घोषणा __STACKS_H

#समावेश "types.h" // u32

#घोषणा CALL32SMM_CMDID    0xb5
#घोषणा CALL32SMM_ENTERID  0x1234
#घोषणा CALL32SMM_RETURNID 0x5678

// stacks.c
बाह्य पूर्णांक HaveSmmCall32;
u32 __call32(व्योम *func, u32 eax, u32 errret);
#घोषणा call32(func, eax, errret) ({                            \
        बाह्य व्योम _cfunc32flat_ ##func (व्योम);                \
        __call32( _cfunc32flat_ ##func , (u32)(eax), (errret)); \
    })
बाह्य u8 Extra_ढेर[], *ढेर_Pos;
u32 __stack_hop(u32 eax, u32 edx, व्योम *func);
#घोषणा ढेर_hop(func, eax, edx)               \
    __stack_hop((u32)(eax), (u32)(edx), (func))
u32 __stack_hop_back(u32 eax, u32 edx, व्योम *func);
#घोषणा ढेर_hop_back(func, eax, edx) ({                               \
        बाह्य व्योम _cfunc16_ ##func (व्योम);                            \
        __stack_hop_back((u32)(eax), (u32)(edx), _cfunc16_ ##func );    \
    })
पूर्णांक on_extra_ढेर(व्योम);
काष्ठा bregs;
व्योम farcall16(काष्ठा bregs *callregs);
व्योम farcall16big(काष्ठा bregs *callregs);
व्योम __call16_int(काष्ठा bregs *callregs, u16 offset);
#घोषणा call16_int(nr, callregs) करो {                           \
        बाह्य व्योम irq_trampoline_ ##nr (व्योम);                \
        __call16_int((callregs), (u32)&irq_trampoline_ ##nr );  \
    } जबतक (0)
व्योम reset(व्योम);
बाह्य काष्ठा thread_info MainThread;
काष्ठा thread_info *getCurThread(व्योम);
व्योम yield(व्योम);
व्योम yield_toirq(व्योम);
व्योम thread_setup(व्योम);
पूर्णांक threads_during_optionroms(व्योम);
व्योम run_thread(व्योम (*func)(व्योम*), व्योम *तथ्य);
व्योम wait_threads(व्योम);
काष्ठा mutex_s { u32 isLocked; };
व्योम mutex_lock(काष्ठा mutex_s *mutex);
व्योम mutex_unlock(काष्ठा mutex_s *mutex);
व्योम start_preempt(व्योम);
व्योम finish_preempt(व्योम);
पूर्णांक wait_preempt(व्योम);
व्योम check_preempt(व्योम);
u32 __call32_params(व्योम *func, u32 eax, u32 edx, u32 ecx, u32 errret);
#घोषणा call32_params(func, eax, edx, ecx, errret) ({                   \
        बाह्य व्योम _cfunc32flat_ ##func (व्योम);                        \
        __call32_params( _cfunc32flat_ ##func , (u32)(eax), (u32)(edx)  \
                        , (u32)(ecx), (errret));                        \
    })

// Inline functions

// Check if a call to stack_hop_back is needed.
अटल अंतरभूत पूर्णांक
need_hop_back(व्योम)
{
    वापस !MODESEGMENT || on_extra_ढेर();
}

#पूर्ण_अगर // stacks.h
