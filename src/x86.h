// Basic x86 asm functions.
#که_نه_تعریف_شوی __X86_H
#تعریف_کړه __X86_H

// CPU flag bitdefs
#تعریف_کړه F_CF (1<<0)
#تعریف_کړه F_ZF (1<<6)
#تعریف_کړه F_IF (1<<9)
#تعریف_کړه F_ID (1<<21)

// CR0 flags
#تعریف_کړه CR0_PG (1<<31) // Paging
#تعریف_کړه CR0_CD (1<<30) // Cache disable
#تعریف_کړه CR0_NW (1<<29) // Not Write-through
#تعریف_کړه CR0_PE (1<<0)  // Protection enable

// PORT_A20 bitdefs
#تعریف_کړه پورټ_A20 0x0092
#تعریف_کړه A20_ENABLE_بټ 0x02

#که_نه_تعریف_شوی __ASSEMBLY__

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

#تعریف_کړه CPUID_TSC (1 << 4)
#تعریف_کړه CPUID_MSR (1 << 5)
#تعریف_کړه CPUID_APIC (1 << 9)
#تعریف_کړه CPUID_MTRR (1 << 12)
#تعریف_کړه CPUID_X2APIC (1 << 21)
ساكن مضمن فراغ __cpuid(u32 شاخص, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (شاخص));
}

ساكن مضمن u32 cr0_read(فراغ) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    أرجع cr0;
}
ساكن مضمن فراغ cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
ساكن مضمن فراغ cr0_ماسک(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
ساكن مضمن u16 cr0_vm86_read(فراغ) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    أرجع cr0;
}

ساكن مضمن u64 rdmsr(u32 شاخص)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(شاخص));
    أرجع ret;
}

ساكن مضمن فراغ wrmsr(u32 شاخص, u64 val)
{
    asm متطاير ("wrmsr" : : "c"(شاخص), "A"(val));
}

ساكن مضمن u64 rdtscll(فراغ)
{
    u64 val;
    asm متطاير("rdtsc" : "=A" (val));
    أرجع val;
}

ساكن مضمن u32 __ffs(u32 کلمه)
{
    asm("bsf %1,%0"
        : "=r" (کلمه)
        : "rm" (کلمه));
    أرجع کلمه;
}
ساكن مضمن u32 __fls(u32 کلمه)
{
    asm("bsr %1,%0"
        : "=r" (کلمه)
        : "rm" (کلمه));
    أرجع کلمه;
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

ساكن مضمن فراغ outb(u8 ارزښت, u16 پورټ) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(ارزښت), "Nd"(پورټ));
}
ساكن مضمن فراغ outw(u16 ارزښت, u16 پورټ) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(ارزښت), "Nd"(پورټ));
}
ساكن مضمن فراغ outl(u32 ارزښت, u16 پورټ) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(ارزښت), "Nd"(پورټ));
}
ساكن مضمن u8 inb(u16 پورټ) {
    u8 ارزښت;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(ارزښت) : "Nd"(پورټ));
    أرجع ارزښت;
}
ساكن مضمن u16 inw(u16 پورټ) {
    u16 ارزښت;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(ارزښت) : "Nd"(پورټ));
    أرجع ارزښت;
}
ساكن مضمن u32 inl(u16 پورټ) {
    u32 ارزښت;
    __asm__ __volatile__("inl %w1, %0" : "=a"(ارزښت) : "Nd"(پورټ));
    أرجع ارزښت;
}

ساكن مضمن فراغ insb(u16 پورټ, u8 *معلومات, u32 شمېر) {
    asm متطاير("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(شمېر), "+D"(معلومات) : "d"(پورټ) : "memory");
}
ساكن مضمن فراغ insw(u16 پورټ, u16 *معلومات, u32 شمېر) {
    asm متطاير("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(شمېر), "+D"(معلومات) : "d"(پورټ) : "memory");
}
ساكن مضمن فراغ insl(u16 پورټ, u32 *معلومات, u32 شمېر) {
    asm متطاير("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(شمېر), "+D"(معلومات) : "d"(پورټ) : "memory");
}
// XXX - outs not limited to es segment
ساكن مضمن فراغ outsb(u16 پورټ, u8 *معلومات, u32 شمېر) {
    asm متطاير("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(شمېر), "+S"(معلومات) : "d"(پورټ) : "memory");
}
ساكن مضمن فراغ outsw(u16 پورټ, u16 *معلومات, u32 شمېر) {
    asm متطاير("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(شمېر), "+S"(معلومات) : "d"(پورټ) : "memory");
}
ساكن مضمن فراغ outsl(u16 پورټ, u32 *معلومات, u32 شمېر) {
    asm متطاير("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(شمېر), "+S"(معلومات) : "d"(پورټ) : "memory");
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
#تعریف_کړه GDT_کوډ     (0x9bULL << 40) // Code segment - P,R,A bits also set
#تعریف_کړه GDT_معلومات     (0x93ULL << 40) // Data segment - W,A bits also set
#تعریف_کړه GDT_B        (0x1ULL << 54)  // Big flag
#تعریف_کړه GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#تعریف_کړه GDT_بنسټ(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#تعریف_کړه GDT_حد(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#تعریف_کړه GDT_GRANLIMIT(v) (GDT_G | GDT_حد((v) >> 12))

بنية descloc_s {
    u16 اوږدتیا;
    u32 addr;
} PACKED;

ساكن مضمن فراغ sgdt(بنية descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
ساكن مضمن فراغ lgdt(بنية descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

ساكن مضمن u8 get_a20(فراغ) {
    أرجع (inb(پورټ_A20) & A20_ENABLE_بټ) != 0;
}

ساكن مضمن u8 سېټ_a20(u8 cond) {
    u8 val = inb(پورټ_A20), a20_enabled = (val & A20_ENABLE_بټ) != 0;
    إذا (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_بټ, پورټ_A20);
    أرجع a20_enabled;
}

// x86.c
فراغ cpuid(u32 شاخص, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#د_که_پای // !__ASSEMBLY__

#د_که_پای // x86.h
