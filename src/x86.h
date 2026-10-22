// Basic x86 asm functions.
#جے_بنا_تعریف __X86_H
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

#جے_بنا_تعریف __ASSEMBLY__

#ضمن "types.h" // u32

ساكن مضمن فراغ irq_disable(فراغ)
{
    asm متطاير("cli": : :"memory");
}

ساكن مضمن فراغ irq_enable(فراغ)
{
    asm متطاير("sti": : :"memory");
}

ساكن مضمن u32 save_flags(فراغ)
{
    u32 flags;
    asm متطاير("pushfl ; popl %0" : "=rm" (flags));
    أرجع flags;
}

ساكن مضمن فراغ restore_flags(u32 flags)
{
    asm متطاير("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

ساكن مضمن فراغ cpu_relax(فراغ)
{
    asm متطاير("rep ; nop": : :"memory");
}

ساكن مضمن فراغ nop(فراغ)
{
    asm متطاير("nop");
}

ساكن مضمن فراغ hlt(فراغ)
{
    asm متطاير("hlt": : :"memory");
}

ساكن مضمن فراغ wbinvd(فراغ)
{
    asm متطاير("wbinvd": : :"memory");
}

#تعریف_کرو CPUID_TSC (1 << 4)
#تعریف_کرو CPUID_MSR (1 << 5)
#تعریف_کرو CPUID_APIC (1 << 9)
#تعریف_کرو CPUID_MTRR (1 << 12)
#تعریف_کرو CPUID_X2APIC (1 << 21)
ساكن مضمن فراغ __cpuid(u32 اشاریہ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (اشاریہ));
}

ساكن مضمن u32 cr0_read(فراغ) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    أرجع cr0;
}
ساكن مضمن فراغ cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
ساكن مضمن فراغ cr0_نقاب(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
ساكن مضمن u16 cr0_vm86_read(فراغ) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    أرجع cr0;
}

ساكن مضمن u64 rdmsr(u32 اشاریہ)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(اشاریہ));
    أرجع ret;
}

ساكن مضمن فراغ wrmsr(u32 اشاریہ, u64 val)
{
    asm متطاير ("wrmsr" : : "c"(اشاریہ), "A"(val));
}

ساكن مضمن u64 rdtscll(فراغ)
{
    u64 val;
    asm متطاير("rdtsc" : "=A" (val));
    أرجع val;
}

ساكن مضمن u32 __ffs(u32 لفظ)
{
    asm("bsf %1,%0"
        : "=r" (لفظ)
        : "rm" (لفظ));
    أرجع لفظ;
}
ساكن مضمن u32 __fls(u32 لفظ)
{
    asm("bsr %1,%0"
        : "=r" (لفظ)
        : "rm" (لفظ));
    أرجع لفظ;
}

ساكن مضمن u32 getesp(فراغ) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    أرجع esp;
}

ساكن مضمن u32 rol(u32 val, u16 rol) {
    u32 res;
    asm متطاير("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    أرجع res;
}

ساكن مضمن u32 ror(u32 val, u16 ror) {
    u32 res;
    asm متطاير("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    أرجع res;
}

ساكن مضمن فراغ outb(u8 قدر, u16 پورٹ) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(قدر), "Nd"(پورٹ));
}
ساكن مضمن فراغ outw(u16 قدر, u16 پورٹ) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(قدر), "Nd"(پورٹ));
}
ساكن مضمن فراغ outl(u32 قدر, u16 پورٹ) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(قدر), "Nd"(پورٹ));
}
ساكن مضمن u8 inb(u16 پورٹ) {
    u8 قدر;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(قدر) : "Nd"(پورٹ));
    أرجع قدر;
}
ساكن مضمن u16 inw(u16 پورٹ) {
    u16 قدر;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(قدر) : "Nd"(پورٹ));
    أرجع قدر;
}
ساكن مضمن u32 inl(u16 پورٹ) {
    u32 قدر;
    __asm__ __volatile__("inl %w1, %0" : "=a"(قدر) : "Nd"(پورٹ));
    أرجع قدر;
}

ساكن مضمن فراغ insb(u16 پورٹ, u8 *انکڑے, u32 گنتی) {
    asm متطاير("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(گنتی), "+D"(انکڑے) : "d"(پورٹ) : "memory");
}
ساكن مضمن فراغ insw(u16 پورٹ, u16 *انکڑے, u32 گنتی) {
    asm متطاير("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(گنتی), "+D"(انکڑے) : "d"(پورٹ) : "memory");
}
ساكن مضمن فراغ insl(u16 پورٹ, u32 *انکڑے, u32 گنتی) {
    asm متطاير("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(گنتی), "+D"(انکڑے) : "d"(پورٹ) : "memory");
}
// XXX - outs not limited to es segment
ساكن مضمن فراغ outsb(u16 پورٹ, u8 *انکڑے, u32 گنتی) {
    asm متطاير("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(گنتی), "+S"(انکڑے) : "d"(پورٹ) : "memory");
}
ساكن مضمن فراغ outsw(u16 پورٹ, u16 *انکڑے, u32 گنتی) {
    asm متطاير("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(گنتی), "+S"(انکڑے) : "d"(پورٹ) : "memory");
}
ساكن مضمن فراغ outsl(u16 پورٹ, u32 *انکڑے, u32 گنتی) {
    asm متطاير("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(گنتی), "+S"(انکڑے) : "d"(پورٹ) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
ساكن مضمن فراغ smp_rmb(فراغ) {
    barrier();
}
ساكن مضمن فراغ smp_wmb(فراغ) {
    barrier();
}

ساكن مضمن فراغ writel(فراغ *addr, u32 val) {
    barrier();
    *(متطاير u32 *)addr = val;
}
ساكن مضمن فراغ writew(فراغ *addr, u16 val) {
    barrier();
    *(متطاير u16 *)addr = val;
}
ساكن مضمن فراغ writeb(فراغ *addr, u8 val) {
    barrier();
    *(متطاير u8 *)addr = val;
}
ساكن مضمن u64 readq(ثابت فراغ *addr) {
    u64 val = *(متطاير ثابت u64 *)addr;
    barrier();
    أرجع val;
}
ساكن مضمن u32 readl(ثابت فراغ *addr) {
    u32 val = *(متطاير ثابت u32 *)addr;
    barrier();
    أرجع val;
}
ساكن مضمن u16 readw(ثابت فراغ *addr) {
    u16 val = *(متطاير ثابت u16 *)addr;
    barrier();
    أرجع val;
}
ساكن مضمن u8 readb(ثابت فراغ *addr) {
    u8 val = *(متطاير ثابت u8 *)addr;
    barrier();
    أرجع val;
}

// GDT bits
#تعریف_کرو GDT_کوڈ     (0x9bULL << 40) // Code segment - P,R,A bits also set
#تعریف_کرو GDT_انکڑے     (0x93ULL << 40) // Data segment - W,A bits also set
#تعریف_کرو GDT_B        (0x1ULL << 54)  // Big flag
#تعریف_کرو GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#تعریف_کرو GDT_ادھار(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#تعریف_کرو GDT_حد_بندی(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#تعریف_کرو GDT_GRANLIMIT(v) (GDT_G | GDT_حد_بندی((v) >> 12))

بنية descloc_s {
    u16 لمائی;
    u32 addr;
} PACKED;

ساكن مضمن فراغ sgdt(بنية descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
ساكن مضمن فراغ lgdt(بنية descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

ساكن مضمن u8 get_a20(فراغ) {
    أرجع (inb(پورٹ_A20) & A20_ENABLE_ثنائی_ہندسہ) != 0;
}

ساكن مضمن u8 سیٹ_a20(u8 cond) {
    u8 val = inb(پورٹ_A20), a20_enabled = (val & A20_ENABLE_ثنائی_ہندسہ) != 0;
    إذا (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_ثنائی_ہندسہ, پورٹ_A20);
    أرجع a20_enabled;
}

// x86.c
فراغ cpuid(u32 اشاریہ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#جے_انت // !__ASSEMBLY__

#جے_انت // x86.h
