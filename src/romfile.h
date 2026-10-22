#ifndef __ROMFILE_H
#define __ROMFILE_H

#ضمن "types.h" // u32

// romfile.c
بنية romfile_s {
    بنية romfile_s *next;
    حرف name[128];
    u32 size;
    صحيح (*copy)(بنية romfile_s *file, فراغ *dest, u32 maxlen);
};
فراغ romfile_add(بنية romfile_s *file);
بنية romfile_s *romfile_findprefix(ثابت حرف *prefix, بنية romfile_s *prev);
بنية romfile_s *romfile_find(ثابت حرف *name);
فراغ *romfile_loadfile(ثابت حرف *name, صحيح *psize);
u64 romfile_loadint(ثابت حرف *name, u64 defval);
u32 romfile_loadbool(ثابت حرف *name, u32 defval);

فراغ const_romfile_add_int(حرف *name, u32 نيلاي);

#endif // romfile.h
