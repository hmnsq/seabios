// Basic x86 asm functions.
#اگر_غیر_تعریف_شدہ __X86_H
#تعریف_کرو __X86_H

// CPU flag bitdefs
#تعریف_کرو F_CF (1<<0)
#تعریف_کرو F_ZF (1<<6)
#تعریف_کرو F_IF (1<<9)
#تعریف_کرو F_ID (1<<21)

// CR0 flags
#تعریف_کرو CR0_PG (1<<31) // Paging
#تعریف_کرو CR0_CD (1<<30) // Cache disable
#تعریف_کرو CR0_NW (1<<29) // Not Write-through
#تعریف_کرو CR0_PE (1<<0)  // Protection enable

// PORT_A20 bitdefs
#تعریف_کرو پورٹ_A20 0x0092
#تعریف_کرو A20_ENABLE_ثنائی_ہندسہ 0x02

#اگر_غیر_تعریف_شدہ __ASSEMBLY__

#شامل "types.h" // u32

ساکن اندرونی خالی irq_disable(خالی)
{
    asm متلون("cli": : :"memory");
}

ساکن اندرونی خالی irq_enable(خالی)
{
    asm متلون("sti": : :"memory");
}

ساکن اندرونی u32 save_flags(خالی)
{
    u32 flags;
    asm متلون("pushfl ; popl %0" : "=rm" (flags));
    واپس flags;
}

ساکن اندرونی خالی restore_flags(u32 flags)
{
    asm متلون("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

ساکن اندرونی خالی cpu_relax(خالی)
{
    asm متلون("rep ; nop": : :"memory");
}

ساکن اندرونی خالی nop(خالی)
{
    asm متلون("nop");
}

ساکن اندرونی خالی hlt(خالی)
{
    asm متلون("hlt": : :"memory");
}

ساکن اندرونی خالی wbinvd(خالی)
{
    asm متلون("wbinvd": : :"memory");
}

#تعریف_کرو CPUID_TSC (1 << 4)
#تعریف_کرو CPUID_MSR (1 << 5)
#تعریف_کرو CPUID_APIC (1 << 9)
#تعریف_کرو CPUID_MTRR (1 << 12)
#تعریف_کرو CPUID_X2APIC (1 << 21)
ساکن اندرونی خالی __cpuid(u32 اشاریہ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (اشاریہ));
}

ساکن اندرونی u32 cr0_read(خالی) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    واپس cr0;
}
ساکن اندرونی خالی cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
ساکن اندرونی خالی cr0_نقاب(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
ساکن اندرونی u16 cr0_vm86_read(خالی) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    واپس cr0;
}

ساکن اندرونی u64 rdmsr(u32 اشاریہ)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(اشاریہ));
    واپس ret;
}

ساکن اندرونی خالی wrmsr(u32 اشاریہ, u64 val)
{
    asm متلون ("wrmsr" : : "c"(اشاریہ), "A"(val));
}

ساکن اندرونی u64 rdtscll(خالی)
{
    u64 val;
    asm متلون("rdtsc" : "=A" (val));
    واپس val;
}

ساکن اندرونی u32 __ffs(u32 لفظ)
{
    asm("bsf %1,%0"
        : "=r" (لفظ)
        : "rm" (لفظ));
    واپس لفظ;
}
ساکن اندرونی u32 __fls(u32 لفظ)
{
    asm("bsr %1,%0"
        : "=r" (لفظ)
        : "rm" (لفظ));
    واپس لفظ;
}

ساکن اندرونی u32 getesp(خالی) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    واپس esp;
}

ساکن اندرونی u32 rol(u32 val, u16 rol) {
    u32 res;
    asm متلون("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    واپس res;
}

ساکن اندرونی u32 ror(u32 val, u16 ror) {
    u32 res;
    asm متلون("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    واپس res;
}

ساکن اندرونی خالی outb(u8 قدر, u16 پورٹ) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(قدر), "Nd"(پورٹ));
}
ساکن اندرونی خالی outw(u16 قدر, u16 پورٹ) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(قدر), "Nd"(پورٹ));
}
ساکن اندرونی خالی outl(u32 قدر, u16 پورٹ) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(قدر), "Nd"(پورٹ));
}
ساکن اندرونی u8 inb(u16 پورٹ) {
    u8 قدر;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(قدر) : "Nd"(پورٹ));
    واپس قدر;
}
ساکن اندرونی u16 inw(u16 پورٹ) {
    u16 قدر;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(قدر) : "Nd"(پورٹ));
    واپس قدر;
}
ساکن اندرونی u32 inl(u16 پورٹ) {
    u32 قدر;
    __asm__ __volatile__("inl %w1, %0" : "=a"(قدر) : "Nd"(پورٹ));
    واپس قدر;
}

ساکن اندرونی خالی insb(u16 پورٹ, u8 *ڈیٹا, u32 گنتی) {
    asm متلون("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(گنتی), "+D"(ڈیٹا) : "d"(پورٹ) : "memory");
}
ساکن اندرونی خالی insw(u16 پورٹ, u16 *ڈیٹا, u32 گنتی) {
    asm متلون("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(گنتی), "+D"(ڈیٹا) : "d"(پورٹ) : "memory");
}
ساکن اندرونی خالی insl(u16 پورٹ, u32 *ڈیٹا, u32 گنتی) {
    asm متلون("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(گنتی), "+D"(ڈیٹا) : "d"(پورٹ) : "memory");
}
// XXX - outs not limited to es segment
ساکن اندرونی خالی outsb(u16 پورٹ, u8 *ڈیٹا, u32 گنتی) {
    asm متلون("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(گنتی), "+S"(ڈیٹا) : "d"(پورٹ) : "memory");
}
ساکن اندرونی خالی outsw(u16 پورٹ, u16 *ڈیٹا, u32 گنتی) {
    asm متلون("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(گنتی), "+S"(ڈیٹا) : "d"(پورٹ) : "memory");
}
ساکن اندرونی خالی outsl(u16 پورٹ, u32 *ڈیٹا, u32 گنتی) {
    asm متلون("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(گنتی), "+S"(ڈیٹا) : "d"(پورٹ) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
ساکن اندرونی خالی smp_rmb(خالی) {
    barrier();
}
ساکن اندرونی خالی smp_wmb(خالی) {
    barrier();
}

ساکن اندرونی خالی writel(خالی *addr, u32 val) {
    barrier();
    *(متلون u32 *)addr = val;
}
ساکن اندرونی خالی writew(خالی *addr, u16 val) {
    barrier();
    *(متلون u16 *)addr = val;
}
ساکن اندرونی خالی writeb(خالی *addr, u8 val) {
    barrier();
    *(متلون u8 *)addr = val;
}
ساکن اندرونی u64 readq(ثابت خالی *addr) {
    u64 val = *(متلون ثابت u64 *)addr;
    barrier();
    واپس val;
}
ساکن اندرونی u32 readl(ثابت خالی *addr) {
    u32 val = *(متلون ثابت u32 *)addr;
    barrier();
    واپس val;
}
ساکن اندرونی u16 readw(ثابت خالی *addr) {
    u16 val = *(متلون ثابت u16 *)addr;
    barrier();
    واپس val;
}
ساکن اندرونی u8 readb(ثابت خالی *addr) {
    u8 val = *(متلون ثابت u8 *)addr;
    barrier();
    واپس val;
}

// GDT bits
#تعریف_کرو GDT_رمز     (0x9bULL << 40) // Code segment - P,R,A bits also set
#تعریف_کرو GDT_ڈیٹا     (0x93ULL << 40) // Data segment - W,A bits also set
#تعریف_کرو GDT_B        (0x1ULL << 54)  // Big flag
#تعریف_کرو GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#تعریف_کرو GDT_بنیاد(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#تعریف_کرو GDT_حد_بندی(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#تعریف_کرو GDT_GRANLIMIT(v) (GDT_G | GDT_حد_بندی((v) >> 12))

ساخت descloc_s {
    u16 لمبائی;
    u32 addr;
} PACKED;

ساکن اندرونی خالی sgdt(ساخت descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
ساکن اندرونی خالی lgdt(ساخت descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

ساکن اندرونی u8 get_a20(خالی) {
    واپس (inb(پورٹ_A20) & A20_ENABLE_ثنائی_ہندسہ) != 0;
}

ساکن اندرونی u8 سیٹ_a20(u8 cond) {
    u8 val = inb(پورٹ_A20), a20_enabled = (val & A20_ENABLE_ثنائی_ہندسہ) != 0;
    اگر (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_ثنائی_ہندسہ, پورٹ_A20);
    واپس a20_enabled;
}

// x86.c
خالی cpuid(u32 اشاریہ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#اگر_ختم // !__ASSEMBLY__

#اگر_ختم // x86.h
