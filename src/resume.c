// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "farptr.h" // FLATPTR_TO_SEGOFF
#અન્તર્ભાવ "hw/pci.h" // pci_reboot
#અન્તર્ભાવ "hw/pic.h" // pic_eoi2
#અન્તર્ભાવ "hw/ps2port.h" // i8042_reboot
#અન્તર્ભાવ "hw/rtc.h" // rtc_read
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "stacks.h" // farcall16big
#અન્તર્ભાવ "std/bda.h" // struct bios_data_area_s
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // dma_setup
#અન્તર્ભાવ "tcgbios.h" // tpm_s3_resume
#અન્તર્ભાવ "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
શૂન્ય VISIBLE16
handle_resume(શૂન્ય)
{
    ASSERT16();
    પૂર્ણાંક status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    પસંદગી (status) {
    સ્થિતિ 0x01 ... 0x04:
    સ્થિતિ 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    સ્થિતિ 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    સ્થિતિ 0x0a:
#define BDA_JUMP (((સંરચના bios_data_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm અસ્થિર(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        તોડો;

    સ્થિતિ 0x0b:
        // resume execution via IRET via 40h:0067h
        asm અસ્થિર(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        તોડો;

    સ્થિતિ 0x0c:
        // resume execution via RETF via 40h:0067h
        asm અસ્થિર(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        તોડો;

    મૂળભૂત:
        તોડો;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm અસ્થિર(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_ઢગલો_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
સ્થિર શૂન્ય
s3_resume(શૂન્ય)
{
    જો (!CONFIG_S3_RESUME)
        પાછા;

    u32 s3_resume_vector = find_resume_vector();
    જો (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        પાછા;
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
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((શૂન્ય*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
સ્થિર શૂન્ય
tryReboot(શૂન્ય)
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
    asm અસ્થિર("int3");

    panic("Could not reboot");
}

શૂન્ય VISIBLE32FLAT
handle_resume32(પૂર્ણાંક status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    જો (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
