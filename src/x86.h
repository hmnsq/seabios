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

#ಅನ್ತರ್ಭಾವ "types.h" // u32

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ irq_disable(ಶೂನ್ಯ)
{
    asm ಚಞ್ಚಲ("cli": : :"memory");
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ irq_enable(ಶೂನ್ಯ)
{
    asm ಚಞ್ಚಲ("sti": : :"memory");
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 save_flags(ಶೂನ್ಯ)
{
    u32 flags;
    asm ಚಞ್ಚಲ("pushfl ; popl %0" : "=rm" (flags));
    ಪ್ರತ್ಯಾಗಮ flags;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ restore_flags(u32 flags)
{
    asm ಚಞ್ಚಲ("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ cpu_relax(ಶೂನ್ಯ)
{
    asm ಚಞ್ಚಲ("rep ; nop": : :"memory");
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ nop(ಶೂನ್ಯ)
{
    asm ಚಞ್ಚಲ("nop");
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ hlt(ಶೂನ್ಯ)
{
    asm ಚಞ್ಚಲ("hlt": : :"memory");
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ wbinvd(ಶೂನ್ಯ)
{
    asm ಚಞ್ಚಲ("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 cr0_read(ಶೂನ್ಯ) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    ಪ್ರತ್ಯಾಗಮ cr0;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u16 cr0_vm86_read(ಶೂನ್ಯ) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    ಪ್ರತ್ಯಾಗಮ cr0;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u64 rdmsr(u32 index)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(index));
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ wrmsr(u32 index, u64 val)
{
    asm ಚಞ್ಚಲ ("wrmsr" : : "c"(index), "A"(val));
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u64 rdtscll(ಶೂನ್ಯ)
{
    u64 val;
    asm ಚಞ್ಚಲ("rdtsc" : "=A" (val));
    ಪ್ರತ್ಯಾಗಮ val;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    ಪ್ರತ್ಯಾಗಮ word;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    ಪ್ರತ್ಯಾಗಮ word;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 getesp(ಶೂನ್ಯ) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    ಪ್ರತ್ಯಾಗಮ esp;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm ಚಞ್ಚಲ("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    ಪ್ರತ್ಯಾಗಮ res;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm ಚಞ್ಚಲ("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    ಪ್ರತ್ಯಾಗಮ res;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ outb(u8 ಮೌಲ್ಯ, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(ಮೌಲ್ಯ), "Nd"(port));
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ outw(u16 ಮೌಲ್ಯ, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(ಮೌಲ್ಯ), "Nd"(port));
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ outl(u32 ಮೌಲ್ಯ, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(ಮೌಲ್ಯ), "Nd"(port));
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u8 inb(u16 port) {
    u8 ಮೌಲ್ಯ;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(ಮೌಲ್ಯ) : "Nd"(port));
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u16 inw(u16 port) {
    u16 ಮೌಲ್ಯ;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(ಮೌಲ್ಯ) : "Nd"(port));
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 inl(u16 port) {
    u32 ಮೌಲ್ಯ;
    __asm__ __volatile__("inl %w1, %0" : "=a"(ಮೌಲ್ಯ) : "Nd"(port));
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ insb(u16 port, u8 *data, u32 ಎಣಿಕೆ) {
    asm ಚಞ್ಚಲ("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(ಎಣಿಕೆ), "+D"(data) : "d"(port) : "memory");
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ insw(u16 port, u16 *data, u32 ಎಣಿಕೆ) {
    asm ಚಞ್ಚಲ("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(ಎಣಿಕೆ), "+D"(data) : "d"(port) : "memory");
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ insl(u16 port, u32 *data, u32 ಎಣಿಕೆ) {
    asm ಚಞ್ಚಲ("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(ಎಣಿಕೆ), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ outsb(u16 port, u8 *data, u32 ಎಣಿಕೆ) {
    asm ಚಞ್ಚಲ("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(ಎಣಿಕೆ), "+S"(data) : "d"(port) : "memory");
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ outsw(u16 port, u16 *data, u32 ಎಣಿಕೆ) {
    asm ಚಞ್ಚಲ("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(ಎಣಿಕೆ), "+S"(data) : "d"(port) : "memory");
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ outsl(u16 port, u32 *data, u32 ಎಣಿಕೆ) {
    asm ಚಞ್ಚಲ("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(ಎಣಿಕೆ), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ smp_rmb(ಶೂನ್ಯ) {
    barrier();
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ smp_wmb(ಶೂನ್ಯ) {
    barrier();
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ writel(ಶೂನ್ಯ *addr, u32 val) {
    barrier();
    *(ಚಞ್ಚಲ u32 *)addr = val;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ writew(ಶೂನ್ಯ *addr, u16 val) {
    barrier();
    *(ಚಞ್ಚಲ u16 *)addr = val;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ writeb(ಶೂನ್ಯ *addr, u8 val) {
    barrier();
    *(ಚಞ್ಚಲ u8 *)addr = val;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u64 readq(ಧ್ರುವ ಶೂನ್ಯ *addr) {
    u64 val = *(ಚಞ್ಚಲ ಧ್ರುವ u64 *)addr;
    barrier();
    ಪ್ರತ್ಯಾಗಮ val;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 readl(ಧ್ರುವ ಶೂನ್ಯ *addr) {
    u32 val = *(ಚಞ್ಚಲ ಧ್ರುವ u32 *)addr;
    barrier();
    ಪ್ರತ್ಯಾಗಮ val;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u16 readw(ಧ್ರುವ ಶೂನ್ಯ *addr) {
    u16 val = *(ಚಞ್ಚಲ ಧ್ರುವ u16 *)addr;
    barrier();
    ಪ್ರತ್ಯಾಗಮ val;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u8 readb(ಧ್ರುವ ಶೂನ್ಯ *addr) {
    u8 val = *(ಚಞ್ಚಲ ಧ್ರುವ u8 *)addr;
    barrier();
    ಪ್ರತ್ಯಾಗಮ val;
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

ಸಂರಚನಾ descloc_s {
    u16 length;
    u32 addr;
} PACKED;

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ sgdt(ಸಂರಚನಾ descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ lgdt(ಸಂರಚನಾ descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u8 get_a20(ಶೂನ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    ಯದಿ (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    ಪ್ರತ್ಯಾಗಮ a20_enabled;
}

// x86.c
ಶೂನ್ಯ cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
