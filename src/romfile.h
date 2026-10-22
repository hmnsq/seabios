#جيڪڏهن_وصف_نه_ٿيل __ROMFILE_H
#وصف_ڪريو __ROMFILE_H

#ضمن "types.h" // u32

// romfile.c
بنية romfile_s {
    بنية romfile_s *ايندڙ;
    حرف name[128];
    u32 سائيز;
    صحيح (*copy)(بنية romfile_s *file, فراغ *dest, u32 maxlen);
};
فراغ romfile_جوڙيو(بنية romfile_s *file);
بنية romfile_s *romfile_findprefix(ثابت حرف *اڳياڙي, بنية romfile_s *prev);
بنية romfile_s *romfile_ڳوليو(ثابت حرف *name);
فراغ *romfile_loadfile(ثابت حرف *name, صحيح *psize);
u64 romfile_loadint(ثابت حرف *name, u64 defval);
u32 romfile_loadbool(ثابت حرف *name, u32 defval);

فراغ const_romfile_جوڙيو_int(حرف *name, u32 قدر);

#جيڪڏهن_پورو // romfile.h
