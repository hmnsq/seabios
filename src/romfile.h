#ifndef __ROMFILE_H
#define __ROMFILE_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

// romfile.c
ସଂରଚନା romfile_s {
    ସଂରଚନା romfile_s *next;
    ଅକ୍ଷର name[128];
    u32 size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*copy)(ସଂରଚନା romfile_s *file, ଶୂନ୍ୟ *dest, u32 maxlen);
};
ଶୂନ୍ୟ romfile_add(ସଂରଚନା romfile_s *file);
ସଂରଚନା romfile_s *romfile_findprefix(ଅଚଳ ଅକ୍ଷର *prefix, ସଂରଚନା romfile_s *prev);
ସଂରଚନା romfile_s *romfile_find(ଅଚଳ ଅକ୍ଷର *name);
ଶୂନ୍ୟ *romfile_loadfile(ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *psize);
u64 romfile_loadint(ଅଚଳ ଅକ୍ଷର *name, u64 defval);
u32 romfile_loadbool(ଅଚଳ ଅକ୍ଷର *name, u32 defval);

ଶୂନ୍ୟ const_romfile_add_int(ଅକ୍ଷର *name, u32 ମୂଲ୍ୟ);

#endif // romfile.h
