#ifndef __ROMFILE_H
#define __ROMFILE_H

#अन्तर्भाव "types.h" // u32

// romfile.c
संरचना romfile_s {
    संरचना romfile_s *next;
    अक्षर name[128];
    u32 size;
    पूर्णांक (*copy)(संरचना romfile_s *file, शून्य *dest, u32 maxlen);
};
शून्य romfile_add(संरचना romfile_s *file);
संरचना romfile_s *romfile_findprefix(स्थिरांक अक्षर *prefix, संरचना romfile_s *prev);
संरचना romfile_s *romfile_find(स्थिरांक अक्षर *name);
शून्य *romfile_loadfile(स्थिरांक अक्षर *name, पूर्णांक *psize);
u64 romfile_loadint(स्थिरांक अक्षर *name, u64 defval);
u32 romfile_loadbool(स्थिरांक अक्षर *name, u32 defval);

शून्य const_romfile_add_int(अक्षर *name, u32 मूल्य);

#endif // romfile.h
