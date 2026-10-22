// Support for generating ACPI tables (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "output.h" // dprintf

శూన్య
acpi_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_ACPI)
        తిరిగి;

    dprintf(1, "ACPI tables for qemu 1.6 and older are not supported any more.\n");
}
