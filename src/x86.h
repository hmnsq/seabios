// Basic x86 asm functions.
#अगर_अघोषित __X86_H
#घोषणा __X86_H

// CPU flag bitdefs
#घोषणा F_CF (1<<0)
#घोषणा F_ZF (1<<6)
#घोषणा F_IF (1<<9)
#घोषणा F_ID (1<<21)

// CR0 flags
#घोषणा CR0_PG (1<<31) // Paging
#घोषणा CR0_CD (1<<30) // Cache disable
#घोषणा CR0_NW (1<<29) // Not Write-through
#घोषणा CR0_PE (1<<0)  // Protection enable

// PORT_A20 bitdefs
#घोषणा PORT_A20 0x0092
#घोषणा A20_ENABLE_BIT 0x02

#अगर_अघोषित __ASSEMBLY__

#समावेश "types.h" // u32

अटल अंतरभूत व्योम irq_disable(व्योम)
{
    यंत्र अस्थिर("cli": : :"memory");
}

अटल अंतरभूत व्योम irq_enable(व्योम)
{
    यंत्र अस्थिर("sti": : :"memory");
}

अटल अंतरभूत u32 save_flags(व्योम)
{
    u32 flags;
    यंत्र अस्थिर("pushfl ; popl %0" : "=rm" (flags));
    वापस flags;
}

अटल अंतरभूत व्योम restore_flags(u32 flags)
{
    यंत्र अस्थिर("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

अटल अंतरभूत व्योम cpu_relax(व्योम)
{
    यंत्र अस्थिर("rep ; nop": : :"memory");
}

अटल अंतरभूत व्योम nop(व्योम)
{
    यंत्र अस्थिर("nop");
}

अटल अंतरभूत व्योम hlt(व्योम)
{
    यंत्र अस्थिर("hlt": : :"memory");
}

अटल अंतरभूत व्योम wbinvd(व्योम)
{
    यंत्र अस्थिर("wbinvd": : :"memory");
}

#घोषणा CPUID_TSC (1 << 4)
#घोषणा CPUID_MSR (1 << 5)
#घोषणा CPUID_APIC (1 << 9)
#घोषणा CPUID_MTRR (1 << 12)
#घोषणा CPUID_X2APIC (1 << 21)
अटल अंतरभूत व्योम __cpuid(u32 सूचकांक, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    यंत्र("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (सूचकांक));
}

अटल अंतरभूत u32 cr0_read(व्योम) {
    u32 cr0;
    यंत्र("movl %%cr0, %0" : "=r"(cr0));
    वापस cr0;
}
अटल अंतरभूत व्योम cr0_write(u32 cr0) {
    यंत्र("movl %0, %%cr0" : : "r"(cr0));
}
अटल अंतरभूत व्योम cr0_mask(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
अटल अंतरभूत u16 cr0_vm86_read(व्योम) {
    u16 cr0;
    यंत्र("smsww %0" : "=r"(cr0));
    वापस cr0;
}

अटल अंतरभूत u64 rdmsr(u32 सूचकांक)
{
    u64 ret;
    यंत्र ("rdmsr" : "=A"(ret) : "c"(सूचकांक));
    वापस ret;
}

अटल अंतरभूत व्योम wrmsr(u32 सूचकांक, u64 val)
{
    यंत्र अस्थिर ("wrmsr" : : "c"(सूचकांक), "A"(val));
}

अटल अंतरभूत u64 rdtscll(व्योम)
{
    u64 val;
    यंत्र अस्थिर("rdtsc" : "=A" (val));
    वापस val;
}

अटल अंतरभूत u32 __ffs(u32 word)
{
    यंत्र("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    वापस word;
}
अटल अंतरभूत u32 __fls(u32 word)
{
    यंत्र("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    वापस word;
}

अटल अंतरभूत u32 getesp(व्योम) {
    u32 esp;
    यंत्र("movl %%esp, %0" : "=rm"(esp));
    वापस esp;
}

अटल अंतरभूत u32 rol(u32 val, u16 rol) {
    u32 res;
    यंत्र अस्थिर("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    वापस res;
}

अटल अंतरभूत u32 ror(u32 val, u16 ror) {
    u32 res;
    यंत्र अस्थिर("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    वापस res;
}

अटल अंतरभूत व्योम outb(u8 मूल्य, u16 port) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(मूल्य), "Nd"(port));
}
अटल अंतरभूत व्योम outw(u16 मूल्य, u16 port) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(मूल्य), "Nd"(port));
}
अटल अंतरभूत व्योम outl(u32 मूल्य, u16 port) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(मूल्य), "Nd"(port));
}
अटल अंतरभूत u8 inb(u16 port) {
    u8 मूल्य;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(मूल्य) : "Nd"(port));
    वापस मूल्य;
}
अटल अंतरभूत u16 inw(u16 port) {
    u16 मूल्य;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(मूल्य) : "Nd"(port));
    वापस मूल्य;
}
अटल अंतरभूत u32 inl(u16 port) {
    u32 मूल्य;
    __asm__ __volatile__("inl %w1, %0" : "=a"(मूल्य) : "Nd"(port));
    वापस मूल्य;
}

अटल अंतरभूत व्योम insb(u16 port, u8 *तथ्य, u32 गिनती) {
    यंत्र अस्थिर("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(गिनती), "+D"(तथ्य) : "d"(port) : "memory");
}
अटल अंतरभूत व्योम insw(u16 port, u16 *तथ्य, u32 गिनती) {
    यंत्र अस्थिर("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(गिनती), "+D"(तथ्य) : "d"(port) : "memory");
}
अटल अंतरभूत व्योम insl(u16 port, u32 *तथ्य, u32 गिनती) {
    यंत्र अस्थिर("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(गिनती), "+D"(तथ्य) : "d"(port) : "memory");
}
// XXX - outs not limited to es segment
अटल अंतरभूत व्योम outsb(u16 port, u8 *तथ्य, u32 गिनती) {
    यंत्र अस्थिर("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(गिनती), "+S"(तथ्य) : "d"(port) : "memory");
}
अटल अंतरभूत व्योम outsw(u16 port, u16 *तथ्य, u32 गिनती) {
    यंत्र अस्थिर("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(गिनती), "+S"(तथ्य) : "d"(port) : "memory");
}
अटल अंतरभूत व्योम outsl(u16 port, u32 *तथ्य, u32 गिनती) {
    यंत्र अस्थिर("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(गिनती), "+S"(तथ्य) : "d"(port) : "memory");
}

/* Compiler barrier is enough as an x86 CPU does not reorder reads or writes */
अटल अंतरभूत व्योम smp_rmb(व्योम) {
    barrier();
}
अटल अंतरभूत व्योम smp_wmb(व्योम) {
    barrier();
}

अटल अंतरभूत व्योम writel(व्योम *addr, u32 val) {
    barrier();
    *(अस्थिर u32 *)addr = val;
}
अटल अंतरभूत व्योम writew(व्योम *addr, u16 val) {
    barrier();
    *(अस्थिर u16 *)addr = val;
}
अटल अंतरभूत व्योम writeb(व्योम *addr, u8 val) {
    barrier();
    *(अस्थिर u8 *)addr = val;
}
अटल अंतरभूत u64 readq(स्थिर व्योम *addr) {
    u64 val = *(अस्थिर स्थिर u64 *)addr;
    barrier();
    वापस val;
}
अटल अंतरभूत u32 readl(स्थिर व्योम *addr) {
    u32 val = *(अस्थिर स्थिर u32 *)addr;
    barrier();
    वापस val;
}
अटल अंतरभूत u16 readw(स्थिर व्योम *addr) {
    u16 val = *(अस्थिर स्थिर u16 *)addr;
    barrier();
    वापस val;
}
अटल अंतरभूत u8 readb(स्थिर व्योम *addr) {
    u8 val = *(अस्थिर स्थिर u8 *)addr;
    barrier();
    वापस val;
}

// GDT bits
#घोषणा GDT_CODE     (0x9bULL << 40) // Code segment - P,R,A bits also set
#घोषणा GDT_तथ्य     (0x93ULL << 40) // Data segment - W,A bits also set
#घोषणा GDT_B        (0x1ULL << 54)  // Big flag
#घोषणा GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#घोषणा GDT_BASE(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#घोषणा GDT_LIMIT(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#घोषणा GDT_GRANLIMIT(v) (GDT_G | GDT_LIMIT((v) >> 12))

काष्ठा descloc_s {
    u16 लंबाई;
    u32 addr;
} PACKED;

अटल अंतरभूत व्योम sgdt(काष्ठा descloc_s *desc) {
    यंत्र("sgdtl %0" : "=m"(*desc));
}
अटल अंतरभूत व्योम lgdt(काष्ठा descloc_s *desc) {
    यंत्र("lgdtl %0" : : "m"(*desc) : "memory");
}

अटल अंतरभूत u8 get_a20(व्योम) {
    वापस (inb(PORT_A20) & A20_ENABLE_BIT) != 0;
}

अटल अंतरभूत u8 set_a20(u8 cond) {
    u8 val = inb(PORT_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    अगर (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, PORT_A20);
    वापस a20_enabled;
}

// x86.c
व्योम cpuid(u32 सूचकांक, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#पूर्ण_अगर // !__ASSEMBLY__

#पूर्ण_अगर // x86.h
