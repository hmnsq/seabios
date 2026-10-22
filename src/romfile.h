#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ROMFILE_H
#ವ್ಯಾಖ್ಯಾನಿಸು __ROMFILE_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

// romfile.c
ರಚನೆ romfile_s {
    ರಚನೆ romfile_s *ಮುಂದಿನ;
    ಅಕ್ಷರ name[128];
    u32 ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ (*copy)(ರಚನೆ romfile_s *file, ಶೂನ್ಯ *dest, u32 maxlen);
};
ಶೂನ್ಯ romfile_ಕೂಡಿಸು(ರಚನೆ romfile_s *file);
ರಚನೆ romfile_s *romfile_findprefix(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ರಚನೆ romfile_s *prev);
ರಚನೆ romfile_s *romfile_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name);
ಶೂನ್ಯ *romfile_loadfile(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ *psize);
u64 romfile_loadint(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, u64 defval);
u32 romfile_loadbool(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, u32 defval);

ಶೂನ್ಯ const_romfile_ಕೂಡಿಸು_int(ಅಕ್ಷರ *name, u32 ಮೌಲ್ಯ);

#ಆದರೆ_ಅಂತ್ಯ // romfile.h
