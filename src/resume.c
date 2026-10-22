// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "farptr.h" // FLATPTR_TO_SEGOFF
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_reboot
#ಅನ್ತರ್ಭಾವ "hw/pic.h" // pic_eoi2
#ಅನ್ತರ್ಭಾವ "hw/ps2port.h" // i8042_reboot
#ಅನ್ತರ್ಭಾವ "hw/rtc.h" // rtc_read
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "stacks.h" // farcall16big
#ಅನ್ತರ್ಭಾವ "std/bda.h" // struct bios_data_area_s
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // dma_setup
#ಅನ್ತರ್ಭಾವ "tcgbios.h" // tpm_s3_resume
#ಅನ್ತರ್ಭಾವ "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
ಶೂನ್ಯ VISIBLE16
handle_resume(ಶೂನ್ಯ)
{
    ASSERT16();
    ಪೂರ್ಣಾಙ್ಕ status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    ವಿಕಲ್ಪ (status) {
    ಪಕ್ಷ 0x01 ... 0x04:
    ಪಕ್ಷ 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    ಪಕ್ಷ 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    ಪಕ್ಷ 0x0a:
#define BDA_JUMP (((ಸಂರಚನಾ bios_data_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm ಚಞ್ಚಲ(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ಭಙ್ಗ;

    ಪಕ್ಷ 0x0b:
        // resume execution via IRET via 40h:0067h
        asm ಚಞ್ಚಲ(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ಭಙ್ಗ;

    ಪಕ್ಷ 0x0c:
        // resume execution via RETF via 40h:0067h
        asm ಚಞ್ಚಲ(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ಭಙ್ಗ;

    ಸ್ವಭಾವ:
        ಭಙ್ಗ;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm ಚಞ್ಚಲ(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_ರಾಶಿ_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
ಸ್ಥಿರ ಶೂನ್ಯ
s3_resume(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_S3_RESUME)
        ಪ್ರತ್ಯಾಗಮ;

    u32 s3_resume_vector = find_resume_vector();
    ಯದಿ (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        ಪ್ರತ್ಯಾಗಮ;
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
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((ಶೂನ್ಯ*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
ಸ್ಥಿರ ಶೂನ್ಯ
tryReboot(ಶೂನ್ಯ)
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
    asm ಚಞ್ಚಲ("int3");

    panic("Could not reboot");
}

ಶೂನ್ಯ VISIBLE32FLAT
handle_resume32(ಪೂರ್ಣಾಙ್ಕ status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    ಯದಿ (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
