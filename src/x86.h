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

#شامل "types.h" // u32

ایستا درونخطی تهی irq_disable(تهی)
{
    asm فرار("cli": : :"memory");
}

ایستا درونخطی تهی irq_enable(تهی)
{
    asm فرار("sti": : :"memory");
}

ایستا درونخطی u32 save_flags(تهی)
{
    u32 flags;
    asm فرار("pushfl ; popl %0" : "=rm" (flags));
    بازگشت flags;
}

ایستا درونخطی تهی restore_flags(u32 flags)
{
    asm فرار("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

ایستا درونخطی تهی cpu_relax(تهی)
{
    asm فرار("rep ; nop": : :"memory");
}

ایستا درونخطی تهی nop(تهی)
{
    asm فرار("nop");
}

ایستا درونخطی تهی hlt(تهی)
{
    asm فرار("hlt": : :"memory");
}

ایستا درونخطی تهی wbinvd(تهی)
{
    asm فرار("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
ایستا درونخطی تهی __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

ایستا درونخطی u32 cr0_read(تهی) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    بازگشت cr0;
}
ایستا درونخطی تهی cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
ایستا درونخطی تهی cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
ایستا درونخطی u16 cr0_vm86_read(تهی) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    بازگشت cr0;
}

ایستا درونخطی u64 rdmsr(u32 index)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(index));
    بازگشت ret;
}

ایستا درونخطی تهی wrmsr(u32 index, u64 val)
{
    asm فرار ("wrmsr" : : "c"(index), "A"(val));
}

ایستا درونخطی u64 rdtscll(تهی)
{
    u64 val;
    asm فرار("rdtsc" : "=A" (val));
    بازگشت val;
}

ایستا درونخطی u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    بازگشت word;
}
ایستا درونخطی u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    بازگشت word;
}

ایستا درونخطی u32 getesp(تهی) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    بازگشت esp;
}

ایستا درونخطی u32 rol(u32 val, u16 rol) {
    u32 res;
    asm فرار("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    بازگشت res;
}

ایستا درونخطی u32 ror(u32 val, u16 ror) {
    u32 res;
    asm فرار("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    بازگشت res;
}

ایستا درونخطی تهی outb(u8 مقدار, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(مقدار), "Nd"(port));
}
ایستا درونخطی تهی outw(u16 مقدار, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(مقدار), "Nd"(port));
}
ایستا درونخطی تهی outl(u32 مقدار, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(مقدار), "Nd"(port));
}
ایستا درونخطی u8 inb(u16 port) {
    u8 مقدار;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(مقدار) : "Nd"(port));
    بازگشت مقدار;
}
ایستا درونخطی u16 inw(u16 port) {
    u16 مقدار;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(مقدار) : "Nd"(port));
    بازگشت مقدار;
}
ایستا درونخطی u32 inl(u16 port) {
    u32 مقدار;
    __asm__ __volatile__("inl %w1, %0" : "=a"(مقدار) : "Nd"(port));
    بازگشت مقدار;
}

ایستا درونخطی تهی insb(u16 port, u8 *data, u32 شمار) {
    asm فرار("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(شمار), "+D"(data) : "d"(port) : "memory");
}
ایستا درونخطی تهی insw(u16 port, u16 *data, u32 شمار) {
    asm فرار("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(شمار), "+D"(data) : "d"(port) : "memory");
}
ایستا درونخطی تهی insl(u16 port, u32 *data, u32 شمار) {
    asm فرار("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(شمار), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
ایستا درونخطی تهی outsb(u16 port, u8 *data, u32 شمار) {
    asm فرار("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(شمار), "+S"(data) : "d"(port) : "memory");
}
ایستا درونخطی تهی outsw(u16 port, u16 *data, u32 شمار) {
    asm فرار("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(شمار), "+S"(data) : "d"(port) : "memory");
}
ایستا درونخطی تهی outsl(u16 port, u32 *data, u32 شمار) {
    asm فرار("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(شمار), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
ایستا درونخطی تهی smp_rmb(تهی) {
    barrier();
}
ایستا درونخطی تهی smp_wmb(تهی) {
    barrier();
}

ایستا درونخطی تهی writel(تهی *addr, u32 val) {
    barrier();
    *(فرار u32 *)addr = val;
}
ایستا درونخطی تهی writew(تهی *addr, u16 val) {
    barrier();
    *(فرار u16 *)addr = val;
}
ایستا درونخطی تهی writeb(تهی *addr, u8 val) {
    barrier();
    *(فرار u8 *)addr = val;
}
ایستا درونخطی u64 readq(ثابت تهی *addr) {
    u64 val = *(فرار ثابت u64 *)addr;
    barrier();
    بازگشت val;
}
ایستا درونخطی u32 readl(ثابت تهی *addr) {
    u32 val = *(فرار ثابت u32 *)addr;
    barrier();
    بازگشت val;
}
ایستا درونخطی u16 readw(ثابت تهی *addr) {
    u16 val = *(فرار ثابت u16 *)addr;
    barrier();
    بازگشت val;
}
ایستا درونخطی u8 readb(ثابت تهی *addr) {
    u8 val = *(فرار ثابت u8 *)addr;
    barrier();
    بازگشت val;
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

ساختار descloc_s {
    u16 length;
    u32 addr;
} PACKED;

ایستا درونخطی تهی sgdt(ساختار descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
ایستا درونخطی تهی lgdt(ساختار descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

ایستا درونخطی u8 get_a20(تهی) {
    بازگشت (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

ایستا درونخطی u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    اگر (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    بازگشت a20_enabled;
}

// x86.c
تهی cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
