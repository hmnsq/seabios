// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "farptr.h" // FLATPTR_TO_SEGOFF
#അന്തര്ഭാവ "hw/pci.h" // pci_reboot
#അന്തര്ഭാവ "hw/pic.h" // pic_eoi2
#അന്തര്ഭാവ "hw/ps2port.h" // i8042_reboot
#അന്തര്ഭാവ "hw/rtc.h" // rtc_read
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "stacks.h" // farcall16big
#അന്തര്ഭാവ "std/bda.h" // struct bios_data_area_s
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // dma_setup
#അന്തര്ഭാവ "tcgbios.h" // tpm_s3_resume
#അന്തര്ഭാവ "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
ശൂന്യം VISIBLE16
handle_resume(ശൂന്യം)
{
    ASSERT16();
    പൂർണ്ണസംഖ്യ status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    തിരഞ്ഞെടുപ്പ് (status) {
    അവസ്ഥ 0x01 ... 0x04:
    അവസ്ഥ 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    അവസ്ഥ 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    അവസ്ഥ 0x0a:
#define BDA_JUMP (((ഘടന bios_data_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm അസ്ഥിര(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        നിർത്തുക;

    അവസ്ഥ 0x0b:
        // resume execution via IRET via 40h:0067h
        asm അസ്ഥിര(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        നിർത്തുക;

    അവസ്ഥ 0x0c:
        // resume execution via RETF via 40h:0067h
        asm അസ്ഥിര(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
        നിർത്തുക;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm അസ്ഥിര(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_അട്ടി_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
സ്ഥിതം ശൂന്യം
s3_resume(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_S3_RESUME)
        തിരികെ;

    u32 s3_resume_vector = find_resume_vector();
    എങ്കിൽ (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        തിരികെ;
    }

    pic_setup();
    smm_setup();
    smp_resume();

    pci_resume();

    /* resume TPM before we may measure option roms */
    tpm_s3_resume();
    s3_resume_vga();

    /* Replay any fw_cfg entries that go back to the host */
    romfile_fw_cfg_resume();

    make_bios_readonly();

    // Invoke the resume vector.
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((ശൂന്യം*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
സ്ഥിതം ശൂന്യം
tryReboot(ശൂന്യം)
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
    asm അസ്ഥിര("int3");

    panic("Could not reboot");
}

ശൂന്യം VISIBLE32FLAT
handle_resume32(പൂർണ്ണസംഖ്യ status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    എങ്കിൽ (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
