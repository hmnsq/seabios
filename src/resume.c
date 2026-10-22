// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "farptr.h" // FLATPTR_TO_SEGOFF
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_reboot
#ଅନ୍ତର୍ଭାଵ "hw/pic.h" // pic_eoi2
#ଅନ୍ତର୍ଭାଵ "hw/ps2port.h" // i8042_reboot
#ଅନ୍ତର୍ଭାଵ "hw/rtc.h" // rtc_read
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "stacks.h" // farcall16big
#ଅନ୍ତର୍ଭାଵ "std/bda.h" // struct bios_data_area_s
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // dma_setup
#ଅନ୍ତର୍ଭାଵ "tcgbios.h" // tpm_s3_resume
#ଅନ୍ତର୍ଭାଵ "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
ଶୂନ୍ୟ VISIBLE16
handle_resume(ଶୂନ୍ୟ)
{
    ASSERT16();
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    ବାଛ (status) {
    ସ୍ଥିତି 0x01 ... 0x04:
    ସ୍ଥିତି 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    ସ୍ଥିତି 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    ସ୍ଥିତି 0x0a:
#define BDA_JUMP (((ସଂରଚନା bios_data_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm ଅସ୍ଥିର(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ଭାଙ୍ଗ;

    ସ୍ଥିତି 0x0b:
        // resume execution via IRET via 40h:0067h
        asm ଅସ୍ଥିର(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ଭାଙ୍ଗ;

    ସ୍ଥିତି 0x0c:
        // resume execution via RETF via 40h:0067h
        asm ଅସ୍ଥିର(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଭାଙ୍ଗ;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm ଅସ୍ଥିର(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_ଗଦା_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
ସ୍ଥିର ଶୂନ୍ୟ
s3_resume(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_S3_RESUME)
        ଫେରନ୍ତୁ;

    u32 s3_resume_vector = find_resume_vector();
    ଯଦି (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        ଫେରନ୍ତୁ;
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
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((ଶୂନ୍ୟ*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
ସ୍ଥିର ଶୂନ୍ୟ
tryReboot(ଶୂନ୍ୟ)
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
    asm ଅସ୍ଥିର("int3");

    panic("Could not reboot");
}

ଶୂନ୍ୟ VISIBLE32FLAT
handle_resume32(ପୂର୍ଣ୍ଣସଂଖ୍ୟା status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    ଯଦି (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
