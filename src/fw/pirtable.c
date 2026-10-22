// PIR table generation (for emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "output.h" // dprintf
#ضمن "std/pirtable.h" // struct pir_header
#ضمن "string.h" // checksum
#ضمن "util.h" // PirAddr

بنية pir_جدول {
    بنية pir_سرلیک pir;
    بنية pir_خونه slots[6];
} PACKED;

ساكن بنية pir_جدول PIR_جدول = {
    .pir = {
        .نسخه_2 = 0x0100,
        .اندازه = حجم(بنية pir_جدول),
        .لارښود_devfunc = 0x08,
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
            .خونه_nr = 0, // embedded
        }, {
            // second slot entry: 1st PCI slot
            .dev = 2<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .خونه_nr = 1,
        }, {
            // third slot entry: 2nd PCI slot
            .dev = 3<<3,
            .links = {
                {.link = 0x62, .bitmap = 0xdef8}, // INTA#
                {.link = 0x63, .bitmap = 0xdef8}, // INTB#
                {.link = 0x60, .bitmap = 0xdef8}, // INTC#
                {.link = 0x61, .bitmap = 0xdef8}, // INTD#
            },
            .خونه_nr = 2,
        }, {
            // 4th slot entry: 3rd PCI slot
            .dev = 4<<3,
            .links = {
                {.link = 0x63, .bitmap = 0xdef8}, // INTA#
                {.link = 0x60, .bitmap = 0xdef8}, // INTB#
                {.link = 0x61, .bitmap = 0xdef8}, // INTC#
                {.link = 0x62, .bitmap = 0xdef8}, // INTD#
            },
            .خونه_nr = 3,
        }, {
            // 5th slot entry: 4th PCI slot
            .dev = 5<<3,
            .links = {
                {.link = 0x60, .bitmap = 0xdef8}, // INTA#
                {.link = 0x61, .bitmap = 0xdef8}, // INTB#
                {.link = 0x62, .bitmap = 0xdef8}, // INTC#
                {.link = 0x63, .bitmap = 0xdef8}, // INTD#
            },
            .خونه_nr = 4,
        }, {
            // 6th slot entry: 5th PCI slot
            .dev = 6<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .خونه_nr = 5,
        },
    }
};

فراغ
pirtable_تیاری(فراغ)
{
    إذا (! CONFIG_PIRTABLE)
        أرجع;

    dprintf(3, "init PIR table\n");

    PIR_جدول.pir.signature = PIR_SIGNATURE;
    PIR_جدول.pir.checksum -= checksum(&PIR_جدول, حجم(PIR_جدول));
    copy_pir(&PIR_جدول);
}
