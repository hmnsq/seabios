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

#അന്തര്ഭാവ "types.h" // u32

സ്ഥിതം വരിയിൽ ശൂന്യം irq_disable(ശൂന്യം)
{
    asm അസ്ഥിര("cli": : :"memory");
}

സ്ഥിതം വരിയിൽ ശൂന്യം irq_enable(ശൂന്യം)
{
    asm അസ്ഥിര("sti": : :"memory");
}

സ്ഥിതം വരിയിൽ u32 save_flags(ശൂന്യം)
{
    u32 flags;
    asm അസ്ഥിര("pushfl ; popl %0" : "=rm" (flags));
    തിരികെ flags;
}

സ്ഥിതം വരിയിൽ ശൂന്യം restore_flags(u32 flags)
{
    asm അസ്ഥിര("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

സ്ഥിതം വരിയിൽ ശൂന്യം cpu_relax(ശൂന്യം)
{
    asm അസ്ഥിര("rep ; nop": : :"memory");
}

സ്ഥിതം വരിയിൽ ശൂന്യം nop(ശൂന്യം)
{
    asm അസ്ഥിര("nop");
}

സ്ഥിതം വരിയിൽ ശൂന്യം hlt(ശൂന്യം)
{
    asm അസ്ഥിര("hlt": : :"memory");
}

സ്ഥിതം വരിയിൽ ശൂന്യം wbinvd(ശൂന്യം)
{
    asm അസ്ഥിര("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
സ്ഥിതം വരിയിൽ ശൂന്യം __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

സ്ഥിതം വരിയിൽ u32 cr0_read(ശൂന്യം) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    തിരികെ cr0;
}
സ്ഥിതം വരിയിൽ ശൂന്യം cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
സ്ഥിതം വരിയിൽ ശൂന്യം cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
സ്ഥിതം വരിയിൽ u16 cr0_vm86_read(ശൂന്യം) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    തിരികെ cr0;
}

സ്ഥിതം വരിയിൽ u64 rdmsr(u32 index)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(index));
    തിരികെ ret;
}

സ്ഥിതം വരിയിൽ ശൂന്യം wrmsr(u32 index, u64 val)
{
    asm അസ്ഥിര ("wrmsr" : : "c"(index), "A"(val));
}

സ്ഥിതം വരിയിൽ u64 rdtscll(ശൂന്യം)
{
    u64 val;
    asm അസ്ഥിര("rdtsc" : "=A" (val));
    തിരികെ val;
}

സ്ഥിതം വരിയിൽ u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    തിരികെ word;
}
സ്ഥിതം വരിയിൽ u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    തിരികെ word;
}

സ്ഥിതം വരിയിൽ u32 getesp(ശൂന്യം) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    തിരികെ esp;
}

സ്ഥിതം വരിയിൽ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm അസ്ഥിര("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    തിരികെ res;
}

സ്ഥിതം വരിയിൽ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm അസ്ഥിര("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    തിരികെ res;
}

സ്ഥിതം വരിയിൽ ശൂന്യം outb(u8 മൂല്യം, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(മൂല്യം), "Nd"(port));
}
സ്ഥിതം വരിയിൽ ശൂന്യം outw(u16 മൂല്യം, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(മൂല്യം), "Nd"(port));
}
സ്ഥിതം വരിയിൽ ശൂന്യം outl(u32 മൂല്യം, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(മൂല്യം), "Nd"(port));
}
സ്ഥിതം വരിയിൽ u8 inb(u16 port) {
    u8 മൂല്യം;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(മൂല്യം) : "Nd"(port));
    തിരികെ മൂല്യം;
}
സ്ഥിതം വരിയിൽ u16 inw(u16 port) {
    u16 മൂല്യം;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(മൂല്യം) : "Nd"(port));
    തിരികെ മൂല്യം;
}
സ്ഥിതം വരിയിൽ u32 inl(u16 port) {
    u32 മൂല്യം;
    __asm__ __volatile__("inl %w1, %0" : "=a"(മൂല്യം) : "Nd"(port));
    തിരികെ മൂല്യം;
}

സ്ഥിതം വരിയിൽ ശൂന്യം insb(u16 port, u8 *data, u32 എണ്ണം) {
    asm അസ്ഥിര("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(എണ്ണം), "+D"(data) : "d"(port) : "memory");
}
സ്ഥിതം വരിയിൽ ശൂന്യം insw(u16 port, u16 *data, u32 എണ്ണം) {
    asm അസ്ഥിര("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(എണ്ണം), "+D"(data) : "d"(port) : "memory");
}
സ്ഥിതം വരിയിൽ ശൂന്യം insl(u16 port, u32 *data, u32 എണ്ണം) {
    asm അസ്ഥിര("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(എണ്ണം), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
സ്ഥിതം വരിയിൽ ശൂന്യം outsb(u16 port, u8 *data, u32 എണ്ണം) {
    asm അസ്ഥിര("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(എണ്ണം), "+S"(data) : "d"(port) : "memory");
}
സ്ഥിതം വരിയിൽ ശൂന്യം outsw(u16 port, u16 *data, u32 എണ്ണം) {
    asm അസ്ഥിര("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(എണ്ണം), "+S"(data) : "d"(port) : "memory");
}
സ്ഥിതം വരിയിൽ ശൂന്യം outsl(u16 port, u32 *data, u32 എണ്ണം) {
    asm അസ്ഥിര("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(എണ്ണം), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
സ്ഥിതം വരിയിൽ ശൂന്യം smp_rmb(ശൂന്യം) {
    barrier();
}
സ്ഥിതം വരിയിൽ ശൂന്യം smp_wmb(ശൂന്യം) {
    barrier();
}

സ്ഥിതം വരിയിൽ ശൂന്യം writel(ശൂന്യം *addr, u32 val) {
    barrier();
    *(അസ്ഥിര u32 *)addr = val;
}
സ്ഥിതം വരിയിൽ ശൂന്യം writew(ശൂന്യം *addr, u16 val) {
    barrier();
    *(അസ്ഥിര u16 *)addr = val;
}
സ്ഥിതം വരിയിൽ ശൂന്യം writeb(ശൂന്യം *addr, u8 val) {
    barrier();
    *(അസ്ഥിര u8 *)addr = val;
}
സ്ഥിതം വരിയിൽ u64 readq(സ്ഥിരം ശൂന്യം *addr) {
    u64 val = *(അസ്ഥിര സ്ഥിരം u64 *)addr;
    barrier();
    തിരികെ val;
}
സ്ഥിതം വരിയിൽ u32 readl(സ്ഥിരം ശൂന്യം *addr) {
    u32 val = *(അസ്ഥിര സ്ഥിരം u32 *)addr;
    barrier();
    തിരികെ val;
}
സ്ഥിതം വരിയിൽ u16 readw(സ്ഥിരം ശൂന്യം *addr) {
    u16 val = *(അസ്ഥിര സ്ഥിരം u16 *)addr;
    barrier();
    തിരികെ val;
}
സ്ഥിതം വരിയിൽ u8 readb(സ്ഥിരം ശൂന്യം *addr) {
    u8 val = *(അസ്ഥിര സ്ഥിരം u8 *)addr;
    barrier();
    തിരികെ val;
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

ഘടന descloc_s {
    u16 length;
    u32 addr;
} PACKED;

സ്ഥിതം വരിയിൽ ശൂന്യം sgdt(ഘടന descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
സ്ഥിതം വരിയിൽ ശൂന്യം lgdt(ഘടന descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

സ്ഥിതം വരിയിൽ u8 get_a20(ശൂന്യം) {
    തിരികെ (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

സ്ഥിതം വരിയിൽ u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    എങ്കിൽ (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    തിരികെ a20_enabled;
}

// x86.c
ശൂന്യം cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
