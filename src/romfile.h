#ifndef __ROMFILE_H
#define __ROMFILE_H

#അന്തര്ഭാവ "types.h" // u32

// romfile.c
ഘടന romfile_s {
    ഘടന romfile_s *next;
    അക്ഷരം name[128];
    u32 size;
    പൂർണ്ണസംഖ്യ (*copy)(ഘടന romfile_s *file, ശൂന്യം *dest, u32 maxlen);
};
ശൂന്യം romfile_add(ഘടന romfile_s *file);
ഘടന romfile_s *romfile_findprefix(സ്ഥിരം അക്ഷരം *prefix, ഘടന romfile_s *prev);
ഘടന romfile_s *romfile_find(സ്ഥിരം അക്ഷരം *name);
ശൂന്യം *romfile_loadfile(സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ *psize);
u64 romfile_loadint(സ്ഥിരം അക്ഷരം *name, u64 defval);
u32 romfile_loadbool(സ്ഥിരം അക്ഷരം *name, u32 defval);

ശൂന്യം const_romfile_add_int(അക്ഷരം *name, u32 മൂല്യം);

#endif // romfile.h
