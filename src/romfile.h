#ifndef __ROMFILE_H
#define __ROMFILE_H

#અન્તર્ભાવ "types.h" // u32

// romfile.c
સંરચના romfile_s {
    સંરચના romfile_s *next;
    અક્ષર name[128];
    u32 size;
    પૂર્ણાંક (*copy)(સંરચના romfile_s *file, શૂન્ય *dest, u32 maxlen);
};
શૂન્ય romfile_add(સંરચના romfile_s *file);
સંરચના romfile_s *romfile_findprefix(અચલ અક્ષર *prefix, સંરચના romfile_s *prev);
સંરચના romfile_s *romfile_find(અચલ અક્ષર *name);
શૂન્ય *romfile_loadfile(અચલ અક્ષર *name, પૂર્ણાંક *psize);
u64 romfile_loadint(અચલ અક્ષર *name, u64 defval);
u32 romfile_loadbool(અચલ અક્ષર *name, u32 defval);

શૂન્ય const_romfile_add_int(અક્ષર *name, u32 કિંમત);

#endif // romfile.h
