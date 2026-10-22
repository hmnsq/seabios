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

#अन्तर्भाव "types.h" // u32

स्थिर ओळीत शून्य irq_disable(शून्य)
{
    यंत्र अस्थिर("cli": : :"memory");
}

स्थिर ओळीत शून्य irq_enable(शून्य)
{
    यंत्र अस्थिर("sti": : :"memory");
}

स्थिर ओळीत u32 save_flags(शून्य)
{
    u32 flags;
    यंत्र अस्थिर("pushfl ; popl %0" : "=rm" (flags));
    परत flags;
}

स्थिर ओळीत शून्य restore_flags(u32 flags)
{
    यंत्र अस्थिर("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

स्थिर ओळीत शून्य cpu_relax(शून्य)
{
    यंत्र अस्थिर("rep ; nop": : :"memory");
}

स्थिर ओळीत शून्य nop(शून्य)
{
    यंत्र अस्थिर("nop");
}

स्थिर ओळीत शून्य hlt(शून्य)
{
    यंत्र अस्थिर("hlt": : :"memory");
}

स्थिर ओळीत शून्य wbinvd(शून्य)
{
    यंत्र अस्थिर("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
स्थिर ओळीत शून्य __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    यंत्र("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

स्थिर ओळीत u32 cr0_read(शून्य) {
    u32 cr0;
    यंत्र("movl %%cr0, %0" : "=r"(cr0));
    परत cr0;
}
स्थिर ओळीत शून्य cr0_write(u32 cr0) {
    यंत्र("movl %0, %%cr0" : : "r"(cr0));
}
स्थिर ओळीत शून्य cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
स्थिर ओळीत u16 cr0_vm86_read(शून्य) {
    u16 cr0;
    यंत्र("smsww %0" : "=r"(cr0));
    परत cr0;
}

स्थिर ओळीत u64 rdmsr(u32 index)
{
    u64 ret;
    यंत्र ("rdmsr" : "=A"(ret) : "c"(index));
    परत ret;
}

स्थिर ओळीत शून्य wrmsr(u32 index, u64 val)
{
    यंत्र अस्थिर ("wrmsr" : : "c"(index), "A"(val));
}

स्थिर ओळीत u64 rdtscll(शून्य)
{
    u64 val;
    यंत्र अस्थिर("rdtsc" : "=A" (val));
    परत val;
}

स्थिर ओळीत u32 __ffs(u32 word)
{
    यंत्र("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    परत word;
}
स्थिर ओळीत u32 __fls(u32 word)
{
    यंत्र("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    परत word;
}

स्थिर ओळीत u32 getesp(शून्य) {
    u32 esp;
    यंत्र("movl %%esp, %0" : "=rm"(esp));
    परत esp;
}

स्थिर ओळीत u32 rol(u32 val, u16 rol) {
    u32 res;
    यंत्र अस्थिर("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    परत res;
}

स्थिर ओळीत u32 ror(u32 val, u16 ror) {
    u32 res;
    यंत्र अस्थिर("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    परत res;
}

स्थिर ओळीत शून्य outb(u8 मूल्य, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(मूल्य), "Nd"(port));
}
स्थिर ओळीत शून्य outw(u16 मूल्य, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(मूल्य), "Nd"(port));
}
स्थिर ओळीत शून्य outl(u32 मूल्य, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(मूल्य), "Nd"(port));
}
स्थिर ओळीत u8 inb(u16 port) {
    u8 मूल्य;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(मूल्य) : "Nd"(port));
    परत मूल्य;
}
स्थिर ओळीत u16 inw(u16 port) {
    u16 मूल्य;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(मूल्य) : "Nd"(port));
    परत मूल्य;
}
स्थिर ओळीत u32 inl(u16 port) {
    u32 मूल्य;
    __asm__ __volatile__("inl %w1, %0" : "=a"(मूल्य) : "Nd"(port));
    परत मूल्य;
}

स्थिर ओळीत शून्य insb(u16 port, u8 *data, u32 मोजणी) {
    यंत्र अस्थिर("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(मोजणी), "+D"(data) : "d"(port) : "memory");
}
स्थिर ओळीत शून्य insw(u16 port, u16 *data, u32 मोजणी) {
    यंत्र अस्थिर("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(मोजणी), "+D"(data) : "d"(port) : "memory");
}
स्थिर ओळीत शून्य insl(u16 port, u32 *data, u32 मोजणी) {
    यंत्र अस्थिर("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(मोजणी), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
स्थिर ओळीत शून्य outsb(u16 port, u8 *data, u32 मोजणी) {
    यंत्र अस्थिर("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(मोजणी), "+S"(data) : "d"(port) : "memory");
}
स्थिर ओळीत शून्य outsw(u16 port, u16 *data, u32 मोजणी) {
    यंत्र अस्थिर("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(मोजणी), "+S"(data) : "d"(port) : "memory");
}
स्थिर ओळीत शून्य outsl(u16 port, u32 *data, u32 मोजणी) {
    यंत्र अस्थिर("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(मोजणी), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
स्थिर ओळीत शून्य smp_rmb(शून्य) {
    barrier();
}
स्थिर ओळीत शून्य smp_wmb(शून्य) {
    barrier();
}

स्थिर ओळीत शून्य writel(शून्य *addr, u32 val) {
    barrier();
    *(अस्थिर u32 *)addr = val;
}
स्थिर ओळीत शून्य writew(शून्य *addr, u16 val) {
    barrier();
    *(अस्थिर u16 *)addr = val;
}
स्थिर ओळीत शून्य writeb(शून्य *addr, u8 val) {
    barrier();
    *(अस्थिर u8 *)addr = val;
}
स्थिर ओळीत u64 readq(स्थिरांक शून्य *addr) {
    u64 val = *(अस्थिर स्थिरांक u64 *)addr;
    barrier();
    परत val;
}
स्थिर ओळीत u32 readl(स्थिरांक शून्य *addr) {
    u32 val = *(अस्थिर स्थिरांक u32 *)addr;
    barrier();
    परत val;
}
स्थिर ओळीत u16 readw(स्थिरांक शून्य *addr) {
    u16 val = *(अस्थिर स्थिरांक u16 *)addr;
    barrier();
    परत val;
}
स्थिर ओळीत u8 readb(स्थिरांक शून्य *addr) {
    u8 val = *(अस्थिर स्थिरांक u8 *)addr;
    barrier();
    परत val;
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

संरचना descloc_s {
    u16 length;
    u32 addr;
} PACKED;

स्थिर ओळीत शून्य sgdt(संरचना descloc_s *desc) {
    यंत्र("sgdtl %0" : "=m"(*desc));
}
स्थिर ओळीत शून्य lgdt(संरचना descloc_s *desc) {
    यंत्र("lgdtl %0" : : "m"(*desc) : "memory");
}

स्थिर ओळीत u8 get_a20(शून्य) {
    परत (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

स्थिर ओळीत u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    जर (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    परत a20_enabled;
}

// x86.c
शून्य cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
