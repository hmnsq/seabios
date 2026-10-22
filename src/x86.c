// X86 utility functions.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "x86.h" // __cpuid

શૂન્ય
cpuid(u32 index, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    // Check for cpu id
    u32 origflags = save_flags();
    restore_flags(origflags ^ F_ID);
    u32 newflags = save_flags();
    restore_flags(origflags);

    જો (((origflags ^ newflags) & F_ID) != F_ID)
        // no cpuid
        *eax = *ebx = *ecx = *edx = 0;
    નહીંતર
        __cpuid(index, eax, ebx, ecx, edx);
}
