#اگر_غیر_تعریف_شدہ __ROMFILE_H
#تعریف_کرو __ROMFILE_H

#شامل "types.h" // u32

// romfile.c
ساخت romfile_s {
    ساخت romfile_s *اگلا;
    حرف name[128];
    u32 سائز;
    صحیح (*copy)(ساخت romfile_s *file, خالی *dest, u32 maxlen);
};
خالی romfile_جوڑو(ساخت romfile_s *file);
ساخت romfile_s *romfile_findprefix(ثابت حرف *سابقہ, ساخت romfile_s *پچھلا);
ساخت romfile_s *romfile_ڈھونڈو(ثابت حرف *name);
خالی *romfile_loadfile(ثابت حرف *name, صحیح *psize);
u64 romfile_loadint(ثابت حرف *name, u64 defval);
u32 romfile_loadbool(ثابت حرف *name, u32 defval);

خالی const_romfile_جوڑو_int(حرف *name, u32 قدر);

#اگر_ختم // romfile.h
