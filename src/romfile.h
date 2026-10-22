#अगर_अघोषित __ROMFILE_H
#घोषणा __ROMFILE_H

#समावेश "types.h" // u32

// romfile.c
काष्ठा romfile_s {
    काष्ठा romfile_s *अग्र;
    अक्षर name[128];
    u32 आकार;
    पूर्णांक (*copy)(काष्ठा romfile_s *file, व्योम *dest, u32 maxlen);
};
व्योम romfile_add(काष्ठा romfile_s *file);
काष्ठा romfile_s *romfile_findprefix(स्थिर अक्षर *prefix, काष्ठा romfile_s *पूर्व);
काष्ठा romfile_s *romfile_find(स्थिर अक्षर *name);
व्योम *romfile_loadfile(स्थिर अक्षर *name, पूर्णांक *psize);
u64 romfile_loadint(स्थिर अक्षर *name, u64 defval);
u32 romfile_loadbool(स्थिर अक्षर *name, u32 defval);

व्योम const_romfile_add_int(अक्षर *name, u32 मूल्य);

#पूर्ण_अगर // romfile.h
