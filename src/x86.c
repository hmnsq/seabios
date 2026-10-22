// X86 utility functions.
//
// Copyright (C) 2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "x86.h" // __cpuid

خالی
cpuid(u32 اشاریہ, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
    // Check for cpu id
    u32 origflags = save_flags();
    restore_flags(origflags ^ F_ID);
    u32 newflags = save_flags();
    restore_flags(origflags);

    اگر (((origflags ^ newflags) & F_ID) != F_ID)
        // no cpuid
        *eax = *ebx = *ecx = *edx = 0;
    ورنہ
        __cpuid(اشاریہ, eax, ebx, ecx, edx);
}
