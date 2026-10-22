#ifndef __ROMFILE_H
#define __ROMFILE_H

#include "types.h" // u32

// romfile.c
sa_nracanaa romfile_s {
    sa_nracanaa romfile_s *next;
    var_na name[128];
    u32 size;
    පූර්ණ_සංඛ්‍යා (*copy)(sa_nracanaa romfile_s *file, void *dest, u32 maxlen);
};
void romfile_add(sa_nracanaa romfile_s *file);
sa_nracanaa romfile_s *romfile_findprefix(const var_na *prefix, sa_nracanaa romfile_s *prev);
sa_nracanaa romfile_s *romfile_find(const var_na *name);
void *romfile_loadfile(const var_na *name, පූර්ණ_සංඛ්‍යා *psize);
u64 romfile_loadint(const var_na *name, u64 defval);
u32 romfile_loadbool(const var_na *name, u32 defval);

void const_romfile_add_int(var_na *name, u32 value);

#endif // romfile.h
