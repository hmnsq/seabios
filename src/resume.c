// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "farptr.h" // FLATPTR_TO_SEGOFF
#अन्तर्भाव "hw/pci.h" // pci_reboot
#अन्तर्भाव "hw/pic.h" // pic_eoi2
#अन्तर्भाव "hw/ps2port.h" // i8042_reboot
#अन्तर्भाव "hw/rtc.h" // rtc_read
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "stacks.h" // farcall16big
#अन्तर्भाव "std/bda.h" // struct bios_data_area_s
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // dma_setup
#अन्तर्भाव "tcgbios.h" // tpm_s3_resume
#अन्तर्भाव "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
शून्य VISIBLE16
handle_resume(शून्य)
{
    ASSERT16();
    पूर्णाङ्क स्थितिः = rtc_read(CMOS_RESET_संहिता);
    rtc_write(CMOS_RESET_संहिता, 0);
    dprintf(1, "In resume (status=%d)\n", स्थितिः);

    dma_सज्जता();

    विकल्प (स्थितिः) {
    पक्ष 0x01 ... 0x04:
    पक्ष 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", स्थितिः);

    पक्ष 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    पक्ष 0x0a:
#परिभाषय BDA_JUMP (((संरचना bios_दत्त_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm चञ्चल(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        भङ्ग;

    पक्ष 0x0b:
        // resume execution via IRET via 40h:0067h
        asm चञ्चल(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        भङ्ग;

    पक्ष 0x0c:
        // resume execution via RETF via 40h:0067h
        asm चञ्चल(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        भङ्ग;

    स्वभाव:
        भङ्ग;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm चञ्चल(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_राशिः_ADDR), "r"(0), "a"(स्थितिः)
        );
}

// Handle an S3 resume event
स्थिर शून्य
s3_resume(शून्य)
{
    यदि (!CONFIG_S3_RESUME)
        प्रत्यागम;

    u32 s3_resume_सदिशम् = find_resume_सदिशम्();
    यदि (!s3_resume_सदिशम्) {
        dprintf(1, "No resume vector set!\n");
        प्रत्यागम;
    }

    pic_सज्जता();
    smm_सज्जता();
    smp_resume();

    pci_resume();

    /* resume TPM before we may measure option roms */
    tpm_s3_resume();
    s3_resume_vga();

    /* Replay any fw_cfg entries that go back to the host */
    romfile_fw_cfg_resume();

    make_bios_readonly();

    // Invoke the resume vector.
    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_सदिशम्);
    br.संहिता = FLATPTR_गन्तव्यम्_SEGOFF((शून्य*)s3_resume_सदिशम्);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
स्थिर शून्य
tryReboot(शून्य)
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
    asm चञ्चल("int3");

    panic("Could not reboot");
}

शून्य VISIBLE32FLAT
handle_resume32(पूर्णाङ्क स्थितिः)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    यदि (स्थितिः == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
