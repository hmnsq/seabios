// Access to pseudo "file" interface for configuration information.
//
// Copyright (C) 2012  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // struct romfile_s
#ಅನ್ತರ್ಭಾವ "string.h" // memcmp

ಸ್ಥಿರ ರಚನೆ romfile_s *Romfile_ಬೇರು VARVERIFY32INIT;

ಶೂನ್ಯ
romfile_ಕೂಡಿಸು(ರಚನೆ romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->ಗಾತ್ರ_2);
    file->ಮುಂದಿನ = Romfile_ಬೇರು;
    Romfile_ಬೇರು = file;
}

// Search for the specified file.
ಸ್ಥಿರ ರಚನೆ romfile_s *
__romfile_findprefix(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ಪೂರ್ಣಾಂಕ prefixlen, ರಚನೆ romfile_s *prev)
{
    ರಚನೆ romfile_s *cur = Romfile_ಬೇರು;
    ಆದರೆ (prev)
        cur = prev->ಮುಂದಿನ;
    ಆಗ (cur) {
        ಆದರೆ (memcmp(ಪೂರ್ವಪ್ರತ್ಯಯ, cur->name, prefixlen) == 0)
            ಹಿಂತಿರುಗಿ cur;
        cur = cur->ಮುಂದಿನ;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ romfile_s *
romfile_findprefix(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ರಚನೆ romfile_s *prev)
{
    ಹಿಂತಿರುಗಿ __romfile_findprefix(ಪೂರ್ವಪ್ರತ್ಯಯ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಪೂರ್ವಪ್ರತ್ಯಯ), prev);
}

ರಚನೆ romfile_s *
romfile_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
    ಹಿಂತಿರುಗಿ __romfile_findprefix(name, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + 1, ಖಾಲಿ);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
ಶೂನ್ಯ *
romfile_loadfile(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ *psize)
{
    ರಚನೆ romfile_s *file = romfile_ಕಂಡುಹಿಡಿ(name);
    ಆದರೆ (!file)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    ಪೂರ್ಣಾಂಕ filesize = file->ಗಾತ್ರ_2;
    ಆದರೆ (!filesize)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    ಅಕ್ಷರ *ದತ್ತಾಂಶ = malloc_tmphigh(filesize+1);
    ಆದರೆ (!ದತ್ತಾಂಶ) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    ಪೂರ್ಣಾಂಕ ret = file->copy(file, ದತ್ತಾಂಶ, filesize);
    ಆದರೆ (ret < 0) {
        ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಆದರೆ (psize)
        *psize = filesize;
    ದತ್ತಾಂಶ[filesize] = '\0';
    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, u64 defval)
{
    ರಚನೆ romfile_s *file = romfile_ಕಂಡುಹಿಡಿ(name);
    ಆದರೆ (!file)
        ಹಿಂತಿರುಗಿ defval;

    ಪೂರ್ಣಾಂಕ filesize = file->ಗಾತ್ರ_2;
    ಆದರೆ (!filesize || filesize > ಗಾತ್ರ(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ಹಿಂತಿರುಗಿ defval;

    u64 val = 0;
    ಪೂರ್ಣಾಂಕ ret = file->copy(file, &val, ಗಾತ್ರ(val));
    ಆದರೆ (ret < 0)
        ಹಿಂತಿರುಗಿ defval;
    ಹಿಂತಿರುಗಿ val;
}

u32
romfile_loadbool(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, u32 defval)
{
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *true_ಅಕ್ಷರಸರಣಿಗಳು[] = { "1", "on", "yes" };
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *false_ಅಕ್ಷರಸರಣಿಗಳು[] = { "0", "off", "no" };
    ಅಕ್ಷರ *str = romfile_loadfile(name, ಖಾಲಿ);
    ಪೂರ್ಣಾಂಕ i;

    ಆದರೆ (!str)
        ಹಿಂತಿರುಗಿ defval;

    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(true_ಅಕ್ಷರಸರಣಿಗಳು); i++)
        ಆದರೆ (0 == strcmp(str, true_ಅಕ್ಷರಸರಣಿಗಳು[i]))
            ಹಿಂತಿರುಗಿ 1;

    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(false_ಅಕ್ಷರಸರಣಿಗಳು); i++)
        ಆದರೆ (0 == strcmp(str, false_ಅಕ್ಷರಸರಣಿಗಳು[i]))
            ಹಿಂತಿರುಗಿ 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ಹಿಂತಿರುಗಿ defval;
}

ರಚನೆ const_romfile_s {
    ರಚನೆ romfile_s file;
    ಶೂನ್ಯ *ದತ್ತಾಂಶ;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
const_read_file(ರಚನೆ romfile_s *file, ಶೂನ್ಯ *dst, u32 maxlen)
{
    ಆದರೆ (file->ಗಾತ್ರ_2 > maxlen)
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ const_romfile_s *cfile;
    cfile = container_ದ(file, ರಚನೆ const_romfile_s, file);
    ಆದರೆ (maxlen > file->ಗಾತ್ರ_2)
        maxlen = file->ಗಾತ್ರ_2;
    ಸ್ಮರಣೆ_ನಕಲು(dst, cfile->ದತ್ತಾಂಶ, maxlen);
    ಹಿಂತಿರುಗಿ file->ಗಾತ್ರ_2;
}

ಸ್ಥಿರ ಶೂನ್ಯ
const_romfile_ಕೂಡಿಸು(ಅಕ್ಷರ *name, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
    ರಚನೆ const_romfile_s *cfile = malloc_tmp(ಗಾತ್ರ(*cfile));
    ಆದರೆ (!cfile) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(cfile, 0, ಗಾತ್ರ(*cfile));
    strtcpy(cfile->file.name, name, ಗಾತ್ರ(cfile->file.name));
    cfile->file.ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
    cfile->file.copy = const_read_file;
    cfile->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
    romfile_ಕೂಡಿಸು(&cfile->file);
}

ಶೂನ್ಯ
const_romfile_ಕೂಡಿಸು_int(ಅಕ್ಷರ *name, u32 ಮೌಲ್ಯ)
{
    u32 *ದತ್ತಾಂಶ = malloc_tmp(ಗಾತ್ರ(*ದತ್ತಾಂಶ));
    ಆದರೆ (!ದತ್ತಾಂಶ) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    *ದತ್ತಾಂಶ = ಮೌಲ್ಯ;
    const_romfile_ಕೂಡಿಸು(name, ದತ್ತಾಂಶ, ಗಾತ್ರ(*ದತ್ತಾಂಶ));
}
