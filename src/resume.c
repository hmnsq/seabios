// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "farptr.h" // FLATPTR_TO_SEGOFF
#ضمن "hw/pci.h" // pci_reboot
#ضمن "hw/pic.h" // pic_eoi2
#ضمن "hw/ps2port.h" // i8042_reboot
#ضمن "hw/rtc.h" // rtc_read
#ضمن "output.h" // dprintf
#ضمن "stacks.h" // farcall16big
#ضمن "std/bda.h" // struct bios_data_area_s
#ضمن "string.h" // memset
#ضمن "util.h" // dma_setup
#ضمن "tcgbios.h" // tpm_s3_resume
#ضمن "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
فراغ VISIBLE16
handle_resume(فراغ)
{
    ASSERT16();
    صحيح status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    بدل (status) {
    حالة 0x01 ... 0x04:
    حالة 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    حالة 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    حالة 0x0a:
#define BDA_JUMP (((بنية bios_data_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm متطاير(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        اكسر;

    حالة 0x0b:
        // resume execution via IRET via 40h:0067h
        asm متطاير(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        اكسر;

    حالة 0x0c:
        // resume execution via RETF via 40h:0067h
        asm متطاير(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        اكسر;

    افتراضي:
        اكسر;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm متطاير(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_تر_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
ساكن فراغ
s3_resume(فراغ)
{
    إذا (!CONFIG_S3_RESUME)
        أرجع;

    u32 s3_resume_vector = find_resume_vector();
    إذا (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        أرجع;
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
    بنية bregs br;
    memset(&br, 0, حجم(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((فراغ*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
ساكن فراغ
tryReboot(فراغ)
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
    asm متطاير("int3");

    panic("Could not reboot");
}

فراغ VISIBLE32FLAT
handle_resume32(صحيح status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    إذا (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
