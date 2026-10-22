// Code for manipulating stack locations.
//
// Copyright (C) 2009-2015  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "bregs.h" // CR0_PE
#अन्तर्भाव "fw/paravirt.h" // PORT_SMI_CMD
#अन्तर्भाव "hw/rtc.h" // rtc_use
#अन्तर्भाव "list.h" // hlist_node
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "stacks.h" // struct mutex_s
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // useRTC

#परिभाषय MAIN_राशिः_MAX (1024*1024)


/****************************************************************
 * 16bit / 32bit calling
 ****************************************************************/

संरचना {
    u8 विधानम्;
    u8 cmosindex;
    u8 a20;
    u16 ss, fs, gs;
    u32 cr0;
    संरचना descloc_s gdt;
} Call16_दत्त VARLOW;

#परिभाषय C16_BIG 1
#परिभाषय C16_SMM 2

पूर्णाङ्क HaveSmmCall32 VARFSEG;

// Backup state in preparation for call32
स्थिर पूर्णाङ्क
call32_prep(u8 विधानम्)
{
    यदि (!CONFIG_CALL32_SMM || विधानम् != C16_SMM) {
        // Backup cr0
        u32 cr0 = cr0_read();
        यदि (cr0 & CR0_PE)
            // Called in 16bit protected mode?!
            प्रत्यागम -1;
        समुच्चयः_नीचः(Call16_दत्त.cr0, cr0);

        // Backup fs/gs and gdt
        समुच्चयः_नीचः(Call16_दत्त.fs, GET_SEG(FS));
        समुच्चयः_नीचः(Call16_दत्त.gs, GET_SEG(GS));
        संरचना descloc_s gdt;
        sgdt(&gdt);
        समुच्चयः_नीचः(Call16_दत्त.gdt.दीर्घता, gdt.दीर्घता);
        समुच्चयः_नीचः(Call16_दत्त.gdt.addr, gdt.addr);

        // Enable a20 and backup its previous state
        समुच्चयः_नीचः(Call16_दत्त.a20, समुच्चयः_a20(1));
    }

    // Backup ss
    समुच्चयः_नीचः(Call16_दत्त.ss, GET_SEG(SS));

    // Backup cmos index register and disable nmi
    u8 cmosindex = inb(पत्तनम्_CMOS_सूचकांक);
    यदि (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex | NMI_DISABLE_BIT, पत्तनम्_CMOS_सूचकांक);
        inb(पत्तनम्_CMOS_दत्त);
    }
    समुच्चयः_नीचः(Call16_दत्त.cmosindex, cmosindex);

    समुच्चयः_नीचः(Call16_दत्त.विधानम्, विधानम्);
    प्रत्यागम 0;
}

// Restore state backed up during call32
स्थिर u8
call32_post(शून्य)
{
    u8 विधानम् = GET_नीचः(Call16_दत्त.विधानम्);
    समुच्चयः_नीचः(Call16_दत्त.विधानम्, 0);
    समुच्चयः_नीचः(Call16_दत्त.ss, 0);

    यदि (!CONFIG_CALL32_SMM || विधानम् != C16_SMM) {
        // Restore a20
        u8 a20 = GET_नीचः(Call16_दत्त.a20);
        यदि (!a20)
            समुच्चयः_a20(0);

        // Restore gdt and fs/gs
        संरचना descloc_s gdt;
        gdt.दीर्घता = GET_नीचः(Call16_दत्त.gdt.दीर्घता);
        gdt.addr = GET_नीचः(Call16_दत्त.gdt.addr);
        lgdt(&gdt);
        समुच्चयः_SEG(FS, GET_नीचः(Call16_दत्त.fs));
        समुच्चयः_SEG(GS, GET_नीचः(Call16_दत्त.gs));

        // Restore cr0
        u32 cr0_caching = GET_नीचः(Call16_दत्त.cr0) & (CR0_CD|CR0_NW);
        यदि (cr0_caching)
            cr0_आवरणम्(CR0_CD|CR0_NW, cr0_caching);
    }

    // Restore cmos index register
    u8 cmosindex = GET_नीचः(Call16_दत्त.cmosindex);
    यदि (!(cmosindex & NMI_DISABLE_BIT)) {
        outb(cmosindex, पत्तनम्_CMOS_सूचकांक);
        inb(पत्तनम्_CMOS_दत्त);
    }
    प्रत्यागम विधानम्;
}

// Force next call16() to restore to a pristine cpu environment state
स्थिर शून्य
call16_override(पूर्णाङ्क big)
{
    ASSERT32FLAT();
    यदि (getesp() > BUILD_राशिः_ADDR)
        panic("call16_override with invalid stack\n");
    स्मृतिं_पूरय(&Call16_दत्त, 0, परिमाण(Call16_दत्त));
    यदि (big) {
        Call16_दत्त.विधानम् = C16_BIG;
        Call16_दत्त.a20 = 1;
    } अन्यथा {
        Call16_दत्त.a20 = !CONFIG_DISABLE_A20;
    }
}

// 16bit handler code called from call16() / call16_smm()
u32 VISIBLE16
call16_helper(u32 eax, u32 edx, u32 (*func)(u32 eax, u32 edx))
{
    u8 विधानम् = call32_post();
    u32 ret = func(eax, edx);
    call32_prep(विधानम्);
    प्रत्यागम ret;
}

#परिभाषय ASM32_SWITCH16 "  .pushsection .text.32fseg." UNIQSEC "\n  .code16\n"
#परिभाषय ASM32_BACK32   "  .popsection\n  .code32\n"
#परिभाषय ASM16_SWITCH32 "  .code32\n"
#परिभाषय ASM16_BACK16   "  .code16gcc\n"

// Call a SeaBIOS C function in 32bit mode using smm trampoline
स्थिर u32
call32_smm(शून्य *func, u32 eax)
{
    ASSERT16();
    dprintf(9, "call32_smm %p %x\n", func, eax);
    call32_prep(C16_SMM);
    u32 bkup_esp;
    asm चञ्चल(
        // Backup esp / set esp to flat stack location
        "  movl %%esp, %0\n"
        "  movl %%ss, %%eax\n"
        "  shll $4, %%eax\n"
        "  addl %%eax, %%esp\n"

        // Transition to 32bit mode, call func, return to 16bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_ENTERID) ", %%ecx\n"
        "  movl $(" __stringify(BUILD_BIOS_ADDR) " + 1f), %%ebx\n"
        "  outb %%al, $" __stringify(पत्तनम्_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        ASM16_SWITCH32
        "1:movl %1, %%eax\n"
        "  calll *%2\n"
        "  movl %%eax, %1\n"

        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $2f, %%ebx\n"
        "  outb %%al, $" __stringify(पत्तनम्_SMI_CMD) "\n"
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
    प्रत्यागम eax;
}

स्थिर u32
call16_smm(u32 eax, u32 edx, शून्य *func)
{
    ASSERT32FLAT();
    यदि (!CONFIG_CALL32_SMM)
        प्रत्यागम eax;
    func -= BUILD_BIOS_ADDR;
    dprintf(9, "call16_smm %p %x %x\n", func, eax, edx);
    u32 stackoffset = Call16_दत्त.ss << 4;
    asm चञ्चल(
        // Restore esp
        "  subl %0, %%esp\n"

        // Transition to 16bit mode, call func, return to 32bit
        "  movl $" __stringify(CALL32SMM_CMDID) ", %%eax\n"
        "  movl $" __stringify(CALL32SMM_RETURNID) ", %%ecx\n"
        "  movl $(1f - " __stringify(BUILD_BIOS_ADDR) "), %%ebx\n"
        "  outb %%al, $" __stringify(पत्तनम्_SMI_CMD) "\n"
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
        "  outb %%al, $" __stringify(पत्तनम्_SMI_CMD) "\n"
        "  rep; nop\n"
        "  hlt\n"

        // Set esp to flat stack location
        ASM32_BACK32
        "2:addl %0, %%esp\n"
        : "+r" (stackoffset), "+r" (eax), "+d" (edx)
        : "r" (func)
        : "eax", "ecx", "ebx", "cc", "memory");
    प्रत्यागम eax;
}

// Call a 32bit SeaBIOS function from a 16bit SeaBIOS function.
u32 VISIBLE16
__call32(शून्य *func, u32 eax, u32 errret)
{
    ASSERT16();
    यदि (CONFIG_CALL32_SMM && GET_GLOBAL(HaveSmmCall32))
        प्रत्यागम call32_smm(func, eax);
    // Jump direclty to 32bit mode - this clobbers the 16bit segment
    // selector registers.
    पूर्णाङ्क ret = call32_prep(C16_BIG);
    यदि (ret)
        प्रत्यागम errret;
    u32 bkup_ss, bkup_esp;
    asm चञ्चल(
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
    प्रत्यागम eax;
}

// Call a 16bit SeaBIOS function, restoring the mode from last call32().
स्थिर u32
call16(u32 eax, u32 edx, शून्य *func)
{
    ASSERT32FLAT();
    यदि (getesp() > MAIN_राशिः_MAX)
        panic("call16 with invalid stack\n");
    यदि (CONFIG_CALL32_SMM && Call16_दत्त.विधानम् == C16_SMM)
        प्रत्यागम call16_smm(eax, edx, func);

    बाह्य शून्य transition16big(शून्य);
    बाह्य शून्य transition16(शून्य);
    शून्य *thunk = transition16;
    यदि (Call16_दत्त.विधानम् == C16_BIG || अन्तर्_post())
        thunk = transition16big;
    func -= BUILD_BIOS_ADDR;
    u32 stackseg = Call16_दत्त.ss;
    asm चञ्चल(
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
    प्रत्यागम eax;
}


/****************************************************************
 * Extra 16bit stack
 ****************************************************************/

// Space for a stack for 16bit code.
u8 Extra_राशिः[BUILD_EXTRA_राशिः_आकार+1] VARLOW __aligned(8);
u8 *राशिः_Pos VARLOW;

// Test if currently on the extra stack
पूर्णाङ्क
on_extra_राशिः(शून्य)
{
    प्रत्यागम MODE16 && GET_SEG(SS) == SEG_नीचः && getesp() > (u32)Extra_राशिः;
}

// Switch to the extra stack and call a function.
u32
__stack_hop(u32 eax, u32 edx, शून्य *func)
{
    यदि (on_extra_राशिः())
        प्रत्यागम ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 राशिः_seg = SEG_नीचः;
    u32 bkup_ss, bkup_esp;
    asm चञ्चल(
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
        : "m" (राशिः_Pos), "r" (राशिः_seg)
        : "cc", "memory");
    प्रत्यागम eax;
}

// Switch back to original caller's stack and call a function.
u32
__stack_hop_back(u32 eax, u32 edx, शून्य *func)
{
    यदि (!MODESEGMENT)
        प्रत्यागम call16(eax, edx, func);
    यदि (!MODE16 || !on_extra_राशिः())
        प्रत्यागम ((u32 (*)(u32, u32))func)(eax, edx);
    ASSERT16();
    u16 bkup_ss;
    u32 bkup_राशिः_pos, अस्थायी;
    asm चञ्चल(
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
          , "=&r" (bkup_राशिः_pos), "=&r" (अस्थायी), "+m" (राशिः_Pos)
        :
        : "cc", "memory");
    प्रत्यागम eax;
}


/****************************************************************
 * External 16bit interface calling
 ****************************************************************/

// Far call 16bit code with a specified register state.
शून्य VISIBLE16
_farcall16(संरचना bregs *callregs, u16 callregseg)
{
    यदि (need_hop_पृष्ठभागः()) {
        राशिः_hop_पृष्ठभागः(_farcall16, callregs, callregseg);
        प्रत्यागम;
    }
    ASSERT16();
    asm चञ्चल(
        "calll __farcall16\n"
        : "+a" (callregs), "+m" (*callregs), "+d" (callregseg)
        :
        : "ebx", "ecx", "esi", "edi", "cc", "memory");
}

// Invoke external 16bit code.
शून्य
farcall16(संरचना bregs *callregs)
{
    call16_override(0);
    _farcall16(callregs, 0);
}

// Invoke external 16bit code in "big real" mode.
शून्य
farcall16big(संरचना bregs *callregs)
{
    call16_override(1);
    _farcall16(callregs, 0);
}

// Invoke a 16bit software interrupt.
शून्य
__call16_int(संरचना bregs *callregs, u16 विस्थापनम्_2)
{
    callregs->संहिता.विस्थापनम्_2 = विस्थापनम्_2;
    यदि (!MODESEGMENT) {
        callregs->संहिता.seg = SEG_BIOS;
        _farcall16((शून्य*)callregs - Call16_दत्त.ss * 16, Call16_दत्त.ss);
        प्रत्यागम;
    }
    callregs->संहिता.seg = GET_SEG(CS);
    _farcall16(callregs, GET_SEG(SS));
}

// Reset the machine
शून्य
reset(शून्य)
{
    बाह्य शून्य reset_सदिशम्(शून्य) __noreturn;
    यदि (!MODE16)
        call16(0, 0, reset_सदिशम्);
    reset_सदिशम्();
}


/****************************************************************
 * Threads
 ****************************************************************/

// Thread info - stored at bottom of each thread stack - don't change
// without also updating the inline assembler below.
संरचना thread_info {
    शून्य *stackpos;
    संरचना hlist_ग्रन्थिः ग्रन्थिः;
};
संरचना thread_info MainThread VARFSEG = {
    रिक्त, { &MainThread.ग्रन्थिः, &MainThread.ग्रन्थिः.अग्र }
};
#परिभाषय THREADSTACKSIZE 4096

// Check if any threads are running.
स्थिर पूर्णाङ्क
have_तन्तवः(शून्य)
{
    प्रत्यागम (CONFIG_तन्तवः
            && GET_FLATPTR(MainThread.ग्रन्थिः.अग्र) != &MainThread.ग्रन्थिः);
}

// Return the 'struct thread_info' for the currently running thread.
संरचना thread_info *
getCurThread(शून्य)
{
    u32 esp = getesp();
    यदि (esp <= MAIN_राशिः_MAX)
        प्रत्यागम &MainThread;
    प्रत्यागम (शून्य*)ALIGN_अधस्तात्(esp, THREADSTACKSIZE);
}

स्थिर u8 Can_व्यवधानम्, ThreadControl;

// Initialize the support for internal threads.
शून्य
thread_सज्जता(शून्य)
{
    Can_व्यवधानम् = 1;
    call16_override(1);
    यदि (! CONFIG_तन्तवः)
        प्रत्यागम;
    ThreadControl = romfile_loadint("etc/threads", 1);
}

// Should hardware initialization threads run during optionrom execution.
पूर्णाङ्क
तन्तवः_during_optionroms(शून्य)
{
    प्रत्यागम CONFIG_तन्तवः && CONFIG_RTC_TIMER && ThreadControl == 2 && अन्तर्_post();
}

// Switch to next thread stack.
स्थिर शून्य
switch_अग्र(संरचना thread_info *cur)
{
    संरचना thread_info *अग्र = container_सम्बन्धि(
        cur->ग्रन्थिः.अग्र, संरचना thread_info, ग्रन्थिः);
    यदि (cur == अग्र)
        // Nothing to do.
        प्रत्यागम;
    asm चञ्चल(
        "  pushl $1f\n"                 // store return pc
        "  pushl %%ebp\n"               // backup %ebp
        "  movl %%esp, (%%eax)\n"       // cur->stackpos = %esp
        "  movl (%%ecx), %%esp\n"       // %esp = next->stackpos
        "  popl %%ebp\n"                // restore %ebp
        "  retl\n"                      // restore pc
        "1:\n"
        : "+a"(cur), "+c"(अग्र)
        :
        : "ebx", "edx", "esi", "edi", "cc", "memory");
}

// Last thing called from a thread (called on MainThread stack).
स्थिर शून्य
__end_thread(संरचना thread_info *old)
{
    hlist_del(&old->ग्रन्थिः);
    dprintf(DEBUG_thread, "\\%08x/ End thread\n", (u32)old);
    मुक्त(old);
    यदि (!have_तन्तवः())
        dprintf(1, "All threads complete.\n");
}

शून्य VISIBLE16 check_irqs(शून्य);

// Create a new thread and start executing 'func' in it.
शून्य
सततम्_thread(शून्य (*func)(शून्य*), शून्य *दत्त)
{
    ASSERT32FLAT();
    यदि (! CONFIG_तन्तवः || ! ThreadControl)
        गच्छ fail;
    संरचना thread_info *thread;
    thread = memalign_tmphigh(THREADSTACKSIZE, THREADSTACKSIZE);
    यदि (!thread)
        गच्छ fail;

    dprintf(DEBUG_thread, "/%08x\\ Start thread\n", (u32)thread);
    thread->stackpos = (शून्य*)thread + THREADSTACKSIZE;
    संरचना thread_info *cur = getCurThread();
    संरचना thread_info *edx = cur;
    hlist_सङ्कलय_after(&thread->ग्रन्थिः, &cur->ग्रन्थिः);
    asm चञ्चल(
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
        : "+a"(दत्त), "+c"(func), "+b"(thread), "+d"(edx)
        : "m"(*(u8*)__end_thread), "m"(MainThread)
        : "esi", "edi", "cc", "memory");
    यदि (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
    प्रत्यागम;

fail:
    func(दत्त);
}


/****************************************************************
 * Thread helpers
 ****************************************************************/

// Low-level irq enable.
शून्य VISIBLE16
check_irqs(शून्य)
{
    यदि (!MODESEGMENT && !Can_व्यवधानम्) {
        // Can't enable interrupts (PIC and/or IVT not yet setup)
        cpu_relax();
        प्रत्यागम;
    }
    यदि (need_hop_पृष्ठभागः()) {
        राशिः_hop_पृष्ठभागः(check_irqs, 0, 0);
        प्रत्यागम;
    }
    यदि (MODE16)
        clock_poll_irq();
    asm चञ्चल("sti ; nop ; rep ; nop ; cli ; cld" : : :"memory");
}

// Briefly permit irqs to occur.
शून्य
yield(शून्य)
{
    यदि (MODESEGMENT || !CONFIG_तन्तवः) {
        check_irqs();
        प्रत्यागम;
    }
    संरचना thread_info *cur = getCurThread();
    // Switch to the next thread
    switch_अग्र(cur);
    यदि (cur == &MainThread)
        // Permit irqs to fire
        check_irqs();
}

शून्य VISIBLE16
wait_irq(शून्य)
{
    यदि (need_hop_पृष्ठभागः()) {
        राशिः_hop_पृष्ठभागः(wait_irq, 0, 0);
        प्रत्यागम;
    }
    asm चञ्चल("sti ; hlt ; cli ; cld": : :"memory");
}

// Wait for next irq to occur.
शून्य
yield_toirq(शून्य)
{
    यदि (!CONFIG_HARDWARE_IRQ
        || (!MODESEGMENT && (have_तन्तवः() || !Can_व्यवधानम्))) {
        // Threads still active or irqs not available - do a yield instead.
        yield();
        प्रत्यागम;
    }
    wait_irq();
}

// Wait for all threads (other than the main thread) to complete.
शून्य
wait_तन्तवः(शून्य)
{
    ASSERT32FLAT();
    यावत् (have_तन्तवः())
        yield();
}

शून्य
mutex_तालकम्(संरचना mutex_s *mutex)
{
    ASSERT32FLAT();
    यदि (! CONFIG_तन्तवः)
        प्रत्यागम;
    यावत् (mutex->isLocked)
        yield();
    mutex->isLocked = 1;
}

शून्य
mutex_unlock(संरचना mutex_s *mutex)
{
    ASSERT32FLAT();
    यदि (! CONFIG_तन्तवः)
        प्रत्यागम;
    mutex->isLocked = 0;
}


/****************************************************************
 * Thread preemption
 ****************************************************************/

पूर्णाङ्क CanPreempt VARFSEG;
स्थिर u32 Preempt_गणनम्;

// Turn on RTC irqs and arrange for them to check the 32bit threads.
शून्य
start_preempt(शून्य)
{
    यदि (! तन्तवः_during_optionroms())
        प्रत्यागम;
    CanPreempt = 1;
    Preempt_गणनम् = 0;
    rtc_use();
}

// Turn off RTC irqs / stop checking for thread execution.
शून्य
finish_preempt(शून्य)
{
    यदि (! तन्तवः_during_optionroms()) {
        yield();
        प्रत्यागम;
    }
    CanPreempt = 0;
    rtc_release();
    dprintf(9, "Done preempt - %d checks\n", Preempt_गणनम्);
    yield();
}

// Check if preemption is on, and wait for it to complete if so.
पूर्णाङ्क
wait_preempt(शून्य)
{
    यदि (MODESEGMENT || !CONFIG_तन्तवः || !CanPreempt
        || getesp() < MAIN_राशिः_MAX)
        प्रत्यागम 0;
    यावत् (CanPreempt)
        yield();
    प्रत्यागम 1;
}

// Try to execute 32bit threads.
शून्य VISIBLE32INIT
yield_preempt(शून्य)
{
    Preempt_गणनम्++;
    switch_अग्र(&MainThread);
}

// 16bit code that checks if threads are pending and executes them if so.
शून्य
check_preempt(शून्य)
{
    यदि (CONFIG_तन्तवः && GET_GLOBAL(CanPreempt) && have_तन्तवः())
        call32(yield_preempt, 0, 0);
}


/****************************************************************
 * call32 helper
 ****************************************************************/

संरचना call32_params_s {
    शून्य *func;
    u32 eax, edx, ecx;
};

u32 VISIBLE32FLAT
call32_params_helper(संरचना call32_params_s *params)
{
    प्रत्यागम ((u32 (*)(u32, u32, u32))params->func)(
        params->eax, params->edx, params->ecx);
}

u32
__call32_params(शून्य *func, u32 eax, u32 edx, u32 ecx, u32 errret)
{
    ASSERT16();
    संरचना call32_params_s params = {func, eax, edx, ecx};
    प्रत्यागम call32(call32_params_helper, MAKE_FLATPTR(GET_SEG(SS), &params)
                  , errret);
}
