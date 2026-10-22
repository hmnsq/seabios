// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "bregs.h" // CR0_PE
#అన్తర్భావ "fw/paravirt.h" // PORT_SMI_CMD
#అన్తర్భావ "hw/rtc.h" // rtc_use
#అన్తర్భావ "list.h" // hlist_node
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "stacks.h" // struct mutex_s
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // useRTC

#నిర్వచించు MAIN_దొంతర_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

నిర్మాణం {
    u8 పద్ధతి;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    నిర్మాణం descloc_s gdt;
} Call16_దత్తాంశం VARLOW;

#నిర్వచించు C16_BIG 1
#నిర్వచించు C16_SMM 2

పూర్ణాంకం HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
స్థిర పూర్ణాంకం
call32_prep(u8 పద్ధతి)
{
    ఒకవేళ (!CONFIG_CALL32_SMM || పద్ధతి != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        ఒకవేళ (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            తిరిగి -1;
        సమితి_దిగువ(Call16_దత్తాంశం.cr0, cr0);

        // Backup fs/gs and gdt
        సమితి_దిగువ(Call16_దత్తాంశం.fs, GET_SEG(FS));
        సమితి_దిగువ(Call16_దత్తాంశం.gs, GET_SEG(GS));
        నిర్మాణం descloc_s gdt;
        sgdt(&gdt);
        సమితి_దిగువ(Call16_దత్తాంశం.gdt.నిడివి, gdt.నిడివి);
        సమితి_దిగువ(Call16_దత్తాంశం.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        సమితి_దిగువ(Call16_దత్తాంశం.a20, సమితి_a20(1));
    }

    // Backup ss
    సమితి_దిగువ(Call16_దత్తాంశం.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(పోర్టు_CMOS_సూచీ);
    ఒకవేళ (!(cmosindex & NMI_DISABLE_బిట్టు)) {
        outb(cmosindex | NMI_DISABLE_బిట్టు, పోర్టు_CMOS_సూచీ);
        inb(పోర్టు_CMOS_దత్తాంశం);
    }
    సమితి_దిగువ(Call16_దత్తాంశం.cmosindex, cmosindex);

    సమితి_దిగువ(Call16_దత్తాంశం.పద్ధతి, పద్ధతి);
    తిరిగి 0;
}

// Restore state backed up during call32
స్థిర u8
call32_post(శూన్య)
{
    u8 పద్ధతి = GET_దిగువ(Call16_దత్తాంశం.పద్ధతి);
    సమితి_దిగువ(Call16_దత్తాంశం.పద్ధతి, 0);
    సమితి_దిగువ(Call16_దత్తాంశం.ss, 0);

    ఒకవేళ (!CONFIG_CALL32_SMM || పద్ధతి != C16_SMM) {
        // Restore a20
        u8 a20 = GET_దిగువ(Call16_దత్తాంశం.a20);
        ఒకవేళ (!a20)
            సమితి_a20(0);

        // Restore gdt and fs/gs
        నిర్మాణం descloc_s gdt;
        gdt.నిడివి = GET_దిగువ(Call16_దత్తాంశం.gdt.నిడివి);
        gdt.addr = GET_దిగువ(Call16_దత్తాంశం.gdt.addr);
        lgdt(&gdt);
        సమితి_SEG(FS, GET_దిగువ(Call16_దత్తాంశం.fs));
        సమితి_SEG(GS, GET_దిగువ(Call16_దత్తాంశం.gs));

        // Restore cr0
        u32 cr0_caching = GET_దిగువ(Call16_దత్తాంశం.cr0) & (CR0_CD|CR0_NW);
        ఒకవేళ (cr0_caching)
            cr0_ముసుగు(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_దిగువ(Call16_దత్తాంశం.cmosindex);
    ఒకవేళ (!(cmosindex & NMI_DISABLE_బిట్టు)) {
        outb(cmosindex, పోర్టు_CMOS_సూచీ);
        inb(పోర్టు_CMOS_దత్తాంశం);
    }
    తిరిగి పద్ధతి;
}

// Force next call16() to restore to a pristine cpu environment state
స్థిర శూన్య
call16_override(పూర్ణాంకం big)
{
    ASSERT32FLAT();
    ఒకవేళ (getesp() > BUILD_దొంతర_ADDR)
        panic("call16_override with invalid stack\n");
    memset(&Call16_దత్తాంశం, 0, పరిమాణం(Call16_దత్తాంశం));
    ఒకవేళ (big) {
        Call16_దత్తాంశం.పద్ధతి = C16_BIG;
        Call16_దత్తాంశం.a20 = 1;
    } లేకపోతే {
        Call16_దత్తాంశం.a20 = !CONFIG_DISABLE_A20;
    }
}

// 16bit handler code called from call16() / call16_smm()
u32 VISIBLE16
call16_helper(u32 eax, u32 edx, u32 (*func)(u32 eax, u32 edx))
{
    u8 పద్ధతి = call32_post();
    u32 ret = func(eax, edx);
    call32_prep(పద్ధతి);
    తిరిగి ret;
}

#నిర్వచించు ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#నిర్వచించు ASM32_BACK32   "  .popsection\n  .code32\n"
#నిర్వచించు ASM16_SWITCH32 "  .code32\n"
#నిర్వచించు ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
స్థిర u32
call32_smm(శూన్య *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm అస్థిర(
        // Backup esp / set esp to flat stack location
        "  movl %%esp, %0\n"
        "  movl %%ss, %%eax\n"
        "  shll $4, %%eax\n"
        "  addl %%eax, %%esp\n"

        // Transition to 32bit mode, call func, return to 16bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_ENTERID) ", %%ecx\n"
        "  movl $(" __stringify(BUILD_BIOS_ADDR) " + 1f), %%ebx\n"
        "  outb %%al, $" __stringify(పోర్టు_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        ASM16_SWITCH32
        "1:movl %1, %%eax\n"
        "  calll *%2\n"
        "  movl %%eax, %1\n"

        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $2f, %%ebx\n"
        "  outb %%al, $" __stringify(పోర్టు_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        // Restore esp
        ASM16_BACK16
        "2:movl %0, %%esp\n"
        : "=&r" (bkup_esp), "+r" (eax)
        : "r" (func)
        : "eax", "ecx", "edx", "ebx", "cc", "memory");
    call32_post();

    dprintf(9, "call32_smm done %p %x\n", func, eax);
    తిరిగి eax;
}

స్థిర u32
call16_smm(u32 eax, u32 edx, శూన్య *func)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_CALL32_SMM)
        తిరిగి eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16_దత్తాంశం.ss << 4;
    asm అస్థిర(
        // Restore esp
        "  subl %0, %%esp\n"

        // Transition to 16bit mode, call func, return to 32bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $(1f - " __stringify(BUILD_BIOS_ADDR) "), %%ebx\n"
        "  outb %%al, $" __stringify(పోర్టు_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        ASM32_SWITCH16
        "1:movl %1, %%eax\n"
        "  movl %3, %%ecx\n"
        "  calll _cfunc16_call16_helper\n"
        "  movl %%eax, %1\n"

        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_ENTERID) ", %%ecx\n"
        "  movl $2f, %%ebx\n"
        "  outb %%al, $" __stringify(పోర్టు_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        // Set esp to flat stack location
        ASM32_BACK32
        "2:addl %0, %%esp\n"
        : "+r" (stackoffset), "+r" (eax), "+d" (edx)
        : "r" (func)
        : "eax", "ecx", "ebx", "cc", "memory");
    తిరిగి eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(శూన్య *func, u32 eax, u32 errret)
{
    ASSERT16();
    ఒకవేళ (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        తిరిగి call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    పూర్ణాంకం ret = call32_prep(C16_BIG);
    ఒకవేళ (ret)
        తిరిగి errret;
    u32 bkup_ss, bkup_esp;
    asm అస్థిర(
        // Backup ss/esp / set esp to flat stack location
        "  movl %%ss, %0\n"
        "  movl %%esp, %1\n"
        "  shll $4, %0\n"
        "  addl %0, %%esp\n"
        "  shrl $4, %0\n"

        // Transition to 32bit mode, call func, return to 16bit
        "  movl $(" __stringify(BUILD_BIOS_ADDR) " + 1f), %%edx\n"
        "  jmp transition32_nmi_off\n"
        ASM16_SWITCH32
        "1:calll *%3\n"
        "  movl $2f, %%edx\n"
        "  jmp transition16big\n"

        // Restore ds/ss/esp
        ASM16_BACK16
        "2:movl %0, %%ds\n"
        "  movl %0, %%ss\n"
        "  movl %1, %%esp\n"
        : "=&r" (bkup_ss), "=&r" (bkup_esp), "+a" (eax)
        : "r" (func)
        : "ecx", "edx", "cc", "memory");
    call32_post();
    తిరిగి eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
స్థిర u32
call16(u32 eax, u32 edx, శూన్య *func)
{
    ASSERT32FLAT();
    ఒకవేళ (getesp() > MAIN_దొంతర_MAX)
        panic("call16 with invalid stack\n");
    ఒకవేళ (CONFIG_CALL32_SMM && Call16_దత్తాంశం.పద్ధతి == C16_SMM)
        తిరిగి call16_smm(eax, edx, func);

    బాహ్య శూన్య transition16big(శూన్య);
    బాహ్య శూన్య transition16(శూన్య);
    శూన్య *thunk = transition16;
    ఒకవేళ (Call16_దత్తాంశం.పద్ధతి == C16_BIG || లోపల_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16_దత్తాంశం.ss;
    asm అస్థిర(
        // Transition to 16bit mode
        "  movl $(1f - " __stringify(BUILD_BIOS_ADDR) "), %%edx\n"
        "  jmp *%%ecx\n"
        // Setup ss/esp and call func
        ASM32_SWITCH16
        "1:movl %2, %%ecx\n"
        "  shll $4, %2\n"
        "  movw %%cx, %%ss\n"
        "  subl %2, %%esp\n"
        "  movw %%cx, %%ds\n"
        "  movl %4, %%edx\n"
        "  movl %3, %%ecx\n"
        "  calll _cfunc16_call16_helper\n"
        // Return to 32bit and restore esp
        "  movl $2f, %%edx\n"
        "  jmp transition32_nmi_off\n"
        ASM32_BACK32
        "2:addl %2, %%esp\n"
        : "+a" (eax), "+c"(thunk), "+r"(stackseg)
        : "r" (func), "r" (edx)
        : "edx", "cc", "memory");
    తిరిగి eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_దొంతర[BUILD_EXTRA_దొంతర_కొలత+1] VARLOW __aligned(8);
u8 *దొంతర_Pos VARLOW;

// Test if currently on the extra stack
పూర్ణాంకం
on_extra_దొంతర(శూన్య)
{
    తిరిగి MODE16 && GET_SEG(SS) == SEG_దిగువ && getesp() > (u32)Extra_దొంతర;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, శూన్య *func)
{
    ఒకవేళ (on_extra_దొంతర())
        తిరిగి ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 దొంతర_seg = SEG_దిగువ;
    u32 bkup_ss, bkup_esp;
    asm అస్థిర(
        // Backup current %ss/%esp values.
        "movw %%ss, %w3\n"
        "movl %%esp, %4\n"
        // Copy stack seg to %ds/%ss and set %esp
        "movw %w6, %%ds\n"
        "movw %w6, %%ss\n"
        "movl %5, %%esp\n"
        "pushl %3\n"
        "pushl %4\n"
        // Call func
        "calll *%2\n"
        "popl %4\n"
        "popl %3\n"
        // Restore segments and stack
        "movw %w3, %%ds\n"
        "movw %w3, %%ss\n"
        "movl %4, %%esp"
        : "+a" (eax), "+d" (edx), "+c" (func), "=&r" (bkup_ss), "=&r" (bkup_esp)
        : "m" (దొంతర_Pos), "r" (దొంతర_seg)
        : "cc", "memory");
    తిరిగి eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, శూన్య *func)
{
    ఒకవేళ (!MODESEGMENT)
        తిరిగి call16(eax, edx, func);
    ఒకవేళ (!MODE16 || !on_extra_దొంతర())
        తిరిగి ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_దొంతర_pos, తాత్కాలిక;
    asm అస్థిర(
        // Backup stack_pos and current %ss/%esp
        "movl %6, %4\n"
        "movw %%ss, %w3\n"
        "movl %%esp, %6\n"
        // Restore original callers' %ss/%esp
        "movl -4(%4), %5\n"
        "movl %5, %%ss\n"
        "movw %%ds:-8(%4), %%sp\n"
        "movl %5, %%ds\n"
        // Call func
        "calll *%2\n"
        // Restore %ss/%esp and stack_pos
        "movw %w3, %%ds\n"
        "movw %w3, %%ss\n"
        "movl %6, %%esp\n"
        "movl %4, %6"
        : "+a" (eax), "+d" (edx), "+c" (func), "=&r" (bkup_ss)
          , "=&r" (bkup_దొంతర_pos), "=&r" (తాత్కాలిక), "+m" (దొంతర_Pos)
        :
        : "cc", "memory");
    తిరిగి eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
శూన్య VISIBLE16
_farcall16(నిర్మాణం bregs *callregs, u16 callregseg)
{
    ఒకవేళ (need_hop_వెనుక()) {
        దొంతర_hop_వెనుక(_farcall16, callregs, callregseg);
        తిరిగి;
    }
    ASSERT16();
    asm అస్థిర(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
శూన్య
farcall16(నిర్మాణం bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
శూన్య
farcall16big(నిర్మాణం bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
శూన్య
__call16_int(నిర్మాణం bregs *callregs, u16 స్థానభ్రంశం_2)
{
    callregs->సంకేతం_2.స్థానభ్రంశం_2 = స్థానభ్రంశం_2;
    ఒకవేళ (!MODESEGMENT) {
        callregs->సంకేతం_2.seg = SEG_BIOS;
        _farcall16((శూన్య*)callregs - Call16_దత్తాంశం.ss * 16, Call16_దత్తాంశం.ss);
        తిరిగి;
    }
    callregs->సంకేతం_2.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
శూన్య
reset(శూన్య)
{
    బాహ్య శూన్య reset_సదిశ(శూన్య) __noreturn;
    ఒకవేళ (!MODE16)
        call16(0, 0, reset_సదిశ);
    reset_సదిశ();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
నిర్మాణం thread_info {
    శూన్య *stackpos;
    నిర్మాణం hlist_కణుపు కణుపు;
};
నిర్మాణం thread_info MainThread VARFSEG = {
    ఖాళీ, { &MainThread.కణుపు, &MainThread.కణుపు.తదుపరి }
};
#నిర్వచించు THREADSTACKSIZE 4096

// Check if any threads are running.
స్థిర పూర్ణాంకం
have_దారాలు(శూన్య)
{
    తిరిగి (CONFIG_దారాలు
            && GET_FLATPTR(MainThread.కణుపు.తదుపరి) != &MainThread.కణుపు);
}

// Return the 'struct thread_info' for the currently running thread.
నిర్మాణం thread_info *
getCurThread(శూన్య)
{
    u32 esp = getesp();
    ఒకవేళ (esp <= MAIN_దొంతర_MAX)
        తిరిగి &MainThread;
    తిరిగి (శూన్య*)ALIGN_కిందికి(esp, THREADSTACKSIZE);
}

స్థిర u8 Can_అంతరాయం, ThreadControl;

// Initialize the support for internal threads.
శూన్య
thread_అమరిక(శూన్య)
{
    Can_అంతరాయం = 1;
    call16_override(1);
    ఒకవేళ (! CONFIG_దారాలు)
        తిరిగి;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
పూర్ణాంకం
దారాలు_during_optionroms(శూన్య)
{
    తిరిగి CONFIG_దారాలు && CONFIG_RTC_TIMER && ThreadControl == 2 && లోపల_post();
}

// Switch to next thread stack.
స్థిర శూన్య
switch_తదుపరి(నిర్మాణం thread_info *cur)
{
    నిర్మాణం thread_info *తదుపరి = container_యొక్క(
        cur->కణుపు.తదుపరి, నిర్మాణం thread_info, కణుపు);
    ఒకవేళ (cur == తదుపరి)
        // Nothing to do.
        తిరిగి;
    asm అస్థిర(
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%eax)\n"       // cur->stackpos = %esp
        "  movl (%%ecx), %%esp\n"       // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(cur), "+c"(తదుపరి)
        :
        : "ebx", "edx", "esi", "edi", "cc", "memory");
}

// Last thing called from a thread (called on MainThread stack).
స్థిర శూన్య
__end_thread(నిర్మాణం thread_info *old)
{
    hlist_del(&old->కణుపు);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    విడుదల(old);
    ఒకవేళ (!have_దారాలు())
        dprintf(1, "All threads complete.\n");
}

శూన్య VISIBLE16 check_irqs(శూన్య);

// Create a new thread and start executing 'func' in it.
శూన్య
నిరంతరం_thread(శూన్య (*func)(శూన్య*), శూన్య *దత్తాంశం)
{
    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_దారాలు || ! ThreadControl)
        వెళ్ళు fail;
    నిర్మాణం thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    ఒకవేళ (!thread)
        వెళ్ళు fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (శూన్య*)thread + THREADSTACKSIZE;
    నిర్మాణం thread_info *cur = getCurThread();
    నిర్మాణం thread_info *edx = cur;
    hlist_కూడు_after(&thread->కణుపు, &cur->కణుపు);
    asm అస్థిర(
        // Start thread
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%edx)\n"       // cur->stackpos = %esp
        "  movl (%%ebx), %%esp\n"       // %esp = thread->stackpos
        "  calll *%%ecx\n"              // Call func

        // End thread
        "  movl %%ebx, %%eax\n"         // %eax = thread
        "  movl 4(%%ebx), %%ebx\n"      // %ebx = thread->node.next
        "  movl (%5), %%esp\n"          // %esp = MainThread.stackpos
        "  calll %4\n"                  // call __end_thread(thread)
        "  movl -4(%%ebx), %%esp\n"     // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(దత్తాంశం), "+c"(func), "+b"(thread), "+d"(edx)
        : "m"(*(u8*)__end_thread), "m"(MainThread)
        : "esi", "edi", "cc", "memory");
    ఒకవేళ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    తిరిగి;

fail:
    func(దత్తాంశం);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
శూన్య VISIBLE16
check_irqs(శూన్య)
{
    ఒకవేళ (!MODESEGMENT && !Can_అంతరాయం) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        తిరిగి;
    }
    ఒకవేళ (need_hop_వెనుక()) {
        దొంతర_hop_వెనుక(check_irqs, 0, 0);
        తిరిగి;
    }
    ఒకవేళ (MODE16)
        clock_poll_irq();
    asm అస్థిర("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
శూన్య
yield(శూన్య)
{
    ఒకవేళ (MODESEGMENT || !CONFIG_దారాలు) {
        check_irqs();
        తిరిగి;
    }
    నిర్మాణం thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_తదుపరి(cur);
    ఒకవేళ (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

శూన్య VISIBLE16
wait_irq(శూన్య)
{
    ఒకవేళ (need_hop_వెనుక()) {
        దొంతర_hop_వెనుక(wait_irq, 0, 0);
        తిరిగి;
    }
    asm అస్థిర("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
శూన్య
yield_toirq(శూన్య)
{
    ఒకవేళ (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_దారాలు() || !Can_అంతరాయం))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        తిరిగి;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
శూన్య
wait_దారాలు(శూన్య)
{
    ASSERT32FLAT();
    అయితే (have_దారాలు())
        yield();
}

శూన్య
mutex_తాళం(నిర్మాణం mutex_s *mutex)
{
    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_దారాలు)
        తిరిగి;
    అయితే (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

శూన్య
mutex_unlock(నిర్మాణం mutex_s *mutex)
{
    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_దారాలు)
        తిరిగి;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

పూర్ణాంకం CanPreempt VARFSEG;
స్థిర u32 Preempt_లెక్క;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
శూన్య
ప్రారంభం_preempt(శూన్య)
{
    ఒకవేళ (! దారాలు_during_optionroms())
        తిరిగి;
    CanPreempt = 1;
    Preempt_లెక్క = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
శూన్య
ముగింపు_preempt(శూన్య)
{
    ఒకవేళ (! దారాలు_during_optionroms()) {
        yield();
        తిరిగి;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_లెక్క);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
పూర్ణాంకం
wait_preempt(శూన్య)
{
    ఒకవేళ (MODESEGMENT || !CONFIG_దారాలు || !CanPreempt
        || getesp() < MAIN_దొంతర_MAX)
        తిరిగి 0;
    అయితే (CanPreempt)
        yield();
    తిరిగి 1;
}

// Try to execute 32bit threads.
శూన్య VISIBLE32INIT
yield_preempt(శూన్య)
{
    Preempt_లెక్క++;
    switch_తదుపరి(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
శూన్య
check_preempt(శూన్య)
{
    ఒకవేళ (CONFIG_దారాలు && GET_GLOBAL(CanPreempt) && have_దారాలు())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

నిర్మాణం call32_params_s {
    శూన్య *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(నిర్మాణం call32_params_s *params)
{
    తిరిగి ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(శూన్య *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    నిర్మాణం call32_params_s params = {func, eax, edx, ecx};
    తిరిగి call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
