// Support for generating ACPI tables (on emulators)
// DO NOT ADD NEW FEATURES HERE.  (See paravirt.c / biostables.c instead.)
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "output.h" // dprintf

શૂન્ય
acpi_setup(શૂન્ય)
{
    જો (! CONFIG_ACPI)
        પાછા;

    dprintf(1, "ACPI tables for qemu 1.6 and older are not supported any more.\n");
}
