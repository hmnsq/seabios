// Support for generating ACPI tables (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "config.h" // CONFIG_*
#شامل "output.h" // dprintf

تهی
acpi_setup(تهی)
{
    اگر (! CONFIG_ACPI)
        بازگشت;

    dprintf(1, "ACPI tables for qemu 1.6 and older are not supported any more.\n");
}
