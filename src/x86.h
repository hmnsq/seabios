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

स्थिर अन्तःस्थ शून्य irq_disable(शून्य)
{
    यंत्र चञ्चल("cli": : :"memory");
}

स्थिर अन्तःस्थ शून्य irq_enable(शून्य)
{
    यंत्र चञ्चल("sti": : :"memory");
}

स्थिर अन्तःस्थ u32 save_flags(शून्य)
{
    u32 flags;
    यंत्र चञ्चल("pushfl ; popl %0" : "=rm" (flags));
    फर्काऊ flags;
}

स्थिर अन्तःस्थ शून्य restore_flags(u32 flags)
{
    यंत्र चञ्चल("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

स्थिर अन्तःस्थ शून्य cpu_relax(शून्य)
{
    यंत्र चञ्चल("rep ; nop": : :"memory");
}

स्थिर अन्तःस्थ शून्य nop(शून्य)
{
    यंत्र चञ्चल("nop");
}

स्थिर अन्तःस्थ शून्य hlt(शून्य)
{
    यंत्र चञ्चल("hlt": : :"memory");
}

स्थिर अन्तःस्थ शून्य wbinvd(शून्य)
{
    यंत्र चञ्चल("wbinvd": : :"memory");
}

#define CPUID_TSC (1 << 4)
#define CPUID_MSR (1 << 5)
#define CPUID_APIC (1 << 9)
#define CPUID_MTRR (1 << 12)
#define CPUID_X2APIC (1 << 21)
स्थिर अन्तःस्थ शून्य __cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    यंत्र("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (index));
}

स्थिर अन्तःस्थ u32 cr0_read(शून्य) {
    u32 cr0;
    यंत्र("movl %%cr0, %0" : "=r"(cr0));
    फर्काऊ cr0;
}
स्थिर अन्तःस्थ शून्य cr0_write(u32 cr0) {
    यंत्र("movl %0, %%cr0" : : "r"(cr0));
}
स्थिर अन्तःस्थ शून्य cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
स्थिर अन्तःस्थ u16 cr0_vm86_read(शून्य) {
    u16 cr0;
    यंत्र("smsww %0" : "=r"(cr0));
    फर्काऊ cr0;
}

स्थिर अन्तःस्थ u64 rdmsr(u32 index)
{
    u64 ret;
    यंत्र ("rdmsr" : "=A"(ret) : "c"(index));
    फर्काऊ ret;
}

स्थिर अन्तःस्थ शून्य wrmsr(u32 index, u64 val)
{
    यंत्र चञ्चल ("wrmsr" : : "c"(index), "A"(val));
}

स्थिर अन्तःस्थ u64 rdtscll(शून्य)
{
    u64 val;
    यंत्र चञ्चल("rdtsc" : "=A" (val));
    फर्काऊ val;
}

स्थिर अन्तःस्थ u32 __ffs(u32 word)
{
    यंत्र("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    फर्काऊ word;
}
स्थिर अन्तःस्थ u32 __fls(u32 word)
{
    यंत्र("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    फर्काऊ word;
}

स्थिर अन्तःस्थ u32 getesp(शून्य) {
    u32 esp;
    यंत्र("movl %%esp, %0" : "=rm"(esp));
    फर्काऊ esp;
}

स्थिर अन्तःस्थ u32 rol(u32 val, u16 rol) {
    u32 res;
    यंत्र चञ्चल("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    फर्काऊ res;
}

स्थिर अन्तःस्थ u32 ror(u32 val, u16 ror) {
    u32 res;
    यंत्र चञ्चल("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    फर्काऊ res;
}

स्थिर अन्तःस्थ शून्य outb(u8 मान, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(मान), "Nd"(port));
}
स्थिर अन्तःस्थ शून्य outw(u16 मान, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(मान), "Nd"(port));
}
स्थिर अन्तःस्थ शून्य outl(u32 मान, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(मान), "Nd"(port));
}
स्थिर अन्तःस्थ u8 inb(u16 port) {
    u8 मान;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(मान) : "Nd"(port));
    फर्काऊ मान;
}
स्थिर अन्तःस्थ u16 inw(u16 port) {
    u16 मान;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(मान) : "Nd"(port));
    फर्काऊ मान;
}
स्थिर अन्तःस्थ u32 inl(u16 port) {
    u32 मान;
    __asm__ __volatile__("inl %w1, %0" : "=a"(मान) : "Nd"(port));
    फर्काऊ मान;
}

स्थिर अन्तःस्थ शून्य insb(u16 port, u8 *data, u32 गन्ती) {
    यंत्र चञ्चल("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(गन्ती), "+D"(data) : "d"(port) : "memory");
}
स्थिर अन्तःस्थ शून्य insw(u16 port, u16 *data, u32 गन्ती) {
    यंत्र चञ्चल("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(गन्ती), "+D"(data) : "d"(port) : "memory");
}
स्थिर अन्तःस्थ शून्य insl(u16 port, u32 *data, u32 गन्ती) {
    यंत्र चञ्चल("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(गन्ती), "+D"(data) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
स्थिर अन्तःस्थ शून्य outsb(u16 port, u8 *data, u32 गन्ती) {
    यंत्र चञ्चल("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(गन्ती), "+S"(data) : "d"(port) : "memory");
}
स्थिर अन्तःस्थ शून्य outsw(u16 port, u16 *data, u32 गन्ती) {
    यंत्र चञ्चल("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(गन्ती), "+S"(data) : "d"(port) : "memory");
}
स्थिर अन्तःस्थ शून्य outsl(u16 port, u32 *data, u32 गन्ती) {
    यंत्र चञ्चल("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(गन्ती), "+S"(data) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
स्थिर अन्तःस्थ शून्य smp_rmb(शून्य) {
    barrier();
}
स्थिर अन्तःस्थ शून्य smp_wmb(शून्य) {
    barrier();
}

स्थिर अन्तःस्थ शून्य writel(शून्य *addr, u32 val) {
    barrier();
    *(चञ्चल u32 *)addr = val;
}
स्थिर अन्तःस्थ शून्य writew(शून्य *addr, u16 val) {
    barrier();
    *(चञ्चल u16 *)addr = val;
}
स्थिर अन्तःस्थ शून्य writeb(शून्य *addr, u8 val) {
    barrier();
    *(चञ्चल u8 *)addr = val;
}
स्थिर अन्तःस्थ u64 readq(अचर शून्य *addr) {
    u64 val = *(चञ्चल अचर u64 *)addr;
    barrier();
    फर्काऊ val;
}
स्थिर अन्तःस्थ u32 readl(अचर शून्य *addr) {
    u32 val = *(चञ्चल अचर u32 *)addr;
    barrier();
    फर्काऊ val;
}
स्थिर अन्तःस्थ u16 readw(अचर शून्य *addr) {
    u16 val = *(चञ्चल अचर u16 *)addr;
    barrier();
    फर्काऊ val;
}
स्थिर अन्तःस्थ u8 readb(अचर शून्य *addr) {
    u8 val = *(चञ्चल अचर u8 *)addr;
    barrier();
    फर्काऊ val;
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

स्थिर अन्तःस्थ शून्य sgdt(संरचना descloc_s *desc) {
    यंत्र("sgdtl %0" : "=m"(*desc));
}
स्थिर अन्तःस्थ शून्य lgdt(संरचना descloc_s *desc) {
    यंत्र("lgdtl %0" : : "m"(*desc) : "memory");
}

स्थिर अन्तःस्थ u8 get_a20(शून्य) {
    फर्काऊ (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

स्थिर अन्तःस्थ u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    यदि (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    फर्काऊ a20_enabled;
}

// x86.c
शून्य cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#endif // !__ASSEMBLY__

#endif // x86.h
