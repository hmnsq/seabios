#ifndef __ROMFILE_H
#define __ROMFILE_H

#அந்தர்பாவ "types.h" // u32

// romfile.c
கட்டமைப்பு romfile_s {
    கட்டமைப்பு romfile_s *next;
    எழுத்து name[128];
    u32 size;
    முழு (*copy)(கட்டமைப்பு romfile_s *file, வெறுமை *dest, u32 maxlen);
};
வெறுமை romfile_add(கட்டமைப்பு romfile_s *file);
கட்டமைப்பு romfile_s *romfile_findprefix(மாறிலி எழுத்து *prefix, கட்டமைப்பு romfile_s *prev);
கட்டமைப்பு romfile_s *romfile_find(மாறிலி எழுத்து *name);
வெறுமை *romfile_loadfile(மாறிலி எழுத்து *name, முழு *psize);
u64 romfile_loadint(மாறிலி எழுத்து *name, u64 defval);
u32 romfile_loadbool(மாறிலி எழுத்து *name, u32 defval);

வெறுமை const_romfile_add_int(எழுத்து *name, u32 மதிப்பு);

#endif // romfile.h
