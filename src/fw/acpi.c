// Support for generating ACPI tables (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf

ಶೂನ್ಯ
acpi_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_ACPI)
        ಹಿಂತಿರುಗಿ;

    dprintf(1, "ACPI tables for qemu 1.6 and older are not supported any more.\n");
}
