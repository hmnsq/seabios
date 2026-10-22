#यदि_अपरिभाषितम् __ROMFILE_H
#परिभाषय __ROMFILE_H

#अन्तर्भाव "types.h" // u32

// romfile.c
संरचना romfile_s {
    संरचना romfile_s *अग्र;
    वर्ण name[128];
    u32 आकार;
    पूर्णाङ्क (*copy)(संरचना romfile_s *file, शून्य *dest, u32 maxlen);
};
शून्य romfile_सङ्कलय(संरचना romfile_s *file);
संरचना romfile_s *romfile_findprefix(ध्रुव वर्ण *उपसर्गः, संरचना romfile_s *पूर्व);
संरचना romfile_s *romfile_find(ध्रुव वर्ण *name);
शून्य *romfile_loadfile(ध्रुव वर्ण *name, पूर्णाङ्क *psize);
u64 romfile_loadint(ध्रुव वर्ण *name, u64 defval);
u32 romfile_loadbool(ध्रुव वर्ण *name, u32 defval);

शून्य const_romfile_सङ्कलय_int(वर्ण *name, u32 मानम्);

#यदि_अन्तः // romfile.h
