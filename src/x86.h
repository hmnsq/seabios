// Basic x86 asm functions.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __X86_H
#ವ್ಯಾಖ್ಯಾನಿಸು __X86_H

// CPU flag bitdefs
#ವ್ಯಾಖ್ಯಾನಿಸು F_CF (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು F_ZF (1<<6)
#ವ್ಯಾಖ್ಯಾನಿಸು F_IF (1<<9)
#ವ್ಯಾಖ್ಯಾನಿಸು F_ID (1<<21)

// CR0 flags
#ವ್ಯಾಖ್ಯಾನಿಸು CR0_PG (1<<31) // Paging
#ವ್ಯಾಖ್ಯಾನಿಸು CR0_CD (1<<30) // Cache disable
#ವ್ಯಾಖ್ಯಾನಿಸು CR0_NW (1<<29) // Not Write-through
#ವ್ಯಾಖ್ಯಾನಿಸು CR0_PE (1<<0)  // Protection enable

// PORT_A20 bitdefs
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_A20 0x0092
#ವ್ಯಾಖ್ಯಾನಿಸು A20_ENABLE_ಬಿಟ್ 0x02

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ASSEMBLY__

#ಅನ್ತರ್ಭಾವ "types.h" // u32

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ irq_disable(ಶೂನ್ಯ)
{
    asm ಅಸ್ಥಿರ("cli": : :"memory");
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ irq_enable(ಶೂನ್ಯ)
{
    asm ಅಸ್ಥಿರ("sti": : :"memory");
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 save_flags(ಶೂನ್ಯ)
{
    u32 flags;
    asm ಅಸ್ಥಿರ("pushfl ; popl %0" : "=rm" (flags));
    ಹಿಂತಿರುಗಿ flags;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ restore_flags(u32 flags)
{
    asm ಅಸ್ಥಿರ("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ cpu_relax(ಶೂನ್ಯ)
{
    asm ಅಸ್ಥಿರ("rep ; nop": : :"memory");
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ nop(ಶೂನ್ಯ)
{
    asm ಅಸ್ಥಿರ("nop");
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ hlt(ಶೂನ್ಯ)
{
    asm ಅಸ್ಥಿರ("hlt": : :"memory");
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ wbinvd(ಶೂನ್ಯ)
{
    asm ಅಸ್ಥಿರ("wbinvd": : :"memory");
}

#ವ್ಯಾಖ್ಯಾನಿಸು CPUID_TSC (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು CPUID_MSR (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು CPUID_APIC (1 << 9)
#ವ್ಯಾಖ್ಯಾನಿಸು CPUID_MTRR (1 << 12)
#ವ್ಯಾಖ್ಯಾನಿಸು CPUID_X2APIC (1 << 21)
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ __cpuid(u32 ಸೂಚ್ಯಂಕ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (ಸೂಚ್ಯಂಕ));
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 cr0_read(ಶೂನ್ಯ) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    ಹಿಂತಿರುಗಿ cr0;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ cr0_ಮುಖವಾಡ(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 cr0_vm86_read(ಶೂನ್ಯ) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    ಹಿಂತಿರುಗಿ cr0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 rdmsr(u32 ಸೂಚ್ಯಂಕ)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(ಸೂಚ್ಯಂಕ));
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ wrmsr(u32 ಸೂಚ್ಯಂಕ, u64 val)
{
    asm ಅಸ್ಥಿರ ("wrmsr" : : "c"(ಸೂಚ್ಯಂಕ), "A"(val));
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 rdtscll(ಶೂನ್ಯ)
{
    u64 val;
    asm ಅಸ್ಥಿರ("rdtsc" : "=A" (val));
    ಹಿಂತಿರುಗಿ val;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 __ffs(u32 ಶಬ್ದ)
{
    asm("bsf %1,%0"
        : "=r" (ಶಬ್ದ)
        : "rm" (ಶಬ್ದ));
    ಹಿಂತಿರುಗಿ ಶಬ್ದ;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 __fls(u32 ಶಬ್ದ)
{
    asm("bsr %1,%0"
        : "=r" (ಶಬ್ದ)
        : "rm" (ಶಬ್ದ));
    ಹಿಂತಿರುಗಿ ಶಬ್ದ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 getesp(ಶೂನ್ಯ) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    ಹಿಂತಿರುಗಿ esp;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm ಅಸ್ಥಿರ("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    ಹಿಂತಿರುಗಿ res;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm ಅಸ್ಥಿರ("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    ಹಿಂತಿರುಗಿ res;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outb(u8 ಮೌಲ್ಯ, u16 ಪೋರ್ಟ್) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(ಮೌಲ್ಯ), "Nd"(ಪೋರ್ಟ್));
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outw(u16 ಮೌಲ್ಯ, u16 ಪೋರ್ಟ್) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(ಮೌಲ್ಯ), "Nd"(ಪೋರ್ಟ್));
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outl(u32 ಮೌಲ್ಯ, u16 ಪೋರ್ಟ್) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(ಮೌಲ್ಯ), "Nd"(ಪೋರ್ಟ್));
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8 inb(u16 ಪೋರ್ಟ್) {
    u8 ಮೌಲ್ಯ;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(ಮೌಲ್ಯ) : "Nd"(ಪೋರ್ಟ್));
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 inw(u16 ಪೋರ್ಟ್) {
    u16 ಮೌಲ್ಯ;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(ಮೌಲ್ಯ) : "Nd"(ಪೋರ್ಟ್));
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 inl(u16 ಪೋರ್ಟ್) {
    u32 ಮೌಲ್ಯ;
    __asm__ __volatile__("inl %w1, %0" : "=a"(ಮೌಲ್ಯ) : "Nd"(ಪೋರ್ಟ್));
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ insb(u16 ಪೋರ್ಟ್, u8 *ದತ್ತಾಂಶ, u32 ಎಣಿಕೆ_2) {
    asm ಅಸ್ಥಿರ("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(ಎಣಿಕೆ_2), "+D"(ದತ್ತಾಂಶ) : "d"(ಪೋರ್ಟ್) : "memory");
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ insw(u16 ಪೋರ್ಟ್, u16 *ದತ್ತಾಂಶ, u32 ಎಣಿಕೆ_2) {
    asm ಅಸ್ಥಿರ("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(ಎಣಿಕೆ_2), "+D"(ದತ್ತಾಂಶ) : "d"(ಪೋರ್ಟ್) : "memory");
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ insl(u16 ಪೋರ್ಟ್, u32 *ದತ್ತಾಂಶ, u32 ಎಣಿಕೆ_2) {
    asm ಅಸ್ಥಿರ("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(ಎಣಿಕೆ_2), "+D"(ದತ್ತಾಂಶ) : "d"(ಪೋರ್ಟ್) : "memory");
}
// XXX - outs not limited to es segment
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outsb(u16 ಪೋರ್ಟ್, u8 *ದತ್ತಾಂಶ, u32 ಎಣಿಕೆ_2) {
    asm ಅಸ್ಥಿರ("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(ಎಣಿಕೆ_2), "+S"(ದತ್ತಾಂಶ) : "d"(ಪೋರ್ಟ್) : "memory");
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outsw(u16 ಪೋರ್ಟ್, u16 *ದತ್ತಾಂಶ, u32 ಎಣಿಕೆ_2) {
    asm ಅಸ್ಥಿರ("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(ಎಣಿಕೆ_2), "+S"(ದತ್ತಾಂಶ) : "d"(ಪೋರ್ಟ್) : "memory");
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ outsl(u16 ಪೋರ್ಟ್, u32 *ದತ್ತಾಂಶ, u32 ಎಣಿಕೆ_2) {
    asm ಅಸ್ಥಿರ("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(ಎಣಿಕೆ_2), "+S"(ದತ್ತಾಂಶ) : "d"(ಪೋರ್ಟ್) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ smp_rmb(ಶೂನ್ಯ) {
    barrier();
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ smp_wmb(ಶೂನ್ಯ) {
    barrier();
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ writel(ಶೂನ್ಯ *addr, u32 val) {
    barrier();
    *(ಅಸ್ಥಿರ u32 *)addr = val;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ writew(ಶೂನ್ಯ *addr, u16 val) {
    barrier();
    *(ಅಸ್ಥಿರ u16 *)addr = val;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ writeb(ಶೂನ್ಯ *addr, u8 val) {
    barrier();
    *(ಅಸ್ಥಿರ u8 *)addr = val;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u64 readq(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *addr) {
    u64 val = *(ಅಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u64 *)addr;
    barrier();
    ಹಿಂತಿರುಗಿ val;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 readl(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *addr) {
    u32 val = *(ಅಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u32 *)addr;
    barrier();
    ಹಿಂತಿರುಗಿ val;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 readw(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *addr) {
    u16 val = *(ಅಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u16 *)addr;
    barrier();
    ಹಿಂತಿರುಗಿ val;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8 readb(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *addr) {
    u8 val = *(ಅಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 *)addr;
    barrier();
    ಹಿಂತಿರುಗಿ val;
}

// GDT bits
#ವ್ಯಾಖ್ಯಾನಿಸು GDT_ಸಂಕೇತ     (0x9bULL << 40) // Code segment - P,R,A bits also set
#ವ್ಯಾಖ್ಯಾನಿಸು GDT_ದತ್ತಾಂಶ     (0x93ULL << 40) // Data segment - W,A bits also set
#ವ್ಯಾಖ್ಯಾನಿಸು GDT_B        (0x1ULL << 54)  // Big flag
#ವ್ಯಾಖ್ಯಾನಿಸು GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#ವ್ಯಾಖ್ಯಾನಿಸು GDT_ತಳ(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#ವ್ಯಾಖ್ಯಾನಿಸು GDT_ಎಲ್ಲೆ(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#ವ್ಯಾಖ್ಯಾನಿಸು GDT_GRANLIMIT(v) (GDT_G | GDT_ಎಲ್ಲೆ((v) >> 12))

ರಚನೆ descloc_s {
    u16 ಉದ್ದಳತೆ;
    u32 addr;
} PACKED;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ sgdt(ರಚನೆ descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ lgdt(ರಚನೆ descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8 get_a20(ಶೂನ್ಯ) {
    ಹಿಂತಿರುಗಿ (inb(ಪೋರ್ಟ್_A20) & A20_ENABLE_ಬಿಟ್) != 0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8 ಗಣ_a20(u8 cond) {
    u8 val = inb(ಪೋರ್ಟ್_A20), a20_enabled = (val & A20_ENABLE_ಬಿಟ್) != 0;
    ಆದರೆ (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_ಬಿಟ್, ಪೋರ್ಟ್_A20);
    ಹಿಂತಿರುಗಿ a20_enabled;
}

// x86.c
ಶೂನ್ಯ cpuid(u32 ಸೂಚ್ಯಂಕ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#ಆದರೆ_ಅಂತ್ಯ // !__ASSEMBLY__

#ಆದರೆ_ಅಂತ್ಯ // x86.h
