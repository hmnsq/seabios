#ifndef __ROMFILE_H
#define __ROMFILE_H

#شامل "types.h" // u32

// romfile.c
ساختار romfile_s {
    ساختار romfile_s *next;
    نویسه name[128];
    u32 size;
    صحیح (*copy)(ساختار romfile_s *file, تهی *dest, u32 maxlen);
};
تهی romfile_add(ساختار romfile_s *file);
ساختار romfile_s *romfile_findprefix(ثابت نویسه *prefix, ساختار romfile_s *prev);
ساختار romfile_s *romfile_find(ثابت نویسه *name);
تهی *romfile_loadfile(ثابت نویسه *name, صحیح *psize);
u64 romfile_loadint(ثابت نویسه *name, u64 defval);
u32 romfile_loadbool(ثابت نویسه *name, u32 defval);

تهی const_romfile_add_int(نویسه *name, u32 مقدار);

#endif // romfile.h
