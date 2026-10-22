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

#અન્તર્ભાવ "types.h" // u32

સ્થિર અંતઃરેખ શૂન્ય irq_disable(શૂન્ય)
{
    asm અસ્થિર("cli": : :"memory");
}

સ્થિર અંતઃરેખ શૂન્ય irq_enable(શૂન્ય)
{
    asm અસ્થિર("sti": : :"memory");
}

સ્થિર અંતઃરેખ u32 save_flags(શૂન્ય)
{
    u32 flags;
    asm અસ્થિર("pushfl ; popl %0" : "=rm" (flags));
    પાછા flags;
}

સ્થિર અંતઃરેખ શૂન્ય restore_flags(u32 flags)
{
    asm અસ્થિર("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

સ્થિર અંતઃરેખ શૂન્ય cpu_relax(શૂન્ય)
{
    asm અસ્થિર("rep ; nop": : :"memory");
}

સ્થિર અંતઃરેખ શૂન્ય nop(શૂન્ય)
{
    asm અસ્થિર("nop");
}

સ્થિર અંતઃરેખ શૂન્ય hlt(શૂન્ય)
{
    asm અસ્થિર("hlt": : :"memory");
}

સ્થિર અંતઃરેખ શૂન્ય wbinvd(શૂન્ય)
{
    asm અસ્થિર("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
સ્થિર અંતઃરેખ શૂન્ય __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

સ્થિર અંતઃરેખ u32 cr0_read(શૂન્ય) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    પાછા cr0;
}
સ્થિર અંતઃરેખ શૂન્ય cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
સ્થિર અંતઃરેખ શૂન્ય cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
સ્થિર અંતઃરેખ u16 cr0_vm86_read(શૂન્ય) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    પાછા cr0;
}

સ્થિર અંતઃરેખ u64 rdmsr(u32 index)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(index));
    પાછા ret;
}

સ્થિર અંતઃરેખ શૂન્ય wrmsr(u32 index, u64 val)
{
    asm અસ્થિર ("wrmsr" : : "c"(index), "A"(val));
}

સ્થિર અંતઃરેખ u64 rdtscll(શૂન્ય)
{
    u64 val;
    asm અસ્થિર("rdtsc" : "=A" (val));
    પાછા val;
}

સ્થિર અંતઃરેખ u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    પાછા word;
}
સ્થિર અંતઃરેખ u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    પાછા word;
}

સ્થિર અંતઃરેખ u32 getesp(શૂન્ય) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    પાછા esp;
}

સ્થિર અંતઃરેખ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm અસ્થિર("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    પાછા res;
}

સ્થિર અંતઃરેખ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm અસ્થિર("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    પાછા res;
}

સ્થિર અંતઃરેખ શૂન્ય outb(u8 કિંમત, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(કિંમત), "Nd"(port));
}
સ્થિર અંતઃરેખ શૂન્ય outw(u16 કિંમત, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(કિંમત), "Nd"(port));
}
સ્થિર અંતઃરેખ શૂન્ય outl(u32 કિંમત, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(કિંમત), "Nd"(port));
}
સ્થિર અંતઃરેખ u8 inb(u16 port) {
    u8 કિંમત;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(કિંમત) : "Nd"(port));
    પાછા કિંમત;
}
સ્થિર અંતઃરેખ u16 inw(u16 port) {
    u16 કિંમત;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(કિંમત) : "Nd"(port));
    પાછા કિંમત;
}
સ્થિર અંતઃરેખ u32 inl(u16 port) {
    u32 કિંમત;
    __asm__ __volatile__("inl %w1, %0" : "=a"(કિંમત) : "Nd"(port));
    પાછા કિંમત;
}

સ્થિર અંતઃરેખ શૂન્ય insb(u16 port, u8 *data, u32 ગણતરી) {
    asm અસ્થિર("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(ગણતરી), "+D"(data) : "d"(port) : "memory");
}
સ્થિર અંતઃરેખ શૂન્ય insw(u16 port, u16 *data, u32 ગણતરી) {
    asm અસ્થિર("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(ગણતરી), "+D"(data) : "d"(port) : "memory");
}
સ્થિર અંતઃરેખ શૂન્ય insl(u16 port, u32 *data, u32 ગણતરી) {
    asm અસ્થિર("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(ગણતરી), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
સ્થિર અંતઃરેખ શૂન્ય outsb(u16 port, u8 *data, u32 ગણતરી) {
    asm અસ્થિર("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(ગણતરી), "+S"(data) : "d"(port) : "memory");
}
સ્થિર અંતઃરેખ શૂન્ય outsw(u16 port, u16 *data, u32 ગણતરી) {
    asm અસ્થિર("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(ગણતરી), "+S"(data) : "d"(port) : "memory");
}
સ્થિર અંતઃરેખ શૂન્ય outsl(u16 port, u32 *data, u32 ગણતરી) {
    asm અસ્થિર("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(ગણતરી), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
સ્થિર અંતઃરેખ શૂન્ય smp_rmb(શૂન્ય) {
    barrier();
}
સ્થિર અંતઃરેખ શૂન્ય smp_wmb(શૂન્ય) {
    barrier();
}

સ્થિર અંતઃરેખ શૂન્ય writel(શૂન્ય *addr, u32 val) {
    barrier();
    *(અસ્થિર u32 *)addr = val;
}
સ્થિર અંતઃરેખ શૂન્ય writew(શૂન્ય *addr, u16 val) {
    barrier();
    *(અસ્થિર u16 *)addr = val;
}
સ્થિર અંતઃરેખ શૂન્ય writeb(શૂન્ય *addr, u8 val) {
    barrier();
    *(અસ્થિર u8 *)addr = val;
}
સ્થિર અંતઃરેખ u64 readq(અચલ શૂન્ય *addr) {
    u64 val = *(અસ્થિર અચલ u64 *)addr;
    barrier();
    પાછા val;
}
સ્થિર અંતઃરેખ u32 readl(અચલ શૂન્ય *addr) {
    u32 val = *(અસ્થિર અચલ u32 *)addr;
    barrier();
    પાછા val;
}
સ્થિર અંતઃરેખ u16 readw(અચલ શૂન્ય *addr) {
    u16 val = *(અસ્થિર અચલ u16 *)addr;
    barrier();
    પાછા val;
}
સ્થિર અંતઃરેખ u8 readb(અચલ શૂન્ય *addr) {
    u8 val = *(અસ્થિર અચલ u8 *)addr;
    barrier();
    પાછા val;
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

સંરચના descloc_s {
    u16 length;
    u32 addr;
} PACKED;

સ્થિર અંતઃરેખ શૂન્ય sgdt(સંરચના descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
સ્થિર અંતઃરેખ શૂન્ય lgdt(સંરચના descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

સ્થિર અંતઃરેખ u8 get_a20(શૂન્ય) {
    પાછા (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

સ્થિર અંતઃરેખ u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    જો (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    પાછા a20_enabled;
}

// x86.c
શૂન્ય cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
