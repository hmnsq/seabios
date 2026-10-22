#ifndef __ROMFILE_H
#define __ROMFILE_H

#অন্তর্ভাব "types.h" // u32

// romfile.c
সংরচনা romfile_s {
    সংরচনা romfile_s *next;
    বর্ণ name[128];
    u32 size;
    পূর্ণাঙ্ক (*copy)(সংরচনা romfile_s *file, শূন্য *dest, u32 maxlen);
};
শূন্য romfile_add(সংরচনা romfile_s *file);
সংরচনা romfile_s *romfile_findprefix(ধ্রুব বর্ণ *prefix, সংরচনা romfile_s *prev);
সংরচনা romfile_s *romfile_find(ধ্রুব বর্ণ *name);
শূন্য *romfile_loadfile(ধ্রুব বর্ণ *name, পূর্ণাঙ্ক *psize);
u64 romfile_loadint(ধ্রুব বর্ণ *name, u64 defval);
u32 romfile_loadbool(ধ্রুব বর্ণ *name, u32 defval);

শূন্য const_romfile_add_int(বর্ণ *name, u32 মান);

#endif // romfile.h
