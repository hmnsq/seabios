// Basic x86 asm functions.
#నిర్వచితం_కాకపోతే __X86_H
#నిర్వచించు __X86_H

// CPU flag bitdefs
#నిర్వచించు F_CF (1<<0)
#నిర్వచించు F_ZF (1<<6)
#నిర్వచించు F_IF (1<<9)
#నిర్వచించు F_ID (1<<21)

// CR0 flags
#నిర్వచించు CR0_PG (1<<31) // Paging
#నిర్వచించు CR0_CD (1<<30) // Cache disable
#నిర్వచించు CR0_NW (1<<29) // Not Write-through
#నిర్వచించు CR0_PE (1<<0)  // Protection enable

// PORT_A20 bitdefs
#నిర్వచించు పోర్టు_A20 0x0092
#నిర్వచించు A20_ENABLE_బిట్టు 0x02

#నిర్వచితం_కాకపోతే __ASSEMBLY__

#అన్తర్భావ "types.h" // u32

స్థిర అంతఃవరుస శూన్య irq_disable(శూన్య)
{
    asm అస్థిర("cli": : :"memory");
}

స్థిర అంతఃవరుస శూన్య irq_enable(శూన్య)
{
    asm అస్థిర("sti": : :"memory");
}

స్థిర అంతఃవరుస u32 save_flags(శూన్య)
{
    u32 flags;
    asm అస్థిర("pushfl ; popl %0" : "=rm" (flags));
    తిరిగి flags;
}

స్థిర అంతఃవరుస శూన్య restore_flags(u32 flags)
{
    asm అస్థిర("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

స్థిర అంతఃవరుస శూన్య cpu_relax(శూన్య)
{
    asm అస్థిర("rep ; nop": : :"memory");
}

స్థిర అంతఃవరుస శూన్య nop(శూన్య)
{
    asm అస్థిర("nop");
}

స్థిర అంతఃవరుస శూన్య hlt(శూన్య)
{
    asm అస్థిర("hlt": : :"memory");
}

స్థిర అంతఃవరుస శూన్య wbinvd(శూన్య)
{
    asm అస్థిర("wbinvd": : :"memory");
}

#నిర్వచించు CPUID_TSC (1 << 4)
#నిర్వచించు CPUID_MSR (1 << 5)
#నిర్వచించు CPUID_APIC (1 << 9)
#నిర్వచించు CPUID_MTRR (1 << 12)
#నిర్వచించు CPUID_X2APIC (1 << 21)
స్థిర అంతఃవరుస శూన్య __cpuid(u32 సూచీ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (సూచీ));
}

స్థిర అంతఃవరుస u32 cr0_read(శూన్య) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    తిరిగి cr0;
}
స్థిర అంతఃవరుస శూన్య cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
స్థిర అంతఃవరుస శూన్య cr0_ముసుగు(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
స్థిర అంతఃవరుస u16 cr0_vm86_read(శూన్య) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    తిరిగి cr0;
}

స్థిర అంతఃవరుస u64 rdmsr(u32 సూచీ)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(సూచీ));
    తిరిగి ret;
}

స్థిర అంతఃవరుస శూన్య wrmsr(u32 సూచీ, u64 val)
{
    asm అస్థిర ("wrmsr" : : "c"(సూచీ), "A"(val));
}

స్థిర అంతఃవరుస u64 rdtscll(శూన్య)
{
    u64 val;
    asm అస్థిర("rdtsc" : "=A" (val));
    తిరిగి val;
}

స్థిర అంతఃవరుస u32 __ffs(u32 మాట)
{
    asm("bsf %1,%0"
        : "=r" (మాట)
        : "rm" (మాట));
    తిరిగి మాట;
}
స్థిర అంతఃవరుస u32 __fls(u32 మాట)
{
    asm("bsr %1,%0"
        : "=r" (మాట)
        : "rm" (మాట));
    తిరిగి మాట;
}

స్థిర అంతఃవరుస u32 getesp(శూన్య) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    తిరిగి esp;
}

స్థిర అంతఃవరుస u32 rol(u32 val, u16 rol) {
    u32 res;
    asm అస్థిర("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    తిరిగి res;
}

స్థిర అంతఃవరుస u32 ror(u32 val, u16 ror) {
    u32 res;
    asm అస్థిర("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    తిరిగి res;
}

స్థిర అంతఃవరుస శూన్య outb(u8 విలువ, u16 పోర్టు) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(విలువ), "Nd"(పోర్టు));
}
స్థిర అంతఃవరుస శూన్య outw(u16 విలువ, u16 పోర్టు) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(విలువ), "Nd"(పోర్టు));
}
స్థిర అంతఃవరుస శూన్య outl(u32 విలువ, u16 పోర్టు) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(విలువ), "Nd"(పోర్టు));
}
స్థిర అంతఃవరుస u8 inb(u16 పోర్టు) {
    u8 విలువ;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(విలువ) : "Nd"(పోర్టు));
    తిరిగి విలువ;
}
స్థిర అంతఃవరుస u16 inw(u16 పోర్టు) {
    u16 విలువ;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(విలువ) : "Nd"(పోర్టు));
    తిరిగి విలువ;
}
స్థిర అంతఃవరుస u32 inl(u16 పోర్టు) {
    u32 విలువ;
    __asm__ __volatile__("inl %w1, %0" : "=a"(విలువ) : "Nd"(పోర్టు));
    తిరిగి విలువ;
}

స్థిర అంతఃవరుస శూన్య insb(u16 పోర్టు, u8 *దత్తాంశం, u32 లెక్క) {
    asm అస్థిర("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(లెక్క), "+D"(దత్తాంశం) : "d"(పోర్టు) : "memory");
}
స్థిర అంతఃవరుస శూన్య insw(u16 పోర్టు, u16 *దత్తాంశం, u32 లెక్క) {
    asm అస్థిర("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(లెక్క), "+D"(దత్తాంశం) : "d"(పోర్టు) : "memory");
}
స్థిర అంతఃవరుస శూన్య insl(u16 పోర్టు, u32 *దత్తాంశం, u32 లెక్క) {
    asm అస్థిర("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(లెక్క), "+D"(దత్తాంశం) : "d"(పోర్టు) : "memory");
}
// XXX - outs not limited to es segment
స్థిర అంతఃవరుస శూన్య outsb(u16 పోర్టు, u8 *దత్తాంశం, u32 లెక్క) {
    asm అస్థిర("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(లెక్క), "+S"(దత్తాంశం) : "d"(పోర్టు) : "memory");
}
స్థిర అంతఃవరుస శూన్య outsw(u16 పోర్టు, u16 *దత్తాంశం, u32 లెక్క) {
    asm అస్థిర("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(లెక్క), "+S"(దత్తాంశం) : "d"(పోర్టు) : "memory");
}
స్థిర అంతఃవరుస శూన్య outsl(u16 పోర్టు, u32 *దత్తాంశం, u32 లెక్క) {
    asm అస్థిర("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(లెక్క), "+S"(దత్తాంశం) : "d"(పోర్టు) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
స్థిర అంతఃవరుస శూన్య smp_rmb(శూన్య) {
    barrier();
}
స్థిర అంతఃవరుస శూన్య smp_wmb(శూన్య) {
    barrier();
}

స్థిర అంతఃవరుస శూన్య writel(శూన్య *addr, u32 val) {
    barrier();
    *(అస్థిర u32 *)addr = val;
}
స్థిర అంతఃవరుస శూన్య writew(శూన్య *addr, u16 val) {
    barrier();
    *(అస్థిర u16 *)addr = val;
}
స్థిర అంతఃవరుస శూన్య writeb(శూన్య *addr, u8 val) {
    barrier();
    *(అస్థిర u8 *)addr = val;
}
స్థిర అంతఃవరుస u64 readq(స్థిరరాశి శూన్య *addr) {
    u64 val = *(అస్థిర స్థిరరాశి u64 *)addr;
    barrier();
    తిరిగి val;
}
స్థిర అంతఃవరుస u32 readl(స్థిరరాశి శూన్య *addr) {
    u32 val = *(అస్థిర స్థిరరాశి u32 *)addr;
    barrier();
    తిరిగి val;
}
స్థిర అంతఃవరుస u16 readw(స్థిరరాశి శూన్య *addr) {
    u16 val = *(అస్థిర స్థిరరాశి u16 *)addr;
    barrier();
    తిరిగి val;
}
స్థిర అంతఃవరుస u8 readb(స్థిరరాశి శూన్య *addr) {
    u8 val = *(అస్థిర స్థిరరాశి u8 *)addr;
    barrier();
    తిరిగి val;
}

// GDT bits
#నిర్వచించు GDT_సంకేతం     (0x9bULL << 40) // Code segment - P,R,A bits also set
#నిర్వచించు GDT_దత్తాంశం     (0x93ULL << 40) // Data segment - W,A bits also set
#నిర్వచించు GDT_B        (0x1ULL << 54)  // Big flag
#నిర్వచించు GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#నిర్వచించు GDT_పునాది(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#నిర్వచించు GDT_హద్దు(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#నిర్వచించు GDT_GRANLIMIT(v) (GDT_G | GDT_హద్దు((v) >> 12))

నిర్మాణం descloc_s {
    u16 నిడివి;
    u32 addr;
} PACKED;

స్థిర అంతఃవరుస శూన్య sgdt(నిర్మాణం descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
స్థిర అంతఃవరుస శూన్య lgdt(నిర్మాణం descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

స్థిర అంతఃవరుస u8 get_a20(శూన్య) {
    తిరిగి (inb(పోర్టు_A20) & A20_ENABLE_బిట్టు) != 0;
}

స్థిర అంతఃవరుస u8 సమితి_a20(u8 cond) {
    u8 val = inb(పోర్టు_A20), a20_enabled = (val & A20_ENABLE_బిట్టు) != 0;
    ఒకవేళ (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_బిట్టు, పోర్టు_A20);
    తిరిగి a20_enabled;
}

// x86.c
శూన్య cpuid(u32 సూచీ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#అయితే_ముగింపు // !__ASSEMBLY__

#అయితే_ముగింపు // x86.h
