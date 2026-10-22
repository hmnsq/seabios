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
خالی VISIBLE16
handle_resume(خالی)
{
    ASSERT16();
    صحیح حالت_2 = rtc_read(CMOS_RESET_رمز);
    rtc_write(CMOS_RESET_رمز, 0);
    dprintf(1, "In resume (status=%d)\n", حالت_2);

    dma_تیاری();

    انتخاب (حالت_2) {
    حالت 0x01 ... 0x04:
    حالت 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", حالت_2);

    حالت 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    حالت 0x0a:
#تعریف_کرو BDA_JUMP (((ساخت bios_ڈیٹا_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm متلون(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        توڑو;

    حالت 0x0b:
        // resume execution via IRET via 40h:0067h
        asm متلون(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        توڑو;

    حالت 0x0c:
        // resume execution via RETF via 40h:0067h
        asm متلون(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        توڑو;

    طےشدہ:
        توڑو;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm متلون(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_ڈھیر_ADDR), "r"(0), "a"(حالت_2)
        );
}

// Handle an S3 resume event
ساکن خالی
s3_resume(خالی)
{
    اگر (!CONFIG_S3_RESUME)
        واپس;

    u32 s3_resume_سمتیہ = ڈھونڈو_resume_سمتیہ();
    اگر (!s3_resume_سمتیہ) {
        dprintf(1, "No resume vector set!\n");
        واپس;
    }

    pic_تیاری();
    smm_تیاری();
    smp_resume();

    pci_resume();

    /* resume TPM before we may measure option roms */
    tpm_s3_resume();
    s3_resume_vga();

    /* Replay any fw_cfg entries that go back to the host */
    romfile_fw_cfg_resume();

    make_bios_readonly();

    // Invoke the resume vector.
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_سمتیہ);
    br.رمز = FLATPTR_مقصد_SEGOFF((خالی*)s3_resume_سمتیہ);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
ساکن خالی
tryReboot(خالی)
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
    asm متلون("int3");

    panic("Could not reboot");
}

خالی VISIBLE32FLAT
handle_resume32(صحیح حالت_2)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    اگر (حالت_2 == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
