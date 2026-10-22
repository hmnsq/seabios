#ifndef __ROMFILE_H
#define __ROMFILE_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// romfile.c
ಸಂರಚನಾ romfile_s {
    ಸಂರಚನಾ romfile_s *next;
    ವರ್ಣ name[128];
    u32 size;
    ಪೂರ್ಣಾಙ್ಕ (*copy)(ಸಂರಚನಾ romfile_s *file, ಶೂನ್ಯ *dest, u32 maxlen);
};
ಶೂನ್ಯ romfile_add(ಸಂರಚನಾ romfile_s *file);
ಸಂರಚನಾ romfile_s *romfile_findprefix(ಧ್ರುವ ವರ್ಣ *prefix, ಸಂರಚನಾ romfile_s *prev);
ಸಂರಚನಾ romfile_s *romfile_find(ಧ್ರುವ ವರ್ಣ *name);
ಶೂನ್ಯ *romfile_loadfile(ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ *psize);
u64 romfile_loadint(ಧ್ರುವ ವರ್ಣ *name, u64 defval);
u32 romfile_loadbool(ಧ್ರುವ ವರ್ಣ *name, u32 defval);

ಶೂನ್ಯ const_romfile_add_int(ವರ್ಣ *name, u32 ಮೌಲ್ಯ);

#endif // romfile.h
