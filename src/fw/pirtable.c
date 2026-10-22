// PIR table generation (for emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "std/pirtable.h" // struct pir_header
#ಅನ್ತರ್ಭಾವ "string.h" // checksum
#ಅನ್ತರ್ಭಾವ "util.h" // PirAddr

ರಚನೆ pir_ಕೋಷ್ಟಕ {
    ರಚನೆ pir_ಶೀರ್ಷಿಕೆ pir;
    ರಚನೆ pir_ಸ್ಥಾನ slots[6];
} PACKED;

ಸ್ಥಿರ ರಚನೆ pir_ಕೋಷ್ಟಕ PIR_ಕೋಷ್ಟಕ = {
    .pir = {
        .ಆವೃತ್ತಿ_2 = 0x0100,
        .ಗಾತ್ರ_2 = ಗಾತ್ರ(ರಚನೆ pir_ಕೋಷ್ಟಕ),
        .ಮಾರ್ಗನಿರ್ದೇಶಕ_devfunc = 0x08,
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
            .ಸ್ಥಾನ_nr = 0, // embedded
        }, {
            // second slot entry: 1st PCI slot
            .dev = 2<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .ಸ್ಥಾನ_nr = 1,
        }, {
            // third slot entry: 2nd PCI slot
            .dev = 3<<3,
            .links = {
                {.link = 0x62, .bitmap = 0xdef8}, // INTA#
                {.link = 0x63, .bitmap = 0xdef8}, // INTB#
                {.link = 0x60, .bitmap = 0xdef8}, // INTC#
                {.link = 0x61, .bitmap = 0xdef8}, // INTD#
            },
            .ಸ್ಥಾನ_nr = 2,
        }, {
            // 4th slot entry: 3rd PCI slot
            .dev = 4<<3,
            .links = {
                {.link = 0x63, .bitmap = 0xdef8}, // INTA#
                {.link = 0x60, .bitmap = 0xdef8}, // INTB#
                {.link = 0x61, .bitmap = 0xdef8}, // INTC#
                {.link = 0x62, .bitmap = 0xdef8}, // INTD#
            },
            .ಸ್ಥಾನ_nr = 3,
        }, {
            // 5th slot entry: 4th PCI slot
            .dev = 5<<3,
            .links = {
                {.link = 0x60, .bitmap = 0xdef8}, // INTA#
                {.link = 0x61, .bitmap = 0xdef8}, // INTB#
                {.link = 0x62, .bitmap = 0xdef8}, // INTC#
                {.link = 0x63, .bitmap = 0xdef8}, // INTD#
            },
            .ಸ್ಥಾನ_nr = 4,
        }, {
            // 6th slot entry: 5th PCI slot
            .dev = 6<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .ಸ್ಥಾನ_nr = 5,
        },
    }
};

ಶೂನ್ಯ
pirtable_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_PIRTABLE)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init PIR table\n");

    PIR_ಕೋಷ್ಟಕ.pir.signature = PIR_SIGNATURE;
    PIR_ಕೋಷ್ಟಕ.pir.checksum -= checksum(&PIR_ಕೋಷ್ಟಕ, ಗಾತ್ರ(PIR_ಕೋಷ್ಟಕ));
    copy_pir(&PIR_ಕೋಷ್ಟಕ);
}
