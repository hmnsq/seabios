// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "bregs.h" // struct bregs
#समावेश "config.h" // CONFIG_*
#समावेश "farptr.h" // FLATPTR_TO_SEGOFF
#समावेश "hw/pci.h" // pci_reboot
#समावेश "hw/pic.h" // pic_eoi2
#समावेश "hw/ps2port.h" // i8042_reboot
#समावेश "hw/rtc.h" // rtc_read
#समावेश "output.h" // dprintf
#समावेश "stacks.h" // farcall16big
#समावेश "std/bda.h" // struct bios_data_area_s
#समावेश "string.h" // memset
#समावेश "util.h" // dma_setup
#समावेश "tcgbios.h" // tpm_s3_resume
#समावेश "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
व्योम VISIBLE16
handle_resume(व्योम)
{
    ASSERT16();
    पूर्णांक status = rtc_read(CMOS_RESET_CODE);
    rtc_write(CMOS_RESET_CODE, 0);
    dprintf(1, "In resume (status=%d)\n", status);

    dma_setup();

    चयन (status) {
    हाल 0x01 ... 0x04:
    हाल 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", status);

    हाल 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    हाल 0x0a:
#घोषणा BDA_JUMP (((काष्ठा bios_तथ्य_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        यंत्र अस्थिर(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        अवरोध;

    हाल 0x0b:
        // resume execution via IRET via 40h:0067h
        यंत्र अस्थिर(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        अवरोध;

    हाल 0x0c:
        // resume execution via RETF via 40h:0067h
        यंत्र अस्थिर(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        अवरोध;

    शेष:
        अवरोध;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    यंत्र अस्थिर(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_ढेर_ADDR), "r"(0), "a"(status)
        );
}

// Handle an S3 resume event
अटल व्योम
s3_resume(व्योम)
{
    अगर (!CONFIG_S3_RESUME)
        वापस;

    u32 s3_resume_vector = find_resume_vector();
    अगर (!s3_resume_vector) {
        dprintf(1, "No resume vector set!\n");
        वापस;
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
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_vector);
    br.code = FLATPTR_TO_SEGOFF((व्योम*)s3_resume_vector);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
अटल व्योम
tryReboot(व्योम)
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
    यंत्र अस्थिर("int3");

    panic("Could not reboot");
}

व्योम VISIBLE32FLAT
handle_resume32(पूर्णांक status)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    अगर (status == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
