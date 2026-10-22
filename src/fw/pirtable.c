// PIR table generation (for emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "std/pirtable.h" // struct pir_header
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // checksum
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // PirAddr

ਬਣਤਰ pir_ਸਾਰਣੀ {
    ਬਣਤਰ pir_ਸਿਰਲੇਖ pir;
    ਬਣਤਰ pir_ਥਾਂ slots[6];
} PACKED;

ਸਥਿਰ ਬਣਤਰ pir_ਸਾਰਣੀ PIR_ਸਾਰਣੀ = {
    .pir = {
        .ਸੰਸਕਰਣ_2 = 0x0100,
        .ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ pir_ਸਾਰਣੀ),
        .ਰਾਊਟਰ_devfunc = 0x08,
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
            .ਥਾਂ_nr = 0, // embedded
        }, {
            // second slot entry: 1st PCI slot
            .dev = 2<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .ਥਾਂ_nr = 1,
        }, {
            // third slot entry: 2nd PCI slot
            .dev = 3<<3,
            .links = {
                {.link = 0x62, .bitmap = 0xdef8}, // INTA#
                {.link = 0x63, .bitmap = 0xdef8}, // INTB#
                {.link = 0x60, .bitmap = 0xdef8}, // INTC#
                {.link = 0x61, .bitmap = 0xdef8}, // INTD#
            },
            .ਥਾਂ_nr = 2,
        }, {
            // 4th slot entry: 3rd PCI slot
            .dev = 4<<3,
            .links = {
                {.link = 0x63, .bitmap = 0xdef8}, // INTA#
                {.link = 0x60, .bitmap = 0xdef8}, // INTB#
                {.link = 0x61, .bitmap = 0xdef8}, // INTC#
                {.link = 0x62, .bitmap = 0xdef8}, // INTD#
            },
            .ਥਾਂ_nr = 3,
        }, {
            // 5th slot entry: 4th PCI slot
            .dev = 5<<3,
            .links = {
                {.link = 0x60, .bitmap = 0xdef8}, // INTA#
                {.link = 0x61, .bitmap = 0xdef8}, // INTB#
                {.link = 0x62, .bitmap = 0xdef8}, // INTC#
                {.link = 0x63, .bitmap = 0xdef8}, // INTD#
            },
            .ਥਾਂ_nr = 4,
        }, {
            // 6th slot entry: 5th PCI slot
            .dev = 6<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .ਥਾਂ_nr = 5,
        },
    }
};

ਖਾਲੀ
pirtable_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_PIRTABLE)
        ਵਾਪਸ;

    dprintf(3, "init PIR table\n");

    PIR_ਸਾਰਣੀ.pir.signature = PIR_SIGNATURE;
    PIR_ਸਾਰਣੀ.pir.checksum -= checksum(&PIR_ਸਾਰਣੀ, ਆਕਾਰ(PIR_ਸਾਰਣੀ));
    copy_pir(&PIR_ਸਾਰਣੀ);
}
