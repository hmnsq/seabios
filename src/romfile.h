#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ROMFILE_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __ROMFILE_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

// romfile.c
ਬਣਤਰ romfile_s {
    ਬਣਤਰ romfile_s *ਅਗਲਾ;
    ਅੱਖਰ name[128];
    u32 ਮਾਪ;
    ਪੂਰਨ (*copy)(ਬਣਤਰ romfile_s *file, ਖਾਲੀ *dest, u32 maxlen);
};
ਖਾਲੀ romfile_ਜੋੜੋ(ਬਣਤਰ romfile_s *file);
ਬਣਤਰ romfile_s *romfile_findprefix(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਬਣਤਰ romfile_s *prev);
ਬਣਤਰ romfile_s *romfile_ਲੱਭੋ(ਅਚਲ ਅੱਖਰ *name);
ਖਾਲੀ *romfile_loadfile(ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ *psize);
u64 romfile_loadint(ਅਚਲ ਅੱਖਰ *name, u64 defval);
u32 romfile_loadbool(ਅਚਲ ਅੱਖਰ *name, u32 defval);

ਖਾਲੀ const_romfile_ਜੋੜੋ_int(ਅੱਖਰ *name, u32 ਮੁੱਲ);

#ਜੇ_ਅੰਤ // romfile.h
