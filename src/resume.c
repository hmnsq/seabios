// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // FLATPTR_TO_SEGOFF
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_reboot
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pic.h" // pic_eoi2
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ps2port.h" // i8042_reboot
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/rtc.h" // rtc_read
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // farcall16big
#ਅਨ੍ਤਰ੍ਭਾਵ "std/bda.h" // struct bios_data_area_s
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // dma_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "tcgbios.h" // tpm_s3_resume
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
ਖਾਲੀ VISIBLE16
handle_resume(ਖਾਲੀ)
{
    ASSERT16();
    ਪੂਰਨ ਸਥਿਤੀ = rtc_read(CMOS_RESET_ਕੋਡ);
    rtc_write(CMOS_RESET_ਕੋਡ, 0);
    dprintf(1, "In resume (status=%d)\n", ਸਥਿਤੀ);

    dma_ਤਿਆਰੀ();

    ਚੋਣ (ਸਥਿਤੀ) {
    ਹਾਲਤ 0x01 ... 0x04:
    ਹਾਲਤ 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", ਸਥਿਤੀ);

    ਹਾਲਤ 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    ਹਾਲਤ 0x0a:
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BDA_JUMP (((ਬਣਤਰ bios_ਅੰਕੜੇ_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm ਅਸਥਿਰ(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ਰੋਕੋ;

    ਹਾਲਤ 0x0b:
        // resume execution via IRET via 40h:0067h
        asm ਅਸਥਿਰ(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ਰੋਕੋ;

    ਹਾਲਤ 0x0c:
        // resume execution via RETF via 40h:0067h
        asm ਅਸਥਿਰ(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ਰੋਕੋ;

    ਮੂਲ:
        ਰੋਕੋ;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm ਅਸਥਿਰ(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_ਢੇਰ_ADDR), "r"(0), "a"(ਸਥਿਤੀ)
        );
}

// Handle an S3 resume event
ਸਥਿਰ ਖਾਲੀ
s3_resume(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_S3_RESUME)
        ਵਾਪਸ;

    u32 s3_resume_ਸਦਿਸ਼ = ਲੱਭੋ_resume_ਸਦਿਸ਼();
    ਜੇ (!s3_resume_ਸਦਿਸ਼) {
        dprintf(1, "No resume vector set!\n");
        ਵਾਪਸ;
    }

    pic_ਤਿਆਰੀ();
    smm_ਤਿਆਰੀ();
    smp_resume();

    pci_resume();

    /* resume TPM before we may measure option roms */
    tpm_s3_resume();
    s3_resume_vga();

    /* Replay any fw_cfg entries that go back to the host */
    romfile_fw_cfg_resume();

    make_bios_readonly();

    // Invoke the resume vector.
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_ਸਦਿਸ਼);
    br.ਕੋਡ = FLATPTR_ਮੰਜ਼ਿਲ_SEGOFF((ਖਾਲੀ*)s3_resume_ਸਦਿਸ਼);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
ਸਥਿਰ ਖਾਲੀ
tryReboot(ਖਾਲੀ)
{
    dprintf(1, "Attempting a hard reboot\n");

    // Use a QEMU specific reboot on QEMU
    qemu_reboot();

    // Reboot using ACPI RESET_REG
    acpi_reboot();

    // Try keyboard controller reboot.
    i8042_reboot();

    // Try PCI 0xcf9 reboot
    pci_reboot();

    // Try triple fault
    asm ਅਸਥਿਰ("int3");

    panic("Could not reboot");
}

ਖਾਲੀ VISIBLE32FLAT
handle_resume32(ਪੂਰਨ ਸਥਿਤੀ)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    ਜੇ (ਸਥਿਤੀ == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
