// Basic x86 asm functions.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __X86_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __X86_H

// CPU flag bitdefs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F_CF (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F_ZF (1<<6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F_IF (1<<9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F_ID (1<<21)

// CR0 flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CR0_PG (1<<31) // Paging
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CR0_CD (1<<30) // Cache disable
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CR0_NW (1<<29) // Not Write-through
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CR0_PE (1<<0)  // Protection enable

// PORT_A20 bitdefs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_A20 0x0092
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ A20_ENABLE_ਦੋਆਧਾਰੀ_ਅੰਕ 0x02

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ASSEMBLY__

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ irq_disable(ਖਾਲੀ)
{
    asm ਅਸਥਿਰ("cli": : :"memory");
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ irq_enable(ਖਾਲੀ)
{
    asm ਅਸਥਿਰ("sti": : :"memory");
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u32 save_flags(ਖਾਲੀ)
{
    u32 flags;
    asm ਅਸਥਿਰ("pushfl ; popl %0" : "=rm" (flags));
    ਵਾਪਸ flags;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ restore_flags(u32 flags)
{
    asm ਅਸਥਿਰ("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ cpu_relax(ਖਾਲੀ)
{
    asm ਅਸਥਿਰ("rep ; nop": : :"memory");
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ nop(ਖਾਲੀ)
{
    asm ਅਸਥਿਰ("nop");
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ hlt(ਖਾਲੀ)
{
    asm ਅਸਥਿਰ("hlt": : :"memory");
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ wbinvd(ਖਾਲੀ)
{
    asm ਅਸਥਿਰ("wbinvd": : :"memory");
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CPUID_TSC (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CPUID_MSR (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CPUID_APIC (1 << 9)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CPUID_MTRR (1 << 12)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CPUID_X2APIC (1 << 21)
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ __cpuid(u32 ਸੂਚਕਾਂਕ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (ਸੂਚਕਾਂਕ));
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u32 cr0_read(ਖਾਲੀ) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    ਵਾਪਸ cr0;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ cr0_ਨਕਾਬ(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 cr0_vm86_read(ਖਾਲੀ) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    ਵਾਪਸ cr0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u64 rdmsr(u32 ਸੂਚਕਾਂਕ)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(ਸੂਚਕਾਂਕ));
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ wrmsr(u32 ਸੂਚਕਾਂਕ, u64 val)
{
    asm ਅਸਥਿਰ ("wrmsr" : : "c"(ਸੂਚਕਾਂਕ), "A"(val));
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u64 rdtscll(ਖਾਲੀ)
{
    u64 val;
    asm ਅਸਥਿਰ("rdtsc" : "=A" (val));
    ਵਾਪਸ val;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u32 __ffs(u32 ਸ਼ਬਦ)
{
    asm("bsf %1,%0"
        : "=r" (ਸ਼ਬਦ)
        : "rm" (ਸ਼ਬਦ));
    ਵਾਪਸ ਸ਼ਬਦ;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 __fls(u32 ਸ਼ਬਦ)
{
    asm("bsr %1,%0"
        : "=r" (ਸ਼ਬਦ)
        : "rm" (ਸ਼ਬਦ));
    ਵਾਪਸ ਸ਼ਬਦ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u32 getesp(ਖਾਲੀ) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    ਵਾਪਸ esp;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm ਅਸਥਿਰ("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    ਵਾਪਸ res;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm ਅਸਥਿਰ("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    ਵਾਪਸ res;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outb(u8 ਮੁੱਲ, u16 ਪੋਰਟ) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(ਮੁੱਲ), "Nd"(ਪੋਰਟ));
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outw(u16 ਮੁੱਲ, u16 ਪੋਰਟ) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(ਮੁੱਲ), "Nd"(ਪੋਰਟ));
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outl(u32 ਮੁੱਲ, u16 ਪੋਰਟ) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(ਮੁੱਲ), "Nd"(ਪੋਰਟ));
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u8 inb(u16 ਪੋਰਟ) {
    u8 ਮੁੱਲ;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(ਮੁੱਲ) : "Nd"(ਪੋਰਟ));
    ਵਾਪਸ ਮੁੱਲ;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 inw(u16 ਪੋਰਟ) {
    u16 ਮੁੱਲ;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(ਮੁੱਲ) : "Nd"(ਪੋਰਟ));
    ਵਾਪਸ ਮੁੱਲ;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 inl(u16 ਪੋਰਟ) {
    u32 ਮੁੱਲ;
    __asm__ __volatile__("inl %w1, %0" : "=a"(ਮੁੱਲ) : "Nd"(ਪੋਰਟ));
    ਵਾਪਸ ਮੁੱਲ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ insb(u16 ਪੋਰਟ, u8 *ਅੰਕੜੇ, u32 ਗਿਣਤੀ_2) {
    asm ਅਸਥਿਰ("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(ਗਿਣਤੀ_2), "+D"(ਅੰਕੜੇ) : "d"(ਪੋਰਟ) : "memory");
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ insw(u16 ਪੋਰਟ, u16 *ਅੰਕੜੇ, u32 ਗਿਣਤੀ_2) {
    asm ਅਸਥਿਰ("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(ਗਿਣਤੀ_2), "+D"(ਅੰਕੜੇ) : "d"(ਪੋਰਟ) : "memory");
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ insl(u16 ਪੋਰਟ, u32 *ਅੰਕੜੇ, u32 ਗਿਣਤੀ_2) {
    asm ਅਸਥਿਰ("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(ਗਿਣਤੀ_2), "+D"(ਅੰਕੜੇ) : "d"(ਪੋਰਟ) : "memory");
}
// XXX - outs not limited to es segment
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outsb(u16 ਪੋਰਟ, u8 *ਅੰਕੜੇ, u32 ਗਿਣਤੀ_2) {
    asm ਅਸਥਿਰ("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(ਗਿਣਤੀ_2), "+S"(ਅੰਕੜੇ) : "d"(ਪੋਰਟ) : "memory");
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outsw(u16 ਪੋਰਟ, u16 *ਅੰਕੜੇ, u32 ਗਿਣਤੀ_2) {
    asm ਅਸਥਿਰ("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(ਗਿਣਤੀ_2), "+S"(ਅੰਕੜੇ) : "d"(ਪੋਰਟ) : "memory");
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ outsl(u16 ਪੋਰਟ, u32 *ਅੰਕੜੇ, u32 ਗਿਣਤੀ_2) {
    asm ਅਸਥਿਰ("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(ਗਿਣਤੀ_2), "+S"(ਅੰਕੜੇ) : "d"(ਪੋਰਟ) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ smp_rmb(ਖਾਲੀ) {
    barrier();
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ smp_wmb(ਖਾਲੀ) {
    barrier();
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ writel(ਖਾਲੀ *addr, u32 val) {
    barrier();
    *(ਅਸਥਿਰ u32 *)addr = val;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ writew(ਖਾਲੀ *addr, u16 val) {
    barrier();
    *(ਅਸਥਿਰ u16 *)addr = val;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ writeb(ਖਾਲੀ *addr, u8 val) {
    barrier();
    *(ਅਸਥਿਰ u8 *)addr = val;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u64 readq(ਅਚਲ ਖਾਲੀ *addr) {
    u64 val = *(ਅਸਥਿਰ ਅਚਲ u64 *)addr;
    barrier();
    ਵਾਪਸ val;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u32 readl(ਅਚਲ ਖਾਲੀ *addr) {
    u32 val = *(ਅਸਥਿਰ ਅਚਲ u32 *)addr;
    barrier();
    ਵਾਪਸ val;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 readw(ਅਚਲ ਖਾਲੀ *addr) {
    u16 val = *(ਅਸਥਿਰ ਅਚਲ u16 *)addr;
    barrier();
    ਵਾਪਸ val;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u8 readb(ਅਚਲ ਖਾਲੀ *addr) {
    u8 val = *(ਅਸਥਿਰ ਅਚਲ u8 *)addr;
    barrier();
    ਵਾਪਸ val;
}

// GDT bits
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDT_ਕੋਡ     (0x9bULL << 40) // Code segment - P,R,A bits also set
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDT_ਅੰਕੜੇ     (0x93ULL << 40) // Data segment - W,A bits also set
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDT_B        (0x1ULL << 54)  // Big flag
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDT_ਅਧਾਰ(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDT_ਹੱਦਬੰਦੀ(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDT_GRANLIMIT(v) (GDT_G | GDT_ਹੱਦਬੰਦੀ((v) >> 12))

ਬਣਤਰ descloc_s {
    u16 ਲੰਮਾਈ;
    u32 addr;
} PACKED;

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ sgdt(ਬਣਤਰ descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ lgdt(ਬਣਤਰ descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u8 get_a20(ਖਾਲੀ) {
    ਵਾਪਸ (inb(ਪੋਰਟ_A20) & A20_ENABLE_ਦੋਆਧਾਰੀ_ਅੰਕ) != 0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ u8 ਸੈੱਟ_a20(u8 cond) {
    u8 val = inb(ਪੋਰਟ_A20), a20_enabled = (val & A20_ENABLE_ਦੋਆਧਾਰੀ_ਅੰਕ) != 0;
    ਜੇ (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_ਦੋਆਧਾਰੀ_ਅੰਕ, ਪੋਰਟ_A20);
    ਵਾਪਸ a20_enabled;
}

// x86.c
ਖਾਲੀ cpuid(u32 ਸੂਚਕਾਂਕ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#ਜੇ_ਅੰਤ // !__ASSEMBLY__

#ਜੇ_ਅੰਤ // x86.h
