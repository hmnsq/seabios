// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "farptr.h" // FLATPTR_TO_SEGOFF
#شامل "hw/pci.h" // pci_reboot
#شامل "hw/pic.h" // pic_eoi2
#شامل "hw/ps2port.h" // i8042_reboot
#شامل "hw/rtc.h" // rtc_read
#شامل "output.h" // dprintf
#شامل "stacks.h" // farcall16big
#شامل "std/bda.h" // struct bios_data_area_s
#شامل "string.h" // memset
#شامل "util.h" // dma_setup
#شامل "tcgbios.h" // tpm_s3_resume
#شامل "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
تهی VISIBLE16
handle_resume(تهی)
{
    ASSERT16();
    صحیح status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    انتخاب (status) {
    حالت 0x01 ... 0x04:
    حالت 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    حالت 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    حالت 0x0a:
#define BDA_JUMP (((ساختار bios_data_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm فرار(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        بشکن;

    حالت 0x0b:
        // resume execution via IRET via 40h:0067h
        asm فرار(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        بشکن;

    حالت 0x0c:
        // resume execution via RETF via 40h:0067h
        asm فرار(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        بشکن;

    پیشفرض:
        بشکن;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm فرار(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_پشته_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
ایستا تهی
s3_resume(تهی)
{
    اگر (!CONFIG_S3_RESUME)
        بازگشت;

    u32 s3_resume_vector = find_resume_vector();
    اگر (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        بازگشت;
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
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((تهی*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
ایستا تهی
tryReboot(تهی)
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
    asm فرار("int3");

    panic("Could not reboot");
}

تهی VISIBLE32FLAT
handle_resume32(صحیح status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    اگر (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
