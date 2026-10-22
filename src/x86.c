// X86 utility functions.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "x86.h" // __cpuid

ಶೂನ್ಯ
cpuid(u32 ಸೂಚ್ಯಂಕ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    // Check for cpu id
    u32 origflags = save_flags();
    restore_flags(origflags ^ F_ID);
    u32 newflags = save_flags();
    restore_flags(origflags);

    ಆದರೆ (((origflags ^ newflags) & F_ID) != F_ID)
        // no cpuid
        *eax = *ebx = *ecx = *edx = 0;
    ಇಲ್ಲದಿದ್ದರೆ
        __cpuid(ಸೂಚ್ಯಂಕ, eax, ebx, ecx, edx);
}
