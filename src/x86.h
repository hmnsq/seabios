// Basic x86 asm functions.
#ifndef __X86_H
#define __X86_H

// CPU flag bitdefs
#define F_CF (1<<0)
#define F_ZF (1<<6)
#define F_IF (1<<9)
#define F_ID (1<<21)

// CR0 flags
#define CR0_PG (1<<31) // Paging
#define CR0_CD (1<<30) // Cache disable
#define CR0_NW (1<<29) // Not Write-through
#define CR0_PE (1<<0)  // Protection enable

// PORT_A20 bitdefs
#define PORT_A20 0x0092
#define A20_ENABLE_BIT 0x02

#ifndef __ASSEMBLY__

#অন্তর্ভাব "types.h" // u32

স্থির অন্তঃরেখ শূন্য irq_disable(শূন্য)
{
    asm অস্থির("cli": : :"memory");
}

স্থির অন্তঃরেখ শূন্য irq_enable(শূন্য)
{
    asm অস্থির("sti": : :"memory");
}

স্থির অন্তঃরেখ u32 save_flags(শূন্য)
{
    u32 flags;
    asm অস্থির("pushfl ; popl %0" : "=rm" (flags));
    ফেরত flags;
}

স্থির অন্তঃরেখ শূন্য restore_flags(u32 flags)
{
    asm অস্থির("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

স্থির অন্তঃরেখ শূন্য cpu_relax(শূন্য)
{
    asm অস্থির("rep ; nop": : :"memory");
}

স্থির অন্তঃরেখ শূন্য nop(শূন্য)
{
    asm অস্থির("nop");
}

স্থির অন্তঃরেখ শূন্য hlt(শূন্য)
{
    asm অস্থির("hlt": : :"memory");
}

স্থির অন্তঃরেখ শূন্য wbinvd(শূন্য)
{
    asm অস্থির("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
স্থির অন্তঃরেখ শূন্য __cpuid(u32 সূচক, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (সূচক));
}

স্থির অন্তঃরেখ u32 cr0_read(শূন্য) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    ফেরত cr0;
}
স্থির অন্তঃরেখ শূন্য cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
স্থির অন্তঃরেখ শূন্য cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
স্থির অন্তঃরেখ u16 cr0_vm86_read(শূন্য) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    ফেরত cr0;
}

স্থির অন্তঃরেখ u64 rdmsr(u32 সূচক)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(সূচক));
    ফেরত ret;
}

স্থির অন্তঃরেখ শূন্য wrmsr(u32 সূচক, u64 val)
{
    asm অস্থির ("wrmsr" : : "c"(সূচক), "A"(val));
}

স্থির অন্তঃরেখ u64 rdtscll(শূন্য)
{
    u64 val;
    asm অস্থির("rdtsc" : "=A" (val));
    ফেরত val;
}

স্থির অন্তঃরেখ u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    ফেরত word;
}
স্থির অন্তঃরেখ u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    ফেরত word;
}

স্থির অন্তঃরেখ u32 getesp(শূন্য) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    ফেরত esp;
}

স্থির অন্তঃরেখ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm অস্থির("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    ফেরত res;
}

স্থির অন্তঃরেখ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm অস্থির("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    ফেরত res;
}

স্থির অন্তঃরেখ শূন্য outb(u8 মান, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(মান), "Nd"(port));
}
স্থির অন্তঃরেখ শূন্য outw(u16 মান, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(মান), "Nd"(port));
}
স্থির অন্তঃরেখ শূন্য outl(u32 মান, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(মান), "Nd"(port));
}
স্থির অন্তঃরেখ u8 inb(u16 port) {
    u8 মান;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(মান) : "Nd"(port));
    ফেরত মান;
}
স্থির অন্তঃরেখ u16 inw(u16 port) {
    u16 মান;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(মান) : "Nd"(port));
    ফেরত মান;
}
স্থির অন্তঃরেখ u32 inl(u16 port) {
    u32 মান;
    __asm__ __volatile__("inl %w1, %0" : "=a"(মান) : "Nd"(port));
    ফেরত মান;
}

স্থির অন্তঃরেখ শূন্য insb(u16 port, u8 *তথ্য, u32 গুনতি) {
    asm অস্থির("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(গুনতি), "+D"(তথ্য) : "d"(port) : "memory");
}
স্থির অন্তঃরেখ শূন্য insw(u16 port, u16 *তথ্য, u32 গুনতি) {
    asm অস্থির("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(গুনতি), "+D"(তথ্য) : "d"(port) : "memory");
}
স্থির অন্তঃরেখ শূন্য insl(u16 port, u32 *তথ্য, u32 গুনতি) {
    asm অস্থির("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(গুনতি), "+D"(তথ্য) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
স্থির অন্তঃরেখ শূন্য outsb(u16 port, u8 *তথ্য, u32 গুনতি) {
    asm অস্থির("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(গুনতি), "+S"(তথ্য) : "d"(port) : "memory");
}
স্থির অন্তঃরেখ শূন্য outsw(u16 port, u16 *তথ্য, u32 গুনতি) {
    asm অস্থির("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(গুনতি), "+S"(তথ্য) : "d"(port) : "memory");
}
স্থির অন্তঃরেখ শূন্য outsl(u16 port, u32 *তথ্য, u32 গুনতি) {
    asm অস্থির("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(গুনতি), "+S"(তথ্য) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
স্থির অন্তঃরেখ শূন্য smp_rmb(শূন্য) {
    barrier();
}
স্থির অন্তঃরেখ শূন্য smp_wmb(শূন্য) {
    barrier();
}

স্থির অন্তঃরেখ শূন্য writel(শূন্য *addr, u32 val) {
    barrier();
    *(অস্থির u32 *)addr = val;
}
স্থির অন্তঃরেখ শূন্য writew(শূন্য *addr, u16 val) {
    barrier();
    *(অস্থির u16 *)addr = val;
}
স্থির অন্তঃরেখ শূন্য writeb(শূন্য *addr, u8 val) {
    barrier();
    *(অস্থির u8 *)addr = val;
}
স্থির অন্তঃরেখ u64 readq(ধ্রুবক শূন্য *addr) {
    u64 val = *(অস্থির ধ্রুবক u64 *)addr;
    barrier();
    ফেরত val;
}
স্থির অন্তঃরেখ u32 readl(ধ্রুবক শূন্য *addr) {
    u32 val = *(অস্থির ধ্রুবক u32 *)addr;
    barrier();
    ফেরত val;
}
স্থির অন্তঃরেখ u16 readw(ধ্রুবক শূন্য *addr) {
    u16 val = *(অস্থির ধ্রুবক u16 *)addr;
    barrier();
    ফেরত val;
}
স্থির অন্তঃরেখ u8 readb(ধ্রুবক শূন্য *addr) {
    u8 val = *(অস্থির ধ্রুবক u8 *)addr;
    barrier();
    ফেরত val;
}

// GDT bits
#define GDT_CODE     (0x9bULL << 40) // Code segment - P,R,A bits also set
#define GDT_তথ্য     (0x93ULL << 40) // Data segment - W,A bits also set
#define GDT_B        (0x1ULL << 54)  // Big flag
#define GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#define GDT_BASE(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#define GDT_LIMIT(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#define GDT_GRANLIMIT(v) (GDT_G | GDT_LIMIT((v) >> 12))

কাঠামো descloc_s {
    u16 দৈর্ঘ্য;
    u32 addr;
} PACKED;

স্থির অন্তঃরেখ শূন্য sgdt(কাঠামো descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
স্থির অন্তঃরেখ শূন্য lgdt(কাঠামো descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

স্থির অন্তঃরেখ u8 get_a20(শূন্য) {
    ফেরত (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

স্থির অন্তঃরেখ u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    যদি (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    ফেরত a20_enabled;
}

// x86.c
শূন্য cpuid(u32 সূচক, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
