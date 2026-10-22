// Code for handling calls to "post" that are resume related.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "farptr.h" // FLATPTR_TO_SEGOFF
#అన్తర్భావ "hw/pci.h" // pci_reboot
#అన్తర్భావ "hw/pic.h" // pic_eoi2
#అన్తర్భావ "hw/ps2port.h" // i8042_reboot
#అన్తర్భావ "hw/rtc.h" // rtc_read
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "stacks.h" // farcall16big
#అన్తర్భావ "std/bda.h" // struct bios_data_area_s
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // dma_setup
#అన్తర్భావ "tcgbios.h" // tpm_s3_resume
#అన్తర్భావ "fw/romfile_loader.h" // romfile_fw_cfg_resume

// Handler for post calls that look like a resume.
శూన్య VISIBLE16
handle_resume(శూన్య)
{
    ASSERT16();
    పూర్ణాంకం స్థితి_2 = rtc_read(CMOS_RESET_సంకేతం);
    rtc_write(CMOS_RESET_సంకేతం, 0);
    dprintf(1, "In resume (status=%d)\n", స్థితి_2);

    dma_అమరిక();

    ఎంపిక (స్థితి_2) {
    స్థితి 0x01 ... 0x04:
    స్థితి 0x06 ... 0x09:
        panic("Unimplemented shutdown status: %02x\n", స్థితి_2);

    స్థితి 0x05:
        // flush keyboard (issue EOI) and jump via 40h:0067h
        pic_eoi2();
        // NO BREAK
    స్థితి 0x0a:
#నిర్వచించు BDA_JUMP (((నిర్మాణం bios_దత్తాంశం_area_s *)0)->jump)
        // resume execution by jump via 40h:0067h
        asm అస్థిర(
            "movw %w1, %%ds\n"
            "ljmpw *%0\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ఆపు;

    స్థితి 0x0b:
        // resume execution via IRET via 40h:0067h
        asm అస్థిర(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "iretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ఆపు;

    స్థితి 0x0c:
        // resume execution via RETF via 40h:0067h
        asm అస్థిర(
            "movw %w1, %%ds\n"
            "lssw %0, %%sp\n"
            "lretw\n"
            : : "m"(BDA_JUMP), "r"(SEG_BDA)
            );
        ఆపు;

    అప్రమేయ:
        ఆపు;
    }

    // Not a 16bit resume - do remaining checks in 32bit mode
    asm అస్థిర(
        "movw %w1, %%ss\n"
        "movl %0, %%esp\n"
        "movl $_cfunc32flat_handle_resume32, %%edx\n"
        "jmp transition32\n"
        : : "i"(BUILD_S3_RESUME_దొంతర_ADDR), "r"(0), "a"(స్థితి_2)
        );
}

// Handle an S3 resume event
స్థిర శూన్య
s3_resume(శూన్య)
{
    ఒకవేళ (!CONFIG_S3_RESUME)
        తిరిగి;

    u32 s3_resume_సదిశ = కనుగొను_resume_సదిశ();
    ఒకవేళ (!s3_resume_సదిశ) {
        dprintf(1, "No resume vector set!\n");
        తిరిగి;
    }

    pic_అమరిక();
    smm_అమరిక();
    smp_resume();

    pci_resume();

    /* resume TPM before we may measure option roms */
    tpm_s3_resume();
    s3_resume_vga();

    /* Replay any fw_cfg entries that go back to the host */
    romfile_fw_cfg_resume();

    make_bios_readonly();

    // Invoke the resume vector.
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    dprintf(1, "Jump to resume vector (%x)\n", s3_resume_సదిశ);
    br.సంకేతం_2 = FLATPTR_గమ్యం_SEGOFF((శూన్య*)s3_resume_సదిశ);
    farcall16big(&br);
}

// Attempt to invoke a hard-reboot.
స్థిర శూన్య
tryReboot(శూన్య)
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
    asm అస్థిర("int3");

    panic("Could not reboot");
}

శూన్య VISIBLE32FLAT
handle_resume32(పూర్ణాంకం స్థితి_2)
{
    ASSERT32FLAT();
    dprintf(1, "In 32bit resume\n");

    ఒకవేళ (స్థితి_2 == 0xfe)
        s3_resume();

    // Must be a soft reboot - invoke a hard reboot.
    tryReboot();
}
