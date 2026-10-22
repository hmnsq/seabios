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

ಸ್ಥಿರ ಸಂರಚನಾ romfile_s *Romfile_ಬೇರು VARVERIFY32INIT;

ಶೂನ್ಯ
romfile_add(ಸಂರಚನಾ romfile_s *file)
{
    dprintf(3, "Add romfile: %s (size=%d)\n", file->name, file->size);
    file->next = Romfile_ಬೇರು;
    Romfile_ಬೇರು = file;
}

// Search for the specified file.
ಸ್ಥಿರ ಸಂರಚನಾ romfile_s *
__romfile_findprefix(ಧ್ರುವ ವರ್ಣ *prefix, ಪೂರ್ಣಾಙ್ಕ prefixlen, ಸಂರಚನಾ romfile_s *prev)
{
    ಸಂರಚನಾ romfile_s *cur = Romfile_ಬೇರು;
    ಯದಿ (prev)
        cur = prev->next;
    ಯಾವತ್ (cur) {
        ಯದಿ (memcmp(prefix, cur->name, prefixlen) == 0)
            ಪ್ರತ್ಯಾಗಮ cur;
        cur = cur->next;
    }
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ romfile_s *
romfile_findprefix(ಧ್ರುವ ವರ್ಣ *prefix, ಸಂರಚನಾ romfile_s *prev)
{
    ಪ್ರತ್ಯಾಗಮ __romfile_findprefix(prefix, strlen(prefix), prev);
}

ಸಂರಚನಾ romfile_s *
romfile_find(ಧ್ರುವ ವರ್ಣ *name)
{
    ಪ್ರತ್ಯಾಗಮ __romfile_findprefix(name, strlen(name) + 1, ರಿಕ್ತ);
}

// Helper function to find, malloc_tmphigh, and copy a romfile.  This
// function adds a trailing zero to the malloc'd copy.
ಶೂನ್ಯ *
romfile_loadfile(ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ *psize)
{
    ಸಂರಚನಾ romfile_s *file = romfile_find(name);
    ಯದಿ (!file)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    ಪೂರ್ಣಾಙ್ಕ filesize = file->size;
    ಯದಿ (!filesize)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    ವರ್ಣ *data = malloc_tmphigh(filesize+1);
    ಯದಿ (!data) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    dprintf(5, "Copying romfile '%s' (len %d)\n", name, filesize);
    ಪೂರ್ಣಾಙ್ಕ ret = file->copy(file, data, filesize);
    ಯದಿ (ret < 0) {
        ಮುಕ್ತ(data);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    ಯದಿ (psize)
        *psize = filesize;
    data[filesize] = '\0';
    ಪ್ರತ್ಯಾಗಮ data;
}

// Attempt to load an integer from the given file - return 'defval'
// if unsuccessful.
u64
romfile_loadint(ಧ್ರುವ ವರ್ಣ *name, u64 defval)
{
    ಸಂರಚನಾ romfile_s *file = romfile_find(name);
    ಯದಿ (!file)
        ಪ್ರತ್ಯಾಗಮ defval;

    ಪೂರ್ಣಾಙ್ಕ filesize = file->size;
    ಯದಿ (!filesize || filesize > ಪರಿಮಾಣ(u64) || (filesize & (filesize-1)))
        // Doesn't look like a valid integer.
        ಪ್ರತ್ಯಾಗಮ defval;

    u64 val = 0;
    ಪೂರ್ಣಾಙ್ಕ ret = file->copy(file, &val, ಪರಿಮಾಣ(val));
    ಯದಿ (ret < 0)
        ಪ್ರತ್ಯಾಗಮ defval;
    ಪ್ರತ್ಯಾಗಮ val;
}

u32
romfile_loadbool(ಧ್ರುವ ವರ್ಣ *name, u32 defval)
{
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *true_strings[] = { "1", "on", "yes" };
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *false_strings[] = { "0", "off", "no" };
    ವರ್ಣ *str = romfile_loadfile(name, ರಿಕ್ತ);
    ಪೂರ್ಣಾಙ್ಕ i;

    ಯದಿ (!str)
        ಪ್ರತ್ಯಾಗಮ defval;

    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(true_strings); i++)
        ಯದಿ (0 == strcmp(str, true_strings[i]))
            ಪ್ರತ್ಯಾಗಮ 1;

    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(false_strings); i++)
        ಯದಿ (0 == strcmp(str, false_strings[i]))
            ಪ್ರತ್ಯಾಗಮ 0;

    dprintf(1, "%s: unknown bool string: %s\n", __func__, str);
    ಪ್ರತ್ಯಾಗಮ defval;
}

ಸಂರಚನಾ const_romfile_s {
    ಸಂರಚನಾ romfile_s file;
    ಶೂನ್ಯ *data;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
const_read_file(ಸಂರಚನಾ romfile_s *file, ಶೂನ್ಯ *dst, u32 maxlen)
{
    ಯದಿ (file->size > maxlen)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ const_romfile_s *cfile;
    cfile = container_of(file, ಸಂರಚನಾ const_romfile_s, file);
    ಯದಿ (maxlen > file->size)
        maxlen = file->size;
    memcpy(dst, cfile->data, maxlen);
    ಪ್ರತ್ಯಾಗಮ file->size;
}

ಸ್ಥಿರ ಶೂನ್ಯ
const_romfile_add(ವರ್ಣ *name, ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ size)
{
    ಸಂರಚನಾ const_romfile_s *cfile = malloc_tmp(ಪರಿಮಾಣ(*cfile));
    ಯದಿ (!cfile) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(cfile, 0, ಪರಿಮಾಣ(*cfile));
    strtcpy(cfile->file.name, name, ಪರಿಮಾಣ(cfile->file.name));
    cfile->file.size = size;
    cfile->file.copy = const_read_file;
    cfile->data = data;
    romfile_add(&cfile->file);
}

ಶೂನ್ಯ
const_romfile_add_int(ವರ್ಣ *name, u32 ಮೌಲ್ಯ)
{
    u32 *data = malloc_tmp(ಪರಿಮಾಣ(*data));
    ಯದಿ (!data) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    *data = ಮೌಲ್ಯ;
    const_romfile_add(name, data, ಪರಿಮಾಣ(*data));
}
