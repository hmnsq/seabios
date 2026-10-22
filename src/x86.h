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

#அந்தர்பாவ "types.h" // u32

நிலையான உள்வரி வெறுமை irq_disable(வெறுமை)
{
    asm நிலையற்ற("cli": : :"memory");
}

நிலையான உள்வரி வெறுமை irq_enable(வெறுமை)
{
    asm நிலையற்ற("sti": : :"memory");
}

நிலையான உள்வரி u32 save_flags(வெறுமை)
{
    u32 flags;
    asm நிலையற்ற("pushfl ; popl %0" : "=rm" (flags));
    திரும்பு flags;
}

நிலையான உள்வரி வெறுமை restore_flags(u32 flags)
{
    asm நிலையற்ற("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

நிலையான உள்வரி வெறுமை cpu_relax(வெறுமை)
{
    asm நிலையற்ற("rep ; nop": : :"memory");
}

நிலையான உள்வரி வெறுமை nop(வெறுமை)
{
    asm நிலையற்ற("nop");
}

நிலையான உள்வரி வெறுமை hlt(வெறுமை)
{
    asm நிலையற்ற("hlt": : :"memory");
}

நிலையான உள்வரி வெறுமை wbinvd(வெறுமை)
{
    asm நிலையற்ற("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
நிலையான உள்வரி வெறுமை __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

நிலையான உள்வரி u32 cr0_read(வெறுமை) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    திரும்பு cr0;
}
நிலையான உள்வரி வெறுமை cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
நிலையான உள்வரி வெறுமை cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
நிலையான உள்வரி u16 cr0_vm86_read(வெறுமை) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    திரும்பு cr0;
}

நிலையான உள்வரி u64 rdmsr(u32 index)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(index));
    திரும்பு ret;
}

நிலையான உள்வரி வெறுமை wrmsr(u32 index, u64 val)
{
    asm நிலையற்ற ("wrmsr" : : "c"(index), "A"(val));
}

நிலையான உள்வரி u64 rdtscll(வெறுமை)
{
    u64 val;
    asm நிலையற்ற("rdtsc" : "=A" (val));
    திரும்பு val;
}

நிலையான உள்வரி u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    திரும்பு word;
}
நிலையான உள்வரி u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    திரும்பு word;
}

நிலையான உள்வரி u32 getesp(வெறுமை) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    திரும்பு esp;
}

நிலையான உள்வரி u32 rol(u32 val, u16 rol) {
    u32 res;
    asm நிலையற்ற("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    திரும்பு res;
}

நிலையான உள்வரி u32 ror(u32 val, u16 ror) {
    u32 res;
    asm நிலையற்ற("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    திரும்பு res;
}

நிலையான உள்வரி வெறுமை outb(u8 மதிப்பு, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(மதிப்பு), "Nd"(port));
}
நிலையான உள்வரி வெறுமை outw(u16 மதிப்பு, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(மதிப்பு), "Nd"(port));
}
நிலையான உள்வரி வெறுமை outl(u32 மதிப்பு, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(மதிப்பு), "Nd"(port));
}
நிலையான உள்வரி u8 inb(u16 port) {
    u8 மதிப்பு;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(மதிப்பு) : "Nd"(port));
    திரும்பு மதிப்பு;
}
நிலையான உள்வரி u16 inw(u16 port) {
    u16 மதிப்பு;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(மதிப்பு) : "Nd"(port));
    திரும்பு மதிப்பு;
}
நிலையான உள்வரி u32 inl(u16 port) {
    u32 மதிப்பு;
    __asm__ __volatile__("inl %w1, %0" : "=a"(மதிப்பு) : "Nd"(port));
    திரும்பு மதிப்பு;
}

நிலையான உள்வரி வெறுமை insb(u16 port, u8 *data, u32 மொத்தம்) {
    asm நிலையற்ற("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(மொத்தம்), "+D"(data) : "d"(port) : "memory");
}
நிலையான உள்வரி வெறுமை insw(u16 port, u16 *data, u32 மொத்தம்) {
    asm நிலையற்ற("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(மொத்தம்), "+D"(data) : "d"(port) : "memory");
}
நிலையான உள்வரி வெறுமை insl(u16 port, u32 *data, u32 மொத்தம்) {
    asm நிலையற்ற("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(மொத்தம்), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
நிலையான உள்வரி வெறுமை outsb(u16 port, u8 *data, u32 மொத்தம்) {
    asm நிலையற்ற("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(மொத்தம்), "+S"(data) : "d"(port) : "memory");
}
நிலையான உள்வரி வெறுமை outsw(u16 port, u16 *data, u32 மொத்தம்) {
    asm நிலையற்ற("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(மொத்தம்), "+S"(data) : "d"(port) : "memory");
}
நிலையான உள்வரி வெறுமை outsl(u16 port, u32 *data, u32 மொத்தம்) {
    asm நிலையற்ற("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(மொத்தம்), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
நிலையான உள்வரி வெறுமை smp_rmb(வெறுமை) {
    barrier();
}
நிலையான உள்வரி வெறுமை smp_wmb(வெறுமை) {
    barrier();
}

நிலையான உள்வரி வெறுமை writel(வெறுமை *addr, u32 val) {
    barrier();
    *(நிலையற்ற u32 *)addr = val;
}
நிலையான உள்வரி வெறுமை writew(வெறுமை *addr, u16 val) {
    barrier();
    *(நிலையற்ற u16 *)addr = val;
}
நிலையான உள்வரி வெறுமை writeb(வெறுமை *addr, u8 val) {
    barrier();
    *(நிலையற்ற u8 *)addr = val;
}
நிலையான உள்வரி u64 readq(மாறிலி வெறுமை *addr) {
    u64 val = *(நிலையற்ற மாறிலி u64 *)addr;
    barrier();
    திரும்பு val;
}
நிலையான உள்வரி u32 readl(மாறிலி வெறுமை *addr) {
    u32 val = *(நிலையற்ற மாறிலி u32 *)addr;
    barrier();
    திரும்பு val;
}
நிலையான உள்வரி u16 readw(மாறிலி வெறுமை *addr) {
    u16 val = *(நிலையற்ற மாறிலி u16 *)addr;
    barrier();
    திரும்பு val;
}
நிலையான உள்வரி u8 readb(மாறிலி வெறுமை *addr) {
    u8 val = *(நிலையற்ற மாறிலி u8 *)addr;
    barrier();
    திரும்பு val;
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

கட்டமைப்பு descloc_s {
    u16 length;
    u32 addr;
} PACKED;

நிலையான உள்வரி வெறுமை sgdt(கட்டமைப்பு descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
நிலையான உள்வரி வெறுமை lgdt(கட்டமைப்பு descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

நிலையான உள்வரி u8 get_a20(வெறுமை) {
    திரும்பு (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

நிலையான உள்வரி u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    எனில் (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    திரும்பு a20_enabled;
}

// x86.c
வெறுமை cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
