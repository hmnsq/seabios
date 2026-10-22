#که_نه_تعریف_شوی __ROMFILE_H
#تعریف_کړه __ROMFILE_H

#ضمن "types.h" // u32

// romfile.c
بنية romfile_s {
    بنية romfile_s *بل;
    حرف name[128];
    u32 اندازه;
    صحيح (*copy)(بنية romfile_s *file, فراغ *dest, u32 maxlen);
};
فراغ romfile_جمع_کړه(بنية romfile_s *file);
بنية romfile_s *romfile_findprefix(ثابت حرف *مختاړی, بنية romfile_s *prev);
بنية romfile_s *romfile_پیدا_کړه(ثابت حرف *name);
فراغ *romfile_loadfile(ثابت حرف *name, صحيح *psize);
u64 romfile_loadint(ثابت حرف *name, u64 defval);
u32 romfile_loadbool(ثابت حرف *name, u32 defval);

فراغ const_romfile_جمع_کړه_int(حرف *name, u32 ارزښت);

#د_که_پای // romfile.h
