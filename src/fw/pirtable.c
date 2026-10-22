// PIR table generation (for emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "std/pirtable.h" // struct pir_header
#अन्तर्भाव "string.h" // checksum
#अन्तर्भाव "util.h" // PirAddr

संरचना pir_table {
    संरचना pir_header pir;
    संरचना pir_slot slots[6];
} PACKED;

स्थिर संरचना pir_table PIR_TABLE = {
    .pir = {
        .version = 0x0100,
        .size = परिमाण(संरचना pir_table),
        .router_devfunc = 0x08,
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
            .slot_nr = 0, // embedded
        }, {
            // second slot entry: 1st PCI slot
            .dev = 2<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .slot_nr = 1,
        }, {
            // third slot entry: 2nd PCI slot
            .dev = 3<<3,
            .links = {
                {.link = 0x62, .bitmap = 0xdef8}, // INTA#
                {.link = 0x63, .bitmap = 0xdef8}, // INTB#
                {.link = 0x60, .bitmap = 0xdef8}, // INTC#
                {.link = 0x61, .bitmap = 0xdef8}, // INTD#
            },
            .slot_nr = 2,
        }, {
            // 4th slot entry: 3rd PCI slot
            .dev = 4<<3,
            .links = {
                {.link = 0x63, .bitmap = 0xdef8}, // INTA#
                {.link = 0x60, .bitmap = 0xdef8}, // INTB#
                {.link = 0x61, .bitmap = 0xdef8}, // INTC#
                {.link = 0x62, .bitmap = 0xdef8}, // INTD#
            },
            .slot_nr = 3,
        }, {
            // 5th slot entry: 4th PCI slot
            .dev = 5<<3,
            .links = {
                {.link = 0x60, .bitmap = 0xdef8}, // INTA#
                {.link = 0x61, .bitmap = 0xdef8}, // INTB#
                {.link = 0x62, .bitmap = 0xdef8}, // INTC#
                {.link = 0x63, .bitmap = 0xdef8}, // INTD#
            },
            .slot_nr = 4,
        }, {
            // 6th slot entry: 5th PCI slot
            .dev = 6<<3,
            .links = {
                {.link = 0x61, .bitmap = 0xdef8}, // INTA#
                {.link = 0x62, .bitmap = 0xdef8}, // INTB#
                {.link = 0x63, .bitmap = 0xdef8}, // INTC#
                {.link = 0x60, .bitmap = 0xdef8}, // INTD#
            },
            .slot_nr = 5,
        },
    }
};

शून्य
pirtable_setup(शून्य)
{
    सचे (! CONFIG_PIRTABLE)
        पटिनिवत्त;

    dprintf(3, "init PIR table\n");

    PIR_TABLE.pir.signature = PIR_SIGNATURE;
    PIR_TABLE.pir.checksum -= checksum(&PIR_TABLE, परिमाण(PIR_TABLE));
    copy_pir(&PIR_TABLE);
}
