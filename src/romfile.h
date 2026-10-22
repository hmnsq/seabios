#جے_بنا_تعریف __ROMFILE_H
#تعریف_کرو __ROMFILE_H

#ضمن "types.h" // u32

// romfile.c
بنية romfile_s {
    بنية romfile_s *اگلا;
    حرف name[128];
    u32 ناپ;
    صحيح (*copy)(بنية romfile_s *file, فراغ *dest, u32 maxlen);
};
فراغ romfile_جوڑو(بنية romfile_s *file);
بنية romfile_s *romfile_findprefix(ثابت حرف *اگیتر, بنية romfile_s *prev);
بنية romfile_s *romfile_لبھو(ثابت حرف *name);
فراغ *romfile_loadfile(ثابت حرف *name, صحيح *psize);
u64 romfile_loadint(ثابت حرف *name, u64 defval);
u32 romfile_loadbool(ثابت حرف *name, u32 defval);

فراغ const_romfile_جوڑو_int(حرف *name, u32 قدر);

#جے_انت // romfile.h
