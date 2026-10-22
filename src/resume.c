// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "farptr.h" // FLATPTR_TO_SEGOFF
#অন্তর্ভাব "hw/pci.h" // pci_reboot
#অন্তর্ভাব "hw/pic.h" // pic_eoi2
#অন্তর্ভাব "hw/ps2port.h" // i8042_reboot
#অন্তর্ভাব "hw/rtc.h" // rtc_read
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "stacks.h" // farcall16big
#অন্তর্ভাব "std/bda.h" // struct bios_data_area_s
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // dma_setup
#অন্তর্ভাব "tcgbios.h" // tpm_s3_resume
#অন্তর্ভাব "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
শূন্য VISIBLE16
handle_resume(শূন্য)
{
    ASSERT16();
    পূর্ণসংখ্যা status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    নির্বাচন (status) {
    ক্ষেত্র 0x01 ... 0x04:
    ক্ষেত্র 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    ক্ষেত্র 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    ক্ষেত্র 0x0a:
#define BDA_JUMP (((কাঠামো bios_তথ্য_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm অস্থির(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        বিরতি;

    ক্ষেত্র 0x0b:
        // resume execution via IRET via 40h:0067h
        asm অস্থির(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        বিরতি;

    ক্ষেত্র 0x0c:
        // resume execution via RETF via 40h:0067h
        asm অস্থির(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        বিরতি;

    পূর্বনির্ধারিত:
        বিরতি;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm অস্থির(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_স্তূপ_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
স্থির শূন্য
s3_resume(শূন্য)
{
    যদি (!CONFIG_S3_RESUME)
        ফেরত;

    u32 s3_resume_vector = find_resume_vector();
    যদি (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        ফেরত;
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
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((শূন্য*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
স্থির শূন্য
tryReboot(শূন্য)
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
    asm অস্থির("int3");

    panic("Could not reboot");
}

শূন্য VISIBLE32FLAT
handle_resume32(পূর্ণসংখ্যা status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    যদি (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
