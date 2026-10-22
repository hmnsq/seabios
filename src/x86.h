// Basic x86 asm functions.
#यदि_अपरिभाषितम् __X86_H
#परिभाषय __X86_H

// CPU flag bitdefs
#परिभाषय F_CF (1<<0)
#परिभाषय F_ZF (1<<6)
#परिभाषय F_IF (1<<9)
#परिभाषय F_ID (1<<21)

// CR0 flags
#परिभाषय CR0_PG (1<<31) // Paging
#परिभाषय CR0_CD (1<<30) // Cache disable
#परिभाषय CR0_NW (1<<29) // Not Write-through
#परिभाषय CR0_PE (1<<0)  // Protection enable

// PORT_A20 bitdefs
#परिभाषय पत्तनम्_A20 0x0092
#परिभाषय A20_ENABLE_BIT 0x02

#यदि_अपरिभाषितम् __ASSEMBLY__

#अन्तर्भाव "types.h" // u32

स्थिर अन्तःस्थ शून्य irq_disable(शून्य)
{
    asm चञ्चल("cli": : :"memory");
}

स्थिर अन्तःस्थ शून्य irq_enable(शून्य)
{
    asm चञ्चल("sti": : :"memory");
}

स्थिर अन्तःस्थ u32 save_flags(शून्य)
{
    u32 flags;
    asm चञ्चल("pushfl ; popl %0" : "=rm" (flags));
    प्रत्यागम flags;
}

स्थिर अन्तःस्थ शून्य restore_flags(u32 flags)
{
    asm चञ्चल("pushl %0 ; popfl" : : "g" (flags) : "memory", "cc");
}

स्थिर अन्तःस्थ शून्य cpu_relax(शून्य)
{
    asm चञ्चल("rep ; nop": : :"memory");
}

स्थिर अन्तःस्थ शून्य nop(शून्य)
{
    asm चञ्चल("nop");
}

स्थिर अन्तःस्थ शून्य hlt(शून्य)
{
    asm चञ्चल("hlt": : :"memory");
}

स्थिर अन्तःस्थ शून्य wbinvd(शून्य)
{
    asm चञ्चल("wbinvd": : :"memory");
}

#परिभाषय CPUID_TSC (1 << 4)
#परिभाषय CPUID_MSR (1 << 5)
#परिभाषय CPUID_APIC (1 << 9)
#परिभाषय CPUID_MTRR (1 << 12)
#परिभाषय CPUID_X2APIC (1 << 21)
स्थिर अन्तःस्थ शून्य __cpuid(u32 सूचकांक, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    asm("cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "0" (सूचकांक));
}

स्थिर अन्तःस्थ u32 cr0_read(शून्य) {
    u32 cr0;
    asm("movl %%cr0, %0" : "=r"(cr0));
    प्रत्यागम cr0;
}
स्थिर अन्तःस्थ शून्य cr0_write(u32 cr0) {
    asm("movl %0, %%cr0" : : "r"(cr0));
}
स्थिर अन्तःस्थ शून्य cr0_आवरणम्(u32 off, u32 on) {
    cr0_write((cr0_read() & ~off) | on);
}
स्थिर अन्तःस्थ u16 cr0_vm86_read(शून्य) {
    u16 cr0;
    asm("smsww %0" : "=r"(cr0));
    प्रत्यागम cr0;
}

स्थिर अन्तःस्थ u64 rdmsr(u32 सूचकांक)
{
    u64 ret;
    asm ("rdmsr" : "=A"(ret) : "c"(सूचकांक));
    प्रत्यागम ret;
}

स्थिर अन्तःस्थ शून्य wrmsr(u32 सूचकांक, u64 val)
{
    asm चञ्चल ("wrmsr" : : "c"(सूचकांक), "A"(val));
}

स्थिर अन्तःस्थ u64 rdtscll(शून्य)
{
    u64 val;
    asm चञ्चल("rdtsc" : "=A" (val));
    प्रत्यागम val;
}

स्थिर अन्तःस्थ u32 __ffs(u32 word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    प्रत्यागम word;
}
स्थिर अन्तःस्थ u32 __fls(u32 word)
{
    asm("bsr %1,%0"
        : "=r" (word)
        : "rm" (word));
    प्रत्यागम word;
}

स्थिर अन्तःस्थ u32 getesp(शून्य) {
    u32 esp;
    asm("movl %%esp, %0" : "=rm"(esp));
    प्रत्यागम esp;
}

स्थिर अन्तःस्थ u32 rol(u32 val, u16 rol) {
    u32 res;
    asm चञ्चल("roll %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (rol));
    प्रत्यागम res;
}

स्थिर अन्तःस्थ u32 ror(u32 val, u16 ror) {
    u32 res;
    asm चञ्चल("rorl %%cl, %%eax"
                 : "=a" (res) : "a" (val), "c" (ror));
    प्रत्यागम res;
}

स्थिर अन्तःस्थ शून्य outb(u8 मानम्, u16 पत्तनम्) {
    __asm__ __volatile__("outb %b0, %w1" : : "a"(मानम्), "Nd"(पत्तनम्));
}
स्थिर अन्तःस्थ शून्य outw(u16 मानम्, u16 पत्तनम्) {
    __asm__ __volatile__("outw %w0, %w1" : : "a"(मानम्), "Nd"(पत्तनम्));
}
स्थिर अन्तःस्थ शून्य outl(u32 मानम्, u16 पत्तनम्) {
    __asm__ __volatile__("outl %0, %w1" : : "a"(मानम्), "Nd"(पत्तनम्));
}
स्थिर अन्तःस्थ u8 inb(u16 पत्तनम्) {
    u8 मानम्;
    __asm__ __volatile__("inb %w1, %b0" : "=a"(मानम्) : "Nd"(पत्तनम्));
    प्रत्यागम मानम्;
}
स्थिर अन्तःस्थ u16 inw(u16 पत्तनम्) {
    u16 मानम्;
    __asm__ __volatile__("inw %w1, %w0" : "=a"(मानम्) : "Nd"(पत्तनम्));
    प्रत्यागम मानम्;
}
स्थिर अन्तःस्थ u32 inl(u16 पत्तनम्) {
    u32 मानम्;
    __asm__ __volatile__("inl %w1, %0" : "=a"(मानम्) : "Nd"(पत्तनम्));
    प्रत्यागम मानम्;
}

स्थिर अन्तःस्थ शून्य insb(u16 पत्तनम्, u8 *दत्त, u32 गणनम्) {
    asm चञ्चल("rep insb (%%dx), %%es:(%%edi)"
                 : "+c"(गणनम्), "+D"(दत्त) : "d"(पत्तनम्) : "memory");
}
स्थिर अन्तःस्थ शून्य insw(u16 पत्तनम्, u16 *दत्त, u32 गणनम्) {
    asm चञ्चल("rep insw (%%dx), %%es:(%%edi)"
                 : "+c"(गणनम्), "+D"(दत्त) : "d"(पत्तनम्) : "memory");
}
स्थिर अन्तःस्थ शून्य insl(u16 पत्तनम्, u32 *दत्त, u32 गणनम्) {
    asm चञ्चल("rep insl (%%dx), %%es:(%%edi)"
                 : "+c"(गणनम्), "+D"(दत्त) : "d"(पत्तनम्) : "memory");
}
// XXX - outs not limited to es segment
स्थिर अन्तःस्थ शून्य outsb(u16 पत्तनम्, u8 *दत्त, u32 गणनम्) {
    asm चञ्चल("rep outsb %%es:(%%esi), (%%dx)"
                 : "+c"(गणनम्), "+S"(दत्त) : "d"(पत्तनम्) : "memory");
}
स्थिर अन्तःस्थ शून्य outsw(u16 पत्तनम्, u16 *दत्त, u32 गणनम्) {
    asm चञ्चल("rep outsw %%es:(%%esi), (%%dx)"
                 : "+c"(गणनम्), "+S"(दत्त) : "d"(पत्तनम्) : "memory");
}
स्थिर अन्तःस्थ शून्य outsl(u16 पत्तनम्, u32 *दत्त, u32 गणनम्) {
    asm चञ्चल("rep outsl %%es:(%%esi), (%%dx)"
                 : "+c"(गणनम्), "+S"(दत्त) : "d"(पत्तनम्) : "memory");
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
स्थिर अन्तःस्थ u64 readq(ध्रुव शून्य *addr) {
    u64 val = *(चञ्चल ध्रुव u64 *)addr;
    barrier();
    प्रत्यागम val;
}
स्थिर अन्तःस्थ u32 readl(ध्रुव शून्य *addr) {
    u32 val = *(चञ्चल ध्रुव u32 *)addr;
    barrier();
    प्रत्यागम val;
}
स्थिर अन्तःस्थ u16 readw(ध्रुव शून्य *addr) {
    u16 val = *(चञ्चल ध्रुव u16 *)addr;
    barrier();
    प्रत्यागम val;
}
स्थिर अन्तःस्थ u8 readb(ध्रुव शून्य *addr) {
    u8 val = *(चञ्चल ध्रुव u8 *)addr;
    barrier();
    प्रत्यागम val;
}

// GDT bits
#परिभाषय GDT_संहिता     (0x9bULL << 40) // Code segment - P,R,A bits also set
#परिभाषय GDT_दत्त     (0x93ULL << 40) // Data segment - W,A bits also set
#परिभाषय GDT_B        (0x1ULL << 54)  // Big flag
#परिभाषय GDT_G        (0x1ULL << 55)  // Granularity flag
// GDT bits for segment base
#परिभाषय GDT_आधारः(v)  ((((u64)(v) & 0xff000000) << 32)           \
                      | (((u64)(v) & 0x00ffffff) << 16))
// GDT bits for segment limit (0-1Meg)
#परिभाषय GDT_सीमा(v) ((((u64)(v) & 0x000f0000) << 32)   \
                      | (((u64)(v) & 0x0000ffff) << 0))
// GDT bits for segment limit (0-4Gig in 4K chunks)
#परिभाषय GDT_GRANLIMIT(v) (GDT_G | GDT_सीमा((v) >> 12))

संरचना descloc_s {
    u16 दीर्घता;
    u32 addr;
} PACKED;

स्थिर अन्तःस्थ शून्य sgdt(संरचना descloc_s *desc) {
    asm("sgdtl %0" : "=m"(*desc));
}
स्थिर अन्तःस्थ शून्य lgdt(संरचना descloc_s *desc) {
    asm("lgdtl %0" : : "m"(*desc) : "memory");
}

स्थिर अन्तःस्थ u8 get_a20(शून्य) {
    प्रत्यागम (inb(पत्तनम्_A20) & A20_ENABLE_BIT) != 0;
}

स्थिर अन्तःस्थ u8 समुच्चयः_a20(u8 cond) {
    u8 val = inb(पत्तनम्_A20), a20_enabled = (val & A20_ENABLE_BIT) != 0;
    यदि (a20_enabled != !!cond)
        outb(val ^ A20_ENABLE_BIT, पत्तनम्_A20);
    प्रत्यागम a20_enabled;
}

// x86.c
शून्य cpuid(u32 सूचकांक, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

#यदि_अन्तः // !__ASSEMBLY__

#यदि_अन्तः // x86.h
