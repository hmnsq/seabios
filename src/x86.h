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

#অন্তর্ভাৱ "types.h" // u32

স্থির অন্তঃস্থ শূন্য irq_disable(শূন্য)
{
    asm চঞ্চল("cli": : :"memory");
}

স্থির অন্তঃস্থ শূন্য irq_enable(শূন্য)
{
    asm চঞ্চল("sti": : :"memory");
}

স্থির অন্তঃস্থ u32 save_flags(শূন্য)
{
    u32 flags;
    asm চঞ্চল("pushfl ; popl %0" : "=rm" (flags));
    ঘূৰাই flags;
}

স্থির অন্তঃস্থ শূন্য restore_flags(u32 flags)
{
    asm চঞ্চল("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

স্থির অন্তঃস্থ শূন্য cpu_relax(শূন্য)
{
    asm চঞ্চল("rep ; nop": : :"memory");
}

স্থির অন্তঃস্থ শূন্য nop(শূন্য)
{
    asm চঞ্চল("nop");
}

স্থির অন্তঃস্থ শূন্য hlt(শূন্য)
{
    asm চঞ্চল("hlt": : :"memory");
}

স্থির অন্তঃস্থ শূন্য wbinvd(শূন্য)
{
    asm চঞ্চল("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
স্থির অন্তঃস্থ শূন্য __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

স্থির অন্তঃস্থ u32 cr0_read(শূন্য) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    ঘূৰাই cr0;
}
স্থির অন্তঃস্থ শূন্য cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
স্থির অন্তঃস্থ শূন্য cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
স্থির অন্তঃস্থ u16 cr0_vm86_read(শূন্য) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    ঘূৰাই cr0;
}

স্থির অন্তঃস্থ u64 rdmsr(u32 index)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(index));
    ঘূৰাই ret;
}

স্থির অন্তঃস্থ শূন্য wrmsr(u32 index, u64 val)
{
    asm চঞ্চল ("wrmsr" : : "c"(index), "A"(val));
}

স্থির অন্তঃস্থ u64 rdtscll(শূন্য)
{
    u64 val;
    asm চঞ্চল("rdtsc" : "=A" (val));
    ঘূৰাই val;
}

স্থির অন্তঃস্থ u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    ঘূৰাই word;
}
স্থির অন্তঃস্থ u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    ঘূৰাই word;
}

স্থির অন্তঃস্থ u32 getesp(শূন্য) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    ঘূৰাই esp;
}

স্থির অন্তঃস্থ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm চঞ্চল("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    ঘূৰাই res;
}

স্থির অন্তঃস্থ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm চঞ্চল("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    ঘূৰাই res;
}

স্থির অন্তঃস্থ শূন্য outb(u8 মান, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(মান), "Nd"(port));
}
স্থির অন্তঃস্থ শূন্য outw(u16 মান, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(মান), "Nd"(port));
}
স্থির অন্তঃস্থ শূন্য outl(u32 মান, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(মান), "Nd"(port));
}
স্থির অন্তঃস্থ u8 inb(u16 port) {
    u8 মান;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(মান) : "Nd"(port));
    ঘূৰাই মান;
}
স্থির অন্তঃস্থ u16 inw(u16 port) {
    u16 মান;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(মান) : "Nd"(port));
    ঘূৰাই মান;
}
স্থির অন্তঃস্থ u32 inl(u16 port) {
    u32 মান;
    __asm__ __volatile__("inl %w1, %0" : "=a"(মান) : "Nd"(port));
    ঘূৰাই মান;
}

স্থির অন্তঃস্থ শূন্য insb(u16 port, u8 *data, u32 গুনতি) {
    asm চঞ্চল("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(গুনতি), "+D"(data) : "d"(port) : "memory");
}
স্থির অন্তঃস্থ শূন্য insw(u16 port, u16 *data, u32 গুনতি) {
    asm চঞ্চল("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(গুনতি), "+D"(data) : "d"(port) : "memory");
}
স্থির অন্তঃস্থ শূন্য insl(u16 port, u32 *data, u32 গুনতি) {
    asm চঞ্চল("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(গুনতি), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
স্থির অন্তঃস্থ শূন্য outsb(u16 port, u8 *data, u32 গুনতি) {
    asm চঞ্চল("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(গুনতি), "+S"(data) : "d"(port) : "memory");
}
স্থির অন্তঃস্থ শূন্য outsw(u16 port, u16 *data, u32 গুনতি) {
    asm চঞ্চল("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(গুনতি), "+S"(data) : "d"(port) : "memory");
}
স্থির অন্তঃস্থ শূন্য outsl(u16 port, u32 *data, u32 গুনতি) {
    asm চঞ্চল("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(গুনতি), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
স্থির অন্তঃস্থ শূন্য smp_rmb(শূন্য) {
    barrier();
}
স্থির অন্তঃস্থ শূন্য smp_wmb(শূন্য) {
    barrier();
}

স্থির অন্তঃস্থ শূন্য writel(শূন্য *addr, u32 val) {
    barrier();
    *(চঞ্চল u32 *)addr = val;
}
স্থির অন্তঃস্থ শূন্য writew(শূন্য *addr, u16 val) {
    barrier();
    *(চঞ্চল u16 *)addr = val;
}
স্থির অন্তঃস্থ শূন্য writeb(শূন্য *addr, u8 val) {
    barrier();
    *(চঞ্চল u8 *)addr = val;
}
স্থির অন্তঃস্থ u64 readq(ধ্রুৱ শূন্য *addr) {
    u64 val = *(চঞ্চল ধ্রুৱ u64 *)addr;
    barrier();
    ঘূৰাই val;
}
স্থির অন্তঃস্থ u32 readl(ধ্রুৱ শূন্য *addr) {
    u32 val = *(চঞ্চল ধ্রুৱ u32 *)addr;
    barrier();
    ঘূৰাই val;
}
স্থির অন্তঃস্থ u16 readw(ধ্রুৱ শূন্য *addr) {
    u16 val = *(চঞ্চল ধ্রুৱ u16 *)addr;
    barrier();
    ঘূৰাই val;
}
স্থির অন্তঃস্থ u8 readb(ধ্রুৱ শূন্য *addr) {
    u8 val = *(চঞ্চল ধ্রুৱ u8 *)addr;
    barrier();
    ঘূৰাই val;
}

// GDT bits
#define GDT_CODE     (0x9bULL << 40) // Code segment - P,R,A bits also set
#define GDT_DATA     (0x93ULL << 40) // Data segment - W,A bits also set
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

সংরচনা descloc_s {
    u16 length;
    u32 addr;
} PACKED;

স্থির অন্তঃস্থ শূন্য sgdt(সংরচনা descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
স্থির অন্তঃস্থ শূন্য lgdt(সংরচনা descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

স্থির অন্তঃস্থ u8 get_a20(শূন্য) {
    ঘূৰাই (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

স্থির অন্তঃস্থ u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    যদি (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    ঘূৰাই a20_enabled;
}

// x86.c
শূন্য cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
