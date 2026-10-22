// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "farptr.h" // FLATPTR_TO_SEGOFF
#அந்தர்பாவ "hw/pci.h" // pci_reboot
#அந்தர்பாவ "hw/pic.h" // pic_eoi2
#அந்தர்பாவ "hw/ps2port.h" // i8042_reboot
#அந்தர்பாவ "hw/rtc.h" // rtc_read
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "stacks.h" // farcall16big
#அந்தர்பாவ "std/bda.h" // struct bios_data_area_s
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // dma_setup
#அந்தர்பாவ "tcgbios.h" // tpm_s3_resume
#அந்தர்பாவ "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
வெறுமை VISIBLE16
handle_resume(வெறுமை)
{
    ASSERT16();
    முழு status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    தேர்வு (status) {
    நிலை 0x01 ... 0x04:
    நிலை 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    நிலை 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    நிலை 0x0a:
#define BDA_JUMP (((கட்டமைப்பு bios_data_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm நிலையற்ற(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        நிறுத்து;

    நிலை 0x0b:
        // resume execution via IRET via 40h:0067h
        asm நிலையற்ற(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        நிறுத்து;

    நிலை 0x0c:
        // resume execution via RETF via 40h:0067h
        asm நிலையற்ற(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        நிறுத்து;

    இயல்பு:
        நிறுத்து;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm நிலையற்ற(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_அடுக்கு_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
நிலையான வெறுமை
s3_resume(வெறுமை)
{
    எனில் (!CONFIG_S3_RESUME)
        திரும்பு;

    u32 s3_resume_vector = find_resume_vector();
    எனில் (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        திரும்பு;
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
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((வெறுமை*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
நிலையான வெறுமை
tryReboot(வெறுமை)
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
    asm நிலையற்ற("int3");

    panic("Could not reboot");
}

வெறுமை VISIBLE32FLAT
handle_resume32(முழு status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    எனில் (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
