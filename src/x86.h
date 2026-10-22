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

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ irq_disable(ଶୂନ୍ୟ)
{
    asm ଅସ୍ଥିର("cli": : :"memory");
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ irq_enable(ଶୂନ୍ୟ)
{
    asm ଅସ୍ଥିର("sti": : :"memory");
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 save_flags(ଶୂନ୍ୟ)
{
    u32 flags;
    asm ଅସ୍ଥିର("pushfl ; popl %0" : "=rm" (flags));
    ଫେରନ୍ତୁ flags;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ restore_flags(u32 flags)
{
    asm ଅସ୍ଥିର("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ cpu_relax(ଶୂନ୍ୟ)
{
    asm ଅସ୍ଥିର("rep ; nop": : :"memory");
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ nop(ଶୂନ୍ୟ)
{
    asm ଅସ୍ଥିର("nop");
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ hlt(ଶୂନ୍ୟ)
{
    asm ଅସ୍ଥିର("hlt": : :"memory");
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ wbinvd(ଶୂନ୍ୟ)
{
    asm ଅସ୍ଥିର("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 cr0_read(ଶୂନ୍ୟ) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    ଫେରନ୍ତୁ cr0;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 cr0_vm86_read(ଶୂନ୍ୟ) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    ଫେରନ୍ତୁ cr0;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 rdmsr(u32 index)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(index));
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ wrmsr(u32 index, u64 val)
{
    asm ଅସ୍ଥିର ("wrmsr" : : "c"(index), "A"(val));
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 rdtscll(ଶୂନ୍ୟ)
{
    u64 val;
    asm ଅସ୍ଥିର("rdtsc" : "=A" (val));
    ଫେରନ୍ତୁ val;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    ଫେରନ୍ତୁ word;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    ଫେରନ୍ତୁ word;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 getesp(ଶୂନ୍ୟ) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    ଫେରନ୍ତୁ esp;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm ଅସ୍ଥିର("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    ଫେରନ୍ତୁ res;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm ଅସ୍ଥିର("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    ଫେରନ୍ତୁ res;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outb(u8 ମୂଲ୍ୟ, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(ମୂଲ୍ୟ), "Nd"(port));
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outw(u16 ମୂଲ୍ୟ, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(ମୂଲ୍ୟ), "Nd"(port));
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outl(u32 ମୂଲ୍ୟ, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(ମୂଲ୍ୟ), "Nd"(port));
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8 inb(u16 port) {
    u8 ମୂଲ୍ୟ;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(ମୂଲ୍ୟ) : "Nd"(port));
    ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 inw(u16 port) {
    u16 ମୂଲ୍ୟ;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(ମୂଲ୍ୟ) : "Nd"(port));
    ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 inl(u16 port) {
    u32 ମୂଲ୍ୟ;
    __asm__ __volatile__("inl %w1, %0" : "=a"(ମୂଲ୍ୟ) : "Nd"(port));
    ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ insb(u16 port, u8 *data, u32 ଗଣନା_2) {
    asm ଅସ୍ଥିର("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(ଗଣନା_2), "+D"(data) : "d"(port) : "memory");
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ insw(u16 port, u16 *data, u32 ଗଣନା_2) {
    asm ଅସ୍ଥିର("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(ଗଣନା_2), "+D"(data) : "d"(port) : "memory");
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ insl(u16 port, u32 *data, u32 ଗଣନା_2) {
    asm ଅସ୍ଥିର("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(ଗଣନା_2), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outsb(u16 port, u8 *data, u32 ଗଣନା_2) {
    asm ଅସ୍ଥିର("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(ଗଣନା_2), "+S"(data) : "d"(port) : "memory");
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outsw(u16 port, u16 *data, u32 ଗଣନା_2) {
    asm ଅସ୍ଥିର("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(ଗଣନା_2), "+S"(data) : "d"(port) : "memory");
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ outsl(u16 port, u32 *data, u32 ଗଣନା_2) {
    asm ଅସ୍ଥିର("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(ଗଣନା_2), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ smp_rmb(ଶୂନ୍ୟ) {
    barrier();
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ smp_wmb(ଶୂନ୍ୟ) {
    barrier();
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ writel(ଶୂନ୍ୟ *addr, u32 val) {
    barrier();
    *(ଅସ୍ଥିର u32 *)addr = val;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ writew(ଶୂନ୍ୟ *addr, u16 val) {
    barrier();
    *(ଅସ୍ଥିର u16 *)addr = val;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ writeb(ଶୂନ୍ୟ *addr, u8 val) {
    barrier();
    *(ଅସ୍ଥିର u8 *)addr = val;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u64 readq(ଅଚଳ ଶୂନ୍ୟ *addr) {
    u64 val = *(ଅସ୍ଥିର ଅଚଳ u64 *)addr;
    barrier();
    ଫେରନ୍ତୁ val;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 readl(ଅଚଳ ଶୂନ୍ୟ *addr) {
    u32 val = *(ଅସ୍ଥିର ଅଚଳ u32 *)addr;
    barrier();
    ଫେରନ୍ତୁ val;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 readw(ଅଚଳ ଶୂନ୍ୟ *addr) {
    u16 val = *(ଅସ୍ଥିର ଅଚଳ u16 *)addr;
    barrier();
    ଫେରନ୍ତୁ val;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8 readb(ଅଚଳ ଶୂନ୍ୟ *addr) {
    u8 val = *(ଅସ୍ଥିର ଅଚଳ u8 *)addr;
    barrier();
    ଫେରନ୍ତୁ val;
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

ସଂରଚନା descloc_s {
    u16 length;
    u32 addr;
} PACKED;

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ sgdt(ସଂରଚନା descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ lgdt(ସଂରଚନା descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8 get_a20(ଶୂନ୍ୟ) {
    ଫେରନ୍ତୁ (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    ଯଦି (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    ଫେରନ୍ତୁ a20_enabled;
}

// x86.c
ଶୂନ୍ୟ cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
