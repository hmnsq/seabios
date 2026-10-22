#ifndef __ROMFILE_H
#define __ROMFILE_H

#अन्तर्भाव "types.h" // u32

// romfile.c
संरचना romfile_s {
    संरचना romfile_s *next;
    वर्ण name[128];
    u32 size;
    पूर्णाङ्क (*copy)(संरचना romfile_s *file, शून्य *dest, u32 maxlen);
};
शून्य romfile_add(संरचना romfile_s *file);
संरचना romfile_s *romfile_findprefix(अचर वर्ण *prefix, संरचना romfile_s *prev);
संरचना romfile_s *romfile_find(अचर वर्ण *name);
शून्य *romfile_loadfile(अचर वर्ण *name, पूर्णाङ्क *psize);
u64 romfile_loadint(अचर वर्ण *name, u64 defval);
u32 romfile_loadbool(अचर वर्ण *name, u32 defval);

शून्य const_romfile_add_int(वर्ण *name, u32 मान);

#endif // romfile.h
