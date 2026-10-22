// PIR table generation (for emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "output.h" // dprintf
#شامل "std/pirtable.h" // struct pir_header
#شامل "string.h" // checksum
#شامل "util.h" // PirAddr

ساخت pir_جدول {
    ساخت pir_سرخی pir;
    ساخت pir_خانہ slots[6];
} PACKED;

ساکن ساخت pir_جدول PIR_جدول = {
    .pir = {
        .نسخہ_2 = 0x0100,
        .سائز = حجم(ساخت pir_جدول),
        .رہنما_آلہ_devfunc = 0x08,
        .compatible_devid = 0x122e8086,
    },
    .slots = {
        {
            // first slot entry PCI-to-ISA (embedded)
            .dev = 1<<3,
            .links = {
                {.link = 0x60, .bitmap = 0xdef8}, // INTA#
                {.link = 0x61, .bitmap = 0xdef8}, // INTB#
                {.link = 0x62, .bitmap = 0xdef8}, // INTC#
                {.link = 0x63, .bitmap = 0xdef8}, // INTD#
            },
            .خانہ_nr = 0, // embedded
        }, {
            // second slot entry: 1st PCI slot
            .dev = 2<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .خانہ_nr = 1,
        }, {
            // third slot entry: 2nd PCI slot
            .dev = 3<<3,
            .links = {
                {.link = 0x62, .bitmap = 0xdef8}, // INTA#
                {.link = 0x63, .bitmap = 0xdef8}, // INTB#
                {.link = 0x60, .bitmap = 0xdef8}, // INTC#
                {.link = 0x61, .bitmap = 0xdef8}, // INTD#
            },
            .خانہ_nr = 2,
        }, {
            // 4th slot entry: 3rd PCI slot
            .dev = 4<<3,
            .links = {
                {.link = 0x63, .bitmap = 0xdef8}, // INTA#
                {.link = 0x60, .bitmap = 0xdef8}, // INTB#
                {.link = 0x61, .bitmap = 0xdef8}, // INTC#
                {.link = 0x62, .bitmap = 0xdef8}, // INTD#
            },
            .خانہ_nr = 3,
        }, {
            // 5th slot entry: 4th PCI slot
            .dev = 5<<3,
            .links = {
                {.link = 0x60, .bitmap = 0xdef8}, // INTA#
                {.link = 0x61, .bitmap = 0xdef8}, // INTB#
                {.link = 0x62, .bitmap = 0xdef8}, // INTC#
                {.link = 0x63, .bitmap = 0xdef8}, // INTD#
            },
            .خانہ_nr = 4,
        }, {
            // 6th slot entry: 5th PCI slot
            .dev = 6<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .خانہ_nr = 5,
        },
    }
};

خالی
pirtable_تیاری(خالی)
{
    اگر (! CONFIG_PIRTABLE)
        واپس;

    dprintf(3, "init PIR table\n");

    PIR_جدول.pir.signature = PIR_SIGNATURE;
    PIR_جدول.pir.checksum -= checksum(&PIR_جدول, حجم(PIR_جدول));
    copy_pir(&PIR_جدول);
}
